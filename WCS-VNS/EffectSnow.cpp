// EffectSnow.cpp
// For managing Snow Effects
// Built from scratch SnowEffect.cpp on 06/12/97 by Gary R. Huber
// Copyright 1997 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "Conservatory.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Random.h"
#include "Raster.h"
#include "Texture.h"
#include "GraphData.h"
#include "requester.h"
#include "SnowEditGUI.h"
#include "Render.h"
#include "Database.h"
#include "Security.h"
#include "Lists.h"
#include "VectorNode.h"
#include "VectorPolygon.h"
#include "FeatureConfig.h"

SnowEffectBase::SnowEffectBase(void)
{

SetDefaults();

} // SnowEffectBase::SnowEffectBase

/*===========================================================================*/

void SnowEffectBase::SetDefaults(void)
{

Resolution = (float)90.0;
GeoRast = NULL;
EdgesExist = GradientsExist = 0;
OverlapOK = Floating = 1;

} // SnowEffectBase::SetDefaults

/*===========================================================================*/

void SnowEffectBase::SetFloating(int NewFloating, Project *CurProj)
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
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_SNOW, WCS_EFFECTSSUBCLASS_SNOW, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

} // SnowEffectBase::SetFloating

/*===========================================================================*/

void SnowEffectBase::SetFloating(int NewFloating, float NewResolution)
{
NotifyTag Changes[2];

Changes[1] = 0;

Floating = NewFloating;

if (NewFloating)
	{
	Resolution = NewResolution;
	} // if
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_SNOW, WCS_EFFECTSSUBCLASS_SNOW, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

} // SnowEffectBase::SetFloating

/*===========================================================================*/

