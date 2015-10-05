// Atmosphere.cpp
// For managing WCS Atmospheres
// Built from scratch on 06/9/99 by Gary R. Huber
// Copyright 1999 Questar Productions

#include "stdafx.h"
#include "Texture.h"
#include "Raster.h"
#include "Useful.h"
#include "MathSupport.h"
#include "Application.h"
#include "EffectsLib.h"
#include "EffectsIO.h"
#include "Joe.h"
#include "Conservatory.h"
#include "requester.h"
#include "Toolbar.h"
#include "DragnDropListGUI.h"
#include "AtmosphereEditGUI.h"
#include "Render.h"
#include "Raster.h"
#include "Database.h"
#include "PixelManager.h"
#include "Lists.h"

VolumetricSubstance::VolumetricSubstance()
{
int SideCt;

NextSub = NULL; 
RayList = ShadowRayList = CurRayList = CurShadowRayList = NULL;
BoxBoundsX[0] = BoxBoundsY[0] = BoxBoundsZ[0] = BoxBoundsX[1] = BoxBoundsY[1] = BoxBoundsZ[1] = 0.0;

for (SideCt = 0; SideCt < 6; SideCt ++)
	{
	QuadNormal[SideCt][0] = 1.0;
	QuadNormal[SideCt][1] = QuadNormal[SideCt][2] = 0.0;
	} // for

} // VolumetricSubstance::VolumetricSubstance

/*===========================================================================*/

VolumetricSubstance::~VolumetricSubstance()
{

DeleteRayList();
DeleteShadowRayList();

} // VolumetricSubstance::~VolumetricSubstance

/*===========================================================================*/

void VolumetricSubstance::DeleteRayList(void)
{
VolumeRayList *DelList, *CurList;

CurList = RayList;
while (CurList)
	{
	DelList = CurList;
	CurList = CurList->Next;
	delete DelList;
	} // while
RayList = NULL;

} // VolumetricSubstance::DeleteRayList

/*===========================================================================*/

void VolumetricSubstance::DeleteShadowRayList(void)
{
VolumeRayList *DelList, *CurList;

CurList = ShadowRayList;
while (CurList)
	{
	DelList = CurList;
	CurList = CurList->Next;
	delete DelList;
	} // while
ShadowRayList = NULL;

} // VolumetricSubstance::DeleteRayList

/*===========================================================================*/

void VolumetricSubstance::ComputeBoundingBox(CoordSys *DefCoords, double PlanetRad, double ProjRefLat, double ProjRefLon)
{

BoxBoundsX[0] = BoxBoundsY[0] = BoxBoundsZ[0] = BoxBoundsX[1] = BoxBoundsY[1] = BoxBoundsZ[1] = 0.0;

} // VolumetricSubstance::ComputeBoundingBox

/*===========================================================================*/

static int Xside[6][4] = {
// left
0, 0, 0, 0,
// right
1, 1, 1, 1,
// front
0, 0, 1, 1,
// back
1, 1, 0, 0,
// top
0, 0, 1, 1,
// bottom
1, 1, 0, 0
};
static int Yside[6][4] = {
// left
0, 1, 1, 0,
// right
0, 1, 1, 0,
// front
0, 1, 1, 0,
// back
0, 1, 1, 0,
// top
1, 1, 1, 1,
// bottom
0, 0, 0, 0
};
static int Zside[6][4] = {
// left
1, 1, 0, 0,
// right
0, 0, 1, 1,
// front
0, 0, 0, 0,
// back
1, 1, 1, 1,
// top
0, 1, 1, 0,
// bottom
1, 0, 0, 1
};

void VolumetricSubstance::CompareSetBounds(VertexDEM *CurVert)
{

if (CurVert->XYZ[0] < BoxBoundsX[0])
	BoxBoundsX[0] = CurVert->XYZ[0];
if (CurVert->XYZ[1] < BoxBoundsY[0])
	BoxBoundsY[0] = CurVert->XYZ[1];
if (CurVert->XYZ[2] < BoxBoundsZ[0])
	BoxBoundsZ[0] = CurVert->XYZ[2];
if (CurVert->XYZ[0] > BoxBoundsX[1])
	BoxBoundsX[1] = CurVert->XYZ[0];
if (CurVert->XYZ[1] > BoxBoundsY[1])
	BoxBoundsY[1] = CurVert->XYZ[1];
if (CurVert->XYZ[2] > BoxBoundsZ[1])
	BoxBoundsZ[1] = CurVert->XYZ[2];

} // VolumetricSubstance::CompareSetBounds

/*===========================================================================*/

void VolumetricSubstance::SetQuadNormals(void)
{
Point3d Side1, Side2;
int SideCt;

for (SideCt = 0; SideCt < 6; SideCt ++)
	{
	Side1[0] = BoxBoundsX[Xside[SideCt][1]];
	Side1[1] = BoxBoundsY[Yside[SideCt][1]];
	Side1[2] = BoxBoundsZ[Zside[SideCt][1]];
	Side2[0] = BoxBoundsX[Xside[SideCt][2]];
	Side2[1] = BoxBoundsY[Yside[SideCt][2]];
	Side2[2] = BoxBoundsZ[Zside[SideCt][2]];
	Side1[0] -= BoxBoundsX[Xside[SideCt][0]];
	Side1[1] -= BoxBoundsY[Yside[SideCt][0]];
	Side1[2] -= BoxBoundsZ[Zside[SideCt][0]];
	Side2[0] -= BoxBoundsX[Xside[SideCt][0]];
	Side2[1] -= BoxBoundsY[Yside[SideCt][0]];
	Side2[2] -= BoxBoundsZ[Zside[SideCt][0]];

	SurfaceNormal(&QuadNormal[SideCt][0], Side1, Side2);
	} // for

} // VolumetricSubstance::SetQuadNormals

/*===========================================================================*/

// RayVec needs to be a unit vector
VolumeRayList *VolumetricSubstance::BuildRayList(CoordSys *DefCoords, double PlanetRad, double RayStart[3], double RayVec[3])
{
Point3d VecOrigin, Intersection;
double Dist, QuadVert[4][3], HitDist[2];
int Intersections = 0, SideCt, VertCt;
VolumeRayList *RayHits = NULL;

HitDist[0] = HitDist[1] = 0.0;

// hit test all sides of the bounding box and record distance to hits
// only hits within the side polygon count
// only two intersections allowed since it is a box with parallel sides
for (SideCt = 0; SideCt < 6 && Intersections < 2; SideCt ++)
	{
	for (VertCt = 0; VertCt < 4; VertCt ++)
		{
		QuadVert[VertCt][0] = BoxBoundsX[Xside[SideCt][VertCt]];
		QuadVert[VertCt][1] = BoxBoundsY[Yside[SideCt][VertCt]];
		QuadVert[VertCt][2] = BoxBoundsZ[Zside[SideCt][VertCt]];
		} // for
	// VecOrigin may be modified by RayQuadrilateralIntersect so reset each pass
	VecOrigin[0] = RayStart[0];
	VecOrigin[1] = RayStart[1];
	VecOrigin[2] = RayStart[2];

	if (RayQuadrilateralIntersect(VecOrigin, RayVec, &QuadNormal[SideCt][0], QuadVert, Intersection, Dist))
		{
		HitDist[Intersections] = Dist;
		Intersections ++;
		} // if
	} // for

if (HitDist[0] <= 0.0 && HitDist[1] <= 0.0)
	return (NULL);

if (Intersections && (RayHits = new VolumeRayList))
	{
	if (Intersections == 1 && HitDist[0] > 0.0)
		{
		RayHits->RayOnDist = 0.0;
		RayHits->RayOffDist = HitDist[0];
		} // if
	else
		{
		if (HitDist[0] < HitDist[1])
			{
			if (HitDist[0] > 0.0)
				{
				RayHits->RayOnDist = HitDist[0];
				} // if
			else
				{
				RayHits->RayOnDist = 0.0;
				} // else
			RayHits->RayOffDist = HitDist[1];
			} // if
		else
			{
			if (HitDist[1] > 0.0)
				{
				RayHits->RayOnDist = HitDist[1];
				} // if
			else
				{
				RayHits->RayOnDist = 0.0;
				} // else
			RayHits->RayOffDist = HitDist[0];
			} // else
		} // else
	RayHits->SampleRate = (RayHits->RayOffDist - RayHits->RayOnDist) * .01;
	RayHits->SegMidXYZ[0] = (RayHits->RayOnDist + RayHits->SampleRate * .5) * RayVec[0] + RayStart[0];
	RayHits->SegMidXYZ[1] = (RayHits->RayOnDist + RayHits->SampleRate * .5) * RayVec[1] + RayStart[1];
	RayHits->SegMidXYZ[2] = (RayHits->RayOnDist + RayHits->SampleRate * .5) * RayVec[2] + RayStart[2];
	RayHits->SegStartDist = RayHits->RayOnDist;
	RayHits->SegEndDist = WCS_min(RayHits->RayOnDist + RayHits->SampleRate, RayHits->RayOffDist);
	RayHits->SegEvaluated = 0;
	} // if

return (RayHits);

} // VolumetricSubstance::BuildRayList

/*===========================================================================*/
/*===========================================================================*/

AtmosphereBase::AtmosphereBase()
{

SetDefaults();

} // AtmosphereBase::AtmosphereBase

/*===========================================================================*/

void AtmosphereBase::SetDefaults(void)
{

SpeedBoost = WCS_ATMOSPHEREBASE_VOLUMESPEEDBOOST_LOW;
AdaptiveThreshold = 20;

} // AtmosphereBase::SetDefaults

/*===========================================================================*/

void AtmosphereBase::InitToRender(void)
{

} // AtmosphereBase::InitToRender

/*===========================================================================*/

