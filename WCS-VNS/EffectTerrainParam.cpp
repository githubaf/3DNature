// EffectTerrainParam.cpp
// For managing Terrain Parameter Effects
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
#include "Render.h"
#include "Log.h"
#include "Toolbar.h"
#include "ProjUpdateGUI.h"
#include "requester.h"
#include "TerrainParamEditGUI.h"
#include "Database.h"
#include "Security.h"
#include "Lists.h"
#include "FeatureConfig.h"

extern int EngineOnly;

TerrainParamEffectBase::TerrainParamEffectBase(void)
{

SetDefaults();

} // TerrainParamEffectBase::TerrainParamEffectBase

/*===========================================================================*/

void TerrainParamEffectBase::SetDefaults(void)
{

Resolution = (float)90.0;
GeoRast = NULL;
EdgesExist = GradientsExist = 0;
OverlapOK = Floating = 1;

HorFractDisplace = FrdInvalid = 1;
BackfaceCull = 0;
RegenFDMsEachRender = 0;
#ifdef WCS_VECPOLY_EFFECTS
DepthMapPixSize = 10.0;
#else // WCS_VECPOLY_EFFECTS
DepthMapPixSize = 1.0;
#endif // WCS_VECPOLY_EFFECTS
FractalMethod = WCS_FRACTALMETHOD_VARIABLE;

} // TerrainParamEffectBase::SetDefaults

/*===========================================================================*/

GeoRaster *TerrainParamEffectBase::Init(GeneralEffect *EffectList, RenderJoeList *&JL, double N, double S, double E, double W, 
	double MetersPerDegLat, double RefLat, double RefLon, long MaxMem, long &UsedMem)
{
double CellSize, NWLat, SELat, NWLon, SELon, CellsNS, CellsWE, TotalCells, SizeMult;
GeoRaster *DrawRast = NULL, *DR, *LastDrawRast, *LastGradRast;
RenderJoeList *RJL;
TerrainParamEffect *MyEffect;
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
			if (MyEffect = (TerrainParamEffect *)RJL->Effect)
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
	UBYTE *Bitmap[3];

	DR = DrawRast;
	DrawRast = GeoRast;
	while (DrawRast)
		{
		Bitmap[0] = Bitmap[1] = Bitmap[2] = DrawRast->ByteMap[0];
		sprintf(Name, "d:\\Frames\\TerrainParamRastTest%1d.iff", Count);
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

} // TerrainParamEffectBase::Init

/*===========================================================================*/

void TerrainParamEffectBase::Destroy(void)
{
GeoRaster *Rast = GeoRast, *NextRast;

while (Rast)
	{
	NextRast = (GeoRaster *)Rast->Next;
	delete (Rast);
	Rast = NextRast;
	} // while
GeoRast = NULL;

} // TerrainParamEffectBase::Destroy

/*===========================================================================*/

short TerrainParamEffectBase::AreThereEdges(GeneralEffect *EffectList)
{

while (EffectList)
	{
	if (EffectList->Enabled && EffectList->HiResEdge)
		return (1);
	EffectList = EffectList->Next;
	} // while

return (0);

} // TerrainParamEffectBase::AreThereEdges

/*===========================================================================*/

short TerrainParamEffectBase::AreThereGradients(GeneralEffect *EffectList)
{

while (EffectList)
	{
//	if (EffectList->Enabled && EffectList->UseGradient && EffectList->Prof)
	if (EffectList->Enabled && EffectList->UseGradient)
		return (1);
	EffectList = EffectList->Next;
	} // while

return (0);

} // TerrainParamEffectBase::AreThereGradients

/*===========================================================================*/

void TerrainParamEffectBase::SetFloating(int NewFloating, Project *CurProj)
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
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_TERRAINPARAM, WCS_EFFECTSSUBCLASS_TERRAINPARAM, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);
#endif // WCS_VECPOLY_EFFECTS
} // TerrainParamEffectBase::SetFloating

/*===========================================================================*/

void TerrainParamEffectBase::SetFloating(int NewFloating, float NewResolution)
{
#ifndef WCS_VECPOLY_EFFECTS
NotifyTag Changes[2];

Changes[1] = 0;

Floating = NewFloating;

if (NewFloating)
	{
	Resolution = NewResolution;
	} // if
Changes[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_TERRAINPARAM, WCS_EFFECTSSUBCLASS_TERRAINPARAM, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
GlobalApp->AppEx->GenerateNotify(Changes, NULL);
#endif // WCS_VECPOLY_EFFECTS
} // TerrainParamEffectBase::SetFloating

/*===========================================================================*/

int TerrainParamEffectBase::CreateFractalMaps(int ReportOptimum)
{
int Success;
Renderer *Rend;

if (Rend = new Renderer())
	{
	Success = Rend->MakeAllFractalDepthMaps(ReportOptimum);
	delete Rend;
	return (Success);
	} // if

return (0);

} // TerrainParamEffectBase::CreateFractalMaps

/*===========================================================================*/
/*===========================================================================*/

TerrainParamEffect::TerrainParamEffect()
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM;
SetDefaults();

} // TerrainParamEffect::TerrainParamEffect

