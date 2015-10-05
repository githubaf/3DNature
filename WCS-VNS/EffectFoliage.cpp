// EffectFoliage.cpp
// For managing Foliage Effects
// Built from scratch on 04/30/97 by Gary R. Huber
// Copyright 1997 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "AppMem.h"
#include "Conservatory.h"
#include "FoliageEffectEditGUI.h"
#include "FoliageEffectFolFileEditGUI.h"
#include "Project.h"
#include "Types.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Log.h"
#include "Raster.h"
#include "Points.h"
#include "MathSupport.h"
#include "Render.h"
#include "requester.h"
#include "Database.h"
#include "Security.h"
#include "DEM.h"
#include "Lists.h"
#include "FeatureConfig.h"

#ifndef min
#define   min(a,b)    ((a) <= (b) ? (a) : (b))
#endif

FoliageEffectBase::FoliageEffectBase(void)
{

SetDefaults();

} // FoliageEffectBase::FoliageEffectBase

/*===========================================================================*/

void FoliageEffectBase::SetDefaults(void)
{

VerticesToRender = 0;
VertList = NULL;

} // FoliageEffectBase::SetDefaults

/*===========================================================================*/

int FoliageEffectBase::Init(FoliageEffect *EffectList, RenderJoeList *JL)
{
long Ct;
#ifdef WCS_BUILD_SX2
long CellNum;
#endif // WCS_BUILD_SX2
RenderJoeList *CurJL;
VectorPoint *CurPt;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM MyVert;
#ifdef WCS_BUILD_SX2
FoliageEffect *CurEffect;
FoliagePreviewData FPD;
RealtimeFoliageData *RTFD;
#endif // WCS_BUILD_SX2

if (VertList)
	delete [] VertList;
VertList = NULL;
VerticesToRender = 0;
CurJL = JL;
while (CurJL)
	{
	if (CurJL->Me && CurJL->Me->GetNumRealPoints() > 0)
		VerticesToRender += (CurJL->Me->GetNumRealPoints());
	CurJL = (RenderJoeList *)CurJL->Next;
	} // while
#ifdef WCS_BUILD_SX2
// might be some foliage effects that render from a foliage file
for (CurEffect = EffectList; CurEffect; CurEffect = (FoliageEffect *)CurEffect->Next)
	{
	if (CurEffect->UseFoliageFile && CurEffect->Enabled)
		{
		if (CurEffect->FoliageFileIndex.LoadFoliageIndex((char *)CurEffect->FoliageFile.GetName(), (char *)CurEffect->FoliageFile.GetPath()))
			{
			// add number of foliage file vertices to the count to render
			if (CurEffect->FoliageFileIndex.CellDat)
				{
				if (CurEffect->FoliageFileIndex.LoadAllFoliage())
					for (CellNum = 0; CellNum < CurEffect->FoliageFileIndex.NumCells; CellNum ++)
						VerticesToRender += CurEffect->FoliageFileIndex.CellDat[CellNum].DatCt;
				} // if some cells to read
			} // if
		} // if
	} // for
#endif // WCS_BUILD_SX2
if (VerticesToRender > 0)
	{
	if (VertList = new FoliageVertexList[VerticesToRender])
		{
		CurJL = JL;
		Ct = 0;
		while (CurJL)
			{
			if (CurJL->Me && CurJL->Me->GetNumRealPoints() > 0)
				{
				if (MyAttr = (JoeCoordSys *)CurJL->Me->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
					MyCoords = MyAttr->Coord;
				else
					MyCoords = NULL;
				for (CurPt = CurJL->Me->GetFirstRealPoint(); CurPt; CurPt = CurPt->Next)
					{
					if (CurPt->ProjToDefDeg(MyCoords, &MyVert))
						{
						VertList[Ct].Fol = (FoliageEffect *)CurJL->Effect;
						VertList[Ct].Vec = CurJL->Me;
						VertList[Ct].Lat = MyVert.Lat;
						VertList[Ct].Lon = MyVert.Lon;
						VertList[Ct ++].Point = CurPt;
						} // if
					else
						VerticesToRender --;
					} // for
				} // if
			CurJL = (RenderJoeList *)CurJL->Next;
			} // while
		#ifdef WCS_BUILD_SX2
		// might be some foliage effects that render from a foliage file
		for (CurEffect = EffectList; CurEffect; CurEffect = (FoliageEffect *)CurEffect->Next)
			{
			if (CurEffect->UseFoliageFile && CurEffect->Enabled && CurEffect->FoliageFileIndex.CellDat)
				{
				PlanetOpt *DefPlanetOpt;
				if (DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL))
					MyCoords = DefPlanetOpt->Coords;
				else
					MyCoords = NULL;

				for (RTFD = CurEffect->FoliageFileIndex.FirstWalk(); RTFD; RTFD = CurEffect->FoliageFileIndex.NextWalk())
					{
					if (CurEffect->FoliageFileIndex.RasterInterp && RTFD->ElementID > 0)
						{
						RTFD->ElementID = (short)CurEffect->FoliageFileIndex.RasterInterp[RTFD->ElementID];
						} // if
					MyVert.xyz[1] = MyVert.Lat = CurEffect->FoliageFileIndex.RefXYZ[1] + RTFD->XYZ[1];
					MyVert.xyz[0] = MyVert.Lon = CurEffect->FoliageFileIndex.RefXYZ[0] + RTFD->XYZ[0];
					MyVert.xyz[2] = MyVert.Elev = CurEffect->FoliageFileIndex.RefXYZ[2] + RTFD->XYZ[2];
					if (MyCoords)
						MyCoords->ProjToDefDeg(&MyVert);
					VertList[Ct].Fol = CurEffect;
					VertList[Ct].Vec = NULL;
					VertList[Ct].Lat = MyVert.Lat;
					VertList[Ct].Lon = MyVert.Lon;
					VertList[Ct ++].Point = RTFD;
					} // if
				} // if
			} // for
		#endif // WCS_BUILD_SX2
		return (1);
		} // if
	} // if

return (0);

} // FoliageEffectBase::Init

/*===========================================================================*/

void FoliageEffectBase::Destroy(void)
{

if (VertList)
	delete [] VertList;
VertList = NULL;
VerticesToRender = 0;

} // FoliageEffectBase::Destroy

