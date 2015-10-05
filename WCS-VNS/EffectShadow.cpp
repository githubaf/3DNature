// EffectShadow.cpp
// For managing Shadow Effects
// Built from scratch ShadowEffect.cpp on 06/12/97 by Gary R. Huber
// Copyright 1997 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "AppMem.h"
#include "Conservatory.h"
#include "ShadowEditGUI.h"
#include "Project.h"
#include "Types.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Random.h"
#include "Log.h"
#include "Raster.h"
#include "GraphData.h"
#include "requester.h"
#include "Render.h"
#include "Points.h"
#include "Database.h"
#include "Security.h"
#include "Lists.h"
#include "FeatureConfig.h"

ShadowEffectBase::ShadowEffectBase(void)
{

SetDefaults();

} // ShadowEffectBase::ShadowEffectBase

/*===========================================================================*/

void ShadowEffectBase::SetDefaults(void)
{

Resolution = (float)90.0;
GeoRast = NULL;
EdgesExist = GradientsExist = 0;
OverlapOK = Floating = 1;

} // ShadowEffectBase::SetDefaults

/*===========================================================================*/

GeoRaster *ShadowEffectBase::Init(GeneralEffect *EffectList, RenderJoeList *&JL, double N, double S, double E, double W, 
	double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem)
{
double CellSize, NWLat, SELat, NWLon, SELon, CellsNS, CellsWE, TotalCells, SizeMult;
GeoRaster *DrawRast = NULL, *DR, *LastDrawRast, *LastGradRast;
RenderJoeList *RJL;
ShadowEffect *MyEffect;
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

CellSize = Resolution / MetersPerDegLat;

EdgesExist = WCS_SHADOW_HIRESEDGES_ENABLED;	//AreThereEdges(EffectList);
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
JL->SortPriority((WCS_SHADOW_OVERLAP_ENABLED || GradientsExist) ? WCS_JOELIST_SORTORDER_HILO: WCS_JOELIST_SORTORDER_LOHI);

// Start with a fixed resolution and adjust it based on a target amount of memory for this effect type
Resolution = 50.0;
CellSize = Resolution / MetersPerDegLat;

// round out to next even cell size from reference lat/lon 
CellsNS = fabs(WCS_floor((S - RefLat) / CellSize)) + fabs(WCS_ceil((N - RefLat) / CellSize));
CellsWE = fabs(WCS_floor((E - RefLon) / CellSize)) + fabs(WCS_ceil((W - RefLon) / CellSize));

// Desired number of cells yields a WCS_SHADOWEFFECT_MAXCELLS GeoRaster file max.
// That will be one file for edges and at least one, probably two files, for rasters so a total of 
// 3 * WCS_SHADOWEFFECT_MAXCELLS bytes for a typical scenario of two slightly overlapping effect vectors.
TotalCells = CellsNS * CellsWE;
if (TotalCells > WCS_SHADOWEFFECT_MAXCELLS)
	{
	SizeMult = sqrt(TotalCells / WCS_SHADOWEFFECT_MAXCELLS);
	CellSize *= SizeMult;
	Resolution = (float)(SizeMult * Resolution);
	} // if
else if (TotalCells < WCS_SHADOWEFFECT_MINCELLS)
	{
	SizeMult = sqrt(TotalCells / WCS_SHADOWEFFECT_MINCELLS);
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
			if (MyEffect = (ShadowEffect *)RJL->Effect)
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
							if ((DrawRast = DrawRast->PolyRasterCopyFloodByte(MaxMem, UsedMem, WCS_RASTER_EFFECT_BAND_NORMAL, FillVal, WCS_SHADOW_OVERLAP_ENABLED, FALSE, MinX, MinY)) == NULL)
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
						if ((DrawRast = DrawRast->PolyRasterFillByte(MaxMem, UsedMem, RJL->Me, WCS_RASTER_EFFECT_BAND_NORMAL, FillVal, WCS_SHADOW_OVERLAP_ENABLED, FALSE)) == NULL)
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
	UBYTE *Bitmap[3];

	DR = DrawRast;
	DrawRast = GeoRast;
	while (DrawRast)
		{
		Bitmap[0] = Bitmap[1] = Bitmap[2] = DrawRast->ByteMap[0];
		sprintf(Name, "d:\\Frames\\RasterTARastTest%1d.iff", Count);
		saveILBM(24, 0, Bitmap, NULL, 0, 1, 1, (short)DrawRast->Cols, (short)DrawRast->Rows, Name);
		DrawRast = (GeoRaster *)DrawRast->Next;
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

} // ShadowEffectBase::Init

/*===========================================================================*/

void ShadowEffectBase::Destroy(void)
{
GeoRaster *Rast = GeoRast, *NextRast;

while (Rast)
	{
	NextRast = (GeoRaster *)Rast->Next;
	delete (Rast);
	Rast = NextRast;
	} // while
GeoRast = NULL;

} // ShadowEffectBase::Destroy

/*===========================================================================*/

short ShadowEffectBase::AreThereEdges(GeneralEffect *EffectList)
{

while (EffectList)
	{
	if (EffectList->Enabled && EffectList->HiResEdge)
		return (1);
	EffectList = EffectList->Next;
	} // while

return (0);

} // ShadowEffectBase::AreThereEdges

/*===========================================================================*/

short ShadowEffectBase::AreThereGradients(GeneralEffect *EffectList)
{

while (EffectList)
	{
	if (EffectList->Enabled && EffectList->UseGradient)
		return (1);
	EffectList = EffectList->Next;
	} // while

return (0);

} // ShadowEffectBase::AreThereGradients

/*===========================================================================*/

void ShadowEffectBase::SetFloating(int NewFloating, Project *CurProj)
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
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_SHADOW, WCS_EFFECTSSUBCLASS_SHADOW, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);
#endif // WCS_VECPOLY_EFFECTS
} // ShadowEffectBase::SetFloating

