// EffectLake.cpp
// For managing Lake Effects
// Built from scratch on 04/30/97 by Gary R. Huber
// Copyright 1997 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "Conservatory.h"
#include "LakeEditGUI.h"
#include "Types.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Random.h"
#include "Raster.h"
#include "GraphData.h"
#include "Texture.h"
#include "requester.h"
#include "Render.h"
#include "Database.h"
#include "Interactive.h"
#include "ViewGUI.h"
#include "Security.h"
#include "Lists.h"
#include "VectorNode.h"
#include "VectorPolygon.h"
#include "FeatureConfig.h"

LakeEffectBase::LakeEffectBase(void)
{

SetDefaults();

} // LakeEffectBase::LakeEffectBase

/*===========================================================================*/

void LakeEffectBase::SetDefaults(void)
{

Resolution = (float)90.0;
GeoRast = NULL;
EdgesExist = 0;
OverlapOK = Floating = 1;
RefMat = NULL;

} // LakeEffectBase::SetDefaults

/*===========================================================================*/

GeoRaster *LakeEffectBase::Init(GeneralEffect *EffectList, RenderJoeList *&JL, double N, double S, double E, double W, 
	double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem)
{
double CellSize, CellsNS, CellsWE;
GeoRaster *DrawRast = NULL, *DR, *LastDrawRast;
RenderJoeList *RJL;
LakeEffect *MyEffect;
#ifdef WCS_ISLAND_EFFECTS
GeoRaster *FloodRast;
RenderJoeList *MatchJL;
JoeList *NegJL = NULL, *NegJLCur, **NegJLPtr;
LayerStub *MultiPartStub;
LayerEntry *MultiPartLayer;
long MinX, MinY;
#endif // WCS_ISLAND_EFFECTS
short CheckItem;
UBYTE ItemNum = 2, FillVal, ReuseItem;	// item 0 = no effect, item 1 = edge
#ifdef WCS_ISLAND_EFFECTS
UBYTE ByteFill = 255;
#endif // WCS_ISLAND_EFFECTS

CellSize = Resolution / MetersPerDegLat;

EdgesExist = AreThereEdges(EffectList);

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
JL->SortPriority(OverlapOK ? WCS_JOELIST_SORTORDER_HILO: WCS_JOELIST_SORTORDER_LOHI);

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
	for (RJL = JL; RJL; RJL = (RenderJoeList *)RJL->Next)
		{
		// skip it if already drawn
		if (! RJL->Drawn)
			{
			if (RJL->Me->TestFlags(WCS_JOEFLAG_HASKIDS) || ! RJL->Me->TestFlags(WCS_JOEFLAG_ACTIVATED))
				{
				continue;
				} // if 
			if (MyEffect = (LakeEffect *)RJL->Effect)
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
				DrawRast = GeoRast;
				if (EdgesExist)
					{
					if (MyEffect->HiResEdge)
						GeoRast->PolyRasterEdgeByte(MaxMem, UsedMem, RJL->Me, WCS_RASTER_EFFECT_BAND_NORMAL, 1, FALSE, TRUE);
					LastDrawRast = GeoRast;
					DrawRast = (GeoRaster *)GeoRast->Next;
					} // if
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
		sprintf(Name, "d:\\Frames\\LakeRastTest%1d.iff", Count);
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

} // LakeEffectBase::Init

/*===========================================================================*/

void LakeEffectBase::Destroy(void)
{
GeoRaster *Rast = GeoRast, *NextRast;

while (Rast)
	{
	NextRast = (GeoRaster *)Rast->Next;
	delete (Rast);
	Rast = NextRast;
	} // while
GeoRast = NULL;

#ifdef WCS_QUANTAALLOCATOR_DETAILED_CLEANUP
if (RefMat)
	delete RefMat;
#endif // WCS_QUANTAALLOCATOR_DETAILED_CLEANUP
RefMat = NULL;

} // LakeEffectBase::Destroy

/*===========================================================================*/

short LakeEffectBase::AreThereEdges(GeneralEffect *EffectList)
{

while (EffectList)
	{
	if (EffectList->Enabled && EffectList->HiResEdge)
		return (1);
	EffectList = EffectList->Next;
	} // while

return (0);

} // LakeEffectBase::AreThereEdges

/*===========================================================================*/

void LakeEffectBase::SetFloating(int NewFloating, Project *CurProj)
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
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_LAKE, WCS_EFFECTSSUBCLASS_LAKE, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

} // LakeEffectBase::SetFloating

/*===========================================================================*/

void LakeEffectBase::SetFloating(int NewFloating, float NewResolution)
{
NotifyTag Changes[2];

Changes[1] = 0;

Floating = NewFloating;

if (NewFloating)
	{
	Resolution = NewResolution;
	} // if
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_LAKE, WCS_EFFECTSSUBCLASS_LAKE, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

} // LakeEffectBase::SetFloating

/*===========================================================================*/

// Au garde! No error checking here for speed - be sure to verify if the given
// effect has been initialized before calling the Eval methods.

int LakeEffectBase::Eval(RenderData *Rend, VertexData *Vert)
{
GeoRaster *Rast = GeoRast, *FirstDataRast = GeoRast;
LakeEffect *Effect;
short Priority = -1000, Found = 0;
double Lat, Lon, ThemedMaxLevel, ThemedWaterLevel, ThemedBeachLevel;

Lat = Vert->Lat;
Lon = Vert->Lon;
CurBeach = NULL;

// get value from first map
if (EdgesExist)
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
					Effect = (LakeEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
					if (Effect->Priority >= Priority)
						{
						if (Effect->RenderJoes->SimpleContained(Lat, Lon))
							{
							Priority = Effect->Priority;
							if (Vert->Elev < (ThemedMaxLevel = GetThemedMaxLevel(Rend, Vert, Effect, ThemedWaterLevel, ThemedBeachLevel)))
								{
								if (Compare(Rend, Vert, Effect, ThemedWaterLevel))
									{
									Vert->Lake = Effect;
									} // if
								if (CompareBeach(Rend, Vert, Effect, ThemedBeachLevel))
									CurBeach = &Effect->BeachMat;
								} // if
							Found = 1;
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
			{ // find first containing object
			while (Rast)
				{
				if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
					{ // do edge test
					Effect = (LakeEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
					if (Effect->RenderJoes->SimpleContained(Lat, Lon))
						{
						if (Vert->Elev < (ThemedMaxLevel = GetThemedMaxLevel(Rend, Vert, Effect, ThemedWaterLevel, ThemedBeachLevel)))
							{
							if (Compare(Rend, Vert, Effect, ThemedWaterLevel))
								{
								Vert->Lake = Effect;
								} // if
							if (CompareBeach(Rend, Vert, Effect, ThemedBeachLevel))
								CurBeach = &Effect->BeachMat;
							} // if
						Found = 1;
						break;
						} // if
					} // if
				Rast = (GeoRaster *)Rast->Next;
				} // while
			} // else
		if (! Found)
			Found = -1;
		} // if a value in edge map
	} // if hires edges
if (! Found)
	{
	Rast = FirstDataRast;
	if (OverlapOK)
		{ // check maps until lower priority is encountered
		while (Rast)
			{
			if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
				{ // process data
				Effect = (LakeEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
				if (Effect->Priority >= Priority)
					{
					Priority = Effect->Priority;
					if (Vert->Elev < (ThemedMaxLevel = GetThemedMaxLevel(Rend, Vert, Effect, ThemedWaterLevel, ThemedBeachLevel)))
						{
						if (Compare(Rend, Vert, Effect, ThemedWaterLevel))
							{
							Vert->Lake = Effect;
							} // if
						if (CompareBeach(Rend, Vert, Effect, ThemedBeachLevel))
							CurBeach = &Effect->BeachMat;
						} // if
					Found = 1;
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
			Effect = (LakeEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
			if (Vert->Elev < (ThemedMaxLevel = GetThemedMaxLevel(Rend, Vert, Effect, ThemedWaterLevel, ThemedBeachLevel)))
				{
				if (Compare(Rend, Vert, Effect, ThemedWaterLevel))
					{
					Vert->Lake = Effect;
					} // if
				if (CompareBeach(Rend, Vert, Effect, ThemedBeachLevel))
					CurBeach = &Effect->BeachMat;
				} // if
			Found = 1;
			} // if a value in edge map
		} // else
	} // if

if (CurBeach && (! Vert->Beach || TempBeachLevel < Vert->BeachLevel))
	{
	Vert->Beach = CurBeach;
	Vert->BeachLevel = TempBeachLevel;
	} // if

return (Found > 0);

} // LakeEffectBase::Eval

/*===========================================================================*/

double LakeEffectBase::GetThemedMaxLevel(RenderData *Rend, VertexData *Vert, LakeEffect *Effect, double &ThemedWaterLevel, double &ThemedBeachLevel)
{
double ThemedMaxLevel, Value[3];

if (Effect->GetEnabledTheme(WCS_EFFECTS_LAKE_THEME_ELEV) &&
	Effect->Theme[WCS_EFFECTS_LAKE_THEME_ELEV]->Eval(Value, Vert->Vector))
	{
	// water
	ThemedWaterLevel = Value[0];
	if (Rend->ExagerateElevLines)
		{
		ThemedWaterLevel = (ThemedWaterLevel - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
		} // if
	// beach
	if (Effect->WaterlineRefType == WCS_LAKE_REFTYPE_WATERLINE)
		{
		ThemedBeachLevel = ThemedWaterLevel + Effect->AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHHT].CurValue + 
			Effect->AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHHTVAR].CurValue;
		} // if
	else
		{
		ThemedBeachLevel = Effect->MaxBeachLevel;
		} // else
	ThemedMaxLevel = WCS_max(ThemedWaterLevel + Effect->MaxWaveHeight, ThemedBeachLevel);
	} // if
else
	{
	ThemedWaterLevel = Effect->Level;
	ThemedBeachLevel = Effect->MaxBeachLevel;
	ThemedMaxLevel = Effect->MaxLevel;
	} // else no theme


return (ThemedMaxLevel);

} // LakeEffectBase::GetThemedMaxLevel

/*===========================================================================*/

int LakeEffectBase::Compare(RenderData *Rend, VertexData *Vert, LakeEffect *Effect, double ThemedWaterLevel)
{
TwinMaterials BaseMat;
double CurLevel, OldWaterElev, CurWaveHt = 0.0;

//get the right materials, texture evaluator needs water level
OldWaterElev = Vert->WaterElev;
CurLevel = Vert->WaterElev = ThemedWaterLevel;
//Vert->TexDataInitialized = 0;	// since we've changed some vertex values
if (Effect->WaterMat.GetRenderMaterial(&BaseMat, Effect->GetRenderWaterMatGradientPos(Rend, Vert)))
	{
	Vert->WaterElev = OldWaterElev;
	CurLevel += BaseMat.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WATERDEPTH].CurValue * BaseMat.Covg[0];
	if (Rend->Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_WAVE])
		CurWaveHt = BaseMat.Mat[0]->EvalWaves(Rend, Vert) * BaseMat.Covg[0];
	if (BaseMat.Mat[1])
		{
		CurLevel += BaseMat.Mat[1]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WATERDEPTH].CurValue * BaseMat.Covg[1];
		if (Rend->Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_WAVE])
			CurWaveHt += BaseMat.Mat[1]->EvalWaves(Rend, Vert) * BaseMat.Covg[1];
		} // if

	if (Vert->Lake)
		{
		if (Vert->WaterElev + Vert->WaveHeight < CurLevel + CurWaveHt)
			{
			Vert->WaterElev = CurLevel;
			Vert->WaveHeight = CurWaveHt;
			Vert->Flags |= WCS_VERTEXDATA_FLAG_WAVEAPPLIED;
			return (1);
			} // if
		} // if
	else
		{
		Vert->WaterElev = CurLevel;
		Vert->WaveHeight = CurWaveHt;
		Vert->Flags |= WCS_VERTEXDATA_FLAG_WAVEAPPLIED;
		return (1);
		} // else
	} // if material found
else
	{
	Vert->WaterElev = OldWaterElev;
	//Vert->TexDataInitialized = 0;	// since we've changed some vertex values
	} // else

return (0);

} // LakeEffectBase::Compare

/*===========================================================================*/

int LakeEffectBase::CompareBeach(RenderData *Rend, VertexData *Vert, LakeEffect *Effect, double ThemedMaxLevel)
{
double BeachHt;

BeachHt = ThemedMaxLevel;

if (Effect->AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHHTVAR].CurValue > 0.0)
	{
	Rand.Seed64BitShift(Vert->LatSeed, Vert->LonSeed);
	BeachHt = BeachHt - Effect->AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHHTVAR].CurValue + Rand.GenPRN() * Effect->AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHHTVAR].CurValue;
	} // if

