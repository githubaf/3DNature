// Cloud.cpp
// For managing WCS clouds
// Built from scratch on 04/15/99 by Gary R. Huber
// Copyright 1999 Questar Productions

#include "stdafx.h"
#include "Texture.h"
#include "Raster.h"
#include "Useful.h"
#include "Application.h"
#include "EffectsLib.h"
#include "EffectsIO.h"
#include "Joe.h"
#include "requester.h"
#include "Conservatory.h"
#include "CloudEditGUI.h"
#include "Render.h"
#include "Database.h"
#include "Project.h"
#include "Interactive.h"
#include "ViewGUI.h"
#include "PixelManager.h"
#include "Security.h"
#include "Lists.h"
#include "FeatureConfig.h"

CloudEffect::CloudEffect()
: GeneralEffect(NULL), ColorGrad(this, 1)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_CLOUD;
SetDefaults();

} // CloudEffect::CloudEffect

/*===========================================================================*/

CloudEffect::CloudEffect(RasterAnimHost *RAHost)
: GeneralEffect(RAHost), ColorGrad(this, 1)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_CLOUD;
SetDefaults();

} // CloudEffect::CloudEffect

/*===========================================================================*/

CloudEffect::CloudEffect(RasterAnimHost *RAHost, EffectsLib *Library, CloudEffect *Proto)
: GeneralEffect(RAHost), ColorGrad(this, 1)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_CLOUD;
if (Library)
	{
	Prev = Library->LastCloud;
	if (Library->LastCloud)
		{
		Library->LastCloud->Next = this;
		Library->LastCloud = this;
		} // if
	else
		{
		Library->Cloud = Library->LastCloud = this;
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
	strcpy(NameBase, "Cloud Model");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // CloudEffect::CloudEffect

/*===========================================================================*/

CloudEffect::~CloudEffect()
{
WaveSource *CurSource;
RootTexture *DelTex;
long Ct;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->CLG && GlobalApp->GUIWins->CLG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->CLG;
		GlobalApp->GUIWins->CLG = NULL;
		} // if
	} // if

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (TexRoot[Ct])
		{
		DelTex = TexRoot[Ct];
		TexRoot[Ct] = NULL;
		delete DelTex;
		} // if
	} // for

// Vertices were used for rendering
if (Vertices)
	delete [] Vertices;

while (WaveSources)
	{
	CurSource = WaveSources;
	WaveSources = WaveSources->Next;
	delete CurSource;
	} // while

} // CloudEffect::~CloudEffect

/*===========================================================================*/

void CloudEffect::SetDefaults(void)
{
long Ct;
double EffectDefault[WCS_EFFECTS_CLOUD_NUMANIMPAR] = {0.0, 0.0, 10000.0, 10000.0, 5000.0, 
	1.0, 1.0, 500.0, 2.0, .5, .75, 1000.0, 75.0, 100.0, .5, 5.0, 0.0};
double RangeDefaults[WCS_EFFECTS_CLOUD_NUMANIMPAR][3] = {90.0, -90.0, .01,			// center lat in degrees
														FLT_MAX, -FLT_MAX, .01,		// center lon in degrees
														FLT_MAX, 1.0, 100.0,		// map width in meters
														FLT_MAX, 1.0, 100.0,		// map height in meters
														FLT_MAX, -10000.0, 100.0,	// base elevation in meters
														1.0, 0.0, .01,				// coverage in %
														5.0, 0.0, .01,				// density in %
														100000.0, 0.0, 10.0,		// thickness in meters
														10.0, 1.0, 1.0,				// falloff
														1.0, 0.0, .01,				// self-shading %
														1.0, 0.0, .01,				// received shadow intensity
														FLT_MAX, 1.0, 100.0,		// optical depth
														FLT_MAX, .01, 10.0,			// min volumetric sample rate
														FLT_MAX, .01, 10.0,			// max volumetric sample rate
														FLT_MAX, 0.0, .01,			// backlight %
														200.0, 0.0, 1.0,			// backlight exponent
														1.0, 0.0, .01};			// alt. backlight color %
GraphNode *CurNode;
GradientCritter *CurGrad;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
BacklightColor.SetDefaults(this, (char)Ct);
BacklightColor.SetValue3(1.0, 1.0, 1.0);
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	TexRoot[Ct] = NULL;
	} // for
Rows = Cols = 256;
NumLayers = 20;
Feather = 1;
PixelTexturesExist = 0;
PreviewSize = 8;
WavePreviewSize = 4;
CloudType = WCS_EFFECTS_CLOUDTYPE_NIMBUS;
MetersPerDegLon = RefLat = RefLon = 0.0;
CastShadows = 0;
CastShadowStyle = WCS_EFFECTS_CLOUD_SHADOWSTYLE_COMBINATION;
ReceiveShadowsTerrain = ReceiveShadowsFoliage = ReceiveShadows3DObject = ReceiveShadowsCloudSM = 
	ReceiveShadowsVolumetric = 0;
ReceiveShadowsMisc = 0;
ShadowMapWidth = 512;
UseMapFile = RegenMapFile = 0;
Vertices = NULL;
WaveSources = NULL;
NumSources = 0;
ShadowsOnly = 0;
ShadowFlags = 0;
Volumetric = 0;
VolumeBeforeRefl = 1;
CompleteBacklightColor[0] = CompleteBacklightColor[1] = CompleteBacklightColor[2] = 0.0;
LowElev = HighElev = ElevRange = 0.0;
CovgTextureExists = 0;
RadiusInside = RadiusOutside = 0.0;
VolumeSub = RenderShadowMap = RenderVolumetric = RenderPlanar = CastVolumeShadows = 0;

CovgProf.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ShadeProf.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
DensityProf.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ADProf.SetDefaults(this, (char)GetNumAnimParams());
CovgProf.SetDefaults(this, (char)GetNumAnimParams() + 1);
DensityProf.SetDefaults(this, (char)GetNumAnimParams() + 2);
ShadeProf.SetDefaults(this, (char)GetNumAnimParams() + 3);
SetCloudType();
CovgProf.ReleaseNodes();
if (CurNode = CovgProf.AddNode(0.0, 0.0, 0.0))
	CurNode->SetTension(-1.0);
if (CurNode = CovgProf.AddNode(1.0, 0.0, 0.0))
	CurNode->SetTension(-1.0);
CovgProf.AddNode(.4, 1.0, 0.0);
ShadeProf.ReleaseNodes();
ShadeProf.AddNode(0.0, 1.0, 0.0);
ShadeProf.AddNode(1.0, 0.0, 0.0);
DensityProf.ReleaseNodes();
DensityProf.AddNode(0.0, 1.0, 0.0);
DensityProf.AddNode(1.0, 1.0, 0.0);
CovgProf.ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
ShadeProf.ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
DensityProf.ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
if (CurGrad = ColorGrad.GetActiveNode())
	{
	((ColorTextureThing *)CurGrad->GetThing())->Color.SetValue3(1.0, 1.0, 1.0);
	} // if

AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPWIDTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPHEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BASEELEV].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_THICKNESS].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_OPTICALDEPTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MINSAMPLE].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAXSAMPLE].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_COVERAGE].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_DENSITY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_SELFSHADING].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_RECVSHADOWINTENS].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BACKLIGHTPCT].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_ALTBACKLIGHTCOLORPCT].SetMultiplier(100.0);

} // CloudEffect::SetDefaults

/*===========================================================================*/

void CloudEffect::Copy(CloudEffect *CopyTo, CloudEffect *CopyFrom)
{
WaveSource *CurrentFrom = CopyFrom->WaveSources, **ToPtr, *CurSource;

// delete existing wave sources
while (CopyTo->WaveSources)
	{
	CurSource = CopyTo->WaveSources;
	CopyTo->WaveSources = CopyTo->WaveSources->Next;
	delete CurSource;
	} // while

ToPtr = &CopyTo->WaveSources;

while (CurrentFrom)
	{
	if (*ToPtr = new WaveSource(CopyTo))
		{
		(*ToPtr)->Copy(*ToPtr, CurrentFrom);
		} // if
	ToPtr = &(*ToPtr)->Next;
	CurrentFrom = CurrentFrom->Next;
	} // while

CopyTo->ColorGrad.Copy(&CopyTo->ColorGrad, &CopyFrom->ColorGrad);
CopyTo->BacklightColor.Copy(&CopyTo->BacklightColor, &CopyFrom->BacklightColor);
CopyTo->CovgProf.Copy(&CopyTo->CovgProf, &CopyFrom->CovgProf);
CopyTo->ShadeProf.Copy(&CopyTo->ShadeProf, &CopyFrom->ShadeProf);
CopyTo->DensityProf.Copy(&CopyTo->DensityProf, &CopyFrom->DensityProf);

CopyTo->Rows = CopyFrom->Rows;
CopyTo->Cols =CopyFrom->Cols;
CopyTo->NumLayers = CopyFrom->NumLayers;
CopyTo->Volumetric = CopyFrom->Volumetric;
CopyTo->VolumeBeforeRefl = CopyFrom->VolumeBeforeRefl;
CopyTo->PreviewSize = CopyFrom->PreviewSize;
CopyTo->WavePreviewSize = CopyFrom->WavePreviewSize;
CopyTo->CloudType = CopyFrom->CloudType;
CopyTo->Feather = CopyFrom->Feather;
CopyTo->CastShadows = CopyFrom->CastShadows;
CopyTo->CastShadowStyle = CopyFrom->CastShadowStyle;
CopyTo->ReceiveShadowsTerrain = CopyFrom->ReceiveShadowsTerrain;
CopyTo->ReceiveShadowsFoliage = CopyFrom->ReceiveShadowsFoliage;
CopyTo->ReceiveShadows3DObject = CopyFrom->ReceiveShadows3DObject;
CopyTo->ReceiveShadowsCloudSM = CopyFrom->ReceiveShadowsCloudSM;
CopyTo->ReceiveShadowsVolumetric = CopyFrom->ReceiveShadowsVolumetric;
CopyTo->ReceiveShadowsMisc = CopyFrom->ReceiveShadowsMisc;
CopyTo->ShadowsOnly = CopyFrom->ShadowsOnly;
CopyTo->ShadowFlags = CopyFrom->ShadowFlags;
CopyTo->ShadowMapWidth = CopyFrom->ShadowMapWidth;
CopyTo->UseMapFile = CopyFrom->UseMapFile;
CopyTo->RegenMapFile = CopyFrom->UseMapFile ? 1: 0;
ADProf.Copy(&CopyTo->ADProf, &CopyFrom->ADProf);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // CloudEffect::Copy

/*===========================================================================*/

WaveSource *CloudEffect::AddWave(WaveSource *AddMe)
{
WaveSource **CurSource = &WaveSources;
NotifyTag Changes[2];

while (*CurSource)
	{
	CurSource = &(*CurSource)->Next;
	} // while
if (*CurSource = new WaveSource(this))
	{
	if (AddMe)
		(*CurSource)->Copy(*CurSource, AddMe);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
return (*CurSource);

} // CloudEffect::AddWave

/*===========================================================================*/

WaveSource *CloudEffect::AddWave(double NewLat, double NewLon)
{
WaveSource **CurSource = &WaveSources;
NotifyTag Changes[2];
double LatScaleMeters, LonScaleMeters, RelX, RelY, TempLon;

while (*CurSource)
	{
	CurSource = &(*CurSource)->Next;
	} // while
if (*CurSource = new WaveSource(this))
	{
	LatScaleMeters = LatScale(GlobalApp->AppEffects->GetPlanetRadius());
	LonScaleMeters = LonScale(GlobalApp->AppEffects->GetPlanetRadius(), AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].CurValue);

	TempLon = NewLon - AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue;
	if (fabs(TempLon) > 180.0)
		{
		TempLon += 180;
		if (fabs(TempLon) >= 360.0)
			TempLon = fmod(TempLon, 360.0);	// retains the original sign
		if (TempLon < 0.0)
			TempLon += 360.0;
		TempLon -= 180.0;
		NewLon = TempLon + AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue;
		} // if
	// replaced by above
	//if (NewLon > AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue)
	//	{
	//	while (NewLon - AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue > 180.0)
	//		NewLon -= 360.0;
	//	} // if
	//else if (NewLon < AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue)
	//	{
	//	while (AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue - NewLon  > 180.0)
	//		NewLon += 360.0;
	//	} // if
	RelY = (NewLat - AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].CurValue) * LatScaleMeters;
	RelX = (AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue - NewLon) * LonScaleMeters;

	(*CurSource)->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETY].SetValue(RelY);
	(*CurSource)->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETX].SetValue(RelX);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