GeoRaster *SnowEffectBase::Init(GeneralEffect *EffectList, RenderJoeList *&JL, double N, double S, double E, double W, 
	double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem)
{
double CellSize, NWLat, SELat, NWLon, SELon, CellsNS, CellsWE;
GeoRaster *DrawRast = NULL, *DR, *LastDrawRast, *LastGradRast;
RenderJoeList *RJL;
SnowEffect *MyEffect;
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

EdgesExist = AreThereEdges(EffectList);
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
JL->SortPriority((OverlapOK || GradientsExist) ? WCS_JOELIST_SORTORDER_HILO: WCS_JOELIST_SORTORDER_LOHI);

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
	for (RJL = JL; RJL; RJL = (RenderJoeList *)RJL->Next)
		{
		// skip it if already drawn
		if (! RJL->Drawn)
			{
			if (RJL->Me->TestFlags(WCS_JOEFLAG_HASKIDS) || ! RJL->Me->TestFlags(WCS_JOEFLAG_ACTIVATED))
				{
				continue;
				} // if 
			if (MyEffect = (SnowEffect *)RJL->Effect)
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
		sprintf(Name, "d:\\Frames\\SnowRastTest%1d.iff", Count);
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

} // SnowEffectBase::Init

/*===========================================================================*/

void SnowEffectBase::Destroy(void)
{
GeoRaster *Rast = GeoRast, *NextRast;

while (Rast)
	{
	NextRast = (GeoRaster *)Rast->Next;
	delete (Rast);
	Rast = NextRast;
	} // while
GeoRast = NULL;

} // SnowEffectBase::Destroy

/*===========================================================================*/

short SnowEffectBase::AreThereEdges(GeneralEffect *EffectList)
{

while (EffectList)
	{
	if (EffectList->Enabled && EffectList->HiResEdge)
		return (1);
	EffectList = EffectList->Next;
	} // while

return (0);

} // SnowEffectBase::AreThereEdges

/*===========================================================================*/

short SnowEffectBase::AreThereGradients(GeneralEffect *EffectList)
{

while (EffectList)
	{
	if (EffectList->Enabled && EffectList->UseGradient)
		return (1);
	EffectList = EffectList->Next;
	} // while

return (0);

} // SnowEffectBase::AreThereGradients

/*===========================================================================*/

// note that align to vector textured snows only work if high-res edges are turned on
// since there is no Joe list search otherwise
int SnowEffectBase::Eval(PolygonData *Poly)
{
GeoRaster *Rast = GeoRast, *FirstDataRast = GeoRast;
SnowEffect *Effect;
short Priority = -1000, ValuesRead = 0, Done = 0;
double Lat, Lon, ActualSum;

Lat = Poly->Lat;
Lon = Poly->Lon;
Rand.Seed64BitShift(Poly->LatSeed + WCS_SEEDOFFSET_SNOW, Poly->LonSeed + WCS_SEEDOFFSET_SNOW);
TempSnow = NULL;

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
							Effect = (SnowEffect *)Rast->LUT[0];
							if (Effect->Priority >= Priority)
								{
								if (Effect->RenderJoes->SimpleContained(Lat, Lon))
									{
									Priority = Effect->Priority;
									Wt = Effect->Eco.ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * Resolution * .5);
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
							Effect = (SnowEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
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
					Poly->Snow = TempSnow;
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
							Effect = (SnowEffect *)Rast->LUT[0];
							if (Effect->RenderJoes->SimpleContained(Lat, Lon))
								{
								Wt = Effect->Eco.ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * Resolution * .5);
								SumWt = fabs(Wt);
								Done = 1;
								} // if contained
							} // if gradfill
						else
							{
							Effect = (SnowEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
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
									Poly->Snow = Effect;
									} // if
								else
									Done = 0;
								} // if
							else
								{
								Poly->Snow = Effect;
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
						Effect = (SnowEffect *)Rast->LUT[0];
						if (Effect->Priority >= Priority)
							{
							Priority = Effect->Priority;
							Wt = Effect->Eco.ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * Resolution * .5);
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
						Effect = (SnowEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
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
				Poly->Snow = TempSnow;
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
						Effect = (SnowEffect *)Rast->LUT[0];
						Wt = Effect->Eco.ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * Resolution * .5);
						SumWt = fabs(Wt);
						} // if
					else
						{
						Effect = (SnowEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
						Wt = 1.0;
						SumWt = 1.0;
						} // if
					if (SumWt < 1.0)
						{
						if (Rand.GenPRN() <= SumWt)
							{
							Poly->Snow = Effect;
							Done = 1;
							} // if
						} // if
					else
						{
						Poly->Snow = Effect;
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
		if (OverlapOK)
			{ // find all containing objects until priority changes
			while (Rast)
				{
				if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
					{ // process data
					Effect = (SnowEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
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
				Poly->Snow = TempSnow;
				} // if
			} // if
		else
			{ // find first containing object
			while (Rast)
				{
				if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
					{ // do edge test
					Effect = (SnowEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
					if (Effect->RenderJoes->SimpleContained(Lat, Lon))
						{
						Poly->Snow = Effect;
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
	if (OverlapOK)
		{ // check maps until lower priority is encountered
		while (Rast)
			{
			if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
				{ // process data
				Effect = (SnowEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
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
			Poly->Snow = TempSnow;
			} // if
		} // if overlap
	else
		{ // everything in one map - nice 'n easy
		if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
			{ // process data
			Effect = (SnowEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
			Poly->Snow = Effect;
			Done = 1;
			} // if a value in edge map
		} // else
	} // if

return (Done > 0);

} // SnowEffectBase::Eval

/*===========================================================================*/

short SnowEffectBase::Compare(SnowEffect *Effect, short ValuesRead)
{

if (ValuesRead <= 1 || Rand.GenPRN() <= (1.0 / ValuesRead))
	{
	TempSnow = Effect;
	return (1);
	} // if

return (0);

} // SnowEffectBase::Compare

/*===========================================================================*/

short SnowEffectBase::CompareWeighted(SnowEffect *Effect)
{

if (Wt >= SumWt || (SumWt > 0.0 && Rand.GenPRN() <= fabs(Wt) / SumWt))
	{
	TempSnow = Effect;
	return (1);
	} // if

return (0);

} // SnowEffectBase::CompareWeighted

/*===========================================================================*/

// VNS 3 version
bool SnowEffectBase::Eval(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode, SnowEffect *SnowList, double PlowedSnow)
{
double DistanceToVector, dX, dY, dZ, ProfWt, AccumulatedWt = 0.0, SnowMatWt, SnowCoverage, RemainingSnow;
long EffectCt, CurCt;
EffectJoeList *CurList;
SnowEffect *CurEffect;
bool Done = false, Success = true;
short TopPriority = -10000;
TwinMaterials MatTwin;

// build comprehensive list of the effects to evaluate

for (EffectCt = 0, CurList = MyEffects; CurList; CurList = CurList->Next)
	{
	if (CurList->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_SNOW)
		EffectsLib::EffectChain[EffectCt++] = CurList;
	} // for

EffectsLib::SortEffectChain(EffectCt, WCS_EFFECTSSUBCLASS_SNOW);
SumWt = 0.0;
RemainingSnow = PlowedSnow < 1.0 ? 1.0 - PlowedSnow: 0.0;

for (CurCt = 0; CurCt < EffectCt; ++CurCt)
	{
	if (CurList = EffectsLib::EffectChain[CurCt])
		{
		CurEffect = (SnowEffect *)CurList->MyEffect;
		if (CurEffect->Priority < TopPriority)
			break;
		if (CurList->VSData)
			{
			for (TfxDetail *CurVS = CurList->VSData; CurVS; CurVS = CurVS->Next)
				{
				} // for
			} // if
		else
			{
			if (CurNode->NodeData->Slope >= CurEffect->Eco.MinSlope &&	// both in degrees
				CurNode->NodeData->Slope <= CurEffect->Eco.MaxSlope)
				{
				if (CurNode->NodeData->RelEl >= CurEffect->Eco.MinRelEl &&
					CurNode->NodeData->RelEl <= CurEffect->Eco.MaxRelEl)
					{
					if (CurEffect->UseGradient)
						{
						DistanceToVector = fabs(CurList->MyJoe->MinDistToPoint(CurNode->Lat, CurNode->Lon, CurNode->Elev, 
							Rend->EarthLatScaleMeters, true, Rend->ElevDatum, Rend->Exageration, dX, dY, dZ));
						ProfWt = CurEffect->Eco.ADProf.GetValue(0, DistanceToVector);
						} // if
					else
						ProfWt = 1.0;
						
					if (CurEffect->Eco.EcoMat.GetRenderMaterial(&MatTwin, CurEffect->Eco.GetRenderMatGradientPos(Rend, CurList->MyJoe, CurNode, NULL)))
						{
						Done = true;
						TopPriority = CurEffect->Priority;
						if (MatTwin.Mat[0])
							{
							SnowMatWt = ProfWt * MatTwin.Covg[0] * RemainingSnow;
							AccumulatedWt += SnowMatWt * (1.0 - MatTwin.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
							if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[0], CurEffect, CurList->MyJoe, (float)(SnowMatWt)))
								{
								Success = false;
								break;
								} // if
							} // if
						if (MatTwin.Mat[1])
							{
							SnowMatWt = ProfWt * MatTwin.Covg[1] * RemainingSnow;
							AccumulatedWt += SnowMatWt * (1.0 - MatTwin.Mat[1]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
							if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[1], CurEffect, CurList->MyJoe, (float)(SnowMatWt)))
								{
								Success = false;
								break;
								} // if
							} // if
						} // if
					} // if
				} // if
			} // else
		} // if
	} // for

if (AccumulatedWt < 1.0 && Success)
	{
	// add any snows that are not vector bounded
	for (CurEffect = SnowList; CurEffect && AccumulatedWt < 1.0; CurEffect = (SnowEffect *)CurEffect->Next)
		{
		if (CurEffect->Enabled && ! CurEffect->Search && ! CurEffect->Joes)
			{
			if ((SnowCoverage = CurEffect->EvaluateCoverage(Rend, CurNode) * RemainingSnow) > 0.0)
				{
				if (CurEffect->Eco.EcoMat.GetRenderMaterial(&MatTwin, CurEffect->Eco.GetRenderMatGradientPos(Rend, NULL, CurNode, NULL)))
					{
					Done = true;
					if (MatTwin.Mat[0])
						{
						SnowMatWt = SnowCoverage * MatTwin.Covg[0];
						AccumulatedWt += SnowMatWt * (1.0 - MatTwin.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
						if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[0], CurEffect, NULL, (float)(SnowMatWt)))
							{
							Success = false;
							break;
							} // if
						} // if
					if (MatTwin.Mat[1])
						{
						SnowMatWt = SnowCoverage * MatTwin.Covg[1];
						AccumulatedWt += SnowMatWt * (1.0 - MatTwin.Mat[1]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
						if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[1], CurEffect, NULL, (float)(SnowMatWt)))
							{
							Success = false;
							break;
							} // if
						} // if
					} // if
				} // if
			} // if
		} // for
	} // if
	
return (Success);

} // SnowEffectBase::Eval

/*===========================================================================*/

bool SnowEffectBase::AddDefaultSnow(RenderData *Rend, VectorNode *CurNode, SnowEffect *SnowList, double PlowedSnow)
{
double SnowCoverage, SnowMatWt, RemainingSnow, AccumulatedWt = 0.0;
SnowEffect *CurEffect;
bool Done = false, Success = true;
TwinMaterials MatTwin;

RemainingSnow = PlowedSnow < 1.0 ? 1.0 - PlowedSnow: 0.0;

// add the whatever snow is lying around
for (CurEffect = SnowList; CurEffect && AccumulatedWt < 1.0; CurEffect = (SnowEffect *)CurEffect->Next)
	{
	if (CurEffect->Enabled && ! CurEffect->Search && ! CurEffect->Joes)
		{
		if ((SnowCoverage = CurEffect->EvaluateCoverage(Rend, CurNode) * RemainingSnow) > 0.0)
			{
			if (CurEffect->Eco.EcoMat.GetRenderMaterial(&MatTwin, CurEffect->Eco.GetRenderMatGradientPos(Rend, NULL, CurNode, NULL)))
				{
				Done = true;
				if (MatTwin.Mat[0])
					{
					SnowMatWt = SnowCoverage * MatTwin.Covg[0];
					AccumulatedWt += SnowMatWt * (1.0 - MatTwin.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
					if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[0], CurEffect, NULL, (float)(SnowMatWt)))
						{
						Success = false;
						break;
						} // if
					} // if
				if (MatTwin.Mat[1])
					{
					SnowMatWt = SnowCoverage * MatTwin.Covg[1];
					AccumulatedWt += SnowMatWt * (1.0 - MatTwin.Mat[1]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
					if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[1], CurEffect, NULL, (float)(SnowMatWt)))
						{
						Success = false;
						break;
						} // if
					} // if
				} // if
			} // if
		} // if
	} // for
	
return (Success);

} // SnowEffectBase::AddDefaultSnow

/*===========================================================================*/
/*===========================================================================*/

SnowEffect::SnowEffect()
: GeneralEffect(NULL), Eco(this, WCS_EFFECTS_MATERIALTYPE_SNOW)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_SNOW;
SetDefaults();

} // SnowEffect::SnowEffect

/*===========================================================================*/

SnowEffect::SnowEffect(RasterAnimHost *RAHost)
: GeneralEffect(RAHost), Eco(this, WCS_EFFECTS_MATERIALTYPE_SNOW)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_SNOW;
SetDefaults();

} // SnowEffect::SnowEffect

/*===========================================================================*/

SnowEffect::SnowEffect(RasterAnimHost *RAHost, EffectsLib *Library, SnowEffect *Proto)
: GeneralEffect(RAHost), Eco(this, WCS_EFFECTS_MATERIALTYPE_SNOW)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_SNOW;
Prev = Library->LastSnow;
if (Library->LastSnow)
	{
	Library->LastSnow->Next = this;
	Library->LastSnow = this;
	} // if
else
	{
	Library->Snow = Library->LastSnow = this;
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
	strcpy(NameBase, "Snow");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // SnowEffect::SnowEffect

/*===========================================================================*/

SnowEffect::~SnowEffect()
{
long Ct;
RootTexture *DelTex;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->SNG && GlobalApp->GUIWins->SNG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->SNG;
		GlobalApp->GUIWins->SNG = NULL;
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

} // SnowEffect::~SnowEffect

/*===========================================================================*/

void SnowEffect::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_SNOW_NUMANIMPAR] = {0.0, 100.0, 0.0};
double RangeDefaults[WCS_EFFECTS_SNOW_NUMANIMPAR][3] = {1.0, 0.0, .01, FLT_MAX, -FLT_MAX, 10.0, 90.0, -90.0, 1.0};
long Ct;
GradientCritter *CurGrad;

for (Ct = 0; Ct < WCS_EFFECTS_SNOW_NUMANIMPAR; Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (unsigned char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	TexRoot[Ct] = NULL;
	} // for
GlobalGradientsEnabled = 0;
if (CurGrad = Eco.EcoMat.GetActiveNode())
	{
	((MaterialEffect *)CurGrad->GetThing())->DiffuseColor.SetValue3(1.0, 1.0, 1.0);
	((MaterialEffect *)CurGrad->GetThing())->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY].SetValue(1.0);
	((MaterialEffect *)CurGrad->GetThing())->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].SetValue(2.0);
	} // if

AnimPar[WCS_EFFECTS_SNOW_ANIMPAR_FEATHERING].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_SNOW_ANIMPAR_GLOBALSNOWGRAD].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_SNOW_ANIMPAR_GLOBALREFLAT].SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);

} // SnowEffect::SetDefaults