/*===========================================================================*/

TerrainParamEffect::TerrainParamEffect(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM;
SetDefaults();

} // TerrainParamEffect::TerrainParamEffect

/*===========================================================================*/

TerrainParamEffect::TerrainParamEffect(RasterAnimHost *RAHost, EffectsLib *Library, TerrainParamEffect *Proto)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM;
Prev = Library->LastTerrainParam;
if (Library->LastTerrainParam)
	{
	Library->LastTerrainParam->Next = this;
	Library->LastTerrainParam = this;
	} // if
else
	{
	Library->TerrainParam = Library->LastTerrainParam = this;
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
	strcpy(NameBase, "Terrain Parameters");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // TerrainParamEffect::TerrainParamEffect

/*===========================================================================*/

TerrainParamEffect::~TerrainParamEffect()
{
long Ct;
RootTexture *DelTex;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->TPG && GlobalApp->GUIWins->TPG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->TPG;
		GlobalApp->GUIWins->TPG = NULL;
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

} // TerrainParamEffect::~TerrainParamEffect

/*===========================================================================*/

void TerrainParamEffect::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_TERRAINPARAM_NUMANIMPAR] = {0.0, 1.2};
double RangeDefaults[WCS_EFFECTS_TERRAINPARAM_NUMANIMPAR][3] = {
												#ifdef WCS_VECPOLY_EFFECTS
												1000.0, 0.0, .1,	// displacement m
												#else // WCS_VECPOLY_EFFECTS
												1.0, 0.0, .01,	// displacement %
												#endif // WCS_VECPOLY_EFFECTS
												2.0, 1.0, .1	// slope factor
												};
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
FractalDepth = 0;		// range of 0 - 9
PhongShading = true;

#ifdef WCS_VECPOLY_EFFECTS
AnimPar[WCS_EFFECTS_TERRAINPARAM_ANIMPAR_DISPLACEMENT].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
#else // WCS_VECPOLY_EFFECTS
AnimPar[WCS_EFFECTS_TERRAINPARAM_ANIMPAR_DISPLACEMENT].SetMultiplier(100.0);
#endif // WCS_VECPOLY_EFFECTS

} // TerrainParamEffect::SetDefaults

/*===========================================================================*/