return (*CurSource);

} // CloudEffect::AddWave

/*===========================================================================*/

unsigned long CloudEffect::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char TexRootNumber = -1, TempReceiveShadows;
WaveSource **LoadTo = &WaveSources;
RootTexture *DelTex;

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
					case WCS_EFFECTS_CLOUD_ROWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Rows, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_COLS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Cols, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_NUMLAYERS:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumLayers, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_PREVIEWSIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&PreviewSize, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_WAVEPREVIEWSIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&WavePreviewSize, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_CLOUDTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&CloudType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_FEATHER:
						{
						BytesRead = ReadBlock(ffile, (char *)&Feather, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_CASTSHADOWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&CastShadows, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_CASTSHADOWSTYLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&CastShadowStyle, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_RECEIVESHADOWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempReceiveShadows, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempReceiveShadows)
							{
							ReceiveShadowsTerrain = ReceiveShadowsFoliage = ReceiveShadows3DObject = 
								ReceiveShadowsCloudSM = ReceiveShadowsMisc = 1;
							ReceiveShadowsVolumetric = 0;
							} // if
						break;
						}
					case WCS_EFFECTS_CLOUD_RECEIVESHADOWSTER:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsTerrain, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_RECEIVESHADOWSFOL:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsFoliage, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_RECEIVESHADOWS3D:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadows3DObject, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_RECEIVESHADOWSSM:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsCloudSM, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_RECEIVESHADOWSVOL:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsVolumetric, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_RECEIVESHADOWSMISC:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadowsMisc, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_SHADOWSONLY:
						{
						BytesRead = ReadBlock(ffile, (char *)&ShadowsOnly, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_VOLUMETRIC:
						{
						BytesRead = ReadBlock(ffile, (char *)&Volumetric, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_VOLBEFOREREFL:
						{
						BytesRead = ReadBlock(ffile, (char *)&VolumeBeforeRefl, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_SHADOWMAPWIDTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&ShadowMapWidth, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_USEMAPFILE:
						{
						BytesRead = ReadBlock(ffile, (char *)&UseMapFile, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_REGENMAPFILE:
						{
						BytesRead = ReadBlock(ffile, (char *)&RegenMapFile, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_CENTERLAT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_CENTERLON:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_MAPWIDTH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPWIDTH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_MAPHEIGHT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPHEIGHT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_BASEELEV:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BASEELEV].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_COVERAGE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_COVERAGE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_DENSITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_DENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_THICKNESS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_THICKNESS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_FALLOFF:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_FALLOFF].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_SELFSHADING:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_SELFSHADING].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_RECVSHADOWINTENS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_RECVSHADOWINTENS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_OPTICALDEPTH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_OPTICALDEPTH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_MINSAMPLE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MINSAMPLE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_MAXSAMPLE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAXSAMPLE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_BACKLIGHTPCT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BACKLIGHTPCT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_BACKLIGHTEXP:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BACKLIGHTEXP].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_ALTBACKLIGHTPCT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_ALTBACKLIGHTCOLORPCT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_COLORGRAD:
						{
						BytesRead = ColorGrad.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_BACKLIGHTCOLOR:
						{
						BytesRead = BacklightColor.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_COVGPROFILE:
						{
						BytesRead = CovgProf.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_DENSITYPROFILE:
						{
						BytesRead = DensityProf.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_SHADEPROFILE:
						{
						BytesRead = ShadeProf.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CLOUD_TEXTUREROOTNUM:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexRootNumber, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures())
							{
							if (DelTex = TexRoot[TexRootNumber])
								{
								TexRoot[TexRootNumber] = NULL;
								delete DelTex;
								} // if
							TexRoot[TexRootNumber] = new RootTexture(this, 0, 0, 0);
							} // if
						break;
						}
					case WCS_EFFECTS_CLOUD_TEXTUREROOT:
						{
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures() && TexRoot[TexRootNumber])
							BytesRead = TexRoot[TexRootNumber]->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						TexRootNumber = -1;
						break;
						}
					case WCS_EFFECTS_CLOUD_TEXCOVERAGE:
						{
						if (DelTex = TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE])
							{
							TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE] = NULL;
							delete DelTex;
							} // if
						if (TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_CLOUD_WAVESOURCE:
						{
						if (*LoadTo = new WaveSource(this))
							{
							BytesRead = (*LoadTo)->Load(ffile, Size, ByteFlip);
							LoadTo = &(*LoadTo)->Next;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_CLOUD_PROFILE:
						{
						BytesRead = ADProf.Load(ffile, Size, ByteFlip);
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

return (TotalRead);

} // CloudEffect::Load

/*===========================================================================*/

unsigned long CloudEffect::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
WaveSource *Current;
unsigned long AnimItemTag[WCS_EFFECTS_CLOUD_NUMANIMPAR] = {WCS_EFFECTS_CLOUD_CENTERLAT,
																WCS_EFFECTS_CLOUD_CENTERLON,
																WCS_EFFECTS_CLOUD_MAPWIDTH,
																WCS_EFFECTS_CLOUD_MAPHEIGHT,
																WCS_EFFECTS_CLOUD_BASEELEV,
																WCS_EFFECTS_CLOUD_COVERAGE,
																WCS_EFFECTS_CLOUD_DENSITY,
																WCS_EFFECTS_CLOUD_THICKNESS,
																WCS_EFFECTS_CLOUD_FALLOFF,
																WCS_EFFECTS_CLOUD_SELFSHADING,
																WCS_EFFECTS_CLOUD_RECVSHADOWINTENS,
																WCS_EFFECTS_CLOUD_OPTICALDEPTH,
																WCS_EFFECTS_CLOUD_MINSAMPLE,
																WCS_EFFECTS_CLOUD_MAXSAMPLE,
																WCS_EFFECTS_CLOUD_BACKLIGHTPCT,
																WCS_EFFECTS_CLOUD_BACKLIGHTEXP,
																WCS_EFFECTS_CLOUD_ALTBACKLIGHTPCT};
unsigned long TextureItemTag[WCS_EFFECTS_CLOUD_NUMTEXTURES] = {WCS_EFFECTS_CLOUD_TEXCOVERAGE};

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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_ROWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Rows)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_COLS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Cols)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_NUMLAYERS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumLayers)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_PREVIEWSIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&PreviewSize)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_WAVEPREVIEWSIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&WavePreviewSize)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_CLOUDTYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&CloudType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_FEATHER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Feather)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_CASTSHADOWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&CastShadows)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_CASTSHADOWSTYLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&CastShadowStyle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_RECEIVESHADOWSTER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsTerrain)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_RECEIVESHADOWSFOL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsFoliage)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_RECEIVESHADOWS3D, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadows3DObject)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_RECEIVESHADOWSSM, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsCloudSM)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_RECEIVESHADOWSVOL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsVolumetric)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_RECEIVESHADOWSMISC, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadowsMisc)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_SHADOWSONLY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ShadowsOnly)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_VOLUMETRIC, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Volumetric)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_VOLBEFOREREFL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VolumeBeforeRefl)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_SHADOWMAPWIDTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShadowMapWidth)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_USEMAPFILE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&UseMapFile)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CLOUD_REGENMAPFILE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&RegenMapFile)) == NULL)
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

Current = WaveSources;
while (Current)
	{
	ItemTag = WCS_EFFECTS_CLOUD_WAVESOURCE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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
				} // if wave source saved 
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

ItemTag = WCS_EFFECTS_CLOUD_COLORGRAD + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = ColorGrad.Save(ffile))
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
			} // if color saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_EFFECTS_CLOUD_BACKLIGHTCOLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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

ItemTag = WCS_EFFECTS_CLOUD_COVGPROFILE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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

ItemTag = WCS_EFFECTS_CLOUD_DENSITYPROFILE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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

ItemTag = WCS_EFFECTS_CLOUD_SHADEPROFILE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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

ItemTag = WCS_EFFECTS_CLOUD_PROFILE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = ADProf.Save(ffile))
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

} // CloudEffect::Save

/*===========================================================================*/