/*===========================================================================*/

void FoliageEffectBase::FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt, DEM *CurDEM)
{
long VertCt;
VertexData ObjVert;
PolygonData Poly;

if (! CurDEM)
	return;

for (VertCt = 0; VertCt < VerticesToRender; VertCt ++)
	{
	if (! VertList[VertCt].Rendered)
		{ 
		if (CurDEM->GeographicPointContained(Rend->DefCoords, VertList[VertCt].Lat, VertList[VertCt].Lon, TRUE))
			{
			VertList[VertCt].Fol->FindBasePosition(Rend, &ObjVert, &Poly, VertList[VertCt].Vec, VertList[VertCt].Point);
			Rend->Cam->ProjectVertexDEM(Rend->DefCoords, &ObjVert, Rend->EarthLatScaleMeters, Rend->PlanetRad, 1);
			PolyArray[PolyCt].PolyType = WCS_POLYSORTTYPE_FOLIAGE;
			PolyArray[PolyCt].PolyNumber = (unsigned long)&VertList[VertCt];
			PolyArray[PolyCt ++].PolyQ = (float)(ObjVert.ScrnXYZ[2] - Rend->ShadowMapDistanceOffset);
			VertList[VertCt].Rendered = 1;
			} // if
		} // if
	} // for

} // FoliageEffectBase::FillRenderPolyArray

/*===========================================================================*/

void FoliageEffectBase::FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt, 
	CoordSys *MyCoords, GeoRegister *MyBounds)
{
long VertCt;
VertexData ObjVert;
PolygonData Poly;

if (! MyBounds)
	return;

for (VertCt = 0; VertCt < VerticesToRender; VertCt ++)
	{
	if (! VertList[VertCt].Rendered)
		{ 
		if (MyBounds->GeographicPointContained(MyCoords, Rend->DefCoords, VertList[VertCt].Lat, VertList[VertCt].Lon))
			{
			VertList[VertCt].Fol->FindBasePosition(Rend, &ObjVert, &Poly, VertList[VertCt].Vec, VertList[VertCt].Point);
			#ifdef WCS_BUILD_VNS
			Rend->DefCoords->DegToCart(&ObjVert);
			#else // WCS_BUILD_VNS
			ObjVert.DegToCart(Rend->PlanetRad);
			#endif // WCS_BUILD_VNS
			PolyArray[PolyCt].PolyType = WCS_POLYSORTTYPE_FOLIAGE;
			PolyArray[PolyCt].PolyNumber = (unsigned long)&VertList[VertCt];
			PolyArray[PolyCt ++].PolyQ = 1.0f;
			VertList[VertCt].Rendered = 1;
			} // if
		} // if
	} // for

} // FoliageEffectBase::FillRenderPolyArray

/*===========================================================================*/

int FoliageEffectBase::InitFrameToRender(void)
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

} // FoliageEffectBase::InitFrameToRender

/*===========================================================================*/
/*
short FoliageEffectBase::EvalFourPoints(VertexDEM *Vert[3], long &StartRastNum, VectorPoint *&StartPoint)
{
GeoRaster *TestRast = NULL;
double HiLat = -1000000.0, LoLat = 1000000.0, HiLon = -1000000.0, LoLon = 1000000.0;
short HiLatPt, LoLatPt, HiLonPt, LoLonPt, PtCt;
long RastNum = 0;
VectorPoint *TestPoint;

for (PtCt = 0; PtCt < 3; PtCt ++)
	{
	if (Vert[PtCt]->Lat > HiLat)
		{
		HiLat = Vert[PtCt]->Lat;
		HiLatPt = PtCt;
		} // if
	if (Vert[PtCt]->Lat < LoLat)
		{
		LoLat = Vert[PtCt]->Lat;
		LoLatPt = PtCt;
		} // if
	if (Vert[PtCt]->Lon > HiLon)
		{
		HiLon = Vert[PtCt]->Lon;
		HiLonPt = PtCt;
		} // if
	if (Vert[PtCt]->Lon < LoLon)
		{
		LoLon = Vert[PtCt]->Lon;
		LoLonPt = PtCt;
		} // if
	} // for

if (LoLat <= GeoRast->N && HiLat >= GeoRast->S)
	{
	if (LoLon <= GeoRast->W && HiLon >= GeoRast->E)
		{
		TestRast = (GeoRaster *)GeoRast->Next;
		} // if
	} // if
while (TestRast)
	{
	RastNum ++;
	if (LoLat <= TestRast->N && HiLat >= TestRast->S)
		{
		if (LoLon <= TestRast->W && HiLon >= TestRast->E)
			{
			for (TestPoint = TestRast->JLUT[0]->Points->Next; TestPoint; TestPoint = TestPoint->Next)
				{
				if (TestPoint->Latitude <= HiLat && TestPoint->Latitude >= LoLat && 
					TestPoint->Longitude <= HiLon && TestPoint->Longitude >= LoLon)
					{
					if (PointEnclosedPoly3(Vert, TestPoint->Latitude, TestPoint->Longitude))
						{
						StartRastNum = RastNum;
						StartPoint = TestPoint;
						return (1);
						} // if
					} // if
				} // if
			} // if
		} // if
	TestRast = (GeoRaster *)TestRast->Next;
	} // while

return (0);

} // FoliageEffectBase::EvalFourPoints
*/
/*===========================================================================*/
/*
long FoliageEffectBase::EvalThreePoints(RenderData *Rend, VertexDEM *Vert[3], long PointsAlreadyRendered, long StartRastNum, VectorPoint *StartPoint,
	PolygonData *Poly, Ecotype *&Eco)
{
GeoRaster *TestRast;
double HiLat = -1000000.0, LoLat = 1000000.0, HiLon = -1000000.0, LoLon = 1000000.0;
short HiLatPt, LoLatPt, HiLonPt, LoLonPt, PtCt;
long RastNum = 0, PointsFound = 0;
VectorPoint *TestPoint;
FoliageEffect *Effect;

for (PtCt = 0; PtCt < 3; PtCt ++)
	{
	if (Vert[PtCt]->Lat > HiLat)
		{
		HiLat = Vert[PtCt]->Lat;
		HiLatPt = PtCt;
		} // if
	if (Vert[PtCt]->Lat < LoLat)
		{
		LoLat = Vert[PtCt]->Lat;
		LoLatPt = PtCt;
		} // if
	if (Vert[PtCt]->Lon > HiLon)
		{
		HiLon = Vert[PtCt]->Lon;
		HiLonPt = PtCt;
		} // if
	if (Vert[PtCt]->Lon < LoLon)
		{
		LoLon = Vert[PtCt]->Lon;
		LoLonPt = PtCt;
		} // if
	} // for

TestRast = (GeoRaster *)GeoRast->Next;
for (RastNum = 1; RastNum < StartRastNum; RastNum ++)
	TestRast = (GeoRaster *)TestRast->Next;

while (TestRast)
	{
	if (LoLat <= TestRast->N && HiLat >= TestRast->S)
		{
		if (LoLon <= TestRast->W && HiLon >= TestRast->E)
			{
			for (TestPoint = RastNum == StartRastNum ? StartPoint: TestRast->JLUT[0]->Points->Next; TestPoint; TestPoint = TestPoint->Next)
				{
				if (TestPoint->Latitude <= HiLat && TestPoint->Latitude >= LoLat && 
					TestPoint->Longitude <= HiLon && TestPoint->Longitude >= LoLon)
					{
					if (PointEnclosedPoly3(Vert, TestPoint->Latitude, TestPoint->Longitude))
						{
						// bingo!
						PointsFound ++;
						if (PointsFound > PointsAlreadyRendered)
							{
							Effect = (FoliageEffect *)TestRast->LUT[0];
							Poly->Lat = TestPoint->Latitude;
							Poly->Lon = TestPoint->Longitude;
							Poly->Object = Effect;
							Poly->Vector = TestRast->JLUT[0];
							Eco = &Effect->Ecotp;
							if (Effect->Absolute == WCS_EFFECT_RELATIVETOJOE)
								{
								Poly->Elev = Effect->Elev + 
									(TestPoint->Elevation - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
								} // if
							else if (Effect->Absolute == WCS_EFFECT_ABSOLUTE)
								{
								Poly->Elev = Effect->Elev;
								} // else if
							else
								{
								Poly->Elev += Effect->Elev;
								} // else
							return (PointsFound);
							} // if
						} // if
					} // if
				} // if
			} // if
		} // if
	TestRast = (GeoRaster *)TestRast->Next;
	RastNum ++;
	} // while

return (-1);

} // FoliageEffectBase::EvalThreePoints
*/
/*===========================================================================*/
/*
void FoliageEffectBase::InitCompare(struct Ecosystem *&Eco)
{

} // FoliageEffectBase::InitCompare
*/
/*===========================================================================*/
/*
short FoliageEffectBase::Compare(FoliageEffect *Effect, short ValuesRead)
{

return (0);

} // FoliageEffectBase::Compare
*/
/*===========================================================================*/
/*===========================================================================*/

