// EffectGround.cpp
// For managing Ground Effects
// Built from scratch on 03/24/99 by Gary R. Huber
// Copyright 1999 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "AppMem.h"
#include "Conservatory.h"
#include "Project.h"
#include "Types.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Random.h"
#include "Raster.h"
#include "Log.h"
#include "requester.h"
#include "GroundEditGUI.h"
#include "Render.h"
#include "Database.h"
#include "Security.h"
#include "Lists.h"
#include "VectorNode.h"
#include "VectorPolygon.h"
#include "FeatureConfig.h"

GroundEffectBase::GroundEffectBase(void)
{

SetDefaults();

} // GroundEffectBase::GroundEffectBase

/*===========================================================================*/

void GroundEffectBase::SetDefaults(void)
{

Resolution = 90.0f;
GeoRast = NULL;
EdgesExist = GradientsExist = 0;
OverlapOK = Floating = 1;

} // GroundEffectBase::SetDefaults

/*===========================================================================*/

void GroundEffectBase::SetFloating(int NewFloating, Project *CurProj)
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
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_GROUND, WCS_EFFECTSSUBCLASS_GROUND, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

} // GroundEffectBase::SetFloating

/*===========================================================================*/

void GroundEffectBase::SetFloating(int NewFloating, float NewResolution)
{
NotifyTag Changes[2];

Changes[1] = 0;

Floating = NewFloating;

if (NewFloating)
	{
	Resolution = NewResolution;
	} // if
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_GROUND, WCS_EFFECTSSUBCLASS_GROUND, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

} // GroundEffectBase::SetFloating

/*===========================================================================*/

