// EffectRasterTA.cpp
// For managing RasterTA Effects
// Built from scratch RasterTerraffectorEffect.cpp on 06/12/97 by Gary R. Huber
// Copyright 1997 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "AppMem.h"
#include "Conservatory.h"
#include "RasterTAEditGUI.h"
#include "Project.h"
#include "Types.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Log.h"
#include "Raster.h"
#include "GraphData.h"
#include "requester.h"
#include "Render.h"
#include "Database.h"
#include "Security.h"
#include "Lists.h"
#include "VectorNode.h"
#include "VectorPolygon.h"
#include "FeatureConfig.h"

RasterTerraffectorEffectBase::RasterTerraffectorEffectBase(void)
{

SetDefaults();

} // RasterTerraffectorEffectBase::RasterTerraffectorEffectBase

/*===========================================================================*/

void RasterTerraffectorEffectBase::SetDefaults(void)
{

Resolution = (float)90.0;
GeoRast = NULL;
EdgesExist = GradientsExist = 0;
OverlapOK = Floating = 1;

} // RasterTerraffectorEffectBase::SetDefaults

/*===========================================================================*/

GeoRaster *RasterTerraffectorEffectBase::Init(GeneralEffect *EffectList, RenderJoeList *&JL, double N, double S, double E, double W, 
	double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem)
{
double CellSize, NWLat, SELat, NWLon, SELon, CellsNS, CellsWE;
GeoRaster *DrawRast = NULL, *DR, *LastDrawRast, *LastGradRast;
RenderJoeList *RJL;
RasterTerraffectorEffect *MyEffect;
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
JL->SortEvalOrder(WCS_JOELIST_SORTORDER_LOHI);

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
			if (MyEffect = (RasterTerraffectorEffect *)RJL->Effect)
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

} // RasterTerraffectorEffectBase::Init

/*===========================================================================*/

void RasterTerraffectorEffectBase::Destroy(void)
{
GeoRaster *Rast = GeoRast, *NextRast;

while (Rast)
	{
	NextRast = (GeoRaster *)Rast->Next;
	delete (Rast);
	Rast = NextRast;
	} // while
GeoRast = NULL;

} // RasterTerraffectorEffectBase::Destroy

/*===========================================================================*/

short RasterTerraffectorEffectBase::AreThereEdges(GeneralEffect *EffectList)
{

while (EffectList)
	{
	if (EffectList->Enabled && EffectList->HiResEdge)
		return (1);
	EffectList = EffectList->Next;
	} // while

return (0);

} // RasterTerraffectorEffectBase::AreThereEdges

/*===========================================================================*/

short RasterTerraffectorEffectBase::AreThereGradients(GeneralEffect *EffectList)
{

while (EffectList)
	{
	if (EffectList->Enabled && EffectList->UseGradient)
		return (1);
	EffectList = EffectList->Next;
	} // while

return (0);

} // RasterTerraffectorEffectBase::AreThereGradients

/*===========================================================================*/

void RasterTerraffectorEffectBase::SetFloating(int NewFloating, Project *CurProj)
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
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_RASTERTA, WCS_EFFECTSSUBCLASS_RASTERTA, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

} // RasterTerraffectorEffectBase::SetFloating

/*===========================================================================*/

void RasterTerraffectorEffectBase::SetFloating(int NewFloating, float NewResolution)
{
NotifyTag Changes[2];

Changes[1] = 0;

Floating = NewFloating;

if (NewFloating)
	{
	Resolution = NewResolution;
	} // if
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_RASTERTA, WCS_EFFECTSSUBCLASS_RASTERTA, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

} // RasterTerraffectorEffectBase::SetFloating

/*===========================================================================*/

// Au garde! No error checking here for speed - be sure to verify if the given
// effect has been initialized before calling the Eval methods.

