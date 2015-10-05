// SceneExporter.cpp
// For managing SceneExporter Effects
// Built from scratch on 4/18/03 by Gary R. Huber
// Copyright 2003 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "EffectsLib.h"
#include "Application.h"
#include "Conservatory.h"
#include "Useful.h"
#include "Joe.h"
#include "Requester.h"
#include "EffectsIO.h"
#include "Security.h"
#include "Raster.h"
#include "Database.h"
#include "Project.h"
#include "ExporterEditGUI.h"
#include "SXExtension.h"
#include "Lists.h"
#include "FeatureConfig.h"

long WW_level;
static double origN, origS, origW, origE;

VectorExportItem::VectorExportItem()
{

MyJoe = NULL;
Points = NULL;
NumPoints = 0;

} // VectorExportItem::~VectorExportItem

/*===========================================================================*/

VectorExportItem::~VectorExportItem()
{

if (Points)
	GlobalApp->AppDB->MasterPoint.DeAllocate(Points);
Points = NULL;

} // VectorExportItem::~VectorExportItem

/*===========================================================================*/
/*===========================================================================*/

SceneExporter::SceneExporter()
: GeneralEffect(NULL), GeoReg(this)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_EXPORTER;
SetDefaults();

} // SceneExporter::SceneExporter

/*===========================================================================*/

SceneExporter::SceneExporter(RasterAnimHost *RAHost)
: GeneralEffect(RAHost), GeoReg(this)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_EXPORTER;
SetDefaults();

} // SceneExporter::SceneExporter

/*===========================================================================*/

SceneExporter::SceneExporter(RasterAnimHost *RAHost, EffectsLib *Library, SceneExporter *Proto)
: GeneralEffect(RAHost), GeoReg(this)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_EXPORTER;
Prev = Library->LastExporter;
if (Library->LastExporter)
	{
	Library->LastExporter->Next = this;
	Library->LastExporter = this;
	} // if
else
	{
	Library->Exporter = Library->LastExporter = this;
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
	strcpy(NameBase, "Scene Exporter");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // SceneExporter::SceneExporter

/*===========================================================================*/

SceneExporter::~SceneExporter()
{
EffectList *NextScenario;
NameList *NextName;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->EXP && GlobalApp->GUIWins->EXP->GetActive() == this)
		{
		delete GlobalApp->GUIWins->EXP;
		GlobalApp->GUIWins->EXP = NULL;
		} // if
	} // if

while (Scenarios)
	{
	NextScenario = Scenarios;
	Scenarios = Scenarios->Next;
	delete NextScenario;
	} // while

DeleteExportItems(WCS_EFFECTSSUBCLASS_CAMERA);
DeleteExportItems(WCS_EFFECTSSUBCLASS_LIGHT);
DeleteExportItems(WCS_EFFECTSSUBCLASS_ATMOSPHERE);
while (ScenarioNames)
	{
	NextName = ScenarioNames;
	ScenarioNames = ScenarioNames->Next;
	delete NextName;
	} // if
while (LightNames)
	{
	NextName = LightNames;
	LightNames = LightNames->Next;
	delete NextName;
	} // if
while (CameraNames)
	{
	NextName = CameraNames;
	CameraNames = CameraNames->Next;
	delete NextName;
	} // if
while (HazeNames)
	{
	NextName = HazeNames;
	HazeNames = HazeNames->Next;
	delete NextName;
	} // if
DeleteObjectInstanceList();
DeleteObjectEphemeralList();
DeleteVectorInstanceList();
RemoveFormatExtension();

} // SceneExporter::~SceneExporter

/*===========================================================================*/

void SceneExporter::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_SCENEEXPORTER_NUMANIMPAR] = {0.0, 1.0, 1.0, 0.0, .1, .01, 1.0, 5000.0, 1000000.0, 5000.0, 5000.0, 10000.0, 10000.0};
double RangeDefaults[WCS_EFFECTS_SCENEEXPORTER_NUMANIMPAR][3] = {
												FLT_MAX, 0.0, .01,		// export time
												FLT_MAX, 0.0, .1,		// minimum foliage height
												50.0, 0.001, .1,		// vector width multiplier
												FLT_MAX, 0.0, .1,		// vector elev add
												FLT_MAX, 0.001, .1,		// optimal wall texture scale (m/pixel)
												FLT_MAX, 0.001, .01,	// optimal 3do texture scale (m/pixel)
												1.1, 1.0, .01,			// 3do tex stretch %
												FLT_MAX, 1.0, 10.0,		// distance between levels of detail
												FLT_MAX, 0.0, 100.0,	// distance terrain disappears
												FLT_MAX, 0.0, 100.0,	// distance foliage disappears
												FLT_MAX, 0.0, 100.0,	// distance objects become boxes
												FLT_MAX, 0.0, 100.0,	// distance objects disappear
												FLT_MAX, 0.0, 100.0		// distance labels disappear
												};
long Ct;
double FrameRate;
char TempName[256], TempDirName[512];

if (GlobalApp && GlobalApp->MainProj && GlobalApp->MainProj->Interactive)
	{
	if ((FrameRate = GlobalApp->MainProj->Interactive->GetFrameRate()) <= 0.0)
		FrameRate = 30.0;
	} // if
else
	FrameRate = 30.0;

RangeDefaults[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME][2] = 1.0 / FrameRate;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for

FormatExtension = NULL;
Coords = NULL;
Scenarios = NULL;
ScenarioNames = CameraNames = LightNames = HazeNames = NULL;
ObjectInstanceList = NULL;
DEMTilesX = DEMTilesY = TexTilesX = TexTilesY = DEMTileOverlap = TexTileOverlap = 1;
DEMResX = DEMResY = TexResX = TexResY = SkyRes = 512;
ExportTerrain = ExportVectors = ExportSky = ExportCelest = ExportClouds = ExportStars = ExportFoliage = ExportLabels =
	ExportTexture = Export3DObjects = Export3DFoliage = ExportWalls = ExportAtmosphere = ExportVolumetrics = 1; 
VectorExpType = WCS_EFFECTS_SCENEEXPORTER_VECTYPE_LINES;
TextureFoliageType = WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_BOTHINFIRST;
BoundsType = WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLEDGES;
DEMResOption = TexResOption = SkyResOption = FolResOption = WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE;
FoliageStyle = WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_FLIPBOARDS;
FolTransparencyStyle = WCS_EFFECTS_SCENEEXPORTER_FOLTRANSPARSTYLE_ALPHA;
BurnShadows = BurnShading = TRUE;
BurnVectors = FALSE;
FractalMethod = WCS_FRACTALMETHOD_CONSTANT;
FragmentCollapseType = WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_BOTHINFIRST;
TransparentPixelsExist = 0;
ExportTarget[0] = 0;
strcpy(ImageFormat, "PNG");
strcpy(FoliageImageFormat, "PNG");
SetTarget("NatureView");
strmfp(TempDirName, GlobalApp->MainProj->dirname, "Exports");
TempPath.SetPath(TempDirName);
strcpy(TempName, GlobalApp->MainProj->projectname);
StripExtension(TempName);
OutPath.SetPathAndName(TempDirName, TempName);
RowsRendered = ColsRendered = FrameRendered = 0;
TimeRendered = 0.0;
SingleOpt[0] = MultiOpt[0] = 0;
strcpy(ObjectFormat, "3DS");
strcpy(DEMFormat, "WCS DEM (.elev)");
DEMFormatBackup[0] = 0;
OrigDEMResX = OrigDEMResY = 0;
TextureImageFormat[0] = 0;
MaxDEMTiles = MaxTexTiles = MaxFolRes = MaxTexRes = LONG_MAX;
FoliageRes = MaxWallTexSize = Max3DOTexSize = 512;
MaxDEMRes = MaxDEMVerts = LONG_MAX;
MaxDEMTileOverlap = MaxTexTileOverlap = 1;
MinDEMTileOverlap = MinTexTileOverlap = 1;
RenderSizesSet = 0;
ObjectTreatment = WCS_EFFECTS_SCENEEXPORTER_OBJECTTREATMENT_COPYANY;
LODLevels = 3;
ExportCameras = ExportLights = ExportHaze = 1;
Cameras = NULL;
Lights = NULL;
Haze = NULL;
EphemeralObjects = NULL;
VecInstanceList = NULL;
NumVecInstances = 0;
ZipItUp = 0;
TerrainAsObj = FoliageAsObj = SkyAsObj = VectorAsObj = 0;
OneDEMResX = OneDEMResY = OneTexResX = OneTexResY = 126;
EqualTiles = 0;
NumFoliageBoards = 2;
LODFillGaps = 1;
TileWallTex = 1;
BurnWallShading = 0;
AllowDEMNULL = 1;
MaxCrossBd = 20;
MinCrossBd = 1;
ReplaceNULLElev = (float)-9999.0;
FoliageInstancesAsObj = 0;
AlignFlipBoardsToCamera = 0;
PadFolImageBottom = 0;
Unique3DObjectInstances = 0;
LabelInstance = 0;
SXActionIterator = NULL;
SXActionItemIterator = NULL;
PathBackup[0] = 0;
WallFloors = 0;
WorldFile = 0;
PlanetRadius = EffectsLib::CelestialPresetRadius[3];
ObjectAsObj = WallAsObj = 1;	// for backward compatibility 7/8/05
SquareCells = 0;
MinRenderedElevation = FLT_MAX;
MaxRenderedElevation = -FLT_MAX;

AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_VECELEVADD].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_MINFOLHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].SetMetricType(WCS_ANIMDOUBLE_METRIC_TIME);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMALWALLTEXSCALE].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMAL3DOTEXSCALE].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LODDISTBETWEEN].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LODDISTVANISH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_FOLDISTVANISH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OBJECTDISTBOX].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OBJECTDISTVANISH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LABELDISTVANISH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_3DOTEXSTRETCH].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_MINFOLHT].SetNoNodes(1);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_VECWIDTHMULT].SetNoNodes(1);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_VECELEVADD].SetNoNodes(1);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].SetNoNodes(1);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LODDISTBETWEEN].SetNoNodes(1);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LODDISTVANISH].SetNoNodes(1);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_FOLDISTVANISH].SetNoNodes(1);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OBJECTDISTBOX].SetNoNodes(1);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OBJECTDISTVANISH].SetNoNodes(1);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LABELDISTVANISH].SetNoNodes(1);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMALWALLTEXSCALE].SetNoNodes(1);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMAL3DOTEXSCALE].SetNoNodes(1);
AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_3DOTEXSTRETCH].SetNoNodes(1);

} // SceneExporter::SetDefaults

/*===========================================================================*/

void SceneExporter::Copy(SceneExporter *CopyTo, SceneExporter *CopyFrom)
{
EffectList *NextEffect, **ToEffect;
long Result = -1;
#ifdef WCS_BUILD_VNS
NotifyTag Changes[2];
#endif // WCS_BUILD_VNS

CopyTo->Coords = NULL;
#ifdef WCS_BUILD_VNS
if (CopyFrom->Coords)
	{
	if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
		{
		CopyTo->Coords = (CoordSys *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->Coords);
		} // if no need to make another copy, its all in the family
	else
		{
		if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->Coords->EffectType, CopyFrom->Coords->Name))
			{
			Result = UserMessageCustom("Copy Scene Exporter", "How do you wish to resolve Coordinate System name collisions?\n\nLink to existing Coordinate Systems, replace existing Systems, or create new Systems?",
				"Link", "Create", "Overwrite", 1);
			} // if
		if (Result <= 0)
			{
			CopyTo->Coords = (CoordSys *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->Coords->EffectType, NULL, CopyFrom->Coords);
			} // if create new
		else if (Result == 1)
			{
			CopyTo->Coords = (CoordSys *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->Coords);
			} // if link to existing
		else if (CopyTo->Coords = (CoordSys *)GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->Coords->EffectType, CopyFrom->Coords->Name))
			{
			CopyTo->Coords->Copy(CopyTo->Coords, CopyFrom->Coords);
			Changes[0] = MAKE_ID(CopyTo->Coords->GetNotifyClass(), CopyTo->Coords->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, CopyTo->Coords);
			} // else if found and overwrite
		else
			{
			CopyTo->Coords = (CoordSys *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->Coords->EffectType, NULL, CopyFrom->Coords);
			} // else
		} // else better copy or overwrite it since its important to get just the right object
	} // if