char *CloudEffectCritterNames[WCS_EFFECTS_CLOUD_NUMANIMPAR] = {"Latitude (deg)", "Longitude (deg)",
	"Cloud Map Width (m)", "Cloud Map Height (m)", "Base Elevation (m)", "Coverage (%)", "Density (%)",
	"Thickness (m)", "Edge Sharpness (1-10)", "Self-shading (%)", "Received Shadow Intensity (%)", "Optical Depth (m)",
	"Minimum Sample Spacing (m) ", "Maximum Sample Spacing (m) ", "Backlight Intensity (%)", "Backlight Exponent", 
	"Alt. Backlight Color (%)"};
char *CloudEffectTextureNames[WCS_EFFECTS_CLOUD_NUMTEXTURES] = {"Coverage (%)"};

char *CloudEffect::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (CloudEffectCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (CloudEffectTextureNames[Ct]);
		} // if
	} // for
if (Test == &ColorGrad)
	return ("Cloud");
if (Test == &BacklightColor)
	return ("Alt. Backlight Color");
if (Test == &CovgProf)
	return ("Vertical Coverage Profile");
if (Test == &DensityProf)
	return ("Vertical Density Profile");
if (Test == &ShadeProf)
	return ("Vertical Shading Profile");
if (Test == &ADProf)
	return ("Edge Feathering Profile");

return ("");

} // CloudEffect::GetCritterName

/*===========================================================================*/

char *CloudEffect::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as a Cloud Texture! Remove anyway?");

} // CloudEffect::OKRemoveRaster

/*===========================================================================*/

int CloudEffect::RemoveRAHost(RasterAnimHost *RemoveMe)
{
WaveSource *CurWave = WaveSources, *PrevWave = NULL;
NotifyTag Changes[2];

while (CurWave)
	{
	if (CurWave == (WaveSource *)RemoveMe)
		{
		if (PrevWave)
			PrevWave->Next = CurWave->Next;
		else
			WaveSources = CurWave->Next;

		delete CurWave;

		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

		return (1);
		} // if
	PrevWave = CurWave;
	CurWave = CurWave->Next;
	} // while

return (GeneralEffect::RemoveRAHost(RemoveMe));

} // CloudEffect::RemoveRAHost

/*===========================================================================*/

void CloudEffect::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->CLG)
	{
	delete GlobalApp->GUIWins->CLG;
	}
GlobalApp->GUIWins->CLG = new CloudEditGUI(GlobalApp->AppEffects, GlobalApp->AppDB, this);
if(GlobalApp->GUIWins->CLG)
	{
	GlobalApp->GUIWins->CLG->Open(GlobalApp->MainProj);
	}

} // CloudEffect::Edit

/*===========================================================================*/

char CloudEffect::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORGRADIENT
	|| DropType == WCS_RAHOST_OBJTYPE_COLORTEXTURE
	|| DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME
	|| DropType == WCS_RAHOST_OBJTYPE_VECTOR
	|| DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE
	|| DropType == WCS_RAHOST_OBJTYPE_WAVESOURCE)
	return (1);

return (0);

} // CloudEffect::GetRAHostDropOK

/*===========================================================================*/

int CloudEffect::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
int QueryResult, Success;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];
char QueryStr[256], NameStr[128];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (CloudEffect *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (CloudEffect *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORGRADIENT ||
	DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_COLORTEXTURE ||
	DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
	{
	Success = ColorGrad.ProcessRAHostDragDrop(DropSource);
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE)
	{
	Success = -1;
	sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, "which Profile");
	if (QueryResult = UserMessageCustom(NameStr, QueryStr, GetCritterName(&CovgProf), "Cancel", GetCritterName(&ShadeProf), 0))
		{
		if (QueryResult == 1)
			{
			Success = CovgProf.ProcessRAHostDragDrop(DropSource);
			} // if
		else if (QueryResult == 2)
			{
			Success = ShadeProf.ProcessRAHostDragDrop(DropSource);
			} // else if
		} // if
	Success = ADProf.ProcessRAHostDragDrop(DropSource);
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_WAVESOURCE)
	{
	Success = -1;
	sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		if (AddWave((WaveSource *)DropSource->DropSource))
			{
			Success = 1;
			} // if
		} // if
	} // else if
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

} // CloudEffect::ProcessRAHostDragDrop

/*===========================================================================*/

char *CloudEffect::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? CloudEffectTextureNames[TexNumber]: (char*)"");

} // CloudEffect::GetTextureName

/*===========================================================================*/

RootTexture *CloudEffect::NewRootTexture(long TexNumber)
{
char ApplyToColor = 0;
char ApplyToDisplace = 1;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // CloudEffect::NewRootTexture

/*===========================================================================*/

int CloudEffect::GetRAHostAnimated(void)
{
WaveSource *Current = WaveSources;

if (GeneralEffect::GetRAHostAnimated())
	return (1);
if (ColorGrad.GetRAHostAnimated())
	return (1);
while (Current)
	{
	if (Current->GetRAHostAnimated())
		return (1);
	Current = Current->Next;
	} // while

return (0);

} // CloudEffect::GetRAHostAnimated

/*===========================================================================*/

int CloudEffect::AnimateCloudShadows(void)
{
WaveSource *CurWave = WaveSources;
long Ct;

for (Ct = 0; Ct < WCS_EFFECTS_CLOUD_ANIMPAR_SELFSHADING; Ct ++)
	{
	if (AnimPar[Ct].GetNumNodes(0) > 1)
		return (1);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (TexRoot[Ct] && TexRoot[Ct]->IsAnimated())	// this will check to see if velocity is non-zero
		return (1);
	} // for
while (CurWave)
	{
	if (CurWave->Enabled)
		{
		if (CurWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE].GetNumNodes(0) > 1)
			return (1);
		if (CurWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_PHASE].GetNumNodes(0) > 1)
			return (1);
		if (CurWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_VELOCITY].CurValue != 0.0)
			return (1);
		} // if
	CurWave = CurWave->Next;
	} // while

return (0);

} // CloudEffect::AnimateCloudShadows

/*===========================================================================*/

int CloudEffect::SetToTime(double Time)
{
long Found = 0;
WaveSource *Current = WaveSources;

if (GeneralEffect::SetToTime(Time))
	Found = 1;
if (ColorGrad.SetToTime(Time))
	Found = 1;
if (BacklightColor.SetToTime(Time))
	Found = 1;
while (Current)
	{
	if (Current->SetToTime(Time))
		Found = 1;
	Current = Current->Next;
	} // while

return (Found);

} // CloudEffect::SetToTime

/*===========================================================================*/

long CloudEffect::InitImageIDs(long &ImageID)
{
long NumImages = 0;
WaveSource *CurWave = WaveSources;

while (CurWave)
	{
	NumImages += CurWave->InitImageIDs(ImageID);
	CurWave = CurWave->Next;
	} // while
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // CloudEffect::InitImageIDs

/*===========================================================================*/

// maximum is in array elements [0], minimum in [1]
int CloudEffect::GetMaterialBoundsXYZ(double XRange[2], double YRange[2], double ZRange[2])
{

XRange[0] = .5 * AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPWIDTH].CurValue;
XRange[1] = -.5 * AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPWIDTH].CurValue;
YRange[0] = .5 * AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPHEIGHT].CurValue;
YRange[1] = -.5 * AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPHEIGHT].CurValue;

ZRange[0] = ZRange[1] = 0.0;

return (1);

} // CloudEffect::GetMaterialBoundsXYZ

/*===========================================================================*/

long CloudEffect::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Found = 0;
WaveSource *CurWave = WaveSources;

if (GeneralEffect::GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (ColorGrad.GetKeyFrameRange(MinDist, MaxDist))
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
while (CurWave)
	{
	if (CurWave->GetKeyFrameRange(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	CurWave = CurWave->Next;
	} // while

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

} // CloudEffect::GetKeyFrameRange

/*===========================================================================*/

unsigned long CloudEffect::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_CLOUD | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // CloudEffect::GetRAFlags

/*===========================================================================*/

int CloudEffect::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
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

} // CloudEffect::GetAffiliates

/*===========================================================================*/

void CloudEffect::GetInterFlags(RasterAnimHostProperties *Prop, RasterAnimHost *FlagMe)
{

if (! FlagMe)
	{
	Prop->InterFlags = 0;
	return;
	} // if

Prop->InterFlags = (WCS_RAHOST_INTERBIT_CLICKTOPOS | WCS_RAHOST_INTERBIT_MOVEX | 
					WCS_RAHOST_INTERBIT_MOVEY | WCS_RAHOST_INTERBIT_MOVEZ | 
					WCS_RAHOST_INTERBIT_MOVEELEV |
					WCS_RAHOST_INTERBIT_SCALEX | WCS_RAHOST_INTERBIT_SCALEY);

} // CloudEffect::GetInterFlags

/*===========================================================================*/

double CloudEffect::GetMaxProfileDistance(void)
{
double FindMin, FindMax;

if (ADProf.GetMinMaxDist(FindMin, FindMax))
	return (FindMax);

return (0.0);

} // CloudEffect::GetMaxProfileDistance

/*===========================================================================*/

int CloudEffect::ScaleMoveRotate(RasterAnimHost *MoveMe, DiagnosticData *Data, unsigned char Operation)
{
double NewVal, NewLat, NewLon;

if (! MoveMe)
	{
	return (0);
	} // if

// Camera group
if (Operation == WCS_RAHOST_INTERACTIVEOP_SETPOS)
	{
	if (GlobalApp->GUIWins->CVG->CollideCoord(Data->ViewSource, NewLat, NewLon, Data->PixelX, Data->PixelY, 
		AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BASEELEV].CurValue))
		{
		AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].SetCurValue(NewLat);
		AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].SetCurValue(NewLon);
		} // if
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_SETPOSNOQUERY)
	{
	if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE])
		AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LATITUDE]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
		AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LONGITUDE]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION])
		AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BASEELEV].SetCurValue(Data->Value[WCS_DIAGNOSTIC_ELEVATION]);
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_MOVEXYZ ||
	Operation == WCS_RAHOST_INTERACTIVEOP_MOVELATLONELEV)
	{
	Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] = 1;
	Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE] = 1;
	Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION] = 1;
	Data->Value[WCS_DIAGNOSTIC_LATITUDE] = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].CurValue;
	Data->Value[WCS_DIAGNOSTIC_LONGITUDE] = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue;
	Data->Value[WCS_DIAGNOSTIC_ELEVATION] = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BASEELEV].CurValue;
	GlobalApp->GUIWins->CVG->ScaleMotion(Data);
	if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE])
		AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LATITUDE]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
		AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LONGITUDE]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION])
		AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BASEELEV].SetCurValue(Data->Value[WCS_DIAGNOSTIC_ELEVATION]);
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_SCALE)
	{
	// allow map width or height
	NewVal = Data->MoveX / 100.0;
	NewVal += 1.0;
	if (NewVal < .01)
		NewVal = .01;
	// let animcritter do clamping
	AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPWIDTH].SetCurValue(AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPWIDTH].CurValue * NewVal);
	NewVal = -Data->MoveY / 100.0;	// sign reverse necessary to make expand correspond to mouse up direction
	NewVal += 1.0;
	if (NewVal < .01)
		NewVal = .01;
	// let animcritter do clamping
	AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPHEIGHT].SetCurValue(AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPHEIGHT].CurValue * NewVal);
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_SETSIZE)
	{
	if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALX])
		{
		AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPWIDTH].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALX]);
		} // if
	if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALY])
		{
		AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPHEIGHT].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALX]);
		} // if
	return (1);
	} // if 