int RasterTerraffectorEffectBase::Eval(RenderData *Rend, VertexData *Vert, short AfterFractals)
{
double HalfRes, Lat, Lon;
GeoRaster *Rast = GeoRast, *FirstDataRast = GeoRast;
RasterTerraffectorEffect *Effect;
short Priority = -1000, Done = 0, InTheArea = 0;

HalfRes = Resolution * 0.5;
Lat = Vert->Lat;
Lon = Vert->Lon;
TempElev = OrigElev = Vert->Elev - Vert->Displacement;


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
				while (Rast)
					{
					if ((Rast->GradFill && (Rast->Mask = Rast->GetByteCellMask(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon))) || (! Rast->GradFill && Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0))
						{ // process data
						InTheArea = 1;
						if (Rast->GradFill)
							{
							Effect = (RasterTerraffectorEffect *)Rast->LUT[0];
							//if (AfterFractals == 2 || Effect->EvalAfterFractals == AfterFractals)
								{
								if (Effect->Priority >= Priority)
									{
									if (Effect->RenderJoes->SimpleContained(Lat, Lon))
										{
										Priority = Effect->Priority;
										if (Compare(Rend, Vert, Effect, Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes)))
											Done = 1;
										} // if
									} // if
								else
									break;
								} // if
							} // if
						else
							{
							Effect = (RasterTerraffectorEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
							//if (AfterFractals == 2 || Effect->EvalAfterFractals == AfterFractals)
								{
								if (Effect->Priority >= Priority)
									{
									if (Effect->RenderJoes->SimpleContained(Lat, Lon))
										{
										Priority = Effect->Priority;
										if (Compare(Rend, Vert, Effect, 1.0))
											Done = 1;
										} // if
									} // if
								else
									break;
								} // if
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
						InTheArea = 1;
						if (Rast->GradFill)
							{
							Effect = (RasterTerraffectorEffect *)Rast->LUT[0];
							//if (AfterFractals == 2 || Effect->EvalAfterFractals == AfterFractals)
								{
								if (Effect->RenderJoes->SimpleContained(Lat, Lon))
									{
									if (Compare(Rend, Vert, Effect, Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes)))
										Done = 1;
									break;
									} // if
								} // if
							} // if
						else
							{
							Effect = (RasterTerraffectorEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
							//if (AfterFractals == 2 || Effect->EvalAfterFractals == AfterFractals)
								{
								if (Effect->RenderJoes->SimpleContained(Lat, Lon))
									{
									if (Compare(Rend, Vert, Effect, 1.0))
										Done = 1;
									break;
									} // if
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
		if (OverlapOK)
			{ // check maps until lower priority is encountered
			while (Rast)
				{
				if ((Rast->GradFill && (Rast->Mask = Rast->GetByteCellMask(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon))) || (! Rast->GradFill && Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0))
					{ // process data
					InTheArea = 1;
					if (Rast->GradFill)
						{
						Effect = (RasterTerraffectorEffect *)Rast->LUT[0];
						//if (AfterFractals == 2 || Effect->EvalAfterFractals == AfterFractals)
							{
							if (Effect->Priority >= Priority)
								{
								Priority = Effect->Priority;
								if (Compare(Rend, Vert, Effect, Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes)))
									Done = 1;
								} // if
							else
								break;
							} // if
						} // if
					else
						{
						Effect = (RasterTerraffectorEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
						//if (AfterFractals == 2 || Effect->EvalAfterFractals == AfterFractals)
							{
							if (Effect->Priority >= Priority)
								{
								Priority = Effect->Priority;
								if (Compare(Rend, Vert, Effect, 1.0))
									Done = 1;
								} // if
							else
								break;
							} // if
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
					InTheArea = 1;
					if (Rast->GradFill)
						{
						Effect = (RasterTerraffectorEffect *)Rast->LUT[0];
						//if (AfterFractals == 2 || Effect->EvalAfterFractals == AfterFractals)
							{
							if (Compare(Rend, Vert, Effect, Effect->ADProf.GetValue(0, (Rast->GetByteMaskInterp(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) - 2.0) * HalfRes)))
								Done = 1;
							break;
							} // if
						} // if
					else
						{
						Effect = (RasterTerraffectorEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
						//if (AfterFractals == 2 || Effect->EvalAfterFractals == AfterFractals)
							{
							if (Compare(Rend, Vert, Effect, 1.0))
								Done = 1;
							break;
							} // if
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
		InTheArea = 1;
		if (OverlapOK)
			{ // find all containing objects until priority changes
			while (Rast)
				{
				if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
					{ // process data
					Effect = (RasterTerraffectorEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
					//if (AfterFractals == 2 || Effect->EvalAfterFractals == AfterFractals)
						{
						if (Effect->Priority >= Priority)
							{
							if (Effect->RenderJoes->SimpleContained(Lat, Lon))
								{
								Priority = Effect->Priority;
								if (Compare(Rend, Vert, Effect, 1.0))
									Done = 1;
								} // if
							} // if equal or higher priority
						else
							break;
						} // if
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
					Effect = (RasterTerraffectorEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
					//if (AfterFractals == 2 || Effect->EvalAfterFractals == AfterFractals)
						{
						if (Effect->RenderJoes->SimpleContained(Lat, Lon))
							{
							if (Compare(Rend, Vert, Effect, 1.0))
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
				Effect = (RasterTerraffectorEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
				InTheArea = 1;
				//if (AfterFractals == 2 || Effect->EvalAfterFractals == AfterFractals)
					{
					if (Effect->Priority >= Priority)
						{
						Priority = Effect->Priority;
						if (Compare(Rend, Vert, Effect, 1.0))
							Done = 1;
						} // if equal or higher priority
					else
						break;
					} // if
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
			InTheArea = 1;
			Effect = (RasterTerraffectorEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
			//if (AfterFractals == 2 || Effect->EvalAfterFractals == AfterFractals)
				{
				if (Compare(Rend, Vert, Effect, 1.0))
					Done = 1;
				} // if
			} // if a value in edge map
		} // else
	} // if

if (Done > 0)
	{
	Vert->Elev = TempElev;
	Vert->Displacement *= TempRoughness;
	Vert->Elev += Vert->Displacement;
	} // if

return (Done > 0 ? 1: -InTheArea);

} // RasterTerraffectorEffectBase::Eval

/*===========================================================================*/

int RasterTerraffectorEffectBase::Compare(RenderData *Rend, VertexData *Vert, RasterTerraffectorEffect *Effect, double ProfWt)
{
double VeryTempElev, ElevDiff, TexOpacity, Value[3];
int Success = 0;

Value[0] = Value[1] = Value[2] = 0.0;

if (Effect->GetEnabledTheme(WCS_EFFECTS_RASTERTA_THEME_ELEV) &&
	Effect->Theme[WCS_EFFECTS_RASTERTA_THEME_ELEV]->Eval(Value, Vert->Vector))
	{
	VeryTempElev = Value[0];
	if (Effect->Absolute && Rend->ExagerateElevLines)
		VeryTempElev = (VeryTempElev - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
	} // if
else if (Effect->Absolute && Rend->ExagerateElevLines)
	VeryTempElev = (Effect->AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ELEV].CurValue - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
else
	VeryTempElev = Effect->AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ELEV].CurValue;
if (Effect->Absolute)
	{
	if (Effect->ApplyToOrigElev)
		{
		ElevDiff = ProfWt * (VeryTempElev - OrigElev);
		VeryTempElev = OrigElev + ElevDiff;
		} // if
	else
		{
		ElevDiff = ProfWt * (VeryTempElev - TempElev);
		VeryTempElev = TempElev + ElevDiff;
		} // else
	} // if
else
	{
	if (Effect->ApplyToOrigElev)
		{
		ElevDiff = ProfWt * VeryTempElev;
		VeryTempElev = OrigElev + ElevDiff;
		} // if
	else
		{
		ElevDiff = ProfWt * VeryTempElev;
		VeryTempElev = TempElev + ElevDiff;
		} // else
	} // else

if (Effect->TexRoot[WCS_EFFECTS_RASTERTA_TEXTURE_ELEV] && Effect->TexRoot[WCS_EFFECTS_RASTERTA_TEXTURE_ELEV]->Enabled)
	{
	if ((TexOpacity = Effect->TexRoot[WCS_EFFECTS_RASTERTA_TEXTURE_ELEV]->Eval(Value, Rend->TransferTextureData(Vert))) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			// Value[0] has already been diminished by the texture's opacity
			ElevDiff *= (TexOpacity - Value[0]);
			} // if
		else
			ElevDiff *= (1.0 - Value[0]);
		VeryTempElev -= ElevDiff;
		} // if
	} // if

if (Effect->CompareType == WCS_EFFECTS_RASTERTA_COMPARETYPE_INCDEC)
	{
	TempElev = VeryTempElev;
	TempRoughness = Effect->AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ROUGHNESS].CurValue;
	Success = 1;
	} // if
else if (Effect->CompareType == WCS_EFFECTS_RASTERTA_COMPARETYPE_INCREASE)
	{
	if (VeryTempElev > TempElev)
		{
		TempElev = VeryTempElev;
		TempRoughness = Effect->AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ROUGHNESS].CurValue;
		Success = 1;
		} // if
	} // else if increase only
else
	{
	if (VeryTempElev < TempElev)
		{
		TempElev = VeryTempElev;
		TempRoughness = Effect->AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ROUGHNESS].CurValue;
		Success = 1;
		} // if
	} // else decrease only

if (Success)
	{
	if (Effect->TexRoot[WCS_EFFECTS_RASTERTA_TEXTURE_ROUGHNESS] && Effect->TexRoot[WCS_EFFECTS_RASTERTA_TEXTURE_ROUGHNESS]->Enabled)
		{
		if ((TexOpacity = Effect->TexRoot[WCS_EFFECTS_RASTERTA_TEXTURE_ROUGHNESS]->Eval(Value, Rend->TransferTextureData(Vert))) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				TempRoughness *= (1.0 - TexOpacity + Value[0]);
				} // if
			else
				TempRoughness *= Value[0];
			} // if
		} // if
	} // if

return (Success);

} // RasterTerraffectorEffectBase::Compare

/*===========================================================================*/

// VNS 3 version
bool RasterTerraffectorEffectBase::Eval(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode)
{
double DistanceToVector, dX, dY, dZ;
long EffectCt, CurCt;
EffectJoeList *CurList;
RasterTerraffectorEffect *CurEffect;
bool Done = false, Success = true;
short TopPriority = -10000;

// test this to see if there are any area terraffectors
EffectsLib::EffectChain[0] = NULL;

// build comprehensive list of the effects to evaluate

for (EffectCt = 0, CurList = MyEffects; CurList; CurList = CurList->Next)
	{
	if (CurList->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_RASTERTA)
		EffectsLib::EffectChain[EffectCt++] = CurList;
	} // for

if (CurNode->FlagCheck(WCS_VECTORNODE_FLAG_VECTORNODE))
	{
	for (VectorNodeLink *SearchNodeLink = CurNode->LinkedNodes; SearchNodeLink; SearchNodeLink = (VectorNodeLink *)SearchNodeLink->NextNodeList)
		{
		for (CurList = SearchNodeLink->LinkedPolygon->MyEffects; CurList; CurList = CurList->Next)
			{
			if (CurList->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_RASTERTA)
				EffectsLib::EffectChain[EffectCt++] = CurList;
			} // for
		} // for
	} // if
	
if (EffectsLib::EffectChain[0])
	{
	EffectsLib::SortEffectChain(EffectCt, WCS_EFFECTSSUBCLASS_RASTERTA);
	OrigElev = TempElev = CurNode->Elev - CurNode->NodeData->TexDisplacement;

	for (CurCt = 0; CurCt < EffectCt; ++CurCt)
		{
		if (CurList = EffectsLib::EffectChain[CurCt])
			{
			CurEffect = (RasterTerraffectorEffect *)CurList->MyEffect;
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
					// compare to see if elevation is modified
					if (Compare(Rend, CurEffect, CurList->MyJoe, CurNode, CurEffect->ADProf.GetValue(0, DistanceToVector)))
						{
						Done = true;
						TopPriority = CurEffect->Priority;
						} // if
					} // if
				else
					{
					if (CurNode->FlagCheck(WCS_VECTORNODE_FLAG_VECTORNODE))
						{
						if ((DistanceToVector = fabs(CurList->MyJoe->MinDistToPoint(CurNode->Lat, CurNode->Lon, CurNode->Elev, 
							Rend->EarthLatScaleMeters, true, Rend->ElevDatum, Rend->Exageration, dX, dY, dZ))) > .01)
							{
							if (Compare(Rend, CurEffect, CurList->MyJoe, CurNode, 1.0))
								{
								Done = true;
								TopPriority = CurEffect->Priority;
								} // if
							} // if
						} // if
					else if (Compare(Rend, CurEffect, CurList->MyJoe, CurNode, 1.0))
						{
						Done = true;
						TopPriority = CurEffect->Priority;
						} // if
					} // else
				} // else
			} // if
		} // for

	if (Done)
		{
		CurNode->NodeData->TexDisplacement *= (float)TempRoughness;
		CurNode->Elev = TempElev + CurNode->NodeData->TexDisplacement;
		} // if
	} // if
	
return (Success);

} // RasterTerraffectorEffectBase::Eval

/*===========================================================================*/

// VNS 3 version
bool RasterTerraffectorEffectBase::Compare(RenderData *Rend, RasterTerraffectorEffect *Effect, Joe *RefVec, VectorNode *Node, double ProfWt)
{
double VeryTempElev, ElevDiff, TexOpacity, Value[3];
bool Success = false;

Value[0] = Value[1] = Value[2] = 0.0;

if (Effect->Theme[WCS_EFFECTS_RASTERTA_THEME_ELEV] && Effect->Theme[WCS_EFFECTS_RASTERTA_THEME_ELEV]->Enabled &&
	Effect->Theme[WCS_EFFECTS_RASTERTA_THEME_ELEV]->Eval(Value, RefVec))
	{
	VeryTempElev = Value[0];
	if (Effect->Absolute && Rend->ExagerateElevLines)
		VeryTempElev = (VeryTempElev - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
	} // if
else
	VeryTempElev = Effect->Elev;
if (Effect->Absolute)
	{
	if (Effect->ApplyToOrigElev)
		{
		ElevDiff = ProfWt * (VeryTempElev - OrigElev);
		VeryTempElev = OrigElev + ElevDiff;
		} // if
	else
		{
		ElevDiff = ProfWt * (VeryTempElev - TempElev);
		VeryTempElev = TempElev + ElevDiff;
		} // else
	} // if
else
	{
	if (Effect->ApplyToOrigElev)
		{
		ElevDiff = ProfWt * VeryTempElev;
		VeryTempElev = OrigElev + ElevDiff;
		} // if
	else
		{
		ElevDiff = ProfWt * VeryTempElev;
		VeryTempElev = TempElev + ElevDiff;
		} // else
	} // else

if (Effect->TexRoot[WCS_EFFECTS_RASTERTA_TEXTURE_ELEV] && Effect->TexRoot[WCS_EFFECTS_RASTERTA_TEXTURE_ELEV]->Enabled)
	{
	if ((TexOpacity = Effect->TexRoot[WCS_EFFECTS_RASTERTA_TEXTURE_ELEV]->Eval(Value, Rend->TransferTextureData(Node, RefVec, NULL))) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			// Value[0] has already been diminished by the texture's opacity
			ElevDiff *= (TexOpacity - Value[0]);
			} // if
		else
			ElevDiff *= (1.0 - Value[0]);
		VeryTempElev -= ElevDiff;
		} // if
	} // if

if (Effect->CompareType == WCS_EFFECTS_RASTERTA_COMPARETYPE_INCDEC)
	{
	TempElev = VeryTempElev;
	TempRoughness = Effect->AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ROUGHNESS].CurValue;
	Success = true;
	} // if
else if (Effect->CompareType == WCS_EFFECTS_RASTERTA_COMPARETYPE_INCREASE)
	{
	if (VeryTempElev > TempElev)
		{
		TempElev = VeryTempElev;
		TempRoughness = Effect->AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ROUGHNESS].CurValue;
		Success = true;
		} // if
	} // else if increase only
else
	{
	if (VeryTempElev < TempElev)
		{
		TempElev = VeryTempElev;
		TempRoughness = Effect->AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ROUGHNESS].CurValue;
		Success = true;
		} // if
	} // else decrease only

if (Success)
	{
	if (Effect->TexRoot[WCS_EFFECTS_RASTERTA_TEXTURE_ROUGHNESS] && Effect->TexRoot[WCS_EFFECTS_RASTERTA_TEXTURE_ROUGHNESS]->Enabled)
		{
		if ((TexOpacity = Effect->TexRoot[WCS_EFFECTS_RASTERTA_TEXTURE_ROUGHNESS]->Eval(Value, Rend->TransferTextureData(Node, RefVec, NULL))) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				TempRoughness *= (1.0 - TexOpacity + Value[0]);
				} // if
			else
				TempRoughness *= Value[0];
			} // if
		} // if
	} // if

return (Success);

} // RasterTerraffectorEffectBase::Compare

/*===========================================================================*/
/*===========================================================================*/

void EffectsLib::EvalRasterTAsNoInit(RenderData *Rend, VertexData *Vert, short AfterFractals)
{
short Done = 0, Priority = -1000;
double Distance;
RasterTerraffectorEffect *MyEffect;
RenderJoeList *JL = NULL, *CurJL;
Joe *VecStash = Vert->Vector;

// build a list of all the effects to be considered
if (Rend->DBase && (CurJL = (JL = Rend->DBase->CreateRenderJoeList(this, WCS_EFFECTSSUBCLASS_RASTERTA))))
	{
	// sort JoeList according to whatever is the correct priority order for this effect
	JL->SortPriority(WCS_JOELIST_SORTORDER_HILO);
	JL->SortEvalOrder(WCS_JOELIST_SORTORDER_LOHI);
	RasterTerraffectorBase.TempElev = RasterTerraffectorBase.OrigElev = Vert->Elev - Vert->Displacement;
	while (CurJL)
		{
		if (CurJL->Me->TestFlags(WCS_JOEFLAG_HASKIDS) || ! CurJL->Me->TestFlags(WCS_JOEFLAG_ACTIVATED))
			{
			continue;
			} // if 
		if (MyEffect = (RasterTerraffectorEffect *)CurJL->Effect)
			{
			//if (AfterFractals == 2 || MyEffect->EvalAfterFractals == AfterFractals)
				{
				if (MyEffect->Priority >= Priority)
					{
					// test to see if point is really contained in Joe - SimpleContained does a trivial discard of non-overlapping points
					if (CurJL->Me->SimpleContained(Vert->Lat, Vert->Lon))
						{
						MyEffect->InitFrameToRender(this, Rend);
						Priority = MyEffect->Priority;
						// might be useful for TM evaluation
						Vert->Vector = CurJL->Me;
						// if gradient find distance to edge
						if (MyEffect->UseGradient)
							{
							Distance = CurJL->Me->MinDistToPoint(Vert->Lat, Vert->Lon, Rend->EarthLatScaleMeters, NULL, NULL);
							// compare to see if elevation is modified
							if (RasterTerraffectorBase.Compare(Rend, Vert, MyEffect, MyEffect->ADProf.GetValue(0, Distance)))
								Done = 1;
							} // if
						else
							{
							if (RasterTerraffectorBase.Compare(Rend, Vert, MyEffect, 1.0))
								Done = 1;
							} // else
						Vert->Vector = NULL;
						if (! RasterTerraffectorBase.OverlapOK)
							break;
						} // if
					} // if
				else
					break;
				} // if
			} // if there truly is an effect
		CurJL = (RenderJoeList *)CurJL->Next;
		} // while

	while (JL)
		{
		CurJL = (RenderJoeList *)JL->Next;
		delete JL;
		JL = CurJL;
		} // while

	// fill in values
	if (Done > 0)
		{
		Vert->Elev = RasterTerraffectorBase.TempElev;
		Vert->Displacement *= RasterTerraffectorBase.TempRoughness;
		Vert->Elev += Vert->Displacement;
		} // if
	} // if

// put it back like it was
Vert->Vector = VecStash;

} // EffectsLib::EvalRasterTAsNoInit

/*===========================================================================*/
/*===========================================================================*/

RasterTerraffectorEffect::RasterTerraffectorEffect()
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR;
SetDefaults();

} // RasterTerraffectorEffect::RasterTerraffectorEffect

/*===========================================================================*/

RasterTerraffectorEffect::RasterTerraffectorEffect(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR;
SetDefaults();

} // RasterTerraffectorEffect::RasterTerraffectorEffect

/*===========================================================================*/

RasterTerraffectorEffect::RasterTerraffectorEffect(RasterAnimHost *RAHost, EffectsLib *Library, RasterTerraffectorEffect *Proto)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR;
Prev = Library->LastRasterTA;
if (Library->LastRasterTA)
	{
	Library->LastRasterTA->Next = this;
	Library->LastRasterTA = this;
	} // if
else
	{
	Library->RasterTA = Library->LastRasterTA = this;
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
	strcpy(NameBase, "Area Terraffector");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // RasterTerraffectorEffect::RasterTerraffectorEffect

/*===========================================================================*/

RasterTerraffectorEffect::~RasterTerraffectorEffect()
{
long Ct;
RootTexture *DelTex;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->RTG && GlobalApp->GUIWins->RTG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->RTG;
		GlobalApp->GUIWins->RTG = NULL;
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

} // RasterTerraffectorEffect::~RasterTerraffectorEffect

/*===========================================================================*/

void RasterTerraffectorEffect::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_RASTERTA_NUMANIMPAR] = {0.0, 1.0};
double RangeDefaults[WCS_EFFECTS_RASTERTA_NUMANIMPAR][3] = {1000000.0, -1000000.0, 1.0,
															5.0, 0.0, .01};
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
for (Ct = 0; Ct < GetNumThemes(); Ct ++)
	{
	Theme[Ct] = NULL;
	} // for
EvalOrder = 1;
CompareType = WCS_EFFECTS_RASTERTA_COMPARETYPE_INCDEC;
ApplyToOrigElev = 0;
Elev = 0.0;
ADProf.SetDefaults(this, (char)GetNumAnimParams());

AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ELEV].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ROUGHNESS].SetMultiplier(100.0);

} // RasterTerraffectorEffect::SetDefaults

/*===========================================================================*/

void RasterTerraffectorEffect::Copy(RasterTerraffectorEffect *CopyTo, RasterTerraffectorEffect *CopyFrom)
{

CopyTo->EvalOrder = CopyFrom->EvalOrder;
CopyTo->CompareType = CopyFrom->CompareType;
CopyTo->ApplyToOrigElev = CopyFrom->ApplyToOrigElev;
ADProf.Copy(&CopyTo->ADProf, &CopyFrom->ADProf);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // RasterTerraffectorEffect::Copy

/*===========================================================================*/

ULONG RasterTerraffectorEffect::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char MatchName[WCS_EFFECT_MAXNAMELENGTH];
char TexRootNumber = -1;
float TempFlt;

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
					case WCS_EFFECTS_RASTERTERRAFFECTOR_FLT_ELEV:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempFlt, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ELEV].SetValue((double)TempFlt);
						break;
						}
					case WCS_EFFECTS_RASTERTERRAFFECTOR_DBL_ELEV:
						{
						BytesRead = AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ELEV].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RASTERTERRAFFECTOR_FLT_ROUGHNESSx100:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempFlt, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ROUGHNESS].SetValue((double)TempFlt / 100.0);
						break;
						}
					case WCS_EFFECTS_RASTERTERRAFFECTOR_DBL_ROUGHNESSx100:
						{
						BytesRead = AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ROUGHNESS].Load(ffile, Size, ByteFlip);
						AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ROUGHNESS].ScaleValues(.01);
						break;
						}
					case WCS_EFFECTS_RASTERTERRAFFECTOR_ROUGHNESS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ROUGHNESS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RASTERTERRAFFECTOR_EVALORDER:
						{
						BytesRead = ReadBlock(ffile, (char *)&EvalOrder, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RASTERTERRAFFECTOR_COMPARETYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&CompareType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RASTERTERRAFFECTOR_APPLYTOORIGELEV:
						{
						BytesRead = ReadBlock(ffile, (char *)&ApplyToOrigElev, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RASTERTERRAFFECTOR_TEXTUREROOTNUM:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexRootNumber, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures())
							{
							TexRoot[TexRootNumber] = new RootTexture(this, 0, 0, 0);
							} // if
						break;
						}
					case WCS_EFFECTS_RASTERTERRAFFECTOR_TEXTUREROOT:
						{
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures() && TexRoot[TexRootNumber])
							BytesRead = TexRoot[TexRootNumber]->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						TexRootNumber = -1;
						break;
						}
					case WCS_EFFECTS_RASTERTERRAFFECTOR_TEXELEV:
						{
						if (TexRoot[WCS_EFFECTS_RASTERTA_TEXTURE_ELEV] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_RASTERTA_TEXTURE_ELEV]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_RASTERTERRAFFECTOR_TEXROUGHNESS:
						{
						if (TexRoot[WCS_EFFECTS_RASTERTA_TEXTURE_ROUGHNESS] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_RASTERTA_TEXTURE_ROUGHNESS]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_RASTERTERRAFFECTOR_PROFILE:
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
					case WCS_EFFECTS_RASTERTERRAFFECTOR_THEMEELEV:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_RASTERTA_THEME_ELEV] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
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

} // RasterTerraffectorEffect::Load

/*===========================================================================*/

unsigned long int RasterTerraffectorEffect::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_EFFECTS_RASTERTA_NUMANIMPAR] = {WCS_EFFECTS_RASTERTERRAFFECTOR_DBL_ELEV,
																	WCS_EFFECTS_RASTERTERRAFFECTOR_ROUGHNESS};