#endif // WCS_BUILD_VNS

#ifdef WCS_RENDER_SCENARIOS
while (CopyTo->Scenarios)
	{
	NextEffect = CopyTo->Scenarios;
	CopyTo->Scenarios = CopyTo->Scenarios->Next;
	delete NextEffect;
	} // if
NextEffect = CopyFrom->Scenarios;
ToEffect = &CopyTo->Scenarios;
while (NextEffect)
	{
	if (NextEffect->Me)
		{
		if (*ToEffect = new EffectList())
			{
			if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
				{
				(*ToEffect)->Me = GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(NextEffect->Me);
				} // if no need to make another copy, its all in the family
			else
				{
				if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(NextEffect->Me->EffectType, NextEffect->Me->Name))
					{
					Result = UserMessageCustom("Copy Scene Exporter", "How do you wish to resolve Render Scenario name collisions?\n\nLink to existing Render Scenarios, replace existing Render Scenarios, or create new Render Scenarios?",
						"Link", "Create", "Overwrite", 1);
					} // if
				if (Result <= 0)
					{
					(*ToEffect)->Me = GlobalApp->CopyToEffectsLib->AddEffect(NextEffect->Me->EffectType, NULL, NextEffect->Me);
					} // if create new
				else if (Result == 1)
					{
					(*ToEffect)->Me = GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(NextEffect->Me);
					} // if link to existing
				else if ((*ToEffect)->Me = GlobalApp->CopyToEffectsLib->FindByName(NextEffect->Me->EffectType, NextEffect->Me->Name))
					{
					((RenderScenario *)(*ToEffect)->Me)->Copy((RenderScenario *)(*ToEffect)->Me, (RenderScenario *)NextEffect->Me);
					Changes[0] = MAKE_ID((*ToEffect)->Me->GetNotifyClass(), (*ToEffect)->Me->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, (*ToEffect)->Me);
					} // else if found and overwrite
				else
					{
					(*ToEffect)->Me = GlobalApp->CopyToEffectsLib->AddEffect(NextEffect->Me->EffectType, NULL, NextEffect->Me);
					} // else
				} // else better copy or overwrite it since its important to get just the right ecosystem
			if ((*ToEffect)->Me)
				ToEffect = &(*ToEffect)->Next;
			else
				{
				delete *ToEffect;
				*ToEffect = NULL;
				} // if
			} // if
		} // if
	NextEffect = NextEffect->Next;
	} // while
#endif // WCS_RENDER_SCENARIOS

// cameras
while (CopyTo->Cameras)
	{
	NextEffect = CopyTo->Cameras;
	CopyTo->Cameras = CopyTo->Cameras->Next;
	delete NextEffect;
	} // if