return (0);	// return 0 if nothing changed

} // CloudEffect::ScaleMoveRotate

/*===========================================================================*/

RasterAnimHost *CloudEffect::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;
JoeList *CurJoe = Joes;
WaveSource *CurWave;

if (! Current)
	return (&ColorGrad);
if (Current == &ColorGrad)
	return (&BacklightColor);
if (Current == &BacklightColor)
	return (&CovgProf);
if (Current == &CovgProf)
	return (&DensityProf);
if (Current == &DensityProf)
	return (&ShadeProf);
if (Current == &ShadeProf)
	return (&ADProf);
if (Current == &ADProf)
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
CurWave = WaveSources;
while (CurWave)
	{
	if (Found)
		return (CurWave);
	if (Current == CurWave)
		Found = 1;
	CurWave = CurWave->Next;
	} // while
while (CurJoe)
	{
	if (Found && CurJoe->Me)
		return (CurJoe->Me);
	if (Current == CurJoe->Me)
		Found = 1;
	CurJoe = CurJoe->Next;
	} // while

return (NULL);

} // CloudEffect::GetRAHostChild

/*===========================================================================*/

RasterAnimHost *CloudEffect::GetNextGroupSibling(RasterAnimHost *FindMyBrother)
{

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT))
	return (GetAnimPtr(WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON))
	return (GetAnimPtr(WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_CLOUD_ANIMPAR_MAPWIDTH))
	return (GetAnimPtr(WCS_EFFECTS_CLOUD_ANIMPAR_MAPHEIGHT));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_CLOUD_ANIMPAR_MAPHEIGHT))
	return (GetAnimPtr(WCS_EFFECTS_CLOUD_ANIMPAR_MAPWIDTH));

return (NULL);

} // CloudEffect::GetNextGroupSibling

/*===========================================================================*/

int CloudEffect::GetDeletable(RasterAnimHost *Test)
{
char Ct;
WaveSource *CurWave;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (1);
		} // if
	} // for
CurWave = WaveSources;
while (CurWave)
	{
	if (Test == CurWave)
		return (1);
	CurWave = CurWave->Next;
	} // while

return (0);

} // CloudEffect::GetDeletable

/*===========================================================================*/

bool CloudEffect::ConfirmType(void)
{
bool Confirmed = false;
RootTexture *Root;
Texture *TurbTex, *BellTex, *GainTex;

if (CloudType == WCS_EFFECTS_CLOUDTYPE_CUSTOM)
	return (true);
	
if ((Root = GetTexRootPtr(WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE)) &&
	(TurbTex = Root->Tex) && 
	(TurbTex->TexType == WCS_TEXTURE_TYPE_TURBULENCE || TurbTex->TexType == WCS_TEXTURE_TYPE_FRACTALNOISE) &&
	(GainTex = TurbTex->Tex[WCS_TEXTURE_STRATAFUNC]) &&
	(GainTex->TexType == WCS_TEXTURE_TYPE_GAIN) &&
	(BellTex = TurbTex->Tex[WCS_TEXTURE_BLENDINGFUNC]) &&
	(BellTex->TexType == WCS_TEXTURE_TYPE_BELLCURVE))
	{
	if (TurbTex->TexParam[5].CurValue == 0.0 && // roughness
		GainTex->TexParam[3].CurValue == 100.0 &&	// amplitude
		GainTex->TexParam[4].CurValue == 80.0 &&	// shift
		GainTex->TexParam[5].CurValue == 33.0 &&	// gain
		BellTex->TexParam[8].CurValue == 78.0)	// skew
		{
		switch (CloudType)
			{
			case WCS_EFFECTS_CLOUDTYPE_CIRRUS:
				{
				if (NumLayers == 1 && AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_SELFSHADING].CurValue == 0.0 &&
					BellTex->TexParam[6].CurValue == 40.0)
					Confirmed = true;
				break;
				} // WCS_EFFECTS_CLOUDTYPE_CIRRUS
			case WCS_EFFECTS_CLOUDTYPE_STRATUS:
				{
				if (NumLayers == 3 && AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_SELFSHADING].CurValue == 0.3 &&
					BellTex->TexParam[6].CurValue == 30.0)
					Confirmed = true;
				break;
				} // WCS_EFFECTS_CLOUDTYPE_STRATUS
			case WCS_EFFECTS_CLOUDTYPE_NIMBUS:
				{
				if (NumLayers == 20 && AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_SELFSHADING].CurValue == 0.5 &&
					BellTex->TexParam[6].CurValue == 20.0)
					Confirmed = true;
				break;
				} // WCS_EFFECTS_CLOUDTYPE_NIMBUS
			case WCS_EFFECTS_CLOUDTYPE_CUMULUS:
				{
				if (NumLayers == 40 && AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_SELFSHADING].CurValue == 0.75 &&
					BellTex->TexParam[6].CurValue == 0.0)
					Confirmed = true;
				break;
				} // WCS_EFFECTS_CLOUDTYPE_CUMULUS
			} // switch
		if (Confirmed && AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_THICKNESS].CurValue != (NumLayers - 1) * WCS_EFFECTS_CLOUD_LAYERTHICKNESS)
			Confirmed = false;
		} // if
	} // if

if (! Confirmed)
	{
	CloudType = WCS_EFFECTS_CLOUDTYPE_CUSTOM;
	} // if
	
return (Confirmed);

} // CloudEffect::ConfirmType

/*===========================================================================*/

int CloudEffect::SetCloudType(void)
{
double OldSize[3], OldVelocity[3], OldPreviewSize;
RootTexture *Root, *TempRoot;
Texture *TurbTex, *BellTex, *GainTex;
int SizeNoted = 0;
NotifyTag Changes[2];

if (CloudType != WCS_EFFECTS_CLOUDTYPE_CUSTOM)
	{
	if (! (Root = NewRootTexture(WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE)))
		return (0);

	if (TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE] && TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->Tex && 
		(TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->Tex->TexType == WCS_TEXTURE_TYPE_TURBULENCE ||
		TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->Tex->TexType == WCS_TEXTURE_TYPE_FRACTALNOISE))
		{
		OldSize[0] = TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->Tex->TexSize[0].CurValue;
		OldSize[1] = TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->Tex->TexSize[1].CurValue;
		OldSize[2] = TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->Tex->TexSize[2].CurValue;
		OldVelocity[0] = TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->Tex->TexVelocity[0].CurValue;
		OldVelocity[1] = TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->Tex->TexVelocity[1].CurValue;
		OldVelocity[2] = TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->Tex->TexVelocity[2].CurValue;
		OldPreviewSize = TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->PreviewSize.CurValue;
		SizeNoted = 1;
		} // if

	if (TempRoot = new RootTexture(this, 1, 0, 1))
		{
		if (TurbTex = TempRoot->AddNewTexture(NULL, WCS_TEXTURE_TYPE_TURBULENCE))
			{
			if (GainTex = TurbTex->NewTexture(WCS_TEXTURE_STRATAFUNC, NULL, WCS_TEXTURE_TYPE_GAIN))
				{
				if (BellTex = TurbTex->NewTexture(WCS_TEXTURE_BLENDINGFUNC, NULL, WCS_TEXTURE_TYPE_BELLCURVE))
					{
					// set num layers
					TurbTex->TexParam[4].SetValue(quickdblfloor(xrand48() * 2000.0));
					TurbTex->TexParam[5].SetValue(0.0);		// roughness
					GainTex->TexParam[3].SetValue(100.0);	// amplitude
					GainTex->TexParam[4].SetValue(80.0);	// shift
					GainTex->TexParam[5].SetValue(33.0);	// gain
					BellTex->TexParam[8].SetValue(78.0);	// skew
					switch (CloudType)
						{
						case WCS_EFFECTS_CLOUDTYPE_CIRRUS:
							{
							NumLayers = 1;
							AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_SELFSHADING].SetValue(0.0);
							BellTex->TexParam[6].SetValue(40.0);	// phase
							break;
							} // WCS_EFFECTS_CLOUDTYPE_CIRRUS
						case WCS_EFFECTS_CLOUDTYPE_STRATUS:
							{
							NumLayers = 3;
							AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_SELFSHADING].SetValue(.3);
							BellTex->TexParam[6].SetValue(30.0);	// phase
							break;
							} // WCS_EFFECTS_CLOUDTYPE_STRATUS
						case WCS_EFFECTS_CLOUDTYPE_NIMBUS:
							{
							NumLayers = 20;
							AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_SELFSHADING].SetValue(.5);
							BellTex->TexParam[6].SetValue(20.0);	// phase
							break;
							} // WCS_EFFECTS_CLOUDTYPE_NIMBUS
						case WCS_EFFECTS_CLOUDTYPE_CUMULUS:
							{
							NumLayers = 40;
							AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_SELFSHADING].SetValue(0.75);
							BellTex->TexParam[6].SetValue(0.0);		// phase
							break;
							} // WCS_EFFECTS_CLOUDTYPE_CUMULUS
						} // switch
					AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_THICKNESS].SetValue((NumLayers - 1) * WCS_EFFECTS_CLOUD_LAYERTHICKNESS);
					if (SizeNoted)
						{
						TempRoot->PreviewSize.SetValue(OldPreviewSize);
						TurbTex->TexSize[0].SetValue(OldSize[0]);	// size x
						TurbTex->TexSize[1].SetValue(OldSize[1]);	// size y
						TurbTex->TexSize[2].SetValue(OldSize[2]);	// size z
						TurbTex->TexVelocity[0].SetValue(OldVelocity[0]);	// velocity x
						TurbTex->TexVelocity[1].SetValue(OldVelocity[1]);	// velocity y
						TurbTex->TexVelocity[2].SetValue(OldVelocity[2]);	// velocity z
						} // if
					else
						{
						TempRoot->PreviewSize.SetValue(1500.0);
						TurbTex->TexSize[0].SetValue(1000.0);	// size x
						TurbTex->TexSize[1].SetValue(1000.0);	// size y
						TurbTex->TexSize[2].SetValue(1000.0);	// size z
						} // else
					Root->Copy(Root, TempRoot);
					Changes[0] = MAKE_ID(Root->GetNotifyClass(), Root->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, Root->GetRAHostRoot());
					delete TempRoot;
					return (1);
					} // if
				} // if
			} // if
		} // if

	if (TempRoot)
		delete TempRoot;
	} // if
	
