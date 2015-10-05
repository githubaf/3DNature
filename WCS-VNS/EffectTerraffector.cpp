// EffectTerraffector.cpp
// For managing Line Terraffector Effects
// Built from scratch on 06/26/97 by Gary R. Huber
// Copyright 1997 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "AppMem.h"
#include "Conservatory.h"
#include "TerraffectorEditGUI.h"
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

#define WCS_TERRAFFECTOR_SLOPEELEV_TOLERANCE	((float)1.0E-3)

TerraffectorEffectBase::TerraffectorEffectBase(void)
{

SetDefaults();

} // TerraffectorEffectBase::TerraffectorEffectBase

/*===========================================================================*/

void TerraffectorEffectBase::SetDefaults(void)
{

Resolution = (float)90.0;
GeoRast = NULL;
EdgesExist = GradientsExist = 0;
OverlapOK = Floating = 1;
RefMat = NULL;
CurSlope = -1.0;

} // TerraffectorEffectBase::SetDefaults

/*===========================================================================*/

void TerraffectorEffectBase::SetFloating(int NewFloating, Project *CurProj)
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
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_TERRAFFECTOR, WCS_EFFECTSSUBCLASS_TERRAFFECTOR, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

} // TerraffectorEffectBase::SetFloating

/*===========================================================================*/

void TerraffectorEffectBase::SetFloating(int NewFloating, float NewResolution)
{
NotifyTag Changes[2];

Changes[1] = 0;

Floating = NewFloating;

if (NewFloating)
	{
	Resolution = NewResolution;
	} // if
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_TERRAFFECTOR, WCS_EFFECTSSUBCLASS_TERRAFFECTOR, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

} // TerraffectorEffectBase::SetFloating

/*===========================================================================*/

GeoRaster *TerraffectorEffectBase::Init(GeneralEffect *EffectList, RenderJoeList *JL, double N, double S, double E, double W, 
	double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem)
{
double CellSize, Radius, NWLat, NWLon, SELat, SELon, CellsNS, CellsWE;
GeoRaster *DrawRast = NULL, *LastDrawRast, *FirstDrawRast;
TerraffectorEffect *MyEffect;
long CopyBounds[4];
UBYTE FillVal;

CellSize = Resolution / MetersPerDegLat;

// sort JoeList according to whatever is the correct priority order for this effect
JL->SortPriority(OverlapOK ? WCS_JOELIST_SORTORDER_HILO: WCS_JOELIST_SORTORDER_LOHI);
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
		if (MyEffect = (TerraffectorEffect *)JL->Effect)
			{
			Radius = max(MyEffect->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_RADIUS].CurValue, MyEffect->MaxSectionDist);
			FillVal = 1;
			while (FirstDrawRast && FirstDrawRast->Overflow)
				{
				FirstDrawRast = (GeoRaster *)FirstDrawRast->Next;
				LastDrawRast = (GeoRaster *)LastDrawRast->Next;
				} // if
			if (JL->Me->GetLineStyle() >= 4)
				GeoRast->PolyRasterEdgeByte(MaxMem, UsedMem, JL->Me, WCS_RASTER_EFFECT_BAND_NORMAL, FillVal, FALSE, FALSE);
			else
				GeoRast->PolyRasterEdgeBytePoint(MaxMem, UsedMem, JL->Me, WCS_RASTER_EFFECT_BAND_NORMAL, FillVal, FALSE, FALSE);
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
		saveILBM(8, 0, &DrawRast->ByteMap, NULL, 0, 1, 1, (short)DrawRast->Cols, (short)DrawRast->Rows, Name);
		DrawRast = DrawRast->Next;
		Count ++;
		} // while
	DrawRast = DR;
	} // if
*/
return (DrawRast);	// if the last attempt to allocate failed then NULL will be returned

} // TerraffectorEffectBase::Init

/*===========================================================================*/

void TerraffectorEffectBase::Destroy(void)
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

} // TerraffectorEffectBase::Destroy

/*===========================================================================*/

// Au garde! No error checking here for speed - be sure to verify if the given
// effect has been initialized before calling the Eval methods.

int TerraffectorEffectBase::Eval(RenderData *Rend, VertexData *Vert)
{
double Lat, Lon, Distance, JoeElev, CurJoeElev, JoeSlope;
float VeryTempOffsets[3], CurOffsets[3];
int Done = 0, InTheArea = 0, OffEnd = 0, CurSplined = 0;
GeoRaster *Rast = GeoRast;
TerraffectorEffect *Effect;
Joe *VeryTempVec, *CurVec;
short Priority = -1000;

CurEco = NULL;
CurVec = NULL;
CurRough = LastSegPercentDistance = 0.0;
HighestSegPriority = -1000;
Lat = Vert->Lat;
Lon = Vert->Lon;
OrigElev = CurElev = Vert->Elev - Vert->Displacement;
Rand.Seed64BitShift(Vert->LatSeed + WCS_SEEDOFFSET_TERRAFFECTOR, Vert->LonSeed + WCS_SEEDOFFSET_TERRAFFECTOR);

// get value from first map
if (OverlapOK)
	{ // check maps until lower priority is encountered
	while (Rast)
		{
		if (Rast->GetValidByteCell(WCS_RASTER_EFFECT_BAND_NORMAL, Lat, Lon) >= 0)
			{ // process data
			Effect = (TerraffectorEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
			if (Effect->Priority >= Priority)
				{
				VeryTempVec = Rast->JLUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
				if ((Distance = VeryTempVec->MinDistToPoint(Lat, Lon, Vert->Elev, Rend->EarthLatScaleMeters, Rend->Exageration, 
					Rend->ElevDatum, Effect->Splined, TRUE, FALSE, FALSE, OffEnd, JoeElev, VeryTempOffsets, &JoeSlope)) <= Effect->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_RADIUS].CurValue)
					{
					Priority = Effect->Priority;
					if (Compare(Rend, Vert, Effect, VeryTempVec, JoeElev, JoeSlope, Distance, VeryTempOffsets))
						{
						CurVec = VeryTempVec;
						CurOffsets[0] = VeryTempOffsets[0];
						CurOffsets[1] = VeryTempOffsets[1];
						// elevation coord not stored since it may be modified by tfx
						CurJoeElev = JoeElev;
						CurSplined = Effect->Splined;
						Done = 1;
						} // if
					} // if within effect radius
				} // if equal or higher effect priority
			else
				break;
			InTheArea = 1;
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
			Effect = (TerraffectorEffect *)Rast->LUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
			VeryTempVec = Rast->JLUT[Rast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL][Rast->RasterCell]];
			if ((Distance = VeryTempVec->MinDistToPoint(Lat, Lon, Vert->Elev, Rend->EarthLatScaleMeters, Rend->Exageration, 
				Rend->ElevDatum, Effect->Splined, TRUE, FALSE, FALSE, OffEnd, JoeElev, VeryTempOffsets, &JoeSlope)) <= Effect->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_RADIUS].CurValue)
				{
				Priority = Effect->Priority;
				if (Compare(Rend, Vert, Effect, VeryTempVec, JoeElev, JoeSlope, Distance, VeryTempOffsets))
					{
					CurVec = VeryTempVec;
					CurOffsets[0] = VeryTempOffsets[0];
					CurOffsets[1] = VeryTempOffsets[1];
					// elevation coord not stored since it may be modified by tfx
					CurJoeElev = JoeElev;
					CurSplined = Effect->Splined;
					Done = 1;
					} // if
				} // if within effect radius
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
	Vert->Elev = CurElev;
	Vert->Elev += (Vert->Displacement * CurRough);
	Vert->Displacement *= CurRough;
	if (CurEco)
		{
		Vert->Eco = CurEco;
		Vert->Vector = CurVec;
		Vert->VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE | WCS_TEXTURE_VECTOREFFECTTYPE_SPLINEELEV;
		Vert->VectorType |= (CurSplined ? WCS_TEXTURE_VECTOREFFECTTYPE_SPLINELATLON: 0);
		Vert->VecOffsets[0] = CurOffsets[0];
		Vert->VecOffsets[1] = CurOffsets[1];
		Vert->VecOffsets[2] = (float)(Vert->Elev - CurJoeElev);
		} // if
	} // if

return (Done > 0 ? 1: -InTheArea);

} // TerraffectorEffectBase::Eval

/*===========================================================================*/

