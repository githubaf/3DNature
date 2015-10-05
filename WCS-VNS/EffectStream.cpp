// EffectStream.cpp
// For managing Line Stream Effects
// Built from scratch on 06/26/97 by Gary R. Huber
// Copyright 1997 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "AppMem.h"
#include "Conservatory.h"
#include "StreamEditGUI.h"
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
#include "Database.h"
#include "Security.h"
#include "Lists.h"
#include "VectorNode.h"
#include "VectorPolygon.h"

#include "FeatureConfig.h"

StreamEffectBase::StreamEffectBase(void)
{

SetDefaults();

} // StreamEffectBase::StreamEffectBase

/*===========================================================================*/

void StreamEffectBase::SetDefaults(void)
{

Resolution = (float)90.0;
GeoRast = NULL;
OverlapOK = Floating = 1;
RefMat = NULL;

} // StreamEffectBase::SetDefaults

/*===========================================================================*/

void StreamEffectBase::SetFloating(int NewFloating, Project *CurProj)
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
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_STREAM, WCS_EFFECTSSUBCLASS_STREAM, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

} // StreamEffectBase::SetFloating

/*===========================================================================*/

void StreamEffectBase::SetFloating(int NewFloating, float NewResolution)
{
NotifyTag Changes[2];

Changes[1] = 0;

Floating = NewFloating;

if (NewFloating)
	{
	Resolution = NewResolution;
	} // if
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_STREAM, WCS_EFFECTSSUBCLASS_STREAM, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

} // StreamEffectBase::SetFloating

/*===========================================================================*/

GeoRaster *StreamEffectBase::Init(GeneralEffect *EffectList, RenderJoeList *JL, double N, double S, double E, double W, 
	double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem)
{
GeoRaster *DrawRast = NULL, *LastDrawRast, *FirstDrawRast;
double CellSize, Radius, NWLat, NWLon, SELat, SELon, CellsNS, CellsWE;
StreamEffect *MyEffect;
long CopyBounds[4];
UBYTE FillVal;

CellSize = Resolution / MetersPerDegLat;

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

if ((GeoRast = new GeoRaster(N, S, E, W, CellSize, MaxMem, UsedMem, WCS_RASTER_BANDSET_BYTE, 256, 256)) && (! GeoRast->ConstructError))
	{
	FirstDrawRast = (GeoRaster *)GeoRast->Next;
	LastDrawRast = GeoRast;
	while (JL)
		{
		if (JL->Me->TestFlags(WCS_JOEFLAG_HASKIDS) || ! JL->Me->TestFlags(WCS_JOEFLAG_ACTIVATED))
			{
			continue;
			} // if 
		if (MyEffect = (StreamEffect *)JL->Effect)
			{
			Radius = MyEffect->AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_RADIUS].CurValue;
			FillVal = 1;
			while (FirstDrawRast && FirstDrawRast->Overflow)
				{
				FirstDrawRast = (GeoRaster *)FirstDrawRast->Next;
				LastDrawRast = (GeoRaster *)LastDrawRast->Next;
				} // if
			GeoRast->PolyRasterEdgeByte(MaxMem, UsedMem, JL->Me, WCS_RASTER_EFFECT_BAND_NORMAL, FillVal, FALSE, FALSE);
			// make the copy area oversize by .001 to account for the single precision floats of Joe bounds
			JL->Me->GetTrueBounds(NWLat, NWLon, SELat, SELon);
			// round out to next even cell size from reference lat/lon 
			CellsNS = WCS_floor((SELat - RefLat) / GeoRast->CellSizeNS);
			CellsWE = WCS_floor((SELon - RefLon) / GeoRast->CellSizeEW);
			SELat = RefLat + CellsNS * GeoRast->CellSizeNS;
			SELon = RefLon + CellsWE * GeoRast->CellSizeEW;
			CellsNS = WCS_ceil((NWLat - RefLat) / GeoRast->CellSizeNS);
			CellsWE = WCS_ceil((NWLon - RefLon) / GeoRast->CellSizeEW);
			NWLat = RefLat + CellsNS * GeoRast->CellSizeNS;
			NWLon = RefLon + CellsWE * GeoRast->CellSizeEW;

			CopyBounds[0] = (long)((GeoRast->N - NWLat) / GeoRast->CellSizeNS);	// north
			CopyBounds[1] = (long)((GeoRast->N - SELat) / GeoRast->CellSizeNS);	// south
			CopyBounds[2] = (long)((GeoRast->W - NWLon) / GeoRast->CellSizeEW);	// west
			CopyBounds[3] = (long)((GeoRast->W - SELon) / GeoRast->CellSizeEW);	// east
			if (CopyBounds[0] > 0)
				CopyBounds[0] --;
			if (CopyBounds[1] < GeoRast->Rows - 1)
				CopyBounds[1] ++;
			if (CopyBounds[2] > 0)
				CopyBounds[2] --;
			if (CopyBounds[3] < GeoRast->Cols - 1)
				CopyBounds[3] ++;
			GeoRast->PolyRasterFillOutByte(WCS_RASTER_EFFECT_BAND_NORMAL, FillVal, (long)((Radius + .5) / Resolution), CopyBounds);
			if (! FirstDrawRast)
				{
				if (((LastDrawRast->Next = new GeoRaster(N, S, E, W, CellSize, MaxMem, UsedMem, WCS_RASTER_BANDSET_BYTE, 256, 256)) == NULL) || LastDrawRast->Next->ConstructError)
					break;
				FirstDrawRast = (GeoRaster *)LastDrawRast->Next;
				} // if
			DrawRast = FirstDrawRast;
			if ((DrawRast = DrawRast->PolyRasterCopyByte(GeoRast, MaxMem, UsedMem, JL->Me, (GeneralEffect *)MyEffect, WCS_RASTER_EFFECT_BAND_NORMAL, CopyBounds, OverlapOK)) == NULL)
				break;	// returns the last attempt to allocate new map
			GeoRast->ClearByte(WCS_RASTER_EFFECT_BAND_NORMAL, CopyBounds);
			} // if there truly is an effect
		JL = (RenderJoeList *)JL->Next;
		} // while
	if (DrawRast)
		{
		FirstDrawRast = GeoRast;
		GeoRast = (GeoRaster *)GeoRast->Next;
		delete FirstDrawRast;
		} // if
	} // if