AtmosphereComponent::AtmosphereComponent(RasterAnimHost *RAHost)
: RasterAnimHost(RAHost)
{
long Ct;
double EffectDefault[WCS_EFFECTS_ATMOCOMPONENT_NUMANIMPAR] = {0.0, 
	1.0, 1.0, 5000.0, .5, 1.0, 50000.0, 200.0, 300.0, .5, 5.0, 0.0};
double RangeDefaults[WCS_EFFECTS_ATMOCOMPONENT_NUMANIMPAR][3] = {FLT_MAX, -10000.0, 100.0,	// base elevation in meters
														1.0, 0.0, .01,				// coverage in %
														5.0, 0.0, .01,				// density in %
														1000000.0, 0.0, 10.0,		// thickness in meters
														1.0, 0.0, .01,				// self-shading %
														1.0, 0.0, .01,				// received shadow intensity
														FLT_MAX, 1.0, 100.0,		// optical depth
														FLT_MAX, .01, 1.0,			// min volumetric sample rate
														FLT_MAX, .01, 1.0,			// max volumetric sample rate
														FLT_MAX, 0.0, .01,			// backlight %
														200.0, 0.0, 1.0,			// backlight exponent
														1.0, 0.0, .01};				// alt. backlight color %
GraphNode *CurNode;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct + WCS_EFFECTS_ATMOSPHERE_NUMANIMPAR, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
BacklightColor.SetDefaults(this, (char)Ct ++);
BacklightColor.SetValue3(1.0, 1.0, 1.0);
ParticleColor.SetDefaults(this, (char)Ct);
ParticleColor.SetValue3(1.0, 1.0, 1.0);
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	TexRoot[Ct] = NULL;
	} // for
CastShadows = 0;
ReceiveShadowsTerrain = ReceiveShadowsFoliage = ReceiveShadows3DObject = ReceiveShadowsCloudSM = 
	ReceiveShadowsVolumetric = 1;
ReceiveShadowsMisc = 0;
ShadowFlags = 0;
VolumeBeforeRefl = 1;
CompleteBacklightColor[0] = CompleteBacklightColor[1] = CompleteBacklightColor[2] = 0.0;
Enabled = 1;
Next = NULL;
strcpy(Name, "Sky");
CompleteParticleColor[0] = CompleteParticleColor[1] = CompleteParticleColor[2] = 0.0;
LowElev = HighElev = ElevRange = 0.0;
CovgTextureExists = 0;
RadiusInside = RadiusOutside = 0.0;
NoSubBaseDensity = 0;
RefLon = RefLat = 0.0;

CovgProf.SetDefaults(this, (char)GetNumAnimParams() + 1);
DensityProf.SetDefaults(this, (char)GetNumAnimParams() + 2);
ShadeProf.SetDefaults(this, (char)GetNumAnimParams() + 3);
CovgProf.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ShadeProf.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
DensityProf.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
CovgProf.ReleaseNodes();
CurNode = CovgProf.AddNode(0.0, 1.0, 0.0);
CurNode = CovgProf.AddNode(1.0, 1.0, 0.0);
ShadeProf.ReleaseNodes();
ShadeProf.AddNode(0.0, 1.0, 0.0);
ShadeProf.AddNode(1.0, 0.0, 0.0);
DensityProf.ReleaseNodes();
DensityProf.AddNode(0.0, 1.0, 0.0);
DensityProf.AddNode(1.0, 0.0, 0.0);
CovgProf.ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ShadeProf.ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
DensityProf.ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BASEELEV].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_THICKNESS].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_OPTICALDEPTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_MINSAMPLE].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_MAXSAMPLE].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_COVERAGE].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_DENSITY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_SELFSHADING].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_RECVSHADOWINTENS].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTPCT].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_ALTBACKLIGHTCOLORPCT].SetMultiplier(100.0);

Next = NULL;
strcpy(Name, "Haze");

} // AtmosphereComponent::AtmosphereComponent

/*===========================================================================*/

AtmosphereComponent::~AtmosphereComponent()
{
long Ct;
RootTexture *DelTex;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (TexRoot[Ct])
		{
		DelTex = TexRoot[Ct];
		TexRoot[Ct] = NULL;
		delete DelTex;
		} // if
	} // for

} // AtmosphereComponent::~AtmosphereComponent

/*===========================================================================*/

void AtmosphereComponent::Copy(AtmosphereComponent *CopyTo, AtmosphereComponent *CopyFrom)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].Copy((AnimCritter *)&CopyTo->AnimPar[Ct], (AnimCritter *)&CopyFrom->AnimPar[Ct]);
	} // for

ParticleColor.Copy(&CopyTo->ParticleColor, &CopyFrom->ParticleColor);
CopyTo->BacklightColor.Copy(&CopyTo->BacklightColor, &CopyFrom->BacklightColor);
CopyTo->CovgProf.Copy(&CopyTo->CovgProf, &CopyFrom->CovgProf);
CopyTo->ShadeProf.Copy(&CopyTo->ShadeProf, &CopyFrom->ShadeProf);
CopyTo->DensityProf.Copy(&CopyTo->DensityProf, &CopyFrom->DensityProf);

CopyTo->Enabled = CopyFrom->Enabled;
CopyTo->VolumeBeforeRefl = CopyFrom->VolumeBeforeRefl;
CopyTo->CastShadows = CopyFrom->CastShadows;
CopyTo->ReceiveShadowsTerrain = CopyFrom->ReceiveShadowsTerrain;
CopyTo->ReceiveShadowsFoliage = CopyFrom->ReceiveShadowsFoliage;
CopyTo->ReceiveShadows3DObject = CopyFrom->ReceiveShadows3DObject;
CopyTo->ReceiveShadowsCloudSM = CopyFrom->ReceiveShadowsCloudSM;
CopyTo->ReceiveShadowsVolumetric = CopyFrom->ReceiveShadowsVolumetric;
CopyTo->ReceiveShadowsMisc = CopyFrom->ReceiveShadowsMisc;
CopyTo->ShadowFlags = CopyFrom->ShadowFlags;
strcpy(CopyTo->Name, CopyFrom->Name);
RootTextureParent::Copy(CopyTo, CopyFrom);
RasterAnimHost::Copy(CopyTo, CopyFrom);

} // AtmosphereComponent::Copy

/*===========================================================================*/

unsigned long AtmosphereComponent::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
RootTexture *DelTex;
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

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
					case WCS_EFFECTS_ATMOCOMP_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_CASTSHADOWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&CastShadows, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_RECEIVESHADOWSTER:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsTerrain, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_RECEIVESHADOWSFOL:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsFoliage, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_RECEIVESHADOWS3D:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadows3DObject, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_RECEIVESHADOWSSM:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsCloudSM, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_RECEIVESHADOWSVOL:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsVolumetric, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_RECEIVESHADOWSMISC:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsMisc, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_VOLBEFOREREFL:
						{
						BytesRead = ReadBlock(ffile, (char *)&VolumeBeforeRefl, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_GRADIENTDATUM:	// V5
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BASEELEV].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_SCALEHEIGHT:	// V5
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_THICKNESS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_BASEELEV:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BASEELEV].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_COVERAGE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_COVERAGE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_DENSITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_DENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_THICKNESS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_THICKNESS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_SELFSHADING:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_SELFSHADING].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_RECVSHADOWINTENS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_RECVSHADOWINTENS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_OPTICALDEPTH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_OPTICALDEPTH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_MINSAMPLE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_MINSAMPLE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_MAXSAMPLE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_MAXSAMPLE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_BACKLIGHTPCT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTPCT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_BACKLIGHTEXP:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTEXP].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_ALTBACKLIGHTPCT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_ALTBACKLIGHTCOLORPCT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_PARTICLECOLOR:
						{
						BytesRead = ParticleColor.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_BACKLIGHTCOLOR:
						{
						BytesRead = BacklightColor.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_COVGPROFILE:
						{
						BytesRead = CovgProf.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_DENSITYPROFILE:
						{
						BytesRead = DensityProf.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_SHADEPROFILE:
						{
						BytesRead = ShadeProf.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOCOMP_TEXCOVERAGE:
						{
						if (DelTex = TexRoot[WCS_EFFECTS_ATMOCOMPONENT_TEXTURE_COVERAGE])
							{
							TexRoot[WCS_EFFECTS_ATMOCOMPONENT_TEXTURE_COVERAGE] = NULL;
							delete DelTex;
							} // if
						if (TexRoot[WCS_EFFECTS_ATMOCOMPONENT_TEXTURE_COVERAGE] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_ATMOCOMPONENT_TEXTURE_COVERAGE]->Load(ffile, Size, ByteFlip);
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

return (TotalRead);

} // AtmosphereComponent::Load

/*===========================================================================*/

unsigned long AtmosphereComponent::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long AnimItemTag[WCS_EFFECTS_ATMOCOMPONENT_NUMANIMPAR] = {WCS_EFFECTS_ATMOCOMP_BASEELEV,
																	WCS_EFFECTS_ATMOCOMP_COVERAGE,
																	WCS_EFFECTS_ATMOCOMP_DENSITY,
																	WCS_EFFECTS_ATMOCOMP_THICKNESS,
																	WCS_EFFECTS_ATMOCOMP_SELFSHADING,
																	WCS_EFFECTS_ATMOCOMP_RECVSHADOWINTENS,
																	WCS_EFFECTS_ATMOCOMP_OPTICALDEPTH,
																	WCS_EFFECTS_ATMOCOMP_MINSAMPLE,
																	WCS_EFFECTS_ATMOCOMP_MAXSAMPLE,
																	WCS_EFFECTS_ATMOCOMP_BACKLIGHTPCT,
																	WCS_EFFECTS_ATMOCOMP_BACKLIGHTEXP,
																	WCS_EFFECTS_ATMOCOMP_ALTBACKLIGHTPCT};
unsigned long TextureItemTag[WCS_EFFECTS_ATMOCOMPONENT_NUMTEXTURES] = {WCS_EFFECTS_ATMOCOMP_TEXCOVERAGE};

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
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ATMOCOMP_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ATMOCOMP_CASTSHADOWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&CastShadows)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ATMOCOMP_RECEIVESHADOWSTER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsTerrain)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ATMOCOMP_RECEIVESHADOWSFOL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsFoliage)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ATMOCOMP_RECEIVESHADOWS3D, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadows3DObject)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ATMOCOMP_RECEIVESHADOWSSM, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsCloudSM)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ATMOCOMP_RECEIVESHADOWSVOL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsVolumetric)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ATMOCOMP_RECEIVESHADOWSMISC, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsMisc)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ATMOCOMP_VOLBEFOREREFL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VolumeBeforeRefl)) == NULL)
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

ItemTag = WCS_EFFECTS_ATMOCOMP_PARTICLECOLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = ParticleColor.Save(ffile))
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
			} /* if color saved */
		else
			goto WriteError;
		} /* if size written */
	else
		goto WriteError;
	} /* if tag written */
else
	goto WriteError;

