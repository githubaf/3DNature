// RenderPoly.cpp
// Pulling together all the data needed to render a DEM
// Code for World Construction Set Renderer by Gary R. Huber, 9/3/99.
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "Application.h"
#include "Render.h"
#include "Raster.h"
#include "EffectsLib.h"
#include "Useful.h"
#include "AppMem.h"
#include "DEM.h"
#include "PixelManager.h"
#include "Joe.h"
#include "Requester.h"
#include "VectorPolygon.h"
#include "VectorNode.h"
#include "Lists.h"

// return codes
#define WCS_ERROR		0	// this can be used to halt the renderer if a memory failure occurs
//#define WCS_NO_ERROR	1
//#define WCS_DIAGNOSTIC_POLYNUMBERASRELEL
//#define TESTING_VECTOR_POLYGON_RENDERTIME

extern int FragMemFailed;

//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//extern unsigned __int64 DevCounter[50];
//#endif // FRANK or GARY

#ifdef WCS_VECPOLY_EFFECTS

int Renderer::TerrainVectorPolygonSetup(VectorPolygon *VecPoly, VertexData *Vtx[3])
{
double PolySide[2][3];
PolygonData Poly;	// creates and initializes members, needs to be initialized for each polygon rendered
VertexBase TempVert;
MaterialList *Matty;
int Ct, Success = true;

if (! (Vtx[0]->NodeData && Vtx[1]->NodeData && Vtx[2]->NodeData))
	{
	UserMessageOK("VNS 3 Debug Message", "One of the vertex data pointers is NULL.\nPlease advise 3D Nature immediately. Thank you.");
	return (0);
	} // if
	
// At each vertex of the polygon, if a lake or stream is present in the polygon water elevations will be
// derived for each vertex by analyzing the lake or stream component.

// At each vertex none or one or more than one ecosystems may be declared.
// For terraffected polygons, the vertices may specify an ecosystem or a mix of ecosystems which may be
// different for the other vertices of the polygon.
// If tfx ecosystems do not fill all the coverage at any vertex, vector-bounded ecosystems are examined
// in order of their priority until the eco coverage is full.
// If still not complete, ecosystem-matching color maps are exmined.
// Finally if there is still coverage available then rules of nature must be evaluated.

// The result will be a list of ecosystems and their weighted percentage.
// Within each ecosystem is the possibility of either one or two materials, each with a prescribed percentage.
// In addition lakes and streams may be supplying a beach with one or two materials and a water surface with
// one or two materials and an alternate set of elevations.

// A list of materials should be made from the ecosystem list. The material list might be a linked list of
// objects that contain a percentage value, a material pointer, a component pointer pointing back to the ecosystem, lake...
// an extra set of elevations for things like lakes and stream surfaces.

// Dynamic and positional textural information may be required to determine the mix of materials so the list will be 
// finalized after all elevations, slopes, aspects, surface normals, etc. are available for each vertex.

// Each of the three vertices will have its own list of materials and percentages.
// To render the materials it will be necessary to construct a gradient function across the triangle
// for each material.
// Will have to find out which materials are shared by more than one vertex.
// Basically it will be like creating a three column table with rows representing unique materials.
// Values in the columns will represent coverage % for the material at each vertex.
// Each pixel of surface color will be composed of a barycentrically sampled proportion of all the materials.
// Foliage stems' positions will be calculated based on foliage density for the materials. Then elliminated to
// satisfy material density textures and the barycentric weighting functions of the triangle for given materials.

if (LakesExist || StreamsExist)
	BuildWaterMaterialListTable(Vtx, WaterMatTable, EcoMatTable);
if (EcosExist)
	BuildMaterialListTable(Vtx, WCS_EFFECTSSUBCLASS_ECOSYSTEM, EcoMatTable, LakesExist || StreamsExist);
if (SnowsExist)
	BuildMaterialListTable(Vtx, WCS_EFFECTSSUBCLASS_SNOW, SnowMatTable, false);
if (CmapsExist)
	BuildMaterialListTable(Vtx, WCS_EFFECTSSUBCLASS_CMAP, FoliageTintTable, false);
BuildMaterialListTable(Vtx, WCS_EFFECTSSUBCLASS_GROUND, GroundMatTable, false);

for (Ct = 0; Ct < 3; Ct ++)
	{
	// convert degrees to cartesian and project to screen
	Cam->ProjectVertexDEM(DefCoords, Vtx[Ct], EarthLatScaleMeters, PlanetRad, 1);
	if (! Poly.Snow && SnowMatTable[0].Material)
		{
		if (Matty = SnowMatTable[0].MatListPtr[Ct])
			{
			Poly.Snow = (SnowEffect *)Matty->MyEffect;
			Poly.SnowCoverage = 1.0;
			} // if
		} // if
	if (! Poly.Eco && EcoMatTable[0].Material)
		{
		if (Matty = EcoMatTable[0].MatListPtr[Ct])
			{
			if (Matty->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_ECOSYSTEM)
				Poly.Eco = (EcosystemEffect *)Matty->MyEffect;
			Poly.Vector = Matty->MyVec;
			} // if
		} // if
	if (! Poly.Ground && GroundMatTable[0].Material)
		{
		if (Matty = GroundMatTable[0].MatListPtr[Ct])
			{
			Poly.Ground = (GroundEffect *)Matty->MyEffect;
			} // if
		} // if
	if (! Poly.Env && Vtx[Ct]->NodeData->NodeEnvironment)
		{
		Poly.Env = Vtx[Ct]->NodeData->NodeEnvironment;
		} // if
	} // for

if (! Poly.Ground)
	Poly.Ground = EffectsBase->DefaultGround;
if (! Poly.Env)
	Poly.Env = EffectsBase->DefaultEnvironment;

// set polygon area from the DEM cell size and fractal depth,
// a more rigorous and accurate computation of triangle area, 
// requires XYZ of triangle points in cartesian space
Poly.Area = SolveTriangleArea(Vtx);

// average vertex values to obtain polygon values
// these values will be used in colormap, ecosystem, and various other polygon-based evaluations
Poly.Lat = (Vtx[0]->Lat + Vtx[1]->Lat + Vtx[2]->Lat) * (1.0 / 3.0);
Poly.Lon = (Vtx[0]->Lon + Vtx[1]->Lon + Vtx[2]->Lon) * (1.0 / 3.0);
Poly.Elev = (Vtx[0]->Elev + Vtx[1]->Elev + Vtx[2]->Elev) * (1.0 / 3.0);
Poly.Z = (Vtx[0]->ScrnXYZ[2] + Vtx[1]->ScrnXYZ[2] + Vtx[2]->ScrnXYZ[2]) * (1.0 / 3.0);
Poly.Q = (Vtx[0]->Q + Vtx[1]->Q + Vtx[2]->Q) * (1.0 / 3.0);
Poly.RelEl = (Vtx[0]->RelEl + Vtx[1]->RelEl + Vtx[2]->RelEl) * (float)(1.0 / 3.0);

Poly.LonSeed = (ULONG)((Poly.Lon - floor(Poly.Lon)) * ULONG_MAX);
Poly.LatSeed = (ULONG)((Poly.Lat - floor(Poly.Lat)) * ULONG_MAX);

// compute surface normal
FindPosVector(PolySide[0], Vtx[1]->XYZ, Vtx[0]->XYZ);
FindPosVector(PolySide[1], Vtx[2]->XYZ, Vtx[0]->XYZ);
// the order in VNS 3 is reversed from VNS 2
SurfaceNormal(Poly.Normal, PolySide[0], PolySide[1]);

// compute slope
// the table lookup for the slope was giving some pretty erroneous results for slopes near 0 
// (like showing slope of 6 degrees instead of 0).
//Poly.Slope = acos(VectorAngle(Vtx[0]->XYZ, Poly.Normal)) * PiUnder180;	// in degrees
Poly.Slope = GlobalApp->MathTables->ACosTab.LookupLerped(VectorAngle(Vtx[0]->XYZ, Poly.Normal)) * PiUnder180;	// in degrees
if (Poly.Slope < 0.0)
	Poly.Slope = 0.0;
else if (Poly.Slope > 90.0)
	Poly.Slope = 90.0;

// compute aspect
TempVert.XYZ[0] = Poly.Normal[0];
TempVert.XYZ[1] = Poly.Normal[1];
TempVert.XYZ[2] = Poly.Normal[2];
TempVert.RotateY(-Poly.Lon);
TempVert.RotateX(90.0 - Poly.Lat);
Poly.Aspect = TempVert.FindRoughAngleYfromZ();
if (Poly.Aspect < 0.0)
	Poly.Aspect += 360.0;

// big change for VNS 3
if (PhongTerrain)
	Poly.ShadeType = WCS_EFFECT_MATERIAL_SHADING_PHONG;

if (ShadowsExist)
	EffectsBase->EvalShadows(&RendData, &Poly);

return (Success && InstigateTerrainPolygon(&Poly, Vtx));

} // Renderer::TerrainVectorPolygonSetup

/*===========================================================================*/

void Renderer::BuildMaterialListTable(VertexData *Vtx[3], long EffectType, MaterialTable *FillMe, bool AppendTable)
{
MaterialList *Matty;
unsigned long VtxNum, MatTableEntry, MatTableEntries, FirstTableEntry;
bool FoundMat;

if (AppendTable && FillMe[0].Material)
	{
	for (MatTableEntries = 1; MatTableEntries < MAX_MATERIALTABLE_ENTRIES; ++MatTableEntries)
		{
		if (! FillMe[MatTableEntries].Material)
			break;
		} // for
	FirstTableEntry = MatTableEntries;
	} // if
else
	FirstTableEntry = MatTableEntries = 0;
	
for (VtxNum = 0; VtxNum < 3; ++VtxNum)
	{
	for (Matty = Vtx[VtxNum]->NodeData->FindFirstEffect(EffectType); Matty;
		Matty = Vtx[VtxNum]->NodeData->FindNextEffect(Matty, EffectType))
		{
		FoundMat = false;
		for (MatTableEntry = FirstTableEntry; MatTableEntry < MatTableEntries; ++MatTableEntry)
			{
			if (FillMe[MatTableEntry].Material == Matty->MyMat)
				{
				if ((FillMe[MatTableEntry].MatListPtr[0] && FillMe[MatTableEntry].MatListPtr[0]->MyVec == Matty->MyVec)
					|| (FillMe[MatTableEntry].MatListPtr[1] && FillMe[MatTableEntry].MatListPtr[1]->MyVec == Matty->MyVec)
					|| (FillMe[MatTableEntry].MatListPtr[2] && FillMe[MatTableEntry].MatListPtr[2]->MyVec == Matty->MyVec))
					{
					FillMe[MatTableEntry].MatListPtr[VtxNum] = Matty;
					FoundMat = true;
					break;
					} // if
				} // if
			} // for
		if (! FoundMat)
			{
			if (MatTableEntries >= MAX_MATERIALTABLE_ENTRIES)
				break;
			FillMe[MatTableEntries].Material = Matty->MyMat;
			FillMe[MatTableEntries].MatListPtr[0] = FillMe[MatTableEntries].MatListPtr[1] = FillMe[MatTableEntries].MatListPtr[2] = NULL;
			FillMe[MatTableEntries].MatListPtr[VtxNum] = Matty;
			++MatTableEntries;
			if (MatTableEntries >= MAX_MATERIALTABLE_ENTRIES)
				break;
			} // if
		/* I just can't imagine how this can be helping anything in vector-aligned textures - GRH 8/8/08
		else if (FillMe[MatTableEntry].MatListPtr[VtxNum]->VectorProp)
			{
			if (VtxNum == 2)
				{
				// problems with texture coords resulting from terraffector vector end wrapping
				if (FillMe[MatTableEntry].MatListPtr[1] && FillMe[MatTableEntry].MatListPtr[1]->VectorProp)
					{
					if ((FillMe[MatTableEntry].MatListPtr[2]->VectorProp->VecOffsets[0] > 0.0f && FillMe[MatTableEntry].MatListPtr[1]->VectorProp->VecOffsets[0] < 0.0f) ||
						(FillMe[MatTableEntry].MatListPtr[2]->VectorProp->VecOffsets[0] < 0.0f && FillMe[MatTableEntry].MatListPtr[1]->VectorProp->VecOffsets[0] > 0.0f))
						{
						FillMe[MatTableEntry].MatListPtr[2]->VectorProp->VecOffsets[0] = -FillMe[MatTableEntry].MatListPtr[2]->VectorProp->VecOffsets[0]; 
						} // if
					} // if
				else if (FillMe[MatTableEntry].MatListPtr[0] && FillMe[MatTableEntry].MatListPtr[0]->VectorProp)
					{
					if ((FillMe[MatTableEntry].MatListPtr[2]->VectorProp->VecOffsets[0] > 0.0f && FillMe[MatTableEntry].MatListPtr[0]->VectorProp->VecOffsets[0] < 0.0f) ||
						(FillMe[MatTableEntry].MatListPtr[2]->VectorProp->VecOffsets[0] < 0.0f && FillMe[MatTableEntry].MatListPtr[0]->VectorProp->VecOffsets[0] > 0.0f))
						{
						FillMe[MatTableEntry].MatListPtr[2]->VectorProp->VecOffsets[0] = -FillMe[MatTableEntry].MatListPtr[2]->VectorProp->VecOffsets[0]; 
						} // if
					} // if
				} // else
			else if (VtxNum == 1)
				{
				if (FillMe[MatTableEntry].MatListPtr[0] && FillMe[MatTableEntry].MatListPtr[0]->VectorProp)
					{
					// problems with texture coords resulting from terraffector vector end wrapping
					if ((FillMe[MatTableEntry].MatListPtr[1]->VectorProp->VecOffsets[0] > 0.0f && FillMe[MatTableEntry].MatListPtr[0]->VectorProp->VecOffsets[0] < 0.0f) ||
						(FillMe[MatTableEntry].MatListPtr[1]->VectorProp->VecOffsets[0] < 0.0f && FillMe[MatTableEntry].MatListPtr[0]->VectorProp->VecOffsets[0] > 0.0f))
						{
						FillMe[MatTableEntry].MatListPtr[1]->VectorProp->VecOffsets[0] = -FillMe[MatTableEntry].MatListPtr[1]->VectorProp->VecOffsets[0]; 
						} // if
					} // if
				} // else
			} // if
		*/
		} // for
	} // for

FillMe[MatTableEntries].Material = NULL;

} // Renderer::BuildMaterialListTable

/*===========================================================================*/

void Renderer::BuildWaterMaterialListTable(VertexData *Vtx[3], MaterialTable *WaterTable, MaterialTable *EcoTable)
{
long EffectType = 0, NextWaterMatListToUse = 0, SearchMatTableEntry;
unsigned long VtxNum, MatTableEntry, WaterMatTableEntries = 0, EcoMatTableEntries = 0;
unsigned long *MatTableEntries;
MaterialList *Matty;
MaterialTable *TableToUse;
MatListWaterProperties *FoundWaterProp;
MatListVectorProperties *FoundVectorProp;
bool FoundMat;

WaterTable[0].AuxValue[2] = WaterTable[0].AuxValue[1] = WaterTable[0].AuxValue[0] = -FLT_MAX;

for (EffectType = WCS_EFFECTSSUBCLASS_LAKE; EffectType; EffectType = (EffectType == WCS_EFFECTSSUBCLASS_LAKE) ? WCS_EFFECTSSUBCLASS_STREAM: 0)
	{
	for (VtxNum = 0; VtxNum < 3; ++VtxNum)
		{
		for (Matty = Vtx[VtxNum]->NodeData->FindFirstEffect(EffectType); Matty;
			Matty = Vtx[VtxNum]->NodeData->FindNextEffect(Matty, EffectType))
			{
			if (Matty->MyMat->MaterialType == WCS_EFFECTS_MATERIALTYPE_WATER)
				{
				TableToUse = WaterTable;
				MatTableEntries = &WaterMatTableEntries;
				if (Matty->WaterProp && Matty->WaterProp->WaterDepth > WaterTable[0].AuxValue[VtxNum])
					WaterTable[0].AuxValue[VtxNum] = Matty->WaterProp->WaterDepth;
				} // if
			else
				{
				TableToUse = EcoTable;
				MatTableEntries = &EcoMatTableEntries;
				} // else
				
			FoundMat = false;
			for (MatTableEntry = 0; MatTableEntry < *MatTableEntries; ++MatTableEntry)
				{
				if (TableToUse[MatTableEntry].Material == Matty->MyMat)
					{
					if ((TableToUse[MatTableEntry].MatListPtr[0] && TableToUse[MatTableEntry].MatListPtr[0]->MyVec == Matty->MyVec)
						|| (TableToUse[MatTableEntry].MatListPtr[1] && TableToUse[MatTableEntry].MatListPtr[1]->MyVec == Matty->MyVec)
						|| (TableToUse[MatTableEntry].MatListPtr[2] && TableToUse[MatTableEntry].MatListPtr[2]->MyVec == Matty->MyVec))
						{
						TableToUse[MatTableEntry].MatListPtr[VtxNum] = Matty;
						FoundMat = true;
						break;
						} // if
					} // if
				} // for
			if (! FoundMat)
				{
				if (*MatTableEntries >= MAX_MATERIALTABLE_ENTRIES)
					break;
				TableToUse[*MatTableEntries].Material = Matty->MyMat;
				TableToUse[*MatTableEntries].MatListPtr[0] = TableToUse[*MatTableEntries].MatListPtr[1] = TableToUse[*MatTableEntries].MatListPtr[2] = NULL;
				TableToUse[*MatTableEntries].MatListPtr[VtxNum] = Matty;
				++(*MatTableEntries);
				if (*MatTableEntries >= MAX_MATERIALTABLE_ENTRIES)
					break;
				} // if
			} // for
		} // for
	} // for

// if there are missing water table entries, link empty slots to one of the existing material list entries
for (MatTableEntry = 0; MatTableEntry < WaterMatTableEntries; ++MatTableEntry)
	{
	Matty = NULL;
	if (WaterTable[MatTableEntry].MatListPtr[0] && WaterTable[MatTableEntry].MatListPtr[0]->WaterProp)
		Matty = WaterTable[MatTableEntry].MatListPtr[0];
	else if (WaterTable[MatTableEntry].MatListPtr[1] && WaterTable[MatTableEntry].MatListPtr[1]->WaterProp)
		Matty = WaterTable[MatTableEntry].MatListPtr[1];
	else if (WaterTable[MatTableEntry].MatListPtr[2] && WaterTable[MatTableEntry].MatListPtr[2]->WaterProp)
		Matty = WaterTable[MatTableEntry].MatListPtr[2];

	for (VtxNum = 0; VtxNum < 3; ++VtxNum)
		{
		if (! WaterTable[MatTableEntry].MatListPtr[VtxNum])
			{
			WaterTable[MatTableEntry].MatListPtr[VtxNum] = &RendererScopeWaterMatList[NextWaterMatListToUse++];
			WaterTable[MatTableEntry].MatListPtr[VtxNum]->MyMat = Matty->MyMat;
			WaterTable[MatTableEntry].MatListPtr[VtxNum]->MatCoverage = 0.0;
			WaterTable[MatTableEntry].MatListPtr[VtxNum]->MyEffect = Matty->MyEffect;
			WaterTable[MatTableEntry].MatListPtr[VtxNum]->MyVec = Matty->MyVec;
			// look for a MatListWaterProperties in other materials for this node
			FoundWaterProp = NULL;
			FoundVectorProp = NULL;
			for (SearchMatTableEntry = (long)MatTableEntry - 1; SearchMatTableEntry < (long)MatTableEntry + 2; ++SearchMatTableEntry)
				{
				if (SearchMatTableEntry < 0 || SearchMatTableEntry == MatTableEntry)
					continue;
				if ((unsigned long)SearchMatTableEntry >= WaterMatTableEntries)
					break;
				if (WaterTable[SearchMatTableEntry].MatListPtr[VtxNum])
					{
					if (WaterTable[SearchMatTableEntry].MatListPtr[VtxNum]->MyEffect == WaterTable[MatTableEntry].MatListPtr[VtxNum]->MyEffect
						&& WaterTable[SearchMatTableEntry].MatListPtr[VtxNum]->MyVec == WaterTable[MatTableEntry].MatListPtr[VtxNum]->MyVec)
						{
						FoundWaterProp = WaterTable[SearchMatTableEntry].MatListPtr[VtxNum]->WaterProp;
						FoundVectorProp = WaterTable[SearchMatTableEntry].MatListPtr[VtxNum]->VectorProp;
						break;
						} // if
					} // if
				} // for
			if (FoundWaterProp)
				WaterTable[MatTableEntry].MatListPtr[VtxNum]->WaterProp = FoundWaterProp;
			else
				{
				WaterTable[MatTableEntry].MatListPtr[VtxNum]->WaterProp = Matty->WaterProp;
				if (Matty->WaterProp && Matty->WaterProp->WaterDepth > WaterTable[0].AuxValue[VtxNum])
					WaterTable[0].AuxValue[VtxNum] = Matty->WaterProp->WaterDepth;
				} // else
			WaterTable[MatTableEntry].MatListPtr[VtxNum]->VectorProp = FoundVectorProp;
			} // if
		} // for

	/*		
	if (! WaterTable[MatTableEntry].MatListPtr[0])
		{
		WaterTable[MatTableEntry].MatListPtr[0] = Matty;
		if (Matty->WaterProp && Matty->WaterProp->WaterDepth > WaterTable[0].AuxValue[0])
			WaterTable[0].AuxValue[0] = Matty->WaterProp->WaterDepth;
		} // if
	if (! WaterTable[MatTableEntry].MatListPtr[1])
		{
		WaterTable[MatTableEntry].MatListPtr[1] = Matty;
		if (Matty->WaterProp && Matty->WaterProp->WaterDepth > WaterTable[0].AuxValue[1])
			WaterTable[0].AuxValue[1] = Matty->WaterProp->WaterDepth;
		} // if
	if (! WaterTable[MatTableEntry].MatListPtr[2])
		{
		WaterTable[MatTableEntry].MatListPtr[2] = Matty;
		if (Matty->WaterProp && Matty->WaterProp->WaterDepth > WaterTable[0].AuxValue[2])
			WaterTable[0].AuxValue[2] = Matty->WaterProp->WaterDepth;
		} // if
	*/
	// problems with texture coords resulting from terraffector vector end wrapping
	/* causes texture distortion along centerline of vector
	if (WaterTable[MatTableEntry].MatListPtr[1]->VectorProp && WaterTable[MatTableEntry].MatListPtr[0]->VectorProp)
		{
		if ((WaterTable[MatTableEntry].MatListPtr[1]->VectorProp->VecOffsets[0] > 0.0f && WaterTable[MatTableEntry].MatListPtr[0]->VectorProp->VecOffsets[0] < 0.0f) ||
			(WaterTable[MatTableEntry].MatListPtr[1]->VectorProp->VecOffsets[0] < 0.0f && WaterTable[MatTableEntry].MatListPtr[0]->VectorProp->VecOffsets[0] > 0.0f))
			{
			WaterTable[MatTableEntry].MatListPtr[1]->VectorProp->VecOffsets[0] = -WaterTable[MatTableEntry].MatListPtr[1]->VectorProp->VecOffsets[0]; 
			} // if
		} // if
	if (WaterTable[MatTableEntry].MatListPtr[2]->VectorProp && WaterTable[MatTableEntry].MatListPtr[0]->VectorProp)
		{
		if ((WaterTable[MatTableEntry].MatListPtr[2]->VectorProp->VecOffsets[0] > 0.0f && WaterTable[MatTableEntry].MatListPtr[0]->VectorProp->VecOffsets[0] < 0.0f) ||
			(WaterTable[MatTableEntry].MatListPtr[2]->VectorProp->VecOffsets[0] < 0.0f && WaterTable[MatTableEntry].MatListPtr[0]->VectorProp->VecOffsets[0] > 0.0f))
			{
			WaterTable[MatTableEntry].MatListPtr[2]->VectorProp->VecOffsets[0] = -WaterTable[MatTableEntry].MatListPtr[2]->VectorProp->VecOffsets[0]; 
			} // if
		} // if
	*/
	} // for

WaterTable[WaterMatTableEntries].Material = NULL;
EcoTable[EcoMatTableEntries].Material = NULL;

} // Renderer::BuildWaterMaterialListTable
#else // WCS_VECPOLY_EFFECTS
/*===========================================================================*/

int Renderer::TerrainPolygonSetup(VertexData *Vtx[3], unsigned char CurFrd)
{
PolygonData Poly;	// creates and initializes members, needs to be initialized for each polygon rendered
VertexBase TempVert;
char LevelCt, Ct, LevelDif, ComputeLerp;
int Success;
double PolySide[2][3], EdgeLerp;
PolygonEdge *EdgeParent;
VertexData *UseVertex;

// takes the vertices of the subdivided polygon
// at this point the vertex just contains its coordinates, modified perhaps by pre-fractal area terraffectors,
// and any interpolated values like relel.
#ifndef TESTING_VECTOR_POLYGON_RENDERTIME
// make adjustments to elevations, search for ecosystems and water bodies
for (Ct = 0; Ct < 3; Ct ++)
	{
	// Set the texture data flag to 0 unless this is done earlier
	//Vtx[Ct]->TexDataInitialized = 0;
	EdgeParent = NULL;
	ComputeLerp = 1;
	// evaluate area terraffectors
	// only post-fractal area terraffectors evaluate here
	// area terraffectors can
		// modify elevations
		// adjust fractal displacement
	if (RasterTAsExist && ! (Vtx[Ct]->Flags & WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED))
		{
		// EdgeParent is always NULL in this first section
		if ((/*EdgeParent || */(EdgeParent = Vtx[Ct]->EdgeParent)) && Vtx[Ct]->Frd > Vtx[Ct]->EdgeParent->Frd)
			{
			// ComputeLerp is always true in this first section
			//if (ComputeLerp)
				{
				LevelDif = Vtx[Ct]->Frd - EdgeParent->Frd;
				for (LevelCt = 1; LevelCt < LevelDif; LevelCt ++)
					{
					EdgeParent = EdgeParent->Parent;
					} // for
				EdgeLerp = (EdgeParent->End[0]->Lat != EdgeParent->End[1]->Lat) ? 
					(Vtx[Ct]->Lat - EdgeParent->End[0]->Lat) / (EdgeParent->End[1]->Lat - EdgeParent->End[0]->Lat):
					(Vtx[Ct]->Lon - EdgeParent->End[0]->Lon) / (EdgeParent->End[1]->Lon - EdgeParent->End[0]->Lon);
				ComputeLerp = 0;
				} // if
			if (! (EdgeParent->End[0]->Flags & WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED))
				{
				EffectsBase->EvalRasterTerraffectors(&RendData, EdgeParent->End[0], 1);		// 1 = after fractals
				EdgeParent->End[0]->Flags |= WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED;
				} // if
			if (! (EdgeParent->End[1]->Flags & WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED))
				{
				EffectsBase->EvalRasterTerraffectors(&RendData, EdgeParent->End[1], 1);
				EdgeParent->End[1]->Flags |= WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED;
				} // if
			// lerp the value
			Vtx[Ct]->Elev = EdgeParent->End[0]->Elev + (EdgeParent->End[1]->Elev - EdgeParent->End[0]->Elev) * EdgeLerp;
			// <<<>>>gh more values to lerp?
			} // if
		else
			{
			EffectsBase->EvalRasterTerraffectors(&RendData, Vtx[Ct], 1);
			} // else
		Vtx[Ct]->Flags |= WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED;
		} // if

	// evaluate terraffectors
	// terraffectors can
		// modify elevations
		// adjust fractal displacement
		// specify an ecosystem
		// specify a vector for later texturing
	if (TerraffectorsExist && ! (Vtx[Ct]->Flags & WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED))
		{
		if ((EdgeParent || (EdgeParent = Vtx[Ct]->EdgeParent)) && Vtx[Ct]->Frd > Vtx[Ct]->EdgeParent->Frd)
			{
			if (ComputeLerp)
				{
				LevelDif = Vtx[Ct]->Frd - EdgeParent->Frd;
				for (LevelCt = 1; LevelCt < LevelDif; LevelCt ++)
					{
					EdgeParent = EdgeParent->Parent;
					} // for
				EdgeLerp = (EdgeParent->End[0]->Lat != EdgeParent->End[1]->Lat) ? 
					(Vtx[Ct]->Lat - EdgeParent->End[0]->Lat) / (EdgeParent->End[1]->Lat - EdgeParent->End[0]->Lat):
					(Vtx[Ct]->Lon - EdgeParent->End[0]->Lon) / (EdgeParent->End[1]->Lon - EdgeParent->End[0]->Lon);
				ComputeLerp = 0;
				} // if
			if (! (EdgeParent->End[0]->Flags & WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED))
				{
				EffectsBase->EvalTerraffectors(&RendData, EdgeParent->End[0]);
				EdgeParent->End[0]->Flags |= WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED;
				} // if
			if (! (EdgeParent->End[1]->Flags & WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED))
				{
				EffectsBase->EvalTerraffectors(&RendData, EdgeParent->End[1]);
				EdgeParent->End[1]->Flags |= WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED;
				} // if
			// lerp the values
			// EdgeLerp has been initialized at this point in spite of what LINT may think
			Vtx[Ct]->Elev = EdgeParent->End[0]->Elev + (EdgeParent->End[1]->Elev - EdgeParent->End[0]->Elev) * EdgeLerp;
			Vtx[Ct]->Eco = EdgeLerp <= .5 ? EdgeParent->End[0]->Eco: EdgeParent->End[1]->Eco;
			Vtx[Ct]->Vector = EdgeLerp <= .5 ? EdgeParent->End[0]->Vector: EdgeParent->End[1]->Vector;
			if (Vtx[Ct]->Vector)
				{
				Vtx[Ct]->VectorType = EdgeParent->End[0]->Vector ? EdgeParent->End[0]->VectorType: EdgeParent->End[1]->Vector ? EdgeParent->End[1]->VectorType: (char)(WCS_TEXTURE_VECTOREFFECTTYPE_LINE | WCS_TEXTURE_VECTOREFFECTTYPE_SPLINEELEV);
				Vtx[Ct]->VecOffsets[0] = EdgeParent->End[0]->VecOffsets[0] + (EdgeParent->End[1]->VecOffsets[0] - EdgeParent->End[0]->VecOffsets[0]) * (float)EdgeLerp;
				Vtx[Ct]->VecOffsets[1] = EdgeParent->End[0]->VecOffsets[1] + (EdgeParent->End[1]->VecOffsets[1] - EdgeParent->End[0]->VecOffsets[1]) * (float)EdgeLerp;
				Vtx[Ct]->VecOffsets[2] = EdgeParent->End[0]->VecOffsets[2] + (EdgeParent->End[1]->VecOffsets[2] - EdgeParent->End[0]->VecOffsets[2]) * (float)EdgeLerp;
				} // if
			} // if
		else
			{
			EffectsBase->EvalTerraffectors(&RendData, Vtx[Ct]);
			} // else
		Vtx[Ct]->Flags |= WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED;
		} // if

	// evaluate lakes
	// lakes can 
		// add a water elevation,
		// replace stream data if elevation of surface is higher
		// fill in the Lake member of the polygon for water and beach material
	// note that waves are not computed here
	if (LakesExist && ! (Vtx[Ct]->Flags & WCS_VERTEXDATA_FLAG_LAKEAPPLIED))
		{
		if ((EdgeParent || (EdgeParent = Vtx[Ct]->EdgeParent)) && Vtx[Ct]->Frd > Vtx[Ct]->EdgeParent->Frd)
			{
			if (ComputeLerp)
				{
				LevelDif = Vtx[Ct]->Frd - EdgeParent->Frd;
				for (LevelCt = 1; LevelCt < LevelDif; LevelCt ++)
					{
					EdgeParent = EdgeParent->Parent;
					} // for
				EdgeLerp = (EdgeParent->End[0]->Lat != EdgeParent->End[1]->Lat) ? 
					(Vtx[Ct]->Lat - EdgeParent->End[0]->Lat) / (EdgeParent->End[1]->Lat - EdgeParent->End[0]->Lat):
					(Vtx[Ct]->Lon - EdgeParent->End[0]->Lon) / (EdgeParent->End[1]->Lon - EdgeParent->End[0]->Lon);
				ComputeLerp = 0;
				} // if
			if (! (EdgeParent->End[0]->Flags & WCS_VERTEXDATA_FLAG_LAKEAPPLIED))
				{
				EffectsBase->EvalLakes(&RendData, EdgeParent->End[0]);
				EdgeParent->End[0]->Flags |= WCS_VERTEXDATA_FLAG_LAKEAPPLIED;
				} // if
			if (! (EdgeParent->End[1]->Flags & WCS_VERTEXDATA_FLAG_LAKEAPPLIED))
				{
				EffectsBase->EvalLakes(&RendData, EdgeParent->End[1]);
				EdgeParent->End[1]->Flags |= WCS_VERTEXDATA_FLAG_LAKEAPPLIED;
				} // if
			if ((EdgeParent->End[0]->Stream || EdgeParent->End[0]->Lake) && (EdgeParent->End[1]->Stream || EdgeParent->End[1]->Lake))
				{
				Vtx[Ct]->WaterElev = EdgeParent->End[0]->WaterElev + (EdgeParent->End[1]->WaterElev - EdgeParent->End[0]->WaterElev) * EdgeLerp;
				Vtx[Ct]->WaveHeight = EdgeParent->End[0]->WaveHeight + (EdgeParent->End[1]->WaveHeight - EdgeParent->End[0]->WaveHeight) * EdgeLerp;
				Vtx[Ct]->Lake = EdgeLerp <= .5 ? EdgeParent->End[0]->Lake: EdgeParent->End[1]->Lake;
				Vtx[Ct]->Beach = EdgeLerp <= .5 ? EdgeParent->End[0]->Beach: EdgeParent->End[1]->Beach;
				} // if
			else if (EdgeParent->End[0]->Stream || EdgeParent->End[0]->Lake)
				{
				Vtx[Ct]->WaterElev = EdgeParent->End[0]->WaterElev;
				Vtx[Ct]->WaveHeight = EdgeParent->End[0]->WaveHeight;
				Vtx[Ct]->Lake = EdgeParent->End[0]->Lake;
				Vtx[Ct]->Beach = EdgeParent->End[0]->Beach;
				} // if
			else if (EdgeParent->End[1]->Stream || EdgeParent->End[1]->Lake)
				{
				Vtx[Ct]->WaterElev = EdgeParent->End[1]->WaterElev;
				Vtx[Ct]->WaveHeight = EdgeParent->End[1]->WaveHeight;
				Vtx[Ct]->Lake = EdgeParent->End[1]->Lake;
				Vtx[Ct]->Beach = EdgeParent->End[1]->Beach;
				} // if
			} // if
		else
			{
			EffectsBase->EvalLakes(&RendData, Vtx[Ct]);
			} // else
		Vtx[Ct]->Flags |= (WCS_VERTEXDATA_FLAG_LAKEAPPLIED | WCS_VERTEXDATA_FLAG_WAVEAPPLIED);
		} // if

	// evaluate streams
	// streams can 
		// add a water elevation,
		// fill in the Lake member of the polygon for water and beach material
		// fill in the vector member of Polygon for later texturing
	// note that waves are not computed here
	if (StreamsExist && ! (Vtx[Ct]->Flags & WCS_VERTEXDATA_FLAG_STREAMAPPLIED))
		{
		if ((EdgeParent || (EdgeParent = Vtx[Ct]->EdgeParent)) && Vtx[Ct]->Frd > Vtx[Ct]->EdgeParent->Frd)
			{
			if (ComputeLerp)
				{
				LevelDif = Vtx[Ct]->Frd - EdgeParent->Frd;
				for (LevelCt = 1; LevelCt < LevelDif; LevelCt ++)
					{
					EdgeParent = EdgeParent->Parent;
					} // for
				EdgeLerp = (EdgeParent->End[0]->Lat != EdgeParent->End[1]->Lat) ? 
					(Vtx[Ct]->Lat - EdgeParent->End[0]->Lat) / (EdgeParent->End[1]->Lat - EdgeParent->End[0]->Lat):
					(Vtx[Ct]->Lon - EdgeParent->End[0]->Lon) / (EdgeParent->End[1]->Lon - EdgeParent->End[0]->Lon);
				ComputeLerp = 0;
				} // if
			if (! (EdgeParent->End[0]->Flags & WCS_VERTEXDATA_FLAG_STREAMAPPLIED))
				{
				EffectsBase->EvalStreams(&RendData, EdgeParent->End[0]);
				EdgeParent->End[0]->Flags |= WCS_VERTEXDATA_FLAG_STREAMAPPLIED;
				} // if
			if (! (EdgeParent->End[1]->Flags & WCS_VERTEXDATA_FLAG_STREAMAPPLIED))
				{
				EffectsBase->EvalStreams(&RendData, EdgeParent->End[1]);
				EdgeParent->End[1]->Flags |= WCS_VERTEXDATA_FLAG_STREAMAPPLIED;
				} // if
			if ((EdgeParent->End[0]->Stream || EdgeParent->End[0]->Lake) && (EdgeParent->End[1]->Stream || EdgeParent->End[1]->Lake))
				{
				Vtx[Ct]->WaterElev = EdgeParent->End[0]->WaterElev + (EdgeParent->End[1]->WaterElev - EdgeParent->End[0]->WaterElev) * EdgeLerp;
				Vtx[Ct]->WaveHeight = EdgeParent->End[0]->WaveHeight + (EdgeParent->End[1]->WaveHeight - EdgeParent->End[0]->WaveHeight) * EdgeLerp;
				Vtx[Ct]->Stream = EdgeLerp <= .5 ? EdgeParent->End[0]->Stream: EdgeParent->End[1]->Stream;
				Vtx[Ct]->Beach = EdgeLerp <= .5 ? EdgeParent->End[0]->Beach: EdgeParent->End[1]->Beach;
				Vtx[Ct]->Vector = EdgeLerp <= .5 ? EdgeParent->End[0]->Vector: EdgeParent->End[1]->Vector;
				if (Vtx[Ct]->Vector)
					{
					Vtx[Ct]->VecOffsets[0] = EdgeParent->End[0]->VecOffsets[0] + (EdgeParent->End[1]->VecOffsets[0] - EdgeParent->End[0]->VecOffsets[0]) * (float)EdgeLerp;
					Vtx[Ct]->VecOffsets[1] = EdgeParent->End[0]->VecOffsets[1] + (EdgeParent->End[1]->VecOffsets[1] - EdgeParent->End[0]->VecOffsets[1]) * (float)EdgeLerp;
					Vtx[Ct]->VecOffsets[2] = EdgeParent->End[0]->VecOffsets[2] + (EdgeParent->End[1]->VecOffsets[2] - EdgeParent->End[0]->VecOffsets[2]) * (float)EdgeLerp;
					Vtx[Ct]->VectorType = EdgeParent->End[0]->Vector ? EdgeParent->End[0]->VectorType: EdgeParent->End[1]->Vector ? EdgeParent->End[1]->VectorType: (char)(WCS_TEXTURE_VECTOREFFECTTYPE_LINE | WCS_TEXTURE_VECTOREFFECTTYPE_SPLINEELEV);
					} // if
				} // if
			else if (EdgeParent->End[0]->Stream || EdgeParent->End[0]->Lake)
				{
				Vtx[Ct]->WaterElev = EdgeParent->End[0]->WaterElev;
				Vtx[Ct]->WaveHeight = EdgeParent->End[0]->WaveHeight;
				Vtx[Ct]->Stream = EdgeParent->End[0]->Stream;
				Vtx[Ct]->Beach = EdgeParent->End[0]->Beach;
				Vtx[Ct]->Vector = EdgeParent->End[0]->Vector;
				if (Vtx[Ct]->Vector)
					{
					Vtx[Ct]->VecOffsets[0] = EdgeParent->End[0]->VecOffsets[0];
					Vtx[Ct]->VecOffsets[1] = EdgeParent->End[0]->VecOffsets[1];
					Vtx[Ct]->VecOffsets[2] = EdgeParent->End[0]->VecOffsets[2];
					Vtx[Ct]->VectorType = EdgeParent->End[0]->Vector ? EdgeParent->End[0]->VectorType: (char)(WCS_TEXTURE_VECTOREFFECTTYPE_LINE | WCS_TEXTURE_VECTOREFFECTTYPE_SPLINEELEV);
					} // if
				} // if
			else if (EdgeParent->End[1]->Stream || EdgeParent->End[1]->Lake)
				{
				Vtx[Ct]->WaterElev = EdgeParent->End[1]->WaterElev;
				Vtx[Ct]->WaveHeight = EdgeParent->End[1]->WaveHeight;
				Vtx[Ct]->Stream = EdgeParent->End[1]->Stream;
				Vtx[Ct]->Beach = EdgeParent->End[1]->Beach;
				Vtx[Ct]->Vector = EdgeParent->End[1]->Vector;
				if (Vtx[Ct]->Vector)
					{
					Vtx[Ct]->VecOffsets[0] = EdgeParent->End[1]->VecOffsets[0];
					Vtx[Ct]->VecOffsets[1] = EdgeParent->End[1]->VecOffsets[1];
					Vtx[Ct]->VecOffsets[2] = EdgeParent->End[1]->VecOffsets[2];
					Vtx[Ct]->VectorType = EdgeParent->End[1]->Vector ? EdgeParent->End[1]->VectorType: (char)(WCS_TEXTURE_VECTOREFFECTTYPE_LINE | WCS_TEXTURE_VECTOREFFECTTYPE_SPLINEELEV);
					} // if
				} // if
			} // if
		else
			{
			EffectsBase->EvalStreams(&RendData, Vtx[Ct]);
			} // else
		Vtx[Ct]->Flags |= (WCS_VERTEXDATA_FLAG_STREAMAPPLIED | WCS_VERTEXDATA_FLAG_WAVEAPPLIED);
		} // if
	} // for

// determine ecosystems before sorting vertices so that they are not dependent on screen orientation
// check for beaches first
if (Vtx[0]->Beach || Vtx[1]->Beach || Vtx[2]->Beach)
	{
	if (Vtx[1]->Beach == Vtx[2]->Beach)
		{
		Poly.Beach = Vtx[1]->Beach;
		} // else if
	else
		{
		Poly.Beach = Vtx[0]->Beach;
		} // else just pick one
	} // if beach

// then check for streams and lakes
/* this leaves gaps along the water edge
if (Vtx[0]->Stream && Vtx[1]->Stream && Vtx[2]->Stream)
	{
	if (Vtx[1]->Stream == Vtx[2]->Stream)
		{
		Poly.Stream = Vtx[1]->Stream;
		Poly.Vector = Vtx[1]->Vector;
		Poly.VectorType = Vtx[1]->VectorType;
		Poly.VectorSlope = Vtx[1]->VectorSlope;
		} // else if
	else
		{
		Poly.Stream = Vtx[0]->Stream;
		Poly.Vector = Vtx[0]->Vector;
		Poly.VectorType = Vtx[0]->VectorType;
		Poly.VectorSlope = Vtx[0]->VectorSlope;
		} // else just pick one
	} // if lake
*/
// This creates spikes along stream edge where there are non-stream vertices unless
// any non-stream vertices have their water levels and wave heights set to one of the other vertices' levels.
// It is OK if the vertex is not a stream but a lake which also has a water level and wave height.
if (Vtx[0]->Stream || Vtx[1]->Stream || Vtx[2]->Stream)
	{
	if (Vtx[0]->Stream && (Vtx[0]->Stream == Vtx[1]->Stream || Vtx[0]->Stream == Vtx[2]->Stream))
		{
		Poly.Stream = Vtx[0]->Stream;
		Poly.Vector = Vtx[0]->Vector;
		Poly.VectorType = Vtx[0]->VectorType;
		Poly.VectorSlope = Vtx[0]->VectorSlope;
		} // if
	else if (Vtx[1]->Stream && Vtx[1]->Stream == Vtx[2]->Stream)
		{
		Poly.Stream = Vtx[1]->Stream;
		Poly.Vector = Vtx[1]->Vector;
		Poly.VectorType = Vtx[1]->VectorType;
		Poly.VectorSlope = Vtx[1]->VectorSlope;
		} // else if
	else
		{
		if (Vtx[0]->Stream)
			{
			Poly.Stream = Vtx[0]->Stream;
			Poly.Vector = Vtx[0]->Vector;
			Poly.VectorType = Vtx[0]->VectorType;
			Poly.VectorSlope = Vtx[0]->VectorSlope;
			} // if
		else if (Vtx[1]->Stream)
			{
			Poly.Stream = Vtx[1]->Stream;
			Poly.Vector = Vtx[1]->Vector;
			Poly.VectorType = Vtx[1]->VectorType;
			Poly.VectorSlope = Vtx[1]->VectorSlope;
			} // if
		else
			{
			Poly.Stream = Vtx[2]->Stream;
			Poly.Vector = Vtx[2]->Vector;
			Poly.VectorType = Vtx[2]->VectorType;
			Poly.VectorSlope = Vtx[2]->VectorSlope;
			} // if
		} // else just pick one
	// fix up any non-stream vertices with a water level and wave height
	if (! Vtx[0]->Stream && ! Vtx[0]->Lake)
		{
		if (Vtx[1]->Stream || Vtx[1]->Lake)
			{
			if (Vtx[2]->Stream || Vtx[2]->Lake)
				{
				Vtx[0]->WaterElev = (Vtx[1]->WaterElev + Vtx[2]->WaterElev) * 0.5;  // Optimized out division. Was / 2.0
				Vtx[0]->WaveHeight = (Vtx[1]->WaveHeight + Vtx[2]->WaveHeight) * 0.5; // Optimized out division. Was / 2.0
				} // if
			else
				{
				Vtx[0]->WaterElev = Vtx[1]->WaterElev;
				Vtx[0]->WaveHeight = Vtx[1]->WaveHeight;
				} // else
			} // if
		else
			{
			Vtx[0]->WaterElev = Vtx[2]->WaterElev;
			Vtx[0]->WaveHeight = Vtx[2]->WaveHeight;
			} // else
		} // if
	if (! Vtx[1]->Stream && ! Vtx[1]->Lake)
		{
		if (Vtx[0]->Stream || Vtx[0]->Lake)
			{
			if (Vtx[2]->Stream || Vtx[2]->Lake)
				{
				Vtx[1]->WaterElev = (Vtx[0]->WaterElev + Vtx[2]->WaterElev) * 0.5;  // Optimized out division. Was / 2.0 
				Vtx[1]->WaveHeight = (Vtx[0]->WaveHeight + Vtx[2]->WaveHeight) * 0.5; // Optimized out division. Was / 2.0
				} // if
			else
				{
				Vtx[1]->WaterElev = Vtx[0]->WaterElev;
				Vtx[1]->WaveHeight = Vtx[0]->WaveHeight;
				} // else
			} // if
		else
			{
			Vtx[1]->WaterElev = Vtx[2]->WaterElev;
			Vtx[1]->WaveHeight = Vtx[2]->WaveHeight;
			} // else
		} // if
	if (! Vtx[2]->Stream && ! Vtx[2]->Lake)
		{
		if (Vtx[0]->Stream || Vtx[0]->Lake)
			{
			if (Vtx[1]->Stream || Vtx[1]->Lake)
				{
				Vtx[2]->WaterElev = (Vtx[0]->WaterElev + Vtx[1]->WaterElev) * 0.5;  // Optimized out division. Was / 2.0 
				Vtx[2]->WaveHeight = (Vtx[0]->WaveHeight + Vtx[1]->WaveHeight) * 0.5; // Optimized out division. Was / 2.0
				} // if
			else
				{
				Vtx[2]->WaterElev = Vtx[0]->WaterElev;
				Vtx[2]->WaveHeight = Vtx[0]->WaveHeight;
				} // else
			} // if
		else
			{
			Vtx[2]->WaterElev = Vtx[1]->WaterElev;
			Vtx[2]->WaveHeight = Vtx[1]->WaveHeight;
			} // else
		} // if
	} // if stream
else if (Vtx[0]->Lake || Vtx[1]->Lake || Vtx[2]->Lake)
	{
	if (Vtx[0]->Lake && (Vtx[0]->Lake == Vtx[1]->Lake || Vtx[0]->Lake == Vtx[2]->Lake))
		{
		Poly.Lake = Vtx[0]->Lake;
		} // if
	else if (Vtx[1]->Lake && Vtx[1]->Lake == Vtx[2]->Lake)
		{
		Poly.Lake = Vtx[1]->Lake;
		} // else if
	else
		{
		Poly.Lake = Vtx[0]->Lake ? Vtx[0]->Lake: Vtx[1]->Lake ? Vtx[1]->Lake: Vtx[2]->Lake;
		} // else just pick one
	if (Poly.Lake)
		{
		// it is possible that one or more vertices fell outside the lake boundaries and need a water elevation
		if (! Vtx[0]->Lake)
			{
			Vtx[0]->WaterElev = (Vtx[1]->Lake && Vtx[2]->Lake) ? (Vtx[1]->WaterElev + Vtx[2]->WaterElev) * .5:
				Vtx[1]->Lake ? Vtx[1]->WaterElev: Vtx[2]->WaterElev;
			Vtx[0]->WaveHeight = (Vtx[1]->Lake && Vtx[2]->Lake) ? (Vtx[1]->WaveHeight + Vtx[2]->WaveHeight) * .5:
				Vtx[1]->Lake ? Vtx[1]->WaveHeight: Vtx[2]->WaveHeight;
			//Vtx[0]->WaterElev = max(Vtx[1]->WaterElev, Vtx[2]->WaterElev);
			} // if
		if (! Vtx[1]->Lake)
			{
			Vtx[1]->WaterElev = (Vtx[0]->Lake && Vtx[2]->Lake) ? (Vtx[0]->WaterElev + Vtx[2]->WaterElev) * .5:
				Vtx[0]->Lake ? Vtx[0]->WaterElev: Vtx[2]->WaterElev;
			Vtx[1]->WaveHeight = (Vtx[0]->Lake && Vtx[2]->Lake) ? (Vtx[0]->WaveHeight + Vtx[2]->WaveHeight) * .5:
				Vtx[0]->Lake ? Vtx[0]->WaveHeight: Vtx[2]->WaveHeight;
			//Vtx[1]->WaterElev = max(Vtx[0]->WaterElev, Vtx[2]->WaterElev);
			} // if
		if (! Vtx[2]->Lake)
			{
			Vtx[2]->WaterElev = (Vtx[0]->Lake && Vtx[1]->Lake) ? (Vtx[0]->WaterElev + Vtx[1]->WaterElev) * .5:
				Vtx[0]->Lake ? Vtx[0]->WaterElev: Vtx[1]->WaterElev;
			Vtx[2]->WaveHeight = (Vtx[0]->Lake && Vtx[1]->Lake) ? (Vtx[0]->WaveHeight + Vtx[1]->WaveHeight) * .5:
				Vtx[0]->Lake ? Vtx[0]->WaveHeight: Vtx[1]->WaveHeight;
			//Vtx[2]->WaterElev = max(Vtx[0]->WaterElev, Vtx[1]->WaterElev);
			} // if
		} // if
	} // if lake

// if no beach, check for eco
if (! Poly.Beach)
	{
	if (Vtx[0]->Eco || Vtx[1]->Eco || Vtx[2]->Eco)
		{
		if (Vtx[0]->Eco == Vtx[1]->Eco || Vtx[0]->Eco == Vtx[2]->Eco)
			{
			UseVertex = Vtx[0];
			} // if
		else if (Vtx[1]->Eco == Vtx[2]->Eco)
			{
			UseVertex = Vtx[1];
			} // else if
		else
			{
			UseVertex = Vtx[0]->Eco ? Vtx[0]: Vtx[1]->Eco ? Vtx[1]: Vtx[2];
			} // else just pick one
		Poly.Eco = UseVertex->Eco;
		if (! Poly.Lake && ! Poly.Stream)
			{
			Poly.Vector = UseVertex->Vector;
			Poly.VectorType = UseVertex->VectorType;
			} // if
		} // if ecosystem
	} // if
#endif // TESTING_VECTOR_POLYGON_RENDERTIME
// calculate unknown vector offsets for textures
if (Poly.Vector)
	{
	int OffEnd = 0, SplineLatLon, SplineElev, ConnectEnds, SkipFirst;
	double JoeElev;

	SplineLatLon = Poly.VectorType & WCS_TEXTURE_VECTOREFFECTTYPE_SPLINELATLON;
	SplineElev = Poly.VectorType & WCS_TEXTURE_VECTOREFFECTTYPE_SPLINEELEV;
	ConnectEnds = Poly.VectorType & WCS_TEXTURE_VECTOREFFECTTYPE_CONNECTENDS;
	SkipFirst = Poly.VectorType & WCS_TEXTURE_VECTOREFFECTTYPE_SKIPFIRSTPOINT;
	if (Vtx[0]->Vector != Poly.Vector)
		Poly.Vector->MinDistToPoint(Vtx[0]->Lat, Vtx[0]->Lon, Vtx[0]->Elev, EarthLatScaleMeters, Exageration, 
		ElevDatum, SplineLatLon, SplineElev, ConnectEnds, SkipFirst, OffEnd, JoeElev, Vtx[0]->VecOffsets, NULL);
	if (Vtx[1]->Vector != Poly.Vector)
		Poly.Vector->MinDistToPoint(Vtx[1]->Lat, Vtx[1]->Lon, Vtx[1]->Elev, EarthLatScaleMeters, Exageration, 
		ElevDatum, SplineLatLon, SplineElev, ConnectEnds, SkipFirst, OffEnd, JoeElev, Vtx[1]->VecOffsets, NULL);
	if (Vtx[2]->Vector != Poly.Vector)
		Poly.Vector->MinDistToPoint(Vtx[2]->Lat, Vtx[2]->Lon, Vtx[2]->Elev, EarthLatScaleMeters, Exageration, 
		ElevDatum, SplineLatLon, SplineElev, ConnectEnds, SkipFirst, OffEnd, JoeElev, Vtx[2]->VecOffsets, NULL);
	RendData.TexData.VDataVecOffsetsValid = 1;
	} // if
else
	RendData.TexData.VDataVecOffsetsValid = 0;

// this will need to be repeated when water surface is rendered. It needs to be done before
// Poly->Z and Q are calculated which are then used to determine foliage dissolve and
// material textures maybe.
for (Ct = 0; Ct < 3; Ct ++)
	{
	// convert degrees to cartesian and project to screen
	Cam->ProjectVertexDEM(DefCoords, Vtx[Ct], EarthLatScaleMeters, PlanetRad, 1);
	} // for

// set polygon area from the DEM cell size and fractal depth,
// a more rigorous and accurate computation of triangle area, 
// requires XYZ of triangle points in cartesian space
Poly.Area = SolveTriangleArea(Vtx);

// average vertex values to obtain polygon values
// these values will be used in colormap, ecosystem, and various other polygon-based evaluations
Poly.Lat = (Vtx[0]->Lat + Vtx[1]->Lat + Vtx[2]->Lat) * (1.0 / 3.0);
Poly.Lon = (Vtx[0]->Lon + Vtx[1]->Lon + Vtx[2]->Lon) * (1.0 / 3.0);
Poly.Elev = (Vtx[0]->Elev + Vtx[1]->Elev + Vtx[2]->Elev) * (1.0 / 3.0);
Poly.WaterElev = (Vtx[0]->WaterElev + Vtx[1]->WaterElev + Vtx[2]->WaterElev) * (1.0 / 3.0);
Poly.Z = (Vtx[0]->ScrnXYZ[2] + Vtx[1]->ScrnXYZ[2] + Vtx[2]->ScrnXYZ[2]) * (1.0 / 3.0);
Poly.Q = (Vtx[0]->Q + Vtx[1]->Q + Vtx[2]->Q) * (1.0 / 3.0);
Poly.RelEl = (Vtx[0]->RelEl + Vtx[1]->RelEl + Vtx[2]->RelEl) * (float)(1.0 / 3.0);

Poly.LonSeed = (ULONG)((Poly.Lon - WCS_floor(Poly.Lon)) * ULONG_MAX);
Poly.LatSeed = (ULONG)((Poly.Lat - WCS_floor(Poly.Lat)) * ULONG_MAX);

// compute surface normal
FindPosVector(PolySide[0], Vtx[1]->XYZ, Vtx[0]->XYZ);
FindPosVector(PolySide[1], Vtx[2]->XYZ, Vtx[0]->XYZ);
SurfaceNormal(Poly.Normal, PolySide[1], PolySide[0]);

// compute view vector
// <<<>>>gh this is only needed if we're going to do backface culling
/*
FindPosVector(Poly.ViewVec, Vtx[0]->XYZ, Cam->CamPos->XYZ);
FindPosVector(PolySide[0], Vtx[1]->XYZ, Cam->CamPos->XYZ);
FindPosVector(PolySide[1], Vtx[2]->XYZ, Cam->CamPos->XYZ);
AddPoint3d(Poly.ViewVec, PolySide[0]);
AddPoint3d(Poly.ViewVec, PolySide[1]);
*/

// compute slope
//Poly.Slope = acos(VectorAngle(Vtx[0]->XYZ, Poly.Normal)) * PiUnder180;	// in degrees
Poly.Slope = GlobalApp->MathTables->ACosTab.LookupLerped(VectorAngle(Vtx[0]->XYZ, Poly.Normal)) * PiUnder180;	// in degrees
if (Poly.Slope < 0.0)
	Poly.Slope = 0.0;
else if (Poly.Slope > 90.0)
	Poly.Slope = 90.0;

// compute aspect
TempVert.XYZ[0] = Poly.Normal[0];
TempVert.XYZ[1] = Poly.Normal[1];
TempVert.XYZ[2] = Poly.Normal[2];
TempVert.RotateY(-Poly.Lon);
TempVert.RotateX(90.0 - Poly.Lat);
Poly.Aspect = TempVert.FindRoughAngleYfromZ();
if (Poly.Aspect < 0.0)
	Poly.Aspect += 360.0;

// search for ecosystem effect or cmap
if (! Poly.Beach && ! Poly.Eco)
	{
	// look for ecosystem effect first
	if (Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM])
		EffectsBase->EvalEcosystems(&Poly);

	if (! Poly.Eco)
		{
		// evaluate color maps
		// color maps can
			// fill in Ecosystem member of polygon if matched
			// fill in polygon RGB color
			// set flag to cause tinted foliage
			// fill in Cmap member of polygon
		if (CmapsExist)
			EffectsBase->EvalCmaps(&RendData, &Poly);

		// if there was no color map or if the cmap said to tint foliage then need to find an ecosystem
		if (! Poly.Cmap || (! Poly.Eco && Poly.TintFoliage))
			{
			// evaluate environment
			// this can
				// fill in Env member of polygon
			EffectsBase->EvalEnvironment(&Poly);
			if (Poly.Env)
				{
				// evaluate ecosystem through the Rules of Nature
				// this can
					// fill in Eco member of polygon
				Poly.Env->EvalEcosystem(&Poly);
				} // if
			} // if
		} // if
	} // if

// evaluate snow
// this can
	// fill in Snow member of polygon
if (SnowsExist && (! Poly.Eco || ! Poly.Eco->PlowSnow))
	EffectsBase->EvalSnow(&Poly);

// evaluate ground
// Ground must exist, hence must be supplied as a default if there are none created by the user
// Ground may have already been supplied by the Cmap evaluator if it is a draped image cmap
// this will
	// fill in Ground member of polygon
if (! Poly.Ground && ! Poly.RenderCmapColor)
	EffectsBase->EvalGround(&Poly);

// We should now have all the basic material containers established.

// evaluate terrain shadow effects
// this can modify Poly.ReceiveShadows and Poly.ReceivedShadowintensity
if (ShadowsExist)
	EffectsBase->EvalShadows(&RendData, &Poly);

Success = InstigateTerrainPolygon(&Poly, Vtx);

// do Foliage Effects and 3D Objects
/* done a different way now 3/23/01
if (Success)
	{
	long FolEffectsRendered = 0, Object3DEffectsRendered = 0;
	double StashPolyElev;
	Ecotype *LocalEco;
	Object3DEffect *Object3D;

	StashPolyElev = Poly.Elev;
	// check for foliage effects
	if (FoliageRastNum > 0)
		{
		while (FolEffectsRendered >= 0)
			{
			Poly.Elev = StashPolyElev;
			if ((FolEffectsRendered = EffectsBase->EvalFoliageThreePoints(&RendData, (VertexDEM **)Vtx, FolEffectsRendered, FoliageRastNum, FoliageVertexPtr, &Poly, LocalEco)) > 0)
				{
				// and because some people are idiots we have to check to see if there is any foliage
				if (LocalEco->FoliageChain)
					{
					Poly.LonSeed = (ULONG)(fabs((Vtx[0]->Map->pNorthWest.Lon - Poly.Lon) / Vtx[0]->Map->LonRange) * ULONG_MAX);
					Poly.LatSeed = (ULONG)(fabs((Poly.Lat - Vtx[0]->Map->pSouthEast.Lat) / Vtx[0]->Map->LatRange) * ULONG_MAX);
					if (! (Success = RenderFoliage(&Poly, Vtx, LocalEco, 1.0, 1.0, WCS_SEEDOFFSET_FOLIAGEEFFECT, 1)))
						break;
					} // if
				} // if
			} // while
		} // if
	
	if (Object3DRastNum > 0)
		{
		VertexDEM ObjVert;

		while (Object3DEffectsRendered >= 0)
			{
			if ((Object3DEffectsRendered = EffectsBase->EvalObject3DThreePoints(&RendData, (VertexDEM **)Vtx, Object3DEffectsRendered, Object3DRastNum, Object3DVertexPtr, &ObjVert, &Poly, (Object3DEffect *&)Object3D)) > 0)
				{
				Cam->ProjectVertexDEM(&ObjVert, EarthLatScaleMeters, PlanetRad, 1);
				if (! (Success = Render3DObject(&Poly, &ObjVert, Object3D, NULL, -1.0)))
					break;
				} // if Object3DEffectsRendered
			} // while Object3DEffectsRendered >= 0
		} // if Object3DRastNum > 0
	} // if
*/
return (Success);

} // Renderer::TerrainPolygonSetup
#endif // WCS_VECPOLY_EFFECTS

/*===========================================================================*/

int Renderer::InstigateTerrainPolygon(PolygonData *Poly, VertexData *Vtx[3])
{
double PolySide[2][3], StashElev[3];
#ifdef WCS_VECPOLY_EFFECTS
double  ElevEntry[3];
unsigned long FirstWaterMatEntry, LastWaterMatEntry, AboveGroundPoints;
#endif // WCS_VECPOLY_EFFECTS
unsigned long VertCt;
int Result;

#ifdef WCS_VECPOLY_EFFECTS
if (Poly->Fnce)
	{
	Poly->Object = Poly->Fnce;
	Poly->Beach->GetRenderMaterial(&Poly->GroundMat, Poly->Fnce->GetRenderMatGradientPos(&RendData, Poly));
	return (RenderPolygon(Poly, (VertexBase **)Vtx, WCS_POLYGONTYPE_FENCE));
	} // if just fence

// figure out which entities need to be rendered
// terrain ?
if (EcoMatTable[0].Material || SnowMatTable[0].Material || GroundMatTable[0].Material)
	{
	// don't draw it if it is a transparent ecosystem, snow or ground
	if ((Poly->Snow && Poly->Snow->Eco.Transparent) || (Poly->Eco && Poly->Eco->Transparent)
		|| (Poly->Ground && Poly->Ground->Transparent))
		return (1);
	if (Poly->Eco)
		// account for foliage dissolve
		CalculateFoliageDissolve(Poly, EcoMatTable);
	if (Poly->Snow)
		{
		Poly->Object = Poly->Snow;
		if (WaterMatTable[0].Material)
			{
			// create a fadeout of snow from +.5m to water line. By multiplying by 2 .5 becomes 1
			// which is the cutoff below which snow is faded out and above which it is simply 100%.
			// smaller multipliers will give a wider zone of fadeout.
			SnowMatTable[0].AuxValue[0] = -WaterMatTable[0].AuxValue[0] * 2.0;
			SnowMatTable[0].AuxValue[1] = -WaterMatTable[0].AuxValue[1] * 2.0;
			SnowMatTable[0].AuxValue[2] = -WaterMatTable[0].AuxValue[2] * 2.0;
			} // if
		else
			// no water so snow factors are all 1
			SnowMatTable[0].AuxValue[0] = SnowMatTable[0].AuxValue[1] = SnowMatTable[0].AuxValue[2] = 1.0;
		} // else if snow
	else if (Poly->Eco)
		{
		Poly->Object = Poly->Eco;
		} // if
	else if (Poly->Ground)
		{
		// if its a draped image ground use the cmap instead which should be the parent of the ground
		Poly->Object = Poly->Ground->GetRAHostRoot();
		} // else if just ground
		
	if (! RenderPolygon(Poly, (VertexBase **)Vtx, WCS_POLYGONTYPE_TERRAIN))
		return (0);
	} // if
// water ?
if (WaterMatTable[0].Material)
	{
	// render water
	// no water in shadow maps - it causes artifacts and don't want to create shadow maps for moving waves
	// and now that we do water transparency we don't want the water surface shading the water column.
	if (! ShadowMapInProgress)
		{
		StashElev[0] = Vtx[0]->Elev;
		StashElev[1] = Vtx[1]->Elev;
		StashElev[2] = Vtx[2]->Elev;
		
		for (FirstWaterMatEntry = 0; WaterMatTable[FirstWaterMatEntry].Material; ++FirstWaterMatEntry)
			{
			if (WaterMatTable[FirstWaterMatEntry].MatListPtr[0] &&
				WaterMatTable[FirstWaterMatEntry].MatListPtr[1] &&
				WaterMatTable[FirstWaterMatEntry].MatListPtr[2] &&
				WaterMatTable[FirstWaterMatEntry].MatListPtr[0]->WaterProp &&
				WaterMatTable[FirstWaterMatEntry].MatListPtr[1]->WaterProp &&
				WaterMatTable[FirstWaterMatEntry].MatListPtr[2]->WaterProp)
				{
				Poly->Object = WaterMatTable[FirstWaterMatEntry].MatListPtr[0]->MyEffect;
				ElevEntry[0] = Vtx[0]->Elev + WaterMatTable[FirstWaterMatEntry].MatListPtr[0]->WaterProp->WaterDepth + WaterMatTable[FirstWaterMatEntry].MatListPtr[0]->WaterProp->WaveHeight;
				ElevEntry[1] = Vtx[1]->Elev + WaterMatTable[FirstWaterMatEntry].MatListPtr[1]->WaterProp->WaterDepth + WaterMatTable[FirstWaterMatEntry].MatListPtr[1]->WaterProp->WaveHeight;
				ElevEntry[2] = Vtx[2]->Elev + WaterMatTable[FirstWaterMatEntry].MatListPtr[2]->WaterProp->WaterDepth + WaterMatTable[FirstWaterMatEntry].MatListPtr[2]->WaterProp->WaveHeight;
				for (LastWaterMatEntry = FirstWaterMatEntry + 1; WaterMatTable[LastWaterMatEntry].Material; ++LastWaterMatEntry)
					{
					if (WaterMatTable[LastWaterMatEntry].MatListPtr[0] &&
						WaterMatTable[LastWaterMatEntry].MatListPtr[1] &&
						WaterMatTable[LastWaterMatEntry].MatListPtr[2] &&
						WaterMatTable[LastWaterMatEntry].MatListPtr[0]->WaterProp &&
						WaterMatTable[LastWaterMatEntry].MatListPtr[1]->WaterProp &&
						WaterMatTable[LastWaterMatEntry].MatListPtr[2]->WaterProp)
						{
						if (ElevEntry[0] != Vtx[0]->Elev + WaterMatTable[LastWaterMatEntry].MatListPtr[0]->WaterProp->WaterDepth + WaterMatTable[LastWaterMatEntry].MatListPtr[0]->WaterProp->WaveHeight ||
							ElevEntry[1] != Vtx[1]->Elev + WaterMatTable[LastWaterMatEntry].MatListPtr[1]->WaterProp->WaterDepth + WaterMatTable[LastWaterMatEntry].MatListPtr[1]->WaterProp->WaveHeight ||
							ElevEntry[2] != Vtx[2]->Elev + WaterMatTable[LastWaterMatEntry].MatListPtr[2]->WaterProp->WaterDepth + WaterMatTable[LastWaterMatEntry].MatListPtr[2]->WaterProp->WaveHeight)
							{
							break;
							} // if
						} // if
					} // for
				// LastWaterMatEntry is one beyond the current grouping of materials
				WaterMatTableStart = &WaterMatTable[FirstWaterMatEntry];
				WaterMatTableEnd = &WaterMatTable[LastWaterMatEntry];
				
				// find out if at least one water vertex is visible above land, no need to render if it is below ground		
				AboveGroundPoints = 0;
				if (ElevEntry[0] >= Vtx[0]->Elev)
					++AboveGroundPoints;
				else if (ElevEntry[1] >= Vtx[1]->Elev)
					++AboveGroundPoints;
				else if (ElevEntry[2] >= Vtx[2]->Elev)
					++AboveGroundPoints;
				if (AboveGroundPoints)
					{
					// apply water elevations to vertices but remember what the original elevations were
					for (VertCt = 0; VertCt < 3; VertCt ++)
						{
						// terrain renders first so we can dispense with the terrain elevation field
						Vtx[VertCt]->Elev = ElevEntry[VertCt];
						Vtx[VertCt]->WaterDepth = WaterMatTable[FirstWaterMatEntry].MatListPtr[VertCt]->WaterProp->WaterDepth;
						Vtx[VertCt]->WaveHeight = WaterMatTable[FirstWaterMatEntry].MatListPtr[VertCt]->WaterProp->WaveHeight;
						Cam->ProjectVertexDEM(DefCoords, Vtx[VertCt], EarthLatScaleMeters, PlanetRad, 1);
						} // for
					// compute surface normal
					FindPosVector(PolySide[0], Vtx[1]->XYZ, Vtx[0]->XYZ);
					FindPosVector(PolySide[1], Vtx[2]->XYZ, Vtx[0]->XYZ);
					// the order in VNS 3 is reversed from VNS 2
					SurfaceNormal(Poly->Normal, PolySide[0], PolySide[1]);

					Result = RenderPolygon(Poly, (VertexBase **)Vtx, WCS_POLYGONTYPE_WATER);
					// restore elevations of vertices for the next material to evaluate with
					Vtx[0]->Elev = StashElev[0];
					Vtx[1]->Elev = StashElev[1];
					Vtx[2]->Elev = StashElev[2];
					// return error
					if (! Result)
						return (0);
					}// if
				FirstWaterMatEntry = LastWaterMatEntry - 1;
				} // if
			} // for
		} // if
	} // if water

return (1);

#else // WCS_VECPOLY_EFFECTS
char RenderBeach;
int BeachMatFound = 0;
double MaxBeachElev, MinWaterElev, FoliageScaleHt;

Result = 1;

// Poly->Mat should start out as NULL

// figure out what materials are going to be needed by polygon scan converter
// and make one or two calls to RenderPolygon
if (Poly->Stream)
	{
	// render beach
	// if highest beach vertex is above lowest water vertex or maximum foliage height for either beach
	//  ecotype is above lowest water vertex then need to render beach polygon
	RenderBeach = 0;
	MaxBeachElev = max(Vtx[0]->Elev, Vtx[1]->Elev);
	MaxBeachElev = max(Vtx[2]->Elev, MaxBeachElev);
	MinWaterElev = min(Vtx[0]->WaterElev + Vtx[0]->WaveHeight, Vtx[1]->WaterElev + Vtx[1]->WaveHeight);
	MinWaterElev = min(Vtx[2]->WaterElev + Vtx[2]->WaveHeight, MinWaterElev);

	if (Poly->Beach)
		{
		Poly->Object = Poly->Beach;
		if (Poly->Beach->RAParent->GetNotifySubclass() == WCS_EFFECTSSUBCLASS_STREAM)
			BeachMatFound = Poly->Beach->GetRenderMaterial(&Poly->BaseMat, ((StreamEffect *)Poly->Beach->RAParent)->GetRenderBeachMatGradientPos(&RendData, Poly));
		else
			BeachMatFound = Poly->Beach->GetRenderMaterial(&Poly->BaseMat, ((LakeEffect *)Poly->Beach->RAParent)->GetRenderBeachMatGradientPos(&RendData, Poly));
		} // if
	else if (Poly->Eco)
		{
		Poly->Object = Poly->Eco;
		BeachMatFound = Poly->Eco->EcoMat.GetRenderMaterial(&Poly->BaseMat, Poly->Eco->GetRenderMatGradientPos(&RendData, Poly));
		} // if
	else
		Poly->Object = Poly->Stream;

	// for transparent water we always want to render the bathymetry
	if (RenderBathos || MaxBeachElev >= MinWaterElev - 2.0)
		RenderBeach = 1;

	if (! RenderBeach && BeachMatFound)
		{
		FoliageScaleHt = Poly->Env ? Poly->Env->AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_FOLIAGEHTFACT].CurValue: 
						DefaultEnvironment->AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_FOLIAGEHTFACT].CurValue;
		// check foliage height for first material
		if (Poly->BaseMat.Mat[0]->EcoFol[0] && Poly->BaseMat.Mat[0]->EcoFol[0]->Enabled)
			{
			if (MaxBeachElev + Poly->BaseMat.Mat[0]->EcoFol[0]->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue * FoliageScaleHt >= MinWaterElev)
				RenderBeach = 1;
			} // else need to take overstory foliage into account
		else if (Poly->BaseMat.Mat[0]->EcoFol[1] && Poly->BaseMat.Mat[0]->EcoFol[1]->Enabled)
			{
			if (MaxBeachElev + Poly->BaseMat.Mat[0]->EcoFol[1]->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue * FoliageScaleHt >= MinWaterElev)
				RenderBeach = 1;
			} // else need to take understory foliage into account
		// check foliage height for second material
		if (! RenderBeach && Poly->BaseMat.Mat[1])
			{
			if (Poly->BaseMat.Mat[1]->EcoFol[0] && Poly->BaseMat.Mat[1]->EcoFol[0]->Enabled)
				{
				if (MaxBeachElev + Poly->BaseMat.Mat[1]->EcoFol[0]->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue * FoliageScaleHt >= MinWaterElev)
					RenderBeach = 1;
				} // else need to take overstory foliage into account
			else if (Poly->BaseMat.Mat[1]->EcoFol[1] && Poly->BaseMat.Mat[1]->EcoFol[1]->Enabled)
				{
				if (MaxBeachElev + Poly->BaseMat.Mat[1]->EcoFol[1]->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue * FoliageScaleHt >= MinWaterElev)
					RenderBeach = 1;
				} // else need to take understory foliage into account
			} // if
		} // if
	if (RenderBeach)
		{
		// account for foliage dissolve
		CalculateFoliageDissolve(Poly);
		if (Poly->Snow)
			{
			Poly->Snow->Eco.EcoMat.GetRenderMaterial(&Poly->SnowMat, Poly->Snow->Eco.GetRenderMatGradientPos(&RendData, Poly));
			// compute snow coverage based on snowline, snow feathering, rules of nature
			// if snow exists and has transparent flag return without drawing polygon
			if ((Poly->SnowCoverage = Poly->Snow->EvaluateCoverage(&RendData, Poly)) > 0.0 && Poly->Snow->Eco.Transparent)
				return (1);
			} // if
		if (Poly->Ground)
			Poly->Ground->EcoMat.GetRenderMaterial(&Poly->GroundMat, Poly->Ground->GetRenderMatGradientPos(&RendData, Poly));
		RenderPolygon(Poly, (VertexBase **)Vtx, WCS_POLYGONTYPE_TERRAIN);
		} // if

	// render water
	// no water in shadow maps - it causes artifacts and don't want to create shadow maps for moving waves
	// and if we ever do water transparency we don't want the water surface shading the water column.
	if (! ShadowMapInProgress)
		{
		for (VertCt = 0; VertCt < 3; VertCt ++)
			{
			// terrain renders first so we can dispense with the terrain elevation field
			StashElev[VertCt] = Vtx[VertCt]->Elev;
			Vtx[VertCt]->Elev = Vtx[VertCt]->WaterElev + Vtx[VertCt]->WaveHeight;
			Cam->ProjectVertexDEM(DefCoords, Vtx[VertCt], EarthLatScaleMeters, PlanetRad, 1);
			} // for
		FindPosVector(PolySide[0], Vtx[1]->XYZ, Vtx[0]->XYZ);
		FindPosVector(PolySide[1], Vtx[2]->XYZ, Vtx[0]->XYZ);
		SurfaceNormal(Poly->Normal, PolySide[1], PolySide[0]);

		Poly->Stream->WaterMat.GetRenderMaterial(&Poly->BaseMat, Poly->Stream->GetRenderWaterMatGradientPos(&RendData, Poly));
		Poly->Object = Poly->Stream;
		Result = RenderPolygon(Poly, (VertexBase **)Vtx, WCS_POLYGONTYPE_WATER);
		for (VertCt = 0; VertCt < 3; VertCt ++)
			{
			Vtx[VertCt]->Elev = StashElev[VertCt];
			} // for
		} // if
	return (Result);
	} // if
if (Poly->Lake)
	{
	// render beach
	// if highest beach vertex is above lowest water vertex or maximum foliage height for either beach
	//  ecotype is above lowest water vertex then need to render beach polygon
	RenderBeach = 0;
	MaxBeachElev = max(Vtx[0]->Elev, Vtx[1]->Elev);
	MaxBeachElev = max(Vtx[2]->Elev, MaxBeachElev);
	MinWaterElev = min(Vtx[0]->WaterElev + Vtx[0]->WaveHeight, Vtx[1]->WaterElev + Vtx[1]->WaveHeight);
	MinWaterElev = min(Vtx[2]->WaterElev + Vtx[2]->WaveHeight, MinWaterElev);

	if (Poly->Beach)
		{
		Poly->Object = Poly->Beach;
		if (Poly->Beach->RAParent->GetNotifySubclass() == WCS_EFFECTSSUBCLASS_LAKE)
			BeachMatFound = Poly->Beach->GetRenderMaterial(&Poly->BaseMat, ((LakeEffect *)Poly->Beach->RAParent)->GetRenderBeachMatGradientPos(&RendData, Poly));
		else
			BeachMatFound = Poly->Beach->GetRenderMaterial(&Poly->BaseMat, ((StreamEffect *)Poly->Beach->RAParent)->GetRenderBeachMatGradientPos(&RendData, Poly));
		} // if
	else if (Poly->Eco)
		{
		Poly->Object = Poly->Eco;
		BeachMatFound = Poly->Eco->EcoMat.GetRenderMaterial(&Poly->BaseMat, Poly->Eco->GetRenderMatGradientPos(&RendData, Poly));
		} // if
	else
		Poly->Object = Poly->Lake;

	// for transparent water we always want to render the bathymetry
	if (RenderBathos || MaxBeachElev >= MinWaterElev)
		RenderBeach = 1;

	if (! RenderBeach && BeachMatFound)
		{
		FoliageScaleHt = Poly->Env ? Poly->Env->AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_FOLIAGEHTFACT].CurValue: 
						DefaultEnvironment->AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_FOLIAGEHTFACT].CurValue;
		// check foliage height for first material
		if (Poly->BaseMat.Mat[0]->EcoFol[0] && Poly->BaseMat.Mat[0]->EcoFol[0]->Enabled)
			{
			if (MaxBeachElev + Poly->BaseMat.Mat[0]->EcoFol[0]->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue * FoliageScaleHt >= MinWaterElev)
				RenderBeach = 1;
			} // else need to take overstory foliage into account
		else if (Poly->BaseMat.Mat[0]->EcoFol[1] && Poly->BaseMat.Mat[0]->EcoFol[1]->Enabled)
			{
			if (MaxBeachElev + Poly->BaseMat.Mat[0]->EcoFol[1]->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue * FoliageScaleHt >= MinWaterElev)
				RenderBeach = 1;
			} // else need to take understory foliage into account
		// check foliage height for second material
		if (! RenderBeach && Poly->BaseMat.Mat[1])
			{
			if (Poly->BaseMat.Mat[1]->EcoFol[0] && Poly->BaseMat.Mat[1]->EcoFol[0]->Enabled)
				{
				if (MaxBeachElev + Poly->BaseMat.Mat[1]->EcoFol[0]->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue * FoliageScaleHt >= MinWaterElev)
					RenderBeach = 1;
				} // else need to take overstory foliage into account
			else if (Poly->BaseMat.Mat[1]->EcoFol[1] && Poly->BaseMat.Mat[1]->EcoFol[1]->Enabled)
				{
				if (MaxBeachElev + Poly->BaseMat.Mat[1]->EcoFol[1]->AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue * FoliageScaleHt >= MinWaterElev)
					RenderBeach = 1;
				} // else need to take understory foliage into account
			} // if
		} // if
	if (RenderBeach)
		{
		// account for foliage dissolve
		CalculateFoliageDissolve(Poly);
		if (Poly->Snow)
			{
			// these terms are reversed so we can tell if any of the beach is below water line 
			// in which case we don't render snow.
			MaxBeachElev = min(Vtx[0]->Elev, Vtx[1]->Elev);
			MaxBeachElev = min(Vtx[2]->Elev, MaxBeachElev);
			MinWaterElev = max(Vtx[0]->WaterElev + Vtx[0]->WaveHeight, Vtx[1]->WaterElev + Vtx[1]->WaveHeight);
			MinWaterElev = max(Vtx[2]->WaterElev + Vtx[2]->WaveHeight, MinWaterElev);

			if (MaxBeachElev >= MinWaterElev)	// if these were named appropriately they would be MinBeach and MaxWater
				{
				Poly->Snow->Eco.EcoMat.GetRenderMaterial(&Poly->SnowMat, Poly->Snow->Eco.GetRenderMatGradientPos(&RendData, Poly));
				// compute snow coverage based on snowline, snow feathering, rules of nature
				// if snow exists and has transparent flag return without drawing polygon
				if ((Poly->SnowCoverage = Poly->Snow->EvaluateCoverage(&RendData, Poly)) > 0.0 && Poly->Snow->Eco.Transparent)
					return (1);
				} // if
			} // if
		if (Poly->Ground)
			Poly->Ground->EcoMat.GetRenderMaterial(&Poly->GroundMat, Poly->Ground->GetRenderMatGradientPos(&RendData, Poly));
		RenderPolygon(Poly, (VertexBase **)Vtx, WCS_POLYGONTYPE_TERRAIN);
		} // if

	// render water
	// no water in shadow maps - it causes artifacts and don't want to create shadow maps for moving waves
	// and if we ever do water transparency we don't want the water surface shading the water column.
	if (! ShadowMapInProgress)
		{
		for (VertCt = 0; VertCt < 3; VertCt ++)
			{
			// terrain renders first so we can dispense with the terrain elevation field
			StashElev[VertCt] = Vtx[VertCt]->Elev;
			Vtx[VertCt]->Elev = Vtx[VertCt]->WaterElev + Vtx[VertCt]->WaveHeight;
			Cam->ProjectVertexDEM(DefCoords, Vtx[VertCt], EarthLatScaleMeters, PlanetRad, 1);
			} // for
		FindPosVector(PolySide[0], Vtx[1]->XYZ, Vtx[0]->XYZ);
		FindPosVector(PolySide[1], Vtx[2]->XYZ, Vtx[0]->XYZ);
		SurfaceNormal(Poly->Normal, PolySide[1], PolySide[0]);

		Poly->Lake->WaterMat.GetRenderMaterial(&Poly->BaseMat, Poly->Lake->GetRenderWaterMatGradientPos(&RendData, Poly));
		Poly->Object = Poly->Lake;
		Result = RenderPolygon(Poly, (VertexBase **)Vtx, WCS_POLYGONTYPE_WATER);
		for (VertCt = 0; VertCt < 3; VertCt ++)
			{
			Vtx[VertCt]->Elev = StashElev[VertCt];
			} // for
		} // if
	return (Result);
	} // if
if (Poly->Eco)
	{
	// don't draw it if it is a transparent ecosystem
	if (Poly->Eco->Transparent)
		return (1);
	Poly->Object = Poly->Eco;
	// find the appropriate materials within the ecosystem
	if (Poly->Eco->EcoMat.GetRenderMaterial(&Poly->BaseMat, Poly->Eco->GetRenderMatGradientPos(&RendData, Poly)))
		{
		// account for foliage dissolve
		CalculateFoliageDissolve(Poly);
		} // if
	if (Poly->Snow)
		{
		if (Poly->Snow->Eco.EcoMat.GetRenderMaterial(&Poly->SnowMat, Poly->Snow->Eco.GetRenderMatGradientPos(&RendData, Poly)))
			{
			// compute snow coverage based on snowline, snow feathering, rules of nature
			// if snow exists and has transparent flag return without drawing polygon
			if ((Poly->SnowCoverage = Poly->Snow->EvaluateCoverage(&RendData, Poly)) > 0.0 && Poly->Snow->Eco.Transparent)
				return (1);
			} // if
		}
	if (Poly->Ground)
		{
		Poly->Ground->EcoMat.GetRenderMaterial(&Poly->GroundMat, Poly->Ground->GetRenderMatGradientPos(&RendData, Poly));
		}
	return (RenderPolygon(Poly, (VertexBase **)Vtx, WCS_POLYGONTYPE_TERRAIN));
	} // if
if (Poly->Snow)
	{
	Poly->Object = Poly->Snow;
	if (Poly->Snow->Eco.EcoMat.GetRenderMaterial(&Poly->SnowMat, Poly->Snow->Eco.GetRenderMatGradientPos(&RendData, Poly)))
		{
		// compute snow coverage based on snowline, snow feathering, rules of nature
		// if snow exists and has transparent flag return without drawing polygon
		if ((Poly->SnowCoverage = Poly->Snow->EvaluateCoverage(&RendData, Poly)) > 0.0 && Poly->Snow->Eco.Transparent)
			return (1);
		} // if
	if (Poly->Ground)
		{
		if (Poly->SnowCoverage <= 0.0)
			Poly->Object = Poly->Cmap ? (RasterAnimHost *)Poly->Cmap: (RasterAnimHost *)Poly->Ground;
		Poly->Ground->EcoMat.GetRenderMaterial(&Poly->GroundMat, Poly->Ground->GetRenderMatGradientPos(&RendData, Poly));
		}
	return (RenderPolygon(Poly, (VertexBase **)Vtx, WCS_POLYGONTYPE_TERRAIN));
	} // else if snow and ground
if (Poly->Ground)
	{
	// if its a draped image ground use the cmap instead
	Poly->Object = Poly->Cmap ? (RasterAnimHost *)Poly->Cmap: (RasterAnimHost *)Poly->Ground;
	Poly->Ground->EcoMat.GetRenderMaterial(&Poly->GroundMat, Poly->Ground->GetRenderMatGradientPos(&RendData, Poly));
	return (RenderPolygon(Poly, (VertexBase **)Vtx, WCS_POLYGONTYPE_TERRAIN));
	} // else if just ground
if (Poly->Fnce)
	{
	Poly->Object = Poly->Fnce;
	Poly->Beach->GetRenderMaterial(&Poly->GroundMat, Poly->Fnce->GetRenderMatGradientPos(&RendData, Poly));
	return (RenderPolygon(Poly, (VertexBase **)Vtx, WCS_POLYGONTYPE_FENCE));
	} // else if just fence

// else use cmap color
Poly->Object = Poly->Cmap;
return (RenderPolygon(Poly, (VertexBase **)Vtx, WCS_POLYGONTYPE_TERRAIN));
#endif // WCS_VECPOLY_EFFECTS

} // Renderer::InstigateTerrainPolygon

/*===========================================================================*/

void Renderer::CalculateFoliageDissolve(PolygonData *Poly)
{
// account for foliage dissolve
#ifdef WCS_BUILD_RTX
if (! Exporter)
#endif // WCS_BUILD_RTX
	{
	if (! ShadowMapInProgress)
		{
		// compute foliage dissolve for first material
		if (Poly->BaseMat.Mat[0])
			{
			if (Poly->BaseMat.Mat[0]->EcoFol[0] && Poly->BaseMat.Mat[0]->EcoFol[0]->Enabled && Poly->BaseMat.Mat[0]->EcoFol[0]->DissolveEnabled
				&& Poly->Z > Poly->BaseMat.Mat[0]->EcoFol[0]->DissolveDistance)
				{
				// opacity is the amount of dissolve
				if (Poly->BaseMat.Mat[0]->EcoFol[0]->DissolveDistance > 0.0)
					{
					Poly->OverstoryDissolve[0] = (Poly->Z - Poly->BaseMat.Mat[0]->EcoFol[0]->DissolveDistance) / Poly->BaseMat.Mat[0]->EcoFol[0]->DissolveDistance;
					if (Poly->OverstoryDissolve[0] > 1.0)
						Poly->OverstoryDissolve[0] = 1.0;
					} // if
				else
					Poly->OverstoryDissolve[0] = 1.0;
				} // if
			if (Poly->BaseMat.Mat[0]->EcoFol[1] && Poly->BaseMat.Mat[0]->EcoFol[1]->Enabled && Poly->BaseMat.Mat[0]->EcoFol[1]->DissolveEnabled
				&& Poly->Z > Poly->BaseMat.Mat[0]->EcoFol[1]->DissolveDistance)
				{
				if (Poly->BaseMat.Mat[0]->EcoFol[1]->DissolveDistance > 0.0)
					{
					Poly->UnderstoryDissolve[0] = (Poly->Z - Poly->BaseMat.Mat[0]->EcoFol[1]->DissolveDistance) / Poly->BaseMat.Mat[0]->EcoFol[1]->DissolveDistance;
					if (Poly->UnderstoryDissolve[0] > 1.0)
						Poly->UnderstoryDissolve[0] = 1.0;
					} // if
				else
					Poly->UnderstoryDissolve[0] = 1.0;
				} // if
			} // if
		// compute foliage dissolve for second material
		if (Poly->BaseMat.Mat[1])
			{
			if (Poly->BaseMat.Mat[1]->EcoFol[0] && Poly->BaseMat.Mat[1]->EcoFol[0]->Enabled && Poly->BaseMat.Mat[1]->EcoFol[0]->DissolveEnabled
				&& Poly->Z > Poly->BaseMat.Mat[1]->EcoFol[0]->DissolveDistance)
				{
				// opacity is the amount of dissolve
				if (Poly->BaseMat.Mat[1]->EcoFol[0]->DissolveDistance > 0.0)
					{
					Poly->OverstoryDissolve[1] = (Poly->Z - Poly->BaseMat.Mat[1]->EcoFol[0]->DissolveDistance) / Poly->BaseMat.Mat[1]->EcoFol[0]->DissolveDistance;
					if (Poly->OverstoryDissolve[1] > 1.0)
						Poly->OverstoryDissolve[1] = 1.0;
					} // if
				else
					Poly->OverstoryDissolve[1] = 1.0;
				} // if
			if (Poly->BaseMat.Mat[1]->EcoFol[1] && Poly->BaseMat.Mat[1]->EcoFol[1]->Enabled && Poly->BaseMat.Mat[1]->EcoFol[1]->DissolveEnabled
				&& Poly->Z > Poly->BaseMat.Mat[1]->EcoFol[1]->DissolveDistance)
				{
				if (Poly->BaseMat.Mat[1]->EcoFol[1]->DissolveDistance > 0.0)
					{
					Poly->UnderstoryDissolve[1] = (Poly->Z - Poly->BaseMat.Mat[1]->EcoFol[1]->DissolveDistance) / Poly->BaseMat.Mat[1]->EcoFol[1]->DissolveDistance;
					if (Poly->UnderstoryDissolve[1] > 1.0)
						Poly->UnderstoryDissolve[1] = 1.0;
					} // if
				else
					Poly->UnderstoryDissolve[1] = 1.0;
				} // if
			} // if
		} // if
	} // if

} // Renderer::CalculateFoliageDissolve

/*===========================================================================*/

#ifdef WCS_VECPOLY_EFFECTS
void Renderer::CalculateFoliageDissolve(PolygonData *Poly, MaterialTable *EcoMatTab)
{
unsigned long MatTableEntry;

// account for foliage dissolve
// Values of dissolve are a range from 0 to 1 where 0 is no dissolve and 1 is complete dissolve
// Values are stored in the actual MaterialEffect so they are only valid until the next time this function is 
// evaluated which is the next polygon rendered.

#ifdef WCS_BUILD_RTX
if (! Exporter)
#endif // WCS_BUILD_RTX
	{
	if (! ShadowMapInProgress)
		{
		// compute foliage dissolve for each material
		for (MatTableEntry = 0; EcoMatTab[MatTableEntry].Material; ++MatTableEntry)
			{
			if (EcoMatTab[MatTableEntry].Material->EcoFol[0] && 
				EcoMatTab[MatTableEntry].Material->EcoFol[0]->Enabled &&
				EcoMatTab[MatTableEntry].Material->EcoFol[0]->DissolveEnabled &&
				Poly->Z > EcoMatTab[MatTableEntry].Material->EcoFol[0]->DissolveDistance)
				{
				if (EcoMatTab[MatTableEntry].Material->EcoFol[0]->DissolveDistance > 0.0)
					{
					// opacity is the amount of dissolve
					EcoMatTab[MatTableEntry].Material->OverstoryDissolve = (Poly->Z - EcoMatTab[MatTableEntry].Material->EcoFol[0]->DissolveDistance) / EcoMatTab[MatTableEntry].Material->EcoFol[0]->DissolveDistance;
					if (EcoMatTab[MatTableEntry].Material->OverstoryDissolve > 1.0)
						EcoMatTab[MatTableEntry].Material->OverstoryDissolve = 1.0;
					} // if
				else
					EcoMatTab[MatTableEntry].Material->OverstoryDissolve = 1.0;
				} // if
			else
				EcoMatTab[MatTableEntry].Material->OverstoryDissolve = 0.0;
			if (EcoMatTab[MatTableEntry].Material->EcoFol[1] && 
				EcoMatTab[MatTableEntry].Material->EcoFol[1]->Enabled && 
				EcoMatTab[MatTableEntry].Material->EcoFol[1]->DissolveEnabled &&
				Poly->Z > EcoMatTab[MatTableEntry].Material->EcoFol[1]->DissolveDistance)
				{
				if (EcoMatTab[MatTableEntry].Material->EcoFol[1]->DissolveDistance > 0.0)
					{
					EcoMatTab[MatTableEntry].Material->UnderstoryDissolve = (Poly->Z - EcoMatTab[MatTableEntry].Material->EcoFol[1]->DissolveDistance) / EcoMatTab[MatTableEntry].Material->EcoFol[1]->DissolveDistance;
					if (EcoMatTab[MatTableEntry].Material->UnderstoryDissolve > 1.0)
						EcoMatTab[MatTableEntry].Material->UnderstoryDissolve = 1.0;
					} // if
				else
					EcoMatTab[MatTableEntry].Material->UnderstoryDissolve = 1.0;
				} // if
			else
				EcoMatTab[MatTableEntry].Material->UnderstoryDissolve = 0.0;
			} // for
		} // if
	} // if

} // Renderer::CalculateFoliageDissolve
#endif // WCS_VECPOLY_EFFECTS

/*===========================================================================*/

int Renderer::RenderPolygon(PolygonData *Poly, VertexBase *Vtx[3], char PolyType)
{
// for passing to other operations such as lighting or texture evaluators
PixelData Pix;
// for the sorted vertices
VertexBase *Vert[3], *StashVtx[3], *LerpVtx[2];
// x and y will store the center value of a given sub-pixel or pixel
// Xmax and Xmin will be used to store the x extents of the polygon
// m... will store the slopes of the polygon edges
// ScanXMax and ScanYMax will be the upper range for scanning in each "for" loop
// MidX will store the X value on the m1 slope corresponding to the y value of point 1
double x, y, Xmax, Xmin, m1, m2, m3, ScanXMax, ScanYMax, MidX,
// XPixel and YPixel will be used to keep track of which pixel the 10x values fall into.
// (0, 0) is at the upper left of the polygon bounding box.
//	XPixel, YPixel, // replaced with XPixelTimesEight and YPixelTimesEight, below
// for storing the polygon bounding box in screen space
	BoxXmax, BoxXmin, BoxYmax, BoxYmin,
// these hold the left and right edge values for the center of a scan line
	Edge1, Edge2,
// for storing the edge slopes while scan converting a pair of polygon edges.
// Half slopes will be used for testing for completely filled spans. Max and Min
// edge values will store the endpoints of the completely covered span.
	Edge1Slope, Edge2Slope, //HalfEdge1Slope, HalfEdge2Slope, MaxEdge1, MinEdge2,
// for storing polygon coordinates in 10x space with (0, 0) at the upper left of the 
// polygon bounding box
	PolyX10[3], PolyY10[3];
// bump mapping strata realigns the pixel's surface normal, recomputes slope and aspect.
// some values need to be stashed so they will be correct again for the next pixel.
// snow may need to be reevaluated to see if the new slope and aspect changed the coverage.
double NormalStash[3], SlopeStash, AspectStash;
double yp, WY1, WY2, W1, W2, X1, X2, WY1Inc, WY2Inc, WX, VtxPct[3], VtxWt[3], DistDiff, SumWt, InvSumWt, NewWt, VertexIntensity, TempZ[3], TempCovg,
	CovgRemaining, LerpVal;
#ifdef WCS_VECPOLY_EFFECTS
double MatWt[3], DissolveOpacity, OpacityStash, SnowWaterMultiplier;
long MatCount;
#endif // WCS_VECPOLY_EFFECTS
// int*8 versions of XPixel, YPixel -- divide by 8 ( >> 3) before using
unsigned long int XPixelTimesEight, YPixelTimesEight;
// for storing the needed size of the pixel array
long BoxSize, BoxWidth, BoxHeight, BoxWidthX10, BoxHeightX10, RowPixels,
	X, Y, Xp, Yp, MaxX, MaxY, WtZip, PixZip, LocalX, LocalY, LocalZip, VertNum[3], VertIndex[3];
long YBitByte, XBitCt, XBitByte, BitByte;
unsigned long TopPixCovg, BotPixCovg;
int Success = 1, CropCt, VtxNum0, VtxNum1, VtxNum2;
VertexData *LerpVtxData[2];
Vertex3D *LerpVtx3D[2];
VertexReferenceData *VertRefData[3];
rPixelBlockHeader *FragMap3D = NULL;
rPixelFragment *CurFrag;
MaterialEffect *MatToRender;
VertexDEM NormalVert;
bool ReplaceValues, UseFragMap, FirstCropPass = false, TexturesExist = false, OverDissolveExists = false, 
	UnderDissolveExists = false;

LerpVtxData[1] = LerpVtxData[0] = NULL;
LerpVtx3D[1] = LerpVtx3D[0] = NULL;

// project coordinates: which set of elevations depends on whether water or terrain or 3d object
//if (PolyType == WCS_POLYGONTYPE_CLOUD)
//	{
	// already projected
//	} // if terrain
//else 
if (PolyType != WCS_POLYGONTYPE_3DOBJECT)
	{
	//for (Ct = 0; Ct < 3; Ct ++)
	//	{
	//	// don't convert degrees to cartesian, it was done before
	//	Cam->ProjectVertexDEM((VertexDEM *)Vtx[Ct], EarthLatScaleMeters, PlanetRad, 0);
	//	} // for
	// projection of opposite ends of the world can end up being a world apart
	if (Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
		{
		if (! Cam->Projected || ! Cam->Coords || Cam->Coords->GetGeographic())
			{
			if (fabs(Vtx[0]->ScrnXYZ[0] - Vtx[1]->ScrnXYZ[0]) >= Cam->PlanTranslateWidth)
				{
				if (Vtx[0]->ScrnXYZ[0] < Vtx[1]->ScrnXYZ[0])
					Vtx[0]->ScrnXYZ[0] += Cam->PlanWidth;
				else
					Vtx[1]->ScrnXYZ[0] += Cam->PlanWidth;
				} // if
			if (fabs(Vtx[0]->ScrnXYZ[0] - Vtx[2]->ScrnXYZ[0]) >= Cam->PlanTranslateWidth)
				{
				if (Vtx[0]->ScrnXYZ[0] < Vtx[2]->ScrnXYZ[0])
					Vtx[0]->ScrnXYZ[0] += Cam->PlanWidth;
				else
					Vtx[2]->ScrnXYZ[0] += Cam->PlanWidth;
				} // if
			} // if
		} // if
	} // if terrain
//else
//	{
	// 3d objects' vertices are already projected
//	} // else 3D object


// if a polygon is partly in negative Z space, crop it into sub polygons

CropCt = 0;
/*
if (Vtx[0]->ScrnXYZ[2] < 0.001)
	Vtx[0]->ScrnXYZ[2] = .001;
if (Vtx[1]->ScrnXYZ[2] < 0.001)
	Vtx[1]->ScrnXYZ[2] = .001;
if (Vtx[2]->ScrnXYZ[2] < 0.001)
	Vtx[2]->ScrnXYZ[2] = .001;
*/

if (! CamPlanOrtho)
	{
	if (Vtx[0]->ScrnXYZ[2] < 0.001)
		CropCt ++;
	if (Vtx[1]->ScrnXYZ[2] < 0.001)
		CropCt ++;
	if (Vtx[2]->ScrnXYZ[2] < 0.001)
		CropCt ++;
	} // if

if (CropCt > 2)
	return (1);
else if (CropCt > 0)
	{
	StashVtx[0] = Vtx[0];
	StashVtx[1] = Vtx[1];
	StashVtx[2] = Vtx[2];
	if (PolyType != WCS_POLYGONTYPE_3DOBJECT)
		{
		LerpVtxData[0] = new VertexData(); 
		LerpVtxData[1] = new VertexData(); 
		LerpVtx[0] = (VertexBase *)LerpVtxData[0];
		LerpVtx[1] = (VertexBase *)LerpVtxData[1];
		} // if
	else
		{
		LerpVtx3D[0] = new Vertex3D(); 
		LerpVtx3D[1] = new Vertex3D(); 
		LerpVtx[0] = (VertexBase *)LerpVtx3D[0];
		LerpVtx[1] = (VertexBase *)LerpVtx3D[1];
		} // else 3d object
	if (CropCt == 1)
		{
		FirstCropPass = true;
		if (Vtx[0]->ScrnXYZ[2] < 0.001)
			{
			VtxNum0 = 0;
			VtxNum1 = 1;
			VtxNum2 = 2;
			} // if
		else if (Vtx[1]->ScrnXYZ[2] < 0.001)
			{
			VtxNum0 = 1;
			VtxNum1 = 2;
			VtxNum2 = 0;
			} // if
		else
			{
			VtxNum0 = 2;
			VtxNum1 = 0;
			VtxNum2 = 1;
			} // if
		} // if
	else
		{
		if (Vtx[0]->ScrnXYZ[2] > 0.001)
			{
			VtxNum0 = 0;
			VtxNum1 = 1;
			VtxNum2 = 2;
			} // if
		else if (Vtx[1]->ScrnXYZ[2] > 0.001)
			{
			VtxNum0 = 1;
			VtxNum1 = 2;
			VtxNum2 = 0;
			} // if
		else
			{
			VtxNum0 = 2;
			VtxNum1 = 0;
			VtxNum2 = 1;
			} // if
		} // else
	// interpolate a rash of values for first new vertex
	LerpVal = (0.001 - Vtx[VtxNum0]->ScrnXYZ[2]) / (Vtx[VtxNum1]->ScrnXYZ[2] - Vtx[VtxNum0]->ScrnXYZ[2]);
	if (PolyType == WCS_POLYGONTYPE_CLOUD)
		{
		((VertexCloud *)LerpVtx[0])->InterpolateVertexCloud((VertexCloud *)Vtx[VtxNum0], (VertexCloud *)Vtx[VtxNum1], LerpVal);
		} // if
	else if (PolyType != WCS_POLYGONTYPE_3DOBJECT)
		{
		((VertexData *)LerpVtx[0])->InterpolateVertexData((VertexData *)Vtx[VtxNum0], (VertexData *)Vtx[VtxNum1], LerpVal);
		} // if
	else
		{
		((Vertex3D *)LerpVtx[0])->InterpolateVertex3D((Vertex3D *)Vtx[VtxNum0], (Vertex3D *)Vtx[VtxNum1], LerpVal);
		} // else
	Cam->ProjectVertexDEM(DefCoords, (VertexDEM *)LerpVtx[0], EarthLatScaleMeters, PlanetRad, 0);
	if (LerpVtx[0]->ScrnXYZ[2] < 0.001)
		LerpVtx[0]->ScrnXYZ[2] = 0.001;

	// interpolate a rash of values for second new vertex
	LerpVal = (0.001 - Vtx[VtxNum0]->ScrnXYZ[2]) / (Vtx[VtxNum2]->ScrnXYZ[2] - Vtx[VtxNum0]->ScrnXYZ[2]);
	if (PolyType == WCS_POLYGONTYPE_CLOUD)
		{
		((VertexCloud *)LerpVtx[1])->InterpolateVertexCloud((VertexCloud *)Vtx[VtxNum0], (VertexCloud *)Vtx[VtxNum2], LerpVal);
		} // if
	else if (PolyType != WCS_POLYGONTYPE_3DOBJECT)
		{
		((VertexData *)LerpVtx[1])->InterpolateVertexData((VertexData *)Vtx[VtxNum0], (VertexData *)Vtx[VtxNum2], LerpVal);
		} // if
	else
		{
		((Vertex3D *)LerpVtx[1])->InterpolateVertex3D((Vertex3D *)Vtx[VtxNum0], (Vertex3D *)Vtx[VtxNum2], LerpVal);
		} // else
	Cam->ProjectVertexDEM(DefCoords, (VertexDEM *)LerpVtx[1], EarthLatScaleMeters, PlanetRad, 0);
	if (LerpVtx[1]->ScrnXYZ[2] < 0.001)
		LerpVtx[1]->ScrnXYZ[2] = 0.001;

	// set the pointers to render the new polygon
	if (CropCt == 1)
		{
		// first pass
		Vtx[VtxNum0] = LerpVtx[0];
		} // if
	else
		{
		Vtx[VtxNum1] = LerpVtx[0];
		Vtx[VtxNum2] = LerpVtx[1];
		} // else
	} // else if

RepeatCropRender:

// these two tests are complimentary so that a seamless rendering can be compiled
if (Vtx[0]->ScrnXYZ[2] < MinimumZ || Vtx[1]->ScrnXYZ[2] < MinimumZ || Vtx[2]->ScrnXYZ[2] < MinimumZ)
	goto DeleteCropVerts;
if (Vtx[0]->ScrnXYZ[2] > MaximumZ && Vtx[1]->ScrnXYZ[2] > MaximumZ && Vtx[2]->ScrnXYZ[2] > MaximumZ)
	goto DeleteCropVerts;

// do a front-facing test to see if we need to draw the polygon, if not go to trees
// <<<>>>gh

// sort vertices by y screen coord and store in sorted order
if (Vtx[0]->ScrnXYZ[1] <= Vtx[1]->ScrnXYZ[1])
	{
	if (Vtx[0]->ScrnXYZ[1] <= Vtx[2]->ScrnXYZ[1])
		{
		Vert[0] = Vtx[0];
		VertIndex[0] = 0;
		VertNum[0] = Poly->VtxNum[0];
		VertRefData[0] = Poly->VertRefData[0];
		if (Vtx[1]->ScrnXYZ[1] <= Vtx[2]->ScrnXYZ[1])
			{
			Vert[1] = Vtx[1];
			Vert[2] = Vtx[2];
			VertIndex[1] = 1;
			VertIndex[2] = 2;
			VertNum[1] = Poly->VtxNum[1];
			VertNum[2] = Poly->VtxNum[2];
			VertRefData[1] = Poly->VertRefData[1];
			VertRefData[2] = Poly->VertRefData[2];
			} // if 1 <= 2 
		else
			{
			Vert[1] = Vtx[2];
			Vert[2] = Vtx[1];
			VertIndex[1] = 2;
			VertIndex[2] = 1;
			VertNum[1] = Poly->VtxNum[2];
			VertNum[2] = Poly->VtxNum[1];
			VertRefData[1] = Poly->VertRefData[2];
			VertRefData[2] = Poly->VertRefData[1];
			} // else 2 < 1
		} // if 0 <= 2 
	else
		{
		Vert[0] = Vtx[2];
		Vert[1] = Vtx[0];
		Vert[2] = Vtx[1];
		VertIndex[0] = 2;
		VertIndex[1] = 0;
		VertIndex[2] = 1;
		VertNum[0] = Poly->VtxNum[2];
		VertNum[1] = Poly->VtxNum[0];
		VertNum[2] = Poly->VtxNum[1];
		VertRefData[0] = Poly->VertRefData[2];
		VertRefData[1] = Poly->VertRefData[0];
		VertRefData[2] = Poly->VertRefData[1];
		} // else 2 < 0
	} // if 0 <= 1
else
	{
	if (Vtx[1]->ScrnXYZ[1] <= Vtx[2]->ScrnXYZ[1])
		{
		Vert[0] = Vtx[1];
		VertIndex[0] = 1;
		VertNum[0] = Poly->VtxNum[1];
		VertRefData[0] = Poly->VertRefData[1];
		if (Vtx[0]->ScrnXYZ[1] <= Vtx[2]->ScrnXYZ[1])
			{
			Vert[1] = Vtx[0];
			Vert[2] = Vtx[2];
			VertIndex[1] = 0;
			VertIndex[2] = 2;
			VertNum[1] = Poly->VtxNum[0];
			VertNum[2] = Poly->VtxNum[2];
			VertRefData[1] = Poly->VertRefData[0];
			VertRefData[2] = Poly->VertRefData[2];
			} // if 0 <= 2 
		else
			{
			Vert[1] = Vtx[2];
			Vert[2] = Vtx[0];
			VertIndex[1] = 2;
			VertIndex[2] = 0;
			VertNum[1] = Poly->VtxNum[2];
			VertNum[2] = Poly->VtxNum[0];
			VertRefData[1] = Poly->VertRefData[2];
			VertRefData[2] = Poly->VertRefData[0];
			} // else 2 < 0
		} // if 1 <= 2 
	else
		{
		Vert[0] = Vtx[2];
		Vert[1] = Vtx[1];
		Vert[2] = Vtx[0];
		VertIndex[0] = 2;
		VertIndex[1] = 1;
		VertIndex[2] = 0;
		VertNum[0] = Poly->VtxNum[2];
		VertNum[1] = Poly->VtxNum[1];
		VertNum[2] = Poly->VtxNum[0];
		VertRefData[0] = Poly->VertRefData[2];
		VertRefData[1] = Poly->VertRefData[1];
		VertRefData[2] = Poly->VertRefData[0];
		} // else 2 < 0
	} // else 1 < 0

// no rendering of degenerate polygons, either lines or points
if (Vert[0]->ScrnXYZ[1] == Vert[2]->ScrnXYZ[1])
	goto FoliageTime;

Xmin = min(Vert[0]->ScrnXYZ[0], Vert[1]->ScrnXYZ[0]);
Xmin = min(Xmin, Vert[2]->ScrnXYZ[0]);
Xmax = max(Vert[0]->ScrnXYZ[0], Vert[1]->ScrnXYZ[0]);
Xmax = max(Xmax, Vert[2]->ScrnXYZ[0]);

// find bounding box for polygon
// using std routines here since our magic number routines can fail due to numbers over 2 billion
BoxXmin = floor(Xmin);
BoxXmax = ceil(Xmax);
BoxYmin = floor(Vert[0]->ScrnXYZ[1]);
BoxYmax = ceil(Vert[2]->ScrnXYZ[1]);

// cull polygons completely out of the picture
if (BoxYmax <= 0.0 || BoxYmin >= Height || BoxXmax <= 0.0 || BoxXmin >= Width)
	goto FoliageTime;

// clip box size down to image edges
if (BoxYmin < 0.0)
	BoxYmin = 0.0;
if (BoxYmax > Height)
	BoxYmax = Height;
if (BoxXmin < 0.0)
	BoxXmin = 0.0;
if (BoxXmax > Width)
	BoxXmax = Width;

// compute box (pixel array) size needed
// Don't add 1 to each dimension since we will never be able to sample into that 
// extra row or column due to the scan conversion limit rules we shall apply
BoxSize = (BoxWidth = (quickftol(BoxXmax) - quickftol(BoxXmin))) * (BoxHeight = (quickftol(BoxYmax) - quickftol(BoxYmin)));
BoxWidthX10 = BoxWidth * 8;
BoxHeightX10 = BoxHeight * 8;

// compute three slopes, one for each side of the polygon, careful not to divide by 0
// but we already tested that the low and high y values were different.
m1 = (Vert[2]->ScrnXYZ[0] - Vert[0]->ScrnXYZ[0]) / (Vert[2]->ScrnXYZ[1] - Vert[0]->ScrnXYZ[1]);
m2 = Vert[0]->ScrnXYZ[1] != Vert[1]->ScrnXYZ[1] ? (Vert[1]->ScrnXYZ[0] - Vert[0]->ScrnXYZ[0]) / (Vert[1]->ScrnXYZ[1] - Vert[0]->ScrnXYZ[1]): 0.0;
m3 = Vert[1]->ScrnXYZ[1] != Vert[2]->ScrnXYZ[1] ? (Vert[2]->ScrnXYZ[0] - Vert[1]->ScrnXYZ[0]) / (Vert[2]->ScrnXYZ[1] - Vert[1]->ScrnXYZ[1]): 0.0;

// convert poly coords to 10x space referenced from the top left edge of the polygon
// bounding box. Results could be negative since part of the polygon may lie outside
// the image area.
PolyX10[0] = 8.0 * (Vert[0]->ScrnXYZ[0] - BoxXmin);
PolyX10[1] = 8.0 * (Vert[1]->ScrnXYZ[0] - BoxXmin);
PolyX10[2] = 8.0 * (Vert[2]->ScrnXYZ[0] - BoxXmin);
PolyY10[0] = 8.0 * (Vert[0]->ScrnXYZ[1] - BoxYmin);
PolyY10[1] = 8.0 * (Vert[1]->ScrnXYZ[1] - BoxYmin);
PolyY10[2] = 8.0 * (Vert[2]->ScrnXYZ[1] - BoxYmin);

// interpolate X value for slope m1 corresponding to y value of point 1
MidX = (PolyY10[1] - PolyY10[0]) * m1 + PolyX10[0];

// if interpolated point is coincident with point 1 then polygon forms a pure line
// and should not be rendered. Otherwise the sign of the difference will be used
// to determine the appropriate scan order, left to right or right to left.
if (MidX == PolyX10[1])
	goto FoliageTime;

// check PixelWeight array size and reallocate if necessary
if (PixelWeightSize < BoxSize)
	{
	if (PixelWeightSize)
		{
		AppMem_Free(PixelWeight, PixelWeightSize * sizeof (unsigned char));
		AppMem_Free(PixelBitBytes, 8 * PixelWeightSize * sizeof (unsigned char));
		AppMem_Free(VertexWeight[0], PixelWeightSize * sizeof (double));
		AppMem_Free(VertexWeight[1], PixelWeightSize * sizeof (double));
		AppMem_Free(VertexWeight[2], PixelWeightSize * sizeof (double));
		} // if
	VertexWeight[0] = (double *)AppMem_Alloc(BoxSize * sizeof (double), 0);
	VertexWeight[1] = (double *)AppMem_Alloc(BoxSize * sizeof (double), 0);
	VertexWeight[2] = (double *)AppMem_Alloc(BoxSize * sizeof (double), 0);
	PixelWeight = (unsigned char *)AppMem_Alloc(BoxSize * sizeof (unsigned char), 0);
	PixelBitBytes = (unsigned char *)AppMem_Alloc(8 * BoxSize * sizeof (unsigned char), 0);
	if (PixelWeight && PixelBitBytes && VertexWeight[0] && VertexWeight[1] && VertexWeight[2])
		{
		PixelWeightSize = BoxSize;
		} // if
	else
		{
		if (PixelWeight)
			AppMem_Free(PixelWeight, BoxSize * sizeof (unsigned char));
		if (PixelBitBytes)
			AppMem_Free(PixelBitBytes, 8 * BoxSize * sizeof (unsigned char));
		if (VertexWeight[0])
			AppMem_Free(VertexWeight[0], BoxSize * sizeof (double));
		if (VertexWeight[1])
			AppMem_Free(VertexWeight[1], BoxSize * sizeof (double));
		if (VertexWeight[2])
			AppMem_Free(VertexWeight[2], BoxSize * sizeof (double));
		PixelWeightSize = 0;
		return (WCS_ERROR);
		} // else
	} // if


// clear the required amount of the pixel and vertex weight arrays
memset(VertexWeight[0], 0, BoxSize * sizeof (double));
memset(VertexWeight[1], 0, BoxSize * sizeof (double));
memset(VertexWeight[2], 0, BoxSize * sizeof (double));
memset(PixelWeight, 0, BoxSize * sizeof (unsigned char));
memset(PixelBitBytes, 0, 8 * BoxSize * sizeof (unsigned char));

// scan upper part of polygon
if (Vert[1]->ScrnXYZ[1] > Vert[0]->ScrnXYZ[1])
	{
	// find the first row that can have data in it, ie at least 50% covered in y direction
	// then increment the sample point to the pixel row midpt.

	y = WCS_round(PolyY10[0]);
	// keep it within bounding box
	if (y > 0.0)
		y += .5;
	else
		y = .5;
	YPixelTimesEight = (unsigned long int)y;
	// find the last row that is at least 50% covered in y direction
	ScanYMax = PolyY10[1];
	// keep it within bounding box
	if (ScanYMax > BoxHeightX10)
		ScanYMax = BoxHeightX10;

	// by determining the edge ordering in a consistent manner such that scanning always 
	// proceeds from edge1 to edge2, subpixels that fall between the edges but are outside 
	// the polygon, for instance beyond the top and bottom vertices, will automatically be
	// elliminated by the "for x" loop.
	if (MidX < PolyX10[1])
		{
		Edge1Slope = m1;
		Edge2Slope = m2;
		} // if
	else
		{
		Edge1Slope = m2;
		Edge2Slope = m1;
		} // else
	Edge1 = (y - PolyY10[0]) * Edge1Slope + PolyX10[0];
	Edge2 = (y - PolyY10[0]) * Edge2Slope + PolyX10[0];

	WY2Inc = 1.0 / (PolyY10[1] - PolyY10[0]);
	WY1Inc = 1.0 / (PolyY10[2] - PolyY10[0]);

	yp = y - PolyY10[0];
	WY2 = yp * WY2Inc;
	WY1 = yp * WY1Inc;
	W2 = 1.0 - WY2;
	W1 = 1.0 - WY1;

	X2 = yp * m2 + PolyX10[0];
	X1 = yp * m1 + PolyX10[0];

	YBitByte = quickftol(y) * BoxWidth;
	for ( ; y <= ScanYMax; y += 1.0, YPixelTimesEight++, YBitByte += BoxWidth, Edge1 += Edge1Slope, Edge2 += Edge2Slope)
		{
		double InvX1minusX2;
		//bool X1NotEqualX2;
		
		//X1NotEqualX2 = (X1 != X2);

		if (X1 != X2)
			{
			InvX1minusX2 = 1.0 / (X1 - X2);
			} // if
		// this will be constant for all elements of the current row
		RowPixels = (YPixelTimesEight >> 3) * BoxWidth; // y and hence YPixel are always positive so int truncation is safe

		// If it is determined from the edge values and the slopes of the
		// edges that a span of so many pixels is completely covered by the polygon
		// then a much faster method can be used to fill the array for that span.
		// This should result in significant speedup for large polygons and less for
		// small polygons that don't have significant fully covered spans. keep in mind that 
		// we are operating here in 10x space so even an original polygon width of 1
		// will have a significant solid span in some rows.
		// Remember that Edge1 is always on the left side of the polygon.

		// find the first column that can have data in it, ie at least 50% covered in x direction
		// increment the sample point to the pixel column midpt
		x = WCS_round(Edge1);
		// keep it within bounding box
		if (x > 0.0)
			x += .5;
		else
			x = .5;
		XPixelTimesEight = (unsigned long)quickftol(x);
		// find the last row that is at least 50% covered in x direction
		ScanXMax = Edge2;
		// keep it within bounding box
		if (ScanXMax > BoxWidthX10)
			ScanXMax = BoxWidthX10;

		XBitByte = XPixelTimesEight >> 3;
		XBitCt = quickftol(x) - 8 * XBitByte;
		for ( ; x <= ScanXMax; x += 1.0, XPixelTimesEight++, XBitCt ++)
			{
			PixZip = RowPixels + (XPixelTimesEight >> 3);
			PixelWeight[PixZip] ++; // x and hence XPixel are always positive so int truncation is safe
			if (XBitCt >= 8)
				{
				XBitCt = 0;
				XBitByte ++;
				} // if
			BitByte = XBitByte + YBitByte;
			PixelBitBytes[BitByte] |= (1 << (7 - XBitCt));
			
			if (X1 != X2)
				{
				WX = (x - X2) * InvX1minusX2;
				VertexWeight[0][PixZip] += WX * (W1 - W2) + W2;
				VertexWeight[1][PixZip] += WY2 * (1.0 - WX);
				VertexWeight[2][PixZip] += WY1 * WX;
				} // if
			else
				{
				//WX = 0.0;
				VertexWeight[0][PixZip] += W2;
				VertexWeight[1][PixZip] += WY2;
				//VertexWeight[2][PixZip] += WY1 * WX;
				} // else

			} // for
		WY2 += WY2Inc;
		WY1 += WY1Inc;
		W2 -= WY2Inc;
		W1 -= WY1Inc;
		X2 += m2;
		X1 += m1;
		} // for
	} // if

// scan lower part of polygon
// comments same as for upper part
if (Vert[2]->ScrnXYZ[1] > Vert[1]->ScrnXYZ[1])
	{
	y = WCS_round(PolyY10[1]);
	if (y > 0.0)
		y += .5;
	else
		y = .5;
	YPixelTimesEight = (unsigned long)quickftol(y);
	ScanYMax = PolyY10[2];
	if (ScanYMax > BoxHeightX10)
		ScanYMax = BoxHeightX10;

	if (MidX < PolyX10[1])
		{
		Edge1Slope = m1;
		Edge2Slope = m3;
		} // if
	else
		{
		Edge1Slope = m3;
		Edge2Slope = m1;
		} // else
	Edge1 = (y - PolyY10[2]) * Edge1Slope + PolyX10[2];
	Edge2 = (y - PolyY10[2]) * Edge2Slope + PolyX10[2];

	WY2Inc = 1.0 / (PolyY10[1] - PolyY10[2]);
	WY1Inc = 1.0 / (PolyY10[0] - PolyY10[2]);

	yp = y - PolyY10[2];
	WY2 = yp * WY2Inc;
	WY1 = yp * WY1Inc;
	W2 = 1.0 - WY2;
	W1 = 1.0 - WY1;

	X2 = yp * m3 + PolyX10[2];
	X1 = yp * m1 + PolyX10[2];

	YBitByte = quickftol(y) * BoxWidth;
	for ( ; y <= ScanYMax; y += 1.0, YPixelTimesEight++, YBitByte += BoxWidth, Edge1 += Edge1Slope, Edge2 += Edge2Slope)
		{
		double InvX1minusX2;
		//bool X1NotEqualX2;
		
		//X1NotEqualX2 = (X1 != X2);

		if (X1 != X2)
			{
			InvX1minusX2 = 1.0 / (X1 - X2);
			} // if

		x = WCS_round(Edge1);
		if (x > 0.0)
			x += .5;
		else
			x = .5;
		XPixelTimesEight = (unsigned long int)quickftol(x);
		ScanXMax = Edge2;
		if (ScanXMax > BoxWidthX10)
			ScanXMax = BoxWidthX10;
		RowPixels = (YPixelTimesEight >> 3) * BoxWidth;

		XBitByte = (XPixelTimesEight >> 3);
		XBitCt = quickftol(x) - 8 * XBitByte;
		for ( ; x <= ScanXMax; x += 1.0, XPixelTimesEight++, XBitCt ++)
			{
			PixZip = RowPixels + (XPixelTimesEight >> 3);
			PixelWeight[PixZip] ++;
			if (XBitCt >= 8)
				{
				XBitCt = 0;
				XBitByte ++;
				} // if
			BitByte = XBitByte + YBitByte;
			PixelBitBytes[BitByte] |= (1 << (7 - XBitCt));

			if (X1 != X2)
				{
				WX = (x - X2) * InvX1minusX2;
				VertexWeight[2][PixZip] += WX * (W1 - W2) + W2;
				VertexWeight[1][PixZip] += WY2 * (1.0 - WX);
				VertexWeight[0][PixZip] += WY1 * WX;
				} // if
			else
				{
				//WX = 0.0;
				// simplify out the WX=0.0 term
				VertexWeight[2][PixZip] += W2;
				VertexWeight[1][PixZip] += WY2;
				//VertexWeight[0][PixZip] += WY1 * WX;
				} // else
			} // for
		WY2 += WY2Inc;
		WY1 += WY1Inc;
		W2 -= WY2Inc;
		W1 -= WY1Inc;
		X2 += m3;
		X1 += m1;
		} // for
	} // if

// now we have an array of pixel weights which can be applied to the bitmaps
// by offsetting it BoxXmin and BoxYmin pixels to the right and down respectively
// in the destination image.

MaxX = quickftol(BoxXmin) + BoxWidth;
MaxY = quickftol(BoxYmin) + BoxHeight;

if (! ShadowMapInProgress)
	{
	// normalize vertex weights for the whole weight array
	if (! CamPlanOrtho)
		{
		for (WtZip = 0; WtZip < BoxSize; WtZip ++)
			{
			if (PixelWeight[WtZip] > 1)
				{
				VertexWeight[0][WtZip] /= (PixelWeight[WtZip] * Vert[0]->ScrnXYZ[2]);
				VertexWeight[1][WtZip] /= (PixelWeight[WtZip] * Vert[1]->ScrnXYZ[2]);
				VertexWeight[2][WtZip] /= (PixelWeight[WtZip] * Vert[2]->ScrnXYZ[2]);
				} // if
			else if (PixelWeight[WtZip])
				{
				VertexWeight[0][WtZip] /= (Vert[0]->ScrnXYZ[2]);
				VertexWeight[1][WtZip] /= (Vert[1]->ScrnXYZ[2]);
				VertexWeight[2][WtZip] /= (Vert[2]->ScrnXYZ[2]);
				} // else
			} // for
		} // if
	else
		{
		for (WtZip = 0; WtZip < BoxSize; WtZip ++)
			{
			if (PixelWeight[WtZip] > 1)
				{
				VertexWeight[0][WtZip] /= PixelWeight[WtZip];
				VertexWeight[1][WtZip] /= PixelWeight[WtZip];
				VertexWeight[2][WtZip] /= PixelWeight[WtZip];
				} // if
			} // for
		} // else
	// determine if textures exist for any of the materials about to be rendered
	if (LightTexturesExist || AtmoTexturesExist)
		TexturesExist = true;
	else if (PolyType == WCS_POLYGONTYPE_TERRAIN)
		{
		#ifdef WCS_VECPOLY_EFFECTS
		for (MatCount = 0; EcoMatTable[MatCount].Material; ++MatCount)
			{
			if (EcoMatTable[MatCount].Material->OverstoryDissolve > 0.0)
				OverDissolveExists = true;
			if (EcoMatTable[MatCount].Material->UnderstoryDissolve > 0.0)
				UnderDissolveExists = true;
			if (EcoMatTable[MatCount].Material->PixelTexturesExist)
				TexturesExist = true;
			else if (EcoMatTable[MatCount].Material->OverstoryDissolve > 0.0 && EcoMatTable[MatCount].Material->EcoFol[0]->PixelTexturesExist)
				TexturesExist = true;
			else if (EcoMatTable[MatCount].Material->UnderstoryDissolve > 0.0 && EcoMatTable[MatCount].Material->EcoFol[1]->PixelTexturesExist)
				TexturesExist = true;
			} // for
		if (! TexturesExist)
			{
			for (MatCount = 0; GroundMatTable[MatCount].Material; ++MatCount)
				{
				if (GroundMatTable[MatCount].Material->PixelTexturesExist)
					{
					TexturesExist = true;
					break;
					} // if
				} // for
			if (! TexturesExist)
				{
				for (MatCount = 0; SnowMatTable[MatCount].Material; ++MatCount)
					{
					if (SnowMatTable[MatCount].Material->PixelTexturesExist)
						{
						TexturesExist = true;
						break;
						} // if
					} // for
				} // if
			} // if
		#else // WCS_VECPOLY_EFFECTS
		if (Poly->BaseMat.Mat[0] && Poly->BaseMat.Mat[0]->PixelTexturesExist)
			TexturesExist = true;
		else if (Poly->BaseMat.Mat[1] && Poly->BaseMat.Mat[1]->PixelTexturesExist)
			TexturesExist = true;
		else if (Poly->GroundMat.Mat[0] && Poly->GroundMat.Mat[0]->PixelTexturesExist)
			TexturesExist = true;
		else if (Poly->GroundMat.Mat[1] && Poly->GroundMat.Mat[1]->PixelTexturesExist)
			TexturesExist = true;
		else if (Poly->SnowCoverage > 0.0 && Poly->SnowMat.Mat[0]->PixelTexturesExist)
			TexturesExist = true;
		else if (Poly->SnowCoverage > 0.0 && Poly->SnowMat.Mat[1] && Poly->SnowMat.Mat[1]->PixelTexturesExist)
			TexturesExist = true;
		else if (Poly->OverstoryDissolve[0] > 0.0 && Poly->BaseMat.Mat[0]->EcoFol[0]->PixelTexturesExist)
			TexturesExist = true;
		else if (Poly->OverstoryDissolve[1] > 0.0 && Poly->BaseMat.Mat[1]->EcoFol[0]->PixelTexturesExist)
			TexturesExist = true;
		else if (Poly->UnderstoryDissolve[0] > 0.0 && Poly->BaseMat.Mat[0]->EcoFol[1]->PixelTexturesExist)
			TexturesExist = true;
		else if (Poly->UnderstoryDissolve[1] > 0.0 && Poly->BaseMat.Mat[1]->EcoFol[1]->PixelTexturesExist)
			TexturesExist = true;
		#endif // WCS_VECPOLY_EFFECTS
		} // if terrain
	else if (PolyType == WCS_POLYGONTYPE_WATER)
		{
		#ifdef WCS_VECPOLY_EFFECTS
		for (MatCount = 0; WaterMatTableStart[MatCount].Material && (&WaterMatTableStart[MatCount] != WaterMatTableEnd); ++MatCount)
			{
			if (WaterMatTableStart[MatCount].Material->PixelTexturesExist)
				{
				TexturesExist = true;
				break;
				} // if
			} // for
		#else // WCS_VECPOLY_EFFECTS
		if (Poly->BaseMat.Mat[0] && Poly->BaseMat.Mat[0]->PixelTexturesExist)
			TexturesExist = 1;
		else if (Poly->BaseMat.Mat[1] && Poly->BaseMat.Mat[1]->PixelTexturesExist)
			TexturesExist = 1;
		#endif // WCS_VECPOLY_EFFECTS
		} // else if water
	else if (PolyType == WCS_POLYGONTYPE_FENCE)
		{
		if (Poly->GroundMat.Mat[0] && Poly->GroundMat.Mat[0]->PixelTexturesExist)
			TexturesExist = 1;
		else if (Poly->GroundMat.Mat[1] && Poly->GroundMat.Mat[1]->PixelTexturesExist)
			TexturesExist = 1;
		} // else if fence
	else if (PolyType == WCS_POLYGONTYPE_CLOUD)
		{
		if (Poly->Cloud && Poly->Cloud->PixelTexturesExist)	// clouds use textures to control density and shade detail
			TexturesExist = 1;
		} // else if cloud
	else	// 3d object we presume
		{
		if (Poly->BaseMat.Mat[0] && Poly->BaseMat.Mat[0]->PixelTexturesExist)
			TexturesExist = 1;
		} // else 3D Object
	
	if (PolyType == WCS_POLYGONTYPE_3DOBJECT)
		{
		FragMap3D = Poly->Plot3DRast ? Poly->Plot3DRast->rPixelBlock: NULL;
		UseFragMap = true;
		} // if 3D Object
	else
		{
		UseFragMap = rPixelFragMap ? true: false;
		} // else

	// set up texture data
	Pix.TexData = &RendData.TexData;		// TexData is RenderData scope so it doesn't need to be constructed for each polygon.
								// It doesn't need to be cleared.
	Pix.TexData->Vec = Poly->Vector;
	Pix.TexData->VectorEffectType = Poly->VectorType;
	Pix.TexData->VectorSlope = Poly->VectorSlope;
	Pix.ShadowFlags = Poly->ShadowFlags;
	Pix.ShadowOffset = Poly->ShadowOffset;
	Pix.ReceivedShadowIntensity = Poly->ReceivedShadowIntensity;
	Pix.Object = Pix.TexData->Object = Poly->Object;
	Pix.TexData->Object3D = NULL;
	Pix.TexData->VtxNum[0] = VertNum[0];
	Pix.TexData->VtxNum[1] = VertNum[1];
	Pix.TexData->VtxNum[2] = VertNum[2];
	Pix.TexData->VertRefData[0] = VertRefData[0];
	Pix.TexData->VertRefData[1] = VertRefData[1];
	Pix.TexData->VertRefData[2] = VertRefData[2];
	Pix.TexData->PData = Poly;
	Pix.TexData->Slope = Poly->Slope;
	Pix.TexData->Aspect = Poly->Aspect;
	Pix.TexData->PData = NULL;
	Pix.TexData->VDEM[0] = Pix.TexData->VDEM[1] = Pix.TexData->VDEM[2] = NULL;
	Pix.TexData->VData[0] = Pix.TexData->VData[1] = Pix.TexData->VData[2] = NULL;

	// for terrain and water the surface normal is constant over the polygon prior to VNS 3 and constant in VNS 3
	// for all non-phong-shaded surfaces.
	Pix.Normal[0] = Poly->Normal[0];
	Pix.Normal[1] = Poly->Normal[1];
	Pix.Normal[2] = Poly->Normal[2];

	for (WtZip = 0, Y = quickftol(BoxYmin), Yp = 0, y = .5; Y < MaxY; Y ++, Yp ++, y += 1.0)
		{
		PixZip = Y * Width + quickftol(BoxXmin);

		// this could be sped up a bit by calculating the extrema edge values (max or min of the 
		// edges at the top and bottom of the pixel) and scanning only that x range.
		// Another tricky technique would be to calculate the pixel center's edge value and adding or
		// subtracting .5 x slope of the edge. The center edge values might be necessary anyway
		// for interpolating other values across the polygon (such as Z and elevation...).
		for (X = quickftol(BoxXmin), Xp = 0, x = .5; X < MaxX; X ++, Xp ++, x += 1.0, WtZip ++, PixZip ++)
			{
			if (UseFragMap)
				{
				YBitByte = Xp + Yp * 8 * BoxWidth;
				TopPixCovg = (PixelBitBytes[YBitByte] << 24) | (PixelBitBytes[YBitByte + BoxWidth] << 16) |
					(PixelBitBytes[YBitByte + 2 * BoxWidth] << 8) |
					(PixelBitBytes[YBitByte + 3 * BoxWidth]);
				BotPixCovg = (PixelBitBytes[YBitByte + 4 * BoxWidth] << 24) | (PixelBitBytes[YBitByte + 5 * BoxWidth] << 16) |
					(PixelBitBytes[YBitByte + 6 * BoxWidth] << 8) |
					(PixelBitBytes[YBitByte + 7 * BoxWidth]);
				NewWt = 255.99;
				} // if
			else
				{
				NewWt = PixelWeight[WtZip] < 64 ? PixelWeight[WtZip] * 3.9999: 255.99;
				} // else
			if ((UseFragMap && (TopPixCovg || BotPixCovg)) || (! UseFragMap && NewWt))
				{
				VtxPct[0] = VertexWeight[0][WtZip];
				VtxPct[1] = VertexWeight[1][WtZip];
				VtxPct[2] = VertexWeight[2][WtZip];
				if (! CamPlanOrtho)
					{
					// find the Z value for the pixel - we are interpolating 1/Z
					ZTranslator = (VtxPct[0] + VtxPct[1] + VtxPct[2]);
					// restore to real Z
					ZTranslator = 1.0 / ZTranslator;
					Pix.Zflt = (float)ZTranslator;
					} // if
				else
					{
					// find the Z value for the pixel - we are interpolating Z
					Pix.Zflt = (float)(VtxPct[0] * Vert[0]->ScrnXYZ[2] + VtxPct[1] * Vert[1]->ScrnXYZ[2] + VtxPct[2] * Vert[2]->ScrnXYZ[2]);
					// no need to restore to real Z
					ZTranslator = 1.0;
					} // else

				// test z value against z buffer and antialias buffer
				if ((rPixelFragMap/* && rPixelFragMap[PixZip].TestPossibleUsage(Pix.Zflt, TopPixCovg, BotPixCovg, 0)*/) 
					|| ((Pix.Zflt <= TreeZBuf[PixZip] || TreeAABuf[PixZip] < 255) && (Pix.Zflt <= ZBuf[PixZip] || AABuf[PixZip] < 255)))
					{
					// set current pixel used opacity, color, luminosity, reflectivity, specularity... to 0
					Pix.SetDefaults();

					// these values will most certainly be used by either materials or lighting/atmosphere
					// ZTranslator is 1 / (VtxPct[0] + [1] + [2]) so VtxWt[] are normalized versions which add to 1
					VtxWt[0] = ZTranslator * VtxPct[0];
					VtxWt[1] = ZTranslator * VtxPct[1];
					VtxWt[2] = ZTranslator * VtxPct[2];
					Pix.XYZ[0] = VtxWt[0] * Vert[0]->XYZ[0] + VtxWt[1] * Vert[1]->XYZ[0] + VtxWt[2] * Vert[2]->XYZ[0];
					Pix.XYZ[1] = VtxWt[0] * Vert[0]->XYZ[1] + VtxWt[1] * Vert[1]->XYZ[1] + VtxWt[2] * Vert[2]->XYZ[1];
					Pix.XYZ[2] = VtxWt[0] * Vert[0]->XYZ[2] + VtxWt[1] * Vert[1]->XYZ[2] + VtxWt[2] * Vert[2]->XYZ[2];
					Pix.Q = VtxWt[0] * Vert[0]->Q + VtxWt[1] * Vert[1]->Q + VtxWt[2] * Vert[2]->Q;
					Pix.Elev = VtxWt[0] * ((VertexDEM *)Vert[0])->Elev + VtxWt[1] * ((VertexDEM *)Vert[1])->Elev + VtxWt[2] * ((VertexDEM *)Vert[2])->Elev;
					Pix.Lat = VtxWt[0] * ((VertexDEM *)Vert[0])->Lat + VtxWt[1] * ((VertexDEM *)Vert[1])->Lat + VtxWt[2] * ((VertexDEM *)Vert[2])->Lat;
					Pix.Lon = VtxWt[0] * ((VertexDEM *)Vert[0])->Lon + VtxWt[1] * ((VertexDEM *)Vert[1])->Lon + VtxWt[2] * ((VertexDEM *)Vert[2])->Lon;
					Pix.PixZip = PixZip;
					// view vector is used for textures and lighting
					// for specularity it is best to have it point toward the camera
					// for texture dynamic param need pointing towards camera as well
					Pix.TexData->CamAimVec[0] = Pix.ViewVec[0] = Cam->CamPos->XYZ[0] - Pix.XYZ[0];
					Pix.TexData->CamAimVec[1] = Pix.ViewVec[1] = Cam->CamPos->XYZ[1] - Pix.XYZ[1];
					Pix.TexData->CamAimVec[2] = Pix.ViewVec[2] = Cam->CamPos->XYZ[2] - Pix.XYZ[2];
					UnitVector(Pix.ViewVec); // Illumination code requires this to be unitized in advance to save time later

					// some data is only needed if texturing is applied
					if (TexturesExist)
						{
						// fill in texture data, at least some of it's gonna get used

						// generic items
						Pix.TexData->VtxWt[0] = VtxWt[0];
						Pix.TexData->VtxWt[1] = VtxWt[1];
						Pix.TexData->VtxWt[2] = VtxWt[2];
						Pix.TexData->ZDist = Pix.Zflt;
						Pix.TexData->QDist = Pix.Q;
						Pix.TexData->PixelX[0] = (double)X + SegmentOffsetX - Width * Cam->PanoPanels * .5;
						Pix.TexData->PixelX[1] = (double)(X + 1) + SegmentOffsetX - Width * Cam->PanoPanels * .5;
						Pix.TexData->PixelY[0] = -((double)Y + SegmentOffsetY) + Height * Opt->RenderImageSegments * .5;
						Pix.TexData->PixelY[1] = -((double)(Y + 1) + SegmentOffsetY) + Height * Opt->RenderImageSegments * .5;
						Pix.TexData->PixelUnityX[0] = Pix.TexData->PixelX[0] / (Width * Cam->PanoPanels);
						Pix.TexData->PixelUnityX[1] = Pix.TexData->PixelX[1] / (Width * Cam->PanoPanels);
						Pix.TexData->PixelUnityY[0] = Pix.TexData->PixelY[0] / (Height * Opt->RenderImageSegments);
						Pix.TexData->PixelUnityY[1] = Pix.TexData->PixelY[1] / (Height * Opt->RenderImageSegments);
						Pix.TexData->Elev = Pix.Elev;
						Pix.TexData->Latitude = Pix.Lat;
						Pix.TexData->Longitude = Pix.Lon;
						// initialize so it can be calculated only once by textures if needed
						Pix.TexData->VectorOffsetsComputed = 0;

						// items specific to type of polygon
						if (PolyType != WCS_POLYGONTYPE_3DOBJECT)	// either water or terrain or cloud uses same variables
							{
							// this function fills in a large amount texture data
							FillTerrainTextureData(Pix.TexData, (VertexData **)Vert, VtxPct, BoxWidth, BoxHeight, 
								WtZip, Xp, Yp, PolyType, Poly->Cloud);
							} // if terrain or water
						else	// 3D object
							{
							// this function fills in a large amount texture data
							Fill3DObjectTextureData(Pix.TexData, (Vertex3D **)Vert, VtxPct, BoxWidth, BoxHeight, 
								WtZip, Xp, Yp);
							} // else 3d object
						} // if 

					// obtain color from material(s)
						// need interpolated values at each corner of pixel
						// pass an object to material evaluator containing ranges of values for the pixel
					if (PolyType == WCS_POLYGONTYPE_TERRAIN)
						{
						Pix.TexData->ObjectType = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_TERRAIN;
						Pix.PixelType = WCS_PIXELTYPE_TERRAIN;
						
						#ifdef WCS_VECPOLY_EFFECTS
						if (Poly->ShadeType == WCS_EFFECT_MATERIAL_SHADING_PHONG)
							{
							Pix.Normal[0] = VtxWt[0] * ((VertexData *)Vert[0])->NodeData->Normal[0] + VtxWt[1] * ((VertexData *)Vert[1])->NodeData->Normal[0] + VtxWt[2] * ((VertexData *)Vert[2])->NodeData->Normal[0];
							Pix.Normal[1] = VtxWt[0] * ((VertexData *)Vert[0])->NodeData->Normal[1] + VtxWt[1] * ((VertexData *)Vert[1])->NodeData->Normal[1] + VtxWt[2] * ((VertexData *)Vert[2])->NodeData->Normal[1];
							Pix.Normal[2] = VtxWt[0] * ((VertexData *)Vert[0])->NodeData->Normal[2] + VtxWt[1] * ((VertexData *)Vert[1])->NodeData->Normal[2] + VtxWt[2] * ((VertexData *)Vert[2])->NodeData->Normal[2];
							} // if
						Pix.TexData->Slope = VtxWt[0] * ((VertexData *)Vert[0])->NodeData->Slope + VtxWt[1] * ((VertexData *)Vert[1])->NodeData->Slope + VtxWt[2] * ((VertexData *)Vert[2])->NodeData->Slope;
						if ((fabs(((VertexData *)Vert[0])->NodeData->Aspect - 
							((VertexData *)Vert[1])->NodeData->Aspect) > 180.0)
							||
							(fabs(((VertexData *)Vert[0])->NodeData->Aspect - 
							((VertexData *)Vert[2])->NodeData->Aspect) > 180.0)
							||
							(fabs(((VertexData *)Vert[1])->NodeData->Aspect - 
							((VertexData *)Vert[2])->NodeData->Aspect) > 180.0))
							{
							double TempAspect[3];
							TempAspect[0] = ((VertexData *)Vert[0])->NodeData->Aspect < 180.0 ?
								((VertexData *)Vert[0])->NodeData->Aspect + 360.0: ((VertexData *)Vert[0])->NodeData->Aspect;
							TempAspect[1] = ((VertexData *)Vert[1])->NodeData->Aspect < 180.0 ?
								((VertexData *)Vert[1])->NodeData->Aspect + 360.0: ((VertexData *)Vert[1])->NodeData->Aspect;
							TempAspect[2] = ((VertexData *)Vert[2])->NodeData->Aspect < 180.0 ?
								((VertexData *)Vert[2])->NodeData->Aspect + 360.0: ((VertexData *)Vert[2])->NodeData->Aspect;
							Pix.TexData->Aspect = VtxWt[0] * TempAspect[0] + VtxWt[1] * TempAspect[1] + VtxWt[2] * TempAspect[2];
							if (Pix.TexData->Aspect >= 360.0)
								Pix.TexData->Aspect -= 360.0;
							} // if
						else
							Pix.TexData->Aspect = VtxWt[0] * ((VertexData *)Vert[0])->NodeData->Aspect + VtxWt[1] * ((VertexData *)Vert[1])->NodeData->Aspect + VtxWt[2] * ((VertexData *)Vert[2])->NodeData->Aspect;
						Pix.TexData->RelElev = (float)(VtxWt[0] * ((VertexData *)Vert[0])->NodeData->RelEl + VtxWt[1] * ((VertexData *)Vert[1])->NodeData->RelEl + VtxWt[2] * ((VertexData *)Vert[2])->NodeData->RelEl);
						#else // WCS_VECPOLY_EFFECTS
						Pix.TexData->RelElev = (float)(VtxWt[0] * ((VertexData *)Vert[0])->RelEl + VtxWt[1] * ((VertexData *)Vert[1])->RelEl + VtxWt[2] * ((VertexData *)Vert[2])->RelEl);
						#endif // WCS_VECPOLY_EFFECTS
						Pix.TexData->WaveHeight = 0.0;
						#ifdef WCS_VECPOLY_EFFECTS
						if (EcoMatTable[0].Material)
							{
							long MatsValid = 0;
							MatWt[0] = MatWt[1] = MatWt[2] = 0.0;
							if (EcoMatTable[0].MatListPtr[VertIndex[0]] && EcoMatTable[0].MatListPtr[VertIndex[0]]->WaterProp)
								{
								MatWt[0] = VtxWt[0] * EcoMatTable[0].MatListPtr[VertIndex[0]]->WaterProp->WaterDepth;
								++MatsValid;
								} // if
							if (EcoMatTable[0].MatListPtr[VertIndex[1]] && EcoMatTable[0].MatListPtr[VertIndex[1]]->WaterProp)
								{
								MatWt[1] = VtxWt[1] * EcoMatTable[0].MatListPtr[VertIndex[1]]->WaterProp->WaterDepth;
								++MatsValid;
								} // if
							if (EcoMatTable[0].MatListPtr[VertIndex[2]] && EcoMatTable[0].MatListPtr[VertIndex[2]]->WaterProp)
								{
								MatWt[2] = VtxWt[2] * EcoMatTable[0].MatListPtr[VertIndex[2]]->WaterProp->WaterDepth;
								++MatsValid;
								} // if
							if (MatsValid)
								Pix.TexData->WaterDepth = Pix.WaterDepth = (MatWt[0] + MatWt[1] + MatWt[2]) * (3.0 / MatsValid);
							else
								Pix.TexData->WaterDepth = Pix.WaterDepth = 0.0;
							} // if
						else
							Pix.TexData->WaterDepth = Pix.WaterDepth = 0.0;
						#else // WCS_VECPOLY_EFFECTS
						Pix.TexData->WaterDepth = Pix.WaterDepth = VtxWt[0] * ((VertexData *)Vert[0])->WaterDepth + VtxWt[1] * ((VertexData *)Vert[1])->WaterDepth + VtxWt[2] * ((VertexData *)Vert[2])->WaterDepth;
						#endif // WCS_VECPOLY_EFFECTS
						
						// stash normals in case bump mapping changes them
						NormalStash[0] = Pix.Normal[0];
						NormalStash[1] = Pix.Normal[1];
						NormalStash[2] = Pix.Normal[2];
						SlopeStash = Pix.TexData->Slope;
						AspectStash = Pix.TexData->Aspect;

						// need to blend together foliageblend colors, snow, ecosystem, ground, beach
						// evaluate in this order: overstory dissolve, snow, ecosystem (incl. understory dissolve), ground.
						// replace ecosystem with beach material if eco is null but lake is valid
						
					// Overstory Dissolve
						#ifdef WCS_VECPOLY_EFFECTS
						if (OverDissolveExists && (MatToRender = EcoMatTable[0].Material))
							{
							OpacityStash = Pix.OpacityUsed;
							CovgRemaining = 1.0 - Pix.OpacityUsed;
							DissolveOpacity = 0.0;
							for (MatCount = 0; MatToRender; MatToRender = EcoMatTable[MatCount].Material)
								{
								MatWt[0] = EcoMatTable[MatCount].MatListPtr[VertIndex[0]] ? VtxWt[0] * EcoMatTable[MatCount].MatListPtr[VertIndex[0]]->MatCoverage: 0.0;
								MatWt[1] = EcoMatTable[MatCount].MatListPtr[VertIndex[1]] ? VtxWt[1] * EcoMatTable[MatCount].MatListPtr[VertIndex[1]]->MatCoverage: 0.0;
								MatWt[2] = EcoMatTable[MatCount].MatListPtr[VertIndex[2]] ? VtxWt[2] * EcoMatTable[MatCount].MatListPtr[VertIndex[2]]->MatCoverage: 0.0;
								if ((TempCovg = CovgRemaining * (MatWt[0] + MatWt[1] + MatWt[2])) > 0.0)
									{
									// test if there is any dissolve enabled
									if (MatToRender->OverstoryDissolve > 0.0)
										{
										TempCovg *= MatToRender->OverstoryDissolve;
										if (TempCovg + Pix.OpacityUsed > .9999)
											TempCovg = 1.0 - Pix.OpacityUsed;
										Pix.TexData->ExtractMatTableData(&EcoMatTable[MatCount], VertIndex);
										MatToRender->EcoFol[0]->EvaluateDissolve(&Pix, TempCovg);
										// keep track of the amount of opacity actually contributed by overstory dissolve
										DissolveOpacity += TempCovg;
										if (Pix.OpacityUsed >= 1.0)
											break;
										} // if
									else
										Pix.OpacityUsed += TempCovg;
									} // if
								++MatCount;
								} // for
							// actual pixel coverage is the original amount plus the dissolve opacity
							Pix.OpacityUsed = OpacityStash + DissolveOpacity;
							} // if
						#else // WCS_VECPOLY_EFFECTS
						if (Poly->OverstoryDissolve[0] > 0.0)
							{
							Poly->BaseMat.Mat[0]->EcoFol[0]->EvaluateDissolve(&Pix, Poly->OverstoryDissolve[0] * Poly->BaseMat.Covg[0]);
							} // if
						if (Poly->OverstoryDissolve[1] > 0.0)
							{
							Poly->BaseMat.Mat[1]->EcoFol[0]->EvaluateDissolve(&Pix, Poly->OverstoryDissolve[1] * Poly->BaseMat.Covg[1]);
							} // if
						#endif // WCS_VECPOLY_EFFECTS
						
					// Bump maps
						if (Pix.OpacityUsed < 1.0)
							{
							if (TexturesExist)
								#ifdef WCS_VECPOLY_EFFECTS
								InterpretLandBumpMaps(&Pix, Poly, VtxWt, VertIndex);
								#else // WCS_VECPOLY_EFFECTS
								InterpretLandBumpMaps(&Pix, Poly);
								#endif // WCS_VECPOLY_EFFECTS

							// normals may be needed by textures
							Pix.TexData->Normal[0] = Pix.Normal[0];
							Pix.TexData->Normal[1] = Pix.Normal[1];
							Pix.TexData->Normal[2] = Pix.Normal[2];

					// Snow
							// evaluate snow material
							#ifdef WCS_VECPOLY_EFFECTS
							if (MatToRender = SnowMatTable[0].Material)
								{
								CovgRemaining = 1.0 - Pix.OpacityUsed;
								for (MatCount = 0; MatToRender; MatToRender = SnowMatTable[MatCount].Material)
									{
									MatWt[0] = SnowMatTable[MatCount].MatListPtr[VertIndex[0]] ? VtxWt[0] * SnowMatTable[MatCount].MatListPtr[VertIndex[0]]->MatCoverage: 0.0;
									MatWt[1] = SnowMatTable[MatCount].MatListPtr[VertIndex[1]] ? VtxWt[1] * SnowMatTable[MatCount].MatListPtr[VertIndex[1]]->MatCoverage: 0.0;
									MatWt[2] = SnowMatTable[MatCount].MatListPtr[VertIndex[2]] ? VtxWt[2] * SnowMatTable[MatCount].MatListPtr[VertIndex[2]]->MatCoverage: 0.0;
									// Poly->SnowCoverage was set in the bump map interpreter to account for any change in snow coverage
									// resulting from bump mapping
									if ((TempCovg = Poly->SnowCoverage * CovgRemaining * (MatWt[0] + MatWt[1] + MatWt[2])) > 0.0)
										{
										// snow can't go below water so we need to check and see if water is present over this polygon and
										// weight the snow coverage such that it will diminish by the water elevation.
										MatWt[0] = VtxWt[0] * SnowMatTable[MatCount].AuxValue[VertIndex[0]];
										MatWt[1] = VtxWt[1] * SnowMatTable[MatCount].AuxValue[VertIndex[1]];
										MatWt[2] = VtxWt[2] * SnowMatTable[MatCount].AuxValue[VertIndex[2]];
										if ((SnowWaterMultiplier = MatWt[0] + MatWt[1] + MatWt[2]) > 0.0)
											{
											if (SnowWaterMultiplier < 1.0)
												TempCovg *= SnowWaterMultiplier;
											if (TempCovg + Pix.OpacityUsed > .9999)
												TempCovg = 1.0 - Pix.OpacityUsed;
											Pix.TexData->ExtractMatTableData(&SnowMatTable[MatCount], VertIndex);
											MatToRender->Evaluate(&Pix, TempCovg);
											if (Pix.OpacityUsed >= 1.0)
												break;
											} // if
										} // if
									++MatCount;
									} // for
								} // if
							#else // WCS_VECPOLY_EFFECTS
							if (Poly->SnowCoverage > 0.0)
								{
								CovgRemaining = 1.0 - Pix.OpacityUsed;
								TempCovg = Poly->SnowMat.Covg[0] * CovgRemaining;
								Poly->SnowMat.Mat[0]->Evaluate(&Pix, Poly->SnowCoverage * TempCovg);
								if (Poly->SnowMat.Mat[1])
									{
									TempCovg = Poly->SnowMat.Covg[1] * CovgRemaining;
									Poly->SnowMat.Mat[1]->Evaluate(&Pix, Poly->SnowCoverage * TempCovg);
									} // if
								} // if
							#endif // WCS_VECPOLY_EFFECTS
								
						// Understory Dissolve
							if (Pix.OpacityUsed < 1.0)
								{
								#ifdef WCS_VECPOLY_EFFECTS
								if (UnderDissolveExists && (MatToRender = EcoMatTable[0].Material))
									{
									OpacityStash = Pix.OpacityUsed;
									CovgRemaining = 1.0 - Pix.OpacityUsed;
									DissolveOpacity = 0.0;
									for (MatCount = 0; MatToRender; MatToRender = EcoMatTable[MatCount].Material)
										{
										MatWt[0] = EcoMatTable[MatCount].MatListPtr[VertIndex[0]] ? VtxWt[0] * EcoMatTable[MatCount].MatListPtr[VertIndex[0]]->MatCoverage: 0.0;
										MatWt[1] = EcoMatTable[MatCount].MatListPtr[VertIndex[1]] ? VtxWt[1] * EcoMatTable[MatCount].MatListPtr[VertIndex[1]]->MatCoverage: 0.0;
										MatWt[2] = EcoMatTable[MatCount].MatListPtr[VertIndex[2]] ? VtxWt[2] * EcoMatTable[MatCount].MatListPtr[VertIndex[2]]->MatCoverage: 0.0;
										if ((TempCovg = CovgRemaining * (MatWt[0] + MatWt[1] + MatWt[2])) > 0.0)
											{
											// test if there is any dissolve enabled
											if (MatToRender->UnderstoryDissolve > 0.0)
												{
												TempCovg *= MatToRender->UnderstoryDissolve;
												if (TempCovg + Pix.OpacityUsed > .9999)
													TempCovg = 1.0 - Pix.OpacityUsed;
												Pix.TexData->ExtractMatTableData(&EcoMatTable[MatCount], VertIndex);
												MatToRender->EcoFol[1]->EvaluateDissolve(&Pix, TempCovg);
												// keep track of the amount of opacity actually contributed by overstory dissolve
												DissolveOpacity += TempCovg;
												if (Pix.OpacityUsed >= 1.0)
													break;
												} // if
											else
												Pix.OpacityUsed += TempCovg;
											} // if
										++MatCount;
										} // for
									// actual pixel coverage is the original amount plus the dissolve opacity
									Pix.OpacityUsed = OpacityStash + DissolveOpacity;
									} // if
								#else // WCS_VECPOLY_EFFECTS
								if (Poly->UnderstoryDissolve[0] > 0.0)
									{
									CovgRemaining = 1.0 - Pix.OpacityUsed;
									TempCovg = Poly->BaseMat.Covg[0] * CovgRemaining;
									Poly->BaseMat.Mat[0]->EcoFol[1]->EvaluateDissolve(&Pix, Poly->UnderstoryDissolve[0] * TempCovg);
									if (Poly->UnderstoryDissolve[1] > 0.0)
										{
										TempCovg = Poly->BaseMat.Covg[1] * CovgRemaining;
										Poly->BaseMat.Mat[1]->EcoFol[1]->EvaluateDissolve(&Pix, Poly->UnderstoryDissolve[1] * TempCovg);
										} // if
									} // if
								#endif // WCS_VECPOLY_EFFECTS
								
							// Color Map (VNS 2)
								if (Pix.OpacityUsed < 1.0)
									{
									#ifndef WCS_VECPOLY_EFFECTS
									if (Poly->RenderCmapColor)
										{
										Pix.RGB[0] = Poly->RGB[0];
										Pix.RGB[1] = Poly->RGB[1];
										Pix.RGB[2] = Poly->RGB[2];
										if (Poly->LuminousColor)
											Pix.Luminosity += (1.0 - Pix.OpacityUsed);
										Pix.OpacityUsed = 1.0;
										} // if
									else
									#endif // WCS_VECPOLY_EFFECTS
										{
							// Ecosystem, Beach or Color Map (VNS 3)
										#ifdef WCS_VECPOLY_EFFECTS
										if (MatToRender = EcoMatTable[0].Material)
											{
											CovgRemaining = 1.0 - Pix.OpacityUsed;
											for (MatCount = 0; MatToRender; MatToRender = EcoMatTable[MatCount].Material)
												{
												MatWt[0] = EcoMatTable[MatCount].MatListPtr[VertIndex[0]] ? VtxWt[0] * EcoMatTable[MatCount].MatListPtr[VertIndex[0]]->MatCoverage: 0.0;
												MatWt[1] = EcoMatTable[MatCount].MatListPtr[VertIndex[1]] ? VtxWt[1] * EcoMatTable[MatCount].MatListPtr[VertIndex[1]]->MatCoverage: 0.0;
												MatWt[2] = EcoMatTable[MatCount].MatListPtr[VertIndex[2]] ? VtxWt[2] * EcoMatTable[MatCount].MatListPtr[VertIndex[2]]->MatCoverage: 0.0;
												if ((TempCovg = CovgRemaining * (MatWt[0] + MatWt[1] + MatWt[2])) > 0.0)
													{
													if (TempCovg + Pix.OpacityUsed > .9999)
														TempCovg = 1.0 - Pix.OpacityUsed;
													Pix.TexData->ExtractMatTableData(&EcoMatTable[MatCount], VertIndex);
													MatToRender->Evaluate(&Pix, TempCovg);
													if (Pix.OpacityUsed >= 1.0)
														break;
													} // if
												++MatCount;
												} // for
											} // if
										#else // WCS_VECPOLY_EFFECTS
										if (Poly->BaseMat.Mat[0])
											{
											CovgRemaining = 1.0 - Pix.OpacityUsed;
											TempCovg = Poly->BaseMat.Covg[0] * CovgRemaining;
											Poly->BaseMat.Mat[0]->Evaluate(&Pix, TempCovg);
											if (Poly->BaseMat.Mat[1])
												{
												TempCovg = Poly->BaseMat.Covg[1] * CovgRemaining;
												Poly->BaseMat.Mat[1]->Evaluate(&Pix, TempCovg);
												} // if
											} // if
										#endif // WCS_VECPOLY_EFFECTS
										} // else
							// Ground
									Pix.Transparency = 0.0;
									if (Pix.OpacityUsed < 1.0)
										{
										#ifdef WCS_VECPOLY_EFFECTS
										if (MatToRender = GroundMatTable[0].Material)
											{
											CovgRemaining = 1.0 - Pix.OpacityUsed;
											for (MatCount = 0; MatToRender; MatToRender = GroundMatTable[MatCount].Material)
												{
												MatWt[0] = GroundMatTable[MatCount].MatListPtr[VertIndex[0]] ? VtxWt[0] * GroundMatTable[MatCount].MatListPtr[VertIndex[0]]->MatCoverage: 0.0;
												MatWt[1] = GroundMatTable[MatCount].MatListPtr[VertIndex[1]] ? VtxWt[1] * GroundMatTable[MatCount].MatListPtr[VertIndex[1]]->MatCoverage: 0.0;
												MatWt[2] = GroundMatTable[MatCount].MatListPtr[VertIndex[2]] ? VtxWt[2] * GroundMatTable[MatCount].MatListPtr[VertIndex[2]]->MatCoverage: 0.0;
												if ((TempCovg = CovgRemaining * (MatWt[0] + MatWt[1] + MatWt[2])) > 0.0)
													{
													if (TempCovg + Pix.OpacityUsed > .9999)
														TempCovg = 1.0 - Pix.OpacityUsed;
													Pix.TexData->ExtractMatTableData(&GroundMatTable[MatCount], VertIndex);
													MatToRender->Evaluate(&Pix, TempCovg);
													if (Pix.OpacityUsed >= 1.0)
														break;
													} // if
												++MatCount;
												} // for
											} // if
										#else // WCS_VECPOLY_EFFECTS
										if (Poly->GroundMat.Mat[0])
											{
											CovgRemaining = 1.0 - Pix.OpacityUsed;
											TempCovg = Poly->GroundMat.Covg[0] * CovgRemaining;
											Poly->GroundMat.Mat[0]->Evaluate(&Pix, TempCovg);
											if (Poly->GroundMat.Mat[1])
												{
												TempCovg = Poly->GroundMat.Covg[1] * CovgRemaining;
												Poly->GroundMat.Mat[1]->Evaluate(&Pix, TempCovg);
												} // if
											} // if
										#endif // WCS_VECPOLY_EFFECTS
										} // if
									} // if
								} // if
							} // if
						// transparency may be _used_ by ecosystems and snow but we don't render transparent terrain
						// unless using pixel fragments
						if (! rPixelFragMap)
							Pix.Transparency = 0.0;
						PlotTerrainPixel(&Pix, Poly, X, Y, PixZip, NewWt, TopPixCovg, BotPixCovg);

						// restore for the next pixel
						Pix.Normal[0] = NormalStash[0];
						Pix.Normal[1] = NormalStash[1];
						Pix.Normal[2] = NormalStash[2];
						Pix.TexData->Slope = SlopeStash;
						Pix.TexData->Aspect = AspectStash;
						} // if terrain
					else if (PolyType == WCS_POLYGONTYPE_WATER)
						{
						Pix.TexData->ObjectType = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_WATER;
						Pix.PixelType = WCS_PIXELTYPE_WATER;

						#ifdef WCS_VECPOLY_EFFECTS
						if (WaterMatTableStart[0].Material)
							{
							if (Poly->ShadeType == WCS_EFFECT_MATERIAL_SHADING_PHONG)
								{
								MatWt[0] = VtxWt[0] * WaterMatTableStart[0].MatListPtr[VertIndex[0]]->WaterProp->WaterNormal[0];
								MatWt[1] = VtxWt[1] * WaterMatTableStart[0].MatListPtr[VertIndex[1]]->WaterProp->WaterNormal[0];
								MatWt[2] = VtxWt[2] * WaterMatTableStart[0].MatListPtr[VertIndex[2]]->WaterProp->WaterNormal[0];
								Pix.Normal[0] = MatWt[0] + MatWt[1] + MatWt[2];
								MatWt[0] = VtxWt[0] * WaterMatTableStart[0].MatListPtr[VertIndex[0]]->WaterProp->WaterNormal[1];
								MatWt[1] = VtxWt[1] * WaterMatTableStart[0].MatListPtr[VertIndex[1]]->WaterProp->WaterNormal[1];
								MatWt[2] = VtxWt[2] * WaterMatTableStart[0].MatListPtr[VertIndex[2]]->WaterProp->WaterNormal[1];
								Pix.Normal[1] = MatWt[0] + MatWt[1] + MatWt[2];
								MatWt[0] = VtxWt[0] * WaterMatTableStart[0].MatListPtr[VertIndex[0]]->WaterProp->WaterNormal[2];
								MatWt[1] = VtxWt[1] * WaterMatTableStart[0].MatListPtr[VertIndex[1]]->WaterProp->WaterNormal[2];
								MatWt[2] = VtxWt[2] * WaterMatTableStart[0].MatListPtr[VertIndex[2]]->WaterProp->WaterNormal[2];
								Pix.Normal[2] = MatWt[0] + MatWt[1] + MatWt[2];
								} // if
							MatWt[0] = VtxWt[0] * WaterMatTableStart[0].MatListPtr[VertIndex[0]]->WaterProp->WaveHeight;
							MatWt[1] = VtxWt[1] * WaterMatTableStart[0].MatListPtr[VertIndex[1]]->WaterProp->WaveHeight;
							MatWt[2] = VtxWt[2] * WaterMatTableStart[0].MatListPtr[VertIndex[2]]->WaterProp->WaveHeight;
							Pix.TexData->WaveHeight = MatWt[0] + MatWt[1] + MatWt[2];
							MatWt[0] = VtxWt[0] * WaterMatTableStart[0].MatListPtr[VertIndex[0]]->WaterProp->WaterDepth;
							MatWt[1] = VtxWt[1] * WaterMatTableStart[0].MatListPtr[VertIndex[1]]->WaterProp->WaterDepth;
							MatWt[2] = VtxWt[2] * WaterMatTableStart[0].MatListPtr[VertIndex[2]]->WaterProp->WaterDepth;
							Pix.TexData->WaterDepth = Pix.WaterDepth = MatWt[0] + MatWt[1] + MatWt[2];
							} // if
						Pix.TexData->Slope = VtxWt[0] * ((VertexData *)Vert[0])->NodeData->Slope + VtxWt[1] * ((VertexData *)Vert[1])->NodeData->Slope + VtxWt[2] * ((VertexData *)Vert[2])->NodeData->Slope;
						if ((fabs(((VertexData *)Vert[0])->NodeData->Aspect - 
							((VertexData *)Vert[1])->NodeData->Aspect) > 180.0)
							||
							(fabs(((VertexData *)Vert[0])->NodeData->Aspect - 
							((VertexData *)Vert[2])->NodeData->Aspect) > 180.0)
							||
							(fabs(((VertexData *)Vert[1])->NodeData->Aspect - 
							((VertexData *)Vert[2])->NodeData->Aspect) > 180.0))
							{
							double TempAspect[3];
							TempAspect[0] = ((VertexData *)Vert[0])->NodeData->Aspect < 180.0 ?
								((VertexData *)Vert[0])->NodeData->Aspect + 360.0: ((VertexData *)Vert[0])->NodeData->Aspect;
							TempAspect[1] = ((VertexData *)Vert[1])->NodeData->Aspect < 180.0 ?
								((VertexData *)Vert[1])->NodeData->Aspect + 360.0: ((VertexData *)Vert[1])->NodeData->Aspect;
							TempAspect[2] = ((VertexData *)Vert[2])->NodeData->Aspect < 180.0 ?
								((VertexData *)Vert[2])->NodeData->Aspect + 360.0: ((VertexData *)Vert[2])->NodeData->Aspect;
							Pix.TexData->Aspect = VtxWt[0] * TempAspect[0] + VtxWt[1] * TempAspect[1] + VtxWt[2] * TempAspect[2];
							if (Pix.TexData->Aspect >= 360.0)
								Pix.TexData->Aspect -= 360.0;
							} // if
						else
							Pix.TexData->Aspect = VtxWt[0] * ((VertexData *)Vert[0])->NodeData->Aspect + VtxWt[1] * ((VertexData *)Vert[1])->NodeData->Aspect + VtxWt[2] * ((VertexData *)Vert[2])->NodeData->Aspect;
						Pix.TexData->RelElev = (float)(VtxWt[0] * ((VertexData *)Vert[0])->NodeData->RelEl + VtxWt[1] * ((VertexData *)Vert[1])->NodeData->RelEl + VtxWt[2] * ((VertexData *)Vert[2])->NodeData->RelEl);
						#else // WCS_VECPOLY_EFFECTS
						Pix.TexData->RelElev = (float)(VtxWt[0] * ((VertexData *)Vert[0])->RelEl + VtxWt[1] * ((VertexData *)Vert[1])->RelEl + VtxWt[2] * ((VertexData *)Vert[2])->RelEl);
						Pix.TexData->WaveHeight = VtxWt[0] * ((VertexData *)Vert[0])->WaveHeight + VtxWt[1] * ((VertexData *)Vert[1])->WaveHeight + VtxWt[2] * ((VertexData *)Vert[2])->WaveHeight;
						Pix.TexData->WaterDepth = Pix.WaterDepth = VtxWt[0] * ((VertexData *)Vert[0])->WaterDepth + VtxWt[1] * ((VertexData *)Vert[1])->WaterDepth + VtxWt[2] * ((VertexData *)Vert[2])->WaterDepth;
						#endif // WCS_VECPOLY_EFFECTS

						// stash normals in case bump mapping changes them
						NormalStash[0] = Pix.Normal[0];
						NormalStash[1] = Pix.Normal[1];
						NormalStash[2] = Pix.Normal[2];

						if (TexturesExist)
							#ifdef WCS_VECPOLY_EFFECTS
							InterpretWaterBumpMaps(&Pix, VtxWt, VertIndex);
							#else // WCS_VECPOLY_EFFECTS
							InterpretWaterBumpMaps(&Pix, Poly);
							#endif // WCS_VECPOLY_EFFECTS

						// normals may be needed by textures
						Pix.TexData->Normal[0] = Pix.Normal[0];
						Pix.TexData->Normal[1] = Pix.Normal[1];
						Pix.TexData->Normal[2] = Pix.Normal[2];

						// need to evaluate the water material
						#ifdef WCS_VECPOLY_EFFECTS
						if (MatToRender = WaterMatTableStart[0].Material)
							{
							CovgRemaining = 1.0 - Pix.OpacityUsed;
							for (MatCount = 0; MatToRender && (&WaterMatTableStart[MatCount] != WaterMatTableEnd); MatToRender = WaterMatTableStart[MatCount].Material)
								{
								MatWt[0] = VtxWt[0] * WaterMatTableStart[MatCount].MatListPtr[VertIndex[0]]->MatCoverage;
								MatWt[1] = VtxWt[1] * WaterMatTableStart[MatCount].MatListPtr[VertIndex[1]]->MatCoverage;
								MatWt[2] = VtxWt[2] * WaterMatTableStart[MatCount].MatListPtr[VertIndex[2]]->MatCoverage;
								if ((TempCovg = CovgRemaining * (MatWt[0] + MatWt[1] + MatWt[2])) > 0.0)
									{
									// evaluate water material, includes foam
									if (TempCovg + Pix.OpacityUsed > .9999)
										TempCovg = 1.0 - Pix.OpacityUsed;
									Pix.TexData->ExtractMatTableData(&WaterMatTableStart[MatCount], VertIndex);
									MatToRender->Evaluate(&Pix, TempCovg);
									if (Pix.OpacityUsed >= 1.0)
										break;
									} // if
								++MatCount;
								} // for
							} // if
						#else // WCS_VECPOLY_EFFECTS
						if (Poly->BaseMat.Mat[0])
							{
							// evaluate water material, includes foam
							Poly->BaseMat.Mat[0]->Evaluate(&Pix, Poly->BaseMat.Covg[0]);
							} // if
						if (Poly->BaseMat.Mat[1])
							{
							// evaluate water material, includes foam
							Poly->BaseMat.Mat[1]->Evaluate(&Pix, Poly->BaseMat.Covg[1]);
							} // if
						#endif // WCS_VECPOLY_EFFECTS
						Pix.Transparency = 0.0;

						PlotWaterPixel(&Pix, Poly, X, Y, PixZip, NewWt, TopPixCovg, BotPixCovg);

						// restore for the next pixel
						Pix.Normal[0] = NormalStash[0];
						Pix.Normal[1] = NormalStash[1];
						Pix.Normal[2] = NormalStash[2];
						} // else if water
					else if (PolyType == WCS_POLYGONTYPE_CLOUD)
						{
						Pix.TexData->ObjectType = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_CLOUD;
						Pix.PixelType = WCS_PIXELTYPE_CLOUD;

						Pix.TexData->RelElev = 0.0f;
						// F2_Note: check asm for order of memory access
						Pix.TexData->WaterDepth = Pix.WaterDepth = Pix.TexData->WaveHeight = 0.0;

						// need to evaluate the cloud density, illumination, translucency
						// xyz[0] has the normalized amplitude for waves
						// xyz[1] has the intensity factor based on falloff and vectors
						VertexIntensity = VtxWt[0] * Vert[0]->xyz[1] + VtxWt[1] * Vert[1]->xyz[1] + VtxWt[2] * Vert[2]->xyz[1];
						if (VertexIntensity > 0.0)
							{
							Pix.OpacityUsed = VtxWt[0] * Vert[0]->xyz[0] + VtxWt[1] * Vert[1]->xyz[0] + VtxWt[2] * Vert[2]->xyz[0];
							Poly->Cloud->EvaluateDensity(&Pix, Poly->CloudLayer, VertexIntensity);	// if() removed to allow solid cloud so as to prevent reflection streaks
							//Pix.OpacityUsed *= VertexIntensity;
							PlotCloudPixel(&Pix, Poly, X, Y, PixZip, NewWt, TopPixCovg, BotPixCovg);
							} // if Intensity > 0
						} // else if cloud
					else if (PolyType == WCS_POLYGONTYPE_FENCE)
						{
						Pix.TexData->ObjectType = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FENCE;
						Pix.PixelType = WCS_PIXELTYPE_FENCE;

						if (TexturesExist)
							{
							Pix.TexData->WaveHeight = 0.0;
							Pix.TexData->WaterDepth = Pix.WaterDepth = VtxWt[0] * ((VertexData *)Vert[0])->WaterDepth + VtxWt[1] * ((VertexData *)Vert[1])->WaterDepth + VtxWt[2] * ((VertexData *)Vert[2])->WaterDepth;
							Pix.TexData->RelElev = (float)(VtxWt[0] * ((VertexData *)Vert[0])->RelEl + VtxWt[1] * ((VertexData *)Vert[1])->RelEl + VtxWt[2] * ((VertexData *)Vert[2])->RelEl);
							} // if
							
						// stash normals in case bump mapping changes them
						NormalStash[0] = Pix.Normal[0];
						NormalStash[1] = Pix.Normal[1];
						NormalStash[2] = Pix.Normal[2];
						SlopeStash = Poly->Slope;
						AspectStash = Poly->Aspect;

						if (TexturesExist)
							InterpretFenceBumpMaps(&Pix, Poly);

						// normals may be needed by textures
						Pix.TexData->Normal[0] = Pix.Normal[0];
						Pix.TexData->Normal[1] = Pix.Normal[1];
						Pix.TexData->Normal[2] = Pix.Normal[2];

						if (Poly->GroundMat.Mat[0])
							{
							CovgRemaining = 1.0 - Pix.OpacityUsed;
							TempCovg = Poly->GroundMat.Covg[0] * CovgRemaining;
							Poly->GroundMat.Mat[0]->Evaluate(&Pix, TempCovg);
							if (Poly->GroundMat.Mat[1])
								{
								TempCovg = Poly->GroundMat.Covg[1] * CovgRemaining;
								Poly->GroundMat.Mat[1]->Evaluate(&Pix, TempCovg);
								} // if
							} // if
						PlotFencePixel(&Pix, Poly, X, Y, PixZip, NewWt, TopPixCovg, BotPixCovg);

						// restore for the next pixel
						Pix.Normal[0] = NormalStash[0];
						Pix.Normal[1] = NormalStash[1];
						Pix.Normal[2] = NormalStash[2];
						Pix.TexData->Slope = SlopeStash;
						Pix.TexData->Aspect = AspectStash;
						Poly->Slope = SlopeStash;
						Poly->Aspect = AspectStash;

						} // if fence
					else
						{
						// material is a member of the 3d object polygon 
						Pix.TexData->ObjectType = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_3DOBJECT;
						Pix.PixelType = WCS_PIXELTYPE_3DOBJECT;
						if (FragMap3D)
							{
							LocalX = X - Poly->PlotOffset3DX;
							LocalY = Y - Poly->PlotOffset3DY;
							LocalZip = LocalY * Poly->Plot3DRast->Cols + LocalX;
							} // if

						Pix.TexData->WaterDepth = Pix.WaterDepth = Poly->WaterElev - Pix.Elev;
						Pix.TexData->RelElev = Poly->RelEl;
						Pix.TexData->WaveHeight = 0.0;

						// stash normals in case bump mapping changes them
						NormalStash[0] = Pix.Normal[0];
						NormalStash[1] = Pix.Normal[1];
						NormalStash[2] = Pix.Normal[2];
						// evaluate material
						if (Poly->ShadeType == WCS_EFFECT_MATERIAL_SHADING_PHONG)
							{
							Pix.Normal[0] = VtxWt[0] * ((Vertex3D *)Vert[0])->Normal[0] + VtxWt[1] * ((Vertex3D *)Vert[1])->Normal[0] + VtxWt[2] * ((Vertex3D *)Vert[2])->Normal[0];
							Pix.Normal[1] = VtxWt[0] * ((Vertex3D *)Vert[0])->Normal[1] + VtxWt[1] * ((Vertex3D *)Vert[1])->Normal[1] + VtxWt[2] * ((Vertex3D *)Vert[2])->Normal[1];
							Pix.Normal[2] = VtxWt[0] * ((Vertex3D *)Vert[0])->Normal[2] + VtxWt[1] * ((Vertex3D *)Vert[1])->Normal[2] + VtxWt[2] * ((Vertex3D *)Vert[2])->Normal[2];
							UnitVector(Pix.Normal);
							} // if

						if (TexturesExist)
							Interpret3DObjectBumpMaps(&Pix, Poly);

						// normals may be needed by textures
						Pix.TexData->Normal[0] = Pix.Normal[0];
						Pix.TexData->Normal[1] = Pix.Normal[1];
						Pix.TexData->Normal[2] = Pix.Normal[2];

						if (TexturesExist)
							{
							Pix.TexData->Slope = GlobalApp->MathTables->ACosTab.LookupLerped(VectorAngle(Vert[0]->XYZ, Pix.Normal)) * PiUnder180;	// in degrees
							if (Pix.TexData->Slope < 0.0)
								Pix.TexData->Slope = 0.0;
							if (Pix.TexData->Slope > 90.0)
								Pix.TexData->Slope = 90.0;

							// compute aspect
							NormalVert.XYZ[0] = Pix.Normal[0];
							NormalVert.XYZ[1] = Pix.Normal[1];
							NormalVert.XYZ[2] = Pix.Normal[2];
							NormalVert.Lat = Pix.Lat;
							NormalVert.Lon = Pix.Lon;
							NormalVert.RotateToHome();
							Pix.TexData->Aspect = NormalVert.FindRoughAngleYfromZ();
							if (Pix.TexData->Aspect < 0.0)
								Pix.TexData->Aspect += 360.0;
							} // if

						Poly->BaseMat.Mat[0]->Evaluate(&Pix, Poly->BaseMat.Covg[0]);

						if (FragMap3D)
							{
							Plot3DObjectPixel(FragMap3D, &Pix, Poly, LocalX, LocalY, LocalZip, PixZip, NewWt, TopPixCovg, BotPixCovg);
							} // else
						else if (rPixelFragMap)
							{
							NewWt *= Poly->RenderPassWeight;
							Plot3DObjectPixel(&Pix, Poly, X, Y, PixZip, NewWt, TopPixCovg, BotPixCovg);
							} // if
						// restore for the next pixel
						Pix.Normal[0] = NormalStash[0];
						Pix.Normal[1] = NormalStash[1];
						Pix.Normal[2] = NormalStash[2];
						} // else 3d object
					} // if passed z buffer/antialias test
				} // if pixel has weight
			if (FragMemFailed)
				{
				if (! (Success = PixelFragMemoryError()))
					goto DeleteCropVerts;
				} // if
			} // for
		} // for
	} // if
else	// shadow map
	{
	// normalize vertex weights for the whole weight array
	if (! CamPlanOrtho)
		{
		for (WtZip = 0; WtZip < BoxSize; ++WtZip)
			{
			if (PixelWeight[WtZip] > 1)
				{
				VertexWeight[0][WtZip] /= (PixelWeight[WtZip] * Vert[0]->ScrnXYZ[2]);
				VertexWeight[1][WtZip] /= (PixelWeight[WtZip] * Vert[1]->ScrnXYZ[2]);
				VertexWeight[2][WtZip] /= (PixelWeight[WtZip] * Vert[2]->ScrnXYZ[2]);
				} // if
			else if (PixelWeight[WtZip])
				{
				VertexWeight[0][WtZip] /= (Vert[0]->ScrnXYZ[2]);
				VertexWeight[1][WtZip] /= (Vert[1]->ScrnXYZ[2]);
				VertexWeight[2][WtZip] /= (Vert[2]->ScrnXYZ[2]);
				} // else
			} // for
		} // if
	else
		{
		TempZ[0] = Vert[0]->ScrnXYZ[2] - ShadowMapDistanceOffset;
		TempZ[1] = Vert[1]->ScrnXYZ[2] - ShadowMapDistanceOffset;
		TempZ[2] = Vert[2]->ScrnXYZ[2] - ShadowMapDistanceOffset;
		for (WtZip = 0; WtZip < BoxSize; WtZip ++)
			{
			if (PixelWeight[WtZip] > 1)
				{
				VertexWeight[0][WtZip] /= PixelWeight[WtZip];
				VertexWeight[1][WtZip] /= PixelWeight[WtZip];
				VertexWeight[2][WtZip] /= PixelWeight[WtZip];
				} // if
			} // for
		} // else

	Pix.TexData = &RendData.TexData;		// TexData is RenderData scope so it doesn't need to be constructed for each polygon.
	if (PolyType == WCS_POLYGONTYPE_3DOBJECT)
		{
		if (! Poly->Plot3DRast)
			return (1);
		FragMap3D = Poly->Plot3DRast->rPixelBlock;
		if (Poly->BaseMat.Mat[0] && Poly->BaseMat.Mat[0]->TransparencyTexturesExist)
			TexturesExist = 1;
		} // if
	else if (PolyType == WCS_POLYGONTYPE_CLOUD)
		{
		Pix.TexData->RelElev = 0.0f;
		Pix.TexData->WaterDepth = Pix.TexData->WaveHeight = Pix.TexData->Slope = Pix.TexData->Aspect = 0.0;
		} // else if
	else if (PolyType == WCS_POLYGONTYPE_FENCE)
		{
		if (Poly->GroundMat.Mat[0] && Poly->GroundMat.Mat[0]->TransparencyTexturesExist)
			TexturesExist = 1;
		else if (Poly->GroundMat.Mat[1] && Poly->GroundMat.Mat[1]->TransparencyTexturesExist)
			TexturesExist = 1;
		} // if

	Pix.TexData->Vec = Poly->Vector;
	Pix.TexData->VectorEffectType = Poly->VectorType;
	Pix.TexData->VectorSlope = Poly->VectorSlope;
	Pix.ShadowFlags = Poly->ShadowFlags;
	Pix.ShadowOffset = Poly->ShadowOffset;
	Pix.ReceivedShadowIntensity = Poly->ReceivedShadowIntensity;
	Pix.Object = Pix.TexData->Object = Poly->Object;
	Pix.TexData->Object3D = NULL;
	Pix.TexData->VtxNum[0] = VertNum[0];
	Pix.TexData->VtxNum[1] = VertNum[1];
	Pix.TexData->VtxNum[2] = VertNum[2];
	Pix.TexData->VertRefData[0] = VertRefData[0];
	Pix.TexData->VertRefData[1] = VertRefData[1];
	Pix.TexData->VertRefData[2] = VertRefData[2];
	Pix.TexData->PData = Poly;

	for (WtZip = 0, Y = quickftol(BoxYmin), Yp = 0, y = .5; Y < MaxY; Y ++, Yp ++, y += 1.0)
		{
		PixZip = Y * Width + quickftol(BoxXmin);

		// this could be sped up a bit by calculating the extrema edge values (max or min of the 
		// edges at the top and bottom of the pixel) and scanning only that x range.
		// Another tricky technique would be to calculate the pixel center's edge value and adding or
		// subtracting .5 x slope of the edge. The center edge values might be necessary anyway
		// for interpolating other values across the polygon (such as Z and elevation...).
		for (X = quickftol(BoxXmin), Xp = 0, x = .5; X < MaxX; X ++, Xp ++, x += 1.0, WtZip ++, PixZip ++)
			{
			if (NewWt = PixelWeight[WtZip] < 64 ? PixelWeight[WtZip] * 3.9999: 255.99)
				{
				VtxPct[0] = VertexWeight[0][WtZip];
				VtxPct[1] = VertexWeight[1][WtZip];
				VtxPct[2] = VertexWeight[2][WtZip];
				if (! CamPlanOrtho)
					{
					// find the Z value for the pixel - we are interpolating 1/Z
					ZTranslator = (VtxPct[0] + VtxPct[1] + VtxPct[2]);
					// restore to real Z
					ZTranslator = 1.0 / ZTranslator;
					Pix.Zflt = (float)(ZTranslator - ShadowMapDistanceOffset);
					} // if
				else
					{
					// find the Z value for the pixel - we are interpolating Z
					Pix.Zflt = (float)(VtxPct[0] * TempZ[0] + VtxPct[1] * TempZ[1] + VtxPct[2] * TempZ[2]);
					// no need to restore to real Z
					ZTranslator = 1.0;
					} // else

				// test z value against z buffer and antialias buffer
				if ((Pix.Zflt <= TreeZBuf[PixZip] || TreeAABuf[PixZip] < 255) && (Pix.Zflt <= ZBuf[PixZip] || AABuf[PixZip] < 255))	// <<<>>>gh
					{
					// set current pixel used opacity, color, luminosity, reflectivity, specularity... to 0
					Pix.SetDefaults();
					Pix.PixZip = PixZip;
					Pix.TexData->VtxWt[0] = VtxWt[0] = ZTranslator * VtxPct[0];
					Pix.TexData->VtxWt[1] = VtxWt[1] = ZTranslator * VtxPct[1];
					Pix.TexData->VtxWt[2] = VtxWt[2] = ZTranslator * VtxPct[2];

					// obtain color from material(s)
					if (PolyType == WCS_POLYGONTYPE_TERRAIN)
						{
						Pix.PixelType = WCS_PIXELTYPE_TERRAIN;
						// need to blend together foliageblend colors, snow, ecosystem, ground, beach
						// evaluate in this order: overstory dissolve, snow, ecosystem (incl. understory dissolve), ground.
						// replace ecosystem with beach material if eco is null but lake is valid
						#ifdef WCS_VECPOLY_EFFECTS
						if ((MatToRender = GroundMatTable[0].Material) || (MatToRender = EcoMatTable[0].Material))
						#else // WCS_VECPOLY_EFFECTS
						if (MatToRender = Poly->GroundMat.Mat[0])
						#endif // WCS_VECPOLY_EFFECTS
							{
							Pix.RGB[0] = MatToRender->DiffuseColor.CurValue[0];
							Pix.RGB[1] = MatToRender->DiffuseColor.CurValue[1];
							Pix.RGB[2] = MatToRender->DiffuseColor.CurValue[2];
							} // if
						else
							{
							Pix.RGB[0] = Pix.RGB[1] = Pix.RGB[2] = .5;
							} // else
						// transparency may be _used_ by ecosystems and snow but we don't render transparent terrain
						Pix.OpacityUsed = 1.0;
						} // if terrain
					#ifndef WCS_VECPOLY_EFFECTS
					// water is not rendered for shadow maps but this is left in here now because it has always
					// been here for VNS 2
					else if (PolyType == WCS_POLYGONTYPE_WATER)
						{
						// need to evaluate the water material
						if (Poly->BaseMat.Mat[0])
							{
							Pix.PixelType = WCS_PIXELTYPE_WATER;
							Pix.RGB[0] = Poly->BaseMat.Mat[0]->DiffuseColor.CurValue[0];
							Pix.RGB[1] = Poly->BaseMat.Mat[0]->DiffuseColor.CurValue[1];
							Pix.RGB[2] = Poly->BaseMat.Mat[0]->DiffuseColor.CurValue[2];
							} // if
						else
							{
							Pix.RGB[0] = Pix.RGB[1] = Pix.RGB[2] = .5;
							} // else
						Pix.OpacityUsed = 1.0;
						} // else if water
					#endif // WCS_VECPOLY_EFFECTS
					else if (PolyType == WCS_POLYGONTYPE_CLOUD)
						{
						// these values will most certainly be used by either materials or lighting/atmosphere
						Pix.XYZ[0] = VtxWt[0] * Vert[0]->XYZ[0] + VtxWt[1] * Vert[1]->XYZ[0] + VtxWt[2] * Vert[2]->XYZ[0];
						Pix.XYZ[1] = VtxWt[0] * Vert[0]->XYZ[1] + VtxWt[1] * Vert[1]->XYZ[1] + VtxWt[2] * Vert[2]->XYZ[1];
						Pix.XYZ[2] = VtxWt[0] * Vert[0]->XYZ[2] + VtxWt[1] * Vert[1]->XYZ[2] + VtxWt[2] * Vert[2]->XYZ[2];
						Pix.TexData->QDist = VtxWt[0] * Vert[0]->Q + VtxWt[1] * Vert[1]->Q + VtxWt[2] * Vert[2]->Q;
						Pix.TexData->Elev = VtxWt[0] * ((VertexDEM *)Vert[0])->Elev + VtxWt[1] * ((VertexDEM *)Vert[1])->Elev + VtxWt[2] * ((VertexDEM *)Vert[2])->Elev;
						Pix.TexData->Latitude = VtxWt[0] * ((VertexDEM *)Vert[0])->Lat + VtxWt[1] * ((VertexDEM *)Vert[1])->Lat + VtxWt[2] * ((VertexDEM *)Vert[2])->Lat;
						Pix.TexData->Longitude = VtxWt[0] * ((VertexDEM *)Vert[0])->Lon + VtxWt[1] * ((VertexDEM *)Vert[1])->Lon + VtxWt[2] * ((VertexDEM *)Vert[2])->Lon;
						Pix.TexData->ZDist = Pix.Zflt;

						// items specific to type of polygon
						// this function fills in all the rest of the texture data
						FillTerrainTextureData(Pix.TexData, (VertexData **)Vert, VtxPct, BoxWidth, BoxHeight, 
							WtZip, Xp, Yp, PolyType, Poly->Cloud);
						// need to evaluate the cloud density, illumination, translucency
						Pix.PixelType = WCS_PIXELTYPE_CLOUD;
						// xyz[0] has the normalized amplitude for waves
						// xyz[1] has the intensity factor based on falloff and vectors
						VertexIntensity = VtxWt[0] * Vert[0]->xyz[1] + VtxWt[1] * Vert[1]->xyz[1] + VtxWt[2] * Vert[2]->xyz[1];
						if (VertexIntensity > 0.0)
							{
							Pix.OpacityUsed = VtxWt[0] * Vert[0]->xyz[0] + VtxWt[1] * Vert[1]->xyz[0] + VtxWt[2] * Vert[2]->xyz[0];
							if (Poly->Cloud->EvaluateDensity(&Pix, Poly->CloudLayer, VertexIntensity))
								{
								//Pix.OpacityUsed *= VertexIntensity;

								if (Pix.RGB[0] > 1.0)
									Pix.RGB[0] = 1.0;
								if (Pix.RGB[1] > 1.0)
									Pix.RGB[1] = 1.0;
								if (Pix.RGB[2] > 1.0)
									Pix.RGB[2] = 1.0;
								NewWt *= Pix.OpacityUsed;
								if (Pix.Zflt <= CloudZBuf[PixZip])
									{
									ReplaceValues = true;
									if ((DistDiff = CloudZBuf[PixZip] - Pix.Zflt) < ZMergeDistance)
										{
										if (NewWt + CloudAABuf[PixZip] > 255.99)
											{
											CloudAABuf[PixZip] = (unsigned char)(CloudAABuf[PixZip] * (1.0 - (DistDiff / ZMergeDistance)));
											if (NewWt + CloudAABuf[PixZip] < 255.99)
												CloudAABuf[PixZip] = (unsigned char)(255.99 - NewWt);
											} // if
										SumWt = NewWt + CloudAABuf[PixZip];
										InvSumWt = 1.0 / SumWt;
										CloudBitmap[0][PixZip] = (unsigned char)((Pix.RGB[0] * 255.99 * NewWt + CloudBitmap[0][PixZip] * CloudAABuf[PixZip]) * InvSumWt);
										CloudBitmap[1][PixZip] = (unsigned char)((Pix.RGB[1] * 255.99 * NewWt + CloudBitmap[1][PixZip] * CloudAABuf[PixZip]) * InvSumWt);
										CloudBitmap[2][PixZip] = (unsigned char)((Pix.RGB[2] * 255.99 * NewWt + CloudBitmap[2][PixZip] * CloudAABuf[PixZip]) * InvSumWt);
										} // if need to compute a blend factor
									else
										{
										SumWt = NewWt + CloudAABuf[PixZip];
										CloudBitmap[0][PixZip] = (unsigned char)(Pix.RGB[0] * 255.99);
										CloudBitmap[1][PixZip] = (unsigned char)(Pix.RGB[1] * 255.99);
										CloudBitmap[2][PixZip] = (unsigned char)(Pix.RGB[2] * 255.99);
										} // just a straight replacement
									} // if
								else
									{
									// new pixel is farther away and we know that AABuf is not full so blend the values
									ReplaceValues = false;
									if (NewWt + CloudAABuf[PixZip] > 255.99)
										NewWt = 255.99 -  CloudAABuf[PixZip];
									SumWt = NewWt + CloudAABuf[PixZip];
									InvSumWt = 1.0 / SumWt;
									CloudBitmap[0][PixZip] = (unsigned char)((Pix.RGB[0] * 255.99 * NewWt + CloudBitmap[0][PixZip] * CloudAABuf[PixZip]) * InvSumWt);
									CloudBitmap[1][PixZip] = (unsigned char)((Pix.RGB[1] * 255.99 * NewWt + CloudBitmap[1][PixZip] * CloudAABuf[PixZip]) * InvSumWt);
									CloudBitmap[2][PixZip] = (unsigned char)((Pix.RGB[2] * 255.99 * NewWt + CloudBitmap[2][PixZip] * CloudAABuf[PixZip]) * InvSumWt);
									} // else need to blend for sure
								SumWt = WCS_ceil(SumWt);
								if (SumWt >= 254.5)
									CloudAABuf[PixZip] = 255;
								else
									CloudAABuf[PixZip] = (unsigned char)(SumWt);
								if (ReplaceValues)
									{
									CloudZBuf[PixZip] = Pix.Zflt;
									} // if
								ScreenPixelPlotTwoBuf(Bitmap, ZBuf, AABuf, CloudBitmap, CloudZBuf, CloudAABuf, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
								continue;
								} // if
							} // if Intensity > 0
						continue;
						} // else if cloud
					else if (PolyType == WCS_POLYGONTYPE_FENCE)
						{
						Pix.PixelType = WCS_PIXELTYPE_FENCE;

						if (TexturesExist)
							{
							// fill in texture data, at least some of it's gonna get used

							// generic items
							Pix.TexData->ZDist = Pix.Zflt;
							Pix.TexData->QDist = Pix.Q;
							Pix.TexData->PixelX[0] = (double)X + SegmentOffsetX - Width * Cam->PanoPanels * .5;
							Pix.TexData->PixelX[1] = (double)(X + 1) + SegmentOffsetX - Width * Cam->PanoPanels * .5;
							Pix.TexData->PixelY[0] = -((double)Y + SegmentOffsetY) + Height * Opt->RenderImageSegments * .5;
							Pix.TexData->PixelY[1] = -((double)(Y + 1) + SegmentOffsetY) + Height * Opt->RenderImageSegments * .5;
							Pix.TexData->PixelUnityX[0] = Pix.TexData->PixelX[0] / (Width * Cam->PanoPanels);
							Pix.TexData->PixelUnityX[1] = Pix.TexData->PixelX[1] / (Width * Cam->PanoPanels);
							Pix.TexData->PixelUnityY[0] = Pix.TexData->PixelY[0] / (Height * Opt->RenderImageSegments);
							Pix.TexData->PixelUnityY[1] = Pix.TexData->PixelY[1] / (Height * Opt->RenderImageSegments);
							Pix.TexData->Elev = Pix.Elev;
							Pix.TexData->Latitude = Pix.Lat;
							Pix.TexData->Longitude = Pix.Lon;
							// initialize so it can be calculated only once by textures if needed
							Pix.TexData->VectorOffsetsComputed = 0;

							// items specific to type of polygon
							Pix.TexData->RelElev = (float)(VtxWt[0] * ((VertexData *)Vert[0])->RelEl + VtxWt[1] * ((VertexData *)Vert[1])->RelEl + VtxWt[2] * ((VertexData *)Vert[2])->RelEl);
							Pix.TexData->WaterDepth = VtxWt[0] * ((VertexData *)Vert[0])->WaterDepth + VtxWt[1] * ((VertexData *)Vert[1])->WaterDepth + VtxWt[2] * ((VertexData *)Vert[2])->WaterDepth;
							Pix.TexData->WaveHeight = VtxWt[0] * ((VertexData *)Vert[0])->WaveHeight + VtxWt[1] * ((VertexData *)Vert[1])->WaveHeight + VtxWt[2] * ((VertexData *)Vert[2])->WaveHeight;
							// this function fills in all the rest of the texture data
							FillTerrainTextureData(Pix.TexData, (VertexData **)Vert, VtxPct, BoxWidth, BoxHeight, 
								WtZip, Xp, Yp, PolyType, Poly->Cloud);
							} // if textures

						if (Poly->GroundMat.Mat[0])
							{
							CovgRemaining = 1.0 - Pix.OpacityUsed;
							TempCovg = Poly->GroundMat.Covg[0] * CovgRemaining;
							Poly->GroundMat.Mat[0]->EvaluateTransparency(&Pix, TempCovg);
							if (Poly->GroundMat.Mat[1])
								{
								TempCovg = Poly->GroundMat.Covg[1] * CovgRemaining;
								Poly->GroundMat.Mat[1]->EvaluateTransparency(&Pix, TempCovg);
								} // if
							} // if

						if (Pix.Transparency < 1.0)
							{
							NewWt *= (1.0 - Pix.Transparency);

							if (Pix.Zflt < TreeZBuf[PixZip])
								{
								ReplaceValues = true;
								if ((DistDiff = TreeZBuf[PixZip] - Pix.Zflt) < ZMergeDistance)
									{
									if (NewWt + TreeAABuf[PixZip] > 255.99)
										{
										TreeAABuf[PixZip] = (unsigned char)(TreeAABuf[PixZip] * (1.0 - (DistDiff / ZMergeDistance)));
										if (NewWt + TreeAABuf[PixZip] < 255.99)
											TreeAABuf[PixZip] = (unsigned char)(255.99 - NewWt);
										} // if
									SumWt = NewWt + TreeAABuf[PixZip];
									InvSumWt = 1.0 / SumWt;
									TreeBitmap[0][PixZip] = (unsigned char)((Pix.RGB[0] * 255.99 * NewWt + TreeBitmap[0][PixZip] * TreeAABuf[PixZip]) * InvSumWt);
									TreeBitmap[1][PixZip] = (unsigned char)((Pix.RGB[1] * 255.99 * NewWt + TreeBitmap[1][PixZip] * TreeAABuf[PixZip]) * InvSumWt);
									TreeBitmap[2][PixZip] = (unsigned char)((Pix.RGB[2] * 255.99 * NewWt + TreeBitmap[2][PixZip] * TreeAABuf[PixZip]) * InvSumWt);
									} // if need to compute a blend factor
								else
									{
									SumWt = NewWt + TreeAABuf[PixZip];
									TreeBitmap[0][PixZip] = (unsigned char)(Pix.RGB[0] * 255.99);
									TreeBitmap[1][PixZip] = (unsigned char)(Pix.RGB[1] * 255.99);
									TreeBitmap[2][PixZip] = (unsigned char)(Pix.RGB[2] * 255.99);
									} // just a straight replacement
								} // if
							else
								{
								ReplaceValues = false;
								// no test for distance, only merge colors if the pixel is already terrain.
									// new pixel is farther away and we know that AABuf is not full so blend the values
								if (NewWt + TreeAABuf[PixZip] > 255.99)
									NewWt = 255.99 -  TreeAABuf[PixZip];
								SumWt = NewWt + TreeAABuf[PixZip];
								InvSumWt = 1.0 / SumWt;
								TreeBitmap[0][PixZip] = (unsigned char)((Pix.RGB[0] * 255.99 * NewWt + TreeBitmap[0][PixZip] * TreeAABuf[PixZip]) * InvSumWt);
								TreeBitmap[1][PixZip] = (unsigned char)((Pix.RGB[1] * 255.99 * NewWt + TreeBitmap[1][PixZip] * TreeAABuf[PixZip]) * InvSumWt);
								TreeBitmap[2][PixZip] = (unsigned char)((Pix.RGB[2] * 255.99 * NewWt + TreeBitmap[2][PixZip] * TreeAABuf[PixZip]) * InvSumWt);
								} // else need to blend for sure
							if (ReplaceValues)
								{
								TreeZBuf[PixZip] = (float)Pix.Zflt;
								if (Pix.Zflt < ZBuf[PixZip])
									{
									ScreenPixelPlot(TreeBitmap, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
									} // if
								else
									ScreenPixelPlotTwoBuf(Bitmap, ZBuf, AABuf, TreeBitmap, TreeZBuf, TreeAABuf, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
								} // if
							else
								{
								ScreenPixelPlotTwoBuf(Bitmap, ZBuf, AABuf, TreeBitmap, TreeZBuf, TreeAABuf, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
								} // else
							SumWt = WCS_ceil(SumWt);
							if (SumWt >= 254.5)
								TreeAABuf[PixZip] = 255;
							else
								TreeAABuf[PixZip] = (unsigned char)(SumWt);
							} // if not completely transparent

						continue;
						} // else if fence
					else
						{
						// material is a member of the 3d object polygon 
						Pix.PixelType = WCS_PIXELTYPE_3DOBJECT;
						LocalX = X - Poly->PlotOffset3DX;
						LocalY = Y - Poly->PlotOffset3DY;
						LocalZip = LocalY * Poly->Plot3DRast->Cols + LocalX;

						// the new method
						// test for need to evaluate material
						if (FragMap3D->FragMap[LocalZip].TestPossibleUsage(Pix.Zflt, ~0UL, ~0UL))
							{
							// some data is only needed if texturing is applied
							if (TexturesExist)
								{
								// fill in texture data, at least some of it's gonna get used

								// generic items
								Pix.TexData->ZDist = Pix.Zflt;
								Pix.TexData->QDist = Pix.Q;
								Pix.TexData->PixelX[0] = (double)X + SegmentOffsetX - Width * Cam->PanoPanels * .5;
								Pix.TexData->PixelX[1] = (double)(X + 1) + SegmentOffsetX - Width * Cam->PanoPanels * .5;
								Pix.TexData->PixelY[0] = -((double)Y + SegmentOffsetY) + Height * Opt->RenderImageSegments * .5;
								Pix.TexData->PixelY[1] = -((double)(Y + 1) + SegmentOffsetY) + Height * Opt->RenderImageSegments * .5;
								Pix.TexData->PixelUnityX[0] = Pix.TexData->PixelX[0] / (Width * Cam->PanoPanels);
								Pix.TexData->PixelUnityX[1] = Pix.TexData->PixelX[1] / (Width * Cam->PanoPanels);
								Pix.TexData->PixelUnityY[0] = Pix.TexData->PixelY[0] / (Height * Opt->RenderImageSegments);
								Pix.TexData->PixelUnityY[1] = Pix.TexData->PixelY[1] / (Height * Opt->RenderImageSegments);
								Pix.TexData->Elev = Pix.Elev;
								Pix.TexData->Latitude = Pix.Lat;
								Pix.TexData->Longitude = Pix.Lon;
								// initialize so it can be calculated only once by textures if needed
								Pix.TexData->VectorOffsetsComputed = 0;

								// items specific to type of polygon
								Pix.TexData->RelElev = Poly->RelEl;
								Pix.TexData->WaterDepth = Poly->WaterElev - Pix.Elev;
								Pix.TexData->WaveHeight = 0.0;
								// this function fills in all the rest of the texture data
								Fill3DObjectTextureData(Pix.TexData, (Vertex3D **)Vert, VtxPct, BoxWidth, BoxHeight, 
									WtZip, Xp, Yp);
								} // if

							Pix.RGB[0] = Poly->BaseMat.Mat[0]->DiffuseColor.CurValue[0];
							Pix.RGB[1] = Poly->BaseMat.Mat[0]->DiffuseColor.CurValue[1];
							Pix.RGB[2] = Poly->BaseMat.Mat[0]->DiffuseColor.CurValue[2];

							Poly->BaseMat.Mat[0]->EvaluateTransparency(&Pix, 1.0);

							Pix.OpacityUsed = 1.0 - Pix.Transparency;

							// compensate for transparency
							NewWt *= Pix.OpacityUsed;
							if (CurFrag = FragMap3D->FragMap[LocalZip].PlotPixel(FragMap3D, Pix.Zflt, (unsigned char)NewWt, ~0UL, ~0UL, 
								FragmentDepth, 0))
								{
								CurFrag->PlotPixel(Pix.RGB);
								} // if
							} // if

						continue;
						} // else 3D Object

					// plot values to buffers
						// need values for RGB, AA, Z
						// discriminate based on Z and AA values
					if (Pix.Zflt <= ZBuf[PixZip])
						{
						ReplaceValues = true;
						if ((DistDiff = ZBuf[PixZip] - Pix.Zflt) < ZMergeDistance)
							{
							if (NewWt + AABuf[PixZip] > 255.99)
								{
								AABuf[PixZip] = (unsigned char)(AABuf[PixZip] * (1.0 - (DistDiff / ZMergeDistance)));
								if (NewWt + AABuf[PixZip] < 255.99)
									AABuf[PixZip] = (unsigned char)(255.99 - NewWt);
								} // if
							SumWt = NewWt + AABuf[PixZip];
							InvSumWt = 1.0 / SumWt;
							Bitmap[0][PixZip] = (unsigned char)((Pix.RGB[0] * 255.99 * NewWt + Bitmap[0][PixZip] * AABuf[PixZip]) * InvSumWt);
							Bitmap[1][PixZip] = (unsigned char)((Pix.RGB[1] * 255.99 * NewWt + Bitmap[1][PixZip] * AABuf[PixZip]) * InvSumWt);
							Bitmap[2][PixZip] = (unsigned char)((Pix.RGB[2] * 255.99 * NewWt + Bitmap[2][PixZip] * AABuf[PixZip]) * InvSumWt);
							} // if need to compute a blend factor
						else
							{
							SumWt = NewWt + AABuf[PixZip];
							Bitmap[0][PixZip] = (unsigned char)(Pix.RGB[0] * 255.99);
							Bitmap[1][PixZip] = (unsigned char)(Pix.RGB[1] * 255.99);
							Bitmap[2][PixZip] = (unsigned char)(Pix.RGB[2] * 255.99);
							} // just a straight replacement
						} // if
					else
						{
						// new pixel is farther away and we know that AABuf is not full so blend the values
						if (NewWt + AABuf[PixZip] > 255.99)
							NewWt = 255.99 -  AABuf[PixZip];
						SumWt = NewWt + AABuf[PixZip];
						InvSumWt = 1.0 / SumWt;
						if (Pix.Reflectivity > 0.0 && ReflectionBuf && ! ReflectionBuf[PixZip])
							{
							Pix.Reflectivity *= NewWt * InvSumWt;
							ReplaceValues = true;
							} // if
						else
							ReplaceValues = false;
						Bitmap[0][PixZip] = (unsigned char)((Pix.RGB[0] * 255.99 * NewWt + Bitmap[0][PixZip] * AABuf[PixZip]) * InvSumWt);
						Bitmap[1][PixZip] = (unsigned char)((Pix.RGB[1] * 255.99 * NewWt + Bitmap[1][PixZip] * AABuf[PixZip]) * InvSumWt);
						Bitmap[2][PixZip] = (unsigned char)((Pix.RGB[2] * 255.99 * NewWt + Bitmap[2][PixZip] * AABuf[PixZip]) * InvSumWt);
						} // else need to blend for sure
					if (ReplaceValues)
						{
						ZBuf[PixZip] = Pix.Zflt;
						} // if
					SumWt = WCS_ceil(SumWt);
					if (SumWt >= 254.5)
						AABuf[PixZip] = 255;
					else
						AABuf[PixZip] = (unsigned char)(SumWt);
					ScreenPixelPlotTwoBuf(Bitmap, ZBuf, AABuf, TreeBitmap, TreeZBuf, TreeAABuf, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
					} // if passed z buffer/antialias test
				} // if pixel has weight
			if (FragMemFailed)
				{
				if (! (Success = PixelFragMemoryError()))
					goto DeleteCropVerts;
				} // if
			} // for
		} // for
	} // else shadow map

FoliageTime:

if (CropCt > 0)
	{
	if (FirstCropPass)
		{
		Vtx[VtxNum1] = LerpVtx[1];
		FirstCropPass = false;
		goto RepeatCropRender;
		} // if
	Vtx[0] = StashVtx[0];
	Vtx[1] = StashVtx[1];
	Vtx[2] = StashVtx[2];
	} // else

RendData.TexData.VDataVecOffsetsValid = 0;

// render foliage
if (Opt->FoliageEnabled)
	{
	#ifdef WCS_VECPOLY_EFFECTS
	if (PolyType == WCS_POLYGONTYPE_TERRAIN)
		{
		if (MatToRender = EcoMatTable[0].Material)
			{
			double CovgUsed[3];
			GeneralEffect *CurEffect;
			unsigned long UnderstorySeed = WCS_SEEDOFFSET_UNDERSTORY0, OverstorySeed = WCS_SEEDOFFSET_OVERSTORY0;
			
			CovgUsed[0] = CovgUsed[1] = CovgUsed[2] = 0.0;
			
			for (MatCount = 0; MatToRender; MatToRender = EcoMatTable[MatCount].Material, UnderstorySeed += WCS_SEEDOFFSET_UNDERSTORY0, OverstorySeed += WCS_SEEDOFFSET_OVERSTORY0)
				{
				// material weights are abstracted to an array here for easy passage to foliage renderer
				MatWt[0] = EcoMatTable[MatCount].MatListPtr[0] ? EcoMatTable[MatCount].MatListPtr[0]->MatCoverage: 0.0;
				MatWt[1] = EcoMatTable[MatCount].MatListPtr[1] ? EcoMatTable[MatCount].MatListPtr[1]->MatCoverage: 0.0;
				MatWt[2] = EcoMatTable[MatCount].MatListPtr[2] ? EcoMatTable[MatCount].MatListPtr[2]->MatCoverage: 0.0;
				if (CurEffect = EcoMatTable[MatCount].MatListPtr[0] ? EcoMatTable[MatCount].MatListPtr[0]->MyEffect:
					EcoMatTable[MatCount].MatListPtr[1] ? EcoMatTable[MatCount].MatListPtr[1]->MyEffect:
					EcoMatTable[MatCount].MatListPtr[2] ? EcoMatTable[MatCount].MatListPtr[2]->MyEffect: NULL)
					{
					if (! ShadowMapInProgress || (ShadowLight && ShadowLight->PassTest(CurEffect)))
						{
						bool RenderAboveBeachLevel = EcoMatTable[MatCount].Material->MaterialType == WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM;
						if (MatWt[0] > 0.0 && CovgUsed[0] + MatWt[0] > .9999)
							MatWt[0] = 1.0 - CovgUsed[0];
						if (MatWt[1] > 0.0 && CovgUsed[1] + MatWt[1] > .9999)
							MatWt[1] = 1.0 - CovgUsed[1];
						if (MatWt[2] > 0.0 && CovgUsed[2] + MatWt[2] > .9999)
							MatWt[2] = 1.0 - CovgUsed[2];
						Poly->ExtractMatTableData(&EcoMatTable[MatCount]);
						#ifdef WCS_FORESTRY_WIZARD
						if (MatWt[0] == MatWt[1] && MatWt[0] == MatWt[2])
							{
							// even distribution of this material's foliage across the polygon
							if (MatToRender->UnderstoryDissolve < 1.0 && MatToRender->EcoFol[1] && MatToRender->EcoFol[1]->Enabled)
								Success = RenderForestryFoliage(Poly, (VertexData **)Vtx, MatToRender->EcoFol[1], 1.0 - MatToRender->UnderstoryDissolve, MatWt[0], NULL, UnderstorySeed, 0, RenderAboveBeachLevel);
							if (MatToRender->OverstoryDissolve < 1.0 && MatToRender->EcoFol[0] && MatToRender->EcoFol[0]->Enabled)
								Success = RenderForestryFoliage(Poly, (VertexData **)Vtx, MatToRender->EcoFol[0], 1.0 - MatToRender->OverstoryDissolve, MatWt[0], NULL, OverstorySeed, 0, RenderAboveBeachLevel);
							} // if
						else
							{
							// uneven distribution of this material's foliage across the polygon
							if (MatToRender->UnderstoryDissolve < 1.0 && MatToRender->EcoFol[1] && MatToRender->EcoFol[1]->Enabled)
								Success = RenderForestryFoliage(Poly, (VertexData **)Vtx, MatToRender->EcoFol[1], 1.0 - MatToRender->UnderstoryDissolve, 1.0, MatWt, UnderstorySeed, 0, RenderAboveBeachLevel);
							if (MatToRender->OverstoryDissolve < 1.0 && MatToRender->EcoFol[0] && MatToRender->EcoFol[0]->Enabled)
								Success = RenderForestryFoliage(Poly, (VertexData **)Vtx, MatToRender->EcoFol[0], 1.0 - MatToRender->OverstoryDissolve, 1.0, MatWt, OverstorySeed, 0, RenderAboveBeachLevel);
							} // else
						#endif // WCS_FORESTRY_WIZARD
						} // if
					} // if
				CovgUsed[0] += MatWt[0];
				CovgUsed[1] += MatWt[1];
				CovgUsed[2] += MatWt[2];
				if (CovgUsed[0] > .9999)
					CovgUsed[0] = 1.0;
				if (CovgUsed[1] > .9999)
					CovgUsed[1] = 1.0;
				if (CovgUsed[2] > .9999)
					CovgUsed[2] = 1.0;
				if (CovgUsed[0] >= 1.0 && CovgUsed[1] >= 1.0 && CovgUsed[2] >= 1.0)
					break;
				++MatCount;
				} // for
			} // if
		} // if
	#else // WCS_VECPOLY_EFFECTS
	if (! ShadowMapInProgress || (ShadowLight && ShadowLight->PassTest(Poly->Object)))
		{
		if (PolyType == WCS_POLYGONTYPE_TERRAIN)
			{
			#ifdef WCS_FORESTRY_WIZARD
			// understory
			if (Success && Poly->UnderstoryDissolve[0] < 1.0 && Poly->BaseMat.Mat[0] && Poly->BaseMat.Mat[0]->EcoFol[1] && Poly->BaseMat.Mat[0]->EcoFol[1]->ChainList)
				{
				// render foliage for understory material 0 using
					// Poly->BaseMat.Mat[0]->EcoFol[1]
					// Poly->UnderstoryDissolve[0]
					// Poly->BaseMat.Covg[0]);
				Success = RenderForestryFoliage(Poly, (VertexData **)Vtx, Poly->BaseMat.Mat[0]->EcoFol[1], 1.0 - Poly->UnderstoryDissolve[0], Poly->BaseMat.Covg[0], NULL, WCS_SEEDOFFSET_UNDERSTORY0, 0, true);
				} // if
			if (Success && Poly->UnderstoryDissolve[1] < 1.0 && Poly->BaseMat.Mat[1] && Poly->BaseMat.Mat[1]->EcoFol[1] && Poly->BaseMat.Mat[1]->EcoFol[1]->ChainList)
				{
				// render foliage for understory material 1 using
					// Poly->BaseMat.Mat[1]->EcoFol[1]
					// Poly->UnderstoryDissolve[1]
					// Poly->BaseMat.Covg[1]);
				Success = RenderForestryFoliage(Poly, (VertexData **)Vtx, Poly->BaseMat.Mat[1]->EcoFol[1], 1.0 - Poly->UnderstoryDissolve[1], Poly->BaseMat.Covg[1], NULL, WCS_SEEDOFFSET_UNDERSTORY1, 0, true);
				} // if
			// overstory
			if (Success && Poly->OverstoryDissolve[0] < 1.0 && Poly->BaseMat.Mat[0] && Poly->BaseMat.Mat[0]->EcoFol[0] && Poly->BaseMat.Mat[0]->EcoFol[0]->ChainList)
				{
				// render foliage for overstory material 0 using
					// Poly->BaseMat.Mat[0]->EcoFol[0]
					// Poly->OverstoryDissolve[0]
					// Poly->BaseMat.Covg[0]);
				Success = RenderForestryFoliage(Poly, (VertexData **)Vtx, Poly->BaseMat.Mat[0]->EcoFol[0], 1.0 - Poly->OverstoryDissolve[0], Poly->BaseMat.Covg[0], NULL, WCS_SEEDOFFSET_OVERSTORY0, 0, true);
				} // if
			if (Success && Poly->OverstoryDissolve[1] < 1.0 && Poly->BaseMat.Mat[1] && Poly->BaseMat.Mat[1]->EcoFol[0] && Poly->BaseMat.Mat[1]->EcoFol[0]->ChainList)
				{
				// render foliage for overstory material 1 using
					// Poly->BaseMat.Mat[1]->EcoFol[0]
					// Poly->OverstoryDissolve[1]
					// Poly->BaseMat.Covg[1]);
				Success = RenderForestryFoliage(Poly, (VertexData **)Vtx, Poly->BaseMat.Mat[1]->EcoFol[0], 1.0 - Poly->OverstoryDissolve[1], Poly->BaseMat.Covg[1], NULL, WCS_SEEDOFFSET_OVERSTORY1, 0, true);
				} // if
			#endif // WCS_FORESTRY_WIZARD
			} // if
		} // if
	#endif // WCS_VECPOLY_EFFECTS
	} // if

DeleteCropVerts:

if (LerpVtxData[0])
	delete LerpVtxData[0];
if (LerpVtxData[1])
	delete LerpVtxData[1];
if (LerpVtx3D[0])
	delete LerpVtx3D[0];
if (LerpVtx3D[1])
	delete LerpVtx3D[1];

return (Success);	// no errors

} // Renderer::RenderPolygon

/*===========================================================================*/

#ifdef WCS_VECPOLY_EFFECTS
int Renderer::InterpretLandBumpMaps(PixelData *Pix, PolygonData *Poly, double *VtxWt, long *VertIndex)
{
double OpacityStash, PixWidth, CovgRemaining, TempCovg, OldSnowCovg, SnowFactor,
	TempVec[3], SnowVec[3], EcoVec[3], GroundVec[3], MatWt[3], PrevSnowCovg, NewSnowCovg;
int MatCount;
MaterialEffect *MatToRender;
SnowEffect *PrevSnow, *CurSnow;
#else // WCS_VECPOLY_EFFECTS
int Renderer::InterpretLandBumpMaps(PixelData *Pix, PolygonData *Poly)
{
double OpacityStash, PixWidth, CovgRemaining, TempCovg, OldSnowCovg, SnowFactor,
	TempVec[3], SnowVec[3], EcoVec[3], GroundVec[3];
#endif // WCS_VECPOLY_EFFECTS
bool PixWidthComputed = false, BumpMod = false, EcoBumpMod = false, GroundBumpMod = false, SnowBumpMod = false, 
	EcoBumpsExist = false, GroundBumpsExist = false, SnowBumpsExist = false;
VertexDEM NormalVert;

// this value will be modified by bump evaluators and it needs to be reset 
// prior to other material property evaluation
OpacityStash = Pix->OpacityUsed;

#ifdef WCS_VECPOLY_EFFECTS

if (! CamPlanOrtho)
	PixWidth = CenterPixelSize * Pix->Zflt;
else
	PixWidth = CenterPixelSize;

if (MatToRender = SnowMatTable[0].Material)
	{
	SnowVec[0] = SnowVec[1] = SnowVec[2] = 0.0;
	CovgRemaining = 1.0 - Pix->OpacityUsed;
	Poly->SnowCoverage = 1.0;	// used to transmit any reduction or increase in snow coverage back to the scan converter
	for (MatCount = 0; MatToRender; MatToRender = SnowMatTable[MatCount].Material)
		{
		MatWt[0] = SnowMatTable[MatCount].MatListPtr[VertIndex[0]] ? VtxWt[0] * SnowMatTable[MatCount].MatListPtr[VertIndex[0]]->MatCoverage: 0.0;
		MatWt[1] = SnowMatTable[MatCount].MatListPtr[VertIndex[1]] ? VtxWt[1] * SnowMatTable[MatCount].MatListPtr[VertIndex[1]]->MatCoverage: 0.0;
		MatWt[2] = SnowMatTable[MatCount].MatListPtr[VertIndex[2]] ? VtxWt[2] * SnowMatTable[MatCount].MatListPtr[VertIndex[2]]->MatCoverage: 0.0;
		Pix->TexData->ExtractMatTableData(&SnowMatTable[MatCount], VertIndex);
		if ((TempCovg = CovgRemaining * (MatWt[0] + MatWt[1] + MatWt[2]) * MatToRender->EvaluateOpacity(Pix)) > 0.0)
			{
			if (MatToRender->GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP))
				{
				if (TempCovg + Pix->OpacityUsed > .9999)
					TempCovg = 1.0 - Pix->OpacityUsed;
				// bump evaluator increments Pix->OpacityUsed by TempCovg
				if (MatToRender->EvaluateTerrainBump(&RendData, Pix, TempVec, TempCovg, PixWidth, PlanetRad))
					{
					SnowBumpMod = BumpMod = true;
					SnowVec[0] += TempVec[0];
					SnowVec[1] += TempVec[1];
					SnowVec[2] += TempVec[2];
					} // if
				} // if
			else
				Pix->OpacityUsed += TempCovg;
			if (Pix->OpacityUsed >= 1.0)
				break;
			} // if
		++MatCount;
		} // for
	} // if
if (Pix->OpacityUsed < 1.0 && (MatToRender = EcoMatTable[0].Material))
	{
	EcoVec[0] = EcoVec[1] = EcoVec[2] = 0.0;
	CovgRemaining = 1.0 - Pix->OpacityUsed;
	for (MatCount = 0; MatToRender; MatToRender = EcoMatTable[MatCount].Material)
		{
		MatWt[0] = EcoMatTable[MatCount].MatListPtr[VertIndex[0]] ? VtxWt[0] * EcoMatTable[MatCount].MatListPtr[VertIndex[0]]->MatCoverage: 0.0;
		MatWt[1] = EcoMatTable[MatCount].MatListPtr[VertIndex[1]] ? VtxWt[1] * EcoMatTable[MatCount].MatListPtr[VertIndex[1]]->MatCoverage: 0.0;
		MatWt[2] = EcoMatTable[MatCount].MatListPtr[VertIndex[2]] ? VtxWt[2] * EcoMatTable[MatCount].MatListPtr[VertIndex[2]]->MatCoverage: 0.0;
		Pix->TexData->ExtractMatTableData(&EcoMatTable[MatCount], VertIndex);
		if ((TempCovg = CovgRemaining * (MatWt[0] + MatWt[1] + MatWt[2]) * MatToRender->EvaluateOpacity(Pix)) > 0.0)
			{
			if ((MatToRender->Strata && MatToRender->ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_STRATA) && MatToRender->Strata->Enabled && MatToRender->Strata->BumpLines)
				|| MatToRender->GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP))
				{
				if (TempCovg + Pix->OpacityUsed > .9999)
					TempCovg = 1.0 - Pix->OpacityUsed;
				// bump evaluator increments Pix->OpacityUsed by TempCovg
				if (MatToRender->EvaluateTerrainBump(&RendData, Pix, TempVec, TempCovg, PixWidth, PlanetRad))
					{
					EcoBumpMod = BumpMod = true;
					EcoVec[0] += TempVec[0];
					EcoVec[1] += TempVec[1];
					EcoVec[2] += TempVec[2];
					} // if
				} // if
			else
				Pix->OpacityUsed += TempCovg;
			if (Pix->OpacityUsed >= 1.0)
				break;
			} // if
		++MatCount;
		} // for
	} // if
 if (Pix->OpacityUsed < 1.0 && (MatToRender = GroundMatTable[0].Material))
	{
	GroundVec[0] = GroundVec[1] = GroundVec[2] = 0.0;
	CovgRemaining = 1.0 - Pix->OpacityUsed;
	for (MatCount = 0; MatToRender; MatToRender = GroundMatTable[MatCount].Material)
		{
		MatWt[0] = GroundMatTable[MatCount].MatListPtr[VertIndex[0]] ? VtxWt[0] * GroundMatTable[MatCount].MatListPtr[VertIndex[0]]->MatCoverage: 0.0;
		MatWt[1] = GroundMatTable[MatCount].MatListPtr[VertIndex[1]] ? VtxWt[1] * GroundMatTable[MatCount].MatListPtr[VertIndex[1]]->MatCoverage: 0.0;
		MatWt[2] = GroundMatTable[MatCount].MatListPtr[VertIndex[2]] ? VtxWt[2] * GroundMatTable[MatCount].MatListPtr[VertIndex[2]]->MatCoverage: 0.0;
		Pix->TexData->ExtractMatTableData(&GroundMatTable[MatCount], VertIndex);
		if ((TempCovg = CovgRemaining * (MatWt[0] + MatWt[1] + MatWt[2]) * MatToRender->EvaluateOpacity(Pix)) > 0.0)
			{
			if ((MatToRender->Strata && MatToRender->ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_STRATA) && MatToRender->Strata->Enabled && MatToRender->Strata->BumpLines)
				|| MatToRender->GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP))
				{
				if (TempCovg + Pix->OpacityUsed > .9999)
					TempCovg = 1.0 - Pix->OpacityUsed;
				// bump evaluator increments Pix->OpacityUsed by TempCovg
				if (MatToRender->EvaluateTerrainBump(&RendData, Pix, TempVec, TempCovg, PixWidth, PlanetRad))
					{
					GroundBumpMod = BumpMod = true;
					GroundVec[0] += TempVec[0];
					GroundVec[1] += TempVec[1];
					GroundVec[2] += TempVec[2];
					} // if
				} // if
			else
				Pix->OpacityUsed += TempCovg;
			if (Pix->OpacityUsed >= 1.0)
				break;
			} // if
		++MatCount;
		} // for
	} // if

Pix->OpacityUsed = OpacityStash;

if (BumpMod)
	{
	if (EcoBumpMod || GroundBumpMod)
		{
		if (EcoBumpMod)
			{
			Pix->Normal[0] += EcoVec[0];
			Pix->Normal[1] += EcoVec[1];
			Pix->Normal[2] += EcoVec[2];
			} // if
		if (GroundBumpMod)
			{
			Pix->Normal[0] += GroundVec[0];
			Pix->Normal[1] += GroundVec[1];
			Pix->Normal[2] += GroundVec[2];
			} // if
		UnitVector(Pix->Normal);

		// compute new slope
		Pix->TexData->Slope = GlobalApp->MathTables->ACosTab.LookupLerped(VectorAngle(Pix->XYZ, Pix->Normal)) * PiUnder180;	// in degrees

		// compute new aspect
		NormalVert.XYZ[0] = Pix->Normal[0];
		NormalVert.XYZ[1] = Pix->Normal[1];
		NormalVert.XYZ[2] = Pix->Normal[2];
		NormalVert.Lat = Pix->Lat;
		NormalVert.Lon = Pix->Lon;
		NormalVert.RotateToHome();
		Pix->TexData->Aspect = NormalVert.FindRoughAngleYfromZ();
		if (Pix->TexData->Aspect < 0.0)
			Pix->TexData->Aspect += 360.0;

		if (MatToRender = SnowMatTable[0].Material)
			{
			PrevSnow = NULL;
			NewSnowCovg = PrevSnowCovg = OldSnowCovg = 0.0;
			SnowFactor = 1.0;
			// evaluate snow material coverage given new terrain parameters
			for (MatCount = 0; MatToRender; MatToRender = SnowMatTable[MatCount].Material)
				{
				MatWt[0] = SnowMatTable[MatCount].MatListPtr[VertIndex[0]] ? VtxWt[0] * SnowMatTable[MatCount].MatListPtr[VertIndex[0]]->MatCoverage: 0.0;
				MatWt[1] = SnowMatTable[MatCount].MatListPtr[VertIndex[1]] ? VtxWt[1] * SnowMatTable[MatCount].MatListPtr[VertIndex[1]]->MatCoverage: 0.0;
				MatWt[2] = SnowMatTable[MatCount].MatListPtr[VertIndex[2]] ? VtxWt[2] * SnowMatTable[MatCount].MatListPtr[VertIndex[2]]->MatCoverage: 0.0;
				OldSnowCovg += MatWt[0] + MatWt[1] + MatWt[2];
				if (CurSnow = SnowMatTable[MatCount].MatListPtr[VertIndex[0]] ? (SnowEffect *)SnowMatTable[MatCount].MatListPtr[VertIndex[0]]->MyEffect:
					SnowMatTable[MatCount].MatListPtr[VertIndex[1]] ? (SnowEffect *)SnowMatTable[MatCount].MatListPtr[VertIndex[1]]->MyEffect:
					SnowMatTable[MatCount].MatListPtr[VertIndex[2]] ? (SnowEffect *)SnowMatTable[MatCount].MatListPtr[VertIndex[2]]->MyEffect: NULL)
					{
					if (CurSnow != PrevSnow)
						{
						Pix->TexData->ExtractMatTableData(&SnowMatTable[MatCount], VertIndex);
						NewSnowCovg += (PrevSnowCovg = CurSnow->EvaluateCoverage(&RendData, Pix));
						} // if
					else
						NewSnowCovg += PrevSnowCovg;
					PrevSnow = CurSnow;
					} // if
				++MatCount;
				} // for
			if (OldSnowCovg > 0.0)
				Poly->SnowCoverage = NewSnowCovg / OldSnowCovg;
			if (SnowBumpMod && NewSnowCovg > 0.0)
				{
				// there is still some snow and there is a snow bump vector
				if (Poly->SnowCoverage < 1.0)
					{
					// scale snow vector down
					SnowVec[0] *= Poly->SnowCoverage;
					SnowVec[1] *= Poly->SnowCoverage;
					SnowVec[2] *= Poly->SnowCoverage;
					} // if
				} // if
			} // if
		} // if
	if (SnowBumpMod)
		{
		Pix->Normal[0] += SnowVec[0];
		Pix->Normal[1] += SnowVec[1];
		Pix->Normal[2] += SnowVec[2];
		UnitVector(Pix->Normal);

		// compute new slope
		Pix->TexData->Slope = GlobalApp->MathTables->ACosTab.LookupLerped(VectorAngle(Pix->XYZ, Pix->Normal)) * PiUnder180;	// in degrees

		// compute new aspect
		NormalVert.XYZ[0] = Pix->Normal[0];
		NormalVert.XYZ[1] = Pix->Normal[1];
		NormalVert.XYZ[2] = Pix->Normal[2];
		NormalVert.Lat = Pix->Lat;
		NormalVert.Lon = Pix->Lon;
		NormalVert.RotateToHome();
		Pix->TexData->Aspect = NormalVert.FindRoughAngleYfromZ();
		if (Pix->TexData->Aspect < 0.0)
			Pix->TexData->Aspect += 360.0;
		} // else
	return (1);
	} // if

return (0);

#else // WCS_VECPOLY_EFFECTS
	
// this is the percentage of snow. It does not take into account snow material transparency
OldSnowCovg = Poly->SnowCoverage;
if (OldSnowCovg > 0.0 && Poly->SnowMat.Mat[0])
	{
	CovgRemaining = 1.0 - Pix->OpacityUsed;
	SnowVec[0] = SnowVec[1] = SnowVec[2] = 0.0;
	TempCovg = Poly->SnowMat.Covg[0] * CovgRemaining * OldSnowCovg * Poly->SnowMat.Mat[0]->EvaluateOpacity(Pix);
	if (Poly->SnowMat.Mat[0]->GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP))
		{
		if (! PixWidthComputed)
			{
			if (! CamPlanOrtho)
				PixWidth = CenterPixelSize * Pix->Zflt;
			else
				PixWidth = CenterPixelSize;
			PixWidthComputed = true;
			} // if
		if (Poly->SnowMat.Mat[0]->EvaluateTerrainBump(&RendData, Pix, SnowVec, TempCovg, PixWidth, PlanetRad))
			{
			SnowBumpMod = BumpMod = true;
			} // if
		} // if
	else
		Pix->OpacityUsed += TempCovg;
	if (Poly->SnowMat.Mat[1])
		{
		TempCovg = Poly->SnowMat.Covg[1] * CovgRemaining * OldSnowCovg * Poly->SnowMat.Mat[1]->EvaluateOpacity(Pix);
		if (Poly->SnowMat.Mat[1]->GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP))
			{
			if (! PixWidthComputed)
				{
				if (! CamPlanOrtho)
					PixWidth = CenterPixelSize * Pix->Zflt;
				else
					PixWidth = CenterPixelSize;
				PixWidthComputed = true;
				} // if
			if (Poly->SnowMat.Mat[1]->EvaluateTerrainBump(&RendData, Pix, TempVec, TempCovg, PixWidth, PlanetRad))
				{
				SnowBumpMod = BumpMod = true;
				SnowVec[0] += TempVec[0];
				SnowVec[1] += TempVec[1];
				SnowVec[2] += TempVec[2];
				} // if
			} // if
		else
			Pix->OpacityUsed += TempCovg;
		} // if
	} // if
if (Pix->OpacityUsed < 1.0 && Poly->BaseMat.Mat[0])
	{
	CovgRemaining = 1.0 - Pix->OpacityUsed;
	EcoVec[0] = EcoVec[1] = EcoVec[2] = 0.0;
	TempCovg = Poly->BaseMat.Covg[0] * CovgRemaining * Poly->BaseMat.Mat[0]->EvaluateOpacity(Pix);
	if ((Poly->BaseMat.Mat[0]->ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_STRATA) && Poly->BaseMat.Mat[0]->Strata && Poly->BaseMat.Mat[0]->Strata->Enabled && Poly->BaseMat.Mat[0]->Strata->BumpLines)
		|| Poly->BaseMat.Mat[0]->GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP))
		{
		if (! PixWidthComputed)
			{
			if (! CamPlanOrtho)
				PixWidth = CenterPixelSize * Pix->Zflt;
			else
				PixWidth = CenterPixelSize;
			PixWidthComputed = true;
			} // if
		if (Poly->BaseMat.Mat[0]->EvaluateTerrainBump(&RendData, Pix, EcoVec, TempCovg, PixWidth, PlanetRad))
			{
			EcoBumpMod = BumpMod = true;
			} // if
		} // if
	else
		Pix->OpacityUsed += TempCovg;
	if (Poly->BaseMat.Mat[1])
		{
		TempCovg = Poly->BaseMat.Covg[1] * CovgRemaining * Poly->BaseMat.Mat[1]->EvaluateOpacity(Pix);
		if ((Poly->BaseMat.Mat[1]->ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_STRATA) && Poly->BaseMat.Mat[1]->Strata && Poly->BaseMat.Mat[1]->Strata->Enabled && Poly->BaseMat.Mat[1]->Strata->BumpLines)
			|| Poly->BaseMat.Mat[1]->GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP))
			{
			if (! PixWidthComputed)
				{
				if (! CamPlanOrtho)
					PixWidth = CenterPixelSize * Pix->Zflt;
				else
					PixWidth = CenterPixelSize;
				PixWidthComputed = true;
				} // if
			if (Poly->BaseMat.Mat[1]->EvaluateTerrainBump(&RendData, Pix, TempVec, TempCovg, PixWidth, PlanetRad))
				{
				EcoBumpMod = BumpMod = true;
				EcoVec[0] += TempVec[0];
				EcoVec[1] += TempVec[1];
				EcoVec[2] += TempVec[2];
				} // if
			} // if
		else
			Pix->OpacityUsed += TempCovg;
		} // if
	} // if
if (Pix->OpacityUsed < 1.0 && Poly->GroundMat.Mat[0])
	{
	CovgRemaining = 1.0 - Pix->OpacityUsed;
	GroundVec[0] = GroundVec[1] = GroundVec[2] = 0.0;
	TempCovg = Poly->GroundMat.Covg[0] * CovgRemaining * Poly->GroundMat.Mat[0]->EvaluateOpacity(Pix);
	if ((Poly->GroundMat.Mat[0]->ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_STRATA) && Poly->GroundMat.Mat[0]->Strata && Poly->GroundMat.Mat[0]->Strata->Enabled && Poly->GroundMat.Mat[0]->Strata->BumpLines)
		|| Poly->GroundMat.Mat[0]->GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP))
		{
		if (! PixWidthComputed)
			{
			if (! CamPlanOrtho)
				PixWidth = CenterPixelSize * Pix->Zflt;
			else
				PixWidth = CenterPixelSize;
			PixWidthComputed = true;
			} // if
		if (Poly->GroundMat.Mat[0]->EvaluateTerrainBump(&RendData, Pix, GroundVec, TempCovg, PixWidth, PlanetRad))
			{
			GroundBumpMod = BumpMod = true;
			} // if
		} // if
	else
		Pix->OpacityUsed += TempCovg;
	if (Poly->GroundMat.Mat[1])
		{
		TempCovg = Poly->GroundMat.Covg[1] * CovgRemaining * Poly->GroundMat.Mat[1]->EvaluateOpacity(Pix);
		if ((Poly->GroundMat.Mat[1]->ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_STRATA) && Poly->GroundMat.Mat[1]->Strata && Poly->GroundMat.Mat[1]->Strata->Enabled && Poly->GroundMat.Mat[1]->Strata->BumpLines)
			|| Poly->GroundMat.Mat[1]->GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP))
			{
			if (! PixWidthComputed)
				{
				if (! CamPlanOrtho)
					PixWidth = CenterPixelSize * Pix->Zflt;
				else
					PixWidth = CenterPixelSize;
				PixWidthComputed = true;
				} // if
			if (Poly->GroundMat.Mat[1]->EvaluateTerrainBump(&RendData, Pix, TempVec, TempCovg, PixWidth, PlanetRad))
				{
				GroundBumpMod = BumpMod = true;
				GroundVec[0] += TempVec[0];
				GroundVec[1] += TempVec[1];
				GroundVec[2] += TempVec[2];
				} // if
			} // if
		else
			Pix->OpacityUsed += TempCovg;
		} // if
	} // if

Pix->OpacityUsed = OpacityStash;

if (BumpMod)
	{
	if (GroundBumpMod || EcoBumpMod)
		{
		if (EcoBumpMod)
			{
			Pix->Normal[0] += EcoVec[0];
			Pix->Normal[1] += EcoVec[1];
			Pix->Normal[2] += EcoVec[2];
			} // if
		if (GroundBumpMod)
			{
			Pix->Normal[0] += GroundVec[0];
			Pix->Normal[1] += GroundVec[1];
			Pix->Normal[2] += GroundVec[2];
			} // if
		UnitVector(Pix->Normal);

		// compute new slope
		Pix->TexData->Slope = GlobalApp->MathTables->ACosTab.LookupLerped(VectorAngle(Pix->XYZ, Pix->Normal)) * PiUnder180;	// in degrees

		// compute new aspect
		NormalVert.XYZ[0] = Pix->Normal[0];
		NormalVert.XYZ[1] = Pix->Normal[1];
		NormalVert.XYZ[2] = Pix->Normal[2];
		NormalVert.Lat = Pix->Lat;
		NormalVert.Lon = Pix->Lon;
		NormalVert.RotateToHome();
		Pix->TexData->Aspect = NormalVert.FindRoughAngleYfromZ();
		if (Pix->TexData->Aspect < 0.0)
			Pix->TexData->Aspect += 360.0;
		if (Poly->SnowMat.Mat[0])
			{
			// reevaluate snow coverage
			// there is no provision here for making snow pixels clear if transparency 
			// is in effect for snow
			Poly->SnowCoverage = Poly->Snow->EvaluateCoverage(&RendData, Poly);

			if (SnowBumpMod && Poly->SnowCoverage > 0.0)
				{
				if (Poly->SnowCoverage < OldSnowCovg )
					{
					SnowFactor = Poly->SnowCoverage / OldSnowCovg;
					SnowVec[0] *= SnowFactor;
					SnowVec[1] *= SnowFactor;
					SnowVec[2] *= SnowFactor;
					} // if
				Pix->Normal[0] += SnowVec[0];
				Pix->Normal[1] += SnowVec[1];
				Pix->Normal[2] += SnowVec[2];
				UnitVector(Pix->Normal);

				// compute new slope
				Pix->TexData->Slope = GlobalApp->MathTables->ACosTab.LookupLerped(VectorAngle(Pix->XYZ, Pix->Normal)) * PiUnder180;	// in degrees

				// compute new aspect
				NormalVert.XYZ[0] = Pix->Normal[0];
				NormalVert.XYZ[1] = Pix->Normal[1];
				NormalVert.XYZ[2] = Pix->Normal[2];
				NormalVert.RotateToHome();
				Pix->TexData->Aspect = NormalVert.FindRoughAngleYfromZ();
				if (Pix->TexData->Aspect < 0.0)
					Pix->TexData->Aspect += 360.0;
				} // if
			} // if there is snow
		} // if
	else // SnowBumpMod
		{
		Pix->Normal[0] += SnowVec[0];
		Pix->Normal[1] += SnowVec[1];
		Pix->Normal[2] += SnowVec[2];
		UnitVector(Pix->Normal);

		// compute new slope
		Pix->TexData->Slope = GlobalApp->MathTables->ACosTab.LookupLerped(VectorAngle(Pix->XYZ, Pix->Normal)) * PiUnder180;	// in degrees

		// compute new aspect
		NormalVert.XYZ[0] = Pix->Normal[0];
		NormalVert.XYZ[1] = Pix->Normal[1];
		NormalVert.XYZ[2] = Pix->Normal[2];
		NormalVert.Lat = Pix->Lat;
		NormalVert.Lon = Pix->Lon;
		NormalVert.RotateToHome();
		Pix->TexData->Aspect = NormalVert.FindRoughAngleYfromZ();
		if (Pix->TexData->Aspect < 0.0)
			Pix->TexData->Aspect += 360.0;
		} // else if
	return (1);
	} // if

return (0);
#endif // WCS_VECPOLY_EFFECTS

} // Renderer::InterpretLandBumpMaps

/*===========================================================================*/

int Renderer::InterpretFenceBumpMaps(PixelData *Pix, PolygonData *Poly)
{
double OpacityStash, PixWidth, CovgRemaining, TempCovg,
	TempVec[3], GroundVec[3];
int PixWidthComputed = 0, BumpMod = 0;
VertexDEM NormalVert;

// this value will be modified by bump evaluators and it needs to be reset 
// prior to other material property evaluation
OpacityStash = Pix->OpacityUsed;
// this is the percentage of snow. It does not take into account snow material transparency

if (Poly->GroundMat.Mat[0])
	{
	CovgRemaining = 1.0 - Pix->OpacityUsed;
	GroundVec[0] = GroundVec[1] = GroundVec[2] = 0.0;
	TempCovg = Poly->GroundMat.Covg[0] * CovgRemaining * Poly->GroundMat.Mat[0]->EvaluateOpacity(Pix);
	if ((Poly->GroundMat.Mat[0]->ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_STRATA) && Poly->GroundMat.Mat[0]->Strata && Poly->GroundMat.Mat[0]->Strata->Enabled && Poly->GroundMat.Mat[0]->Strata->BumpLines)
		|| Poly->GroundMat.Mat[0]->GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP))
		{
		if (! PixWidthComputed)
			{
			if (! CamPlanOrtho)
				PixWidth = CenterPixelSize * Pix->Zflt;
			else
				PixWidth = CenterPixelSize;
			PixWidthComputed = 1;
			} // if
		if (Poly->GroundMat.Mat[0]->EvaluateTerrainBump(&RendData, Pix, GroundVec, TempCovg, PixWidth, PlanetRad))
			{
			BumpMod = 1;
			} // if
		} // if
	else
		Pix->OpacityUsed += TempCovg;
	if (Poly->GroundMat.Mat[1])
		{
		TempCovg = Poly->GroundMat.Covg[1] * CovgRemaining * Poly->GroundMat.Mat[1]->EvaluateOpacity(Pix);
		if ((Poly->GroundMat.Mat[1]->ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_STRATA) && Poly->GroundMat.Mat[1]->Strata && Poly->GroundMat.Mat[1]->Strata->Enabled && Poly->GroundMat.Mat[1]->Strata->BumpLines)
			|| Poly->GroundMat.Mat[1]->GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP))
			{
			if (! PixWidthComputed)
				{
				if (! CamPlanOrtho)
					PixWidth = CenterPixelSize * Pix->Zflt;
				else
					PixWidth = CenterPixelSize;
				PixWidthComputed = 1;
				} // if
			if (Poly->GroundMat.Mat[1]->EvaluateTerrainBump(&RendData, Pix, TempVec, TempCovg, PixWidth, PlanetRad))
				{
				BumpMod = 1;
				GroundVec[0] += TempVec[0];
				GroundVec[1] += TempVec[1];
				GroundVec[2] += TempVec[2];
				} // if
			} // if
		else
			Pix->OpacityUsed += TempCovg;
		} // if
	} // if

Pix->OpacityUsed = OpacityStash;

if (BumpMod)
	{
	Pix->Normal[0] += GroundVec[0];
	Pix->Normal[1] += GroundVec[1];
	Pix->Normal[2] += GroundVec[2];
	UnitVector(Pix->Normal);

	// compute new slope
	Pix->TexData->Slope = acos(VectorAngle(Pix->XYZ, Pix->Normal)) * (1.0 / PiOver180);	// in degrees

	// compute new aspect
	NormalVert.XYZ[0] = Pix->Normal[0];
	NormalVert.XYZ[1] = Pix->Normal[1];
	NormalVert.XYZ[2] = Pix->Normal[2];
	NormalVert.Lat = Pix->Lat;
	NormalVert.Lon = Pix->Lon;
	NormalVert.RotateToHome();
	Pix->TexData->Aspect = NormalVert.FindRoughAngleYfromZ();
	if (Pix->TexData->Aspect < 0.0)
		Pix->TexData->Aspect += 360.0;
	// slope and aspect may have been changed
	Poly->Aspect = Pix->TexData->Aspect;
	Poly->Slope = Pix->TexData->Slope;
	return (1);
	} // if

return (0);

} // Renderer::InterpretFenceBumpMaps

/*===========================================================================*/

#ifdef WCS_VECPOLY_EFFECTS
int Renderer::InterpretWaterBumpMaps(PixelData *Pix, double *VtxWt, long *VertIndex)
{
double OpacityStash, CovgRemaining, TempCovg, MatWt[3], PixWidth, TempVec[3], WaterVec[3];
MaterialEffect *MatToRender;
int MatCount, rVal = 0;
#else // WCS_VECPOLY_EFFECTS
int Renderer::InterpretWaterBumpMaps(PixelData *Pix, PolygonData *Poly)
{
double OpacityStash, PixWidth, TempVec[3], WaterVec[3];
#endif // WCS_VECPOLY_EFFECTS
bool PixWidthComputed = false, BumpMod = false;

// this value will be modified by bump evaluators and it needs to be reset 
// prior to other material property evaluation
OpacityStash = Pix->OpacityUsed;

#ifdef WCS_VECPOLY_EFFECTS

if (MatToRender = WaterMatTableStart[0].Material)
	{
	WaterVec[0] = WaterVec[1] = WaterVec[2] = 0.0;
	CovgRemaining = 1.0 - Pix->OpacityUsed;
	for (MatCount = 0; MatToRender && (&WaterMatTableStart[MatCount] != WaterMatTableEnd); MatToRender = WaterMatTableStart[MatCount].Material)
		{
		if (MatToRender->GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP))
			{
			MatWt[0] = VtxWt[0] * WaterMatTableStart[MatCount].MatListPtr[VertIndex[0]]->MatCoverage;
			MatWt[1] = VtxWt[1] * WaterMatTableStart[MatCount].MatListPtr[VertIndex[1]]->MatCoverage;
			MatWt[2] = VtxWt[2] * WaterMatTableStart[MatCount].MatListPtr[VertIndex[2]]->MatCoverage;
			if ((TempCovg = CovgRemaining * (MatWt[0] + MatWt[1] + MatWt[2])) > 0.0)
				{
				if (TempCovg + Pix->OpacityUsed > .9999)
					TempCovg = 1.0 - Pix->OpacityUsed;
				if (! PixWidthComputed)
					{
					if (! CamPlanOrtho)
						PixWidth = CenterPixelSize * Pix->Zflt;
					else
						PixWidth = CenterPixelSize;
					PixWidthComputed = true;
					} // if
				// bump evaluator increments Pix->OpacityUsed by TempCovg
				Pix->TexData->ExtractMatTableData(&WaterMatTableStart[MatCount], VertIndex);
				if (MatToRender->EvaluateTerrainBump(&RendData, Pix, TempVec, TempCovg, PixWidth, PlanetRad))
					{
					BumpMod = true;
					WaterVec[0] += TempVec[0];
					WaterVec[1] += TempVec[1];
					WaterVec[2] += TempVec[2];
					} // if
				if (Pix->OpacityUsed >= 1.0)
					break;
				} // if
			} // if
		++MatCount;
		} // for
	} // if

#else // WCS_VECPOLY_EFFECTS
	
if (Poly->BaseMat.Mat[0])
	{
	WaterVec[0] = WaterVec[1] = WaterVec[2] = 0.0;
	if (Poly->BaseMat.Mat[0]->GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP))
		{
		if (! PixWidthComputed)
			{
			if (! CamPlanOrtho)
				PixWidth = CenterPixelSize * Pix->Zflt;
			else
				PixWidth = CenterPixelSize;
			PixWidthComputed = true;
			} // if
		if (Poly->BaseMat.Mat[0]->EvaluateTerrainBump(&RendData, Pix, WaterVec, Poly->BaseMat.Covg[0], PixWidth, PlanetRad))
			{
			BumpMod = true;
			} // if
		} // if
	if (Poly->BaseMat.Mat[1])
		{
		if (Poly->BaseMat.Mat[1]->GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP))
			{
			if (! PixWidthComputed)
				{
				if (! CamPlanOrtho)
					PixWidth = CenterPixelSize * Pix->Zflt;
				else
					PixWidth = CenterPixelSize;
				PixWidthComputed = true;
				} // if
			if (Poly->BaseMat.Mat[1]->EvaluateTerrainBump(&RendData, Pix, TempVec, Poly->BaseMat.Covg[1], PixWidth, PlanetRad))
				{
				BumpMod = true;
				WaterVec[0] += TempVec[0];
				WaterVec[1] += TempVec[1];
				WaterVec[2] += TempVec[2];
				} // if
			} // if
		} // if
	} // if
#endif // WCS_VECPOLY_EFFECTS

Pix->OpacityUsed = OpacityStash;

if (BumpMod)
	{
	Pix->Normal[0] += WaterVec[0];
	Pix->Normal[1] += WaterVec[1];
	Pix->Normal[2] += WaterVec[2];
	UnitVector(Pix->Normal);
	rVal = 1;
	} // if

return(rVal);

} // Renderer::InterpretWaterBumpMaps

/*===========================================================================*/

int Renderer::Interpret3DObjectBumpMaps(PixelData *Pix, PolygonData *Poly)
{
double OpacityStash, PixWidth, BaseVec[3];
int BumpMod = 0, rVal = 0;

// this value will be modified by bump evaluators and it needs to be reset 
// prior to other material property evaluation
OpacityStash = Pix->OpacityUsed;

if (Poly->BaseMat.Mat[0])
	{
	BaseVec[0] = BaseVec[1] = BaseVec[2] = 0.0;
	if (Poly->BaseMat.Mat[0]->GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP))
		{
		if (! CamPlanOrtho)
			PixWidth = CenterPixelSize * Pix->Zflt;
		else
			PixWidth = CenterPixelSize;
		if (Poly->BaseMat.Mat[0]->Evaluate3DBump(&RendData, Pix, BaseVec, Poly->BaseMat.Covg[0], PixWidth, (Object3DEffect *)Poly->Object))
			{
			BumpMod = 1;
			} // if
		} // if
	} // if

Pix->OpacityUsed = OpacityStash;

if (BumpMod)
	{
	Pix->Normal[0] += BaseVec[0];
	Pix->Normal[1] += BaseVec[1];
	Pix->Normal[2] += BaseVec[2];
	UnitVector(Pix->Normal);
	rVal = 1;
	} // if

return(rVal);

} // Renderer::Interpret3DObjectBumpMaps

/*===========================================================================*/

void Renderer::PlotCloudPixel(PixelData *Pix, PolygonData *Poly, long X, long Y, long PixZip, double NewWt, unsigned long TopPixCovg, unsigned long BotPixCovg)
{
double SumWt, InvSumWt;
bool ReplaceValues;

if (Pix->OpacityUsed <= 0.0 && (! rPixelFragMap && CloudAABuf[PixZip]))
	return;

if (Pix->OpacityUsed > 0.0)
	{
	// make surface normal point toward camera
	Pix->Normal[0] = Pix->ViewVec[0];
	Pix->Normal[1] = Pix->ViewVec[1];
	Pix->Normal[2] = Pix->ViewVec[2];
	// needs to be normalized since it will be put in Normal XYZ buffers
	//UnitVector(Pix->Normal); // no longer necessary, since ViewVec is pre-normalized
	Pix->Translucency = (1.0 - Pix->OpacityUsed);
	Pix->Translucency *= Pix->Translucency * ((CloudEffect *)Pix->Object)->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BACKLIGHTPCT].CurValue;	//5;
	Pix->TranslucencyExp = ((CloudEffect *)Pix->Object)->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BACKLIGHTEXP].CurValue;	//3.0;
	Pix->AltBacklightColorPct = ((CloudEffect *)Pix->Object)->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_ALTBACKLIGHTCOLORPCT].CurValue;
	Pix->AltBacklightColor[0] = ((CloudEffect *)Pix->Object)->CompleteBacklightColor[0];
	Pix->AltBacklightColor[1] = ((CloudEffect *)Pix->Object)->CompleteBacklightColor[1];
	Pix->AltBacklightColor[2] = ((CloudEffect *)Pix->Object)->CompleteBacklightColor[2];
	IlluminatePixel(Pix);
	} // if some cloud opacity

NewWt *= Pix->OpacityUsed;

if (! rPixelFragMap)
	{
	if (Pix->RGB[0] > 1.0)
		Pix->RGB[0] = 1.0;
	if (Pix->RGB[1] > 1.0)
		Pix->RGB[1] = 1.0;
	if (Pix->RGB[2] > 1.0)
		Pix->RGB[2] = 1.0;
	if (Pix->Zflt <= CloudZBuf[PixZip])
		{
		ReplaceValues = true;
		// old code was causing dark pixels to appear in clouds when polygons overlapped
		// and the new polygon had very little weight but was dark in color.
		// the present technique seems to solve the problem - changed 8/26/00 by grh.
		if (NewWt + CloudAABuf[PixZip] > 255.99)
			{
			CloudAABuf[PixZip] = (unsigned char)(255.99 - NewWt);
			} // if
		if ((SumWt = NewWt + CloudAABuf[PixZip]) > 0.0)
			{
			InvSumWt = 1.0 / SumWt;
			CloudBitmap[0][PixZip] = (unsigned char)((Pix->RGB[0] * 255.99 * NewWt + CloudBitmap[0][PixZip] * CloudAABuf[PixZip]) * InvSumWt);
			CloudBitmap[1][PixZip] = (unsigned char)((Pix->RGB[1] * 255.99 * NewWt + CloudBitmap[1][PixZip] * CloudAABuf[PixZip]) * InvSumWt);
			CloudBitmap[2][PixZip] = (unsigned char)((Pix->RGB[2] * 255.99 * NewWt + CloudBitmap[2][PixZip] * CloudAABuf[PixZip]) * InvSumWt);
			} // if
		} // if
	else
		{
		// new pixel is farther away and we know that AABuf is not full so blend the values
		ReplaceValues = false;
		if (NewWt + CloudAABuf[PixZip] > 255.99)
			NewWt = 255.99 -  CloudAABuf[PixZip];
		if ((SumWt = NewWt + CloudAABuf[PixZip]) > 0.0)
			{
			InvSumWt = 1.0 / SumWt;
			CloudBitmap[0][PixZip] = (unsigned char)((Pix->RGB[0] * 255.99 * NewWt + CloudBitmap[0][PixZip] * CloudAABuf[PixZip]) * InvSumWt);
			CloudBitmap[1][PixZip] = (unsigned char)((Pix->RGB[1] * 255.99 * NewWt + CloudBitmap[1][PixZip] * CloudAABuf[PixZip]) * InvSumWt);
			CloudBitmap[2][PixZip] = (unsigned char)((Pix->RGB[2] * 255.99 * NewWt + CloudBitmap[2][PixZip] * CloudAABuf[PixZip]) * InvSumWt);
			} // if
		} // else need to blend for sure
	SumWt = WCS_ceil(SumWt);
	if (SumWt >= 254.5)
		CloudAABuf[PixZip] = 255;
	else
		CloudAABuf[PixZip] = max((unsigned char)(SumWt), 1);	// max was added 2/12/00 to make a solid cloud to stop reflection streaks
	} // if
else
	{
	ReplaceValues = (Pix->Zflt <= rPixelFragMap[PixZip].GetFirstZ());
	} // else
if (ReplaceValues)
	{
	if (! rPixelFragMap)
		CloudZBuf[PixZip] = (float)(Pix->Zflt);
	if (LatBuf)
		LatBuf[PixZip] = (float)(Pix->Lat - TexRefLat);
	if (LonBuf)
		LonBuf[PixZip] = (float)(Pix->Lon - TexRefLon);
	if (ElevBuf)
		ElevBuf[PixZip] = (float)Pix->Elev;
	if (IllumBuf)
		IllumBuf[PixZip] = (float)Pix->Illum;
	if (NormalBuf[0])
		NormalBuf[0][PixZip] = (float)Pix->Normal[0];
	if (NormalBuf[1])
		NormalBuf[1][PixZip] = (float)Pix->Normal[1];
	if (NormalBuf[2])
		NormalBuf[2][PixZip] = (float)Pix->Normal[2];
	if (ObjectBuf)
		ObjectBuf[PixZip] = Poly->Object;
	if (ObjTypeBuf)
		ObjTypeBuf[PixZip] = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_CLOUD;
	} // if

if (rPixelFragMap)
	{
	rPixelFragment *PixFrag;
	int PixWt;

	if ((PixWt = (int)ceil(NewWt)) > 0)
		{
		if (PixWt > 255)
			PixWt = 255;
		if (PixFrag = rPixelFragMap[PixZip].PlotPixel(rPixelBlock, (float)Pix->Zflt, (unsigned char)PixWt, TopPixCovg, BotPixCovg, FragmentDepth, 
			(unsigned char)WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_CLOUD))
			{
			PixFrag->PlotPixel(rPixelBlock, Pix->RGB, Pix->Reflectivity, Pix->Normal);
			ScreenPixelPlotFragments(&rPixelFragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);
			} // if
		} // if
	} // if

if (! rPixelFragMap)
	ScreenPixelPlotTwoBuf(Bitmap, ZBuf, AABuf, CloudBitmap, CloudZBuf, CloudAABuf, X + DrawOffsetX, Y + DrawOffsetY, PixZip);

} // Renderer::PlotCloudPixel

/*===========================================================================*/

void Renderer::PlotWaterPixel(PixelData *Pix, PolygonData *Poly, long X, long Y, long PixZip, double NewWt, unsigned long TopPixCovg, unsigned long BotPixCovg)
{
double SumWt, InvSumWt, DistDiff;
bool ReplaceValues;

// plot values to buffers
	// need values for RGB, AA, Z, other buffers as allocated for output or later use
	// discriminate based on Z and AA values
if (Pix->Transparency < 1.0)
	{
	NewWt *= (1.0 - Pix->Transparency);
	if (NewWt < .00001)
		NewWt = 0.0;

	if (NewWt > 0.0)
		{
		// Apply lighting and atmospherics
		// Need the view vector for specularity calculation, 
		// they rely on the vector being from the pixel to the camera.
		// IlluminatePixel will fill in the Pix->Illum field.
		IlluminatePixel(Pix);

		if (! rPixelFragMap)
			{
			// clip color to allowable range
			if (Pix->RGB[0] > 1.0)
				Pix->RGB[0] = 1.0;
			if (Pix->RGB[1] > 1.0)
				Pix->RGB[1] = 1.0;
			if (Pix->RGB[2] > 1.0)
				Pix->RGB[2] = 1.0;

			if (Pix->Zflt < ZBuf[PixZip])
				{
				ReplaceValues = true;
				if (FlagBuf[PixZip])
					{
					if ((DistDiff = ZBuf[PixZip] - Pix->Zflt) < ZMergeDistance)
						{
						if (NewWt + AABuf[PixZip] > 255.99)
							{
							AABuf[PixZip] = (unsigned char)(AABuf[PixZip] * (1.0 - (DistDiff / ZMergeDistance)));
							if (NewWt + AABuf[PixZip] < 255.99)
								AABuf[PixZip] = (unsigned char)(255.99 - NewWt);
							} // if
						SumWt = NewWt + AABuf[PixZip];
						InvSumWt = 1.0 / SumWt;
						Bitmap[0][PixZip] = (unsigned char)((Pix->RGB[0] * 255.99 * NewWt + Bitmap[0][PixZip] * AABuf[PixZip]) * InvSumWt);
						Bitmap[1][PixZip] = (unsigned char)((Pix->RGB[1] * 255.99 * NewWt + Bitmap[1][PixZip] * AABuf[PixZip]) * InvSumWt);
						Bitmap[2][PixZip] = (unsigned char)((Pix->RGB[2] * 255.99 * NewWt + Bitmap[2][PixZip] * AABuf[PixZip]) * InvSumWt);
						} // if need to compute a blend factor
					else
						{
						SumWt = NewWt + AABuf[PixZip];
						Bitmap[0][PixZip] = (unsigned char)(Pix->RGB[0] * 255.99);
						Bitmap[1][PixZip] = (unsigned char)(Pix->RGB[1] * 255.99);
						Bitmap[2][PixZip] = (unsigned char)(Pix->RGB[2] * 255.99);
						} // just a straight replacement
					} // if water pixel already
				else
					{
					SumWt = NewWt + AABuf[PixZip];
					Bitmap[0][PixZip] = (unsigned char)(Pix->RGB[0] * 255.99);
					Bitmap[1][PixZip] = (unsigned char)(Pix->RGB[1] * 255.99);
					Bitmap[2][PixZip] = (unsigned char)(Pix->RGB[2] * 255.99);
					} // terrain pixel or nothing
				} // if
			else
				{
				// FlagBuf[PixZip] means it is a water pixel already
				if (FlagBuf[PixZip] || (Pix->Zflt - ZBuf[PixZip] > 8 * ZMergeDistance))
					{
					// new pixel is farther away and we know that AABuf is not full so blend the values
					if (NewWt + AABuf[PixZip] > 255.99)
						NewWt = 255.99 -  AABuf[PixZip];
					SumWt = NewWt + AABuf[PixZip];
					InvSumWt = 1.0 / SumWt;
					if (Pix->Reflectivity > 0.0 && ReflectionBuf && ! ReflectionBuf[PixZip])
						{
						Pix->Reflectivity *= NewWt * InvSumWt;
						ReplaceValues = true;
						} // if
					else
						ReplaceValues = false;
					Bitmap[0][PixZip] = (unsigned char)((Pix->RGB[0] * 255.99 * NewWt + Bitmap[0][PixZip] * AABuf[PixZip]) * InvSumWt);
					Bitmap[1][PixZip] = (unsigned char)((Pix->RGB[1] * 255.99 * NewWt + Bitmap[1][PixZip] * AABuf[PixZip]) * InvSumWt);
					Bitmap[2][PixZip] = (unsigned char)((Pix->RGB[2] * 255.99 * NewWt + Bitmap[2][PixZip] * AABuf[PixZip]) * InvSumWt);
					} // if water pixel already
				else
					{
					// water pixel is farther away than ground pixel so we don't want to blend
					// but we need to make sure that a hole doesn't appear if for instance water is
					// being rendered behind the edge of a cliff which doesn't completely cover the pixel
					SumWt = NewWt + AABuf[PixZip];
					ReplaceValues = false;
					} // else
				} // else need to blend for sure
			} // if
		else
			{
			ReplaceValues = (Pix->Zflt <= rPixelFragMap[PixZip].GetFirstZ());
			} // else
		if (ReplaceValues)
			{
			if (! rPixelFragMap)
				{
				FlagBuf[PixZip] = WCS_RENDER_FLAGBUFBIT_WATER;
				ZBuf[PixZip] = (float)Pix->Zflt;
				if (ReflectionBuf && Pix->Reflectivity > 0.0)
					{
					ReflectionBuf[PixZip] = (float)(Pix->Reflectivity);
					if (NormalBuf[0])
						NormalBuf[0][PixZip] = (float)Pix->Normal[0];
					if (NormalBuf[1])
						NormalBuf[1][PixZip] = (float)Pix->Normal[1];
					if (NormalBuf[2])
						NormalBuf[2][PixZip] = (float)Pix->Normal[2];
					} // if
				} // if
			if (rPixelFragMap || Pix->Zflt < TreeZBuf[PixZip])
				{
				if (LatBuf)
					LatBuf[PixZip] = (float)(Pix->Lat - TexRefLat);
				if (LonBuf)
					LonBuf[PixZip] = (float)(Pix->Lon - TexRefLon);
				if (ElevBuf)
					ElevBuf[PixZip] = (float)Pix->Elev;
				if (IllumBuf)
					IllumBuf[PixZip] = (float)Pix->Illum;
				if (RelElBuf)
					#ifdef WCS_DIAGNOSTIC_POLYNUMBERASRELEL
					RelElBuf[PixZip] = (float)CurPolyNumber;
					#else // WCS_DIAGNOSTIC_POLYNUMBERASRELEL
					RelElBuf[PixZip] = (float)Pix->TexData->RelElev;
					#endif // WCS_DIAGNOSTIC_POLYNUMBERASRELEL
				if (SlopeBuf)
					SlopeBuf[PixZip] = (float)Pix->TexData->Slope;
				if (AspectBuf)
					AspectBuf[PixZip] = (float)Pix->TexData->Aspect;
				if (NormalBuf[0])
					NormalBuf[0][PixZip] = (float)Pix->Normal[0];
				if (NormalBuf[1])
					NormalBuf[1][PixZip] = (float)Pix->Normal[1];
				if (NormalBuf[2])
					NormalBuf[2][PixZip] = (float)Pix->Normal[2];
				if (ObjectBuf)
					ObjectBuf[PixZip] = Poly->Object;
				if (ObjTypeBuf)
					ObjTypeBuf[PixZip] = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_WATER;
				} // if
			} // if

		if (rPixelFragMap)
			{
			rPixelFragment *PixFrag;
			int PixWt;

			if (RenderBathos && Pix->OpticalDepth > 0.0)
				{
				PixWt = (int)(50 + 14.42695041 * log(Pix->OpticalDepth));
				PixWt = PixWt > 0 ? (int)WCS_min((double)PixWt, 255.0): 0;
				PixWt = 255 - PixWt;
				} // if
			else
				PixWt = 255;
			if (PixFrag = rPixelFragMap[PixZip].PlotPixel(rPixelBlock, (float)Pix->Zflt, (unsigned char)PixWt, TopPixCovg, BotPixCovg, FragmentDepth, 
				WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_WATER | WCS_PIXELFRAG_FLAGBIT_OPTICALDEPTH))
				{
				PixFrag->PlotPixel(rPixelBlock, Pix->RGB, Pix->Reflectivity, Pix->Normal);
				ScreenPixelPlotFragments(&rPixelFragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);
				} // if
			} // if

		if (! rPixelFragMap)
			{
			SumWt = WCS_ceil(SumWt);
			if (SumWt >= 254.5)
				AABuf[PixZip] = 255;
			else
				AABuf[PixZip] = (unsigned char)(SumWt);
			ScreenPixelPlotTwoBuf(Bitmap, ZBuf, AABuf, TreeBitmap, TreeZBuf, TreeAABuf, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
			} // if
		} // if
	} // if not completely transparent

} // Renderer::PlotWaterPixel

/*===========================================================================*/

void Renderer::PlotTerrainPixel(PixelData *Pix, PolygonData *Poly, long X, long Y, long PixZip, double NewWt, unsigned long TopPixCovg, unsigned long BotPixCovg)
{
double SumWt, InvSumWt, DistDiff;
int ReplaceValues;

// plot values to buffers
	// need values for RGB, AA, Z, other buffers as allocated for output or later use
	// discriminate based on Z and AA values
if (Pix->Transparency < 1.0)
	{
	NewWt *= (1.0 - Pix->Transparency);
	if (NewWt < .00001)
		NewWt = 0.0;

	if (NewWt > 0.0)
		{
		// Apply lighting and atmospherics
		// Need the view vector for specularity calculation, 
		// they rely on the vector being from the pixel to the camera.
		// IlluminatePixel will fill in the Pix->Illum field.
		IlluminatePixel(Pix);

		if (! rPixelFragMap)
			{
			// clip color to allowable range
			if (Pix->RGB[0] > 1.0)
				Pix->RGB[0] = 1.0;
			if (Pix->RGB[1] > 1.0)
				Pix->RGB[1] = 1.0;
			if (Pix->RGB[2] > 1.0)
				Pix->RGB[2] = 1.0;

			if (Pix->Zflt < ZBuf[PixZip])
				{
				ReplaceValues = true;
				if (! FlagBuf[PixZip])	// FlagBuf[PixZip] = 1 for water
					{
					if ((DistDiff = ZBuf[PixZip] - Pix->Zflt) < ZMergeDistance)
						{
						if (NewWt + AABuf[PixZip] > 255.99)
							{
							AABuf[PixZip] = (unsigned char)(AABuf[PixZip] * (1.0 - (DistDiff / ZMergeDistance)));
							if (NewWt + AABuf[PixZip] < 255.99)
								AABuf[PixZip] = (unsigned char)(255.99 - NewWt);
							} // if
						SumWt = NewWt + AABuf[PixZip];
						InvSumWt = 1.0 / SumWt;
						Bitmap[0][PixZip] = (unsigned char)((Pix->RGB[0] * 255.99 * NewWt + Bitmap[0][PixZip] * AABuf[PixZip]) * InvSumWt);
						Bitmap[1][PixZip] = (unsigned char)((Pix->RGB[1] * 255.99 * NewWt + Bitmap[1][PixZip] * AABuf[PixZip]) * InvSumWt);
						Bitmap[2][PixZip] = (unsigned char)((Pix->RGB[2] * 255.99 * NewWt + Bitmap[2][PixZip] * AABuf[PixZip]) * InvSumWt);
						} // if need to compute a blend factor
					else
						{
						SumWt = NewWt + AABuf[PixZip];
						Bitmap[0][PixZip] = (unsigned char)(Pix->RGB[0] * 255.99);
						Bitmap[1][PixZip] = (unsigned char)(Pix->RGB[1] * 255.99);
						Bitmap[2][PixZip] = (unsigned char)(Pix->RGB[2] * 255.99);
						} // just a straight replacement
					} // if water pixel already
				else
					{
					SumWt = NewWt + AABuf[PixZip];
					Bitmap[0][PixZip] = (unsigned char)(Pix->RGB[0] * 255.99);
					Bitmap[1][PixZip] = (unsigned char)(Pix->RGB[1] * 255.99);
					Bitmap[2][PixZip] = (unsigned char)(Pix->RGB[2] * 255.99);
					} // terrain pixel or nothing
				} // if
			else
				{
				ReplaceValues = false;
				// FlagBuf[PixZip] = 1 for water
				// no test for distance, only merge colors if the pixel is already terrain.
				if (! FlagBuf[PixZip])
					{
					// new pixel is farther away and we know that AABuf is not full so blend the values
					if (NewWt + AABuf[PixZip] > 255.99)
						NewWt = 255.99 -  AABuf[PixZip];
					SumWt = NewWt + AABuf[PixZip];
					InvSumWt = 1.0 / SumWt;
					Bitmap[0][PixZip] = (unsigned char)((Pix->RGB[0] * 255.99 * NewWt + Bitmap[0][PixZip] * AABuf[PixZip]) * InvSumWt);
					Bitmap[1][PixZip] = (unsigned char)((Pix->RGB[1] * 255.99 * NewWt + Bitmap[1][PixZip] * AABuf[PixZip]) * InvSumWt);
					Bitmap[2][PixZip] = (unsigned char)((Pix->RGB[2] * 255.99 * NewWt + Bitmap[2][PixZip] * AABuf[PixZip]) * InvSumWt);
					} // if ground pixel already
				else
					{
					// ground pixel is farther away than water pixel so we might want to blend
					// but we need to make sure that a hole doesn't appear if for instance water is
					// being rendered behind the edge of a cliff which doesn't completely cover the pixel
					SumWt = NewWt + AABuf[PixZip];
					} // else if water pixel
				} // else need to blend for sure
			} // if
		else
			{
			ReplaceValues = (Pix->Zflt <= rPixelFragMap[PixZip].GetFirstZ());
			} // else

		if (ReplaceValues)
			{
			if (! rPixelFragMap)
				{
				FlagBuf[PixZip] = 0;
				if (ReflectionBuf)
					ReflectionBuf[PixZip] = 0.0f;
				ZBuf[PixZip] = (float)Pix->Zflt;
				} // if
			if (rPixelFragMap || Pix->Zflt < TreeZBuf[PixZip])
				{
				if (LatBuf)
					LatBuf[PixZip] = (float)(Pix->Lat - TexRefLat);
				if (LonBuf)
					LonBuf[PixZip] = (float)(Pix->Lon - TexRefLon);
				if (ElevBuf)
					ElevBuf[PixZip] = (float)Pix->Elev;
				if (IllumBuf)
					IllumBuf[PixZip] = (float)Pix->Illum;
				if (RelElBuf)
					#ifdef WCS_DIAGNOSTIC_POLYNUMBERASRELEL
					RelElBuf[PixZip] = (float)CurPolyNumber;
					#else // WCS_DIAGNOSTIC_POLYNUMBERASRELEL
					RelElBuf[PixZip] = (float)Pix->TexData->RelElev;
					#endif // WCS_DIAGNOSTIC_POLYNUMBERASRELEL
				if (SlopeBuf)
					SlopeBuf[PixZip] = (float)Pix->TexData->Slope;
				if (AspectBuf)
					AspectBuf[PixZip] = (float)Pix->TexData->Aspect;
				if (NormalBuf[0])
					NormalBuf[0][PixZip] = (float)Pix->Normal[0];
				if (NormalBuf[1])
					NormalBuf[1][PixZip] = (float)Pix->Normal[1];
				if (NormalBuf[2])
					NormalBuf[2][PixZip] = (float)Pix->Normal[2];
				if (ObjectBuf)
					ObjectBuf[PixZip] = Poly->Object;
				if (ObjTypeBuf)
					ObjTypeBuf[PixZip] = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_TERRAIN;
				} // if
			} // if

		if (rPixelFragMap)
			{
			rPixelFragment *PixFrag;
			int PixWt;

			PixWt = (int)quickftol(WCS_ceil(NewWt));
			if (PixWt > 255)
				PixWt = 255;
			if (PixFrag = rPixelFragMap[PixZip].PlotPixel(rPixelBlock, (float)Pix->Zflt, (unsigned char)PixWt, TopPixCovg, BotPixCovg, FragmentDepth, 
				WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_TERRAIN))
				{
				PixFrag->PlotPixel(rPixelBlock, Pix->RGB, Pix->Reflectivity, Pix->Normal);
				ScreenPixelPlotFragments(&rPixelFragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);
				} // if
			} // if

		if (! rPixelFragMap)
			{
			SumWt = WCS_ceil(SumWt);
			if (SumWt >= 254.5)
				AABuf[PixZip] = 255;
			else
				AABuf[PixZip] = (unsigned char)(SumWt);
			ScreenPixelPlotTwoBuf(Bitmap, ZBuf, AABuf, TreeBitmap, TreeZBuf, TreeAABuf, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
			} // if
		} // if
	} // if not completely transparent

} // Renderer::PlotTerrainPixel

/*===========================================================================*/

void Renderer::PlotFencePixel(PixelData *Pix, PolygonData *Poly, long X, long Y, long PixZip, double NewWt, unsigned long TopPixCovg, unsigned long BotPixCovg)
{
double SumWt, InvSumWt, DistDiff;
int ReplaceValues;
unsigned char FragFlags;

FragFlags = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FENCE;
#ifdef WCS_BUILD_RTX
if (Exporter)
	FragFlags |= (WCS_PIXELFRAG_FLAGBIT_RENDEROCCLUDED | WCS_PIXELFRAG_FLAGBIT_HALFFRAGLIMITED);
#endif // WCS_BUILD_RTX

// plot values to buffers
	// need values for RGB, AA, Z, other buffers as allocated for output or later use
	// discriminate based on Z and AA values
if (Pix->Transparency < 1.0)
	{
	NewWt *= (1.0 - Pix->Transparency);
	if (NewWt < .00001)
		NewWt = 0.0;

	if (NewWt > 0.0)
		{
		// Apply lighting and atmospherics
		// Need the view vector for specularity calculation, 
		// they rely on the vector being from the pixel to the camera.
		// IlluminatePixel will fill in the Pix->Illum field.
		IlluminatePixel(Pix);

		if (! rPixelFragMap)
			{
			// clip color to allowable range
			if (Pix->RGB[0] > 1.0)
				Pix->RGB[0] = 1.0;
			if (Pix->RGB[1] > 1.0)
				Pix->RGB[1] = 1.0;
			if (Pix->RGB[2] > 1.0)
				Pix->RGB[2] = 1.0;

			if (Pix->Zflt < TreeZBuf[PixZip])
				{
				ReplaceValues = true;
				if ((DistDiff = TreeZBuf[PixZip] - Pix->Zflt) < ZMergeDistance)
					{
					if (NewWt + TreeAABuf[PixZip] > 255.99)
						{
						TreeAABuf[PixZip] = (unsigned char)(TreeAABuf[PixZip] * (1.0 - (DistDiff / ZMergeDistance)));
						if (NewWt + TreeAABuf[PixZip] < 255.99)
							TreeAABuf[PixZip] = (unsigned char)(255.99 - NewWt);
						} // if
					SumWt = NewWt + TreeAABuf[PixZip];
					InvSumWt = 1.0 / SumWt;
					TreeBitmap[0][PixZip] = (unsigned char)((Pix->RGB[0] * 255.99 * NewWt + TreeBitmap[0][PixZip] * TreeAABuf[PixZip]) * InvSumWt);
					TreeBitmap[1][PixZip] = (unsigned char)((Pix->RGB[1] * 255.99 * NewWt + TreeBitmap[1][PixZip] * TreeAABuf[PixZip]) * InvSumWt);
					TreeBitmap[2][PixZip] = (unsigned char)((Pix->RGB[2] * 255.99 * NewWt + TreeBitmap[2][PixZip] * TreeAABuf[PixZip]) * InvSumWt);
					} // if need to compute a blend factor
				else
					{
					SumWt = NewWt + TreeAABuf[PixZip];
					TreeBitmap[0][PixZip] = (unsigned char)(Pix->RGB[0] * 255.99);
					TreeBitmap[1][PixZip] = (unsigned char)(Pix->RGB[1] * 255.99);
					TreeBitmap[2][PixZip] = (unsigned char)(Pix->RGB[2] * 255.99);
					} // just a straight replacement
				} // if
			else
				{
				ReplaceValues = false;
				// no test for distance, only merge colors if the pixel is already terrain.
					// new pixel is farther away and we know that AABuf is not full so blend the values
				if (NewWt + TreeAABuf[PixZip] > 255.99)
					NewWt = 255.99 -  TreeAABuf[PixZip];
				SumWt = NewWt + TreeAABuf[PixZip];
				InvSumWt = 1.0 / SumWt;
				TreeBitmap[0][PixZip] = (unsigned char)((Pix->RGB[0] * 255.99 * NewWt + TreeBitmap[0][PixZip] * TreeAABuf[PixZip]) * InvSumWt);
				TreeBitmap[1][PixZip] = (unsigned char)((Pix->RGB[1] * 255.99 * NewWt + TreeBitmap[1][PixZip] * TreeAABuf[PixZip]) * InvSumWt);
				TreeBitmap[2][PixZip] = (unsigned char)((Pix->RGB[2] * 255.99 * NewWt + TreeBitmap[2][PixZip] * TreeAABuf[PixZip]) * InvSumWt);
				} // else need to blend for sure
			} // if
		else
			{
			ReplaceValues = (Pix->Zflt <= rPixelFragMap[PixZip].GetFirstZ());
			} // else
		if (ReplaceValues)
			{
			if (! rPixelFragMap)
				TreeZBuf[PixZip] = (float)Pix->Zflt;
			if (rPixelFragMap || Pix->Zflt < ZBuf[PixZip])
				{
				if (! rPixelFragMap && ReflectionBuf)
					ReflectionBuf[PixZip] = 0.0f;
				if (LatBuf)
					LatBuf[PixZip] = (float)(Pix->Lat - TexRefLat);
				if (LonBuf)
					LonBuf[PixZip] = (float)(Pix->Lon - TexRefLon);
				if (ElevBuf)
					ElevBuf[PixZip] = (float)Pix->Elev;
				if (IllumBuf)
					IllumBuf[PixZip] = (float)Pix->Illum;
				if (RelElBuf)
					RelElBuf[PixZip] = (float)Poly->RelEl;
				if (SlopeBuf)
					SlopeBuf[PixZip] = (float)Poly->Slope;
				if (AspectBuf)
					AspectBuf[PixZip] = (float)Poly->Aspect;
				if (NormalBuf[0])
					NormalBuf[0][PixZip] = (float)Pix->Normal[0];
				if (NormalBuf[1])
					NormalBuf[1][PixZip] = (float)Pix->Normal[1];
				if (NormalBuf[2])
					NormalBuf[2][PixZip] = (float)Pix->Normal[2];
				if (ObjectBuf)
					ObjectBuf[PixZip] = Poly->Object;
				if (ObjTypeBuf)
					ObjTypeBuf[PixZip] = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FENCE;
				} // if
			} // if

		if (rPixelFragMap)
			{
			rPixelFragment *PixFrag;
			int PixWt;

			PixWt = (int)quickftol(WCS_ceil(NewWt));
			if (PixWt > 255)
				PixWt = 255;
			if (PixFrag = rPixelFragMap[PixZip].PlotPixel(rPixelBlock, (float)Pix->Zflt, (unsigned char)PixWt, TopPixCovg, BotPixCovg, FragmentDepth, 
				FragFlags))
				{
				PixFrag->PlotPixel(rPixelBlock, Pix->RGB, Pix->Reflectivity, Pix->Normal);
				ScreenPixelPlotFragments(&rPixelFragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);
				} // if
			} // if

		if (! rPixelFragMap)
			{
			SumWt = WCS_ceil(SumWt);
			if (SumWt >= 254.5)
				TreeAABuf[PixZip] = 255;
			else
				TreeAABuf[PixZip] = (unsigned char)(SumWt);
			ScreenPixelPlotTwoBuf(Bitmap, ZBuf, AABuf, TreeBitmap, TreeZBuf, TreeAABuf, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
			} // if
		} // if
	} // if not completely transparent

} // Renderer::PlotFencePixel

/*===========================================================================*/

void Renderer::Plot3DObjectPixel(rPixelBlockHeader *FragBlock, PixelData *Pix, PolygonData *Poly, long X, long Y, long LocalZip, long PixZip, double NewWt, 
	unsigned long TopPixCovg, unsigned long BotPixCovg)
{
int PixWt, ReplaceValues;
rPixelFragment *PixFrag;
unsigned char Flags;

Flags = (((GeneralEffect *)Poly->Object)->RenderOccluded
	#ifdef WCS_BUILD_RTX
	|| Exporter
	#endif // WCS_BUILD_RTX
	) ? WCS_PIXELFRAG_FLAGBIT_RENDEROCCLUDED: 0;
Flags |= WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_3DOBJECT;

IlluminatePixel(Pix);	// can modify transparency
NewWt *= (1.0 - Pix->Transparency);
if (NewWt < .00001)
	NewWt = 0.0;
PixWt = (int)WCS_ceil(NewWt);
if (PixWt > 255)
	PixWt = 255;

if (PixFrag = FragBlock->FragMap[LocalZip].PlotPixel(FragBlock, (float)Pix->Zflt, (unsigned char)PixWt, TopPixCovg, BotPixCovg, FragmentDepth, 
	Flags))
	{
	// view vector is used for specularity and for that it is best to have it point toward the camera

	if (rPixelFragMap)
		{
		ReplaceValues = (Pix->Zflt <= rPixelFragMap[PixZip].GetFirstZ() && Pix->Zflt <= FragBlock->FragMap[LocalZip].GetFirstZ());
		} // if
	else
		{
		ReplaceValues = (Pix->Zflt < ZBuf[PixZip] && Pix->Zflt < TreeZBuf[PixZip] && Pix->Zflt <= FragBlock->FragMap[LocalZip].GetFirstZ());
		} // else

	if (ReplaceValues)
		{
		// This is not necessary and actually causes conflict later on when the 3D object is composited.
		//if (! rPixelFragMap)
		//	TreeZBuf[PixZip] = (float)Pix->Zflt;
		if (LatBuf)
			LatBuf[PixZip] = (float)(Pix->Lat - TexRefLat);
		if (LonBuf)
			LonBuf[PixZip] = (float)(Pix->Lon - TexRefLon);
		if (ElevBuf)
			ElevBuf[PixZip] = (float)Pix->Elev;
		if (IllumBuf)
			IllumBuf[PixZip] = (float)Pix->Illum;
		if (RelElBuf)
			RelElBuf[PixZip] = (float)Poly->RelEl;
		if (SlopeBuf)
			SlopeBuf[PixZip] = (float)Pix->TexData->Slope;
		if (AspectBuf)
			AspectBuf[PixZip] = (float)Pix->TexData->Aspect;
		if (rPixelFragMap || (! ReflectionBuf || ! ReflectionBuf[PixZip]))
			{
			if (NormalBuf[0])
				NormalBuf[0][PixZip] = (float)Pix->Normal[0];
			if (NormalBuf[1])
				NormalBuf[1][PixZip] = (float)Pix->Normal[1];
			if (NormalBuf[2])
				NormalBuf[2][PixZip] = (float)Pix->Normal[2];
			if (ReflectionBuf)
				ReflectionBuf[PixZip] = (float)Pix->Reflectivity;
			} // if
		if (ObjectBuf)
			ObjectBuf[PixZip] = Poly->Object;
		if (ObjTypeBuf)
			ObjTypeBuf[PixZip] = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_3DOBJECT;
		} // if

	PixFrag->PlotPixel(FragBlock, Pix->RGB, Pix->Reflectivity, Pix->Normal);
	} // if

} // Renderer::Plot3DObjectPixel

/*===========================================================================*/

void Renderer::Plot3DObjectPixel(PixelData *Pix, PolygonData *Poly, long X, long Y, long PixZip, double NewWt, 
	unsigned long TopPixCovg, unsigned long BotPixCovg)
{
int PixWt, ReplaceValues;
rPixelFragment *PixFrag;
unsigned char Flags;

Flags = (((GeneralEffect *)Poly->Object)->RenderOccluded
	#ifdef WCS_BUILD_RTX
	|| Exporter
	#endif // WCS_BUILD_RTX
	) ? WCS_PIXELFRAG_FLAGBIT_RENDEROCCLUDED: 0;
Flags |= WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_3DOBJECT;
IlluminatePixel(Pix);	// can modify transparency
NewWt *= (1.0 - Pix->Transparency);
if (NewWt < .00001)
	NewWt = 0.0;
PixWt = (int)WCS_ceil(NewWt);
if (PixWt > 255)
	PixWt = 255;

if (PixFrag = rPixelFragMap[PixZip].PlotPixel(rPixelBlock, (float)Pix->Zflt, (unsigned char)PixWt, TopPixCovg, BotPixCovg, FragmentDepth, 
	Flags))
	{
	// view vector is used for specularity and for that it is best to have it point toward the camera

	if (rPixelFragMap)
		{
		ReplaceValues = (Pix->Zflt <= rPixelFragMap[PixZip].GetFirstZ());
		} // if
	else
		{
		ReplaceValues = (Pix->Zflt < ZBuf[PixZip] && Pix->Zflt < TreeZBuf[PixZip]);
		} // else

	if (ReplaceValues)
		{
		if (! rPixelFragMap)
			TreeZBuf[PixZip] = (float)Pix->Zflt;
		if (LatBuf)
			LatBuf[PixZip] = (float)(Pix->Lat - TexRefLat);
		if (LonBuf)
			LonBuf[PixZip] = (float)(Pix->Lon - TexRefLon);
		if (ElevBuf)
			ElevBuf[PixZip] = (float)Pix->Elev;
		if (IllumBuf)
			IllumBuf[PixZip] = (float)Pix->Illum;
		if (RelElBuf)
			RelElBuf[PixZip] = (float)Poly->RelEl;
		if (SlopeBuf)
			SlopeBuf[PixZip] = (float)Pix->TexData->Slope;
		if (AspectBuf)
			AspectBuf[PixZip] = (float)Pix->TexData->Aspect;
		if (rPixelFragMap || (! ReflectionBuf || ! ReflectionBuf[PixZip]))
			{
			if (NormalBuf[0])
				NormalBuf[0][PixZip] = (float)Pix->Normal[0];
			if (NormalBuf[1])
				NormalBuf[1][PixZip] = (float)Pix->Normal[1];
			if (NormalBuf[2])
				NormalBuf[2][PixZip] = (float)Pix->Normal[2];
			if (ReflectionBuf)
				ReflectionBuf[PixZip] = (float)Pix->Reflectivity;
			} // if
		if (ObjectBuf)
			ObjectBuf[PixZip] = Poly->Object;
		if (ObjTypeBuf)
			ObjTypeBuf[PixZip] = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_3DOBJECT;
		} // if

	PixFrag->PlotPixel(rPixelBlock, Pix->RGB, Pix->Reflectivity, Pix->Normal);
	ScreenPixelPlotFragments(&rPixelFragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);
	} // if

} // Renderer::Plot3DObjectPixel

/*===========================================================================*/

// This fills in most of the texture data needed for terrain or water rendering, some however is done in RenderPolygon()
// where certain other pieces of information are available or where common variables between water, terrain 
// and 3D objects are filled in.
// Note the direct reference to certain arrays and variables which are Renderer scope to minimize the arguments passed:
//   VertexWeight[], PixelWeight[], TexRefLat, TexRefLon, TexRefElev, EarthLatScaleMeters, RefLonScaleMeters

void Renderer::FillTerrainTextureData(TextureData *TexDat, VertexData *Vert[3], double CenterPixWt[3], 
	long BoxWidth, long BoxHeight, long WtZip, long Xp, long Yp, char PolyType, CloudEffect *Cloud)
{
double TempVtxPct[3], /*RangeX[4], RangeY[4], RangeZ[4], */ZCorner;
long VtxWtCt, TempZip;

if (PolyType != WCS_POLYGONTYPE_CLOUD)
	{
	TexDat->TexRefLon = TexRefLon;
	TexDat->TexRefLat = TexRefLat;
	TexDat->TexRefElev = TexRefElev;
	TexDat->MetersPerDegLon = RefLonScaleMeters;
	TexDat->MetersPerDegLat = EarthLatScaleMeters;
	} // if
else
	{
	TexDat->TexRefLon = Cloud->RefLon;
	TexDat->TexRefLat = Cloud->RefLat;
	TexDat->TexRefElev = Cloud->LowElev;
	TexDat->MetersPerDegLon = Cloud->MetersPerDegLon;
	TexDat->MetersPerDegLat = Cloud->MetersPerDegLat;
	} // else
TexDat->VDEM[0] = Vert[0];
TexDat->VDEM[1] = Vert[1];
TexDat->VDEM[2] = Vert[2];
TexDat->VData[0] = Vert[0];
TexDat->VData[1] = Vert[1];
TexDat->VData[2] = Vert[2];
TexDat->PData = NULL;
TexDat->Object3D = NULL;

// find weights of vertices at pixel corners
// upper left corner
VtxWtCt = 1;
TempVtxPct[0] = CenterPixWt[0]; 
TempVtxPct[1] = CenterPixWt[1]; 
TempVtxPct[2] = CenterPixWt[2]; 
if (Yp > 0)
	{
	TempZip = WtZip - BoxWidth;
	if (PixelWeight[TempZip])
		{
		VtxWtCt ++;
		TempVtxPct[0] += VertexWeight[0][TempZip];
		TempVtxPct[1] += VertexWeight[1][TempZip];
		TempVtxPct[2] += VertexWeight[2][TempZip];
		} // if
	if (Xp > 0)
		{
		TempZip = WtZip - 1;
		if (PixelWeight[TempZip])
			{
			VtxWtCt ++;
			TempVtxPct[0] += VertexWeight[0][TempZip];
			TempVtxPct[1] += VertexWeight[1][TempZip];
			TempVtxPct[2] += VertexWeight[2][TempZip];
			} // if
		TempZip -= BoxWidth;
		if (PixelWeight[TempZip])
			{
			VtxWtCt ++;
			TempVtxPct[0] += VertexWeight[0][TempZip];
			TempVtxPct[1] += VertexWeight[1][TempZip];
			TempVtxPct[2] += VertexWeight[2][TempZip];
			} // if
		} // if
	} // if
else if (Xp > 0)
	{
	TempZip = WtZip - 1;
	if (PixelWeight[TempZip])
		{
		VtxWtCt ++;
		TempVtxPct[0] += VertexWeight[0][TempZip];
		TempVtxPct[1] += VertexWeight[1][TempZip];
		TempVtxPct[2] += VertexWeight[2][TempZip];
		} // if
	} // if
TempVtxPct[0] /= VtxWtCt;
TempVtxPct[1] /= VtxWtCt;
TempVtxPct[2] /= VtxWtCt;
// compute values for upper left corner
ZCorner = 1.0;
if (! CamPlanOrtho)
	ZCorner /= (TempVtxPct[0] + TempVtxPct[1] + TempVtxPct[2]);

if (PolyType != WCS_POLYGONTYPE_CLOUD)
	{
	// needed for strata
	TexDat->TElevRange[0] = ZCorner * (TempVtxPct[0] * Vert[0]->Elev + TempVtxPct[1] * Vert[1]->Elev + TempVtxPct[2] * Vert[2]->Elev);
	} // if
TexDat->VtxPixCornerWt[0][0] = ZCorner * TempVtxPct[0];
TexDat->VtxPixCornerWt[0][1] = ZCorner * TempVtxPct[1];
TexDat->VtxPixCornerWt[0][2] = ZCorner * TempVtxPct[2];

// upper right corner
VtxWtCt = 1;
TempVtxPct[0] = CenterPixWt[0]; 
TempVtxPct[1] = CenterPixWt[1]; 
TempVtxPct[2] = CenterPixWt[2]; 
if (Yp > 0)
	{
	TempZip = WtZip - BoxWidth;
	if (PixelWeight[TempZip])
		{
		VtxWtCt ++;
		TempVtxPct[0] += VertexWeight[0][TempZip];
		TempVtxPct[1] += VertexWeight[1][TempZip];
		TempVtxPct[2] += VertexWeight[2][TempZip];
		} // if
	if (Xp < BoxWidth - 1)
		{
		TempZip = WtZip + 1;
		if (PixelWeight[TempZip])
			{
			VtxWtCt ++;
			TempVtxPct[0] += VertexWeight[0][TempZip];
			TempVtxPct[1] += VertexWeight[1][TempZip];
			TempVtxPct[2] += VertexWeight[2][TempZip];
			} // if
		TempZip -= BoxWidth;
		if (PixelWeight[TempZip])
			{
			VtxWtCt ++;
			TempVtxPct[0] += VertexWeight[0][TempZip];
			TempVtxPct[1] += VertexWeight[1][TempZip];
			TempVtxPct[2] += VertexWeight[2][TempZip];
			} // if
		} // if
	} // if
else if (Xp < BoxWidth - 1)
	{
	TempZip = WtZip + 1;
	if (PixelWeight[TempZip])
		{
		VtxWtCt ++;
		TempVtxPct[0] += VertexWeight[0][TempZip];
		TempVtxPct[1] += VertexWeight[1][TempZip];
		TempVtxPct[2] += VertexWeight[2][TempZip];
		} // if
	} // if
TempVtxPct[0] /= VtxWtCt;
TempVtxPct[1] /= VtxWtCt;
TempVtxPct[2] /= VtxWtCt;
// compute values for upper right corner
ZCorner = 1.0;
if (! CamPlanOrtho)
	ZCorner /= (TempVtxPct[0] + TempVtxPct[1] + TempVtxPct[2]);

if (PolyType != WCS_POLYGONTYPE_CLOUD)
	{
	TexDat->TElevRange[1] = ZCorner * (TempVtxPct[0] * Vert[0]->Elev + TempVtxPct[1] * Vert[1]->Elev + TempVtxPct[2] * Vert[2]->Elev);
	} // if
TexDat->VtxPixCornerWt[1][0] = ZCorner * TempVtxPct[0];
TexDat->VtxPixCornerWt[1][1] = ZCorner * TempVtxPct[1];
TexDat->VtxPixCornerWt[1][2] = ZCorner * TempVtxPct[2];

// lower left corner
VtxWtCt = 1;
TempVtxPct[0] = CenterPixWt[0]; 
TempVtxPct[1] = CenterPixWt[1]; 
TempVtxPct[2] = CenterPixWt[2]; 
if (Yp < BoxHeight - 1)
	{
	TempZip = WtZip + BoxWidth;
	if (PixelWeight[TempZip])
		{
		VtxWtCt ++;
		TempVtxPct[0] += VertexWeight[0][TempZip];
		TempVtxPct[1] += VertexWeight[1][TempZip];
		TempVtxPct[2] += VertexWeight[2][TempZip];
		} // if
	if (Xp > 0)
		{
		TempZip = WtZip - 1;
		if (PixelWeight[TempZip])
			{
			VtxWtCt ++;
			TempVtxPct[0] += VertexWeight[0][TempZip];
			TempVtxPct[1] += VertexWeight[1][TempZip];
			TempVtxPct[2] += VertexWeight[2][TempZip];
			} // if
		TempZip += BoxWidth;
		if (PixelWeight[TempZip])
			{
			VtxWtCt ++;
			TempVtxPct[0] += VertexWeight[0][TempZip];
			TempVtxPct[1] += VertexWeight[1][TempZip];
			TempVtxPct[2] += VertexWeight[2][TempZip];
			} // if
		} // if
	} // if
else if (Xp > 0)
	{
	TempZip = WtZip - 1;
	if (PixelWeight[TempZip])
		{
		VtxWtCt ++;
		TempVtxPct[0] += VertexWeight[0][TempZip];
		TempVtxPct[1] += VertexWeight[1][TempZip];
		TempVtxPct[2] += VertexWeight[2][TempZip];
		} // if
	} // if
TempVtxPct[0] /= VtxWtCt;
TempVtxPct[1] /= VtxWtCt;
TempVtxPct[2] /= VtxWtCt;
// compute values for lower left corner
ZCorner = 1.0;
if (! CamPlanOrtho)
	ZCorner /= (TempVtxPct[0] + TempVtxPct[1] + TempVtxPct[2]);

if (PolyType != WCS_POLYGONTYPE_CLOUD)
	{
	TexDat->TElevRange[2] = ZCorner * (TempVtxPct[0] * Vert[0]->Elev + TempVtxPct[1] * Vert[1]->Elev + TempVtxPct[2] * Vert[2]->Elev);
	} // if
TexDat->VtxPixCornerWt[2][0] = ZCorner * TempVtxPct[0];
TexDat->VtxPixCornerWt[2][1] = ZCorner * TempVtxPct[1];
TexDat->VtxPixCornerWt[2][2] = ZCorner * TempVtxPct[2];

// lower right corner
VtxWtCt = 1;
TempVtxPct[0] = CenterPixWt[0]; 
TempVtxPct[1] = CenterPixWt[1]; 
TempVtxPct[2] = CenterPixWt[2]; 
if (Yp < BoxHeight - 1)
	{
	TempZip = WtZip + BoxWidth;
	if (PixelWeight[TempZip])
		{
		VtxWtCt ++;
		TempVtxPct[0] += VertexWeight[0][TempZip];
		TempVtxPct[1] += VertexWeight[1][TempZip];
		TempVtxPct[2] += VertexWeight[2][TempZip];
		} // if
	if (Xp < BoxWidth - 1)
		{
		TempZip = WtZip + 1;
		if (PixelWeight[TempZip])
			{
			VtxWtCt ++;
			TempVtxPct[0] += VertexWeight[0][TempZip];
			TempVtxPct[1] += VertexWeight[1][TempZip];
			TempVtxPct[2] += VertexWeight[2][TempZip];
			} // if
		TempZip += BoxWidth;
		if (PixelWeight[TempZip])
			{
			VtxWtCt ++;
			TempVtxPct[0] += VertexWeight[0][TempZip];
			TempVtxPct[1] += VertexWeight[1][TempZip];
			TempVtxPct[2] += VertexWeight[2][TempZip];
			} // if
		} // if
	} // if
else if (Xp < BoxWidth - 1)
	{
	TempZip = WtZip + 1;
	if (PixelWeight[TempZip])
		{
		VtxWtCt ++;
		TempVtxPct[0] += VertexWeight[0][TempZip];
		TempVtxPct[1] += VertexWeight[1][TempZip];
		TempVtxPct[2] += VertexWeight[2][TempZip];
		} // if
	} // if
TempVtxPct[0] /= VtxWtCt;
TempVtxPct[1] /= VtxWtCt;
TempVtxPct[2] /= VtxWtCt;
// compute values for lower right corner
ZCorner = 1.0;
if (! CamPlanOrtho)
	ZCorner /= (TempVtxPct[0] + TempVtxPct[1] + TempVtxPct[2]);

if (PolyType != WCS_POLYGONTYPE_CLOUD)
	{
	TexDat->TElevRange[3] = ZCorner * (TempVtxPct[0] * Vert[0]->Elev + TempVtxPct[1] * Vert[1]->Elev + TempVtxPct[2] * Vert[2]->Elev);
	} // if
TexDat->VtxPixCornerWt[3][0] = ZCorner * TempVtxPct[0];
TexDat->VtxPixCornerWt[3][1] = ZCorner * TempVtxPct[1];
TexDat->VtxPixCornerWt[3][2] = ZCorner * TempVtxPct[2];

} // Renderer::FillTerrainTextureData

/*===========================================================================*/

void Renderer::Fill3DObjectTextureData(TextureData *TexDat, Vertex3D *Vert[3], double CenterPixWt[3], 
	long BoxWidth, long BoxHeight, long WtZip, long Xp, long Yp)
{
double TempVtxPct[3], /*RangeX[4], RangeY[4], RangeZ[4], RangeU[4], RangeV[4], RangeW[4], */ZCorner;
long VtxWtCt, TempZip;

TexDat->TexRefLon = TexRefLon;
TexDat->TexRefLat = TexRefLat;
TexDat->TexRefElev = TexRefElev;
TexDat->MetersPerDegLon = RefLonScaleMeters;
TexDat->MetersPerDegLat = EarthLatScaleMeters;
TexDat->VDEM[0] = Vert[0];
TexDat->VDEM[1] = Vert[1];
TexDat->VDEM[2] = Vert[2];
TexDat->VData[0] = NULL;
TexDat->VData[1] = NULL;
TexDat->VData[2] = NULL;
TexDat->PData = NULL;
TexDat->Object3D = (Object3DEffect *)TexDat->Object;

// find weights of vertices at pixel corners
// upper left corner
VtxWtCt = 1;
TempVtxPct[0] = CenterPixWt[0]; 
TempVtxPct[1] = CenterPixWt[1]; 
TempVtxPct[2] = CenterPixWt[2]; 
if (Yp > 0)
	{
	TempZip = WtZip - BoxWidth;
	if (PixelWeight[TempZip])
		{
		++VtxWtCt;
		TempVtxPct[0] += VertexWeight[0][TempZip];
		TempVtxPct[1] += VertexWeight[1][TempZip];
		TempVtxPct[2] += VertexWeight[2][TempZip];
		} // if
	if (Xp > 0)
		{
		TempZip = WtZip - 1;
		if (PixelWeight[TempZip])
			{
			++VtxWtCt;
			TempVtxPct[0] += VertexWeight[0][TempZip];
			TempVtxPct[1] += VertexWeight[1][TempZip];
			TempVtxPct[2] += VertexWeight[2][TempZip];
			} // if
		TempZip -= BoxWidth;
		if (PixelWeight[TempZip])
			{
			++VtxWtCt;
			TempVtxPct[0] += VertexWeight[0][TempZip];
			TempVtxPct[1] += VertexWeight[1][TempZip];
			TempVtxPct[2] += VertexWeight[2][TempZip];
			} // if
		} // if
	} // if
else if (Xp > 0)
	{
	TempZip = WtZip - 1;
	if (PixelWeight[TempZip])
		{
		++VtxWtCt;
		TempVtxPct[0] += VertexWeight[0][TempZip];
		TempVtxPct[1] += VertexWeight[1][TempZip];
		TempVtxPct[2] += VertexWeight[2][TempZip];
		} // if
	} // if
TempVtxPct[0] /= VtxWtCt;
TempVtxPct[1] /= VtxWtCt;
TempVtxPct[2] /= VtxWtCt;
// compute values for upper left corner
ZCorner = 1.0;
if (! CamPlanOrtho)
	ZCorner /= (TempVtxPct[0] + TempVtxPct[1] + TempVtxPct[2]);
TexDat->TElevRange[0] = ZCorner * (TempVtxPct[0] * Vert[0]->Elev + TempVtxPct[1] * Vert[1]->Elev + TempVtxPct[2] * Vert[2]->Elev);
TexDat->VtxPixCornerWt[0][0] = ZCorner * TempVtxPct[0];
TexDat->VtxPixCornerWt[0][1] = ZCorner * TempVtxPct[1];
TexDat->VtxPixCornerWt[0][2] = ZCorner * TempVtxPct[2];

// upper right corner
VtxWtCt = 1;
TempVtxPct[0] = CenterPixWt[0]; 
TempVtxPct[1] = CenterPixWt[1]; 
TempVtxPct[2] = CenterPixWt[2]; 
if (Yp > 0)
	{
	TempZip = WtZip - BoxWidth;
	if (PixelWeight[TempZip])
		{
		++VtxWtCt;
		TempVtxPct[0] += VertexWeight[0][TempZip];
		TempVtxPct[1] += VertexWeight[1][TempZip];
		TempVtxPct[2] += VertexWeight[2][TempZip];
		} // if
	if (Xp < BoxWidth - 1)
		{
		TempZip = WtZip + 1;
		if (PixelWeight[TempZip])
			{
			++VtxWtCt;
			TempVtxPct[0] += VertexWeight[0][TempZip];
			TempVtxPct[1] += VertexWeight[1][TempZip];
			TempVtxPct[2] += VertexWeight[2][TempZip];
			} // if
		TempZip -= BoxWidth;
		if (PixelWeight[TempZip])
			{
			++VtxWtCt;
			TempVtxPct[0] += VertexWeight[0][TempZip];
			TempVtxPct[1] += VertexWeight[1][TempZip];
			TempVtxPct[2] += VertexWeight[2][TempZip];
			} // if
		} // if
	} // if
else if (Xp < BoxWidth - 1)
	{
	TempZip = WtZip + 1;
	if (PixelWeight[TempZip])
		{
		++VtxWtCt;
		TempVtxPct[0] += VertexWeight[0][TempZip];
		TempVtxPct[1] += VertexWeight[1][TempZip];
		TempVtxPct[2] += VertexWeight[2][TempZip];
		} // if
	} // if
TempVtxPct[0] /= VtxWtCt;
TempVtxPct[1] /= VtxWtCt;
TempVtxPct[2] /= VtxWtCt;
// compute values for upper right corner
ZCorner = 1.0;
if (! CamPlanOrtho)
	ZCorner /= (TempVtxPct[0] + TempVtxPct[1] + TempVtxPct[2]);
TexDat->TElevRange[1] = ZCorner * (TempVtxPct[0] * Vert[0]->Elev + TempVtxPct[1] * Vert[1]->Elev + TempVtxPct[2] * Vert[2]->Elev);
TexDat->VtxPixCornerWt[1][0] = ZCorner * TempVtxPct[0];
TexDat->VtxPixCornerWt[1][1] = ZCorner * TempVtxPct[1];
TexDat->VtxPixCornerWt[1][2] = ZCorner * TempVtxPct[2];

// lower left corner
VtxWtCt = 1;
TempVtxPct[0] = CenterPixWt[0]; 
TempVtxPct[1] = CenterPixWt[1]; 
TempVtxPct[2] = CenterPixWt[2]; 
if (Yp < BoxHeight - 1)
	{
	TempZip = WtZip + BoxWidth;
	if (PixelWeight[TempZip])
		{
		++VtxWtCt;
		TempVtxPct[0] += VertexWeight[0][TempZip];
		TempVtxPct[1] += VertexWeight[1][TempZip];
		TempVtxPct[2] += VertexWeight[2][TempZip];
		} // if
	if (Xp > 0)
		{
		TempZip = WtZip - 1;
		if (PixelWeight[TempZip])
			{
			++VtxWtCt;
			TempVtxPct[0] += VertexWeight[0][TempZip];
			TempVtxPct[1] += VertexWeight[1][TempZip];
			TempVtxPct[2] += VertexWeight[2][TempZip];
			} // if
		TempZip += BoxWidth;
		if (PixelWeight[TempZip])
			{
			++VtxWtCt;
			TempVtxPct[0] += VertexWeight[0][TempZip];
			TempVtxPct[1] += VertexWeight[1][TempZip];
			TempVtxPct[2] += VertexWeight[2][TempZip];
			} // if
		} // if
	} // if
else if (Xp > 0)
	{
	TempZip = WtZip - 1;
	if (PixelWeight[TempZip])
		{
		++VtxWtCt;
		TempVtxPct[0] += VertexWeight[0][TempZip];
		TempVtxPct[1] += VertexWeight[1][TempZip];
		TempVtxPct[2] += VertexWeight[2][TempZip];
		} // if
	} // if
TempVtxPct[0] /= VtxWtCt;
TempVtxPct[1] /= VtxWtCt;
TempVtxPct[2] /= VtxWtCt;
// compute values for lower left corner
ZCorner = 1.0;
if (! CamPlanOrtho)
	ZCorner /= (TempVtxPct[0] + TempVtxPct[1] + TempVtxPct[2]);
TexDat->TElevRange[2] = ZCorner * (TempVtxPct[0] * Vert[0]->Elev + TempVtxPct[1] * Vert[1]->Elev + TempVtxPct[2] * Vert[2]->Elev);
TexDat->VtxPixCornerWt[2][0] = ZCorner * TempVtxPct[0];
TexDat->VtxPixCornerWt[2][1] = ZCorner * TempVtxPct[1];
TexDat->VtxPixCornerWt[2][2] = ZCorner * TempVtxPct[2];

// lower right corner
VtxWtCt = 1;
TempVtxPct[0] = CenterPixWt[0]; 
TempVtxPct[1] = CenterPixWt[1]; 
TempVtxPct[2] = CenterPixWt[2]; 
if (Yp < BoxHeight - 1)
	{
	TempZip = WtZip + BoxWidth;
	if (PixelWeight[TempZip])
		{
		++VtxWtCt;
		TempVtxPct[0] += VertexWeight[0][TempZip];
		TempVtxPct[1] += VertexWeight[1][TempZip];
		TempVtxPct[2] += VertexWeight[2][TempZip];
		} // if
	if (Xp < BoxWidth - 1)
		{
		TempZip = WtZip + 1;
		if (PixelWeight[TempZip])
			{
			++VtxWtCt;
			TempVtxPct[0] += VertexWeight[0][TempZip];
			TempVtxPct[1] += VertexWeight[1][TempZip];
			TempVtxPct[2] += VertexWeight[2][TempZip];
			} // if
		TempZip += BoxWidth;
		if (PixelWeight[TempZip])
			{
			++VtxWtCt;
			TempVtxPct[0] += VertexWeight[0][TempZip];
			TempVtxPct[1] += VertexWeight[1][TempZip];
			TempVtxPct[2] += VertexWeight[2][TempZip];
			} // if
		} // if
	} // if
else if (Xp < BoxWidth - 1)
	{
	TempZip = WtZip + 1;
	if (PixelWeight[TempZip])
		{
		++VtxWtCt;
		TempVtxPct[0] += VertexWeight[0][TempZip];
		TempVtxPct[1] += VertexWeight[1][TempZip];
		TempVtxPct[2] += VertexWeight[2][TempZip];
		} // if
	} // if
TempVtxPct[0] /= VtxWtCt;
TempVtxPct[1] /= VtxWtCt;
TempVtxPct[2] /= VtxWtCt;
// compute values for lower right corner
ZCorner = 1.0;
if (! CamPlanOrtho)
	ZCorner /= (TempVtxPct[0] + TempVtxPct[1] + TempVtxPct[2]);
TexDat->TElevRange[3] = ZCorner * (TempVtxPct[0] * Vert[0]->Elev + TempVtxPct[1] * Vert[1]->Elev + TempVtxPct[2] * Vert[2]->Elev);
TexDat->VtxPixCornerWt[3][0] = ZCorner * TempVtxPct[0];
TexDat->VtxPixCornerWt[3][1] = ZCorner * TempVtxPct[1];
TexDat->VtxPixCornerWt[3][2] = ZCorner * TempVtxPct[2];

} // Renderer::Fill3DObjectTextureData

/*===========================================================================*/

int Renderer::PixelFragMemoryError(void)
{

if (UserMessageYN("Pixel Fragment Manager", "Unable to allocate enough pixel fragments for rendering. Continue rendering with a fragment quality of 1?"))
	{
	FragmentDepth = 1;
	FragMemFailed = 0;
	return (1);
	} // if

UserMessageOK("Renderer", "Rendering will be aborted. Please try rendering in segments or tiles or rendering with a lower fragment quality.");

return (0);

} // Renderer::PixelFragMemoryError