/*===========================================================================*/

void SnowEffect::Copy(SnowEffect *CopyTo, SnowEffect *CopyFrom)
{

CopyTo->GlobalGradientsEnabled = CopyFrom->GlobalGradientsEnabled;
CopyTo->Eco.Copy(&CopyTo->Eco, &CopyFrom->Eco);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // SnowEffect::Copy

/*===========================================================================*/

ULONG SnowEffect::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char TexRootNumber = -1;
#ifdef WCS_BUILD_VNS
char MatchName[WCS_EFFECT_MAXNAMELENGTH];
#endif // WCS_BUILD_VNS

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
					case WCS_EFFECTS_ABSOLUTE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Absolute, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SNOW_GLOBALGRADENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&GlobalGradientsEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SNOW_FEATHERING:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SNOW_ANIMPAR_FEATHERING].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SNOW_GLOBALSNOWGRAD:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SNOW_ANIMPAR_GLOBALSNOWGRAD].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SNOW_GLOBALREFLAT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SNOW_ANIMPAR_GLOBALREFLAT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SNOW_ECOSYSTEM:
						{
						BytesRead = Eco.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SNOW_TEXTUREROOTNUM:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexRootNumber, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures())
							{
							TexRoot[TexRootNumber] = new RootTexture(this, 0, 0, 0);
							} // if
						break;
						}
					case WCS_EFFECTS_SNOW_TEXTUREROOT:
						{
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures() && TexRoot[TexRootNumber])
							BytesRead = TexRoot[TexRootNumber]->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						TexRootNumber = -1;
						break;
						}
					case WCS_EFFECTS_SNOW_TEXFEATHERING:
						{
						if (TexRoot[WCS_EFFECTS_SNOW_TEXTURE_FEATHERING] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_SNOW_TEXTURE_FEATHERING]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
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

} // SnowEffect::Load