ItemTag = WCS_EFFECTS_ATMOCOMP_BACKLIGHTCOLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = BacklightColor.Save(ffile))
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
			} // if BacklightColor saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_EFFECTS_ATMOCOMP_COVGPROFILE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = CovgProf.Save(ffile))
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

ItemTag = WCS_EFFECTS_ATMOCOMP_DENSITYPROFILE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = DensityProf.Save(ffile))
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
			} // if DensityProf saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_EFFECTS_ATMOCOMP_SHADEPROFILE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = ShadeProf.Save(ffile))
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

} // AtmosphereComponent::Save

/*===========================================================================*/

char *AtmosphereComponentCritterNames[WCS_EFFECTS_ATMOCOMPONENT_NUMANIMPAR] = {"Base Elevation (m)", "Coverage (%)", "Density (%)",
	"Thickness (m)", "Self-shading (%)", "Received Shadow Intensity (%)", "Optical Depth (m)",
	"Minimum Sample Spacing (m) ", "Maximum Sample Spacing (m) ", "Backlight Intensity (%)", "Backlight Exponent", 
	"Alt. Backlight Color (%)"};
char *AtmosphereComponentTextureNames[WCS_EFFECTS_ATMOCOMPONENT_NUMTEXTURES] = {"Coverage (%)"};

char *AtmosphereComponent::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (AtmosphereComponentCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (AtmosphereComponentTextureNames[Ct]);
		} // if
	} // for
if (Test == &ParticleColor)
	return ("Particle Color");
if (Test == &BacklightColor)
	return ("Alt. Backlight Color");
if (Test == &CovgProf)
	return ("Vertical Coverage Profile");
if (Test == &DensityProf)
	return ("Vertical Density Profile");
if (Test == &ShadeProf)
	return ("Vertical Shading Profile");

return ("");

} // AtmosphereComponent::GetCritterName

/*===========================================================================*/

char *AtmosphereComponent::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? AtmosphereComponentTextureNames[TexNumber]: (char*)"");

} // AtmosphereComponent::GetTextureName

/*===========================================================================*/

RootTexture *AtmosphereComponent::NewRootTexture(long TexNumber)
{
char ApplyToColor = 0;
char ApplyToDisplace = 1;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // AtmosphereComponent::NewRootTexture

/*===========================================================================*/

int AtmosphereComponent::SetToTime(double Time)
{
long Found = 0, Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for
if (ParticleColor.SetToTime(Time))
	Found = 1;
if (BacklightColor.SetToTime(Time))
	Found = 1;

return (Found);

} // AtmosphereComponent::SetToTime

/*===========================================================================*/

long AtmosphereComponent::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Ct, Found = 0;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetMinMaxDist(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetKeyFrameRange(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // for

if (ParticleColor.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (BacklightColor.GetKeyFrameRange(MinDist, MaxDist))
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

} // AtmosphereComponent::GetKeyFrameRange

/*===========================================================================*/

char AtmosphereComponent::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (GetNumAnimParams() > 0 && DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
	return (1);
if (GetNumTextures() > 0 && (DropType == WCS_RAHOST_OBJTYPE_ROOTTEXTURE || DropType == WCS_RAHOST_OBJTYPE_TEXTURE))
	return (1);

return (0);

} // AtmosphereComponent::GetRAHostDropOK

/*===========================================================================*/

int AtmosphereComponent::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char WinNum, QueryStr[256], NameStr[128];
int Success = 0;
long Ct, NumListItems = 0;
RasterAnimHostProperties Prop;
RasterAnimHost *TargetList[30];
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (AtmosphereComponent *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (AtmosphereComponent *)DropSource->DropSource);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		TargetList[Ct] = GetAnimPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
	{
	Success = ParticleColor.ProcessRAHostDragDrop(DropSource);
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ROOTTEXTURE || DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumTextures(); Ct ++)
		{
		TargetList[Ct] = GetTexRootPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if

if (NumListItems > 0)
	{
	if ((WinNum = GlobalApp->MCP->GetAvailableDragnDropListNumber()) >= 0)
		{
		GlobalApp->GUIWins->DDL[WinNum] = new DragnDropListGUI(WinNum, DropSource, TargetList, NumListItems, this, NULL);
		if(GlobalApp->GUIWins->DDL[WinNum])
			{
			GlobalApp->GUIWins->DDL[WinNum]->Open(GlobalApp->MainProj);
			}
		} // if
	else
		UserMessageOK("Open Drag 'n Drop List", "All List windows are in use.\nTry dropping on a target deeper in the hierarchy\n and closer to your desired target.");
	} // if

return (Success);

} // AtmosphereComponent::ProcessRAHostDragDrop

/*===========================================================================*/

int AtmosphereComponent::GetRAHostAnimated(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for
if (ParticleColor.GetRAHostAnimated())
	return (1);
if (BacklightColor.GetRAHostAnimated())
	return (1);

return (0);

} // AtmosphereComponent::GetRAHostAnimated

/*===========================================================================*/

unsigned long AtmosphereComponent::GetRAFlags(unsigned long Mask)
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
	Flags |= WCS_RAHOST_FLAGBIT_EDITNAME;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_ATMOSPHERE | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // AtmosphereComponent::GetRAFlags

/*===========================================================================*/

int AtmosphereComponent::GetRAEnabled(void)
{

return (RAParent ? (Enabled && RAParent->GetRAEnabled() && 
	((Atmosphere *)RAParent)->AtmosphereType == WCS_EFFECTS_ATMOSPHERETYPE_VOLUMETRIC): Enabled);

} // AtmosphereComponent::GetRAEnabled

/*===========================================================================*/

void AtmosphereComponent::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = GetName();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPE)
	{
	Prop->Type = GetRAHostTypeString();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPENUMBER)
	{
	Prop->TypeNumber = GetRAHostTypeNumber();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPOK)
	{
	Prop->DropOK = GetRAHostDropOK(Prop->TypeNumber);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	GetKeyFrameRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1]);
	} // if
else if (Prop->PropMask & WCS_RAHOST_MASKBIT_NEXTKEY)
	{
	GetNextAnimNode(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_FILEINFO)
	{
	Prop->Path = EffectsLib::DefaultPaths[WCS_EFFECTSSUBCLASS_ATMOSPHERE];
	Prop->Ext = "atc";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_POPCLASS)
	{
	GetPopClassFlags(Prop);
	} // get pop-up menu classes

} // AtmosphereComponent::GetRAHostProperties

/*===========================================================================*/

int AtmosphereComponent::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_ENABLED)
		{
		Enabled = (Prop->Flags & WCS_RAHOST_FLAGBIT_ENABLED) ? 1: 0;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPSOURCE)
	{
	Success = ProcessRAHostDragDrop(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	if (Success = ScaleDeleteAnimNodes(Prop))
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_LOADFILE)
	{
	return(LoadObject(Prop->fFile, 0, Prop->ByteFlip));
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_SAVEFILE)
	{
	return(SaveObject(Prop->fFile));
	} // if

return (Success);

} // AtmosphereComponent::SetRAHostProperties

/*===========================================================================*/

RasterAnimHost *AtmosphereComponent::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;

if (! Current)
	return (&ParticleColor);
if (Current == &ParticleColor)
	return (&BacklightColor);
if (Current == &BacklightColor)
	return (&CovgProf);
if (Current == &CovgProf)
	return (&DensityProf);
if (Current == &DensityProf)
	return (&ShadeProf);
if (Current == &ShadeProf)
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

return (NULL);

} // AtmosphereComponent::GetRAHostChild

/*===========================================================================*/

int AtmosphereComponent::RemoveRAHost(RasterAnimHost *RemoveMe)
{
char Ct;
NotifyTag Changes[2];
int Removed = 0;

if (RemoveMe)
	{
	for (Ct = 0; Ct < GetNumTextures(); Ct ++)
		{
		if (RemoveMe == GetTexRootPtr(Ct))
			{
			SetTexRootPtr(Ct, NULL);
			delete (RootTexture *)RemoveMe;
			Removed = 1;
			} // if
		} // for

	if (Removed)
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if

return (Removed);

} // AtmosphereComponent::RemoveRAHost

/*===========================================================================*/

long AtmosphereComponent::InitImageIDs(long &ImageID)
{
char Ct;
long NumImages = 0;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		NumImages += GetTexRootPtr(Ct)->InitImageIDs(ImageID);
		} // if
	} // for

return (NumImages);

} // AtmosphereComponent::InitImageIDs

/*===========================================================================*/

void AtmosphereComponent::SetDefaultProperties(char *ParticleTypeStr, double NewBaseElev)
{
NotifyTag Changes[2];

if (! stricmp(ParticleTypeStr, "Air"))
	{
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BASEELEV].SetValue(0.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_THICKNESS].SetValue(50000.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_DENSITY].SetValue(1.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_OPTICALDEPTH].SetValue(20000.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTPCT].SetValue(0.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_ALTBACKLIGHTCOLORPCT].SetValue(0.0);
	ParticleColor.SetValue3(111.0 / 255.0, 172.0 / 255.0, 242.0 / 255.0);
	strcpy(Name, ParticleTypeStr);
	} // if
else if (! stricmp(ParticleTypeStr, "Light Haze"))
	{
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BASEELEV].SetValue(NewBaseElev);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_THICKNESS].SetValue(5000.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_DENSITY].SetValue(1.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_OPTICALDEPTH].SetValue(100000.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTPCT].SetValue(0.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_ALTBACKLIGHTCOLORPCT].SetValue(0.0);
	ParticleColor.SetValue3(228.0 / 255.0, 239.0 / 255.0, 251.0 / 255.0);
	strcpy(Name, ParticleTypeStr);
	} // else if
else if (! stricmp(ParticleTypeStr, "Medium Haze"))
	{
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BASEELEV].SetValue(NewBaseElev);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_THICKNESS].SetValue(2000.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_DENSITY].SetValue(1.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_OPTICALDEPTH].SetValue(50000.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTPCT].SetValue(0.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_ALTBACKLIGHTCOLORPCT].SetValue(0.0);
	ParticleColor.SetValue3(228.0 / 255.0, 239.0 / 255.0, 251.0 / 255.0);
	strcpy(Name, ParticleTypeStr);
	} // else if
else if (! stricmp(ParticleTypeStr, "Heavy Haze"))
	{
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BASEELEV].SetValue(NewBaseElev);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_THICKNESS].SetValue(500.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_DENSITY].SetValue(1.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_OPTICALDEPTH].SetValue(5000.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTPCT].SetValue(0.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_ALTBACKLIGHTCOLORPCT].SetValue(0.0);
	ParticleColor.SetValue3(228.0 / 255.0, 239.0 / 255.0, 251.0 / 255.0);
	strcpy(Name, ParticleTypeStr);
	} // else if