unsigned long int TextureItemTag[WCS_EFFECTS_RASTERTA_NUMTEXTURES] = {WCS_EFFECTS_RASTERTERRAFFECTOR_TEXELEV,
																	WCS_EFFECTS_RASTERTERRAFFECTOR_TEXROUGHNESS};
unsigned long int ThemeItemTag[WCS_EFFECTS_RASTERTA_NUMTHEMES] = {WCS_EFFECTS_RASTERTERRAFFECTOR_THEMEELEV};

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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RASTERTERRAFFECTOR_EVALORDER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&EvalOrder)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RASTERTERRAFFECTOR_COMPARETYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&CompareType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RASTERTERRAFFECTOR_APPLYTOORIGELEV, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ApplyToOrigElev)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_EFFECTS_RASTERTERRAFFECTOR_PROFILE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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

} // RasterTerraffectorEffect::Save

/*===========================================================================*/

void RasterTerraffectorEffect::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->RTG)
	{
	delete GlobalApp->GUIWins->RTG;
	}
GlobalApp->GUIWins->RTG = new RasterTAEditGUI(GlobalApp->AppEffects, GlobalApp->MainProj, GlobalApp->AppDB, this);
if(GlobalApp->GUIWins->RTG)
	{
	GlobalApp->GUIWins->RTG->Open(GlobalApp->MainProj);
	}

} // RasterTerraffectorEffect::Edit