if (Vert->Elev <= BeachHt)
	{
	// compare to existing beach, choose lower
	if (CurBeach)
		{
		if (BeachHt < TempBeachLevel)
			{
			TempBeachLevel = BeachHt;
			return (1);
			} // if
		} // if
	else
		{
		TempBeachLevel = BeachHt;
		return (1);
		} // else
	} // if

return (0);

} // LakeEffectBase::CompareBeach

/*===========================================================================*/

bool LakeEffectBase::Eval(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode, LakeEffect *DefaultLake, 
	double &SumOfAllCoverages, bool AddBeachMaterials)
{
double ThemedMaxLevel, ThemedWaterLevel, ThemedBeachLevel;
long EffectCt, CurCt;
EffectJoeList *CurList;
LakeEffect *CurEffect, *BeachEffect;
MaterialList *TempMat, *BeachWaterMat;
Joe *BeachVector;
bool Done = false, Success = true;
short TopPriority = -10000;
TwinMaterials BaseMat;

// build comprehensive list of the effects to evaluate
CurBeach = NULL;

if (! RefMat)
	{
	if (RefMat = new MaterialList())
		{
		if (! RefMat->AddWaterProperties())
			{
			delete RefMat;
			RefMat = NULL;
			Success = false;
			} // if
		} // if
	else
		Success = false;
	} // if

if (Success)
	{
	for (EffectCt = 0, CurList = MyEffects; CurList; CurList = CurList->Next)
		{
		if (CurList->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_LAKE)
			EffectsLib::EffectChain[EffectCt++] = CurList;
		} // for

	EffectsLib::SortEffectChain(EffectCt, WCS_EFFECTSSUBCLASS_LAKE);

	for (CurCt = 0; CurCt < EffectCt && Success; ++CurCt)
		{
		if (CurList = EffectsLib::EffectChain[CurCt])
			{
			CurEffect = (LakeEffect *)CurList->MyEffect;
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
				ThemedMaxLevel = GetThemedMaxLevel(Rend, CurEffect, CurList->MyJoe, ThemedWaterLevel, ThemedBeachLevel);
				if (TempMat = Compare(Rend, CurNode, CurEffect, CurList->MyJoe, ThemedWaterLevel, Success))
					{
					if (CompareBeach(Rend, CurNode, CurEffect, CurList->MyJoe, ThemedBeachLevel))
						{
						CurBeach = &CurEffect->BeachMat;
						BeachEffect = CurEffect;
						BeachVector = CurList->MyJoe;
						BeachWaterMat = TempMat;
						} // if
					TopPriority = CurEffect->Priority;
					Done = true;
					} // if
				} // else
			} // if
		} // for

	// add the default lake
	if (DefaultLake && Success)
		{
		ThemedMaxLevel = GetThemedMaxLevel(Rend, DefaultLake, NULL, ThemedWaterLevel, ThemedBeachLevel);
		if (TempMat = Compare(Rend, CurNode, DefaultLake, NULL, ThemedWaterLevel, Success))
			{
			if (CompareBeach(Rend, CurNode, DefaultLake, NULL, ThemedBeachLevel))
				{
				CurBeach = &DefaultLake->BeachMat;
				BeachEffect = DefaultLake;
				BeachVector = NULL;
				BeachWaterMat = TempMat;
				} // if
			Done = true;
			} // if
		} // if

	if (Done && Success)
		{
		if (CurBeach && (float)TempBeachLevel < CurNode->NodeData->BeachLevel)
			{
			BeachWaterMat->WaterProp->BeachOwner = true;
			CurNode->NodeData->BeachLevel = (float)TempBeachLevel;
			if (AddBeachMaterials)
				{
				CurNode->FlagSet(WCS_VECTORNODE_FLAG_BEACHEVALUATED);
				if (CurBeach->GetRenderMaterial(&BaseMat, BeachEffect->GetRenderBeachMatGradientPos(Rend, BeachVector, CurNode, BeachWaterMat)))
					{
					if (TempMat = CurNode->NodeData->AddWaterMaterial(BaseMat.Mat[0], BeachEffect, BeachVector, (float)BaseMat.Covg[0]))
						{
						TempMat->WaterProp->WaterDepth = BeachWaterMat->WaterProp->WaterDepth;
						SumOfAllCoverages += BaseMat.Covg[0] * (1.0 - BaseMat.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
						if (BaseMat.Mat[1])
							{
							if (TempMat = CurNode->NodeData->AddWaterMaterial(BaseMat.Mat[1], BeachEffect, BeachVector, (float)BaseMat.Covg[1]))
								{
								TempMat->WaterProp->WaterDepth = BeachWaterMat->WaterProp->WaterDepth;
								SumOfAllCoverages += BaseMat.Covg[1] * (1.0 - BaseMat.Mat[1]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
								} // if
							else
								Success = false;
							} // if
						} // if
					else
						Success= false;
					} // if
				} // if
			} // if
		} // if
	} // if
	
return (Success);

} // LakeEffectBase::Eval

/*===========================================================================*/

bool LakeEffectBase::AddDefaultLake(RenderData *Rend, VectorNode *CurNode, LakeEffect *DefaultLake, 
	double &SumOfAllCoverages, bool AddBeachMaterials)
{
double ThemedMaxLevel, ThemedWaterLevel, ThemedBeachLevel;
LakeEffect *BeachEffect;
MaterialList *TempMat, *BeachWaterMat;
bool Done = false, Success = true;
TwinMaterials BaseMat;

// build comprehensive list of the effects to evaluate
CurBeach = NULL;

if (! RefMat)
	{
	if (RefMat = new MaterialList())
		{
		if (! RefMat->AddWaterProperties())
			{
			delete RefMat;
			RefMat = NULL;
			Success = false;
			} // if
		} // if
	else
		Success = false;
	} // if
	
// add the default lake
if (DefaultLake && Success)
	{
	ThemedMaxLevel = GetThemedMaxLevel(Rend, DefaultLake, NULL, ThemedWaterLevel, ThemedBeachLevel);
	if (TempMat = Compare(Rend, CurNode, DefaultLake, NULL, ThemedWaterLevel, Success))
		{
		if (CompareBeach(Rend, CurNode, DefaultLake, NULL, ThemedBeachLevel))
			{
			CurBeach = &DefaultLake->BeachMat;
			BeachEffect = DefaultLake;
			BeachWaterMat = TempMat;
			} // if
		Done = true;
		} // if
	} // if

if (Done && Success)
	{
	if (CurBeach && (float)TempBeachLevel < CurNode->NodeData->BeachLevel)
		{
		BeachWaterMat->WaterProp->BeachOwner = true;
		CurNode->NodeData->BeachLevel = (float)TempBeachLevel;
		if (AddBeachMaterials)
			{
			CurNode->FlagSet(WCS_VECTORNODE_FLAG_BEACHEVALUATED);
			if (CurBeach->GetRenderMaterial(&BaseMat, BeachEffect->GetRenderBeachMatGradientPos(Rend, NULL, CurNode, BeachWaterMat)))
				{
				if (TempMat = CurNode->NodeData->AddWaterMaterial(BaseMat.Mat[0], BeachEffect, NULL, (float)BaseMat.Covg[0]))
					{
					TempMat->WaterProp->WaterDepth = BeachWaterMat->WaterProp->WaterDepth;
					SumOfAllCoverages += BaseMat.Covg[0] * (1.0 - BaseMat.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
					if (BaseMat.Mat[1])
						{
						if (TempMat = CurNode->NodeData->AddWaterMaterial(BaseMat.Mat[1], BeachEffect, NULL, (float)BaseMat.Covg[1]))
							{
							TempMat->WaterProp->WaterDepth = BeachWaterMat->WaterProp->WaterDepth;
							SumOfAllCoverages += BaseMat.Covg[1] * (1.0 - BaseMat.Mat[1]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
							} // if
						else
							Success = false;
						} // if
					} // if
				else
					Success = false;
				} // if
			} // if
		} // if
	} // if
	
return (Success);

} // LakeEffectBase::AddDefaultLake

/*===========================================================================*/

double LakeEffectBase::GetThemedMaxLevel(RenderData *Rend, LakeEffect *CurEffect, Joe *CurVec, 
	double &ThemedWaterLevel, double &ThemedBeachLevel)
{
double ThemedMaxLevel, Value[3], JoeElevDbl;
float JoeMinEl;

if (CurEffect->GetEnabledTheme(WCS_EFFECTS_LAKE_THEME_ELEV) &&
	CurEffect->Theme[WCS_EFFECTS_LAKE_THEME_ELEV]->Eval(Value, CurVec))
	{
	// water
	ThemedWaterLevel = Value[0];
	if (Rend->ExagerateElevLines)
		{
		ThemedWaterLevel = (ThemedWaterLevel - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
		} // if
	// beach
	if (CurEffect->WaterlineRefType == WCS_LAKE_REFTYPE_WATERLINE)
		{
		ThemedBeachLevel = ThemedWaterLevel + CurEffect->AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHHT].CurValue + 
			CurEffect->AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHHTVAR].CurValue;
		} // if
	else
		{
		ThemedBeachLevel = CurEffect->MaxBeachLevel;
		} // else
	ThemedMaxLevel = WCS_max(ThemedWaterLevel + CurEffect->MaxWaveHeight, ThemedBeachLevel);
	} // if
else
	{
	ThemedWaterLevel = CurEffect->Level;
	ThemedBeachLevel = CurEffect->MaxBeachLevel;
	ThemedMaxLevel = CurEffect->MaxLevel;
	} // else no theme

if (CurVec && CurEffect->Absolute == WCS_EFFECT_RELATIVETOJOE)
	{
	// find Joe min elev and add to other levels
	JoeMinEl = CurVec->GetMinElev();
	JoeElevDbl = JoeMinEl;
	if (Rend->ExagerateElevLines)
		JoeElevDbl = (JoeElevDbl - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
	ThemedWaterLevel += JoeElevDbl;
	ThemedBeachLevel += JoeElevDbl;
	ThemedMaxLevel += JoeElevDbl;
	} // if

return (ThemedMaxLevel);

} // LakeEffectBase::GetThemedMaxLevel

/*===========================================================================*/

MaterialList *LakeEffectBase::Compare(RenderData *Rend, VectorNode *CurNode, LakeEffect *CurEffect, Joe *CurVec, 
	double ThemedWaterLevel, bool &Success)
{
TwinMaterials BaseMat;
double CurLevel, CurWaveHt = 0.0;
MaterialList *CurMat1, *CurMat2 = NULL;

//get the right materials, texture evaluator needs water level
CurLevel = ThemedWaterLevel;
RefMat->WaterProp->WaterDepth = (float)(CurLevel - CurNode->Elev);
if (CurEffect->WaterMat.GetRenderMaterial(&BaseMat, CurEffect->GetRenderWaterMatGradientPos(Rend, CurVec, CurNode, RefMat)))
	{
	if (CurMat1 = CurNode->NodeData->AddWaterMaterial(BaseMat.Mat[0], CurEffect, CurVec, (float)BaseMat.Covg[0]))
		{
		CurLevel += BaseMat.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WATERDEPTH].CurValue * BaseMat.Covg[0];
		if (Rend->Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_WAVE])
			CurWaveHt = BaseMat.Mat[0]->EvalWaves(Rend, CurNode, CurVec, RefMat) * BaseMat.Covg[0];
		if (BaseMat.Mat[1])
			{
			if (CurMat2 = CurNode->NodeData->AddWaterMaterial(BaseMat.Mat[1], CurEffect, CurVec, (float)BaseMat.Covg[1]))
				{
				CurLevel += BaseMat.Mat[1]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WATERDEPTH].CurValue * BaseMat.Covg[1];
				if (Rend->Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_WAVE])
					CurWaveHt += BaseMat.Mat[1]->EvalWaves(Rend, CurNode, CurVec, RefMat) * BaseMat.Covg[1];
				CurMat2->WaterProp->WaterDepth = (float)(CurLevel - CurNode->Elev);
				CurMat2->WaterProp->WaveHeight = (float)CurWaveHt;
				} // if
			else
				Success = false;
			} // if
		CurMat1->WaterProp->WaterDepth = (float)(CurLevel - CurNode->Elev);
		CurMat1->WaterProp->WaveHeight = (float)CurWaveHt;
		return (CurMat1);
		} // if
	else
		Success = false;
	} // if material found
	