FoliageEffect::FoliageEffect()
: GeneralEffect(NULL), Ecotp(this)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_FOLIAGE;
SetDefaults();

} // FoliageEffect::FoliageEffect

/*===========================================================================*/

FoliageEffect::FoliageEffect(RasterAnimHost *RAHost)
: GeneralEffect(RAHost), Ecotp(this)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_FOLIAGE;
SetDefaults();

} // FoliageEffect::FoliageEffect

/*===========================================================================*/

FoliageEffect::FoliageEffect(RasterAnimHost *RAHost, EffectsLib *Library, FoliageEffect *Proto)
: GeneralEffect(RAHost), Ecotp(this)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_FOLIAGE;
if (Library)
	{
	Prev = Library->LastFoliage;
	if (Library->LastFoliage)
		{
		Library->LastFoliage->Next = this;
		Library->LastFoliage = this;
		} // if
	else
		{
		Library->Foliage = Library->LastFoliage = this;
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
	strcpy(NameBase, "Foliage");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // FoliageEffect::FoliageEffect

/*===========================================================================*/

FoliageEffect::~FoliageEffect(void)
{

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->FLG && GlobalApp->GUIWins->FLG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->FLG;
		GlobalApp->GUIWins->FLG = NULL;
		} // if
	if (GlobalApp->GUIWins->FFG && GlobalApp->GUIWins->FFG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->FFG;
		GlobalApp->GUIWins->FFG = NULL;
		} // if
	} // if

} // FoliageEffect::~FoliageEffect

/*===========================================================================*/

void FoliageEffect::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_FOLIAGE_NUMANIMPAR] = {0.0};
double RangeDefaults[WCS_EFFECTS_FOLIAGE_NUMANIMPAR][3] = {FLT_MAX, -FLT_MAX, 1.0};
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
Elev = 0.0;
PreviewEnabled = 1;
ClickQueryEnabled = 0;
UseFoliageFile = 0;

AnimPar[WCS_EFFECTS_FOLIAGE_ANIMPAR_ELEV].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);

} // FoliageEffect::SetDefaults

/*===========================================================================*/

void FoliageEffect::Copy(FoliageEffect *CopyTo, FoliageEffect *CopyFrom)
{

CopyTo->PreviewEnabled = CopyFrom->PreviewEnabled;
CopyTo->UseFoliageFile = CopyFrom->UseFoliageFile;
Ecotp.Copy(&CopyTo->Ecotp, &CopyFrom->Ecotp);
FoliageFile.Copy(&CopyTo->FoliageFile, &CopyFrom->FoliageFile);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // FoliageEffect::Copy

/*===========================================================================*/

unsigned long int FoliageEffect::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
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
					case WCS_EFFECTS_FOLIAGE_PREVIEWENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&PreviewEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					#ifdef WCS_BUILD_SX2
					case WCS_EFFECTS_FOLIAGE_USEFOLIAGEFILE:
						{
						BytesRead = ReadBlock(ffile, (char *)&UseFoliageFile, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_FOLIAGE_FOLIAGEFILE:
						{
						BytesRead = FoliageFile.Load(ffile, Size, ByteFlip);
						break;
						}
					#endif // WCS_BUILD_SX2
					case WCS_EFFECTS_FOLIAGE_ECOTYPE:
						{
						BytesRead = Ecotp.Load(ffile, Size, ByteFlip);
						// Fix problem of legacy Foliage Effect ecotypes being disabled if copied from a disabled 
						// ecosystem ecotype. Not adjustable from interface in Foliage Effects so it has to be fixed here.
						Ecotp.Enabled = 1;
						break;
						}
					case WCS_EFFECTS_FOLIAGE_RELATIVEELEV:
						{
						BytesRead = AnimPar[WCS_EFFECTS_FOLIAGE_ANIMPAR_ELEV].Load(ffile, Size, ByteFlip);
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

} // FoliageEffect::Load

/*===========================================================================*/

unsigned long int FoliageEffect::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_EFFECTS_FOLIAGE_NUMANIMPAR] = {WCS_EFFECTS_FOLIAGE_RELATIVEELEV};

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
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_FOLIAGE_PREVIEWENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PreviewEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
#ifdef WCS_BUILD_SX2
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_FOLIAGE_USEFOLIAGEFILE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&UseFoliageFile)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
#endif // WCS_BUILD_SX2

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

ItemTag = WCS_EFFECTS_FOLIAGE_ECOTYPE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Ecotp.Save(ffile))
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