return (0);

} // CloudEffect::SetCloudType

/*===========================================================================*/

void CloudEffect::SetBounds(double LatRange[2], double LonRange[2])
{
double NewCenterLat, NewCenterLon, MapLonScale, NewMapHeight, NewMapWidth;
NotifyTag Changes[2];

// this will set new cloud center lat/lon and cloud map width and height.

// ensure that latitude is within bounds
if (LatRange[0] > 90.0)
	LatRange[0] = 90.0;
if (LatRange[0] < -90.0)
	LatRange[0] = -90.0;
if (LatRange[1] > 90.0)
	LatRange[1] = 90.0;
if (LatRange[1] < -90.0)
	LatRange[1] = -90.0;
// can't use the bounds if they are equal
if (LatRange[0] != LatRange[1] && LonRange[0] != LonRange[1])
	{
	// if bounds appear to wrap more than halfway around earth then probably 
	// want to take the smaller arc
	if (fabs(LonRange[1] - LonRange[0]) > 180.0)
		{
		if (LonRange[1] < LonRange[0])
			swmem(&LonRange[0], &LonRange[1], sizeof (double));
		LonRange[1] -= 360.0;
		} // if
	NewCenterLat = (LatRange[0] + LatRange[1]) * 0.5;
	NewCenterLon = (LonRange[0] + LonRange[1]) * 0.5;
	if (NewCenterLat < 90.0 && NewCenterLat > -90.0)
		{
		MapLonScale = LatScale(GlobalApp->AppEffects->GetPlanetRadius());
		NewMapHeight = fabs(LatRange[0] - LatRange[1]) * MapLonScale;
		MapLonScale *= cos(NewCenterLat * PiOver180);
		NewMapWidth = fabs(LonRange[0] - LonRange[1]) * MapLonScale;
		AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].SetValue(NewCenterLat);
		AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].SetValue(NewCenterLon);
		AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPWIDTH].SetValue(NewMapWidth);
		AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPHEIGHT].SetValue(NewMapHeight);
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if

} // CloudEffect::SetBounds

/*===========================================================================*/

void CloudEffect::SetSourcePosition(WaveSource *SetSource, double NewLat, double NewLon)
{
WaveSource *CurSource = WaveSources;
NotifyTag Changes[2];
double LatScaleMeters, LonScaleMeters, RelX, RelY, TempLon;

// this will set new wave source x and y.

if (NewLat < 90.0 && NewLat > -90.0)
	{
	while (CurSource)
		{
		if (CurSource == SetSource)
			{
			LatScaleMeters = LatScale(GlobalApp->AppEffects->GetPlanetRadius());
			LonScaleMeters = LonScale(GlobalApp->AppEffects->GetPlanetRadius(), AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].CurValue);

			TempLon = NewLon - AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue;
			if (fabs(TempLon) > 180.0)
				{
				TempLon += 180;
				if (fabs(TempLon) >= 360.0)
					TempLon = fmod(TempLon, 360.0);	// retains the original sign
				if (TempLon < 0.0)
					TempLon += 360.0;
				TempLon -= 180.0;
				NewLon = TempLon + AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue;
				} // if
			// replaced by above
			//if (NewLon > AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue)
			//	{
			//	while (NewLon - AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue > 180.0)
			//		NewLon -= 360.0;
			//	} // if
			//else if (NewLon < AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue)
			//	{
			//	while (AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue - NewLon  > 180.0)
			//		NewLon += 360.0;
			//	} // if
			RelY = (NewLat - AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].CurValue) * LatScaleMeters;
			RelX = (AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue - NewLon) * LonScaleMeters;

			CurSource->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETY].SetValue(RelY);
			CurSource->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETX].SetValue(RelX);
			Changes[0] = MAKE_ID(CurSource->GetNotifyClass(), CurSource->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, CurSource->GetRAHostRoot());
			break;
			} // if
		CurSource = CurSource->Next;
		} // while
	} // if

} // CloudEffect::SetSourcePosition

/*===========================================================================*/

void CloudEffect::SetFloating(char NewFloating)
{
double LatScaleMeters, LonScaleMeters, MapHeight, MapWidth;
DEMBounds CurBounds;
NotifyTag Changes[2];

if (GlobalApp->AppDB->FillDEMBounds(&CurBounds))
	{
	LatScaleMeters = LatScale(GlobalApp->AppEffects->GetPlanetRadius());
	LonScaleMeters = cos(AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].CurValue * PiOver180) * LatScaleMeters;
	AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].SetValue((CurBounds.North + CurBounds.South) * 0.5);
	AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].SetValue((CurBounds.West + CurBounds.East) * 0.5);
	MapHeight = min(90.0 - CurBounds.North, 2.0);
	MapHeight = min(CurBounds.South + 90.0, MapHeight);
	MapHeight = (CurBounds.North - CurBounds.South + MapHeight) * LatScaleMeters;
	MapWidth = min(CurBounds.West - CurBounds.East + 4.0, 360.0) * LonScaleMeters;
	AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPWIDTH].SetValue(MapWidth);
	AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPHEIGHT].SetValue(MapHeight);
	} // if
else
	{
	AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].SetValue(GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y));
	AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].SetValue(GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X));
	} // else
Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

} // CloudEffect::SetFloating

/*===========================================================================*/

void CloudEffect::ComputeBoundingBox(CoordSys *DefCoords, double PlanetRad, double ProjRefLat, double ProjRefLon)
{
double LatInt, LonInt;
JoeList *CurJoe;
VectorPoint *PLink;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
int JoesFound = 0;
VertexDEM CurVert;

BoxBoundsX[0] = BoxBoundsY[0] = BoxBoundsZ[0] = FLT_MAX;
BoxBoundsX[1] = BoxBoundsY[1] = BoxBoundsZ[1] = -FLT_MAX;

// if bounded by vectors
CurJoe = Joes;
while (CurJoe)
	{
	if (CurJoe->Me && CurJoe->Me->GetNumRealPoints() > 2)
		{
		// get Coord Sys of Joe
		if (MyAttr = (JoeCoordSys *)CurJoe->Me->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
			MyCoords = MyAttr->Coord;
		else
			MyCoords = NULL;
		PLink = CurJoe->Me->GetFirstRealPoint();
		while (PLink)
			{
			// unproject PLink to default geographic
			// get cartesian coords
			PLink->ProjToDefDeg(MyCoords, &CurVert);
			CurVert.Elev = LowElev;
			#ifdef WCS_BUILD_VNS
			DefCoords->DegToCart(&CurVert);
			#else // WCS_BUILD_VNS
			CurVert.DegToCart(PlanetRad);
			#endif // WCS_BUILD_VNS
			CompareSetBounds(&CurVert);
			PLink = PLink->Next;
			} // while
		PLink = CurJoe->Me->GetFirstRealPoint();
		while (PLink)
			{
			// unproject PLink to default geographic
			// get cartesian coords
			PLink->ProjToDefDeg(MyCoords, &CurVert);
			CurVert.Elev = HighElev;
			#ifdef WCS_BUILD_VNS
			DefCoords->DegToCart(&CurVert);
			#else // WCS_BUILD_VNS
			CurVert.DegToCart(PlanetRad);
			#endif // WCS_BUILD_VNS
			CompareSetBounds(&CurVert);
			PLink = PLink->Next;
			} // while
		JoesFound = 1;
		} // if
	CurJoe = CurJoe->Next;
	} // if
if (! JoesFound)
	{
	// project bottom corners
	CurVert.Elev = LowElev;
	CurVert.Lat = LowLat;
	CurVert.Lon = LowLon;
	#ifdef WCS_BUILD_VNS
	DefCoords->DegToCart(&CurVert);
	#else // WCS_BUILD_VNS
	CurVert.DegToCart(PlanetRad);
	#endif // WCS_BUILD_VNS
	CompareSetBounds(&CurVert);

	CurVert.Lon = HighLon;
	#ifdef WCS_BUILD_VNS
	DefCoords->DegToCart(&CurVert);
	#else // WCS_BUILD_VNS
	CurVert.DegToCart(PlanetRad);
	#endif // WCS_BUILD_VNS
	CompareSetBounds(&CurVert);

	CurVert.Lat = HighLat;
	CurVert.Lon = LowLon;
	#ifdef WCS_BUILD_VNS
	DefCoords->DegToCart(&CurVert);
	#else // WCS_BUILD_VNS
	CurVert.DegToCart(PlanetRad);
	#endif // WCS_BUILD_VNS
	CompareSetBounds(&CurVert);

	CurVert.Lon = HighLon;
	#ifdef WCS_BUILD_VNS
	DefCoords->DegToCart(&CurVert);
	#else // WCS_BUILD_VNS
	CurVert.DegToCart(PlanetRad);
	#endif // WCS_BUILD_VNS
	CompareSetBounds(&CurVert);

	// project top sides at intervals
	CurVert.Elev = HighElev;
	LatInt = (HighLat - LowLat) * .1;
	LonInt = (HighLon - LowLon) * .1;
	for (CurVert.Lat = LowLat; CurVert.Lat <= HighLat; CurVert.Lat += LatInt)
		{
		for (CurVert.Lon = LowLon; CurVert.Lon <= HighLon; CurVert.Lon += LonInt)
			{
			#ifdef WCS_BUILD_VNS
			DefCoords->DegToCart(&CurVert);
			#else // WCS_BUILD_VNS
			CurVert.DegToCart(PlanetRad);
			#endif // WCS_BUILD_VNS
			CompareSetBounds(&CurVert);
			} // for
		} // for
	} // else

SetQuadNormals();

} // CloudEffect::ComputeBoundingBox

/*===========================================================================*/

int CloudEffect::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{
char Ct;
WaveSource *CurWave;

PixelTexturesExist = 0;

if (! ColorGrad.InitToRender())
	return (0);
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (TexRoot[Ct] && TexRoot[Ct]->Enabled)
		{
		if (! TexRoot[Ct]->InitAAChain())
			{
			return (0);
			} // if
		PixelTexturesExist = 1;
		} // if
	} // for
CurWave = WaveSources;
while (CurWave)
	{
	if (CurWave->Enabled)
		{
		if (! CurWave->InitToRender())
			return (0);
		} // if
	CurWave = CurWave->Next;
	} // while

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

RenderVolumetric = (Enabled && Volumetric && ! ShadowsOnly && Rows > 1 && Cols > 1);
RenderPlanar = (Enabled && ! Volumetric && ! ShadowsOnly && Rows > 1 && Cols > 1);
RenderShadowMap = (Enabled && Rows > 1 && Cols > 1 && (CastShadows && 
					(CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_SHADOWMAP || 
					CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_COMBINATION)));
CastVolumeShadows = (Enabled && Rows > 1 && Cols > 1 && (CastShadows && 
					(CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_VOLUMETRIC || 
					CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_COMBINATION)));
VolumeSub = (CastVolumeShadows || RenderVolumetric);

return (1);

} // CloudEffect::InitToRender