GeoRaster *GroundEffectBase::Init(GeneralEffect *EffectList, RenderJoeList *&JL, double N, double S, double E, double W, 
	double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem)
{
double CellSize, NWLat, SELat, NWLon, SELon, CellsNS, CellsWE;
GeoRaster *DrawRast = NULL, *DR, *LastDrawRast, *LastGradRast;
RenderJoeList *RJL;
GroundEffect *MyEffect;
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
			if (MyEffect = (GroundEffect *)RJL->Effect)
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
	UBYTE *Bitmap[3];

	DR = DrawRast;
	DrawRast = GeoRast;
	while (DrawRast)
		{
		Bitmap[0] = Bitmap[1] = Bitmap[2] = DrawRast->ByteMap[0];
		sprintf(Name, "d:\\Frames\\GroundRastTest%1d.iff", Count);
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

} // GroundEffectBase::Init

/*===========================================================================*/

void GroundEffectBase::Destroy(void)
{
GeoRaster *Rast = GeoRast, *NextRast;

while (Rast)
	{
	NextRast = (GeoRaster *)Rast->Next;
	delete (Rast);
	Rast = NextRast;
	} // while
GeoRast = NULL;

} // GroundEffectBase::Destroy

/*===========================================================================*/

short GroundEffectBase::AreThereEdges(GeneralEffect *EffectList)
{

while (EffectList)
	{
	if (EffectList->Enabled && EffectList->HiResEdge)
		return (1);
	EffectList = EffectList->Next;
	} // while

return (0);

} // GroundEffectBase::AreThereEdges

/*===========================================================================*/

short GroundEffectBase::AreThereGradients(GeneralEffect *EffectList)
{

while (EffectList)
	{
//	if (EffectList->Enabled && EffectList->UseGradient && EffectList->Prof)
	if (EffectList->Enabled && EffectList->UseGradient)
		return (1);
	EffectList = EffectList->Next;
	} // while

return (0);

} // GroundEffectBase::AreThereGradients

/*===========================================================================*/

int GroundEffectBase::Eval(PolygonData *Poly)
{
double HalfRes, Lat, Lon, ActualSum;
GeoRaster *Rast = GeoRast, *FirstDataRast = GeoRast;
GroundEffect *Effect;
short Priority = -1000, ValuesRead = 0, Done = 0;

HalfRes = Resolution * 0.5;
Lat = Poly->Lat;
Lon = Poly->Lon;
Rand.Seed64BitShift(Poly->LatSeed + WCS_SEEDOFFSET_GROUND, Poly->LonSeed + WCS_SEEDOFFSET_GROUND);
TempGround = NULL;

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
							Effect = (GroundEffect *)Rast->LUT[0];
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
							Effect = (GroundEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
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
					Poly->Ground = TempGround;
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
							Effect = (GroundEffect *)Rast->LUT[0];
							if (Effect->RenderJoes->SimpleContained(Lat, Lon))
								{
								Wt = Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes);
								SumWt = fabs(Wt);
								Done = 1;
								} // if contained
							} // if gradfill
						else
							{
							Effect = (GroundEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
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
									Poly->Ground = Effect;
									} // if
								else
									Done = 0;
								} // if
							else
								{
								Poly->Ground = Effect;
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
						Effect = (GroundEffect *)Rast->LUT[0];
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
						Effect = (GroundEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
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
				Poly->Ground = TempGround;
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
						Effect = (GroundEffect *)Rast->LUT[0];
						Wt = Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes);
						SumWt = fabs(Wt);
						} // if
					else
						{
						Effect = (GroundEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
						Wt = 1.0;
						SumWt = 1.0;
						} // if
					if (SumWt < 1.0)
						{
						if (Rand.GenPRN() <= SumWt)
							{
							Poly->Ground = Effect;
							Done = 1;
							} // if
						} // if
					else
						{
						Poly->Ground = Effect;
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
					Effect = (GroundEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
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
				Poly->Ground = TempGround;
				} // if
			} // if
		else
			{ // find first containing object
			while (Rast)
				{
				if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
					{ // do edge test
					Effect = (GroundEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
					if (Effect->RenderJoes->SimpleContained(Lat, Lon))
						{
						Poly->Ground = Effect;
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
				Effect = (GroundEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
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
			Poly->Ground = TempGround;
			} // if
		} // if overlap
	else
		{ // everything in one map - nice 'n easy
		if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
			{ // process data
			Effect = (GroundEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
			Poly->Ground = Effect;
			Done = 1;
			} // if a value in edge map
		} // else
	} // if

return (Done > 0);

} // GroundEffectBase::Eval

/*===========================================================================*/

short GroundEffectBase::Compare(GroundEffect *Effect, short ValuesRead)
{

if (ValuesRead <= 1 || Rand.GenPRN() <= (1.0 / ValuesRead))
	{
	TempGround = Effect;
	return (1);
	} // if

return (0);

} // GroundEffectBase::Compare

/*===========================================================================*/

short GroundEffectBase::CompareWeighted(GroundEffect *Effect)
{

if (Wt >= SumWt || (SumWt > 0.0 && Rand.GenPRN() <= fabs(Wt) / SumWt))
	{
	TempGround = Effect;
	return (1);
	} // if

return (0);

} // GroundEffectBase::CompareWeighted

/*===========================================================================*/

bool GroundEffectBase::Eval(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode, GroundEffect *DefaultGround)
{
double DistanceToVector, dX, dY, dZ, ProfWt, AccumulatedWt = 0.0;
long EffectCt, CurCt;
EffectJoeList *CurList;
GroundEffect *CurEffect;
bool Done = false, Success = true;
short TopPriority = -10000;
TwinMaterials MatTwin;

// build comprehensive list of the effects to evaluate

for (EffectCt = 0, CurList = MyEffects; CurList; CurList = CurList->Next)
	{
	if (CurList->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_GROUND)
		EffectsLib::EffectChain[EffectCt++] = CurList;
	} // for

EffectsLib::SortEffectChain(EffectCt, WCS_EFFECTSSUBCLASS_GROUND);

for (CurCt = 0; CurCt < EffectCt; ++CurCt)
	{
	if (CurList = EffectsLib::EffectChain[CurCt])
		{
		CurEffect = (GroundEffect *)CurList->MyEffect;
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
					AccumulatedWt += ProfWt * MatTwin.Covg[0] * MatTwin.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue;
					if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[0], CurEffect, CurList->MyJoe, (float)(ProfWt * MatTwin.Covg[0])))
						{
						Success = false;
						break;
						} // if
					} // if
				if (MatTwin.Mat[1])
					{
					AccumulatedWt += ProfWt * MatTwin.Covg[1] * MatTwin.Mat[1]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue;
					if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[1], CurEffect, CurList->MyJoe, (float)(ProfWt * MatTwin.Covg[1])))
						{
						Success = false;
						break;
						} // if
					} // if
				} // if
			} // else
		} // if
	} // for