int TerraffectorEffectBase::Compare(RenderData *Rend, VertexData *Vert, TerraffectorEffect *Effect, Joe *JoeVec, 
	double JoeElev, double JoeSlope, double Distance, float *VecOffsets)
{
double EcoMixing, EcoDistance, DistFromPrevNode, CurSegWidth, MixingMaxElev, MixingMinElev, CurSlopeMaxElev, CurSlopeMinElev,
	CurSegRough, SectionElev, CurSegPercentDistance, LastSegPercentDistanceStash, TexFactor, TexOpacity, Value[3];
float OldSlope, VecOffsetsStash[3];
EcosystemEffect *CurSegEco;
Joe *OldVec;
short CurSegPriority, HighestSegPriorityStash;
unsigned char OldVecType;

LastSegPercentDistanceStash = LastSegPercentDistance;
HighestSegPriorityStash = HighestSegPriority;

SectionElev = Effect->ADSection.GetValue(0, Distance);

if (Effect->Absolute == WCS_EFFECT_RELATIVETOJOE)
	SectionElev += JoeElev;
else if (Effect->Absolute == WCS_EFFECT_RELATIVETOGROUND)
	{
	if (Effect->ApplyToOrigElev)
		SectionElev += OrigElev;
	else
		SectionElev += CurElev;
	} // else if
// else its absolute elevation and there's no change

// evaluate texture
TexFactor = Effect->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_INTENSITY].CurValue;
if (Effect->GetEnabledTexture(WCS_EFFECTS_TERRAFFECTOR_TEXTURE_INTENSITY))
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	OldSlope = Vert->VectorSlope;
	OldVec = Vert->Vector;
	OldVecType = Vert->VectorType;
	Vert->VectorSlope = (float)fabs(JoeSlope * 100.0);
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
	if ((TexOpacity = Effect->TexRoot[WCS_EFFECTS_TERRAFFECTOR_TEXTURE_INTENSITY]->Eval(Value, Rend->TransferTextureData(Vert))) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			// Value[0] has already been diminished by the texture's opacity
			TexFactor *= (1.0 - TexOpacity + Value[0]);
			} // if
		else
			TexFactor *= Value[0];
		} // if
	// restore values
	Vert->VectorSlope = OldSlope;
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
	//Vert->TexDataInitialized = 0;	// since we've changed some vertex values
	} // if
SectionElev = CurElev + (SectionElev - CurElev) * TexFactor;

if (Distance > Effect->MaxSectionDist)
	{
	CurSegPriority = Effect->SlopePriority;
	if (CurSegPriority >= HighestSegPriority)
		{
		DistFromPrevNode = Distance - Effect->MaxSectionDist;
		CurSegWidth = Effect->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_RADIUS].CurValue - Effect->MaxSectionDist;
		// find range of elevations that will not be affected by terraffector
		MixingMaxElev = DistFromPrevNode * Effect->MaxSlope;	// cut
		MixingMinElev = DistFromPrevNode * Effect->MinSlope;	// fill
		CurSlopeMaxElev = SectionElev + MixingMaxElev;
		CurSlopeMinElev = SectionElev + MixingMinElev;
		CurSegEco = Effect->SlopeEco && Effect->SlopeEco->Enabled ? Effect->SlopeEco: NULL;
		CurSegRough = Effect->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_SLOPEROUGHNESS].CurValue;
		EcoMixing = Rand.GenGauss();
		if (EcoMixing < 0.0)
			EcoMixing = 0.0;
		EcoMixing = (Effect->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_SLOPEECOMIXING].CurValue) * EcoMixing;
		MixingMaxElev += (MixingMaxElev * EcoMixing + SectionElev);
		MixingMinElev += (MixingMinElev * EcoMixing + SectionElev);
		if (CurSegPriority > HighestSegPriority)
			{
			HighestSegPriority = CurSegPriority;
			LastSegPercentDistance = CurSegWidth > 0.0 ? 1.0 - DistFromPrevNode / CurSegWidth: 1.0;
			if (Effect->CompareType == WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCDEC)
				{
				if (CurSlopeMaxElev < CurElev)	// cut
					{
					if (CurElev > MixingMaxElev || CurElev < MixingMinElev)
						CurEco = CurSegEco;
					CurElev = CurSlopeMaxElev;
					CurRough = CurSegRough;
					return (1);
					} // if
				else if (CurSlopeMinElev > CurElev)	// fill
					{
					if (CurElev > MixingMaxElev || CurElev < MixingMinElev)
						CurEco = CurSegEco;
					CurElev = CurSlopeMinElev;
					CurRough = CurSegRough;
					return (1);
					} // else if 
				} // if
			else if (Effect->CompareType == WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCREASE)
				{
				if (CurSlopeMinElev > CurElev)	// fill
					{
					if (CurElev > MixingMaxElev || CurElev < MixingMinElev)
						CurEco = CurSegEco;
					CurElev = CurSlopeMinElev;
					CurRough = CurSegRough;
					return (1);
					} // else if 
				} // else if increase only
			else
				{
				if (CurSlopeMaxElev < CurElev)	// cut
					{
					if (CurElev > MixingMaxElev || CurElev < MixingMinElev)
						CurEco = CurSegEco;
					CurElev = CurSlopeMaxElev;
					CurRough = CurSegRough;
					return (1);
					} // if
				} // else decrease only
			} // if segment priority higher
		else if (CurSegPriority == HighestSegPriority)
			{
			CurSegPercentDistance = CurSegWidth > 0.0 ? 1.0 - DistFromPrevNode / CurSegWidth: 1.0;
			if (Effect->CompareType == WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCDEC)
				{
				if (CurSlopeMaxElev < CurElev)	// cut
					{
					if (CurElev > MixingMaxElev || CurElev < MixingMinElev)
						CurEco = CurSegEco;
					if (CurSegPercentDistance > 0.0 || LastSegPercentDistance > 0.0)
						CurElev = (CurSlopeMaxElev * CurSegPercentDistance + CurElev * LastSegPercentDistance) / (CurSegPercentDistance + LastSegPercentDistance);
					else
						CurElev = (CurSlopeMaxElev + CurElev) * 0.5;
					CurRough = CurSegRough;
					LastSegPercentDistance = CurSegPercentDistance;
					return (1);
					} // if
				if (CurSlopeMinElev > CurElev)	// fill
					{
					if (CurElev > MixingMaxElev || CurElev < MixingMinElev)
						CurEco = CurSegEco;
					if (CurSegPercentDistance > 0.0 || LastSegPercentDistance > 0.0)
						CurElev = (CurSlopeMinElev * CurSegPercentDistance + CurElev * LastSegPercentDistance) / (CurSegPercentDistance + LastSegPercentDistance);
					else
						CurElev = (CurSlopeMinElev + CurElev) * 0.5;
					CurRough = CurSegRough;
					LastSegPercentDistance = CurSegPercentDistance;
					return (1);
					} // if
				} // if
			else if (Effect->CompareType == WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCREASE)
				{
				if (CurSlopeMinElev > CurElev)	// fill
					{
					if (CurElev > MixingMaxElev || CurElev < MixingMinElev)
						CurEco = CurSegEco;
					if (CurSegPercentDistance > 0.0 || LastSegPercentDistance > 0.0)
						CurElev = (CurSlopeMinElev * CurSegPercentDistance + CurElev * LastSegPercentDistance) / (CurSegPercentDistance + LastSegPercentDistance);
					else
						CurElev = (CurSlopeMinElev + CurElev) * 0.5;
					CurRough = CurSegRough;
					LastSegPercentDistance = CurSegPercentDistance;
					return (1);
					} // else if
				} // else if increase only
			else
				{
				if (CurSlopeMaxElev < CurElev)	// cut
					{
					if (CurElev > MixingMaxElev || CurElev < MixingMinElev)
						CurEco = CurSegEco;
					if (CurSegPercentDistance > 0.0 || LastSegPercentDistance > 0.0)
						CurElev = (CurSlopeMaxElev * CurSegPercentDistance + CurElev * LastSegPercentDistance) / (CurSegPercentDistance + LastSegPercentDistance);
					else
						CurElev = (CurSlopeMaxElev + CurElev) * 0.5;
					CurRough = CurSegRough;
					LastSegPercentDistance = CurSegPercentDistance;
					return (1);
					} // else if
				} // else decrease only
			} // else if segment priority same
		} // if segment priority >=
	} // if beyond last key distance
