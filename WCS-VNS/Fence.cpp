// Fence.cpp
// For managing Fence Effects
// Built from scratch on 3/16/01 by Gary R. Huber
// Copyright 2001 by Questar Productions. All rights reserved.

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "AppMem.h"
#include "Conservatory.h"
#include "FenceEditGUI.h"
#include "Project.h"
#include "Types.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Log.h"
#include "GraphData.h"
#include "requester.h"
#include "Render.h"
#include "Database.h"
#include "Security.h"
#include "Raster.h"
#include "DEM.h"
#include "Lists.h"
#include "FeatureConfig.h"

FenceBase::FenceBase(void)
{

SetDefaults();

} // FenceBase::FenceBase

/*===========================================================================*/

void FenceBase::SetDefaults(void)
{

VerticesToRender = 0;
VertList = NULL;

} // FenceBase::SetDefaults

/*===========================================================================*/

int FenceBase::Init(Database *DBHost, RenderJoeList *&JL)
{
#ifndef WCS_FENCE_LIMITED
long PtCt;
#endif // WCS_FENCE_LIMITED
long Ct, VertexCt;
RenderJoeList *CurJL;
VectorPoint *CurPt, *NextPt;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
#if defined WCS_ISLAND_EFFECTS && defined WCS_FENCE_VECPOLY_ROOF
LayerStub *MultiPartStub;
LayerEntry *MultiPartLayer;
RenderJoeList *MatchJL;
JoeList **NegJLPtr;
#endif // WCS_ISLAND_EFFECTS && WCS_FENCE_VECPOLY_ROOF
VertexDEM MyVert;

if (VertList)
	delete [] VertList;
VertList = NULL;
VerticesToRender = 0;

#if defined WCS_ISLAND_EFFECTS && defined WCS_FENCE_VECPOLY_ROOF
// sort by diminishing absolute value of area
for (CurJL = JL; CurJL; CurJL = (RenderJoeList *)CurJL->Next)
	{
	if (CurJL->Me)
		{
		CurJL->Area = CurJL->Me->ComputeAreaDegrees();
		} // if
	} // for
JL = JL->SortByAbsArea(WCS_JOELIST_SORTORDER_HILO);
#endif // WCS_ISLAND_EFFECTS && defined WCS_FENCE_VECPOLY_ROOF


CurJL = JL;
while (CurJL)
	{
	CurJL->Me->RemoveDuplicatePoints(DBHost);
	if (CurJL->Me && CurJL->Me->GetNumRealPoints() > 0)
		{
		#ifndef WCS_FENCE_LIMITED
		if (! ((Fence *)CurJL->Effect)->SkinFrame)
		#endif // WCS_FENCE_LIMITED
			{
			if (((Fence *)CurJL->Effect)->PostsEnabled)
				VerticesToRender += (CurJL->Me->GetNumRealPoints());
			if (((Fence *)CurJL->Effect)->SpansEnabled && CurJL->Me->GetNumRealPoints() > 1)
				{
				VerticesToRender += (CurJL->Me->GetNumRealPoints() - 1);
				if (((Fence *)CurJL->Effect)->ConnectToOrigin)
					{
					for (CurPt = CurJL->Me->GetFirstRealPoint(); CurPt; CurPt = CurPt->Next)
						{
						if (! CurPt->Next)
							{
							if (! CurPt->SamePoint(CurJL->Me->GetFirstRealPoint()))
								VerticesToRender ++;
							break;
							} // if last point
						} // for
					} // if
				} // if
			} // if
		#ifndef WCS_FENCE_LIMITED
		if ((((Fence *)CurJL->Effect)->RoofEnabled || ((Fence *)CurJL->Effect)->SkinFrame) && CurJL->Me->GetNumRealPoints() > 2)
			{
			#if defined WCS_ISLAND_EFFECTS && defined WCS_FENCE_VECPOLY_ROOF
			// find out if there are multiparts and if this is a positive or negative area
			if (CurJL->Me->GetMultiPartLayer())
				{
				if (CurJL->Area > 0.0)
					VerticesToRender += 1;
				} // if
			else
			#endif // WCS_ISLAND_EFFECTS && defined WCS_FENCE_VECPOLY_ROOF
				VerticesToRender += 1;
			} // if
		#endif // WCS_FENCE_LIMITED
		} // if
	CurJL = (RenderJoeList *)CurJL->Next;
	} // while
if (VerticesToRender > 0)
	{
	if (VertList = new FenceVertexList[VerticesToRender])
		{
		for (CurJL = JL, Ct = 0; CurJL; CurJL = (RenderJoeList *)CurJL->Next)
			{
			if (CurJL->Me && CurJL->Me->GetNumRealPoints() > 0)
				{
				if (MyAttr = (JoeCoordSys *)CurJL->Me->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
					MyCoords = MyAttr->Coord;
				else
					MyCoords = NULL;
				#ifndef WCS_FENCE_LIMITED
				if (! ((Fence *)CurJL->Effect)->SkinFrame)
				#endif // WCS_FENCE_LIMITED
					{
					for (VertexCt = 0, CurPt = CurJL->Me->GetFirstRealPoint(); CurPt; CurPt = CurPt->Next, VertexCt ++)
						{
						if (CurPt->ProjToDefDeg(MyCoords, &MyVert))
							{
							if (((Fence *)CurJL->Effect)->PostsEnabled)
								{
								if ((((Fence *)CurJL->Effect)->AllOrKeyPosts == WCS_EFFECTS_FENCE_POSTCOUNT_ALL) || ! (VertexCt % ((Fence *)CurJL->Effect)->KeyPostInterval) || 
									(VertexCt == 0 && ((Fence *)CurJL->Effect)->FirstKeyPost) || (! CurPt->Next && ((Fence *)CurJL->Effect)->LastKeyPost))
									VertList[Ct].PieceType = WCS_FENCEPIECE_POST;
								else
									VertList[Ct].PieceType = WCS_FENCEPIECE_ALTPOST;
								VertList[Ct].Fnce = (Fence *)CurJL->Effect;
								VertList[Ct].Vec = CurJL->Me;
								VertList[Ct].Lat = MyVert.Lat;
								VertList[Ct].Lon = MyVert.Lon;
								VertList[Ct].Elev = MyVert.Elev;
								VertList[Ct ++].PointA = CurPt;
								} // if
							if (((Fence *)CurJL->Effect)->SpansEnabled && CurJL->Me->GetNumRealPoints() > 1 && (CurPt->Next || (((Fence *)CurJL->Effect)->ConnectToOrigin && ! CurPt->SamePoint(CurJL->Me->GetFirstRealPoint()))))
								{
								if (! CurPt->Next || CurPt->Next->SamePoint(CurJL->Me->GetFirstRealPoint()))
									NextPt = CurJL->Me->GetFirstRealPoint();
								else
									NextPt = CurPt->Next;
								VertList[Ct].Fnce = (Fence *)CurJL->Effect;
								VertList[Ct].Vec = CurJL->Me;
								VertList[Ct].PieceType = WCS_FENCEPIECE_SPAN;
								VertList[Ct].Lat = MyVert.Lat;
								VertList[Ct].Lon = MyVert.Lon;
								VertList[Ct].Elev = MyVert.Elev;
								NextPt->ProjToDefDeg(MyCoords, &MyVert);
								VertList[Ct].Lat = (VertList[Ct].Lat + MyVert.Lat) * .5;
								VertList[Ct].Lon = (VertList[Ct].Lon + MyVert.Lon) * .5;
								VertList[Ct].Elev = (VertList[Ct].Elev + MyVert.Elev) * .5;
								VertList[Ct].PointA = CurPt;
								VertList[Ct ++].PointB = NextPt;
								} // if
							} // if
						else
							VerticesToRender --;
						} // for
					} // if
				#ifndef WCS_FENCE_LIMITED
				if ((((Fence *)CurJL->Effect)->RoofEnabled || ((Fence *)CurJL->Effect)->SkinFrame) && CurJL->Me->GetNumRealPoints() > 2)
					{
					#if defined WCS_ISLAND_EFFECTS && defined WCS_FENCE_VECPOLY_ROOF
					// find out if there are multiparts and if this is a positive or negative area
					if (MultiPartLayer = CurJL->Me->GetMultiPartLayer())
						{
						NegJLPtr = &VertList[Ct].Holes;
						// if this is a negative area of a multipart vector, don't draw it
						if (CurJL->Area <= 0.0)
							continue;
						// get first layer stub
						MultiPartStub = MultiPartLayer->FirstStub();
						for (MultiPartStub = MultiPartLayer->FirstStub(); MultiPartStub; MultiPartStub = MultiPartStub->NextObjectInLayer())
							{
							// find the RJL that corresponds by walking the joe list
							MatchJL = CurJL;
							for (MatchJL = CurJL; MatchJL; MatchJL = (RenderJoeList *)MatchJL->Next)
								{
								// both Joe and Effect need to match
								if (MatchJL->Me == MultiPartStub->MyObject() && MatchJL->Effect == CurJL->Effect)
									{
									// must have same file parent as the outside object or don't even consider it
									if (MatchJL->Me->Parent == CurJL->Me->Parent)
										{
										// if negative area and this vector is in same overall region draw into buffer
										if (MatchJL->Area < 0.0 && CurJL->Me->IsJoeContainedInMyGeoBounds(MatchJL->Me))
											{
											if (! (*NegJLPtr = new JoeList()))
												{
												goto ErrorReturn;
												} // if
											(*NegJLPtr)->Me = MatchJL->Me;
											NegJLPtr = &(*NegJLPtr)->Next;
											} // if
										} // if
									break;
									} // if
								} // for
							} // for
						} // if
					#endif // WCS_ISLAND_EFFECTS && defined WCS_FENCE_VECPOLY_ROOF
					if (CurJL->Me->GetFirstRealPoint()->ProjToDefDeg(MyCoords, &MyVert))
						{
						VertList[Ct].Fnce = (Fence *)CurJL->Effect;
						VertList[Ct].Vec = CurJL->Me;
						VertList[Ct].PieceType = ((Fence *)CurJL->Effect)->SkinFrame ? WCS_FENCEPIECE_SKINFRAME: WCS_FENCEPIECE_ROOF;
						VertList[Ct].Lat = MyVert.Lat;
						VertList[Ct].Lon = MyVert.Lon;
						VertList[Ct].Elev = MyVert.Elev;
						VertList[Ct].PointA = CurJL->Me->GetFirstRealPoint();
						CurPt = CurJL->Me->GetSecondRealPoint();
						PtCt = 1;
						while (CurPt)
							{
							CurPt->ProjToDefDeg(MyCoords, &MyVert);
							VertList[Ct].Lat += MyVert.Lat;
							VertList[Ct].Lon += MyVert.Lon;
							VertList[Ct].Elev += MyVert.Elev;
							CurPt = CurPt->Next;
							PtCt ++;
							} // while
						VertList[Ct].Lat /= PtCt;
						VertList[Ct].Lon /= PtCt;
						VertList[Ct ++].Elev /= PtCt;
						} // if
					else
						VerticesToRender --;
					} // if
				#endif // WCS_FENCE_LIMITED
				} // if
			} // for
		return (1);
		} // if
	} // if

#if defined WCS_ISLAND_EFFECTS && defined WCS_FENCE_VECPOLY_ROOF

ErrorReturn:

#endif // WCS_ISLAND_EFFECTS && WCS_FENCE_VECPOLY_ROOF

return (0);

} // FenceBase::Init

/*===========================================================================*/

void FenceBase::Destroy(void)
{

if (VertList)
	delete [] VertList;
VertList = NULL;
VerticesToRender = 0;

} // FenceBase::Destroy

/*===========================================================================*/

void FenceBase::FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt, DEM *CurDEM)
{
long VertCt;
VertexDEM ObjVert;

if (! CurDEM)
	return;

for (VertCt = 0; VertCt < VerticesToRender; VertCt ++)
	{
	if (! VertList[VertCt].Rendered)
		{
		if (CurDEM->GeographicPointContained(Rend->DefCoords, VertList[VertCt].Lat, VertList[VertCt].Lon, TRUE))
			{
			ObjVert.Lat = VertList[VertCt].Lat;
			ObjVert.Lon = VertList[VertCt].Lon;
			ObjVert.Elev = VertList[VertCt].Elev;
			Rend->Cam->ProjectVertexDEM(Rend->DefCoords, &ObjVert, Rend->EarthLatScaleMeters, Rend->PlanetRad, 1);
			PolyArray[PolyCt].PolyType = WCS_POLYSORTTYPE_FENCE;
			PolyArray[PolyCt].PolyNumber = (unsigned long)&VertList[VertCt];
			PolyArray[PolyCt ++].PolyQ = (float)(ObjVert.ScrnXYZ[2] - Rend->ShadowMapDistanceOffset);
			VertList[VertCt].Rendered = 1;
			} // if
		} // if
	} // for

} // FenceBase::FillRenderPolyArray

