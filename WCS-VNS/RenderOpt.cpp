// RenderOpt.cpp
// For managing WCS PlanetOpts
// Built from scratch on 03/24/99 by Gary R. Huber
// Copyright 1999 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Project.h"
#include "Interactive.h"
#include "Application.h"
#include "Conservatory.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "GraphData.h"
#include "requester.h"
#include "RenderOptEditGUI.h"
#include "PostProcEditGUI.h"
#include "ImageOutputEvent.h"
#include "Toolbar.h"
#include "AppMem.h"
#include "PixelManager.h"
#include "Lists.h"

/*===========================================================================*/

RenderOpt::RenderOpt()
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_RENDEROPT;
SetDefaults();

} // RenderOpt::RenderOpt

/*===========================================================================*/

RenderOpt::RenderOpt(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_RENDEROPT;
SetDefaults();

} // RenderOpt::RenderOpt

/*===========================================================================*/

RenderOpt::RenderOpt(RasterAnimHost *RAHost, EffectsLib *Library, RenderOpt *Proto)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_RENDEROPT;
Prev = Library->LastRenderOpt;
if (Library->LastRenderOpt)
	{
	Library->LastRenderOpt->Next = this;
	Library->LastRenderOpt = this;
	} // if
else
	{
	Library->RenderOpts = Library->LastRenderOpt = this;
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
	strcpy(NameBase, "Render Options");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // RenderOpt::RenderOpt

/*===========================================================================*/

RenderOpt::~RenderOpt()
{
ImageOutputEvent *CurrentEvent;
EffectList *NextProc;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->ROG && GlobalApp->GUIWins->ROG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->ROG;
		GlobalApp->GUIWins->ROG = NULL;
		} // if
	} // if

while (OutputEvents)
	{
	CurrentEvent = OutputEvents;
	OutputEvents = OutputEvents->Next;
	delete CurrentEvent;
	} // while

while (Post)
	{
	NextProc = Post;
	Post = Post->Next;
	delete NextProc;
	} // while

} // RenderOpt::~RenderOpt

/*===========================================================================*/

void RenderOpt::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_RENDEROPT_NUMANIMPAR] = {0.0, 0.0, 30.0, 1.0, .05, .1, 10.0};
double RangeDefaults[WCS_EFFECTS_RENDEROPT_NUMANIMPAR][3] = {
												FLT_MAX, 0.0, .01,		// start time
												#ifdef WCS_BUILD_DEMO
												5.0, 0.0, .01,		// end time
												#else // WCS_BUILD_DEMO
												FLT_MAX, 0.0, .01,		// end time
												#endif // WCS_BUILD_DEMO
												FLT_MAX, 1.0, 1.0,		// frame rate
												100.0, 0.00001, .01,	// pixel aspect
												10.0, 0.0, .01,			// side overscan
												10.0, 0.0, .01,			// bottom overscan
												100000.0, 0.0, 1.0		// vector offset
												};
double FrameRate;
long Ct;

for (Ct = 0; Ct < WCS_MAXIMPLEMENTED_EFFECTS; ++Ct)
	{
	EffectEnabled[Ct] = 1;
	} // for

RenderFromOverhead = LuminousVectors = 0;
ReflectionsEnabled = CloudShadowsEnabled = ObjectShadowsEnabled = TerrainEnabled =
	VectorsEnabled = FoliageEnabled = HazeVectors = MultiPassAAEnabled = DepthOfFieldEnabled = TransparentWaterEnabled = 
	FragmentRenderingEnabled = VolumetricsEnabled = 1;
RenderDiagnosticData = 0;
RenderImageSegments = 1;
OutputImageWidth = 640;
OutputImageHeight = 480;
RenderOffsetX = RenderOffsetY = SetupOffsetX = SetupOffsetY = CamSetupRenderWidth = CamSetupRenderHeight = 0;
FrameStep = 1;
FragmentDepth = 40;
OutputEvents = NULL;
Post = NULL;
ReflectionType = WCS_REFLECTIONSTYLE_BTFRAGS;
TilingEnabled = 0;
ConcatenateTiles = 1;
TilesX = TilesY = 2;
FoliageShadowsOnly = 0;

if (GlobalApp && GlobalApp->MainProj && GlobalApp->MainProj->Interactive)
	{
	if ((FrameRate = GlobalApp->MainProj->Interactive->GetFrameRate()) <= 0.0)
		FrameRate = 30.0;
	} // if
else
	FrameRate = 30.0;

EffectDefault[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE] = FrameRate;
RangeDefaults[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME][2] = 1.0 / FrameRate;
RangeDefaults[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME][2] = 1.0 / FrameRate;

for (Ct = 0; Ct < GetNumAnimParams(); ++Ct)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for

LockAspect = 1;
ImageAspectRatio = (double)OutputImageWidth / OutputImageHeight;

TempPath.SetPath("WCSFrames:");

AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_VECTOROFFSET].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].SetMetricType(WCS_ANIMDOUBLE_METRIC_TIME);
AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].SetMetricType(WCS_ANIMDOUBLE_METRIC_TIME);
AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_SIDEOVERSCAN].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_BOTTOMOVERSCAN].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].SetNoNodes(1);
AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].SetNoNodes(1);
AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].SetNoNodes(1);
AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_SIDEOVERSCAN].SetNoNodes(1);
AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_BOTTOMOVERSCAN].SetNoNodes(1);
AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].SetNoNodes(1);
AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_VECTOROFFSET].SetNoNodes(1);

} // RenderOpt::SetDefaults

/*===========================================================================*/

void RenderOpt::Copy(RenderOpt *CopyTo, RenderOpt *CopyFrom)
{
long Ct, Result = -1;
EffectList *NextProc, **ToProc;
NotifyTag Changes[2];
ImageOutputEvent *CurrentFrom = CopyFrom->OutputEvents, **ToPtr, *NextEvent;

while (CopyTo->Post)
	{
	NextProc = CopyTo->Post;
	CopyTo->Post = CopyTo->Post->Next;
	delete NextProc;
	} // if
NextProc = CopyFrom->Post;
ToProc = &CopyTo->Post;
while (NextProc)
	{
	if (NextProc->Me)
		{
		if (*ToProc = new EffectList())
			{
			if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->AppEffects)
				{
				(*ToProc)->Me = GlobalApp->AppEffects->MatchNameMakeEffect(NextProc->Me);
				} // if no need to make another copy, its all in the family
			else
				{
				if (Result < 0 && GlobalApp->AppEffects->FindByName(NextProc->Me->EffectType, NextProc->Me->Name))
					{
					Result = UserMessageCustom("Copy Render Options", "How do you wish to resolve Post Process name collisions?\n\nLink to existing Post Processes, replace existing Post Processes, or create new Post Processes?",
						"Link", "Create", "Overwrite", 1);
					} // if
				if (Result <= 0)
					{
					(*ToProc)->Me = GlobalApp->AppEffects->AddEffect(NextProc->Me->EffectType, NULL, NextProc->Me);
					} // if create new
				else if (Result == 1)
					{
					(*ToProc)->Me = GlobalApp->AppEffects->MatchNameMakeEffect(NextProc->Me);
					} // if link to existing
				else if ((*ToProc)->Me = GlobalApp->AppEffects->FindByName(NextProc->Me->EffectType, NextProc->Me->Name))
					{
					((PostProcess *)(*ToProc)->Me)->Copy((PostProcess *)(*ToProc)->Me, (PostProcess *)NextProc->Me);
					Changes[0] = MAKE_ID((*ToProc)->Me->GetNotifyClass(), (*ToProc)->Me->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, (*ToProc)->Me);
					} // else if found and overwrite
				else
					{
					(*ToProc)->Me = GlobalApp->AppEffects->AddEffect(NextProc->Me->EffectType, NULL, NextProc->Me);
					} // else
				} // else better copy or overwrite it since its important to get just the right ecosystem
			if ((*ToProc)->Me)
				ToProc = &(*ToProc)->Next;
			else
				{
				delete *ToProc;
				*ToProc = NULL;
				} // if
			} // if
		} // if
	NextProc = NextProc->Next;
	} // while

for (Ct = 0; Ct < WCS_MAXIMPLEMENTED_EFFECTS; ++Ct)
	{
	CopyTo->EffectEnabled[Ct] = CopyFrom->EffectEnabled[Ct];
	} // for

// delete existing component sources
while (CopyTo->OutputEvents)
	{
	NextEvent = CopyTo->OutputEvents;
	CopyTo->OutputEvents = CopyTo->OutputEvents->Next;
	delete NextEvent;
	} // while

ToPtr = &CopyTo->OutputEvents;

while (CurrentFrom)
	{
	if (*ToPtr = new ImageOutputEvent())
		{
		(*ToPtr)->Copy(*ToPtr, CurrentFrom);
		} // if
	ToPtr = &(*ToPtr)->Next;
	CurrentFrom = CurrentFrom->Next;
	} // while

CopyTo->LuminousVectors = CopyFrom->LuminousVectors;
CopyTo->RenderFromOverhead = CopyFrom->RenderFromOverhead;
CopyTo->ReflectionsEnabled = CopyFrom->ReflectionsEnabled;
CopyTo->TransparentWaterEnabled = CopyFrom->TransparentWaterEnabled;
CopyTo->FragmentRenderingEnabled = CopyFrom->FragmentRenderingEnabled;
CopyTo->ReflectionType = CopyFrom->ReflectionType;
CopyTo->CloudShadowsEnabled = CopyFrom->CloudShadowsEnabled;
CopyTo->ObjectShadowsEnabled = CopyFrom->ObjectShadowsEnabled;
CopyTo->TerrainEnabled = CopyFrom->TerrainEnabled;
CopyTo->RenderDiagnosticData = CopyFrom->RenderDiagnosticData;
CopyTo->MultiPassAAEnabled = CopyFrom->MultiPassAAEnabled;
CopyTo->DepthOfFieldEnabled = CopyFrom->DepthOfFieldEnabled;
CopyTo->VectorsEnabled = CopyFrom->VectorsEnabled;
CopyTo->FoliageEnabled = CopyFrom->FoliageEnabled;
CopyTo->VolumetricsEnabled = CopyFrom->VolumetricsEnabled;
CopyTo->TilingEnabled = CopyFrom->TilingEnabled;
CopyTo->ConcatenateTiles = CopyFrom->ConcatenateTiles;
CopyTo->FoliageShadowsOnly = CopyFrom->FoliageShadowsOnly;
CopyTo->TilesX = CopyFrom->TilesX;
CopyTo->TilesY = CopyFrom->TilesY;
CopyTo->HazeVectors = CopyFrom->HazeVectors;
CopyTo->RenderImageSegments = CopyFrom->RenderImageSegments;
CopyTo->OutputImageWidth = CopyFrom->OutputImageWidth;
CopyTo->OutputImageHeight = CopyFrom->OutputImageHeight;
CopyTo->LockAspect = CopyFrom->LockAspect;
CopyTo->ImageAspectRatio = CopyFrom->ImageAspectRatio;
CopyTo->FrameStep = CopyFrom->FrameStep;
CopyTo->FragmentDepth = CopyFrom->FragmentDepth;
TempPath.Copy(&CopyTo->TempPath, &CopyFrom->TempPath);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // RenderOpt::Copy

/*===========================================================================*/