// testing - create output bitmaps
/*
if (DrawRast)
	{
	char Name[256];
	int Count = 0;
	GeoRaster *DR;

	DR = DrawRast;
	DrawRast = GeoRast;
	while (DrawRast)
		{
		sprintf(Name, "d:\\Frames\\TARastTest%1d.iff", Count);
		saveILBM(8, 0, &DrawRast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL], NULL, 0, 1, 1, (short)DrawRast->Cols, (short)DrawRast->Rows, Name);
		DrawRast = (GeoRaster *)DrawRast->Next;
		Count ++;
		} // while
	DrawRast = DR;
	} // if
*/
return (DrawRast);	// if the last attempt to allocate failed then NULL will be returned

} // StreamEffectBase::Init

/*===========================================================================*/

void StreamEffectBase::Destroy(void)
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

} // StreamEffectBase::Destroy

/*===========================================================================*/

// Au garde! No error checking here for speed - be sure to verify if the given
// effect has been initialized before calling the Eval methods.

int StreamEffectBase::Eval(RenderData *Rend, VertexData *Vert)
{
double Lat, Lon, Distance, JoeElev, JoeSlope;
float VeryTempOffsets[3], CurOffsets[3];
int Done = 0, InTheArea = 0, OffEnd = 0, CurSplined = 0;
GeoRaster *Rast = GeoRast;
StreamEffect *Effect;
Joe *VeryTempVec, *CurVec;
short Priority = -1000;

CurVec = NULL;
CurStream = NULL;
CurBeach = NULL;
JoeElev = JoeSlope = 0.0;

Lat = Vert->Lat;
Lon = Vert->Lon;

// get value from first map
if (OverlapOK)
	{ // check maps until lower priority is encountered
	while (Rast)
		{
		if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
			{ // process data
			Effect = (StreamEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
			InTheArea = 1;
			if (Effect->Priority >= Priority)
				{
				VeryTempVec = Rast->JLUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
				if ((Distance = VeryTempVec->MinDistToPoint(Lat, Lon, Vert->Elev, Rend->EarthLatScaleMeters, Rend->Exageration, 
					Rend->ElevDatum, Effect->Splined, TRUE, FALSE, FALSE, OffEnd, JoeElev, VeryTempOffsets, &JoeSlope)) <= Effect->AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_RADIUS].CurValue)
					{
					Priority = Effect->Priority;
					if (Vert->Elev < Effect->MaxLevel + JoeElev)
						{
						if (Compare(Rend, Vert, Effect, VeryTempVec, JoeElev, JoeSlope, Distance, VeryTempOffsets))
							{
							CurVec = VeryTempVec;
							CurStream = Effect;
							CurOffsets[0] = VeryTempOffsets[0];
							CurOffsets[1] = VeryTempOffsets[1];
							CurOffsets[2] = VeryTempOffsets[2];
							CurSplined = Effect->Splined;
							Done = 1;
							} // if
						if (CompareBeach(Vert, Effect, JoeElev))
							CurBeach = &Effect->BeachMat;
						} // if
					} // if
				} // if equal or higher priority
			else
				break;
			} // if a value in map
		else if (Rast->Overflow)
			{
			Rast = (GeoRaster *)Rast->Next;
			continue;
			} // else if
		else
			break;
		Rast = (GeoRaster *)Rast->Next;
		} // while
	} // if overlap
else
	{ // everything in one map - nice 'n easy
	while (Rast)
		{
		if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
			{ // process data
			Effect = (StreamEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
			VeryTempVec = Rast->JLUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
			if ((Distance = VeryTempVec->MinDistToPoint(Lat, Lon, Vert->Elev, Rend->EarthLatScaleMeters, Rend->Exageration, 
					Rend->ElevDatum, Effect->Splined, TRUE, FALSE, FALSE, OffEnd, JoeElev, VeryTempOffsets, &JoeSlope)) <= Effect->AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_RADIUS].CurValue)
				{
				if (Vert->Elev < Effect->MaxLevel + JoeElev)
					{
					if (Compare(Rend, Vert, Effect, VeryTempVec, JoeElev, JoeSlope, Distance, VeryTempOffsets))
						{
						CurVec = VeryTempVec;
						CurStream = Effect;
						CurOffsets[0] = VeryTempOffsets[0];
						CurOffsets[1] = VeryTempOffsets[1];
						CurOffsets[2] = VeryTempOffsets[2];
						CurSplined = Effect->Splined;
						Done = 1;
						} // if
					if (CompareBeach(Vert, Effect, JoeElev))
						CurBeach = &Effect->BeachMat;
					} // if
				} // if
			InTheArea = 1;
			break;
			} // if a value in edge map
		else if (Rast->Overflow)
			{
			Rast = (GeoRaster *)Rast->Next;
			continue;
			} // else if
		else
			break;
		} // while
	} // else

if (Done)
	{
	if (! Vert->Lake || (Vert->WaterElev < TempLevel))
		{
		Vert->WaterElev = TempLevel;
		Vert->WaveHeight = TempWaveHt;
		Vert->VectorSlope = (float)TempSlope;
		Vert->Stream = CurStream;
		Vert->Vector = CurVec;
		Vert->VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE | WCS_TEXTURE_VECTOREFFECTTYPE_SPLINEELEV;
		Vert->VectorType |= (CurSplined ? WCS_TEXTURE_VECTOREFFECTTYPE_SPLINELATLON: 0);
		Vert->VecOffsets[0] = CurOffsets[0];
		Vert->VecOffsets[1] = CurOffsets[1];
		Vert->VecOffsets[2] = CurOffsets[2];
		//Vert->TexDataInitialized = 0;	// since we've changed some vertex values
		if (Rend->Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_WAVE])
			{
			Vert->Flags |= WCS_VERTEXDATA_FLAG_WAVEAPPLIED;
			} // if
		} // if
	if (CurBeach && (! Vert->Beach || TempBeachLevel < Vert->BeachLevel))
		{
		Vert->Beach = CurBeach;
		Vert->BeachLevel = TempBeachLevel;
		} // if
	} // if