else if (! stricmp(ParticleTypeStr, "Light Fog"))
	{
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BASEELEV].SetValue(NewBaseElev);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_THICKNESS].SetValue(20.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_DENSITY].SetValue(1.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_OPTICALDEPTH].SetValue(200.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTPCT].SetValue(0.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_ALTBACKLIGHTCOLORPCT].SetValue(0.0);
	ParticleColor.SetValue3(228.0 / 255.0, 239.0 / 255.0, 251.0 / 255.0);
	strcpy(Name, ParticleTypeStr);
	} // else if
else if (! stricmp(ParticleTypeStr, "Medium Fog"))
	{
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BASEELEV].SetValue(NewBaseElev);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_THICKNESS].SetValue(30.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_DENSITY].SetValue(1.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_OPTICALDEPTH].SetValue(50.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTPCT].SetValue(0.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_ALTBACKLIGHTCOLORPCT].SetValue(0.0);
	ParticleColor.SetValue3(228.0 / 255.0, 239.0 / 255.0, 251.0 / 255.0);
	strcpy(Name, ParticleTypeStr);
	} // else if
else if (! stricmp(ParticleTypeStr, "Heavy Fog"))
	{
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BASEELEV].SetValue(NewBaseElev);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_THICKNESS].SetValue(40.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_DENSITY].SetValue(1.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_OPTICALDEPTH].SetValue(10.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTPCT].SetValue(0.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_ALTBACKLIGHTCOLORPCT].SetValue(0.0);
	ParticleColor.SetValue3(228.0 / 255.0, 239.0 / 255.0, 251.0 / 255.0);
	strcpy(Name, ParticleTypeStr);
	} // else if
else if (! stricmp(ParticleTypeStr, "Ground Fog"))
	{
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BASEELEV].SetValue(NewBaseElev);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_THICKNESS].SetValue(5.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_DENSITY].SetValue(1.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_OPTICALDEPTH].SetValue(15.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTPCT].SetValue(0.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_ALTBACKLIGHTCOLORPCT].SetValue(0.0);
	ParticleColor.SetValue3(228.0 / 255.0, 239.0 / 255.0, 251.0 / 255.0);
	strcpy(Name, ParticleTypeStr);
	} // else if
else if (! stricmp(ParticleTypeStr, "Dust"))
	{
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BASEELEV].SetValue(NewBaseElev);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_THICKNESS].SetValue(200.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_DENSITY].SetValue(1.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_OPTICALDEPTH].SetValue(10000.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTPCT].SetValue(0.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_ALTBACKLIGHTCOLORPCT].SetValue(0.0);
	ParticleColor.SetValue3(200.0 / 255.0, 180.0 / 255.0, 120.0 / 255.0);
	strcpy(Name, ParticleTypeStr);
	} // else if
else if (! stricmp(ParticleTypeStr, "Smog"))
	{
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BASEELEV].SetValue(NewBaseElev);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_THICKNESS].SetValue(300.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_DENSITY].SetValue(1.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_OPTICALDEPTH].SetValue(30000.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTPCT].SetValue(0.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_ALTBACKLIGHTCOLORPCT].SetValue(0.0);
	ParticleColor.SetValue3(160.0 / 255.0, 160.0 / 255.0, 100.0 / 255.0);
	strcpy(Name, ParticleTypeStr);
	} // else if
else if (! stricmp(ParticleTypeStr, "Smoke"))
	{
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BASEELEV].SetValue(NewBaseElev);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_THICKNESS].SetValue(1000.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_DENSITY].SetValue(1.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_OPTICALDEPTH].SetValue(100.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BACKLIGHTPCT].SetValue(0.0);
	AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_ALTBACKLIGHTCOLORPCT].SetValue(0.0);
	ParticleColor.SetValue3(50.0 / 255.0, 50.0 / 255.0, 50.0 / 255.0);
	strcpy(Name, ParticleTypeStr);
	} // else if

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

} // AtmosphereComponent::SetDefaultProperties

/*===========================================================================*/

int AtmosphereComponent::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil, 
	RootTexture **&TexAffil)
{
long Ct;

AnimAffil = NULL;

if (ChildA)
	{
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		if (ChildA == GetAnimPtr(Ct))
			{
			AnimAffil = (AnimCritter *)ChildA;
			return (1);
			} // if
		} // for
	if (ChildA == &ParticleColor)
		{
		AnimAffil = (AnimCritter *)ChildA;
		return (1);
		} // if
	if (ChildA == &BacklightColor)
		{
		AnimAffil = (AnimCritter *)ChildA;
		return (1);
		} // if
	} // if
else if (ChildB)
	{
	for (Ct = 0; Ct < GetNumTextures(); Ct ++)
		{
		if (ChildB == (RasterAnimHost **)GetTexRootPtrAddr(Ct))
			{
			TexAffil = (RootTexture **)ChildB;
			return (1);
			} // if
		} // for
	} // else if

return (0);

} // AtmosphereComponent::GetAffiliates

/*===========================================================================*/

int AtmosphereComponent::GetPopClassFlags(RasterAnimHostProperties *Prop)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;

Prop->PopClassFlags = 0;
Prop->PopExistsFlags = 0;
Prop->PopEnabledFlags = 0;

if (GetAffiliates(Prop->ChildA, Prop->ChildB, AnimAffil, TexAffil))
	{
	return (RasterAnimHost::GetPopClassFlags(Prop, AnimAffil, TexAffil, NULL));
	} // if

return (0);

} // AtmosphereComponent::GetPopClassFlags

/*===========================================================================*/

int AtmosphereComponent::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil))
	{
	return (RasterAnimHost::AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, TexAffil, NULL));
	} // if

return(0);

} // AtmosphereComponent::AddSRAHBasePopMenus

/*===========================================================================*/

int AtmosphereComponent::HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil))
	{
	return (RasterAnimHost::HandleSRAHPopMenuSelection(Action, AnimAffil, TexAffil, NULL, this, NULL));
	} // if

return(0);

} // AtmosphereComponent::HandleSRAHPopMenuSelection

/*===========================================================================*/

void AtmosphereComponent::ComputeBoundingBox(CoordSys *DefCoords, double PlanetRad, double ProjRefLat, double ProjRefLon)
{
double Radius;
VertexDEM CurVert;

CurVert.Elev = HighElev;
CurVert.Lat = ProjRefLat;
CurVert.Lon = ProjRefLon;
#ifdef WCS_BUILD_VNS
DefCoords->DegToCart(&CurVert);
#else // WCS_BUILD_VNS
CurVert.DegToCart(PlanetRad);
#endif // WCS_BUILD_VNS
Radius = VectorMagnitude(CurVert.XYZ);

BoxBoundsX[0] = BoxBoundsY[0] = BoxBoundsZ[0] = -Radius;
BoxBoundsX[1] = BoxBoundsY[1] = BoxBoundsZ[1] = Radius;
SetQuadNormals();

} // AtmosphereComponent::ComputeBoundingBox

/*===========================================================================*/

VolumeRayList *AtmosphereComponent::BuildRayList(CoordSys *DefCoords, double PlanetRad, double RayStart[3], double RayVec[3])
{
Point3d WorldOrigin, Intersection1, Intersection2;
double Dist1, Dist2, ClosestIn, FarthestOut, Jitter;
int Intersections;
VolumeRayList *RayHits;

if (AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_DENSITY].CurValue <= 0.0)
	return (NULL);

if (RayHits = VolumetricSubstance::BuildRayList(DefCoords, PlanetRad, RayStart, RayVec))
	{
	WorldOrigin[0] = WorldOrigin[1] = WorldOrigin[2] = 0.0;
	ClosestIn = FarthestOut = -1.0;

	// if first value on density profile is 0 then we can throw out all points inside base elevation
	// test intersection with inside sphere
	if (Intersections = RaySphereIntersect(RadiusInside, RayStart, RayVec, WorldOrigin, Intersection1, Dist1, Intersection2, Dist2))
		{
		if (Intersections == 1)
			{
			if (NoSubBaseDensity)
				ClosestIn = Dist1;
			else
				ClosestIn = 0.0;
			} // if
		else
			{
			if (NoSubBaseDensity)
				FarthestOut = Dist1;
			ClosestIn = 0.0;
			} // else
		} // if
	if (Intersections = RaySphereIntersect(RadiusOutside, RayStart, RayVec, WorldOrigin, Intersection1, Dist1, Intersection2, Dist2))
		{
		if (Intersections == 1)
			{
			if (Dist1 > FarthestOut)
				FarthestOut = Dist1;
			} // if
		else
			{
			if (Dist1 > ClosestIn)
				ClosestIn = Dist1;
			if (Dist2 > FarthestOut)
				FarthestOut = Dist2;
			} // else
		} // if
	if (ClosestIn > RayHits->RayOnDist)
		RayHits->RayOnDist = ClosestIn;
	if (FarthestOut >= 0.0 && FarthestOut < RayHits->RayOffDist)
		RayHits->RayOffDist = FarthestOut;
	if (RayHits->RayOffDist > RayHits->RayOnDist)
		{
		RayHits->SampleRate = (RayHits->RayOffDist - RayHits->RayOnDist) * .01;
		if (RayHits->SampleRate < AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_MINSAMPLE].CurValue)
			RayHits->SampleRate = AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_MINSAMPLE].CurValue;
		else if (RayHits->SampleRate > AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_MAXSAMPLE].CurValue)
			RayHits->SampleRate = AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_MAXSAMPLE].CurValue;
		Jitter = .5 + (.75 * Rand.GenPRN() -.375);
		RayHits->SegMidXYZ[0] = (RayHits->RayOnDist + RayHits->SampleRate * Jitter) * RayVec[0] + RayStart[0];
		RayHits->SegMidXYZ[1] = (RayHits->RayOnDist + RayHits->SampleRate * Jitter) * RayVec[1] + RayStart[1];
		RayHits->SegMidXYZ[2] = (RayHits->RayOnDist + RayHits->SampleRate * Jitter) * RayVec[2] + RayStart[2];
		RayHits->SegStartDist = RayHits->RayOnDist;
		RayHits->SegEndDist = WCS_min(RayHits->RayOnDist + RayHits->SampleRate, RayHits->RayOffDist);
		RayHits->SegEvaluated = 0;
		} // if
	else
		{
		delete RayHits;
		RayHits = NULL;
		} // else
	} // if