else
	{
	Effect->ADSection.FetchSegmentData(CurSegPriority, CurSegRough, EcoMixing, DistFromPrevNode, CurSegWidth, Distance);
	// testing - use total distance instead of distance from start of segment
	DistFromPrevNode = Distance;
	// end test
	EcoDistance = Distance + (EcoMixing * .5 * Rand.GenGauss());
	if (EcoDistance < Effect->MaxSectionDist)
		Effect->ADSection.FetchSegmentEco(CurSegEco, EcoDistance);
	else
		CurSegEco = Effect->SlopeEco;
	CurSegEco = CurSegEco && CurSegEco->Enabled ? CurSegEco: NULL;
	if (CurSegPriority > HighestSegPriority)
		{
		HighestSegPriority = CurSegPriority;
		// testing - use total distance instead of distance from start of segment
		//LastSegPercentDistance = CurSegWidth > 0.0 ? 1.0 - DistFromPrevNode / CurSegWidth: 1.0;
		LastSegPercentDistance = Effect->MaxSectionDist > 0.0 ? 1.0 - Distance / Effect->MaxSectionDist: 1.0;
		// end test
		if (Effect->CompareType == WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCDEC)
			{
			CurElev = SectionElev;
			CurRough = CurSegRough;
			CurEco = CurSegEco;
			return (1);
			} // if
		else if (Effect->CompareType == WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCREASE)
			{
			if (SectionElev > CurElev)
				{
				CurElev = SectionElev;
				CurRough = CurSegRough;
				CurEco = CurSegEco;
				return (1);
				} // else if
			} // else if increase only
		else
			{
			if (SectionElev < CurElev)
				{
				CurElev = SectionElev;
				CurRough = CurSegRough;
				CurEco = CurSegEco;
				return (1);
				} // else if
			} // else decrease only
		} // if segment priority higher
	else if (CurSegPriority == HighestSegPriority)
		{
		// testing - use total distance instead of distance from start of segment
		//CurSegPercentDistance = CurSegWidth > 0.0 ? 1.0 - DistFromPrevNode / CurSegWidth: 1.0;
		CurSegPercentDistance = Effect->MaxSectionDist > 0.0 ? 1.0 - Distance / Effect->MaxSectionDist: 1.0;
		// end test
		if (Effect->CompareType == WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCDEC)
			{
			if (CurSegPercentDistance > 0.0 || LastSegPercentDistance > 0.0)
				CurElev = (SectionElev * CurSegPercentDistance + CurElev * LastSegPercentDistance) / (CurSegPercentDistance + LastSegPercentDistance);
			else
				CurElev = (SectionElev + CurElev) * 0.5;
			CurRough = CurSegRough;
			CurEco = CurSegEco;
			LastSegPercentDistance = CurSegPercentDistance;
			return (1);
			} // if
		else if (Effect->CompareType == WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCREASE)
			{
			if (SectionElev > CurElev)
				{
				if (CurSegPercentDistance > 0.0 || LastSegPercentDistance > 0.0)
					CurElev = (SectionElev * CurSegPercentDistance + CurElev * LastSegPercentDistance) / (CurSegPercentDistance + LastSegPercentDistance);
				else
					CurElev = (SectionElev + CurElev) * 0.5;
				CurRough = CurSegRough;
				CurEco = CurSegEco;
				LastSegPercentDistance = CurSegPercentDistance;
				return (1);
				} // else if
			} // else if increase only
		else
			{
			if (SectionElev < CurElev)
				{
				if (CurSegPercentDistance > 0.0 || LastSegPercentDistance > 0.0)
					CurElev = (SectionElev * CurSegPercentDistance + CurElev * LastSegPercentDistance) / (CurSegPercentDistance + LastSegPercentDistance);
				else
					CurElev = (SectionElev + CurElev) * 0.5;
				CurRough = CurSegRough;
				CurEco = CurSegEco;
				LastSegPercentDistance = CurSegPercentDistance;
				return (1);
				} // else if
			} // else decrease only
		} // else if segment priority same
	} // else within keyed distance range

// return HighestSegPriority to former value so unused segment data does not contaminate next Comparison
LastSegPercentDistance = LastSegPercentDistanceStash;
HighestSegPriority = HighestSegPriorityStash;

return (0);

} // TerraffectorEffectBase::Compare

/*===========================================================================*/