return (Done > 0 ? 1: -InTheArea);

} // StreamEffectBase::Eval

/*===========================================================================*/

int StreamEffectBase::Compare(RenderData *Rend, VertexData *Vert, StreamEffect *Effect, Joe *JoeVec, double JoeElev, 
	double JoeSlope, double Distance, float *VecOffsets)
{
TwinMaterials BaseMat;
double CurLevel, OldWaterElev, CurWaveHt = 0.0;
float VeryTempSlope, OldSlope, VecOffsetsStash[3];
Joe *OldVec;
unsigned char OldVecType;

//get the right materials, texture evaluator needs vector slope
OldSlope = Vert->VectorSlope;
OldWaterElev = Vert->WaterElev;
OldVec = Vert->Vector;
OldVecType = Vert->VectorType;
Vert->VectorSlope = VeryTempSlope = (float)fabs(JoeSlope * 100.0);
Vert->WaterElev = JoeElev;
Vert->Vector = JoeVec;
Vert->VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE | WCS_TEXTURE_VECTOREFFECTTYPE_SPLINEELEV;
Vert->VectorType |= (Effect->Splined ? WCS_TEXTURE_VECTOREFFECTTYPE_SPLINELATLON: 0);
Rend->TexData.VectorOffsetsComputed = 0;
if (VecOffsets)
	{
	VecOffsetsStash[0] = Vert->VecOffsets[0];
	VecOffsetsStash[1] = Vert->VecOffsets[1];
	VecOffsetsStash[2] = Vert->VecOffsets[2];
	Vert->VecOffsets[0] = VecOffsets[0];
	Vert->VecOffsets[1] = VecOffsets[1];
	Vert->VecOffsets[2] = VecOffsets[2];
	Rend->TexData.VDataVecOffsetsValid = 1;
	} // if
//Vert->TexDataInitialized = 0;	// since we've changed some vertex values
if (Effect->WaterMat.GetRenderMaterial(&BaseMat, Effect->GetRenderWaterMatGradientPos(Rend, Vert)))
	{
	CurLevel = JoeElev + BaseMat.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WATERDEPTH].CurValue * BaseMat.Covg[0];
	if (Rend->Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_WAVE])
		CurWaveHt = BaseMat.Mat[0]->EvalWaves(Rend, Vert) * BaseMat.Covg[0];
	if (BaseMat.Mat[1])
		{
		CurLevel += BaseMat.Mat[1]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WATERDEPTH].CurValue * BaseMat.Covg[1];
		if (Rend->Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_WAVE])
			CurWaveHt += BaseMat.Mat[1]->EvalWaves(Rend, Vert) * BaseMat.Covg[1];
		} // if

	// restore values
	Vert->VectorSlope = OldSlope;
	Vert->WaterElev = OldWaterElev;
	Vert->Vector = OldVec;
	Vert->VectorType = OldVecType;
	Rend->TexData.VectorOffsetsComputed = 0;
	if (VecOffsets)
		{
		Vert->VecOffsets[0] = VecOffsetsStash[0];
		Vert->VecOffsets[1] = VecOffsetsStash[1];
		Vert->VecOffsets[2] = VecOffsetsStash[2];
		Rend->TexData.VDataVecOffsetsValid = 0;
		} // if

	if (CurStream)
		{
		if (TempLevel + TempWaveHt < CurLevel + CurWaveHt)
			{
			TempLevel = CurLevel;
			TempWaveHt = CurWaveHt;
			TempSlope = VeryTempSlope;
			return (1);
			} // if
		} // if
	else
		{
		TempLevel = CurLevel;
		TempWaveHt = CurWaveHt;
		TempSlope = VeryTempSlope;
		return (1);
		} // else
	} // if material found
else
	{
	Vert->VectorSlope = OldSlope;
	Vert->WaterElev = OldWaterElev;
	Vert->Vector = OldVec;
	Vert->VectorType = OldVecType;
	Rend->TexData.VectorOffsetsComputed = 0;
	if (VecOffsets)
		{
		Vert->VecOffsets[0] = VecOffsetsStash[0];
		Vert->VecOffsets[1] = VecOffsetsStash[1];
		Vert->VecOffsets[2] = VecOffsetsStash[2];
		Rend->TexData.VDataVecOffsetsValid = 0;
		} // if
	} // else

return (0);

} // StreamEffectBase::Compare

/*===========================================================================*/

int StreamEffectBase::CompareBeach(VertexData *Vert, StreamEffect *Effect, double JoeElev)
{
double BeachHt;

if (Effect->AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHHTVAR].CurValue > 0.0)
	{
	Rand.Seed64BitShift(Vert->LatSeed, Vert->LonSeed);
	BeachHt = Rand.GenPRN() * Effect->AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHHTVAR].CurValue +
		JoeElev + Effect->AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHHT].CurValue;
	} // if