/*===========================================================================*/

int CloudEffect::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{
double LatRange, LonRange;
VertexDEM CurVert;

CompleteBacklightColor[0] = BacklightColor.GetCompleteValue(0);
CompleteBacklightColor[1] = BacklightColor.GetCompleteValue(1);
CompleteBacklightColor[2] = BacklightColor.GetCompleteValue(2);

MetersPerDegLat = Rend->EarthLatScaleMeters;
MetersPerDegLon = Rend->EarthLatScaleMeters * cos(AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].CurValue * PiOver180);
RefLat = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].CurValue;
RefLon = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue;

LatRange = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPHEIGHT].CurValue / MetersPerDegLat;
LonRange = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPWIDTH].CurValue / MetersPerDegLon;
LowLat = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].CurValue - .5 * LatRange;
HighLat = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].CurValue + .5 * LatRange;
if (LowLat < - 90.0)
	LowLat = - 90.0;
if (HighLat > 90.0)
	LowLat = 90.0;
LatRange = HighLat - LowLat;
HighLon = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue + .5 * LonRange;
LowLon = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue - .5 * LonRange;
LowElev = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BASEELEV].CurValue;
ElevRange = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_THICKNESS].CurValue;
HighElev = LowElev + ElevRange;

CurVert.Lat = (HighLat + LowLat) * .5;
CurVert.Lon = (HighLon + LowLon) * .5;
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

MaxAmp = fabs(GetMaxWaveAmp());
MinAmp = -MaxAmp;

// rows run west-east, columns run south-north
LatStep = LatRange / (Rows - 1);
LonStep = LonRange / (Cols - 1);

CenterRow = Rows / 2;
CenterCol = Cols / 2;

// <<<>>>gh this is an instance where bitmap textures should be sampled by barycentric weighting
// Otherwise we might get wicked stairstep shading if cloud resolution is higher than bitmap res.
CovgTextureExists = (TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE] && TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->Enabled);

if (CovgTextureExists)
	MaxAmp += 1.0;	// this is the amplitude potentially added by coverage texture
if ((AmpRange = MaxAmp - MinAmp) <= 0.0)
	AmpRange = 1.0;

return (GeneralEffect::InitFrameToRender(Lib, Rend));

} // CloudEffect::InitFrameToRender

/*===========================================================================*/

double CloudEffect::GetMaxWaveAmp(void)
{
double TempMaxAmp = 0.0;
WaveSource *CurWave = WaveSources;

while (CurWave)
	{
	if (CurWave->Enabled)
		TempMaxAmp += CurWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE].CurValue;
	CurWave = CurWave->Next;
	} // while

return (TempMaxAmp);

} // CloudEffect::GetMaxWaveAmp

/*===========================================================================*/

int CloudEffect::EvalSampleInit(Raster *PlotRast, TextureSampleData *Samp)
{

Samp->PreviewSize = max(AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPWIDTH].CurValue, AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAPHEIGHT].CurValue);
Samp->PreviewSize /= pow(2.0, PreviewSize - 1.0);
if (TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE])
	return (TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->EvalSampleInit(PlotRast, Samp));

return (0);

} // CloudEffect::EvalSampleInit

/*===========================================================================*/


int CloudEffect::EvalOneSampleLine(TextureSampleData *Samp)
{
double CloudWt, SkyWt;
int DefSkyColor[3] = {111, 172, 242};
int Result, X, Zip;
unsigned char Red, Green, Blue;

Zip = Samp->zip;

Result = TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->EvalOneSampleLine(Samp);

for (X = 0; X < WCS_RASTER_TNAIL_SIZE; X ++, Zip ++)
	{
	CloudWt = Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Zip] / 255.0;
	if (AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_COVERAGE].CurValue < 1.0)
		{
		if (CloudWt > 1.0 - AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_COVERAGE].CurValue)
			CloudWt = (1.0 + (CloudWt - 1.0) / AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_COVERAGE].CurValue);
		else
			CloudWt = 0.0;
		} // if
	CloudWt *= AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_DENSITY].CurValue;
	if (! ColorGrad.GetBasicColor(Red, Green, Blue, CloudWt))
		Red = Green = Blue = 0;
	if (CloudWt > 1.0)
		CloudWt = 1.0;
	SkyWt = 1.0 - CloudWt;	
	Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Zip] = (unsigned char)(CloudWt * Red + SkyWt * DefSkyColor[0]);
	Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Zip] = (unsigned char)(CloudWt * Green + SkyWt * DefSkyColor[1]);
	Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Zip] = (unsigned char)(CloudWt * Blue + SkyWt * DefSkyColor[2]);
	} // for

return (Result);

} // CloudEffect::EvalOneSampleLine

/*===========================================================================*/

int CloudEffect::EvalWaveSampleInit(Raster *PlotRast, TextureSampleData *Samp)
{
long ReadyGo;

Samp->Running = 0;

if (! PlotRast->ThumbnailValid())
	ReadyGo = (long)PlotRast->AllocThumbnail();
else
	{
	ReadyGo = 1;
	PlotRast->ClearThumbnail();
	} // else
Samp->Thumb = PlotRast->Thumb;

if (ReadyGo)
	{
	Samp->PreviewSize = 50000.0;
	Samp->PreviewSize /= pow(2.0, WavePreviewSize - 1.0);
	if (! InitToRender(NULL, NULL))
		return (0);
	Samp->SampleInc = GetMaxWaveAmp();
	Samp->y = Samp->zip = 0;
	Samp->Running = 1;
	return (1);
	} // if

return (0);

} // CloudEffect::EvalWaveSampleInit

/*===========================================================================*/

int CloudEffect::EvalOneWaveSampleLine(TextureSampleData *Samp)
{
double WaveX, WaveY, CurTime, WaveHt, MidSample;
int X;

CurTime = GlobalApp->MainProj->Interactive->GetActiveTime();
MidSample = WCS_RASTER_TNAIL_SIZE * 0.5;

// evaluate wave amp relative to maximum range possible

WaveY = ((MidSample - Samp->y) / WCS_RASTER_TNAIL_SIZE) * Samp->PreviewSize;

for (X = 0; X < WCS_RASTER_TNAIL_SIZE; X ++, Samp->zip ++)
	{
	if (Samp->SampleInc <= 0.0)
		{
		Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip] = 
			Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip] = 
			Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip] = 128;
		} // if
	else
		{
		WaveX = ((MidSample - X) / WCS_RASTER_TNAIL_SIZE) * Samp->PreviewSize;
		WaveHt = .5 + .5 * EvalSampleWaveHeight(CurTime, WaveX, WaveY) / Samp->SampleInc;
		Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip] = 
			Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip] = 
			Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip] = (unsigned char)(WaveHt * 255);
		} // else
	} // for

Samp->y ++;
if (Samp->y < WCS_RASTER_TNAIL_SIZE)
	return (0);

Samp->Running = 0;
return (1);

} // CloudEffect::EvalOneWaveSampleLine

/*===========================================================================*/

TextureData *CloudEffect::TransferTextureData(VertexDEM *Vert, TextureData *TexData)
{

TexData->Elev = Vert->Elev;
TexData->Latitude = Vert->Lat;
TexData->Longitude = Vert->Lon;
TexData->ZDist = Vert->ScrnXYZ[2];
TexData->QDist = Vert->Q;
TexData->PData = NULL;
TexData->VData[0] = TexData->VData[1] = TexData->VData[2] = NULL;
TexData->VDEM[0] = Vert;
TexData->VDEM[1] = TexData->VDEM[2] = NULL;

return (TexData);

} // CloudEffect::TransferTextureData

/*===========================================================================*/

double CloudEffect::EvalSampleWaveHeight(double CurTime, double RelX, double RelY)
{
double WaveAmp = 0.0;
WaveSource *CurWave;

if (WaveSources)
	{
	// evaluate each wave source
	CurWave = WaveSources;
	while (CurWave)
		{
		if (CurWave->Enabled)
			WaveAmp += CurWave->EvalSampleCloudWaveHeight(RelX, RelY, CurTime);
		CurWave = CurWave->Next;
		} // while

	return (WaveAmp);
	} // if amplitude > 0

return (0.0);

} // CloudEffect::EvalSampleWaveHeight

/*===========================================================================*/

double CloudEffect::EvalWaveHeight(VertexCloud *Vert, TextureData *TexData, double CurTime)
{
double WaveAmp = 0.0, RelX, RelY;
WaveSource *CurWave;

if (WaveSources)
	{
	// compute vertex offset from cloud map center coords
	RelY = (Vert->Lat - RefLat) * MetersPerDegLat;
	RelX = (RefLon - Vert->Lon) * MetersPerDegLon;

	// evaluate each wave source
	CurWave = WaveSources;
	while (CurWave)
		{
		if (CurWave->Enabled)
			WaveAmp += CurWave->EvalCloudWaveHeight(this, Vert, TexData, RelX, RelY, CurTime);
		CurWave = CurWave->Next;
		} // while

	return (WaveAmp);
	} // if amplitude > 0

return (0.0);

} // CloudEffect::EvalWaveHeight

/*===========================================================================*/