return (NULL);

} // LakeEffectBase::Compare

/*===========================================================================*/

bool LakeEffectBase::CompareBeach(RenderData *Rend, VectorNode *CurNode, LakeEffect *CurEffect, Joe *RefVec, 
	double ThemedMaxLevel)
{
double BeachHt, Value[3], TempVal, TexOpacity;
unsigned long LonSeed, LatSeed;

BeachHt = ThemedMaxLevel;

if (CurEffect->AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHHTVAR].CurValue > 0.0)
	{
	// randomize beach height variation
	LonSeed = (ULONG)((CurNode->Lon - WCS_floor(CurNode->Lon)) * ULONG_MAX);
	LatSeed = (ULONG)((CurNode->Lat - WCS_floor(CurNode->Lat)) * ULONG_MAX);

	Rand.Seed64BitShift(LatSeed, LonSeed);
	TempVal = 1.0;
	if (CurEffect->GetEnabledTexture(WCS_EFFECTS_LAKE_TEXTURE_BEACHHTVAR))
		{
		Value[2] = Value[1] = 0.0;
		if ((TexOpacity = CurEffect->TexRoot[WCS_EFFECTS_LAKE_TEXTURE_BEACHHTVAR]->Eval(Value, Rend->TransferTextureData(CurNode, RefVec, RefMat))) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				TempVal = Value[0] + TempVal * (1.0 - TexOpacity);
				} // if
			else
				TempVal = Value[0];
			} // if
		} // if
	BeachHt = BeachHt - CurEffect->AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHHTVAR].CurValue + 
		TempVal * Rand.GenPRN() * CurEffect->AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHHTVAR].CurValue;
	} // if