else
	BeachHt = JoeElev + Effect->AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHHT].CurValue;

if (Vert->Elev <= BeachHt)
	{
	// randomize beach ht and compare again
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

} // StreamEffectBase::CompareBeach

/*===========================================================================*/

bool StreamEffectBase::Eval(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode, 
	double &SumOfAllCoverages, bool AddBeachMaterials)
{
double DistanceToVector, TestDistance, JoeElev, JoeSlope, TestJoeElev, TestJoeSlope;
float VeryTempOffsets[3], TestTempOffsets[3];
long EffectCt, CurCt;
int OffEnd = 0;
EffectJoeList *CurList;
StreamEffect *CurEffect, *BeachEffect;
MaterialList *TempMat, *BeachWaterMat;
Joe *BeachVector;
TfxDetail *FoundDetail, *CurVS;
bool Done = false, Success = true;
short TopPriority = -10000;
TwinMaterials BaseMat;

// build comprehensive list of the effects to evaluate
CurBeach = NULL;

if (! RefMat)
	{
	if (RefMat = new MaterialList())
		{
		if (! (RefMat->AddWaterProperties() && RefMat->AddVectorProperties()))
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
		if (CurList->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_STREAM)
			EffectsLib::EffectChain[EffectCt++] = CurList;
		} // for

	EffectsLib::SortEffectChain(EffectCt, WCS_EFFECTSSUBCLASS_STREAM);

	for (CurCt = 0; CurCt < EffectCt && Success; ++CurCt)
		{
		if (CurList = EffectsLib::EffectChain[CurCt])
			{
			CurEffect = (StreamEffect *)CurList->MyEffect;
			if (CurEffect->Priority < TopPriority)
				break;
			DistanceToVector = CurEffect->AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_RADIUS].CurValue + 1.0;
			if (CurList->VSData)
				{
				FoundDetail = NULL;
				for (CurVS = CurList->VSData; CurVS; CurVS = CurVS->Next)
					{
					// find the instance where the node is closest to the referenced segment
					if ((TestDistance = CurList->MyJoe->MinDistToPoint(CurNode, CurVS, Rend->EarthLatScaleMeters, Rend->Exageration, 
						Rend->ElevDatum, TRUE, FALSE, TestJoeElev, TestTempOffsets, &TestJoeSlope)) <= DistanceToVector)
						{
						DistanceToVector = TestDistance;
						FoundDetail = CurVS;
						JoeElev = TestJoeElev;
						JoeSlope = TestJoeSlope;
						VeryTempOffsets[0] = TestTempOffsets[0];
						VeryTempOffsets[1] = TestTempOffsets[1];
						VeryTempOffsets[2] = TestTempOffsets[2];
						} // if
					} // for
				if (FoundDetail)
					{
					if (TempMat = Compare(Rend, CurNode, CurEffect, CurList->MyJoe, JoeElev, JoeSlope, DistanceToVector, VeryTempOffsets, Success))
						{
						if (CompareBeach(CurNode, CurEffect, JoeElev))
							{
							CurBeach = &CurEffect->BeachMat;
							BeachEffect = CurEffect;
							BeachVector = CurList->MyJoe;
							BeachWaterMat = TempMat;
							} // if
						TopPriority = CurEffect->Priority;
						Done = true;
						} // if
					} // if
				} // if
			else
				{
				if ((TestDistance = CurList->MyJoe->MinDistToPoint(CurNode->Lat, CurNode->Lon, CurNode->Elev, Rend->EarthLatScaleMeters, Rend->Exageration, 
					Rend->ElevDatum, CurEffect->Splined, TRUE, FALSE, FALSE, OffEnd, JoeElev, VeryTempOffsets, &JoeSlope)) < DistanceToVector)
					{
					if (CurNode->Elev < CurEffect->MaxLevel + JoeElev)
						{
						if (TempMat = Compare(Rend, CurNode, CurEffect, CurList->MyJoe, JoeElev, JoeSlope, TestDistance, VeryTempOffsets, Success))
							{
							if (CompareBeach(CurNode, CurEffect, JoeElev))
								{
								CurBeach = &CurEffect->BeachMat;
								BeachEffect = CurEffect;
								BeachVector = CurList->MyJoe;
								BeachWaterMat = TempMat;
								} // if
							TopPriority = CurEffect->Priority;
							Done = true;
							} // if
						} // if
					} // if
				} // else
			} // if
		} // for

	if (Done && Success)
		{
		if (CurBeach && (float)TempBeachLevel < CurNode->NodeData->BeachLevel)
			{
			// NULL out any beach owner flags already set by lakes
			if (CurNode->FlagCheck(WCS_VECTORNODE_FLAG_BEACHEVALUATED))
				{
				for (TempMat = CurNode->NodeData->Materials; TempMat; TempMat = TempMat->NextMaterial)
					{
					if (TempMat->WaterProp && TempMat->WaterProp->BeachOwner)
						{
						TempMat->WaterProp->BeachOwner = false;
						break;
						} // if
					} // for
				} // if
			BeachWaterMat->WaterProp->BeachOwner = true;
			CurNode->NodeData->BeachLevel = (float)TempBeachLevel;
			if (AddBeachMaterials)
				{
				CurNode->FlagSet(WCS_VECTORNODE_FLAG_BEACHEVALUATED);
				if (CurBeach->GetRenderMaterial(&BaseMat, BeachEffect->GetRenderBeachMatGradientPos(Rend, BeachVector, CurNode, BeachWaterMat)))
					{
					if (TempMat = CurNode->NodeData->AddWaterVectorMaterial(BaseMat.Mat[0], BeachEffect, BeachVector, (float)BaseMat.Covg[0]))
						{
						TempMat->WaterProp->WaterDepth = BeachWaterMat->WaterProp->WaterDepth;
						TempMat->VectorProp->VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE | WCS_TEXTURE_VECTOREFFECTTYPE_SPLINEELEV;
						TempMat->VectorProp->VecOffsets[0] = BeachWaterMat->VectorProp->VecOffsets[0];
						TempMat->VectorProp->VecOffsets[1] = BeachWaterMat->VectorProp->VecOffsets[1];
						TempMat->VectorProp->VecOffsets[2] = BeachWaterMat->VectorProp->VecOffsets[2];
						SumOfAllCoverages += BaseMat.Covg[0] * (1.0 - BaseMat.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
						if (BaseMat.Mat[1])
							{
							if (TempMat = CurNode->NodeData->AddWaterVectorMaterial(BaseMat.Mat[1], BeachEffect, BeachVector, (float)BaseMat.Covg[1]))
								{
								TempMat->WaterProp->WaterDepth = BeachWaterMat->WaterProp->WaterDepth;
								TempMat->VectorProp->VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE | WCS_TEXTURE_VECTOREFFECTTYPE_SPLINEELEV;
								TempMat->VectorProp->VecOffsets[0] = BeachWaterMat->VectorProp->VecOffsets[0];
								TempMat->VectorProp->VecOffsets[1] = BeachWaterMat->VectorProp->VecOffsets[1];
								TempMat->VectorProp->VecOffsets[2] = BeachWaterMat->VectorProp->VecOffsets[2];
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
	} // if
	
return (Success);

} // StreamEffectBase::Eval

/*===========================================================================*/

MaterialList *StreamEffectBase::Compare(RenderData *Rend, VectorNode *CurNode, StreamEffect *CurEffect, Joe *CurVec, double JoeElev, 
	double JoeSlope, double Distance, float *VecOffsets, bool &Success)
{
TwinMaterials BaseMat;
double CurLevel, CurWaveHt = 0.0;
MaterialList *CurMat1, *CurMat2 = NULL;

//get the right materials, texture evaluator needs water level
CurLevel = JoeElev;
RefMat->MyVec = CurVec;
RefMat->WaterProp->WaterDepth = (float)(CurLevel - CurNode->Elev);
RefMat->VectorProp->VectorSlope = (float)fabs(JoeSlope * 100.0);
RefMat->VectorProp->VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE | WCS_TEXTURE_VECTOREFFECTTYPE_SPLINEELEV;
RefMat->VectorProp->VecOffsets[0] = VecOffsets[0];
RefMat->VectorProp->VecOffsets[1] = VecOffsets[1];
RefMat->VectorProp->VecOffsets[2] = VecOffsets[2];
if (CurEffect->WaterMat.GetRenderMaterial(&BaseMat, CurEffect->GetRenderWaterMatGradientPos(Rend, CurVec, CurNode, RefMat)))
	{
	if (CurMat1 = CurNode->NodeData->AddWaterVectorMaterial(BaseMat.Mat[0], CurEffect, CurVec, (float)BaseMat.Covg[0]))
		{
		CurLevel += BaseMat.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WATERDEPTH].CurValue * BaseMat.Covg[0];
		if (Rend->Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_WAVE])
			CurWaveHt = BaseMat.Mat[0]->EvalWaves(Rend, CurNode, CurVec, RefMat) * BaseMat.Covg[0];
		if (BaseMat.Mat[1])
			{
			if (CurMat2 = CurNode->NodeData->AddWaterVectorMaterial(BaseMat.Mat[1], CurEffect, CurVec, (float)BaseMat.Covg[1]))
				{
				CurLevel += BaseMat.Mat[1]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WATERDEPTH].CurValue * BaseMat.Covg[1];
				if (Rend->Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_WAVE])
					CurWaveHt += BaseMat.Mat[1]->EvalWaves(Rend, CurNode, CurVec, RefMat) * BaseMat.Covg[1];
				CurMat2->WaterProp->WaterDepth = (float)(CurLevel - CurNode->Elev);
				CurMat2->WaterProp->WaveHeight = (float)CurWaveHt;
				CurMat2->VectorProp->VectorSlope = RefMat->VectorProp->VectorSlope;
				CurMat2->VectorProp->VecOffsets[0] = VecOffsets[0];
				CurMat2->VectorProp->VecOffsets[1] = VecOffsets[1];
				CurMat2->VectorProp->VecOffsets[2] = VecOffsets[2];
				CurMat2->VectorProp->VectorType = RefMat->VectorProp->VectorType;
				} // if
			else
				Success = false;
			} // if
		CurMat1->WaterProp->WaterDepth = (float)(CurLevel - CurNode->Elev);
		CurMat1->WaterProp->WaveHeight = (float)CurWaveHt;
		CurMat1->VectorProp->VectorSlope = RefMat->VectorProp->VectorSlope;
		CurMat1->VectorProp->VecOffsets[0] = VecOffsets[0];
		CurMat1->VectorProp->VecOffsets[1] = VecOffsets[1];
		CurMat1->VectorProp->VecOffsets[2] = VecOffsets[2];
		CurMat1->VectorProp->VectorType = RefMat->VectorProp->VectorType;
		return (CurMat1);
		} // if
	else
		Success = false;
	} // if material found
	
return (NULL);

} // StreamEffectBase::Compare

/*===========================================================================*/

bool StreamEffectBase::CompareBeach(VectorNode *CurNode, StreamEffect *CurEffect, double JoeElev)
{
double BeachHt;
unsigned long LonSeed, LatSeed;
						
if (CurEffect->AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHHTVAR].CurValue > 0.0)
	{
	LonSeed = (ULONG)((CurNode->Lon - WCS_floor(CurNode->Lon)) * ULONG_MAX);
	LatSeed = (ULONG)((CurNode->Lat - WCS_floor(CurNode->Lat)) * ULONG_MAX);

	Rand.Seed64BitShift(LatSeed, LonSeed);
	BeachHt = Rand.GenPRN() * CurEffect->AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHHTVAR].CurValue +
		JoeElev + CurEffect->AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHHT].CurValue;
	} // if
else
	BeachHt = JoeElev + CurEffect->AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHHT].CurValue;