#ifdef WCS_BUILD_SX2
if (UseFoliageFile)
	{
	ItemTag = WCS_EFFECTS_FOLIAGE_FOLIAGEFILE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = FoliageFile.Save(ffile))
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
				} // if foliage file saved
			else
				goto WriteError;
			} // if size written
		else
			goto WriteError;
		} // if tag written
	else
		goto WriteError;
	} // if
#endif // WCS_BUILD_SX2

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

} // FoliageEffect::Save

/*===========================================================================*/

void FoliageEffect::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->FLG)
	{
	delete GlobalApp->GUIWins->FLG;
	GlobalApp->GUIWins->FLG = NULL;
	}
if(GlobalApp->GUIWins->FFG)
	{
	delete GlobalApp->GUIWins->FFG;
	GlobalApp->GUIWins->FFG = NULL;
	}
if (! UseFoliageFile)
	{
	GlobalApp->GUIWins->FLG = new FoliageEffectEditGUI(GlobalApp->AppEffects, GlobalApp->AppDB, GlobalApp->AppImages, this);
	if(GlobalApp->GUIWins->FLG)
		{
		GlobalApp->GUIWins->FLG->Open(GlobalApp->MainProj);
		}
	} // if
else
	{
	GlobalApp->GUIWins->FFG = new FoliageEffectFolFileEditGUI(GlobalApp->AppEffects, this);
	if(GlobalApp->GUIWins->FFG)
		{
		GlobalApp->GUIWins->FFG->Open(GlobalApp->MainProj);
		}
	} // else

} // FoliageEffect::Edit

/*===========================================================================*/

short FoliageEffect::AnimateShadows(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetNumNodes(0) > 1)
		return (1);
	} // for
if (Ecotp.AnimateShadows())
	return (1);

return (0);

} // FoliageEffect::AnimateShadows

/*===========================================================================*/

char *FoliageEffectCritterNames[WCS_EFFECTS_FOLIAGE_NUMANIMPAR] = {"Elevation (m)"};

char *FoliageEffect::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (FoliageEffectCritterNames[Ct]);
	} // for

return ("");

} // FoliageEffect::GetCritterName

/*===========================================================================*/

int FoliageEffect::GetRAHostAnimated(void)
{
int rVal = 0;

if (GeneralEffect::GetRAHostAnimated())
	rVal = 1;
else if (Ecotp.GetRAHostAnimated())
	rVal = 1;

return (rVal);

} // FoliageEffect::GetRAHostAnimated

/*===========================================================================*/

int FoliageEffect::FindnRemove3DObjects(Object3DEffect *RemoveMe)
{

return (Ecotp.FindnRemove3DObjects(RemoveMe));

} // FoliageEffect::FindnRemove3DObjects

/*===========================================================================*/

int FoliageEffect::SetToTime(double Time)
{
long Found = 0;

if (GeneralEffect::SetToTime(Time))
	Found = 1;
if (Ecotp.SetToTime(Time))
	Found = 1;

return (Found);

} // FoliageEffect::SetToTime

/*===========================================================================*/

long FoliageEffect::GetKeyFrameRange(double &FirstKey, double &LastKey)
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
if (Ecotp.GetKeyFrameRange(MinDist, MaxDist))
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

} // FoliageEffect::GetKeyFrameRange

/*===========================================================================*/

char FoliageEffect::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ECOTYPE
	|| DropType == WCS_RAHOST_OBJTYPE_FOLIAGEGROUP
	|| DropType == WCS_RAHOST_OBJTYPE_FOLIAGE
	|| DropType == WCS_RAHOST_OBJTYPE_RASTER
	#ifdef WCS_BUILD_VNS
	|| DropType == WCS_EFFECTSSUBCLASS_SEARCHQUERY
	#endif // WCS_BUILD_VNS
	|| DropType == WCS_RAHOST_OBJTYPE_VECTOR)
	return (1);

return (0);

} // FoliageEffect::GetRAHostDropOK

/*===========================================================================*/

int FoliageEffect::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
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
	if (this != (FoliageEffect *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (FoliageEffect *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ECOTYPE 
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_FOLIAGEGROUP
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_FOLIAGE
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_RASTER)
	{
	Success = Ecotp.ProcessRAHostDragDrop(DropSource);
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

} // FoliageEffect::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long FoliageEffect::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_FOLEFFECT | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // FoliageEffect::GetRAFlags

/*===========================================================================*/

RasterAnimHost *FoliageEffect::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;
JoeList *CurJoe = Joes;

if (! Current)
	return (&Ecotp);
if (Current == &Ecotp)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Found)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
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

} // FoliageEffect::GetRAHostChild

/*===========================================================================*/