// VNS 3 version
bool TerraffectorEffectBase::Eval(RenderData *Rend, EffectJoeList *MyEffects, VectorNode *CurNode, bool EcoOnly, 
	double &SumOfAllCoverages)
{
double DistanceToVector, TestDistance, JoeElev, CurJoeElev, JoeSlope, CurJoeSlope, TestJoeElev, TestJoeSlope,
	AccumulatedWt = 0.0;
float RestoreSlope, VeryTempOffsets[3], TestTempOffsets[3], CurOffsets[3];
long EffectCt, CurCt;
unsigned long LonSeed, LatSeed;
int OffEnd = 0, CurSplined = 0;
EffectJoeList *CurList;
TerraffectorEffect *CurEffect;
GeoRaster *Rast = GeoRast;
Joe *CurVec;
TfxDetail *FoundDetail, *CurVS;
MaterialList *NewMat, *FirstMaterial = NULL;
short TopPriority = -10000, LowestSegNumber;
bool Done = false, Success = true, Seeded;
TwinMaterials MatTwin;

if (! RefMat)
	{
	if (RefMat = new MaterialList())
		{
		if (! RefMat->AddVectorProperties())
			{
			delete RefMat;
			RefMat = NULL;
			return (0);
			} // if
		} // if
	else
		return (0);
	} // if

// test this to see if there are any area terraffectors
EffectsLib::EffectChain[0] = NULL;

CurEco = NULL;
CurVec = NULL;
CurRough = LastSegPercentDistance = 0.0;
CurSlope = -1.0;
HighestSegPriority = -1000;
OrigElev = CurElev = CurNode->Elev - CurNode->NodeData->TexDisplacement - CurNode->NodeData->TfxDisplacement;
LonSeed = (ULONG)((CurNode->Lon - WCS_floor(CurNode->Lon)) * ULONG_MAX);
LatSeed = (ULONG)((CurNode->Lat - WCS_floor(CurNode->Lat)) * ULONG_MAX);
//Rand.Seed64BitShift(LatSeed + WCS_SEEDOFFSET_TERRAFFECTOR, LonSeed + WCS_SEEDOFFSET_TERRAFFECTOR);

// build comprehensive list of the effects to evaluate
for (EffectCt = 0, CurList = MyEffects; CurList; CurList = CurList->Next)
	{
	if (CurList->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_TERRAFFECTOR)
		EffectsLib::EffectChain[EffectCt++] = CurList;
	} // for

// do not consider ecosystems for these, only elevation
//for (VectorNodeLink *SearchNodeLink = CurNode->LinkedNodes; SearchNodeLink; SearchNodeLink = (VectorNodeLink *)SearchNodeLink->NextNodeList)
//	{
//	for (CurList = SearchNodeLink->LinkedPolygon->MyEffects; CurList; CurList = CurList->Next)
//		{
//		if (CurList->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_TERRAFFECTOR)
//			EffectsLib::EffectChain[EffectCt++] = CurList;
//		} // for
//	} // for

if (EffectsLib::EffectChain[0])
	{
	EffectsLib::SortEffectChain(EffectCt, WCS_EFFECTSSUBCLASS_TERRAFFECTOR);
	
	for (CurCt = 0; CurCt < EffectCt; ++CurCt)
		{
		if (CurList = EffectsLib::EffectChain[CurCt])
			{
			CurEffect = (TerraffectorEffect *)CurList->MyEffect;
			if (CurEffect->Priority < TopPriority)
				break;
			if (CurList->VSData)
				{
				DistanceToVector = CurEffect->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_RADIUS].CurValue + 1.0;
				
				// find out the lowest section segment number, 
				LowestSegNumber = -1;
				for (CurVS = CurList->VSData; CurVS; CurVS = CurVS->Next)
					{
					if (CurVS->XSectSegment >= 0 && (LowestSegNumber < 0 || CurVS->XSectSegment < LowestSegNumber))
						LowestSegNumber = CurVS->XSectSegment;
					} // for
				FoundDetail = NULL;
				Seeded = false;
				for (CurVS = CurList->VSData; CurVS; CurVS = CurVS->Next)
					{
					if (CurVS->XSectSegment == LowestSegNumber)
						{
						// find the instance where the node is closest to the referenced segment
						if ((TestDistance = CurList->MyJoe->MinDistToPoint(CurNode, CurVS, Rend->EarthLatScaleMeters, Rend->Exageration, 
							Rend->ElevDatum, TRUE, FALSE, TestJoeElev, TestTempOffsets, &TestJoeSlope)) <= DistanceToVector)
							{
							DistanceToVector = TestDistance;
							FoundDetail = CurVS;
							JoeElev = TestJoeElev;
							JoeSlope = TestJoeSlope;
							if (fabs(TestTempOffsets[0]) < .001f)
								TestTempOffsets[0] = 0.0f;
							VeryTempOffsets[0] = TestTempOffsets[0];
							VeryTempOffsets[1] = TestTempOffsets[1];
							// elevation coord not stored since it may be modified by tfx
							} // if
						} // if
					} // for
				if (FoundDetail)
					{
					// Since we can't be sure that every time the node is processed we will encounter the same effect and segment chain
					// it seems best to init the random seed every time it may be needed
					if (! Seeded)
						{
						Rand.Seed64BitShift(LatSeed + WCS_SEEDOFFSET_TERRAFFECTOR, LonSeed + WCS_SEEDOFFSET_TERRAFFECTOR);
						Seeded = true;
						} // if
					if (Compare(Rend, CurNode, CurEffect, CurList->MyJoe, FoundDetail->GetSegment(), JoeElev, JoeSlope, DistanceToVector, VeryTempOffsets))
						{
						CurVec = CurList->MyJoe;
						CurOffsets[0] = VeryTempOffsets[0];
						CurOffsets[1] = VeryTempOffsets[1];
						// elevation coord not stored since it may be modified by tfx
						CurJoeElev = JoeElev;
						CurJoeSlope = JoeSlope;
						CurSplined = CurEffect->Splined;
						TopPriority = CurEffect->Priority;
						Done = true;
						} // if
					} // if
				} // if
			} // if
		} // for

	if (Done)
		{
		if (! EcoOnly)
			{
			CurNode->NodeData->TfxDisplacement = (float)(CurElev - OrigElev);
			CurNode->NodeData->TexDisplacement *= (float)CurRough;
			CurNode->Elev = OrigElev + CurNode->NodeData->TfxDisplacement + CurNode->NodeData->TexDisplacement;
			} // if
		if (CurEco)
			{
			RefMat->MyVec = CurVec;
			RefMat->VectorProp->VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE | WCS_TEXTURE_VECTOREFFECTTYPE_SPLINEELEV;
			RefMat->VectorProp->VectorType |= (CurSplined ? WCS_TEXTURE_VECTOREFFECTTYPE_SPLINELATLON: 0);
			RefMat->VectorProp->VectorSlope = (float)CurJoeSlope;
			RefMat->VectorProp->VecOffsets[0] = CurOffsets[0];
			RefMat->VectorProp->VecOffsets[1] = CurOffsets[1];
			RefMat->VectorProp->VecOffsets[2] = (float)(CurNode->Elev - CurJoeElev);
			RestoreSlope = -1.0;
			if (CurSlope >= 0.0)
				{
				RestoreSlope = CurNode->NodeData->Slope;
				CurNode->NodeData->Slope = (float)CurSlope;
				} // if
			if (CurEco->EcoMat.GetRenderMaterial(&MatTwin, CurEco->GetRenderMatGradientPos(Rend, CurVec, CurNode, RefMat)))
				{
				if (MatTwin.Mat[0])
					{
					AccumulatedWt += MatTwin.Covg[0] * (1.0 - MatTwin.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
					if (NewMat = CurNode->NodeData->AddVectorMaterial(MatTwin.Mat[0], CurEco, CurVec, (float)MatTwin.Covg[0]))
						{
						if (! FirstMaterial)
							FirstMaterial = NewMat;
						NewMat->MyVec = RefMat->MyVec;
						NewMat->VectorProp->VectorType = RefMat->VectorProp->VectorType;
						NewMat->VectorProp->VectorSlope = RefMat->VectorProp->VectorSlope;
						NewMat->VectorProp->VecOffsets[0] = RefMat->VectorProp->VecOffsets[0];
						NewMat->VectorProp->VecOffsets[1] = RefMat->VectorProp->VecOffsets[1];
						NewMat->VectorProp->VecOffsets[2] = RefMat->VectorProp->VecOffsets[2];
						} // if
					else
						Success = false;
					} // if
				if (MatTwin.Mat[1] && Success)
					{
					AccumulatedWt += MatTwin.Covg[1] * (1.0 - MatTwin.Mat[1]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
					if (NewMat = CurNode->NodeData->AddVectorMaterial(MatTwin.Mat[1], CurEco, CurVec, (float)MatTwin.Covg[1]))
						{
						NewMat->MyVec = RefMat->MyVec;
						NewMat->VectorProp->VectorType = RefMat->VectorProp->VectorType;
						NewMat->VectorProp->VectorSlope = RefMat->VectorProp->VectorSlope;
						NewMat->VectorProp->VecOffsets[0] = RefMat->VectorProp->VecOffsets[0];
						NewMat->VectorProp->VecOffsets[1] = RefMat->VectorProp->VecOffsets[1];
						NewMat->VectorProp->VecOffsets[2] = RefMat->VectorProp->VecOffsets[2];
						} // if
					else
						Success = false;
					} // if
				} // if
			if (CurSlope >= 0.0)
				CurNode->NodeData->Slope = RestoreSlope;
			} // if
		} // if
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

} // TerraffectorEffectBase::Eval

/*===========================================================================*/

// VNS 3 version
bool TerraffectorEffectBase::Compare(RenderData *Rend, VectorNode *CurNode, TerraffectorEffect *Effect, Joe *JoeVec, 
	short SegNumber, double JoeElev, double JoeSlope, double Distance, float *VecOffsets)
{
double DistFromPrevNode, CurSegWidth, MixingMaxElev, MixingMinElev, CurSlopeMaxElev, CurSlopeMinElev, EcoMixing, EcoDistance,
	CurSegRough, SectionElev, CurSegPercentDistance, LastSegPercentDistanceStash, TexFactor, TexOpacity, Value[3];
EcosystemEffect *CurSegEco;
short CurSegPriority, HighestSegPriorityStash;
bool Success = false;

LastSegPercentDistanceStash = LastSegPercentDistance;
HighestSegPriorityStash = HighestSegPriority;

SectionElev = Effect->ADSection.GetValue(0, Distance);

if (Effect->Absolute == WCS_EFFECT_RELATIVETOJOE)
	SectionElev += JoeElev;
else if (Effect->Absolute == WCS_EFFECT_RELATIVETOGROUND)
	{
	if (Effect->ApplyToOrigElev)
		SectionElev += OrigElev;
	else
		SectionElev += CurElev;
	} // else if
// else its absolute elevation and there's no change

// evaluate texture
TexFactor = Effect->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_INTENSITY].CurValue;
if (Effect->GetEnabledTexture(WCS_EFFECTS_TERRAFFECTOR_TEXTURE_INTENSITY))
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	RefMat->MyVec = JoeVec;
	RefMat->VectorProp->VectorSlope = (float)fabs(JoeSlope * 100.0);
	RefMat->VectorProp->VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE | WCS_TEXTURE_VECTOREFFECTTYPE_SPLINEELEV;
	RefMat->VectorProp->VectorType |= (Effect->Splined ? WCS_TEXTURE_VECTOREFFECTTYPE_SPLINELATLON: 0);
	if (VecOffsets)
		{
		RefMat->VectorProp->VecOffsets[0] = VecOffsets[0];
		RefMat->VectorProp->VecOffsets[1] = VecOffsets[1];
		RefMat->VectorProp->VecOffsets[2] = VecOffsets[2];
		} // if
	//Vert->TexDataInitialized = 0;	// since we've changed some vertex values
	if ((TexOpacity = Effect->TexRoot[WCS_EFFECTS_TERRAFFECTOR_TEXTURE_INTENSITY]->Eval(Value, Rend->TransferTextureData(CurNode, JoeVec, RefMat))) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			// Value[0] has already been diminished by the texture's opacity
			TexFactor *= (1.0 - TexOpacity + Value[0]);
			} // if
		else
			TexFactor *= Value[0];
		} // if
	// restore values
	Rend->TexData1.VectorOffsetsComputed = 0;
	if (VecOffsets)
		{
		Rend->TexData1.VDataVecOffsetsValid = 0;
		} // if
	} // if
SectionElev = CurElev + (SectionElev - CurElev) * TexFactor;