/*===========================================================================*/

short RasterTerraffectorEffect::AnimateShadows(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetNumNodes(0) > 1)
		return (1);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for

return (0);

} // RasterTerraffectorEffect::AnimateShadows

/*===========================================================================*/

char *RasterTAEffectCritterNames[WCS_EFFECTS_RASTERTA_NUMANIMPAR] = {"Elevation (m)", "Roughness (%)"};
char *RasterTAEffectTextureNames[WCS_EFFECTS_RASTERTA_NUMTEXTURES] = {"Elevation (%)", "Roughness (%)"};
char *RasterTAEffectThemeNames[WCS_EFFECTS_RASTERTA_NUMTHEMES] = {"Elevation (m)"};

char *RasterTerraffectorEffect::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (RasterTAEffectCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (RasterTAEffectTextureNames[Ct]);
		} // if
	} // for
if (Test == &ADProf)
	return ("Edge Feathering Profile");

return ("");

} // RasterTerraffectorEffect::GetCritterName

/*===========================================================================*/

char *RasterTerraffectorEffect::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as an Area Terraffector Texture! Remove anyway?");

} // RasterTerraffectorEffect::OKRemoveRaster

/*===========================================================================*/

char *RasterTerraffectorEffect::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? RasterTAEffectTextureNames[TexNumber]: (char*)"");

} // RasterTerraffectorEffect::GetTextureName