if (CurNode->Elev <= BeachHt)
	{
	// compare to existing beach, choose lower
	if (CurBeach)
		{
		if (BeachHt < TempBeachLevel)
			{
			TempBeachLevel = BeachHt;
			return (true);
			} // if
		} // if
	else
		{
		TempBeachLevel = BeachHt;
		return (true);
		} // else
	} // if

return (false);

} // LakeEffectBase::CompareBeach

/*===========================================================================*/

bool LakeEffectBase::AddBeachMaterials(RenderData *Rend, VectorNode *CurNode, MaterialList *BeachWaterMat, double &SumOfAllCoverages)
{
AnimMaterialGradient *CurBeach;
LakeEffect *CurLake;
StreamEffect *CurStream;
MaterialList *TempMat;
TwinMaterials BaseMat;
bool Done = false, Success = true;

CurLake = BeachWaterMat->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_LAKE ? (LakeEffect *)BeachWaterMat->MyEffect: NULL;
CurStream = CurLake ? NULL: (StreamEffect *)BeachWaterMat->MyEffect;

if (CurLake)
	CurBeach = &CurLake->BeachMat;
else
	CurBeach = &CurStream->BeachMat;
	
if ((CurLake && CurBeach->GetRenderMaterial(&BaseMat, CurLake->GetRenderBeachMatGradientPos(Rend, BeachWaterMat->MyVec, CurNode, BeachWaterMat))) ||
	(CurStream && CurBeach->GetRenderMaterial(&BaseMat, CurStream->GetRenderBeachMatGradientPos(Rend, BeachWaterMat->MyVec, CurNode, BeachWaterMat))))
	{
	if (BeachWaterMat->VectorProp)
		{
		if (TempMat = CurNode->NodeData->AddWaterVectorMaterial(BaseMat.Mat[0], CurLake ? CurLake: (GeneralEffect *)CurStream, BeachWaterMat->MyVec, (float)BaseMat.Covg[0]))
			{
			TempMat->WaterProp->WaterDepth = BeachWaterMat->WaterProp->WaterDepth;
			TempMat->VectorProp->VectorType = CurStream ? WCS_TEXTURE_VECTOREFFECTTYPE_LINE | WCS_TEXTURE_VECTOREFFECTTYPE_SPLINEELEV: WCS_TEXTURE_VECTOREFFECTTYPE_AREA;
			TempMat->VectorProp->VecOffsets[0] = BeachWaterMat->VectorProp->VecOffsets[0];
			TempMat->VectorProp->VecOffsets[1] = BeachWaterMat->VectorProp->VecOffsets[1];
			TempMat->VectorProp->VecOffsets[2] = BeachWaterMat->VectorProp->VecOffsets[2];
			SumOfAllCoverages += BaseMat.Covg[0] * (1.0 - BaseMat.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
			if (BaseMat.Mat[1])
				{
				if (TempMat = CurNode->NodeData->AddWaterVectorMaterial(BaseMat.Mat[1], CurLake ? CurLake: (GeneralEffect *)CurStream, BeachWaterMat->MyVec, (float)BaseMat.Covg[1]))
					{
					TempMat->WaterProp->WaterDepth = BeachWaterMat->WaterProp->WaterDepth;
					TempMat->VectorProp->VectorType = CurStream ? WCS_TEXTURE_VECTOREFFECTTYPE_LINE | WCS_TEXTURE_VECTOREFFECTTYPE_SPLINEELEV: WCS_TEXTURE_VECTOREFFECTTYPE_AREA;
					TempMat->VectorProp->VecOffsets[0] = BeachWaterMat->VectorProp->VecOffsets[0];
					TempMat->VectorProp->VecOffsets[1] = BeachWaterMat->VectorProp->VecOffsets[1];
					TempMat->VectorProp->VecOffsets[2] = BeachWaterMat->VectorProp->VecOffsets[2];
					SumOfAllCoverages += BaseMat.Covg[1] * (1.0 - BaseMat.Mat[1]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
					} // if
				else
					Success = false;
				} // if
			Done = true;
			} // if
		else
			Success = false;
		} // if
	else
		{
		if (TempMat = CurNode->NodeData->AddWaterMaterial(BaseMat.Mat[0], CurLake ? CurLake: (GeneralEffect *)CurStream, NULL, (float)BaseMat.Covg[0]))
			{
			TempMat->WaterProp->WaterDepth = BeachWaterMat->WaterProp->WaterDepth;
			SumOfAllCoverages += BaseMat.Covg[0] * (1.0 - BaseMat.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
			if (BaseMat.Mat[1])
				{
				if (TempMat = CurNode->NodeData->AddWaterMaterial(BaseMat.Mat[1], CurLake ? CurLake: (GeneralEffect *)CurStream, NULL, (float)BaseMat.Covg[1]))
					{
					TempMat->WaterProp->WaterDepth = BeachWaterMat->WaterProp->WaterDepth;
					SumOfAllCoverages += BaseMat.Covg[1] * (1.0 - BaseMat.Mat[1]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
					} // if
				else
					Success = false;
				} // if
			Done = true;
			} // if
		else
			Success = false;
		} // else
	} // if

CurNode->FlagSet(WCS_VECTORNODE_FLAG_BEACHEVALUATED);

return (Success);

} // LakeEffectBase::AddBeachMaterials

/*===========================================================================*/
/*===========================================================================*/

LakeEffect::LakeEffect()
: GeneralEffect(NULL), BeachMat(this, 1, WCS_EFFECTS_MATERIALTYPE_BEACH), WaterMat(this, 1, WCS_EFFECTS_MATERIALTYPE_WATER)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_LAKE;
SetDefaults();

} // LakeEffect::LakeEffect

/*===========================================================================*/

LakeEffect::LakeEffect(RasterAnimHost *RAHost)
: GeneralEffect(RAHost), BeachMat(this, 1, WCS_EFFECTS_MATERIALTYPE_BEACH), WaterMat(this, 1, WCS_EFFECTS_MATERIALTYPE_WATER)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_LAKE;
SetDefaults();

} // LakeEffect::LakeEffect

/*===========================================================================*/

LakeEffect::LakeEffect(RasterAnimHost *RAHost, EffectsLib *Library, LakeEffect *Proto)
: GeneralEffect(RAHost), BeachMat(this, 1, WCS_EFFECTS_MATERIALTYPE_BEACH), WaterMat(this, 1, WCS_EFFECTS_MATERIALTYPE_WATER)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_LAKE;
Prev = Library->LastLake;
if (Library->LastLake)
	{
	Library->LastLake->Next = this;
	Library->LastLake = this;
	} // if
else
	{
	Library->Lake = Library->LastLake = this;
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
	strcpy(NameBase, "Lake");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // LakeEffect::LakeEffect

/*===========================================================================*/

LakeEffect::~LakeEffect()
{
long Ct;
RootTexture *DelTex;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->LEG && GlobalApp->GUIWins->LEG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->LEG;
		GlobalApp->GUIWins->LEG = NULL;
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

} // LakeEffect::~LakeEffect

/*===========================================================================*/

void LakeEffect::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_LAKE_NUMANIMPAR] = {0.0, 0.0, .5, .5, 0.0, 0.0};
double RangeDefaults[WCS_EFFECTS_LAKE_NUMANIMPAR][3] = {FLT_MAX, -FLT_MAX, 1.0,
														FLT_MAX, -FLT_MAX, 1.0,
														FLT_MAX, -FLT_MAX, 1.0,
														FLT_MAX, 0.0, 1.0,
														1.0, 0.0, .01,
														1.0, 0.0, .01};
long Ct;

WaterlineRefType = WCS_LAKE_REFTYPE_WATERLINE;
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
PixelTexturesExist = 0;
MaxWaveHeight = MaxBeachLevel = Level = MaxLevel = 0.0;
Absolute = WCS_EFFECT_ABSOLUTE;

AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_WATERLINEREF].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHHTVAR].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHMATDRIVER].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_WATERMATDRIVER].SetMultiplier(100.0);

} // LakeEffect::SetDefaults