if (SegNumber == WCS_TFXDETAIL_SLOPESEGMENT)
	{
	CurSegPriority = Effect->SlopePriority;
	if (CurSegPriority >= HighestSegPriority)
		{
		DistFromPrevNode = Distance - Effect->MaxSectionDist;
		if (DistFromPrevNode < 0.0)
			DistFromPrevNode = 0.0;
		CurSegWidth = Effect->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_RADIUS].CurValue - Effect->MaxSectionDist;
		// find range of elevations that will not be affected by terraffector
		MixingMaxElev = DistFromPrevNode * Effect->MaxSlope - WCS_TERRAFFECTOR_SLOPEELEV_TOLERANCE;	// cut
		MixingMinElev = DistFromPrevNode * Effect->MinSlope + WCS_TERRAFFECTOR_SLOPEELEV_TOLERANCE;	// fill
		CurSlopeMaxElev = SectionElev + MixingMaxElev;
		CurSlopeMinElev = SectionElev + MixingMinElev;
		CurSegEco = Effect->SlopeEco && Effect->SlopeEco->Enabled ? Effect->SlopeEco: NULL;
		CurSegRough = Effect->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_SLOPEROUGHNESS].CurValue;
		EcoMixing = Rand.GenGauss();
		if (EcoMixing < 0.0)
			EcoMixing = 0.0;
		EcoMixing = (Effect->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_SLOPEECOMIXING].CurValue) * EcoMixing;
		MixingMaxElev += (MixingMaxElev * EcoMixing + SectionElev);
		MixingMinElev += (MixingMinElev * EcoMixing + SectionElev);
		MixingMaxElev += SectionElev;
		MixingMinElev += SectionElev;
		if (CurSegPriority > HighestSegPriority)
			{
			HighestSegPriority = CurSegPriority;
			LastSegPercentDistance = CurSegWidth > 0.0 ? 1.0 - DistFromPrevNode / CurSegWidth: 1.0;
			if (Effect->CompareType == WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCDEC)
				{
				if (CurSlopeMaxElev < CurElev)	// cut
					{
					if (CurElev > MixingMaxElev || CurElev < MixingMinElev)
						CurEco = CurSegEco;
					CurElev = CurSlopeMaxElev;
					CurRough = CurSegRough;
					CurSlope = Effect->MaxSlopeDegrees;
					return (true);
					} // if
				else if (CurSlopeMinElev > CurElev)	// fill
					{
					if (CurElev > MixingMaxElev || CurElev < MixingMinElev)
						CurEco = CurSegEco;
					CurElev = CurSlopeMinElev;
					CurRough = CurSegRough;
					CurSlope = Effect->MinSlopeDegrees;
					return (true);
					} // else if 
				} // if
			else if (Effect->CompareType == WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCREASE)
				{
				if (CurSlopeMinElev > CurElev)	// fill
					{
					if (CurElev > MixingMaxElev || CurElev < MixingMinElev)
						CurEco = CurSegEco;
					CurElev = CurSlopeMinElev;
					CurRough = CurSegRough;
					CurSlope = Effect->MinSlopeDegrees;
					return (true);
					} // else if 
				} // else if increase only
			else
				{
				if (CurSlopeMaxElev < CurElev)	// cut
					{
					if (CurElev > MixingMaxElev || CurElev < MixingMinElev)
						CurEco = CurSegEco;
					CurElev = CurSlopeMaxElev;
					CurRough = CurSegRough;
					CurSlope = Effect->MaxSlopeDegrees;
					return (true);
					} // if
				} // else decrease only
			} // if segment priority higher
		else if (CurSegPriority == HighestSegPriority)
			{
			CurSegPercentDistance = CurSegWidth > 0.0 ? 1.0 - DistFromPrevNode / CurSegWidth: 1.0;
			if (Effect->CompareType == WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCDEC)
				{
				if (CurSlopeMaxElev < CurElev)	// cut
					{
					if (CurElev >= MixingMaxElev || CurElev <= MixingMinElev)
						CurEco = CurSegEco;
					if (CurSegPercentDistance > 0.0 || LastSegPercentDistance > 0.0)
						CurElev = (CurSlopeMaxElev * CurSegPercentDistance + CurElev * LastSegPercentDistance) / (CurSegPercentDistance + LastSegPercentDistance);
					else
						CurElev = (CurSlopeMaxElev + CurElev) * 0.5;
					CurRough = CurSegRough;
					CurSlope = Effect->MaxSlopeDegrees;
					LastSegPercentDistance = CurSegPercentDistance;
					return (true);
					} // if
				if (CurSlopeMinElev > CurElev)	// fill
					{
					if (CurElev >= MixingMaxElev || CurElev <= MixingMinElev)
						CurEco = CurSegEco;
					if (CurSegPercentDistance > 0.0 || LastSegPercentDistance > 0.0)
						CurElev = (CurSlopeMinElev * CurSegPercentDistance + CurElev * LastSegPercentDistance) / (CurSegPercentDistance + LastSegPercentDistance);
					else
						CurElev = (CurSlopeMinElev + CurElev) * 0.5;
					CurRough = CurSegRough;
					CurSlope = Effect->MinSlopeDegrees;
					LastSegPercentDistance = CurSegPercentDistance;
					return (true);
					} // if
				} // if
			else if (Effect->CompareType == WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCREASE)
				{
				if (CurSlopeMinElev > CurElev)	// fill
					{
					if (CurElev > MixingMaxElev || CurElev < MixingMinElev)
						CurEco = CurSegEco;
					if (CurSegPercentDistance > 0.0 || LastSegPercentDistance > 0.0)
						CurElev = (CurSlopeMinElev * CurSegPercentDistance + CurElev * LastSegPercentDistance) / (CurSegPercentDistance + LastSegPercentDistance);
					else
						CurElev = (CurSlopeMinElev + CurElev) * 0.5;
					CurRough = CurSegRough;
					CurSlope = Effect->MinSlopeDegrees;
					LastSegPercentDistance = CurSegPercentDistance;
					return (true);
					} // else if
				} // else if increase only
			else
				{
				if (CurSlopeMaxElev < CurElev)	// cut
					{
					if (CurElev > MixingMaxElev || CurElev < MixingMinElev)
						CurEco = CurSegEco;
					if (CurSegPercentDistance > 0.0 || LastSegPercentDistance > 0.0)
						CurElev = (CurSlopeMaxElev * CurSegPercentDistance + CurElev * LastSegPercentDistance) / (CurSegPercentDistance + LastSegPercentDistance);
					else
						CurElev = (CurSlopeMaxElev + CurElev) * 0.5;
					CurRough = CurSegRough;
					CurSlope = Effect->MaxSlopeDegrees;
					LastSegPercentDistance = CurSegPercentDistance;
					return (true);
					} // else if
				} // else decrease only
			} // else if segment priority same
		} // if segment priority >=
	} // if beyond last key distance
else
	{
	Effect->ADSection.FetchSegmentData(CurSegPriority, CurSegRough, EcoMixing, CurSegEco, DistFromPrevNode, CurSegWidth, SegNumber, Distance);
	// testing - use total distance instead of distance from start of segment
	DistFromPrevNode = Distance;
	// end test
	EcoDistance = Distance + (EcoMixing * .5 * Rand.GenGauss());
	if (EcoMixing > 0.0)
		{
		if (EcoDistance < Effect->MaxSectionDist)
			Effect->ADSection.FetchSegmentEco(CurSegEco, EcoDistance);
		else
			CurSegEco = Effect->SlopeEco;
		} // if
	CurSegEco = CurSegEco && CurSegEco->Enabled ? CurSegEco: NULL;
	if (CurSegPriority > HighestSegPriority)
		{
		HighestSegPriority = CurSegPriority;
		// testing - use total distance instead of distance from start of segment
		//LastSegPercentDistance = CurSegWidth > 0.0 ? 1.0 - DistFromPrevNode / CurSegWidth: 1.0;
		LastSegPercentDistance = Effect->MaxSectionDist > 0.0 ? 1.0 - Distance / Effect->MaxSectionDist: 1.0;
		// end test
		if (Effect->CompareType == WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCDEC)
			{
			CurElev = SectionElev;
			CurRough = CurSegRough;
			CurEco = CurSegEco;
			CurSlope = -1.0;
			return (true);
			} // if
		else if (Effect->CompareType == WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCREASE)
			{
			if (SectionElev > CurElev)
				{
				CurElev = SectionElev;
				CurRough = CurSegRough;
				CurEco = CurSegEco;
				CurSlope = -1.0;
				return (true);
				} // else if
			} // else if increase only
		else
			{
			if (SectionElev < CurElev)
				{
				CurElev = SectionElev;
				CurRough = CurSegRough;
				CurEco = CurSegEco;
				CurSlope = -1.0;
				return (true);
				} // else if
			} // else decrease only
		} // if segment priority higher
	else if (CurSegPriority == HighestSegPriority)
		{
		// testing - use total distance instead of distance from start of segment
		//CurSegPercentDistance = CurSegWidth > 0.0 ? 1.0 - DistFromPrevNode / CurSegWidth: 1.0;
		CurSegPercentDistance = Effect->MaxSectionDist > 0.0 ? 1.0 - Distance / Effect->MaxSectionDist: 1.0;
		// end test
		if (Effect->CompareType == WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCDEC)
			{
			if (CurSegPercentDistance > 0.0 || LastSegPercentDistance > 0.0)
				CurElev = (SectionElev * CurSegPercentDistance + CurElev * LastSegPercentDistance) / (CurSegPercentDistance + LastSegPercentDistance);
			else
				CurElev = (SectionElev + CurElev) * 0.5;
			CurRough = CurSegRough;
			CurEco = CurSegEco;
			CurSlope = -1.0;
			LastSegPercentDistance = CurSegPercentDistance;
			return (true);
			} // if
		else if (Effect->CompareType == WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCREASE)
			{
			if (SectionElev > CurElev)
				{
				if (CurSegPercentDistance > 0.0 || LastSegPercentDistance > 0.0)
					CurElev = (SectionElev * CurSegPercentDistance + CurElev * LastSegPercentDistance) / (CurSegPercentDistance + LastSegPercentDistance);
				else
					CurElev = (SectionElev + CurElev) * 0.5;
				CurRough = CurSegRough;
				CurEco = CurSegEco;
				CurSlope = -1.0;
				LastSegPercentDistance = CurSegPercentDistance;
				return (true);
				} // else if
			} // else if increase only
		else
			{
			if (SectionElev < CurElev)
				{
				if (CurSegPercentDistance > 0.0 || LastSegPercentDistance > 0.0)
					CurElev = (SectionElev * CurSegPercentDistance + CurElev * LastSegPercentDistance) / (CurSegPercentDistance + LastSegPercentDistance);
				else
					CurElev = (SectionElev + CurElev) * 0.5;
				CurRough = CurSegRough;
				CurEco = CurSegEco;
				CurSlope = -1.0;
				LastSegPercentDistance = CurSegPercentDistance;
				return (true);
				} // else if
			} // else decrease only
		} // else if segment priority same
	} // else within keyed distance range