return (RayHits);

} // AtmosphereComponent::BuildRayList

/*===========================================================================*/

int AtmosphereComponent::PointSampleVolumetric(VolumeRayList *CurRay, VertexDEM *CurVert, TextureData *TexData, double &Shading)
{
double Value[3], Coverage, Dens, GradientPos, OpacityUsed;

GradientPos = ElevRange > 0.0 ? (CurVert->Elev - LowElev) / ElevRange: 0.0;

if ((GradientPos < 0.0 && NoSubBaseDensity) || GradientPos > 1.0)
	{
	CurRay->SegOpticalDepth = FLT_MAX;
	CurRay->SegColor[0] = CurRay->SegColor[1] = CurRay->SegColor[2] = 0.0;
	return (0);
	} // if not in atmo zone

// may be needed in calling function
TexData->Object = RAParent;
TexData->ObjectType = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_CLOUD;
OpacityUsed = 0.0;

Dens = DensityProf.GetValue(0, GradientPos) * AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_DENSITY].CurValue;
if (CovgTextureExists)
	{
	TexData->TexRefLon = RefLon;
	TexData->TexRefLat = RefLat;
	TexData->TexRefElev = LowElev;
	TexData->MetersPerDegLon = MetersPerDegLon;
	TexData->MetersPerDegLat = MetersPerDegLat;
	TexData->VDEM[0] = CurVert;
	TexData->VDEM[1] = TexData->VDEM[2] = NULL;
	TexData->VData[0] = TexData->VData[1] = TexData->VData[2] = NULL;
	TexData->PData = NULL;
	TexData->ZDist = CurVert->ScrnXYZ[2];
	TexData->QDist = CurVert->Q;
	TexData->Elev = CurVert->Elev;
	TexData->Latitude = CurVert->Lat;
	TexData->Longitude = CurVert->Lon;
	Value[0] = Value[1] = Value[2] = 0.0;
	TexRoot[WCS_EFFECTS_ATMOCOMPONENT_TEXTURE_COVERAGE]->Eval(Value, TexData);
	// Multiply by VertexIntensity moved up from below 7/1/02 GRH to avoid multiplying wave part by intensity twice
	OpacityUsed += Value[0];
	// adjust for coverage value
	Coverage = CovgProf.GetValue(0, GradientPos) * AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_COVERAGE].CurValue;
	if (Coverage < 1.0)
		{
		if (Coverage > 0.0 && OpacityUsed > 1.0 - Coverage)
			OpacityUsed = (Coverage + OpacityUsed - 1.0) / Coverage;
		else
			OpacityUsed = 0.0;
		} // if
	// adjust for layer density
	if (OpacityUsed <= 0.0)
		OpacityUsed = 0.0;
	else
		OpacityUsed *= Dens;
	} // if
else
	OpacityUsed = Dens;

if (OpacityUsed > 0.0)
	{
	if (OpacityUsed > 1.0)
		OpacityUsed = 1.0;
	CurRay->SegColor[0] = CompleteParticleColor[0];
	CurRay->SegColor[1] = CompleteParticleColor[1];
	CurRay->SegColor[2] = CompleteParticleColor[2];
	Shading = ShadeProf.GetValue(0, GradientPos) * AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_SELFSHADING].CurValue;
	CurRay->SegOpticalDepth = AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_OPTICALDEPTH].CurValue / OpacityUsed;
	return (1);
	} // if

CurRay->SegOpticalDepth = FLT_MAX;
CurRay->SegColor[0] = CurRay->SegColor[1] = CurRay->SegColor[2] = 0.0;

return (0);

} // AtmosphereComponent::PointSampleVolumetric

/*===========================================================================*/

int AtmosphereComponent::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
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
if (ReceiveShadowsMisc)
	ShadowFlags |= WCS_SHADOWTYPE_MISC;

return (1);

} // AtmosphereComponent::InitToRender

/*===========================================================================*/

int AtmosphereComponent::InitFrameToRender(RenderData *Rend)
{
VertexDEM CurVert;

CompleteBacklightColor[0] = BacklightColor.GetCompleteValue(0);
CompleteBacklightColor[1] = BacklightColor.GetCompleteValue(1);
CompleteBacklightColor[2] = BacklightColor.GetCompleteValue(2);
CompleteParticleColor[0] = ParticleColor.GetCompleteValue(0);
CompleteParticleColor[1] = ParticleColor.GetCompleteValue(1);
CompleteParticleColor[2] = ParticleColor.GetCompleteValue(2);

LowElev = AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_BASEELEV].CurValue;
ElevRange = AnimPar[WCS_EFFECTS_ATMOCOMPONENT_ANIMPAR_THICKNESS].CurValue;
HighElev = LowElev + ElevRange;

CovgTextureExists = (TexRoot[WCS_EFFECTS_ATMOCOMPONENT_TEXTURE_COVERAGE] && TexRoot[WCS_EFFECTS_ATMOCOMPONENT_TEXTURE_COVERAGE]->Enabled);

CurVert.Lat = Rend->TexRefLat;
CurVert.Lon = Rend->TexRefLon;
CurVert.Elev = LowElev;
#ifdef WCS_BUILD_VNS
Rend->DefCoords->DegToCart(&CurVert);
#else // WCS_BUILD_VNS
CurVert.DegToCart(Rend->PlanetRad);
#endif // WCS_BUILD_VNS
RadiusInside = VectorMagnitude(CurVert.XYZ);

CurVert.Elev = HighElev;
#ifdef WCS_BUILD_VNS
Rend->DefCoords->DegToCart(&CurVert);
#else // WCS_BUILD_VNS
CurVert.DegToCart(Rend->PlanetRad);
#endif // WCS_BUILD_VNS
RadiusOutside = VectorMagnitude(CurVert.XYZ);

NoSubBaseDensity = (DensityProf.GetValue(0, 0.0) <= 0.0);
RefLon = Rend->TexRefLon;
RefLat = Rend->TexRefLat;
MetersPerDegLon = Rend->RefLonScaleMeters;
MetersPerDegLat = Rend->EarthLatScaleMeters;

return (1);

} // AtmosphereComponent::InitFrameToRender

/*===========================================================================*/

int AtmosphereComponent::BuildFileComponentsList(EffectList **Coords)
{
#ifdef WCS_BUILD_VNS
long Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		if (! GetTexRootPtr(Ct)->BuildFileComponentsList(Coords))
			return (0);
		} // if
	} // for
#endif // WCS_BUILD_VNS

return (1);

} // AtmosphereComponent::BuildFileComponentsList

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int AtmosphereComponent::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
AtmosphereComponent *CurrentAtmoComp = NULL;
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
					else if (! strnicmp(ReadBuf, "AtmoComp", 8))
						{
						if (CurrentAtmoComp = new AtmosphereComponent(NULL))
							{
							if ((BytesRead = CurrentAtmoComp->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if AtmoComp
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

if (Success == 1 && CurrentAtmoComp)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentAtmoComp);
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

} // AtmosphereComponent::LoadObject

/*===========================================================================*/

int AtmosphereComponent::SaveObject(FILE *ffile)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
EffectList *CurEffect, *Coords = NULL;
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

if (BuildFileComponentsList(&Coords))
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
	#endif // WCS_BUILD_VNS

	while (Coords)
		{
		CurEffect = Coords;
		Coords = Coords->Next;
		delete CurEffect;
		} // while
	} // if

// AtmosphereComponent
strcpy(StrBuf, "AtmoComp");
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
			} // if AtmosphereComponent saved 
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

} // AtmosphereComponent::SaveObject

/*===========================================================================*/
/*===========================================================================*/

Atmosphere::Atmosphere()
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_ATMOSPHERE;
SetDefaults();

} // Atmosphere::Atmosphere

/*===========================================================================*/

Atmosphere::Atmosphere(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_ATMOSPHERE;
SetDefaults();

} // Atmosphere::Atmosphere

/*===========================================================================*/

Atmosphere::Atmosphere(RasterAnimHost *RAHost, EffectsLib *Library, Atmosphere *Proto)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_ATMOSPHERE;
if (Library)
	{
	Prev = Library->LastAtmosphere;
	if (Library->LastAtmosphere)
		{
		Library->LastAtmosphere->Next = this;
		Library->LastAtmosphere = this;
		} // if
	else
		{
		Library->Atmospheres = Library->LastAtmosphere = this;
		} // else
	} // if
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
	strcpy(NameBase, "Atmosphere");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // Atmosphere::Atmosphere

/*===========================================================================*/

Atmosphere::~Atmosphere()
{
long Ct;
RootTexture *DelTex;
AtmosphereComponent *CurComponent;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->AEG && GlobalApp->GUIWins->AEG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->AEG;
		GlobalApp->GUIWins->AEG = NULL;
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

while (Components)
	{
	CurComponent = Components;
	Components = Components->Next;
	delete CurComponent;
	} // while

} // Atmosphere::~Atmosphere

/*===========================================================================*/

void Atmosphere::SetDefaults(void)
{
long Ct;
double EffectDefault[WCS_EFFECTS_ATMOSPHERE_NUMANIMPAR] = {0.0, 100000.0, 0.0, 200000.0, 0.0, 100.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0};
double RangeDefaults[WCS_EFFECTS_ATMOSPHERE_NUMANIMPAR][3] = {FLT_MAX, 0.0, 100.0, FLT_MAX, .01, 100.0, 
	FLT_MAX, 0.0, 100.0, FLT_MAX, .01, 100.0,
	FLT_MAX, -FLT_MAX, 10.0,
	FLT_MAX, -FLT_MAX, 10.0,
	1.0, 0.0, .01,
	1.0, 0.0, .01,
	1.0, 0.0, .01,
	1.0, 0.0, .01,
	1.0, 0.0, .01,
	1.0, 0.0, .01,
	};

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
TopAmbientColor.SetDefaults(this, WCS_EFFECTS_ATMOSPHERE_NUMANIMPAR);
TopAmbientColor.SetValue3(111.0 / 255.0, 172.0 / 255.0, 242.0 / 255.0);
TopAmbientColor.Intensity.SetValue(.2);
BottomAmbientColor.SetDefaults(this, WCS_EFFECTS_ATMOSPHERE_NUMANIMPAR + 1);
BottomAmbientColor.SetValue3(214.0 / 255.0, 214.0 / 255.0, 205.0 / 255.0);
BottomAmbientColor.Intensity.SetValue(.1);
HazeColor.SetDefaults(this, WCS_EFFECTS_ATMOSPHERE_NUMANIMPAR + 2);
HazeColor.SetValue3(228.0 / 255.0, 239.0 / 255.0, 251.0 / 255.0);
FogColor.SetDefaults(this, WCS_EFFECTS_ATMOSPHERE_NUMANIMPAR + 3);
FogColor.SetValue3(225.0 / 255.0, 229.0 / 255.0, 232.0 / 255.0);
CloudHazeColor.SetDefaults(this, WCS_EFFECTS_ATMOSPHERE_NUMANIMPAR + 4);
CloudHazeColor.SetValue3(228.0 / 255.0, 239.0 / 255.0, 251.0 / 255.0);
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	TexRoot[Ct] = NULL;
	} // for