/*===========================================================================*/

void LakeEffect::Copy(LakeEffect *CopyTo, LakeEffect *CopyFrom)
{

BeachMat.Copy(&CopyTo->BeachMat, &CopyFrom->BeachMat);
WaterMat.Copy(&CopyTo->WaterMat, &CopyFrom->WaterMat);
CopyTo->WaterlineRefType = CopyFrom->WaterlineRefType;
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // LakeEffect::Copy

/*===========================================================================*/

ULONG LakeEffect::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
float TempFlt;
AnimDoubleTime *Reflect = NULL;
#if defined (WCS_BUILD_VNS) || defined (WCS_THEMATIC_MAP)
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
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_ABSOLUTE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Absolute, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LAKE_WATERLINEREFTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&WaterlineRefType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LAKE_FLT_ELEV:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempFlt, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].SetValue((double)TempFlt);
						break;
						}
					case WCS_EFFECTS_LAKE_DBL_ELEV:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LAKE_FLT_REFLECT:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempFlt, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						if (Reflect = new AnimDoubleTime())
							{
							Reflect->SetValue((double)TempFlt / 100.0);
							} // if
						break;
						}
					case WCS_EFFECTS_LAKE_DBL_REFLECT:
						{
						if (Reflect = new AnimDoubleTime())
							{
							BytesRead = Reflect->Load(ffile, Size, ByteFlip);
							Reflect->ScaleValues(.01);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_LAKE_WATERLINEREF:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_WATERLINEREF].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LAKE_BEACHHT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHHT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LAKE_BEACHHTVAR:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHHTVAR].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LAKE_BEACHMATDRIVER:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHMATDRIVER].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LAKE_WATERMATDRIVER:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_WATERMATDRIVER].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LAKE_BEACHMAT:
						{
						BytesRead = BeachMat.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LAKE_WATERMATOBS:
						{
						if (WaterMat.Grad && WaterMat.Grad->GetThing())
							BytesRead = ((MaterialEffect *)WaterMat.Grad->GetThing())->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_LAKE_WATERMAT:
						{
						BytesRead = WaterMat.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LAKE_TEXBEACHHTVAR:
						{
						if (TexRoot[WCS_EFFECTS_LAKE_TEXTURE_BEACHHTVAR] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_LAKE_TEXTURE_BEACHHTVAR]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_LAKE_TEXBEACHMATDRIVER:
						{
						if (TexRoot[WCS_EFFECTS_LAKE_TEXTURE_BEACHMATDRIVER] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_LAKE_TEXTURE_BEACHMATDRIVER]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_LAKE_TEXWATERMATDRIVER:
						{
						if (TexRoot[WCS_EFFECTS_LAKE_TEXTURE_WATERMATDRIVER] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_LAKE_TEXTURE_WATERMATDRIVER]->Load(ffile, Size, ByteFlip);
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
					#ifdef WCS_THEMATIC_MAP
					case WCS_EFFECTS_LAKE_THEMEELEV:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_LAKE_THEME_ELEV] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_LAKE_THEMEBEACHMATDRIVER:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_LAKE_THEME_BEACHMATDRIVER] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_LAKE_THEMEWATERMATDRIVER:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_LAKE_THEME_WATERMATDRIVER] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
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

if (Reflect)
	delete Reflect;

return (TotalRead);

} // LakeEffect::Load

/*===========================================================================*/

unsigned long int LakeEffect::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_EFFECTS_LAKE_NUMANIMPAR] = {WCS_EFFECTS_LAKE_DBL_ELEV, 
																WCS_EFFECTS_LAKE_WATERLINEREF,
																WCS_EFFECTS_LAKE_BEACHHT,
																WCS_EFFECTS_LAKE_BEACHHTVAR,
																WCS_EFFECTS_LAKE_BEACHMATDRIVER,
																WCS_EFFECTS_LAKE_WATERMATDRIVER};
unsigned long int TextureItemTag[WCS_EFFECTS_LAKE_NUMTEXTURES] = {WCS_EFFECTS_LAKE_TEXBEACHHTVAR,
														WCS_EFFECTS_LAKE_TEXBEACHMATDRIVER,
														WCS_EFFECTS_LAKE_TEXWATERMATDRIVER};
unsigned long int ThemeItemTag[WCS_EFFECTS_LAKE_NUMTHEMES] = {WCS_EFFECTS_LAKE_THEMEELEV,
																WCS_EFFECTS_LAKE_THEMEBEACHMATDRIVER,
																WCS_EFFECTS_LAKE_THEMEWATERMATDRIVER};

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
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ABSOLUTE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Absolute)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LAKE_WATERLINEREFTYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&WaterlineRefType)) == NULL)
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