// return HighestSegPriority to former value so unused segment data does not contaminate next Comparison
LastSegPercentDistance = LastSegPercentDistanceStash;
HighestSegPriority = HighestSegPriorityStash;

return (false);

} // TerraffectorEffectBase::Compare

/*===========================================================================*/
/*===========================================================================*/

void EffectsLib::EvalTerraffectorsNoInit(RenderData *Rend, VertexData *Vert)
{
short Done = 0, Priority = -1000, CurSplined = 0;
double JoeElev, JoeSlope, Distance, HighLat, LowLat, TestLat, LatAdd, LonAdd;
TerraffectorEffect *MyEffect;
RenderJoeList *JL = NULL, *CurJL;
Joe *CurVec;

// build a list of all the effects to be considered
if (Rend->DBase && (CurJL = (JL = Rend->DBase->CreateRenderJoeList(this, WCS_EFFECTSSUBCLASS_TERRAFFECTOR))))
	{
	// sort JoeList according to whatever is the correct priority order for this effect
	JL->SortPriority(WCS_JOELIST_SORTORDER_HILO);
	JL->SortEvalOrder(WCS_JOELIST_SORTORDER_LOHI);
	TerraffectorBase.CurEco = NULL;
	CurVec = NULL;
	TerraffectorBase.CurRough = TerraffectorBase.LastSegPercentDistance = 0.0;
	TerraffectorBase.HighestSegPriority = -1000;
	TerraffectorBase.OrigElev = TerraffectorBase.CurElev = Vert->Elev - Vert->Displacement;
	while (CurJL)
		{
		if (CurJL->Me->TestFlags(WCS_JOEFLAG_HASKIDS) || ! CurJL->Me->TestFlags(WCS_JOEFLAG_ACTIVATED))
			{
			continue;
			} // if 
		if (MyEffect = (TerraffectorEffect *)CurJL->Effect)
			{
			if (MyEffect->Priority >= Priority)
				{
				MyEffect->InitFrameToRender(this, Rend);
				// bounds overlap test - ignore effects where point falls outside effect radius + joe bounds
				HighLat = fabs(CurJL->Me->GetNorth());
				LowLat = fabs(CurJL->Me->GetSouth());
				TestLat = max(HighLat, LowLat);
				LatAdd = MyEffect->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_RADIUS].CurValue / Rend->EarthLatScaleMeters;
				LonAdd = TestLat < 90.0 ? LatAdd / cos(TestLat * PiOver180): 180.0;
				LonAdd = min(LonAdd, 180.0);
				if (Vert->Lat <= CurJL->Me->GetNorth() + LatAdd &&
					Vert->Lat >= CurJL->Me->GetSouth() - LatAdd &&
					Vert->Lon <= CurJL->Me->GetWest() + LonAdd &&
					Vert->Lon >= CurJL->Me->GetEast() - LonAdd)
					{
					if ((Distance = CurJL->Me->MinDistToPoint(Vert->Lat, Vert->Lon, Rend->EarthLatScaleMeters, &JoeElev, &JoeSlope)) <= MyEffect->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_RADIUS].CurValue)
						{
						JoeElev = Rend->ElevDatum + (JoeElev - Rend->ElevDatum) * Rend->Exageration;
						Priority = MyEffect->Priority;
						if (TerraffectorBase.Compare(Rend, Vert, MyEffect, CurJL->Me, JoeElev, JoeSlope, Distance, NULL))
							{
							CurVec = CurJL->Me;
							CurSplined = MyEffect->Splined;
							Done = 1;
							} // if
						if (! TerraffectorBase.OverlapOK)
							break;
						} // if within effect radius
					} // if
				} // if
			else
				break;
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
	if (Done)
		{
		Vert->Elev = TerraffectorBase.CurElev;
		Vert->Elev += (Vert->Displacement * TerraffectorBase.CurRough);
		Vert->Displacement *= TerraffectorBase.CurRough;
		if (TerraffectorBase.CurEco)
			{
			Vert->Eco = TerraffectorBase.CurEco;
			Vert->Vector = CurVec;
			Vert->VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE | WCS_TEXTURE_VECTOREFFECTTYPE_SPLINEELEV;
			Vert->VectorType |= (CurSplined ? WCS_TEXTURE_VECTOREFFECTTYPE_SPLINELATLON: 0);
			} // if
		} // if
	} // if

} // EffectsLib::EvalTerraffectorsNoInit

/*===========================================================================*/
/*===========================================================================*/

TerraffectorEffect::TerraffectorEffect()
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR;
SetDefaults();

} // TerraffectorEffect::TerraffectorEffect

/*===========================================================================*/

TerraffectorEffect::TerraffectorEffect(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR;
SetDefaults();

} // TerraffectorEffect::TerraffectorEffect

/*===========================================================================*/

TerraffectorEffect::TerraffectorEffect(RasterAnimHost *RAHost, EffectsLib *Library, TerraffectorEffect *Proto)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR;
Prev = Library->LastTerraffector;
if (Library->LastTerraffector)
	{
	Library->LastTerraffector->Next = this;
	Library->LastTerraffector = this;
	} // if
else
	{
	Library->Terraffector = Library->LastTerraffector = this;
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
	strcpy(NameBase, "Terraffector");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // TerraffectorEffect::TerraffectorEffect

/*===========================================================================*/

TerraffectorEffect::~TerraffectorEffect()
{
long Ct;
RootTexture *DelTex;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->TAG && GlobalApp->GUIWins->TAG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->TAG;
		GlobalApp->GUIWins->TAG = NULL;
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

} // TerraffectorEffect::~TerraffectorEffect

/*===========================================================================*/

void TerraffectorEffect::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_TERRAFFECTOR_NUMANIMPAR] = {10.0, -10.0, 0.0, 0.0, 200.0, 1.0};
double RangeDefaults[WCS_EFFECTS_TERRAFFECTOR_NUMANIMPAR][3] = {FLT_MAX, 0.0, .01,		// max approach slope
																0.0, -FLT_MAX, .01,		// min approach slope
																5.0, 0.0, .01,			// slope roughness
																1.0, 0.0, .01,			// slope eco mixing
																FLT_MAX, 0.0, 1.0,		// radius
																1.0, 0.0, .01};			// intensity
long Ct;

for (Ct = 0; Ct < WCS_EFFECTS_TERRAFFECTOR_NUMANIMPAR; Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	TexRoot[Ct] = NULL;
	} // for
SlopePriority      = 0;
SlopeEco           = NULL;
InterCompareMethod = 0;
MaxSectionDist = MinSlope = MaxSlope = 0.0;
MinSlopeDegrees = MaxSlopeDegrees = -1.0;
EvalOrder = 1;
CompareType = WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCDEC;
ApplyToOrigElev = 0;
Splined = 0;
ADSection.SetDefaults(this, (char)GetNumAnimParams());

AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_RADIUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_MAXSLOPE].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_MINSLOPE].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_SLOPEROUGHNESS].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_SLOPEECOMIXING].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_INTENSITY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_RADIUS].SetNoNodes(1);

} // TerraffectorEffect::SetDefaults

/*===========================================================================*/

void TerraffectorEffect::Copy(TerraffectorEffect *CopyTo, TerraffectorEffect *CopyFrom)
{
long Result = -1;
NotifyTag Changes[2];

CopyTo->SlopeEco = NULL;
if (CopyFrom->SlopeEco)
	{
	if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
		{
		CopyTo->SlopeEco = (EcosystemEffect *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->SlopeEco);
		} // if no need to make another copy, its all in the family
	else
		{
		if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->SlopeEco->EffectType, CopyFrom->SlopeEco->Name))
			{
			Result = UserMessageCustom("Copy Terraffector", "How do you wish to resolve Slope Ecosystem name collisions?\n\nLink to existing Ecosystems, replace existing Ecosystems, or create new Ecosystems?",
				"Link", "Create", "Overwrite", 1);
			} // if
		if (Result <= 0)
			{
			CopyTo->SlopeEco = (EcosystemEffect *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->SlopeEco->EffectType, NULL, CopyFrom->SlopeEco);
			} // if create new
		else if (Result == 1)
			{
			CopyTo->SlopeEco = (EcosystemEffect *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->SlopeEco);
			} // if link to existing
		else if (CopyTo->SlopeEco = (EcosystemEffect *)GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->SlopeEco->EffectType, CopyFrom->SlopeEco->Name))
			{
			CopyTo->SlopeEco->Copy(CopyTo->SlopeEco, CopyFrom->SlopeEco);
			Changes[0] = MAKE_ID(CopyTo->SlopeEco->GetNotifyClass(), CopyTo->SlopeEco->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, CopyTo->SlopeEco);
			} // else if found and overwrite
		else
			{
			CopyTo->SlopeEco = (EcosystemEffect *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->SlopeEco->EffectType, NULL, CopyFrom->SlopeEco);
			} // else
		} // else better copy or overwrite it since its important to get just the right object
	} // if