void TerrainParamEffect::Copy(TerrainParamEffect *CopyTo, TerrainParamEffect *CopyFrom)
{

CopyTo->FractalDepth = CopyFrom->FractalDepth;
CopyTo->PhongShading = CopyFrom->PhongShading;
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // TerrainParamEffect::Copy

/*===========================================================================*/

ULONG TerrainParamEffect::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char TexRootNumber = -1, WarnAboutV5Displacement = 0, WarnAboutV6Displacement = 0, WarnAboutV6FractalDepth = 0;

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
					case WCS_EFFECTS_TERRAINPARAM_V6FRACTALDEPTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&FractalDepth, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						#ifdef WCS_VECPOLY_EFFECTS
						if (FractalDepth > 0)
							WarnAboutV6FractalDepth = 1;
						#endif // WCS_VECPOLY_EFFECTS
						break;
						}
					case WCS_EFFECTS_TERRAINPARAM_FRACTALDEPTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&FractalDepth, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_TERRAINPARAM_PHONGSHADING:
						{
						BytesRead = ReadBlock(ffile, (char *)&PhongShading, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_TERRAINPARAM_V5DISPLACEMENT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_TERRAINPARAM_ANIMPAR_DISPLACEMENT].Load(ffile, Size, ByteFlip);
						#ifdef WCS_VECPOLY_EFFECTS
						WarnAboutV6Displacement = 1;
						AnimPar[WCS_EFFECTS_TERRAINPARAM_ANIMPAR_DISPLACEMENT].SetValue(0.0);
						#else // WCS_VECPOLY_EFFECTS
						WarnAboutV5Displacement = 1;
						#endif // WCS_VECPOLY_EFFECTS
						break;
						}
					case WCS_EFFECTS_TERRAINPARAM_V6DISPLACEMENT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_TERRAINPARAM_ANIMPAR_DISPLACEMENT].Load(ffile, Size, ByteFlip);
						#ifdef WCS_VECPOLY_EFFECTS
						WarnAboutV6Displacement = 1;
						AnimPar[WCS_EFFECTS_TERRAINPARAM_ANIMPAR_DISPLACEMENT].SetValue(0.0);
						#endif // WCS_VECPOLY_EFFECTS
						break;
						}
					case WCS_EFFECTS_TERRAINPARAM_DISPLACEMENT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_TERRAINPARAM_ANIMPAR_DISPLACEMENT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_TERRAINPARAM_SLOPEFACTOR:
						{
						BytesRead = AnimPar[WCS_EFFECTS_TERRAINPARAM_ANIMPAR_SLOPEFACTOR].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_TERRAINPARAM_TEXTUREROOTNUM:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexRootNumber, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures())
							{
							TexRoot[TexRootNumber] = new RootTexture(this, 0, 0, 0);
							} // if
						break;
						}
					case WCS_EFFECTS_TERRAINPARAM_TEXTUREROOT:
						{
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures() && TexRoot[TexRootNumber])
							BytesRead = TexRoot[TexRootNumber]->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						TexRootNumber = -1;
						break;
						}
					case WCS_EFFECTS_TERRAINPARAM_TEXFRACTALDEPTH:
						{
						if (TexRoot[WCS_EFFECTS_TERRAINPARAM_TEXTURE_FRACTALDEPTH] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_TERRAINPARAM_TEXTURE_FRACTALDEPTH]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_TERRAINPARAM_TEXDISPLACEMENT:
						{
						if (TexRoot[WCS_EFFECTS_TERRAINPARAM_TEXTURE_DISPLACEMENT] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_TERRAINPARAM_TEXTURE_DISPLACEMENT]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_TERRAINPARAM_TEXSLOPEFACTOR:
						{
						if (TexRoot[WCS_EFFECTS_TERRAINPARAM_TEXTURE_SLOPEFACTOR] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_TERRAINPARAM_TEXTURE_SLOPEFACTOR]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
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


if (!EngineOnly && (WarnAboutV5Displacement || WarnAboutV6Displacement || WarnAboutV6FractalDepth))
	{
	GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD, WCS_TOOLBAR_ITEM_PUW, 0);
	if(GlobalApp->GUIWins->PUG)
		{
		GlobalApp->GUIWins->PUG->SetTerrainMessagesVisible(WarnAboutV5Displacement ? true : false, WarnAboutV6Displacement ? true : false, WarnAboutV6FractalDepth ? true : false);
		} // if
	} // if
if (WarnAboutV6FractalDepth)
	{
	if (! GetTexRootPtr(WCS_EFFECTS_TERRAINPARAM_TEXTURE_DISPLACEMENT))
		FractalDepth = 0;
	else
		FractalDepth = FractalDepth / 3;	// FD1-2 becomes FD0, FD3-5 becomes FD1, FD6-7 becomes FD2 
	} // if

return (TotalRead);

} // TerrainParamEffect::Load

/*===========================================================================*/

unsigned long int TerrainParamEffect::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
#ifdef WCS_VECPOLY_EFFECTS
unsigned long int AnimItemTag[WCS_EFFECTS_TERRAINPARAM_NUMANIMPAR] = {WCS_EFFECTS_TERRAINPARAM_DISPLACEMENT,
#else // WCS_VECPOLY_EFFECTS
unsigned long int AnimItemTag[WCS_EFFECTS_TERRAINPARAM_NUMANIMPAR] = {WCS_EFFECTS_TERRAINPARAM_V6DISPLACEMENT,
#endif // WCS_VECPOLY_EFFECTS
																 WCS_EFFECTS_TERRAINPARAM_SLOPEFACTOR};
unsigned long int TextureItemTag[WCS_EFFECTS_TERRAINPARAM_NUMTEXTURES] = {WCS_EFFECTS_TERRAINPARAM_TEXFRACTALDEPTH,
																		WCS_EFFECTS_TERRAINPARAM_TEXDISPLACEMENT,
																		WCS_EFFECTS_TERRAINPARAM_TEXSLOPEFACTOR};

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

#ifdef WCS_VECPOLY_EFFECTS
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_TERRAINPARAM_FRACTALDEPTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FractalDepth)) == NULL)
	goto WriteError;
#else // WCS_VECPOLY_EFFECTS
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_TERRAINPARAM_V6FRACTALDEPTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FractalDepth)) == NULL)
	goto WriteError;