ItemTag = WCS_EFFECTS_LAKE_BEACHMAT + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = BeachMat.Save(ffile))
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

ItemTag = WCS_EFFECTS_LAKE_WATERMAT + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = WaterMat.Save(ffile))
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

} // LakeEffect::Save

/*===========================================================================*/

void LakeEffect::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->LEG)
	{
	delete GlobalApp->GUIWins->LEG;
	}
GlobalApp->GUIWins->LEG = new LakeEditGUI(GlobalApp->AppEffects, GlobalApp->MainProj, GlobalApp->AppDB, GlobalApp->AppImages, this);
if(GlobalApp->GUIWins->LEG)
	{
	GlobalApp->GUIWins->LEG->Open(GlobalApp->MainProj);
	}

} // LakeEffect::Edit

/*===========================================================================*/

char *LakeEffectCritterNames[WCS_EFFECTS_LAKE_NUMANIMPAR] = {"Elevation (m)", "Waterline Reference (m)",
															"Beach Height (m)", "Beach Height Variation (m)",
															"Beach Material Driver (%)", "Water Material Driver (%)"};
char *LakeEffectTextureNames[WCS_EFFECTS_LAKE_NUMTEXTURES] = {"Beach Height Variation (%)", "Beach Material Driver (%)",
															"Water Material Driver (%)"};
char *LakeEffectThemeNames[WCS_EFFECTS_LAKE_NUMTHEMES] = {"Elevation (m)", "Beach Material Driver (%)",
															"Water Material Driver (%)"};

char *LakeEffect::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (LakeEffectCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (LakeEffectTextureNames[Ct]);
		} // if
	} // for
if (Test == &BeachMat)
	return ("Beach");
if (Test == &WaterMat)
	return ("Water");

return ("");

} // LakeEffect::GetCritterName

/*===========================================================================*/

char *LakeEffect::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as a Lake Texture! Remove anyway?");

} // LakeEffect::OKRemoveRaster

/*===========================================================================*/

int LakeEffect::FindnRemove3DObjects(Object3DEffect *RemoveMe)
{
int Found = 0;

if (BeachMat.FindnRemove3DObjects(RemoveMe))
	Found = 1;
if (WaterMat.FindnRemove3DObjects(RemoveMe))
	Found = 1;

return (Found);

} // LakeEffect::FindnRemove3DObjects

/*===========================================================================*/

int LakeEffect::SetToTime(double Time)
{
long Found = 0;

if (GeneralEffect::SetToTime(Time))
	Found = 1;
if (BeachMat.SetToTime(Time))
	Found = 1;
if (WaterMat.SetToTime(Time))
	Found = 1;

return (Found);

} // LakeEffect::SetToTime

/*===========================================================================*/

int LakeEffect::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{
long Ct;

PixelTexturesExist = 0;

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
if (! BeachMat.InitToRender(Opt, Buffers))
	return (0);
if (! WaterMat.InitToRender(Opt, Buffers))
	return (0);

return (1);

} // LakeEffect::InitToRender

/*===========================================================================*/

int LakeEffect::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{
double WaveLevel;
GradientCritter *CurGrad = WaterMat.Grad;

if (! WaterMat.InitFrameToRender(Lib, Rend))
	return (0);

if (Rend->ExagerateElevLines)
	Level = (AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].CurValue - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
else
	Level = AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].CurValue;

MaxLevel = -FLT_MAX;
MaxWaveHeight = 0.0;
while (CurGrad)
	{
	if (CurGrad->GetThing())
		{
		WaveLevel = 0.0;
		if (Rend->Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_WAVE])
			{
			WaveLevel = ((MaterialEffect *)CurGrad->GetThing())->MaxWaveAmp;
			} // if
		if (((MaterialEffect *)CurGrad->GetThing())->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WATERDEPTH].CurValue + WaveLevel > MaxLevel)
			{
			MaxLevel = ((MaterialEffect *)CurGrad->GetThing())->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WATERDEPTH].CurValue + WaveLevel;
			} // if
		} // if
	if (WaveLevel > MaxWaveHeight)
		MaxWaveHeight = WaveLevel;
	CurGrad = CurGrad->Next;
	} // while