/*===========================================================================*/

void ShadowEffectBase::SetFloating(int NewFloating, float NewResolution)
{
#ifndef WCS_VECPOLY_EFFECTS
NotifyTag Changes[2];

Changes[1] = 0;

Floating = (short)NewFloating;

if (NewFloating)
	{
	Resolution = NewResolution;
	} // if
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_SHADOW, WCS_EFFECTSSUBCLASS_SHADOW, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);
#endif // WCS_VECPOLY_EFFECTS
} // ShadowEffectBase::SetFloating

/*===========================================================================*/

// Au garde! No error checking here for speed - be sure to verify if the given
// effect has been initialized before calling the Eval methods.

int ShadowEffectBase::Eval(RenderData *Rend, PolygonData *Poly)
{
double HalfRes, Lat, Lon;
GeoRaster *Rast = GeoRast, *FirstDataRast = GeoRast;
ShadowEffect *Effect;
short Priority = -1000, Done = 0;
unsigned long ShadowFlags;

HalfRes = Resolution * 0.5;
Lat = Poly->Lat;
Lon = Poly->Lon;
TempIntensity = TempOffset = 0.0;


// get value from first map
if (GradientsExist)
	{
	if (EdgesExist)
		{
		Rast = FirstDataRast = (GeoRaster *)GeoRast->Next;
		if (GeoRast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
			{ // search maps for values to do edge test on
			if (WCS_SHADOW_OVERLAP_ENABLED)
				{ // find all containing objects until priority changes
				while (Rast)
					{
					if ((Rast->GradFill && (Rast->Mask = Rast->GetByteCellMask(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon))) || (! Rast->GradFill && Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0))
						{ // process data
						if (Rast->GradFill)
							{
							Effect = (ShadowEffect *)Rast->LUT[0];
							if (Effect->Priority >= Priority)
								{
								if (Effect->RenderJoes->SimpleContained(Lat, Lon))
									{
									Priority = Effect->Priority;
									if (Compare(Rend, Poly, Effect, Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes)))
										{
										Done = 1;
										ShadowFlags = Effect->ShadowFlags;
										} // if
									} // if
								} // if
							else
								break;
							} // if
						else
							{
							Effect = (ShadowEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
							if (Effect->Priority >= Priority)
								{
								if (Effect->RenderJoes->SimpleContained(Lat, Lon))
									{
									Priority = Effect->Priority;
									if (Compare(Rend, Poly, Effect, 1.0))
										{
										Done = 1;
										ShadowFlags = Effect->ShadowFlags;
										} // if
									} // if
								} // if
							else
								break;
							} // else
						} // if a value in map
					Rast = (GeoRaster *)Rast->Next;
					} // while
				} // if OverlapOK
			else
				{ // find first containing object
				while (Rast)
					{
					if ((Rast->GradFill && (Rast->Mask = Rast->GetByteCellMask(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon))) || (! Rast->GradFill && Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0))
						{ // process data
						if (Rast->GradFill)
							{
							Effect = (ShadowEffect *)Rast->LUT[0];
							if (Effect->RenderJoes->SimpleContained(Lat, Lon))
								{
								if (Compare(Rend, Poly, Effect, Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes)))
									{
									Done = 1;
									ShadowFlags = Effect->ShadowFlags;
									} // if
								break;
								} // if
							} // if
						else
							{
							Effect = (ShadowEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
							if (Effect->RenderJoes->SimpleContained(Lat, Lon))
								{
								if (Compare(Rend, Poly, Effect, 1.0))
									{
									Done = 1;
									ShadowFlags = Effect->ShadowFlags;
									} // if
								break;
								} // if
							} // else
						} // if a value in edge map
					Rast = (GeoRaster *)Rast->Next;
					} // while
				} // else no overlap
			if (! Done)
				Done = -1;
			} // if a value in edge map
		} // if hires edges
	if (! Done)
		{
		Rast = FirstDataRast;
		if (WCS_SHADOW_OVERLAP_ENABLED)
			{ // check maps until lower priority is encountered
			while (Rast)
				{
				if ((Rast->GradFill && (Rast->Mask = Rast->GetByteCellMask(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon))) || (! Rast->GradFill && Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0))
					{ // process data
					if (Rast->GradFill)
						{
						Effect = (ShadowEffect *)Rast->LUT[0];
						if (Effect->Priority >= Priority)
							{
							Priority = Effect->Priority;
							if (Compare(Rend, Poly, Effect, Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes)))
								{
								Done = 1;
								ShadowFlags = Effect->ShadowFlags;
								} // if
							} // if
						else
							break;
						} // if
					else
						{
						Effect = (ShadowEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
						if (Effect->Priority >= Priority)
							{
							Priority = Effect->Priority;
							if (Compare(Rend, Poly, Effect, 1.0))
								{
								Done = 1;
								ShadowFlags = Effect->ShadowFlags;
								} // if
							} // if
						else
							break;
						} // if
					} // if a value in map
				Rast = (GeoRaster *)Rast->Next;
				} // while
			} // else if overlap
		else
			{ // search first non-gradient map
			while (Rast)
				{
				if ((Rast->GradFill && (Rast->Mask = Rast->GetByteCellMask(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon))) || (! Rast->GradFill && Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0))
					{ // process data
					if (Rast->GradFill)
						{
						Effect = (ShadowEffect *)Rast->LUT[0];
						if (Compare(Rend, Poly, Effect, Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes)))
							{
							Done = 1;
							ShadowFlags = Effect->ShadowFlags;
							} // if
						break;
						} // if
					else
						{
						Effect = (ShadowEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
						if (Compare(Rend, Poly, Effect, 1.0))
							{
							Done = 1;
							ShadowFlags = Effect->ShadowFlags;
							} // if
						break;
						} // if
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
		if (WCS_SHADOW_OVERLAP_ENABLED)
			{ // find all containing objects until priority changes
			while (Rast)
				{
				if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
					{ // process data
					Effect = (ShadowEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
					if (Effect->Priority >= Priority)
						{
						if (Effect->RenderJoes->SimpleContained(Lat, Lon))
							{
							Priority = Effect->Priority;
							if (Compare(Rend, Poly, Effect, 1.0))
								{
								Done = 1;
								ShadowFlags = Effect->ShadowFlags;
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
			} // if
		else
			{ // find first containing object
			while (Rast)
				{
				if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
					{ // do edge test
					Effect = (ShadowEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
					if (Effect->RenderJoes->SimpleContained(Lat, Lon))
						{
						if (Compare(Rend, Poly, Effect, 1.0))
							{
							Done = 1;
							ShadowFlags = Effect->ShadowFlags;
							} // if
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
	if (WCS_SHADOW_OVERLAP_ENABLED)
		{ // check maps until lower priority is encountered
		while (Rast)
			{
			if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
				{ // process data
				Effect = (ShadowEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
				if (Effect->Priority >= Priority)
					{
					Priority = Effect->Priority;
					if (Compare(Rend, Poly, Effect, 1.0))
						{
						Done = 1;
						ShadowFlags = Effect->ShadowFlags;
						} // if
					} // if equal or higher priority
				else
					break;
				} // if a value in map
			else
				break;
			Rast = (GeoRaster *)Rast->Next;
			} // while
		} // if overlap
	else
		{ // everything in one map - nice 'n easy
		if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
			{ // process data
			Effect = (ShadowEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
			if (Compare(Rend, Poly, Effect, 1.0))
				{
				Done = 1;
				ShadowFlags = Effect->ShadowFlags;
				} // if
			} // if a value in edge map
		} // else
	} // if

if (Done > 0)
	{
	Poly->ShadowFlags = ShadowFlags;
	Poly->ReceivedShadowIntensity = TempIntensity;
	Poly->ShadowOffset = TempOffset;
	} // if

return (Done > 0);

} // ShadowEffectBase::Eval

/*===========================================================================*/

short ShadowEffectBase::Compare(RenderData *Rend, PolygonData *Poly, ShadowEffect *Effect, double ProfWt)
{
double VeryTempIntensity, TexOpacity, Value[3];

VeryTempIntensity = Effect->AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_INTENSITY].CurValue * ProfWt;
Value[0] = Value[1] = Value[2] = 0.0;

if (Effect->TexRoot[WCS_EFFECTS_SHADOW_TEXTURE_INTENSITY] && Effect->TexRoot[WCS_EFFECTS_SHADOW_TEXTURE_INTENSITY]->Enabled)
	{
	if ((TexOpacity = Effect->TexRoot[WCS_EFFECTS_SHADOW_TEXTURE_INTENSITY]->Eval(Value, Rend->TransferTextureData(Poly))) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			// Value[0] has already been diminished by the texture's opacity
			VeryTempIntensity *= (1.0 - TexOpacity + Value[0]);
			} // if
		else
			VeryTempIntensity *= Value[0];
		} // if
	} // if

if (VeryTempIntensity > TempIntensity)
	{
	TempIntensity = VeryTempIntensity;
	TempOffset = Effect->AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_SHADOWOFFSET].CurValue;
	return (1);
	} // if

return (0);

} // ShadowEffectBase::Compare

/*===========================================================================*/
/*===========================================================================*/

ShadowEffect::ShadowEffect()
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_SHADOW;
SetDefaults();

} // ShadowEffect::ShadowEffect

/*===========================================================================*/

ShadowEffect::ShadowEffect(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_SHADOW;
SetDefaults();

} // ShadowEffect::ShadowEffect

/*===========================================================================*/

ShadowEffect::ShadowEffect(RasterAnimHost *RAHost, EffectsLib *Library, ShadowEffect *Proto)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_SHADOW;
Prev = Library->LastShadow;
if (Library->LastShadow)
	{
	Library->LastShadow->Next = this;
	Library->LastShadow = this;
	} // if
else
	{
	Library->Shadow = Library->LastShadow = this;
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
	strcpy(NameBase, "Shadow");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // ShadowEffect::ShadowEffect

/*===========================================================================*/

ShadowEffect::~ShadowEffect()
{
long Ct;
RootTexture *DelTex;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->SHG && GlobalApp->GUIWins->SHG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->SHG;
		GlobalApp->GUIWins->SHG = NULL;
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

} // ShadowEffect::~ShadowEffect

/*===========================================================================*/

void ShadowEffect::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_SHADOW_NUMANIMPAR] = {.75, .1, 1.0, 100.0, 100.0};
double RangeDefaults[WCS_EFFECTS_SHADOW_NUMANIMPAR][3] = {1.0, 0.0, .01, 
														1000.0, 0.0, .01,
														100000.0, 0.0, 1.0, 
														100000.0, 0.0, 1.0, 
														10000.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	TexRoot[Ct] = NULL;
	} // for
ShadowMapWidth = 1024;
UseMapFile = 0;
RegenMapFile = 0;
CastShadows = 0;
ShadowFlags = 0;
ReceiveShadowsTerrain = ReceiveShadowsFoliage = ReceiveShadows3DObject = ReceiveShadowsCloudSM = 1;
ReceiveShadowsVolumetric = 0;
ADProf.SetDefaults(this, (char)GetNumAnimParams());

AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_SHADOWOFFSET].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_SAFEADD].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_SAFESUB].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_MINFOLHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_INTENSITY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_SAFEADD].SetNoNodes(1);
AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_SAFESUB].SetNoNodes(1);
AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_MINFOLHT].SetNoNodes(1);

} // ShadowEffect::SetDefaults

/*===========================================================================*/

void ShadowEffect::Copy(ShadowEffect *CopyTo, ShadowEffect *CopyFrom)
{

CopyTo->ShadowMapWidth = CopyFrom->ShadowMapWidth;
CopyTo->UseMapFile = CopyFrom->UseMapFile;
CopyTo->RegenMapFile = CopyFrom->UseMapFile ? 1: 0;
CopyTo->CastShadows = CopyFrom->CastShadows;
CopyTo->ReceiveShadowsTerrain = CopyFrom->ReceiveShadowsTerrain;
CopyTo->ReceiveShadowsFoliage = CopyFrom->ReceiveShadowsFoliage;
CopyTo->ReceiveShadows3DObject = CopyFrom->ReceiveShadows3DObject;
CopyTo->ReceiveShadowsCloudSM = CopyFrom->ReceiveShadowsCloudSM;
CopyTo->ReceiveShadowsVolumetric = CopyFrom->ReceiveShadowsVolumetric;
ADProf.Copy(&CopyTo->ADProf, &CopyFrom->ADProf);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // ShadowEffect::Copy

/*===========================================================================*/

ULONG ShadowEffect::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char TexRootNumber = -1, TempReceiveShadows;

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
					case WCS_EFFECTS_ABSOLUTE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Absolute, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SHADOW_MINFOLHT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_MINFOLHT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SHADOW_INTENSITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_INTENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SHADOW_SAFEADD:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_SAFEADD].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SHADOW_SAFESUB:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_SAFESUB].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SHADOW_SHADOWOFFSET:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_SHADOWOFFSET].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SHADOW_SHADOWMAPWIDTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&ShadowMapWidth, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						// in V4 width could be anything
						if (ShadowMapWidth > 2048)
							ShadowMapWidth = 4096;
						else if (ShadowMapWidth > 1024)
							ShadowMapWidth = 2048;
						else if (ShadowMapWidth > 512)
							ShadowMapWidth = 1024;
						else
							ShadowMapWidth = 512;
						break;
						}
					case WCS_EFFECTS_SHADOW_CASTSHADOWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&CastShadows, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SHADOW_RECEIVESHADOWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempReceiveShadows, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempReceiveShadows)
							{
							ReceiveShadowsTerrain = ReceiveShadowsFoliage = ReceiveShadows3DObject = 
								ReceiveShadowsCloudSM = 1;
							ReceiveShadowsVolumetric = 0;
							} // if
						break;
						}
					case WCS_EFFECTS_SHADOW_RECEIVESHADOWSTER:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsTerrain, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SHADOW_RECEIVESHADOWSFOL:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsFoliage, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SHADOW_RECEIVESHADOWS3D:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadows3DObject, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SHADOW_RECEIVESHADOWSSM:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsCloudSM, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SHADOW_RECEIVESHADOWSVOL:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsVolumetric, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SHADOW_USEMAPFILE:
						{
						BytesRead = ReadBlock(ffile, (char *)&UseMapFile, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SHADOW_REGENMAPFILE:
						{
						BytesRead = ReadBlock(ffile, (char *)&RegenMapFile, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SHADOW_TEXTUREROOTNUM:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexRootNumber, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures())
							{
							TexRoot[TexRootNumber] = new RootTexture(this, 0, 0, 0);
							} // if
						break;
						}
					case WCS_EFFECTS_SHADOW_TEXTUREROOT:
						{
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures() && TexRoot[TexRootNumber])
							BytesRead = TexRoot[TexRootNumber]->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						TexRootNumber = -1;
						break;
						}
					case WCS_EFFECTS_SHADOW_TEXINTENSITY:
						{
						if (TexRoot[WCS_EFFECTS_SHADOW_TEXTURE_INTENSITY] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_SHADOW_TEXTURE_INTENSITY]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_SHADOW_PROFILE:
						{
						BytesRead = ADProf.Load(ffile, Size, ByteFlip);
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

return (TotalRead);

} // ShadowEffect::Load

/*===========================================================================*/

unsigned long int ShadowEffect::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_EFFECTS_SHADOW_NUMANIMPAR] = {WCS_EFFECTS_SHADOW_INTENSITY,
																WCS_EFFECTS_SHADOW_SHADOWOFFSET,
																WCS_EFFECTS_SHADOW_MINFOLHT,
																WCS_EFFECTS_SHADOW_SAFEADD,
																WCS_EFFECTS_SHADOW_SAFESUB};
unsigned long int TextureItemTag[WCS_EFFECTS_SHADOW_NUMTEXTURES] = {WCS_EFFECTS_SHADOW_TEXINTENSITY};

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
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ABSOLUTE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Absolute)) == NULL)
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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SHADOW_CASTSHADOWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&CastShadows)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SHADOW_RECEIVESHADOWSTER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsTerrain)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SHADOW_RECEIVESHADOWSFOL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsFoliage)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SHADOW_RECEIVESHADOWS3D, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadows3DObject)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SHADOW_RECEIVESHADOWSSM, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsCloudSM)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SHADOW_RECEIVESHADOWSVOL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsVolumetric)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SHADOW_SHADOWMAPWIDTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShadowMapWidth)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SHADOW_USEMAPFILE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&UseMapFile)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SHADOW_REGENMAPFILE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&RegenMapFile)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_EFFECTS_SHADOW_PROFILE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // ShadowEffect::Save