#endif // WCS_VECPOLY_EFFECTS
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_TERRAINPARAM_PHONGSHADING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PhongShading)) == NULL)
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

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // TerrainParamEffect::Save

/*===========================================================================*/

void TerrainParamEffect::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->TPG)
	{
	delete GlobalApp->GUIWins->TPG;
	}
GlobalApp->GUIWins->TPG = new TerrainParamEditGUI(GlobalApp->AppEffects, GlobalApp->AppDB, this);
if(GlobalApp->GUIWins->TPG)
	{
	GlobalApp->GUIWins->TPG->Open(GlobalApp->MainProj);
	}

} // TerrainParamEffect::Edit

/*===========================================================================*/

short TerrainParamEffect::AnimateShadows(void)
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

} // TerrainParamEffect::AnimateShadows

/*===========================================================================*/

char *TerrainParamEffectCritterNames[WCS_EFFECTS_TERRAINPARAM_NUMANIMPAR] = {
																			#ifdef WCS_VECPOLY_EFFECTS
																			"Displacement (m)",
																			#else // WCS_VECPOLY_EFFECTS
																			"Displacement (%)",
																			#endif // WCS_VECPOLY_EFFECTS
																				"Slope Factor (1-2)"};
char *TerrainParamEffectTextureNames[WCS_EFFECTS_TERRAINPARAM_NUMTEXTURES] = {"Fractal Depth (0-7)", 
																				#ifdef WCS_VECPOLY_EFFECTS
																				"Displacement (m)",
																				#else // WCS_VECPOLY_EFFECTS
																				"Displacement (%)",
																				#endif // WCS_VECPOLY_EFFECTS
																				"Slope Factor"};

char *TerrainParamEffect::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (TerrainParamEffectCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (TerrainParamEffectTextureNames[Ct]);
		} // if
	} // for

return ("");

} // TerrainParamEffect::GetCritterName

/*===========================================================================*/

char *TerrainParamEffect::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as an Terrain Parameter Texture! Remove anyway?");

} // TerrainParamEffect::OKRemoveRaster

/*===========================================================================*/

char *TerrainParamEffect::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? TerrainParamEffectTextureNames[TexNumber]: (char*)"");

} // TerrainParamEffect::GetTextureName

/*===========================================================================*/

void TerrainParamEffect::GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace)
{

ApplyToColor = 0;
ApplyToDisplace = 1;

} // TerrainParamEffect::GetTextureApplication

/*===========================================================================*/

RootTexture *TerrainParamEffect::NewRootTexture(long TexNumber)
{
char ApplyToColor = 0;
char ApplyToDisplace = 1;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // TerrainParamEffect::NewRootTexture

/*===========================================================================*/

char TerrainParamEffect::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
/*
if (DropType == WCS_RAHOST_OBJTYPE_VECTOR
	||DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE)
	return (1);
*/
return (0);

} // TerrainParamEffect::GetRAHostDropOK

/*===========================================================================*/

int TerrainParamEffect::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
int Success;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];
char QueryStr[256], NameStr[128];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (TerrainParamEffect *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (TerrainParamEffect *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
/*
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
		if (((Joe *)DropSource->DropSource)->AddEffect(this))
			{
			Success = 1;
			} // if
		} // if
	} // else if
*/
else
	{
	Success = GeneralEffect::ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // TerrainParamEffect::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long TerrainParamEffect::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_TERRAINPAR | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // TerrainParamEffect::GetRAFlags

/*===========================================================================*/

int TerrainParamEffect::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
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
				case WCS_EFFECTS_TERRAINPARAM_ANIMPAR_DISPLACEMENT:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_TERRAINPARAM_TEXTURE_DISPLACEMENT);
					break;
					} // 
				case WCS_EFFECTS_TERRAINPARAM_ANIMPAR_SLOPEFACTOR:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_TERRAINPARAM_TEXTURE_SLOPEFACTOR);
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
				case WCS_EFFECTS_TERRAINPARAM_TEXTURE_DISPLACEMENT:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_TERRAINPARAM_ANIMPAR_DISPLACEMENT);
					break;
					} // 
				case WCS_EFFECTS_TERRAINPARAM_TEXTURE_SLOPEFACTOR:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_TERRAINPARAM_ANIMPAR_SLOPEFACTOR);
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

} // TerrainParamEffect::GetAffiliates

/*===========================================================================*/

RasterAnimHost *TerrainParamEffect::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;
JoeList *CurJoe = Joes;

if (! Current)
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

} // TerrainParamEffect::GetRAHostChild

/*===========================================================================*/