Components = NULL;
AtmosphereType = WCS_EFFECTS_ATMOSPHERETYPE_SIMPLE;
HazeEnabled = 1;
FogEnabled = 0;
SeparateCloudHaze = 0;
ActAsFilter = 0;
memset(ODTable, 0, sizeof ODTable);
HazeEndDistance = CloudHazeEndDistance = FogLowElev = FogHighElev = FogRange = HazeIntensityRange = 
	CloudHazeIntensityRange = FogLowIntensity = FogHighIntensity = FogIntensityRange = 0.0;
CompleteTopAmbient[0] = CompleteTopAmbient[1] = CompleteTopAmbient[2] = 
	CompleteBottomAmbient[0] = CompleteBottomAmbient[1] = CompleteBottomAmbient[2] = 
	CompleteHazeColor[0] = CompleteHazeColor[1] = CompleteHazeColor[2] = 
	CompleteFogColor[0] = CompleteFogColor[1] = CompleteFogColor[2] = 
	CompleteCloudHazeColor[0] = CompleteCloudHazeColor[1] = CompleteCloudHazeColor[2] = 0.0;

AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZESTART].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZERANGE].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEV].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEV].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZESTARTINTENSITY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZEENDINTENSITY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEVINTENSITY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEVINTENSITY].SetMultiplier(100.0);

} // Atmosphere::SetDefaults

/*===========================================================================*/

void Atmosphere::Copy(Atmosphere *CopyTo, Atmosphere *CopyFrom)
{
AtmosphereComponent *CurrentFrom = CopyFrom->Components, **ToPtr, *CurComponent;

// delete existing component sources
while (CopyTo->Components)
	{
	CurComponent = CopyTo->Components;
	CopyTo->Components = CopyTo->Components->Next;
	delete CurComponent;
	} // while

ToPtr = &CopyTo->Components;

while (CurrentFrom)
	{
	if (*ToPtr = new AtmosphereComponent(CopyTo))
		{
		(*ToPtr)->Copy(*ToPtr, CurrentFrom);
		} // if
	ToPtr = &(*ToPtr)->Next;
	CurrentFrom = CurrentFrom->Next;
	} // while

CopyTo->AtmosphereType = CopyFrom->AtmosphereType;
CopyTo->HazeEnabled = CopyFrom->HazeEnabled;
CopyTo->SeparateCloudHaze = CopyFrom->SeparateCloudHaze;
CopyTo->ActAsFilter = CopyFrom->ActAsFilter;
CopyTo->FogEnabled = CopyFrom->FogEnabled;
CopyTo->TopAmbientColor.Copy(&CopyTo->TopAmbientColor, &CopyFrom->TopAmbientColor);
CopyTo->BottomAmbientColor.Copy(&CopyTo->BottomAmbientColor, &CopyFrom->BottomAmbientColor);
CopyTo->HazeColor.Copy(&CopyTo->HazeColor, &CopyFrom->HazeColor);
CopyTo->CloudHazeColor.Copy(&CopyTo->CloudHazeColor, &CopyFrom->CloudHazeColor);
CopyTo->FogColor.Copy(&CopyTo->FogColor, &CopyFrom->FogColor);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // Atmosphere::Copy

/*===========================================================================*/

AtmosphereComponent *Atmosphere::AddComponent(AtmosphereComponent *AddMe)
{
AtmosphereComponent **CurComponent = &Components;
NotifyTag Changes[2];

while (*CurComponent)
	{
	CurComponent = &(*CurComponent)->Next;
	} // while
if (*CurComponent = new AtmosphereComponent(this))
	{
	if (AddMe)
		(*CurComponent)->Copy(*CurComponent, AddMe);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
return (*CurComponent);

} // Atmosphere::AddComponent

/*===========================================================================*/

unsigned long Atmosphere::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0, CloudHazeLoaded = 0;
union MultiVal MV;
AtmosphereComponent **LoadTo = &Components;

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
					case WCS_EFFECTS_ATMOSPHERE_ATMOSPHERETYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&AtmosphereType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_HAZEENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&HazeEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_SEPARATECLOUDHAZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&SeparateCloudHaze, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_ACTASFILTER:
						{
						BytesRead = ReadBlock(ffile, (char *)&ActAsFilter, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_FOGENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&FogEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_HAZESTART:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_HAZERANGE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_CLOUDHAZESTART:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZESTART].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_CLOUDHAZERANGE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZERANGE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_FOGNONE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEV].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_FOGFULL:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEV].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_HAZESTARTINTENSITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_HAZEENDINTENSITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_CLOUDHAZESTARTINTENSITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZESTARTINTENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_CLOUDHAZEENDINTENSITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZEENDINTENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_FOGNONEINTENSITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEVINTENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_FOGFULLINTENSITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEVINTENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_TOPAMBIENTCOLOR:
						{
						BytesRead = TopAmbientColor.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_BOTTOMAMBIENTCOLOR:
						{
						BytesRead = BottomAmbientColor.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_HAZECOLOR:
						{
						BytesRead = HazeColor.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_CLOUDHAZECOLOR:
						{
						BytesRead = CloudHazeColor.Load(ffile, Size, ByteFlip);
						CloudHazeLoaded = 1;
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_FOGCOLOR:
						{
						BytesRead = FogColor.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_TEXHAZECOLOR:
						{
						if (TexRoot[WCS_EFFECTS_ATMOSPHERE_TEXTURE_HAZECOLOR] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_ATMOSPHERE_TEXTURE_HAZECOLOR]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_TEXCLOUDHAZECOLOR:
						{
						if (TexRoot[WCS_EFFECTS_ATMOSPHERE_TEXTURE_CLOUDHAZECOLOR] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_ATMOSPHERE_TEXTURE_CLOUDHAZECOLOR]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_TEXFOGCOLOR:
						{
						if (TexRoot[WCS_EFFECTS_ATMOSPHERE_TEXTURE_FOGCOLOR] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_ATMOSPHERE_TEXTURE_FOGCOLOR]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_ATMOSPHERE_ATMOCOMPONENT:
						{
						if (*LoadTo = new AtmosphereComponent(this))
							{
							BytesRead = (*LoadTo)->Load(ffile, Size, ByteFlip);
							LoadTo = &(*LoadTo)->Next;
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

if (! CloudHazeLoaded)
	{
	CloudHazeColor.Copy(&CloudHazeColor, &HazeColor);
	SeparateCloudHaze = 1;
	AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZESTART].Copy(&AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZESTART], &AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART]);
	AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZERANGE].Copy(&AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZERANGE], &AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE]);
	AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZESTARTINTENSITY].Copy(&AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZESTARTINTENSITY], &AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY]);
	AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZEENDINTENSITY].Copy(&AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZEENDINTENSITY], &AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY]);
	AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZESTART].ScaleValues(2.0);
	AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZERANGE].ScaleValues(2.0);
	} // if

return (TotalRead);

} // Atmosphere::Load

/*===========================================================================*/

unsigned long Atmosphere::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long AnimItemTag[WCS_EFFECTS_ATMOSPHERE_NUMANIMPAR] = {WCS_EFFECTS_ATMOSPHERE_HAZESTART,
																	WCS_EFFECTS_ATMOSPHERE_HAZERANGE,
																	WCS_EFFECTS_ATMOSPHERE_CLOUDHAZESTART,
																	WCS_EFFECTS_ATMOSPHERE_CLOUDHAZERANGE,
																	WCS_EFFECTS_ATMOSPHERE_FOGNONE,
																	WCS_EFFECTS_ATMOSPHERE_FOGFULL,
																	WCS_EFFECTS_ATMOSPHERE_HAZESTARTINTENSITY,
																	WCS_EFFECTS_ATMOSPHERE_HAZEENDINTENSITY,
																	WCS_EFFECTS_ATMOSPHERE_CLOUDHAZESTARTINTENSITY,
																	WCS_EFFECTS_ATMOSPHERE_CLOUDHAZEENDINTENSITY,
																	WCS_EFFECTS_ATMOSPHERE_FOGNONEINTENSITY,
																	WCS_EFFECTS_ATMOSPHERE_FOGFULLINTENSITY};
unsigned long TextureItemTag[WCS_EFFECTS_ATMOSPHERE_NUMTEXTURES] = {WCS_EFFECTS_ATMOSPHERE_TEXHAZECOLOR,
																	WCS_EFFECTS_ATMOSPHERE_TEXCLOUDHAZECOLOR,
																	WCS_EFFECTS_ATMOSPHERE_TEXFOGCOLOR};
AtmosphereComponent *Current;

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
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ATMOSPHERE_ATMOSPHERETYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AtmosphereType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ATMOSPHERE_HAZEENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HazeEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ATMOSPHERE_SEPARATECLOUDHAZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&SeparateCloudHaze)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ATMOSPHERE_ACTASFILTER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ActAsFilter)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ATMOSPHERE_FOGENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FogEnabled)) == NULL)
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

ItemTag = WCS_EFFECTS_ATMOSPHERE_TOPAMBIENTCOLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = TopAmbientColor.Save(ffile))
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
			} /* if anim color saved */
		else
			goto WriteError;
		} /* if size written */
	else
		goto WriteError;
	} /* if tag written */
else
	goto WriteError;

