// EffectsLib.cpp
// Code for managing Effects library
// Built from scratch on 04/30/97 by Gary R. Huber
// Copyright 1997 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "AppMem.h"
#include "Project.h"
#include "Types.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Log.h"
#include "Raster.h"
#include "GraphData.h"
#include "Conservatory.h"
#include "Interactive.h"
#include "Texture.h"
#include "Random.h"
#include "requester.h"
#include "DragnDropListGUI.h"
#include "Toolbar.h"
#include "GalleryGUI.h"
#include "BrowseDataGUI.h"
#include "SceneViewGUI.h"
#include "EffectListGUI.h"
#include "Render.h"
#include "KeyScaleDeleteGUI.h"
#include "WCSVersion.h"
#include "Lists.h"
#include "EffectEval.h"
#include "VectorNode.h"
#include "VectorPolygon.h"
#include "FeatureConfig.h"
#include "DBEditGUI.h"
#include "ThematicMapEditGUI.h" // for Thematic Map popup menu units table

PRNGX EffectsLib::EffectsRand;
EffectJoeList *EffectsLib::EffectChain[1000];
RasterAnimHost *RasterAnimHost::ActiveRAHost;
RasterAnimHost *RasterAnimHost::BackupRAHost;
RasterAnimHost *RasterAnimHost::CopyOfRAHost;
int RasterAnimHost::ActiveLock;

char EffectsLib::LoadQueries = 1;

char *EffectsLib::DefaultExtensions[WCS_MAXIMPLEMENTED_EFFECTS] = 
	{
	"",		// WCS_EFFECTSSUBCLASS_GENERIC = 0,
	"",		// WCS_EFFECTSSUBCLASS_COMBO,			// obsolete
	"lak",	// WCS_EFFECTSSUBCLASS_LAKE,
	"eco",	// WCS_EFFECTSSUBCLASS_ECOSYSTEM,
	"",		// WCS_EFFECTSSUBCLASS_FOG,				// these probably won't be used
	"wve",	// WCS_EFFECTSSUBCLASS_WAVE,
	"cld",	// WCS_EFFECTSSUBCLASS_CLOUD,
	"",		// WCS_EFFECTSSUBCLASS_CLOUDSHADOW,		// these probably won't be used
	"env",	// WCS_EFFECTSSUBCLASS_ENVIRONMENT,
	"",		// WCS_EFFECTSSUBCLASS_TINT,				// these probably won't be used
	"cmp",	// WCS_EFFECTSSUBCLASS_CMAP,
	"",		// WCS_EFFECTSSUBCLASS_ILLUMINATION,	// obsolete
	"",		// WCS_EFFECTSSUBCLASS_LANDWAVE,			// these probably won't be used
	"ata",	// WCS_EFFECTSSUBCLASS_RASTERTA,
	"ter",	// WCS_EFFECTSSUBCLASS_TERRAFFECTOR,
	"",		// WCS_EFFECTSSUBCLASS_PROFILE,
	"",		// WCS_EFFECTSSUBCLASS_GRADIENTPROFILE,
	"",		// WCS_EFFECTSSUBCLASS_PATHFOLLOW,			// these probably won't be used
	"",		// WCS_EFFECTSSUBCLASS_PATHCONFORM,		// these probably won't be used
	"",		// WCS_EFFECTSSUBCLASS_MORPH,				// these probably won't be used
	"",		// WCS_EFFECTSSUBCLASS_ECOTYPE,			// these probably won't be used
	"",		// WCS_EFFECTSSUBCLASS_FOLIAGEGRP,			// these probably won't be used
	"fol",	// WCS_EFFECTSSUBCLASS_FOLIAGE,
	"w3d",	// WCS_EFFECTSSUBCLASS_OBJECT3D,
	"",		// WCS_EFFECTSSUBCLASS_OBJECTVEC,			// these probably won't be used
	"shd",	// WCS_EFFECTSSUBCLASS_SHADOW,
	"str",	// WCS_EFFECTSSUBCLASS_STREAM,
	"mat",	// WCS_EFFECTSSUBCLASS_MATERIAL,
	"",		// WCS_EFFECTSSUBCLASS_BACKGROUND,			// these probably won't be used
	"cel",	// WCS_EFFECTSSUBCLASS_CELESTIAL,
	"stf",	// WCS_EFFECTSSUBCLASS_STARFIELD,
	"plo",	// WCS_EFFECTSSUBCLASS_PLANETOPT,
	"tep",	// WCS_EFFECTSSUBCLASS_TERRAINPARAM,
	"gnd",	// WCS_EFFECTSSUBCLASS_GROUND,
	"sno",	// WCS_EFFECTSSUBCLASS_SNOW,
	"sky",	// WCS_EFFECTSSUBCLASS_SKY,
	"atm",	// WCS_EFFECTSSUBCLASS_ATMOSPHERE,
	"lgt",	// WCS_EFFECTSSUBCLASS_LIGHT,
	"cam",	// WCS_EFFECTSSUBCLASS_CAMERA,
	"rnj",	// WCS_EFFECTSSUBCLASS_RENDERJOB,
	"rno",	// WCS_EFFECTSSUBCLASS_RENDEROPT,
	"tgr",	// WCS_EFFECTSSUBCLASS_GRIDDER,
	"tgn",	// WCS_EFFECTSSUBCLASS_GENERATOR,
	"squ",	// WCS_EFFECTSSUBCLASS_SEARCHQUERY,
	"thm",	// WCS_EFFECTSSUBCLASS_THEMATICMAP,
	"cos",	// WCS_EFFECTSSUBCLASS_COORDSYS,
	"fnc",	// WCS_EFFECTSSUBCLASS_FENCE,
	"ppr",	// WCS_EFFECTSSUBCLASS_POSTPROC,
	"scn",	// WCS_EFFECTSSUBCLASS_SCENARIO,
	"mrg",	// WCS_EFFECTSSUBCLASS_DEMMERGER,
	"exp",	// WCS_EFFECTSSUBCLASS_EXPORTER,
	"lbl"	// WCS_EFFECTSSUBCLASS_LABEL,
	}; // EffectsLib::DefaultExtensions
// <<<>>> ADD_NEW_EFFECTS add to end of list a file extension for component files

char *EffectsLib::DefaultPaths[WCS_MAXIMPLEMENTED_EFFECTS] = 
	{
	"",			// WCS_EFFECTSSUBCLASS_GENERIC = 0,
	"",			// WCS_EFFECTSSUBCLASS_COMBO,			// obsolete
	"Water",	// WCS_EFFECTSSUBCLASS_LAKE,
	"LandCover",// WCS_EFFECTSSUBCLASS_ECOSYSTEM,
	"",			// WCS_EFFECTSSUBCLASS_FOG,				// these probably won't be used
	"Water",	// WCS_EFFECTSSUBCLASS_WAVE,
	"Sky",		// WCS_EFFECTSSUBCLASS_CLOUD,
	"",			// WCS_EFFECTSSUBCLASS_CLOUDSHADOW,		// these probably won't be used
	"LandCover",// WCS_EFFECTSSUBCLASS_ENVIRONMENT,
	"",			// WCS_EFFECTSSUBCLASS_TINT,				// these probably won't be used
	"LandCover",// WCS_EFFECTSSUBCLASS_CMAP,
	"",			// WCS_EFFECTSSUBCLASS_ILLUMINATION,	// obsolete
	"",			// WCS_EFFECTSSUBCLASS_LANDWAVE,			// these probably won't be used
	"Terrain",	// WCS_EFFECTSSUBCLASS_RASTERTA,
	"Terrain",	// WCS_EFFECTSSUBCLASS_TERRAFFECTOR,
	"",			// WCS_EFFECTSSUBCLASS_PROFILE,
	"",			// WCS_EFFECTSSUBCLASS_GRADIENTPROFILE,
	"",			// WCS_EFFECTSSUBCLASS_PATHFOLLOW,			// these probably won't be used
	"",			// WCS_EFFECTSSUBCLASS_PATHCONFORM,		// these probably won't be used
	"",			// WCS_EFFECTSSUBCLASS_MORPH,				// these probably won't be used
	"",			// WCS_EFFECTSSUBCLASS_ECOTYPE,			// these probably won't be used
	"",			// WCS_EFFECTSSUBCLASS_FOLIAGEGRP,			// these probably won't be used
	"LandCover",// WCS_EFFECTSSUBCLASS_FOLIAGE,
	"3DObject",	// WCS_EFFECTSSUBCLASS_OBJECT3D,
	"",			// WCS_EFFECTSSUBCLASS_OBJECTVEC,			// these probably won't be used
	"Light",	// WCS_EFFECTSSUBCLASS_SHADOW,
	"Water",	// WCS_EFFECTSSUBCLASS_STREAM,
	"3DObject",	// WCS_EFFECTSSUBCLASS_MATERIAL,
	"",			// WCS_EFFECTSSUBCLASS_BACKGROUND,			// these probably won't be used
	"Sky",		// WCS_EFFECTSSUBCLASS_CELESTIAL,
	"Sky",		// WCS_EFFECTSSUBCLASS_STARFIELD,
	"Terrain",	// WCS_EFFECTSSUBCLASS_PLANETOPT,
	"Terrain",	// WCS_EFFECTSSUBCLASS_TERRAINPARAM,
	"LandCover",// WCS_EFFECTSSUBCLASS_GROUND,
	"LandCover",// WCS_EFFECTSSUBCLASS_SNOW,
	"Sky",		// WCS_EFFECTSSUBCLASS_SKY,
	"Sky",		// WCS_EFFECTSSUBCLASS_ATMOSPHERE,
	"Light",	// WCS_EFFECTSSUBCLASS_LIGHT,
	"Render",	// WCS_EFFECTSSUBCLASS_CAMERA,
	"Render",	// WCS_EFFECTSSUBCLASS_RENDERJOB,
	"Render",	// WCS_EFFECTSSUBCLASS_RENDEROPT,
	"Terrain",	// WCS_EFFECTSSUBCLASS_GRIDDER,
	"Terrain",	// WCS_EFFECTSSUBCLASS_GENERATOR,
	"Vector",	// WCS_EFFECTSSUBCLASS_SEARCHQUERY,
	"LandCover",// WCS_EFFECTSSUBCLASS_THEMATICMAP,
	"Terrain",	// WCS_EFFECTSSUBCLASS_COORDSYS,
	"3DObject",	// WCS_EFFECTSSUBCLASS_FENCE,
	"Render",	// WCS_EFFECTSSUBCLASS_POSTPROC,
	"Render",	// WCS_EFFECTSSUBCLASS_SCENARIO,
	"Terrain",	// WCS_EFFECTSSUBCLASS_DEMMERGER,
	"Render",	// WCS_EFFECTSSUBCLASS_EXPORTER,
	"3DObject"	// WCS_EFFECTSSUBCLASS_LABEL,
	}; // EffectsLib::DefaultPaths
// <<<>>> ADD_NEW_EFFECTS add task mode name to end of list

// this list contains the enum values for effect classes
unsigned char EffectsLib::AlphabetizedEffects[WCS_MAXIMPLEMENTED_EFFECTS] = 
	{
	WCS_EFFECTSSUBCLASS_MATERIAL,		// 3D Material,
	WCS_EFFECTSSUBCLASS_OBJECT3D,		// 3D Object,
	WCS_EFFECTSSUBCLASS_RASTERTA,		// Area Terraffector,
	WCS_EFFECTSSUBCLASS_ATMOSPHERE,		// Atmosphere,
	WCS_EFFECTSSUBCLASS_BACKGROUND,		// Background,			// these probably won't be used
	WCS_EFFECTSSUBCLASS_CAMERA,			// Camera,
	WCS_EFFECTSSUBCLASS_CELESTIAL,		// Celestial,
	WCS_EFFECTSSUBCLASS_CLOUD,			// Cloud,
	WCS_EFFECTSSUBCLASS_CLOUDSHADOW,	// Cloud Shadow,		// these probably won't be used
	WCS_EFFECTSSUBCLASS_CMAP,			// Color Map,
	WCS_EFFECTSSUBCLASS_COMBO,			// Combo,				// obsolete
	WCS_EFFECTSSUBCLASS_COORDSYS,		// Coordinate System,
	WCS_EFFECTSSUBCLASS_DEMMERGER,		// DEM Merger,
	WCS_EFFECTSSUBCLASS_ECOSYSTEM,		// Ecosystem,
	WCS_EFFECTSSUBCLASS_ECOTYPE,		// Ecotype,				// these probably won't be used
	WCS_EFFECTSSUBCLASS_ENVIRONMENT,	// Environment,
	WCS_EFFECTSSUBCLASS_EXPORTER,		// Exporter,
	WCS_EFFECTSSUBCLASS_FOG,			// Fog,					// these probably won't be used
	WCS_EFFECTSSUBCLASS_FOLIAGE,		// Foliage,
	WCS_EFFECTSSUBCLASS_FOLIAGEGRP,		// Foliage Group,		// these probably won't be used
	WCS_EFFECTSSUBCLASS_GENERIC,		// Generic,
	WCS_EFFECTSSUBCLASS_GRADIENTPROFILE,// Gradient Profile,
	WCS_EFFECTSSUBCLASS_GROUND,			// Ground,
	WCS_EFFECTSSUBCLASS_ILLUMINATION,	// Illumination,		// obsolete
	WCS_EFFECTSSUBCLASS_LABEL,			// Label,
	WCS_EFFECTSSUBCLASS_LAKE,			// Lake,
	WCS_EFFECTSSUBCLASS_LANDWAVE,		// Land Wave,			// these probably won't be used
	WCS_EFFECTSSUBCLASS_LIGHT,			// Light,
	WCS_EFFECTSSUBCLASS_MORPH,			// Morph,				// these probably won't be used
	WCS_EFFECTSSUBCLASS_PATHCONFORM,	// Path Conform,		// these probably won't be used
	WCS_EFFECTSSUBCLASS_PATHFOLLOW,		// Path Follow,			// these probably won't be used
	WCS_EFFECTSSUBCLASS_PLANETOPT,		// Planet Options,
	WCS_EFFECTSSUBCLASS_POSTPROC,		// Post Process,
	WCS_EFFECTSSUBCLASS_PROFILE,		// Profile,
	WCS_EFFECTSSUBCLASS_RENDERJOB,		// Render Job,
	WCS_EFFECTSSUBCLASS_RENDEROPT,		// Render Options,
	WCS_EFFECTSSUBCLASS_SCENARIO,		// Render Scenario,
	WCS_EFFECTSSUBCLASS_SEARCHQUERY,	// Search Query,
	WCS_EFFECTSSUBCLASS_SHADOW,			// Shadow,
	WCS_EFFECTSSUBCLASS_SKY,			// Sky,
	WCS_EFFECTSSUBCLASS_SNOW,			// Snow,
	WCS_EFFECTSSUBCLASS_STARFIELD,		// Starfield,
	WCS_EFFECTSSUBCLASS_STREAM,			// Stream,
	WCS_EFFECTSSUBCLASS_TERRAFFECTOR,	// Terraffector,
	WCS_EFFECTSSUBCLASS_GENERATOR,		// Terrain Generator,
	WCS_EFFECTSSUBCLASS_GRIDDER,		// Terrain Gridder,
	WCS_EFFECTSSUBCLASS_TERRAINPARAM,	// Terrain Parameters,
	WCS_EFFECTSSUBCLASS_THEMATICMAP,	// Thematic Map,
	WCS_EFFECTSSUBCLASS_TINT,			// Tint,				// these probably won't be used
	WCS_EFFECTSSUBCLASS_OBJECTVEC,		// Vector Object,		// these probably won't be used
	WCS_EFFECTSSUBCLASS_FENCE,			// Fence or Wall,
	WCS_EFFECTSSUBCLASS_WAVE			// Wave,
	}; // EffectsLib::AlphabetizedEffects
// <<<>>> ADD_NEW_EFFECTS add in alphabetical order by name used in interface

char *EffectsLib::CelestialPresetName[WCS_PRESETS_NUMCELESTIALS] =
	{
	"Sun",
	"Mercury",
	"Venus",
	"Earth",
	"Mars",
	"Jupiter",
	"Saturn",
	"Uranus",
	"Neptune",
	"Pluto",
	"Moon",
	"Comet",
	"Big Meteor",
	"Geostat. Satel.",
	"Low Orbit"
	};

double EffectsLib::CelestialPresetRadius[WCS_PRESETS_NUMCELESTIALS] =
	{
	695300000.0,
	2570000.0,
	6050000.0,
	6362683.195,
	3396000.0,
	71800000.0,
	60300000.0,
	26700000.0,
	24850000.0,
	1123000.0,
	1738000.0,
	16000.0,
	1000.0,
	5.0,
	5.0
	};

double EffectsLib::CelestialPresetDistance[WCS_PRESETS_NUMCELESTIALS] =
	{
	149.598E+9,
	91.0E+9,
	41.0E+9,
	0.0,
	79.0E+9,
	629.0E+9,
	1277.0E+9,
	2720.0E+9,
	4346.0E+9,
	5751.0E+9,
	38.0E+7,
	200.0E+9,
	250000.0,
	42245000.0,
	500000.0
	};

// alphabetically sorted list of component types that can be linked to Joes
int JoeLinkableList[] = {
	WCS_EFFECTSSUBCLASS_OBJECT3D,
	WCS_EFFECTSSUBCLASS_RASTERTA,
	WCS_EFFECTSSUBCLASS_CLOUD,
	WCS_EFFECTSSUBCLASS_ECOSYSTEM,
	WCS_EFFECTSSUBCLASS_ENVIRONMENT,
	WCS_EFFECTSSUBCLASS_FOLIAGE,
	WCS_EFFECTSSUBCLASS_GROUND,
	WCS_EFFECTSSUBCLASS_LABEL,
	WCS_EFFECTSSUBCLASS_LAKE,
	WCS_EFFECTSSUBCLASS_SHADOW,
	WCS_EFFECTSSUBCLASS_SNOW,
	WCS_EFFECTSSUBCLASS_STREAM,
	WCS_EFFECTSSUBCLASS_TERRAFFECTOR,
	WCS_EFFECTSSUBCLASS_THEMATICMAP,
	WCS_EFFECTSSUBCLASS_FENCE,
	WCS_EFFECTSSUBCLASS_WAVE,
// <<<>>> ADD_NEW_EFFECTS add only if effect can be attached to Joe
	NULL
	};

// 1 if item in JoeLinkableList is an area effect, 0 if either linear or point
int JoeLinkableAreaEffect[] = {
	0,	// WCS_EFFECTSSUBCLASS_OBJECT3D
	1,	// RASTERTA
	1,	// CLOUD
	1,	// ECOSYSTEM
	1,	// ENVIRONMENT
	0,	// WCS_EFFECTSSUBCLASS_FOLIAGE
	1,	// GROUND
	0,	// LABEL
	1,	// LAKE
	1,	// SHADOW
	1,	// SNOW
	2,	// WCS_EFFECTSSUBCLASS_STREAM
	2,	// WCS_EFFECTSSUBCLASS_TERRAFFECTOR
	1,	// THEMATIC MAP
	2,	// WCS_EFFECTSSUBCLASS_FENCE
	1,	// WAVE
// <<<>>> ADD_NEW_EFFECTS add only if effect can be attached to Joe
	0
	};


EffectsLib::EffectsLib(void)
{
short i;

Lake = LastLake = DefaultLake = NULL;
Cmap = LastCmap = NULL;
Terraffector = LastTerraffector = NULL;
Wave = LastWave = NULL;
Cloud = LastCloud = NULL;
Environment = LastEnvironment = DefaultEnvironment = NULL;
Ecosystem = LastEcosystem = NULL;
Foliage = LastFoliage = NULL;
Object3D = LastObject3D = NULL;
RasterTA = LastRasterTA = NULL;
Shadow = LastShadow = NULL;
Stream = LastStream = NULL;
Material = LastMaterial = NULL;
Celestial = LastCelestial = NULL;
StarField = LastStarField = NULL;
PlanetOpts = LastPlanetOpt = NULL;
TerrainParam = LastTerrainParam = DefaultTerrainParam = NULL;
Ground = LastGround = DefaultGround = NULL;
Snow = LastSnow = NULL;
Skies = LastSky = NULL;
Atmospheres = LastAtmosphere = NULL;
Lights = LastLight = NULL;
Cameras = LastCamera = NULL;
RenderJobs = LastRenderJob = NULL;
RenderOpts = LastRenderOpt = NULL;
Gridder = LastGridder = NULL;
Generator = LastGenerator = NULL;
Search = LastSearch = NULL;
Theme = LastTheme = NULL;
Coord = LastCoord = NULL;
Fences = LastFence = NULL;
PostProc = LastPostProc = NULL;
Scenario = LastScenario = NULL;
Merger = LastMerger = NULL;
Exporter = LastExporter = NULL;
Labels = LastLabel = NULL;
// <<<>>> ADD_NEW_EFFECTS the name of the effect pointers in the Effects Library

MaxMem = 64;
for (i = 0; i < WCS_MAXIMPLEMENTED_EFFECTS; i ++)
	{
	GroupVecPolyEnabled[i] = 1;
	GroupDisplayAdvancedEnabled[i] = 0;
	} // for
ProjectFileSavedWithForestEd = 0;

DefaultCoords = new CoordSys();

} // EffectsLib::EffectsLib

/*===========================================================================*/

void EffectsLib::SetDefaults(void)
{

LakeBase.SetDefaults();
CmapBase.SetDefaults();
EcosystemBase.SetDefaults();
EnvironmentBase.SetDefaults();
RasterTerraffectorBase.SetDefaults();
TerrainParamBase.SetDefaults();
GroundBase.SetDefaults();
SnowBase.SetDefaults();
TerraffectorBase.SetDefaults();
ShadowBase.SetDefaults();
FoliageBase.SetDefaults();
StreamBase.SetDefaults();
Object3DBase.SetDefaults();
AtmoBase.SetDefaults();
FnceBase.SetDefaults();
LablBase.SetDefaults();
// <<<>>> ADD_NEW_EFFECTS add if there will be a special class used for renderer evaluation

} // EffectsLib::SetDefaults

/*===========================================================================*/

EffectsLib::~EffectsLib(void)
{

DeleteAll(FALSE);

MaterialStrata::FreeNoiseMap();

if (DefaultCoords)
	{
	delete DefaultCoords;
	DefaultCoords = NULL;
	} // if

} // EffectsLib::~EffectsLib

/*===========================================================================*/

void EffectsLib::DeleteAll(int ResetDefaults)
{

while (Lake)
	{
	RemoveEffect(Lake);
	} // while

while (Cmap)
	{
	RemoveEffect(Cmap);
	} // while

while (Terraffector)
	{
	RemoveEffect(Terraffector);
	} // while

while (Wave)
	{
	RemoveEffect(Wave);
	} // while

while (Cloud)
	{
	RemoveEffect(Cloud);
	} // while

while (Environment)
	{
	RemoveEffect(Environment);
	} // while

while (Ecosystem)
	{
	RemoveEffect(Ecosystem);
	} // while

while (Foliage)
	{
	RemoveEffect(Foliage);
	} // while

while (Object3D)
	{
	RemoveEffect(Object3D);
	} // while

while (RasterTA)
	{
	RemoveEffect(RasterTA);
	} // while

while (Shadow)
	{
	RemoveEffect(Shadow);
	} // while

while (Stream)
	{
	RemoveEffect(Stream);
	} // while

while (Material)
	{
	RemoveEffect(Material);
	} // while

while (Celestial)
	{
	RemoveEffect(Celestial);
	} // while

while (StarField)
	{
	RemoveEffect(StarField);
	} // while

while (PlanetOpts)
	{
	RemoveEffect(PlanetOpts);
	} // while

while (TerrainParam)
	{
	RemoveEffect(TerrainParam);
	} // while

while (Ground)
	{
	RemoveEffect(Ground);
	} // while

while (Snow)
	{
	RemoveEffect(Snow);
	} // while

while (Skies)
	{
	RemoveEffect(Skies);
	} // while

while (Atmospheres)
	{
	RemoveEffect(Atmospheres);
	} // while

while (Lights)
	{
	RemoveEffect(Lights);
	} // while

while (Cameras)
	{
	RemoveEffect(Cameras);
	} // while

while (RenderJobs)
	{
	RemoveEffect(RenderJobs);
	} // while

while (RenderOpts)
	{
	RemoveEffect(RenderOpts);
	} // while

while (Gridder)
	{
	RemoveEffect(Gridder);
	} // while

while (Generator)
	{
	RemoveEffect(Generator);
	} // while

while (Search)
	{
	RemoveEffect(Search);
	} // while

while (Theme)
	{
	RemoveEffect(Theme);
	} // while

while (Coord)
	{
	RemoveEffect(Coord);
	} // while

while (Fences)
	{
	RemoveEffect(Fences);
	} // while

while (PostProc)
	{
	RemoveEffect(PostProc);
	} // while

while (Scenario)
	{
	RemoveEffect(Scenario);
	} // while

while (Merger)
	{
	RemoveEffect(Merger);
	} // while

while (Exporter)
	{
	RemoveEffect(Exporter);
	} // while

while (Labels)
	{
	RemoveEffect(Labels);
	} // while

// <<<>>> ADD_NEW_EFFECTS add a removal block using the head of the linked list in the Effects Library

Lake = LastLake = NULL;
Cmap = LastCmap = NULL;
Terraffector = LastTerraffector = NULL;
Wave = LastWave = NULL;
Cloud = LastCloud = NULL;
Environment = LastEnvironment = NULL;
Ecosystem = LastEcosystem = NULL;
Foliage = LastFoliage = NULL;
Object3D = LastObject3D = NULL;
RasterTA = LastRasterTA = NULL;
Shadow = LastShadow = NULL;
Stream = LastStream = NULL;
Material = LastMaterial = NULL;
Celestial = LastCelestial = NULL;
StarField = LastStarField = NULL;
PlanetOpts = LastPlanetOpt = NULL;
TerrainParam = LastTerrainParam = NULL;
Ground = LastGround = NULL;
Snow = LastSnow = NULL;
Skies = LastSky = NULL;
Atmospheres = LastAtmosphere = NULL;
Lights = LastLight = NULL;
Cameras = LastCamera = NULL;
RenderJobs = LastRenderJob = NULL;
RenderOpts = LastRenderOpt = NULL;
Gridder = LastGridder = NULL;
Generator = LastGenerator = NULL;
Search = LastSearch = NULL;
Theme = LastTheme = NULL;
Coord = LastCoord = NULL;
Fences = LastFence = NULL;
PostProc = LastPostProc = NULL;
Scenario = LastScenario = NULL;
Merger = LastMerger = NULL;
Exporter = LastExporter = NULL;
Labels = LastLabel = NULL;
// <<<>>> ADD_NEW_EFFECTS Null the head and tail of linked list in Effects Library

LakeBase.Destroy();
CmapBase.Destroy();
EcosystemBase.Destroy();
EnvironmentBase.Destroy();
RasterTerraffectorBase.Destroy();
TerrainParamBase.Destroy();
GroundBase.Destroy();
SnowBase.Destroy();
TerraffectorBase.Destroy();
ShadowBase.Destroy();
FoliageBase.Destroy();
StreamBase.Destroy();
Object3DBase.Destroy();
FnceBase.Destroy();
LablBase.Destroy();
// <<<>>> ADD_NEW_EFFECTS add if there will be a special class used for renderer evaluation

if (ResetDefaults)
	SetDefaults();

} // EffectsLib::DeleteAll

/*===========================================================================*/

void EffectsLib::DeleteGroup(long EffectType)
{
GeneralEffect *GroupList, *TempEffect;
char TitleStr[64], MessageStr[128];

if (GroupList = GetListPtr(EffectType))
	{
	while (GroupList)
		{
		TempEffect = GroupList->Next;
		if (! RemoveRAHost(GroupList, 1))	// generates notify for each object, 1 = no queries
			{
			sprintf(TitleStr, "Remove %s", GetEffectTypeName(EffectType));
			sprintf(MessageStr, "Continue removing remaining %s?", GetEffectTypeName(EffectType));
			if (! UserMessageOKCAN(TitleStr, MessageStr))
				return;
			} // if remove failed
		GroupList = TempEffect;
		} // while
	} // if

} // EffectsLib::DeleteGroup

/*===========================================================================*/

void EffectsLib::FreeAll(int FinalCleanup)
{
RenderOpt *CleanupOpt;
Light *CleanupLight;
CmapEffect *CleanupCmap;
GeneralEffect *Cleanup;
CloudEffect *CleanupCloud;
Sky *CleanupSky;
long Ct;

for (CleanupOpt = RenderOpts; CleanupOpt; CleanupOpt = (RenderOpt *)CleanupOpt->Next)
	{
	CleanupOpt->CleanupFromRender(FinalCleanup);
	} // for
for (CleanupLight = Lights; CleanupLight; CleanupLight = (Light *)CleanupLight->Next)
	{
	CleanupLight->KillShadows();
	} // for
for (CleanupCmap = Cmap; CleanupCmap; CleanupCmap = (CmapEffect *)CleanupCmap->Next)
	{
	CleanupCmap->CleanupFromRender();
	} // for
for (CleanupCloud = Cloud; CleanupCloud; CleanupCloud = (CloudEffect *)CleanupCloud->Next)
	{
	CleanupCloud->RemoveVertices();
	} // for
for (CleanupSky = Skies; CleanupSky; CleanupSky = (Sky *)CleanupSky->Next)
	{
	CleanupSky->CleanupFromRender();
	} // for

for (Ct = WCS_EFFECTSSUBCLASS_LAKE; Ct < WCS_MAXIMPLEMENTED_EFFECTS; Ct ++)
	{
	for (Cleanup = GetListPtr(Ct); Cleanup; Cleanup = Cleanup->Next)
		Cleanup->RemoveRenderJoes();
	} // for

LakeBase.Destroy();
CmapBase.Destroy();
EcosystemBase.Destroy();
EnvironmentBase.Destroy();
RasterTerraffectorBase.Destroy();
TerrainParamBase.Destroy();
GroundBase.Destroy();
SnowBase.Destroy();
TerraffectorBase.Destroy();
ShadowBase.Destroy();
FoliageBase.Destroy();
StreamBase.Destroy();
Object3DBase.Destroy();
FnceBase.Destroy();
LablBase.Destroy();
// <<<>>> ADD_NEW_EFFECTS add if there will be a special class used for renderer evaluation

} // EffectsLib::FreeAll

/*===========================================================================*/