/*===========================================================================*/

void ShadowEffect::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->SHG)
	{
	delete GlobalApp->GUIWins->SHG;
	}
GlobalApp->GUIWins->SHG = new ShadowEditGUI(GlobalApp->AppEffects, GlobalApp->MainProj, GlobalApp->AppDB, this);
if(GlobalApp->GUIWins->SHG)
	{
	GlobalApp->GUIWins->SHG->Open(GlobalApp->MainProj);
	}

} // ShadowEffect::Edit

/*===========================================================================*/

short ShadowEffect::AnimateShadows(void)
{

return (0);

} // ShadowEffect::AnimateShadows

/*===========================================================================*/

char *ShadowEffectEffectCritterNames[WCS_EFFECTS_SHADOW_NUMANIMPAR] = {"Shadow Intensity (%)", "Shadow Offset (m)", "Minimum Foliage Height (m)", "Elevation Above (m)", "Elevation Below (m)"};
char *ShadowEffectTextureNames[WCS_EFFECTS_SHADOW_NUMTEXTURES] = {"Shadow Intensity (%)"};

char *ShadowEffect::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (ShadowEffectEffectCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (ShadowEffectTextureNames[Ct]);
		} // if
	} // for
if (Test == &ADProf)
	return ("Edge Feathering Profile");