void RenderOpt::CopyEnabledSettings(RenderOpt *CopyTo, RenderOpt *CopyFrom)
{
long Ct;

for (Ct = 0; Ct < WCS_MAXIMPLEMENTED_EFFECTS; ++Ct)
	{
	CopyTo->EffectEnabled[Ct] = CopyFrom->EffectEnabled[Ct];
	} // for

CopyTo->ReflectionsEnabled = CopyFrom->ReflectionsEnabled;
CopyTo->TransparentWaterEnabled = CopyFrom->TransparentWaterEnabled;
CopyTo->FragmentRenderingEnabled = CopyFrom->FragmentRenderingEnabled;
CopyTo->CloudShadowsEnabled = CopyFrom->CloudShadowsEnabled;
CopyTo->ObjectShadowsEnabled = CopyFrom->ObjectShadowsEnabled;
CopyTo->TerrainEnabled = CopyFrom->TerrainEnabled;
CopyTo->RenderDiagnosticData = CopyFrom->RenderDiagnosticData;
CopyTo->MultiPassAAEnabled = CopyFrom->MultiPassAAEnabled;
CopyTo->DepthOfFieldEnabled = CopyFrom->DepthOfFieldEnabled;
CopyTo->VectorsEnabled = CopyFrom->VectorsEnabled;
CopyTo->FoliageEnabled = CopyFrom->FoliageEnabled;
CopyTo->VolumetricsEnabled = CopyFrom->VolumetricsEnabled;
CopyTo->HazeVectors = CopyFrom->HazeVectors;

} // RenderOpt::CopyEnabledSettings

/*===========================================================================*/

ULONG RenderOpt::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
short NumClasses = 0, MaxClasses = WCS_MAXIMPLEMENTED_EFFECTS, ClassesToRead;
char MatchName[WCS_EFFECT_MAXNAMELENGTH];
ImageOutputEvent **LoadTo = &OutputEvents;
EffectList **CurProc = &Post;

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
					case WCS_EFFECTS_RENDEROPT_NUMCLASSES:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumClasses, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_EFFECTENABLED:
						{
						ClassesToRead = NumClasses >= MaxClasses ? MaxClasses: NumClasses;
						BytesRead = ReadBlock(ffile, (char *)&EffectEnabled[0], WCS_BLOCKTYPE_CHAR + ClassesToRead, ByteFlip);
						ClassesToRead = NumClasses > MaxClasses ? NumClasses - MaxClasses: 0;
						if (ClassesToRead && ! fseek(ffile, ClassesToRead, SEEK_CUR))
							BytesRead += ClassesToRead;
						break;
						}
					case WCS_EFFECTS_RENDEROPT_REFLECTIONSENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReflectionsEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_TRANSPARWATERENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&TransparentWaterEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_FRAGRENDERENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&FragmentRenderingEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_REFLECTIONTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReflectionType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (ReflectionType < WCS_REFLECTIONSTYLE_BTFRAGS)
							ReflectionType = WCS_REFLECTIONSTYLE_BEAMTRACE;
						else
							ReflectionType = WCS_REFLECTIONSTYLE_BTFRAGS;
						break;
						}
					case WCS_EFFECTS_RENDEROPT_CLOUDSHADOWSENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&CloudShadowsEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_OBJECTSHADOWSENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&ObjectShadowsEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_TERRAINENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&TerrainEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_RENDERDIAGNOSTICDATA:
						{
						BytesRead = ReadBlock(ffile, (char *)&RenderDiagnosticData, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_MULTIPASSAAENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&MultiPassAAEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_DEPTHOFFIELDENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&DepthOfFieldEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_VECTORSENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&VectorsEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_FOLIAGEENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&FoliageEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_VOLUMETRICSENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&VolumetricsEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					#ifdef WCS_RENDER_TILES
					case WCS_EFFECTS_RENDEROPT_TILINGENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&TilingEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_CONCATENATETILES:
						{
						BytesRead = ReadBlock(ffile, (char *)&ConcatenateTiles, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					#endif // WCS_RENDER_TILES
					case WCS_EFFECTS_RENDEROPT_HAZEVECTORS:
						{
						BytesRead = ReadBlock(ffile, (char *)&HazeVectors, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_RENDERFROMOVERHEAD:
						{
						BytesRead = ReadBlock(ffile, (char *)&RenderFromOverhead, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_LUMINOUSVECTORS:
						{
						BytesRead = ReadBlock(ffile, (char *)&LuminousVectors, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_FOLIAGESHADOWSONLY:
						{
						BytesRead = ReadBlock(ffile, (char *)&FoliageShadowsOnly, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_RENDERIMAGESEGMENTS:
						{
						BytesRead = ReadBlock(ffile, (char *)&RenderImageSegments, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					#ifdef WCS_RENDER_TILES
					case WCS_EFFECTS_RENDEROPT_TILESX:
						{
						BytesRead = ReadBlock(ffile, (char *)&TilesX, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_TILESY:
						{
						BytesRead = ReadBlock(ffile, (char *)&TilesY, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					#endif // WCS_RENDER_TILES
					case WCS_EFFECTS_RENDEROPT_OUTPUTIMAGEWIDTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&OutputImageWidth, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						#ifdef WCS_BUILD_DEMO
						if (OutputImageWidth > 640)
							OutputImageWidth = 640;
						#endif // WCS_BUILD_DEMO
						break;
						}
					case WCS_EFFECTS_RENDEROPT_OUTPUTIMAGEHEIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)&OutputImageHeight, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						#ifdef WCS_BUILD_DEMO
						if (OutputImageHeight > 450)
							OutputImageHeight = 450;
						#endif // WCS_BUILD_DEMO
						break;
						}
					case WCS_EFFECTS_RENDEROPT_FRAMESTEP:
						{
						BytesRead = ReadBlock(ffile, (char *)&FrameStep, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_FRAGMENTDEPTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&FragmentDepth, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_LOCKASPECT:
						{
						BytesRead = ReadBlock(ffile, (char *)&LockAspect, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_ASPECTRATIO:
						{
						BytesRead = ReadBlock(ffile, (char *)&ImageAspectRatio, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_SIDEOVERSCAN:
						{
						BytesRead = AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_SIDEOVERSCAN].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_BOTTOMOVERSCAN:
						{
						BytesRead = AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_BOTTOMOVERSCAN].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_VECTORZOFFSET:
						{
						BytesRead = AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_VECTOROFFSET].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_FIRSTTIME:
						{
						BytesRead = AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_LASTTIME:
						{
						BytesRead = AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].Load(ffile, Size, ByteFlip);
						#ifdef WCS_BUILD_DEMO
						if (AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].CurValue > 5.0)
							AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].SetValue(5.0);
						#endif // WCS_BUILD_DEMO
						break;
						}
					case WCS_EFFECTS_RENDEROPT_FRAMERATE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_PIXELASPECT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_TEMPPATH:
						{
						BytesRead = TempPath.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROPT_OUTPUTEVENT:
						{
						if (*LoadTo = new ImageOutputEvent())
							{
							BytesRead = (*LoadTo)->Load(ffile, Size, ByteFlip);
							LoadTo = &(*LoadTo)->Next;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_RENDEROPT_POSTPROCNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							if (*CurProc = new EffectList())
								{
								if ((*CurProc)->Me = GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_POSTPROC, MatchName))
									CurProc = &(*CurProc)->Next;
								else
									{
									delete *CurProc;
									*CurProc = NULL;
									} // else
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

} // RenderOpt::Load

/*===========================================================================*/

unsigned long int RenderOpt::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
short NumClasses = WCS_MAXIMPLEMENTED_EFFECTS;
unsigned long int AnimItemTag[WCS_EFFECTS_RENDEROPT_NUMANIMPAR] = {WCS_EFFECTS_RENDEROPT_FIRSTTIME,
																 WCS_EFFECTS_RENDEROPT_LASTTIME,
																 WCS_EFFECTS_RENDEROPT_FRAMERATE,
																 WCS_EFFECTS_RENDEROPT_PIXELASPECT,
																 WCS_EFFECTS_RENDEROPT_SIDEOVERSCAN,
																 WCS_EFFECTS_RENDEROPT_BOTTOMOVERSCAN,
																 WCS_EFFECTS_RENDEROPT_VECTORZOFFSET
																 };
ImageOutputEvent *Current;
EffectList *CurProc;

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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_NUMCLASSES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&NumClasses)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_EFFECTENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_MAXIMPLEMENTED_EFFECTS,
	WCS_BLOCKTYPE_CHAR, (char *)&EffectEnabled[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_REFLECTIONSENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReflectionsEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_TRANSPARWATERENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TransparentWaterEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_FRAGRENDERENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FragmentRenderingEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_REFLECTIONTYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReflectionType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_CLOUDSHADOWSENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&CloudShadowsEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_OBJECTSHADOWSENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ObjectShadowsEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_TERRAINENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TerrainEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_RENDERDIAGNOSTICDATA, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RenderDiagnosticData)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_MULTIPASSAAENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&MultiPassAAEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_DEPTHOFFIELDENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DepthOfFieldEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_VECTORSENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VectorsEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_FOLIAGEENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FoliageEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_VOLUMETRICSENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VolumetricsEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_TILINGENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TilingEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_CONCATENATETILES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ConcatenateTiles)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_HAZEVECTORS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HazeVectors)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_RENDERFROMOVERHEAD, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RenderFromOverhead)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_LUMINOUSVECTORS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LuminousVectors)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_FOLIAGESHADOWSONLY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FoliageShadowsOnly)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_RENDERIMAGESEGMENTS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&RenderImageSegments)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_TILESX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&TilesX)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_TILESY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&TilesY)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_OUTPUTIMAGEWIDTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&OutputImageWidth)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_OUTPUTIMAGEHEIGHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&OutputImageHeight)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_FRAMESTEP, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&FrameStep)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_FRAGMENTDEPTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&FragmentDepth)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_LOCKASPECT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LockAspect)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_ASPECTRATIO, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ImageAspectRatio)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < GetNumAnimParams(); ++Ct)
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

ItemTag = WCS_EFFECTS_RENDEROPT_TEMPPATH + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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
			} // if TempPath saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

Current = OutputEvents;
while (Current)
	{
	ItemTag = WCS_EFFECTS_RENDEROPT_OUTPUTEVENT + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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
				} // if output event saved 
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

CurProc = Post;
while (CurProc)
	{
	if (CurProc->Me)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROPT_POSTPROCNAME, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(CurProc->Me->GetName()) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)CurProc->Me->GetName())) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	CurProc = CurProc->Next;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // RenderOpt::Save

/*===========================================================================*/

char *RenderOptCritterNames[WCS_EFFECTS_RENDEROPT_NUMANIMPAR] = {"Start Time", "End Time", 
	"Frame Rate (fr/sec)", "Pixel Aspect", "Side Overscan (%)", "Bottom Overscan (%)", "Vector Z Offset (m)"};

char *RenderOpt::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); ++Ct)
	{
	if (Test == GetAnimPtr(Ct))
		return (RenderOptCritterNames[Ct]);
	} // for
return ("");

} // RenderOpt::GetCritterName

/*===========================================================================*/

void RenderOpt::Edit(void)
{

if(GlobalApp->GUIWins->ROG)
	{
	delete GlobalApp->GUIWins->ROG;
	}
GlobalApp->GUIWins->ROG = new RenderOptEditGUI(GlobalApp->AppEffects, this);
if(GlobalApp->GUIWins->ROG)
	{
	GlobalApp->GUIWins->ROG->Open(GlobalApp->MainProj);
	}

} // RenderOpt::Edit

/*===========================================================================*/

RasterAnimHost *RenderOpt::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;
EffectList *CurProc;

if (! Current)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); ++Ct)
	{
	if (Found)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for
for (Ct = 0; Ct < GetNumTextures(); ++Ct)
	{
	if (Found && GetTexRootPtr(Ct))
		return (GetTexRootPtr(Ct));
	if (Current == GetTexRootPtr(Ct))
		Found = 1;
	} // for
CurProc = Post;
while (CurProc)
	{
	if (Found)
		return (CurProc->Me);
	if (Current == CurProc->Me)
		Found = 1;
	CurProc = CurProc->Next;
	} // while

return (NULL);

} // RenderOpt::GetRAHostChild

/*===========================================================================*/