void EffectsLib::RemoveEffect(GeneralEffect *Remove)
{
JoeList *CurJoe;
JoeAttribute *RemoveAttr;

if (Remove)
	{
	while (Remove->Joes)
		{
		CurJoe = Remove->Joes;
		Remove->Joes = Remove->Joes->Next;
		if (RemoveAttr = CurJoe->Me->RemoveEffectAttribute(WCS_JOE_ATTRIB_INTERNAL, (unsigned char)Remove->EffectType, Remove))
			// Remember, we're calling delete on the base JoeAttribute class, not the derived class,
			// so if we add any fancy dynamic storage to any derived classes, we'll need a
			// virtual destructor...
			delete RemoveAttr;
		delete CurJoe;
		} // while
	if (Remove->Prev)
		Remove->Prev->Next = Remove->Next;
	if (Remove->Next)
		Remove->Next->Prev = Remove->Prev;
	switch (Remove->EffectType)
		{
		case WCS_JOE_ATTRIB_INTERNAL_LAKE:
			{
			if (! Remove->Prev)
				Lake = (LakeEffect *)Remove->Next;
			if (! Remove->Next)
				LastLake = (LakeEffect *)Remove->Prev;
			delete (LakeEffect *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_LAKE
		case WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT:
			{
			if (! Remove->Prev)
				Environment = (EnvironmentEffect *)Remove->Next;
			if (! Remove->Next)
				LastEnvironment = (EnvironmentEffect *)Remove->Prev;
			delete (EnvironmentEffect *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT
		case WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM:
			{
			if (! Remove->Prev)
				Ecosystem = (EcosystemEffect *)Remove->Next;
			if (! Remove->Next)
				LastEcosystem = (EcosystemEffect *)Remove->Prev;
			delete (EcosystemEffect *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM
		case WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR:
			{
			if (! Remove->Prev)
				RasterTA = (RasterTerraffectorEffect *)Remove->Next;
			if (! Remove->Next)
				LastRasterTA = (RasterTerraffectorEffect *)Remove->Prev;
			delete (RasterTerraffectorEffect *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR
		case WCS_JOE_ATTRIB_INTERNAL_CMAP:
			{
			if (! Remove->Prev)
				Cmap = (CmapEffect *)Remove->Next;
			if (! Remove->Next)
				LastCmap = (CmapEffect *)Remove->Prev;
			delete (CmapEffect *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_CMAP
		case WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR:
			{
			if (! Remove->Prev)
				Terraffector = (TerraffectorEffect *)Remove->Next;
			if (! Remove->Next)
				LastTerraffector = (TerraffectorEffect *)Remove->Prev;
			delete (TerraffectorEffect *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR
		case WCS_JOE_ATTRIB_INTERNAL_WAVE:
			{
			if (! Remove->Prev)
				Wave = (WaveEffect *)Remove->Next;
			if (! Remove->Next)
				LastWave = (WaveEffect *)Remove->Prev;
			delete (WaveEffect *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_WAVE
		case WCS_JOE_ATTRIB_INTERNAL_CLOUD:
			{
			if (! Remove->Prev)
				Cloud = (CloudEffect *)Remove->Next;
			if (! Remove->Next)
				LastCloud = (CloudEffect *)Remove->Prev;
			delete (CloudEffect *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_CLOUD
		case WCS_JOE_ATTRIB_INTERNAL_SHADOW:
			{
			if (! Remove->Prev)
				Shadow = (ShadowEffect *)Remove->Next;
			if (! Remove->Next)
				LastShadow = (ShadowEffect *)Remove->Prev;
			delete (ShadowEffect *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_SHADOW
		case WCS_JOE_ATTRIB_INTERNAL_FOLIAGE:
			{
			if (! Remove->Prev)
				Foliage = (FoliageEffect *)Remove->Next;
			if (! Remove->Next)
				LastFoliage = (FoliageEffect *)Remove->Prev;
			delete (FoliageEffect *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_FOLIAGE
		case WCS_JOE_ATTRIB_INTERNAL_STREAM:
			{
			if (! Remove->Prev)
				Stream = (StreamEffect *)Remove->Next;
			if (! Remove->Next)
				LastStream = (StreamEffect *)Remove->Prev;
			delete (StreamEffect *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_STREAM
		case WCS_JOE_ATTRIB_INTERNAL_OBJECT3D:
			{
			if (! Remove->Prev)
				Object3D = (Object3DEffect *)Remove->Next;
			if (! Remove->Next)
				LastObject3D = (Object3DEffect *)Remove->Prev;
			delete (Object3DEffect *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_OBJECT3D
		case WCS_JOE_ATTRIB_INTERNAL_MATERIAL:
			{
			if (! Remove->Prev)
				Material = (MaterialEffect *)Remove->Next;
			if (! Remove->Next)
				LastMaterial = (MaterialEffect *)Remove->Prev;
			delete (MaterialEffect *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_MATERIAL
		case WCS_JOE_ATTRIB_INTERNAL_CELESTIAL:
			{
			if (! Remove->Prev)
				Celestial = (CelestialEffect *)Remove->Next;
			if (! Remove->Next)
				LastCelestial = (CelestialEffect *)Remove->Prev;
			delete (CelestialEffect *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_CELESTIAL
		case WCS_JOE_ATTRIB_INTERNAL_STARFIELD:
			{
			if (! Remove->Prev)
				StarField = (StarFieldEffect *)Remove->Next;
			if (! Remove->Next)
				LastStarField = (StarFieldEffect *)Remove->Prev;
			delete (StarFieldEffect *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_STARFIELD
		case WCS_JOE_ATTRIB_INTERNAL_PLANETOPT:
			{
			if (! Remove->Prev)
				PlanetOpts = (PlanetOpt *)Remove->Next;
			if (! Remove->Next)
				LastPlanetOpt = (PlanetOpt *)Remove->Prev;
			delete (PlanetOpt *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_PLANETOPT
		case WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM:
			{
			if (! Remove->Prev)
				TerrainParam = (TerrainParamEffect *)Remove->Next;
			if (! Remove->Next)
				LastTerrainParam = (TerrainParamEffect *)Remove->Prev;
			delete (TerrainParamEffect *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM
		case WCS_JOE_ATTRIB_INTERNAL_GROUND:
			{
			if (! Remove->Prev)
				Ground = (GroundEffect *)Remove->Next;
			if (! Remove->Next)
				LastGround = (GroundEffect *)Remove->Prev;
			delete (GroundEffect *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_GROUND
		case WCS_JOE_ATTRIB_INTERNAL_SNOW:
			{
			if (! Remove->Prev)
				Snow = (SnowEffect *)Remove->Next;
			if (! Remove->Next)
				LastSnow = (SnowEffect *)Remove->Prev;
			delete (SnowEffect *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_SNOW
		case WCS_JOE_ATTRIB_INTERNAL_SKY:
			{
			if (! Remove->Prev)
				Skies = (Sky *)Remove->Next;
			if (! Remove->Next)
				LastSky = (Sky *)Remove->Prev;
			delete (Sky *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_SKY
		case WCS_JOE_ATTRIB_INTERNAL_ATMOSPHERE:
			{
			if (! Remove->Prev)
				Atmospheres = (Atmosphere *)Remove->Next;
			if (! Remove->Next)
				LastAtmosphere = (Atmosphere *)Remove->Prev;
			delete (Atmosphere *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_ATMOSPHERE
		case WCS_JOE_ATTRIB_INTERNAL_LIGHT:
			{
			if (! Remove->Prev)
				Lights = (Light *)Remove->Next;
			if (! Remove->Next)
				LastLight = (Light *)Remove->Prev;
			delete (Light *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_LIGHT
		case WCS_JOE_ATTRIB_INTERNAL_CAMERA:
			{
			if (! Remove->Prev)
				Cameras = (Camera *)Remove->Next;
			if (! Remove->Next)
				LastCamera = (Camera *)Remove->Prev;
			delete (Camera *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_CAMERA
		case WCS_JOE_ATTRIB_INTERNAL_RENDERJOB:
			{
			if (! Remove->Prev)
				RenderJobs = (RenderJob *)Remove->Next;
			if (! Remove->Next)
				LastRenderJob = (RenderJob *)Remove->Prev;
			delete (RenderJob *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_RENDERJOB
		case WCS_JOE_ATTRIB_INTERNAL_RENDEROPT:
			{
			if (! Remove->Prev)
				RenderOpts = (RenderOpt *)Remove->Next;
			if (! Remove->Next)
				LastRenderOpt = (RenderOpt *)Remove->Prev;
			delete (RenderOpt *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_RENDEROPT
		case WCS_JOE_ATTRIB_INTERNAL_GRIDDER:
			{
			if (! Remove->Prev)
				Gridder = (TerraGridder *)Remove->Next;
			if (! Remove->Next)
				LastGridder = (TerraGridder *)Remove->Prev;
			delete (TerraGridder *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_GRIDDER
		case WCS_JOE_ATTRIB_INTERNAL_GENERATOR:
			{
			if (! Remove->Prev)
				Generator = (TerraGenerator *)Remove->Next;
			if (! Remove->Next)
				LastGenerator = (TerraGenerator *)Remove->Prev;
			delete (TerraGenerator *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_GENERATOR
		case WCS_JOE_ATTRIB_INTERNAL_SEARCHQUERY:
			{
			if (! Remove->Prev)
				Search = (SearchQuery *)Remove->Next;
			if (! Remove->Next)
				LastSearch = (SearchQuery *)Remove->Prev;
			delete (SearchQuery *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_SEARCHQUERY
		case WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP:
			{
			if (! Remove->Prev)
				Theme = (ThematicMap *)Remove->Next;
			if (! Remove->Next)
				LastTheme = (ThematicMap *)Remove->Prev;
			delete (ThematicMap *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP
		case WCS_JOE_ATTRIB_INTERNAL_COORDSYS:
			{
			if (! Remove->Prev)
				Coord = (CoordSys *)Remove->Next;
			if (! Remove->Next)
				LastCoord = (CoordSys *)Remove->Prev;
			delete (CoordSys *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_COORDSYS
		case WCS_JOE_ATTRIB_INTERNAL_FENCE:
			{
			if (! Remove->Prev)
				Fences = (Fence *)Remove->Next;
			if (! Remove->Next)
				LastFence = (Fence *)Remove->Prev;
			delete (Fence *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_FENCE
		case WCS_JOE_ATTRIB_INTERNAL_POSTPROC:
			{
			if (! Remove->Prev)
				PostProc = (PostProcess *)Remove->Next;
			if (! Remove->Next)
				LastPostProc = (PostProcess *)Remove->Prev;
			delete (PostProcess *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_POSTPROC
		case WCS_JOE_ATTRIB_INTERNAL_SCENARIO:
			{
			if (! Remove->Prev)
				Scenario = (RenderScenario *)Remove->Next;
			if (! Remove->Next)
				LastScenario = (RenderScenario *)Remove->Prev;
			delete (RenderScenario *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_SCENARIO
		case WCS_JOE_ATTRIB_INTERNAL_DEMMERGER:
			{
			if (! Remove->Prev)
				Merger = (DEMMerger *)Remove->Next;
			if (! Remove->Next)
				LastMerger = (DEMMerger *)Remove->Prev;
			delete (DEMMerger *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_DEMMERGER
		case WCS_JOE_ATTRIB_INTERNAL_EXPORTER:
			{
			if (! Remove->Prev)
				Exporter = (SceneExporter *)Remove->Next;
			if (! Remove->Next)
				LastExporter = (SceneExporter *)Remove->Prev;
			delete (SceneExporter *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_EXPORTER
		case WCS_JOE_ATTRIB_INTERNAL_LABEL:
			{
			if (! Remove->Prev)
				Labels = (Label *)Remove->Next;
			if (! Remove->Next)
				LastLabel = (Label *)Remove->Prev;
			delete (Label *)Remove;
			break;
			} // WCS_JOE_ATTRIB_INTERNAL_LABEL
// <<<>>> ADD_NEW_EFFECTS every effect needs a block here for item removal
		} // switch
	} // if

} // EffectsLib::RemoveEffect

/*===========================================================================*/

GeneralEffect *EffectsLib::AddEffect(long EffectClass, char *NewName, GeneralEffect *Proto)
{
GeneralEffect *NewEffect = NULL;
NotifyTag Changes[2];

switch (EffectClass)
	{
	case WCS_EFFECTSSUBCLASS_LAKE:
		{
		NewEffect = (GeneralEffect *)new LakeEffect(NULL, this, (LakeEffect *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_LAKE
	case WCS_EFFECTSSUBCLASS_ECOSYSTEM:
		{
		NewEffect = (GeneralEffect *)new EcosystemEffect(NULL, this, (EcosystemEffect *)Proto, WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM);
		break;
		} // WCS_EFFECTSSUBCLASS_ECOSYSTEM
	case WCS_EFFECTSSUBCLASS_WAVE:
		{
		NewEffect = (GeneralEffect *)new WaveEffect(NULL, this, (WaveEffect *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_WAVE
	case WCS_EFFECTSSUBCLASS_CLOUD:
		{
		NewEffect = (GeneralEffect *)new CloudEffect(NULL, this, (CloudEffect *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_CLOUD
	case WCS_EFFECTSSUBCLASS_ENVIRONMENT:
		{
		NewEffect = (GeneralEffect *)new EnvironmentEffect(NULL, this, (EnvironmentEffect *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_ENVIRONMENT
	case WCS_EFFECTSSUBCLASS_CMAP:
		{
		NewEffect = (GeneralEffect *)new CmapEffect(NULL, this, (CmapEffect *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_CMAP
	case WCS_EFFECTSSUBCLASS_RASTERTA:
		{
		NewEffect = (GeneralEffect *)new RasterTerraffectorEffect(NULL, this, (RasterTerraffectorEffect *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_RASTERTA
	case WCS_EFFECTSSUBCLASS_TERRAFFECTOR:
		{
		NewEffect = (GeneralEffect *)new TerraffectorEffect(NULL, this, (TerraffectorEffect *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_TERRAFFECTOR
	case WCS_EFFECTSSUBCLASS_FOLIAGE:
		{
		NewEffect = (GeneralEffect *)new FoliageEffect(NULL, this, (FoliageEffect *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_FOLIAGE
	case WCS_EFFECTSSUBCLASS_OBJECT3D:
		{
		NewEffect = (GeneralEffect *)new Object3DEffect(NULL, this, (Object3DEffect *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_OBJECT3D
	case WCS_EFFECTSSUBCLASS_SHADOW:
		{
		NewEffect = (GeneralEffect *)new ShadowEffect(NULL, this, (ShadowEffect *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_SHADOW
	case WCS_EFFECTSSUBCLASS_STREAM:
		{
		NewEffect = (GeneralEffect *)new StreamEffect(NULL, this, (StreamEffect *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_STREAM
	case WCS_EFFECTSSUBCLASS_MATERIAL:
		{
		NewEffect = (GeneralEffect *)new MaterialEffect(NULL, this, (MaterialEffect *)Proto, WCS_EFFECTS_MATERIALTYPE_OBJECT3D);
		break;
		} // WCS_EFFECTSSUBCLASS_MATERIAL
	case WCS_EFFECTSSUBCLASS_CELESTIAL:
		{
		NewEffect = (GeneralEffect *)new CelestialEffect(NULL, this, (CelestialEffect *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_CELESTIAL
	case WCS_EFFECTSSUBCLASS_STARFIELD:
		{
		NewEffect = (GeneralEffect *)new StarFieldEffect(NULL, this, (StarFieldEffect *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_STARFIELD
	case WCS_EFFECTSSUBCLASS_PLANETOPT:
		{
		NewEffect = (GeneralEffect *)new PlanetOpt(NULL, this, (PlanetOpt *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_PLANETOPT
	case WCS_EFFECTSSUBCLASS_TERRAINPARAM:
		{
		NewEffect = (GeneralEffect *)new TerrainParamEffect(NULL, this, (TerrainParamEffect *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_TERRAINPARAM
	case WCS_EFFECTSSUBCLASS_GROUND:
		{
		NewEffect = (GeneralEffect *)new GroundEffect(NULL, this, (GroundEffect *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_GROUND
	case WCS_EFFECTSSUBCLASS_SNOW:
		{
		NewEffect = (GeneralEffect *)new SnowEffect(NULL, this, (SnowEffect *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_SNOW
	case WCS_EFFECTSSUBCLASS_SKY:
		{
		NewEffect = (GeneralEffect *)new Sky(NULL, this, (Sky *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_SKY
	case WCS_EFFECTSSUBCLASS_ATMOSPHERE:
		{
		NewEffect = (GeneralEffect *)new Atmosphere(NULL, this, (Atmosphere *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_ATMOSPHERE
	case WCS_EFFECTSSUBCLASS_LIGHT:
		{
		NewEffect = (GeneralEffect *)new Light(NULL, this, (Light *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_LIGHT
	case WCS_EFFECTSSUBCLASS_CAMERA:
		{
		NewEffect = (GeneralEffect *)new Camera(NULL, this, (Camera *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_CAMERA
	case WCS_EFFECTSSUBCLASS_RENDERJOB:
		{
		NewEffect = (GeneralEffect *)new RenderJob(NULL, this, (RenderJob *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_RENDERJOB
	case WCS_EFFECTSSUBCLASS_RENDEROPT:
		{
		NewEffect = (GeneralEffect *)new RenderOpt(NULL, this, (RenderOpt *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_RENDEROPT
	case WCS_EFFECTSSUBCLASS_GRIDDER:
		{
		NewEffect = (GeneralEffect *)new TerraGridder(NULL, this, (TerraGridder *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_GRIDDER
	case WCS_EFFECTSSUBCLASS_GENERATOR:
		{
		NewEffect = (GeneralEffect *)new TerraGenerator(NULL, this, (TerraGenerator *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_GENERATOR
	case WCS_EFFECTSSUBCLASS_SEARCHQUERY:
		{
		NewEffect = (GeneralEffect *)new SearchQuery(NULL, this, (SearchQuery *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_SEARCHQUERY
	case WCS_EFFECTSSUBCLASS_THEMATICMAP:
		{
		NewEffect = (GeneralEffect *)new ThematicMap(NULL, this, (ThematicMap *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_THEMATICMAP
	case WCS_EFFECTSSUBCLASS_COORDSYS:
		{
		NewEffect = (GeneralEffect *)new CoordSys(NULL, this, (CoordSys *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_COORDSYS
	case WCS_EFFECTSSUBCLASS_FENCE:
		{
		NewEffect = (GeneralEffect *)new Fence(NULL, this, (Fence *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_FENCE
	case WCS_EFFECTSSUBCLASS_POSTPROC:
		{
		NewEffect = (GeneralEffect *)new PostProcess(NULL, this, (PostProcess *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_POSTPROC
	case WCS_EFFECTSSUBCLASS_SCENARIO:
		{
		NewEffect = (GeneralEffect *)new RenderScenario(NULL, this, (RenderScenario *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_SCENARIO
	case WCS_EFFECTSSUBCLASS_DEMMERGER:
		{
		NewEffect = (GeneralEffect *)new DEMMerger(NULL, this, (DEMMerger *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_DEMMERGER
	case WCS_EFFECTSSUBCLASS_EXPORTER:
		{
		NewEffect = (GeneralEffect *)new SceneExporter(NULL, this, (SceneExporter *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_EXPORTER
	case WCS_EFFECTSSUBCLASS_LABEL:
		{
		NewEffect = (GeneralEffect *)new Label(NULL, this, (Label *)Proto);
		break;
		} // WCS_EFFECTSSUBCLASS_LABEL
// <<<>>> ADD_NEW_EFFECTS every effect needs a block here to create new instances
	} // switch

if (NewEffect && NewName)
	{
	NewEffect->SetUniqueName(this, NewName);
	} // if
if (NewEffect)
	{
	if (! Proto)
		NewEffect->SetFloating(1);
	Changes[0] = MAKE_ID(NewEffect->GetNotifyClass(), NewEffect->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, NewEffect->GetRAHostRoot());
	if (EffectClass == WCS_EFFECTSSUBCLASS_PLANETOPT)
		SetPlanetOptEnabled((PlanetOpt *)NewEffect, 1);
	} // if

return (NewEffect);

} // EffectsLib::AddEffect

/*===========================================================================*/

GeneralEffect *EffectsLib::GetListPtr(long EffectClass)
{

switch (EffectClass)
	{
	case WCS_EFFECTSSUBCLASS_LAKE:
		{
		return ((GeneralEffect *)Lake);
		} // WCS_EFFECTSSUBCLASS_LAKE
	case WCS_EFFECTSSUBCLASS_ECOSYSTEM:
		{
		return ((GeneralEffect *)Ecosystem);
		} // WCS_EFFECTSSUBCLASS_ECOSYSTEM
	case WCS_EFFECTSSUBCLASS_WAVE:
		{
		return ((GeneralEffect *)Wave);
		} // WCS_EFFECTSSUBCLASS_WAVE
	case WCS_EFFECTSSUBCLASS_CLOUD:
		{
		return ((GeneralEffect *)Cloud);
		} // WCS_EFFECTSSUBCLASS_CLOUD
	case WCS_EFFECTSSUBCLASS_ENVIRONMENT:
		{
		return ((GeneralEffect *)Environment);
		} // WCS_EFFECTSSUBCLASS_ENVIRONMENT
	case WCS_EFFECTSSUBCLASS_CMAP:
		{
		return ((GeneralEffect *)Cmap);
		} // WCS_EFFECTSSUBCLASS_CMAP
	case WCS_EFFECTSSUBCLASS_RASTERTA:
		{
		return ((GeneralEffect *)RasterTA);
		} // WCS_EFFECTSSUBCLASS_RASTERTA
	case WCS_EFFECTSSUBCLASS_TERRAFFECTOR:
		{
		return ((GeneralEffect *)Terraffector);
		} // WCS_EFFECTSSUBCLASS_TERRAFFECTOR
	case WCS_EFFECTSSUBCLASS_FOLIAGE:
		{
		return ((GeneralEffect *)Foliage);
		} // WCS_EFFECTSSUBCLASS_FOLIAGE
	case WCS_EFFECTSSUBCLASS_OBJECT3D:
		{
		return ((GeneralEffect *)Object3D);
		} // WCS_EFFECTSSUBCLASS_OBJECT3D
	case WCS_EFFECTSSUBCLASS_SHADOW:
		{
		return ((GeneralEffect *)Shadow);
		} // WCS_EFFECTSSUBCLASS_SHADOW
	case WCS_EFFECTSSUBCLASS_STREAM:
		{
		return ((GeneralEffect *)Stream);
		} // WCS_EFFECTSSUBCLASS_STREAM
	case WCS_EFFECTSSUBCLASS_MATERIAL:
		{
		return ((GeneralEffect *)Material);
		} // WCS_EFFECTSSUBCLASS_MATERIAL
	case WCS_EFFECTSSUBCLASS_CELESTIAL:
		{
		return ((GeneralEffect *)Celestial);
		} // WCS_EFFECTSSUBCLASS_CELESTIAL
	case WCS_EFFECTSSUBCLASS_STARFIELD:
		{
		return ((GeneralEffect *)StarField);
		} // WCS_EFFECTSSUBCLASS_STARFIELD
	case WCS_EFFECTSSUBCLASS_PLANETOPT:
		{
		return ((GeneralEffect *)PlanetOpts);
		} // WCS_EFFECTSSUBCLASS_PLANETOPT
	case WCS_EFFECTSSUBCLASS_TERRAINPARAM:
		{
		return ((GeneralEffect *)TerrainParam);
		} // WCS_EFFECTSSUBCLASS_TERRAINPARAM
	case WCS_EFFECTSSUBCLASS_GROUND:
		{
		return ((GeneralEffect *)Ground);
		} // WCS_EFFECTSSUBCLASS_GROUND
	case WCS_EFFECTSSUBCLASS_SNOW:
		{
		return ((GeneralEffect *)Snow);
		} // WCS_EFFECTSSUBCLASS_SNOW
	case WCS_EFFECTSSUBCLASS_SKY:
		{
		return ((GeneralEffect *)Skies);
		} // WCS_EFFECTSSUBCLASS_SKY
	case WCS_EFFECTSSUBCLASS_ATMOSPHERE:
		{
		return ((GeneralEffect *)Atmospheres);
		} // WCS_EFFECTSSUBCLASS_ATMOSPHERE
	case WCS_EFFECTSSUBCLASS_LIGHT:
		{
		return ((GeneralEffect *)Lights);
		} // WCS_EFFECTSSUBCLASS_LIGHT
	case WCS_EFFECTSSUBCLASS_CAMERA:
		{
		return ((GeneralEffect *)Cameras);
		} // WCS_EFFECTSSUBCLASS_CAMERA
	case WCS_EFFECTSSUBCLASS_RENDERJOB:
		{
		return ((GeneralEffect *)RenderJobs);
		} // WCS_EFFECTSSUBCLASS_RENDERJOB
	case WCS_EFFECTSSUBCLASS_RENDEROPT:
		{
		return ((GeneralEffect *)RenderOpts);
		} // WCS_EFFECTSSUBCLASS_RENDEROPT
	case WCS_EFFECTSSUBCLASS_GRIDDER:
		{
		return ((GeneralEffect *)Gridder);
		} // WCS_EFFECTSSUBCLASS_GRIDDER
	case WCS_EFFECTSSUBCLASS_GENERATOR:
		{
		return ((GeneralEffect *)Generator);
		} // WCS_EFFECTSSUBCLASS_GENERATOR
	case WCS_EFFECTSSUBCLASS_SEARCHQUERY:
		{
		return ((GeneralEffect *)Search);
		} // WCS_EFFECTSSUBCLASS_SEARCHQUERY
	case WCS_EFFECTSSUBCLASS_THEMATICMAP:
		{
		return ((GeneralEffect *)Theme);
		} // WCS_EFFECTSSUBCLASS_THEMATICMAP
	case WCS_EFFECTSSUBCLASS_COORDSYS:
		{
		return ((GeneralEffect *)Coord);
		} // WCS_EFFECTSSUBCLASS_COORDSYS
	case WCS_EFFECTSSUBCLASS_FENCE:
		{
		return ((GeneralEffect *)Fences);
		} // WCS_EFFECTSSUBCLASS_FENCE
	case WCS_EFFECTSSUBCLASS_POSTPROC:
		{
		return ((GeneralEffect *)PostProc);
		} // WCS_EFFECTSSUBCLASS_POSTPROC
	case WCS_EFFECTSSUBCLASS_SCENARIO:
		{
		return ((GeneralEffect *)Scenario);
		} // WCS_EFFECTSSUBCLASS_SCENARIO
	case WCS_EFFECTSSUBCLASS_DEMMERGER:
		{
		return ((GeneralEffect *)Merger);
		} // WCS_EFFECTSSUBCLASS_DEMMERGER
	case WCS_EFFECTSSUBCLASS_EXPORTER:
		{
		return ((GeneralEffect *)Exporter);
		} // WCS_EFFECTSSUBCLASS_EXPORTER
	case WCS_EFFECTSSUBCLASS_LABEL:
		{
		return ((GeneralEffect *)Labels);
		} // WCS_EFFECTSSUBCLASS_LABEL
// <<<>>> ADD_NEW_EFFECTS every effect needs a block here to return the head of the linked list
	default:
		{
		return ((GeneralEffect *)NULL);
		} // 
	} // switch


} // EffectsLib::GetListPtr

/*===========================================================================*/

void EffectsLib::SetListHead(long EffectClass, GeneralEffect *NewHead)
{

switch (EffectClass)
	{
	case WCS_EFFECTSSUBCLASS_LAKE:
		{
		Lake = (LakeEffect *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_LAKE
	case WCS_EFFECTSSUBCLASS_ECOSYSTEM:
		{
		Ecosystem = (EcosystemEffect *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_ECOSYSTEM
	case WCS_EFFECTSSUBCLASS_WAVE:
		{
		Wave = (WaveEffect *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_WAVE
	case WCS_EFFECTSSUBCLASS_CLOUD:
		{
		Cloud = (CloudEffect *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_CLOUD
	case WCS_EFFECTSSUBCLASS_ENVIRONMENT:
		{
		Environment = (EnvironmentEffect *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_ENVIRONMENT
	case WCS_EFFECTSSUBCLASS_CMAP:
		{
		Cmap = (CmapEffect *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_CMAP
	case WCS_EFFECTSSUBCLASS_RASTERTA:
		{
		RasterTA = (RasterTerraffectorEffect *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_RASTERTA
	case WCS_EFFECTSSUBCLASS_TERRAFFECTOR:
		{
		Terraffector = (TerraffectorEffect *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_TERRAFFECTOR
	case WCS_EFFECTSSUBCLASS_FOLIAGE:
		{
		Foliage = (FoliageEffect *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_FOLIAGE
	case WCS_EFFECTSSUBCLASS_OBJECT3D:
		{
		Object3D = (Object3DEffect *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_OBJECT3D
	case WCS_EFFECTSSUBCLASS_SHADOW:
		{
		Shadow = (ShadowEffect *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_SHADOW
	case WCS_EFFECTSSUBCLASS_STREAM:
		{
		Stream = (StreamEffect *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_STREAM
	case WCS_EFFECTSSUBCLASS_MATERIAL:
		{
		Material = (MaterialEffect *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_MATERIAL
	case WCS_EFFECTSSUBCLASS_CELESTIAL:
		{
		Celestial = (CelestialEffect *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_CELESTIAL
	case WCS_EFFECTSSUBCLASS_STARFIELD:
		{
		StarField = (StarFieldEffect *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_STARFIELD
	case WCS_EFFECTSSUBCLASS_PLANETOPT:
		{
		PlanetOpts = (PlanetOpt *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_PLANETOPT
	case WCS_EFFECTSSUBCLASS_TERRAINPARAM:
		{
		TerrainParam = (TerrainParamEffect *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_TERRAINPARAM
	case WCS_EFFECTSSUBCLASS_GROUND:
		{
		Ground = (GroundEffect *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_GROUND
	case WCS_EFFECTSSUBCLASS_SNOW:
		{
		Snow = (SnowEffect *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_SNOW
	case WCS_EFFECTSSUBCLASS_SKY:
		{
		Skies = (Sky *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_SKY
	case WCS_EFFECTSSUBCLASS_ATMOSPHERE:
		{
		Atmospheres = (Atmosphere *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_ATMOSPHERE
	case WCS_EFFECTSSUBCLASS_LIGHT:
		{
		Lights = (Light *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_LIGHT
	case WCS_EFFECTSSUBCLASS_CAMERA:
		{
		Cameras = (Camera *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_CAMERA
	case WCS_EFFECTSSUBCLASS_RENDERJOB:
		{
		RenderJobs = (RenderJob *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_RENDERJOB
	case WCS_EFFECTSSUBCLASS_RENDEROPT:
		{
		RenderOpts = (RenderOpt *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_RENDEROPT
	case WCS_EFFECTSSUBCLASS_GRIDDER:
		{
		Gridder = (TerraGridder *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_GRIDDER
	case WCS_EFFECTSSUBCLASS_GENERATOR:
		{
		Generator = (TerraGenerator *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_GENERATOR
	case WCS_EFFECTSSUBCLASS_SEARCHQUERY:
		{
		Search = (SearchQuery *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_SEARCHQUERY
	case WCS_EFFECTSSUBCLASS_THEMATICMAP:
		{
		Theme = (ThematicMap *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_THEMATICMAP
	case WCS_EFFECTSSUBCLASS_COORDSYS:
		{
		Coord = (CoordSys *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_COORDSYS
	case WCS_EFFECTSSUBCLASS_FENCE:
		{
		Fences = (Fence *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_FENCE
	case WCS_EFFECTSSUBCLASS_POSTPROC:
		{
		PostProc = (PostProcess *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_POSTPROC
	case WCS_EFFECTSSUBCLASS_SCENARIO:
		{
		Scenario = (RenderScenario *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_SCENARIO
	case WCS_EFFECTSSUBCLASS_DEMMERGER:
		{
		Merger = (DEMMerger *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_DEMMERGER
	case WCS_EFFECTSSUBCLASS_EXPORTER:
		{
		Exporter = (SceneExporter *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_EXPORTER
	case WCS_EFFECTSSUBCLASS_LABEL:
		{
		Labels = (Label *)NewHead;
		break;
		} // WCS_EFFECTSSUBCLASS_LABEL
// <<<>>> ADD_NEW_EFFECTS every effect needs a block here to set the head of the linked list
	} // switch

} // EffectsLib::SetListHead

/*===========================================================================*/

void EffectsLib::SetListTail(long EffectClass, GeneralEffect *NewTail)
{

switch (EffectClass)
	{
	case WCS_EFFECTSSUBCLASS_LAKE:
		{
		LastLake = (LakeEffect *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_LAKE
	case WCS_EFFECTSSUBCLASS_ECOSYSTEM:
		{
		LastEcosystem = (EcosystemEffect *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_ECOSYSTEM
	case WCS_EFFECTSSUBCLASS_WAVE:
		{
		LastWave = (WaveEffect *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_WAVE
	case WCS_EFFECTSSUBCLASS_CLOUD:
		{
		LastCloud = (CloudEffect *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_CLOUD
	case WCS_EFFECTSSUBCLASS_ENVIRONMENT:
		{
		LastEnvironment = (EnvironmentEffect *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_ENVIRONMENT
	case WCS_EFFECTSSUBCLASS_CMAP:
		{
		LastCmap = (CmapEffect *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_CMAP
	case WCS_EFFECTSSUBCLASS_RASTERTA:
		{
		LastRasterTA = (RasterTerraffectorEffect *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_RASTERTA
	case WCS_EFFECTSSUBCLASS_TERRAFFECTOR:
		{
		LastTerraffector = (TerraffectorEffect *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_TERRAFFECTOR
	case WCS_EFFECTSSUBCLASS_FOLIAGE:
		{
		LastFoliage = (FoliageEffect *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_FOLIAGE
	case WCS_EFFECTSSUBCLASS_OBJECT3D:
		{
		LastObject3D = (Object3DEffect *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_OBJECT3D
	case WCS_EFFECTSSUBCLASS_SHADOW:
		{
		LastShadow = (ShadowEffect *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_SHADOW
	case WCS_EFFECTSSUBCLASS_STREAM:
		{
		LastStream = (StreamEffect *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_STREAM
	case WCS_EFFECTSSUBCLASS_MATERIAL:
		{
		LastMaterial = (MaterialEffect *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_MATERIAL
	case WCS_EFFECTSSUBCLASS_CELESTIAL:
		{
		LastCelestial = (CelestialEffect *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_CELESTIAL
	case WCS_EFFECTSSUBCLASS_STARFIELD:
		{
		LastStarField = (StarFieldEffect *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_STARFIELD
	case WCS_EFFECTSSUBCLASS_PLANETOPT:
		{
		LastPlanetOpt = (PlanetOpt *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_PLANETOPT
	case WCS_EFFECTSSUBCLASS_TERRAINPARAM:
		{
		LastTerrainParam = (TerrainParamEffect *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_TERRAINPARAM
	case WCS_EFFECTSSUBCLASS_GROUND:
		{
		LastGround = (GroundEffect *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_GROUND
	case WCS_EFFECTSSUBCLASS_SNOW:
		{
		LastSnow = (SnowEffect *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_SNOW
	case WCS_EFFECTSSUBCLASS_SKY:
		{
		LastSky = (Sky *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_SKY
	case WCS_EFFECTSSUBCLASS_ATMOSPHERE:
		{
		LastAtmosphere = (Atmosphere *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_ATMOSPHERE
	case WCS_EFFECTSSUBCLASS_LIGHT:
		{
		LastLight = (Light *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_LIGHT
	case WCS_EFFECTSSUBCLASS_CAMERA:
		{
		LastCamera = (Camera *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_CAMERA
	case WCS_EFFECTSSUBCLASS_RENDERJOB:
		{
		LastRenderJob = (RenderJob *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_RENDERJOB
	case WCS_EFFECTSSUBCLASS_RENDEROPT:
		{
		LastRenderOpt = (RenderOpt *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_RENDEROPT
	case WCS_EFFECTSSUBCLASS_GRIDDER:
		{
		LastGridder = (TerraGridder *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_GRIDDER
	case WCS_EFFECTSSUBCLASS_GENERATOR:
		{
		LastGenerator = (TerraGenerator *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_GENERATOR
	case WCS_EFFECTSSUBCLASS_SEARCHQUERY:
		{
		LastSearch = (SearchQuery *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_SEARCHQUERY
	case WCS_EFFECTSSUBCLASS_THEMATICMAP:
		{
		LastTheme = (ThematicMap *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_THEMATICMAP
	case WCS_EFFECTSSUBCLASS_COORDSYS:
		{
		LastCoord = (CoordSys *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_COORDSYS
	case WCS_EFFECTSSUBCLASS_FENCE:
		{
		LastFence = (Fence *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_FENCE
	case WCS_EFFECTSSUBCLASS_POSTPROC:
		{
		LastPostProc = (PostProcess *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_POSTPROC
	case WCS_EFFECTSSUBCLASS_SCENARIO:
		{
		LastScenario = (RenderScenario *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_SCENARIO
	case WCS_EFFECTSSUBCLASS_DEMMERGER:
		{
		LastMerger = (DEMMerger *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_DEMMERGER
	case WCS_EFFECTSSUBCLASS_EXPORTER:
		{
		LastExporter = (SceneExporter *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_EXPORTER
	case WCS_EFFECTSSUBCLASS_LABEL:
		{
		LastLabel = (Label *)NewTail;
		break;
		} // WCS_EFFECTSSUBCLASS_LABEL
// <<<>>> ADD_NEW_EFFECTS every effect needs a block here to set the tail of the linked list
	} // switch

} // EffectsLib::SetListTail

/*===========================================================================*/

int EffectsLib::EffectTypeImplemented(long EffectClass)
{

switch (EffectClass)
	{
	case WCS_EFFECTSSUBCLASS_COMBO:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_COMBO
	case WCS_EFFECTSSUBCLASS_LAKE:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_LAKE
	case WCS_EFFECTSSUBCLASS_ECOSYSTEM:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_ECOSYSTEM
	case WCS_EFFECTSSUBCLASS_FOG:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_FOG
	case WCS_EFFECTSSUBCLASS_WAVE:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_WAVE
	case WCS_EFFECTSSUBCLASS_CLOUD:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_CLOUD
	case WCS_EFFECTSSUBCLASS_CLOUDSHADOW:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_CLOUDSHADOW
	case WCS_EFFECTSSUBCLASS_ENVIRONMENT:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_ENVIRONMENT
	case WCS_EFFECTSSUBCLASS_TINT:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_TINT
	case WCS_EFFECTSSUBCLASS_CMAP:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_CMAP
	case WCS_EFFECTSSUBCLASS_ILLUMINATION:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_ILLUMINATION
	case WCS_EFFECTSSUBCLASS_LANDWAVE:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_LANDWAVE
	case WCS_EFFECTSSUBCLASS_RASTERTA:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_RASTERTA
	case WCS_EFFECTSSUBCLASS_TERRAFFECTOR:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_TERRAFFECTOR
	case WCS_EFFECTSSUBCLASS_PROFILE:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_PROFILE
	case WCS_EFFECTSSUBCLASS_GRADIENTPROFILE:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_GRADIENTPROFILE
	case WCS_EFFECTSSUBCLASS_PATHFOLLOW:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_PATHFOLLOW
	case WCS_EFFECTSSUBCLASS_PATHCONFORM:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_PATHCONFORM
	case WCS_EFFECTSSUBCLASS_MORPH:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_MORPH
	case WCS_EFFECTSSUBCLASS_FOLIAGE:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_FOLIAGE
	case WCS_EFFECTSSUBCLASS_OBJECT3D:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_OBJECT3D
	case WCS_EFFECTSSUBCLASS_OBJECTVEC:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_OBJECTVEC
	case WCS_EFFECTSSUBCLASS_SHADOW:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_SHADOW
	case WCS_EFFECTSSUBCLASS_STREAM:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_STREAM
	case WCS_EFFECTSSUBCLASS_MATERIAL:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_MATERIAL
	case WCS_EFFECTSSUBCLASS_BACKGROUND:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_BACKGROUND
	case WCS_EFFECTSSUBCLASS_CELESTIAL:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_CELESTIAL
	case WCS_EFFECTSSUBCLASS_STARFIELD:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_STARFIELD
	case WCS_EFFECTSSUBCLASS_PLANETOPT:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_PLANETOPT
	case WCS_EFFECTSSUBCLASS_TERRAINPARAM:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_TERRAINPARAM
	case WCS_EFFECTSSUBCLASS_GROUND:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_GROUND
	case WCS_EFFECTSSUBCLASS_SNOW:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_SNOW
	case WCS_EFFECTSSUBCLASS_SKY:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_SKY
	case WCS_EFFECTSSUBCLASS_ATMOSPHERE:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_ATMOSPHERE
	case WCS_EFFECTSSUBCLASS_LIGHT:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_LIGHT
	case WCS_EFFECTSSUBCLASS_CAMERA:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_CAMERA
	case WCS_EFFECTSSUBCLASS_RENDERJOB:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_RENDERJOB
	case WCS_EFFECTSSUBCLASS_RENDEROPT:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_RENDEROPT
	case WCS_EFFECTSSUBCLASS_GRIDDER:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_GRIDDER
	case WCS_EFFECTSSUBCLASS_GENERATOR:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_GENERATOR
	case WCS_EFFECTSSUBCLASS_SEARCHQUERY:
		{
		#ifdef WCS_SEARCH_QUERY
		return (1);
		#else // WCS_SEARCH_QUERY
		return (0);
		#endif // WCS_SEARCH_QUERY
		} // WCS_EFFECTSSUBCLASS_SEARCHQUERY
	case WCS_EFFECTSSUBCLASS_THEMATICMAP:
		{
		#ifdef WCS_THEMATIC_MAP
		return (1);
		#else // WCS_THEMATIC_MAP
		return (0);
		#endif // WCS_THEMATIC_MAP
		} // WCS_EFFECTSSUBCLASS_THEMATICMAP
	case WCS_EFFECTSSUBCLASS_COORDSYS:
		{
		#ifdef WCS_COORD_SYSTEM
		return (1);
		#else // WCS_COORD_SYSTEM
		return (0);
		#endif // WCS_COORD_SYSTEM
		} // WCS_EFFECTSSUBCLASS_COORDSYS
	case WCS_EFFECTSSUBCLASS_FENCE:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_FENCE
	case WCS_EFFECTSSUBCLASS_POSTPROC:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_POSTPROC
	case WCS_EFFECTSSUBCLASS_SCENARIO:
		{
		#ifdef WCS_RENDER_SCENARIOS
		return (1);
		#else // WCS_RENDER_SCENARIOS
		return (0);
		#endif // WCS_RENDER_SCENARIOS
		} // WCS_EFFECTSSUBCLASS_SCENARIO
	case WCS_EFFECTSSUBCLASS_DEMMERGER:
		{
		#ifdef WCS_DEM_MERGE
		return (1);
		#else // WCS_DEM_MERGE
		return (0);
		#endif // WCS_DEM_MERGE
		} // WCS_EFFECTSSUBCLASS_DEMMERGER
	case WCS_EFFECTSSUBCLASS_EXPORTER:
		{
		#ifdef WCS_BUILD_RTX
		return (GlobalApp->SXAuthorized);
		#else // WCS_BUILD_RTX
		return (0);
		#endif // WCS_BUILD_RTX
		} // WCS_EFFECTSSUBCLASS_EXPORTER
	case WCS_EFFECTSSUBCLASS_LABEL:
		{
		#ifdef WCS_LABEL
		return (1);
		#else // WCS_LABEL
		return (0);
		#endif // WCS_LABEL
		} // WCS_EFFECTSSUBCLASS_LABEL
// <<<>>> ADD_NEW_EFFECTS every effect needs a block here to tell if the effect class is implemented currently.
// If this returns 0 it will not appear in S@G but may still be loaded and saved in project files.
	default:
		{
		return (0);
		} // 
	} // switch

} // EffectsLib::EffectTypeImplemented

/*===========================================================================*/

char EffectsLib::GetIconType(long EffectClass)
{

switch (EffectClass)
	{
	case WCS_EFFECTSSUBCLASS_LAKE:
		{
		return (WCS_RAHOST_ICONTYPE_LAKE);
		} // WCS_EFFECTSSUBCLASS_LAKE
	case WCS_EFFECTSSUBCLASS_ECOSYSTEM:
		{
		return (WCS_RAHOST_ICONTYPE_ECOSYSTEM);
		} // WCS_EFFECTSSUBCLASS_ECOSYSTEM
	case WCS_EFFECTSSUBCLASS_WAVE:
		{
		return (WCS_RAHOST_ICONTYPE_WAVE);
		} // WCS_EFFECTSSUBCLASS_WAVE
	case WCS_EFFECTSSUBCLASS_CLOUD:
		{
		return (WCS_RAHOST_ICONTYPE_CLOUD);
		} // WCS_EFFECTSSUBCLASS_CLOUD
	case WCS_EFFECTSSUBCLASS_ENVIRONMENT:
		{
		return (WCS_RAHOST_ICONTYPE_ENVIRONMENT);
		} // WCS_EFFECTSSUBCLASS_ENVIRONMENT
	case WCS_EFFECTSSUBCLASS_CMAP:
		{
		return (WCS_RAHOST_ICONTYPE_CMAP);
		} // WCS_EFFECTSSUBCLASS_CMAP
	case WCS_EFFECTSSUBCLASS_RASTERTA:
		{
		return (WCS_RAHOST_ICONTYPE_RASTERTA);
		} // WCS_EFFECTSSUBCLASS_RASTERTA
	case WCS_EFFECTSSUBCLASS_TERRAFFECTOR:
		{
		return (WCS_RAHOST_ICONTYPE_TERRAFFECTOR);
		} // WCS_EFFECTSSUBCLASS_TERRAFFECTOR
	case WCS_EFFECTSSUBCLASS_FOLIAGE:
		{
		return (WCS_RAHOST_ICONTYPE_FOLEFFECT);
		} // WCS_EFFECTSSUBCLASS_FOLIAGE
	case WCS_EFFECTSSUBCLASS_OBJECT3D:
		{
		return (WCS_RAHOST_ICONTYPE_3DOBJECT);
		} // WCS_EFFECTSSUBCLASS_OBJECT3D
	case WCS_EFFECTSSUBCLASS_SHADOW:
		{
		return (WCS_RAHOST_ICONTYPE_SHADOW);
		} // WCS_EFFECTSSUBCLASS_SHADOW
	case WCS_EFFECTSSUBCLASS_STREAM:
		{
		return (WCS_RAHOST_ICONTYPE_STREAM);
		} // WCS_EFFECTSSUBCLASS_STREAM
	case WCS_EFFECTSSUBCLASS_MATERIAL:
		{
		return (WCS_RAHOST_ICONTYPE_MATERIAL);
		} // WCS_EFFECTSSUBCLASS_MATERIAL
	case WCS_EFFECTSSUBCLASS_CELESTIAL:
		{
		return (WCS_RAHOST_ICONTYPE_CELESTIAL);
		} // WCS_EFFECTSSUBCLASS_CELESTIAL
	case WCS_EFFECTSSUBCLASS_STARFIELD:
		{
		return (WCS_RAHOST_ICONTYPE_STAR);
		} // WCS_EFFECTSSUBCLASS_STARFIELD
	case WCS_EFFECTSSUBCLASS_PLANETOPT:
		{
		return (WCS_RAHOST_ICONTYPE_PLANET);
		} // WCS_EFFECTSSUBCLASS_PLANETOPT
	case WCS_EFFECTSSUBCLASS_TERRAINPARAM:
		{
		return (WCS_RAHOST_ICONTYPE_TERRAINPAR);
		} // WCS_EFFECTSSUBCLASS_TERRAINPARAM
	case WCS_EFFECTSSUBCLASS_GROUND:
		{
		return (WCS_RAHOST_ICONTYPE_GROUND);
		} // WCS_EFFECTSSUBCLASS_GROUND
	case WCS_EFFECTSSUBCLASS_SNOW:
		{
		return (WCS_RAHOST_ICONTYPE_SNOW);
		} // WCS_EFFECTSSUBCLASS_SNOW
	case WCS_EFFECTSSUBCLASS_SKY:
		{
		return (WCS_RAHOST_ICONTYPE_SKY);
		} // WCS_EFFECTSSUBCLASS_SKY
	case WCS_EFFECTSSUBCLASS_ATMOSPHERE:
		{
		return (WCS_RAHOST_ICONTYPE_ATMOSPHERE);
		} // WCS_EFFECTSSUBCLASS_ATMOSPHERE
	case WCS_EFFECTSSUBCLASS_LIGHT:
		{
		return (WCS_RAHOST_ICONTYPE_LIGHT);
		} // WCS_EFFECTSSUBCLASS_LIGHT
	case WCS_EFFECTSSUBCLASS_CAMERA:
		{
		return (WCS_RAHOST_ICONTYPE_RENDER);
		} // WCS_EFFECTSSUBCLASS_RENDERJOB
	case WCS_EFFECTSSUBCLASS_RENDERJOB:
		{
		return (WCS_RAHOST_ICONTYPE_RENDJOB);
		} // WCS_EFFECTSSUBCLASS_RENDEROPT
	case WCS_EFFECTSSUBCLASS_RENDEROPT:
		{
		return (WCS_RAHOST_ICONTYPE_RENDOPT);
		} // WCS_EFFECTSSUBCLASS_RENDEROPT
	case WCS_EFFECTSSUBCLASS_GRIDDER:
		{
		return (WCS_RAHOST_ICONTYPE_TERRAINGRID);
		} // WCS_EFFECTSSUBCLASS_GRIDDER
	case WCS_EFFECTSSUBCLASS_GENERATOR:
		{
		return (WCS_RAHOST_ICONTYPE_TERRAINGEN);
		} // WCS_EFFECTSSUBCLASS_GENERATOR
	case WCS_EFFECTSSUBCLASS_SEARCHQUERY:
		{
		return (WCS_RAHOST_ICONTYPE_SCHQY);
		} // WCS_EFFECTSSUBCLASS_SEARCHQUERY
	case WCS_EFFECTSSUBCLASS_THEMATICMAP:
		{
		return (WCS_RAHOST_ICONTYPE_THEMATIC);
		} // WCS_EFFECTSSUBCLASS_THEMATICMAP
	case WCS_EFFECTSSUBCLASS_COORDSYS:
		{
		return (WCS_RAHOST_ICONTYPE_COORDSYS);
		} // WCS_EFFECTSSUBCLASS_COORDSYS
	case WCS_EFFECTSSUBCLASS_FENCE:
		{
		return (WCS_RAHOST_ICONTYPE_FENCE);
		} // WCS_EFFECTSSUBCLASS_FENCE
	case WCS_EFFECTSSUBCLASS_POSTPROC:
		{
		return (WCS_RAHOST_ICONTYPE_POSTPROC);
		} // WCS_EFFECTSSUBCLASS_POSTPROC
	case WCS_EFFECTSSUBCLASS_SCENARIO:
		{
		return (WCS_RAHOST_ICONTYPE_RENDSCEN);
		} // WCS_EFFECTSSUBCLASS_SCENARIO
	case WCS_EFFECTSSUBCLASS_DEMMERGER:
		{
		return (WCS_RAHOST_ICONTYPE_DEMMERGE);
		} // WCS_EFFECTSSUBCLASS_DEMMERGER
	case WCS_EFFECTSSUBCLASS_EXPORTER:
		{
		return (WCS_RAHOST_ICONTYPE_SCENEEXP);
		} // WCS_EFFECTSSUBCLASS_EXPORTER
	case WCS_EFFECTSSUBCLASS_LABEL:
		{
		return (WCS_RAHOST_ICONTYPE_LABEL);
		} // WCS_EFFECTSSUBCLASS_LABEL
// <<<>>> ADD_NEW_EFFECTS every effect needs a block here to define the icon type it uses in S@G
// Icon types are enumerated in RasterAnimHost.h. Icons themselves are in a special concatenated bmp file.
	default:
		{
		return (WCS_RAHOST_ICONTYPE_MISC);
		} // 
	} // switch

} // EffectsLib::GetIconType

/*===========================================================================*/

int EffectsLib::ContributesToMaterials(long EffectClass)
{

switch (EffectClass)
	{
	case WCS_EFFECTSSUBCLASS_COMBO:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_COMBO
	case WCS_EFFECTSSUBCLASS_LAKE:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_LAKE
	case WCS_EFFECTSSUBCLASS_ECOSYSTEM:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_ECOSYSTEM
	case WCS_EFFECTSSUBCLASS_FOG:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_FOG
	case WCS_EFFECTSSUBCLASS_WAVE:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_WAVE
	case WCS_EFFECTSSUBCLASS_CLOUD:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_CLOUD
	case WCS_EFFECTSSUBCLASS_CLOUDSHADOW:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_CLOUDSHADOW
	case WCS_EFFECTSSUBCLASS_ENVIRONMENT:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_ENVIRONMENT
	case WCS_EFFECTSSUBCLASS_TINT:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_TINT
	case WCS_EFFECTSSUBCLASS_CMAP:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_CMAP
	case WCS_EFFECTSSUBCLASS_ILLUMINATION:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_ILLUMINATION
	case WCS_EFFECTSSUBCLASS_LANDWAVE:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_LANDWAVE
	case WCS_EFFECTSSUBCLASS_RASTERTA:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_RASTERTA
	case WCS_EFFECTSSUBCLASS_TERRAFFECTOR:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_TERRAFFECTOR
	case WCS_EFFECTSSUBCLASS_PROFILE:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_PROFILE
	case WCS_EFFECTSSUBCLASS_GRADIENTPROFILE:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_GRADIENTPROFILE
	case WCS_EFFECTSSUBCLASS_PATHFOLLOW:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_PATHFOLLOW
	case WCS_EFFECTSSUBCLASS_PATHCONFORM:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_PATHCONFORM
	case WCS_EFFECTSSUBCLASS_MORPH:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_MORPH
	case WCS_EFFECTSSUBCLASS_FOLIAGE:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_FOLIAGE
	case WCS_EFFECTSSUBCLASS_OBJECT3D:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_OBJECT3D
	case WCS_EFFECTSSUBCLASS_OBJECTVEC:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_OBJECTVEC
	case WCS_EFFECTSSUBCLASS_SHADOW:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_SHADOW
	case WCS_EFFECTSSUBCLASS_STREAM:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_STREAM
	case WCS_EFFECTSSUBCLASS_MATERIAL:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_MATERIAL
	case WCS_EFFECTSSUBCLASS_BACKGROUND:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_BACKGROUND
	case WCS_EFFECTSSUBCLASS_CELESTIAL:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_CELESTIAL
	case WCS_EFFECTSSUBCLASS_STARFIELD:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_STARFIELD
	case WCS_EFFECTSSUBCLASS_PLANETOPT:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_PLANETOPT
	case WCS_EFFECTSSUBCLASS_TERRAINPARAM:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_TERRAINPARAM
	case WCS_EFFECTSSUBCLASS_GROUND:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_GROUND
	case WCS_EFFECTSSUBCLASS_SNOW:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_SNOW
	case WCS_EFFECTSSUBCLASS_SKY:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_SKY
	case WCS_EFFECTSSUBCLASS_ATMOSPHERE:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_ATMOSPHERE
	case WCS_EFFECTSSUBCLASS_LIGHT:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_LIGHT
	case WCS_EFFECTSSUBCLASS_CAMERA:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_CAMERA
	case WCS_EFFECTSSUBCLASS_RENDERJOB:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_RENDERJOB
	case WCS_EFFECTSSUBCLASS_RENDEROPT:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_RENDEROPT
	case WCS_EFFECTSSUBCLASS_GRIDDER:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_GRIDDER
	case WCS_EFFECTSSUBCLASS_GENERATOR:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_GENERATOR
	case WCS_EFFECTSSUBCLASS_SEARCHQUERY:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_SEARCHQUERY
	case WCS_EFFECTSSUBCLASS_THEMATICMAP:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_THEMATICMAP
	case WCS_EFFECTSSUBCLASS_COORDSYS:
		{
		return (1);
		} // WCS_EFFECTSSUBCLASS_COORDSYS
	case WCS_EFFECTSSUBCLASS_FENCE:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_FENCE
	case WCS_EFFECTSSUBCLASS_POSTPROC:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_POSTPROC
	case WCS_EFFECTSSUBCLASS_SCENARIO:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_SCENARIO
	case WCS_EFFECTSSUBCLASS_DEMMERGER:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_DEMMERGER
	case WCS_EFFECTSSUBCLASS_EXPORTER:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_EXPORTER
	case WCS_EFFECTSSUBCLASS_LABEL:
		{
		return (0);
		} // WCS_EFFECTSSUBCLASS_LABEL
// <<<>>> ADD_NEW_EFFECTS every effect needs a block here to tell if the effect class is implemented currently.
// If this returns 0 it will not appear in S@G but may still be loaded and saved in project files.
	default:
		{
		return (0);
		} // 
	} // switch

} // EffectsLib::ContributesToMaterials

/*===========================================================================*/

int EffectsLib::TestInitToRender(long EffectClass, short ElevationOnly)
{
int InitIt = 0;

if (EffectTypeImplemented(EffectClass))
	{
	switch (EffectClass)
		{
		case WCS_EFFECTSSUBCLASS_LAKE:
		case WCS_EFFECTSSUBCLASS_RASTERTA:
		case WCS_EFFECTSSUBCLASS_TERRAFFECTOR:
		case WCS_EFFECTSSUBCLASS_STREAM:
		//case WCS_EFFECTSSUBCLASS_TERRAINPARAM:	// did not support vector boundedness in VNS 1 or 2
		#ifdef WCS_THEMATIC_MAP
		#ifndef WCS_VECPOLY_EFFECTS
		case WCS_EFFECTSSUBCLASS_THEMATICMAP:
		#endif // WCS_VECPOLY_EFFECTS
		#endif // WCS_THEMATIC_MAP
// <<<>>> ADD_NEW_EFFECTS add only if effect can be attached to Joe and GeoRasters must be created and it affects terrain elevation
			{
			InitIt = 1;
			break;
			} // elevation only
		case WCS_EFFECTSSUBCLASS_ECOSYSTEM:
		case WCS_EFFECTSSUBCLASS_SHADOW:
		case WCS_EFFECTSSUBCLASS_FOLIAGE:
		case WCS_EFFECTSSUBCLASS_OBJECT3D:
		case WCS_EFFECTSSUBCLASS_FENCE:
		case WCS_EFFECTSSUBCLASS_GROUND:
		case WCS_EFFECTSSUBCLASS_SNOW:
		case WCS_EFFECTSSUBCLASS_ENVIRONMENT:
		#ifdef WCS_LABEL
		case WCS_EFFECTSSUBCLASS_LABEL:
		#endif // WCS_LABEL
		//case WCS_EFFECTSSUBCLASS_ILLUMINATION:	// not in current use
		//case WCS_EFFECTSSUBCLASS_CMAP:			// currently does not support vector boundedness
// <<<>>> ADD_NEW_EFFECTS add only if effect can be attached to Joe and GeoRasters must be created and it does not affect terrain elevation
			{
			InitIt = (! ElevationOnly);
			break;
			} // not elevation
		} // switch
	} // if

return (InitIt);

} // EffectsLib::TestInitToRender

/*===========================================================================*/

int EffectsLib::TestInitToRender(int EffectClass, bool ElevationOnly, bool AreaEffects, bool LineEffects)
{
int InitIt = 0;

if (EffectTypeImplemented(EffectClass))
	{
	switch (EffectClass)
		{
		case WCS_EFFECTSSUBCLASS_LAKE:
		case WCS_EFFECTSSUBCLASS_RASTERTA:
		//case WCS_EFFECTSSUBCLASS_TERRAINPARAM:	// did not support vector boundedness in VNS 1, 2 or 3
// <<<>>> ADD_NEW_EFFECTS add only if effect can be attached to Joe and applies to area and affects terrain elevation
			{
			InitIt = AreaEffects;
			break;
			} // elevation and area

		case WCS_EFFECTSSUBCLASS_TERRAFFECTOR:
		case WCS_EFFECTSSUBCLASS_STREAM:
// <<<>>> ADD_NEW_EFFECTS add only if effect can be attached to Joe and applies to lines and affects terrain elevation
			{
			InitIt = LineEffects;
			break;
			} // elevation and line

		case WCS_EFFECTSSUBCLASS_ECOSYSTEM:
		case WCS_EFFECTSSUBCLASS_SHADOW:
		case WCS_EFFECTSSUBCLASS_GROUND:
		case WCS_EFFECTSSUBCLASS_SNOW:
		case WCS_EFFECTSSUBCLASS_ENVIRONMENT:
// <<<>>> ADD_NEW_EFFECTS add only if effect can be attached to Joe and applies to area and does not affect terrain elevation
			{
			InitIt = AreaEffects && ! ElevationOnly;
			break;
			} // not elevation
		} // switch
	} // if

return (InitIt);

} // EffectsLib::TestInitToRender

/*===========================================================================*/

GeneralEffect *EffectsLib::FindByName(long EffectType, char *FindName)
{
GeneralEffect *CurEffect;

for (CurEffect = GetListPtr(EffectType); CurEffect; CurEffect = CurEffect->Next)
	{
	if (! stricmp(CurEffect->Name, FindName))
		break;
	} // for

return (CurEffect);

} // EffectsLib::FindByName

/*===========================================================================*/

GeneralEffect *EffectsLib::FindDuplicateByName(long EffectType, GeneralEffect *FindMe)
{
GeneralEffect *CurEffect;

for (CurEffect = GetListPtr(EffectType); CurEffect; CurEffect = CurEffect->Next)
	{
	if (CurEffect != FindMe)
		{
		if (! stricmp(CurEffect->Name, FindMe->Name))
			break;
		} // if
	} // for

return (CurEffect);

} // EffectsLib::FindDuplicateByName

/*===========================================================================*/

unsigned long EffectsLib::GetObjectID(long EffectType, GeneralEffect *FindMe)
{
GeneralEffect *CurEffect;
unsigned long ObjID = 0;

for (CurEffect = GetListPtr(EffectType); CurEffect; CurEffect = CurEffect->Next)
	{
	ObjID ++;
	if (CurEffect == FindMe)
		break;
	} // for

return (ObjID);

} // EffectsLib::GetObjectID

/*===========================================================================*/

GeneralEffect *EffectsLib::FindByID(long EffectType, unsigned long MatchID)
{
GeneralEffect *CurEffect;
unsigned long ObjID = 0;

for (CurEffect = GetListPtr(EffectType); CurEffect; CurEffect = CurEffect->Next)
	{
	ObjID ++;
	if (ObjID == MatchID)
		break;
	} // for

return (CurEffect);

} // EffectsLib::FindByID

/*===========================================================================*/

int EffectsLib::EnabledEffectExists(long EffectType)
{
GeneralEffect *CurEffect;

if (CurEffect = GetListPtr(EffectType))
	{
	while (CurEffect)
		{
		if (CurEffect->Enabled)
			{
			return (1);
			}// if
		CurEffect = CurEffect->Next;
		} // while
	} // if

return (0);

} // EffectsLib::EnabledEffectExists

/*===========================================================================*/

char *EffectsLib::CreateUniqueName(long EffectType, char *NameBase)
{
char NewName[WCS_EFFECT_MAXNAMELENGTH], FoundMatch = 1;
long Ct;
GeneralEffect *Effect;

if (NameBase)
	{
	if (FindByName(EffectType, NameBase))
		{
		StripExtension(NameBase);
		if (strlen(NameBase) > WCS_EFFECT_MAXNAMELENGTH - 4)
			NameBase[WCS_EFFECT_MAXNAMELENGTH - 4] = 0;
		for (Ct = 0; Ct < 9999 && FoundMatch; Ct ++)
			{
			FoundMatch = 0;
			if (Ct)
				sprintf(NewName, "%s.%d", NameBase, Ct);
			else
				sprintf(NewName, "%s", NameBase);
			Effect = GetListPtr(EffectType);
			while (Effect)
				{
				if (! stricmp(Effect->Name, NewName))
					{
					FoundMatch = 1;
					break;
					} // if
				Effect = Effect->Next;
				} // while
			} // for
		if (! FoundMatch)
			strcpy(NameBase, NewName);
		} // if
	else
		{
		NameBase[WCS_EFFECT_MAXNAMELENGTH - 1] = NULL;
		} // else
	} // if

return (NameBase);

} // EffectsLib::CreateUniqueName

/*===========================================================================*/

short EffectsLib::IsEffectValid(GeneralEffect *TestMe, char CheckChildren)
{
long EffectType;

for (EffectType = WCS_EFFECTSSUBCLASS_LAKE; EffectType < WCS_MAXIMPLEMENTED_EFFECTS; EffectType ++)
	{
	if (IsEffectValid(TestMe, EffectType, CheckChildren))
		return (1);
	} // for

return (0);

} // EffectsLib::IsEffectValid

/*===========================================================================*/

short EffectsLib::IsEffectValid(GeneralEffect *TestMe, long EffectType, char CheckChildren)
{
GeneralEffect *CurEffect;
RasterAnimHost *CurChild;

CurEffect = GetListPtr(EffectType);
while (CurEffect)
	{
	if (CurEffect == TestMe)
		{
		return (1);
		} // if
	if(CheckChildren)
		{
		CurChild = NULL;
		for (CurChild = CurEffect->GetRAHostChild(CurChild, 0); CurChild; CurChild = CurEffect->GetRAHostChild(CurChild, 0))
			{
			if (CurChild == TestMe)
				{
				return (1);
				} // if
			} // for
		} // if
	CurEffect = CurEffect->Next;
	} // while

return (0);

} // EffectsLib::IsEffectValid

/*===========================================================================*/

int EffectsLib::RemoveRAHost(RasterAnimHost *RemoveMe, int NoQuery)
{
long EffectType;
int RemoveAll = NoQuery;
GeneralEffect *CurEffect;
JoeAttribute *MyAttr;
JoeList *CurJL;
char QueryStr[256];
RasterAnimHostProperties Prop;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
RemoveMe->GetRAHostProperties(&Prop);
if (! NoQuery)
	sprintf(QueryStr, "Remove %s %s from the Project?", Prop.Name, Prop.Type);

if (NoQuery || UserMessageOKCAN(Prop.Name, QueryStr))
	{
	for (EffectType = WCS_EFFECTSSUBCLASS_LAKE; EffectType < WCS_MAXIMPLEMENTED_EFFECTS; EffectType ++)
		{
		// find the JoeCoordSys attribute and force the removal here and now
		if (EffectType == WCS_EFFECTSSUBCLASS_COORDSYS && Prop.TypeNumber >= WCS_RAHOST_OBJTYPE_VECTOR && Prop.TypeNumber <= WCS_RAHOST_OBJTYPE_CONTROLPT
			&& (MyAttr = ((Joe *)RemoveMe)->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, (unsigned char)EffectType)))
			{
			CurJL = ((JoeCoordSys *)MyAttr)->CoordsJoeList;
			((JoeCoordSys *)MyAttr)->Coord->RemoveFromJoeList(CurJL);
			} // if
		else
			{
			CurEffect = GetListPtr(EffectType);
			while (CurEffect)
				{
				if (! CurEffect->FindnRemoveRAHostChild(RemoveMe, RemoveAll))
					{
					return (0);
					} // if
				/* this may no longer be necessary, I think FindnRemoveRAHostChild() now finds all occurrences
				// of 3D objects plus it should also find ecosystems in cross-section profiles
				// Search ecosystems and other effects for foliage 3D Objects: They are embedded
				// too deeply in the RAHost hierarchy to be found and removed by FindnRemoveRAHostChild()
				if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_OBJECT3D)
					{
					if (EffectType == WCS_EFFECTSSUBCLASS_ECOSYSTEM)
						{
						if (! ((EcosystemEffect *)CurEffect)->FindnRemove3DObjects((Object3DEffect *)RemoveMe))
							return (0);
						} // if
					else if (EffectType == WCS_EFFECTSSUBCLASS_FOLIAGE)
						{
						if (! ((FoliageEffect *)CurEffect)->FindnRemove3DObjects((Object3DEffect *)RemoveMe))
							return (0);
						} // if
					else if (EffectType == WCS_EFFECTSSUBCLASS_LAKE)
						{
						if (! ((LakeEffect *)CurEffect)->FindnRemove3DObjects((Object3DEffect *)RemoveMe))
							return (0);
						} // if
					else if (EffectType == WCS_EFFECTSSUBCLASS_STREAM)
						{
						if (! ((StreamEffect *)CurEffect)->FindnRemove3DObjects((Object3DEffect *)RemoveMe))
							return (0);
						} // if
					else if (EffectType == WCS_EFFECTSSUBCLASS_GROUND)
						{
						if (! ((GroundEffect *)CurEffect)->FindnRemove3DObjects((Object3DEffect *)RemoveMe))
							return (0);
						} // if
					else if (EffectType == WCS_EFFECTSSUBCLASS_SNOW)
						{
						if (! ((SnowEffect *)CurEffect)->FindnRemove3DObjects((Object3DEffect *)RemoveMe))
							return (0);
						} // if
					else if (EffectType == WCS_EFFECTSSUBCLASS_MATERIAL)
						{
						if (! ((MaterialEffect *)CurEffect)->FindnRemove3DObjects((Object3DEffect *)RemoveMe))
							return (0);
						} // if
					else if (EffectType == WCS_EFFECTSSUBCLASS_FENCE)
						{
						if (! ((Fence *)CurEffect)->FindnRemove3DObjects((Object3DEffect *)RemoveMe))
							return (0);
						} // if
					} // if
				*/
				// ecosystems must be removed from terraffector profile data structures which are not RAHosts
				if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_ECOSYSTEM)
					{
					if (EffectType == WCS_EFFECTSSUBCLASS_TERRAFFECTOR)
						{
						if (! ((TerraffectorEffect *)CurEffect)->FindnRemoveEcosystems((EcosystemEffect *)RemoveMe))
							return (0);
						} // if
					} // if
				CurEffect = CurEffect->Next;
				} // while
			} // else not CoordSys
		} // for

	if (Prop.TypeNumber >= WCS_EFFECTSSUBCLASS_LAKE && Prop.TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
		{
		if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_OBJECT3D)
			{
			if (NoQuery || UserMessageYN(Prop.Name, "Remove associated Materials from the Project?"))
				{
				RemoveObjectMaterials((Object3DEffect *)RemoveMe);
				} // if
			} // if
		else if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_PLANETOPT)
			{
			SetPlanetOptEnabled((PlanetOpt *)RemoveMe, 0);
			} // if
		Changes[0] = MAKE_ID(RemoveMe->GetNotifyClass(), RemoveMe->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
		Changes[1] = NULL;
		RemoveEffect((GeneralEffect *)RemoveMe);
		GlobalApp->AppEx->GenerateNotify(Changes, NULL);
		} // if
	} // if

return (1);

} // EffectsLib::RemoveRAHost

/*===========================================================================*/

short EffectsLib::RemoveFromJoeLists(Joe *RemoveMe)
{
long EffectType;
short Removed = 0;
GeneralEffect *CurEffect;
JoeList *CurJL, *LastJL;
JoeAttribute *MyAttr;

for (EffectType = WCS_EFFECTSSUBCLASS_LAKE; EffectType < WCS_MAXIMPLEMENTED_EFFECTS; EffectType ++)
	{
	CurEffect = GetListPtr(EffectType);
	while (CurEffect)
		{
		// special case for coordinate systems since they tend to be attached to many Joes
		if (EffectType == WCS_EFFECTSSUBCLASS_COORDSYS)
			{
			if (MyAttr = RemoveMe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, (unsigned char)EffectType))
				{
				CurJL = ((JoeCoordSys *)MyAttr)->CoordsJoeList;
				if (CurEffect->RemoveFromJoeList(CurJL))
					Removed ++;
				} // if
			} // if
		else
			{
			CurJL = CurEffect->Joes;
			LastJL = NULL;
			while (CurJL)
				{
				RepeatFirst:
				if (CurJL->Me == RemoveMe)
					{
					if (LastJL)
						{
						LastJL->Next = CurJL->Next;
						if (CurJL->Next)
							CurJL->Next->Prev = LastJL;
						delete CurJL;
						Removed ++;
						CurJL = LastJL;
						} // if
					else
						{
						CurEffect->Joes = CurJL->Next;
						if (CurEffect->Joes)
							CurEffect->Joes->Prev = NULL;
						delete CurJL;
						Removed ++;
						CurJL = CurEffect->Joes;
						if (! CurJL)
							break;
						goto RepeatFirst;
						} // else
					} // if
				LastJL = CurJL;
				CurJL = CurJL->Next;
				} // while
			} // else
		CurEffect = CurEffect->Next;
		} // while
	} // for

return (Removed);

} // EffectsLib::RemoveFromJoeLists

/*===========================================================================*/

long EffectsLib::RemoveDBLinks(void)
{
long EffectType, Removed = 0;
GeneralEffect *CurEffect;
JoeList *CurJL;
JoeAttribute *RemoveAttr;

for (EffectType = WCS_EFFECTSSUBCLASS_LAKE; EffectType < WCS_MAXIMPLEMENTED_EFFECTS; EffectType ++)
	{
	CurEffect = GetListPtr(EffectType);
	while (CurEffect)
		{
		while (CurEffect->Joes)
			{
			CurJL = CurEffect->Joes;
			CurEffect->Joes = CurEffect->Joes->Next;
			if (RemoveAttr = CurJL->Me->RemoveEffectAttribute(WCS_JOE_ATTRIB_INTERNAL, (unsigned char)CurEffect->EffectType, CurEffect))
				delete RemoveAttr;
			delete CurJL;
			Removed ++;
			} // while
		CurEffect = CurEffect->Next;
		} // while
	} // for

return (Removed);

} // EffectsLib::RemoveDBLinks

/*===========================================================================*/

void EffectsLib::AddAttributeByList(RasterAnimHost *AddToMe, long EffectType, ThematicOwner *ThemeOwner, long ThemeNumber)
{

if (GlobalApp->GUIWins->EFL)
	{
	delete GlobalApp->GUIWins->EFL;
	} // if
if (GlobalApp->GUIWins->EFL = new EffectListGUI(this, AddToMe, EffectType, ThemeOwner, ThemeNumber))
	{
	if (GlobalApp->GUIWins->EFL->ConstructError)
		{
		delete GlobalApp->GUIWins->EFL;
		GlobalApp->GUIWins->EFL = NULL;
		return;
		} // if
	GlobalApp->GUIWins->EFL->Open(GlobalApp->MainProj);
	} // if

} // EffectsLib::AddAttributeByList


/*===========================================================================*/

void EffectsLib::ReGenerateDelayedEditNext(unsigned char SubClass, void *NotifyData)
{
GeneralEffect *Current;
int EffectClass;
short GoForward = 1;

// extract and reconstruct the parameters from the stored data
if(Current = (GeneralEffect *)NotifyData)
	{
	if(SubClass == WCS_NOTIFYSUBCLASS_REVERSE) GoForward = 0;
	EffectClass = Current->EffectType;

	// fire it off
	EditNext(GoForward, Current, EffectClass);
	} // if

} // EffectsLib::ReGenerateDelayedEditNext


/*===========================================================================*/

void EffectsLib::EditNext(short GoForward, GeneralEffect *Current, int EffectClass)
{
GeneralEffect *ListHead, *FirstEffect, *EditEffect = NULL;

ListHead = FirstEffect = GetListPtr(EffectClass);

while (FirstEffect)
	{
	if (FirstEffect == Current)
		{
		if (GoForward)
			{
			if (Current->Next)
				EditEffect = Current->Next;
			else
				EditEffect = ListHead;
			} // if
		else
			{
			if (Current->Prev)
				EditEffect = Current->Prev;
			else
				{
				EditEffect = Current;
				while (EditEffect->Next)
					EditEffect = EditEffect->Next;
				} // else
			} // else
		break;
		} // if
	FirstEffect = FirstEffect->Next;
	} // if

if (EditEffect)
	{
	EditEffect->Edit();
	RasterAnimHost::SetActiveRAHost(EditEffect);
	} // if

} // EffectsLib::EditNext

/*===========================================================================*/

short EffectsLib::AnimateShadows(void)
{
GeneralEffect *CurEffect;
long EffectType;

for (EffectType = WCS_EFFECTSSUBCLASS_LAKE; EffectType < WCS_MAXIMPLEMENTED_EFFECTS; EffectType ++)
	{
	// the current light is tested separately from CreateShadowMap()
	if (EffectType == WCS_EFFECTSSUBCLASS_LIGHT)
		continue;
	CurEffect = GetListPtr(EffectType);
	while (CurEffect)
		{
		if (CurEffect->Enabled && CurEffect->AnimateShadows())
			return (1);
		CurEffect = CurEffect->Next;
		} // while
	} // for

return (0);

} // EffectsLib::AnimateShadows

/*===========================================================================*/

bool EffectsLib::AnimateMaterials(void)
{
GeneralEffect *CurEffect;
long EffectType;

for (EffectType = WCS_EFFECTSSUBCLASS_LAKE; EffectType < WCS_MAXIMPLEMENTED_EFFECTS; EffectType ++)
	{
	if (! ContributesToMaterials(EffectType))
		continue;
	CurEffect = GetListPtr(EffectType);
	while (CurEffect)
		{
		if (CurEffect->Enabled && CurEffect->AnimateMaterials())
			return (true);
		CurEffect = CurEffect->Next;
		} // while
	} // for

return (false);

} // EffectsLib::AnimateMaterials

/*===========================================================================*/

void EffectsLib::ApplicationSetTime(double Time)
{
long EffectType, UpdateGroup;
GeneralEffect *CurEffect;

for (EffectType = WCS_EFFECTSSUBCLASS_LAKE; EffectType < WCS_MAXIMPLEMENTED_EFFECTS; EffectType ++)
	{
	UpdateGroup = 0;
	CurEffect = GetListPtr(EffectType);
	while (CurEffect)
		{
		if (CurEffect->SetToTime(Time))
			UpdateGroup = 1;
		CurEffect = CurEffect->Next;
		} // while
	if (UpdateGroup)
		{
		// <<<>>> re-initialize this effect class
		} // if
	} // for

} // EffectsLib::ApplicationSetTime

/*===========================================================================*/

void EffectsLib::GetRAHostProperties(RasterAnimHostProperties *Prop)
{
long EffectType, Found = 0;
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
GeneralEffect *CurEffect;

if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	if (Prop->ItemOperator == WCS_KEYOPERATION_ALLOBJ)
		{
		for (EffectType = WCS_EFFECTSSUBCLASS_LAKE; EffectType < WCS_MAXIMPLEMENTED_EFFECTS; EffectType ++)
			{
			for (CurEffect = GetListPtr(EffectType); CurEffect; CurEffect = CurEffect->Next)
				{
				if (CurEffect->GetKeyFrameRange(MinDist, MaxDist))
					{
					if (MinDist < TestFirst)
						TestFirst = MinDist;
					if (MaxDist > TestLast)
						TestLast = MaxDist;
					Found = 1;
					} // if
				} // for
			} // for
		} // if
	else if (Prop->ItemOperator == WCS_KEYOPERATION_OBJCLASS)
		{
		for (CurEffect = GetListPtr(Prop->TypeNumber); CurEffect; CurEffect = CurEffect->Next)
			{
			if (CurEffect->GetKeyFrameRange(MinDist, MaxDist))
				{
				if (MinDist < TestFirst)
					TestFirst = MinDist;
				if (MaxDist > TestLast)
					TestLast = MaxDist;
				Found = 1;
				} // if
			} // for
		} // else if
	if (Found)
		{
		Prop->KeyNodeRange[0] = TestFirst;
		Prop->KeyNodeRange[1] = TestLast;
		} // if
	else
		{
		Prop->KeyNodeRange[0] = Prop->KeyNodeRange[1] = 0;
		} // else
	} // if
else if (Prop->PropMask & WCS_RAHOST_MASKBIT_NEXTKEY)
	{
	if (Prop->ItemOperator == WCS_KEYOPERATION_ALLOBJ)
		{
		for (EffectType = WCS_EFFECTSSUBCLASS_LAKE; EffectType < WCS_MAXIMPLEMENTED_EFFECTS; EffectType ++)
			{
			for (CurEffect = GetListPtr(EffectType); CurEffect; CurEffect = CurEffect->Next)
				{
				if (CurEffect->GetNextAnimNode(Prop))
					{
					Found = 1;
					} // if
				} // for
			} // for
		} // if
	else if (Prop->ItemOperator == WCS_KEYOPERATION_OBJCLASS)
		{
		for (CurEffect = GetListPtr(Prop->TypeNumber); CurEffect; CurEffect = CurEffect->Next)
			{
			if (CurEffect->GetNextAnimNode(Prop))
				{
				Found = 1;
				} // if
			} // for
		} // else if
	} // if
else if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = 0;
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_ANIMATED)
		{
		if (Prop->ItemOperator == WCS_KEYOPERATION_ALLOBJ)
			{
			for (EffectType = WCS_EFFECTSSUBCLASS_LAKE; EffectType < WCS_MAXIMPLEMENTED_EFFECTS && ! Found; EffectType ++)
				{
				for (CurEffect = GetListPtr(EffectType); CurEffect; CurEffect = CurEffect->Next)
					{
					CurEffect->GetRAHostProperties(Prop);
					if (Prop->Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
						{
						Found = 1;
						break;
						} // if
					} // for
				} // for
			} // if
		else if (Prop->ItemOperator == WCS_KEYOPERATION_OBJCLASS)
			{
			for (CurEffect = GetListPtr(Prop->TypeNumber); CurEffect; CurEffect = CurEffect->Next)
				{
				CurEffect->GetRAHostProperties(Prop);
				if (Prop->Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
					break;
				} // for
			} // else if
		} // if
	} // if

} // EffectsLib::GetRAHostProperties

/*===========================================================================*/

int EffectsLib::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
long EffectType;
GeneralEffect *CurEffect;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	if (Prop->ItemOperator == WCS_KEYOPERATION_ALLOBJ)
		{
		for (EffectType = WCS_EFFECTSSUBCLASS_LAKE; EffectType < WCS_MAXIMPLEMENTED_EFFECTS; EffectType ++)
			{
			for (CurEffect = GetListPtr(EffectType); CurEffect; CurEffect = CurEffect->Next)
				{
				if (CurEffect->ScaleDeleteAnimNodes(Prop))
					{
					Success = 1;
					Changes[0] = MAKE_ID(CurEffect->GetNotifyClass(), CurEffect->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, CurEffect->GetRAHostRoot());
					GlobalApp->MCP->FrameScroll();
					} // if
				} // for
			} // for
		} // if
	else if (Prop->ItemOperator == WCS_KEYOPERATION_OBJCLASS)
		{
		for (CurEffect = GetListPtr(Prop->TypeNumber); CurEffect; CurEffect = CurEffect->Next)
			{
			if (CurEffect->ScaleDeleteAnimNodes(Prop))
				{
				Success = 1;
				Changes[0] = MAKE_ID(CurEffect->GetNotifyClass(), CurEffect->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, CurEffect->GetRAHostRoot());
				GlobalApp->MCP->FrameScroll();
				} // if
			} // for
		} // else if
	} // if

return (Success);

} // EffectsLib::SetRAHostProperties

/*===========================================================================*/

GeneralEffect *EffectsLib::GetDefaultEffect(long EffectClass, short MakeEffect, Database *DBSource)
{
GeneralEffect *List, *BestChoice = NULL;
short BestPriority = -1000;

if (List = GetListPtr(EffectClass))
	{
	while (List)
		{
		if (List->Enabled)
			{
			if (! List->Joes && ! List->Search)
				{
				if (List->Priority > BestPriority)
					{
					BestChoice = List;
					BestPriority = List->Priority;
					} // if
				} // if
			} // if
		List = List->Next;
		} // while
	} // if

if (BestChoice)
	return (BestChoice);

if (MakeEffect && (List = AddEffect(EffectClass)))
	{
	if (DBSource)
		{
		// <<<>>> might want to do some initializing here based on database
		} // if
	return (List);
	} // if

return (NULL);

} // EffectsLib::GetDefaultEffect

/*===========================================================================*/

char *EffectsLib::GetEffectTypeName(long EffectType)
{
static char *EffectTypeNames[WCS_MAXIMPLEMENTED_EFFECTS] = {"Generic", "Combo Effects", "Lakes", "Ecosystems", "", "Wave Models",
	"Cloud Models", "", "Environments", "", "Color Maps", "Illumination Effects", "",
	"Area Terraffectors", "Terraffectors", "Profiles", "Gradient Profiles", "", "", "",
	"", "", "Foliage Effects", "3D Objects", "", "Shadows", "Streams", "3D Materials",
	"", "Celestial Objects", "Starfields", "Planet Options", "Terrain Parameters", "Ground Effects", "Snow Effects",
	"Skies", "Atmospheres", "Lights", "Cameras", "Render Jobs", "Render Options", "Terrain Gridders", "Terrain Generators", 
	"Search Queries", "Thematic Maps", "Coordinate Systems", "Walls", "Post Processes", "Render Scenarios", "DEM Mergers",
	"Scene Exporters", "Labels"};
// <<<>>> ADD_NEW_EFFECTS add interface name in the plural at end of list - as it should appear in S@G

return (EffectTypeNames[EffectType]);

} // EffectsLib::GetEffectTypeName

/*===========================================================================*/

char *EffectsLib::GetEffectTypeNameNonPlural(long EffectType)
{
static char *EffectTypeNamesNonPlural[WCS_MAXIMPLEMENTED_EFFECTS] = {"Generic", "Combo Effect", "Lake", "Ecosystem", "", "Wave Model",
	"Cloud Model", "", "Environment", "", "Color Map", "Illumination Effect", "",
	"Area Terraffector", "Terraffector", "Profile", "Gradient Profile", "", "", "",
	"", "", "Foliage Effect", "3D Object", "", "Shadow", "Stream", "3D Material",
	"", "Celestial Object", "Starfield", "Planet Option", "Terrain Parameter", "Ground Effect", "Snow Effect",
	"Sky", "Atmosphere", "Light", "Camera", "Render Job", "Render Option", "Terrain Gridder", "Terrain Generator", 
	"Search Query", "Thematic Map", "Coordinate System", "Wall", "Post Process", "Render Scenario", "DEM Merger",
	"Scene Exporter", "Label"};
// <<<>>> ADD_NEW_EFFECTS add interface name in the singular at end of list

return (EffectTypeNamesNonPlural[EffectType]);

} // EffectsLib::GetEffectTypeNameNonPlural

/*===========================================================================*/

long EffectsLib::GetEffectTypeFromName(char *GroupName)
{
long Ct;

for (Ct = 1; Ct < WCS_MAXIMPLEMENTED_EFFECTS; Ct ++)
	{
	if (! stricmp(GroupName, GetEffectTypeNameNonPlural(Ct)))
		return (Ct);
	} // for

return (0);

} // EffectsLib::GetEffectTypeFromName

/*===========================================================================*/

void EffectsLib::SetShadowMapRegen(void)
{
ShadowEffect *Current;

for (Current = (ShadowEffect *)GetListPtr(WCS_EFFECTSSUBCLASS_SHADOW); Current; Current = (ShadowEffect *)Current->Next)
	{
	if (Current->UseMapFile)
		Current->RegenMapFile = 1;
	} // for

} // EffectsLib::SetShadowMapRegen

/*===========================================================================*/

void EffectsLib::SetPlanetOptEnabled(PlanetOpt *Me, short EnabledState)
{
PlanetOpt *Current;
NotifyTag Changes[2];

if (Me)
	{
	Changes[0] = MAKE_ID(Me->GetNotifyClass(), Me->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	Changes[1] = NULL;
	for (Current = (PlanetOpt *)GetListPtr(WCS_EFFECTSSUBCLASS_PLANETOPT); Current; Current = (PlanetOpt *)Current->Next)
		{
		if (Current->Enabled)
			{
			Current->Enabled = 0;
			GlobalApp->AppEx->GenerateNotify(Changes, Current->GetRAHostRoot());
			} // if
		} // for

	if (EnabledState)
		{
		Me->Enabled = 1;
		GlobalApp->AppEx->GenerateNotify(Changes, Me->GetRAHostRoot());
		} // if
	else
		{
		for (Current = (PlanetOpt *)GetListPtr(WCS_EFFECTSSUBCLASS_PLANETOPT); Current; Current = (PlanetOpt *)Current->Next)
			{
			if (Current != Me || ! Current->Next)
				{
				Current->Enabled = 1;
				GlobalApp->AppEx->GenerateNotify(Changes, Current->GetRAHostRoot());
				break;
				} // if
			} // for
		} // else
	} // if

#ifdef WCS_BUILD_VNS
if (Current = (PlanetOpt *)GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL))
	UpdateDefaultCoords(Current->Coords, TRUE);
#endif // WCS_BUILD_VNS

} // EffectsLib::SetPlanetOptEnabled

/*===========================================================================*/

void EffectsLib::MaterialNameChanging(char *OldName, char *NewName)
{
Object3DEffect *Current;

for (Current = (Object3DEffect *)GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D); Current; Current = (Object3DEffect *)Current->Next)
	{
	Current->MaterialNameChanging(OldName, NewName);
	} // for

} // EffectsLib::MaterialNameChanging

/*===========================================================================*/

void EffectsLib::RemoveObjectMaterials(Object3DEffect *PurgeMe)
{
Object3DEffect *CurrentObj;
GeneralEffect *RemoveMe;
int Found, MatNum;
NotifyTag Changes[2];

for (MatNum = 0; MatNum < PurgeMe->NumMaterials; MatNum ++)
	{
	Found = 0;
	for (CurrentObj = (Object3DEffect *)GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D); CurrentObj; CurrentObj = (Object3DEffect *)CurrentObj->Next)
		{
		if (CurrentObj == PurgeMe)
			continue;
		if (Found = CurrentObj->MatchMaterialName(PurgeMe->NameTable[MatNum].Name))
			break;
		} // for
	if (! Found)
		{
		if (RemoveMe = FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, PurgeMe->NameTable[MatNum].Name))
			{
			PurgeMe->NameTable[MatNum].Name[0] = 0;
			PurgeMe->NameTable[MatNum].Mat = NULL;
			Changes[0] = MAKE_ID(RemoveMe->GetNotifyClass(), RemoveMe->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
			Changes[1] = NULL;
			RemoveEffect(RemoveMe);
			GlobalApp->AppEx->GenerateNotify(Changes, NULL);
			} // if
		} // if
	} // for

} // EffectsLib::RemoveObjectMaterials

/*===========================================================================*/

GeneralEffect *EffectsLib::MatchNameMakeEffect(GeneralEffect *MatchMe)
{
GeneralEffect *Found;

if (GlobalApp->CopyFromEffectsLib && GlobalApp->CopyFromEffectsLib->IsEffectValid(MatchMe, MatchMe->EffectType, 0))
	{
	if (Found = FindByName(MatchMe->EffectType, MatchMe->Name))
		{
		return (Found);
		} // if
	return (AddEffect(MatchMe->EffectType, MatchMe->Name, MatchMe));
	} // if

return (NULL);

} // EffectsLib::MatchNameMakeEffect

/*===========================================================================*/

GeneralEffect *EffectsLib::MatchNameMakeEffectNoValidation(GeneralEffect *MatchMe)
{
GeneralEffect *Found;

if (Found = FindByName(MatchMe->EffectType, MatchMe->Name))
	{
	return (Found);
	} // if
return (AddEffect(MatchMe->EffectType, MatchMe->Name, MatchMe));

} // EffectsLib::MatchNameMakeEffectNoValidation

/*===========================================================================*/

void EffectsLib::SyncFloaters(Database *CurDB, Project *CurProj, int SyncResolution)
{
Camera *CurCam;
Light *CurLight;
double CellSizeNS, CellSizeWE;
float Resolution;

for (CurCam = (Camera *)GetListPtr(WCS_EFFECTSSUBCLASS_CAMERA); CurCam; CurCam = (Camera *)CurCam->Next)
	{
	if (CurCam->Floating)
		CurCam->SetFloating(1);
	} // for
for (CurLight = (Light *)GetListPtr(WCS_EFFECTSSUBCLASS_LIGHT); CurLight; CurLight = (Light *)CurLight->Next)
	{
	if (CurLight->Floating)
		CurLight->SetFloating(1);
	} // for

if (SyncResolution && CurDB->GetMinDEMCellSizeMeters(CellSizeNS, CellSizeWE, CurProj))
	{
	CellSizeNS = min(CellSizeNS, CellSizeWE);
	CellSizeNS = (int)max(CellSizeNS, 1.0);
	Resolution = (float)CellSizeNS;

	if (LakeBase.Floating)
		LakeBase.SetFloating(1, Resolution);
	if (EcosystemBase.Floating)
		EcosystemBase.SetFloating(1, Resolution);
	if (EnvironmentBase.Floating)
		EnvironmentBase.SetFloating(1, Resolution);
	if (RasterTerraffectorBase.Floating)
		RasterTerraffectorBase.SetFloating(1, Resolution);
	if (TerrainParamBase.Floating)
		TerrainParamBase.SetFloating(1, Resolution);
	if (GroundBase.Floating)
		GroundBase.SetFloating(1, Resolution);
	if (SnowBase.Floating)
		SnowBase.SetFloating(1, Resolution);
	if (TerraffectorBase.Floating)
		TerraffectorBase.SetFloating(1, Resolution);
	if (ShadowBase.Floating)
		ShadowBase.SetFloating(1, Resolution);
	if (StreamBase.Floating)
		StreamBase.SetFloating(1, Resolution);
	} // if

} // EffectsLib::SyncFloaters

/*===========================================================================*/

void EffectsLib::UpdateDefaultCoords(CoordSys *Source, int SyncAll)
{

if (DefaultCoords)
	{
	if (Source)
		{
		DefaultCoords->Datum.Copy(&DefaultCoords->Datum, &Source->Datum);
		strcpy(DefaultCoords->Name, Source->Name);
		strcpy(DefaultCoords->ProjSysName, Source->ProjSysName);
		} // if
	else
		{
		DefaultCoords->Datum.SetDatum(1);	// WCS back-compatible sphere
		strcpy(DefaultCoords->Name, "WCS Back-compatible Sphere");
		strcpy(DefaultCoords->ProjSysName, "WCS Back-compatible Sphere");
		} // else
	DefaultCoords->Geographic = (char)DefaultCoords->GetGeographic();
	DefaultCoords->Initialized = 0;
	if (SyncAll && this == GlobalApp->AppEffects)
		{
		GlobalApp->AppDB->UpdateProjectedJoeBounds();
		SyncFloaters(GlobalApp->AppDB, GlobalApp->MainProj, 1);
		} // if
	} // if

} // EffectsLib::UpdateDefaultCoords

/*===========================================================================*/

void EffectsLib::UpdateExportDefaultCoords(CoordSys *Source, int SyncAll, Database *ExportDB, Project *ExportProj)
{

if (DefaultCoords)
	{
	if (Source)
		{
		DefaultCoords->Datum.Copy(&DefaultCoords->Datum, &Source->Datum);
		strcpy(DefaultCoords->Name, Source->Name);
		strcpy(DefaultCoords->ProjSysName, Source->ProjSysName);
		} // if
	else
		{
		DefaultCoords->Datum.SetDatum(1);	// WCS back-compatible sphere
		strcpy(DefaultCoords->Name, "WCS Back-compatible Sphere");
		strcpy(DefaultCoords->ProjSysName, "WCS Back-compatible Sphere");
		} // else
	DefaultCoords->Geographic = (char)DefaultCoords->GetGeographic();
	DefaultCoords->Initialized = 0;
	if (SyncAll && this == GlobalApp->CopyToEffectsLib)
		{
		ExportDB->UpdateProjectedJoeBounds();
		SyncFloaters(ExportDB, ExportProj, 1);
		} // if
	} // if

} // EffectsLib::UpdateExportDefaultCoords

/*===========================================================================*/

int EffectsLib::SetTfxGeoClip(GeoBounds *GeoClip, Database *DBase, double EarthLatScaleMeters)
{
int Found = 0;
double HighLat, LowLat, TestLat, LatAdd, LonAdd;
TerraffectorEffect *MyEffect;
RenderJoeList *JL = NULL, *CurJL;

GeoClip->North = GeoClip->West = -FLT_MAX;
GeoClip->South = GeoClip->East = FLT_MAX;
if (DBase && (CurJL = (JL = DBase->CreateRenderJoeList(this, WCS_EFFECTSSUBCLASS_TERRAFFECTOR))))
	{
	while (CurJL)
		{
		if (CurJL->Me->TestFlags(WCS_JOEFLAG_HASKIDS) || ! CurJL->Me->TestFlags(WCS_JOEFLAG_ACTIVATED))
			{
			continue;
			} // if 
		if (MyEffect = (TerraffectorEffect *)CurJL->Effect)
			{
			// bounds overlap test - ignore effects where point falls outside effect radius + joe bounds
			HighLat = fabs(CurJL->Me->GetNorth());
			LowLat = fabs(CurJL->Me->GetSouth());
			TestLat = max(HighLat, LowLat);
			LatAdd = MyEffect->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_RADIUS].CurValue / EarthLatScaleMeters;
			LonAdd = TestLat < 90.0 ? LatAdd / cos(TestLat * PiOver180): 180.0;
			LonAdd = min(LonAdd, 180.0);
			if (GeoClip->North < CurJL->Me->GetNorth() + LatAdd)
				{
				GeoClip->North = CurJL->Me->GetNorth() + LatAdd;
				Found = 1;
				} // if
			if (GeoClip->South > CurJL->Me->GetSouth() - LatAdd)
				GeoClip->South = CurJL->Me->GetSouth() - LatAdd;
			if (GeoClip->West < CurJL->Me->GetWest() + LonAdd)
				GeoClip->West = CurJL->Me->GetWest() + LonAdd;
			if (GeoClip->East > CurJL->Me->GetEast() - LonAdd)
				GeoClip->East = CurJL->Me->GetEast() - LonAdd;
			} // if
		CurJL = (RenderJoeList *)CurJL->Next;
		} // while

	while (JL)
		{
		CurJL = (RenderJoeList *)JL->Next;
		delete JL;
		JL = CurJL;
		} // while
	} // if

if (DBase && (CurJL = (JL = DBase->CreateRenderJoeList(this, WCS_EFFECTSSUBCLASS_RASTERTA))))
	{
	while (CurJL)
		{
		if (CurJL->Me->TestFlags(WCS_JOEFLAG_HASKIDS) || ! CurJL->Me->TestFlags(WCS_JOEFLAG_ACTIVATED))
			{
			continue;
			} // if 
		if (CurJL->Effect)
			{
			if (GeoClip->North < CurJL->Me->GetNorth())
				{
				GeoClip->North = CurJL->Me->GetNorth();
				Found = 1;
				} // if
			if (GeoClip->South > CurJL->Me->GetSouth())
				GeoClip->South = CurJL->Me->GetSouth();
			if (GeoClip->West < CurJL->Me->GetWest())
				GeoClip->West = CurJL->Me->GetWest();
			if (GeoClip->East > CurJL->Me->GetEast())
				GeoClip->East = CurJL->Me->GetEast();
			} // if there truly is an effect
		CurJL = (RenderJoeList *)CurJL->Next;
		} // while

	while (JL)
		{
		CurJL = (RenderJoeList *)JL->Next;
		delete JL;
		JL = CurJL;
		} // while
	} // if

if (! Found)
	{
	GeoClip->North = GeoClip->West = GeoClip->South = GeoClip->East = 0.0;
	} // if

return (Found);

} // EffectsLib::SetTfxGeoClip

/*===========================================================================*/

#ifdef WCS_VECPOLY_EFFECTS
int EffectsLib::InitToRender(Database *DBase, Project *CurProj, RenderOpt *Opt, BufferNode *Buffers, EffectEval *EvalEffects, long Width, long Height, 
	int InitForShadows, int InitElevationEffectsOnly, int FirstInit)
#else // WCS_VECPOLY_EFFECTS
int EffectsLib::InitToRender(Database *DBase, Project *CurProj, RenderOpt *Opt, BufferNode *Buffers, long Width, long Height, 
	int InitForShadows, int InitElevationEffectsOnly, int FirstInit)
#endif // WCS_VECPOLY_EFFECTS
{

// in this f() we should determine which effects are to be rendered
// meaning are they enabled, do they have vectors attached and if so are the vectors enabled,
// is their group enabled in the render options.

if (! InitForShadows)
	{

	#ifdef WCS_VECPOLY_EFFECTS
	if (! EvalEffects->InitToRender(0, Opt->EffectEnabled, InitElevationEffectsOnly != 0, LatScale(GetPlanetRadius())))
		{
		UserMessageOK("Render Module", "Error initializing Effects.");
		return (0);
		} // if
	#endif // WCS_VECPOLY_EFFECTS

	// We need to call the database f() that inits effects attached to vectors.
	// MetersPerDegLat is needed to convert effect radius to degrees for terraffectors, streams, etc.
	// We will use the current planet radius to compute the value.

	if (! DBase->InitEffects(CurProj, this, EvalEffects, Opt->EffectEnabled, InitElevationEffectsOnly, LatScale(GetPlanetRadius())))
		{
		UserMessageOK("Render Module", "Error initializing Effects.\nIncrease allocated Effects memory in Component Library [ALT+L] and try again.");
		return (0);
		} // if

	// we need to determine if all the prerequisite object types are available and if not create
	// some default ones.

	// we need to determine what are the default objects needed by the renderer
	// such as a terrain param, environment, atmosphere. some are only needed if certain other
	// objects are enabled. For instance if terrain is disabled then we don't need terrain params.

	// we need to create Buffer nodes for any buffers required during or after rendering
	// these might include Elevation map and Slope map for reflections (or surface normals),
	// buffers needed for post-processing filters, etc.

	if (Buffers && Opt->ReflectionsEnabled)
		{
		if (! Buffers->AddBufferNode("REFLECTION", WCS_RASTER_BANDSET_FLOAT))
			return (0);
		if (! Buffers->AddBufferNode("SURFACE NORMAL X", WCS_RASTER_BANDSET_FLOAT))
			return (0);
		if (! Buffers->AddBufferNode("SURFACE NORMAL Y", WCS_RASTER_BANDSET_FLOAT))
			return (0);
		if (! Buffers->AddBufferNode("SURFACE NORMAL Z", WCS_RASTER_BANDSET_FLOAT))
			return (0);
		} // if
	} // if

if (! Opt->InitToRender(Buffers, Width, Height, FirstInit))	// pass the size that will need to be saved
	return (0);

return (1);

} // EffectsLib::InitToRender

/*===========================================================================*/

int EffectsLib::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{
long EffectType;
GeneralEffect *CurEffect;

// create default effects as required by renderer
DefaultLake = (LakeEffect *)GetDefaultEffect(WCS_EFFECTSSUBCLASS_LAKE, 0, NULL);
DefaultEnvironment = (EnvironmentEffect *)GetDefaultEffect(WCS_EFFECTSSUBCLASS_ENVIRONMENT, 1, NULL);
DefaultTerrainParam = (TerrainParamEffect *)GetDefaultEffect(WCS_EFFECTSSUBCLASS_TERRAINPARAM, 1, NULL);
DefaultGround = (GroundEffect *)GetDefaultEffect(WCS_EFFECTSSUBCLASS_GROUND, 1, NULL);

AtmoBase.InitToRender();

// see if there are any initializations needed by individual effects.
// this could include foliage chain establishment
for (EffectType = WCS_EFFECTSSUBCLASS_LAKE; EffectType < WCS_MAXIMPLEMENTED_EFFECTS; EffectType ++)
	{
	CurEffect = GetListPtr(EffectType);
	while (CurEffect)
		{
		if (! CurEffect->InitToRender(Opt, Buffers))
			return (0);
		CurEffect = CurEffect->Next;
		} // while
	} // for

return (DefaultEnvironment && DefaultTerrainParam && DefaultGround);

} // EffectsLib::InitToRender

/*===========================================================================*/

int EffectsLib::InitFrameToRender(RenderData *Rend)
{
long EffectType;
GeneralEffect *CurEffect;
VolumetricSubstance *CurSub;
CloudEffect *CurCloud;

if (! Rend->Cam->InitFrameToRender(this, Rend))
	return (0);

for (EffectType = WCS_EFFECTSSUBCLASS_LAKE; EffectType < WCS_MAXIMPLEMENTED_EFFECTS; EffectType ++)
	{
	if (EffectType == WCS_EFFECTSSUBCLASS_CAMERA)
		continue;
	CurEffect = GetListPtr(EffectType);
	while (CurEffect)
		{
		if (CurEffect->Enabled)
			{
			if (! CurEffect->InitFrameToRender(this, Rend))
				return (0);
			} // if
		CurEffect = CurEffect->Next;
		} // while
	} // for

// create a list of volumetric things
// this could be done in render init
if (Rend->Substances = BuildVolumeSubstanceList(Rend->Opt))
	{
	// do something to set up bounding conditions of each thing for ray hit testing
	// this could be done in render init
	CurSub = Rend->Substances;
	while (CurSub)
		{
		CurSub->ComputeBoundingBox(Rend->DefCoords, Rend->PlanetRad, Rend->TexRefLat, Rend->TexRefLon);
		CurSub = CurSub->NextSub;
		} // while
	// initialize volumetric cloud models
	// allocate an array of VertexCloud for each Cloud
	CurCloud = (CloudEffect *)GetListPtr(WCS_EFFECTSSUBCLASS_CLOUD);
	while (CurCloud)
		{
		if (CurCloud->VolumeSub)
			{
			if (! CurCloud->AllocVertices())
				{
				return (0);
				} // if
			// evaluate density at each vertex and put value (0-1) in vertex->xyz.
			// xyz[0] has the normalized amplitude for waves
			// xyz[1] has the intensity factor based on falloff and vectors
			CurCloud->EvalVertices(Rend->RenderTime, Rend->EarthLatScaleMeters);
			} // if
		CurCloud = (CloudEffect *)CurCloud->Next;
		} // while
	} // if

return (InitBaseToRender());

} // EffectsLib::InitFrameToRender

/*===========================================================================*/

int EffectsLib::InitBaseToRender(void)
{

if (! Object3DBase.InitFrameToRender())
	return (0);
if (! FoliageBase.InitFrameToRender())
	return (0);
if (! FnceBase.InitFrameToRender())
	return (0);
if (! LablBase.InitFrameToRender())
	return (0);

return (1);

} // EffectsLib::InitBaseToRender

/*===========================================================================*/

char EffectsLib::GetMaxFractalDepth(void)
{
TerrainParamEffect *CurEffect;
JoeList *CurJoe;
char Enabled, MaxFract = 0;

if (CurEffect = (TerrainParamEffect *)GetListPtr(WCS_EFFECTSSUBCLASS_TERRAINPARAM))
	{
	while (CurEffect)
		{
		if (CurEffect->Enabled && CurEffect->FractalDepth > MaxFract)
			{
			if (CurJoe = CurEffect->Joes)
				{
				Enabled = 0;
				while (CurJoe)
					{
					if (CurJoe->Me && CurJoe->Me->TestFlags(WCS_JOEFLAG_ACTIVATED))
						{
						Enabled = 1;
						break;
						} // if
					CurJoe = CurJoe->Next;
					} // while
				} // if
			else
				Enabled = 1;
			if (Enabled)
				MaxFract = CurEffect->FractalDepth;
			} // if
		CurEffect = (TerrainParamEffect *)CurEffect->Next;
		} // while
	} // if

return (MaxFract);

} // EffectsLib::GetMaxFractalDepth

/*===========================================================================*/

int EffectsLib::AreThereVolumetricAtmospheres(void)
{
Atmosphere *CurEffect;
AtmosphereComponent *CurComponent;

if (CurEffect = (Atmosphere *)GetListPtr(WCS_EFFECTSSUBCLASS_ATMOSPHERE))
	{
	while (CurEffect)
		{
		if (CurEffect->Enabled && CurEffect->AtmosphereType == WCS_EFFECTS_ATMOSPHERETYPE_VOLUMETRIC &&
			CurEffect->Components)
			{
			for (CurComponent = CurEffect->Components; CurComponent; CurComponent = CurComponent->Next)
				{
				if (CurComponent->Enabled)
					return (1);
				} // for
			} // if
		CurEffect = (Atmosphere *)CurEffect->Next;
		} // while
	} // if

return (0);

} // EffectsLib::AreThereVolumetricAtmospheres

/*===========================================================================*/

int EffectsLib::AreThereLightTextures(void)
{
Light *CurEffect;

if (CurEffect = (Light *)GetListPtr(WCS_EFFECTSSUBCLASS_LIGHT))
	{
	while (CurEffect)
		{
		if (CurEffect->Enabled && CurEffect->GetTexRootPtr(WCS_EFFECTS_LIGHT_TEXTURE_COLOR) &&
			CurEffect->GetTexRootPtr(WCS_EFFECTS_LIGHT_TEXTURE_COLOR)->Enabled)
			{
			return (1);
			} // if
		CurEffect = (Light *)CurEffect->Next;
		} // while
	} // if

return (0);

} // EffectsLib::AreThereLightTextures

/*===========================================================================*/

int EffectsLib::AreThereAtmoTextures(void)
{
Atmosphere *CurEffect;

if (CurEffect = (Atmosphere *)GetListPtr(WCS_EFFECTSSUBCLASS_ATMOSPHERE))
	{
	while (CurEffect)
		{
		if (CurEffect->Enabled && CurEffect->AtmosphereType != WCS_EFFECTS_ATMOSPHERETYPE_VOLUMETRIC &&
			(CurEffect->HazeEnabled && CurEffect->GetTexRootPtr(WCS_EFFECTS_ATMOSPHERE_TEXTURE_HAZECOLOR) &&
			CurEffect->GetTexRootPtr(WCS_EFFECTS_ATMOSPHERE_TEXTURE_HAZECOLOR)->Enabled) ||
			(CurEffect->HazeEnabled && CurEffect->SeparateCloudHaze && CurEffect->GetTexRootPtr(WCS_EFFECTS_ATMOSPHERE_TEXTURE_CLOUDHAZECOLOR) &&
			CurEffect->GetTexRootPtr(WCS_EFFECTS_ATMOSPHERE_TEXTURE_CLOUDHAZECOLOR)->Enabled) ||
			(CurEffect->FogEnabled && CurEffect->GetTexRootPtr(WCS_EFFECTS_ATMOSPHERE_TEXTURE_FOGCOLOR) &&
			CurEffect->GetTexRootPtr(WCS_EFFECTS_ATMOSPHERE_TEXTURE_FOGCOLOR)->Enabled))
			{
			return (1);
			} // if
		CurEffect = (Atmosphere *)CurEffect->Next;
		} // while
	} // if

return (0);

} // EffectsLib::AreThereAtmoTextures

/*===========================================================================*/

int EffectsLib::AreThereOpticallyTransparentEffects(void)
{
LakeEffect *CurLake;
StreamEffect *CurStream;

if (CurLake = (LakeEffect *)GetListPtr(WCS_EFFECTSSUBCLASS_LAKE))
	{
	while (CurLake)
		{
		if (CurLake->Enabled && CurLake->IsThereOpticallyTransparentMaterial())
			{
			return (1);
			} // if
		CurLake = (LakeEffect *)CurLake->Next;
		} // while
	} // if

if (CurStream = (StreamEffect *)GetListPtr(WCS_EFFECTSSUBCLASS_STREAM))
	{
	while (CurStream)
		{
		if (CurStream->Enabled && CurStream->IsThereOpticallyTransparentMaterial())
			{
			return (1);
			} // if
		CurStream = (StreamEffect *)CurStream->Next;
		} // while
	} // if

return (0);

} // EffectsLib::AreThereOpticallyTransparentEffects

/*===========================================================================*/

VolumetricSubstance *EffectsLib::BuildVolumeSubstanceList(RenderOpt *Opt)
{
Atmosphere *CurAtmo;
AtmosphereComponent *CurComp;
CloudEffect *CurCloud;
VolumetricSubstance *Substances = NULL, **CurSubPtr;

CurSubPtr = &Substances;

if (Opt->VolumetricsEnabled)
	{
	if (Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE])
		{
		if (CurAtmo = (Atmosphere *)GetListPtr(WCS_EFFECTSSUBCLASS_ATMOSPHERE))
			{
			while (CurAtmo)
				{
				if (CurAtmo->Enabled && CurAtmo->AtmosphereType == WCS_EFFECTS_ATMOSPHERETYPE_VOLUMETRIC)
					{
					CurComp = CurAtmo->Components;
					while (CurComp)
						{
						if (CurComp->Enabled)
							{
							*CurSubPtr = CurComp;
							(*CurSubPtr)->NextSub = NULL;
							CurSubPtr = &(*CurSubPtr)->NextSub;
							} // if
						CurComp = CurComp->Next;
						} // while
					} // if
				CurAtmo = (Atmosphere *)CurAtmo->Next;
				} // while
			} // if
		} // if
	if (Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_CLOUD])
		{
		if (CurCloud = (CloudEffect *)GetListPtr(WCS_EFFECTSSUBCLASS_CLOUD))
			{
			while (CurCloud)
				{
				if (CurCloud->VolumeSub)
					{
					*CurSubPtr = CurCloud;
					(*CurSubPtr)->NextSub = NULL;
					CurSubPtr = &(*CurSubPtr)->NextSub;
					} // if
				CurCloud = (CloudEffect *)CurCloud->Next;
				} // while
			} // if
		} // if

	} // if

return (Substances);

} // EffectsLib::BuildVolumeSubstanceList

/*===========================================================================*/

void EffectsLib::InitScenarios(EffectList *UsedScenarios, double RenderTime, Database *DBHost)
{
int RegenShadowMaps = 0, TempRegenShadowMaps = 0;
EffectList *CurScenario = UsedScenarios;
ShadowEffect *CurShadow;

while (CurScenario)
	{
	if (CurScenario->Me)
		((RenderScenario *)CurScenario->Me)->SetupToRender(RenderTime, TempRegenShadowMaps, DBHost);
	if (TempRegenShadowMaps)
		RegenShadowMaps = 1;
	CurScenario = CurScenario->Next;
	} // while

if (RegenShadowMaps)
	{
	for (CurShadow = (ShadowEffect *)GetListPtr(WCS_EFFECTSSUBCLASS_SHADOW); CurShadow; CurShadow = (ShadowEffect *)CurShadow->Next)
		{
		if (CurShadow->Enabled && CurShadow->UseMapFile)
			CurShadow->RegenMapFile = 1;
		} // for
	} // if

} // EffectsLib::InitScenarios

/*===========================================================================*/

void EffectsLib::RestoreScenarios(EffectList *UsedScenarios, Database *DBHost)
{
EffectList *CurScenario, *BackScenarios = NULL, *TempScenario;
int RunListBackward;

if (RunListBackward = (UsedScenarios && UsedScenarios->Next))	// if more than one scenario
	{
	// build a backwards scenario list
	// scenario items must be restored to their original states in opposite order so that if two scenarios
	// affect the same item it will be restored correctly.
	CurScenario = UsedScenarios;
	while (CurScenario)
		{
		if (TempScenario = new EffectList)
			{
			TempScenario->Me = CurScenario->Me;
			if (BackScenarios)
				{
				TempScenario->Next = BackScenarios;
				BackScenarios = TempScenario;
				} // if
			else
				BackScenarios = TempScenario;
			} // if
		else
			{
			RunListBackward = 0;
			break;
			} // else
		CurScenario = CurScenario->Next;
		} // while
	} // if

if (RunListBackward)
	{
	// run backwards list.
	CurScenario = BackScenarios;
	} // if
else
	{
	// run list forwards
	CurScenario = UsedScenarios;
	} // else

while (CurScenario)
	{
	if (CurScenario->Me)
		((RenderScenario *)CurScenario->Me)->CleanupFromRender(DBHost);
	CurScenario = CurScenario->Next;
	} // while

// delete backwards list
while (BackScenarios)
	{
	CurScenario = BackScenarios->Next;
	delete BackScenarios;
	BackScenarios = CurScenario;
	} // while

} // EffectsLib::RestoreScenarios

/*===========================================================================*/

int EffectsLib::UpdateScenarios(EffectList *ProcessScenarios, double RenderTime, Database *DBHost)
{
int RegenShadowMaps = 0, TempRegenShadowMaps = 0, ReInit = 0;
ShadowEffect *CurShadow;
EffectList *CurScenario;

for (CurScenario = ProcessScenarios; CurScenario; CurScenario = CurScenario->Next)
	{
	if (CurScenario->Me)
		{
		if (((RenderScenario *)CurScenario->Me)->ProcessFrameToRender(RenderTime, TempRegenShadowMaps, DBHost))
			ReInit = 1;
		if (TempRegenShadowMaps)
			RegenShadowMaps = 1;
		} // if
	} // for

if (RegenShadowMaps)
	{
	for (CurShadow = (ShadowEffect *)GetListPtr(WCS_EFFECTSSUBCLASS_SHADOW); CurShadow; CurShadow = (ShadowEffect *)CurShadow->Next)
		{
		if (CurShadow->Enabled && CurShadow->UseMapFile)
			CurShadow->RegenMapFile = 1;
		} // for
	} // if

return (ReInit);

} // EffectsLib::UpdateScenarios

/*===========================================================================*/

long EffectsLib::CountEffects(long EffectClass, int EnabledOnly)
{
long Count = 0;
GeneralEffect *CurEffect;

if (CurEffect = GetListPtr(EffectClass))
	{
	while (CurEffect)
		{
		if (! EnabledOnly || CurEffect->Enabled)
			Count ++;
		CurEffect = CurEffect->Next;
		} // while
	} // if

return (Count);

} // EffectsLib::CountEffects

/*===========================================================================*/

long EffectsLib::CloneEffects(EffectsLib *EffectsHost, long EffectClass, int EnabledOnly)
{
GeneralEffect *CurEffect;
long Success = 1;

if (CurEffect = EffectsHost->GetListPtr(EffectClass))
	{
	while (CurEffect)
		{
		if (! EnabledOnly || CurEffect->Enabled)
			{
			if (! (AddEffect(EffectClass, CurEffect->Name, CurEffect)))
				{
				// memory error
				Success = 0;
				break;
				} // if
			} // if
		CurEffect = CurEffect->Next;
		} // while
	} // if

return (Success);

} // EffectsLib::CloneEffects

/*===========================================================================*/

GeneralEffect *EffectsLib::CloneNamedEffect(EffectsLib *EffectsHost, long EffectClass, char *FXName)
{
GeneralEffect *CurEffect, *Result = NULL;

if (CurEffect = EffectsHost->FindByName(EffectClass, FXName))
	{
	Result = AddEffect(EffectClass, FXName, CurEffect);
	} // if

return (Result);

} // EffectsLib::CloneNamedEffect

/*===========================================================================*/

void EffectsLib::SortEffectsByAlphabet(long EffectClass)
{
long NumEffects, EffectCt, FirstFound, LowFound, FirstCheck;
GeneralEffect **EffectPointers, *CurEffect;
NotifyTag Changes[2];

// count images
if ((NumEffects = CountEffects(EffectClass, false)) > 1)
	{
	// alloc an array
	if (EffectPointers = (GeneralEffect **)AppMem_Alloc(NumEffects * sizeof (GeneralEffect *), APPMEM_CLEAR))
		{
		// copy pointers to array
		for (CurEffect = GetListPtr(EffectClass), EffectCt = 0; EffectCt < NumEffects && CurEffect; EffectCt ++, CurEffect = CurEffect->Next)
			{
			EffectPointers[EffectCt] = CurEffect;
			} // while

		// sort array
		for (FirstFound = 0; FirstFound < NumEffects - 1; FirstFound ++)
			{
			LowFound = FirstFound;
			for (FirstCheck = FirstFound + 1; FirstCheck < NumEffects; FirstCheck ++)
				{
				if (stricmp(EffectPointers[LowFound]->Name, EffectPointers[FirstCheck]->Name) > 0)
					{
					LowFound = FirstCheck;
					} // if
				} // for
			if (LowFound > FirstFound)
				swmem(&EffectPointers[LowFound], &EffectPointers[FirstFound], sizeof (GeneralEffect *));
			} // for

		// allocate Next pointers
		SetListHead(EffectClass, EffectPointers[0]);
		for (EffectCt = 0; EffectCt < NumEffects - 1; EffectCt ++)
			{
			EffectPointers[EffectCt]->Next = EffectPointers[EffectCt + 1];
			} // for
		for (EffectCt = 1; EffectCt < NumEffects; EffectCt ++)
			{
			EffectPointers[EffectCt]->Prev = EffectPointers[EffectCt - 1];
			} // for
		EffectPointers[0]->Prev = NULL;
		EffectPointers[NumEffects - 1]->Next = NULL;
		SetListTail(EffectClass, EffectPointers[NumEffects - 1]);

		AppMem_Free(EffectPointers, NumEffects * sizeof (GeneralEffect *));

		// send notify
		Changes[0] = MAKE_ID(EffectClass, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, NULL);
		} // if
	} // if

} // EffectsLib::SortEffectsByAlphabet

/*===========================================================================*/

int EffectsLib::GetEffectTypePrecedence(long EffectType)
{
int Precedence;

// terrain parameter - no
// area tfx
// linear tfx
// lake
// stream
// environment - no
// ecosystem
// ground
// snow
// shadow - no

// Note: terrain parameter, shadows and environments are likely to bound large regions and take a lot of time to merge 
// with other vector polygons. Also they tend to be effects that do not have very precise needs for user control over the
// edges of the effect area. They have been elliminated from the list here so that they will continue to generate
// raster type effect evaluation as in VNS 2. The need for a resolution will have to be solved so that the program
// can figure it out without user intervention.

switch (EffectType)
	{
	//case WCS_EFFECTSSUBCLASS_TERRAINPARAM:
	//	{
	//	Precedence = 9;
	//	break;
	//	} // WCS_EFFECTSSUBCLASS_TERRAINPARAM
	case WCS_EFFECTSSUBCLASS_RASTERTA:
		{
		Precedence = 8;
		break;
		} // WCS_EFFECTSSUBCLASS_RASTERTA
	case WCS_EFFECTSSUBCLASS_TERRAFFECTOR:
		{
		Precedence = 7;
		break;
		} // WCS_EFFECTSSUBCLASS_TERRAFFECTOR
	case WCS_EFFECTSSUBCLASS_LAKE:
		{
		Precedence = 6;
		break;
		} // WCS_EFFECTSSUBCLASS_LAKE
	case WCS_EFFECTSSUBCLASS_STREAM:
		{
		Precedence = 5;
		break;
		} // WCS_EFFECTSSUBCLASS_STREAM
	//case WCS_EFFECTSSUBCLASS_ENVIRONMENT:
	//	{
	//	Precedence = 4;
	//	break;
	//	} // WCS_EFFECTSSUBCLASS_ENVIRONMENT
	case WCS_EFFECTSSUBCLASS_ECOSYSTEM:
		{
		if (GroupVecPolyEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM])
			Precedence = 3;
		else
			Precedence = -1;
		break;
		} // WCS_EFFECTSSUBCLASS_ECOSYSTEM
	case WCS_EFFECTSSUBCLASS_GROUND:
		{
		Precedence = 2;
		break;
		} // WCS_EFFECTSSUBCLASS_GROUND
	case WCS_EFFECTSSUBCLASS_SNOW:
		{
		Precedence = 1;
		break;
		} // WCS_EFFECTSSUBCLASS_SNOW
	//case WCS_EFFECTSSUBCLASS_SHADOW:
	//	{
	//	Precedence = 0;
	//	break;
	//	} // WCS_EFFECTSSUBCLASS_SHADOW
	default:
		{
		Precedence = -1;
		break;
		} // all the rest
	// <<<>>> ADD_NEW_EFFECTS add case if the effect can can be linked to vectors and covers area (tfx, streams included)
	} // switch

return (Precedence);

} // EffectsLib::GetEffectTypePrecedence

/*===========================================================================*/

void EffectsLib::SortEffectChain(long &EffectCt, long EffectType)
{
long MoreToSort, EJLCt, NextCt;
bool SameEffectAndVector;

MoreToSort = 1;
while (MoreToSort)
	{
	MoreToSort = 0;
	for (EJLCt = 0, NextCt = 1; NextCt < EffectCt; ++NextCt)
		{
		if (EffectChain[NextCt]->MyEffect->Priority > EffectChain[EJLCt]->MyEffect->Priority)
			{
			SwapV((void **)&EffectChain[NextCt], (void **)&EffectChain[EJLCt]);
			MoreToSort = 1;
			} // if
		else if (EffectChain[NextCt]->MyEffect->Priority == EffectChain[EJLCt]->MyEffect->Priority)
			{
			if (EffectType == WCS_EFFECTSSUBCLASS_TERRAFFECTOR)
				{
				if (((TerraffectorEffect *)EffectChain[NextCt]->MyEffect)->EvalOrder < ((TerraffectorEffect *)EffectChain[EJLCt]->MyEffect)->EvalOrder)
					{
					SwapV((void **)&EffectChain[NextCt], (void **)&EffectChain[EJLCt]);
					MoreToSort = 1;
					} // if
				else if (((TerraffectorEffect *)EffectChain[NextCt]->MyEffect)->EvalOrder == ((TerraffectorEffect *)EffectChain[EJLCt]->MyEffect)->EvalOrder)
					{
					if (EffectChain[NextCt]->MyEffect < EffectChain[EJLCt]->MyEffect)
						{
						SwapV((void **)&EffectChain[NextCt], (void **)&EffectChain[EJLCt]);
						MoreToSort = 1;
						} // if
					} // if
				} // if terraffector
			else if (EffectType == WCS_EFFECTSSUBCLASS_RASTERTA)
				{
				if (((RasterTerraffectorEffect *)EffectChain[NextCt]->MyEffect)->EvalOrder < ((RasterTerraffectorEffect *)EffectChain[EJLCt]->MyEffect)->EvalOrder)
					{
					SwapV((void **)&EffectChain[NextCt], (void **)&EffectChain[EJLCt]);
					MoreToSort = 1;
					} // if
				else if (((RasterTerraffectorEffect *)EffectChain[NextCt]->MyEffect)->EvalOrder == ((RasterTerraffectorEffect *)EffectChain[EJLCt]->MyEffect)->EvalOrder)
					{
					if (EffectChain[NextCt]->MyEffect < EffectChain[EJLCt]->MyEffect)
						{
						SwapV((void **)&EffectChain[NextCt], (void **)&EffectChain[EJLCt]);
						MoreToSort = 1;
						} // if
					} // if
				} // area tfx
			else
				{
				if (EffectChain[NextCt]->MyEffect < EffectChain[EJLCt]->MyEffect)
					{
					SwapV((void **)&EffectChain[NextCt], (void **)&EffectChain[EJLCt]);
					MoreToSort = 1;
					} // if
				} // else
			} // else if
		EJLCt = NextCt;
		} // for
	} // while
	
// remove duplicates
for (EJLCt = 0, NextCt = 1; NextCt < EffectCt; ++NextCt)
	{
	if (EffectChain[NextCt]->MyEffect == EffectChain[EJLCt]->MyEffect)
		{
		if (EffectChain[NextCt]->Identical(EffectChain[EJLCt], SameEffectAndVector))
			EffectChain[NextCt] = NULL;
		else
			EJLCt = NextCt;
		} // if
	else
		EJLCt = NextCt;
	} // for
if (EJLCt == 0)
	EffectCt = 1;
	
} // EffectsLib::SortEffectChain

/*===========================================================================*/

void EffectsLib::LogInitMemoryUsage(int Category, long UsedMemStart, long UsedMemEnd)
{
char MemMsg[256];

if (Category >= 0)
	sprintf(MemMsg, "%s Memory - %.3f Mb", GetEffectTypeNameNonPlural(Category), (UsedMemEnd - UsedMemStart) / 1000000.0);
else
	sprintf(MemMsg, "Total Effects Memory - %.3f Mb", (UsedMemEnd - UsedMemStart) / 1000000.0);
GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_DTA, MemMsg);

} // EffectsLib::LogInitMemoryUsage

/*===========================================================================*/

void EffectsLib::EvalRasterTerraffectors(RenderData *Rend, VertexData *Vert, short AfterFractals)
{
short Found = 0;

if (AreThereRasterTerraffectors())
	{
	Found = RasterTerraffectorBase.Eval(Rend, Vert, AfterFractals);
	} // if

} // EffectsLib::EvalRasterTerraffectors

/*===========================================================================*/

void EffectsLib::EvalTerraffectors(RenderData *Rend, VertexData *Vert)
{

if (AreThereTerraffectors())
	{
	TerraffectorBase.Eval(Rend, Vert);
	} // if

} // EffectsLib::EvalTerraffectors

/*===========================================================================*/

void EffectsLib::EvalStreams(RenderData *Rend, VertexData *Vert)
{

if (AreThereStreams())
	{
	StreamBase.Eval(Rend, Vert);
	} // if

if (Vert->Stream)
	{
	Vert->WaterDepth = Vert->WaterElev - Vert->Elev;
	} // if

} // EffectsLib::EvalStreams

/*===========================================================================*/

void EffectsLib::EvalLakes(RenderData *Rend, VertexData *Vert)
{
int Found = 0;
double CurLevel, OldWaterElev, CurWaveHt = 0.0;
LakeEffect *CurEffect;
TwinMaterials BaseMat;

if (AreThereLakes())
	{
	Found = LakeBase.Eval(Rend, Vert);
	} // if
else
	LakeBase.CurBeach = NULL;

if (! Found && (CurEffect = Lake))
	{
	while (CurEffect)
		{
		if (CurEffect->Enabled && ! CurEffect->Joes && ! CurEffect->Search && ! CurEffect->Theme[WCS_EFFECTS_LAKE_THEME_ELEV])
			{
			if (Vert->Elev < CurEffect->MaxLevel)
				{
				//get the right materials, texture evaluator needs water level
				OldWaterElev = Vert->WaterElev;
				Vert->WaterElev = CurEffect->Level;
				if (CurEffect->WaterMat.GetRenderMaterial(&BaseMat, CurEffect->GetRenderWaterMatGradientPos(Rend, Vert)))
					{
					Vert->WaterElev = OldWaterElev;
					CurLevel = CurEffect->Level + BaseMat.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WATERDEPTH].CurValue * BaseMat.Covg[0];
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
							Vert->Lake = CurEffect;
							Vert->Flags |= WCS_VERTEXDATA_FLAG_WAVEAPPLIED;
							} // if
						} // if
					else
						{
						Vert->WaterElev = CurLevel;
						Vert->WaveHeight = CurWaveHt;
						Vert->Lake = CurEffect;
						Vert->Flags |= WCS_VERTEXDATA_FLAG_WAVEAPPLIED;
						} // else
					} // if material found
				else
					{
					Vert->WaterElev = OldWaterElev;
					} // else
				if (LakeBase.CompareBeach(Rend, Vert, CurEffect, CurEffect->MaxBeachLevel))
					LakeBase.CurBeach = &CurEffect->BeachMat;
				} // if
			} // if
		CurEffect = (LakeEffect *)CurEffect->Next;
		} // while
	if (LakeBase.CurBeach && (! Vert->Beach || LakeBase.TempBeachLevel < Vert->BeachLevel))
		{
		Vert->Beach = LakeBase.CurBeach;
		Vert->BeachLevel = LakeBase.TempBeachLevel;
		} // if
	} // if

if (Vert->Lake)
	{
	Vert->WaterDepth = Vert->WaterElev - Vert->Elev;
	} // if

} // EffectsLib::EvalLakes

/*===========================================================================*/

void EffectsLib::EvalEcosystems(PolygonData *Poly)
{

if (AreThereEcosystems())
	{
	EcosystemBase.Eval(Poly);
	} // if

} // EffectsLib::EvalEcosystems

/*===========================================================================*/

void EffectsLib::EvalCmaps(RenderData *Rend, PolygonData *Poly)
{

CmapBase.Eval(Rend, Poly);

} // EffectsLib::EvalCmaps

/*===========================================================================*/
#ifdef WCS_VECPOLY_EFFECTS
void EffectsLib::EvalEnvironment(RenderData *Rend, VectorNode *CurNode)
{
bool Done = false;

if (AreThereEnvironments())
	{
	Done = EnvironmentBase.Eval(Rend, CurNode);
	} // if

if (! Done)
	{
	CurNode->NodeData->NodeEnvironment = DefaultEnvironment;
	} // if

} // EffectsLib::EvalEnvironment

/*===========================================================================*/
#else // WCS_VECPOLY_EFFECTS
void EffectsLib::EvalEnvironment(PolygonData *Poly)
{
short Found = 0;

if (AreThereEnvironments())
	{
	Found = EnvironmentBase.Eval(Poly);
	} // if

if (! Found)
	{
	Poly->Env = DefaultEnvironment;
	} // if

} // EffectsLib::EvalEnvironment
#endif // WCS_VECPOLY_EFFECTS
/*===========================================================================*/

void EffectsLib::EvalSnow(PolygonData *Poly)
{
short Found = 0;
SnowEffect *CurEffect;

if (AreThereSnows())
	{
	Found = SnowBase.Eval(Poly);
	} // if

if (! Found && (CurEffect = Snow))
	{
	while (CurEffect)
		{
		if (CurEffect->Enabled && ! CurEffect->Joes && ! CurEffect->Search)
			{
			Poly->Snow = CurEffect;
			} // if
		CurEffect = (SnowEffect *)CurEffect->Next;
		} // while
	} // if

} // EffectsLib::EvalSnow

/*===========================================================================*/

void EffectsLib::EvalGround(PolygonData *Poly)
{
short Found = 0;

if (AreThereGrounds())
	{
	Found = GroundBase.Eval(Poly);
	} // if

if (! Found)
	{
	Poly->Ground = DefaultGround;
	} // if

} // EffectsLib::EvalGround

/*===========================================================================*/

void EffectsLib::EvalShadows(RenderData *Rend, PolygonData *Poly)
{
short Found = 0;
double TexOpacity, Value[3];
ShadowEffect *CurEffect;

if (AreThereShadows())
	{
	Found = ShadowBase.Eval(Rend, Poly);
	} // if

if (! Found && (CurEffect = Shadow))
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	while (CurEffect)
		{
		if (CurEffect->Enabled && ! CurEffect->Joes && CurEffect->ShadowFlags)
			{
			Poly->ShadowFlags = CurEffect->ShadowFlags;
			Poly->ReceivedShadowIntensity = CurEffect->AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_INTENSITY].CurValue;
			if (CurEffect->TexRoot[WCS_EFFECTS_SHADOW_TEXTURE_INTENSITY] && CurEffect->TexRoot[WCS_EFFECTS_SHADOW_TEXTURE_INTENSITY]->Enabled)
				{
				if ((TexOpacity = CurEffect->TexRoot[WCS_EFFECTS_SHADOW_TEXTURE_INTENSITY]->Eval(Value, Rend->TransferTextureData(Poly))) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						Poly->ReceivedShadowIntensity *= (1.0 - TexOpacity + Value[0]);
						} // if
					else
						Poly->ReceivedShadowIntensity *= Value[0];
					} // if
				} // if
			Poly->ShadowOffset = CurEffect->AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_SHADOWOFFSET].CurValue;
			break;
			} // if
		CurEffect = (ShadowEffect *)CurEffect->Next;
		} // while
	} // if

} // EffectsLib::EvalShadows

/*===========================================================================*/

void EffectsLib::EvalHighLowEffectExtrema(VectorPolygonListDouble *ListOfPolygons, RenderData *Rend, double &RelativeEffectAdd,
	double &RelativeEffectSubtract, double &AbsoluteEffectMax, double &AbsoluteEffectMin)
{
double MaximumMod, MinimumMod, ThemedWaterLevel, ThemedBeachLevel, ATFXRelativeIncrease, ATFXRelativeDecrease, LTFXRelativeIncrease, LTFXRelativeDecrease;
VectorPolygonListDouble *CurVPList;
EffectJoeList *CurEffectList;
Joe *CurVec;

RelativeEffectAdd = RelativeEffectSubtract = ATFXRelativeIncrease = ATFXRelativeDecrease = 
	LTFXRelativeIncrease = LTFXRelativeDecrease = 0.0;
AbsoluteEffectMax = -FLT_MAX;
AbsoluteEffectMin = FLT_MAX;

for (CurVPList = ListOfPolygons; CurVPList; CurVPList = (VectorPolygonListDouble *)CurVPList->NextPolygonList)
	{
	for (CurEffectList = CurVPList->MyPolygon->MyEffects; CurEffectList; CurEffectList = CurEffectList->Next)
		{
		CurVec = CurEffectList->MyJoe;
		switch (CurEffectList->MyEffect->EffectType)
			{
			case WCS_EFFECTSSUBCLASS_LAKE:
				{
				LakeEffect *CurLake = (LakeEffect *)CurEffectList->MyEffect;
				// lake elevation will be the one variable to consider here
				// elevation might come from a thematic map
				LakeBase.GetThemedMaxLevel(Rend, CurLake, CurVec, ThemedWaterLevel, ThemedBeachLevel);
					
				// find range of material water depths and wave heights
				MaximumMod = MinimumMod = 0.0;
				CurLake->GetWaterDepthAndWaveRange(MaximumMod, MinimumMod);
				MaximumMod += ThemedWaterLevel;
				MinimumMod += ThemedWaterLevel;
				
				if (MaximumMod > AbsoluteEffectMax)
					AbsoluteEffectMax = MaximumMod;
				if (MinimumMod < AbsoluteEffectMin)
					AbsoluteEffectMin = MinimumMod;
				break;
				} // WCS_EFFECTSSUBCLASS_LAKE
			case WCS_EFFECTSSUBCLASS_STREAM:
				{
				StreamEffect *CurStream = (StreamEffect *)CurEffectList->MyEffect;
				float MaxEl, MinEl;
				// elevation is affected by the maximum and minimum elevations of the vector and the water depth
				CurVec->GetElevRange(MaxEl, MinEl);
				if (Rend->Exageration != 1.0)
					{
					MaxEl = (float)(Rend->ElevDatum + (MaxEl - Rend->ElevDatum) * Rend->Exageration);
					MinEl = (float)(Rend->ElevDatum + (MinEl - Rend->ElevDatum) * Rend->Exageration);
					} // if
				CurStream->GetWaterDepthAndWaveRange(MaximumMod, MinimumMod);
				MaximumMod += (double)MaxEl; 
				MinimumMod += (double)MinEl;
				
				if (MaximumMod > AbsoluteEffectMax)
					AbsoluteEffectMax = MaximumMod;
				if (MinimumMod < AbsoluteEffectMin)
					AbsoluteEffectMin = MinimumMod;
				break;
				} // WCS_EFFECTSSUBCLASS_STREAM
			case WCS_EFFECTSSUBCLASS_TERRAFFECTOR:
				{
				TerraffectorEffect *CurTerraffector = (TerraffectorEffect *)CurEffectList->MyEffect;
				float MaxEl, MinEl;
				// elevation is affected by the atfx or thematic elevation and the atfx settings
				CurTerraffector->ADSection.GetMinMaxVal(MinimumMod, MaximumMod);
				if (CurTerraffector->Absolute == WCS_EFFECT_ABSOLUTE)
					{
					if (MaximumMod > AbsoluteEffectMax)
						AbsoluteEffectMax = MaximumMod;
					if (MaximumMod < AbsoluteEffectMin)
						AbsoluteEffectMin = MaximumMod;
					} // if
				else if (CurTerraffector->Absolute == WCS_EFFECT_RELATIVETOGROUND)
					{
					if (MaximumMod > 0.0)
						{
						if (CurTerraffector->ApplyToOrigElev)
							{
							if (MaximumMod > LTFXRelativeIncrease)
								LTFXRelativeIncrease = MaximumMod;
							} // if
						else
							LTFXRelativeIncrease += MaximumMod;
						} // if
					else
						{
						if (CurTerraffector->ApplyToOrigElev)
							{
							if (MaximumMod < LTFXRelativeDecrease)
								LTFXRelativeDecrease = MaximumMod;
							} // if
						else
							LTFXRelativeDecrease += MaximumMod;
						} // else
					} // else if
				else	// relative to vector
					{
					CurVec->GetElevRange(MaxEl, MinEl);
					if (Rend->Exageration != 1.0)
						{
						MaxEl = (float)(Rend->ElevDatum + (MaxEl - Rend->ElevDatum) * Rend->Exageration);
						MinEl = (float)(Rend->ElevDatum + (MinEl - Rend->ElevDatum) * Rend->Exageration);
						} // if
					MaximumMod += MaxEl;
					MinimumMod += MinEl;
					if (MaximumMod > AbsoluteEffectMax)
						AbsoluteEffectMax = MaximumMod;
					if (MaximumMod < AbsoluteEffectMin)
						AbsoluteEffectMin = MaximumMod;
					} // else
				break;
				} // WCS_EFFECTSSUBCLASS_TERRAFFECTOR
			case WCS_EFFECTSSUBCLASS_RASTERTA:
				{
				RasterTerraffectorEffect *CurRasterTA = (RasterTerraffectorEffect *)CurEffectList->MyEffect;
				double Value[3];
				Value[0] = Value[1] = Value[2] = 0.0;

				// elevation is affected by the atfx or thematic elevation and the atfx settings
				if (CurRasterTA->Theme[WCS_EFFECTS_RASTERTA_THEME_ELEV] && CurRasterTA->Theme[WCS_EFFECTS_RASTERTA_THEME_ELEV]->Enabled &&
					CurRasterTA->Theme[WCS_EFFECTS_RASTERTA_THEME_ELEV]->Eval(Value, CurVec))
					{
					MaximumMod = Value[0];
					if (CurRasterTA->Absolute && Rend->ExagerateElevLines)
						MaximumMod = (MaximumMod - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
					} // if
				else
					MaximumMod = CurRasterTA->Elev;	// already has exageration applied as appropriate
				if (CurRasterTA->Absolute == WCS_EFFECT_ABSOLUTE)
					{
					if (MaximumMod > AbsoluteEffectMax)
						AbsoluteEffectMax = MaximumMod;
					if (MaximumMod < AbsoluteEffectMin)
						AbsoluteEffectMin = MaximumMod;
					} // if
				else if (CurRasterTA->Absolute == WCS_EFFECT_RELATIVETOGROUND)
					{
					if (MaximumMod > 0.0)
						{
						if (CurRasterTA->ApplyToOrigElev)
							{
							if (MaximumMod > ATFXRelativeIncrease)
								ATFXRelativeIncrease = MaximumMod;
							} // if
						else
							ATFXRelativeIncrease += MaximumMod;
						} // if
					else
						{
						if (CurRasterTA->ApplyToOrigElev)
							{
							if (MaximumMod < ATFXRelativeDecrease)
								ATFXRelativeDecrease = MaximumMod;
							} // if
						else
							ATFXRelativeDecrease += MaximumMod;
						} // else
					} // else if
				break;
				} // WCS_EFFECTSSUBCLASS_RASTERTA
			} // if
		} // for
	} // for

RelativeEffectAdd += ATFXRelativeIncrease;
RelativeEffectSubtract += ATFXRelativeDecrease;
RelativeEffectAdd += LTFXRelativeIncrease;
RelativeEffectSubtract += LTFXRelativeDecrease;

if (DefaultLake)
	{
	// lake elevation will be the one variable to consider here
	// elevation might come from a thematic map
	LakeBase.GetThemedMaxLevel(Rend, DefaultLake, NULL, ThemedWaterLevel, ThemedBeachLevel);
		
	// find range of material water depths and wave heights
	MaximumMod = MinimumMod = 0.0;
	DefaultLake->GetWaterDepthAndWaveRange(MaximumMod, MinimumMod);
	MaximumMod += ThemedWaterLevel;
	MinimumMod += ThemedWaterLevel;
	
	if (MaximumMod > AbsoluteEffectMax)
		AbsoluteEffectMax = MaximumMod;
	if (MinimumMod < AbsoluteEffectMin)
		AbsoluteEffectMin = MinimumMod;
	} // if
	
} // EffectsLib::EvalHighLowEffectExtrema

/*===========================================================================*/

double EffectsLib::GetPlanetRadius(void)
{

#ifdef WCS_BUILD_VNS
double Major, Minor;
// returns the larger of semi-major and semi-minor axes
Major = FetchDefaultCoordSys()->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].CurValue;
Minor = FetchDefaultCoordSys()->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].CurValue;
return (max(Major, Minor));
#else // WCS_BUILD_VNS
PlanetOpt *DefPlanetOpt;

if (DefPlanetOpt = (PlanetOpt *)GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL))
	{
	return (DefPlanetOpt->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_RADIUS].CurValue);
	} // if

return (CelestialPresetRadius[WCS_CELESTIAL_PRESETS_EARTH]);
#endif // WCS_BUILD_VNS

} // EffectsLib::GetPlanetRadius

/*===========================================================================*/

CoordSys *EffectsLib::CompareMakeCoordSys(CoordSys *MatchMe, int MakeNew)
{
CoordSys *CurCoord;
PlanetOpt *DefPlanetOpt;

if (CurCoord = (CoordSys *)GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS))
	{
	while (CurCoord)
		{
		if (CurCoord->Equals(MatchMe))
			return (CurCoord);
		CurCoord = (CoordSys *)CurCoord->Next;
		} // while
	} // if

if (MakeNew)
	{
	if (CurCoord = (CoordSys *)AddEffect(WCS_EFFECTSSUBCLASS_COORDSYS, NULL, MatchMe))
		{
		if (DefPlanetOpt = (PlanetOpt *)GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL))
			{
			if (this == GlobalApp->AppEffects &&
				UserMessageYN(CurCoord->GetName(), "Make this new Coordinate System the default for Viewing and Rendering?"))
				{
				DefPlanetOpt->SetCoords(CurCoord);
				} // if
			} // if
		} // if
	return (CurCoord);
	} // if

return (NULL);

} // EffectsLib::CompareMakeCoordSys

/*===========================================================================*/

long EffectsLib::CountRenderItemVertices(RenderData *Rend, Database *DB)
{
long NumVerts = 0;

if (! Rend->ShadowMapInProgress)
	NumVerts += Object3DBase.VerticesToRender;
NumVerts += FoliageBase.VerticesToRender;
NumVerts += FnceBase.VerticesToRender;
NumVerts += LablBase.VerticesToRender;

return (NumVerts);

} // EffectsLib::CountRenderItemVertices

/*===========================================================================*/

void EffectsLib::FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt, DEM *CurDEM)
{

if (! Rend->ShadowMapInProgress)
	Object3DBase.FillRenderPolyArray(Rend, PolyArray, PolyCt, CurDEM);
FoliageBase.FillRenderPolyArray(Rend, PolyArray, PolyCt, CurDEM);
FnceBase.FillRenderPolyArray(Rend, PolyArray, PolyCt, CurDEM);
LablBase.FillRenderPolyArray(Rend, PolyArray, PolyCt, CurDEM);

} // EffectsLib::FillRenderPolyArray

/*===========================================================================*/

void EffectsLib::FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt, 
	CoordSys *MyCoords, GeoRegister *MyBounds)
{

if (! Rend->ShadowMapInProgress)
	Object3DBase.FillRenderPolyArray(Rend, PolyArray, PolyCt, MyCoords, MyBounds);
FoliageBase.FillRenderPolyArray(Rend, PolyArray, PolyCt, MyCoords, MyBounds);
FnceBase.FillRenderPolyArray(Rend, PolyArray, PolyCt, MyCoords, MyBounds);
LablBase.FillRenderPolyArray(Rend, PolyArray, PolyCt, MyCoords, MyBounds);

} // EffectsLib::FillRenderPolyArray

/*===========================================================================*/

EcosystemEffect *EffectsLib::FindEcoInEnvironment(EcosystemEffect *FindMe)
{
EnvironmentEffect *Current;

for (Current = (EnvironmentEffect *)GetListPtr(WCS_EFFECTSSUBCLASS_ENVIRONMENT); Current; Current = (EnvironmentEffect *)Current->Next)
	{
	if (Current->FindEco(FindMe))
		return (FindMe);
	} // for

return (NULL);

} // EffectsLib::FindEcoInEnvironment

/*===========================================================================*/
/*===========================================================================*/

GeneralEffect::GeneralEffect(RasterAnimHost *RAHost)
: RasterAnimHost(RAHost)
{

SetDefaults();

} // GeneralEffect::GeneralEffect

/*===========================================================================*/

GeneralEffect::~GeneralEffect()
{
class JoeList *CurJoeList;

while (Joes)
	{
	CurJoeList = Joes;
	Joes = Joes->Next;
	delete CurJoeList;
	} // while
RemoveRenderJoes();

} // GeneralEffect::GeneralEffect

/*===========================================================================*/

void GeneralEffect::SetDefaults(void)
{

EffectType = 0;
Joes = RenderJoes = NULL;
Prev = Next = NULL;
HiResEdge = UseGradient = Absolute = 0;
Enabled = 1;
Priority = 0;
Name[0] = 0;
Search = NULL;
RenderOccluded = 0;

} // GeneralEffect::SetDefaults

/*===========================================================================*/

void GeneralEffect::Copy(GeneralEffect *CopyTo, GeneralEffect *CopyFrom)
{
long Ct, Result = -1;
#ifdef WCS_BUILD_VNS
NotifyTag Changes[2];
#endif // WCS_BUILD_VNS

CopyTo->EffectType = CopyFrom->EffectType;
CopyTo->Enabled = CopyFrom->Enabled;
CopyTo->Priority = CopyFrom->Priority;
CopyTo->HiResEdge = CopyFrom->HiResEdge;
CopyTo->UseGradient = CopyFrom->UseGradient;
CopyTo->Absolute = CopyFrom->Absolute;
CopyTo->RenderOccluded = CopyFrom->RenderOccluded;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	GetAnimPtr(Ct)->Copy((AnimCritter *)CopyTo->GetAnimPtr(Ct), (AnimCritter *)CopyFrom->GetAnimPtr(Ct));
	} // for

/*
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (CopyTo->GetTexRootPtr(Ct))
		{
		delete CopyTo->GetTexRootPtr(Ct);		// removes links to images
		CopyTo->SetTexRootPtr(Ct, NULL);
		} // if
	if (CopyFrom->GetTexRootPtr(Ct))
		{
		CopyTo->SetTexRootPtr(Ct, new RootTexture(CopyTo, CopyFrom->GetTexRootPtr(Ct)->ApplyToEcosys, CopyFrom->GetTexRootPtr(Ct)->ApplyToColor, CopyFrom->GetTexRootPtr(Ct)->ApplyToDisplace));
		if (CopyTo->GetTexRootPtr(Ct))
			{
			CopyTo->GetTexRootPtr(Ct)->Copy(CopyTo->GetTexRootPtr(Ct), CopyFrom->GetTexRootPtr(Ct));
			} // if
		} // if
	} // for
*/
#ifdef WCS_BUILD_VNS
CopyTo->Search = NULL;
if (CopyFrom->Search)
	{
	if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib || GlobalApp->CopyToEffectsLib != GlobalApp->AppEffects)
		{
		CopyTo->Search = (SearchQuery *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->Search);
		} // if no need to make another copy, its all in the family
	else
		{
		if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->Search->EffectType, CopyFrom->Search->Name))
			{
			Result = UserMessageCustom("Copy Component", "How do you wish to resolve Search Query name collisions?\n\nLink to existing Search Queries, replace existing Search Queries, or create new Search Queries?",
				"Link", "Create", "Overwrite", 1);
			} // if
		if (Result <= 0)
			{
			CopyTo->Search = (SearchQuery *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->Search->EffectType, NULL, CopyFrom->Search);
			} // if create new
		else if (Result == 1)
			{
			CopyTo->Search = (SearchQuery *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->Search);
			} // if link to existing
		else if (CopyTo->Search = (SearchQuery *)GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->Search->EffectType, CopyFrom->Search->Name))
			{
			CopyTo->Search->Copy(CopyTo->Search, CopyFrom->Search);
			Changes[0] = MAKE_ID(CopyTo->Search->GetNotifyClass(), CopyTo->Search->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, CopyTo->Search);
			} // else if found and overwrite
		else
			{
			CopyTo->Search = (SearchQuery *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->Search->EffectType, NULL, CopyFrom->Search);
			} // else
		} // else better copy or overwrite it since its important to get just the right object
	} // if
#endif // WCS_BUILD_VNS

strcpy(CopyTo->Name, CopyFrom->Name);
RootTextureParent::Copy(CopyTo, CopyFrom);
ThematicOwner::Copy(CopyTo, CopyFrom);
RasterAnimHost::Copy(CopyTo, CopyFrom);

} // GeneralEffect::Copy

/*===========================================================================*/

// does not check to see if joe already in joe list
// inserts new JoeList/Joe at head of chain instead of
// walking to end and appending
JoeList *GeneralEffect::FastDangerousAddToJoeList(Joe *Add)
{
class JoeList *NextJoeList, *NewJoeList = NULL;
//NotifyTag Changes[2];

if (Add)
	{
	NextJoeList = Joes;
	if(NewJoeList = new JoeList())
		{
		NewJoeList->Me = Add;
		NewJoeList->Next = NextJoeList;
		if (NextJoeList)
			NextJoeList->Prev = NewJoeList;
		Joes = NewJoeList;
		} // if
	} // if

//Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
//Changes[1] = NULL;
//GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (NewJoeList);

} // GeneralEffect::FastDangerousAddToJoeList

/*===========================================================================*/

JoeList *GeneralEffect::AddToJoeList(Joe *Add)
{
class JoeList *NextJoeList, *NewJoeList = NULL;
NotifyTag Changes[2];

if (Add)
	{
	if (Joes)
		{
		NextJoeList = Joes;
		while (NextJoeList->Next)
			{
			if (NextJoeList->Me == Add)
				return (NextJoeList);
			NextJoeList = NextJoeList->Next;
			} // while
		if (NextJoeList->Me == Add)
			return (NextJoeList);
		if (NextJoeList->Next = new JoeList())
			{
			NewJoeList = NextJoeList->Next;
			NewJoeList->Me = Add;
			NewJoeList->Prev = NextJoeList;
			} // if
		} // if
	else if (NewJoeList = Joes = new JoeList())
		Joes->Me = Add;
	} // if

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (NewJoeList);

} // GeneralEffect::AddToJoeList

/*===========================================================================*/

int GeneralEffect::RemoveFromJoeList(Joe *Remove)
{
class JoeList *CurJoeList, *RemoveJoe;
JoeAttribute *MyAttr;
int Removed = 0;
NotifyTag Changes[2];

if (Remove)
	{
	if (Joes)
		{
		// special treatment for coordinate systems to expedite removal
		// Joe attribute may already have been removed if removal originated in the Joe,
		if (EffectType == WCS_EFFECTSSUBCLASS_COORDSYS)
			{
			if (MyAttr = Remove->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, (unsigned char)EffectType))
				{
				CurJoeList = ((JoeCoordSys *)MyAttr)->CoordsJoeList;
				return (RemoveFromJoeList(CurJoeList));
				} // if
			} // if
		else if (Joes->Me == Remove)
			{
			RemoveJoe = Joes;
			Joes = Joes->Next;
			if (Joes)
				Joes->Prev = NULL;
			if (MyAttr = RemoveJoe->Me->RemoveEffectAttribute(WCS_JOE_ATTRIB_INTERNAL, (unsigned char)EffectType, this))
				{
				delete MyAttr;
				} // if
			delete RemoveJoe;
			Removed = 1;
			} // if
		else
			{
			CurJoeList = Joes;
			while (CurJoeList->Next)
				{
				if (CurJoeList->Next->Me == Remove)
					{
					RemoveJoe = CurJoeList->Next;
					CurJoeList->Next = CurJoeList->Next->Next;
					if (CurJoeList->Next)
						CurJoeList->Next->Prev = NULL;
					if (MyAttr = RemoveJoe->Me->RemoveEffectAttribute(WCS_JOE_ATTRIB_INTERNAL, (unsigned char)EffectType, this))
						{
						delete MyAttr;
						} // if
					delete RemoveJoe;
					Removed = 1;
					break;
					} // if
				CurJoeList = CurJoeList->Next;
				} // while
			} // else
		} // if
	} // if

if (Removed)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

return (Removed);

} // GeneralEffect::RemoveFromJoeList

/*===========================================================================*/

int GeneralEffect::RemoveFromJoeList(JoeList *Remove)
{
class JoeList *PrevJoe, *NextJoe;
JoeAttribute *MyAttr;
Joe *JoeMe;
int Removed = 0;
NotifyTag Changes[2];

if (Remove)
	{
	JoeMe = Remove->Me;
	if (Joes)
		{
		PrevJoe = Remove->Prev;
		NextJoe = Remove->Next;

		if (PrevJoe)
			PrevJoe->Next = NextJoe;
		if (NextJoe)
			NextJoe->Prev = PrevJoe;
		if (Joes == Remove)
			Joes = NextJoe;
		if (JoeMe && (MyAttr = JoeMe->RemoveEffectAttribute(WCS_JOE_ATTRIB_INTERNAL, (unsigned char)EffectType, this)))
			{
			delete MyAttr;
			} // if
		delete Remove;
		Removed = 1;
		// this is now done in RemoveEffectAttribute
		//if (JoeMe)
		//	JoeMe->RecheckBounds();
		} // if
	} // if

if (Removed)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

return (Removed);

} // GeneralEffect::RemoveFromJoeList

/*===========================================================================*/

void GeneralEffect::AddRenderJoe(Joe *Add)
{
JoeList *NextJoeList;

if (Add)
	{
	if (RenderJoes)
		{
		NextJoeList = RenderJoes;
		while (NextJoeList->Next)
			{
			if (NextJoeList->Me == Add)
				return;
			NextJoeList = NextJoeList->Next;
			} // while
		if (NextJoeList->Me == Add)
			return;
		if (NextJoeList->Next = new JoeList())
			{
			NextJoeList->Next->Me = Add;
			} // if
		} // if
	else if (RenderJoes = new JoeList())
		RenderJoes->Me = Add;
	} // if

} // GeneralEffect::AddRenderJoe

/*===========================================================================*/

void GeneralEffect::RemoveRenderJoes(void)
{
JoeList *RemoveJoe;

while (RenderJoes)
	{
	RemoveJoe = RenderJoes;
	RenderJoes = RenderJoes->Next;
	delete RemoveJoe;
	} // while

} // GeneralEffect::RemoveRenderJoes

/*===========================================================================*/

int GeneralEffect::SetQuery(SearchQuery *NewQuery)
{
#ifdef WCS_BUILD_VNS
NotifyTag Changes[2];

Search = NewQuery;
Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (1);
#else // WCS_BUILD_VNS
return (0);
#endif // WCS_BUILD_VNS

} // GeneralEffect::SetQuery

/*===========================================================================*/

int GeneralEffect::BuildFileComponentsList(EffectList **Queries, EffectList **Themes, EffectList **Coords)
{
#ifdef WCS_BUILD_VNS
long Ct;
EffectList **ListPtr;

if (Search)
	{
	ListPtr = Queries;
	while (*ListPtr)
		{
		if ((*ListPtr)->Me == Search)
			break;
		ListPtr = &(*ListPtr)->Next;
		} // if
	if (! (*ListPtr))
		{
		if (*ListPtr = new EffectList())
			(*ListPtr)->Me = Search;
		else
			return (0);
		} // if
	} // if
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		if (! GetTexRootPtr(Ct)->BuildFileComponentsList(Coords))
			return (0);
		} // if
	} // for
#endif // WCS_BUILD_VNS

return (ThematicOwner::BuildFileComponentsList(Themes));

} // GeneralEffect::BuildFileComponentsList

/*===========================================================================*/

void GeneralEffect::ResolveLoadLinkages(EffectsLib *Lib)
{
long Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		GetTexRootPtr(Ct)->ResolveLoadLinkages(Lib);
		} // if
	} // for

} // GeneralEffect::ResolveLoadLinkages

/*===========================================================================*/

void GeneralEffect::HardLinkVectors(Database *DBHost)
{

if (Search)
	{
	Search->HardLinkVectors(DBHost, this);
	if (UserMessageYN(Name, "Remove Dynamic Linkage now?"))
		RemoveRAHost(Search);
	} // if
else
	UserMessageOK(Name, "There is no Search Query attached to this Component.");

} // GeneralEffect::HardLinkVectors

/*===========================================================================*/

void GeneralEffect::EnableVectors(Database *DBHost, bool NewState)
{
JoeList *CurJoe;
NotifyTag EventFlags[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEOBJ, WCS_NOTIFYDBASECHANGE_FLAGS, 0), 0};

GlobalApp->AppDB->SuppressNotifiesDuringLoad = 1;

if (Search)
	{
	Search->EnableVectors(DBHost, NewState);
	} // if
for (CurJoe = Joes; CurJoe; CurJoe = CurJoe->Next)
	{
	if (CurJoe->Me)
		{
		if (NewState)
			CurJoe->Me->SetFlags(WCS_JOEFLAG_ACTIVATED);
		else
			CurJoe->Me->ClearFlags(WCS_JOEFLAG_ACTIVATED);
		} // if
	} // for

GlobalApp->AppDB->SuppressNotifiesDuringLoad = 0;
GlobalApp->AppDB->GenerateNotify(EventFlags, NULL);
EventFlags[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
GlobalApp->AppEx->GenerateNotify(EventFlags, NULL);

} // GeneralEffect::EnableVectors

/*===========================================================================*/

void GeneralEffect::SelectVectors(Database *DBHost)
{

if(!GlobalApp->GUIWins->DBE)
	{
	if(GlobalApp->GUIWins->DBE = new DBEditGUI(GlobalApp->AppDB, GlobalApp->MainProj, GlobalApp->AppEffects))
		{
		if(GlobalApp->GUIWins->DBE->ConstructError)
			{
			delete GlobalApp->GUIWins->DBE;
			GlobalApp->GUIWins->DBE = NULL;
			} // if
		} // if
	} // if
if(GlobalApp->GUIWins->DBE)
	{
	GlobalApp->GUIWins->DBE->Open(GlobalApp->MainProj);
	// clear selected items
	GlobalApp->GUIWins->DBE->DeselectAll();
	GlobalApp->AppDB->ResetGeoClip();
	GlobalApp->GUIWins->DBE->HardFreeze();
	
	if (Search)
		Search->SelectVectors(DBHost, false, false);
	if (Joes)
		{
		GlobalApp->GUIWins->DBE->SelectByJoeList(Joes);
		} // if

	GlobalApp->GUIWins->DBE->HardUnFreeze();
	GlobalApp->GUIWins->DBE->UpdateTitle();
	} // if
	
} // GeneralEffect::SelectVectors

/*===========================================================================*/

void GeneralEffect::HardLinkSelectedVectors(Database *DBHost)
{

if(GlobalApp->GUIWins->DBE)
	{
	GlobalApp->GUIWins->DBE->ApplyEffectToSelected(this);
	} // if
else
	UserMessageOK("Hard Link Selected Vectors", "There are no vectors currently selected in the Database");
	
} // GeneralEffect::HardLinkSelectedVectors

/*===========================================================================*/

char *GeneralEffect::SetUniqueName(EffectsLib *Library, char *NameBase)
{
char NewName[WCS_EFFECT_MAXNAMELENGTH], FoundMatch = 1;
long Ct;
GeneralEffect *Effect;

if (NameBase)
	{
	if (Library->FindByName(EffectType, NameBase))
		{
		StripExtension(NameBase);
		if (strlen(NameBase) > WCS_EFFECT_MAXNAMELENGTH - 4)
			NameBase[WCS_EFFECT_MAXNAMELENGTH - 4] = 0;
		for (Ct = 0; Ct < 9999 && FoundMatch; Ct ++)
			{
			FoundMatch = 0;
			if (Ct)
				sprintf(NewName, "%s.%d", NameBase, Ct);
			else
				sprintf(NewName, "%s", NameBase);
			Effect = Library->GetListPtr(EffectType);
			while (Effect)
				{
				if (Effect != this && ! stricmp(Effect->Name, NewName))
					{
					FoundMatch = 1;
					break;
					} // if
				Effect = Effect->Next;
				} // while
			} // for
		if (! FoundMatch)
			strcpy(Name, NewName);
		} // if
	else
		{
		strncpy(Name, NameBase, WCS_EFFECT_MAXNAMELENGTH - 1);
		Name[WCS_EFFECT_MAXNAMELENGTH - 1] = NULL; // ensure NULL-terminated after strncpy
		} // else
	} // if

return (Name);

} // GeneralEffect::SetUniqueName

/*===========================================================================*/

int GeneralEffect::SetToTime(double Time)
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

return (Found);

} // GeneralEffect::SetToTime

/*===========================================================================*/

int GeneralEffect::SetTheme(long ThemeNum, ThematicMap *NewTheme)
{
NotifyTag Changes[2];

if (ThemeNum < GetNumThemes())
	{
	SetThemePtr(ThemeNum, NewTheme);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	return (1);
	} // if

return (0);

} // GeneralEffect::SetTheme

/*===========================================================================*/

long GeneralEffect::InitImageIDs(long &ImageID)
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

} // GeneralEffect::InitImageIDs

/*===========================================================================*/

// maximum is in array elements [0], minimum in [1]
int GeneralEffect::GetMaterialBoundsXYZ(double XRange[2], double YRange[2], double ZRange[2])
{
int Found = 0;
double NWLatitude, NWLongitude, SELatitude, SELongitude, RefLat, RefLon, EarthLatScaleMeters, RefLonScaleMeters,
	HighX, LowX, HighY, LowY;
Joe *CurJoe;
JoeList *CurJL;

EarthLatScaleMeters = LatScale(GlobalApp->AppEffects->GetPlanetRadius());
RefLat = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y);
RefLon = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X);
RefLonScaleMeters = EarthLatScaleMeters * cos(RefLat * PiOver180);

XRange[0] = YRange[0] = -FLT_MAX;
XRange[1] = YRange[1] = FLT_MAX;

if (CurJL = Joes)
	{
	while (CurJL)
		{
		CurJL->Me->GetTrueBounds(NWLatitude, NWLongitude, SELatitude, SELongitude);
		HighX = -(SELongitude - RefLon) * RefLonScaleMeters;
		LowX = -(NWLongitude - RefLon) * RefLonScaleMeters;
		HighY = (NWLatitude - RefLat) * EarthLatScaleMeters;
		LowY = (SELatitude - RefLat) * EarthLatScaleMeters;
		if (HighX > XRange[0])
			{
			XRange[0] = HighX;
			Found = 1;
			} // if
		if (LowX < XRange[1])
			XRange[1] = LowX;
		if (HighY > YRange[0])
			YRange[0] = HighY;
		if (LowY < YRange[1])
			YRange[1] = LowY;
		CurJL = CurJL->Next;
		} // while
	} // if
if (Search)
	{
	CurJoe = NULL;
	while (CurJoe = GlobalApp->AppDB->GetNextByQuery(Search, CurJoe))
		{
		CurJoe->GetTrueBounds(NWLatitude, NWLongitude, SELatitude, SELongitude);
		HighX = -(SELongitude - RefLon) * RefLonScaleMeters;
		LowX = -(NWLongitude - RefLon) * RefLonScaleMeters;
		HighY = (NWLatitude - RefLat) * EarthLatScaleMeters;
		LowY = (SELatitude - RefLat) * EarthLatScaleMeters;
		if (HighX > XRange[0])
			{
			XRange[0] = HighX;
			Found = 1;
			} // if
		if (LowX < XRange[1])
			XRange[1] = LowX;
		if (HighY > YRange[0])
			YRange[0] = HighY;
		if (LowY < YRange[1])
			YRange[1] = LowY;
		} // while
	} // if

if (! Found)
	{
	if (GlobalApp->AppDB->GetDEMExtents(NWLatitude, SELatitude, NWLongitude, SELongitude))
		{
		XRange[0] = -(SELongitude - RefLon) * RefLonScaleMeters;
		XRange[1] = -(NWLongitude - RefLon) * RefLonScaleMeters;
		YRange[0] = (NWLatitude - RefLat) * EarthLatScaleMeters;
		YRange[1] = (SELatitude - RefLat) * EarthLatScaleMeters;
		Found = 1;
		} // if
	else
		{
		XRange[0] = XRange[1] = 0.0;
		YRange[0] = YRange[1] = 0.0;
		} // else still not found
	} // if
ZRange[0] = ZRange[1] = 0.0;

return (Found);

} // GeneralEffect::GetMaterialBoundsXYZ

/*===========================================================================*/

void GeneralEffect::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = RAParent ? RAParent->GetCritterName(this): GetName();
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
	Prop->Path = EffectsLib::DefaultPaths[EffectType];
	Prop->Ext = EffectsLib::DefaultExtensions[EffectType];
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_INTERFLAGS)
	{
	GetInterFlags(Prop, this);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_POPCLASS)
	{
	GetPopClassFlags(Prop);
	} // get pop-up menu classes

} // GeneralEffect::GetRAHostProperties

/*===========================================================================*/

int GeneralEffect::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
char NameStr[WCS_EFFECT_MAXNAMELENGTH];
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_ENABLED)
		{
		if (EffectType == WCS_EFFECTSSUBCLASS_PLANETOPT)
			{
			GlobalApp->AppEffects->SetPlanetOptEnabled((PlanetOpt *)this, (Prop->Flags & WCS_RAHOST_FLAGBIT_ENABLED) ? 1: 0);
			} // if
		else if (EffectType != WCS_EFFECTSSUBCLASS_CAMERA && EffectType != WCS_EFFECTSSUBCLASS_RENDEROPT)
			{
			Enabled = (Prop->Flags & WCS_RAHOST_FLAGBIT_ENABLED) ? 1: 0;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	strncpy(NameStr, Prop->Name, WCS_EFFECT_MAXNAMELENGTH);
	NameStr[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
	SetUniqueName(GlobalApp->AppEffects, NameStr);
	Success = 1;
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
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
	return(SaveObject(Prop->fFile, Prop->Path));
	} // if

return (Success);

} // GeneralEffect::SetRAHostProperties

/*===========================================================================*/

char GeneralEffect::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (GetNumAnimParams() > 0 && DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	return (1);
if (GetNumTextures() > 0 && (DropType == WCS_RAHOST_OBJTYPE_ROOTTEXTURE || DropType == WCS_RAHOST_OBJTYPE_TEXTURE))
	return (1);

return (0);

} // GeneralEffect::GetRAHostDropOK

/*===========================================================================*/

RasterAnimHost *GeneralEffect::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
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

} // GeneralEffect::GetRAHostChild

/*===========================================================================*/

int GeneralEffect::RemoveRAHost(RasterAnimHost *RemoveMe)
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
	for (Ct = 0; Ct < GetNumThemes(); Ct ++)
		{
		if (RemoveMe == GetTheme(Ct))
			{
			SetThemePtr(Ct, NULL);
			Removed = 1;
			} // if
		} // for

	if (Search == (SearchQuery *)RemoveMe)
		{
		Search = NULL;
		Removed = 1;
		} // if

	if (Removed)
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	else
		{
		if (RemoveFromJoeList((Joe *)RemoveMe))		// sends its own notification
			Removed = 1;
		} // else
	} // if

return (Removed);

} // GeneralEffect::RemoveRAHost

/*===========================================================================*/

int GeneralEffect::GetDeletable(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		return (1);
	} // for

if (Test == Search)
	return (1);

return (0);

} // GeneralEffect::GetDeletable

/*===========================================================================*/

int GeneralEffect::GetRAHostAnimated(void)
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

return (0);

} // GeneralEffect::GetRAHostAnimated

/*===========================================================================*/

bool GeneralEffect::AnimateMaterials(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetRAHostAnimated())
		return (true);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetRAHostAnimatedInclVelocity())
		return (true);
	} // for

return (false);

} // GeneralEffect::GetRAHostAnimated

/*===========================================================================*/

long GeneralEffect::GetKeyFrameRange(double &FirstKey, double &LastKey)
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

} // GeneralEffect::GetKeyFrameRange

/*===========================================================================*/

int GeneralEffect::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char WinNum;
int Success = 0;
long Ct, NumListItems = 0;
RasterAnimHost *TargetList[100];

if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		TargetList[Ct] = GetAnimPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if
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

} // GeneralEffect::ProcessRAHostDragDrop

/*===========================================================================*/

int GeneralEffect::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{
char Ct;

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

return (1);

} // GeneralEffect::InitToRender

/*===========================================================================*/

int GeneralEffect::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{
char Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->Enabled)
		{
		if (! GetTexRootPtr(Ct)->InitFrameToRender(Lib, Rend))
			{
			return (0);
			} // if
		} // if
	} // for

return (1);

} // GeneralEffect::InitFrameToRender

/*===========================================================================*/

int GeneralEffect::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
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

} // GeneralEffect::GetAffiliates

/*===========================================================================*/

int GeneralEffect::GetPopClassFlags(RasterAnimHostProperties *Prop)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;
ThematicMap **ThemeAffil = NULL;

Prop->PopClassFlags = 0;
Prop->PopExistsFlags = 0;
Prop->PopEnabledFlags = 0;

if (GetAffiliates(Prop->ChildA, Prop->ChildB, AnimAffil, TexAffil, ThemeAffil))
	{
	return (RasterAnimHost::GetPopClassFlags(Prop, AnimAffil, TexAffil, ThemeAffil));
	} // if

return (0);

} // GeneralEffect::GetPopClassFlags

/*===========================================================================*/

int GeneralEffect::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;
ThematicMap **ThemeAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil, ThemeAffil))
	{
	return (RasterAnimHost::AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, TexAffil, ThemeAffil));
	} // if

return(0);

} // GeneralEffect::AddSRAHBasePopMenus

/*===========================================================================*/

int GeneralEffect::HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;
ThematicMap **ThemeAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil, ThemeAffil))
	{
	return (RasterAnimHost::HandleSRAHPopMenuSelection(Action, AnimAffil, TexAffil, ThemeAffil, this, this));
	} // if

return(0);

} // GeneralEffect::HandleSRAHPopMenuSelection

/*===========================================================================*/
/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::Load(FILE *ffile, ULONG ReadSize, struct ChunkIODetail *Detail)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0, ByteOrder;
char Version, Revision;
union MultiVal MV;
short ByteFlip, NumClasses = 0, MaxClasses = WCS_MAXIMPLEMENTED_EFFECTS, ClassesToRead;
static char StructBuf[256];  // Plenty big enuf to hold anything we currently fread...
struct ChunkIODetail *LocalDetail;
RenderOpt *LoadToRenderOpt;
#ifdef WCS_BUILD_VNS
PlanetOpt *CurrentPlanetOpt;
#endif // WCS_BUILD_VNS

if (! ffile)
	{
	return (0);
	} // if need to open file

// header data is 6 bytes
if ((fread((char *)StructBuf, 6, 1, ffile)) != 1)
	return (0);

Version = StructBuf[0];
Revision = StructBuf[1];
memcpy(&ByteOrder, &StructBuf[2], 4);

// Endian flop if necessary

if (ByteOrder == 0xaabbccdd)
	ByteFlip = 0;
else if (ByteOrder == 0xddccbbaa)
	ByteFlip = 1;
else
	return (0);

ProjectFileSavedWithForestEd = 0;

TotalRead = 6;
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

				LocalDetail = NULL;
				switch (ItemTag & 0xffff0000)
					{
					case WCS_EFFECTS_NUMCLASSES:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumClasses, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GROUPENABLED:
						{
						if (! Detail)
							{
							if (LoadToRenderOpt = (RenderOpt *)GetDefaultEffect(WCS_EFFECTSSUBCLASS_RENDEROPT, 1, NULL))
								{
								ClassesToRead = NumClasses >= MaxClasses ? MaxClasses: NumClasses;
								BytesRead = ReadBlock(ffile, (char *)&LoadToRenderOpt->EffectEnabled[0], WCS_BLOCKTYPE_CHAR + ClassesToRead, ByteFlip);
								ClassesToRead = NumClasses > MaxClasses ? NumClasses - MaxClasses: 0;
								if (ClassesToRead && ! fseek(ffile, ClassesToRead, SEEK_CUR))
									BytesRead += ClassesToRead;
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_GROUPVECPOLYENABLED:
						{
						if (! Detail)
							{
							ClassesToRead = NumClasses >= MaxClasses ? MaxClasses: NumClasses;
							BytesRead = ReadBlock(ffile, (char *)&GroupVecPolyEnabled[0], WCS_BLOCKTYPE_CHAR + ClassesToRead, ByteFlip);
							ClassesToRead = NumClasses > MaxClasses ? NumClasses - MaxClasses: 0;
							if (ClassesToRead && ! fseek(ffile, ClassesToRead, SEEK_CUR))
								BytesRead += ClassesToRead;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_GROUPADVANCEDENABLED:
						{
						if (! Detail)
							{
							ClassesToRead = NumClasses >= MaxClasses ? MaxClasses: NumClasses;
							BytesRead = ReadBlock(ffile, (char *)&GroupDisplayAdvancedEnabled[0], WCS_BLOCKTYPE_CHAR + ClassesToRead, ByteFlip);
							ClassesToRead = NumClasses > MaxClasses ? NumClasses - MaxClasses: 0;
							if (ClassesToRead && ! fseek(ffile, ClassesToRead, SEEK_CUR))
								BytesRead += ClassesToRead;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_FILESAVEDWITHFORESTED:
						{
						BytesRead = ReadBlock(ffile, (char *)&ProjectFileSavedWithForestEd, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MAXMEM:
						{
						BytesRead = ReadBlock(ffile, (char *)&MaxMem, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					#ifdef WCS_BUILD_VNS
					case WCS_EFFECTS_COORDSYS:
						{
						if (! Detail || (LocalDetail = GlobalApp->MainProj->ChunkIODetailSearch(Detail, WCS_EFFECTSIO_COORDSYS)))
							{
							BytesRead = CoordSys_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_SEARCHQUERY:
						{
						if (! Detail || (LocalDetail = GlobalApp->MainProj->ChunkIODetailSearch(Detail, WCS_EFFECTSIO_SEARCHQUERY)))
							{
							BytesRead = SearchQuery_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#endif // WCS_BUILD_VNS
					#ifdef WCS_THEMATIC_MAP
					case WCS_EFFECTS_THEMATICMAP:
						{
						if (! Detail || (LocalDetail = GlobalApp->MainProj->ChunkIODetailSearch(Detail, WCS_EFFECTSIO_THEMATICMAP)))
							{
							BytesRead = ThematicMap_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#endif // WCS_THEMATIC_MAP
					#ifdef WCS_RENDER_SCENARIOS
					case WCS_EFFECTS_SCENARIO:
						{
						if (! Detail || (LocalDetail = GlobalApp->MainProj->ChunkIODetailSearch(Detail, WCS_EFFECTSIO_SCENARIO)))
							{
							BytesRead = RenderScenario_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#endif // WCS_RENDER_SCENARIOS
					case WCS_EFFECTS_OTHER:
						{
						if (! Detail || (LocalDetail = GlobalApp->MainProj->ChunkIODetailSearch(Detail, WCS_EFFECTSIO_OTHER)))
							{
							BytesRead = OtherEffects_Load(ffile, Size, ByteFlip, LocalDetail);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_AREA:
						{
						if (! Detail || (LocalDetail = GlobalApp->MainProj->ChunkIODetailSearch(Detail, WCS_EFFECTSIO_AREA)))
							{
							BytesRead = AreaEffects_Load(ffile, Size, ByteFlip, LocalDetail);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_LINE:
						{
						if (! Detail || (LocalDetail = GlobalApp->MainProj->ChunkIODetailSearch(Detail, WCS_EFFECTSIO_LINE)))
							{
							BytesRead = LineEffects_Load(ffile, Size, ByteFlip, LocalDetail);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_POINT:
						{
						if (! Detail || (LocalDetail = GlobalApp->MainProj->ChunkIODetailSearch(Detail, WCS_EFFECTSIO_POINT)))
							{
							BytesRead = PointEffects_Load(ffile, Size, ByteFlip, LocalDetail);
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

ResolveLoadLinkages();

#ifdef WCS_BUILD_VNS
if (CurrentPlanetOpt = (PlanetOpt *)GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL))
	UpdateDefaultCoords(CurrentPlanetOpt->Coords, FALSE);
#endif // WCS_BUILD_VNS

return (TotalRead);

} // EffectsLib::Load();

/*===========================================================================*/

ULONG EffectsLib::OtherEffects_Load(FILE *ffile, ULONG ReadSize, short ByteFlip, struct ChunkIODetail *Detail)
{
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
					case WCS_EFFECTSOTHER_MATERIAL:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_MATERIAL) >= 0)
							{
							BytesRead = MaterialEffect_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSOTHER_CELESTIAL:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_CELESTIAL) >= 0)
							{
							BytesRead = CelestialEffect_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSOTHER_STARFIELD:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_STARFIELD) >= 0)
							{
							BytesRead = StarFieldEffect_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSOTHER_PLANETOPT:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_PLANETOPT) >= 0)
							{
							BytesRead = PlanetOpt_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSOTHER_SKY:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_SKY) >= 0)
							{
							BytesRead = Sky_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSOTHER_ATMOSPHERE:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_ATMOSPHERE) >= 0)
							{
							BytesRead = Atmosphere_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSOTHER_LIGHT:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_LIGHT) >= 0)
							{
							BytesRead = Light_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSOTHER_CAMERA:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_CAMERA) >= 0)
							{
							BytesRead = Camera_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSOTHER_RENDERJOB:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_RENDERJOB) >= 0)
							{
							BytesRead = RenderJob_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSOTHER_RENDEROPT:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_RENDEROPT) >= 0)
							{
							BytesRead = RenderOpt_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSOTHER_GRIDDER:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_GRIDDER) >= 0)
							{
							BytesRead = TerraGridder_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSOTHER_GENERATOR:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_GENERATOR) >= 0)
							{
							BytesRead = TerraGenerator_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSOTHER_FENCE:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_FENCE) >= 0)
							{
							BytesRead = Fence_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSOTHER_POSTPROC:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_POSTPROC) >= 0)
							{
							BytesRead = PostProcess_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSOTHER_DEMMERGER:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_DEMMERGER) >= 0)
							{
							BytesRead = DEMMerger_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#ifdef WCS_BUILD_RTX
					case WCS_EFFECTSOTHER_EXPORTER:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_EXPORTER) >= 0)
							{
							BytesRead = SceneExporter_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#endif // WCS_BUILD_RTX
					// <<<>>> ADD_NEW_EFFECTS if it is in the "Other" effects class
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

} // EffectsLib::OtherEffects_Load()

/*===========================================================================*/

ULONG EffectsLib::AreaEffects_Load(FILE *ffile, ULONG ReadSize, short ByteFlip, struct ChunkIODetail *Detail)
{
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
					case WCS_EFFECTSAREA_LAKE:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_LAKE) >= 0)
							{
							BytesRead = LakeEffect_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSAREA_ECOSYSTEM:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_ECOSYSTEM) >= 0)
							{
							BytesRead = EcosystemEffect_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSAREA_ENVIRONMENT:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_ENVIRONMENT) >= 0)
							{
							BytesRead = EnvironmentEffect_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSAREA_RASTERTERRAFFECTOR:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_RASTERTERRAFFECTOR) >= 0)
							{
							BytesRead = RasterTerraffectorEffect_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSAREA_SHADOW:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_SHADOW) >= 0)
							{
							BytesRead = ShadowEffect_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSAREA_CMAP:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_CMAP) >= 0)
							{
							BytesRead = CmapEffect_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSAREA_TERRAINPARAM:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_TERRAINPARAM) >= 0)
							{
							BytesRead = TerrainParamEffect_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSAREA_GROUND:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_GROUND) >= 0)
							{
							BytesRead = GroundEffect_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSAREA_SNOW:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_SNOW) >= 0)
							{
							BytesRead = SnowEffect_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSAREA_WAVE:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_WAVE) >= 0)
							{
							BytesRead = WaveEffect_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSAREA_CLOUD:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_CLOUD) >= 0)
							{
							BytesRead = CloudEffect_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					// <<<>>> ADD_NEW_EFFECTS if it is in the "Area" effects class
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

} // EffectsLib::AreaEffects_Load()

/*===========================================================================*/

ULONG EffectsLib::LineEffects_Load(FILE *ffile, ULONG ReadSize, short ByteFlip, struct ChunkIODetail *Detail)
{
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
					case WCS_EFFECTSLINE_TERRAFFECTOR:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_TERRAFFECTOR) >= 0)
							{
							BytesRead = TerraffectorEffect_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSLINE_STREAM:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_STREAM) >= 0)
							{
							BytesRead = StreamEffect_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					// <<<>>> ADD_NEW_EFFECTS if it is in the "Line" effects class
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

} // EffectsLib::LineEffects_Load()

/*===========================================================================*/

ULONG EffectsLib::PointEffects_Load(FILE *ffile, ULONG ReadSize, short ByteFlip, struct ChunkIODetail *Detail)
{
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
					case WCS_EFFECTSPOINT_FOLIAGE:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_FOLIAGE) >= 0)
							{
							BytesRead = FoliageEffect_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTSPOINT_OBJECT3D:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_OBJECT3D) >= 0)
							{
							BytesRead = Object3DEffect_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#ifdef WCS_LABEL
					case WCS_EFFECTSPOINT_LABEL:
						{
						if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_LABEL) >= 0)
							{
							BytesRead = Label_Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#endif // WCS_LABEL
					// <<<>>> ADD_NEW_EFFECTS if it is in the "Point" effects class
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

} // EffectsLib::PointEffects_Load()

/*===========================================================================*/
/*===========================================================================*/

// <<<>>> ADD_NEW_EFFECTS somewhere, usually a separate file, include a loading function for the class

/*===========================================================================*/
/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::Save(FILE *ffile, struct ChunkIODetail *Detail)
{
char Version = WCS_EFFECTS_VERSION, Revision = WCS_EFFECTS_REVISION, str[32];
short NumClasses = WCS_MAXIMPLEMENTED_EFFECTS;
ULONG ItemTag, TotalWritten = 0, ByteOrder = 0xaabbccdd;
long BytesWritten;
struct ChunkIODetail *LocalDetail;

ProjectFileSavedWithForestEd = GlobalApp->ForestryAuthorized ? 1: 0;

if (ffile)
	{
	// no tags or sizes for first three items: version, revision & byte order
	if ((BytesWritten = WriteBlock(ffile, (char *)&Version,
		WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = WriteBlock(ffile, (char *)&Revision,
		WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = WriteBlock(ffile, (char *)&ByteOrder,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_NUMCLASSES, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (char *)&NumClasses)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GROUPVECPOLYENABLED, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_MAXIMPLEMENTED_EFFECTS,
		WCS_BLOCKTYPE_CHAR, (char *)&GroupVecPolyEnabled[0])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GROUPADVANCEDENABLED, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_MAXIMPLEMENTED_EFFECTS,
		WCS_BLOCKTYPE_CHAR, (char *)&GroupDisplayAdvancedEnabled[0])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_FILESAVEDWITHFORESTED, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
		WCS_BLOCKTYPE_LONGINT, (char *)&ProjectFileSavedWithForestEd)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_MAXMEM, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
		WCS_BLOCKTYPE_LONGINT, (char *)&MaxMem)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	#ifdef WCS_BUILD_VNS
	// Coordinate Systems
	LocalDetail = NULL;
	if (! Detail || (LocalDetail = GlobalApp->MainProj->ChunkIODetailSearch(Detail, WCS_EFFECTSIO_COORDSYS)))
		{ 
		ItemTag = WCS_EFFECTS_COORDSYS + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = CoordSys_Save(ffile))
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
					} // if coordinate systems group saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if save coordinate systems
	#endif // WCS_BUILD_VNS

	// Search Queries
	LocalDetail = NULL;
	if (! Detail || (LocalDetail = GlobalApp->MainProj->ChunkIODetailSearch(Detail, WCS_EFFECTSIO_SEARCHQUERY)))
		{ 
		ItemTag = WCS_EFFECTS_SEARCHQUERY + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = SearchQuery_Save(ffile))
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
					} // if search query group saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if save Search Queries

	// Thematic Maps
	LocalDetail = NULL;
	if (! Detail || (LocalDetail = GlobalApp->MainProj->ChunkIODetailSearch(Detail, WCS_EFFECTSIO_THEMATICMAP)))
		{ 
		ItemTag = WCS_EFFECTS_THEMATICMAP + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = ThematicMap_Save(ffile))
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
					} // if thematic map group saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if save Thematic Maps

	// Point Effects
	LocalDetail = NULL;
	if (! Detail || (LocalDetail = GlobalApp->MainProj->ChunkIODetailSearch(Detail, WCS_EFFECTSIO_POINT)))
		{
		ItemTag = WCS_EFFECTS_POINT + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = PointEffects_Save(ffile, LocalDetail))
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
					} // if point effects group saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if save point effects

	// Other Effects, ie. not area, line, point
	LocalDetail = NULL;
	if (! Detail || (LocalDetail = GlobalApp->MainProj->ChunkIODetailSearch(Detail, WCS_EFFECTSIO_OTHER)))
		{ 
		ItemTag = WCS_EFFECTS_OTHER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = OtherEffects_Save(ffile, LocalDetail))
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
					} // if other effects group saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if save other effects

	// Area Effects
	LocalDetail = NULL;
	if (! Detail || (LocalDetail = GlobalApp->MainProj->ChunkIODetailSearch(Detail, WCS_EFFECTSIO_AREA)))
		{ 
		ItemTag = WCS_EFFECTS_AREA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = AreaEffects_Save(ffile, LocalDetail))
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
					} // if area effects group saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if save area effects

	// Line Effects
	LocalDetail = NULL;
	if (! Detail || (LocalDetail = GlobalApp->MainProj->ChunkIODetailSearch(Detail, WCS_EFFECTSIO_LINE)))
		{
		ItemTag = WCS_EFFECTS_LINE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = LineEffects_Save(ffile, LocalDetail))
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
					} // if line effects group saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if save line effects

	// Scenarios
	#ifdef WCS_RENDER_SCENARIOS
	LocalDetail = NULL;
	if (! Detail || (LocalDetail = GlobalApp->MainProj->ChunkIODetailSearch(Detail, WCS_EFFECTSIO_SCENARIO)))
		{ 
		ItemTag = WCS_EFFECTS_SCENARIO + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = RenderScenario_Save(ffile))
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
					} // if Scenarios group saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if save Scenarios
	#endif // WCS_RENDER_SCENARIOS
	
	ItemTag = WCS_PARAM_DONE;
	if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if file 

return (TotalWritten);

WriteError:

sprintf(str, "Effects, Ver %d", Version);
GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, str);

return (0L);

} // EffectsLib::Save()

/*===========================================================================*/

ULONG EffectsLib::OtherEffects_Save(FILE *ffile, struct ChunkIODetail *Detail)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if (Lights)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_LIGHT) >= 0)
		{
		ItemTag = WCS_EFFECTSOTHER_LIGHT + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = Light_Save(ffile))
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
					} // if material effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if lights

if (PlanetOpts)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_PLANETOPT) >= 0)
		{
		ItemTag = WCS_EFFECTSOTHER_PLANETOPT + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = PlanetOpt_Save(ffile))
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
					} // if material effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if planet opts

if (Material)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_MATERIAL) >= 0)
		{
		ItemTag = WCS_EFFECTSOTHER_MATERIAL + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = MaterialEffect_Save(ffile))
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
					} // if material effects saved 
				else
					goto WriteError;
				} // if size written
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if material

if (Atmospheres)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_ATMOSPHERE) >= 0)
		{
		ItemTag = WCS_EFFECTSOTHER_ATMOSPHERE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = Atmosphere_Save(ffile))
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
					} // if material effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if atmospheres

if (Celestial)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_CELESTIAL) >= 0)
		{
		ItemTag = WCS_EFFECTSOTHER_CELESTIAL + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = CelestialEffect_Save(ffile))
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
					} // if material effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if celestial

if (StarField)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_STARFIELD) >= 0)
		{
		ItemTag = WCS_EFFECTSOTHER_STARFIELD + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = StarFieldEffect_Save(ffile))
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
					} // if material effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if starfields

if (Skies)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_SKY) >= 0)
		{
		ItemTag = WCS_EFFECTSOTHER_SKY + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = Sky_Save(ffile))
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
					} // if material effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if skies

if (PostProc)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_POSTPROC) >= 0)
		{
		ItemTag = WCS_EFFECTSOTHER_POSTPROC + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = PostProcess_Save(ffile))
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
					} // if post processes saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if post processes

if (RenderOpts)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_RENDEROPT) >= 0)
		{
		ItemTag = WCS_EFFECTSOTHER_RENDEROPT + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = RenderOpt_Save(ffile))
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
					} // if material effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if render opts

if (Cameras)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_CAMERA) >= 0)
		{
		ItemTag = WCS_EFFECTSOTHER_CAMERA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = Camera_Save(ffile))
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
					} // if material effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if cameras

if (RenderJobs)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_RENDERJOB) >= 0)
		{
		ItemTag = WCS_EFFECTSOTHER_RENDERJOB + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = RenderJob_Save(ffile))
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
					} // if material effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if render jobs

if (Gridder)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_GRIDDER) >= 0)
		{
		ItemTag = WCS_EFFECTSOTHER_GRIDDER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = TerraGridder_Save(ffile))
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
					} // if gridders saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if Gridder

if (Generator)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_GENERATOR) >= 0)
		{
		ItemTag = WCS_EFFECTSOTHER_GENERATOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = TerraGenerator_Save(ffile))
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
					} // if terrain generator saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if Generator

if (Fences)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_FENCE) >= 0)
		{
		ItemTag = WCS_EFFECTSOTHER_FENCE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = Fence_Save(ffile))
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
	} // if Fences

if (Merger)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_DEMMERGER) >= 0)
		{
		ItemTag = WCS_EFFECTSOTHER_DEMMERGER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = DEMMerger_Save(ffile))
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
					} // if Merger saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if Merger

if (Exporter)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_EXPORTER) >= 0)
		{
		ItemTag = WCS_EFFECTSOTHER_EXPORTER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = SceneExporter_Save(ffile))
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
					} // if Exporter saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if Exporter

// <<<>>> ADD_NEW_EFFECTS if it is in the "Other" effects class

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // EffectsLib::OtherEffects_Save()

/*===========================================================================*/

ULONG EffectsLib::AreaEffects_Save(FILE *ffile, struct ChunkIODetail *Detail)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if (TerrainParam)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_TERRAINPARAM) >= 0)
		{
		ItemTag = WCS_EFFECTSAREA_TERRAINPARAM + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = TerrainParamEffect_Save(ffile))
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
					} // if cmap effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if terrain param

if (RasterTA)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_RASTERTERRAFFECTOR) >= 0)
		{
		ItemTag = WCS_EFFECTSAREA_RASTERTERRAFFECTOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = RasterTerraffectorEffect_Save(ffile))
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
					} // if rasterTA effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if raster TA

if (Snow)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_SNOW) >= 0)
		{
		ItemTag = WCS_EFFECTSAREA_SNOW + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = SnowEffect_Save(ffile))
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
					} // if cmap effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if snow

if (Wave)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_WAVE) >= 0)
		{
		ItemTag = WCS_EFFECTSAREA_WAVE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = WaveEffect_Save(ffile))
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
					} // if wave effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if wave

if (Lake)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_LAKE) >= 0)
		{
		ItemTag = WCS_EFFECTSAREA_LAKE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = LakeEffect_Save(ffile))
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
					} // if lake effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if lakes

if (Ground)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_GROUND) >= 0)
		{
		ItemTag = WCS_EFFECTSAREA_GROUND + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = GroundEffect_Save(ffile))
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
					} // if ground effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if ground

if (Ecosystem)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_ECOSYSTEM) >= 0)
		{
		ItemTag = WCS_EFFECTSAREA_ECOSYSTEM + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = EcosystemEffect_Save(ffile))
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
					} // if ecosystem effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if ecosystems

if (Environment)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_ENVIRONMENT) >= 0)
		{
		ItemTag = WCS_EFFECTSAREA_ENVIRONMENT + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = EnvironmentEffect_Save(ffile))
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
					} // if Environment effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if environment

if (Cmap)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_CMAP) >= 0)
		{
		ItemTag = WCS_EFFECTSAREA_CMAP + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = CmapEffect_Save(ffile))
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
					} // if cmap effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if cmap

if (Shadow)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_SHADOW) >= 0)
		{
		ItemTag = WCS_EFFECTSAREA_SHADOW + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = ShadowEffect_Save(ffile))
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
					} // if shadow effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if shadow

if (Cloud)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_CLOUD) >= 0)
		{
		ItemTag = WCS_EFFECTSAREA_CLOUD + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = CloudEffect_Save(ffile))
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
					} // if cloud effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if cloud

// <<<>>> ADD_NEW_EFFECTS if it is in the "Area" effects class

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // EffectsLib::AreaEffects_Save()

/*===========================================================================*/

ULONG EffectsLib::LineEffects_Save(FILE *ffile, struct ChunkIODetail *Detail)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if (Terraffector)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_TERRAFFECTOR) >= 0)
		{
		ItemTag = WCS_EFFECTSLINE_TERRAFFECTOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = TerraffectorEffect_Save(ffile))
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
					} // if lake effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if

if (Stream)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_STREAM) >= 0)
		{
		ItemTag = WCS_EFFECTSLINE_STREAM + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = StreamEffect_Save(ffile))
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
					} // if stream effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if

// <<<>>> ADD_NEW_EFFECTS if it is in the "Line" effects class

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // EffectsLib::LineEffects_Save()

/*===========================================================================*/

ULONG EffectsLib::PointEffects_Save(FILE *ffile, struct ChunkIODetail *Detail)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if (Object3D)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_OBJECT3D) >= 0)
		{
		ItemTag = WCS_EFFECTSPOINT_OBJECT3D + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = Object3DEffect_Save(ffile))
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
					} // if lake effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if

if (Foliage)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_FOLIAGE) >= 0)
		{
		ItemTag = WCS_EFFECTSPOINT_FOLIAGE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = FoliageEffect_Save(ffile))
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
					} // if lake effects saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if

#ifdef WCS_LABEL
if (Labels)
	{
	if (! Detail || ! Detail->ItemList || GlobalApp->MainProj->ItemIODetailSearch(Detail, WCS_EFFECTSIO_LABEL) >= 0)
		{
		ItemTag = WCS_EFFECTSPOINT_LABEL + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = Label_Save(ffile))
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
					} // if labels saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	} // if
#endif // WCS_LABEL

// <<<>>> ADD_NEW_EFFECTS if it is in the "Point" effects class

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // EffectsLib::PointEffects_Save()

/*===========================================================================*/
/*===========================================================================*/

// <<<>>> ADD_NEW_EFFECTS somewhere, usually in a separate file, add a class saver function

/*===========================================================================*/

void EffectsLib::ResolveLoadLinkages(void)
{
long Ct;
GeneralEffect *CurEffect;

for (Ct = WCS_EFFECTSSUBCLASS_LAKE; Ct < WCS_MAXIMPLEMENTED_EFFECTS; Ct ++)
	{
	CurEffect = GetListPtr(Ct);
	while (CurEffect)
		{
		CurEffect->ResolveLoadLinkages(this);
		CurEffect = CurEffect->Next;
		} // while
	} // for

} // EffectsLib::ResolveLoadLinkages

/*===========================================================================*/

void EffectsLib::ResolveDBLoadLinkages(Database *HostDB)
{
long Ct;
unsigned long HighestDBID;
GeneralEffect *CurEffect;
Joe **UniqueIDTable;

if ((HighestDBID = HostDB->CountHighestUniqueID()) > 0)
	{
	if (UniqueIDTable = HostDB->PopulateUniqueIDTable(HighestDBID))
		{
		for (Ct = WCS_EFFECTSSUBCLASS_LAKE; Ct < WCS_MAXIMPLEMENTED_EFFECTS; Ct ++)
			{
			CurEffect = GetListPtr(Ct);
			while (CurEffect)
				{
				CurEffect->ResolveDBLoadLinkages(HostDB, UniqueIDTable, HighestDBID);
				CurEffect = CurEffect->Next;
				} // while
			} // for
		HostDB->FreeUniqueIDTable(UniqueIDTable);
		} // if
	} // if

} // EffectsLib::ResolveLoadLinkages

/*===========================================================================*/
/*===========================================================================*/

Polygon3DRenderData::Polygon3DRenderData()
{

Mat = NULL;
Obj = NULL;
TexData = new TextureData();
ShadingModel = 0;
ShadowInstance = 0;
AffectedByHaze = AffectedByFog = TexturesExist = 0;
SunAngle[0] = SunAngle[1] = SunAngle[2] = Illumination = ShadowIntensity =
	DefLuminosity = DefTransparency = DefSpecularity = DefSpecularExp = DefTransluminance = DefTranslumExp =
	Luminosity = Transparency = Specularity = SpecularExp = Transluminance = TranslumExp = 0.0;
VtxNormalPtr[0] = VtxNormalPtr[1] = VtxNormalPtr[2] = VertexPtr[0] = VertexPtr[1] = VertexPtr[2] = NULL;
ViewVec[0] = ViewVec[1] = ViewVec[2] = 0.0;
NegViewVec[0] = NegViewVec[1] = NegViewVec[2] = 0.0;
LocalVertexPtr[0] = LocalVertexPtr[1] = LocalVertexPtr[2] = NULL;
ShadowMap = NULL;
SunVec = NULL;
DefCC.RGB[0] = DefCC.RGB[1] = DefCC.RGB[2] = CC.RGB[0] = CC.RGB[1] = CC.RGB[2] = 0;

} // Polygon3DRenderData::Polygon3DRenderData

/*===========================================================================*/

Polygon3DRenderData::~Polygon3DRenderData()
{

if (TexData)
	delete TexData;

} // Polygon3DRenderData::~Polygon3DRenderData

/*===========================================================================*/
/*===========================================================================*/

RasterAnimHostProperties::RasterAnimHostProperties()
{

Flags = InterFlags = PopClassFlags = PopEnabledFlags = PopExistsFlags = 0;
PropMask = (unsigned long)~0;
FlagsMask = (unsigned long)~0;
Name = NULL;
Type = NULL;
Path = NULL;
Ext = NULL;
ChildA = NULL;
ChildB = NULL;
DropSource = NULL;
TypeNumber = ChildTypeFilter = 0;
ByteFlip = 0;
ItemOperator = FrameOperator = KeyframeOperation = DropOK = 0;
KeyNodeRange[0] = KeyNodeRange[1] = NewKeyNodeRange[0] = NewKeyNodeRange[1] = 0.0;
Queries = 1;
FileVersion = FileRevision = 0;

} // RasterAnimHostProperties::RasterAnimHostProperties

/*===========================================================================*/

void RasterAnimHostProperties::Copy(RasterAnimHostProperties *CopyFrom)
{

Flags = CopyFrom->Flags;
InterFlags = CopyFrom->InterFlags;
PropMask = CopyFrom->PropMask;
FlagsMask = CopyFrom->FlagsMask;
Name = CopyFrom->Name;
Type = CopyFrom->Type;
Path = CopyFrom->Path;
Ext = CopyFrom->Ext;
DropSource = CopyFrom->DropSource;
TypeNumber = CopyFrom->TypeNumber;
ChildTypeFilter = CopyFrom->ChildTypeFilter;
ByteFlip = CopyFrom->ByteFlip;
ItemOperator = CopyFrom->ItemOperator;
FrameOperator = CopyFrom->FrameOperator;
KeyframeOperation = CopyFrom->KeyframeOperation;
DropOK = CopyFrom->DropOK;
KeyNodeRange[0] = CopyFrom->KeyNodeRange[0];
KeyNodeRange[1] = CopyFrom->KeyNodeRange[1];
NewKeyNodeRange[0] = CopyFrom->NewKeyNodeRange[0];
NewKeyNodeRange[1] = CopyFrom->NewKeyNodeRange[1];
Queries = CopyFrom->Queries;
FileVersion = CopyFrom->FileVersion;
FileRevision = CopyFrom->FileRevision;

} // RasterAnimHostProperties::Copy

/*===========================================================================*/
/*===========================================================================*/

RasterAnimHost::~RasterAnimHost()
{

if (GlobalApp && GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->LVE && GlobalApp->GUIWins->LVE->GetActive() == this)
		{
		delete GlobalApp->GUIWins->LVE;
		GlobalApp->GUIWins->LVE = NULL;
		} // if
	if (GlobalApp->GUIWins->BRD && GlobalApp->GUIWins->BRD->GetActive() == this)
		{
		delete GlobalApp->GUIWins->BRD;
		GlobalApp->GUIWins->BRD = NULL;
		} // if
	} // if

if (CopyOfRAHost == this)
	CopyOfRAHost = NULL;

if (BackupRAHost == this)
	BackupRAHost = NULL;

if (ActiveRAHost == this)
	{
	ActiveRAHost = NULL;
	if (GlobalApp && GlobalApp->MCP)
		GlobalApp->MCP->SetCurrentObject(NULL, 0);
	} // if

if (BrowseInfo)
	delete BrowseInfo;
BrowseInfo = NULL;

} // RasterAnimHost::~RasterAnimHost

/*===========================================================================*/

void RasterAnimHost::Copy(RasterAnimHost *CopyTo, RasterAnimHost *CopyFrom)
{

if (CopyFrom->BrowseInfo)
	{
	if (CopyTo->BrowseInfo || (CopyTo->BrowseInfo = new BrowseData()))
		CopyTo->BrowseInfo->Copy(CopyTo->BrowseInfo, CopyFrom->BrowseInfo);
	} // if
else if (CopyTo->BrowseInfo)
	{
	delete CopyTo->BrowseInfo;
	CopyTo->BrowseInfo = NULL;
	} // else if

} // RasterAnimHost::Copy

/*===========================================================================*/

// this now searches children since there is a way to avoid endless loops
int RasterAnimHost::FindnRemoveRAHostChild(RasterAnimHost *RemoveMe, int &RemoveAll)
{
RasterAnimHost *CurChild = NULL;
int DontAdvance = 0, Result = 0;

while ((DontAdvance && CurChild) || (! DontAdvance && (CurChild = GetRAHostChild(CurChild, 0))))
	{
	DontAdvance = 0;
	if (CurChild == RemoveMe)
		{
		if (RemoveAll || (Result = RemoveRAHostQuery(RemoveMe)))
			{
			RemoveAll = RemoveAll || (Result >= 2);
			CurChild = GetRAHostChild(CurChild, 0);
			DontAdvance = 1; 
			RemoveRAHost(RemoveMe);
			} // if
		else
			return (0);
		} // if
	else if (CurChild->RAParent == this)
		{
		if (! CurChild->FindnRemoveRAHostChild(RemoveMe, RemoveAll))
			return (0);
		} // else if
	} // while

return (1);

} // RasterAnimHost::FindnRemoveRAHostChild

/*===========================================================================*/

int RasterAnimHost::RemoveRAHostQuery(RasterAnimHost *RemoveMe)
{
char QueryStr[256], NameStr[128];
RasterAnimHostProperties Prop;

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
RemoveMe->GetRAHostProperties(&Prop);
sprintf(QueryStr, "Remove %s %s from %s?", Prop.Name, Prop.Type, NameStr);

return (UserMessageCustom(NameStr, QueryStr, "Yes", "No", "Yes to All", 0));

} // RasterAnimHost::RemoveRAHostQuery

/*===========================================================================*/

int RasterAnimHost::GetPopClassFlags(RasterAnimHostProperties *Prop, AnimCritter *AnimAffil,
	RootTexture **TexAffil, ThematicMap **ThemeAffil)
{

if (AnimAffil)
	{
	Prop->PopClassFlags |= WCS_RAH_POPMENU_CLASS_ANIM;
	if(AnimAffil->GetRAHostAnimated())
		{
		Prop->PopExistsFlags |= WCS_RAH_POPMENU_CLASS_ANIM;
		Prop->PopEnabledFlags |= WCS_RAH_POPMENU_CLASS_ANIM;
		} // if
	} // if
if (TexAffil)
	{
	Prop->PopClassFlags |= WCS_RAH_POPMENU_CLASS_TEX;
	if (*TexAffil)
		{
		Prop->PopExistsFlags |= WCS_RAH_POPMENU_CLASS_TEX;
		if ((*TexAffil)->Enabled)
			Prop->PopEnabledFlags |= WCS_RAH_POPMENU_CLASS_TEX;
		} // if
	} // if
#ifdef WCS_THEMATIC_MAP
if (ThemeAffil)
	{
	Prop->PopClassFlags |= WCS_RAH_POPMENU_CLASS_THEME;
	if (*ThemeAffil)
		{
		Prop->PopExistsFlags |= WCS_RAH_POPMENU_CLASS_THEME;
		if ((*ThemeAffil)->Enabled)
			Prop->PopEnabledFlags |= WCS_RAH_POPMENU_CLASS_THEME;
		} // if
	} // if
#endif // WCS_THEMATIC_MAP

return (Prop->PopClassFlags ? 1: 0);

} // RasterAnimHost::GetPopClassFlags

/*===========================================================================*/

int RasterAnimHost::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, 
	RasterAnimHost **ChildB, AnimCritter *AnimAffil, RootTexture **TexAffil, ThematicMap **ThemeAffil)
{
int MenuAdded = 0, PasteTexOK = 0, PasteAnimOK = 0;
double CurTime, Value;
char NameStr[256];
RasterAnimHostProperties Prop;

if (AnimAffil && MenuClassFlags & WCS_RAH_POPMENU_CLASS_ANIM)
	{ // anim related
	if (RasterAnimHost::GetCopyOfRAHost())
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
		Prop.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
		RasterAnimHost::GetCopyOfRAHost()->GetRAHostProperties(&Prop);
		Prop.PropMask = WCS_RAHOST_MASKBIT_DROPOK;
		AnimAffil->GetRAHostProperties(&Prop);
		if (Prop.DropOK)
			PasteAnimOK = 1;
		} // if
	Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_KEYRANGE;
	Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATABLE | WCS_RAHOST_FLAGBIT_ANIMATED;
	AnimAffil->GetRAHostProperties(&Prop);

	if (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATABLE)
		{ // keyframe related
		PMA->AddPopMenuItem("Create Key", "AD_CREATEKEY", 1, 1);
		PMA->AddPopMenuItem("View Timeline", "AD_VIEWTIMELINE", 1, 1);
		if (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
			{
			PMA->AddPopMenuItem("Delete Key(s)", "AD_DELETEKEY", 1, 1);
			PMA->AddPopMenuItem("Scale Keyframe(s)", "AD_SCALEKEY", 1, 1);
			CurTime = GlobalApp->MainProj->Interactive->GetActiveTime();
			if (CurTime > Prop.KeyNodeRange[0])
				PMA->AddPopMenuItem("Previous Key", "AD_PREVKEY", 1, 1);
			if (CurTime < Prop.KeyNodeRange[1])
				PMA->AddPopMenuItem("Next Key", "AD_NEXTKEY", 1, 1);
			} // if
		} // if
	if (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
		{
		PMA->AddPopMenuItem("Copy Value/Key Frame(s)", "AD_COPY", 1, 1);
		if (PasteAnimOK)
			PMA->AddPopMenuItem("Paste Value/Key Frame(s)", "AD_PASTE", 1, 1);
		} // if
	else
		{
		PMA->AddPopMenuItem("Copy Value", "AD_COPY", 1, 1);
		if (PasteAnimOK)
			PMA->AddPopMenuItem("Paste Value", "AD_PASTE", 1, 1);
		} // else
	sprintf(NameStr, "Activate \'%s\'", Prop.Name);
	PMA->AddPopMenuItem(NameStr, "AD_ACTIVATE", 1, 1);
	PMA->AddPopMenuItem("Value Limits", "AD_VALUE", 1, 1);
	Value = AnimAffil->GetMaxVal();
	if (Value == FLT_MAX || Value == DBL_MAX)
		sprintf(NameStr, "  Max %s", "Very Large");
	else if (Value == -FLT_MAX || Value == -DBL_MAX)
		sprintf(NameStr, "  Max %s", "-Very Large");
	else
		{
		sprintf(NameStr, "  Max %f", Value * AnimAffil->GetMultiplier());
		TrimZeros(NameStr);
		} // else
	strcat(NameStr, " ");
	strcat(NameStr, AnimAffil->GetMetricSpecifier());
	PMA->AddPopMenuItem(NameStr, "AD_VALUE", 1, 1);
	Value = AnimAffil->GetMinVal();
	if (Value == FLT_MAX || Value == DBL_MAX)
		sprintf(NameStr, "  Min %s", "Very Large");
	else if (Value == -FLT_MAX || Value == -DBL_MAX)
		sprintf(NameStr, "  Min %s", "-Very Large");
	else
		{
		sprintf(NameStr, "  Min %f", Value * AnimAffil->GetMultiplier());
		TrimZeros(NameStr);
		} // else
	strcat(NameStr, " ");
	strcat(NameStr, AnimAffil->GetMetricSpecifier());
	PMA->AddPopMenuItem(NameStr, "AD_VALUE", 1, 1);
	MenuAdded = 1;
	} // if
if (TexAffil && MenuClassFlags & WCS_RAH_POPMENU_CLASS_TEX)
	{ // texture related
	if (RasterAnimHost::GetCopyOfRAHost())
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
		Prop.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
		RasterAnimHost::GetCopyOfRAHost()->GetRAHostProperties(&Prop);
		if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ROOTTEXTURE)
			PasteTexOK = 1;
		} // if
	if (! *TexAffil)
		{ //no texture
		if (PasteTexOK)
			PMA->AddPopMenuItem("Paste Texture", "TX_PASTE", 1, 1);
		PMA->AddPopMenuItem("Create Texture", "TX_CREATE", 1, 1);
		} // if
	bool ShowDrapeGeo = true;
	// we'll always show it, but show a N/A submenu if no items available
	/*
	for (Raster *MyRast = GlobalApp->AppImages->GetFirstRast(); MyRast; MyRast = GlobalApp->AppImages->GetNextRast(MyRast))
		{
		if(MyRast->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF))
			{
			ShowDrapeGeo = true;
			} // if
		} // for
	*/
	if(ShowDrapeGeo) // only offer this if some georeferenced images are available
		{
		PMA->BeginPopSubMenu("Drape Georeferenced Image");
		int ImageCount = 0;
		for (Raster *MyRast = GlobalApp->AppImages->GetFirstRast(); MyRast; MyRast = GlobalApp->AppImages->GetNextRast(MyRast))
			{
			if(MyRast->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF))
				{
				bool Checked = false;
				char ImageMenuText[32];

				if(*TexAffil) // is there an existing texture?
					{
					Texture *Tex;
					if(Tex = (*TexAffil)->Tex)
						{
						if(Tex->GetTexType() == WCS_TEXTURE_TYPE_PLANARIMAGE)
							{
							if (Tex->Img)
								{
								if (Tex->Img->GetRaster() == MyRast)
									{
									Checked = true;
									} // if
								} // if
							} // if
						} // if
					} // if

				sprintf(ImageMenuText, "TX_DRAPEGEOIMAGE_%04d", ImageCount++);
				PMA->AddPopMenuItem(MyRast->GetUserName(), ImageMenuText, 1, 1, Checked ? 1 : 0);
				} // if
			} // for
		if(ImageCount == 0)
			{
			PMA->AddPopMenuItem("No Georeferenced Images Available", "TX_NOGEOIMAGESAVAIL", 1, 0, 0);
			} // if
		PMA->EndPopSubMenu();
		} // if
	if (*TexAffil)
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_KEYRANGE;
		Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATABLE | WCS_RAHOST_FLAGBIT_ANIMATED;
		(*TexAffil)->GetRAHostProperties(&Prop);
		PMA->AddPopMenuItem("Edit Texture", "TX_EDIT", 1, 1);
		PMA->AddPopMenuItem("Copy Texture", "TX_COPY", 1, 1);
		if (PasteTexOK)
			PMA->AddPopMenuItem("Paste Texture", "TX_PASTE", 1, 1);
		PMA->AddPopMenuItem("Delete Texture", "TX_DELETE", 1, 1);
		if ((*TexAffil)->Enabled)
			PMA->AddPopMenuItem("Disable Texture", "TX_DISABLE", 1, 1);
		else
			PMA->AddPopMenuItem("Enable Texture", "TX_ENABLE", 1, 1);
		if (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
			{
			PMA->AddPopMenuItem("Delete Texture Key(s)", "TX_DELETEKEY", 1, 1);
			PMA->AddPopMenuItem("Scale Texture Keyframe(s)", "TX_SCALEKEY", 1, 1);
			CurTime = GlobalApp->MainProj->Interactive->GetActiveTime();
			if (CurTime > Prop.KeyNodeRange[0])
				PMA->AddPopMenuItem("Previous Texture Key", "TX_PREVKEY", 1, 1);
			if (CurTime < Prop.KeyNodeRange[1])
				PMA->AddPopMenuItem("Next Texture Key", "TX_NEXTKEY", 1, 1);
			} // if
		PMA->AddPopMenuItem("Activate Texture", "TX_ACTIVATE", 1, 1);
		} // else
	MenuAdded = 1;
	} // if
#ifdef WCS_THEMATIC_MAP
if (ThemeAffil && MenuClassFlags & WCS_RAH_POPMENU_CLASS_THEME)
	{ // theme related
	bool ShowThematicLinkTo = false;
	if(1) // should we only show this if some attributes are available?
		{
		ShowThematicLinkTo = true;
		} // if
	if(ShowThematicLinkTo)
		{
		char *PrevAttributeName = NULL;
		double PrevThemeMult = 0.0;
		PMA->BeginPopSubMenu("Link To Attribute");

		// identify current thematic attribute, if any
		if(*ThemeAffil)
			{
			// is there a non-blank attribute already recorded here?
			if(strlen((*ThemeAffil)->AttribField[0]))
				{
				PrevAttributeName = (*ThemeAffil)->AttribField[0];
				PrevThemeMult = (*ThemeAffil)->AttribFactor[0];
				} // if
			} // if

		int ThemeCount = 0;
		LayerEntry *Entry;
		Database *DBHost = GlobalApp->AppDB; // can't get it any other way, easily.
		const char *LayerName;
		for(Entry = DBHost->DBLayers.FirstEntry(); Entry; Entry = DBHost->DBLayers.NextEntry(Entry))
			{
			if (! Entry->TestFlags(WCS_LAYER_LINKATTRIBUTE))
				{
				LayerName = Entry->GetName();
				if (LayerName[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_DBL) // is it a numeric attribute
					{
					bool AttribChecked = false;
					char ThemeMenuText[32], ThemeMenuUnit[32];
					if(PrevAttributeName && !stricmp(PrevAttributeName, LayerName)) AttribChecked = true; // if it's a name match, we'll display a check
					PMA->BeginPopSubMenu(&LayerName[1], 1, AttribChecked ? 1 : 0);
					sprintf(ThemeMenuText, "TM_LINKATTRIB_%04d_nnnn", ThemeCount);
					PMA->AddPopMenuItem("Without Unit Conversion", ThemeMenuText, 1, 1, AttribChecked && (PrevThemeMult == 1.0 ? 1 : 0));
					for(int UnitCount = 0; ThematicMapEditGUI::MultPresetNames[UnitCount]; UnitCount++)
						{
						bool Checked = false;
						
						// is the current item the represenation of the unit scaling factor in currently effect?
						if(PrevThemeMult == ThematicMapEditGUI::MultPresetValues[UnitCount])
							{
							Checked = true; // display a checkmark
							} // if
						sprintf(ThemeMenuText, "TM_LINKATTRIB_%04d_%04d", ThemeCount, UnitCount);
						sprintf(ThemeMenuUnit, "via %s", ThematicMapEditGUI::MultPresetNames[UnitCount]);
						// we only show unit check if we're also on the submenu of the checked attribute
						PMA->AddPopMenuItem(ThemeMenuUnit, ThemeMenuText, 1, 1, AttribChecked && Checked ? 1 : 0);
						} // for					
					PMA->EndPopSubMenu();
					ThemeCount++;
					} // if an attribute layer
				} // if
			} // for
		if(ThemeCount == 0)
			{
			PMA->AddPopMenuItem("No Numeric Attributes Available", "TM_NONUMERICATTRIBAVAIL", 1, 0, 0);
			} // if
		PMA->EndPopSubMenu();
		} // if
	if (! *ThemeAffil)
		{ //no theme
		PMA->AddPopMenuItem("Create Thematic Map", "TM_CREATE", 1, 1);
		PMA->AddPopMenuItem("Link Thematic Map", "TM_LINK", 1, 1);
		} // if
	else
		{
		sprintf(NameStr, "Edit \'%s\'", (*ThemeAffil)->GetName());
		PMA->AddPopMenuItem(NameStr, "TM_EDIT", 1, 1);
		PMA->AddPopMenuItem("Change Thematic Link", "TM_LINK", 1, 1);
		PMA->AddPopMenuItem("Remove Thematic Link", "TM_DELETE", 1, 1);
		if ((*ThemeAffil)->Enabled)
			{
			sprintf(NameStr, "Disable \'%s\'", (*ThemeAffil)->GetName());
			PMA->AddPopMenuItem(NameStr, "TM_DISABLE", 1, 1);
			} // if
		else
			{
			sprintf(NameStr, "Enable \'%s\'", (*ThemeAffil)->GetName());
			PMA->AddPopMenuItem(NameStr, "TM_ENABLE", 1, 1);
			} // else
		sprintf(NameStr, "Activate \'%s\'", (*ThemeAffil)->GetName());
		PMA->AddPopMenuItem(NameStr, "TM_ACTIVATE", 1, 1);
		} // else
	MenuAdded = 1;
	} // if
#endif // WCS_THEMATIC_MAP

return(MenuAdded);

} // RasterAnimHost::AddSRAHBasePopMenus

/*===========================================================================*/

int RasterAnimHost::HandleSRAHPopMenuSelection(void *Action, AnimCritter *AnimAffil, RootTexture **TexAffil, 
	ThematicMap **ThemeAffil, RootTextureParent *TexParent, ThematicOwner *ThemeOwner)
{
int Handled = 0, RemoveAll = 0, TexCreated = 0, SiblingsExist, CopyResult;
long ItemNumber;
#ifdef WCS_THEMATIC_MAP
ThematicMap *NewTheme;
#endif // WCS_THEMATIC_MAP
RasterAnimHost *PasteHost, *Sib;
RasterAnimHostProperties Prop;
char CopyMsg[256];
NotifyTag Changes[2];

if (! strncmp((char *)Action, "AD", 2))
	{
	if (! strcmp((char *)Action, "AD_ACTIVATE") && AnimAffil)
		{
		RasterAnimHost::SetActiveRAHost((RasterAnimHost *)AnimAffil, 1);
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "AD_VIEWTIMELINE") && AnimAffil)
		{
		AnimAffil->OpenTimeline();
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "AD_CREATEKEY") && AnimAffil)
		{
		AnimAffil->AddNode();
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "AD_COPY") && AnimAffil)
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
		Prop.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
		AnimAffil->GetRAHostProperties(&Prop);
		if (Prop.Flags & WCS_RAHOST_FLAGBIT_DRAGGABLE)
			{
			RasterAnimHost::SetCopyOfRAHost(AnimAffil);
			sprintf(CopyMsg, "%s %s copied to clipboard", Prop.Name ? Prop.Name: "", Prop.Type ? Prop.Type: "");
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
			} // if
		else
			{
			UserMessageOK("Copy", "Selected item cannot be copied.");
			} // else
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "AD_PASTE") && AnimAffil)
		{
		if (RasterAnimHost::GetCopyOfRAHost())
			{
			Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
			Prop.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
			RasterAnimHost::GetCopyOfRAHost()->GetRAHostProperties(&Prop);
			Prop.PropMask = WCS_RAHOST_MASKBIT_DROPOK;
			AnimAffil->GetRAHostProperties(&Prop);
			if (Prop.DropOK)
				{
				Prop.PropMask = WCS_RAHOST_MASKBIT_DROPSOURCE;
				Prop.DropSource = RasterAnimHost::GetCopyOfRAHost();
				// Result = 1 if drop complete, 0 if failed and -1 if inconclusive, 
				//  eg. still in progress through a DragnDropListGUI
				AnimAffil->SetRAHostProperties(&Prop);
				sprintf(CopyMsg, "%s %s pasted from clipboard", Prop.Name ? Prop.Name: "", Prop.Type ? Prop.Type: "");
				GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
				} // if
			else
				{
				UserMessageOK(Prop.Name ? Prop.Name: "Paste", "Item in copy buffer cannot be pasted on selected target.");
				} // else
			} // if
		else
			{
			UserMessageOK("Paste", "There is nothing in the clipboard to paste.");
			} // else
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "AD_DELETEKEY") && AnimAffil)
		{
		if (GlobalApp->GUIWins->DKG)
			{
			delete GlobalApp->GUIWins->DKG;
			GlobalApp->GUIWins->DKG = NULL;
			} // if
		if (GlobalApp->GUIWins->DKG = new KeyScaleDeleteGUI(GlobalApp->MainProj, GlobalApp->AppEffects, AnimAffil, WCS_KEYOPERATION_DELETE))
			{
			if (GlobalApp->GUIWins->DKG->ConstructError)
				{
				delete GlobalApp->GUIWins->DKG;
				GlobalApp->GUIWins->DKG = NULL;
				} // if
			} // if
		if (GlobalApp->GUIWins->DKG)
			{
			GlobalApp->GUIWins->DKG->Open(GlobalApp->MainProj);
			GlobalApp->GUIWins->DKG->SetModalInhibit(TRUE);
			} // if
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "AD_SCALEKEY") && AnimAffil)
		{
		if (GlobalApp->GUIWins->DKG)
			{
			delete GlobalApp->GUIWins->DKG;
			GlobalApp->GUIWins->DKG = NULL;
			} // if
		if (GlobalApp->GUIWins->DKG = new KeyScaleDeleteGUI(GlobalApp->MainProj, GlobalApp->AppEffects, AnimAffil, WCS_KEYOPERATION_SCALE))
			{
			if (GlobalApp->GUIWins->DKG->ConstructError)
				{
				delete GlobalApp->GUIWins->DKG;
				GlobalApp->GUIWins->DKG = NULL;
				} // if
			} // if
		if (GlobalApp->GUIWins->DKG)
			{
			GlobalApp->GUIWins->DKG->Open(GlobalApp->MainProj);
			GlobalApp->GUIWins->DKG->SetModalInhibit(TRUE);
			} // if
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "AD_PREVKEY") && AnimAffil)
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
		Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = GlobalApp->MainProj->Interactive->GetActiveTime();
		Prop.NewKeyNodeRange[0] = -DBL_MAX;
		Prop.NewKeyNodeRange[1] = DBL_MAX;
		SiblingsExist = (AnimAffil->RAParent && (Sib = AnimAffil->RAParent->GetNextGroupSibling(AnimAffil))
			&& (Sib != AnimAffil));
		Prop.ItemOperator = SiblingsExist && GlobalApp->MainProj->GetKeyGroupMode() ? WCS_KEYOPERATION_CUROBJGROUP: WCS_KEYOPERATION_CUROBJ;
		AnimAffil->GetRAHostProperties(&Prop);
		if (Prop.NewKeyNodeRange[0] > Prop.KeyNodeRange[0] || Prop.NewKeyNodeRange[0] == -DBL_MAX)
			Prop.NewKeyNodeRange[0] = Prop.KeyNodeRange[0];
		if (Prop.NewKeyNodeRange[0] < Prop.KeyNodeRange[0])
			GlobalApp->MainProj->Interactive->SetActiveTime(Prop.NewKeyNodeRange[0]);
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "AD_NEXTKEY") && AnimAffil)
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
		Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = GlobalApp->MainProj->Interactive->GetActiveTime();
		Prop.NewKeyNodeRange[0] = -DBL_MAX;
		Prop.NewKeyNodeRange[1] = DBL_MAX;
		SiblingsExist = (AnimAffil->RAParent && (Sib = AnimAffil->RAParent->GetNextGroupSibling(AnimAffil))
			&& (Sib != AnimAffil));
		Prop.ItemOperator = SiblingsExist && GlobalApp->MainProj->GetKeyGroupMode() ? WCS_KEYOPERATION_CUROBJGROUP: WCS_KEYOPERATION_CUROBJ;
		AnimAffil->GetRAHostProperties(&Prop);
		if (Prop.NewKeyNodeRange[1] < Prop.KeyNodeRange[1] || Prop.NewKeyNodeRange[1] == DBL_MAX)
			Prop.NewKeyNodeRange[1] = Prop.KeyNodeRange[1];
		if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1])
			GlobalApp->MainProj->Interactive->SetActiveTime(Prop.NewKeyNodeRange[1]);
		Handled = 1;
		} // if
	} // if anim
else if (! strncmp((char *)Action, "TX", 2))
	{
	if (! strcmp((char *)Action, "TX_ACTIVATE") && TexAffil && (*TexAffil))
		{
		RasterAnimHost::SetActiveRAHost((RasterAnimHost *)(*TexAffil), 1);
		Handled = 1;
		} // if
	// TX_DRAPEGEOIMAGE
	else if (! strncmp((char *)Action, "TX_DRAPEGEOIMAGE_", 17) && TexAffil) // this works even if a texture exists. it will be discarded
		{
		Raster *DrapeImage = NULL;
		int WhichImage;
		WhichImage = atoi(&((char *)Action)[17]);

		int ImageCount = 0;
		for (Raster *MyRast = GlobalApp->AppImages->GetFirstRast(); MyRast; MyRast = GlobalApp->AppImages->GetNextRast(MyRast))
			{
			if(MyRast->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF))
				{
				if(ImageCount == WhichImage)
					{
					DrapeImage = MyRast;
					break;
					} // if
				ImageCount++;
				} // if
			} // for

		if(*TexAffil) // is there an existing texture?
			{ // delete it
			FindnRemoveRAHostChild(*TexAffil, RemoveAll);
			} // if
		// create a new one
		RootTexture *DrapeTextureRoot;
		if (((ItemNumber = TexParent->GetTexNumberFromAddr(TexAffil)) >= 0) && ((DrapeTextureRoot = TexParent->NewRootTexture(ItemNumber)) != NULL))
			{
			// set it up as a planar georeferenced image
			Texture *Tex;
			if (Tex = DrapeTextureRoot->AddNewTexture(NULL, WCS_TEXTURE_TYPE_PLANARIMAGE))
				{
				// install image
				Tex->SetRaster(DrapeImage);
				Tex->CoordSpace = WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED;
				} // if
			
			// edit it?
			//(*TexAffil)->EditRAHost();
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), WCS_SUBCLASS_ROOTTEXTURE, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "TX_CREATE") && TexAffil && ! (*TexAffil))
		{
		if ((ItemNumber = TexParent->GetTexNumberFromAddr(TexAffil)) >= 0 && TexParent->NewRootTexture(ItemNumber))
			{
			(*TexAffil)->EditRAHost();
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), WCS_SUBCLASS_ROOTTEXTURE, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "TX_EDIT") && TexAffil && (*TexAffil))
		{
		(*TexAffil)->EditRAHost();
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "TX_DELETE") && TexAffil && (*TexAffil))
		{
		FindnRemoveRAHostChild(*TexAffil, RemoveAll);
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "TX_ENABLE") && TexAffil && (*TexAffil))
		{
		Prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
		Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;
		Prop.Flags = WCS_RAHOST_FLAGBIT_ENABLED;
		(*TexAffil)->SetRAHostProperties(&Prop);
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "TX_DISABLE") && TexAffil && (*TexAffil))
		{
		Prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
		Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;
		Prop.Flags = 0;
		(*TexAffil)->SetRAHostProperties(&Prop);
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "TX_COPY") && TexAffil && (*TexAffil))
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
		Prop.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
		(*TexAffil)->GetRAHostProperties(&Prop);
		if (Prop.Flags & WCS_RAHOST_FLAGBIT_DRAGGABLE)
			{
			RasterAnimHost::SetCopyOfRAHost(*TexAffil);
			sprintf(CopyMsg, "%s %s copied to clipboard", Prop.Name ? Prop.Name: "", Prop.Type ? Prop.Type: "");
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
			} // if
		else
			{
			UserMessageOK("Copy", "Selected item cannot be copied.");
			} // else
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "TX_PASTE") && TexAffil)
		{
		if (RasterAnimHost::GetCopyOfRAHost())
			{
			Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
			Prop.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
			RasterAnimHost::GetCopyOfRAHost()->GetRAHostProperties(&Prop);
			Prop.PropMask = WCS_RAHOST_MASKBIT_DROPOK;
			if (*TexAffil)
				{
				// texture already exists. Copy directly to it.
				PasteHost = (*TexAffil);
				} // if
			else if (TexParent && (ItemNumber = TexParent->GetTexNumberFromAddr(TexAffil)) >= 0 && TexParent->NewRootTexture(ItemNumber))
				{
				// new texture created. Copy directly to it
				TexCreated = 1;
				PasteHost = *TexAffil;
				Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
				} // if
			else
				{
				// unable to create new texture or copy to existing one. Try drag/drop on this
				PasteHost = this;
				} // else
			PasteHost->GetRAHostProperties(&Prop);
			if (Prop.DropOK)
				{
				Prop.PropMask = WCS_RAHOST_MASKBIT_DROPSOURCE;
				Prop.DropSource = RasterAnimHost::GetCopyOfRAHost();
				// Result = 1 if drop complete, 0 if failed and -1 if inconclusive, 
				//  eg. still in progress through a DragnDropListGUI
				CopyResult = PasteHost->SetRAHostProperties(&Prop);
				if (CopyResult == 0 && TexCreated)
					{
					// copying aborted to newly created texture: delete created texture
					PasteHost = *TexAffil;
					TexParent->SetTexRootPtr(ItemNumber, NULL);
					delete (RootTexture *)PasteHost;
					Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
					} // if
				else
					{
					sprintf(CopyMsg, "%s %s pasted from clipboard", Prop.Name ? Prop.Name: "", Prop.Type ? Prop.Type: "");
					GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
					} // else
				} // if
			else
				{
				UserMessageOK(Prop.Name ? Prop.Name: "Paste", "Item in copy buffer cannot be pasted on selected target.");
				} // else
			} // if
		else
			{
			UserMessageOK("Paste", "There is nothing in the clipboard to paste.");
			} // else
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "TX_DELETEKEY") && TexAffil && (*TexAffil))
		{
		if (GlobalApp->GUIWins->DKG)
			{
			delete GlobalApp->GUIWins->DKG;
			GlobalApp->GUIWins->DKG = NULL;
			} // if
		if (GlobalApp->GUIWins->DKG = new KeyScaleDeleteGUI(GlobalApp->MainProj, GlobalApp->AppEffects, (*TexAffil), WCS_KEYOPERATION_DELETE))
			{
			if (GlobalApp->GUIWins->DKG->ConstructError)
				{
				delete GlobalApp->GUIWins->DKG;
				GlobalApp->GUIWins->DKG = NULL;
				} // if
			} // if
		if (GlobalApp->GUIWins->DKG)
			{
			GlobalApp->GUIWins->DKG->Open(GlobalApp->MainProj);
			GlobalApp->GUIWins->DKG->SetModalInhibit(TRUE);
			} // if
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "TX_SCALEKEY") && TexAffil && (*TexAffil))
		{
		if (GlobalApp->GUIWins->DKG)
			{
			delete GlobalApp->GUIWins->DKG;
			GlobalApp->GUIWins->DKG = NULL;
			} // if
		if (GlobalApp->GUIWins->DKG = new KeyScaleDeleteGUI(GlobalApp->MainProj, GlobalApp->AppEffects, (*TexAffil), WCS_KEYOPERATION_SCALE))
			{
			if (GlobalApp->GUIWins->DKG->ConstructError)
				{
				delete GlobalApp->GUIWins->DKG;
				GlobalApp->GUIWins->DKG = NULL;
				} // if
			} // if
		if (GlobalApp->GUIWins->DKG)
			{
			GlobalApp->GUIWins->DKG->Open(GlobalApp->MainProj);
			GlobalApp->GUIWins->DKG->SetModalInhibit(TRUE);
			} // if
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "TX_PREVKEY") && TexAffil && (*TexAffil))
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
		Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = GlobalApp->MainProj->Interactive->GetActiveTime();
		Prop.NewKeyNodeRange[0] = -DBL_MAX;
		Prop.NewKeyNodeRange[1] = DBL_MAX;
		Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
		(*TexAffil)->GetRAHostProperties(&Prop);
		if (Prop.NewKeyNodeRange[0] > Prop.KeyNodeRange[0] || Prop.NewKeyNodeRange[0] == -DBL_MAX)
			Prop.NewKeyNodeRange[0] = Prop.KeyNodeRange[0];
		if (Prop.NewKeyNodeRange[0] < Prop.KeyNodeRange[0])
			GlobalApp->MainProj->Interactive->SetActiveTime(Prop.NewKeyNodeRange[0]);
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "TX_NEXTKEY") && TexAffil && (*TexAffil))
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
		Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = GlobalApp->MainProj->Interactive->GetActiveTime();
		Prop.NewKeyNodeRange[0] = -DBL_MAX;
		Prop.NewKeyNodeRange[1] = DBL_MAX;
		Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
		(*TexAffil)->GetRAHostProperties(&Prop);
		if (Prop.NewKeyNodeRange[1] < Prop.KeyNodeRange[1] || Prop.NewKeyNodeRange[1] == DBL_MAX)
			Prop.NewKeyNodeRange[1] = Prop.KeyNodeRange[1];
		if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1])
			GlobalApp->MainProj->Interactive->SetActiveTime(Prop.NewKeyNodeRange[1]);
		Handled = 1;
		} // if
	} // else if texture
#ifdef WCS_THEMATIC_MAP
else if (! strncmp((char *)Action, "TM", 2))
	{
	if (! strcmp((char *)Action, "TM_ACTIVATE") && ThemeAffil && (*ThemeAffil))
		{
		RasterAnimHost::SetActiveRAHost((RasterAnimHost *)(*ThemeAffil), 1);
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "TM_CREATE") && ThemeAffil)
		{
		if ((ItemNumber = ThemeOwner->GetThemeNumberFromAddr(ThemeAffil)) >= 0)
			{
			unsigned long CurLen;
			strcpy(CopyMsg, GetRAHostName());
			strcat(CopyMsg, " ");
			CurLen = strlen(CopyMsg);
			strcat(CopyMsg, RAParent ? RAParent->GetCritterName(this): "");
			if (strlen(CopyMsg) > CurLen)
				strcat(CopyMsg, " ");
			strcat(CopyMsg, ThemeOwner->GetThemeName(ItemNumber));
			CopyMsg[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
			if (NewTheme = (ThematicMap *)GlobalApp->AppEffects->AddEffect(WCS_EFFECTSSUBCLASS_THEMATICMAP, CopyMsg[0] ? CopyMsg: NULL, NULL))
				{
				ThemeOwner->SetTheme(ItemNumber, NewTheme);
				NewTheme->EditRAHost();
				} // if
			} // if
		Handled = 1;
		} // if
	else if (! strncmp((char *)Action, "TM_LINKATTRIB_", 14) && ThemeAffil)
		{
		char AttribDetailScratch[64]; // used to dissect the attribute detail encoded in the Action string
		strcpy(AttribDetailScratch, (&((char *)Action)[14]));
		AttribDetailScratch[4] = NULL; // drop a NULL after the attrib number so atio() can parse it, below
		int WhichAttrib = -1;
		int WhichMult = -1;
		ThematicMap *PreviousMap = NULL;

		WhichAttrib = atoi(AttribDetailScratch);
		if(AttribDetailScratch[5] != 'n') // "nnnn" string is "no units conversion", which makes us leave WhichMult at no-data value of -1
			{
			WhichMult = atoi(&AttribDetailScratch[5]);
			} // if
		
		// remove existing thematic map, if we are the only ones using it
		if ((ItemNumber = ThemeOwner->GetThemeNumberFromAddr(ThemeAffil)) >= 0)
			{
			PreviousMap = ThemeOwner->GetTheme(ItemNumber);
			//if(PreviousMap = ThemeOwner->GetTheme(ItemNumber))
				{
				//ThemeOwner->SetTheme(ItemNumber, NULL);
				} // if
			} // if
		
		// create a new TM
		if ((ItemNumber = ThemeOwner->GetThemeNumberFromAddr(ThemeAffil)) >= 0)
			{
			//strcpy(CopyMsg, GetRAHostName());
			//strcat(CopyMsg, " ");
			//strcpy(CopyMsg, RAParent ? RAParent->GetCritterName(this): "");
			//strcat(CopyMsg, " ");
			//strcat(CopyMsg, ThemeOwner->GetThemeName(ItemNumber));
			//CopyMsg[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
			//if (NewTheme = (ThematicMap *)GlobalApp->AppEffects->AddEffect(WCS_EFFECTSSUBCLASS_THEMATICMAP, CopyMsg[0] ? CopyMsg: NULL, NULL))
			//	{
			//	ThemeOwner->SetTheme(ItemNumber, NewTheme);
				// set up the attribute of the new TM

			int ThemeCount = 0;
			LayerEntry *Entry;
			ThematicMap *FoundTM = NULL;
			Database *DBHost = GlobalApp->AppDB; // can't get main Database any other way, easily.
			const char *LayerName;
			for(Entry = DBHost->DBLayers.FirstEntry(); Entry; Entry = DBHost->DBLayers.NextEntry(Entry))
				{
				if (! Entry->TestFlags(WCS_LAYER_LINKATTRIBUTE))
					{
					LayerName = Entry->GetName();
					if (LayerName[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_DBL) // is it a numeric attribute
						{
						if(ThemeCount == WhichAttrib)
							{
							// do we create a new TM or reuse an existing one?
							// test the layer entry against the list of existing TM layers
							// test the multiplier
							for (ThematicMap *CurTM = GlobalApp->AppEffects->Theme; CurTM; CurTM = (ThematicMap *)CurTM->Next)
								{
								if (! strcmp(CurTM->AttribField[0], LayerName))
									{
									if (WhichMult == -1)
										{
										if (CurTM->AttribFactor[0] == 1.0) // no scaling
											{
											FoundTM = CurTM;
											break;
											} // if
										} // if
									else
										{
										if (CurTM->AttribFactor[0] == ThematicMapEditGUI::MultPresetValues[WhichMult])
											{
											FoundTM = CurTM;
											break;
											} // if
										} // else
									} // if
								} // for
							if (FoundTM)
								{
								if (FoundTM != PreviousMap)
									ThemeOwner->SetTheme(ItemNumber, FoundTM);
								} // if
							else
								{
								strcpy(CopyMsg, &LayerName[1]);
								if (NewTheme = (ThematicMap *)GlobalApp->AppEffects->AddEffect(WCS_EFFECTSSUBCLASS_THEMATICMAP, CopyMsg[0] ? CopyMsg: NULL, NULL))
									{
									ThemeOwner->SetTheme(ItemNumber, NewTheme);
									// set up the attribute of the new TM
									NewTheme->SetAttribute(Entry, 0);
									// set up units conversion of the new theme
									if(WhichMult == -1)
										{
										NewTheme->AttribFactor[0] = 1.0; // no scaling
										} // if
									else
										{
										NewTheme->AttribFactor[0] = ThematicMapEditGUI::MultPresetValues[WhichMult];
										} // else
									} // if
								} // else
							break;
							} // if
						ThemeCount++;
						} // if an attribute layer
					} // if
				} // for
//				} // if
			} // if
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "TM_LINK") && ThemeAffil)
		{
		if ((ItemNumber = ThemeOwner->GetThemeNumberFromAddr(ThemeAffil)) >= 0)
			GlobalApp->AppEffects->AddAttributeByList(this, WCS_EFFECTSSUBCLASS_THEMATICMAP, ThemeOwner, ItemNumber);
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "TM_DELETE") && ThemeAffil && (*ThemeAffil))
		{
		if ((ItemNumber = ThemeOwner->GetThemeNumberFromAddr(ThemeAffil)) >= 0)
			ThemeOwner->SetTheme(ItemNumber, NULL);
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "TM_EDIT") && ThemeAffil && (*ThemeAffil))
		{
		(*ThemeAffil)->EditRAHost();
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "TM_ENABLE") && ThemeAffil && (*ThemeAffil))
		{
		Prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
		Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;
		Prop.Flags = WCS_RAHOST_FLAGBIT_ENABLED;
		(*ThemeAffil)->SetRAHostProperties(&Prop);
		Handled = 1;
		} // if
	else if (! strcmp((char *)Action, "TM_DISABLE") && ThemeAffil && (*ThemeAffil))
		{
		Prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
		Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;
		Prop.Flags = 0;
		(*ThemeAffil)->SetRAHostProperties(&Prop);
		Handled = 1;
		} // if
	} // else if thematic map
#endif // WCS_THEMATIC_MAP

//if (! Handled)
//	UserMessageOK("HandleSRAHPopMenuSelection", (char *)Action);
return(Handled);

} // RasterAnimHost::HandleSRAHPopMenuSelection

/*===========================================================================*/

// return -1 if user aborts, 0 if failure to open or write file, 1 if total success
int RasterAnimHost::LoadFilePrep(RasterAnimHostProperties *Prop)
{
int Success = -1, IsProject;
char Title[12], ReadBuf[32], SuccessBuf[264];
ULONG ByteOrder;
NotifyTag Changes[2];

IsProject = (Prop->Path && strlen(Prop->Path) >= 5 && ! stricmp(&Prop->Path[strlen(Prop->Path) - 5], ".proj"));

Prop->PropMask = WCS_RAHOST_MASKBIT_LOADFILE;
Prop->fFile = NULL;
Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, NULL);

if (Prop->Path)
	{
	if (Prop->fFile = PROJ_fopen(Prop->Path, "rb"))
		{
		fread((char *)ReadBuf, 14, 1, Prop->fFile);

		memcpy(Title, &ReadBuf[0], 8);
		memcpy(&Prop->FileVersion, &ReadBuf[8], 1);
		memcpy(&Prop->FileRevision, &ReadBuf[9], 1);
		memcpy(&ByteOrder, &ReadBuf[10], 4);

		if (ByteOrder == 0xaabbccdd)
			Prop->ByteFlip = 0;
		else if (ByteOrder == 0xddccbbaa)
			Prop->ByteFlip = 1;
		else
			goto ReadError;

		Title[11] = '\0';
		if (! strnicmp(Title, "WCS File", 8))
			{
			EffectsLib::LoadQueries = Prop->Queries;	// suppresses copy messages

			if ((Success = SetRAHostProperties(Prop)) == 0)
				{
				if (Prop->Queries)
					{
					if (IsProject)
						UserMessageOK("Load Project", "Project load failed! Possible reasons include corrupt file or out of memory.");
					else
						UserMessageOK("Load Component", "Component load failed! Possible reasons include corrupt file or out of memory.");
					} // if
				GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Prop->Path);
				} // if
			else if (Success < -1)
				{
				if (Prop->Queries)
					{
					if (IsProject)
						UserMessageOK("Load Project", "Project load failed! Function not implemented in this version.");
					else
						UserMessageOK("Load Component", "Component load failed! Function not implemented in this version.");
					} // if
				GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Prop->Path);
				} // if
			else if (Success < 0)
				{
				if (Prop->Queries)
					{
					if (IsProject)
						UserMessageOK("Load Project", "Project load failed! File does not contain correct object type.");
					else
						UserMessageOK("Load Component", "Component load failed! File does not contain correct object type.");
					} // if
				GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Prop->Path);
				} // if
			else
				{
				HeresYourNewFilePathIfYouCare(Prop->Path);
				if (Prop->Queries)
					{
					if (IsProject)
						UserMessageOK("Load Project", "Project loaded successfully.");
					else
						{
						// UserMessage causes widget repainting messages to get through and be processed. This in turn can cuase massive brain hemhoraging 
						// because widgets have not yet been configured with respect to their new data members. 
						// Hence this notification must be given BEFORE the user message.
						Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
						Changes[1] = NULL;
						GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
						UserMessageOK("Load Component", "Component loaded successfully.");
						} // else
					} // if
				sprintf(SuccessBuf, "%s File Version %d.%d", Prop->Path, Prop->FileVersion, Prop->FileRevision);
				GlobalApp->StatusLog->PostStockError(WCS_LOG_MSG_COMPONENT_LOAD, SuccessBuf);
				} // else if OK
			} // if
		else
			{
			if (Prop->Queries)
				{
				if (IsProject)
					UserMessageOK("Load Project", "Project load failed! Not a supported "APP_TLA" file.");
				else
					UserMessageOK("Load Component", "Component load failed! Not a supported "APP_TLA" file.");
				} // if
			GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_OPEN_FAIL, Prop->Path);
			Success = 0;
			} // else
		fclose(Prop->fFile);
		EffectsLib::LoadQueries = 1;
		} // if file
	else
		{
		if (Prop->Queries)
			{
			if (IsProject)
				UserMessageOK("Load Project", "Project load failed! Possible reasons include invalid path or disk access denied.");
			else
				UserMessageOK("Load Component", "Component load failed! Possible reasons include invalid path or disk access denied.");
			} // if
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_OPEN_FAIL, Prop->Path);
		Success = 0;
		} // else
	} // if
else
	{
		UserMessageOK("Load Component", "Component load failed! No file path specified.");
	Success = 0;
	} // else

Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, NULL);
return (Success);

ReadError:

if (Prop->fFile)
	fclose(Prop->fFile);
if (Prop->Queries)
	{
	if (Prop->Path && IsProject)
		UserMessageOK("Load Project", "Project load failed! Unknown byte order found in file.");
	else
		UserMessageOK("Load Component", "Component load failed! Unknown byte order found in file.");
	} // if
GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, Prop->Path);
Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, NULL);
return (0);

} // RasterAnimHost::LoadFilePrep

/*===========================================================================*/

// return -1 if user aborts, 0 if failure to open or write file, 1 if total success
int RasterAnimHost::SaveFilePrep(RasterAnimHostProperties *Prop)
{
#ifndef WCS_BUILD_DEMO
char FilePath[256], FileName[64], FilePtrn[32], FileType[12], StrBuf[12], SaveLabel[24];
char Version = WCS_PROJECT_CURRENT_VERSION;
char Revision = WCS_PROJECT_CURRENT_REVISION;
ULONG ItemTag, TotalWritten = 0, ByteOrder = 0xaabbccdd;
long BytesWritten, Result, OrigLen, StrippedLen;
int Success = -1, SavingProject = 0;

Prop->PropMask = WCS_RAHOST_MASKBIT_SAVEFILE;
Prop->fFile = NULL;
#endif // !WCS_BUILD_DEMO
int IsProject;

IsProject = (Prop->Path && strlen(Prop->Path) >= 5 && ! stricmp(&Prop->Path[strlen(Prop->Path) - 5], ".proj"));

#ifdef WCS_BUILD_DEMO

if (Prop->Path && IsProject)
	{
	UserMessageDemo("Project files and preferences cannot be saved.");
	return (0);
	} // if
else
	{
	UserMessageDemo("Component files cannot be saved.");
	return (0);
	} // else

#else // WCS_BUILD_DEMO

RepeatOpen:

if (Prop->Path)
	{
	if (Prop->Queries && (Prop->fFile = PROJ_fopen(Prop->Path, "rb")))
		{
		fclose(Prop->fFile);
		Prop->fFile = NULL;
		if (IsProject)
			{
			if ((Result = UserMessageCustom("Save Project", "A Project file of that name already exists! Overwrite it or choose a new name?",
				"New Name", "Cancel", "Overwrite", 1)) == 0)
				{
				return (Success);
				} // if abort
			SavingProject = 1;
			} // if
		else if (ComponentOverwriteOK(Prop))
			{
			if ((Result = UserMessageCustom("Save Component", "A Component file of that name already exists! Overwrite it or choose a new name?",
				"New Name", "Cancel", "Overwrite", 1)) == 0)
				{
				return (Success);
				} // if abort
			} // else
		else
			{
			if ((Result = UserMessageCustom("Save Component", "A Component file of that name already exists! Choose a new name?",
				"New Name", "Cancel", NULL, 1)) == 0)
				{
				return (Success);
				} // if abort
			} // else
		if (Result == 1)
			{
			BreakFileName(Prop->Path, FilePath, 256, FileName, 64);
			OrigLen = (long)strlen(FileName);
			StripExtension(FileName);
			StrippedLen = (long)strlen(FileName);
			if (StrippedLen < OrigLen - 1)
				strcpy(FilePtrn, &FileName[StrippedLen + 1]);
			else
				strcpy(FilePtrn, WCS_REQUESTER_WILDCARD);
			if (IsProject)
				strcpy(SaveLabel, "Save Project");
			else
				strcpy(SaveLabel, "Save Component");
			if (GetFileNamePtrn(1, SaveLabel, FilePath, FileName, FilePtrn, 64))
				{
				StripExtension(FileName);
				if (strcmp(FilePtrn, WCS_REQUESTER_WILDCARD))
					{
					strcat(FileName, ".");
					strcat(FileName, FilePtrn);
					} // if
				strmfp(Prop->Path, FilePath, FileName);
				goto RepeatOpen;
				} // if
			else
				{
				return (Success);
				} // else
			} // else if choose new name
		} // if file exists

	if (! SavingProject)
		GlobalApp->ComponentSaveInProgress = 1;

	if (Prop->fFile = PROJ_fopen(Prop->Path, "wb"))
		{
		strcpy(FileType, "WCS File");

		// no tags or sizes for first four items: file descriptor, version, revision & byte order
		if ((BytesWritten = WriteBlock(Prop->fFile, (char *)FileType,
			WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		if ((BytesWritten = WriteBlock(Prop->fFile, (char *)&Version,
			WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		if ((BytesWritten = WriteBlock(Prop->fFile, (char *)&Revision,
			WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		if ((BytesWritten = WriteBlock(Prop->fFile, (char *)&ByteOrder,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		// write browse chunk
		if (BrowseInfo)
			{
			strcpy(StrBuf, "Browse");
			if (BytesWritten = WriteBlock(Prop->fFile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(Prop->fFile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = BrowseInfo->Save(Prop->fFile))
						{
						TotalWritten += BytesWritten;
						fseek(Prop->fFile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
						if (WriteBlock(Prop->fFile, (char *)&BytesWritten,
							WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
							{
							fseek(Prop->fFile, 0, SEEK_END);
							} // if wrote size of block 
						else
							goto WriteError;
						} // if Browse Data saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if

		if ((Success = SetRAHostProperties(Prop)) <= -1)
			{
			if (Prop->Queries)
				{
				if (IsProject)
					UserMessageOK("Save Project", "Project save failed! Function not implemented in this version.");
				else
					UserMessageOK("Save Component", "Component save failed! Function not implemented in this version.");
				} // if
			GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, Prop->Path);
			} // if
		else if (Success == 0)
			{
			if (Prop->Queries)
				{
				if (IsProject)
					UserMessageOK("Save Project", "Project save failed! Possible reasons include disk full.");
				else
					UserMessageOK("Save Component", "Component save failed! Possible reasons include disk full.");
				} // if
			GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, Prop->Path);
			} // if
		else
			{
			if (Prop->Queries)
				{
				if (IsProject)
					UserMessageOK("Save Project", "Project saved successfully.");
				else
					UserMessageOK("Save Component", "Component saved successfully.");
				} // if
			GlobalApp->StatusLog->PostStockError(WCS_LOG_MSG_COMPONENT_SAVE, Prop->Path);
			} // else if OK
		fclose(Prop->fFile);
		if (Success <= 0)
			PROJ_remove(Prop->Path);
		} // if file
	else
		{
		if (Prop->Queries)
			{
			if (IsProject)
				UserMessageOK("Save Project", "Project save failed! Possible reasons include invalid path or disk access denied.");
			else
				UserMessageOK("Save Component", "Component save failed! Possible reasons include invalid path or disk access denied.");
			} // if
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_OPEN_FAIL, Prop->Path);
		Success = 0;
		} // else
	} // if
else
	{
	if (Prop->Queries)
		{
		UserMessageOK("Save Component", "Component save failed! No file path specified.");
		} // if
	Success = 0;
	} // else

GlobalApp->ComponentSaveInProgress = 0;
return (Success);

WriteError:

if (Prop->fFile)
	fclose(Prop->fFile);
PROJ_remove(Prop->Path);
Success = 0;
if (Prop->Queries)
	{
	if (Prop->Path && IsProject)
		UserMessageOK("Save Project", "Project save failed! Possible reasons include disk full.");
	else
		UserMessageOK("Save Component", "Component save failed! Possible reasons include disk full.");
	} // if
GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, Prop->Path);
GlobalApp->ComponentSaveInProgress = 0;

return (Success);

#endif // WCS_BUILD_DEMO

} // RasterAnimHost::SaveFilePrep

/*===========================================================================*/

void RasterAnimHost::OpenGallery(EffectsLib *EffectsHost)
{

if(GlobalApp->GUIWins->LVE)
	{
	delete GlobalApp->GUIWins->LVE;
	}
GlobalApp->GUIWins->LVE = new GalleryGUI(EffectsHost, this);
if(GlobalApp->GUIWins->LVE)
	{
	GlobalApp->GUIWins->LVE->Open(GlobalApp->MainProj);
	}

} // RasterAnimHost::OpenGallery

/*===========================================================================*/

void RasterAnimHost::OpenBrowseData(EffectsLib *EffectsHost)
{

if(GlobalApp->GUIWins->BRD)
	{
	delete GlobalApp->GUIWins->BRD;
	}
GlobalApp->GUIWins->BRD = new BrowseDataGUI(EffectsHost, this);
if(GlobalApp->GUIWins->BRD)
	{
	GlobalApp->GUIWins->BRD->Open(GlobalApp->MainProj);
	}

} // RasterAnimHost::OpenBrowseData

/*===========================================================================*/

int RasterAnimHost::LoadComponentFile(char *NameSupplied, char Queries)
{
RasterAnimHostProperties Prop;
char FullPath[512], DefaultPath[512], FileName[128], DefaultExt[32];
NotifyTag Changes[2];

if (NameSupplied)
	{
	strcpy(FullPath, NameSupplied);
	} // if
else
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER | WCS_RAHOST_MASKBIT_FILEINFO;
	GetRAHostProperties(&Prop);

	if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_PROJECT)
		strcpy(DefaultPath, Prop.Path ? Prop.Path: "WCSProjects:");
	else
		{
		strcpy(DefaultPath, "WCSContent:");
		if (Prop.Path)
			strcat(DefaultPath, Prop.Path);
		} // else
	FileName[0] = 0;
	if (Prop.Ext)
		{
		strcpy(DefaultExt, "*.");
		strcat(DefaultExt, Prop.Ext);
		} // if
	else
		strcpy(DefaultExt, "*.*");
	} // else

if (NameSupplied || GetFileNamePtrn(0, "Select Component to Load", DefaultPath, FileName, DefaultExt, 128))
	{
	if (! NameSupplied)
		strmfp(FullPath, DefaultPath, FileName); //lint !e645
	Prop.Path = FullPath;
	Prop.PropMask = WCS_RAHOST_MASKBIT_LOADFILE;
	Prop.Queries = Queries;
	if (LoadFilePrep(&Prop) > 0)
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		return (1);
		} // if
	} // if

return (0);

} // RasterAnimHost::LoadComponentFile

/*===========================================================================*/

// called from ViewGUI when switching Manipulate Camera modes
void RasterAnimHost::SetActiveRAHostNoBackup(RasterAnimHost *NewActive, int CheckSAG)
{
NotifyTag Changes[2];

if ((! NewActive || NewActive != ActiveRAHost) && (! ActiveLock || ! ActiveRAHost))
	{
	ActiveRAHost = NewActive;
	GlobalApp->MCP->SetCurrentObject(ActiveRAHost, CheckSAG);
	if (NewActive && (NewActive->GetNotifyClass() == WCS_RAHOST_OBJTYPE_VECTOR ||
		NewActive->GetNotifyClass() == WCS_RAHOST_OBJTYPE_DEM ||
		NewActive->GetNotifyClass() == WCS_RAHOST_OBJTYPE_CONTROLPT))
		{
		GlobalApp->AppDB->ActiveObj = (Joe *)NewActive;
		Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEACTIVE, 0xff, 0xff);
		Changes[1] = NULL;
		GlobalApp->AppDB->GenerateNotify(Changes, NewActive);
		} // if
	} // if
if (GlobalApp->GUIWins->SAG)
	GlobalApp->GUIWins->SAG->ActivateActiveItem();

} // RasterAnimHost::SetActiveRAHostNoBackup

/*===========================================================================*/

void RasterAnimHost::SetActiveRAHost(RasterAnimHost *NewActive, int CheckSAG)
{
NotifyTag Changes[2];
RasterAnimHostProperties Prop;

if ((! NewActive || NewActive != ActiveRAHost) && (! ActiveLock || ! ActiveRAHost))
	{
	ActiveRAHost = NewActive;
	if (BackupRAHost)
		BackupRAHost = ActiveRAHost;
	GlobalApp->MCP->SetCurrentObject(ActiveRAHost, CheckSAG);
	if (NewActive)
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
		NewActive->GetRAHostProperties(&Prop);
		// database
		if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR ||
			Prop.TypeNumber == WCS_RAHOST_OBJTYPE_DEM ||
			Prop.TypeNumber == WCS_RAHOST_OBJTYPE_CONTROLPT)
			{
			GlobalApp->AppDB->ActiveObj = (Joe *)NewActive;
			Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEACTIVE, 0xff, 0xff);
			Changes[1] = NULL;
			GlobalApp->AppDB->GenerateNotify(Changes, NewActive);
			} // if
		// image object
		if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_RASTER)
			{
			// generates its own notification
			GlobalApp->AppImages->SetActive((Raster *)NewActive);
			} // if
		} // if
	} // if
if (GlobalApp->GUIWins->SAG)
	GlobalApp->GUIWins->SAG->ActivateActiveItem();

} // RasterAnimHost::SetActiveRAHost

/*===========================================================================*/

void RasterAnimHost::SetBackupRAHost(RasterAnimHost *NewActive, int CheckSAG)
{
NotifyTag Changes[2];

if ((! NewActive || NewActive != ActiveRAHost) && (! ActiveLock || ! ActiveRAHost))
	{
	BackupRAHost = ActiveRAHost;
	ActiveRAHost = NewActive;
	GlobalApp->MCP->SetCurrentObject(ActiveRAHost, CheckSAG);
	if (NewActive && (NewActive->GetNotifyClass() == WCS_RAHOST_OBJTYPE_VECTOR ||
		NewActive->GetNotifyClass() == WCS_RAHOST_OBJTYPE_DEM ||
		NewActive->GetNotifyClass() == WCS_RAHOST_OBJTYPE_CONTROLPT))
		{
		Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEACTIVE, 0xff, 0xff);
		Changes[1] = NULL;
		GlobalApp->AppDB->GenerateNotify(Changes, NewActive);
		} // if
	} // if
if (GlobalApp->GUIWins->SAG)
	GlobalApp->GUIWins->SAG->ActivateActiveItem();

} // RasterAnimHost::SetBackupRAHost

/*===========================================================================*/

void RasterAnimHost::RestoreBackupRAHost(void)
{
NotifyTag Changes[2];

if (BackupRAHost && (BackupRAHost != ActiveRAHost) && (! ActiveLock || ! ActiveRAHost))
	{
	ActiveRAHost = BackupRAHost;
	BackupRAHost = NULL;
	GlobalApp->MCP->SetCurrentObject(ActiveRAHost, 0);
	if (ActiveRAHost->GetNotifyClass() == WCS_RAHOST_OBJTYPE_VECTOR ||
		ActiveRAHost->GetNotifyClass() == WCS_RAHOST_OBJTYPE_DEM ||
		ActiveRAHost->GetNotifyClass() == WCS_RAHOST_OBJTYPE_CONTROLPT)
		{
		Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEACTIVE, 0xff, 0xff);
		Changes[1] = NULL;
		GlobalApp->AppDB->GenerateNotify(Changes, ActiveRAHost);
		} // if
	} // if
if (GlobalApp->GUIWins->SAG)
	GlobalApp->GUIWins->SAG->ActivateActiveItem();

} // RasterAnimHost::RestoreBackupRAHost

/*===========================================================================*/

int RasterAnimHost::GetRADropVectorOK(long TypeNumber)
{

switch(TypeNumber)
	{
	case WCS_JOE_ATTRIB_INTERNAL_LAKE:
	case WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM:
	case WCS_JOE_ATTRIB_INTERNAL_WAVE:
	case WCS_JOE_ATTRIB_INTERNAL_CLOUD:
	case WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT:
	case WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR:
	case WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR:
	case WCS_JOE_ATTRIB_INTERNAL_FOLIAGE:
	case WCS_JOE_ATTRIB_INTERNAL_OBJECT3D:
	case WCS_JOE_ATTRIB_INTERNAL_SHADOW:
	case WCS_JOE_ATTRIB_INTERNAL_STREAM:
	case WCS_JOE_ATTRIB_INTERNAL_GROUND:
	case WCS_JOE_ATTRIB_INTERNAL_SNOW:
	case WCS_JOE_ATTRIB_INTERNAL_FENCE:
	case WCS_JOE_ATTRIB_INTERNAL_LABEL:
// <<<>>> ADD_NEW_EFFECTS add new case right above this line if items in the class can have a vector attached 
		return(1);
	default:
		return(0);
	} //

} // RasterAnimHost::GetRADropVectorOK

/*===========================================================================*/

int RasterAnimHost::ScaleDeleteAnimNodes(RasterAnimHostProperties *Prop)
{
int Success = 0;
RasterAnimHost *CurChild = NULL;

while (CurChild = GetRAHostChild(CurChild, 0))
	{
	if (CurChild->RAParent == this)
		{
		if (CurChild->ScaleDeleteAnimNodes(Prop))
			Success = 1;
		} // if
	} // while

return (Success);

} // RasterAnimHost::ScaleDeleteAnimNodes

/*===========================================================================*/

int RasterAnimHost::GetNextAnimNode(RasterAnimHostProperties *Prop)
{
int Success = 0;
RasterAnimHost *CurChild = NULL;

while (CurChild = GetRAHostChild(CurChild, 0))
	{
	if (CurChild->RAParent == this)
		{
		if (CurChild->GetNextAnimNode(Prop))
			Success = 1;
		} // if
	} // while

return (Success);

} // RasterAnimHost::GetNextAnimNode

/*===========================================================================*/

int RasterAnimHost::AddBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags)
{
RasterAnimHostProperties Prop;
int Deletable, CanAdd, VecOrCtrlPt, Draggable, EdNum, Loadable, Embedable, Editable, Enableable, IsLayer;
int CanCreate;

Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_TYPENUMBER | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_FILEINFO;
Prop.FlagsMask = WCS_RAHOST_FLAGBIT_DELETABLE | WCS_RAHOST_FLAGBIT_DRAGGABLE;
// Call this on ourself
GetRAHostProperties(&Prop);

Editable = 1;
Deletable = (Prop.Flags & WCS_RAHOST_FLAGBIT_DELETABLE);
Enableable = Deletable;
CanAdd = (MenuClassFlags == WCS_RAH_POPMENU_CLASS_ECO) || ! (Prop.TypeNumber >= WCS_MAXIMPLEMENTED_EFFECTS);
VecOrCtrlPt = (MenuClassFlags != WCS_RAH_POPMENU_CLASS_ECO) && (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR || Prop.TypeNumber == WCS_RAHOST_OBJTYPE_CONTROLPT);
Draggable = (Prop.Flags & WCS_RAHOST_FLAGBIT_DRAGGABLE);
EdNum = (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME);
Loadable = (Prop.Path ? 1 : 0);
Embedable = (TemplateItem && Prop.TypeNumber >= WCS_EFFECTSSUBCLASS_LAKE && Prop.TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS);
CanCreate = (MenuClassFlags != WCS_RAH_POPMENU_CLASS_ECO) && RasterAnimHost::IsDigitizeLegal(this, 0);
if(IsLayer = (Prop.TypeNumber == WCS_SUBCLASS_LAYER))
	{
	Deletable = 0; // Layer will offer its own phrasing of this with the same command text
	Editable = 0;
	Draggable = 0;
	Loadable = 0;
	} // if

// WCS_RAH_POPMENU_CHECK_APPLICABLE(a, b)
// We don't check applicability in Base menus, only in derived ones.
// Base menus are only added to S@G currently.

if(Enableable && !IsLayer)
	{
	PMA->AddPopMenuItem("Enable Component", "ENABLE", 0);
	PMA->AddPopMenuItem("Disable Component", "DISABLE", 0);
	} // if
if(IsLayer)
	{ // same action as above, but different text
	PMA->AddPopMenuItem("Enable Layer Members", "ENABLE", 0);
	PMA->AddPopMenuItem("Disable Layer Members", "DISABLE", 0);
	} // if

if(Editable)	PMA->AddPopMenuItem("Edit Component", "EDIT", 0);
if(CanCreate)	PMA->AddPopMenuItem("Create Vector or Path", "CREATE", 0);
if(CanAdd && MenuClassFlags == WCS_RAH_POPMENU_CLASS_ECO)		
	{
	PMA->AddPopMenuItem("Add Foliage Group", "ADDFOLIAGEGROUP", 0);
	PMA->AddPopMenuItem("Add Foliage Group from Gallery", "ADDFOLIAGEGROUPGALLERY", 0);
	PMA->AddPopMenuItem("Add Foliage", "ADDFOLIAGE", 0);
	} // if
else if(CanAdd)		PMA->AddPopMenuItem("Clone Component", "CLONE", 0);
if(VecOrCtrlPt)	PMA->AddPopMenuItem("Clone Vector", "CLONEVEC", 0);
if(Draggable)	PMA->AddPopMenuItem("Copy Component\tCTRL+C", "COPY", 0);
if(Draggable)	PMA->AddPopMenuItem("Paste Component\tCTRL+V", "PASTE", 0);
if(Loadable)	PMA->AddPopMenuItem("Load Component from Gallery", "GALLERY", 0);
if(Loadable)	PMA->AddPopMenuItem("Load Component from File", "LOAD", 0);
if(Loadable)	PMA->AddPopMenuItem("Sign and Save Component", "SIGNANDSAVE", 0);
if(Deletable)	PMA->AddPopMenuItem("Delete Component", "DELETE", 0);
if(Embedable)	PMA->AddPopMenuItem("Embed Component", "EMBED", 0);
if(EdNum)		PMA->AddPopMenuItem("Edit Numeric\tALT+.", "EDNUM", 0);
if(VecOrCtrlPt)	PMA->AddPopMenuItem("Edit Vector Profile", "EDVECPROF", 0);
//PMA->AddPopMenuItem("", "", 0);

return(1);
} // RasterAnimHost::AddBasePopMenus


/*===========================================================================*/

int RasterAnimHost::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
return(0);
} // RasterAnimHost::AddSRAHBasePopMenus

/*===========================================================================*/

void RasterAnimHost::Embed(void)
{
NotifyTag Changes[2];

if (TemplateItem)
	{
	TemplateItem = 0;
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, this);
	} // if

} // RasterAnimHost::Embed

/*===========================================================================*/

int RasterAnimHost::IsDigitizeLegal(RasterAnimHost *TestMe, long CategoryOnly)
{
RasterAnimHost *RAH = NULL, *RAHRoot = NULL;
long Cat = 0, DigitizeLegal = 0;
RasterAnimHostProperties Prop;
// See DigitizeGUI::ConfigureDigitize for up-to-date rules

if (CategoryOnly)
	Cat = CategoryOnly;
else if (TestMe)
	RAH = TestMe;
else if (! (RAH = RasterAnimHost::GetActiveRAHost()))
	{
	RAH = GlobalApp->GUIWins->SAG->GetSelectedObjectAndCategory(Cat);
	} // if
if (RAH) // is there an active object?
	{
	RAHRoot = RAH->GetRAHostRoot();
	// check type
	Prop.PropMask  = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_TYPENUMBER;
	Prop.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGTARGET;
	RAHRoot->GetRAHostProperties(&Prop);
	if (Prop.TypeNumber != WCS_SUBCLASS_LAYER)
		{
		Cat = Prop.TypeNumber;
		// Can we (create and) attach a vector to this type of RasterAnimHost
		// Can I drop a vector on you?
		Prop.TypeNumber = WCS_RAHOST_OBJTYPE_VECTOR;
		Prop.PropMask   = WCS_RAHOST_MASKBIT_DROPOK;
		RAHRoot->GetRAHostProperties(&Prop);
		if (Prop.DropOK || Cat == WCS_JOE_ATTRIB_INTERNAL_CAMERA || Cat == WCS_JOE_ATTRIB_INTERNAL_LIGHT || Cat == WCS_JOE_ATTRIB_INTERNAL_CELESTIAL)
			{
			DigitizeLegal = 1;
			} // else
		} // if
	} // if
else if (Cat == WCS_RAHOST_OBJTYPE_VECTOR || Cat == WCS_RAHOST_OBJTYPE_CONTROLPT || 
	Cat == WCS_JOE_ATTRIB_INTERNAL_CAMERA || Cat == WCS_JOE_ATTRIB_INTERNAL_LIGHT || 
	Cat == WCS_JOE_ATTRIB_INTERNAL_CELESTIAL ||
	(Cat != WCS_SUBCLASS_LAYER && RasterAnimHost::GetRADropVectorOK(Cat)))
	{
	DigitizeLegal = 1;
	} // else

return (DigitizeLegal);

} // RasterAnimHost::IsDigitizeLegal

/*===========================================================================*/
/*===========================================================================*/

BrowseData::BrowseData()
{

Thumb = NULL;
Author = Author2 = Comment = Date = Address = Category = NULL;

} // BrowseData::BrowseData

/*===========================================================================*/

BrowseData::~BrowseData()
{

FreeAll();

} // BrowseData::~BrowseData

/*===========================================================================*/

void BrowseData::Copy(BrowseData *CopyTo, BrowseData *CopyFrom)
{

CopyTo->FreeAll();
if (CopyFrom->Author)
	{
	if (CopyTo->NewAuthor((long)(strlen(CopyFrom->Author) + 1)))
		strcpy(CopyTo->Author, CopyFrom->Author);
	} // if
if (CopyFrom->Author2)
	{
	if (CopyTo->NewAuthor2((long)(strlen(CopyFrom->Author2) + 1)))
		strcpy(CopyTo->Author2, CopyFrom->Author2);
	} // if
if (CopyFrom->Address)
	{
	if (CopyTo->NewAddress((long)(strlen(CopyFrom->Address) + 1)))
		strcpy(CopyTo->Address, CopyFrom->Address);
	} // if
if (CopyFrom->Comment)
	{
	if (CopyTo->NewComment((long)(strlen(CopyFrom->Comment) + 1)))
		strcpy(CopyTo->Comment, CopyFrom->Comment);
	} // if
if (CopyFrom->Date)
	{
	if (CopyTo->NewDate((long)(strlen(CopyFrom->Date) + 1)))
		strcpy(CopyTo->Date, CopyFrom->Date);
	} // if
if (CopyFrom->Category)
	{
	if (CopyTo->NewCategory((long)(strlen(CopyFrom->Category) + 1)))
		strcpy(CopyTo->Category, CopyFrom->Category);
	} // if
if (CopyFrom->Thumb)
	{
	if (CopyTo->Thumb = new Thumbnail())
		CopyTo->Thumb->Copy(CopyTo->Thumb, CopyFrom->Thumb);
	} // if

} // BrowseData::Copy

/*===========================================================================*/

void BrowseData::FreeAll(void)
{

FreeThumb();
FreeAuthor();
FreeAuthor2();
FreeComment();
FreeAddress();
FreeDate();
FreeCategory();

} // BrowseData::FreeAll

/*===========================================================================*/

void BrowseData::FreeAuthor(void)
{

if (Author)
	AppMem_Free(Author, strlen(Author) + 1);
Author = NULL;

} // BrowseData::FreeAuthor

/*===========================================================================*/

void BrowseData::FreeAuthor2(void)
{

if (Author2)
	AppMem_Free(Author2, strlen(Author2) + 1);
Author2 = NULL;

} // BrowseData::FreeAuthor2

/*===========================================================================*/

void BrowseData::FreeAddress(void)
{

if (Address)
	AppMem_Free(Address, strlen(Address) + 1);
Address = NULL;

} // BrowseData::FreeAddress

/*===========================================================================*/

void BrowseData::FreeComment(void)
{

if (Comment)
	AppMem_Free(Comment, strlen(Comment) + 1);
Comment = NULL;

} // BrowseData::FreeComment

/*===========================================================================*/

void BrowseData::FreeDate(void)
{

if (Date)
	AppMem_Free(Date, strlen(Date) + 1);
Date = NULL;

} // BrowseData::FreeDate

/*===========================================================================*/

void BrowseData::FreeCategory(void)
{

if (Category)
	AppMem_Free(Category, strlen(Category) + 1);
Category = NULL;

} // BrowseData::FreeCategory

/*===========================================================================*/

void BrowseData::FreeThumb(void)
{

if (Thumb)
	delete Thumb;
Thumb = NULL;

} // BrowseData::FreeThumb

/*===========================================================================*/

char *BrowseData::NewAuthor(long NewLength)
{

FreeAuthor();
return (Author = (char *)AppMem_Alloc(NewLength, APPMEM_CLEAR));

} // BrowseData::NewAuthor

/*===========================================================================*/

char *BrowseData::NewAuthor2(long NewLength)
{

FreeAuthor2();
return (Author2 = (char *)AppMem_Alloc(NewLength, APPMEM_CLEAR));

} // BrowseData::NewAuthor2

/*===========================================================================*/

char *BrowseData::NewAddress(long NewLength)
{

FreeAddress();
return (Address = (char *)AppMem_Alloc(NewLength, APPMEM_CLEAR));

} // BrowseData::NewAddress

/*===========================================================================*/

char *BrowseData::NewComment(long NewLength)
{

FreeComment();
return (Comment = (char *)AppMem_Alloc(NewLength, APPMEM_CLEAR));

} // BrowseData::NewComment

/*===========================================================================*/

char *BrowseData::NewDate(long NewLength)
{

FreeDate();
return (Date = (char *)AppMem_Alloc(NewLength, APPMEM_CLEAR));

} // BrowseData::NewDate

/*===========================================================================*/

char *BrowseData::NewCategory(long NewLength)
{

FreeCategory();
return (Category = (char *)AppMem_Alloc(NewLength, APPMEM_CLEAR));

} // BrowseData::NewCategory

/*===========================================================================*/

Thumbnail *BrowseData::NewThumb(void)
{

FreeThumb();
return (Thumb = new Thumbnail());

} // BrowseData::NewThumb

/*===========================================================================*/

unsigned long BrowseData::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
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
					case WCS_BROWSEDATA_AUTHOR:
						{
						if (NewAuthor(Size))
							BytesRead = ReadLongBlock(ffile, (char *)Author, Size);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_BROWSEDATA_AUTHOR2:
						{
						if (NewAuthor2(Size))
							BytesRead = ReadLongBlock(ffile, (char *)Author2, Size);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_BROWSEDATA_ADDRESS:
						{
						if (NewAddress(Size))
							BytesRead = ReadLongBlock(ffile, (char *)Address, Size);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_BROWSEDATA_DATE:
						{
						if (NewDate(Size))
							BytesRead = ReadLongBlock(ffile, (char *)Date, Size);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_BROWSEDATA_COMMENT:
						{
						if (NewComment(Size))
							BytesRead = ReadLongBlock(ffile, (char *)Comment, Size);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_BROWSEDATA_CATEGORY:
						{
						if (NewCategory(Size))
							BytesRead = ReadLongBlock(ffile, (char *)Category, Size);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_BROWSEDATA_TNAIL:
						{
						if (NewThumb())
							BytesRead = Thumb->Load(ffile, Size, ByteFlip);
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

} // BrowseData::Load

/*===========================================================================*/

unsigned long BrowseData::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if (Author)
	{
	if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_BROWSEDATA_AUTHOR, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(Author) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Author)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (Author2)
	{
	if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_BROWSEDATA_AUTHOR2, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(Author2) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Author2)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (Address)
	{
	if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_BROWSEDATA_ADDRESS, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(Address) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Address)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (Date)
	{
	if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_BROWSEDATA_DATE, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(Date) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Date)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (Comment)
	{
	if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_BROWSEDATA_COMMENT, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(Comment) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Comment)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (Category)
	{
	if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_BROWSEDATA_CATEGORY, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(Category) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Category)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

if (Thumb && Thumb->TNailsValid())
	{
	ItemTag = WCS_BROWSEDATA_TNAIL + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = Thumb->Save(ffile))
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
				} // if thumbnail saved 
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

} // BrowseData::Save

/*===========================================================================*/

int BrowseData::LoadBrowseInfo(char *FileName)
{
char Title[12], ReadBuf[32];
char Version, Revision;
short ByteFlip;
FILE *ffile;
ULONG Size, BytesRead, TotalRead = 0, ByteOrder;
int Success = 0;

if (FileName)
	{
	if (ffile = PROJ_fopen(FileName, "rb"))
		{
		fread((char *)ReadBuf, 14, 1, ffile);

		memcpy(Title, &ReadBuf[0], 8);
		memcpy(&Version, &ReadBuf[8], 1);
		memcpy(&Revision, &ReadBuf[9], 1);
		memcpy(&ByteOrder, &ReadBuf[10], 4);

		if (ByteOrder == 0xaabbccdd)
			ByteFlip = 0;
		else if (ByteOrder == 0xddccbbaa)
			ByteFlip = 1;
		else
			goto ReadError;

		Title[11] = '\0';
		if (! strnicmp(Title, "WCS File", 8))
			{
			TotalRead = BytesRead = 14;

			while (BytesRead)
				{
				/* read block descriptor tag from file */
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
						if (! strnicmp(ReadBuf, "Browse", 8))
							{
							if ((BytesRead = Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;
							} // if material
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						TotalRead += BytesRead;
						if (BytesRead != Size)
							{
							break;
							} // if error
						} // if size block read 
					else
						break;
					} // if tag block read 
				else
					break;
				} // while 

			} // if WCS File
		fclose(ffile);
		} // if file
	else
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_OPEN_FAIL, FileName);
		return (0);
		} // else
	} // if name

return (Success);

ReadError:

fclose(ffile);
return (0);

} // BrowseData::LoadBrowseInfo

/*===========================================================================*/

DEMBounds::DEMBounds()
{

North = South = West = East = HighElev = LowElev = 0.0;

} // DEMBounds::DEMBounds

/*===========================================================================*/

unsigned long DEMBounds::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
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
					case WCS_DEMBOUNDS_NORTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&North, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_DEMBOUNDS_SOUTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&South, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_DEMBOUNDS_WEST:
						{
						BytesRead = ReadBlock(ffile, (char *)&West, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_DEMBOUNDS_EAST:
						{
						BytesRead = ReadBlock(ffile, (char *)&East, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_DEMBOUNDS_HIGHELEV:
						{
						BytesRead = ReadBlock(ffile, (char *)&HighElev, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_DEMBOUNDS_LOWELEV:
						{
						BytesRead = ReadBlock(ffile, (char *)&LowElev, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
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

} // DEMBounds::Load

/*===========================================================================*/

unsigned long DEMBounds::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEMBOUNDS_NORTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&North)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEMBOUNDS_SOUTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&South)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEMBOUNDS_WEST, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&West)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEMBOUNDS_EAST, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&East)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEMBOUNDS_HIGHELEV, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&HighElev)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_DEMBOUNDS_LOWELEV, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&LowElev)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

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

} // DEMBounds::Save

/*===========================================================================*/
/*===========================================================================*/