/*===========================================================================*/

unsigned long int SnowEffect::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_EFFECTS_SNOW_NUMANIMPAR] = {WCS_EFFECTS_SNOW_FEATHERING,
																WCS_EFFECTS_SNOW_GLOBALSNOWGRAD,
																WCS_EFFECTS_SNOW_GLOBALREFLAT};
unsigned long int TextureItemTag[WCS_EFFECTS_SNOW_NUMTEXTURES] = {WCS_EFFECTS_SNOW_TEXFEATHERING};

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
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SNOW_GLOBALGRADENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&GlobalGradientsEnabled)) == NULL)
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

ItemTag = WCS_EFFECTS_SNOW_ECOSYSTEM + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Eco.Save(ffile))
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
			} /* if ecosystem saved */
		else
			goto WriteError;
		} /* if size written */
	else
		goto WriteError;
	} /* if tag written */
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

} // SnowEffect::Save

/*===========================================================================*/

void SnowEffect::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->SNG)
	{
	delete GlobalApp->GUIWins->SNG;
	}
GlobalApp->GUIWins->SNG = new SnowEditGUI(GlobalApp->AppEffects, GlobalApp->MainProj, GlobalApp->AppDB, this);
if(GlobalApp->GUIWins->SNG)
	{
	GlobalApp->GUIWins->SNG->Open(GlobalApp->MainProj);
	}

} // SnowEffect::Edit