return ("");

} // ShadowEffect::GetCritterName

/*===========================================================================*/

char *ShadowEffect::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as a Shadow Effect Texture! Remove anyway?");

} // ShadowEffect::OKRemoveRaster

/*===========================================================================*/

char *ShadowEffect::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? ShadowEffectTextureNames[TexNumber]: (char*)"");

} // ShadowEffect::GetTextureName

/*===========================================================================*/

RootTexture *ShadowEffect::NewRootTexture(long TexNumber)
{
char ApplyToColor = 0;
char ApplyToDisplace = 0;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // ShadowEffect::NewRootTexture

/*===========================================================================*/

char ShadowEffect::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_VECTOR
	|| DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE)
	return (1);

return (0);

} // ShadowEffect::GetRAHostDropOK

/*===========================================================================*/

int ShadowEffect::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
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
	if (this != (ShadowEffect *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (ShadowEffect *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE)
	{
	Success = ADProf.ProcessRAHostDragDrop(DropSource);
	} // else if
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

} // ShadowEffect::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long ShadowEffect::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_SHADOW | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // ShadowEffect::GetRAFlags

/*===========================================================================*/

int ShadowEffect::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
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
				case WCS_EFFECTS_SHADOW_ANIMPAR_INTENSITY:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_SHADOW_TEXTURE_INTENSITY);
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
				case WCS_EFFECTS_SHADOW_TEXTURE_INTENSITY:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_SHADOW_ANIMPAR_INTENSITY);
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
			return (1);
			} // if
		} // for
	} // else if