/*===========================================================================*/

void RasterTerraffectorEffect::GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace)
{

ApplyToColor = 0;
ApplyToDisplace = 1;

} // RasterTerraffectorEffect::GetTextureApplication

/*===========================================================================*/

RootTexture *RasterTerraffectorEffect::NewRootTexture(long TexNumber)
{
char ApplyToColor = 0;
char ApplyToDisplace = 1;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // RasterTerraffectorEffect::NewRootTexture

/*===========================================================================*/

char *RasterTerraffectorEffect::GetThemeName(long ThemeNum)
{

return (ThemeNum < GetNumThemes() ? RasterTAEffectThemeNames[ThemeNum]: (char*)"");

} // RasterTerraffectorEffect::GetThemeName

/*===========================================================================*/

char RasterTerraffectorEffect::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_VECTOR
	#ifdef WCS_BUILD_VNS
	|| DropType == WCS_EFFECTSSUBCLASS_SEARCHQUERY
	#endif // WCS_BUILD_VNS
	|| DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE)
	return (1);

return (0);

} // RasterTerraffectorEffect::GetRAHostDropOK

/*===========================================================================*/

int RasterTerraffectorEffect::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
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
	if (this != (RasterTerraffectorEffect *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (RasterTerraffectorEffect *)DropSource->DropSource);
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

} // RasterTerraffectorEffect::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long RasterTerraffectorEffect::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_RASTERTA | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // RasterTerraffectorEffect::GetRAFlags