MaxLevel += Level;

// calculate beach top elev
if (WaterlineRefType == WCS_LAKE_REFTYPE_WATERLINE)
	{
	MaxBeachLevel = Level;
	} // if
else
	{
	MaxBeachLevel = AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_WATERLINEREF].CurValue;
	if (Rend->ExagerateElevLines)
		MaxBeachLevel = (MaxBeachLevel - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
	} // if
MaxBeachLevel += AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHHT].CurValue + AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHHTVAR].CurValue;

MaxLevel = max(MaxBeachLevel, MaxLevel);

if (! BeachMat.InitFrameToRender(Lib, Rend))
	return (0);

return (GeneralEffect::InitFrameToRender(Lib, Rend));

} // LakeEffect::InitFrameToRender

/*===========================================================================*/

long LakeEffect::InitImageIDs(long &ImageID)
{
long NumImages = 0;

NumImages += BeachMat.InitImageIDs(ImageID);
NumImages += WaterMat.InitImageIDs(ImageID);
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // LakeEffect::InitImageIDs

/*===========================================================================*/

int LakeEffect::BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves, 
	EffectList **Queries, EffectList **Themes, EffectList **Coords)
{

if (! BeachMat.BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
	return (0);
if (! WaterMat.BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
	return (0);

return (1);

} // LakeEffect::BuildFileComponentsList

/*===========================================================================*/

long LakeEffect::GetKeyFrameRange(double &FirstKey, double &LastKey)
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
if (BeachMat.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (WaterMat.GetKeyFrameRange(MinDist, MaxDist))
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

} // LakeEffect::GetKeyFrameRange

/*===========================================================================*/

char LakeEffect::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMMATERIALGRADIENT
	|| DropType == WCS_EFFECTSSUBCLASS_MATERIAL
	|| DropType == WCS_EFFECTSSUBCLASS_WAVE
	#ifdef WCS_BUILD_VNS
	|| DropType == WCS_EFFECTSSUBCLASS_SEARCHQUERY
	#endif // WCS_BUILD_VNS
	|| DropType == WCS_RAHOST_OBJTYPE_VECTOR)
	return (1);

return (0);

} // LakeEffect::GetRAHostDropOK

/*===========================================================================*/

int LakeEffect::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
int QueryResult, Success;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (LakeEffect *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (LakeEffect *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMMATERIALGRADIENT ||
	DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_MATERIAL)
	{
	Success = -1;
	sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, "which");
	if (QueryResult = UserMessageCustom(NameStr, QueryStr, GetCritterName(&WaterMat), "Cancel", GetCritterName(&BeachMat), 0))
		{
		if (QueryResult == 1)
			{
			Success = WaterMat.ProcessRAHostDragDrop(DropSource);
			} // if
		else if (QueryResult == 2)
			{
			Success = BeachMat.ProcessRAHostDragDrop(DropSource);
			} // else if
		} // if
	} // else if
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_WAVE)
	{
	Success = WaterMat.ProcessRAHostDragDrop(DropSource);
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

} // LakeEffect::ProcessRAHostDragDrop

/*===========================================================================*/

char *LakeEffect::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? LakeEffectTextureNames[TexNumber]: (char*)"");

} // LakeEffect::GetTextureName

/*===========================================================================*/

RootTexture *LakeEffect::NewRootTexture(long TexNumber)
{
char ApplyToColor = 0;
char ApplyToDisplace = 0;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // LakeEffect::NewRootTexture

/*===========================================================================*/

char *LakeEffect::GetThemeName(long ThemeNum)
{

return (ThemeNum < GetNumThemes() ? LakeEffectThemeNames[ThemeNum]: (char*)"");

} // LakeEffect::GetThemeName

/*===========================================================================*/

int LakeEffect::GetRAHostAnimated(void)
{

if (GeneralEffect::GetRAHostAnimated())
	return (1);
if (BeachMat.GetRAHostAnimated())
	return (1);
if (WaterMat.GetRAHostAnimated())
	return (1);

return (0);

} // LakeEffect::GetRAHostAnimated

/*===========================================================================*/

bool LakeEffect::AnimateMaterials(void)
{

if (GeneralEffect::AnimateMaterials())
	return (true);
if (BeachMat.AnimateMaterials())
	return (true);
if (WaterMat.AnimateMaterials())
	return (true);

return (false);

} // LakeEffect::AnimateMaterials

/*===========================================================================*/

unsigned long LakeEffect::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_LAKE | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // LakeEffect::GetRAFlags

/*===========================================================================*/

void LakeEffect::GetInterFlags(RasterAnimHostProperties *Prop, RasterAnimHost *FlagMe)
{

if (! FlagMe)
	{
	Prop->InterFlags = 0;
	return;
	} // if

Prop->InterFlags = (WCS_RAHOST_INTERBIT_CLICKTOPOS | WCS_RAHOST_INTERBIT_MOVEY | 
					WCS_RAHOST_INTERBIT_MOVEZ | WCS_RAHOST_INTERBIT_MOVEELEV);

} // LakeEffect::GetInterFlags

/*===========================================================================*/

int LakeEffect::ScaleMoveRotate(RasterAnimHost *MoveMe, DiagnosticData *Data, unsigned char Operation)
{
DEMBounds CurBounds;
double ElevChange, ElevRange, NewElev, Datum, Exag;
PlanetOpt *DefPlanetOpt;

if (! MoveMe)
	{
	return (0);
	} // if

// Camera group
if (Operation == WCS_RAHOST_INTERACTIVEOP_SETPOS ||
	Operation == WCS_RAHOST_INTERACTIVEOP_SETPOSNOQUERY)
	{
	if (Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION] && 
		(DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, NULL)))
		{
		Datum = DefPlanetOpt->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue;
		if ((Exag = DefPlanetOpt->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].CurValue) != 0.0)
			{
			NewElev = Datum + (Data->Value[WCS_DIAGNOSTIC_ELEVATION] - Datum) / Exag;
				AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].SetCurValue(NewElev);
			} // if
		} // if
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_MOVEXYZ ||
	Operation == WCS_RAHOST_INTERACTIVEOP_MOVELATLONELEV)
	{
	if (GlobalApp->AppDB->FillDEMBounds(&CurBounds))
		{
		Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] = 1;
		Data->Value[WCS_DIAGNOSTIC_LATITUDE] = (CurBounds.North + CurBounds.South) * 0.5;
		Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE] = 1;
		Data->Value[WCS_DIAGNOSTIC_LONGITUDE] = (CurBounds.West + CurBounds.East) * 0.5;
		ElevRange = max(CurBounds.HighElev - CurBounds.LowElev, 10.0);
		} // if
	else
		ElevRange = 10.0;
	Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION] = 1;
	Data->Value[WCS_DIAGNOSTIC_ELEVATION] = AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].CurValue;
	GlobalApp->GUIWins->CVG->ScaleMotion(Data);
	if (Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION])
		{
		if (Data->Value[WCS_DIAGNOSTIC_ELEVATION] > AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].CurValue)
			ElevChange = min(Data->Value[WCS_DIAGNOSTIC_ELEVATION] - AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].CurValue, .001 * ElevRange);
		else
			ElevChange = max(Data->Value[WCS_DIAGNOSTIC_ELEVATION] - AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].CurValue, -.001 * ElevRange);
		AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].SetCurValue(AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].CurValue + ElevChange);
		} // if
	return (1);
	} // if 