NextEffect = CopyFrom->Cameras;
ToEffect = &CopyTo->Cameras;
if (GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
	{
	while (NextEffect)
		{
		if (NextEffect->Me)
			{
			if (*ToEffect = new EffectList())
				{
				(*ToEffect)->Me = GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(NextEffect->Me);
				if ((*ToEffect)->Me)
					ToEffect = &(*ToEffect)->Next;
				else
					{
					delete *ToEffect;
					*ToEffect = NULL;
					} // if
				} // if
			} // if
		NextEffect = NextEffect->Next;
		} // while
	} // if
else
	{
	while (NextEffect)
		{
		if (NextEffect->Me)
			{
			if (*ToEffect = new EffectList())
				{
				(*ToEffect)->Me = GlobalApp->CopyToEffectsLib->FindByName(NextEffect->Me->EffectType, NextEffect->Me->GetName());
				if ((*ToEffect)->Me)
					ToEffect = &(*ToEffect)->Next;
				else
					{
					delete *ToEffect;
					*ToEffect = NULL;
					} // if
				} // if
			} // if
		NextEffect = NextEffect->Next;
		} // while
	} // if

// lights
while (CopyTo->Lights)
	{
	NextEffect = CopyTo->Lights;
	CopyTo->Lights = CopyTo->Lights->Next;
	delete NextEffect;
	} // if
NextEffect = CopyFrom->Lights;
ToEffect = &CopyTo->Lights;
if (GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
	{
	while (NextEffect)
		{
		if (NextEffect->Me)
			{
			if (*ToEffect = new EffectList())
				{
				(*ToEffect)->Me = GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(NextEffect->Me);
				if ((*ToEffect)->Me)
					ToEffect = &(*ToEffect)->Next;
				else
					{
					delete *ToEffect;
					*ToEffect = NULL;
					} // if
				} // if
			} // if
		NextEffect = NextEffect->Next;
		} // while
	} // if
else
	{
	while (NextEffect)
		{
		if (NextEffect->Me)
			{
			if (*ToEffect = new EffectList())
				{
				(*ToEffect)->Me = GlobalApp->CopyToEffectsLib->FindByName(NextEffect->Me->EffectType, NextEffect->Me->GetName());
				if ((*ToEffect)->Me)
					ToEffect = &(*ToEffect)->Next;
				else
					{
					delete *ToEffect;
					*ToEffect = NULL;
					} // if
				} // if
			} // if
		NextEffect = NextEffect->Next;
		} // while
	} // if

// atmospheres
while (CopyTo->Haze)
	{
	NextEffect = CopyTo->Haze;
	CopyTo->Haze = CopyTo->Haze->Next;
	delete NextEffect;
	} // if
NextEffect = CopyFrom->Haze;
ToEffect = &CopyTo->Haze;
if (GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
	{
	while (NextEffect)
		{
		if (NextEffect->Me)
			{
			if (*ToEffect = new EffectList())
				{
				(*ToEffect)->Me = GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(NextEffect->Me);
				if ((*ToEffect)->Me)
					ToEffect = &(*ToEffect)->Next;
				else
					{
					delete *ToEffect;
					*ToEffect = NULL;
					} // if
				} // if
			} // if
		NextEffect = NextEffect->Next;
		} // while
	} // if
else
	{
	while (NextEffect)
		{
		if (NextEffect->Me)
			{
			if (*ToEffect = new EffectList())
				{
				(*ToEffect)->Me = GlobalApp->CopyToEffectsLib->FindByName(NextEffect->Me->EffectType, NextEffect->Me->GetName());
				if ((*ToEffect)->Me)
					ToEffect = &(*ToEffect)->Next;
				else
					{
					delete *ToEffect;
					*ToEffect = NULL;
					} // if
				} // if
			} // if
		NextEffect = NextEffect->Next;
		} // while
	} // if

CopyTo->SetTarget(CopyFrom->ExportTarget);
CopyTo->DEMTilesX = CopyFrom->DEMTilesX;
CopyTo->DEMTilesY = CopyFrom->DEMTilesY;
CopyTo->TexTilesX = CopyFrom->TexTilesX;
CopyTo->TexTilesY = CopyFrom->TexTilesY;
CopyTo->OneDEMResX = CopyFrom->OneDEMResX;
CopyTo->OneDEMResY = CopyFrom->OneDEMResY;
CopyTo->OneTexResX = CopyFrom->OneTexResX;
CopyTo->OneTexResY = CopyFrom->OneTexResY;
CopyTo->DEMResX = CopyFrom->DEMResX;
CopyTo->DEMResY = CopyFrom->DEMResY;
CopyTo->TexResX = CopyFrom->TexResX;
CopyTo->TexResY = CopyFrom->TexResY;
CopyTo->SkyRes = CopyFrom->SkyRes;
CopyTo->DEMTileOverlap = CopyFrom->DEMTileOverlap;
CopyTo->TexTileOverlap = CopyFrom->TexTileOverlap;
CopyTo->ExportTerrain = CopyFrom->ExportTerrain;
CopyTo->ExportVectors = CopyFrom->ExportVectors;
CopyTo->ExportSky = CopyFrom->ExportSky;
CopyTo->ExportCelest = CopyFrom->ExportCelest;
CopyTo->ExportClouds = CopyFrom->ExportClouds;
CopyTo->ExportStars = CopyFrom->ExportStars;
CopyTo->ExportFoliage = CopyFrom->ExportFoliage;
CopyTo->ExportLabels = CopyFrom->ExportLabels;
CopyTo->ExportTexture = CopyFrom->ExportTexture;
CopyTo->Export3DObjects = CopyFrom->Export3DObjects;
CopyTo->Export3DFoliage = CopyFrom->Export3DFoliage;
CopyTo->ExportWalls = CopyFrom->ExportWalls;
CopyTo->ExportAtmosphere = CopyFrom->ExportAtmosphere;
CopyTo->ExportVolumetrics = CopyFrom->ExportVolumetrics;
CopyTo->VectorExpType = CopyFrom->VectorExpType;
CopyTo->TextureFoliageType = CopyFrom->TextureFoliageType;
CopyTo->BoundsType = CopyFrom->BoundsType;
CopyTo->DEMResOption = CopyFrom->DEMResOption;
CopyTo->TexResOption = CopyFrom->TexResOption;
CopyTo->SkyResOption = CopyFrom->SkyResOption;
CopyTo->FoliageStyle = CopyFrom->FoliageStyle;
CopyTo->FolTransparencyStyle = CopyFrom->FolTransparencyStyle;
CopyTo->ObjectTreatment = CopyFrom->ObjectTreatment;
CopyTo->BurnShadows = CopyFrom->BurnShadows;
CopyTo->BurnShading = CopyFrom->BurnShading;
CopyTo->BurnVectors = CopyFrom->BurnVectors;
CopyTo->FractalMethod = CopyFrom->FractalMethod;
CopyTo->FolResOption = CopyFrom->FolResOption;
CopyTo->MaxDEMTiles = CopyFrom->MaxDEMTiles;
CopyTo->MaxTexTiles = CopyFrom->MaxTexTiles;
CopyTo->MaxFolRes = CopyFrom->MaxFolRes;
CopyTo->MaxTexRes = CopyFrom->MaxTexRes;
CopyTo->FoliageRes = CopyFrom->FoliageRes;
CopyTo->MaxWallTexSize = CopyFrom->MaxWallTexSize;
CopyTo->Max3DOTexSize = CopyFrom->Max3DOTexSize;
CopyTo->MaxDEMRes = CopyFrom->MaxDEMRes;
CopyTo->MaxDEMVerts = CopyFrom->MaxDEMVerts;
CopyTo->MaxDEMTileOverlap = CopyFrom->MaxDEMTileOverlap;
CopyTo->MinDEMTileOverlap = CopyFrom->MinDEMTileOverlap;
CopyTo->MaxTexTileOverlap = CopyFrom->MaxTexTileOverlap;
CopyTo->MinTexTileOverlap = CopyFrom->MinTexTileOverlap;
CopyTo->LODLevels = CopyFrom->LODLevels;
CopyTo->ExportCameras = CopyFrom->ExportCameras;
CopyTo->ExportLights = CopyFrom->ExportLights;
CopyTo->ExportHaze = CopyFrom->ExportHaze;
CopyTo->ZipItUp = CopyFrom->ZipItUp;
CopyTo->TerrainAsObj = CopyFrom->TerrainAsObj;
CopyTo->FoliageAsObj = CopyFrom->FoliageAsObj;
CopyTo->SkyAsObj = CopyFrom->SkyAsObj;
CopyTo->VectorAsObj = CopyFrom->VectorAsObj;
CopyTo->ObjectAsObj = CopyFrom->ObjectAsObj;
CopyTo->WallAsObj = CopyFrom->WallAsObj;
CopyTo->EqualTiles = CopyFrom->EqualTiles;
CopyTo->NumFoliageBoards = CopyFrom->NumFoliageBoards;
CopyTo->LODFillGaps = CopyFrom->LODFillGaps;
CopyTo->TileWallTex = CopyFrom->TileWallTex;
CopyTo->BurnWallShading = CopyFrom->BurnWallShading;
CopyTo->AllowDEMNULL = CopyFrom->AllowDEMNULL;
CopyTo->MaxCrossBd = CopyFrom->MaxCrossBd;
CopyTo->MinCrossBd = CopyFrom->MinCrossBd;
CopyTo->FoliageInstancesAsObj = CopyFrom->FoliageInstancesAsObj;
CopyTo->AlignFlipBoardsToCamera = CopyFrom->AlignFlipBoardsToCamera;
CopyTo->PadFolImageBottom = CopyFrom->PadFolImageBottom;
CopyTo->WallFloors = CopyFrom->WallFloors;
CopyTo->WorldFile = CopyFrom->WorldFile;
CopyTo->SquareCells = CopyFrom->SquareCells;
strcpy(CopyTo->ImageFormat, CopyFrom->ImageFormat);
strcpy(CopyTo->FoliageImageFormat, CopyFrom->FoliageImageFormat);
strcpy(CopyTo->SingleOpt, CopyFrom->SingleOpt);
strcpy(CopyTo->MultiOpt, CopyFrom->MultiOpt);
strcpy(CopyTo->ObjectFormat, CopyFrom->ObjectFormat);
strcpy(CopyTo->DEMFormat, CopyFrom->DEMFormat);
strcpy(CopyTo->DEMFormatBackup, CopyFrom->DEMFormatBackup);
CopyTo->OrigDEMResX = CopyFrom->OrigDEMResX;
CopyTo->OrigDEMResY = CopyFrom->OrigDEMResY;
strcpy(CopyTo->TextureImageFormat, CopyFrom->TextureImageFormat);
GeoReg.Copy(&CopyTo->GeoReg, &CopyFrom->GeoReg);
OutPath.Copy(&CopyTo->OutPath, &CopyFrom->OutPath);
TempPath.Copy(&CopyTo->TempPath, &CopyFrom->TempPath);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

if (CopyTo->FormatExtension && CopyFrom->FormatExtension)
	CopyTo->FormatExtension->Copy(CopyFrom->FormatExtension);

} // SceneExporter::Copy

/*===========================================================================*/

SXExtension *SceneExporter::AllocFormatExtension(void)
{

if (! stricmp(ExportTarget, "STL") || ! stricmp(ExportTarget, "VRML-STL"))
	{
	// two formats share one extension type
	if (FormatExtension && FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_STL)
		return (FormatExtension);
	RemoveFormatExtension();
	FormatExtension = (SXExtension *)(new SXExtensionSTL);
	} // if
else if (! stricmp(ExportTarget, "NatureView"))
	{
	if (FormatExtension && FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_NVE)
		return (FormatExtension);
	RemoveFormatExtension();
	FormatExtension = (SXExtension *)(new SXExtensionNVE);
	} // if
else if (! stricmp(ExportTarget, "OpenFlight"))
	{
	if (FormatExtension && FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_OPENFLIGHT)
		return (FormatExtension);
	RemoveFormatExtension();
	FormatExtension = (SXExtension *)(new SXExtensionOF);
	} // if
else if (! stricmp(ExportTarget, "FBX"))
	{
	if (FormatExtension && FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_FBX)
		return (FormatExtension);
	RemoveFormatExtension();
	FormatExtension = (SXExtension *)(new SXExtensionFBX);
	} // if
else if (! stricmp(ExportTarget, "GoogleEarth"))
	{
	if (FormatExtension && FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_GE)
		return (FormatExtension);
	RemoveFormatExtension();
	FormatExtension = (SXExtension *)(new SXExtensionGE);
	} // if
else
	RemoveFormatExtension();

return FormatExtension;

} // SceneExporter::AllocFormatExtension

/*===========================================================================*/

void SceneExporter::RemoveFormatExtension(void)
{

if (FormatExtension)
	delete FormatExtension;
FormatExtension = NULL;

} // SceneExporter::RemoveFormatExtension

/*===========================================================================*/

long SceneExporter::SaveLabelImage(Raster *LabelRast, ImageLib *Lib, Label *Labl)
{
char NameStr[24];

// find export directory
LabelRast->PAF.SetPath((char *)OutPath.GetPath());
// come up with a name
sprintf(NameStr, "LI%06d.png", LabelInstance ++);
LabelRast->PAF.SetName(NameStr);
strcpy(LabelRast->Name, NameStr);	// don't care if it is unique

// create alpha channel from AltFloatMap
LabelRast->CopyCoverageToAlpha();

// save image
LabelRast->SaveImage(TRUE);

// add to image lib
AddObjectToEphemeralList(LabelRast);

// set foliage attribute
LabelRast->AddAttribute(WCS_RASTERSHELL_TYPE_FOLIAGE, NULL, Labl);

// return image ID if success
return (Lib->AddRasterReturnID(LabelRast));

} // SceneExporter::SaveLabelImage

/*===========================================================================*/

ULONG SceneExporter::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char TempName[256];
EffectList **CurScenario = &Scenarios, **CurCamera = &Cameras, **CurLight = &Lights, **CurHaze = &Haze;
NameList **CurScenarioName = &ScenarioNames, **CurCameraName = &CameraNames, **CurLightName = &LightNames, **CurHazeName = &HazeNames;

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
					case WCS_EFFECTS_SCENEEXPORTER_EXPORTTARGET:
						{
						BytesRead = ReadBlock(ffile, (char *)TempName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						SetTarget(TempName);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_IMAGEFORMAT:
						{
						BytesRead = ReadBlock(ffile, (char *)ImageFormat, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_FOLIMAGEFORMAT:
						{
						BytesRead = ReadBlock(ffile, (char *)FoliageImageFormat, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_SINGLEOPT:
						{
						BytesRead = ReadBlock(ffile, (char *)SingleOpt, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_MULTIOPT:
						{
						BytesRead = ReadBlock(ffile, (char *)MultiOpt, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_OBJECTFORMAT:
						{
						BytesRead = ReadBlock(ffile, (char *)ObjectFormat, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_DEMFORMAT:
						{
						BytesRead = ReadBlock(ffile, (char *)DEMFormat, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_EXPORTTERRAIN:
						{
						BytesRead = ReadBlock(ffile, (char *)&ExportTerrain, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_EXPORTVECTORS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ExportVectors, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_EXPORTSKY:
						{
						BytesRead = ReadBlock(ffile, (char *)&ExportSky, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_EXPORTCELEST:
						{
						BytesRead = ReadBlock(ffile, (char *)&ExportCelest, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_EXPORTCLOUDS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ExportClouds, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_EXPORTSTARS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ExportStars, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_EXPORTFOLIAGE:
						{
						BytesRead = ReadBlock(ffile, (char *)&ExportFoliage, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_EXPORTLABELS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ExportLabels, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_EXPORTTEXTURE:
						{
						BytesRead = ReadBlock(ffile, (char *)&ExportTexture, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_EXPORT3DOBJECTS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Export3DObjects, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_EXPORT3DFOLIAGE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Export3DFoliage, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_EXPORTWALLS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ExportWalls, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_EXPORTATMOSPHERE:
						{
						BytesRead = ReadBlock(ffile, (char *)&ExportAtmosphere, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_EXPORTVOLUMETRICS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ExportVolumetrics, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_VECTOREXPTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&VectorExpType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_TEXTUREFOLIAGETYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&TextureFoliageType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_OBJECTTREATMENT:
						{
						BytesRead = ReadBlock(ffile, (char *)&ObjectTreatment, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&BoundsType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_DEMRESOPTION:
						{
						BytesRead = ReadBlock(ffile, (char *)&DEMResOption, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_TEXRESOPTION:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexResOption, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_SKYRESOPTION:
						{
						BytesRead = ReadBlock(ffile, (char *)&SkyResOption, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_FOLRESOPTION:
						{
						BytesRead = ReadBlock(ffile, (char *)&FolResOption, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_FOLIAGESTYLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&FoliageStyle, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_EXPORTCAMERAS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ExportCameras, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_EXPORTLIGHTS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ExportLights, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_EXPORTHAZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&ExportHaze, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_ZIPITUP:
						{
						BytesRead = ReadBlock(ffile, (char *)&ZipItUp, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_FOLIAGETRANSPARSTYLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&FolTransparencyStyle, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_BURNSHADOWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&BurnShadows, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_BURNSHADING:
						{
						BytesRead = ReadBlock(ffile, (char *)&BurnShading, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_BURNVECTORS:
						{
						BytesRead = ReadBlock(ffile, (char *)&BurnVectors, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_FRACTALMETHOD:
						{
						BytesRead = ReadBlock(ffile, (char *)&FractalMethod, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_TERRAINASOBJ:
						{
						BytesRead = ReadBlock(ffile, (char *)&TerrainAsObj, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_FOLIAGEASOBJ:
						{
						BytesRead = ReadBlock(ffile, (char *)&FoliageAsObj, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_SKYASOBJ:
						{
						BytesRead = ReadBlock(ffile, (char *)&SkyAsObj, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_VECTORASOBJ:
						{
						BytesRead = ReadBlock(ffile, (char *)&VectorAsObj, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_OBJECTASOBJ:
						{
						BytesRead = ReadBlock(ffile, (char *)&ObjectAsObj, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_WALLASOBJ:
						{
						BytesRead = ReadBlock(ffile, (char *)&WallAsObj, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_EQUALTILES:
						{
						BytesRead = ReadBlock(ffile, (char *)&EqualTiles, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_LODFILLGAPS:
						{
						BytesRead = ReadBlock(ffile, (char *)&LODFillGaps, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_BURNWALLSHADING:
						{
						BytesRead = ReadBlock(ffile, (char *)&BurnWallShading, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_TILEWALLTEX:
						{
						BytesRead = ReadBlock(ffile, (char *)&TileWallTex, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_ALLOWDEMNULL:
						{
						BytesRead = ReadBlock(ffile, (char *)&AllowDEMNULL, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_FOLINSTANCEASOBJ:
						{
						BytesRead = ReadBlock(ffile, (char *)&FoliageInstancesAsObj, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_ALIGNFLIPBDTOCAM:
						{
						BytesRead = ReadBlock(ffile, (char *)&AlignFlipBoardsToCamera, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_PADFOLIMGBOTTOM:
						{
						BytesRead = ReadBlock(ffile, (char *)&PadFolImageBottom, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_WALLFLOORS:
						{
						BytesRead = ReadBlock(ffile, (char *)&WallFloors, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_WORLDFILE:
						{
						BytesRead = ReadBlock(ffile, (char *)&WorldFile, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_SQUARECELLS:
						{
						BytesRead = ReadBlock(ffile, (char *)&SquareCells, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_DEMTILESX:
						{
						BytesRead = ReadBlock(ffile, (char *)&DEMTilesX, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_DEMTILESY:
						{
						BytesRead = ReadBlock(ffile, (char *)&DEMTilesY, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_TEXTILESX:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexTilesX, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_TEXTILESY:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexTilesY, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_DEMRESX:
						{
						BytesRead = ReadBlock(ffile, (char *)&DEMResX, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_DEMRESY:
						{
						BytesRead = ReadBlock(ffile, (char *)&DEMResY, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_TEXRESX:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexResX, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_TEXRESY:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexResY, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_ONEDEMRESX:
						{
						BytesRead = ReadBlock(ffile, (char *)&OneDEMResX, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_ONEDEMRESY:
						{
						BytesRead = ReadBlock(ffile, (char *)&OneDEMResY, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_ONETEXRESX:
						{
						BytesRead = ReadBlock(ffile, (char *)&OneTexResX, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_ONETEXRESY:
						{
						BytesRead = ReadBlock(ffile, (char *)&OneTexResY, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_SKYRES:
						{
						BytesRead = ReadBlock(ffile, (char *)&SkyRes, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_FOLRES:
						{
						BytesRead = ReadBlock(ffile, (char *)&FoliageRes, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_DEMTILEOVERLAP:
						{
						BytesRead = ReadBlock(ffile, (char *)&DEMTileOverlap, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_TEXTILEOVERLAP:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexTileOverlap, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_MAXDEMTILES:
						{
						BytesRead = ReadBlock(ffile, (char *)&MaxDEMTiles, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_MAXTEXTILES:
						{
						BytesRead = ReadBlock(ffile, (char *)&MaxTexTiles, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_MAXFOLRES:
						{
						BytesRead = ReadBlock(ffile, (char *)&MaxFolRes, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_MAXTEXRES:
						{
						BytesRead = ReadBlock(ffile, (char *)&MaxTexRes, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_MAXDEMRES:
						{
						BytesRead = ReadBlock(ffile, (char *)&MaxDEMRes, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_MAXDEMVERTS:
						{
						BytesRead = ReadBlock(ffile, (char *)&MaxDEMVerts, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_MAXDEMTILEOVERLAP:
						{
						BytesRead = ReadBlock(ffile, (char *)&MaxDEMTileOverlap, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_MINDEMTILEOVERLAP:
						{
						BytesRead = ReadBlock(ffile, (char *)&MinDEMTileOverlap, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_MAXTEXTILEOVERLAP:
						{
						BytesRead = ReadBlock(ffile, (char *)&MaxTexTileOverlap, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_MINTEXTILEOVERLAP:
						{
						BytesRead = ReadBlock(ffile, (char *)&MinTexTileOverlap, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_LODLEVELS:
						{
						BytesRead = ReadBlock(ffile, (char *)&LODLevels, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_MAXWALLTEXSIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&MaxWallTexSize, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_MAX3DOTEXSIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Max3DOTexSize, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_NUMFOLIAGEBOARDS:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumFoliageBoards, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_MAXCROSSBD:
						{
						BytesRead = ReadBlock(ffile, (char *)&MaxCrossBd, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_MINCROSSBD:
						{
						BytesRead = ReadBlock(ffile, (char *)&MinCrossBd, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_EXPORTTIME:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_MINFOLHT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_MINFOLHT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_VECWIDTHMULT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_VECWIDTHMULT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_VECELEVADD:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_VECELEVADD].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_OPTWALLTEXSCALE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMALWALLTEXSCALE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_OPT3DOTEXSCALE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OPTIMAL3DOTEXSCALE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_3DOTEXSTRETCH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_3DOTEXSTRETCH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_LODDISTBETWEEN:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LODDISTBETWEEN].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_LODDISTVANISH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LODDISTVANISH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_FOLDISTVANISH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_FOLDISTVANISH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_OBJECTDISTBOX:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OBJECTDISTBOX].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_OBJECTDISTVANISH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OBJECTDISTVANISH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_LABELDISTVANISH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LABELDISTVANISH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_GEOREG:
						{
						BytesRead = GeoReg.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_TEMPPATH:
						{
						BytesRead = TempPath.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_OUTPATH:
						{
						BytesRead = OutPath.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_COORDSYSNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)TempName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						#ifdef WCS_BUILD_VNS
						if (TempName[0])
							{
							Coords = (CoordSys *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_COORDSYS, TempName);
							} // if
						#endif // WCS_BUILD_VNS
						break;
						}
					#ifdef WCS_RENDER_SCENARIOS
					case WCS_EFFECTS_SCENEEXPORTER_SCENARIONAME:
						{
						BytesRead = ReadBlock(ffile, (char *)TempName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempName[0])
							{
							if (*CurScenario = new EffectList())
								{
								if ((*CurScenario)->Me = GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_SCENARIO, TempName))
									CurScenario = &(*CurScenario)->Next;
								else
									{
									delete *CurScenario;
									*CurScenario = NULL;
									if (*CurScenarioName = new NameList(TempName, WCS_EFFECTSSUBCLASS_SCENARIO))
										{
										CurScenarioName = &(*CurScenarioName)->Next;
										} // if
									} // else
								} // if
							} // if
						break;
						}
					#endif // WCS_RENDER_SCENARIOS
					case WCS_EFFECTS_SCENEEXPORTER_CAMERANAME:
						{
						BytesRead = ReadBlock(ffile, (char *)TempName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempName[0])
							{
							if (*CurCamera = new EffectList())
								{
								if ((*CurCamera)->Me = GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_CAMERA, TempName))
									CurCamera = &(*CurCamera)->Next;
								else
									{
									delete *CurCamera;
									*CurCamera = NULL;
									if (*CurCameraName = new NameList(TempName, WCS_EFFECTSSUBCLASS_CAMERA))
										{
										CurCameraName = &(*CurCameraName)->Next;
										} // if
									} // else
								} // if
							} // if
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_LIGHTNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)TempName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempName[0])
							{
							if (*CurLight = new EffectList())
								{
								if ((*CurLight)->Me = GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_LIGHT, TempName))
									CurLight = &(*CurLight)->Next;
								else
									{
									delete *CurLight;
									*CurLight = NULL;
									if (*CurLightName = new NameList(TempName, WCS_EFFECTSSUBCLASS_LIGHT))
										{
										CurLightName = &(*CurLightName)->Next;
										} // if
									} // else
								} // if
							} // if
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_ATMONAME:
						{
						BytesRead = ReadBlock(ffile, (char *)TempName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempName[0])
							{
							if (*CurHaze = new EffectList())
								{
								if ((*CurHaze)->Me = GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_ATMOSPHERE, TempName))
									CurHaze = &(*CurHaze)->Next;
								else
									{
									delete *CurHaze;
									*CurHaze = NULL;
									if (*CurHazeName = new NameList(TempName, WCS_EFFECTSSUBCLASS_ATMOSPHERE))
										{
										CurHazeName = &(*CurHazeName)->Next;
										} // if
									} // else
								} // if
							} // if
						break;
						}
					case WCS_EFFECTS_SCENEEXPORTER_SXEXTENSION:
						{
						if (AllocFormatExtension())
							BytesRead = FormatExtension->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} // WCS_EFFECTS_SCENEEXPORTER_SXEXTENSION
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

#ifndef WCS_BUILD_VNS
// largest DEM and tex res in WCS is 2049
if (OneDEMResX > 2049)
	{
	if (DEMResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1)
		OneDEMResX = 2049;
	else
		OneDEMResX = 2048;
	} // if
if (OneDEMResY > 2049)
	{
	if (DEMResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1)
		OneDEMResY = 2049;
	else
		OneDEMResY = 2048;
	} // if
if (OneTexResX > 2049)
	{
	if (TexResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1)
		OneTexResX = 2049;
	else
		OneTexResX = 2048;
	} // if
if (OneTexResY > 2049)
	{
	if (DEMResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1)
		OneTexResY = 2049;
	else
		OneTexResY = 2048;
	} // if
if (DEMTilesX * OneDEMResX > 2049)
	DEMTilesX = 2049 / OneDEMResX;
if (DEMTilesY * OneDEMResY > 2049)
	DEMTilesY = 2049 / OneDEMResY;
if (TexTilesX * OneTexResX > 2049)
	TexTilesX = 2049 / OneTexResX;
if (TexTilesY * OneTexResY > 2049)
	TexTilesY = 2049 / OneTexResY;

#endif // WCS_BUILD_VNS

return (TotalRead);

} // SceneExporter::Load

/*===========================================================================*/

unsigned long int SceneExporter::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_EFFECTS_SCENEEXPORTER_NUMANIMPAR] = {WCS_EFFECTS_SCENEEXPORTER_EXPORTTIME,
																		WCS_EFFECTS_SCENEEXPORTER_MINFOLHT,
																		WCS_EFFECTS_SCENEEXPORTER_VECWIDTHMULT,
																		WCS_EFFECTS_SCENEEXPORTER_VECELEVADD,
																		WCS_EFFECTS_SCENEEXPORTER_OPTWALLTEXSCALE,
																		WCS_EFFECTS_SCENEEXPORTER_OPT3DOTEXSCALE,
																		WCS_EFFECTS_SCENEEXPORTER_3DOTEXSTRETCH,
																		WCS_EFFECTS_SCENEEXPORTER_LODDISTBETWEEN,
																		WCS_EFFECTS_SCENEEXPORTER_LODDISTVANISH,
																		WCS_EFFECTS_SCENEEXPORTER_FOLDISTVANISH,
																		WCS_EFFECTS_SCENEEXPORTER_OBJECTDISTBOX,
																		WCS_EFFECTS_SCENEEXPORTER_OBJECTDISTVANISH,
																		WCS_EFFECTS_SCENEEXPORTER_LABELDISTVANISH};
EffectList *CurEffect;

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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_EXPORTTARGET, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(ExportTarget) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)ExportTarget)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_IMAGEFORMAT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(ImageFormat) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)ImageFormat)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_FOLIMAGEFORMAT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(FoliageImageFormat) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)FoliageImageFormat)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_SINGLEOPT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(SingleOpt) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)SingleOpt)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_MULTIOPT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(MultiOpt) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)MultiOpt)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_OBJECTFORMAT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(ObjectFormat) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)ObjectFormat)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_DEMFORMAT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(DEMFormat) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)DEMFormat)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_EXPORTTERRAIN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ExportTerrain)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_EXPORTVECTORS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ExportVectors)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_EXPORTSKY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ExportSky)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_EXPORTCELEST, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ExportCelest)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_EXPORTCLOUDS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ExportClouds)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_EXPORTSTARS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ExportStars)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_EXPORTFOLIAGE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ExportFoliage)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_EXPORTLABELS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ExportLabels)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_EXPORTTEXTURE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ExportTexture)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_EXPORT3DOBJECTS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Export3DObjects)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_EXPORT3DFOLIAGE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Export3DFoliage)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_EXPORTWALLS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ExportWalls)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_EXPORTATMOSPHERE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ExportAtmosphere)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_EXPORTVOLUMETRICS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ExportVolumetrics)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_VECTOREXPTYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VectorExpType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_TEXTUREFOLIAGETYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TextureFoliageType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_OBJECTTREATMENT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ObjectTreatment)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&BoundsType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_DEMRESOPTION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DEMResOption)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_TEXRESOPTION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TexResOption)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_SKYRESOPTION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&SkyResOption)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_FOLRESOPTION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FolResOption)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_FOLIAGESTYLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FoliageStyle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_FOLIAGETRANSPARSTYLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FolTransparencyStyle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_BURNSHADOWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&BurnShadows)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_BURNSHADING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&BurnShading)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_BURNVECTORS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&BurnVectors)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_FRACTALMETHOD, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FractalMethod)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_EXPORTCAMERAS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ExportCameras)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_EXPORTLIGHTS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ExportLights)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_EXPORTHAZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ExportHaze)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_ZIPITUP, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ZipItUp)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_TERRAINASOBJ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TerrainAsObj)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_FOLIAGEASOBJ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FoliageAsObj)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_SKYASOBJ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&SkyAsObj)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_VECTORASOBJ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VectorAsObj)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_OBJECTASOBJ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ObjectAsObj)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_WALLASOBJ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&WallAsObj)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_EQUALTILES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&EqualTiles)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_LODFILLGAPS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LODFillGaps)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_TILEWALLTEX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TileWallTex)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_BURNWALLSHADING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&BurnWallShading)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_ALLOWDEMNULL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AllowDEMNULL)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_FOLINSTANCEASOBJ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FoliageInstancesAsObj)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_ALIGNFLIPBDTOCAM, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AlignFlipBoardsToCamera)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_PADFOLIMGBOTTOM, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PadFolImageBottom)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_WALLFLOORS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&WallFloors)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_WORLDFILE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&WorldFile)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_SQUARECELLS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&SquareCells)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_DEMTILESX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&DEMTilesX)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_DEMTILESY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&DEMTilesY)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_TEXTILESX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&TexTilesX)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_TEXTILESY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&TexTilesY)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_DEMRESX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&DEMResX)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_DEMRESY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&DEMResY)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_TEXRESX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&TexResX)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_TEXRESY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&TexResY)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_ONEDEMRESX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&OneDEMResX)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_ONEDEMRESY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&OneDEMResY)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_ONETEXRESX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&OneTexResX)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_ONETEXRESY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&OneTexResY)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_SKYRES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&SkyRes)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_DEMTILEOVERLAP, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&DEMTileOverlap)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_TEXTILEOVERLAP, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&TexTileOverlap)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_FOLRES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&FoliageRes)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_MAXDEMTILES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&MaxDEMTiles)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_MAXTEXTILES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&MaxTexTiles)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_MAXFOLRES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&MaxFolRes)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_MAXTEXRES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&MaxTexRes)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_MAXDEMRES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&MaxDEMRes)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_MAXDEMVERTS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&MaxDEMVerts)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_MAXDEMTILEOVERLAP, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&MaxDEMTileOverlap)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_MINDEMTILEOVERLAP, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&MinDEMTileOverlap)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_MAXTEXTILEOVERLAP, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&MaxTexTileOverlap)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_MINTEXTILEOVERLAP, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&MinTexTileOverlap)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_LODLEVELS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&LODLevels)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_MAXWALLTEXSIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&MaxWallTexSize)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_MAX3DOTEXSIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Max3DOTexSize)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_NUMFOLIAGEBOARDS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumFoliageBoards)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_MAXCROSSBD, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&MaxCrossBd)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_MINCROSSBD, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&MinCrossBd)) == NULL)
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

ItemTag = WCS_EFFECTS_SCENEEXPORTER_GEOREG + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = GeoReg.Save(ffile))
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
			} // if registration saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_EFFECTS_SCENEEXPORTER_TEMPPATH + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = TempPath.Save(ffile))
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
			} // if file path saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_EFFECTS_SCENEEXPORTER_OUTPATH + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = OutPath.Save(ffile))
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
			} // if file path saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

#ifdef WCS_BUILD_VNS
if (Coords)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_COORDSYSNAME, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Coords->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Coords->GetName())) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
#endif // WCS_BUILD_VNS

#ifdef WCS_RENDER_SCENARIOS
CurEffect = Scenarios;
while (CurEffect)
	{
	if (CurEffect->Me)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_SCENARIONAME, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(CurEffect->Me->GetName()) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)CurEffect->Me->GetName())) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	CurEffect = CurEffect->Next;
	} // while
#endif // WCS_RENDER_SCENARIOS

CurEffect = Cameras;
while (CurEffect)
	{
	if (CurEffect->Me)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_CAMERANAME, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(CurEffect->Me->GetName()) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)CurEffect->Me->GetName())) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	CurEffect = CurEffect->Next;
	} // while

CurEffect = Lights;
while (CurEffect)
	{
	if (CurEffect->Me)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_LIGHTNAME, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(CurEffect->Me->GetName()) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)CurEffect->Me->GetName())) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	CurEffect = CurEffect->Next;
	} // while

CurEffect = Haze;
while (CurEffect)
	{
	if (CurEffect->Me)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENEEXPORTER_ATMONAME, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(CurEffect->Me->GetName()) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)CurEffect->Me->GetName())) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	CurEffect = CurEffect->Next;
	} // while

if (FormatExtension)
	{
	ItemTag = WCS_EFFECTS_SCENEEXPORTER_SXEXTENSION + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = FormatExtension->Save(ffile))
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
				} // if FormatExtension saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
   goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // SceneExporter::Save

/*===========================================================================*/

void SceneExporter::ResolveLoadLinkages(EffectsLib *Lib)
{
NameList *CurCamera = CameraNames, *NextName;
EffectList **CurCameraItem = &Cameras;
NameList *CurLight = LightNames;
EffectList **CurLightItem = &Lights;
NameList *CurHaze = HazeNames;
EffectList **CurHazeItem = &Haze;
#ifdef WCS_RENDER_SCENARIOS
NameList *CurScenario = ScenarioNames;
EffectList **CurScenarioItem = &Scenarios;

while (*CurScenarioItem)
	{
	CurScenarioItem = &(*CurScenarioItem)->Next;
	} // while

while (CurScenario)
	{
	if (*CurScenarioItem = new EffectList())
		{
		if ((*CurScenarioItem)->Me = Lib->FindByName(CurScenario->ItemClass, CurScenario->Name))
			CurScenarioItem = &(*CurScenarioItem)->Next;
		else
			{
			delete *CurScenarioItem;
			*CurScenarioItem = NULL;
			} // else
		} // if
	CurScenario = CurScenario->Next;
	} // if

CurScenario = ScenarioNames;
while (CurScenario)
	{
	NextName = CurScenario->Next;
	delete CurScenario;
	CurScenario = NextName;
	} // if
ScenarioNames = NULL;
#endif // WCS_RENDER_SCENARIOS

// cameras
while (*CurCameraItem)
	{
	CurCameraItem = &(*CurCameraItem)->Next;
	} // while

while (CurCamera)
	{
	if (*CurCameraItem = new EffectList())
		{
		if ((*CurCameraItem)->Me = Lib->FindByName(CurCamera->ItemClass, CurCamera->Name))
			CurCameraItem = &(*CurCameraItem)->Next;
		else
			{
			delete *CurCameraItem;
			*CurCameraItem = NULL;
			} // else
		} // if
	CurCamera = CurCamera->Next;
	} // if

CurCamera = CameraNames;
while (CurCamera)
	{
	NextName = CurCamera->Next;
	delete CurCamera;
	CurCamera = NextName;
	} // if
CameraNames = NULL;

// lights
while (*CurLightItem)
	{
	CurLightItem = &(*CurLightItem)->Next;
	} // while

while (CurLight)
	{
	if (*CurLightItem = new EffectList())
		{
		if ((*CurLightItem)->Me = Lib->FindByName(CurLight->ItemClass, CurLight->Name))
			CurLightItem = &(*CurLightItem)->Next;
		else
			{
			delete *CurLightItem;
			*CurLightItem = NULL;
			} // else
		} // if
	CurLight = CurLight->Next;
	} // if

CurLight = LightNames;
while (CurLight)
	{
	NextName = CurLight->Next;
	delete CurLight;
	CurLight = NextName;
	} // if
LightNames = NULL;

// haze
while (*CurHazeItem)
	{
	CurHazeItem = &(*CurHazeItem)->Next;
	} // while

while (CurHaze)
	{
	if (*CurHazeItem = new EffectList())
		{
		if ((*CurHazeItem)->Me = Lib->FindByName(CurHaze->ItemClass, CurHaze->Name))
			CurHazeItem = &(*CurHazeItem)->Next;
		else
			{
			delete *CurHazeItem;
			*CurHazeItem = NULL;
			} // else
		} // if
	CurHaze = CurHaze->Next;
	} // if

CurHaze = HazeNames;
while (CurHaze)
	{
	NextName = CurHaze->Next;
	delete CurHaze;
	CurHaze = NextName;
	} // if
HazeNames = NULL;

} // SceneExporter::ResolveLoadLinkages

/*===========================================================================*/

void SceneExporter::ResolveDBLoadLinkages(Database *HostDB, Joe **UniqueIDTable, unsigned long HighestDBID)
{

if (FormatExtension)
	FormatExtension->ResolveDBLoadLinkages(HostDB, UniqueIDTable, HighestDBID);

} // SceneExporter::ResolveDBLoadLinkages

/*===========================================================================*/

char *SceneExporterCritterNames[WCS_EFFECTS_SCENEEXPORTER_NUMANIMPAR] = {"Export Time", "Minimum Foliage Height (m)", 
	"Vector Width Multiplier", "Vector Added Elevation", "Opt Wall Tex Scale (m/pix)", "Opt 3D Object Tex Scale (m/pix)",
	"3D Object Tex Stretch (%)", "LOD Separation Dist (m)", "Terrain Disappear Dist (m)", "Foliage Disappear Dist (m)",
	"Object Box Dist (m)", "Object Disappear Dist (m)", "Label Disappear Dist (m)"};

char *SceneExporter::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (SceneExporterCritterNames[Ct]);
	} // for
return ("");

} // SceneExporter::GetCritterName

/*===========================================================================*/

void SceneExporter::Edit(void)
{

if (GlobalApp->GUIWins->EXP)
	{
	delete GlobalApp->GUIWins->EXP;
	}
GlobalApp->GUIWins->EXP = new ExporterEditGUI(GlobalApp->AppEffects, GlobalApp->MainProj, GlobalApp->AppDB, GlobalApp->AppImages, this);
if (GlobalApp->GUIWins->EXP)
	{
	if (! GlobalApp->GUIWins->EXP->ConstructError)
		GlobalApp->GUIWins->EXP->Open(GlobalApp->MainProj);
	else
		{
		delete GlobalApp->GUIWins->EXP;
		GlobalApp->GUIWins->EXP = NULL;
		} // else
	} // if

} // SceneExporter::Edit

/*===========================================================================*/

int SceneExporter::RemoveRAHost(RasterAnimHost *RemoveMe)
{
EffectList *CurScenario = Scenarios, *PrevScenario = NULL;
EffectList *CurCamera = Cameras, *PrevCamera = NULL;
EffectList *CurLight = Lights, *PrevLight = NULL;
EffectList *CurHaze = Haze, *PrevHaze = NULL;
NotifyTag Changes[2];
int Removed = 0;

if (Coords == (CoordSys *)RemoveMe)
	{
	Coords = NULL;
	Removed = 1;
	} // if
#ifdef WCS_RENDER_SCENARIOS
if (! Removed)
	{
	while (CurScenario)
		{
		if (CurScenario->Me == (GeneralEffect *)RemoveMe)
			{
			if (PrevScenario)
				PrevScenario->Next = CurScenario->Next;
			else
				Scenarios = CurScenario->Next;
			delete CurScenario;
			Removed = 1;
			break;
			} // if
		PrevScenario = CurScenario;
		CurScenario = CurScenario->Next;
		} // while
	} // if
#endif // WCS_RENDER_SCENARIOS

if (! Removed)
	{
	while (CurCamera)
		{
		if (CurCamera->Me == (GeneralEffect *)RemoveMe)
			{
			if (PrevCamera)
				PrevCamera->Next = CurCamera->Next;
			else
				Cameras = CurCamera->Next;
			delete CurCamera;
			Removed = 1;
			break;
			} // if
		PrevCamera = CurCamera;
		CurCamera = CurCamera->Next;
		} // while
	} // if

if (! Removed)
	{
	while (CurLight)
		{
		if (CurLight->Me == (GeneralEffect *)RemoveMe)
			{
			if (PrevLight)
				PrevLight->Next = CurLight->Next;
			else
				Lights = CurLight->Next;
			delete CurLight;
			Removed = 1;
			break;
			} // if
		PrevLight = CurLight;
		CurLight = CurLight->Next;
		} // while
	} // if

if (! Removed)
	{
	while (CurHaze)
		{
		if (CurHaze->Me == (GeneralEffect *)RemoveMe)
			{
			if (PrevHaze)
				PrevHaze->Next = CurHaze->Next;
			else
				Haze = CurHaze->Next;
			delete CurHaze;
			Removed = 1;
			break;
			} // if
		PrevHaze = CurHaze;
		CurHaze = CurHaze->Next;
		} // while
	} // if

if (! Removed)
	{
	if (FormatExtension && FormatExtension->GetSupportsQueryAction())
		{
		Removed = ((SXExtensionActionable *)FormatExtension)->RemoveRAHost(RemoveMe, this);
		} // if
	} // if

if (Removed)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

return (Removed);

} // SceneExporter::RemoveRAHost

/*===========================================================================*/

int SceneExporter::SetCoords(CoordSys *NewCoords)
{
NotifyTag Changes[2];

Coords = NewCoords;

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (1);

} // SceneExporter::SetCoords

/*===========================================================================*/

EffectList *SceneExporter::AddScenario(GeneralEffect *AddMe)
{
#ifdef WCS_RENDER_SCENARIOS
EffectList **CurScenario = &Scenarios;
NotifyTag Changes[2];

if (AddMe)
	{
	while (*CurScenario)
		{
		if ((*CurScenario)->Me == AddMe)
			return (NULL);
		CurScenario = &(*CurScenario)->Next;
		} // while
	if (*CurScenario = new EffectList())
		{
		(*CurScenario)->Me = AddMe;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	return (*CurScenario);
	} // if
#endif // WCS_RENDER_SCENARIOS

return (NULL);

} // SceneExporter::AddScenario

/*===========================================================================*/

EffectList *SceneExporter::AddCamera(GeneralEffect *AddMe, int SendNotify)
{
EffectList **CurCamera = &Cameras;
NotifyTag Changes[2];

if (AddMe)
	{
	while (*CurCamera)
		{
		if ((*CurCamera)->Me == AddMe)
			return (NULL);
		CurCamera = &(*CurCamera)->Next;
		} // while
	if (*CurCamera = new EffectList())
		{
		(*CurCamera)->Me = AddMe;
		if (SendNotify)
			{
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	return (*CurCamera);
	} // if

return (NULL);

} // SceneExporter::AddCamera

/*===========================================================================*/

EffectList *SceneExporter::AddLight(GeneralEffect *AddMe, int SendNotify)
{
EffectList **CurLight = &Lights;
NotifyTag Changes[2];

if (AddMe)
	{
	while (*CurLight)
		{
		if ((*CurLight)->Me == AddMe)
			return (NULL);
		CurLight = &(*CurLight)->Next;
		} // while
	if (*CurLight = new EffectList())
		{
		(*CurLight)->Me = AddMe;
		if (SendNotify)
			{
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	return (*CurLight);
	} // if

return (NULL);

} // SceneExporter::AddLight

/*===========================================================================*/

EffectList *SceneExporter::AddHaze(GeneralEffect *AddMe, int SendNotify)
{
EffectList **CurHaze = &Haze;
NotifyTag Changes[2];

if (AddMe)
	{
	while (*CurHaze)
		{
		if ((*CurHaze)->Me == AddMe)
			return (NULL);
		CurHaze = &(*CurHaze)->Next;
		} // while
	if (*CurHaze = new EffectList())
		{
		(*CurHaze)->Me = AddMe;
		if (SendNotify)
			{
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	return (*CurHaze);
	} // if

return (NULL);

} // SceneExporter::AddHaze

/*===========================================================================*/

RasterAnimHostList *SceneExporter::AddObjectToEphemeralList(RasterAnimHost *AddMe)
{
RasterAnimHostList **CurObject = &EphemeralObjects;

if (AddMe)
	{
	while (*CurObject)
		{
		if ((*CurObject)->Me == AddMe)
			return (NULL);
		CurObject = &(*CurObject)->Next;
		} // while
	if (*CurObject = new RasterAnimHostList())
		{
		(*CurObject)->Me = AddMe;
		} // if
	return (*CurObject);
	} // if

return (NULL);

} // SceneExporter::AddObjectToEphemeralList

/*===========================================================================*/

int SceneExporter::FindInEphemeralList(RasterAnimHost *FindMe)
{
RasterAnimHostList *CurObject = EphemeralObjects;

while (CurObject)
	{
	if (CurObject->Me == FindMe)
		return (1);
	CurObject = CurObject->Next;
	} // while

return (0);

} // SceneExporter::FindInEphemeralList

/*===========================================================================*/

EffectList *SceneExporter::AddExportItem(GeneralEffect *AddMe, int SendNotify)
{

if (AddMe->EffectType == WCS_EFFECTSSUBCLASS_CAMERA)
	return (AddCamera(AddMe, SendNotify));
if (AddMe->EffectType == WCS_EFFECTSSUBCLASS_LIGHT)
	return (AddLight(AddMe, SendNotify));
if (AddMe->EffectType == WCS_EFFECTSSUBCLASS_ATMOSPHERE)
	return (AddHaze(AddMe, SendNotify));

return (NULL);

} // SceneExporter::AddExportItem

/*===========================================================================*/

void SceneExporter::DeleteExportItems(long EffectClass)
{
EffectList *NextEffect;

if (EffectClass == WCS_EFFECTSSUBCLASS_CAMERA)
	{
	while (Cameras)
		{
		NextEffect = Cameras->Next;
		delete Cameras;
		Cameras = NextEffect;
		} // while
	} // if
else if (EffectClass == WCS_EFFECTSSUBCLASS_LIGHT)
	{
	while (Lights)
		{
		NextEffect = Lights->Next;
		delete Lights;
		Lights = NextEffect;
		} // while
	} // if
else if (EffectClass == WCS_EFFECTSSUBCLASS_ATMOSPHERE)
	{
	while (Haze)
		{
		NextEffect = Haze->Next;
		delete Haze;
		Haze = NextEffect;
		} // while
	} // if

} // SceneExporter::AddExportItem

/*===========================================================================*/

void SceneExporter::ValidateTileRowsCols(long &OutRows, long &OutCols, long TilesY, long TilesX, long Overlap,
	long &TileRows, long &TileCols, int ResOption)
{
long CheckRows, CheckCols, Res;

if (Overlap)
	{
	TileRows = (OutRows + (TilesY - 1) * Overlap) / TilesY;
	CheckRows = TileRows * TilesY - (TilesY - 1) * Overlap;
	TileCols = (OutCols + (TilesX - 1) * Overlap) / TilesX;
	CheckCols = TileCols * TilesX - (TilesX - 1) * Overlap;
	} // if
else
	{
	TileRows = OutRows / TilesY;
	CheckRows = TileRows * TilesY;
	TileCols = OutCols / TilesX;
	CheckCols = TileCols * TilesX;
	} // else

if (CheckRows != OutRows)
	{
	// what rule for size
	if (ResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE)
		OutRows = CheckRows;
	else
		{
		if (ResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1)
			{
			CheckRows --;
			} // if

		// find next highest res for both X and Y
		for (Res = 2; Res <= 262144; Res *= 2)
			{
			if (Res >= CheckRows || Res == 262144)
				{
				CheckRows = Res;
				break;
				} // for
			} // for

		if (ResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1)
			{
			CheckRows ++;
			} // if

		OutRows = CheckRows;
		} // else
	} // if not divisible correctly

if (CheckCols != OutCols)
	{
	// what rule for size
	if (ResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_ANYSIZE)
		OutCols = CheckCols;
	else
		{
		if (ResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1)
			{
			CheckCols --;
			} // if

		// find next highest res for both X and Y
		for (Res = 2; Res <= 262144; Res *= 2)
			{
			if (Res >= CheckCols || Res == 262144)
				{
				CheckCols = Res;
				break;
				} // for
			} // for

		if (ResOption == WCS_EFFECTS_SCENEEXPORTER_RESOPTION_POWEROF2PLUS1)
			{
			CheckCols ++;
			} // if

		OutCols = CheckCols;
		} // else
	} // if not divisible correctly

} // SceneExporter::ValidateTileRowsCols

/*===========================================================================*/

void SceneExporter::DeleteObjectEphemeralList(void)
{
RasterAnimHostList *CurEffect;
RasterAnimHostProperties Prop;
char FullPath[512];

while (EphemeralObjects)
	{
	CurEffect = EphemeralObjects;
	EphemeralObjects = EphemeralObjects->Next;
	if (CurEffect->Me)
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
		CurEffect->Me->GetRAHostProperties(&Prop);
		if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_RASTER)
			{
			if (((Raster *)CurEffect->Me)->PAF.GetPathAndName(FullPath))
				{
				//PROJ_remove(FullPath);	// removing file not a good idea, might be needed as a texture map
				} // if
			GlobalApp->AppImages->RemoveRaster((Raster *)CurEffect->Me, GlobalApp->AppEffects);
			} // if
		else if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_OBJECT3D)
			{
			Object3DEffect *Temp3DO;
			Temp3DO = (Object3DEffect *)CurEffect->Me;
			Temp3DO->ActionList = NULL; // When in the Ephemeral list, this points to shared data that shouldn't be deleted.
			delete CurEffect->Me;
			} // if
		else
			delete CurEffect->Me;
		} // if
	delete CurEffect;
	} // if

} // SceneExporter::DeleteObjectEphemeralList

/*===========================================================================*/

void SceneExporter::DeleteObjectInstanceList(void)
{
Object3DInstance *CurInstance;

while (ObjectInstanceList)
	{
	CurInstance = ObjectInstanceList;
	ObjectInstanceList = ObjectInstanceList->Next;
	delete CurInstance;
	} // if

} // SceneExporter::DeleteObjectInstanceList

/*===========================================================================*/

void SceneExporter::DeleteVectorInstanceList(void)
{

if (VecInstanceList)
	delete [] VecInstanceList;
VecInstanceList = NULL;
NumVecInstances = 0;

} // SceneExporter::DeleteVectorInstanceList

/*===========================================================================*/

int SceneExporter::InitToExport(void)
{
char BaseName[256], FormatStr[256], FullPath[512];

CalcFullResolutions();
ClearRBounds();
RenderSizesSet = 0;
FrameRendered = 0;
TimeRendered = 0.0;
RowsRendered = 0;
ColsRendered = 0;
Unique3DObjectInstances = 0;
LabelInstance = 0;
TransparentPixelsExist = 0;
MinRenderedElevation = FLT_MAX;
MaxRenderedElevation = -FLT_MAX;

strcpy(BaseName, OutPath.GetName());
strcat(BaseName, "_Fol");
sprintf(FormatStr, "%s.dat", BaseName);
strmfp(FullPath, OutPath.GetPath(), FormatStr);
PROJ_remove(FullPath);
sprintf(FormatStr, "%sFoliageList%d.dat", BaseName, 0);
strmfp(FullPath, OutPath.GetPath(), FormatStr);
PROJ_remove(FullPath);

if (FormatExtension)
	{
	if (! FormatExtension->InitToExport(OutPath.GetPath()))
		return (0);
	} // if

if (! stricmp(ExportTarget, "NatureView"))
	{
	strcpy(TextureImageFormat, "PNG");
	if (! stricmp(ImageFormat, "ERMapper ECW"))
		{
		strcpy(DEMFormatBackup, DEMFormat);
		strcpy(DEMFormat, "ERMapper ECW DEM");
		} // if
	else if (! stricmp(ImageFormat, "JPEG 2000"))
		{
		strcpy(DEMFormatBackup, DEMFormat);
		strcpy(DEMFormat, "JPEG 2000 DEM");
		} // if
	else if (! stricmp(ImageFormat, "Lossless JPEG 2000"))
		{
		strcpy(DEMFormatBackup, DEMFormat);
		strcpy(DEMFormat, "Lossless JPEG 2000 DEM");
		} // if
	} // if
else if (! stricmp(ExportTarget, "GoogleEarth"))
	{
	strcpy(TextureImageFormat, ImageFormat);
	if (! ExportTerrain)
		{
		OrigDEMResX = DEMResX;
		OrigDEMResY = DEMResY;
		DEMResX = 2;	// hack down so only texture effects export time
		DEMResY = 2;
		} // if
	} // if Google Earth
else if (! stricmp(ExportTarget, "WorldWind"))
	{
	strcpy(TextureImageFormat, "PNG");
	if (ExportTerrain)
		{
		double degrees, deltaNS, deltaWE, minDelta;
		double tmpN, tmpS, tmpW, tmpE;
		long startLevel = 0;

		// save values for restoration later
		OrigDEMResX = DEMResX;
		OrigDEMResY = DEMResY;
		origN = GeoReg.AnimPar[0].CurValue;
		origS = GeoReg.AnimPar[1].CurValue;
		origW = GeoReg.AnimPar[2].CurValue;
		origE = GeoReg.AnimPar[3].CurValue;

		// change terrain res to what is needed for highest LOD
		DEMResX *= 1 << (LODLevels - 1);
		DEMResY *= 1 << (LODLevels - 1);

		// figure out an appropriate quad tree base size
		deltaWE = GeoReg.AnimPar[2].CurValue - GeoReg.AnimPar[3].CurValue;
		deltaNS = GeoReg.AnimPar[0].CurValue - GeoReg.AnimPar[1].CurValue;
		minDelta = min(deltaWE, deltaNS);

		degrees = 1.0;	// our level 0 tile size

		while (minDelta < degrees)
			{
			degrees *= 0.5;
			startLevel++;
			} // while

		// snap bounds to nearest grid value outward
		tmpN = fmod(GeoReg.AnimPar[0].CurValue, degrees);
		if (tmpN == 0.0)
			tmpN = degrees;	// will leave original N unchanged
		tmpN = GeoReg.AnimPar[0].CurValue + degrees - tmpN;
		GeoReg.AnimPar[0].SetCurValue(tmpN);
		tmpS = fmod(GeoReg.AnimPar[1].CurValue, degrees);
		tmpS = GeoReg.AnimPar[1].CurValue - tmpS;
		GeoReg.AnimPar[1].SetCurValue(tmpS);
		tmpW = fmod(GeoReg.AnimPar[2].CurValue, degrees);
		if (tmpW == 0.0)
			tmpW = degrees;	// will leave original W unchanged
		tmpW = GeoReg.AnimPar[2].CurValue + degrees - tmpW;
		GeoReg.AnimPar[2].SetCurValue(tmpW);
		tmpE = fmod(GeoReg.AnimPar[3].CurValue, degrees);
		tmpE = GeoReg.AnimPar[3].CurValue - tmpE;
		GeoReg.AnimPar[3].SetCurValue(tmpE);

		WW_level = startLevel;
		printf("foo");
		} // if
	} // if World Wind
else
	{
	strcpy(TextureImageFormat, ImageFormat);
	} // else

return (1);

} // SceneExporter::InitToExport

/*===========================================================================*/

void SceneExporter::CleanupFromExport(void)
{

DeleteObjectInstanceList();
DeleteObjectEphemeralList();
DeleteVectorInstanceList();
if (FormatExtension)
	FormatExtension->CleanupFromExport(this);
RestorePath();
if (! stricmp(ExportTarget, "NatureView"))
	{
	if (! stricmp(ImageFormat, "ERMapper ECW"))
		{
		strcpy(DEMFormat, DEMFormatBackup);
		} // if
	if (! stricmp(ImageFormat, "JPEG 2000"))
		{
		strcpy(DEMFormat, DEMFormatBackup);
		} // if
	} // if
if (! stricmp(ExportTarget, "GoogleEarth"))
	{
	if (! ExportTerrain)
		{
		DEMResX = OrigDEMResX;
		DEMResY = OrigDEMResY;
		} // if
	} // if Google Earth
if (! stricmp(ExportTarget, "WorldWind"))
	{
	if (ExportTerrain)
		{
		// restore the original settings
		DEMResX = OrigDEMResX;
		DEMResY = OrigDEMResY;
		GeoReg.AnimPar[0].SetCurValue(origN);
		GeoReg.AnimPar[1].SetCurValue(origS);
		GeoReg.AnimPar[2].SetCurValue(origW);
		GeoReg.AnimPar[3].SetCurValue(origE);
		} // if
	} // if WorldWind

} // SceneExporter::CleanupFromExport

/*===========================================================================*/

void SceneExporter::CloseQueryFiles(void)
{

if (FormatExtension)
	{
	FormatExtension->CloseQueryFiles();
	} // if

} // SceneExporter::CloseQueryFiles

/*===========================================================================*/

void SceneExporter::BackupPath(void)
{

strcpy(PathBackup, OutPath.GetPath());

} // SceneExporter::BackupPath

/*===========================================================================*/

void SceneExporter::RestorePath(void)
{

if (PathBackup[0])
	OutPath.SetPath(PathBackup);
PathBackup[0] = 0;

} // SceneExporter::RestorePath

/*===========================================================================*/

void SceneExporter::SetupRBounds(Database *DBHost, EffectsLib *EffectsHost)
{
double RenderWidth, RenderHeight, PlanetRad, PixResX, PixResY;
PlanetOpt *DefPlanetOpt;
float DBMaxEl, DBMinEl;
long ImageWidth, ImageHeight;

DBHost->GetDEMElevRange(DBMaxEl, DBMinEl);
if (DefPlanetOpt = (PlanetOpt *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL))
	{
	DBMaxEl = (float)CalcExag((double)DBMaxEl, DefPlanetOpt);
	DBMinEl = (float)CalcExag((double)DBMinEl, DefPlanetOpt);
	} // if
PlanetRad = EffectsHost->GetPlanetRadius();
GeoReg.GetMetricWidthHeight(Coords, PlanetRad, RenderWidth, RenderHeight);
// add a cell width and height if bounds are centers
if (ExportTexture)
	{
	ImageWidth = TexResX;
	ImageHeight = TexResY;
	if (ExportTerrain)
		{
		ImageWidth = max(ImageWidth, DEMResX);
		ImageHeight = max(ImageHeight, DEMResY);
		} // if
	} // if
else
	{
	ImageWidth = DEMResX;
	ImageHeight = DEMResY;
	} // else
RBounds.SetCoords(Coords ? Coords: EffectsHost->FetchDefaultCoordSys());
if (BoundsType == WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLCENTERS)
	{
	PixResX = RenderWidth / (ImageWidth - 1);
	RenderWidth += PixResX;
	PixResY = RenderHeight / (ImageHeight - 1);
	RenderHeight += PixResY;
	RBounds.SetOutsideBoundsFromCenters(&GeoReg, ImageHeight, ImageWidth);
	} // if
else
	{
	RBounds.SetOutsideBounds(&GeoReg);
	} // else
RBounds.DeriveCoords(ImageHeight, ImageWidth);
SetupExportReferenceData(PlanetRad, (double)DBMinEl, (double)DBMaxEl);

} // SceneExporter::SetupRBounds

/*===========================================================================*/

void SceneExporter::SetupExportReferenceData(double PlanetRad, double MinElev, double MaxElev)
{
double TempRefLat, TempRefLon;
VertexDEM Vert;

RBounds.FetchRegionCenterWCS(TempRefLat, TempRefLon);
ExportRefData.RefElev = MinElev;
ExportRefData.MaxElev = MaxElev;
if (RBounds.IsGeographic)
	{
	// TempRefLon is geographic in WCS longitude sign convention
	// data delivered above is not in default CS but in export CS.
	ExportRefData.ExportLatScale = ExportRefData.WCSLatScale = LatScale(PlanetRad);
	ExportRefData.ExportLonScale = ExportRefData.WCSLonScale = LonScale(PlanetRad, TempRefLat);
	ExportRefData.ExportRefLat = TempRefLat;
	ExportRefData.ExportRefLon = -TempRefLon;
	if (RBounds.MyCoords)
		{
		Vert.xyz[1] = TempRefLat;
		Vert.xyz[0] = TempRefLon;
		RBounds.MyCoords->ProjToDefDeg(&Vert);
		ExportRefData.WCSRefLat = Vert.Lat;
		ExportRefData.WCSRefLon = Vert.Lon;
		} // if
	} // if
else
	{
	// TempRefLat and TempRefLon are metric in GIS convention
	ExportRefData.ExportRefLat = TempRefLat;
	ExportRefData.ExportRefLon = TempRefLon;
	ExportRefData.ExportLatScale = ExportRefData.ExportLonScale = 1.0;
	Vert.xyz[1] = TempRefLat;
	Vert.xyz[0] = TempRefLon;
	// convert to default degrees in WCS longitude sign convention
	RBounds.MyCoords->ProjToDefDeg(&Vert);
	ExportRefData.WCSRefLat = Vert.Lat;
	ExportRefData.WCSRefLon = Vert.Lon;
	ExportRefData.WCSLatScale = LatScale(PlanetRad);
	ExportRefData.WCSLonScale = LonScale(PlanetRad, ExportRefData.WCSRefLat);
	} // else
ExportRefData.ElevScale = 1.0;

} // SceneExporter::SetupExportReferenceData

/*===========================================================================*/

double SceneExporter::CalcDefaultSpeed(void)
{
double XSpace, ZSpace;

// this sets the DEMResX/Y
CalcFullResolutions();

// find size of export region
if (BoundsType == WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLCENTERS)
	{
	RBounds.SetOutsideBoundsFromCenters(&GeoReg, DEMResY, DEMResX);
	} // if
else
	{
	RBounds.SetOutsideBounds(&GeoReg);
	} // else
RBounds.DeriveCoords(DEMResY, DEMResX);
if (RBounds.IsGeographic)
	{
	XSpace = RBounds.CellSizeX * EARTHLATSCALE_METERS;	// convert geographic to meters
	ZSpace = RBounds.CellSizeY * EARTHLATSCALE_METERS;
	} // if
else
	{
	XSpace = RBounds.CellSizeX;
	ZSpace = RBounds.CellSizeY;
	} // else

return (min(XSpace, ZSpace) * 100.0);

} // SceneExporter::CalcDefaultSpeed

/*===========================================================================*/

SXQueryAction *SceneExporter::AddQueryAction(void)
{

#ifdef WCS_BUILD_SX2
if (FormatExtension)
	{
	if (FormatExtension->GetSupportsQueryAction())
		{
		return (((SXExtensionActionable *)FormatExtension)->AddQueryAction(NULL));
		} // if
	} // if
#endif // WCS_BUILD_SX2

return (NULL);

} // SceneExporter::AddQueryAction

/*===========================================================================*/

SXQueryAction *SceneExporter::RemoveQueryAction(SXQueryAction *RemoveMe)
{

if (FormatExtension)
	{
	if (FormatExtension->GetSupportsQueryAction())
		{
		return (((SXExtensionActionable *)FormatExtension)->RemoveQueryAction(RemoveMe));
		} // if
	} // if

return (NULL);

} // SceneExporter::RemoveQueryAction

/*===========================================================================*/

SXQueryAction *SceneExporter::VerifyActiveAction(SXQueryAction *VerifyMe)
{

if (FormatExtension)
	{
	if (FormatExtension->GetSupportsQueryAction())
		{
		return (((SXExtensionActionable *)FormatExtension)->VerifyActiveAction(VerifyMe));
		} // if
	} // if

return (NULL);

} // SceneExporter::VerifyActiveAction

/*===========================================================================*/

bool SceneExporter::ApproveActionAvailable(int TestType)
{

if (FormatExtension)
	{
	if (FormatExtension->GetSupportsQueryAction())
		{
		return (FormatExtension->ApproveActionAvailable(TestType));
		} // if
	} // if

return (false);

} // SceneExporter::ApproveActionAvailable

/*===========================================================================*/

bool SceneExporter::ApproveActionItemAvailable(int TestItem)
{

if (FormatExtension)
	{
	if (FormatExtension->GetSupportsQueryAction())
		{
		return (FormatExtension->ApproveActionItemAvailable(TestItem));
		} // if
	} // if

return (false);

} // SceneExporter::ApproveActionItemAvailable

/*===========================================================================*/

void SceneExporter::SetActiveAction(SXQueryAction *NewActive)
{

if (FormatExtension)
	{
	if (FormatExtension->GetSupportsQueryAction())
		{
		((SXExtensionActionable *)FormatExtension)->SetActiveAction(NewActive);
		} // if
	} // if

} // SceneExporter::SetActiveAction

/*===========================================================================*/

SXQueryAction *SceneExporter::GetActionList(void)
{

if (FormatExtension)
	{
	if (FormatExtension->GetSupportsQueryAction())
		{
		return (((SXExtensionActionable *)FormatExtension)->GetActionList());
		} // if
	} // if

return (NULL);

} // SceneExporter::GetActionList

/*===========================================================================*/

long SceneExporter::InitImageIDs(long &ImageID)
{
EffectList *CurScenario = Scenarios;
long NumImages = 0;

if (Coords)
	NumImages += Coords->InitImageIDs(ImageID);
#ifdef WCS_RENDER_SCENARIOS
while (CurScenario)
	{
	if (CurScenario->Me)
		NumImages += CurScenario->Me->InitImageIDs(ImageID);
	CurScenario = CurScenario->Next;
	} // while
NumImages += GeneralEffect::InitImageIDs(ImageID);
#endif // WCS_RENDER_SCENARIOS

return (NumImages);

} // SceneExporter::InitImageIDs

/*===========================================================================*/

int SceneExporter::BuildFileComponentsList(EffectList **ScenarioList, EffectList **CoordSystems)
{
#if (defined WCS_RENDER_SCENARIOS || defined WCS_BUILD_VNS)
EffectList **ListPtr;
#endif // WCS_RENDER_SCENARIOS || WCS_BUILD_VNS
#ifdef WCS_RENDER_SCENARIOS
EffectList *CurScenario = Scenarios;
#endif // WCS_RENDER_SCENARIOS

#ifdef WCS_BUILD_VNS
if (Coords)
	{
	ListPtr = CoordSystems;
	while (*ListPtr)
		{
		if ((*ListPtr)->Me == Coords)
			break;
		ListPtr = &(*ListPtr)->Next;
		} // if
	if (! (*ListPtr))
		{
		if (*ListPtr = new EffectList())
			(*ListPtr)->Me = Coords;
		else
			return (0);
		} // if
	} // if
#endif // WCS_BUILD_VNS

#ifdef WCS_RENDER_SCENARIOS
while (CurScenario)
	{
	if (CurScenario->Me)
		{
		ListPtr = ScenarioList;
		while (*ListPtr)
			{
			if ((*ListPtr)->Me == CurScenario->Me)
				break;
			ListPtr = &(*ListPtr)->Next;
			} // if
		if (! (*ListPtr))
			{
			if (*ListPtr = new EffectList())
				(*ListPtr)->Me = CurScenario->Me;
			else
				return (0);
			} // if
		} // if
	CurScenario = CurScenario->Next;
	} // while
#endif // WCS_RENDER_SCENARIOS

return (1);

} // SceneExporter::BuildFileComponentsList

/*===========================================================================*/

char SceneExporter::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
#ifdef WCS_BUILD_VNS
if (DropType == WCS_EFFECTSSUBCLASS_COORDSYS)
	return (1);
#endif // WCS_BUILD_VNS
#ifdef WCS_RENDER_SCENARIOS
if (DropType == WCS_EFFECTSSUBCLASS_SCENARIO)
	return (1);
#endif

return (0);

} // SceneExporter::GetRAHostDropOK

/*===========================================================================*/

int SceneExporter::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
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
	if (this != (SceneExporter *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (SceneExporter *)DropSource->DropSource);
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
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_COORDSYS)
	{
	Success = SetCoords((CoordSys *)DropSource->DropSource);
	} // else if
#endif // WCS_BUILD_VNS
#ifdef WCS_RENDER_SCENARIOS
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_SCENARIO)
	{
	Success = -1;
	sprintf(QueryStr, "Add %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		if (AddScenario((GeneralEffect *)DropSource->DropSource))
			{
			Success = 1;
			} // if
		} // if
	} // else if
#endif // WCS_RENDER_SCENARIOS
#ifdef WCS_BUILD_SX2
else if (FormatExtension && FormatExtension->AcceptDragDrop(DropSource->TypeNumber))
	{
	Success = FormatExtension->ProcessRAHostDragDrop(DropSource, true, this);
	} // else if
#endif // WCS_BUILD_SX2
else
	{
	Success = GeneralEffect::ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // SceneExporter::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long SceneExporter::GetRAFlags(unsigned long Mask)
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
if (Mask & WCS_RAHOST_FLAGBIT_CHILDREN)
	{
	Flags |= WCS_RAHOST_FLAGBIT_CHILDREN;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_SCENEEXP | WCS_RAHOST_FLAGBIT_DRAGGABLE | 
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // SceneExporter::GetRAFlags

/*===========================================================================*/

RasterAnimHost *SceneExporter::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
EffectList *CurEffect;
char Found = 0, Ct;

if (! Current)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Found)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for
#ifdef WCS_RENDER_SCENARIOS
CurEffect = Scenarios;
while (CurEffect)
	{
	if (Found)
		return (CurEffect->Me);
	if (Current == CurEffect->Me)
		Found = 1;
	CurEffect = CurEffect->Next;
	} // while
#endif // WCS_RENDER_SCENARIOS
CurEffect = Cameras;
while (CurEffect)
	{
	if (Found)
		return (CurEffect->Me);
	if (Current == CurEffect->Me)
		Found = 1;
	CurEffect = CurEffect->Next;
	} // while
CurEffect = Lights;
while (CurEffect)
	{
	if (Found)
		return (CurEffect->Me);
	if (Current == CurEffect->Me)
		Found = 1;
	CurEffect = CurEffect->Next;
	} // while
CurEffect = Haze;
while (CurEffect)
	{
	if (Found)
		return (CurEffect->Me);
	if (Current == CurEffect->Me)
		Found = 1;
	CurEffect = CurEffect->Next;
	} // while
#ifdef WCS_BUILD_VNS
if (Found && Coords)
	return (Coords);
if (Current == Coords)
	Found = 1;
#endif // WCS_BUILD_VNS
#ifdef WCS_BUILD_SX2
if (FormatExtension
	&& FormatExtension->GetSupportsQueryAction() 
	&& ((SXExtensionActionable *)FormatExtension)->ActionList)
	{
	SXQueryAction *CurAction;
	RasterAnimHostList *CurItem;
	int DoneChecking, Duplicate;

	if (Found)
		{
		SXActionIterator = ((SXExtensionActionable *)FormatExtension)->ActionList;
		SXActionItemIterator = SXActionIterator->Items;
		} // if
	else
		{
		// pick up where left off last time
		if (SXActionItemIterator)
			SXActionItemIterator = SXActionItemIterator->Next;
		} // else
	while (SXActionIterator && ! SXActionItemIterator)
		{
		SXActionIterator = SXActionIterator->Next;
		if (SXActionIterator)
			SXActionItemIterator = SXActionIterator->Items;
		} // while
	while (SXActionIterator && SXActionItemIterator)
		{
		// find out if the item at SXActionItemIterator has been used before
		DoneChecking = 0;
		Duplicate = 0;
		for (CurAction = ((SXExtensionActionable *)FormatExtension)->ActionList; CurAction && ! DoneChecking && ! Duplicate; CurAction = CurAction->Next)
			{
			CurItem = CurAction->Items;
			while (CurItem)
				{
				if (CurItem == SXActionItemIterator)
					{
					DoneChecking = 1;
					break;
					} // if
				if (CurItem->Me == SXActionItemIterator->Me)
					{
					Duplicate = 1;
					break;
					} // if
				CurItem = CurItem->Next;
				} // while
			} // for
		if (! Duplicate)
			{
			return (SXActionItemIterator->Me);
			} // if
		SXActionItemIterator = SXActionItemIterator->Next;
		while (SXActionIterator && ! SXActionItemIterator)
			{
			SXActionIterator = SXActionIterator->Next;
			if (SXActionIterator)
				SXActionItemIterator = SXActionIterator->Items;
			} // while
		} // while
	} // if

/* proven not to work, possibility exists of endless loop
if (FormatExtension)
	{
	if (FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_NVE)
		{
		for (SXQueryAction *CurAction = ((SXExtensionNVE *)FormatExtension)->ActionList; CurAction; CurAction = CurAction->Next)
			{
			RasterAnimHostList *CurItem = CurAction->Items;
			while (CurItem)
				{
				if (Found)
					{
					int Duplicate = 0;
					// is the instance unique
					for (SXQueryAction *SearchAction = CurAction; SearchAction && ! Duplicate; SearchAction = SearchAction->Next)
						{
						RasterAnimHostList *SearchItem = CurItem->Next;
						while (SearchItem)
							{
							if (Current == SearchItem->Me)
								{
								Duplicate = 1;
								break;
								} // if item found again
							SearchItem = SearchItem->Next;
							} // while
						} // for
					if (! Duplicate)
						return (CurItem->Me);
					} // if Found
				if (Current == CurItem->Me)
					Found = 1;
				CurItem = CurItem->Next;
				} // while
			} // for
		} // if
	} // if
*/
#endif // WCS_BUILD_SX2

return (NULL);

} // SceneExporter::GetRAHostChild

/*===========================================================================*/

void SceneExporter::SetTarget(char *NewTarget)
{
int SetFormatExt = 0;

if (stricmp(NewTarget, ExportTarget))
	SetFormatExt = 1;
strncpy(ExportTarget, NewTarget, 64);
ExportTarget[63] = 0;

if (SetFormatExt)
	AllocFormatExtension();

} // SceneExporter::SetTarget

/*===========================================================================*/

void SceneExporter::ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds)
{

GeoReg.ScaleToDEMBounds(OldBounds, CurBounds);

} // SceneExporter::ScaleToDEMBounds

/*===========================================================================*/

void SceneExporter::CalcFullResolutions(void)
{

DEMResX = DEMTilesX * OneDEMResX - DEMTileOverlap * (DEMTilesX - 1);
DEMResY = DEMTilesY * OneDEMResY - DEMTileOverlap * (DEMTilesY - 1);

TexResX = TexTilesX * OneTexResX - TexTileOverlap * (TexTilesX - 1);
TexResY = TexTilesY * OneTexResY - TexTileOverlap * (TexTilesY - 1);

} // SceneExporter::CalcFullResolutions

/*===========================================================================*/

void SceneExporter::ComputeRowsFromNumCells(long NumCells)
{

DEMResY = NumCells;
OneDEMResY = DEMResY + DEMTileOverlap * (DEMTilesY - 1) / DEMTilesY;

} // SceneExporter::ComputeRowsFromNumCells

/*===========================================================================*/

void SceneExporter::ComputeColsFromNumCells(long NumCells)
{

DEMResX = NumCells;
OneDEMResX = DEMResX + DEMTileOverlap * (DEMTilesX - 1) / DEMTilesX;

} // SceneExporter::ComputeColsFromNumCells

/*===========================================================================*/

int SceneExporter::MultipleObjectUVMappingsSupported(void)
{

if (! stricmp(ObjectFormat, "LWO2") || ! stricmp(ObjectFormat, "W3D") || ! stricmp(ObjectFormat, "W3O"))
	return (1);

return (0);

} // SceneExporter::MultipleObjectUVMappingsSupported

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int SceneExporter::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG ItemTag = 0, Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
SceneExporter *CurrentSceneExporter = NULL;
RenderScenario *CurrentScenario = NULL;
#ifdef WCS_BUILD_VNS
CoordSys *CurrentCoords = NULL;
#endif // WCS_BUILD_VNS
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
					#ifdef WCS_BUILD_VNS
					else if (! strnicmp(ReadBuf, "CoordSys", 8))
						{
						if (CurrentCoords = new CoordSys(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentCoords->Load(ffile, Size, ByteFlip);
							}
						} // if Camera
					#endif // WCS_BUILD_VNS
					#ifdef WCS_RENDER_SCENARIOS
					else if (! strnicmp(ReadBuf, "Scenario", 8))
						{
						if (CurrentScenario = new RenderScenario(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentScenario->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;
							}
						} // if Scenario
					#endif // WCS_RENDER_SCENARIOS
					else if (! strnicmp(ReadBuf, "Exporter", 8))
						{
						if (CurrentSceneExporter = new SceneExporter(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentSceneExporter->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if SceneExporter
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

if (Success == 1 && CurrentSceneExporter)
	{
	if (OldBoundsLoaded && GlobalApp->AppDB->FillDEMBounds(&CurBounds))
		{
		if (UserMessageYN("Load Scene Exporter", "Do you wish the loaded Scene Exporter's bounds\n to be scaled to current DEM bounds?"))
			{
			CurrentSceneExporter->ScaleToDEMBounds(&OldBounds, &CurBounds);
			} // if
		} // if
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentSceneExporter);
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

} // SceneExporter::LoadObject

/*===========================================================================*/

int SceneExporter::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
EffectList *CurEffect, *ScenarioList = NULL, *CoordsList = NULL;
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

if (BuildFileComponentsList(&ScenarioList, &CoordsList))
	{
	#ifdef WCS_BUILD_VNS
	CurEffect = CoordsList;
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

	#ifdef WCS_RENDER_SCENARIOS
	CurEffect = ScenarioList;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "Scenario");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((RenderScenario *)CurEffect->Me)->Save(ffile))
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
						} // if Scenario saved 
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
	#endif // WCS_RENDER_SCENARIOS

	while (CoordsList)
		{
		CurEffect = CoordsList;
		CoordsList = CoordsList->Next;
		delete CurEffect;
		} // while
	while (ScenarioList)
		{
		CurEffect = ScenarioList;
		ScenarioList = ScenarioList->Next;
		delete CurEffect;
		} // while
	} // if

// SceneExporter
strcpy(StrBuf, "Exporter");
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
			} // if SceneExporter saved 
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

} // SceneExporter::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::SceneExporter_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
SceneExporter *Current;

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
					default:
						break;
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new SceneExporter(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (SceneExporter *)FindDuplicateByName(Current->EffectType, Current))
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

} // EffectsLib::SceneExporter_Load()

/*===========================================================================*/

ULONG EffectsLib::SceneExporter_Save(FILE *ffile)
{
SceneExporter *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

Current = Exporter;
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
					} // if SceneExporter saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (SceneExporter *)Current->Next;
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

} // EffectsLib::SceneExporter_Save()

/*===========================================================================*/