/*===========================================================================*/

int RasterTerraffectorEffect::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
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
				case WCS_EFFECTS_RASTERTA_ANIMPAR_ELEV:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_RASTERTA_TEXTURE_ELEV);
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_RASTERTA_THEME_ELEV);
					break;
					} // 
				case WCS_EFFECTS_RASTERTA_ANIMPAR_ROUGHNESS:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_RASTERTA_TEXTURE_ROUGHNESS);
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
				case WCS_EFFECTS_RASTERTA_TEXTURE_ELEV:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_RASTERTA_ANIMPAR_ELEV);
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_RASTERTA_THEME_ELEV);
					break;
					} // 
				case WCS_EFFECTS_RASTERTA_TEXTURE_ROUGHNESS:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_RASTERTA_ANIMPAR_ROUGHNESS);
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
				case WCS_EFFECTS_RASTERTA_THEME_ELEV:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_RASTERTA_ANIMPAR_ELEV);
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_RASTERTA_TEXTURE_ELEV);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	} // else if

return (0);

} // RasterTerraffectorEffect::GetAffiliates

/*===========================================================================*/

RasterAnimHost *RasterTerraffectorEffect::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
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

} // RasterTerraffectorEffect::GetRAHostChild

/*===========================================================================*/

double RasterTerraffectorEffect::GetMaxProfileDistance(void)
{
double FindMin, FindMax;

if (ADProf.GetMinMaxDist(FindMin, FindMax))
	return (FindMax);

return (0.0);

} // RasterTerraffectorEffect::GetMaxProfileDistance