return (0);	// return 0 if nothing changed

} // LakeEffect::ScaleMoveRotate

/*===========================================================================*/

int LakeEffect::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
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
				case WCS_EFFECTS_LAKE_ANIMPAR_ELEV:
					{
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_LAKE_THEME_ELEV);
					break;
					} // 
				case WCS_EFFECTS_LAKE_ANIMPAR_BEACHHTVAR:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_LAKE_TEXTURE_BEACHHTVAR);
					break;
					} // 
				case WCS_EFFECTS_LAKE_ANIMPAR_BEACHMATDRIVER:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_LAKE_TEXTURE_BEACHMATDRIVER);
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_LAKE_THEME_BEACHMATDRIVER);
					break;
					} // 
				case WCS_EFFECTS_LAKE_ANIMPAR_WATERMATDRIVER:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_LAKE_TEXTURE_WATERMATDRIVER);
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_LAKE_THEME_WATERMATDRIVER);
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
				case WCS_EFFECTS_LAKE_TEXTURE_BEACHHTVAR:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_LAKE_ANIMPAR_BEACHHTVAR);
					break;
					} // 
				case WCS_EFFECTS_LAKE_TEXTURE_BEACHMATDRIVER:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_LAKE_ANIMPAR_BEACHMATDRIVER);
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_LAKE_THEME_BEACHMATDRIVER);
					break;
					} // 
				case WCS_EFFECTS_LAKE_TEXTURE_WATERMATDRIVER:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_LAKE_ANIMPAR_WATERMATDRIVER);
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_LAKE_THEME_WATERMATDRIVER);
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
				case WCS_EFFECTS_LAKE_THEME_ELEV:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_LAKE_ANIMPAR_ELEV);
					break;
					} // 
				case WCS_EFFECTS_LAKE_THEME_BEACHMATDRIVER:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_LAKE_ANIMPAR_BEACHMATDRIVER);
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_LAKE_TEXTURE_BEACHMATDRIVER);
					break;
					} // 
				case WCS_EFFECTS_LAKE_THEME_WATERMATDRIVER:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_LAKE_ANIMPAR_WATERMATDRIVER);
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_LAKE_TEXTURE_WATERMATDRIVER);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	} // else if

return (0);

} // LakeEffect::GetAffiliates

/*===========================================================================*/

RasterAnimHost *LakeEffect::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;
JoeList *CurJoe = Joes;

if (! Current)
	return (&BeachMat);
if (Current == &BeachMat)
	return (&WaterMat);
if (Current == &WaterMat)
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

} // LakeEffect::GetRAHostChild

/*===========================================================================*/

void LakeEffect::ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds)
{
double Scale;
GraphNode *CurNode;

if (OldBounds->HighElev > OldBounds->LowElev)
	Scale = (CurBounds->HighElev - CurBounds->LowElev) / (OldBounds->HighElev - OldBounds->LowElev);
else
	Scale = 1.0;

AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].SetValue(
	(AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].CurValue - OldBounds->LowElev) * Scale + CurBounds->LowElev);
if (CurNode = AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].GetFirstNode(0))
	{
	CurNode->SetValue((CurNode->GetValue() - OldBounds->LowElev) * Scale + CurBounds->LowElev);
	while (CurNode = AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].GetNextNode(0, CurNode))
		{
		CurNode->SetValue((CurNode->GetValue() - OldBounds->LowElev) * Scale + CurBounds->LowElev);
		} // while
	} // if

} // LakeEffect::ScaleToDEMBounds

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int LakeEffect::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
LakeEffect *CurrentLake = NULL;
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
						} // if Images
					else if (! strnicmp(ReadBuf, "CoordSys", 8))
						{
						if (CurrentCoordSys = new CoordSys(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentCoordSys->Load(ffile, Size, ByteFlip);
							}
						} // if CoordSys
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
					else if (! strnicmp(ReadBuf, "Lake", 8))
						{
						if (CurrentLake = new LakeEffect(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentLake->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if LakeEffect
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

if (Success == 1 && CurrentLake)
	{
	if (OldBoundsLoaded && GlobalApp->AppDB->FillDEMBounds(&CurBounds))
		{
		if (LoadToEffects->GetListPtr(WCS_EFFECTSSUBCLASS_WAVE) && UserMessageYN("Load Lake", "Do you wish the loaded Lake's Wave positions\n to be scaled to current DEM bounds?"))
			{
			for (CurrentWave = (WaveEffect *)LoadToEffects->GetListPtr(WCS_EFFECTSSUBCLASS_WAVE); CurrentWave; CurrentWave = (WaveEffect *)CurrentWave->Next)
				{
				CurrentWave->ScaleToDEMBounds(&OldBounds, &CurBounds);
				} // for
			} // if
		if (UserMessageYN("Load Lake", "Do you wish the loaded Lake's elevation\n to be scaled to current DEM elevation range?\n Note that you still need to set the elevation to a\n specific value based on your terrain and the\n Lake's position in it."))
			{
			for (CurrentWave = (WaveEffect *)LoadToEffects->GetListPtr(WCS_EFFECTSSUBCLASS_WAVE); CurrentWave; CurrentWave = (WaveEffect *)CurrentWave->Next)
				{
				CurrentWave->ScaleToDEMBounds(&OldBounds, &CurBounds);
				} // for
			} // if
		} // if
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentLake);
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

} // LakeEffect::LoadObject

/*===========================================================================*/

int LakeEffect::SaveObject(FILE *ffile, const char *SuppliedFileName)
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

// LakeEffect
strcpy(StrBuf, "Lake");
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
			} // if LakeEffect saved 
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

} // LakeEffect::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::LakeEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
LakeEffect *Current;
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
						BytesRead = ReadBlock(ffile, (char *)&LakeBase.Resolution, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_OVERLAPOK:
						{
						BytesRead = ReadBlock(ffile, (char *)&LakeBase.OverlapOK, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_FLOATING:
						{
						BytesRead = ReadBlock(ffile, (char *)&LakeBase.Floating, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						FloatingLoaded = 1;
						break;
						}
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new LakeEffect(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (LakeEffect *)FindDuplicateByName(Current->EffectType, Current))
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
	if (fabs(LakeBase.Resolution - 90.0) > .0001)
		LakeBase.Floating = 0;
	} // if older file

return (TotalRead);

} // EffectsLib::LakeEffect_Load()

/*===========================================================================*/

ULONG EffectsLib::LakeEffect_Save(FILE *ffile)
{
LakeEffect *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_RESOLUTION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&LakeBase.Resolution)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_OVERLAPOK, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&LakeBase.OverlapOK)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_FLOATING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&LakeBase.Floating)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

Current = Lake;
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
	Current = (LakeEffect *)Current->Next;
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

} // EffectsLib::LakeEffect_Save()

/*===========================================================================*/