int CloudEffect::EvaluateDensity(PixelData *Pix, CloudLayerData *LayerData, double VertexIntensity)
{
double Value[3];

if (CovgTextureExists)
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->Eval(Value, Pix->TexData);
	// Multiply by VertexIntensity moved up from RenderPolygon() 7/1/02 GRH to avoid multiplying wave part by intensity twice
	Pix->OpacityUsed += VertexIntensity * Value[0] / AmpRange;
	// adjust for coverage value
	if (LayerData->Coverage < 1.0)
		{
		if (LayerData->Coverage > 0.0 && Pix->OpacityUsed > 1.0 - LayerData->Coverage)
			Pix->OpacityUsed = (LayerData->Coverage + Pix->OpacityUsed - 1.0) / LayerData->Coverage;
		else
			Pix->OpacityUsed = 0.0;
		} // if
	// adjust for layer density
	if (Pix->OpacityUsed <= 0.0)
		Pix->OpacityUsed = 0.0;
	else
		Pix->OpacityUsed *= LayerData->Density;
	} // if
else
	Pix->OpacityUsed += VertexIntensity * LayerData->Density / AmpRange;

if (Pix->OpacityUsed > 0.0)
	{
	if (Pix->OpacityUsed > 1.0)
		Pix->OpacityUsed = 1.0;
	// users voted to have layer colors rather than density based color
	//ColorGrad.GetBasicColor(Pix->RGB[0], Pix->RGB[1], Pix->RGB[2], Pix->OpacityUsed);
	Pix->RGB[0] = LayerData->RGB[0];
	Pix->RGB[1] = LayerData->RGB[1];
	Pix->RGB[2] = LayerData->RGB[2];
	Pix->CloudShading = LayerData->Shading;
	return (1);
	} // if
else
	{
	Pix->RGB[0] = Pix->RGB[1] = Pix->RGB[2] = 0.0;
	Pix->CloudShading = LayerData->Shading;
	} // else

return (0);

} // CloudEffect::EvaluateDensity

/*===========================================================================*/

// RayVec needs to be a unit vector
VolumeRayList *CloudEffect::BuildRayList(CoordSys *DefCoords, double PlanetRad, double RayStart[3], double RayVec[3])
{
Point3d WorldOrigin, Intersection1, Intersection2;
double Dist1, Dist2, ClosestIn, FarthestOut, Jitter;
int Intersections;
VolumeRayList *RayHits;

if (AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_DENSITY].CurValue <= 0.0)
	return (NULL);

if (RayHits = VolumetricSubstance::BuildRayList(DefCoords, PlanetRad, RayStart, RayVec))
	{
	WorldOrigin[0] = WorldOrigin[1] = WorldOrigin[2] = 0.0;
	ClosestIn = FarthestOut = -1.0;

	// test intersection with inside sphere
	if (Intersections = RaySphereIntersect(RadiusInside, RayStart, RayVec, WorldOrigin, Intersection1, Dist1, Intersection2, Dist2))
		{
		if (Intersections == 1)
			{
			ClosestIn = Dist1;
			} // if
		else
			{
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
		if (RayHits->SampleRate < AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MINSAMPLE].CurValue)
			RayHits->SampleRate = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MINSAMPLE].CurValue;
		else if (RayHits->SampleRate > AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAXSAMPLE].CurValue)
			RayHits->SampleRate = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_MAXSAMPLE].CurValue;
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

} // CloudEffect::BuildRayList

/*===========================================================================*/

int CloudEffect::PointSampleVolumetric(VolumeRayList *CurRay, VertexDEM *CurVert, TextureData *TexData, double &Shading)
{
double Value[3], Coverage, Dens, GradientPos, OpacityUsed, VertexIntensity, dCellX, dCellY, Xint, Yint;
long Zip;

GradientPos = ElevRange > 0.0 ? (CurVert->Elev - LowElev) / ElevRange: 0.0;

if (GradientPos < 0.0 || GradientPos > 1.0)
	{
	CurRay->SegOpticalDepth = FLT_MAX;
	CurRay->SegColor[0] = CurRay->SegColor[1] = CurRay->SegColor[2] = 0.0;
	return (0);
	} // if not in cloud zone

//find cloud cell and offsets from cell base
dCellY = (Rows - 1) * (CurVert->Lat - LowLat) / (HighLat - LowLat);
dCellX = (Cols - 1) * (HighLon - CurVert->Lon) / (HighLon - LowLon);
if (dCellX >= 0.0 && dCellY >= 0.0 && dCellX <= Cols - 1 && dCellY <= Rows - 1)
	{
	Zip = quickftol(dCellY) + Rows * quickftol(dCellX);
	Xint = dCellX - floor(dCellX);
	Yint = dCellY - floor(dCellY);
	if (dCellX == Cols - 1)
		{
		Zip -= Rows;
		Xint = 1.0;
		} // if
	if (dCellY == Rows - 1)
		{
		Zip --;
		Yint = 1.0;
		} // if
	//VertexIntensity = interpolate cloud vertices xyz[1]
	VertexIntensity = Vertices[Zip].xyz[1] * (1.0 - Xint) * (1.0 - Yint)
		+ Vertices[Zip + 1 + Rows].xyz[1] * Xint * Yint
		+ Vertices[Zip + 1].xyz[1] * (1.0 - Xint) * Yint
		+ Vertices[Zip + Rows].xyz[1] * Xint * (1.0 - Yint);
	} // if
else
	{
	CurRay->SegOpticalDepth = FLT_MAX;
	CurRay->SegColor[0] = CurRay->SegColor[1] = CurRay->SegColor[2] = 0.0;
	return (0);
	} // if not in cloud zone

if (VertexIntensity > 0.0)
	{
	// may be needed in calling function
	TexData->Object = this;
	TexData->ObjectType = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_CLOUD;
	//OpacityUsed = interpolate vertices xyz[0]
	OpacityUsed = Vertices[Zip].xyz[0] * (1.0 - Xint) * (1.0 - Yint)
		+ Vertices[Zip + 1 + Rows].xyz[0] * Xint * Yint
		+ Vertices[Zip + 1].xyz[0] * (1.0 - Xint) * Yint
		+ Vertices[Zip + Rows].xyz[0] * Xint * (1.0 - Yint);

	Dens = DensityProf.GetValue(0, GradientPos) * AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_DENSITY].CurValue;
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
		TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->Eval(Value, TexData);
		// Multiply by VertexIntensity moved up from below 7/1/02 GRH to avoid multiplying wave part by intensity twice
		OpacityUsed += VertexIntensity * Value[0] / AmpRange;
		// adjust for coverage value
		Coverage = CovgProf.GetValue(0, GradientPos) * AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_COVERAGE].CurValue;
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
		OpacityUsed += VertexIntensity * Dens / AmpRange;
	// why are we multiplying by intensity sans waves here? Moved up to above 7/1/02 GRH
	//OpacityUsed *= VertexIntensity;

	if (OpacityUsed > 0.0)
		{
		if (OpacityUsed > 1.0)
			OpacityUsed = 1.0;
		ColorGrad.GetBasicColor(CurRay->SegColor[0], CurRay->SegColor[1], CurRay->SegColor[2], GradientPos);
		Shading = ShadeProf.GetValue(0, GradientPos) * AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_SELFSHADING].CurValue;
		CurRay->SegOpticalDepth = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_OPTICALDEPTH].CurValue / OpacityUsed;
		return (1);
		} // if
	} // if VertexIntensity > 0

CurRay->SegOpticalDepth = FLT_MAX;
CurRay->SegColor[0] = CurRay->SegColor[1] = CurRay->SegColor[2] = 0.0;

return (0);

} // CloudEffect::PointSampleVolumetric

/*===========================================================================*/

void CloudEffect::EvalVertices(double CurTime, double LocalMetersPerDegLat)
{
long Row, Col, Zip;
TextureData TexData;
double Value, Intensity, CurJoeIntensity, MaxJoeIntensity, D1, D2, DistX, DistY, DistZ;
JoeList *CurJoe;

TexData.TexRefLon = RefLon;
TexData.TexRefLat = RefLat;
TexData.TexRefElev = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BASEELEV].CurValue;
TexData.MetersPerDegLon = MetersPerDegLon;
TexData.MetersPerDegLat = MetersPerDegLat;

for (Col = Zip = 0; Col < Cols; Col ++)
	{
	for (Row = 0; Row < Rows; Row ++, Zip ++)
		{
		if (AmpRange > 0.0)
			{
			// is feathering on?
			if (Feather)
				{
				D1 = (double)(2 * (Row - CenterRow)) / (double)Rows; //lint !e790
				D2 = (double)(2 * (Col - CenterCol)) / (double)Cols; //lint !e790
				Intensity = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_FALLOFF].CurValue * (1.0 - sqrt(D1 * D1 + D2 * D2));
				if (Intensity < 0.0)
					Intensity = 0.0;
				else if (Intensity > 1.0)
					Intensity = 1.0;
				else
					Intensity = sqrt(Intensity);		// this rounds the falloff curve at upper end
				} // if
			else
				Intensity = 1.0;

			// is vector bounded?
			if (Intensity > 0.0 && (CurJoe = Joes))
				{
				MaxJoeIntensity = 0.0;
				while (CurJoe)
					{
					if (CurJoe->Me && CurJoe->Me->TestFlags(WCS_JOEFLAG_ACTIVATED))
						{
						// find out if vertex coords are within joe outline
						if (CurJoe->Me->SimpleContained(Vertices[Zip].Lat, Vertices[Zip].Lon))
							{
							// reduce amplitude if there is a gradient profile
							if (UseGradient)
								{
								// this undoubtedly isn't the most efficient way to get a distance but it
								// doesn't require georasters and it is more accurate than using georasters.
								DistX = fabs(CurJoe->Me->MinDistToPoint(Vertices[Zip].Lat, Vertices[Zip].Lon, Vertices[Zip].Elev, LocalMetersPerDegLat, 1, 0.0, 1.0, DistX, DistY, DistZ));
								if ((CurJoeIntensity = ADProf.GetValue(0, DistX)) > MaxJoeIntensity)
									MaxJoeIntensity = CurJoeIntensity;
								} // if
							else
								MaxJoeIntensity = 1.0;
							} // if
						} // if
					CurJoe = CurJoe->Next;
					} // while
				Intensity *= MaxJoeIntensity;
				} // if

			// evaluate at current vertex
			if (Intensity > 0.0)
				{
				Value = EvalWaveHeight(&Vertices[Zip], &TexData, CurTime);
				Vertices[Zip].xyz[0] = Intensity * (Value - MinAmp) / AmpRange;
				Vertices[Zip].xyz[1] = Intensity;
				} // if
			else
				Vertices[Zip].xyz[0] = Vertices[Zip].xyz[1] = 0.0;
			} // if
		else
			Vertices[Zip].xyz[0] = Vertices[Zip].xyz[1] = 0.0;
		} // for
	} // for

} // CloudEffect::EvalVertices