/*===========================================================================*/

char *SnowEffectCritterNames[WCS_EFFECTS_SNOW_NUMANIMPAR] = {"Feathering (%)", "Global Snow Gradient (m/deg)", "Gradient Reference Latitude (deg)"};
char *SnowEffectTextureNames[WCS_EFFECTS_SNOW_NUMTEXTURES] = {"Feathering (%)"};


char *SnowEffect::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (SnowEffectCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (SnowEffectTextureNames[Ct]);
		} // if
	} // for
if (Test == &Eco)
	return ("Snow Ecosystem");

return ("");

} // SnowEffect::GetCritterName

/*===========================================================================*/

char *SnowEffect::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as a Snow Texture! Remove anyway?");

} // SnowEffect::OKRemoveRaster

/*===========================================================================*/

int SnowEffect::FindnRemove3DObjects(Object3DEffect *RemoveMe)
{

return (Eco.FindnRemove3DObjects(RemoveMe));

} // SnowEffect::FindnRemove3DObjects

/*===========================================================================*/

int SnowEffect::GetRAHostAnimated(void)
{

if (GeneralEffect::GetRAHostAnimated())
	return (1);
if (Eco.GetRAHostAnimated())
	return (1);

return (0);

} // SnowEffect::GetRAHostAnimated