double TerrainParamEffect::GetMaxProfileDistance(void)
{
double FindMin, FindMax;

if (ADProf.GetMinMaxDist(FindMin, FindMax))
	return (FindMax);

return (0.0);

} // TerrainParamEffect::GetMaxProfileDistance

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int TerrainParamEffect::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
DEMBounds OldBounds, CurBounds;
CoordSys *CurrentCoordSys = NULL;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
SearchQuery *CurrentQuery = NULL;
TerrainParamEffect *CurrentTerrain = NULL;
ThematicMap *CurrentTheme = NULL;
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];

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
					else if (! strnicmp(ReadBuf, "TeranPar", 8))
						{
						if (CurrentTerrain = new TerrainParamEffect(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentTerrain->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if TeranPar
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

if (Success == 1 && CurrentTerrain)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentTerrain);
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

} // TerrainParamEffect::LoadObject

/*===========================================================================*/

int TerrainParamEffect::SaveObject(FILE *ffile, const char *SuppliedFileName)
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

// TerrainParamEffect
strcpy(StrBuf, "TeranPar");
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
			} // if TerrainParamEffect saved 
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

} // TerrainParamEffect::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::TerrainParamEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
char TempChar;
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
TerrainParamEffect *Current;
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
						BytesRead = ReadBlock(ffile, (char *)&TerrainParamBase.Resolution, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_OVERLAPOK:
						{
						BytesRead = ReadBlock(ffile, (char *)&TerrainParamBase.OverlapOK, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_FLOATING:
						{
						BytesRead = ReadBlock(ffile, (char *)&TerrainParamBase.Floating, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						FloatingLoaded = 1;
						break;
						}
					case WCS_EFFECTSBASE_TERRAINPARAM_HORFRACTDISPLACE:
						{
						BytesRead = ReadBlock(ffile, (char *)&TerrainParamBase.HorFractDisplace, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_TERRAINPARAM_BACKFACECULL:
						{
						BytesRead = ReadBlock(ffile, (char *)&TerrainParamBase.BackfaceCull, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_TERRAINPARAM_FRDINVALID:
						{
						BytesRead = ReadBlock(ffile, (char *)&TerrainParamBase.FrdInvalid, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_TERRAINPARAM_FRACTALMETHOD:
						{
						BytesRead = ReadBlock(ffile, (char *)&TerrainParamBase.FractalMethod, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_TERRAINPARAM_REGENFDMS:
						{
						BytesRead = ReadBlock(ffile, (char *)&TerrainParamBase.RegenFDMsEachRender, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_TERRAINPARAM_MAPPIXSIZECHAR:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempChar, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						TerrainParamBase.DepthMapPixSize = TempChar;
						break;
						}
					case WCS_EFFECTSBASE_TERRAINPARAM_MAPPIXSIZEV5:
						{
						BytesRead = ReadBlock(ffile, (char *)&TerrainParamBase.DepthMapPixSize, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						TerrainParamBase.DepthMapPixSize *= 10.0;
						break;
						}
					case WCS_EFFECTSBASE_TERRAINPARAM_MAPPIXSIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&TerrainParamBase.DepthMapPixSize, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new TerrainParamEffect(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (TerrainParamEffect *)FindDuplicateByName(Current->EffectType, Current))
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
	if (fabs(TerrainParamBase.Resolution - 90.0) > .0001)
		TerrainParamBase.Floating = 0;
	} // if older file
#endif // WCS_VECPOLY_EFFECTS

return (TotalRead);

} // EffectsLib::TerrainParamEffect_Load()

/*===========================================================================*/

ULONG EffectsLib::TerrainParamEffect_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
TerrainParamEffect *Current;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_RESOLUTION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_FLOAT, (char *)&TerrainParamBase.Resolution)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_OVERLAPOK, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&TerrainParamBase.OverlapOK)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_FLOATING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&TerrainParamBase.Floating)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_TERRAINPARAM_HORFRACTDISPLACE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TerrainParamBase.HorFractDisplace)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_TERRAINPARAM_BACKFACECULL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TerrainParamBase.BackfaceCull)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_TERRAINPARAM_FRDINVALID, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TerrainParamBase.FrdInvalid)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_TERRAINPARAM_FRACTALMETHOD, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TerrainParamBase.FractalMethod)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_TERRAINPARAM_REGENFDMS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TerrainParamBase.RegenFDMsEachRender)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_TERRAINPARAM_MAPPIXSIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&TerrainParamBase.DepthMapPixSize)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

Current = TerrainParam;
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
					} // if TerrainParam effect saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (TerrainParamEffect *)Current->Next;
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

} // EffectsLib::TerrainParamEffect_Save()

/*===========================================================================*/