/*===========================================================================*/

void FenceBase::FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt, 
	CoordSys *MyCoords, GeoRegister *MyBounds)
{
long VertCt;
VertexDEM ObjVert;

if (! MyBounds)
	return;

for (VertCt = 0; VertCt < VerticesToRender; VertCt ++)
	{
	if (! VertList[VertCt].Rendered)
		{
		if (MyBounds->GeographicPointContained(MyCoords, Rend->DefCoords, VertList[VertCt].Lat, VertList[VertCt].Lon))
			{
			ObjVert.Lat = VertList[VertCt].Lat;
			ObjVert.Lon = VertList[VertCt].Lon;
			ObjVert.Elev = VertList[VertCt].Elev;
			#ifdef WCS_BUILD_VNS
			Rend->DefCoords->DegToCart(&ObjVert);
			#else // WCS_BUILD_VNS
			ObjVert.DegToCart(Rend->PlanetRad);
			#endif // WCS_BUILD_VNS
			PolyArray[PolyCt].PolyType = WCS_POLYSORTTYPE_FENCE;
			PolyArray[PolyCt].PolyNumber = (unsigned long)&VertList[VertCt];
			PolyArray[PolyCt ++].PolyQ = 1.0f;
			VertList[VertCt].Rendered = 1;
			} // if
		} // if
	} // for

} // FenceBase::FillRenderPolyArray