ItemTag = WCS_EFFECTS_ATMOSPHERE_BOTTOMAMBIENTCOLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = BottomAmbientColor.Save(ffile))
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
			} /* if anim color saved */
		else
			goto WriteError;
		} /* if size written */
	else
		goto WriteError;
	} /* if tag written */
else
	goto WriteError;

ItemTag = WCS_EFFECTS_ATMOSPHERE_HAZECOLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = HazeColor.Save(ffile))
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
			} /* if anim color saved */
		else
			goto WriteError;
		} /* if size written */
	else
		goto WriteError;
	} /* if tag written */
else
	goto WriteError;

ItemTag = WCS_EFFECTS_ATMOSPHERE_CLOUDHAZECOLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = CloudHazeColor.Save(ffile))
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
			} /* if anim color saved */
		else
			goto WriteError;
		} /* if size written */
	else
		goto WriteError;
	} /* if tag written */
else
	goto WriteError;

ItemTag = WCS_EFFECTS_ATMOSPHERE_FOGCOLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = FogColor.Save(ffile))
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
			} /* if anim color saved */
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
					} // if texture saved
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

Current = Components;
while (Current)
	{
	ItemTag = WCS_EFFECTS_ATMOSPHERE_ATMOCOMPONENT + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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
				} // if component saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	Current = Current->Next;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // Atmosphere::Save

/*===========================================================================*/

char *AtmosphereCritterNames[WCS_EFFECTS_ATMOSPHERE_NUMANIMPAR] = {"Haze Start Distance (m)", "Haze Range Distance (m)",
	"Cloud Haze Start Distance (m)", "Cloud Haze Range Distance (m)", "Fog Low Elevation (m)", "Fog High Elevation (m)", "Haze Start Intensity (%)", "Haze End Intensity (%)",
	"Fog Low Elev Intensity (%)", "Fog High Elev Intensity (%)", "Cloud Haze Start Intensity (%)", "Cloud Haze End Intensity (%)"};
char *AtmosphereTextureNames[WCS_EFFECTS_ATMOSPHERE_NUMTEXTURES] = {"Haze Color", "Cloud Haze Color", "Fog Color"};

char *Atmosphere::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (AtmosphereCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (AtmosphereTextureNames[Ct]);
		} // if
	} // for
if (Test == &TopAmbientColor)
	return ("Top Ambient Color");
if (Test == &BottomAmbientColor)
	return ("Bottom Ambient Color");
if (Test == &HazeColor)
	return ("Haze Color");
if (Test == &CloudHazeColor)
	return ("Cloud Haze Color");
if (Test == &FogColor)
	return ("Fog Color");

return ("");

} // Atmosphere::GetCritterName

/*===========================================================================*/

char *Atmosphere::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as an Atmosphere Texture! Remove anyway?");

} // Atmosphere::OKRemoveRaster

/*===========================================================================*/

char *Atmosphere::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? AtmosphereTextureNames[TexNumber]: (char*)"");

} // Atmosphere::GetTextureName

/*===========================================================================*/

void Atmosphere::GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace)
{

ApplyToColor = 1;
ApplyToDisplace = 0;

} // Atmosphere::GetTextureApplication

/*===========================================================================*/

RootTexture *Atmosphere::NewRootTexture(long TexNumber)
{
char ApplyToColor = 1;
char ApplyToDisplace = 0;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // Atmosphere::NewRootTexture

/*===========================================================================*/

void Atmosphere::Edit(void)
{

DONGLE_INLINE_CHECK()

if(GlobalApp->GUIWins->AEG)
	{
	delete GlobalApp->GUIWins->AEG;
	}
GlobalApp->GUIWins->AEG = new AtmosphereEditGUI(GlobalApp->AppEffects, this);
if(GlobalApp->GUIWins->AEG)
	{
	GlobalApp->GUIWins->AEG->Open(GlobalApp->MainProj);
	}

} // Atmosphere::Edit

/*===========================================================================*/

int Atmosphere::GetRAHostAnimated(void)
{
AtmosphereComponent *Current = Components;

if (GeneralEffect::GetRAHostAnimated())
	return (1);
while (Current)
	{
	if (Current->GetRAHostAnimated())
		return (1);
	Current = Current->Next;
	} // while
if (TopAmbientColor.GetRAHostAnimated())
	return (1);
if (BottomAmbientColor.GetRAHostAnimated())
	return (1);
if (HazeColor.GetRAHostAnimated())
	return (1);
if (CloudHazeColor.GetRAHostAnimated())
	return (1);
if (FogColor.GetRAHostAnimated())
	return (1);

return (0);

} // Atmosphere::GetRAHostAnimated

/*===========================================================================*/

int Atmosphere::SetToTime(double Time)
{
long Found = 0;
AtmosphereComponent *Current = Components;

if (GeneralEffect::SetToTime(Time))
	Found = 1;
while (Current)
	{
	if (Current->SetToTime(Time))
		Found = 1;
	Current = Current->Next;
	} // while
if (TopAmbientColor.SetToTime(Time))
	Found = 1;
if (BottomAmbientColor.SetToTime(Time))
	Found = 1;
if (HazeColor.SetToTime(Time))
	Found = 1;
if (CloudHazeColor.SetToTime(Time))
	Found = 1;
if (FogColor.SetToTime(Time))
	Found = 1;

return (Found);

} // Atmosphere::SetToTime

/*===========================================================================*/

int Atmosphere::RemoveRAHost(RasterAnimHost *RemoveMe)
{
AtmosphereComponent *CurAtmo = Components, *PrevAtmo = NULL;
NotifyTag Changes[2];

while (CurAtmo)
	{
	if (CurAtmo == (AtmosphereComponent *)RemoveMe)
		{
		if (PrevAtmo)
			PrevAtmo->Next = CurAtmo->Next;
		else
			Components = CurAtmo->Next;

		delete CurAtmo;

		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

		return (1);
		} // if
	PrevAtmo = CurAtmo;
	CurAtmo = CurAtmo->Next;
	} // while

return (GeneralEffect::RemoveRAHost(RemoveMe));

} // Atmosphere::RemoveRAHost

/*===========================================================================*/

long Atmosphere::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Found = 0;
AtmosphereComponent *CurAtmo = Components;

if (GeneralEffect::GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
while (CurAtmo)
	{
	if (CurAtmo->GetKeyFrameRange(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	CurAtmo = CurAtmo->Next;
	} // while
if (TopAmbientColor.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (BottomAmbientColor.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (HazeColor.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (CloudHazeColor.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (FogColor.GetKeyFrameRange(MinDist, MaxDist))
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

} // Atmosphere::GetKeyFrameRange

/*===========================================================================*/

char Atmosphere::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ATMOCOMPONENT)
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
	return (1);

return (0);

} // Atmosphere::GetRAHostDropOK

/*===========================================================================*/

int Atmosphere::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char WinNum, QueryStr[256], NameStr[128];
int Success;
long NumListItems = 0;
RasterAnimHostProperties Prop;
RasterAnimHost *TargetList[30];
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (Atmosphere *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (Atmosphere *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ATMOCOMPONENT)
	{
	Success = -1;
	sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		if (AddComponent((AtmosphereComponent *)DropSource->DropSource))
			{
			Success = 1;
			} // if
		} // if
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
	{
	Success = -1;
	TargetList[0] = &TopAmbientColor;
	TargetList[1] = &BottomAmbientColor;
	TargetList[2] = &HazeColor;
	TargetList[3] = &CloudHazeColor;
	TargetList[4] = &FogColor;
	NumListItems = 5;
	} // else if
else
	{
	Success = GeneralEffect::ProcessRAHostDragDrop(DropSource);
	} // else if

if (NumListItems > 0)
	{
	if ((WinNum = GlobalApp->MCP->GetAvailableDragnDropListNumber()) >= 0)
		{
		GlobalApp->GUIWins->DDL[WinNum] = new DragnDropListGUI(WinNum, DropSource, TargetList, NumListItems, this, NULL);
		if(GlobalApp->GUIWins->DDL[WinNum])
			{
			GlobalApp->GUIWins->DDL[WinNum]->Open(GlobalApp->MainProj);
			}
		} // if
	else
		UserMessageOK("Open Drag 'n Drop List", "All List windows are in use.\nTry dropping on a target deeper in the hierarchy\n and closer to your desired target.");
	} // if

return (Success);

} // Atmosphere::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long Atmosphere::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_ATMOSPHERE | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // Atmosphere::GetRAFlags

/*===========================================================================*/

int Atmosphere::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
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
			return (1);
			} // if
		} // for
	if (ChildA == &TopAmbientColor || ChildA == &BottomAmbientColor)
		{
		AnimAffil = (AnimCritter *)ChildA;
		return (1);
		} // if
	if (ChildA == &HazeColor)
		{
		AnimAffil = (AnimCritter *)ChildA;
		TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_ATMOSPHERE_TEXTURE_HAZECOLOR);
		return (1);
		} // if
	if (ChildA == &CloudHazeColor)
		{
		AnimAffil = (AnimCritter *)ChildA;
		TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_ATMOSPHERE_TEXTURE_CLOUDHAZECOLOR);
		return (1);
		} // if
	if (ChildA == &FogColor)
		{
		AnimAffil = (AnimCritter *)ChildA;
		TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_ATMOSPHERE_TEXTURE_FOGCOLOR);
		return (1);
		} // if
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
				case WCS_EFFECTS_ATMOSPHERE_TEXTURE_HAZECOLOR:
					{
					AnimAffil = &HazeColor;
					break;
					} // 
				case WCS_EFFECTS_ATMOSPHERE_TEXTURE_CLOUDHAZECOLOR:
					{
					AnimAffil = &CloudHazeColor;
					break;
					} // 
				case WCS_EFFECTS_ATMOSPHERE_TEXTURE_FOGCOLOR:
					{
					AnimAffil = &FogColor;
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

} // Atmosphere::GetAffiliates

/*===========================================================================*/

RasterAnimHost *Atmosphere::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;
AtmosphereComponent *CurAtmo;

if (! Current)
	return (&TopAmbientColor);
if (Current == &TopAmbientColor)
	return (&BottomAmbientColor);
if (Current == &BottomAmbientColor)
	Found = 1;
if (AtmosphereType < WCS_EFFECTS_ATMOSPHERETYPE_VOLUMETRIC)
	{
	if (Found)
		return (&HazeColor);
	if (Current == &HazeColor)
		return (&CloudHazeColor);
	if (Current == &CloudHazeColor)
		return (&FogColor);
	if (Current == &FogColor)
		Found = 1;
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		if (Found)
			return (GetAnimPtr(Ct));
		if (Current == GetAnimPtr(Ct))
			Found = 1;
		} // for
	} // if