if (CurNode->Elev <= BeachHt)
	{
	// randomize beach ht and compare again
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

} // StreamEffectBase::CompareBeach

/*===========================================================================*/
/*===========================================================================*/

StreamEffect::StreamEffect()
: GeneralEffect(NULL), BeachMat(this, 1, WCS_EFFECTS_MATERIALTYPE_BEACH), WaterMat(this, 1, WCS_EFFECTS_MATERIALTYPE_WATER) 
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_STREAM;
SetDefaults();

} // StreamEffect::StreamEffect

/*===========================================================================*/

StreamEffect::StreamEffect(RasterAnimHost *RAHost)
: GeneralEffect(RAHost), BeachMat(this, 1, WCS_EFFECTS_MATERIALTYPE_BEACH), WaterMat(this, 1, WCS_EFFECTS_MATERIALTYPE_WATER)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_STREAM;
SetDefaults();

} // StreamEffect::StreamEffect

/*===========================================================================*/

StreamEffect::StreamEffect(RasterAnimHost *RAHost, EffectsLib *Library, StreamEffect *Proto)
: GeneralEffect(RAHost), BeachMat(this, 1, WCS_EFFECTS_MATERIALTYPE_BEACH), WaterMat(this, 1, WCS_EFFECTS_MATERIALTYPE_WATER)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_STREAM;
Prev = Library->LastStream;
if (Library->LastStream)
	{
	Library->LastStream->Next = this;
	Library->LastStream = this;
	} // if