/*===========================================================================*/

int FenceBase::InitFrameToRender(void)
{
long VertCt;

if (VertList)
	{
	for (VertCt = 0; VertCt < VerticesToRender; VertCt ++)
		{
		VertList[VertCt].Rendered = 0;
		} // for
	} // if

return (1);

} // FenceBase::InitFrameToRender

/*===========================================================================*/
/*===========================================================================*/

FenceVertexList::~FenceVertexList()
{

for (JoeList *CurHole = Holes; Holes; CurHole = Holes)
	{
	Holes = Holes->Next;
	delete CurHole;
	} // for

} // FenceVertexList::~FenceVertexList

/*===========================================================================*/
/*===========================================================================*/

Fence::Fence()
: GeneralEffect(NULL), SpanMat(this, 1, WCS_EFFECTS_MATERIALTYPE_FENCE), RoofMat(this, 1, WCS_EFFECTS_MATERIALTYPE_FENCE)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_FENCE;
SetDefaults();

} // Fence::Fence

/*===========================================================================*/

Fence::Fence(RasterAnimHost *RAHost)
: GeneralEffect(RAHost), SpanMat(this, 1, WCS_EFFECTS_MATERIALTYPE_FENCE), RoofMat(this, 1, WCS_EFFECTS_MATERIALTYPE_FENCE)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_FENCE;
SetDefaults();

} // Fence::Fence

/*===========================================================================*/

Fence::Fence(RasterAnimHost *RAHost, EffectsLib *Library, Fence *Proto)
: GeneralEffect(RAHost), SpanMat(this, 1, WCS_EFFECTS_MATERIALTYPE_FENCE), RoofMat(this, 1, WCS_EFFECTS_MATERIALTYPE_FENCE)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_FENCE;
Prev = Library->LastFence;
if (Library->LastFence)
	{
	Library->LastFence->Next = this;
	Library->LastFence = this;
	} // if
else
	{
	Library->Fences = Library->LastFence = this;
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
	strcpy(NameBase, "Wall");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // Fence::Fence

/*===========================================================================*/

Fence::~Fence()
{
long Ct;
RootTexture *DelTex;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->FCG && GlobalApp->GUIWins->FCG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->FCG;
		GlobalApp->GUIWins->FCG = NULL;
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

} // Fence::~Fence

/*===========================================================================*/

void Fence::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_FENCE_NUMANIMPAR] = {/*1.0, 0.0,*/ 1.0, 0.0, 1.0, 0.0, 0.0};
#ifndef WCS_FENCE_LIMITED
double RangeDefaults[WCS_EFFECTS_FENCE_NUMANIMPAR][3] = {//1000000.0, -1000000.0, 1.0,		// post top elev
															//1000000.0, -1000000.0, 1.0,	// post bot elev
															1000000.0, -1000000.0, 1.0,	// span top elev
															1000000.0, -1000000.0, 1.0,	// span bottom elev
															1000000.0, -1000000.0, 1.0,	// roof elev
															1.0, 0.0, .01,				// span mat driver
															1.0, 0.0, .01};				// roof mat driver
#else // WCS_FENCE_LIMITED
double RangeDefaults[WCS_EFFECTS_FENCE_NUMANIMPAR][3] = {//1000000.0, -1000000.0, 1.0,		// post top elev
															//1000000.0, -1000000.0, 1.0,	// post bot elev
															5.0, -10.0, 1.0,	// span top elev
															5.0, -10.0, 1.0,	// span bottom elev
															5.0, 0.0, 1.0,	// roof elev
															1.0, 0.0, .01,				// span mat driver
															1.0, 0.0, .01};				// roof mat driver
#endif // WCS_FENCE_LIMITED
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

PostsEnabled = RoofEnabled = FALSE;
SpansEnabled = TRUE;
KeyPostType = AltPostType = WCS_EFFECTSSUBCLASS_OBJECT3D;
AllOrKeyPosts = WCS_EFFECTS_FENCE_POSTCOUNT_ALL;
FirstKeyPost = LastKeyPost = TRUE;
PostHtFromObject = AltPostHtFromObject = FALSE;
Absolute = WCS_EFFECT_RELATIVETOJOE;
KeyPostInterval = 5;
SeparateRoofMat = ConnectToOrigin = SkinFrame = 0;
KeyPost = AltPost = NULL;
ClickQueryEnabled = 0;

//AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_POSTTOPELEV].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
//AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_POSTBOTELEV].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANTOPELEV].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANBOTELEV].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_ROOFELEV].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANMATDRIVER].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_ROOFMATDRIVER].SetMultiplier(100.0);

} // Fence::SetDefaults

/*===========================================================================*/