CopyTo->SlopePriority = CopyFrom->SlopePriority;
CopyTo->InterCompareMethod = CopyFrom->InterCompareMethod;
CopyTo->EvalOrder = CopyFrom->EvalOrder;
CopyTo->CompareType = CopyFrom->CompareType;
CopyTo->ApplyToOrigElev = CopyFrom->ApplyToOrigElev;
CopyTo->Splined = CopyFrom->Splined;
ADSection.Copy(&CopyTo->ADSection, &CopyFrom->ADSection);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // TerraffectorEffect::Copy

/*===========================================================================*/

int TerraffectorEffect::RemoveRAHost(RasterAnimHost *RemoveMe)
{
NotifyTag Changes[2];

if (SlopeEco == (EcosystemEffect *)RemoveMe)
	{
	SlopeEco = NULL;

	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

	return (1);
	} // if

return (GeneralEffect::RemoveRAHost(RemoveMe));

} // TerraffectorEffect::RemoveRAHost

/*===========================================================================*/

int TerraffectorEffect::SetSlopeEco(EcosystemEffect *NewEco)
{
NotifyTag Changes[2];

SlopeEco = NewEco;
Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (1);

} // TerraffectorEffect::SetSlopeEco

/*===========================================================================*/

ULONG TerraffectorEffect::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
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
					case WCS_EFFECTS_ABSOLUTE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Absolute, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_TERRAFFECTOR_RADIUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_RADIUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_TERRAFFECTOR_MAXSLOPE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_MAXSLOPE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_TERRAFFECTOR_MINSLOPE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_MINSLOPE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_TERRAFFECTOR_SLOPEROUGH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_SLOPEROUGHNESS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_TERRAFFECTOR_SLOPEECOMIXING:
						{
						BytesRead = AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_SLOPEECOMIXING].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_TERRAFFECTOR_INTENSITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_INTENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_TERRAFFECTOR_EVALORDER:
						{
						BytesRead = ReadBlock(ffile, (char *)&EvalOrder, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&CompareType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_TERRAFFECTOR_APPLYTOORIGELEV:
						{
						BytesRead = ReadBlock(ffile, (char *)&ApplyToOrigElev, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_TERRAFFECTOR_SPLINED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Splined, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_TERRAFFECTOR_SLOPEPRIOR:
						{
						BytesRead = ReadBlock(ffile, (char *)&SlopePriority, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_TERRAFFECTOR_INTERCOMPARE:
						{
						BytesRead = ReadBlock(ffile, (char *)&InterCompareMethod, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (InterCompareMethod == WCS_COMPARE_TA_USEASIS || 
							InterCompareMethod == WCS_COMPARE_TA_AVERAGE || 
							InterCompareMethod == WCS_COMPARE_TA_SUM)
							CompareType = WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCDEC;
						else if (InterCompareMethod == WCS_COMPARE_TA_MAXIMUM)
							CompareType = WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_INCREASE; 
						else
							CompareType = WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE_DECREASE; 
						break;
						}
					case WCS_EFFECTS_TERRAFFECTOR_TEXTUREROOTNUM:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexRootNumber, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures())
							{
							TexRoot[TexRootNumber] = new RootTexture(this, 0, 0, 0);
							} // if
						break;
						}
					case WCS_EFFECTS_TERRAFFECTOR_TEXTUREROOT:
						{
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures() && TexRoot[TexRootNumber])
							BytesRead = TexRoot[TexRootNumber]->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						TexRootNumber = -1;
						break;
						}
					case WCS_EFFECTS_TERRAFFECTOR_TEXINTENSITY:
						{
						if (TexRoot[WCS_EFFECTS_TERRAFFECTOR_TEXTURE_INTENSITY] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_TERRAFFECTOR_TEXTURE_INTENSITY]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_TERRAFFECTOR_CROSSSECTION:
						{
						BytesRead = ADSection.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_TERRAFFECTOR_SLOPEECONAME:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							SlopeEco = (EcosystemEffect *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_ECOSYSTEM, MatchName);
							} // if
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

} // TerraffectorEffect::Load

/*===========================================================================*/

unsigned long int TerraffectorEffect::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_EFFECTS_TERRAFFECTOR_NUMANIMPAR] = {WCS_EFFECTS_TERRAFFECTOR_MAXSLOPE,
																		WCS_EFFECTS_TERRAFFECTOR_MINSLOPE,
																		WCS_EFFECTS_TERRAFFECTOR_SLOPEROUGH,
																		WCS_EFFECTS_TERRAFFECTOR_SLOPEECOMIXING,
																		WCS_EFFECTS_TERRAFFECTOR_RADIUS,
																		WCS_EFFECTS_TERRAFFECTOR_INTENSITY};
unsigned long int TextureItemTag[WCS_EFFECTS_TERRAFFECTOR_NUMTEXTURES] = {WCS_EFFECTS_TERRAFFECTOR_TEXINTENSITY};

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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_TERRAFFECTOR_EVALORDER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&EvalOrder)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_TERRAFFECTOR_SLOPEPRIOR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&SlopePriority)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_TERRAFFECTOR_COMPARETYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&CompareType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_TERRAFFECTOR_APPLYTOORIGELEV, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ApplyToOrigElev)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_TERRAFFECTOR_SPLINED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Splined)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_EFFECTS_TERRAFFECTOR_CROSSSECTION + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = ADSection.Save(ffile))
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
			} // if cross-section saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

if (SlopeEco)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_TERRAFFECTOR_SLOPEECONAME, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(SlopeEco->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)SlopeEco->GetName())) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

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

} // TerraffectorEffect::Save

/*===========================================================================*/

void TerraffectorEffect::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->TAG)
	{
	delete GlobalApp->GUIWins->TAG;
	}
GlobalApp->GUIWins->TAG = new TerraffectorEditGUI(GlobalApp->AppEffects, GlobalApp->MainProj, GlobalApp->AppDB, this);
if(GlobalApp->GUIWins->TAG)
	{
	GlobalApp->GUIWins->TAG->Open(GlobalApp->MainProj);
	}

} // TerraffectorEffect::Edit

/*===========================================================================*/

short TerraffectorEffect::AnimateShadows(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams() - 1; Ct ++)
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

} // TerraffectorEffect::AnimateShadows

/*===========================================================================*/

int TerraffectorEffect::SetToTime(double Time)
{
long Found = 0;

if (GeneralEffect::SetToTime(Time))
	Found = 1;
if (ADSection.SetToTime(Time))
	Found = 1;

return (Found);

} // TerraffectorEffect::SetToTime

/*===========================================================================*/

char *TerraffectorEffectCritterNames[WCS_EFFECTS_TERRAFFECTOR_NUMANIMPAR] = {"Max Approach Slope (%)", "Min Approach Slope (%)",
											"Approach Slope Roughness (%)", "Approach Slope Eco Mixing (%)", "Effect Radius (m)", "Intensity (%)"};
char *TerraffectorEffectTextureNames[WCS_EFFECTS_TERRAFFECTOR_NUMTEXTURES] = {"Intensity (%)"};

char *TerraffectorEffect::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (TerraffectorEffectCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (TerraffectorEffectTextureNames[Ct]);
		} // if
	} // for
if (Test == &ADSection)
	return ("Cross-section Profile");

return ("");

} // TerraffectorEffect::GetCritterName

/*===========================================================================*/

char *TerraffectorEffect::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as a Terraffector Texture! Remove anyway?");

} // TerraffectorEffect::OKRemoveRaster

/*===========================================================================*/

char *TerraffectorEffect::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? TerraffectorEffectTextureNames[TexNumber]: (char*)"");

} // TerraffectorEffect::GetTextureName

/*===========================================================================*/

void TerraffectorEffect::GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace)
{

ApplyToColor = 0;
ApplyToDisplace = 1;

} // TerraffectorEffect::GetTextureApplication

/*===========================================================================*/