else
	{
	Library->Stream = Library->LastStream = this;
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
	strcpy(NameBase, "Stream");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // StreamEffect::StreamEffect

/*===========================================================================*/

StreamEffect::~StreamEffect()
{
long Ct;
RootTexture *DelTex;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->SEG && GlobalApp->GUIWins->SEG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->SEG;
		GlobalApp->GUIWins->SEG = NULL;
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

} // StreamEffect::~StreamEffect

/*===========================================================================*/

void StreamEffect::SetDefaults(void)
{
/*
double EffectDefault[WCS_EFFECTS_STREAM_NUMANIMPAR] = {3.0, 15.0, 4.0, 2.0, 2.0, 3.0, 30.0, 60.0, 5.0, 50.0,
														.5, -.5, 80.0, 0.0, 10.0, 1.0};
double RangeDefaults[WCS_EFFECTS_STREAM_NUMANIMPAR][3] = {	FLT_MAX, 0.0, 1.0, FLT_MAX, 0.0, 1.0,
															FLT_MAX, -FLT_MAX, 1.0, FLT_MAX, -FLT_MAX, 1.0,
															FLT_MAX, 0.0, 1.0, FLT_MAX, 0.0, 1.0,
															FLT_MAX, 0.0, 1.0, FLT_MAX, 0.0, 1.0,
															100.0, 0.0, 5.0, 100.0, 0.0, 5.0,
															100.0, -100.0, .1, 100.0, -100.0, .1,
															100.0, 0.0, 5.0, 100.0, 0.0, 5.0,
															FLT_MAX, 0.0, 1.0, 200.0, 0.0, .1};
*/
double EffectDefault[WCS_EFFECTS_STREAM_NUMANIMPAR] = {.5, .5, 0.0, 0.0, 50.0};
double RangeDefaults[WCS_EFFECTS_STREAM_NUMANIMPAR][3] = {FLT_MAX, -FLT_MAX, 1.0,
														FLT_MAX, 0.0, 1.0,
														1.0, 0.0, .01,
														1.0, 0.0, .01,
														FLT_MAX, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_EFFECTS_STREAM_NUMANIMPAR; Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	TexRoot[Ct] = NULL;
	} // for
PixelTexturesExist = 0;
Splined = 0;

AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_RADIUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHHTVAR].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHMATDRIVER].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_WATERMATDRIVER].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_RADIUS].SetNoNodes(1);

} // StreamEffect::SetDefaults

/*===========================================================================*/