void Fence::Copy(Fence *CopyTo, Fence *CopyFrom)
{
long Result = -1;
NotifyTag Changes[2];


SpanMat.Copy(&CopyTo->SpanMat, &CopyFrom->SpanMat);
RoofMat.Copy(&CopyTo->RoofMat, &CopyFrom->RoofMat);

Result = -1;	// reinitialize - new object type to copy

CopyTo->KeyPost = NULL;
if (CopyFrom->KeyPost)
	{
	if (CopyFrom->KeyPostType == WCS_EFFECTSSUBCLASS_OBJECT3D)
		{
		if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
			{
			CopyTo->KeyPost = (Object3DEffect *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->KeyPost);
			} // if no need to make another copy, its all in the family
		else
			{
			if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->KeyPost->EffectType, CopyFrom->KeyPost->Name))
				{
				Result = UserMessageCustom("Copy Fence", "How do you wish to resolve 3D Object and Foliage Effect name collisions?\n\nLink to existing items, replace existing items, or create new items?",
					"Link", "Create", "Overwrite", 1);
				} // if
			if (Result <= 0)
				{
				CopyTo->KeyPost = (Object3DEffect *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->KeyPost->EffectType, NULL, CopyFrom->KeyPost);
				} // if create new
			else if (Result == 1)
				{
				CopyTo->KeyPost = (Object3DEffect *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->KeyPost);
				} // if link to existing
			else if (CopyTo->KeyPost = (Object3DEffect *)GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->KeyPost->EffectType, CopyFrom->KeyPost->Name))
				{
				((Object3DEffect *)CopyTo->KeyPost)->Copy((Object3DEffect *)CopyTo->KeyPost, (Object3DEffect *)CopyFrom->KeyPost);
				Changes[0] = MAKE_ID(CopyTo->KeyPost->GetNotifyClass(), CopyTo->KeyPost->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, CopyTo->KeyPost);
				} // else if found and overwrite
			else
				{
				CopyTo->KeyPost = (Object3DEffect *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->KeyPost->EffectType, NULL, CopyFrom->KeyPost);
				} // else
			} // else better copy or overwrite it since its important to get just the right object
		}  // if
	else
		{
		if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
			{
			CopyTo->KeyPost = (FoliageEffect *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->KeyPost);
			} // if no need to make another copy, its all in the family
		else
			{
			if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->KeyPost->EffectType, CopyFrom->KeyPost->Name))
				{
				Result = UserMessageCustom("Copy Fence", "How do you wish to resolve Foliage Effect and 3D Object name collisions?\n\nLink to existing items, replace existing items, or create new items?",
					"Link", "Create", "Overwrite", 1);
				} // if
			if (Result <= 0)
				{
				CopyTo->KeyPost = (FoliageEffect *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->KeyPost->EffectType, NULL, CopyFrom->KeyPost);
				} // if create new
			else if (Result == 1)
				{
				CopyTo->KeyPost = (FoliageEffect *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->KeyPost);
				} // if link to existing
			else if (CopyTo->KeyPost = (FoliageEffect *)GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->KeyPost->EffectType, CopyFrom->KeyPost->Name))
				{
				((FoliageEffect *)CopyTo->KeyPost)->Copy((FoliageEffect *)CopyTo->KeyPost, (FoliageEffect *)CopyFrom->KeyPost);
				Changes[0] = MAKE_ID(CopyTo->KeyPost->GetNotifyClass(), CopyTo->KeyPost->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, CopyTo->KeyPost);
				} // else if found and overwrite
			else
				{
				CopyTo->KeyPost = (FoliageEffect *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->KeyPost->EffectType, NULL, CopyFrom->KeyPost);
				} // else
			} // else better copy or overwrite it since its important to get just the right object
		}  // if
	} // if

CopyTo->AltPost = NULL;
if (CopyFrom->AltPost)
	{
	if (CopyFrom->KeyPostType == WCS_EFFECTSSUBCLASS_OBJECT3D)
		{
		if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
			{
			CopyTo->AltPost = (Object3DEffect *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->AltPost);
			} // if no need to make another copy, its all in the family
		else
			{
			if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->AltPost->EffectType, CopyFrom->AltPost->Name))
				{
				Result = UserMessageCustom("Copy Fence", "How do you wish to resolve 3D Object and Foliage Effect name collisions?\n\nLink to existing items, replace existing items, or create new items?",
					"Link", "Create", "Overwrite", 1);
				} // if
			if (Result <= 0)
				{
				CopyTo->AltPost = (Object3DEffect *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->AltPost->EffectType, NULL, CopyFrom->AltPost);
				} // if create new
			else if (Result == 1)
				{
				CopyTo->AltPost = (Object3DEffect *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->AltPost);
				} // if link to existing
			else if (CopyTo->AltPost = (Object3DEffect *)GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->AltPost->EffectType, CopyFrom->AltPost->Name))
				{
				((Object3DEffect *)CopyTo->AltPost)->Copy((Object3DEffect *)CopyTo->AltPost, (Object3DEffect *)CopyFrom->AltPost);
				Changes[0] = MAKE_ID(CopyTo->AltPost->GetNotifyClass(), CopyTo->AltPost->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, CopyTo->AltPost);
				} // else if found and overwrite
			else
				{
				CopyTo->AltPost = (Object3DEffect *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->AltPost->EffectType, NULL, CopyFrom->AltPost);
				} // else
			} // else better copy or overwrite it since its important to get just the right object
		} // if
	else
		{
		if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
			{
			CopyTo->AltPost = (FoliageEffect *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->AltPost);
			} // if no need to make another copy, its all in the family
		else
			{
			if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->AltPost->EffectType, CopyFrom->AltPost->Name))
				{
				Result = UserMessageCustom("Copy Fence", "How do you wish to resolve Foliage Effect and 3D Object name collisions?\n\nLink to existing items, replace existing items, or create new items?",
					"Link", "Create", "Overwrite", 1);
				} // if
			if (Result <= 0)
				{
				CopyTo->AltPost = (FoliageEffect *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->AltPost->EffectType, NULL, CopyFrom->AltPost);
				} // if create new
			else if (Result == 1)
				{
				CopyTo->AltPost = (FoliageEffect *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->AltPost);
				} // if link to existing
			else if (CopyTo->AltPost = (FoliageEffect *)GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->AltPost->EffectType, CopyFrom->AltPost->Name))
				{
				((FoliageEffect *)CopyTo->AltPost)->Copy((FoliageEffect *)CopyTo->AltPost, (FoliageEffect *)CopyFrom->AltPost);
				Changes[0] = MAKE_ID(CopyTo->AltPost->GetNotifyClass(), CopyTo->AltPost->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, CopyTo->AltPost);
				} // else if found and overwrite
			else
				{
				CopyTo->AltPost = (FoliageEffect *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->AltPost->EffectType, NULL, CopyFrom->AltPost);
				} // else
			} // else better copy or overwrite it since its important to get just the right object
		}  // if
	} // if

CopyTo->PostsEnabled = CopyFrom->PostsEnabled;
CopyTo->SpansEnabled = CopyFrom->SpansEnabled;
CopyTo->RoofEnabled = CopyFrom->RoofEnabled;
CopyTo->KeyPostType = CopyFrom->KeyPostType;
CopyTo->AltPostType = CopyFrom->AltPostType;
CopyTo->AllOrKeyPosts = CopyFrom->AllOrKeyPosts;
CopyTo->FirstKeyPost = CopyFrom->FirstKeyPost;
CopyTo->LastKeyPost = CopyFrom->LastKeyPost;
CopyTo->PostHtFromObject = CopyFrom->PostHtFromObject;
CopyTo->AltPostHtFromObject = CopyFrom->AltPostHtFromObject;
CopyTo->KeyPostInterval = CopyFrom->KeyPostInterval;
CopyTo->SeparateRoofMat = CopyFrom->SeparateRoofMat;
CopyTo->ConnectToOrigin = CopyFrom->ConnectToOrigin;
CopyTo->SkinFrame = CopyFrom->SkinFrame;
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // Fence::Copy

/*===========================================================================*/