if (AccumulatedWt < 1.0 && Success)
	{
	// add the default ground
	if (DefaultGround->EcoMat.GetRenderMaterial(&MatTwin, DefaultGround->GetRenderMatGradientPos(Rend, NULL, CurNode, NULL)))
		{
		Done = true;
		if (MatTwin.Mat[0])
			{
			if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[0], DefaultGround, NULL, (float)(MatTwin.Covg[0])))
				Success = false;
			} // if
		if (MatTwin.Mat[1] && Success)
			{
			if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[1], DefaultGround, NULL, (float)(MatTwin.Covg[1])))
				Success = false;
			} // if
		} // if
	} // if
	
return (Success);

} // GroundEffectBase::Eval

/*===========================================================================*/

bool GroundEffectBase::AddDefaultGround(RenderData *Rend, VectorNode *CurNode, GroundEffect *DefaultGround)
{
bool Done = false, Success = true;
TwinMaterials MatTwin;

// add the default ground
if (DefaultGround->EcoMat.GetRenderMaterial(&MatTwin, DefaultGround->GetRenderMatGradientPos(Rend, NULL, CurNode, NULL)))
	{
	Done = true;
	if (MatTwin.Mat[0])
		{
		if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[0], DefaultGround, NULL, (float)(MatTwin.Covg[0])))
			Success = false;
		} // if
	if (MatTwin.Mat[1] && Success)
		{
		if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[1], DefaultGround, NULL, (float)(MatTwin.Covg[1])))
			Success = false;
		} // if
	} // if
	
return (Success);

} // GroundEffectBase::AddDefaultGround

/*===========================================================================*/
/*===========================================================================*/

GroundEffect::GroundEffect()
: EcosystemEffect(NULL, WCS_EFFECTS_MATERIALTYPE_GROUND)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_GROUND;
SetDefaults();

} // GroundEffect::GroundEffect

/*===========================================================================*/

GroundEffect::GroundEffect(RasterAnimHost *RAHost)
: EcosystemEffect(RAHost, WCS_EFFECTS_MATERIALTYPE_GROUND)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_GROUND;
SetDefaults();

} // GroundEffect::GroundEffect

/*===========================================================================*/

GroundEffect::GroundEffect(RasterAnimHost *RAHost, EffectsLib *Library, GroundEffect *Proto)
: EcosystemEffect(RAHost, WCS_EFFECTS_MATERIALTYPE_GROUND)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_GROUND;
Prev = Library->LastGround;
if (Library->LastGround)
	{
	Library->LastGround->Next = this;
	Library->LastGround = this;
	} // if
else
	{
	Library->Ground = Library->LastGround = this;
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
	strcpy(NameBase, "Ground");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // GroundEffect::GroundEffect

/*===========================================================================*/

GroundEffect::~GroundEffect()
{

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->GNG && GlobalApp->GUIWins->GNG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->GNG;
		GlobalApp->GUIWins->GNG = NULL;
		} // if
	} // if

} // GroundEffect::~GroundEffect

/*===========================================================================*/