void StreamEffect::Copy(StreamEffect *CopyTo, StreamEffect *CopyFrom)
{

BeachMat.Copy(&CopyTo->BeachMat, &CopyFrom->BeachMat);
WaterMat.Copy(&CopyTo->WaterMat, &CopyFrom->WaterMat);
CopyTo->Splined = CopyFrom->Splined;
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // StreamEffect::Copy

/*===========================================================================*/

ULONG StreamEffect::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
RootTexture *DelTex;
#ifdef WCS_BUILD_VNS
char MatchName[WCS_EFFECT_MAXNAMELENGTH];
#endif // WCS_BUILD_VNS

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
					case WCS_EFFECTS_STREAM_SPLINED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Splined, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_STREAM_BEACHMAT:
						{
						BytesRead = BeachMat.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_STREAM_WATERMAT:
						{
						BytesRead = WaterMat.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_STREAM_BEACHHT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHHT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_STREAM_BEACHHTVAR:
						{
						BytesRead = AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHHTVAR].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_STREAM_BEACHMATDRIVER:
						{
						BytesRead = AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHMATDRIVER].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_STREAM_WATERMATDRIVER:
						{
						BytesRead = AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_WATERMATDRIVER].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_STREAM_RADIUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_RADIUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_STREAM_TEXBEACHHTVAR:
						{
						if (TexRoot[WCS_EFFECTS_STREAM_TEXTURE_BEACHHTVAR] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_STREAM_TEXTURE_BEACHHTVAR]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_STREAM_TEXBEACHMATDRIVER:
						{
						if (DelTex = TexRoot[WCS_EFFECTS_STREAM_TEXTURE_BEACHMATDRIVER])
							{
							TexRoot[WCS_EFFECTS_STREAM_TEXTURE_BEACHMATDRIVER] = NULL;
							delete DelTex;
							} // if
						if (TexRoot[WCS_EFFECTS_STREAM_TEXTURE_BEACHMATDRIVER] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_STREAM_TEXTURE_BEACHMATDRIVER]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_STREAM_TEXWATERMATDRIVER:
						{
						if (DelTex = TexRoot[WCS_EFFECTS_STREAM_TEXTURE_WATERMATDRIVER])
							{
							TexRoot[WCS_EFFECTS_STREAM_TEXTURE_WATERMATDRIVER] = NULL;
							delete DelTex;
							} // if
						if (TexRoot[WCS_EFFECTS_STREAM_TEXTURE_WATERMATDRIVER] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_STREAM_TEXTURE_WATERMATDRIVER]->Load(ffile, Size, ByteFlip);
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

} // StreamEffect::Load

/*===========================================================================*/

unsigned long int StreamEffect::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_EFFECTS_STREAM_NUMANIMPAR] = {WCS_EFFECTS_STREAM_BEACHHT,
																WCS_EFFECTS_STREAM_BEACHHTVAR,
																WCS_EFFECTS_STREAM_BEACHMATDRIVER,
																WCS_EFFECTS_STREAM_WATERMATDRIVER,
																WCS_EFFECTS_STREAM_RADIUS};
unsigned long int TextureItemTag[WCS_EFFECTS_STREAM_NUMTEXTURES] = {WCS_EFFECTS_STREAM_TEXBEACHHTVAR,
																	WCS_EFFECTS_STREAM_TEXBEACHMATDRIVER,
																	WCS_EFFECTS_STREAM_TEXWATERMATDRIVER};

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
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_STREAM_SPLINED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Splined)) == NULL)
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

ItemTag = WCS_EFFECTS_STREAM_BEACHMAT + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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

ItemTag = WCS_EFFECTS_STREAM_WATERMAT + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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

} // StreamEffect::Save

/*===========================================================================*/

void StreamEffect::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->SEG)
	{
	delete GlobalApp->GUIWins->SEG;
	}
GlobalApp->GUIWins->SEG = new StreamEditGUI(GlobalApp->AppEffects, GlobalApp->MainProj, GlobalApp->AppDB, GlobalApp->AppImages, this);
if(GlobalApp->GUIWins->SEG)
	{
	GlobalApp->GUIWins->SEG->Open(GlobalApp->MainProj);
	}

} // StreamEffect::Edit

/*===========================================================================*/

char *StreamEffectCritterNames[WCS_EFFECTS_STREAM_NUMANIMPAR] = {"Beach Height (m)", "Beach Height Variation (m)",
															"Beach Material Driver (%)", "Water Material Driver (%)",
															"Effect Radius (m)"};

char *StreamEffectTextureNames[WCS_EFFECTS_STREAM_NUMTEXTURES] = {"Beach Height Variation (%)", 
						"Beach Material Driver (%)", "Water Material Driver (%)"};

char *StreamEffect::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (StreamEffectCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (StreamEffectTextureNames[Ct]);
		} // if
	} // for
if (Test == &BeachMat)
	return ("Beach");
if (Test == &WaterMat)
	return ("Water");
return ("");

} // StreamEffect::GetCritterName

/*===========================================================================*/

char *StreamEffect::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as a Stream Texture! Remove anyway?");

} // StreamEffect::OKRemoveRaster

/*===========================================================================*/

char *StreamEffect::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? StreamEffectTextureNames[TexNumber]: (char*)"");

} // StreamEffect::GetTextureName

/*===========================================================================*/

RootTexture *StreamEffect::NewRootTexture(long TexNumber)
{
char ApplyToColor = 0;
char ApplyToDisplace = 0;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // StreamEffect::NewRootTexture

/*===========================================================================*/

int StreamEffect::FindnRemove3DObjects(Object3DEffect *RemoveMe)
{
int Found = 0;

if (BeachMat.FindnRemove3DObjects(RemoveMe))
	Found = 1;
if (WaterMat.FindnRemove3DObjects(RemoveMe))
	Found = 1;

return (Found);

} // StreamEffect::FindnRemove3DObjects

/*===========================================================================*/

int StreamEffect::SetToTime(double Time)
{
long Found = 0;

if (GeneralEffect::SetToTime(Time))
	Found = 1;
if (BeachMat.SetToTime(Time))
	Found = 1;
if (WaterMat.SetToTime(Time))
	Found = 1;

return (Found);

} // StreamEffect::SetToTime

/*===========================================================================*/

int StreamEffect::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
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

} // StreamEffect::InitToRender

/*===========================================================================*/

int StreamEffect::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{
GradientCritter *CurGrad = WaterMat.Grad;
double WaveLevel;

if (! WaterMat.InitFrameToRender(Lib, Rend))
	return (0);

MaxLevel = -FLT_MAX;
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
	CurGrad = CurGrad->Next;
	} // while

// calculate beach top elev
MaxBeachLevel = AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHHT].CurValue + 
	AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHHTVAR].CurValue;

MaxLevel = max(MaxBeachLevel, MaxLevel);

if (! BeachMat.InitFrameToRender(Lib, Rend))
	return (0);

return (GeneralEffect::InitFrameToRender(Lib, Rend));

} // StreamEffect::InitFrameToRender