int Fence::SetPost(GeneralEffect *NewPost, int WhichPost)
{
NotifyTag Changes[2];

if (WhichPost == WCS_EFFECTS_FENCE_POSTTYPE_KEY)
	{
	KeyPost = NewPost;
	if (KeyPost == AltPost)
		{
		AltPost = NULL;
		AllOrKeyPosts = WCS_EFFECTS_FENCE_POSTCOUNT_ALL;
		} // if
	if (KeyPost)
		KeyPostType = KeyPost->EffectType;
	} // if
else
	{
	AltPost = NewPost;
	if (KeyPost == AltPost)
		{
		AltPost = NULL;
		AllOrKeyPosts = WCS_EFFECTS_FENCE_POSTCOUNT_ALL;
		} // if
	if (AltPost)
		AltPostType = AltPost->EffectType;
	} // else
Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (1);

} // Fence::SetPost

/*===========================================================================*/

int Fence::RemoveRAHost(RasterAnimHost *RemoveMe)
{
NotifyTag Changes[2];

if (KeyPost == (GeneralEffect *)RemoveMe)
	{
	KeyPost = NULL;

	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

	return (1);
	} // if

if (AltPost == (GeneralEffect *)RemoveMe)
	{
	AltPost = NULL;

	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

	return (1);
	} // if

return (GeneralEffect::RemoveRAHost(RemoveMe));

} // Fence::RemoveRAHost

/*===========================================================================*/

ULONG Fence::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char MatchName[WCS_EFFECT_MAXNAMELENGTH];

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
					case WCS_EFFECTS_ABSOLUTE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Absolute, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_FENCE_POSTSENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&PostsEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_FENCE_SPANSENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&SpansEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						#ifdef WCS_FENCE_LIMITED
						SpansEnabled = 1;
						#endif // WCS_FENCE_LIMITED
						break;
						}
					#ifndef WCS_FENCE_LIMITED
					case WCS_EFFECTS_FENCE_ROOFENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&RoofEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					#endif // WCS_FENCE_LIMITED
					case WCS_EFFECTS_FENCE_ALLORKEYPOSTS:
						{
						BytesRead = ReadBlock(ffile, (char *)&AllOrKeyPosts, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_FENCE_FIRSTKEYPOST:
						{
						BytesRead = ReadBlock(ffile, (char *)&FirstKeyPost, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_FENCE_LASTKEYPOST:
						{
						BytesRead = ReadBlock(ffile, (char *)&LastKeyPost, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_FENCE_POSTHTFROMOBJ:
						{
						BytesRead = ReadBlock(ffile, (char *)&PostHtFromObject, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_FENCE_ALTPOSTHTFROMOBJ:
						{
						BytesRead = ReadBlock(ffile, (char *)&AltPostHtFromObject, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					#ifndef WCS_FENCE_LIMITED
					case WCS_EFFECTS_FENCE_SEPARATEROOFMAT:
						{
						BytesRead = ReadBlock(ffile, (char *)&SeparateRoofMat, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_FENCE_SKINFRAME:
						{
						BytesRead = ReadBlock(ffile, (char *)&SkinFrame, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					#endif // WCS_FENCE_LIMITED
					case WCS_EFFECTS_FENCE_CONNECTTOORIGIN:
						{
						BytesRead = ReadBlock(ffile, (char *)&ConnectToOrigin, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_FENCE_KEYPOSTINTERVAL:
						{
						BytesRead = ReadBlock(ffile, (char *)&KeyPostInterval, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_FENCE_KEYPOSTTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&KeyPostType, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_FENCE_ALTPOSTTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&AltPostType, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					//case WCS_EFFECTS_FENCE_POSTTOPELEV:
					//	{
					//	BytesRead = AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_POSTTOPELEV].Load(ffile, Size, ByteFlip);
					//	break;
					//	}
					//case WCS_EFFECTS_FENCE_POSTBOTELEV:
					//	{
					//	BytesRead = AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_POSTBOTELEV].Load(ffile, Size, ByteFlip);
					//	break;
					//	}
					case WCS_EFFECTS_FENCE_SPANTOPELEV:
						{
						BytesRead = AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANTOPELEV].Load(ffile, Size, ByteFlip);
						#ifdef WCS_FENCE_LIMITED
						if (AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANTOPELEV].CurValue > 5.0)
							AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANTOPELEV].ScaleValues(5.0 / AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANTOPELEV].CurValue);
						else if (AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANTOPELEV].CurValue < -10.0)
							AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANTOPELEV].ScaleValues(-10.0 / AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANTOPELEV].CurValue);
						#endif // WCS_FENCE_LIMITED
						break;
						}
					case WCS_EFFECTS_FENCE_SPANBOTELEV:
						{
						BytesRead = AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANBOTELEV].Load(ffile, Size, ByteFlip);
						#ifdef WCS_FENCE_LIMITED
						if (AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANBOTELEV].CurValue > 5.0)
							AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANBOTELEV].ScaleValues(5.0 / AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANBOTELEV].CurValue);
						else if (AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANBOTELEV].CurValue < -10.0)
							AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANBOTELEV].ScaleValues(-10.0 / AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANBOTELEV].CurValue);
						#endif // WCS_FENCE_LIMITED
						break;
						}
					#ifndef WCS_FENCE_LIMITED
					case WCS_EFFECTS_FENCE_ROOFELEV:
						{
						BytesRead = AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_ROOFELEV].Load(ffile, Size, ByteFlip);
						break;
						}
					#endif // WCS_FENCE_LIMITED
					case WCS_EFFECTS_FENCE_SPANMATDRIVER:
						{
						BytesRead = AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANMATDRIVER].Load(ffile, Size, ByteFlip);
						break;
						}
					#ifndef WCS_FENCE_LIMITED
					case WCS_EFFECTS_FENCE_ROOFMATDRIVER:
						{
						BytesRead = AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_ROOFMATDRIVER].Load(ffile, Size, ByteFlip);
						break;
						}
					#endif // WCS_FENCE_LIMITED
					case WCS_EFFECTS_FENCE_SPANMATERIAL:
						{
						BytesRead = SpanMat.Load(ffile, Size, ByteFlip);
						break;
						}
					#ifndef WCS_FENCE_LIMITED
					case WCS_EFFECTS_FENCE_ROOFMATERIAL:
						{
						BytesRead = RoofMat.Load(ffile, Size, ByteFlip);
						break;
						}
					#endif // WCS_FENCE_LIMITED
					case WCS_EFFECTS_FENCE_KEYPOSTNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							if (KeyPostType == WCS_EFFECTSSUBCLASS_OBJECT3D)
								KeyPost = (GeneralEffect *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_OBJECT3D, MatchName);
							else
								KeyPost = (GeneralEffect *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_FOLIAGE, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_FENCE_ALTPOSTNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							if (AltPostType == WCS_EFFECTSSUBCLASS_OBJECT3D)
								AltPost = (GeneralEffect *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_OBJECT3D, MatchName);
							else
								AltPost = (GeneralEffect *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_FOLIAGE, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_FENCE_TEXSPANMATDRIVER:
						{
						if (TexRoot[WCS_EFFECTS_FENCE_TEXTURE_SPANMATDRIVER] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_FENCE_TEXTURE_SPANMATDRIVER]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#ifndef WCS_FENCE_LIMITED
					case WCS_EFFECTS_FENCE_TEXROOFMATDRIVER:
						{
						if (TexRoot[WCS_EFFECTS_FENCE_TEXTURE_ROOFMATDRIVER] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_FENCE_TEXTURE_ROOFMATDRIVER]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#endif // WCS_FENCE_LIMITED
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
					case WCS_EFFECTS_FENCE_THEMESPANTOPELEV:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_FENCE_THEME_SPANTOPELEV] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_FENCE_THEMESPANBOTELEV:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_FENCE_THEME_SPANBOTELEV] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_FENCE_THEMEROOFELEV:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_FENCE_THEME_ROOFELEV] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_FENCE_THEMESPANMATDRIVER:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_FENCE_THEME_SPANMATDRIVER] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					#ifndef WCS_FENCE_LIMITED
					case WCS_EFFECTS_FENCE_THEMEROOFMATDRIVER:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_FENCE_THEME_ROOFMATDRIVER] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					#endif // WCS_FENCE_LIMITED
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

} // Fence::Load