return (0);

} // ShadowEffect::GetAffiliates

/*===========================================================================*/

RasterAnimHost *ShadowEffect::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;
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
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Found && GetTexRootPtr(Ct))
		return (GetTexRootPtr(Ct));
	if (Current == GetTexRootPtr(Ct))
		Found = 1;
	} // for
while (CurJoe)
	{
	if (Found && CurJoe->Me)
		return (CurJoe->Me);
	if (Current == CurJoe->Me)
		Found = 1;
	CurJoe = CurJoe->Next;
	} // while

return (NULL);

} // ShadowEffect::GetRAHostChild

/*===========================================================================*/

double ShadowEffect::GetMaxProfileDistance(void)
{
double FindMin, FindMax;

if (ADProf.GetMinMaxDist(FindMin, FindMax))
	return (FindMax);

return (0.0);

} // ShadowEffect::GetMaxProfileDistance

/*===========================================================================*/

int ShadowEffect::ProjectBounds(Camera *Cam, RenderData *Rend, Joe *CurJoe, 
	double *LatBounds, double *LonBounds, double *ElevBounds,
	double *LimitsX, double *LimitsY, double *LimitsZ)
{
long Ct, Ct1, Ct2, Found = 0;
double ElevStash;
VectorPoint *PLink;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM MyVert;

LimitsX[0] = LimitsY[0] = LimitsZ[0] = -FLT_MAX;
LimitsX[1] = LimitsY[1] = LimitsZ[1] = FLT_MAX;

if (CurJoe)
	{
	if (PLink = CurJoe->GetFirstRealPoint())
		{
		if (MyAttr = (JoeCoordSys *)CurJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
			MyCoords = MyAttr->Coord;
		else
			MyCoords = NULL;
		for (; PLink; PLink = PLink->Next)
			{
			if (PLink->ProjToDefDeg(MyCoords, &MyVert))
				{
				MyVert.Elev = (MyVert.Elev - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
				ElevStash = MyVert.Elev;
				MyVert.Elev += AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_SAFEADD].CurValue;
				Cam->ProjectVertexDEM(Rend->DefCoords, &MyVert, Rend->EarthLatScaleMeters, Rend->PlanetRad, 1);
				if (MyVert.ScrnXYZ[2] > 0.0)
					{
					if (MyVert.ScrnXYZ[0] > LimitsX[0])
						LimitsX[0] = MyVert.ScrnXYZ[0];
					if (MyVert.ScrnXYZ[0] < LimitsX[1])
						LimitsX[1] = MyVert.ScrnXYZ[0];
					if (MyVert.ScrnXYZ[1] > LimitsY[0])
						LimitsY[0] = MyVert.ScrnXYZ[1];
					if (MyVert.ScrnXYZ[1] < LimitsY[1])
						LimitsY[1] = MyVert.ScrnXYZ[1];
					if (MyVert.ScrnXYZ[2] > LimitsZ[0])
						LimitsZ[0] = MyVert.ScrnXYZ[2];
					if (MyVert.ScrnXYZ[2] < LimitsZ[1])
						LimitsZ[1] = MyVert.ScrnXYZ[2];
					Found ++;
					} // if
				if (AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_SAFEADD].CurValue != -AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_SAFESUB].CurValue)
					{
					MyVert.Elev = ElevStash;
					MyVert.Elev -= AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_SAFESUB].CurValue;
					Cam->ProjectVertexDEM(Rend->DefCoords, &MyVert, Rend->EarthLatScaleMeters, Rend->PlanetRad, 1);
					if (MyVert.ScrnXYZ[2] > 0.0)
						{
						if (MyVert.ScrnXYZ[0] > LimitsX[0])
							LimitsX[0] = MyVert.ScrnXYZ[0];
						if (MyVert.ScrnXYZ[0] < LimitsX[1])
							LimitsX[1] = MyVert.ScrnXYZ[0];
						if (MyVert.ScrnXYZ[1] > LimitsY[0])
							LimitsY[0] = MyVert.ScrnXYZ[1];
						if (MyVert.ScrnXYZ[1] < LimitsY[1])
							LimitsY[1] = MyVert.ScrnXYZ[1];
						if (MyVert.ScrnXYZ[2] > LimitsZ[0])
							LimitsZ[0] = MyVert.ScrnXYZ[2];
						if (MyVert.ScrnXYZ[2] < LimitsZ[1])
							LimitsZ[1] = MyVert.ScrnXYZ[2];
						Found ++;
						} // if
					} // if
				} // if
			else
				return (0);
			} // for
		} // if points
	else
		{
		LimitsX[0] = LimitsY[0] = LimitsZ[0] = 0.0;
		LimitsX[1] = LimitsY[1] = LimitsZ[1] = 0.0;
		} // else
	} // if Joe