/*===========================================================================*/

long StreamEffect::InitImageIDs(long &ImageID)
{
long NumImages = 0;

NumImages += BeachMat.InitImageIDs(ImageID);
NumImages += WaterMat.InitImageIDs(ImageID);
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // StreamEffect::InitImageIDs

/*===========================================================================*/

int StreamEffect::BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves,
	EffectList **Queries, EffectList **Themes, EffectList **Coords)
{

if (! BeachMat.BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
	return (0);
if (! WaterMat.BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
	return (0);

return (1);

} // StreamEffect::BuildFileComponentsList

/*===========================================================================*/

long StreamEffect::GetKeyFrameRange(double &FirstKey, double &LastKey)
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

} // StreamEffect::GetKeyFrameRange

/*===========================================================================*/

char StreamEffect::GetRAHostDropOK(long DropType)
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

} // StreamEffect::GetRAHostDropOK

/*===========================================================================*/

int StreamEffect::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
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
	if (this != (StreamEffect *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (StreamEffect *)DropSource->DropSource);
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

} // StreamEffect::ProcessRAHostDragDrop

/*===========================================================================*/

int StreamEffect::GetRAHostAnimated(void)
{

if (GeneralEffect::GetRAHostAnimated())
	return (1);
if (BeachMat.GetRAHostAnimated())
	return (1);
if (WaterMat.GetRAHostAnimated())
	return (1);

return (0);

} // StreamEffect::GetRAHostAnimated

/*===========================================================================*/

bool StreamEffect::AnimateMaterials(void)
{

if (GeneralEffect::AnimateMaterials())
	return (true);
if (BeachMat.AnimateMaterials())
	return (true);
if (WaterMat.AnimateMaterials())
	return (true);

return (false);

} // StreamEffect::AnimateMaterials

/*===========================================================================*/

unsigned long StreamEffect::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_STREAM | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // StreamEffect::GetRAFlags

/*===========================================================================*/

int StreamEffect::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
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
				case WCS_EFFECTS_STREAM_ANIMPAR_BEACHHTVAR:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_STREAM_TEXTURE_BEACHHTVAR);
					break;
					} // 
				case WCS_EFFECTS_STREAM_ANIMPAR_BEACHMATDRIVER:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_STREAM_TEXTURE_BEACHMATDRIVER);
					break;
					} // 
				case WCS_EFFECTS_STREAM_ANIMPAR_WATERMATDRIVER:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_STREAM_TEXTURE_WATERMATDRIVER);
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
				case WCS_EFFECTS_STREAM_TEXTURE_BEACHHTVAR:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_STREAM_ANIMPAR_BEACHHTVAR);
					break;
					} // 
				case WCS_EFFECTS_STREAM_TEXTURE_BEACHMATDRIVER:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_STREAM_ANIMPAR_BEACHMATDRIVER);
					break;
					} // 
				case WCS_EFFECTS_STREAM_TEXTURE_WATERMATDRIVER:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_STREAM_ANIMPAR_WATERMATDRIVER);
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

} // StreamEffect::GetAffiliates

/*===========================================================================*/

RasterAnimHost *StreamEffect::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
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

} // StreamEffect::GetRAHostChild

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int StreamEffect::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
StreamEffect *CurrentStream = NULL;
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
					else if (! strnicmp(ReadBuf, "Stream", 8))
						{
						if (CurrentStream = new StreamEffect(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentStream->Load(ffile, Size, ByteFlip)) == Size)
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

if (Success == 1 && CurrentStream)
	{
	if (OldBoundsLoaded && GlobalApp->AppDB->FillDEMBounds(&CurBounds))
		{
		if (LoadToEffects->GetListPtr(WCS_EFFECTSSUBCLASS_WAVE) && UserMessageYN("Load Stream", "Do you wish the loaded Stream's Wave positions\n to be scaled to current DEM bounds?"))
			{
			for (CurrentWave = (WaveEffect *)LoadToEffects->GetListPtr(WCS_EFFECTSSUBCLASS_WAVE); CurrentWave; CurrentWave = (WaveEffect *)CurrentWave->Next)
				{
				CurrentWave->ScaleToDEMBounds(&OldBounds, &CurBounds);
				} // for
			} // if
		} // if
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentStream);
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

} // StreamEffect::LoadObject

/*===========================================================================*/

int StreamEffect::SaveObject(FILE *ffile, const char *SuppliedFileName)
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

// StreamEffect
strcpy(StrBuf, "Stream");
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
			} // if StreamEffect saved 
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

} // StreamEffect::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::StreamEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
StreamEffect *Current;
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
						BytesRead = ReadBlock(ffile, (char *)&StreamBase.Resolution, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_OVERLAPOK:
						{
						BytesRead = ReadBlock(ffile, (char *)&StreamBase.OverlapOK, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_FLOATING:
						{
						BytesRead = ReadBlock(ffile, (char *)&StreamBase.Floating, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						FloatingLoaded = 1;
						break;
						}
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new StreamEffect(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (StreamEffect *)FindDuplicateByName(Current->EffectType, Current))
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
	if (fabs(StreamBase.Resolution - 90.0) > .0001)
		StreamBase.Floating = 0;
	} // if older file

return (TotalRead);

} // EffectsLib::StreamEffect_Load()

/*===========================================================================*/

ULONG EffectsLib::StreamEffect_Save(FILE *ffile)
{
StreamEffect *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_RESOLUTION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&StreamBase.Resolution)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_OVERLAPOK, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&StreamBase.OverlapOK)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_FLOATING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&StreamBase.Floating)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

Current = Stream;
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
					} // if stream effect saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (StreamEffect *)Current->Next;
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

} // EffectsLib::StreamEffect_Save()

/*===========================================================================*/