/*===========================================================================*/

int RasterTerraffectorEffect::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{

if (Absolute && Rend->ExagerateElevLines)
	Elev = (AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ELEV].CurValue - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
else
	Elev = AnimPar[WCS_EFFECTS_RASTERTA_ANIMPAR_ELEV].CurValue;

return (GeneralEffect::InitFrameToRender(Lib, Rend));

} // RasterTerraffectorEffect::InitFrameToRender

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int RasterTerraffectorEffect::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
RasterTerraffectorEffect *CurrentTA = NULL;
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
					else if (! strnicmp(ReadBuf, "AreaTA", 8))
						{
						if (CurrentTA = new RasterTerraffectorEffect(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentTA->Load(ffile, Size, ByteFlip)) == Size)
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

if (Success == 1 && CurrentTA)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentTA);
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

} // RasterTerraffectorEffect::LoadObject

/*===========================================================================*/

int RasterTerraffectorEffect::SaveObject(FILE *ffile, const char *SuppliedFileName)
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

// RasterTerraffectorEffect
strcpy(StrBuf, "AreaTA");
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
			} // if RasterTerraffectorEffect saved 
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

} // RasterTerraffectorEffect::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::RasterTerraffectorEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
short CompareMethod, SetCompareMethod = 0;
union MultiVal MV;
RasterTerraffectorEffect *Current;
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
						BytesRead = ReadBlock(ffile, (char *)&RasterTerraffectorBase.Resolution, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_OVERLAPOK:
						{
						BytesRead = ReadBlock(ffile, (char *)&RasterTerraffectorBase.OverlapOK, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_COMPAREMETHOD:
						{
						BytesRead = ReadBlock(ffile, (char *)&CompareMethod, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						SetCompareMethod = 1;
						break;
						}
					case WCS_EFFECTSBASE_FLOATING:
						{
						BytesRead = ReadBlock(ffile, (char *)&RasterTerraffectorBase.Floating, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						FloatingLoaded = 1;
						break;
						}
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new RasterTerraffectorEffect(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							if (SetCompareMethod)
								{
								if (CompareMethod == 3)
									{
									Current->CompareType = WCS_EFFECTS_RASTERTA_COMPARETYPE_INCREASE;
									} // if
								else if (CompareMethod == 4)
									{
									Current->CompareType = WCS_EFFECTS_RASTERTA_COMPARETYPE_DECREASE;
									} // if
								} // if
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (RasterTerraffectorEffect *)FindDuplicateByName(Current->EffectType, Current))
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
	if (fabs(RasterTerraffectorBase.Resolution - 90.0) > .0001)
		RasterTerraffectorBase.Floating = 0;
	} // if older file

return (TotalRead);

} // EffectsLib::RasterTerraffectorEffect_Load()

/*===========================================================================*/

ULONG EffectsLib::RasterTerraffectorEffect_Save(FILE *ffile)
{
RasterTerraffectorEffect *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_RESOLUTION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&RasterTerraffectorBase.Resolution)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_OVERLAPOK, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&RasterTerraffectorBase.OverlapOK)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_FLOATING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&RasterTerraffectorBase.Floating)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

Current = RasterTA;
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
					} // if RasterTA effect saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (RasterTerraffectorEffect *)Current->Next;
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

} // EffectsLib::RasterTerraffectorEffect_Save()

/*===========================================================================*/