else
	{
	for (Ct = 0; Ct < 2; Ct ++)
		{
		MyVert.Lat = LatBounds[Ct];
		for (Ct1 = 0; Ct1 < 2; Ct1 ++)
			{
			MyVert.Lon = LonBounds[Ct1];
			for (Ct2 = 0; Ct2 < 2; Ct2 ++)
				{
				if (Ct2 && ElevBounds[0] == ElevBounds[1])
					break;
				MyVert.Elev = ElevBounds[Ct2];
				MyVert.Elev = (MyVert.Elev - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
				Cam->ProjectVertexDEM(Rend->DefCoords, &MyVert, Rend->EarthLatScaleMeters, Rend->PlanetRad, 1);
				if (MyVert.ScrnXYZ[2] > 0.0)
					{
					if (MyVert.ScrnXYZ[0] > LimitsX[0])
						LimitsX[0] = MyVert.ScrnXYZ[0];
					if (MyVert.ScrnXYZ[0] < LimitsX[1])
						LimitsX[1] = MyVert.ScrnXYZ[0];
					if (MyVert.ScrnXYZ[1] > LimitsY[0])
						LimitsY[0] = MyVert.ScrnXYZ[1];
					if (MyVert.ScrnXYZ[1] < LimitsY[1])
						LimitsY[1] = MyVert.ScrnXYZ[1];
					if (MyVert.ScrnXYZ[2] > LimitsZ[0])
						LimitsZ[0] = MyVert.ScrnXYZ[2];
					if (MyVert.ScrnXYZ[2] < LimitsZ[1])
						LimitsZ[1] = MyVert.ScrnXYZ[2];
					Found ++;
					} // if
				} // for
			} // for
		} // for
	MyVert.Lat = (LatBounds[0] + LatBounds[1]) * 0.5;
	MyVert.Lon = (LonBounds[0] + LonBounds[1]) * 0.5;
	MyVert.Elev = ElevBounds[0];
	MyVert.Elev = (MyVert.Elev - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
	Cam->ProjectVertexDEM(Rend->DefCoords, &MyVert, Rend->EarthLatScaleMeters, Rend->PlanetRad, 1);
	if (MyVert.ScrnXYZ[2] > 0.0)
		{
		if (MyVert.ScrnXYZ[0] > LimitsX[0])
			LimitsX[0] = MyVert.ScrnXYZ[0];
		if (MyVert.ScrnXYZ[0] < LimitsX[1])
			LimitsX[1] = MyVert.ScrnXYZ[0];
		if (MyVert.ScrnXYZ[1] > LimitsY[0])
			LimitsY[0] = MyVert.ScrnXYZ[1];
		if (MyVert.ScrnXYZ[1] < LimitsY[1])
			LimitsY[1] = MyVert.ScrnXYZ[1];
		if (MyVert.ScrnXYZ[2] > LimitsZ[0])
			LimitsZ[0] = MyVert.ScrnXYZ[2];
		if (MyVert.ScrnXYZ[2] < LimitsZ[1])
			LimitsZ[1] = MyVert.ScrnXYZ[2];
		Found ++;
		} // if
	if (ElevBounds[0] != ElevBounds[1])
		{
		MyVert.Elev = ElevBounds[1];
		MyVert.Elev = (MyVert.Elev - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
		Cam->ProjectVertexDEM(Rend->DefCoords, &MyVert, Rend->EarthLatScaleMeters, Rend->PlanetRad, 1);
		if (MyVert.ScrnXYZ[2] > 0.0)
			{
			if (MyVert.ScrnXYZ[0] > LimitsX[0])
				LimitsX[0] = MyVert.ScrnXYZ[0];
			if (MyVert.ScrnXYZ[0] < LimitsX[1])
				LimitsX[1] = MyVert.ScrnXYZ[0];
			if (MyVert.ScrnXYZ[1] > LimitsY[0])
				LimitsY[0] = MyVert.ScrnXYZ[1];
			if (MyVert.ScrnXYZ[1] < LimitsY[1])
				LimitsY[1] = MyVert.ScrnXYZ[1];
			if (MyVert.ScrnXYZ[2] > LimitsZ[0])
				LimitsZ[0] = MyVert.ScrnXYZ[2];
			if (MyVert.ScrnXYZ[2] < LimitsZ[1])
				LimitsZ[1] = MyVert.ScrnXYZ[2];
			Found ++;
			} // if
		} // if
	} // else

return (Found > 1);


} // ShadowEffect::ProjectBounds

/*===========================================================================*/

int ShadowEffect::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{
char Ct;

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

ShadowFlags = 0;
if (ReceiveShadowsTerrain)
	ShadowFlags |= WCS_SHADOWTYPE_TERRAIN;
if (ReceiveShadowsFoliage)
	ShadowFlags |= WCS_SHADOWTYPE_FOLIAGE;
if (ReceiveShadows3DObject)
	ShadowFlags |= WCS_SHADOWTYPE_3DOBJECT;
if (ReceiveShadowsCloudSM)
	ShadowFlags |= WCS_SHADOWTYPE_CLOUDSM;
if (ReceiveShadowsVolumetric)
	ShadowFlags |= WCS_SHADOWTYPE_VOLUME;

return (1);

} // ShadowEffect::InitToRender

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int ShadowEffect::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
ShadowEffect *CurrentShadow = NULL;
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
						} // if Images
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
					else if (! strnicmp(ReadBuf, "Shadow", 8))
						{
						if (CurrentShadow = new ShadowEffect(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentShadow->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if 3d object
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

if (Success == 1 && CurrentShadow)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentShadow);
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

} // ShadowEffect::LoadObject

/*===========================================================================*/

int ShadowEffect::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
EffectList *CurEffect, *Queries = NULL, *Themes = NULL, *Coords = NULL;
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

if (GeneralEffect::BuildFileComponentsList(&Queries, &Themes, &Coords))
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
	} // if

// ShadowEffect
strcpy(StrBuf, "Shadow");
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
			} // if ShadowEffect saved 
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

} // ShadowEffect::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::ShadowEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
ShadowEffect *Current;
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
						BytesRead = ReadBlock(ffile, (char *)&ShadowBase.Resolution, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_OVERLAPOK:
						{
						BytesRead = ReadBlock(ffile, (char *)&ShadowBase.OverlapOK, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_FLOATING:
						{
						BytesRead = ReadBlock(ffile, (char *)&ShadowBase.Floating, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						FloatingLoaded = 1;
						break;
						}
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new ShadowEffect(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (ShadowEffect *)FindDuplicateByName(Current->EffectType, Current))
								{
								RemoveRAHost(Current, 1);
								Current = NULL;
								} // if
							}
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))	//lint !e713
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

#ifdef WCS_VECPOLY_EFFECTS
if (! FloatingLoaded)
	{
	// if user has previously modified, turn floating off
	if (fabs(ShadowBase.Resolution - 90.0) > .0001)
		ShadowBase.Floating = 0;
	} // if older file
#endif // WCS_VECPOLY_EFFECTS

return (TotalRead);

} // EffectsLib::ShadowEffect_Load()

/*===========================================================================*/

ULONG EffectsLib::ShadowEffect_Save(FILE *ffile)
{
ShadowEffect *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_RESOLUTION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&ShadowBase.Resolution)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_OVERLAPOK, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShadowBase.OverlapOK)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_FLOATING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShadowBase.Floating)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

Current = Shadow;
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
					} // if illumination effect saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (ShadowEffect *)Current->Next;
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

} // EffectsLib::ShadowEffect_Save()

/*===========================================================================*/