/*===========================================================================*/

unsigned long int Fence::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_EFFECTS_FENCE_NUMANIMPAR] = {//WCS_EFFECTS_FENCE_POSTTOPELEV,
																	//WCS_EFFECTS_FENCE_POSTBOTELEV,
																	WCS_EFFECTS_FENCE_SPANTOPELEV,
																	WCS_EFFECTS_FENCE_SPANBOTELEV,
																	WCS_EFFECTS_FENCE_ROOFELEV,
																	WCS_EFFECTS_FENCE_SPANMATDRIVER,
																	WCS_EFFECTS_FENCE_ROOFMATDRIVER};
unsigned long int TextureItemTag[WCS_EFFECTS_FENCE_NUMTEXTURES] = {WCS_EFFECTS_FENCE_TEXSPANMATDRIVER,
																	WCS_EFFECTS_FENCE_TEXROOFMATDRIVER};
unsigned long int ThemeItemTag[WCS_EFFECTS_FENCE_NUMTHEMES] = {WCS_EFFECTS_FENCE_THEMESPANTOPELEV,
																	WCS_EFFECTS_FENCE_THEMESPANBOTELEV,
																	WCS_EFFECTS_FENCE_THEMEROOFELEV,
																	WCS_EFFECTS_FENCE_THEMESPANMATDRIVER,
																	WCS_EFFECTS_FENCE_THEMEROOFMATDRIVER};

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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_FENCE_POSTSENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PostsEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_FENCE_SPANSENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&SpansEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_FENCE_ROOFENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RoofEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_FENCE_ALLORKEYPOSTS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AllOrKeyPosts)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_FENCE_FIRSTKEYPOST, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FirstKeyPost)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_FENCE_LASTKEYPOST, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LastKeyPost)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_FENCE_POSTHTFROMOBJ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PostHtFromObject)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_FENCE_ALTPOSTHTFROMOBJ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AltPostHtFromObject)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_FENCE_SEPARATEROOFMAT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&SeparateRoofMat)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_FENCE_SKINFRAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&SkinFrame)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_FENCE_CONNECTTOORIGIN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ConnectToOrigin)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_FENCE_KEYPOSTINTERVAL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&KeyPostInterval)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_FENCE_KEYPOSTTYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&KeyPostType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_FENCE_ALTPOSTTYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&AltPostType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_EFFECTS_FENCE_SPANMATERIAL + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = SpanMat.Save(ffile))
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

ItemTag = WCS_EFFECTS_FENCE_ROOFMATERIAL + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = RoofMat.Save(ffile))
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

if (KeyPost)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_FENCE_KEYPOSTNAME, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(KeyPost->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)KeyPost->GetName())) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

if (AltPost)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_FENCE_ALTPOSTNAME, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(AltPost->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)AltPost->GetName())) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

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

} // Fence::Save

/*===========================================================================*/

void Fence::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->FCG)
	{
	delete GlobalApp->GUIWins->FCG;
	}
GlobalApp->GUIWins->FCG = new FenceEditGUI(GlobalApp->AppEffects, GlobalApp->AppDB, this);
if(GlobalApp->GUIWins->FCG)
	{
	GlobalApp->GUIWins->FCG->Open(GlobalApp->MainProj);
	}

} // Fence::Edit

/*===========================================================================*/

short Fence::AnimateShadows(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetNumNodes(0) > 1)
		return (1);
	} // for
if (SpanMat.Animate3DShadows())
	return (1);
if (SeparateRoofMat && RoofMat.Animate3DShadows())
	return (1);
	
return (0);

} // Fence::AnimateShadows

/*===========================================================================*/

char *FenceCritterNames[WCS_EFFECTS_FENCE_NUMANIMPAR] = {/*"Post Top Elevation (m)", "Post Bottom Elevation (m)",*/
														"Panel Top Elevation (m)", "Panel Bottom Elevation (m)",
														"Roof Elevation (m)", "Panel Material Driver (%)", "Roof Material Driver (%)"};
char *FenceTextureNames[WCS_EFFECTS_FENCE_NUMTEXTURES] = {"Panel Material Driver (%)", "Roof Material Driver (%)"};
char *FenceThemeNames[WCS_EFFECTS_FENCE_NUMTHEMES] = {"Panel Top Elevation (m)", "Panel Bottom Elevation (m)", "Roof Elevation (m)",
														"Panel Material Driver (%)", "Roof Material Driver (%)"};

char *Fence::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (FenceCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (FenceTextureNames[Ct]);
		} // if
	} // for
if (Test == &SpanMat)
	return ("Panel Material");
if (Test == &RoofMat)
	return ("Roof Material");

return ("");

} // Fence::GetCritterName

/*===========================================================================*/

char *Fence::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as a Fence Texture! Remove anyway?");

} // Fence::OKRemoveRaster

/*===========================================================================*/

int Fence::FindnRemove3DObjects(Object3DEffect *RemoveMe)
{
int Found = 0;

Found = SpanMat.FindnRemove3DObjects(RemoveMe);
return (RoofMat.FindnRemove3DObjects(RemoveMe) || Found);

} // Fence::FindnRemove3DObjects

/*===========================================================================*/

int Fence::SetToTime(double Time)
{
long Found = 0;

if (GeneralEffect::SetToTime(Time))
	Found = 1;
if (SpanMat.SetToTime(Time))
	Found = 1;
if (RoofMat.SetToTime(Time))
	Found = 1;

return (Found);

} // Fence::SetToTime

/*===========================================================================*/

int Fence::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
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
if (! SpanMat.InitToRender(Opt, Buffers))
	return (0);