int FoliageEffect::FindBasePosition(RenderData *Rend, VertexDEM *Vert, PolygonData *Poly, Joe *Vector, void *CurVtx)
{
VertexData VertData;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
unsigned long Flags;

if (CurVtx)
	{
	#ifdef WCS_BUILD_SX2
	if (UseFoliageFile && FoliageFileIndex.CellDat)
		{
		PlanetOpt *DefPlanetOpt;
		RealtimeFoliageData *RTFD = (RealtimeFoliageData *)CurVtx;

		if (DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL))
			MyCoords = DefPlanetOpt->Coords;
		else
			MyCoords = NULL;
		// CurVtx contains a RealtimeFoliageData pointer
		VertData.xyz[1] = VertData.Lat = FoliageFileIndex.RefXYZ[1] + RTFD->XYZ[1];
		VertData.xyz[0] = VertData.Lon = FoliageFileIndex.RefXYZ[0] + RTFD->XYZ[0];
		VertData.xyz[2] = VertData.Elev = FoliageFileIndex.RefXYZ[2] + RTFD->XYZ[2];
		if (MyCoords)
			MyCoords->ProjToDefDeg(&VertData);
		Vert->Lat = VertData.Lat;
		Vert->Lon = VertData.Lon;
		Vert->Elev = VertData.Elev;
		Poly->Lat = VertData.Lat;
		Poly->Lon = VertData.Lon;
		Poly->Elev = VertData.Elev;
		Poly->RelEl = 0.0;
		Poly->WaterElev = 0.0;
		Poly->Object = this;
		Poly->Vector = NULL;
		Poly->VectorType = 0;

		Poly->LonSeed = (ULONG)((Vert->Lon - WCS_floor(Vert->Lon)) * ULONG_MAX);
		Poly->LatSeed = (ULONG)((Vert->Lat - WCS_floor(Vert->Lat)) * ULONG_MAX);
		} // else
	else
	#endif // WCS_BUILD_SX2
		{
		// CurVtx contains a VectorPoint
		if (Vector && (MyAttr = (JoeCoordSys *)Vector->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
			MyCoords = MyAttr->Coord;
		else
			MyCoords = NULL;
		if (((VectorPoint *)CurVtx)->ProjToDefDeg(MyCoords, &VertData))
			{
			if (Absolute == WCS_EFFECT_RELATIVETOJOE)			// relative to Joe - apply exaggeration to vector elevation
				{
				Flags = (WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_RELEL | 
					WCS_VERTEXDATA_FLAG_WATERELEV | WCS_VERTEXDATA_FLAG_WATERDEPTH | WCS_VERTEXDATA_FLAG_LAKEAPPLIED |
					WCS_VERTEXDATA_FLAG_STREAMAPPLIED | WCS_VERTEXDATA_FLAG_WAVEHEIGHT);
				Rend->Interactive->VertexDataPoint(Rend, &VertData, Poly, Flags);
				VertData.Elev = AnimPar[WCS_EFFECTS_FOLIAGE_ANIMPAR_ELEV].CurValue + 
					Rend->ElevDatum + (((VectorPoint *)CurVtx)->Elevation - Rend->ElevDatum) * Rend->Exageration;
				} // if relative to Joe
			else if (Absolute == WCS_EFFECT_ABSOLUTE)		// absolute - no exaggeration
				{
				Flags = (WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_RELEL | 
					WCS_VERTEXDATA_FLAG_WATERELEV | WCS_VERTEXDATA_FLAG_WATERDEPTH | WCS_VERTEXDATA_FLAG_LAKEAPPLIED |
					WCS_VERTEXDATA_FLAG_STREAMAPPLIED | WCS_VERTEXDATA_FLAG_WAVEHEIGHT);
				Rend->Interactive->VertexDataPoint(Rend, &VertData, Poly, Flags);
				VertData.Elev = AnimPar[WCS_EFFECTS_FOLIAGE_ANIMPAR_ELEV].CurValue;
				} // if absolute
			else
				{						// relative to ground - exageration applied in VertexDataPoint() to terrain elevation
				Flags = (WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_RELEL | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED |
					WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED | 
					WCS_VERTEXDATA_FLAG_WATERELEV | WCS_VERTEXDATA_FLAG_WATERDEPTH | WCS_VERTEXDATA_FLAG_LAKEAPPLIED |
					WCS_VERTEXDATA_FLAG_STREAMAPPLIED | WCS_VERTEXDATA_FLAG_WAVEHEIGHT);
				Rend->Interactive->VertexDataPoint(Rend, &VertData, Poly, Flags);
				VertData.Elev += AnimPar[WCS_EFFECTS_FOLIAGE_ANIMPAR_ELEV].CurValue;
				} // else relative to ground
			// copy relevant data to vertex and polygon
			Vert->Lat = VertData.Lat;
			Vert->Lon = VertData.Lon;
			Vert->Elev = VertData.Elev;
			Poly->Lat = VertData.Lat;
			Poly->Lon = VertData.Lon;
			Poly->Elev = VertData.Elev;
			Poly->RelEl = VertData.RelEl;
			Poly->WaterElev = VertData.WaterElev;
			Poly->Object = this;
			Poly->Vector = Vector;
			Poly->VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE;

			Poly->LonSeed = (ULONG)((Vert->Lon - WCS_floor(Vert->Lon)) * ULONG_MAX);
			Poly->LatSeed = (ULONG)((Vert->Lat - WCS_floor(Vert->Lat)) * ULONG_MAX);
			return (1);
			} // if
		} // if
	} // if

return (0);

} // FoliageEffect::FindBasePosition

/*===========================================================================*/

#ifdef WCS_USE_OLD_FOLIAGE
int FoliageEffect::SelectImageOrObject(RenderData *Rend, VertexDEM *Vert, PolygonData *Poly,
	FoliagePreviewData *PointData)
{
double Random[10], FolDens, FolHeight, MinHeight, EnvHeightFactor, Value[3], TexOpacity;
FoliageLink *FolLink;
FoliageChainList *CurChain;
VertexDEM FolVtx;
int Success = 1;

if (CurChain = Ecotp.ChainList)
	{
	Value[2] = Value[1] = Value[0] = 0.0;
	Ecotp.Rand.Seed64BitShift(Poly->LatSeed + WCS_SEEDOFFSET_FOLIAGEEFFECT, Poly->LonSeed + WCS_SEEDOFFSET_FOLIAGEEFFECT);

	// seed the random number generator with the polygon center location
	// we'll need potentially 10 random #s for each foliage
	// generate two to start, one to determine if a foliage is needed and one to determine which foliage
	Ecotp.Rand.GenMultiPRN(2, &Random[0]);
	// the granlarity of the random numbers is .00001 - 
	// this prohibits fine control over foliage density at low densities per polygon.
	// The solution is to add a fraction of that granularity if the random # is very small.
	if (Random[0] < .0001)
		Random[0] += (.00001 * Ecotp.Rand.GenPRN());

	// pick a foliage to draw
	FolLink = CurChain->FoliageChain;
	while (Random[1] > FolLink->Density)
		{
		if (! (FolLink = FolLink->Next))
			{
			break;
			} // if
		} // while

	// Foliage may be in the list but disabled if the Image Object or 3D Object itself is disabled
	// The raster may disabled if it failed to load correctly.
	if (FolLink && FolLink->Enabled && ((FolLink->Rast && FolLink->Rast->Enabled) || (FolLink->Fol->Obj && FolLink->Fol->Obj->Enabled)))
		{
		Ecotp.Rand.GenMultiPRN(8, &Random[2]);
		FolVtx.Lat = Poly->Lat;
		FolVtx.Lon = Poly->Lon;
		FolVtx.Elev = Poly->Elev;

		// project the foliage base position
		//Rend->Cam->ProjectVertexDEM(DefCoords, &FolVtx, EarthLatScaleMeters, PlanetRad, 1);	// 1 means convert vertex to cartesian

		// if there are any textures need to transfer texture data, process textures
		FolDens = 1.0;
		if (FolLink->EcoDensTex || FolLink->GrpDensTex || FolLink->FolDensTex || 
			FolLink->EcoHtTex || FolLink->GrpHtTex || FolLink->FolHtTex || FolLink->FolColorTex ||
			FolLink->GrpDensTheme || FolLink->FolDensTheme)
			{
			// transfer polygon data to PixelData
			Rend->TransferTextureData(Poly);

			// set foliage coordinates
			Rend->TexData.Elev = FolVtx.Elev;
			Rend->TexData.Latitude = FolVtx.Lat;
			Rend->TexData.Longitude = FolVtx.Lon;
			Rend->TexData.ZDist = 0.0;	//FolVtx.ScrnXYZ[2];
			Rend->TexData.QDist = 0.0;	//FolVtx.Q;
			Rend->TexData.TLowX = Rend->TexData.THighX = (Rend->TexRefLon - FolVtx.Lon) * Rend->RefLonScaleMeters;
			Rend->TexData.TLowY = Rend->TexData.THighY = (FolVtx.Lat - Rend->TexRefLat) * Rend->EarthLatScaleMeters;
			Rend->TexData.TLowZ = Rend->TexData.THighZ = FolVtx.Elev - Rend->TexRefElev;
			Rend->TexData.TLatRange[0] = Rend->TexData.TLatRange[1] = Rend->TexData.TLatRange[2] = Rend->TexData.TLatRange[3] = FolVtx.Lat;
			Rend->TexData.TLonRange[0] = Rend->TexData.TLonRange[1] = Rend->TexData.TLonRange[2] = Rend->TexData.TLonRange[3] = FolVtx.Lon;
			Rend->TexData.WaterDepth = Poly->WaterElev - FolVtx.Elev;
			Rend->TexData.VectorOffsetsComputed = 0;

			// if there are any density textures we need to reduce the probability of this foliage rendering
			if (FolLink->EcoDensTex)
				{
				// evaluate ecotype density texture
				if ((TexOpacity = FolLink->EcoDensTex->Eval(Value, &Rend->TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						FolDens *= (1.0 - TexOpacity + Value[0]);
						} // if
					else
						FolDens *= Value[0];
					} // if
				} // if
			if (FolLink->GrpDensTheme && FolDens > 0.0)
				{
				if (FolLink->GrpDensTheme->Eval(Value, Poly->Vector))
					FolDens *= .01 * Value[0];	// theme density is given in %
				} // if thematic map
			if (FolLink->GrpDensTex && FolDens > 0.0)
				{
				// evaluate foliage group density texture
				if ((TexOpacity = FolLink->GrpDensTex->Eval(Value, &Rend->TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						FolDens *= (1.0 - TexOpacity + Value[0]);
						} // if
					else
						FolDens *= Value[0];
					} // if
				} // if
			if (FolLink->FolDensTheme && FolDens > 0.0)
				{
				if (FolLink->FolDensTheme->Eval(Value, Poly->Vector))
					FolDens *= .01 * Value[0];	// theme density is given in %
				} // if thematic map
			if (FolLink->FolDensTex && FolDens > 0.0)
				{
				// evaluate foliage density texture
				if ((TexOpacity = FolLink->FolDensTex->Eval(Value, &Rend->TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						FolDens *= (1.0 - TexOpacity + Value[0]);
						} // if
					else
						FolDens *= Value[0];
					} // if
				} // if
			} // if
		//  reevaluate density now that it has been reduced by textures
		if (Random[5] < FolDens)
			{
			// now we determine the height of the foliage, first in meters from the ecotype or group
			if (Ecotp.AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
				{
				if (FolLink->GrpHtTheme && FolLink->GrpHtTheme->Eval(Value, Poly->Vector))
					{
					FolHeight = Value[0];
					if (Ecotp.SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
						{
						MinHeight = FolLink->Grp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue;
						if (MinHeight < FolHeight)
							{
							FolHeight = MinHeight + (FolHeight - MinHeight) * Random[6];
							} // if
						} // if
					else if (Ecotp.SecondHeightType == WCS_ECOTYPE_SECONDHT_RANGEPCT)
						{
						MinHeight = FolHeight - .01 * FolLink->Grp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue;
						FolHeight = MinHeight + 2.0 * (FolHeight - MinHeight) * Random[6];
						} // else if
					else
						{
						MinHeight = FolHeight * .01 * FolLink->Grp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue;
						FolHeight = MinHeight + (FolHeight - MinHeight) * Random[6];
						} // else
					} // if group height thematic map
				else
					FolHeight = CurChain->MaxHeight - CurChain->HeightRange + CurChain->HeightRange * Random[6];

				if (FolLink->FolHtTheme && FolLink->FolHtTheme->Eval(Value, Poly->Vector))
					FolHeight *= .01 * Value[0];	// theme height is in %
				else
					FolHeight *= FolLink->Fol->AnimPar[WCS_FOLIAGE_ANIMPAR_HEIGHT].CurValue;
				} // if
			else
				{
				if (FolLink->EcoHtTheme && FolLink->EcoHtTheme->Eval(Value, Poly->Vector))
					{
					FolHeight = Value[0];
					if (Ecotp.SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
						{
						MinHeight = Ecotp.AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue;
						if (MinHeight < FolHeight)
							{
							FolHeight = MinHeight + (FolHeight - MinHeight) * Random[6];
							} // if
						} // if
					else if (Ecotp.SecondHeightType == WCS_ECOTYPE_SECONDHT_RANGEPCT)
						{
						MinHeight = FolHeight - .01 * Ecotp.AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue;
						FolHeight = MinHeight + 2.0 * (FolHeight - MinHeight) * Random[6];
						} // else if
					else
						{
						MinHeight = FolHeight * .01 * Ecotp.AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue;
						FolHeight = MinHeight + (FolHeight - MinHeight) * Random[6];
						} // else
					} // if ecotype height thematic map
				else
					FolHeight = CurChain->MaxHeight - CurChain->HeightRange + CurChain->HeightRange * Random[6];

				if (FolLink->GrpHtTheme && FolLink->GrpHtTheme->Eval(Value, Poly->Vector))
					FolHeight *= .01 * Value[0];	// theme height is in %
				else
					FolHeight *= .01 * FolLink->Grp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].CurValue;
				if (FolLink->FolHtTheme && FolLink->FolHtTheme->Eval(Value, Poly->Vector))
					FolHeight *= .01 * Value[0];	// theme height is in %
				else
					FolHeight *= FolLink->Fol->AnimPar[WCS_FOLIAGE_ANIMPAR_HEIGHT].CurValue;
				} // else

			// and now with textures which can only reduce the height further
			if (FolLink->EcoHtTex && FolHeight > 0.0)
				{
				// evaluate ecotype height texture
				if ((TexOpacity = FolLink->EcoHtTex->Eval(Value, &Rend->TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						FolHeight *= (1.0 - TexOpacity + Value[0]);
						} // if
					else
						FolHeight *= Value[0];
					} // if
				} // if
			if (FolLink->GrpHtTex && FolHeight > 0.0)
				{
				// evaluate foliage group height texture
				if ((TexOpacity = FolLink->GrpHtTex->Eval(Value, &Rend->TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						FolHeight *= (1.0 - TexOpacity + Value[0]);
						} // if
					else
						FolHeight *= Value[0];
					} // if
				} // if
			if (FolLink->FolHtTex && FolHeight > 0.0)
				{
				// evaluate foliage height texture
				if ((TexOpacity = FolLink->FolHtTex->Eval(Value, &Rend->TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						FolHeight *= (1.0 - TexOpacity + Value[0]);
						} // if
					else
						FolHeight *= Value[0];
					} // if
				} // if
			if (FolHeight > 0.0)
				{
				// Use factor in default Environment if no Environment specified for this polygon (could be Foliage or Ecosystem Effect).
				EnvHeightFactor = Poly->Env ? Poly->Env->AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_FOLIAGEHTFACT].CurValue: 
					1.0;
				FolHeight *= EnvHeightFactor;
				// now paths diverge between Image Objects and 3D Objects
				PointData->Height = FolHeight;
				if (FolLink->Fol->FoliageType == WCS_FOLIAGE_TYPE_OBJECT3D)
					{
					PointData->Rotate[0] = FolLink->Fol->RandomRotate[0] ? FolLink->Fol->Rotate[0] * 2.0 * (Random[7] - .5): 0.0;
					PointData->Rotate[1] = FolLink->Fol->RandomRotate[1] ? FolLink->Fol->Rotate[1] * 2.0 * (Random[8] - .5): 0.0;
					PointData->Rotate[2] = FolLink->Fol->RandomRotate[2] ? FolLink->Fol->Rotate[2] * 2.0 * (Random[9] - .5): 0.0;
					// render the object, pass the rotation set, height, shading options, position of base
					FolLink->Fol->Obj->Rand.Copy(&Ecotp.Rand);

					PointData->Object3D = FolLink->Fol->Obj;
					//Render3DObject(Poly, &FolVtx, FolLink->Fol->Obj, PointData->Rotate, PointData->Height);
					} // if
				else
					{
					PointData->FlipX = (FolLink->Fol->FlipX && Random[7] >= .5);
					PointData->Width = PointData->Height * FolLink->ImageWidthFactor;
					// Determine if there is a color texture and evaluate it.
					// If the opacity of the texture is less than 100% then the 
					// texture color is either blended with the replacement color
					// or blended with the image color during rendering.

					// To apply the color Value[3] to a foliage:
					//	if gray image
					//		RGB = Luminosity * Value[]
					//  if color image
					//		RGB = Luminosity * Value[] + ImageRGB * (1 - Texture Opacity)
					if (Poly->TintFoliage)
						{
						PointData->RGB[0] = Poly->RGB[0];
						PointData->RGB[1] = Poly->RGB[1];
						PointData->RGB[2] = Poly->RGB[2];
						TexOpacity = 1.0;
						} // if
					else if (FolLink->FolColorTheme && FolLink->FolColorTheme->Eval(Value, Poly->Vector))
						{
						double inv255 = 1.0 / 255.0;

						// colors will be in range 0-255 hopefully so we need to reduce to 0-1 and elliminate negatives
						if (Value[0] <= 0.0)
							Value[0] = 0.0;
						else
							Value[0] *= inv255;
						if (Value[1] <= 0.0)
							Value[1] = 0.0;
						else
							Value[1] *= inv255;
						if (Value[2] <= 0.0)
							Value[2] = 0.0;
						else
							Value[2] *= inv255;
						TexOpacity = 1.0;
						} // else if
					else if (FolLink->FolColorTex)
						{
						PointData->RGB[0] = PointData->RGB[1] = PointData->RGB[2] = 0.0;
						if ((TexOpacity = FolLink->FolColorTex->Eval(PointData->RGB, &Rend->TexData)) > 0.0)
							{
							if (TexOpacity < 1.0)
								{
								// if gray image blend texture color with replacement color
								if (! FolLink->ColorImage)
									{
									PointData->RGB[0] += FolLink->Fol->Color.GetCompleteValue(0) * (1.0 - TexOpacity);
									PointData->RGB[1] += FolLink->Fol->Color.GetCompleteValue(1) * (1.0 - TexOpacity);
									PointData->RGB[2] += FolLink->Fol->Color.GetCompleteValue(2) * (1.0 - TexOpacity);
									} // else
								} // if
							} // if
						else if (! FolLink->ColorImage)
							{
							PointData->RGB[0] = FolLink->Fol->Color.GetCompleteValue(0);
							PointData->RGB[1] = FolLink->Fol->Color.GetCompleteValue(1);
							PointData->RGB[2] = FolLink->Fol->Color.GetCompleteValue(2);
							} // else if gray image
						// else Value[] already set to 0's by texture evaluator
						} // if
					else
						{
						TexOpacity = 0.0;
						if (! FolLink->ColorImage)
							{
							PointData->RGB[0] = FolLink->Fol->Color.GetCompleteValue(0);
							PointData->RGB[1] = FolLink->Fol->Color.GetCompleteValue(1);
							PointData->RGB[2] = FolLink->Fol->Color.GetCompleteValue(2);
							} // if gray image
						else
							{
							PointData->RGB[0] = PointData->RGB[1] = PointData->RGB[2] = 0.0;
							} // else need to set Value[] to 0's
						} // else

					PointData->ColorImageOpacity = 1.0 - TexOpacity;
					PointData->Shade3D = FolLink->Shade3D;
					PointData->CurRast = FolLink->Rast;
					// paste the foliage image into the scene
					// pass Value[] for replacement color, 1 - TexOpacity for color image color opacity,
					//  Opacity for foliage opacity, FolLink for various values including raster,
					//  pasted image size and position, FolElevGrad, FolTopElev...
					//if (FlipX)
					//	PlotFoliageReverse(Poly, FolLink, &FolVtx, FolLink->Rast, FolPixelWidth, FolPixelHeight, Value,
					//		FolTopElev, FolElevGrad, Opacity, 1.0 - TexOpacity, MaxZOffset, TestMergeOffset);
					//else
					//	PlotFoliage(Poly, FolLink, &FolVtx, FolLink->Rast, FolPixelWidth, FolPixelHeight, Value,
					//		FolTopElev, FolElevGrad, Opacity, 1.0 - TexOpacity, MaxZOffset, TestMergeOffset);
					} // else it's a 3D Object
				} // if it's got some height
			} // if we pass the last density test
		} // if
	} // if

return (Success);

} // FoliageEffect::SelectImageOrObject
#endif // WCS_USE_OLD_FOLIAGE

/*===========================================================================*/
/*===========================================================================*/

#ifdef WCS_FORESTRY_WIZARD
int FoliageEffect::SelectForestryImageOrObject(RenderData *Rend, PolygonData *Poly, FoliagePreviewData *PointData)
{

return (Ecotp.SelectForestryImageOrObject(Rend, Poly, NULL, WCS_SEEDOFFSET_FOLIAGEEFFECT, TRUE, PointData));

} // FoliageEffect::SelectForestryImageOrObject
#endif // WCS_FORESTRY_WIZARD

/*===========================================================================*/

int FoliageEffect::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{

if (! Ecotp.InitToRender())
	return (0);

return (1);

} // FoliageEffect::InitToRender

/*===========================================================================*/

int FoliageEffect::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{

if ((Absolute  == WCS_EFFECT_ABSOLUTE) && Rend->ExagerateElevLines)
	Elev = (AnimPar[WCS_EFFECTS_FOLIAGE_ANIMPAR_ELEV].CurValue - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
else
	Elev = AnimPar[WCS_EFFECTS_FOLIAGE_ANIMPAR_ELEV].CurValue;

if (! Ecotp.BuildFoliageChain(Rend) || ! Ecotp.InitFrameToRender(Lib, Rend))
	return (0);

return (GeneralEffect::InitFrameToRender(Lib, Rend));

} // FoliageEffect::InitFrameToRender

/*===========================================================================*/

long FoliageEffect::InitImageIDs(long &ImageID)
{
long NumImages = 0;

NumImages += Ecotp.InitImageIDs(ImageID);
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // FoliageEffect::InitImageIDs

/*===========================================================================*/

int FoliageEffect::BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves,
	EffectList **Queries, EffectList **Themes, EffectList **Coords)
{

if (! Ecotp.BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
	return (0);

return (1);

} // FoliageEffect::BuildFileComponentsList

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int FoliageEffect::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
FoliageEffect *CurrentFol = NULL;
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
					else if (! strnicmp(ReadBuf, "Foliage", 8))
						{
						if (CurrentFol = new FoliageEffect(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentFol->Load(ffile, Size, ByteFlip)) == Size)
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

if (Success == 1 && CurrentFol)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentFol);
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

} // FoliageEffect::LoadObject

/*===========================================================================*/

int FoliageEffect::SaveObject(FILE *ffile, const char *SuppliedFileName)
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

// FoliageEffect
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
			} // if FoliageEffect saved 
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

} // FoliageEffect::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::FoliageEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
FoliageEffect *Current;

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
					//case WCS_EFFECTSBASE_RESOLUTION:
					// {
					// BytesRead = ReadBlock(ffile, (char *)&FoliageBase.Resolution, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
					// break;
					// }
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new FoliageEffect(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (FoliageEffect *)FindDuplicateByName(Current->EffectType, Current))
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

} // EffectsLib::FoliageEffect_Load()

/*===========================================================================*/

ULONG EffectsLib::FoliageEffect_Save(FILE *ffile)
{
FoliageEffect *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

//if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTSBASE_RESOLUTION, WCS_BLOCKSIZE_CHAR,
//	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
//	WCS_BLOCKTYPE_FLOAT, (char *)&FoliageBase.Resolution)) == NULL)
// goto WriteError;
//TotalWritten += BytesWritten;

Current = Foliage;
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
	Current = (FoliageEffect *)Current->Next;
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

} // EffectsLib::FoliageEffect_Save()

/*===========================================================================*/