// this loop needs to not be conditional or texture images might be missed if user tries to delete them
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Found && GetTexRootPtr(Ct))
		return (GetTexRootPtr(Ct));
	if (Current == GetTexRootPtr(Ct))
		Found = 1;
	} // for
CurAtmo = Components;
while (CurAtmo)
	{
	if (Found)
		return (CurAtmo);
	if (Current == CurAtmo)
		Found = 1;
	CurAtmo = CurAtmo->Next;
	} // while

return (NULL);

} // Atmosphere::GetRAHostChild

/*===========================================================================*/

long Atmosphere::InitImageIDs(long &ImageID)
{
long Ct, NumImages = 0;
AtmosphereComponent *CurAtmo;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		NumImages += GetTexRootPtr(Ct)->InitImageIDs(ImageID);
		} // if
	} // for

CurAtmo = Components;
while (CurAtmo)
	{
	NumImages += CurAtmo->InitImageIDs(ImageID);
	CurAtmo = CurAtmo->Next;
	} // while

return (NumImages);

} // Atmosphere::InitImageIDs

/*===========================================================================*/

RasterAnimHost *Atmosphere::GetNextGroupSibling(RasterAnimHost *FindMyBrother)
{

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART))
	return (GetAnimPtr(WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE))
	return (GetAnimPtr(WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEV))
	return (GetAnimPtr(WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEV));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEV))
	return (GetAnimPtr(WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEV));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY))
	return (GetAnimPtr(WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY))
	return (GetAnimPtr(WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEVINTENSITY))
	return (GetAnimPtr(WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEVINTENSITY));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEVINTENSITY))
	return (GetAnimPtr(WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEVINTENSITY));

return (NULL);

} // Atmosphere::GetNextGroupSibling

/*===========================================================================*/

int Atmosphere::GetDeletable(RasterAnimHost *Test)
{
AtmosphereComponent *CurAtmo;

CurAtmo = Components;
while (CurAtmo)
	{
	if (Test == CurAtmo)
		return (1);
	CurAtmo = CurAtmo->Next;
	} // while

return (0);

} // Atmosphere::GetDeletable

/*===========================================================================*/

int Atmosphere::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{
long Ct;
AtmosphereComponent *CurComponent;
 
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->Enabled)
		{
		if (! GetTexRootPtr(Ct)->InitAAChain())
			{
			return (0);
			} // if
		} // if
	} // for
if (AtmosphereType == WCS_EFFECTS_ATMOSPHERETYPE_VOLUMETRIC)
	{
	CurComponent = Components;
	while (CurComponent)
		{
		CurComponent->InitToRender(Opt, Buffers);
		CurComponent = CurComponent->Next;
		} // while
	} // if

return (1);

} // Atmosphere::InitToRender

/*===========================================================================*/

int Atmosphere::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{
AtmosphereComponent *CurComponent;

// simple atmospheres
HazeEndDistance = AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].CurValue + AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE].CurValue;
CloudHazeEndDistance = AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZESTART].CurValue + AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZERANGE].CurValue;
FogLowElev = min(AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEV].CurValue, AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEV].CurValue);
FogHighElev = max(AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEV].CurValue, AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEV].CurValue);
if (Rend->ExagerateElevLines)
	{
	FogLowElev = Rend->ElevDatum + (FogLowElev - Rend->ElevDatum) * Rend->Exageration;
	FogHighElev = Rend->ElevDatum + (FogHighElev - Rend->ElevDatum) * Rend->Exageration;
	} // if
FogRange = FogHighElev - FogLowElev;
HazeIntensityRange = AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY].CurValue - AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY].CurValue;
CloudHazeIntensityRange = AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZEENDINTENSITY].CurValue - AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZESTARTINTENSITY].CurValue;
FogLowIntensity = AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEV].CurValue <= AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEV].CurValue ?
	AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEVINTENSITY].CurValue: AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEVINTENSITY].CurValue;
FogHighIntensity = AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEV].CurValue <= AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEV].CurValue ?
	AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEVINTENSITY].CurValue: AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEVINTENSITY].CurValue;
FogIntensityRange = FogHighIntensity - FogLowIntensity;

CompleteTopAmbient[0] = TopAmbientColor.GetCompleteValue(0);
CompleteTopAmbient[1] = TopAmbientColor.GetCompleteValue(1);
CompleteTopAmbient[2] = TopAmbientColor.GetCompleteValue(2);
CompleteBottomAmbient[0] = BottomAmbientColor.GetCompleteValue(0);
CompleteBottomAmbient[1] = BottomAmbientColor.GetCompleteValue(1);
CompleteBottomAmbient[2] = BottomAmbientColor.GetCompleteValue(2);
CompleteHazeColor[0] = HazeColor.GetCompleteValue(0);
CompleteHazeColor[1] = HazeColor.GetCompleteValue(1);
CompleteHazeColor[2] = HazeColor.GetCompleteValue(2);
CompleteCloudHazeColor[0] = CloudHazeColor.GetCompleteValue(0);
CompleteCloudHazeColor[1] = CloudHazeColor.GetCompleteValue(1);
CompleteCloudHazeColor[2] = CloudHazeColor.GetCompleteValue(2);
CompleteFogColor[0] = FogColor.GetCompleteValue(0);
CompleteFogColor[1] = FogColor.GetCompleteValue(1);
CompleteFogColor[2] = FogColor.GetCompleteValue(2);

// volumetric atmospheres
if (AtmosphereType == WCS_EFFECTS_ATMOSPHERETYPE_VOLUMETRIC)
	{
	CurComponent = Components;
	while (CurComponent)
		{
		CurComponent->InitFrameToRender(Rend);
		CurComponent = CurComponent->Next;
		} // while
	} // if

return (1);

} // Atmosphere::InitFrameToRender

/*===========================================================================*/

void Atmosphere::EvalColor(PixelData *Pix, int ColorType)
{
double Value[3], TexOpacity;
RootTexture *TR;

if (ColorType == WCS_EFFECTS_ATMOSPHERECOLORTYPE_HAZE)
	{
	CompleteColor[0] = CompleteHazeColor[0];
	CompleteColor[1] = CompleteHazeColor[1];
	CompleteColor[2] = CompleteHazeColor[2];
	TR = TexRoot[WCS_EFFECTS_ATMOSPHERE_TEXTURE_HAZECOLOR];
	} // if
else if (ColorType == WCS_EFFECTS_ATMOSPHERECOLORTYPE_CLOUDHAZE)
	{
	CompleteColor[0] = CompleteCloudHazeColor[0];
	CompleteColor[1] = CompleteCloudHazeColor[1];
	CompleteColor[2] = CompleteCloudHazeColor[2];
	TR = TexRoot[WCS_EFFECTS_ATMOSPHERE_TEXTURE_CLOUDHAZECOLOR];
	} // if
else
	{
	CompleteColor[0] = CompleteFogColor[0];
	CompleteColor[1] = CompleteFogColor[1];
	CompleteColor[2] = CompleteFogColor[2];
	TR = TexRoot[WCS_EFFECTS_ATMOSPHERE_TEXTURE_FOGCOLOR];
	} // if

if (TR && TR->Enabled)
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	if ((TexOpacity = TR->Eval(Value, Pix->TexData)) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			TexOpacity = 1.0 - TexOpacity;
			CompleteColor[0] = CompleteColor[0] * TexOpacity + Value[0];
			CompleteColor[1] = CompleteColor[1] * TexOpacity + Value[1];
			CompleteColor[2] = CompleteColor[2] * TexOpacity + Value[2];
			} // if
		else
			{
			CompleteColor[0] = Value[0];
			CompleteColor[1] = Value[1];
			CompleteColor[2] = Value[2];
			} // else
		} // if
	} // if

} // Atmosphere::EvalColor

/*===========================================================================*/

int Atmosphere::BuildFileComponentsList(EffectList **Coords)
{
AtmosphereComponent *CurComponent;

CurComponent = Components;
while (CurComponent)
	{
	if (! CurComponent->BuildFileComponentsList(Coords))
		return (0);
	CurComponent = CurComponent->Next;
	} // while

return (1);

} // Atmosphere::BuildFileComponentsList

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int Atmosphere::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
Atmosphere *CurrentAtmosphere = NULL;
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
					else if (! strnicmp(ReadBuf, "Atmosphr", 8))
						{
						if (CurrentAtmosphere = new Atmosphere(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentAtmosphere->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if Atmosphere
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

if (Success == 1 && CurrentAtmosphere)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentAtmosphere);
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

} // Atmosphere::LoadObject

/*===========================================================================*/

int Atmosphere::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
EffectList *CurEffect, *Coords = NULL;
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

if (BuildFileComponentsList(&Coords))
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
	#endif // WCS_BUILD_VNS

	while (Coords)
		{
		CurEffect = Coords;
		Coords = Coords->Next;
		delete CurEffect;
		} // while
	} // if

// Atmosphere
strcpy(StrBuf, "Atmosphr");
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
			} // if Atmosphere saved 
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

} // Atmosphere::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::Atmosphere_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
Atmosphere *Current;
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

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
					case WCS_EFFECTSBASE_ATMOSPHERE_SPEEDBOOST:
						{
						BytesRead = ReadBlock(ffile, (char *)&AtmoBase.SpeedBoost, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_ATMOSPHERE_ADAPTIVETHRESH:
						{
						BytesRead = ReadBlock(ffile, (char *)&AtmoBase.AdaptiveThreshold, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new Atmosphere(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (Atmosphere *)FindDuplicateByName(Current->EffectType, Current))
								{
								RemoveRAHost(Current, 1);
								Current = NULL;
								} // if
							} // if
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

} // EffectsLib::Atmosphere_Load()

/*===========================================================================*/

ULONG EffectsLib::Atmosphere_Save(FILE *ffile)
{
Atmosphere *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_ATMOSPHERE_SPEEDBOOST, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AtmoBase.SpeedBoost)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_ATMOSPHERE_ADAPTIVETHRESH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&AtmoBase.AdaptiveThreshold)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

Current = Atmospheres;
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
					} // if atmosphere saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (Atmosphere *)Current->Next;
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

} // EffectsLib::Atmosphere_Save()

/*===========================================================================*/
/*===========================================================================*/