if (! RoofMat.InitToRender(Opt, Buffers))
	return (0);

return (1);

} // Fence::InitToRender

/*===========================================================================*/

int Fence::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{

if (! SpanMat.InitFrameToRender(Lib, Rend))
	return (0);
if (! RoofMat.InitFrameToRender(Lib, Rend))
	return (0);

return (GeneralEffect::InitFrameToRender(Lib, Rend));

} // Fence::InitFrameToRender

/*===========================================================================*/

long Fence::InitImageIDs(long &ImageID)
{
long NumImages = 0;

NumImages += SpanMat.InitImageIDs(ImageID);
NumImages += RoofMat.InitImageIDs(ImageID);
if (KeyPost)
	NumImages += KeyPost->InitImageIDs(ImageID);
if (AltPost)
	NumImages += AltPost->InitImageIDs(ImageID);
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // Fence::InitImageIDs

/*===========================================================================*/

int Fence::BuildFileComponentsList(EffectList **Foliages, EffectList **Material3Ds, EffectList **Object3Ds, 
	EffectList **Waves, EffectList **Queries, EffectList **Themes, EffectList **Coords)
{
EffectList **ListPtr;

if (KeyPost)
	{
	if (KeyPost->EffectType == WCS_EFFECTSSUBCLASS_OBJECT3D)
		{
		ListPtr = Object3Ds;
		while (*ListPtr)
			{
			if ((*ListPtr)->Me == KeyPost)
				break;
			ListPtr = &(*ListPtr)->Next;
			} // if
		if (! (*ListPtr))
			{
			if (*ListPtr = new EffectList())
				(*ListPtr)->Me = KeyPost;
			else
				return (0);
			} // if
		if (! ((Object3DEffect *)KeyPost)->BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
			return (0);
		} // if
	else
		{
		ListPtr = Foliages;
		while (*ListPtr)
			{
			if ((*ListPtr)->Me == KeyPost)
				break;
			ListPtr = &(*ListPtr)->Next;
			} // if
		if (! (*ListPtr))
			{
			if (*ListPtr = new EffectList())
				(*ListPtr)->Me = KeyPost;
			else
				return (0);
			} // if
		if (! ((FoliageEffect *)KeyPost)->BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
			return (0);
		} // else
	} // if
if (AltPost)
	{
	if (AltPost->EffectType == WCS_EFFECTSSUBCLASS_OBJECT3D)
		{
		ListPtr = Object3Ds;
		while (*ListPtr)
			{
			if ((*ListPtr)->Me == AltPost)
				break;
			ListPtr = &(*ListPtr)->Next;
			} // if
		if (! (*ListPtr))
			{
			if (*ListPtr = new EffectList())
				(*ListPtr)->Me = AltPost;
			else
				return (0);
			} // if
		if (! ((Object3DEffect *)AltPost)->BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
			return (0);
		} // if
	else
		{
		ListPtr = Foliages;
		while (*ListPtr)
			{
			if ((*ListPtr)->Me == AltPost)
				break;
			ListPtr = &(*ListPtr)->Next;
			} // if
		if (! (*ListPtr))
			{
			if (*ListPtr = new EffectList())
				(*ListPtr)->Me = AltPost;
			else
				return (0);
			} // if
		if (! ((FoliageEffect *)AltPost)->BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
			return (0);
		} // else
	} // if

return (1);

} // Fence::BuildFileComponentsList

/*===========================================================================*/

long Fence::GetKeyFrameRange(double &FirstKey, double &LastKey)
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
if (SpanMat.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (RoofMat.GetKeyFrameRange(MinDist, MaxDist))
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

} // Fence::GetKeyFrameRange

/*===========================================================================*/

char Fence::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMMATERIALGRADIENT
	|| DropType == WCS_EFFECTSSUBCLASS_MATERIAL
	|| DropType == WCS_RAHOST_OBJTYPE_VECTOR
	#ifdef WCS_BUILD_VNS
	|| DropType == WCS_EFFECTSSUBCLASS_SEARCHQUERY
	#endif // WCS_BUILD_VNS
	//|| DropType == WCS_EFFECTSSUBCLASS_FOLIAGE
	//|| DropType == WCS_EFFECTSSUBCLASS_OBJECT3D
	)
	return (1);

return (0);

} // Fence::GetRAHostDropOK

/*===========================================================================*/

int Fence::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
#ifndef WCS_FENCE_LIMITED
int QueryResult;
#endif // WCS_FENCE_LIMITED
int Success;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (Fence *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (Fence *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMMATERIALGRADIENT 
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_MATERIAL)
	{
	#ifndef WCS_FENCE_LIMITED
	Success = -1;
	sprintf(QueryStr, "Apply %s %s to %s?", DropSource->Name, DropSource->Type, "which");
	if (QueryResult = UserMessageCustom(NameStr, QueryStr, "Span Material", "Cancel", "Roof Material", 0))
		{
		if (QueryResult == 1)
			{
			Success = SpanMat.ProcessRAHostDragDrop(DropSource);
			} // if
		else if (QueryResult == 2)
			{
			Success = RoofMat.ProcessRAHostDragDrop(DropSource);
			} // else if
		} // if
	#else // WCS_FENCE_LIMITED
	Success = SpanMat.ProcessRAHostDragDrop(DropSource);
	#endif // WCS_FENCE_LIMITED
	} // else if
/*
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_FOLIAGE || DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_OBJECT3D)
	{
	Success = -1;
	sprintf(QueryStr, "Apply %s %s to %s?", DropSource->Name, DropSource->Type, "which");
	if (QueryResult = UserMessageCustom(NameStr, QueryStr, "Key Posts", "Cancel", "Alternate Posts", 0))
		{
		if (QueryResult == 1)
			{
			Success = SetPost((GeneralEffect *)DropSource->DropSource, WCS_EFFECTS_FENCE_POSTTYPE_KEY);
			} // if
		else if (QueryResult == 2)
			{
			Success = SetPost((GeneralEffect *)DropSource->DropSource, WCS_EFFECTS_FENCE_POSTTYPE_ALT);
			} // else if
		} // if
	} // else if
*/
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

} // Fence::ProcessRAHostDragDrop

/*===========================================================================*/

char *Fence::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? (char*)FenceTextureNames[TexNumber]: (char*)"");

} // Fence::GetTextureName

/*===========================================================================*/

RootTexture *Fence::NewRootTexture(long TexNumber)
{
char ApplyToColor = 0;
char ApplyToDisplace = 0;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // Fence::NewRootTexture

/*===========================================================================*/

char *Fence::GetThemeName(long ThemeNum)
{

return (ThemeNum < GetNumThemes() ? (char*)FenceThemeNames[ThemeNum]: (char*)"");

} // Fence::GetThemeName

/*===========================================================================*/

int Fence::GetRAHostAnimated(void)
{

if (GeneralEffect::GetRAHostAnimated())
	return (1);
if (SpanMat.GetRAHostAnimated())
	return (1);
if (RoofMat.GetRAHostAnimated())
	return (1);

return (0);

} // Fence::GetRAHostAnimated

/*===========================================================================*/

unsigned long Fence::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_FENCE | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // Fence::GetRAFlags

/*===========================================================================*/

RasterAnimHost *Fence::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;
JoeList *CurJoe = Joes;

if (! Current)
	return (&SpanMat);
if (Current == &SpanMat)
#ifndef WCS_FENCE_LIMITED
	return (&RoofMat);
if (Current == &RoofMat)
#endif // WCS_FENCE_LIMITED
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	#ifndef WCS_FENCE_LIMITED
	if (Found)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	#else // WCS_FENCE_LIMITED
	if (Found && Ct != WCS_EFFECTS_FENCE_ANIMPAR_ROOFELEV && Ct != WCS_EFFECTS_FENCE_ANIMPAR_ROOFMATDRIVER)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	#endif // WCS_FENCE_LIMITED
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	#ifndef WCS_FENCE_LIMITED
	if (Found && GetTexRootPtr(Ct))
		return (GetTexRootPtr(Ct));
	if (Current == GetTexRootPtr(Ct))
		Found = 1;
	#else // WCS_FENCE_LIMITED
	if (Found && Ct != WCS_EFFECTS_FENCE_TEXTURE_ROOFMATDRIVER && GetTexRootPtr(Ct))
		return (GetTexRootPtr(Ct));
	if (Current == GetTexRootPtr(Ct))
		Found = 1;
	#endif // WCS_FENCE_LIMITED
	} // for
for (Ct = 0; Ct < GetNumThemes(); Ct ++)
	{
	#ifndef WCS_FENCE_LIMITED
	if (Found && GetTheme(Ct) && GetThemeUnique(Ct) && GetThemeUnique(Ct))
		return (GetTheme(Ct));
	if (Current == GetTheme(Ct) && GetThemeUnique(Ct) && GetThemeUnique(Ct))
		Found = 1;
	#endif // WCS_FENCE_LIMITED
	} // for
if (Found && KeyPost)
	return (KeyPost);
if (KeyPost && Current == KeyPost)
	Found = 1;
if (Found && AltPost)
	return (AltPost);
if (AltPost && Current == AltPost)
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

} // Fence::GetRAHostChild