ImageOutputEvent *RenderOpt::AddOutputEvent(void)
{
ImageOutputEvent *CurEvent = OutputEvents, *TempEvent;
char *TextStr, BaseName[256];
long BufCt, Found, Iteration, BaseNameLen;
NotifyTag Changes[2];

if (CurEvent)
	{
	while (CurEvent->Next)
		CurEvent = CurEvent->Next;
	CurEvent->Next = new ImageOutputEvent();
	CurEvent = CurEvent->Next;
	} // if
else
	OutputEvents = CurEvent = new ImageOutputEvent();

if (CurEvent)
	{
	// configure the new event
	// we're making PNG our new default format for the future
#ifdef WCS_BUILD_PNG_SUPPORT
	strcpy(CurEvent->FileType, "PNG");
#else // fall back to IFF
	if (TextStr = ImageSaverLibrary::GetNextFileFormat(NULL))
		strcpy(CurEvent->FileType, TextStr);
#endif // WCS_BUILD_PNG_SUPPORT
	if (TextStr = ImageSaverLibrary::GetNextCodec(CurEvent->FileType, NULL))
		strcpy(CurEvent->Codec, TextStr);
	TextStr = NULL;
	BufCt = 0;
	while (TextStr = ImageSaverLibrary::GetNextDefaultBuffer(CurEvent->FileType, TextStr))
		strcpy(CurEvent->OutBuffers[BufCt ++], TextStr);
	CurEvent->PAF.SetPath("WCSFrames:");

	Found = 1;
	Iteration = 0;
	strcpy(BaseName, GlobalApp->MainProj->projectname);
	StripExtension(BaseName);
	BaseNameLen = (long)strlen(BaseName);
	while (Found && Iteration < 27)
		{
		Found = 0;
		for (TempEvent = OutputEvents; TempEvent; TempEvent = TempEvent->Next)
			{
			if (TempEvent == CurEvent)
				continue;
			if (! stricmp(TempEvent->PAF.GetName(), BaseName))
				{
				Found = 1;
				break;
				} // if found match
			} // for
		if (Found)
			{
			BaseName[BaseNameLen] = (char)(Iteration + 65);
			BaseName[BaseNameLen + 1] = 0;
			Iteration ++;
			}
		} // while
	CurEvent->PAF.SetName(BaseName);

	// notify the world
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

return (CurEvent);

} // RenderOpt::AddOutputEvent

/*===========================================================================*/

int RenderOpt::RemoveOutputEvent(ImageOutputEvent *RemoveMe)
{
ImageOutputEvent *CurEvent = OutputEvents, *LastEvent = NULL;
int Removed = 0;
NotifyTag Changes[2];

while (CurEvent)
	{
	if (CurEvent == RemoveMe)
		{
		if (LastEvent)
			{
			LastEvent->Next = CurEvent->Next;
			} // if
		else
			{
			OutputEvents = CurEvent->Next;
			} // else
		delete CurEvent;
		Removed = 1;
		break;
		} // if
	LastEvent = CurEvent;
	CurEvent = CurEvent->Next;
	} // while

if (Removed)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
return (Removed);

} // RenderOpt::RemoveOutputEvent

/*===========================================================================*/

EffectList *RenderOpt::AddPostProc(GeneralEffect *AddMe)
{
EffectList **CurProc = &Post;
NotifyTag Changes[2];

if (AddMe)
	{
	while (*CurProc)
		{
		if ((*CurProc)->Me == AddMe)
			return (NULL);
		CurProc = &(*CurProc)->Next;
		} // while
	if (*CurProc = new EffectList())
		{
		(*CurProc)->Me = AddMe;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	return (*CurProc);
	} // if

return (NULL);

} // RenderOpt::AddPostProc

/*===========================================================================*/

int RenderOpt::ValidateOutputPaths(void)
{
ImageOutputEvent *CurEvent = OutputEvents;

while (CurEvent)
	{
	if (CurEvent->Enabled)
		{
		if (! CurEvent->PAF.ValidatePath())
			return (0);
		} // if
	CurEvent = CurEvent->Next;
	} // while

return (1);

} // RenderOpt::ValidateOutputPaths

/*===========================================================================*/

long RenderOpt::InitImageIDs(long &ImageID)
{
long NumImages = 0;
EffectList *CurProc = Post;

while (CurProc)
	{
	if (CurProc->Me)
		NumImages += CurProc->Me->InitImageIDs(ImageID);
	CurProc = CurProc->Next;
	} // while
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // RenderOpt::InitImageIDs

/*===========================================================================*/

int RenderOpt::BuildFileComponentsList(EffectList **PostProc, EffectList **Coords)
{
EffectList **ListPtr, *CurProc = Post, *Queries = NULL, *Themes = NULL;

while (CurProc)
	{
	if (CurProc->Me)
		{
		ListPtr = PostProc;
		while (*ListPtr)
			{
			if ((*ListPtr)->Me == CurProc->Me)
				break;
			ListPtr = &(*ListPtr)->Next;
			} // if
		if (! (*ListPtr))
			{
			if (*ListPtr = new EffectList())
				(*ListPtr)->Me = CurProc->Me;
			else
				return (0);
			} // if
		if (! CurProc->Me->BuildFileComponentsList(&Queries, &Themes, Coords))	// calls on GeneralEffect
			return (0);
		} // if
	CurProc = CurProc->Next;
	} // while

return (1);

} // RenderOpt::BuildFileComponentsList

/*===========================================================================*/

int RenderOpt::RemoveRAHost(RasterAnimHost *RemoveMe)
{
EffectList *CurProc = Post, *PrevProc = NULL;
NotifyTag Changes[2];

while (CurProc)
	{
	if (CurProc->Me == (GeneralEffect *)RemoveMe)
		{
		if (PrevProc)
			PrevProc->Next = CurProc->Next;
		else
			Post = CurProc->Next;

		delete CurProc;

		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

		return (1);
		} // if
	PrevProc = CurProc;
	CurProc = CurProc->Next;
	} // while

return (GeneralEffect::RemoveRAHost(RemoveMe));

} // RenderOpt::RemoveRAHost

/*===========================================================================*/

char RenderOpt::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_EFFECTSSUBCLASS_POSTPROC)
	return (1);

return (0);

} // RenderOpt::GetRAHostDropOK

/*===========================================================================*/

int RenderOpt::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
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
	if (this != (RenderOpt *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (RenderOpt *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_POSTPROC)
	{
	Success = -1;
	sprintf(QueryStr, "Add %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		if (AddPostProc((GeneralEffect *)DropSource->DropSource))
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

} // RenderOpt::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long RenderOpt::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_RENDOPT | WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_CHILDREN |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // RenderOpt::GetRAFlags

/*===========================================================================*/

int RenderOpt::OutputImages(void)
{
ImageOutputEvent *CurEvent;

if (OutputEvents)
	{
	if ((CurEvent = OutputEvents->SaveBufferQuery("RED")) && CurEvent->Enabled)
		return (1);
	if ((CurEvent = OutputEvents->SaveBufferQuery("GREEN")) && CurEvent->Enabled)
		return (1);
	if ((CurEvent = OutputEvents->SaveBufferQuery("BLUE")) && CurEvent->Enabled)
		return (1);
	} // if

return (0);

} // RenderOpt::OutputImages

/*===========================================================================*/

int RenderOpt::OutputZBuffer(void)
{
ImageOutputEvent *CurEvent;

if (OutputEvents)
	{
	if ((CurEvent = OutputEvents->SaveBufferQuery("ZBUF")) && CurEvent->Enabled)
		return (1);
	} // if

return (0);

} // RenderOpt::OutputZBuffer

/*===========================================================================*/

const char *RenderOpt::GetFirstOutputName(char &MoreOutputEvents)
{
ImageOutputEvent *CurEvent;

MoreOutputEvents = 0;

if (OutputEvents)
	{
	if ((CurEvent = OutputEvents->SaveBufferQuery("RED")) ||
		(CurEvent = OutputEvents->SaveBufferQuery("GREEN")) ||
		(CurEvent = OutputEvents->SaveBufferQuery("BLUE")))
		{
		if (CurEvent->Next)
			MoreOutputEvents = 1;
		return (CurEvent->PAF.GetName());
		} // if
	} // if

return ("");

} // RenderOpt::GetFirstOutputName

/*===========================================================================*/

int RenderOpt::OutputDiagnosticData(void)
{
ImageOutputEvent *CurEvent;

if (OutputEvents)
	{
	if ((CurEvent = OutputEvents->SaveBufferQuery("LATITUDE")) && CurEvent->Enabled)
		return (1);
	if ((CurEvent = OutputEvents->SaveBufferQuery("LONGITUDE")) && CurEvent->Enabled)
		return (1);
	if ((CurEvent = OutputEvents->SaveBufferQuery("ELEVATION")) && CurEvent->Enabled)
		return (1);
	if ((CurEvent = OutputEvents->SaveBufferQuery("ILLUMINATION")) && CurEvent->Enabled)
		return (1);
	if ((CurEvent = OutputEvents->SaveBufferQuery("SLOPE")) && CurEvent->Enabled)
		return (1);
	if ((CurEvent = OutputEvents->SaveBufferQuery("ASPECT")) && CurEvent->Enabled)
		return (1);
	if ((CurEvent = OutputEvents->SaveBufferQuery("OBJECT")) && CurEvent->Enabled)
		return (1);
	} // if

return (0);

} // RenderOpt::OutputDiagnosticData

/*===========================================================================*/

int RenderOpt::InitToRender(BufferNode *Buffers, long Width, long Height, int InitSequence)
{
ImageOutputEvent *CurEvent = OutputEvents;
EffectList *CurPost = Post;
long Ct;

if (Buffers)
	{
	while (CurEvent)
		{
		if(CurEvent->Enabled)
			{
			for (Ct = 0; Ct < WCS_MAX_IMAGEOUTBUFFERS; ++Ct)
				{
				if (CurEvent->OutBuffers[Ct][0])
					{
					// this will warn user if attempt to allocate an unrecognized buffer and renderer will bail
					if (! Buffers->AddBufferNode(CurEvent->OutBuffers[Ct], -1))
						return (0);
					} // if
				} // for
			// Buffers is NULL during FDM creation, and non-null during actual rendering
			if(Buffers)
				{
				// <<<>>>
				// the first arg here should really be a pointer to the renderer,
				// but I can't seem to get access to it from here. I don't need it
				// right now, but it seems like it could be useful in the future
				CurEvent->SaveOpts = this;
				if (InitSequence)
					CurEvent->InitSequence(NULL, Buffers, CurEvent->SaveOpts->OutputImageWidth, CurEvent->SaveOpts->OutputImageHeight);
				} // if
			} // if
		CurEvent = CurEvent->Next;
		} // while
	while (CurPost)
		{
		if (CurPost->Me)
			((PostProcess *)CurPost->Me)->AddRenderBuffers(this, Buffers);
		CurPost = CurPost->Next;
		} // while
	} // if

return (1);

} // RenderOpt::InitToRender

/*===========================================================================*/

int RenderOpt::CleanupFromRender(int EndSequence)
{
ImageOutputEvent *CurEvent = OutputEvents;

while (CurEvent)
	{
	if(CurEvent->Enabled)
		{
		if (EndSequence)
			CurEvent->EndSequence();
		} // if
	CurEvent = CurEvent->Next;
	} // while

return (1);
} // RenderOpt::CleanupFromRender

/*===========================================================================*/

int RenderOpt::SaveImage(Renderer *RHost, RasterBounds *RBounds, BufferNode *Buffers, long Width, long Height, long Frame, long StereoSide, long PanoPanel, long Field, 
	long AAPass, long Segment, long XTile, long YTile, long PanoPanels, long Fields, long AAPasses, long ImageSegments, long XTiles, long YTiles, long FieldDominance, long BeforePost)
{
double StashCenterX, StashCenterY;
BufferNode *CurBuf;
ImageOutputEvent *CurEvent;
int Success = 1, StuffToDo = 0, NameChanged = 0, CenterXYModified = 0;
long SaveWidth, SaveHeight;
char TempName[256], TempName2[256];

if (OutputEvents)
	{
	for (CurEvent = OutputEvents; CurEvent; CurEvent = CurEvent->Next)
		{
		if (CurEvent->Enabled && CurEvent->BeforePost == BeforePost)
			{
			StuffToDo = 1;
			} // if
		} // for
	if (StuffToDo)
		{
		if (ImageSegments > 1)
			{
			// save or append Segment file
			for (CurBuf = Buffers; Success && CurBuf; CurBuf = CurBuf->Next)
				{
				if (CurBuf->Buffer && OutputEvents->SaveBufferQuery(CurBuf->Name))
					{
					Success = SaveSegmentTempFile(CurBuf, Width, Height, Frame, Segment, BeforePost);
					} // if
				} // for
			} // if
		if (Success && Segment == ImageSegments - 1)
			{
			// last segment
			if (AAPasses > 1)
				{
				// save or update AAPass file
				for (CurBuf = Buffers; Success && CurBuf; CurBuf = CurBuf->Next)
					{
					if (CurBuf->Buffer && OutputEvents->SaveBufferQuery(CurBuf->Name))
						{
						Success = SaveAAPassTempFile(Buffers, CurBuf, Width, Height * ImageSegments, Frame, AAPass, AAPasses, ImageSegments, BeforePost);
						} // if
					} // for
				} // if
			if (Success && AAPass == AAPasses - 1)
				{
				// last AAPass
				if (Fields > 1)
					{
					// save or update Field file
					for (CurBuf = Buffers; Success && CurBuf; CurBuf = CurBuf->Next)
						{
						if (CurBuf->Buffer && OutputEvents->SaveBufferQuery(CurBuf->Name))
							{
							Success = SaveFieldTempFile(CurBuf, Width, Height * ImageSegments, Frame, Field, FieldDominance, AAPasses, ImageSegments, BeforePost);
							} // if
						} // for
					} // if
				if (Success && Field == Fields - 1)
					{
					// last field
					if (PanoPanels > 1)
						{
						// save or update Panorama file
						for (CurBuf = Buffers; Success && CurBuf; CurBuf = CurBuf->Next)
							{
							if (CurBuf->Buffer && OutputEvents->SaveBufferQuery(CurBuf->Name))
								{
								Success = SavePanoramaTempFile(CurBuf, Width, Height * ImageSegments, Frame, PanoPanel, Fields, AAPasses, ImageSegments, BeforePost);
								} // if
							} // for
						} // if
					if (Success && PanoPanel == PanoPanels - 1)
						{
						// last panorama panel
						#if (defined(WCS_RENDER_TILES) || defined(WCS_BUILD_RTX))
						if ((XTiles > 1 || YTiles > 1) && ConcatenateTiles)
							{
							// save or update tile file
							for (CurBuf = Buffers; Success && CurBuf; CurBuf = CurBuf->Next)
								{
								if (CurBuf->Buffer && OutputEvents->SaveBufferQuery(CurBuf->Name))
									{
									Success = SaveTileTempFile(CurBuf, Width, Height, Frame, XTile, YTile, XTiles, BeforePost);
									} // if
								} // for
							} // if

						if (Success && ((XTile == XTiles - 1 && YTile == YTiles -1) || ! ConcatenateTiles))
						#else // (defined(WCS_RENDER_TILES) || defined(WCS_BUILD_RTX))
						if (Success)
						#endif // (defined(WCS_RENDER_TILES) || defined(WCS_BUILD_RTX))
							{
							#if (defined(WCS_RENDER_TILES) || defined(WCS_BUILD_RTX))
							// last tile
							if (XTiles > 1 || YTiles > 1)
								{
								if (ConcatenateTiles)
									{
									SaveWidth = Width * XTiles;
									SaveHeight = Height * YTiles;
									// set these to full image size position so world file writers unproject upper left of image correctly
									StashCenterX = RHost->Cam->CenterX;
									StashCenterY = RHost->Cam->CenterY;
									RHost->Cam->CenterX = RHost->TrueCenterX;
									RHost->Cam->CenterY = RHost->TrueCenterY;
									CenterXYModified = 1;
									} // if
								else
									{
									SaveWidth = Width;
									SaveHeight = Height;
									} // else
								} // if tiles
							else
							#endif // (defined(WCS_RENDER_TILES) || defined(WCS_BUILD_RTX))
								{
								SaveWidth = Width * PanoPanels;
								SaveHeight = Height * ImageSegments;
								// set these to full image size position so world file writers unproject upper left of image correctly
								StashCenterX = RHost->Cam->CenterX;
								StashCenterY = RHost->Cam->CenterY;
								RHost->Cam->CenterX = RHost->TrueCenterX;
								RHost->Cam->CenterY = RHost->TrueCenterY;
								CenterXYModified = 1;
								} // else
							// Init/prep all buffers that will be saved
							for (CurBuf = Buffers; CurBuf; CurBuf = CurBuf->Next)
								{
								if (CurBuf->Buffer && OutputEvents->SaveBufferQuery(CurBuf->Name))
									{
									CurBuf->PrepToSave(this, Frame, SaveWidth, BeforePost);
									} // if
								} // for
							// Save them
							for (CurEvent = OutputEvents; CurEvent; CurEvent = CurEvent->Next)
								{
								if (CurEvent->Enabled && CurEvent->BeforePost == BeforePost)
									{
									if (StereoSide)
										{
										strcpy(TempName, CurEvent->PAF.GetName());
										strcpy(TempName2, TempName);
										strcat(TempName2, (StereoSide == WCS_CAMERA_STEREOCHANNEL_LEFT ? "_L_": "_R_"));
										CurEvent->PAF.SetName(TempName2);
										NameChanged = 1;
										} // if
									#if (defined(WCS_RENDER_TILES) || defined(WCS_BUILD_RTX))
									else if ((XTiles > 1 || YTiles > 1) && ! ConcatenateTiles)
										{
										strcpy(TempName, CurEvent->PAF.GetName());
										if (strlen(TempName) > 0 && TempName[strlen(TempName) - 1] == '.')
											sprintf(TempName2, "%s%dX_%dY.", TempName, XTile, YTile);
										else
											sprintf(TempName2, "%s_%dX_%dY", TempName, XTile, YTile);
										CurEvent->PAF.SetName(TempName2);
										NameChanged = 1;
										} // else if
									#endif // (defined(WCS_RENDER_TILES) || defined(WCS_BUILD_RTX))
									Success = CurEvent->SaveImage(RBounds, Buffers, SaveWidth, SaveHeight, Frame, this);
									if (NameChanged)
										{
										CurEvent->PAF.SetName(TempName);
										} // if
									} // if
								} // for
							// Cleanup vectors
							/* not done here anymore - see Render.cpp
							if (VectorsEnabled)
								{
								for (CurEvent = OutputEvents; CurEvent; CurEvent = CurEvent->Next)
									{
									if (CurEvent->Enabled && CurEvent->BeforePost == BeforePost)
										{
										CurEvent->EndVectorFrame();
										} // if
									} // for
								} // if
							*/
							// Cleanup all prep work
							for (CurBuf = Buffers; CurBuf; CurBuf = CurBuf->Next)
								{
								if (CurBuf->Buffer && OutputEvents->SaveBufferQuery(CurBuf->Name))
									{
									CurBuf->CleanupFromSave();
									} // if
								} // for
							if (CenterXYModified)
								{
								RHost->Cam->CenterX = StashCenterX;
								RHost->Cam->CenterY = StashCenterY;
								} // if
							} // if
						} // if
					} // if
				} // if
			} // if
		} // if
	} // if

return (Success);

} // RenderOpt::SaveImage

/*===========================================================================*/

int RenderOpt::SaveSegmentTempFile(BufferNode *Node, long Width, long Height, long Frame, long Segment, long BeforePost)
{
int Success = 1, BlockSize;
char *Mode;
FILE *ffile;

if (Segment == 0)
	Mode = "wb";
else
	Mode = "ab";

if (Node->Type == WCS_RASTER_BANDSET_BYTE)
	BlockSize = sizeof (unsigned char);
else if (Node->Type == WCS_RASTER_BANDSET_FLOAT)
	BlockSize = sizeof (float);
else if (Node->Type == WCS_RASTER_BANDSET_SHORT)
	BlockSize = sizeof (unsigned short);
else
	{
	UserMessageOK("Image Output", "Unable to save temporary file. Undefined variable size.");
	return (0);
	} // else

if (ffile = OpenTempFile(Mode, "Segment", Node->Name, Frame, BeforePost))
	{
	if (Segment == 0 || ! fseek(ffile, 0, SEEK_END))
		{
		if (fwrite(Node->Buffer, Width * Height * BlockSize, 1, ffile) != 1)
			Success = 0;
		} // if
	fclose(ffile);
	} // if

return (Success);

} // RenderOpt::SaveSegmentTempFile

/*===========================================================================*/

int RenderOpt::SaveAAPassTempFile(BufferNode *Buffers, BufferNode *Node, long Width, long Height, long Frame, long AAPass, long AAPasses, long ImageSegments, long BeforePost)
{
int Success = 1;
long BlockSize, ExpBlockSize, RowSize, ExpRowSize, RowCt, PixCt, NodeBufZip;
unsigned long IntColors[3], NewIntValue, NewByteValue, OldIntValue, TotalPasses, TempExp[3], TempExp2[3];
double NewFloatValue;
char *Mode;
unsigned char *TempByteRow, *TempByteRow2, *RedBuf = NULL, *GreenBuf = NULL, *BlueBuf = NULL;
unsigned short *TempShortRow, *TempShortRow2, *TempExpRow, *TempExpRow2, *ExponentBuf = NULL;
float *TempFloatRow, *TempFloatRow2;
FILE *aafile, *aafileExp = NULL, *segfile, *segfileExp = NULL;
BufferNode *ExponentBufNode = NULL, *RedBufNode = NULL, *GreenBufNode = NULL, *BlueBufNode = NULL;

if (AAPass == 0)
	Mode = "wb";
else
	Mode = "rb+";

if (Node->Type == WCS_RASTER_BANDSET_BYTE)
	BlockSize = sizeof (unsigned char);
else if (Node->Type == WCS_RASTER_BANDSET_FLOAT)
	BlockSize = sizeof (float);
else if (Node->Type == WCS_RASTER_BANDSET_SHORT)
	BlockSize = sizeof (unsigned short);
else
	{
	UserMessageOK("Image Output", "Unable to save temporary file. Undefined variable size.");
	return (0);
	} // else
ExpBlockSize = sizeof (unsigned short);

// see if this is one of the RGB channels
if (! stricmp(Node->Name, "RED"))
	RedBufNode = Node;
else if (! stricmp(Node->Name, "GREEN"))
	GreenBufNode = Node;
else if (! stricmp(Node->Name, "BLUE"))
	BlueBufNode = Node;
// see if it is the exponent buffer
else if (! stricmp(Node->Name, "RGB EXPONENT"))
	ExponentBufNode = Node;
// see if there is an exponent buffer and RGB buffers
ExponentBuf = (unsigned short *)Buffers->FindBuffer("RGB EXPONENT",		WCS_RASTER_BANDSET_SHORT);
RedBuf = (unsigned char *)Buffers->FindBuffer("RED",		WCS_RASTER_BANDSET_BYTE);
GreenBuf = (unsigned char *)Buffers->FindBuffer("GREEN",		WCS_RASTER_BANDSET_BYTE);
BlueBuf = (unsigned char *)Buffers->FindBuffer("BLUE",		WCS_RASTER_BANDSET_BYTE);

RowSize = Width * BlockSize;
ExpRowSize = Width * ExpBlockSize;
TotalPasses = AAPass + 1;

if (aafile = OpenTempFile(Mode, "AAPass", Node->Name, Frame, BeforePost))
	{
	if (AAPass == 0)
		{
		// first AA pass does not require merging so exponent buffer does not need to be treated differently
		if (ImageSegments > 1)
			{
			// copy Segment file to AAPass file
			if (segfile = OpenTempFile("rb", "Segment", Node->Name, Frame, BeforePost))
				{
				// we can treat both bytes and floats the same since no manipulation of values equired
				if (TempByteRow = (unsigned char *)AppMem_Alloc(RowSize, 0))
					{
					for (RowCt = 0; RowCt < Height; RowCt ++)
						{
						if (fread(TempByteRow, RowSize, 1, segfile) != 1)
							{
							UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
							Success = 0;
							break;
							} // if
						if (fwrite(TempByteRow, RowSize, 1, aafile) != 1)
							{
							UserMessageOK("Image Output", "Unable to save temporary file. Possible reasons include disk full.");
							Success = 0;
							break;
							} // else
						} // for
					AppMem_Free(TempByteRow, RowSize);
					} // if TempByteRow
				fclose(segfile);
				} // if segfile
			else
				{
				UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
				Success = 0;
				} // else
			} // if ImageSegments > 1
		else
			{
			// save AAPass file
			if (fwrite(Node->Buffer, RowSize * Height, 1, aafile) != 1)
				{
				UserMessageOK("Image Output", "Unable to save temporary file. Possible reasons include disk full.");
				Success = 0;
				} // else
			} // else ImageSegments == 1
		} // if AAPass == 0
	else
		{
		if ((RedBufNode || GreenBufNode || BlueBufNode) && ExponentBuf)
			{
			if (ImageSegments > 1)
				{
				// merge Segment file into AAPass file
				if ((segfile = OpenTempFile("rb", "Segment", Node->Name, Frame, BeforePost)) &&
					(segfileExp = OpenTempFile("rb", "Segment", "RGB EXPONENT", Frame, BeforePost)) &&
					(aafileExp = OpenTempFile(Mode, "AAPass", "RGB EXPONENT", Frame, BeforePost)))
					{
					// TempByteRow will hold the new stuff
					if (TempByteRow = (unsigned char *)AppMem_Alloc(RowSize, 0))
						{
						TempFloatRow = (float *)TempByteRow;
						TempShortRow = (unsigned short *)TempByteRow;
						// TempByteRow2 will hold the old stuff
						if (TempByteRow2 = (unsigned char *)AppMem_Alloc(RowSize, 0))
							{
							TempFloatRow2 = (float *)TempByteRow2;
							TempShortRow2 = (unsigned short *)TempByteRow2;
							// TempExpRow will hold the new stuff
							if (TempExpRow = (unsigned short *)AppMem_Alloc(ExpRowSize, 0))
								{
								// TempExpRow2 will hold the old stuff
								if (TempExpRow2 = (unsigned short *)AppMem_Alloc(ExpRowSize, 0))
									{
									for (RowCt = 0; RowCt < Height; RowCt ++)
										{
										// read a line from each file
										if (fread(TempByteRow, RowSize, 1, segfile) != 1)
											{
											UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
											Success = 0;
											break;
											} // if
										if (fread(TempByteRow2, RowSize, 1, aafile) != 1)
											{
											UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
											Success = 0;
											break;
											} // else
										if (fread(TempExpRow, ExpRowSize, 1, segfileExp) != 1)
											{
											UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
											Success = 0;
											break;
											} // if
										if (fread(TempExpRow2, ExpRowSize, 1, aafileExp) != 1)
											{
											UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
											Success = 0;
											break;
											} // else
										// merge values, cast rows of data appropriately
										if (Node->Type == WCS_RASTER_BANDSET_BYTE)
											{
											for (PixCt = 0; PixCt < Width; PixCt ++)
												{
												// interpret values in light of exponent value
												// new value
												IntColors[0] = RedBufNode ? TempByteRow[PixCt]: 0;
												IntColors[1] = GreenBufNode ? TempByteRow[PixCt]: 0;
												IntColors[2] = BlueBufNode ? TempByteRow[PixCt]: 0;
												if (TempExpRow[PixCt])
													rPixelFragment::ExtractUnclippedExponentialColors(IntColors, TempExpRow[PixCt]);
												NewIntValue = RedBufNode ? IntColors[0]: GreenBufNode ? IntColors[1]: IntColors[2];
												// old value
												IntColors[0] = RedBufNode ? TempByteRow2[PixCt]: 0;
												IntColors[1] = GreenBufNode ? TempByteRow2[PixCt]: 0;
												IntColors[2] = BlueBufNode ? TempByteRow2[PixCt]: 0;
												if (TempExpRow2[PixCt])
													rPixelFragment::ExtractUnclippedExponentialColors(IntColors, TempExpRow2[PixCt]);
												OldIntValue = RedBufNode ? IntColors[0]: GreenBufNode ? IntColors[1]: IntColors[2];
												NewIntValue = NewIntValue + OldIntValue * AAPass;
												NewIntValue /= (TotalPasses);
												// band number
												OldIntValue = RedBufNode ? 0: GreenBufNode ? 1: 2;
												// fetch new exponent value and the de-exponentiated color value
												rPixelFragment::ExtractExponentialColor(NewIntValue, TempExpRow2[PixCt], OldIntValue);
												TempByteRow2[PixCt] = (unsigned char)NewIntValue;
												} // for
											} // if byte
										// go back and re-write AApass line to file
										if (fseek(aafile, -RowSize, SEEK_CUR))
											{
											UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
											Success = 0;
											break;
											} // else
										if (fwrite(TempByteRow2, RowSize, 1, aafile) != 1)
											{
											UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
											Success = 0;
											break;
											} // else
										// write out the new exponent value
										if (fseek(aafileExp, -ExpRowSize, SEEK_CUR))
											{
											UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
											Success = 0;
											break;
											} // else
										if (fwrite(TempExpRow2, ExpRowSize, 1, aafileExp) != 1)
											{
											UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
											Success = 0;
											break;
											} // else
										// This seemingly useless fseek is required between reading and writing operations
										// from the same file. See SAS and MS docs if you don't believe it. Failing to do so
										// may cause the next fread to fail.
										if (fseek(aafile, 0, SEEK_CUR))
											{
											UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
											Success = 0;
											break;
											} // else
										if (fseek(aafileExp, 0, SEEK_CUR))
											{
											UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
											Success = 0;
											break;
											} // else
										} // for Row
									AppMem_Free(TempExpRow2, ExpRowSize);
									} // if TempExpRow2
								else
									{
									UserMessageOK("Image Output", "Unable to save temporary file. Out of memory.");
									Success = 0;
									} // else
								AppMem_Free(TempExpRow, ExpRowSize);
								} // if TempExpRow
							else
								{
								UserMessageOK("Image Output", "Unable to save temporary file. Out of memory.");
								Success = 0;
								} // else
							AppMem_Free(TempByteRow2, RowSize);
							} // if TempByteRow2
						else
							{
							UserMessageOK("Image Output", "Unable to save temporary file. Out of memory.");
							Success = 0;
							} // else
						AppMem_Free(TempByteRow, RowSize);
						} // if TempByteRow
					else
						{
						UserMessageOK("Image Output", "Unable to save temporary file. Out of memory.");
						Success = 0;
						} // else
					fclose(segfile);
					} // if segfile
				else
					{
					UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
					Success = 0;
					} // else
				if (segfileExp)
					fclose(segfileExp);
				if (aafileExp)
					fclose(aafileExp);
				} // if ImageSegments > 1
			else
				{
				// merge AAPass file into memory
				if (aafileExp = OpenTempFile(Mode, "AAPass", "RGB EXPONENT", Frame, BeforePost))
					{
					if (TempByteRow = (unsigned char *)AppMem_Alloc(RowSize, 0))
						{
						TempFloatRow = (float *)TempByteRow;
						TempShortRow = (unsigned short *)TempByteRow;
						if (TempExpRow = (unsigned short *)AppMem_Alloc(ExpRowSize, 0))
							{
							NodeBufZip = 0;
							for (RowCt = 0; RowCt < Height; RowCt ++)
								{
								// read a line from AAPass file
								if (fread(TempByteRow, RowSize, 1, aafile) != 1)
									{
									UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
									Success = 0;
									break;
									} // if
								if (fread(TempExpRow, ExpRowSize, 1, aafileExp) != 1)
									{
									UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
									Success = 0;
									break;
									} // if
								// merge values, cast rows of data appropriately
								if (Node->Type == WCS_RASTER_BANDSET_BYTE)
									{
									for (PixCt = 0; PixCt < Width; PixCt ++, NodeBufZip ++)
										{
										if (PixCt == 115 && RowCt == 62)
											printf("");
										// interpret values in light of exponent value
										// new value
										IntColors[0] = RedBufNode ? ((unsigned char *)Node->Buffer)[NodeBufZip]: 0;
										IntColors[1] = GreenBufNode ? ((unsigned char *)Node->Buffer)[NodeBufZip]: 0;
										IntColors[2] = BlueBufNode ? ((unsigned char *)Node->Buffer)[NodeBufZip]: 0;
										if (ExponentBuf[NodeBufZip])
											rPixelFragment::ExtractUnclippedExponentialColors(IntColors, ExponentBuf[NodeBufZip]);
										NewIntValue = RedBufNode ? IntColors[0]: GreenBufNode ? IntColors[1]: IntColors[2];
										// old value
										IntColors[0] = RedBufNode ? TempByteRow[PixCt]: 0;
										IntColors[1] = GreenBufNode ? TempByteRow[PixCt]: 0;
										IntColors[2] = BlueBufNode ? TempByteRow[PixCt]: 0;
										if (TempExpRow[PixCt])
											rPixelFragment::ExtractUnclippedExponentialColors(IntColors, TempExpRow[PixCt]);
										OldIntValue = RedBufNode ? IntColors[0]: GreenBufNode ? IntColors[1]: IntColors[2];
										NewIntValue = NewIntValue + OldIntValue * AAPass;
										NewIntValue /= (TotalPasses);
										// band number
										OldIntValue = RedBufNode ? 0: GreenBufNode ? 1: 2;
										// fetch new exponent value and the de-exponentiated color value
										rPixelFragment::ExtractExponentialColor(NewIntValue, ExponentBuf[NodeBufZip], OldIntValue);
										((unsigned char *)Node->Buffer)[NodeBufZip] = (unsigned char)NewIntValue;
										} // for PixCt
									} // if byte
								} // for Row
							AppMem_Free(TempExpRow, ExpRowSize);
							} // if TempExpRow
						AppMem_Free(TempByteRow, RowSize);
						} // if TempByteRow
					else
						{
						UserMessageOK("Image Output", "Unable to save temporary file. Out of memory.");
						Success = 0;
						} // else
					} // if aafileExp
				else
					{
					UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
					Success = 0;
					} // else

				if (Success)
					{
					// save AAPass file
					if (fseek(aafile, 0, SEEK_SET))
						{
						UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
						Success = 0;
						} // else
					if (fwrite(Node->Buffer, RowSize * Height, 1, aafile) != 1)
						{
						UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
						Success = 0;
						} // else
					// on last color write out exponent buffer - don't do it sooner or it will interfere with reading previous values
					if (BlueBufNode)
						{
						if (fseek(aafileExp, 0, SEEK_SET))
							{
							UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
							Success = 0;
							} // else
						if (fwrite(ExponentBuf, ExpRowSize * Height, 1, aafileExp) != 1)
							{
							UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
							Success = 0;
							} // else
						} // if
					} // if Success
				if (aafileExp)
					fclose(aafileExp);
				} // else ImageSegments == 1
			} // if 
		else if (ExponentBufNode && RedBuf && GreenBuf && BlueBuf)
			{
			// exponent buffer has been handled during RGB processing
			} // else if
		else
			{
			if (ImageSegments > 1)
				{
				// merge Segment file into AAPass file
				if (segfile = OpenTempFile("rb", "Segment", Node->Name, Frame, BeforePost))
					{
					// TempByteRow will hold the new stuff
					if (TempByteRow = (unsigned char *)AppMem_Alloc(RowSize, 0))
						{
						TempFloatRow = (float *)TempByteRow;
						TempShortRow = (unsigned short *)TempByteRow;
						// TempByteRow2 will hold the old stuff
						if (TempByteRow2 = (unsigned char *)AppMem_Alloc(RowSize, 0))
							{
							TempFloatRow2 = (float *)TempByteRow2;
							TempShortRow2 = (unsigned short *)TempByteRow2;
							for (RowCt = 0; RowCt < Height; RowCt ++)
								{
								// read a line from each file
								if (fread(TempByteRow, RowSize, 1, segfile) != 1)
									{
									UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
									Success = 0;
									break;
									} // if
								if (fread(TempByteRow2, RowSize, 1, aafile) != 1)
									{
									UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
									Success = 0;
									break;
									} // else
								// merge values, cast rows of data appropriately
								if (Node->Type == WCS_RASTER_BANDSET_BYTE)
									{
									for (PixCt = 0; PixCt < Width; PixCt ++)
										{
										NewByteValue = TempByteRow[PixCt] + TempByteRow2[PixCt] * AAPass;
										NewByteValue /= (TotalPasses);
										TempByteRow2[PixCt] = (unsigned char)NewByteValue;
										} // for
									} // if byte
								else if (Node->Type == WCS_RASTER_BANDSET_SHORT)
									{
									for (PixCt = 0; PixCt < Width; PixCt ++)
										{
										TempExp[0] = TempShortRow[PixCt] >> 10;
										TempExp[1] = ((TempShortRow[PixCt] & 992) >> 5);
										TempExp[2] = (TempShortRow[PixCt] & 31);
										TempExp2[0] = TempShortRow2[PixCt] >> 10;
										TempExp2[1] = ((TempShortRow2[PixCt] & 992) >> 5);
										TempExp2[2] = (TempShortRow2[PixCt] & 31);
										TempExp[0] = TempExp[0] + TempExp2[0] * AAPass;
										TempExp[0] /= (TotalPasses);
										TempExp[1] = TempExp[1] + TempExp2[1] * AAPass;
										TempExp[1] /= (TotalPasses);
										TempExp[2] = TempExp[2] + TempExp2[2] * AAPass;
										TempExp[2] /= (TotalPasses);
										TempShortRow2[PixCt] = (unsigned short)((TempExp[0] << 10) | (TempExp[1] << 5) | TempExp[2]);
										} // for
									} // if byte
								else
									{
									for (PixCt = 0; PixCt < Width; PixCt ++)
										{
										NewFloatValue = TempFloatRow[PixCt] + TempFloatRow2[PixCt] * AAPass;
										NewFloatValue /= (TotalPasses);
										TempFloatRow2[PixCt] = (float)NewFloatValue;
										} // for
									} // else float
								// go back and re-write AApass line to file
								if (fseek(aafile, -RowSize, SEEK_CUR))
									{
									UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
									Success = 0;
									break;
									} // else
								if (fwrite(TempByteRow2, RowSize, 1, aafile) != 1)
									{
									UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
									Success = 0;
									break;
									} // else
								// This seemingly useless fseek is required between reading and writing operations
								// fom the same file. See SAS and MS docs if you don't believe it. Failing to do so
								// may cause the next fread to fail.
								if (fseek(aafile, 0, SEEK_CUR))
									{
									UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
									Success = 0;
									break;
									} // else
								} // for Row
							AppMem_Free(TempByteRow2, RowSize);
							} // if TempByteRow2
						else
							{
							UserMessageOK("Image Output", "Unable to save temporary file. Out of memory.");
							Success = 0;
							} // else
						AppMem_Free(TempByteRow, RowSize);
						} // if TempByteRow
					else
						{
						UserMessageOK("Image Output", "Unable to save temporary file. Out of memory.");
						Success = 0;
						} // else
					fclose(segfile);
					} // if segfile
				else
					{
					UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
					Success = 0;
					} // else
				} // if ImageSegments > 1
			else
				{
				// merge AAPass file into memory
				if (TempByteRow = (unsigned char *)AppMem_Alloc(RowSize, 0))
					{
					TempFloatRow = (float *)TempByteRow;
					TempShortRow = (unsigned short *)TempByteRow;
					NodeBufZip = 0;
					for (RowCt = 0; RowCt < Height; RowCt ++)
						{
						// read a line from AAPass file
						if (fread(TempByteRow, RowSize, 1, aafile) != 1)
							{
							UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
							Success = 0;
							break;
							} // if
						// merge values, cast rows of data appropriately
						if (Node->Type == WCS_RASTER_BANDSET_BYTE)
							{
							for (PixCt = 0; PixCt < Width; PixCt ++, NodeBufZip ++)
								{
								NewByteValue = ((unsigned char *)Node->Buffer)[NodeBufZip] + TempByteRow[PixCt] * AAPass;
								NewByteValue /= (TotalPasses);
								((unsigned char *)Node->Buffer)[NodeBufZip] = (unsigned char)NewByteValue;
								} // for PixCt
							} // if byte
						else if (Node->Type == WCS_RASTER_BANDSET_SHORT)
							{
							for (PixCt = 0; PixCt < Width; PixCt ++, NodeBufZip ++)
								{
								TempExp[0] = ((unsigned short *)Node->Buffer)[NodeBufZip] >> 10;
								TempExp[1] = ((((unsigned short *)Node->Buffer)[NodeBufZip] & 992) >> 5);
								TempExp[2] = (((unsigned short *)Node->Buffer)[NodeBufZip] & 31);
								TempExp2[0] = TempShortRow[PixCt] >> 10;
								TempExp2[1] = ((TempShortRow[PixCt] & 992) >> 5);
								TempExp2[2] = (TempShortRow[PixCt] & 31);
								TempExp[0] = TempExp[0] + TempExp2[0] * AAPass;
								TempExp[0] /= (TotalPasses);
								TempExp[1] = TempExp[1] + TempExp2[1] * AAPass;
								TempExp[1] /= (TotalPasses);
								TempExp[2] = TempExp[2] + TempExp2[2] * AAPass;
								TempExp[2] /= (TotalPasses);
								((unsigned short *)Node->Buffer)[NodeBufZip] = (unsigned short)((TempExp[0] << 10) | (TempExp[1] << 5) | TempExp[2]);
								} // for PixCt
							} // if byte
						else
							{
							for (PixCt = 0; PixCt < Width; PixCt ++, NodeBufZip ++)
								{
								NewFloatValue = ((float *)Node->Buffer)[NodeBufZip] + TempFloatRow[PixCt] * AAPass;
								NewFloatValue /= (TotalPasses);
								((float *)Node->Buffer)[NodeBufZip] = (float)NewFloatValue;
								} // for PixCt
							} // else float
						} // for Row
					AppMem_Free(TempByteRow, RowSize);
					} // if TempByteRow
				else
					{
					UserMessageOK("Image Output", "Unable to save temporary file. Out of memory.");
					Success = 0;
					} // else

				if (Success)
					{
					// save AAPass file
					if (fseek(aafile, 0, SEEK_SET))
						{
						UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
						Success = 0;
						} // else
					if (fwrite(Node->Buffer, RowSize * Height, 1, aafile) != 1)
						{
						UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
						Success = 0;
						} // else
					} // if Success
				} // else ImageSegments == 1
			} // if not RGB channel or exponent buffer and exponent buffer exists
		} // else AAPass > 0
	fclose(aafile);
	} // if aafile
else
	{
	UserMessageOK("Image Output", "Unable to open temporary file. Possible reasons include invalid Temp Path in Render Options or disk access permission denied.");
	Success = 0;
	} // else

return (Success);

} // RenderOpt::SaveAAPassTempFile

/*===========================================================================*/

int RenderOpt::SaveFieldTempFile(BufferNode *Node, long Width, long Height, long Frame, long Field, long FieldDominance, long AAPasses, long ImageSegments, long BeforePost)
{
int Success = 1;
long BlockSize, RowSize, RowCt, NodeBufZip;
char *Mode;
unsigned char *TempByteRow;
FILE *fldfile, *aasegfile;

if (Field == 0)
	Mode = "wb";
else
	Mode = "rb+";

if (Node->Type == WCS_RASTER_BANDSET_BYTE)
	BlockSize = sizeof (unsigned char);
else if (Node->Type == WCS_RASTER_BANDSET_FLOAT)
	BlockSize = sizeof (float);
else if (Node->Type == WCS_RASTER_BANDSET_SHORT)
	BlockSize = sizeof (unsigned short);
else
	{
	UserMessageOK("Image Output", "Unable to save temporary file. Undefined variable size.");
	return (0);
	} // else

RowSize = Width * BlockSize;

if (fldfile = OpenTempFile(Mode, "Field", Node->Name, Frame, BeforePost))
	{
	if (Field == 0)
		{
		if (AAPasses > 1 || ImageSegments > 1)
			{
			// copy Segment or AAPass file to Field file
			if ((aasegfile = OpenTempFile("rb", "AAPass", Node->Name, Frame, BeforePost)) ||
				(aasegfile = OpenTempFile("rb", "Segment", Node->Name, Frame, BeforePost)))
				{
				// we can treat both bytes and floats the same since no manipulation of values equired
				if (TempByteRow = (unsigned char *)AppMem_Alloc(RowSize, 0))
					{
					for (RowCt = 0; RowCt < Height; RowCt ++)
						{
						if (fread(TempByteRow, RowSize, 1, aasegfile) != 1)
							{
							UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
							Success = 0;
							break;
							} // if
						if (fwrite(TempByteRow, RowSize, 1, fldfile) != 1)
							{
							UserMessageOK("Image Output", "Unable to save temporary file. Possible reasons include disk full.");
							Success = 0;
							break;
							} // else
						} // for
					AppMem_Free(TempByteRow, RowSize);
					} // if TempByteRow
				fclose(aasegfile);
				} // if aasegfile
			else
				{
				UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
				Success = 0;
				} // else
			} // if AAPass > 1 || ImageSegments > 1
		else
			{
			// save Field file
			if (fwrite(Node->Buffer, RowSize * Height, 1, fldfile) != 1)
				{
				UserMessageOK("Image Output", "Unable to save temporary file. Possible reasons include disk full.");
				Success = 0;
				} // else
			} // else AAPasses == 1 && ImageSegments == 1
		} // if Field == 0
	else
		{
		if (AAPasses > 1 || ImageSegments > 1)
			{
			// merge Segment file into AAPass file
			if ((aasegfile = OpenTempFile("rb", "AAPass", Node->Name, Frame, BeforePost)) ||
				(aasegfile = OpenTempFile("rb", "Segment", Node->Name, Frame, BeforePost)))
				{
				if (TempByteRow = (unsigned char *)AppMem_Alloc(RowSize, 0))
					{
					// if FieldDominance is TRUE then skip first row of field 0 which is in the field file
					if (! FieldDominance)
						{
						if (fseek(aasegfile, RowSize, SEEK_CUR))
							{
							UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
							Success = 0;
							} // else
						if (fseek(fldfile, RowSize, SEEK_CUR))
							{
							UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
							Success = 0;
							} // else
						} // if
					if (Success)
						{
						for (RowCt = FieldDominance ? 0: 1; RowCt < Height; RowCt += 2)
							{
							// read a line from aaseg file
							if (fread(TempByteRow, RowSize, 1, aasegfile) != 1)
								{
								UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
								Success = 0;
								break;
								} // if

							// write the line to field file
							if (fwrite(TempByteRow, RowSize, 1, fldfile) != 1)
								{
								UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
								Success = 0;
								break;
								} // else
							if (RowCt < Height - 2)
								{
								if (fseek(fldfile, RowSize, SEEK_CUR))
									{
									UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
									Success = 0;
									break;
									} // else
								if (fseek(aasegfile, RowSize, SEEK_CUR))
									{
									UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
									Success = 0;
									break;
									} // else
								} // if
							} // for Row
						} // if Success
					AppMem_Free(TempByteRow, RowSize);
					} // if TempByteRow
				else
					{
					UserMessageOK("Image Output", "Unable to save temporary file. Out of memory.");
					Success = 0;
					} // else
				fclose(aasegfile);
				} // if aasegfile
			else
				{
				UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
				Success = 0;
				} // else
			} // if AAPasses > 1 || ImageSegments > 1
		else
			{
			// merge Field file into memory
			NodeBufZip = FieldDominance ? Width: 0;
			if (FieldDominance)
				{
				if (fseek(fldfile, RowSize, SEEK_CUR))
					{
					UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
					Success = 0;
					} // else
				} // if
			if (Success)
				{
				for (RowCt = FieldDominance ? 1: 0; RowCt < Height; RowCt += 2)
					{
					// read a line from Field file directly into current memory buffer
					if (Node->Type == WCS_RASTER_BANDSET_BYTE)
						{
						if (fread(&((unsigned char *)Node->Buffer)[NodeBufZip], RowSize, 1, fldfile) != 1)
							{
							UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
							Success = 0;
							break;
							} // if
						} // if byte
					else if (Node->Type == WCS_RASTER_BANDSET_SHORT)
						{
						if (fread(&((unsigned short *)Node->Buffer)[NodeBufZip], RowSize, 1, fldfile) != 1)
							{
							UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
							Success = 0;
							break;
							} // if
						} // if byte
					else
						{
						if (fread(&((float *)Node->Buffer)[NodeBufZip], RowSize, 1, fldfile) != 1)
							{
							UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
							Success = 0;
							break;
							} // if
						} // if byte

					// advance file position
					if (RowCt < Height - 2)
						{
						NodeBufZip += 2 * Width;
						if (fseek(fldfile, RowSize, SEEK_CUR))
							{
							UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
							Success = 0;
							break;
							} // else
						} // if
					} // for Row
				} // if Success
			if (Success)
				{
				// save Field file
				if (fseek(fldfile, 0, SEEK_SET))
					{
					UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
					Success = 0;
					} // else
				if (fwrite(Node->Buffer, RowSize * Height, 1, fldfile) != 1)
					{
					UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
					Success = 0;
					} // else
				} // if Success
			} // else AAPasses == 1 && ImageSegments == 1
		} // else Field > 0
	fclose(fldfile);
	} // if fldfile
else
	{
	UserMessageOK("Image Output", "Unable to open temporary file. Possible reasons include invalid Temp Path in Render Options or disk access permission denied.");
	Success = 0;
	} // else

return (Success);

} // RenderOpt::SaveFieldTempFile

/*===========================================================================*/

int RenderOpt::SavePanoramaTempFile(BufferNode *Node, long Width, long Height, long Frame, long PanoPanel, long Fields, long AAPasses, long ImageSegments, long BeforePost)
{
int Success = 1;
long BlockSize, RenderedRowSize, PanoFileRowSize, RowCt, NodeBufZip;
unsigned char *TempByteRow, *OldPanoByteRow;
FILE *panofile, *oldpanofile, *fieldaasegfile;

if (Node->Type == WCS_RASTER_BANDSET_BYTE)
	BlockSize = sizeof (unsigned char);
else if (Node->Type == WCS_RASTER_BANDSET_FLOAT)
	BlockSize = sizeof (float);
else if (Node->Type == WCS_RASTER_BANDSET_SHORT)
	BlockSize = sizeof (unsigned short);
else
	{
	UserMessageOK("Image Output", "Unable to save temporary file. Undefined variable size.");
	return (0);
	} // else

RenderedRowSize = Width * BlockSize;
PanoFileRowSize = Width * BlockSize * PanoPanel;

if (panofile = OpenVeryTempFile("wb", "Pano", Node->Name, Frame, BeforePost))
	{
	if (PanoPanel == 0)
		{
		if (Fields > 1 || AAPasses > 1 || ImageSegments > 1)
			{
			// copy Segment or AAPass file to Field file
			if ((fieldaasegfile = OpenTempFile("rb", "Field", Node->Name, Frame, BeforePost)) ||
				(fieldaasegfile = OpenTempFile("rb", "AAPass", Node->Name, Frame, BeforePost)) ||
				(fieldaasegfile = OpenTempFile("rb", "Segment", Node->Name, Frame, BeforePost)))
				{
				// we can treat both bytes and floats the same since no manipulation of values equired
				if (TempByteRow = (unsigned char *)AppMem_Alloc(RenderedRowSize, 0))
					{
					for (RowCt = 0; RowCt < Height; RowCt ++)
						{
						if (fread(TempByteRow, RenderedRowSize, 1, fieldaasegfile) != 1)
							{
							UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
							Success = 0;
							break;
							} // if
						if (fwrite(TempByteRow, RenderedRowSize, 1, panofile) != 1)
							{
							UserMessageOK("Image Output", "Unable to save temporary file. Possible reasons include disk full.");
							Success = 0;
							break;
							} // else
						} // for
					AppMem_Free(TempByteRow, RenderedRowSize);
					} // if TempByteRow
				fclose(fieldaasegfile);
				} // if fieldaasegfile
			else
				{
				UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
				Success = 0;
				} // else
			} // Fields > 1 || if AAPasses > 1 || ImageSegments > 1
		else
			{
			// save Pano file
			if (fwrite(Node->Buffer, RenderedRowSize * Height, 1, panofile) != 1)
				{
				UserMessageOK("Image Output", "Unable to save temporary file. Possible reasons include disk full.");
				Success = 0;
				} // else
			} // else Fields == 1 && AAPasses == 1 && ImageSegments == 1
		} // if PanoPanel == 0
	else
		{
		if (oldpanofile = OpenTempFile("rb", "Pano", Node->Name, Frame, BeforePost))
			{
			if (Fields > 1 || AAPasses > 1 || ImageSegments > 1)
				{
				// merge Segment file into AAPass file
				if ((fieldaasegfile = OpenTempFile("rb", "Field", Node->Name, Frame, BeforePost)) ||
					(fieldaasegfile = OpenTempFile("rb", "AAPass", Node->Name, Frame, BeforePost)) ||
					(fieldaasegfile = OpenTempFile("rb", "Segment", Node->Name, Frame, BeforePost)))
					{
					if (TempByteRow = (unsigned char *)AppMem_Alloc(RenderedRowSize, 0))
						{
						if (OldPanoByteRow = (unsigned char *)AppMem_Alloc(PanoFileRowSize, 0))
							{
							for (RowCt = 0; RowCt < Height; RowCt ++)
								{
								// read a line from fieldaasegfile file
								if (fread(TempByteRow, RenderedRowSize, 1, fieldaasegfile) != 1)
									{
									UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
									Success = 0;
									break;
									} // if
								// read a line from old pano file
								if (fread(OldPanoByteRow, PanoFileRowSize, 1, oldpanofile) != 1)
									{
									UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
									Success = 0;
									break;
									} // if

								// write the line to new pano file
								if (fwrite(OldPanoByteRow, PanoFileRowSize, 1, panofile) != 1)
									{
									UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
									Success = 0;
									break;
									} // else
								// write the line to new pano file
								if (fwrite(TempByteRow, RenderedRowSize, 1, panofile) != 1)
									{
									UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
									Success = 0;
									break;
									} // else
								} // for Row
							AppMem_Free(OldPanoByteRow, RenderedRowSize);
							} // if
						else
							{
							UserMessageOK("Image Output", "Unable to save temporary file. Out of memory.");
							Success = 0;
							} // else
						AppMem_Free(TempByteRow, RenderedRowSize);
						} // if TempByteRow
					else
						{
						UserMessageOK("Image Output", "Unable to save temporary file. Out of memory.");
						Success = 0;
						} // else
					fclose(fieldaasegfile);
					} // if fieldaasegfile
				else
					{
					UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
					Success = 0;
					} // else
				} // if Fields > 1 || AAPasses > 1 || ImageSegments > 1
			else
				{
				// merge previous Pano file with current memory and save to new file
				if (OldPanoByteRow = (unsigned char *)AppMem_Alloc(PanoFileRowSize, 0))
					{
					for (RowCt = 0, NodeBufZip = 0; RowCt < Height; RowCt ++, NodeBufZip += Width)
						{
						// read a line from old pano file
						if (fread(OldPanoByteRow, PanoFileRowSize, 1, oldpanofile) != 1)
							{
							UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
							Success = 0;
							break;
							} // if

						// write the old line to new pano file
						if (fwrite(OldPanoByteRow, PanoFileRowSize, 1, panofile) != 1)
							{
							UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
							Success = 0;
							break;
							} // else
						// write a new line to new pano file
						if (Node->Type == WCS_RASTER_BANDSET_BYTE)
							{
							if (fwrite(&((unsigned char *)Node->Buffer)[NodeBufZip], RenderedRowSize, 1, panofile) != 1)
								{
								UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
								Success = 0;
								break;
								} // if
							} // if byte
						else if (Node->Type == WCS_RASTER_BANDSET_SHORT)
							{
							if (fwrite(&((unsigned short *)Node->Buffer)[NodeBufZip], RenderedRowSize, 1, panofile) != 1)
								{
								UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
								Success = 0;
								break;
								} // if
							} // if byte
						else
							{
							if (fwrite(&((float *)Node->Buffer)[NodeBufZip], RenderedRowSize, 1, panofile) != 1)
								{
								UserMessageOK("Image Output", "Unable to re-write temporary file. Possible reasons include disk file corrupt.");
								Success = 0;
								break;
								} // if
							} // if byte
						} // for Row
					AppMem_Free(OldPanoByteRow, PanoFileRowSize);
					} // if
				} // else Fields == 1 && AAPasses == 1 && ImageSegments == 1
			fclose(oldpanofile);
			RemoveTempFile("Pano", Node->Name, Frame, BeforePost);
			} // if old pano file opened
		else
			{
			UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
			Success = 0;
			} // else
		} // else PanoPanel > 0
	fclose(panofile);
	RenameTempFile("Pano", Node->Name, Frame, BeforePost);
	} // if panofile
else
	{
	UserMessageOK("Image Output", "Unable to open temporary file. Possible reasons include invalid Temp Path in Render Options or disk access permission denied.");
	Success = 0;
	} // else

return (Success);

} // RenderOpt::SavePanoramaTempFile

/*===========================================================================*/

int RenderOpt::SaveTileTempFile(BufferNode *Node, long Width, long Height, long Frame, long XTile, long YTile, long XTiles, long BeforePost)
{
int Success = 1, BlockSize;
long PrevWidth, PrevHeight, FileWidth, Row, RowZip;
float *FloatBuf = NULL;
short *ShortBuf = NULL;
char *CurMode, *CharBuf = NULL;
FILE *ffile;
char ByteVal = 0;

if (Node->Type == WCS_RASTER_BANDSET_BYTE)
	{
	BlockSize = sizeof (unsigned char);
	CharBuf = (char *)Node->Buffer;
	} // if
else if (Node->Type == WCS_RASTER_BANDSET_FLOAT)
	{
	BlockSize = sizeof (float);
	FloatBuf = (float *)Node->Buffer;
	} // else if
else if (Node->Type == WCS_RASTER_BANDSET_SHORT)
	{
	BlockSize = sizeof (unsigned short);
	ShortBuf = (short *)Node->Buffer;
	} // else if
else
	{
	UserMessageOK("Image Output", "Unable to save temporary file. Undefined variable size.");
	return (0);
	} // else

if (XTile == 0 && YTile == 0)
	{
	CurMode = "wb";
	if (ffile = OpenTempFile(CurMode, "Tile", Node->Name, Frame, BeforePost))
		{
		FileWidth = XTiles * Width;
		for (Row = RowZip = 0; Success && Row < Height; Row ++, RowZip += Width)
			{
			// write a line
			if (CharBuf)
				{
				if (fwrite(&CharBuf[RowZip], Width * BlockSize, 1, ffile) != 1)
					Success = 0;
				} // if
			else if (ShortBuf)
				{
				if (fwrite(&ShortBuf[RowZip], Width * BlockSize, 1, ffile) != 1)
					Success = 0;
				} // if
			else if (FloatBuf)
				{
				if (fwrite(&FloatBuf[RowZip], Width * BlockSize, 1, ffile) != 1)
					Success = 0;
				} // if
			else
				Success = 0;	// undefined size
			// fseek to start of next line
			if (fseek(ffile, ((Row + 1) * FileWidth) * BlockSize, SEEK_SET))
				Success = 0;
			} // for
		// fseek to last position in tile row and write something
		if (Success)
			{
			if (fseek(ffile, (Height * FileWidth) * BlockSize - 1, SEEK_SET))
				Success = 0;
			if (fwrite(&ByteVal, 1, 1, ffile) != 1)
				Success = 0;
			} // if
		fclose(ffile);
		} // if
	else
		Success = 0;
	} // if
else
	{
	CurMode = "rb+";
	if (ffile = OpenTempFile(CurMode, "Tile", Node->Name, Frame, BeforePost))
		{
		PrevHeight = YTile * Height;
		PrevWidth = XTile * Width;
		FileWidth = XTiles * Width;
		for (Row = RowZip = 0; Success && Row < Height; Row ++, RowZip += Width)
			{
			// seek the start of the data line
			if (fseek(ffile, ((Row + PrevHeight) * FileWidth + PrevWidth) * BlockSize, SEEK_SET))
				Success = 0;
			// write new data line
			if (Success)
				{
				if (CharBuf)
					{
					if (fwrite(&CharBuf[RowZip], Width * BlockSize, 1, ffile) != 1)
						Success = 0;
					} // if
				else if (ShortBuf)
					{
					if (fwrite(&ShortBuf[RowZip], Width * BlockSize, 1, ffile) != 1)
						Success = 0;
					} // if
				else if (FloatBuf)
					{
					if (fwrite(&FloatBuf[RowZip], Width * BlockSize, 1, ffile) != 1)
						Success = 0;
					} // if
				else
					Success = 0;	// undefined size
				} // if
			} // for
		if (XTile == 0)
			{
			// write a byte at the end of the tile row to resize the file
			if (fseek(ffile, ((Height + PrevHeight) * FileWidth) * BlockSize - 1, SEEK_SET))
				Success = 0;
			if (fwrite(&ByteVal, 1, 1, ffile) != 1)
				Success = 0;
			} // if
		fclose(ffile);
		} // if ffile
	else
		Success = 0;
	} // else

return (Success);

} // RenderOpt::SaveTileTempFile

/*===========================================================================*/
/* replaced by above 12/14/05 - GRH to speed large file saving. Copying from file to file was slow
int RenderOpt::SaveTileTempFile(BufferNode *Node, long Width, long Height, long Frame, long XTile, long YTile, long XTiles, long BeforePost)
{
int Success = 1, BlockSize;
long PrevWidth, PrevHeight, Row, RowZip;
float *FloatBuf = NULL;
short *ShortBuf = NULL;
char *PrevMode, *CurMode, *PrevRowBuffer, *CharBuf = NULL;
FILE *ffile, *fprevfile;

PrevMode = "rb";
CurMode = "wb";

if (Node->Type == WCS_RASTER_BANDSET_BYTE)
	{
	BlockSize = sizeof (unsigned char);
	CharBuf = (char *)Node->Buffer;
	} // if
else if (Node->Type == WCS_RASTER_BANDSET_FLOAT)
	{
	BlockSize = sizeof (float);
	FloatBuf = (float *)Node->Buffer;
	} // else if
else if (Node->Type == WCS_RASTER_BANDSET_SHORT)
	{
	BlockSize = sizeof (unsigned short);
	ShortBuf = (short *)Node->Buffer;
	} // else if
else
	{
	UserMessageOK("Image Output", "Unable to save temporary file. Undefined variable size.");
	return (0);
	} // else

if (XTile == 0 && YTile == 0)
	{
	if (ffile = OpenTempFile(CurMode, "Tile", Node->Name, Frame, BeforePost))
		{
		if (fwrite(Node->Buffer, Width * Height * BlockSize, 1, ffile) != 1)
			Success = 0;
		fclose(ffile);
		} // if
	} // if
else
	{
	if (PrevRowBuffer = (char *)AppMem_Alloc(XTiles * Width * BlockSize, 0))
		{
		if (ffile = OpenVeryTempFile(CurMode, "Tile", Node->Name, Frame, BeforePost))
			{
			if (fprevfile = OpenTempFile(PrevMode, "Tile", Node->Name, Frame, BeforePost))
				{
				PrevWidth = XTiles * Width;
				PrevHeight = YTile * Height;
				for (Row = 0; Success && Row < PrevHeight; Row ++)
					{
					if (fread(PrevRowBuffer, PrevWidth * BlockSize, 1, fprevfile) != 1)
						Success = 0;
					if (Success)
						{
						if (fwrite(PrevRowBuffer, PrevWidth * BlockSize, 1, ffile) != 1)
							Success = 0;
						} // if
					} // for
				PrevWidth = XTile * Width;
				for (Row = RowZip = 0; Success && Row < Height; Row ++, RowZip += Width)
					{
					if (XTile > 0)
						{
						// read a line from prevfile
						if (fread(PrevRowBuffer, PrevWidth * BlockSize, 1, fprevfile) != 1)
							Success = 0;
						// write old data line
						if (Success)
							{
							if (fwrite(PrevRowBuffer, PrevWidth * BlockSize, 1, ffile) != 1)
								Success = 0;
							} // if
						} // if
					// write new data line
					if (Success)
						{
						if (CharBuf)
							{
							if (fwrite(&CharBuf[RowZip], Width * BlockSize, 1, ffile) != 1)
								Success = 0;
							} // if
						else if (ShortBuf)
							{
							if (fwrite(&ShortBuf[RowZip], Width * BlockSize, 1, ffile) != 1)
								Success = 0;
							} // if
						else if (FloatBuf)
							{
							if (fwrite(&FloatBuf[RowZip], Width * BlockSize, 1, ffile) != 1)
								Success = 0;
							} // if
						else
							Success = 0;	// undefined size
						} // if
					} // for
				fclose(fprevfile);
				RemoveTempFile("Tile", Node->Name, Frame, BeforePost);
				} // if fprevfile
			else
				Success = 0;
			fclose(ffile);
			RenameTempFile("Tile", Node->Name, Frame, BeforePost);
			} // if ffile
		else
			Success = 0;
		AppMem_Free(PrevRowBuffer, XTiles * Width * BlockSize);
		} // if alloc mem
	else
		Success = 0;
	} // else

return (Success);

} // RenderOpt::SaveTileTempFile
*/
/*===========================================================================*/

FILE *RenderOpt::OpenTempFile(char *Mode, char *FileType, char *BufferType, long Frame, long BeforePost)
{
char FullPath[512];

return (PROJ_fopen(MakeTempFileName(FullPath, FileType, BufferType, Frame, BeforePost), Mode));

} // RenderOpt::OpenTempFile

/*===========================================================================*/

FILE *RenderOpt::OpenVeryTempFile(char *Mode, char *FileType, char *BufferType, long Frame, long BeforePost)
{
char FullPath[512];

MakeTempFileName(FullPath, FileType, BufferType, Frame, BeforePost);
strcat(FullPath, "VT");

return (PROJ_fopen(FullPath, Mode));

} // RenderOpt::OpenVeryTempFile

/*===========================================================================*/

void RenderOpt::RemoveTempFile(char *FileType, char *BufferType, long Frame, long BeforePost)
{
char FullPath[512];

PROJ_remove(MakeTempFileName(FullPath, FileType, BufferType, Frame, BeforePost));

} // RenderOpt::RemoveTempFile

/*===========================================================================*/

void RenderOpt::RenameTempFile(char *FileType, char *BufferType, long Frame, long BeforePost)
{
char FullPath[512], OldFullPath[512];

MakeTempFileName(FullPath, FileType, BufferType, Frame, BeforePost);
strcpy(OldFullPath, FullPath);
strcat(OldFullPath, "VT");

PROJ_rename(OldFullPath, FullPath);

} // RenderOpt::RenameTempFile

/*===========================================================================*/

char *RenderOpt::MakeTempFileName(char *FullPath, char *FileType, char *BufferType, long Frame, long BeforePost)
{
char FileName[512], TempStr[24], SysNameStr[200];
unsigned long WCSInstance = GlobalApp->GetSystemProcessID();

(void)GlobalApp->InquireSystemNameString(SysNameStr, 200);

strcpy(FileName, "WCS_");
sprintf(TempStr, "%s_%d", SysNameStr, WCSInstance);
strcat(FileName, TempStr);
strcat(FileName, "_");
strcat(FileName, FileType);
strcat(FileName, "_");
strcat(FileName, BufferType);
strcat(FileName, "_");
sprintf(TempStr, "%d", Frame);
strcat(FileName, TempStr);
if (BeforePost)
	strcat(FileName, "BP");

strmfp(FullPath, TempPath.GetPath(), FileName);

return (FullPath);

} // RenderOpt::MakeTempFileName

/*===========================================================================*/

int RenderOpt::RenderPostProc(int PreReflection, RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, long FrameNum, int UpdateDiagnostics)
{
EffectList *CurProc = Post;

while (CurProc)
	{
	if (CurProc->Me && CurProc->Me->Enabled && (((PostProcess *)CurProc->Me)->BeforeReflection == PreReflection))
		{
		// don't do this one if it is a preview render and preview is enabled for the PPE since it will result in it being done twice
		if (Rend->IsCamView && GlobalApp->GUIWins->PPR && GlobalApp->GUIWins->PPR->GetActive() == (PostProcess *)CurProc->Me)
			{
			if (GlobalApp->GUIWins->PPR->PreviewEnabled)
				{
				CurProc = CurProc->Next;
				continue;
				} // if
			} // if
		if (! ((PostProcess *)CurProc->Me)->RenderPostProc(Rend, Buffers, FragBlock, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics))
			return (0);
		} // if
	CurProc = CurProc->Next;
	} // while

return (1);

} // RenderOpt::RenderPostProc

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int RenderOpt::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG ItemTag = 0, Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
RenderOpt *CurrentRenderOpt = NULL;
PostProcess *CurrentProc = NULL;
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
					else if (! strnicmp(ReadBuf, "PostProc", 8))
						{
						if (CurrentProc = new PostProcess(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentProc->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;
							}
						} // if post process
					else if (! strnicmp(ReadBuf, "RendrOpt", 8))
						{
						if (CurrentRenderOpt = new RenderOpt(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentRenderOpt->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if light
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

if (Success == 1 && CurrentRenderOpt)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentRenderOpt);
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

} // RenderOpt::LoadObject

/*===========================================================================*/

int RenderOpt::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
DEMBounds CurBounds;
EffectList *CurEffect, *PostProc = NULL, *Coords = NULL;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
char StrBuf[12];

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

if (BuildFileComponentsList(&PostProc, &Coords))
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

	CurEffect = PostProc;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "PostProc");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((PostProcess *)CurEffect->Me)->Save(ffile))
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
						} // if PostProcess saved 
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
	while (PostProc)
		{
		CurEffect = PostProc;
		PostProc = PostProc->Next;
		delete CurEffect;
		} // while
	} // if

// RenderOpt
strcpy(StrBuf, "RendrOpt");
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
			} // if RenderOpt saved 
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

} // RenderOpt::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::RenderOpt_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
RenderOpt *Current;
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
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new RenderOpt(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (RenderOpt *)FindDuplicateByName(Current->EffectType, Current))
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

} // EffectsLib::RenderOpt_Load()

/*===========================================================================*/

ULONG EffectsLib::RenderOpt_Save(FILE *ffile)
{
RenderOpt *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

Current = RenderOpts;
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
					} // if RenderOpt saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (RenderOpt *)Current->Next;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // EffectsLib::RenderOpt_Save()

/*===========================================================================*/