/*===========================================================================*/

int CloudEffect::AllocVertices(void)
{
double Lat, Lon;
long Row, Col, Zip;

if (Vertices)
	delete [] Vertices;

// create vertex array
// rows run west-east, columns run south-north

//  north
//  col +>
// ||||||| r 
// ||||||| o +^
// ||||||| w 
// |||||||

if (Vertices = new VertexCloud[Rows * Cols])
	{
	Lon = HighLon;
	for (Col = Zip = 0; Col < Cols; Col ++, Lon -= LonStep)
		{
		Lat = LowLat;
		for (Row = 0; Row < Rows; Row ++, Zip ++, Lat += LatStep)
			{
			Vertices[Zip].Lat = Lat;
			Vertices[Zip].Lon = Lon;
			Vertices[Zip].Elev = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BASEELEV].CurValue;
			} // for
		} // for
	} // if

return (1);

} // CloudEffect::AllocVertices

/*===========================================================================*/

void CloudEffect::RemoveVertices(void)
{

if (Vertices)
	delete [] Vertices;
Vertices = NULL;

} // CloudEffect::RemoveVertices

/*===========================================================================*/

int CloudEffect::ProjectBounds(Camera *Cam, RenderData *Rend, double *LimitsX, double *LimitsY, double *LimitsZ)
{
long Ct, Ct1, Ct2, Found = 0;
VertexDEM Vert;
double LatBounds[2], LonBounds[2], ElevBounds[2];

// HighLat... were initialized in InitFrameToRender

LatBounds[0] = HighLat;
LatBounds[1] = LowLat;
LonBounds[0] = HighLon;
LonBounds[1] = LowLon;
ElevBounds[0] = HighElev;
ElevBounds[1] = LowElev;

LimitsX[0] = LimitsY[0] = LimitsZ[0] = -FLT_MAX;
LimitsX[1] = LimitsY[1] = LimitsZ[1] = FLT_MAX;

for (Ct = 0; Ct < 2; Ct ++)
	{
	Vert.Lat = LatBounds[Ct];
	for (Ct1 = 0; Ct1 < 2; Ct1 ++)
		{
		Vert.Lon = LonBounds[Ct1];
		for (Ct2 = 0; Ct2 < 2; Ct2 ++)
			{
			if (Ct2 && ElevBounds[0] == ElevBounds[1])
				break;
			Vert.Elev = ElevBounds[Ct2];
			Cam->ProjectVertexDEM(Rend->DefCoords, &Vert, Rend->EarthLatScaleMeters, Rend->PlanetRad, 1);
			if (Vert.ScrnXYZ[2] > 0.0)
				{
				if (Vert.ScrnXYZ[0] > LimitsX[0])
					LimitsX[0] = Vert.ScrnXYZ[0];
				if (Vert.ScrnXYZ[0] < LimitsX[1])
					LimitsX[1] = Vert.ScrnXYZ[0];
				if (Vert.ScrnXYZ[1] > LimitsY[0])
					LimitsY[0] = Vert.ScrnXYZ[1];
				if (Vert.ScrnXYZ[1] < LimitsY[1])
					LimitsY[1] = Vert.ScrnXYZ[1];
				if (Vert.ScrnXYZ[2] > LimitsZ[0])
					LimitsZ[0] = Vert.ScrnXYZ[2];
				if (Vert.ScrnXYZ[2] < LimitsZ[1])
					LimitsZ[1] = Vert.ScrnXYZ[2];
				Found ++;
				} // if
			} // for
		} // for
	} // for
Vert.Lat = (LatBounds[0] + LatBounds[1]) * 0.5;
Vert.Lon = (LonBounds[0] + LonBounds[1]) * 0.5;
Vert.Elev = ElevBounds[0];
Cam->ProjectVertexDEM(Rend->DefCoords, &Vert, Rend->EarthLatScaleMeters, Rend->PlanetRad, 1);
if (Vert.ScrnXYZ[2] > 0.0)
	{
	if (Vert.ScrnXYZ[0] > LimitsX[0])
		LimitsX[0] = Vert.ScrnXYZ[0];
	if (Vert.ScrnXYZ[0] < LimitsX[1])
		LimitsX[1] = Vert.ScrnXYZ[0];
	if (Vert.ScrnXYZ[1] > LimitsY[0])
		LimitsY[0] = Vert.ScrnXYZ[1];
	if (Vert.ScrnXYZ[1] < LimitsY[1])
		LimitsY[1] = Vert.ScrnXYZ[1];
	if (Vert.ScrnXYZ[2] > LimitsZ[0])
		LimitsZ[0] = Vert.ScrnXYZ[2];
	if (Vert.ScrnXYZ[2] < LimitsZ[1])
		LimitsZ[1] = Vert.ScrnXYZ[2];
	Found ++;
	} // if
if (ElevBounds[0] != ElevBounds[1])
	{
	Vert.Elev = ElevBounds[1];
	Cam->ProjectVertexDEM(Rend->DefCoords, &Vert, Rend->EarthLatScaleMeters, Rend->PlanetRad, 1);
	if (Vert.ScrnXYZ[2] > 0.0)
		{
		if (Vert.ScrnXYZ[0] > LimitsX[0])
			LimitsX[0] = Vert.ScrnXYZ[0];
		if (Vert.ScrnXYZ[0] < LimitsX[1])
			LimitsX[1] = Vert.ScrnXYZ[0];
		if (Vert.ScrnXYZ[1] > LimitsY[0])
			LimitsY[0] = Vert.ScrnXYZ[1];
		if (Vert.ScrnXYZ[1] < LimitsY[1])
			LimitsY[1] = Vert.ScrnXYZ[1];
		if (Vert.ScrnXYZ[2] > LimitsZ[0])
			LimitsZ[0] = Vert.ScrnXYZ[2];
		if (Vert.ScrnXYZ[2] < LimitsZ[1])
			LimitsZ[1] = Vert.ScrnXYZ[2];
		Found ++;
		} // if
	} // if

return (Found > 1);

} // CloudEffect::ProjectBounds

/*===========================================================================*/

void CloudEffect::ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds)
{
double ShiftWE, ShiftNS, TempVal;
GraphNode *CurNode;

ShiftWE = ((CurBounds->West + CurBounds->East) - (OldBounds->West + OldBounds->East)) * 0.5;
ShiftNS = ((CurBounds->North + CurBounds->South) - (OldBounds->North + OldBounds->South)) * 0.5;

AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].SetValue(
	AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].CurValue + ShiftNS);
if (CurNode = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].GetFirstNode(0))
	{
	TempVal = CurNode->GetValue() + ShiftNS;
	if (TempVal > 89.0)
		TempVal = 89.0;
	if (TempVal < -89.0)
		TempVal = -89.0;
	CurNode->SetValue(TempVal);
	while (CurNode = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].GetNextNode(0, CurNode))
		{
		TempVal = CurNode->GetValue() + ShiftNS;
		if (TempVal > 89.0)
			TempVal = 89.0;
		if (TempVal < -89.0)
			TempVal = -89.0;
		CurNode->SetValue(TempVal);
		} // while
	} // if
AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].SetValue(
	AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue + ShiftWE);
if (CurNode = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].GetFirstNode(0))
	{
	CurNode->SetValue(CurNode->GetValue() + ShiftWE);
	while (CurNode = AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].GetNextNode(0, CurNode))
		{
		CurNode->SetValue(CurNode->GetValue() + ShiftWE);
		} // while
	} // if

} // CloudEffect::ScaleToDEMBounds

/*===========================================================================*/

int CloudEffect::BuildFileComponentsList(EffectList **Coords)
{
WaveSource *CurWave = WaveSources;

while (CurWave)
	{
	if (! CurWave->BuildFileComponentsList(Coords))
		return (0);
	CurWave = CurWave->Next;
	} // while

return (1);

} // CloudEffect::BuildFileComponentsList

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int CloudEffect::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
DEMBounds OldBounds, CurBounds;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
CloudEffect *CurrentCloud = NULL;
SearchQuery *CurrentQuery = NULL;
ThematicMap *CurrentTheme = NULL;
CoordSys *CurrentCoordSys = NULL;
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
					else if (! strnicmp(ReadBuf, "Cloud", 8))
						{
						if (CurrentCloud = new CloudEffect(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentCloud->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if CelestialEffect
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

if (Success == 1 && CurrentCloud)
	{
	if (OldBoundsLoaded && GlobalApp->AppDB->FillDEMBounds(&CurBounds))
		{
		if (UserMessageYN("Load Cloud Model", "Do you wish the loaded Cloud Model's position\n to be scaled to current DEM bounds?"))
			{
			CurrentCloud->ScaleToDEMBounds(&OldBounds, &CurBounds);
			} // if
		} // if
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentCloud);
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

} // CloudEffect::LoadObject

/*===========================================================================*/

int CloudEffect::SaveObject(FILE *ffile, const char *SuppliedFileName)
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

if (BuildFileComponentsList(&Coords)
	&& GeneralEffect::BuildFileComponentsList(&Queries, &Themes, &Coords))
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

// CloudEffect
strcpy(StrBuf, "Cloud");
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
			} // if CloudEffect saved 
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

} // CloudEffect::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::CloudEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
CloudEffect *Current;

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
						if (Current = new CloudEffect(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (CloudEffect *)FindDuplicateByName(Current->EffectType, Current))
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

} // EffectsLib::CloudEffect_Load()

/*===========================================================================*/

ULONG EffectsLib::CloudEffect_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
CloudEffect *Current;

Current = Cloud;
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
	Current = (CloudEffect *)Current->Next;
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

} // EffectsLib::CloudEffect_Save()

/*===========================================================================*/

CloudLayerData::CloudLayerData()
{

RGB[2] = RGB[1] = RGB[0] = Dist = Elev = Shading = Density = Coverage = 0.0;
Cloud = NULL;
LayerNum = 0;
Above = 0;

} // CloudLayerData::CloudLayerData