/*===========================================================================*/

bool SnowEffect::AnimateMaterials(void)
{

if (GeneralEffect::AnimateMaterials())
	return (true);
if (Eco.AnimateMaterials())
	return (true);

return (false);

} // SnowEffect::AnimateMaterials

/*===========================================================================*/

int SnowEffect::SetToTime(double Time)
{
long Found = 0;

if (GeneralEffect::SetToTime(Time))
	Found = 1;
if (Eco.SetToTime(Time))
	Found = 1;

return (Found);

} // SnowEffect::SetToTime

/*===========================================================================*/

int SnowEffect::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
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
if (! Eco.InitToRender(Opt, Buffers))
	return (0);

return (1);

} // SnowEffect::InitToRender

/*===========================================================================*/

int SnowEffect::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{

if (! Eco.InitFrameToRender(Lib, Rend))
	return (0);
GlobalGradient = AnimPar[WCS_EFFECTS_SNOW_ANIMPAR_GLOBALSNOWGRAD].CurValue;
if (Rend->ExagerateElevLines)
	{
	GlobalGradient *= Rend->Exageration;
	} // if

return (GeneralEffect::InitFrameToRender(Lib, Rend));

} // SnowEffect::InitFrameToRender

/*===========================================================================*/

long SnowEffect::GetKeyFrameRange(double &FirstKey, double &LastKey)
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
if (Eco.GetKeyFrameRange(MinDist, MaxDist))
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

} // SnowEffect::GetKeyFrameRange

/*===========================================================================*/

long SnowEffect::InitImageIDs(long &ImageID)
{
long NumImages = 0;

NumImages += Eco.InitImageIDs(ImageID);
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // SnowEffect::InitImageIDs

/*===========================================================================*/

int SnowEffect::BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves,
	EffectList **Queries, EffectList **Themes, EffectList **Coords)
{

if (! Eco.BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
	return (0);

return (1);

} // SnowEffect::BuildFileComponentsList

/*===========================================================================*/

char SnowEffect::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_EFFECTSSUBCLASS_ECOSYSTEM
	#ifdef WCS_BUILD_VNS
	|| DropType == WCS_EFFECTSSUBCLASS_SEARCHQUERY
	#endif // WCS_BUILD_VNS
	|| DropType == WCS_RAHOST_OBJTYPE_VECTOR)
	return (1);

return (0);

} // SnowEffect::GetRAHostDropOK