/*===========================================================================*/

int Fence::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
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
				case WCS_EFFECTS_FENCE_ANIMPAR_SPANTOPELEV:
					{
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_FENCE_THEME_SPANTOPELEV);
					break;
					} // 
				case WCS_EFFECTS_FENCE_ANIMPAR_SPANBOTELEV:
					{
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_FENCE_THEME_SPANBOTELEV);
					break;
					} // 
				case WCS_EFFECTS_FENCE_ANIMPAR_ROOFELEV:
					{
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_FENCE_THEME_ROOFELEV);
					break;
					} // 
				case WCS_EFFECTS_FENCE_ANIMPAR_SPANMATDRIVER:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_FENCE_TEXTURE_SPANMATDRIVER);
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_FENCE_THEME_SPANMATDRIVER);
					break;
					} // 
				case WCS_EFFECTS_FENCE_ANIMPAR_ROOFMATDRIVER:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_FENCE_TEXTURE_ROOFMATDRIVER);
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_FENCE_THEME_ROOFMATDRIVER);
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
				case WCS_EFFECTS_FENCE_TEXTURE_SPANMATDRIVER:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_FENCE_ANIMPAR_SPANMATDRIVER);
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_FENCE_THEME_SPANMATDRIVER);
					break;
					} // 
				case WCS_EFFECTS_FENCE_TEXTURE_ROOFMATDRIVER:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_FENCE_ANIMPAR_ROOFMATDRIVER);
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_FENCE_THEME_ROOFMATDRIVER);
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
				case WCS_EFFECTS_FENCE_THEME_SPANTOPELEV:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_FENCE_ANIMPAR_SPANTOPELEV);
					break;
					} // 
				case WCS_EFFECTS_FENCE_THEME_SPANBOTELEV:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_FENCE_ANIMPAR_SPANBOTELEV);
					break;
					} // 
				case WCS_EFFECTS_FENCE_THEME_ROOFELEV:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_FENCE_ANIMPAR_ROOFELEV);
					break;
					} // 
				case WCS_EFFECTS_FENCE_THEME_SPANMATDRIVER:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_FENCE_ANIMPAR_SPANMATDRIVER);
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_FENCE_TEXTURE_SPANMATDRIVER);
					break;
					} // 
				case WCS_EFFECTS_FENCE_THEME_ROOFMATDRIVER:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_FENCE_ANIMPAR_ROOFMATDRIVER);
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_FENCE_TEXTURE_ROOFMATDRIVER);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	} // else if

return (0);

} // Fence::GetAffiliates

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int Fence::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
Fence *CurrentFence = NULL;
Object3DEffect *CurrentObj = NULL;
MaterialEffect *CurrentMaterial = NULL;
WaveEffect *CurrentWave = NULL;
FoliageEffect *CurrentFol = NULL;
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
						} // if Object3D
					else if (! strnicmp(ReadBuf, "Foliage", 8))
						{
						if (CurrentFol = new FoliageEffect(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentFol->Load(ffile, Size, ByteFlip);
							}
						} // if Foliage
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
					else if (! strnicmp(ReadBuf, "Fence", 8))
						{
						if (CurrentFence = new Fence(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentFence->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if fence
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

if (Success == 1 && CurrentFence)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentFence);
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

} // Fence::LoadObject

/*===========================================================================*/

int Fence::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
EffectList *CurEffect, *Foliages = NULL, *Material3Ds = NULL, *Object3Ds = NULL, *Waves = NULL, *Queries = NULL, *Themes = NULL, *Coords = NULL;
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
	&& BuildFileComponentsList(&Foliages, &Material3Ds, &Object3Ds, &Waves, &Queries, &Themes, &Coords))
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

	CurEffect = Foliages;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "Foliage");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((FoliageEffect *)CurEffect->Me)->Save(ffile))
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
						} // if Foliage Effect saved 
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
	while (Foliages)
		{
		CurEffect = Foliages;
		Foliages = Foliages->Next;
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

// Fence
strcpy(StrBuf, "Fence");
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
			} // if Fence saved 
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

} // Fence::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::Fence_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
Fence *Current;

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
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new Fence(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (Fence *)FindDuplicateByName(Current->EffectType, Current))
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

return (TotalRead);

} // EffectsLib::Fence_Load()

/*===========================================================================*/

ULONG EffectsLib::Fence_Save(FILE *ffile)
{
Fence *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

Current = Fences;
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
					} // if fence saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (Fence *)Current->Next;
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

} // EffectsLib::Fence_Save()

/*===========================================================================*/