RootTexture *TerraffectorEffect::NewRootTexture(long TexNumber)
{
char ApplyToColor = 0;
char ApplyToDisplace = 1;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // TerraffectorEffect::NewRootTexture

/*===========================================================================*/

long TerraffectorEffect::InitImageIDs(long &ImageID)
{
long NumImages = 0;

NumImages += ADSection.InitImageIDs(ImageID);
if (SlopeEco)
	NumImages += SlopeEco->InitImageIDs(ImageID);
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // TerraffectorEffect::InitImageIDs

/*===========================================================================*/

int TerraffectorEffect::BuildFileComponentsList(EffectList **Ecosystems, EffectList **Material3Ds, 
			EffectList **Object3Ds, EffectList **Waves, EffectList **Queries, EffectList **Themes, EffectList **Coords)
{
EffectList **ListPtr;

if (! ADSection.BuildFileComponentsList(Ecosystems, Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
	return (0);

if (SlopeEco)
	{
	ListPtr = Ecosystems;
	while (*ListPtr)
		{
		if ((*ListPtr)->Me == SlopeEco)
			break;
		ListPtr = &(*ListPtr)->Next;
		} // if
	if (! (*ListPtr))
		{
		if (*ListPtr = new EffectList())
			(*ListPtr)->Me = SlopeEco;
		else
			return (0);
		} // if
	if (! SlopeEco->BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
		return (0);
	} // if

return (1);

} // TerraffectorEffect::BuildFileComponentsList

/*===========================================================================*/

char TerraffectorEffect::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_VECTOR
	#ifdef WCS_BUILD_VNS
	|| DropType == WCS_EFFECTSSUBCLASS_SEARCHQUERY
	#endif // WCS_BUILD_VNS
	|| DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLESECTION)
	return (1);

return (0);

} // TerraffectorEffect::GetRAHostDropOK

/*===========================================================================*/

int TerraffectorEffect::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
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
	if (this != (TerraffectorEffect *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (TerraffectorEffect *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLESECTION)
	{
	Success = ADSection.ProcessRAHostDragDrop(DropSource);
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

} // TerraffectorEffect::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long TerraffectorEffect::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_TERRAFFECTOR | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // TerraffectorEffect::GetRAFlags

/*===========================================================================*/

int TerraffectorEffect::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
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
				case WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_INTENSITY:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_TERRAFFECTOR_TEXTURE_INTENSITY);
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
				case WCS_EFFECTS_TERRAFFECTOR_TEXTURE_INTENSITY:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_INTENSITY);
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

} // TerraffectorEffect::GetAffiliates

/*===========================================================================*/

RasterAnimHost *TerraffectorEffect::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;
JoeList *CurJoe = Joes;

if (! Current)
	return (&ADSection);
if (Current == &ADSection)
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
if (Found && SlopeEco)
	return (SlopeEco);
if (Current == SlopeEco)
	Found = 1;
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

} // TerraffectorEffect::GetRAHostChild

/*===========================================================================*/

int TerraffectorEffect::GetDeletable(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (1);
		} // if
	} // for
if (Test == SlopeEco)
	return (1);

return (0);

} // TerraffectorEffect::GetDeletable

/*===========================================================================*/

int TerraffectorEffect::FindnRemoveEcosystems(EcosystemEffect *RemoveMe)
{

return (ADSection.FindnRemoveEcosystems(RemoveMe));

} // TerraffectorEffect::FindnRemoveEcosystems

/*===========================================================================*/

double TerraffectorEffect::GetMaxProfileDistance(void)
{
double FindMin, FindMax;

if (ADSection.GetMinMaxDist(FindMin, FindMax))
	return (FindMax);

return (0.0);

} // TerraffectorEffect::GetMaxProfileDistance

/*===========================================================================*/

double TerraffectorEffect::GetRadiusWidth(void)
{
double Radius, MaxProfileDist;

MaxProfileDist = GetMaxProfileDistance();
Radius = AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_RADIUS].CurValue;

if (Radius > MaxProfileDist)
	return (Radius - MaxProfileDist);

return (0.0);

} // TerraffectorEffect::GetRadiusWidth

/*===========================================================================*/

unsigned long TerraffectorEffect::GetNumSegments(void)
{
void *PlaceHolder = NULL;
double SegWidth;
unsigned long SegCt = 0;

// count the section segments that are non-zero
while (PlaceHolder = GetNextSegmentWidth(PlaceHolder, SegWidth))
	{
	if (SegWidth > 0.0)
		++SegCt;
	} // while
// add one for radius width
if (GetRadiusWidth() > 0.0)
	++SegCt;

return (SegCt);

} // TerraffectorEffect::GetNumSegments

/*===========================================================================*/

int TerraffectorEffect::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{

// set MaxSectionDist since it is a bit time consuming and don't want to have to do it during rendering
MaxSectionDist = GetMaxProfileDistance();

MaxSlope = AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_MAXSLOPE].CurValue;	// cut
MinSlope = AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_MINSLOPE].CurValue;	// fill
MinSlopeDegrees = atan(fabs(MinSlope)) * PiUnder180;
MaxSlopeDegrees = atan(fabs(MaxSlope)) * PiUnder180;

if (MinSlope > MaxSlope)
	{
	swmem(&MinSlope, &MaxSlope, sizeof (double));
	swmem(&MinSlopeDegrees, &MaxSlopeDegrees, sizeof (double));
	} // if

return (GeneralEffect::InitFrameToRender(Lib, Rend));

} // TerraffectorEffect::InitFrameToRender

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int TerraffectorEffect::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
TerraffectorEffect *CurrentTerra = NULL;
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
					else if (! strnicmp(ReadBuf, "Ecosys", 8))
						{
						if (CurrentEco = new EcosystemEffect(NULL, LoadToEffects, NULL, WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM))
							{
							if ((BytesRead = CurrentEco->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if eco
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
					else if (! strnicmp(ReadBuf, "Terafctr", 8))
						{
						if (CurrentTerra = new TerraffectorEffect(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentTerra->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if Terraffector
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

if (Success == 1 && CurrentTerra)
	{
	if (OldBoundsLoaded && GlobalApp->AppDB->FillDEMBounds(&CurBounds))
		{
		if (LoadToEffects->GetListPtr(WCS_EFFECTSSUBCLASS_ECOSYSTEM) && UserMessageYN("Load Terraffector", "Do you wish the loaded Terraffector's Ecosystem\n elevation lines to be scaled to current DEM elevations?\n This will not affect the rendering of Terraffectors\n but may affect the use of the Ecosystems for other purposes."))
			{
			for (CurrentEco = (EcosystemEffect *)LoadToEffects->GetListPtr(WCS_EFFECTSSUBCLASS_ECOSYSTEM); CurrentEco; CurrentEco = (EcosystemEffect *)CurrentEco->Next)
				{
				CurrentEco->ScaleToDEMBounds(&OldBounds, &CurBounds);
				} // for
			} // if
		if (LoadToEffects->GetListPtr(WCS_EFFECTSSUBCLASS_WAVE) && UserMessageYN("Load Terraffector", "Do you wish the loaded Terraffector's Wave positions\n to be scaled to current DEM bounds?"))
			{
			for (CurrentWave = (WaveEffect *)LoadToEffects->GetListPtr(WCS_EFFECTSSUBCLASS_WAVE); CurrentWave; CurrentWave = (WaveEffect *)CurrentWave->Next)
				{
				CurrentWave->ScaleToDEMBounds(&OldBounds, &CurBounds);
				} // for
			} // if
		} // if
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentTerra);
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

} // TerraffectorEffect::LoadObject

/*===========================================================================*/

int TerraffectorEffect::SaveObject(FILE *ffile, const char *SuppliedFileName)
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

if (GeneralEffect::BuildFileComponentsList(&Queries, &Themes, &Coords)
	&& BuildFileComponentsList(&EcoList, &Material3Ds, &Object3Ds, &Waves, &Queries, &Themes, &Coords))
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
						} // if Ecosystem saved 
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

// Terraffector
strcpy(StrBuf, "Terafctr");
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
			} // if Terraffector saved 
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

} // TerraffectorEffect::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::TerraffectorEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
TerraffectorEffect *Current;
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
						BytesRead = ReadBlock(ffile, (char *)&TerraffectorBase.Resolution, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_OVERLAPOK:
						{
						BytesRead = ReadBlock(ffile, (char *)&TerraffectorBase.OverlapOK, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_FLOATING:
						{
						BytesRead = ReadBlock(ffile, (char *)&TerraffectorBase.Floating, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						FloatingLoaded = 1;
						break;
						}
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new TerraffectorEffect(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (TerraffectorEffect *)FindDuplicateByName(Current->EffectType, Current))
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
	if (fabs(TerraffectorBase.Resolution - 90.0) > .0001)
		TerraffectorBase.Floating = 0;
	} // if older file

return (TotalRead);

} // EffectsLib::TerraffectorEffect_Load()

/*===========================================================================*/

ULONG EffectsLib::TerraffectorEffect_Save(FILE *ffile)
{
TerraffectorEffect *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_RESOLUTION, WCS_BLOCKSIZE_CHAR,
WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
WCS_BLOCKTYPE_FLOAT, (char *)&TerraffectorBase.Resolution)) == NULL)
goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_OVERLAPOK, WCS_BLOCKSIZE_CHAR,
WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
WCS_BLOCKTYPE_SHORTINT, (char *)&TerraffectorBase.OverlapOK)) == NULL)
goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_FLOATING, WCS_BLOCKSIZE_CHAR,
WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
WCS_BLOCKTYPE_SHORTINT, (char *)&TerraffectorBase.Floating)) == NULL)
goto WriteError;
TotalWritten += BytesWritten;

Current = Terraffector;
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
					} // if Terraffector effect saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (TerraffectorEffect *)Current->Next;
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

} // EffectsLib::TerraffectorEffect_Save()

/*===========================================================================*/