/*===========================================================================*/

int SnowEffect::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
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
	if (this != (SnowEffect *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (SnowEffect *)DropSource->DropSource);
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
	Success = Eco.ProcessRAHostDragDrop(DropSource);
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

} // SnowEffect::ProcessRAHostDragDrop

/*===========================================================================*/

char *SnowEffect::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? SnowEffectTextureNames[TexNumber]: (char*)"");

} // SnowEffect::GetTextureName

/*===========================================================================*/

RootTexture *SnowEffect::NewRootTexture(long TexNumber)
{
char ApplyToColor = 0;
char ApplyToDisplace = 0;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // SnowEffect::NewRootTexture

/*===========================================================================*/

unsigned long SnowEffect::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_SNOW | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // SnowEffect::GetRAFlags

/*===========================================================================*/

int SnowEffect::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
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
				case WCS_EFFECTS_SNOW_ANIMPAR_FEATHERING:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_SNOW_TEXTURE_FEATHERING);
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
				case WCS_EFFECTS_SNOW_TEXTURE_FEATHERING:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_SNOW_ANIMPAR_FEATHERING);
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

} // SnowEffect::GetAffiliates

/*===========================================================================*/

RasterAnimHost *SnowEffect::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;
JoeList *CurJoe = Joes;

if (! Current)
	return (&Eco);
if (Current == &Eco)
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

} // SnowEffect::GetRAHostChild

/*===========================================================================*/

void SnowEffect::SetFloating(char NewFloating)
{
DEMBounds CurBounds;
NotifyTag Changes[2];

if (GlobalApp->AppDB->FillDEMBounds(&CurBounds))
	{
	Eco.AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_ELEVLINE].SetValue((CurBounds.HighElev + CurBounds.LowElev) * .5);
	} // if
Changes[0] = MAKE_ID(Eco.GetNotifyClass(), Eco.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Eco.GetRAHostRoot());

} // SnowEffect::SetFloating

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int SnowEffect::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
SnowEffect *CurrentSnow = NULL;
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
					else if (! strnicmp(ReadBuf, "Snow", 8))
						{
						if (CurrentSnow = new SnowEffect(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentSnow->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if Snow
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

if (Success == 1 && CurrentSnow)
	{
	if (OldBoundsLoaded && GlobalApp->AppDB->FillDEMBounds(&CurBounds))
		{
		if (UserMessageYN("Load Snow", "Do you wish the loaded Snow's elevation line\n to be scaled to current DEM elevations?"))
			CurrentSnow->Eco.ScaleToDEMBounds(&OldBounds, &CurBounds);
		} // if
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentSnow);
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

} // SnowEffect::LoadObject

/*===========================================================================*/

int SnowEffect::SaveObject(FILE *ffile, const char *SuppliedFileName)
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

// SnowEffect
strcpy(StrBuf, "Snow");
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
			} // if SnowEffect saved 
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

} // SnowEffect::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::SnowEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
SnowEffect *Current;
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
						BytesRead = ReadBlock(ffile, (char *)&SnowBase.Resolution, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_OVERLAPOK:
						{
						BytesRead = ReadBlock(ffile, (char *)&SnowBase.OverlapOK, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_FLOATING:
						{
						BytesRead = ReadBlock(ffile, (char *)&SnowBase.Floating, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						FloatingLoaded = 1;
						break;
						}
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new SnowEffect(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (SnowEffect *)FindDuplicateByName(Current->EffectType, Current))
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
	if (fabs(SnowBase.Resolution - 90.0) > .0001)
		SnowBase.Floating = 0;
	} // if older file

return (TotalRead);

} // EffectsLib::SnowEffect_Load()

/*===========================================================================*/

ULONG EffectsLib::SnowEffect_Save(FILE *ffile)
{
SnowEffect *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_RESOLUTION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&SnowBase.Resolution)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_OVERLAPOK, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&SnowBase.OverlapOK)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_FLOATING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&SnowBase.Floating)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

Current = Snow;
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
					} // if Snow effect saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (SnowEffect *)Current->Next;
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

} // EffectsLib::SnowEffect_Save()

/*===========================================================================*/