void GroundEffect::SetDefaults(void)
{
GradientCritter *CurGrad;

if (CurGrad = EcoMat.GetActiveNode())
	{
	((MaterialEffect *)CurGrad->GetThing())->DiffuseColor.SetValue3(128.0 / 255, 128.0 / 255, 128.0 / 255);
	} // if

} // GroundEffect::SetDefaults

/*===========================================================================*/

void GroundEffect::Copy(GroundEffect *CopyTo, GroundEffect *CopyFrom)
{

EcosystemEffect::Copy((EcosystemEffect *)CopyTo, (EcosystemEffect *)CopyFrom);

} // GroundEffect::Copy

/*===========================================================================*/

void GroundEffect::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->GNG)
	{
	delete GlobalApp->GUIWins->GNG;
	}
GlobalApp->GUIWins->GNG = new GroundEditGUI(GlobalApp->AppEffects, GlobalApp->MainProj, GlobalApp->AppDB, this);
if(GlobalApp->GUIWins->GNG)
	{
	GlobalApp->GUIWins->GNG->Open(GlobalApp->MainProj);
	}

} // GroundEffect::Edit

/*===========================================================================*/

char *GroundEffect::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as a Ground Texture! Remove anyway?");

} // GroundEffect::OKRemoveRaster

/*===========================================================================*/

char GroundEffect::GetRAHostDropOK(long DropType)
{

if (EcosystemEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_VECTOR
	#ifdef WCS_BUILD_VNS
	|| DropType == WCS_EFFECTSSUBCLASS_SEARCHQUERY
	#endif // WCS_BUILD_VNS
	|| DropType == WCS_EFFECTSSUBCLASS_ECOSYSTEM)
	return (1);

return (0);

} // GroundEffect::GetRAHostDropOK

/*===========================================================================*/

int GroundEffect::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
int Success;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber() || DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_ECOSYSTEM)
	{
	Success = -1;
	if (this != (GroundEffect *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (GroundEffect *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
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
	Success = EcosystemEffect::ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // GroundEffect::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long GroundEffect::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_GROUND | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // GroundEffect::GetRAFlags



/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int GroundEffect::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
GroundEffect *CurrentGround = NULL;
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
					else if (! strnicmp(ReadBuf, "Ground", 8))
						{
						if (CurrentGround = new GroundEffect(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentGround->Load(ffile, Size, ByteFlip)) == Size)
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

if (Success == 1 && CurrentGround)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentGround);
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

} // GroundEffect::LoadObject

/*===========================================================================*/

int GroundEffect::SaveObject(FILE *ffile, const char *SuppliedFileName)
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

// GroundEffect
strcpy(StrBuf, "Ground");
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
			} // if GroundEffect saved 
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

} // GroundEffect::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::GroundEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
GroundEffect *Current;
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
						BytesRead = ReadBlock(ffile, (char *)&GroundBase.Resolution, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_OVERLAPOK:
						{
						BytesRead = ReadBlock(ffile, (char *)&GroundBase.OverlapOK, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_FLOATING:
						{
						BytesRead = ReadBlock(ffile, (char *)&GroundBase.Floating, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						FloatingLoaded = 1;
						break;
						}
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new GroundEffect(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (GroundEffect *)FindDuplicateByName(Current->EffectType, Current))
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
	if (fabs(GroundBase.Resolution - 90.0) > .0001)
		GroundBase.Floating = 0;
	} // if older file

return (TotalRead);

} // EffectsLib::GroundEffect_Load()

/*===========================================================================*/

ULONG EffectsLib::GroundEffect_Save(FILE *ffile)
{
GroundEffect *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_RESOLUTION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&GroundBase.Resolution)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_OVERLAPOK, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&GroundBase.OverlapOK)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_FLOATING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&GroundBase.Floating)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

Current = Ground;
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
					} // if Ground effect saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (GroundEffect *)Current->Next;
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

} // EffectsLib::GroundEffect_Save()

/*===========================================================================*/
