// Render.cpp
// The all new for v5 Renderer
// Built mostly from scratch by Gary R. Huber, 8/99
// Copyright 1999 by Questar Productions. All rights reserved.

#include "stdafx.h"
#include "stdlib.h"
#include "Render.h"
#include "Application.h"
#include "Project.h"
#include "EffectsLib.h"
#include "Raster.h"
#include "Log.h"
#include "Requester.h"
#include "Interactive.h"
#include "RenderControlGUI.h"
#include "RenderPreviewGUI.h"
#include "resource.h"
#include "AppMem.h"
#include "Joe.h"
#include "ImageOutputEvent.h"
#include "GUI.h"
#include "WCSVersion.h"
#include "Realtime.h"
#include "PixelManager.h"
#include "Conservatory.h"
#include "DiagnosticGUI.h"
#include "PostProcEditGUI.h"
#include "ImageFormat.h"
#include "RasterBounds.h"
#include "VectorNode.h"
#include "VectorPolygon.h"
#include "Lists.h"
#include "EffectEval.h"
#include "UsefulUnit.h"
#include "DEMCore.h"
#include "EXIF.h"
//#include "ThinThread.h"

extern EXIFtool gEXIF;
extern ImageFormatAI *VectorOutput; // are we setup to write vectors to a file?
extern int suppress_rendernotify;
extern int FragMemFailed;
extern VNAccelerator VNA;
#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
extern unsigned __int64 DevCounter[50];
#endif // FRANK or GARY

// debugging stuff for forestry features
#ifdef WCS_FORESTRY_WIZARD
#ifdef WCS_BUILD_GARY
extern double AreaRendered, StemsRendered, AvgStemsRendered, PolysRendered, GroupStemsRendered[4], ImageStemsRendered[4][8],
	SumPRN, PRNAbove, PRNBelow;
extern long PRNCt, TinyPRN;
#endif // WCS_BUILD_GARY
#endif // WCS_FORESTRY_WIZARD

double UStartSecsFP, UEndSecsFP;

#ifdef WCS_VECPOLY_EFFECTS
QuantaAllocator *Renderer::Quanta;
RendererQuantaAllocatorErrorFunctor *Renderer::MyErrorFunctor;
unsigned long Renderer::DEMMemoryCurrentSize;
// enable this or not depending if you want the VectorNode array for DEM subdivision
// to be manually deleted before removing the Quanta Allocator. The memory is actually freed by the QA
// in either event.
//#define WCS_QUANTAALLOCATOR_DETAILED_CLEANUP
#endif // WCS_VECPOLY_EFFECTS


#define MAGIC_TREEPT_CUTOFF		128
//#define DEFAULTAAKERNSIZE	16

float DefaultAAKernX[] =
	{
	0.00f,	// 0
	-.50f,
	+.50f,
	+.50f,
	-.50f,
	-.25f,	// 5
	+.25f,
	+.25f,
	-.25f,
	0.00f,
	+.50f,	// 10
	0.00f,
	-.50f,
	0.00f,
	+.25f,
	0.00f,	// 15
	-.25f
	}; // DefaultAAKernX

float DefaultAAKernY[] =
	{
	0.00f,	// 0
	-.50f,
	-.50f,
	+.50f,
	+.50f,
	-.25f,	// 5
	-.25f,
	+.25f,
	+.25f,
	-.50f,
	0.00f,	// 10
	+.50f,
	0.00f,
	-.25f,
	0.00f,
	+.25f,	// 15
	0.00f
	}; // DefaultAAKernY

extern unsigned char ThumbNailR[2048 * 3], ThumbNailG[2048], ThumbNailB[2048];

NotifyTag FreezeEvent[2] = {MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff), NULL};
NotifyTag ThawEvent[2] = {MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff), NULL};

/*===========================================================================*/
/*===========================================================================*/

Renderer::Renderer()
: RendData(this)
{
long Ct;

Master = NULL;
Job = NULL;
Opt = NULL;
Cam = ViewCamCopy = NULL;
AppBase = NULL;
EffectsBase = NULL;
ImageBase = NULL;
ProjectBase = NULL;
DBase = NULL;
LocalLog = NULL;
Buffers = NULL;
Rast = LastRast = NULL;
CurDEM = NULL;
DEMCue = NULL;
Preview = NULL;
Substances = NULL;
Exporter = NULL;
#ifdef WCS_VECPOLY_EFFECTS
EvalEffects = NULL;
PhongTerrain = false;
//Quanta = NULL;
//MyErrorFunctor = NULL;
QuantaCreated = false;
#endif // WCS_VECPOLY_EFFECTS
RemoteGauge = NULL;

Bitmap[2] = Bitmap[1] = Bitmap[0] = AABuf = TreeBitmap[2] = TreeBitmap[1] = TreeBitmap[0] = TreeAABuf = 
	CloudBitmap[2] = CloudBitmap[1] = CloudBitmap[0] = CloudAABuf = FlagBuf = ObjTypeBuf = PlotBuf[2] = PlotBuf[1] = PlotBuf[0] = NULL;
ZBuf = TreeZBuf = CloudZBuf = LatBuf = LonBuf = ElevBuf = RelElBuf = ReflectionBuf = IllumBuf = 
	SlopeBuf = AspectBuf = NormalBuf[2] = NormalBuf[1] = NormalBuf[0] = NULL;
ObjectBuf = NULL;
ExponentBuf = NULL;
Pane = NULL;
PixelWeight = PixelBitBytes = NULL;
VertexWeight[2] = VertexWeight[1] = VertexWeight[0] = NULL;
PolyEdge1 = PolyEdge2 = NULL;
PolyEdgeSize = 0;
PixelWeightSize = 0;
SetupWidth = SetupHeight = Width = Height = OSLowX = OSHighX = OSLowY = OSHighY = 0;
StartSecs = FirstSecs = FrameSecs = NowTime = FramesRendered = 0;
FramesToRender = 1;
DrawOffsetX = DrawOffsetY = 0;
FieldInterval = 0.0;
MoBlurInterval = 0.0;
StashProjectTime = 0.0;
StashProjectFrameRate = 30.0;
ExagerateElevLines = ShadowMapInProgress = 0;
ElevDatum = Exageration = PlanetRad = EarthRotation= 0.0;
RenderTime = ShadowMinFolHt = ShadowMapDistanceOffset = QMax = 0.0;
PanoPanel = Segment = StereoStart = StereoEnd = 0;
NumPanels = NumSegments = 1;
EarthLatScaleMeters = 1.0;
MaxFractalDepth = 0;
FrdWarned = DefaultFrd = 0;
ShadowsExist = RasterTAsExist = TerraffectorsExist = StreamsExist = LakesExist = WaterExists = SnowsExist = RenderBathos =
	CmapsExist = EcosExist = EcoRastersExist = VolumetricAtmospheresExist = FoliageEffectsExist = Object3DsExist = LightTexturesExist = AtmoTexturesExist = BackfaceCull = 0;
SegmentOffsetX = SegmentOffsetY = 0.0;
SegmentOffsetUnityX = SegmentOffsetUnityY = 0.0;
TexRefLat = TexRefLon = TexRefElev = RefLonScaleMeters = ZMergeDistance = 0.0;
MinimumZ = 0.0;
MaximumZ = FLT_MAX;
ZTranslator = InverseFromZ = CenterPixelSize = 1.0;
IsCamView = IsProcessing = MultiBuffer = CamPlanOrtho = 0;
DisplayBuffer = WCS_DIAGNOSTIC_RGB;
DEMCellSize = 100.0;
TrueCenterX = TrueCenterY = 0.0;
ElScaleMult = 1.0;
ShadowLight = NULL;
CurFolListDat =	FolListDat = CurLabelListDat = LabelListDat = NULL;
RealTimeFoliageWrite = NULL;
TexRefVert = NULL;
rPixelFragMap = NULL;
rPixelBlock = NULL;
FragmentDepth = 1;
PlotFromFrags = 0;
RenderImageSegments = XTiles = YTiles = 1;
DEMstr[0] = TileStr[0] = 0;
CurPolyNumber = LastPolyNumber = -1;
#ifdef WCS_VECPOLY_EFFECTS
WaterMatTableStart = WaterMatTableEnd = NULL;
#endif // WCS_VECPOLY_EFFECTS

PerturbTable[0] = 1.0;
for (Ct = 1; Ct < WCS_MAX_FRACTALDEPTH + 1; Ct ++)
	{
	PerturbTable[Ct] = PerturbTable[Ct - 1] * 0.5;  // Optimized out division. Was / 2.0
	} // for

DefaultTerrainPar = NULL;
DefaultPlanetOpt = NULL;
DefaultEnvironment = NULL;
DefCoords = NULL;

VertexRoot[0] = VertexRoot[1] = VertexRoot[2] = NULL;
EdgeRoot[0] = EdgeRoot[1] = EdgeRoot[2] = NULL;

#ifdef WCS_BUILD_DEMO
// need a random seed that will be different each time a renderer is created
// If the limits for render image size or frames have been hacked a bailout provision
// will be called based on a random number before any DEMs are rendered (which will reset
// the seed value).
RendRand.Seed64((unsigned long)this, (unsigned long)this);
#endif

} // Renderer::Renderer

/*===========================================================================*/

Renderer::~Renderer()
{
BufferNode *NextBuf;

rFreePixelFragmentMap();

if (Preview)
	{
	ClosePreview();
	} // if

while (Buffers)
	{
	NextBuf = Buffers->Next;
	delete Buffers;
	Buffers = NextBuf;
	} // if

if (Rast)
	delete Rast;
if (LastRast)
	delete LastRast;

if (DEMCue)
	delete DEMCue;
if (ViewCamCopy)
	delete ViewCamCopy;
if (TexRefVert)
	delete TexRefVert;
#ifdef WCS_VECPOLY_EFFECTS
long CurMatCt;

for (CurMatCt = 0; CurMatCt < MAX_MATERIALTABLE_ENTRIES * 3; ++CurMatCt)
	{
	RendererScopeWaterMatList[CurMatCt].WaterProp = NULL;
	RendererScopeWaterMatList[CurMatCt].VectorProp = NULL;
	} // for
if (EvalEffects)
	delete EvalEffects;
if (QuantaCreated)
	RemoveQuantaAllocator();
#endif // WCS_VECPOLY_EFFECTS

} // Renderer::~Renderer

/*===========================================================================*/

int Renderer::Init(RenderJob *JobSource, WCSApp *AppSource, EffectsLib *EffectsSource, ImageLib *ImageSource, 
	Database *DBSource, Project *ProjectSource, MessageLog *LogSource, RenderInterface *MasterSource,
	DrawingFenetre *SuppliedPane, int FirstInit)
{

if (Job = JobSource)
	{
	Opt = JobSource->Options;
	Cam = JobSource->Cam;
	AppBase = AppSource;
	EffectsBase = EffectsSource;
	ImageBase = ImageSource;
	ProjectBase = ProjectSource;
	DBase = DBSource;
	Master = MasterSource;
	LocalLog = LogSource;
	Pane = SuppliedPane;

	#ifdef WCS_VECPOLY_EFFECTS
	if (Opt && Cam && AppBase && EffectsBase && ImageBase && ProjectBase && DBase && LocalLog && 
		(EvalEffects = new EffectEval(EffectsBase, DBase, ProjectBase, this)))
	#else // WCS_VECPOLY_EFFECTS
	if (Opt && Cam && AppBase && EffectsBase && ImageBase && ProjectBase && DBase && LocalLog)
	#endif // WCS_VECPOLY_EFFECTS
		{
		StashProjectTime = ProjectBase->Interactive->GetActiveTime();
		StashProjectFrameRate = ProjectBase->Interactive->GetFrameRate();

		DefCoords = EffectsSource->FetchDefaultCoordSys();

		// lock out view updates which cause cameras to Init to the view sizes
		GlobalApp->AppEx->GenerateNotify(FreezeEvent, NULL);
		InitRemoteGauge("Initializing", 100, 0);
		if (SetImageSize(FALSE))
			{
			#ifdef WCS_VECPOLY_EFFECTS
			if (AddBasicBufferNodes() && AddQuantaAllocator())
				{
				#ifdef WCS_TEST_EARLY_TERRAIN_INIT
				if (InitTerrainFactors())
				#endif // WCS_TEST_EARLY_TERRAIN_INIT
					{
					if (EffectsBase->InitToRender(DBase, ProjectBase, Opt, Buffers, EvalEffects, Width, Height, 0, 0, FirstInit))	// 0 = not for Shadows, 0 = not elevation only
			#else // WCS_VECPOLY_EFFECTS
			if (AddBasicBufferNodes())
				{
				#ifdef WCS_TEST_EARLY_TERRAIN_INIT
				if (InitTerrainFactors())
				#endif // WCS_TEST_EARLY_TERRAIN_INIT
					{
					if (EffectsBase->InitToRender(DBase, ProjectBase, Opt, Buffers, Width, Height, 0, 0, FirstInit))	// 0 = not for Shadows, 0 = not elevation only
			#endif // WCS_VECPOLY_EFFECTS
						{
						if (AllocateBuffers())
							{
							if (EffectsBase->InitToRender(Opt, Buffers))
								{
								ImageBase->ClearImageRasters(WCS_RASTER_LONGEVITY_SHORT);
								if (InitOncePerRender())
									{
									SetDrawOffsets();
									LocalLog->GoModal();
									AppBase->SetProcPri((signed char)ProjectBase->InquireRenderPri());
									if (Master)
										{
										if (FirstInit)
											SetInitialTexts();
										else
											{
											Master->UpdateStamp();
											Master->UpdateLastFrame();
											} // else
										Master->GUIGoModal();
										if (Master->GetPreview())
											{
											OpenPreview();
											if (Pane)
												Pane->Clear();
											else
												Master->SetPreview(0);
											} // if
										} // if
									GetTime(FirstSecs);	// in Useful.cpp
									UStartSecsFP = GetProcessTimeFP();
									RemoveRemoteGauge();

									#ifdef WCS_BUILD_DEMO
									// in case they've hacked the limits
									if (Width > 640)
										return (0);
									if (Height > 450)
										return (0);
									#endif // WCS_BUILD_DEMO

									return (1);
									} // if
								} // if
							} // if
						} // if
					} // if
				} // if
			} // if
		} // if
	} // if

RemoveRemoteGauge();
return (0);

} // Renderer::Init

/*===========================================================================*/

int Renderer::InitForProcessing(RenderJob *JobSource, WCSApp *AppSource, EffectsLib *EffectsSource, ImageLib *ImageSource, 
	Database *DBSource, Project *ProjectSource, MessageLog *LogSource, int ElevationsOnly)
{

if (Job = JobSource)
	{
	Opt = JobSource->Options;
	Cam = JobSource->Cam;
	AppBase = AppSource;
	EffectsBase = EffectsSource;
	ImageBase = ImageSource;
	ProjectBase = ProjectSource;
	DBase = DBSource;
	LocalLog = LogSource;

	#ifdef WCS_VECPOLY_EFFECTS
	if (Opt && Cam && AppBase && EffectsBase && ImageBase && ProjectBase && DBase && LocalLog && 
		(EvalEffects = new EffectEval(EffectsBase, DBase, ProjectBase, this)))
	#else // WCS_VECPOLY_EFFECTS
	if (Opt && Cam && AppBase && EffectsBase && ImageBase && ProjectBase && DBase && LocalLog)
	#endif // WCS_VECPOLY_EFFECTS
		{
		StashProjectTime = ProjectBase->Interactive->GetActiveTime();
		StashProjectFrameRate = ProjectBase->Interactive->GetFrameRate();

		DefCoords = EffectsSource->FetchDefaultCoordSys();

		InitRemoteGauge("Initializing", 100, 0);
		if (SetImageSize(TRUE))
			{
			#ifdef WCS_VECPOLY_EFFECTS
			if (AddQuantaAllocator())
				{
				#ifdef WCS_TEST_EARLY_TERRAIN_INIT
				if (InitTerrainFactors())
				#endif // WCS_TEST_EARLY_TERRAIN_INIT
					{
					if (EffectsBase->InitToRender(DBase, ProjectBase, Opt, Buffers, EvalEffects, Width, Height, 0, ElevationsOnly, 1))	// 0 = not for Shadows, 1 = elevation only, 1 = first init
			#else // WCS_VECPOLY_EFFECTS
				#ifdef WCS_TEST_EARLY_TERRAIN_INIT
				if (InitTerrainFactors())
				#endif // WCS_TEST_EARLY_TERRAIN_INIT
					{
					if (EffectsBase->InitToRender(DBase, ProjectBase, Opt, Buffers, Width, Height, 0, ElevationsOnly, 1))	// 0 = not for Shadows, 1 = elevation only, 1 = first init
			#endif // WCS_VECPOLY_EFFECTS
						{
						if (EffectsBase->InitToRender(Opt, Buffers))
							{
							if (InitOncePerRender())
								{
								IsCamView = 1;
								IsProcessing = 1;
								GetTime(FirstSecs);	// in Useful.cpp
								UStartSecsFP = GetProcessTimeFP();
								RemoveRemoteGauge();
								return (1);
								} // if
							} // if
						} // if
					} // if
			#ifdef WCS_VECPOLY_EFFECTS
				} // if
			#endif // WCS_VECPOLY_EFFECTS
			} // if
		} // if
	} // if

RemoveRemoteGauge();
return (0);

} // Renderer::InitForProcessing

/*===========================================================================*/

void Renderer::InitTextureData(TextureData *TexData)
{

TexData->MetersPerDegLat = EarthLatScaleMeters;
TexData->Datum = ElevDatum;
TexData->Exageration = Exageration;
TexData->ExagerateElevLines = ExagerateElevLines;

} // Renderer::InitTextureData

/*===========================================================================*/

void Renderer::InitRemoteGauge(char *NewTitle, unsigned long  NewMaxSteps, unsigned long  NewCurStep)
{

if (Master)
	{
	IsCamView = 0;
	Master->ProcInit(NewMaxSteps, NewTitle);
	Master->ProcUpdate(NewCurStep);
	} // if
else
	{
	IsCamView = 1;
	if (RemoteGauge)
		RemoteGauge->Reconfigure(NewTitle, NewMaxSteps, NewCurStep);
	else if (RemoteGauge = new BusyWin(NewTitle, NewMaxSteps, 'BWDE', 0))
		RemoteGauge->Update(NewCurStep);
	} // else

} // Renderer::InitRemoteGauge

/*===========================================================================*/

void Renderer::RemoveRemoteGauge(void)
{

if (RemoteGauge)
	{
	delete RemoteGauge;
	RemoteGauge = NULL;
	} // if

} // Renderer::RemoveRemoteGauge

/*===========================================================================*/

bool Renderer::UpdateRemoteGauge(unsigned long  NewCurStep)
{

if (Master)
	{
	Master->ProcUpdate(NewCurStep);
	if (! Master->IsRunning())
		return (false);
	} // if
else
	{
	if (RemoteGauge)
		{
		if (RemoteGauge->Update(NewCurStep))
			return (false);
		} // if
	} // else

return (true);

} // Renderer::UpdateRemoteGauge

/*===========================================================================*/

int Renderer::SetupAndAllocBitmaps(long ImageWidth, long ImageHeight)
{
BufferNode *NextBuf;
int PreviewStash = 0;

// check to see if anything has changed

if (Width != Opt->OutputImageWidth || Height != Opt->OutputImageHeight)
	{
	rFreePixelFragmentMap();
	if (Preview)
		{
		PreviewStash = 1;
		delete Preview;
		Preview = NULL;
		Pane = NULL;
		} // if
	while (Buffers)
		{
		NextBuf = Buffers->Next;
		delete Buffers;
		Buffers = NextBuf;
		} // if
	if (Rast)
		delete Rast;
	Rast = NULL;
	if (LastRast)
		delete LastRast;
	LastRast = NULL;
	Bitmap[2] = Bitmap[1] = Bitmap[0] = AABuf = TreeBitmap[2] = TreeBitmap[1] = TreeBitmap[0] = TreeAABuf = NULL;
	ZBuf = TreeZBuf = LatBuf = LonBuf = ElevBuf = ReflectionBuf = IllumBuf = SlopeBuf = AspectBuf = NormalBuf[2] = NormalBuf[1] = NormalBuf[0] = NULL;
	ObjectBuf = NULL;
	ExponentBuf = NULL;
	PlotBuf[2] = PlotBuf[1] = PlotBuf[0] = NULL;

	SetupWidth = SetupHeight = Width = Height = OSLowX = OSHighX = OSLowY = OSHighY = 0;
	ViewCamCopy = NULL;

	if (SetImageSize(FALSE))
		{
		if (AddBasicBufferNodes())
			{
			if (AllocateBuffers())
				{
				#ifdef WCS_TEST_EARLY_TERRAIN_INIT
				if (InitTerrainFactors())
				#endif // WCS_TEST_EARLY_TERRAIN_INIT
					{
					if (InitOncePerRender())
						{
						SetDrawOffsets();
						if (Master)
							{
							Master->SetPreview(PreviewStash);
							if (Master->GetPreview())
								{
								OpenPreview();
								if (Pane)
									Pane->Clear();
								else
									Master->SetPreview(0);
								} // if
							} // if

						return (1);
						} // if
					} // if
				} // if
			} // if
		} // if
	} // if
else
	{
	ClearBuffers();
	return (1);
	} // else

return (0);

} // Renderer::SetupAndAllocBitmaps

/*===========================================================================*/

int Renderer::SetImageSize(int InitForProcessingOnly)
{

#if (defined(WCS_RENDER_TILES) || defined(WCS_BUILD_RTX))
if (Opt->TilingEnabled)
	{
	RenderImageSegments = 1;
	XTiles = Opt->TilesX <= 1 ? 1: Opt->TilesX;
	YTiles = Opt->TilesY <= 1 ? 1: Opt->TilesY;
	} // if
else
#endif // defined(WCS_RENDER_TILES) || defined(WCS_BUILD_RTX)
	{
	if (InitForProcessingOnly)
		RenderImageSegments = 1;
	else
		RenderImageSegments = Opt->RenderImageSegments <= 1 ? 1: Opt->RenderImageSegments;
	XTiles = 1;
	YTiles = 1;
	} // else
if ((Width = Opt->OutputImageWidth / XTiles) > 0)
	{
	#ifdef WCS_BUILD_DEMO
	Width = min(Width, 640);
	#endif // WCS_BUILD_DEMO
	if ((Height = Opt->OutputImageHeight / max(RenderImageSegments, YTiles)) > 0)
		{
		#ifdef WCS_BUILD_DEMO
		Height = min(Height, 450);
		#endif // WCS_BUILD_DEMO
		OSLowX = - quickftol(Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_SIDEOVERSCAN].CurValue * Width);
		OSHighX = Width + quickftol(Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_SIDEOVERSCAN].CurValue * Width);
		OSLowY = - 10;
		OSHighY = Height + quickftol(Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_BOTTOMOVERSCAN].CurValue * Height * max(RenderImageSegments, YTiles));

		SetupWidth = Opt->CamSetupRenderWidth > 0 ? Opt->CamSetupRenderWidth: Width * XTiles;
		SetupHeight = Opt->CamSetupRenderHeight > 0 ? Opt->CamSetupRenderHeight: Height * max(RenderImageSegments, YTiles);

		#ifdef WCS_BUILD_DEMO
		// in case they've hacked the limits
		if (Width > 640)
			return (0);
		if (Height > 450)
			return (0);
		#endif // WCS_BUILD_DEMO

		return (1);
		} // if
	} // if

return (0);

} // Renderer::SetImageSize

/*===========================================================================*/

#ifdef WCS_TEST_EARLY_TERRAIN_INIT
int Renderer::InitTerrainFactors(void)
{

if (Opt->TerrainEnabled)
	{
	MaxFractalDepth = EffectsBase->GetMaxFractalDepth();

	// allocate all vertices and polygon edges for rendering up to MaxFractalDepth
	if (! AllocVertices())
		return (0);

	if (! DEMCue)
		{
		if (DEMCue = new RenderQ())
			{
			if (! DEMCue->FillList(DBase, 1))	// pass 0 = Draw, 1 = Render
				return (0);
			} // if
		else
			return (0);	
		} // if
	} // if

return (1);

} // Renderer::InitTerrainFactors
#endif // WCS_TEST_EARLY_TERRAIN_INIT

/*===========================================================================*/

int Renderer::InitOncePerRender(void)
{
double LatStepMeters, LonStepMeters;

// retore wide-open ZMin/ZMax values while generating Shadow Maps
if (ShadowMapInProgress)
	{
	// defaults taken from Camera.cpp, Camera::SetDefaults();
	MinimumZ = 0.0;
	MaximumZ = ConvertToMeters(10.0, WCS_USEFUL_UNIT_AU); // 10 AU
	} // if

// certain image objects such as celestial objects and textures need this global value
// It will be cleared to its original value in cleanup
if (! IsCamView && ! ShadowMapInProgress)
	ProjectBase->Interactive->SetActiveTimeAndRate(0.0, Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue);

if (! ShadowMapInProgress)
	ImageBase->InitRasterIDs();

FieldInterval = 1.0 / Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue;
if (Cam->FieldRender)
	FieldInterval *= .5;

if (Cam->AAPasses <= 0)
	Cam->AAPasses = 1;

StereoStart = StereoEnd = 0;
if (Cam->StereoCam)
	{
	if (IsCamView)
		{
		if (Cam->StereoPreviewChannel == WCS_CAMERA_STEREOCHANNEL_LEFT)
			StereoStart = StereoEnd = -1;
		else if (Cam->StereoPreviewChannel == WCS_CAMERA_STEREOCHANNEL_RIGHT)
			StereoStart = StereoEnd = 1;
		else
			{
			StereoStart = 0;
			StereoEnd = 0;
			} // if
		} // if
	else
		{
		if (Cam->StereoRenderChannel == WCS_CAMERA_STEREOCHANNEL_LEFT)
			StereoStart = StereoEnd = -1;
		else if (Cam->StereoRenderChannel == WCS_CAMERA_STEREOCHANNEL_RIGHT)
			StereoStart = StereoEnd = 1;
		else
			{
			StereoStart = -1;
			StereoEnd = 1;
			} // if
		} // else
	} // if

// this needs to be recalculated for each frame since it may itself be animated
MoBlurInterval = FieldInterval * Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_MOBLURPERCENT].CurValue / Cam->AAPasses;

if (! (DefaultPlanetOpt = (PlanetOpt *)EffectsBase->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, DBase)))
	return (0);
if (! (DefaultTerrainPar = (TerrainParamEffect *)EffectsBase->GetDefaultEffect(WCS_EFFECTSSUBCLASS_TERRAINPARAM, 1, DBase)))
	return (0);
if (! (DefaultEnvironment = (EnvironmentEffect *)EffectsBase->GetDefaultEffect(WCS_EFFECTSSUBCLASS_ENVIRONMENT, 1, DBase)))
	return (0);

ExagerateElevLines = DefaultPlanetOpt->EcoExageration;
PhongTerrain = DefaultTerrainPar->PhongShading;

// these variables will speed things up while rendering
RasterTAsExist = (char)(Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_RASTERTA] && EffectsBase->EnabledEffectExists(WCS_EFFECTSSUBCLASS_RASTERTA));
ShadowsExist = (char)(Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] && EffectsBase->EnabledEffectExists(WCS_EFFECTSSUBCLASS_SHADOW));
TerraffectorsExist = (char)(Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAFFECTOR] && EffectsBase->EnabledEffectExists(WCS_EFFECTSSUBCLASS_TERRAFFECTOR));
StreamsExist = (char)(Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM] && EffectsBase->EnabledEffectExists(WCS_EFFECTSSUBCLASS_STREAM));
LakesExist = (char)(Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE] && EffectsBase->EnabledEffectExists(WCS_EFFECTSSUBCLASS_LAKE));
EcosExist = (char)(Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM] && EffectsBase->EnabledEffectExists(WCS_EFFECTSSUBCLASS_ECOSYSTEM));
EcoRastersExist = (char)(EcosExist && EffectsBase->EcosystemBase.GeoRast);
SnowsExist = (char)(Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_SNOW] && EffectsBase->EnabledEffectExists(WCS_EFFECTSSUBCLASS_SNOW));
CmapsExist = (char)(Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_CMAP] && EffectsBase->EnabledEffectExists(WCS_EFFECTSSUBCLASS_CMAP));
VolumetricAtmospheresExist = (char)(Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE] && EffectsBase->AreThereVolumetricAtmospheres());
LightTexturesExist = (char)(Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT] && EffectsBase->AreThereLightTextures());
AtmoTexturesExist = (char)(Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE] && EffectsBase->AreThereAtmoTextures());
WaterExists = (LakesExist || StreamsExist);

// Backface culling should be disabled if overhead or planimetric
BackfaceCull = (EffectsBase->TerrainParamBase.BackfaceCull && ! (Cam->CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD || Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC));

// planimetric and orthographic cameras receive different texture treatment - no perspective warping
CamPlanOrtho = (Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC || Cam->Orthographic);

// for DEM rendering and foliage blending
DBase->GetMinDEMCellSizeMetersMaxCount(LatStepMeters, LonStepMeters, 100, ProjectBase); // we only sample 100 total DEMs instead of all of them
DEMCellSize = ZMergeDistance = min(LatStepMeters, LonStepMeters);	// Renderer scope, for pixel blending
ZMergeDistance /= 4.0;
FragmentDepth = Opt->FragmentDepth;

#ifndef WCS_TEST_EARLY_TERRAIN_INIT
if (Opt->TerrainEnabled)
	{
	MaxFractalDepth = EffectsBase->GetMaxFractalDepth();

	// allocate all vertices and polygon edges for rendering up to MaxFractalDepth
	if (! AllocVertices())
		return (0);

	if (! DEMCue)
		{
		if (DEMCue = new RenderQ())
			{
			if (! DEMCue->FillList(DBase, 1))	// pass 0 = Draw, 1 = Render
				return (0);
			} // if
		else
			return (0);	
		} // if
	// additional merge distance restriction added in VNS on 5/27/01
	if (MaxFractalDepth > 0)
		ZMergeDistance /= pow(2.0, MaxFractalDepth);
	} // if
#else // WCS_TEST_EARLY_TERRAIN_INIT
// additional merge distance restriction added in VNS on 5/27/01
if (MaxFractalDepth > 0)
	ZMergeDistance /= pow(2.0, MaxFractalDepth);
#endif // WCS_TEST_EARLY_TERRAIN_INIT

return (1);

} // Renderer::InitOncePerRender

/*===========================================================================*/

void Renderer::SetDrawOffsets(void)
{

if (AppBase->WinSys->InquireDisplayWidth() < Width)
	DrawOffsetX = (AppBase->WinSys->InquireDisplayWidth() - Width);
else
	DrawOffsetX = Opt->RenderOffsetX;

if (AppBase->WinSys->InquireDisplayHeight() < Height)
	DrawOffsetY = (AppBase->WinSys->InquireDisplayHeight() - Height);
else
	DrawOffsetY = Opt->RenderOffsetY;

} // Renderer::SetDrawOffsets

/*===========================================================================*/

void Renderer::SetInitialTexts(void)
{
char TextStr[256], FrameStr[64], FrameNumStr[32], MoreOutputEvents;

if (Master && Master->GetGUINativeWin())
	{
	Master->ClearAll();

	FrameStr[0] = FrameNumStr[0] = NULL;
	if (RenderImageSegments > 1)
		{
		sprintf(FrameNumStr, " %d Segs", RenderImageSegments);
		} // if
	else if (YTiles > 1 || XTiles > 1)
		{
		sprintf(FrameNumStr, " %d Tiles", YTiles * XTiles);
		} // if

	sprintf(FrameStr, "%f", Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue);
	TrimZeros(FrameStr);

	sprintf(TextStr, "%dx%d%s, %s Pixel Aspect", XTiles * (Opt->OutputImageWidth / XTiles),	max(RenderImageSegments, YTiles) * (Opt->OutputImageHeight / max(RenderImageSegments, YTiles)), FrameNumStr, FrameStr);

	Master->SetResText(TextStr);


	if (Opt->OutputImages())
		{
		MoreOutputEvents = 0;
		strcpy(TextStr, Opt->GetFirstOutputName(MoreOutputEvents));
		if (MoreOutputEvents)
			strcat(TextStr, "...");
		} // if
	else
		{
		strcpy(TextStr, "No Image Save");
		} // else

	Master->SetImageText(TextStr);

	if (Opt->TerrainEnabled)
		{
		switch (EffectsBase->TerrainParamBase.FractalMethod)
			{
			case WCS_FRACTALMETHOD_VARIABLE:
				{
				sprintf(TextStr, "Variable Fractal Depth, Fractal Depth %d", DefaultTerrainPar ? DefaultTerrainPar->FractalDepth: 0);
				break;
				} // variable
			case WCS_FRACTALMETHOD_CONSTANT:
				{
				sprintf(TextStr, "Constant Fractal Depth, Fractal Depth %d", DefaultTerrainPar ? DefaultTerrainPar->FractalDepth: 0);
				break;
				} // variable
			case WCS_FRACTALMETHOD_DEPTHMAPS:
				{
				sprintf(TextStr, "Fractal Depth Maps, Fractal Depth %d", DefaultTerrainPar ? DefaultTerrainPar->FractalDepth: 0);
				break;
				} // variable
			} // switch
		} // if
	else
		TextStr[0] = 0;

	Master->SetFractText(TextStr);

	if (Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].CurValue != Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].CurValue)
		{
		FramesToRender = 1 + quickftol(fabs(Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].CurValue
			- Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].CurValue) * 
			Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue + .5) / Opt->FrameStep;
		sprintf(TextStr, "%d frames", FramesToRender);
		Master->AnimInit(FramesToRender, TextStr);
		} // if rendering more than one frame 
	} // if

} // Renderer::SetInitialTexts

/*===========================================================================*/

int Renderer::AddBasicBufferNodes(void)
{
BufferNode *CurBuf;

// ALWAYS ASK FOR THE FIRST 10 IN THIS PRECISE ORDER or thumbnails will get whacky
if (CurBuf = Buffers = new BufferNode("ZBUF", WCS_RASTER_BANDSET_FLOAT))
	{
	// we know we aren't creating duplicate nodes since these are the first to be created
	// so we'll just call AddBufferNode on the last node created to save a bit of string comparing
	if (! (CurBuf = CurBuf->AddBufferNode("RED", WCS_RASTER_BANDSET_BYTE)))
		return (0);
	if (! (CurBuf = CurBuf->AddBufferNode("GREEN", WCS_RASTER_BANDSET_BYTE)))
		return (0);
	if (! (CurBuf = CurBuf->AddBufferNode("BLUE", WCS_RASTER_BANDSET_BYTE)))
		return (0);
	if (! (CurBuf = CurBuf->AddBufferNode("ANTIALIAS", WCS_RASTER_BANDSET_BYTE)))
		return (0);
	if (! (CurBuf = CurBuf->AddBufferNode("FOLIAGE ZBUF", WCS_RASTER_BANDSET_FLOAT)))
		return (0);
	if (! (CurBuf = CurBuf->AddBufferNode("FOLIAGE RED", WCS_RASTER_BANDSET_BYTE)))
		return (0);
	if (! (CurBuf = CurBuf->AddBufferNode("FOLIAGE GREEN", WCS_RASTER_BANDSET_BYTE)))
		return (0);
	if (! (CurBuf = CurBuf->AddBufferNode("FOLIAGE BLUE", WCS_RASTER_BANDSET_BYTE)))
		return (0);
	if (! (CurBuf = CurBuf->AddBufferNode("FOLIAGE ANTIALIAS", WCS_RASTER_BANDSET_BYTE)))
		return (0);
	if (! (CurBuf = CurBuf->AddBufferNode("RENDER FLAGS", WCS_RASTER_BANDSET_BYTE)))
		return (0);
	// Diagnostic buffers, etc
	if (Opt->RenderDiagnosticData)
		{
		if (! (CurBuf = CurBuf->AddBufferNode("LATITUDE", WCS_RASTER_BANDSET_FLOAT)))
			return (0);
		if (! (CurBuf = CurBuf->AddBufferNode("LONGITUDE", WCS_RASTER_BANDSET_FLOAT)))
			return (0);
		if (! (CurBuf = CurBuf->AddBufferNode("ELEVATION", WCS_RASTER_BANDSET_FLOAT)))
			return (0);
		if (! (CurBuf = CurBuf->AddBufferNode("RELATIVE ELEVATION", WCS_RASTER_BANDSET_FLOAT)))
			return (0);
		if (! (CurBuf = CurBuf->AddBufferNode("ILLUMINATION", WCS_RASTER_BANDSET_FLOAT)))
			return (0);
		if (! (CurBuf = CurBuf->AddBufferNode("SLOPE", WCS_RASTER_BANDSET_FLOAT)))
			return (0);
		if (! (CurBuf = CurBuf->AddBufferNode("ASPECT", WCS_RASTER_BANDSET_FLOAT)))
			return (0);
		if (! (CurBuf = CurBuf->AddBufferNode("REFLECTION", WCS_RASTER_BANDSET_FLOAT)))
			return (0);
		if (! (CurBuf = CurBuf->AddBufferNode("SURFACE NORMAL X", WCS_RASTER_BANDSET_FLOAT)))
			return (0);
		if (! (CurBuf = CurBuf->AddBufferNode("SURFACE NORMAL Y", WCS_RASTER_BANDSET_FLOAT)))
			return (0);
		if (! (CurBuf = CurBuf->AddBufferNode("SURFACE NORMAL Z", WCS_RASTER_BANDSET_FLOAT)))
			return (0);
		if (! (CurBuf = CurBuf->AddBufferNode("OBJECT", WCS_RASTER_BANDSET_FLOAT)))
			return (0);
		if (! (CurBuf = CurBuf->AddBufferNode("OBJECT TYPE", WCS_RASTER_BANDSET_BYTE)))
			return (0);
		} // if
	if (! ShadowMapInProgress)
		{
		if (Opt->FragmentRenderingEnabled)
			{
			if (! rAllocPixelFragmentMap())
				return (0);
			if (! (CurBuf = CurBuf->AddBufferNode("RGB EXPONENT", WCS_RASTER_BANDSET_SHORT)))
				return (0);
			} // if
		} // if
	} // if
else
	return (0);

return (1);

} // Renderer::AddBasicBufferNodes

/*===========================================================================*/

int Renderer::AllocateBuffers(void)
{
BufferNode *CurBuf = Buffers;

if ((Rast = new Raster()) && (LastRast = new Raster()))
	{
	// set up raster size from render options
	Rast->Rows = Height;
	Rast->Cols = Width;

	// allocate thumbnail
	if (Rast->AllocThumbnail() && LastRast->AllocThumbnail())
		{
		if (Rast->AllocRowZip())
			{
			// allocate buffers requested
			while (CurBuf)
				{
				if (CurBuf->Name[0])
					{
					if (! Rast->AllocBuffer(CurBuf))
						return (0);
					} // if
				CurBuf = CurBuf->Next;
				} // while

			// assign buffer ptrs - at least all the ones the renderer needs to know about
			Bitmap[0] = (unsigned char *)Buffers->FindBuffer("RED",					WCS_RASTER_BANDSET_BYTE);
			Bitmap[1] = (unsigned char *)Buffers->FindBuffer("GREEN",				WCS_RASTER_BANDSET_BYTE);
			Bitmap[2] = (unsigned char *)Buffers->FindBuffer("BLUE",				WCS_RASTER_BANDSET_BYTE);
			AABuf = (unsigned char *)Buffers->FindBuffer("ANTIALIAS",				WCS_RASTER_BANDSET_BYTE);
			CloudBitmap[0] = TreeBitmap[0] = 
				(unsigned char *)Buffers->FindBuffer("FOLIAGE RED",					WCS_RASTER_BANDSET_BYTE);
			CloudBitmap[1] = TreeBitmap[1] = 
				(unsigned char *)Buffers->FindBuffer("FOLIAGE GREEN",				WCS_RASTER_BANDSET_BYTE);
			CloudBitmap[2] = TreeBitmap[2] = 
				(unsigned char *)Buffers->FindBuffer("FOLIAGE BLUE",				WCS_RASTER_BANDSET_BYTE);
			CloudAABuf = TreeAABuf = 
				(unsigned char *)Buffers->FindBuffer("FOLIAGE ANTIALIAS",			WCS_RASTER_BANDSET_BYTE);
			FlagBuf = (unsigned char *)Buffers->FindBuffer("RENDER FLAGS",			WCS_RASTER_BANDSET_BYTE);
			ObjTypeBuf = (unsigned char *)Buffers->FindBuffer("OBJECT TYPE",		WCS_RASTER_BANDSET_BYTE);

			ZBuf = (float *)Buffers->FindBuffer("ZBUF",								WCS_RASTER_BANDSET_FLOAT);
			CloudZBuf = TreeZBuf = (float *)Buffers->FindBuffer("FOLIAGE ZBUF",		WCS_RASTER_BANDSET_FLOAT);
			LatBuf = (float *)Buffers->FindBuffer("LATITUDE",						WCS_RASTER_BANDSET_FLOAT);
			LonBuf = (float *)Buffers->FindBuffer("LONGITUDE",						WCS_RASTER_BANDSET_FLOAT);
			ElevBuf = (float *)Buffers->FindBuffer("ELEVATION",						WCS_RASTER_BANDSET_FLOAT);
			RelElBuf = (float *)Buffers->FindBuffer("RELATIVE ELEVATION",			WCS_RASTER_BANDSET_FLOAT);
			ReflectionBuf = (float *)Buffers->FindBuffer("REFLECTION",				WCS_RASTER_BANDSET_FLOAT);
			IllumBuf = (float *)Buffers->FindBuffer("ILLUMINATION",					WCS_RASTER_BANDSET_FLOAT);
			SlopeBuf = (float *)Buffers->FindBuffer("SLOPE",						WCS_RASTER_BANDSET_FLOAT);
			AspectBuf = (float *)Buffers->FindBuffer("ASPECT",						WCS_RASTER_BANDSET_FLOAT);
			NormalBuf[0] = (float *)Buffers->FindBuffer("SURFACE NORMAL X",			WCS_RASTER_BANDSET_FLOAT);
			NormalBuf[1] = (float *)Buffers->FindBuffer("SURFACE NORMAL Y",			WCS_RASTER_BANDSET_FLOAT);
			NormalBuf[2] = (float *)Buffers->FindBuffer("SURFACE NORMAL Z",			WCS_RASTER_BANDSET_FLOAT);
			ExponentBuf = (unsigned short *)Buffers->FindBuffer("RGB EXPONENT",		WCS_RASTER_BANDSET_SHORT);

			ObjectBuf = (RasterAnimHost **)Buffers->FindBuffer("OBJECT",			WCS_RASTER_BANDSET_FLOAT);

			ClearBuffers();
			return (1);
			} // if rowzip array
		} // if thumbnails
	} // if rasters

return (0);

} // Renderer::AllocateBuffers

/*===========================================================================*/

void Renderer::ClearBuffers(void)
{
BufferNode *CurBuf = Buffers;

while (CurBuf)
	{
	if (CurBuf->Buffer)
		{
		if (CurBuf->Type == WCS_RASTER_BANDSET_BYTE)
			Rast->ClearByteBand(CurBuf->Index);
		else if (CurBuf->Type == WCS_RASTER_BANDSET_SHORT)
			Rast->ClearShortBand(CurBuf->Index);
		else if (CurBuf->Type == WCS_RASTER_BANDSET_FLOAT)
			{
			if (CurBuf->Buffer == ZBuf || CurBuf->Buffer == TreeZBuf)
				Rast->ClearFloatBandValue(CurBuf->Index, FLT_MAX);
			else
				Rast->ClearFloatBand(CurBuf->Index);
			} // else if
		} // if
	CurBuf = CurBuf->Next;
	} // while

FragMemFailed = 0;
if (rPixelBlock)
	{
	// they've already been allocated once so don't need to worry about failing
	// This is called here so that the fragment chains are NULL terminated after the first entry
	// and the Coverage value and UsedFrags are reset
	rPixelBlock->AllocFirstFrags(Height * Width);
	#ifdef WCS_BUILD_DEMO
	PlotDemoRainbowFrags();
	#endif // WCS_BUILD_DEMO
	} // if

} // Renderer::ClearBuffers

/*===========================================================================*/

int Renderer::ClearCloudBuffers(void)
{
BufferNode *CurBuf;

// if you change which buffers are cleared here be sure to change which
// buffers are called CloudBitmap[]... in AllocateBuffers()
if (CurBuf = Buffers->FindBufferNode("FOLIAGE RED", WCS_RASTER_BANDSET_BYTE))
	Rast->ClearByteBand(CurBuf->Index);
else
	return (0);
if (CurBuf = Buffers->FindBufferNode("FOLIAGE GREEN", WCS_RASTER_BANDSET_BYTE))
	Rast->ClearByteBand(CurBuf->Index);
else
	return (0);
if (CurBuf = Buffers->FindBufferNode("FOLIAGE BLUE", WCS_RASTER_BANDSET_BYTE))
	Rast->ClearByteBand(CurBuf->Index);
else
	return (0);
if (CurBuf = Buffers->FindBufferNode("FOLIAGE ANTIALIAS", WCS_RASTER_BANDSET_BYTE))
	Rast->ClearByteBand(CurBuf->Index);
else
	return (0);
if (CurBuf = Buffers->FindBufferNode("FOLIAGE ZBUF", WCS_RASTER_BANDSET_FLOAT))
	Rast->ClearFloatBandValue(CurBuf->Index, FLT_MAX);
else
	return (0);

return (1);

} // Renderer::ClearCloudBuffers

/*===========================================================================*/

int Renderer::RenderFrame(double CurTime, long FrameNumber)
{
double BaseTime = CurTime;
int Success = 1, MapCollapsed, TerrainRendered, FirstPass = 1;
unsigned long  StashCurSteps, StashMaxSteps;
time_t StashStartSecs;
long RenderSeg, AAPass, AAPasses, Field, XTile, YTile, RenderPanels;
Renderer *ShadowRend;
RasterBounds RBounds;
#ifdef WCS_BUILD_DEMO
unsigned char MessyRed, MessyGreen, MessyBlue, OMessyRed;
long MessyRows, FirstMessyRow, MessyRow, MessyCol, MessyRowCt, MessyZip;
time_t MessyTime;
double MessyStep;
#endif // WCS_BUILD_DEMO
char FrameNumStr[200], AnotherStr[32], StashText[200];

//ttFoliage.Start(&Renderer::PlotFoliageReverse, &ttFoliageW);

#ifdef WCS_BUILD_DEMO
// in case they've hacked the limits
if (CurTime > 5.0)
	CurTime = 5.0;
if (FrameNumber > 150)
	FrameNumber = 150;
#endif // WCS_BUILD_DEMO

GetTime(StartSecs);
GetTime(FrameSecs);
VNA.Initialize();

// debugging stuff for forestry features
#ifdef WCS_FORESTRY_WIZARD
#ifdef WCS_BUILD_GARY
{
int iii, jjj;
AreaRendered = StemsRendered = AvgStemsRendered = PolysRendered = SumPRN = PRNAbove = PRNBelow = 0.0;
PRNCt = TinyPRN = 0;
for (iii = 0; iii < 4; iii ++)
	{
	GroupStemsRendered[0] = GroupStemsRendered[1] = GroupStemsRendered[2] = GroupStemsRendered[3] = 0.0;
	for (jjj = 0; jjj < 8; jjj ++)
		ImageStemsRendered[iii][jjj] = 0.0;
	} // for
} // limited scope
#endif // WCS_BUILD_GARY
#endif // WCS_FORESTRY_WIZARD


AAPasses = Opt->MultiPassAAEnabled ? Cam->AAPasses: 1;
NumSegments = RenderImageSegments < 1 ? 1: RenderImageSegments;
NumPanels = Cam->PanoPanels < 1 ? 1: Cam->PanoPanels;
if (Cam->PanoPanels > 1 && IsCamView)
	{
	RenderPanels = UserMessageYN("Panoramic Camera Preview", "Preview each panel of this Panoramic Camera?") ? Cam->PanoPanels: 1;
	} // if
else
	RenderPanels = Cam->PanoPanels;

for (StereoSide = StereoStart; StereoSide <= StereoEnd; StereoSide += 2)
	{
	for (PanoPanel = 0; Success && PanoPanel < RenderPanels; PanoPanel ++)
		{
		for (Field = 0; Success && (Field < (Cam->FieldRender ? 2: 1)); Field ++)
			{
			CurTime = Field * FieldInterval + BaseTime;
			for (AAPass = 0; Success && (AAPass < AAPasses); AAPass ++)
				{
				// put current frame string in RenderControlGUI
				if (Master)
					{
					sprintf(FrameNumStr, "%1d", FrameNumber);
					if (Cam->FieldRender)
						strcat(FrameNumStr, Field ? "B": "A");
					if (AAPasses > 1)
						{
						sprintf(AnotherStr, ", AA %d/%d", AAPass + 1, AAPasses);
						strcat(FrameNumStr, AnotherStr);
						} // if
					if (Cam->PanoPanels > 1)
						{
						sprintf(AnotherStr, ", Pano %d/%d", PanoPanel + 1, Cam->PanoPanels);
						strcat(FrameNumStr, AnotherStr);
						} // if
					Master->SetFrameNum(FrameNumStr);
					} // if
				// motion blur
				if (Cam->MotionBlur && AAPass > 0)
					{
					CurTime += MoBlurInterval;
					} // if
				// initialize everything for this frame
				if (InitFrame(CurTime, FrameNumber, ! IsCamView))
					{
					MapCollapsed = 0;
					TrueCenterX = Cam->CenterX + DefaultAAKernX[AAPass];
					TrueCenterY = Cam->CenterY + DefaultAAKernY[AAPass];

					// Shadows
					if (Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT] && 
						((Opt->ObjectShadowsEnabled  && Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D]) || 
						(Opt->CloudShadowsEnabled && Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_CLOUD]) || 
						(Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] && Opt->TerrainEnabled)))
						{
						if (ShadowRend = new Renderer())
							{
							if (Master)
								Master->GetFrameSetup(StashCurSteps, StashMaxSteps, StashStartSecs, StashText);
							#ifdef WCS_VECPOLY_EFFECTS
							Success = ShadowRend->InitForShadows(AppBase, EffectsBase, ImageBase, 
								DBase, ProjectBase, LocalLog, Master, Opt, this, FrameNumber, CurTime);
							#else // WCS_VECPOLY_EFFECTS
							Success = ShadowRend->InitForShadows(AppBase, EffectsBase, ImageBase, 
								DBase, ProjectBase, LocalLog, Master, Opt, this, FrameNumber);
							#endif // WCS_VECPOLY_EFFECTS
							if (Master)
								{
								Master->SetRenderer(this);
								Master->SetPreview(Pane ? 1: 0);
								if (strlen(StashText) > 1)  //lint !e645
									{
									StashText[strlen(StashText) - 1] = 0;
									Master->RestoreFrameSetup(StashCurSteps, StashMaxSteps, StashStartSecs, &StashText[1]);
									} // if
								else
									Master->SetFrameText("");
								} // if
							delete ShadowRend;
							if (! Success)
								break;
							// restore foliage, fence and 3d object vertex non-rendered status
							EffectsBase->InitBaseToRender();
							} // if
						} // if
					
					// render each segment
					for (RenderSeg = 0; Success && (RenderSeg < RenderImageSegments); RenderSeg ++)
						{
						// restore foliage, fence and 3d object vertex non-rendered status
						Segment = RendData.Segment = RenderSeg;

						for (YTile = 0; Success && (YTile < YTiles); YTile ++)
							{
							for (XTile = 0; Success && (XTile < XTiles); XTile ++)
								{
								// update current frame string in RenderControlGUI in case using segments or tiles
								if (Master)
									{
									sprintf(FrameNumStr, "Frame %1d", FrameNumber);
									if (Cam->FieldRender)
										strcat(FrameNumStr, Field ? "B": "A");
									if (AAPasses > 1)
										{
										sprintf(AnotherStr, ", AA %d/%d", AAPass + 1, AAPasses);
										strcat(FrameNumStr, AnotherStr);
										} // if
									if (Cam->PanoPanels > 1)
										{
										sprintf(AnotherStr, ", Pano %d/%d", PanoPanel + 1, Cam->PanoPanels);
										strcat(FrameNumStr, AnotherStr);
										} // if
									if (RenderImageSegments > 1)
										{
										sprintf(AnotherStr, ", Seg %d/%d", RenderSeg + 1, RenderImageSegments);
										strcat(FrameNumStr, AnotherStr);
										} // if
									if (XTiles > 1)
										{
										sprintf(AnotherStr, ", X Tile %d/%d", XTile + 1, XTiles);
										strcat(FrameNumStr, AnotherStr);
										} // if
									if (YTiles > 1)
										{
										sprintf(AnotherStr, ", Y Tile %d/%d", YTile + 1, YTiles);
										strcat(FrameNumStr, AnotherStr);
										} // if
									sprintf(TileStr, "Tile %d/%d", YTile * XTiles + XTile + 1, XTiles * YTiles);
									Master->FrameTextInit(FrameNumStr);
									if (GlobalApp->WinSys->InquireMinimized())
										{
										sprintf(StashText, "%s: %s", APP_TLA, FrameNumStr);
										GlobalApp->WinSys->DisplayRenderTitle(StashText);
										} // if
									else
										{
										GlobalApp->WinSys->ClearRenderTitle();
										} // else
									} // if

								if (RenderSeg > 0 || XTile > 0 || YTile > 0)
									EffectsBase->InitBaseToRender();
								SegmentOffsetY = Height * max(RenderSeg, YTile);
								SegmentOffsetUnityY = (double)max(RenderSeg, YTile) / max(RenderImageSegments, YTiles);
								SegmentOffsetX = Width * max(PanoPanel, XTile);
								SegmentOffsetUnityX = (double)max(PanoPanel, XTile) / max(Cam->PanoPanels, XTiles);
								// adjust CenterY for render segment
								Cam->CenterY = TrueCenterY - max(RenderSeg, YTile) * Height;
								Cam->CenterX = TrueCenterX - XTile * Width;
								
								ClearBuffers();
								PlotProjectedNULLRegionFrags();
								TerrainRendered = 0;

								#ifdef WCS_BUILD_DEMO
								MessyStep = (double)Height / Width;
								MessyRows = Height / 10;
								GetTime(MessyTime);
								RendRand.Seed64(MessyTime, MessyTime);
								OMessyRed = (unsigned char)(RendRand.GenPRN() * 255);

								for (MessyCol = 0; MessyCol < Width; MessyCol ++)
									{
									MessyRed = OMessyRed;
									MessyGreen = OMessyRed + 128;
									MessyBlue = OMessyRed + 255;
									FirstMessyRow = (long)(MessyCol * MessyStep - MessyRows / 2);
									for (MessyRow = FirstMessyRow, MessyRowCt = 0; MessyRowCt < max(MessyRows, 10) && MessyRow < Height; MessyRowCt ++, MessyRow ++)
										{
										MessyRed += 10;
										MessyGreen += 15;
										MessyBlue += 20;
										if (MessyRow < 0)
											continue;
										MessyZip = MessyRow * Width + MessyCol;
										AABuf[MessyZip] = 128;
										ZBuf[MessyZip] = 0.0f;
										Bitmap[0][MessyZip] = MessyRed;
										Bitmap[1][MessyZip] = MessyGreen;
										Bitmap[2][MessyZip] = MessyBlue;
										} // for
									} // for
								#endif // WCS_BUILD_DEMO

								if (Pane)
									Pane->Clear();
								PlotFromFrags = rPixelFragMap ? 1: 0;

								#ifdef WCS_BUILD_DEMO
								// in case they've hacked the limits
								if (Width > 640 || Height > 450 || CurTime > 5.0 || FrameNumber > 150)
									return (0);
								#endif // WCS_BUILD_DEMO

								// terrain and 3d objects
								if (Opt->TerrainEnabled || Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D])
									{
									if (Opt->TerrainEnabled)
										{
										// load foliage
										if (Opt->FoliageEnabled || Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE])
											{
											if (! (Success = ImageBase->InitFoliageRasters(RenderTime, Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue, 
												Opt->FoliageEnabled, Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE], 
												Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM], 
												Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE], this)))
												break;
											} // if
										// render terrain, water, foliage, 3D objects
										SetMultiBuf(1);
										#ifdef WCS_VECPOLY_EFFECTS
										Success = RenderTerrain(FrameNumber, Field, CurTime);
										#else // WCS_VECPOLY_EFFECTS
										Success = RenderTerrain(FrameNumber, Field);
										#endif // WCS_VECPOLY_EFFECTS
										} // if
									// any 3d objects not over terrain
									if (Success && Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D])
										{
										SetMultiBuf(1);
										Success = RenderStrayObjects();
										} // if
									if (! rPixelFragMap)
										MergeZBufRGBPanels(Bitmap, ZBuf, AABuf, TreeBitmap, TreeZBuf, TreeAABuf);
									SetMultiBuf(0);
									if (! Success)
										break;
									// clear short-longevity foliage from memory
									ImageBase->ClearImageRasters(WCS_RASTER_LONGEVITY_SHORT);
									TerrainRendered = 1;
									} // if


								#ifdef WCS_BUILD_DEMO
								// In case they've hacked the limits in 3 or 4 places before this it's time to seriously mess with them!
								// It may be significant that this nefarious scheme was hatched on Friday, October 13.
								if (Width > 645 || Height > 454 || CurTime > 5.11 || FrameNumber > 151)
									AppMem_Alloc((unsigned long)(RendRand.GenPRN() * 25000000) + 35000000, 0);
								#endif // WCS_BUILD_DEMO

								// vectors
								if (FirstPass || ! VectorOutput)
									{
									if (Opt->VectorsEnabled)
										{
										ImageOutputEvent *CurEvent;
										// Prep for vectors
										for (CurEvent = Opt->OutputEvents; CurEvent; CurEvent = CurEvent->Next)
											{
											if (CurEvent->Enabled)
												{
												CurEvent->StartVectorFrame(Width * max(Cam->PanoPanels, XTiles), Height * max(RenderImageSegments, YTiles), FrameNumber);
												} // if
											} // for
										SetMultiBuf(1);
										if (VectorOutput)
											{
											Cam->CenterY = TrueCenterY;
											Cam->CenterX = TrueCenterX;
											} // if
										Success = RenderVectors(Width, Height, Width * max(Cam->PanoPanels, XTiles), Height * max(RenderImageSegments, YTiles), rPixelFragMap);
										if (VectorOutput)
											{
											Cam->CenterY = TrueCenterY - max(RenderSeg, YTile) * (double)Height;
											Cam->CenterX = TrueCenterX - XTile * (double)Width;
											} // if
										if (! rPixelFragMap)
											MergeZBufRGBPanels(Bitmap, ZBuf, AABuf, TreeBitmap, TreeZBuf, TreeAABuf);
										SetMultiBuf(0);
										// Cleanup for vectors
										for (CurEvent = Opt->OutputEvents; CurEvent; CurEvent = CurEvent->Next)
											{
											if (CurEvent->Enabled)
												{
												CurEvent->EndVectorFrame();
												} // if
											} // for
										if (! Success)
											break;
										} // if
									} // if
									
								#ifdef WCS_BUILD_DEMO
								// In case they've hacked the limits in 3 or 4 places before this it's time to seriously mess with them!
								// It may be significant that this nefarious scheme was hatched on Friday, October 13.
								if (Width > 641 || Height > 453 || CurTime > 5.1 || FrameNumber > 152)
									AppMem_Alloc((unsigned long)(RendRand.GenPRN() * 20000000) + 30000000, 0);
								#endif // WCS_BUILD_DEMO

								// clouds
								if (Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_CLOUD])
									{
									SetMultiBuf(1);
									Success = RenderClouds(NULL);	// NULL means render all cloud models
									if (! rPixelFragMap)
										MergeZBufRGBPanels(Bitmap, ZBuf, AABuf, CloudBitmap, CloudZBuf, CloudAABuf);
									SetMultiBuf(0);
									if (! Success)
										break;
									} // if
								SetMultiBuf(1);
								if (! rPixelFragMap)
									ClearCloudBuffers();

								ClearProjectedNULLRegionFrags();

								// camera background image
								if (Cam->BackgroundImageEnabled)
									{
									Success = RenderBackground();
									if (! Success)
										{
										if (! rPixelFragMap)
											MergeZBufRGBPanels(Bitmap, ZBuf, AABuf, CloudBitmap, CloudZBuf, CloudAABuf);
										SetMultiBuf(0);
										break;
										} // if
									} // if

								#ifdef WCS_BUILD_DEMO
								// In case they've hacked the limits
								if (Width > 643 || Height > 451 || CurTime > 5.05 || FrameNumber > 153)
									return (0);
								#endif // WCS_BUILD_DEMO

								// starlight, star bright
								if (Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_STARFIELD])
									{
									Success = RenderStars();
									if (! Success)
										{
										if (! rPixelFragMap)
											MergeZBufRGBPanels(Bitmap, ZBuf, AABuf, CloudBitmap, CloudZBuf, CloudAABuf);
										SetMultiBuf(0);
										break;
										} // if
									} // if

								// celestial objects
								if (Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_CELESTIAL])
									{
									Success = RenderCelestial();
									if (! Success)
										{
										if (! rPixelFragMap)
											MergeZBufRGBPanels(Bitmap, ZBuf, AABuf, CloudBitmap, CloudZBuf, CloudAABuf);
										SetMultiBuf(0);
										break;
										} // if
									} // if

								// skies
								if (Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_SKY])
									{
									Success = RenderSky();
									if (! Success)
										{
										if (! rPixelFragMap)
											MergeZBufRGBPanels(Bitmap, ZBuf, AABuf, CloudBitmap, CloudZBuf, CloudAABuf);
										SetMultiBuf(0);
										break;
										} // if
									} // if
								if (! rPixelFragMap)
									MergeZBufRGBPanels(Bitmap, ZBuf, AABuf, CloudBitmap, CloudZBuf, CloudAABuf);
								SetMultiBuf(0);

								// pre-reflection post process - used for compositing
								if (Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_POSTPROC])
									{
									if (! (Success = RenderPostProc(TRUE, NULL, NULL, FrameNumber)))
										break;
									} // if

								// apply water transparency
								if (TerrainRendered)
									{
									if (rPixelFragMap)
										{
										rPixelFragMap->RefractMap(rPixelFragMap, Width * Height);
										if (Pane)
											DrawPreview();
										} // if
									} // if

								if (Opt->VolumetricsEnabled)
									{
									if (! (Success = RenderVolumetrics(1)))
										break;
									} // if

								if (rPixelFragMap && Opt->ReflectionsEnabled && (Opt->ReflectionType == WCS_REFLECTIONSTYLE_BEAMTRACE || 
									Opt->ReflectionType == WCS_REFLECTIONSTYLE_ZONESAMPLED))
									{
									CollapseMap();
									} // if

								// reflections
								if (Opt->ReflectionsEnabled)
									{
									if (! (Success = RenderReflections()))
										break;
									} // if

								if (Opt->VolumetricsEnabled)
									{
									if (! (Success = RenderVolumetrics(0)))
										break;
									} // if

								if (rPixelFragMap)
									{
									// reflections have altered pixel colors
									CollapseMap();
									MapCollapsed = 1;
									} // if
								PlotFromFrags = 0;

								// depth of field
								if (Opt->DepthOfFieldEnabled)
									{
									if (! (Success = RenderDepthOfField()))
										break;
									} // if

								// camera box filter
								if (Cam->BoxFilter)
									{
									if (! (Success = RenderBoxFilter()))
										break;
									} // if

								// set up geo coords for savers - either from Exporter or from unprojecting image corners
								if (Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
									{
									if (Exporter && Exporter->RBounds.CoordsValid)
										{
										RBounds = Exporter->RBounds;
										} // if
									else
										{
										SetRBounds(&RBounds, Width, Height, Cam->PanoPanels, 
											RenderImageSegments, Opt->ConcatenateTiles);
										} // else
									} // if

								// save image before post process in case there are events that need to be saved now
								if (Success)
									{
									if (Master)
										Master->SetProcText("Saving Image");
									Success = Opt->SaveImage(this, Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC ? &RBounds: NULL, Buffers, Width, Height, FrameNumber, StereoSide, PanoPanel, Field, AAPass, RenderSeg, XTile, YTile,
										Cam->PanoPanels, (Cam->FieldRender ? 2: 1), AAPasses, RenderImageSegments, XTiles, YTiles, Cam->FieldRenderPriority, TRUE);
									if (Master)
										Master->ProcClear();
									} // if

								// post process
								if (Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_POSTPROC])
									{
									if (! (Success = RenderPostProc(FALSE, NULL, NULL, FrameNumber)))
										break;
									} // if

								// clear short-longevity image objects from memory
								ImageBase->ClearImageRasters(WCS_RASTER_LONGEVITY_SHORT);

								// save image
								if (Success)
									{
									if (Master)
										Master->SetProcText("Saving Image");
									gEXIF.CreateEXIF(this);
									Success = Opt->SaveImage(this, Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC ? &RBounds: NULL, Buffers, Width, Height, FrameNumber, StereoSide, PanoPanel, Field, AAPass, RenderSeg, XTile, YTile,
										Cam->PanoPanels, (Cam->FieldRender ? 2: 1), AAPasses, RenderImageSegments, XTiles, YTiles, Cam->FieldRenderPriority, FALSE);
									if (Master)
										Master->ProcClear();
									} // if
								} // for XTile
							} // for YTile
						} // for Renderseg
					if (rPixelFragMap && ! MapCollapsed)
						{
						CollapseMap();
						} // if
					} // if
				} // for AAPass
			} // for Field
		} // for PanoPanel
	} // for StereoSide

Buffers->RemoveTempFiles(Opt, FrameNumber);

#ifdef WCS_BUILD_DEMO
// In case they've hacked the limits in 3 or 4 places before this it's time to seriously mess with them!
// It may be significant that this nefarious scheme was hatched on Friday, October 13.
if (Width > 644 || Height > 452 || CurTime > 5.23 || FrameNumber > 154)
	AppMem_Alloc((unsigned long)(RendRand.GenPRN() * 22000000) + 32000000, 0);
#endif // WCS_BUILD_DEMO

FramesRendered ++;
GetTime(NowTime); // Time for frame
if (Master)
	{
	// unlikely that an single frame will take longer than 2^32 seconds, so casting down from possibly 64-bit time_t not harmful
	Master->StashFrame((unsigned long)(NowTime - FrameSecs));
	Master->AnimUpdate(FramesRendered);
	} // if

//GlobalApp->WinSys->DoBeep();

VNA.FreeAll();

return (Success);

} // Renderer::RenderFrame

/*===========================================================================*/

int Renderer::InitFrame(double CurTime, long FrameNumber, int SetTime)
{
int Success = 1;

if (SetTime)
	{
	// set time value
	ProjectBase->Interactive->SetActiveTime(CurTime);

	// update application by time
	AppBase->UpdateProjectByTime();
	} // if not from CamView

// set all sorts of values needed by renderer
RenderTime = CurTime;	// RenderTime is Renderer scope for passing to wave evaluators

#ifdef WCS_BUILD_DEMO
Cam->CenterX = .5 * SetupWidth;
Cam->CenterY = .5 * SetupHeight;
#else
Cam->CenterX = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX].CurValue * SetupWidth;
Cam->CenterY = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY].CurValue * SetupHeight;
#endif // WCS_BUILD_DEMO
// this corrects a View's limited region offsets
Cam->CenterX += (Opt->SetupOffsetX - Opt->RenderOffsetX);
Cam->CenterY += (Opt->SetupOffsetY - Opt->RenderOffsetY);


EarthLatScaleMeters = LatScale(EffectsBase->GetPlanetRadius());
TexRefLat = ProjectBase->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y);
TexRefLon = ProjectBase->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X);
TexRefElev = ProjectBase->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Z);
RefLonScaleMeters = EarthLatScaleMeters * cos(TexRefLat * PiOver180);
// these will be overridden in Renderer::InitOncePerRender if we are Shadowmapping
MinimumZ = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_ZMIN].CurValue;
MaximumZ = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_ZMAX].CurValue;

// these will be used to adjust elevations in ecosystems, snow, lakes and terraffectors.
// They must be set before calling EffectsBase->InitFrameToRender()
ElevDatum = DefaultPlanetOpt->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue;
Exageration = DefaultPlanetOpt->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].CurValue;
PlanetRad = EffectsBase->GetPlanetRadius();
EarthRotation = DefaultPlanetOpt->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_ROTATION].CurValue;
QMax = PlanetRad + Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue;
QMax = sqrt(QMax * QMax - PlanetRad * PlanetRad);
RenderBathos = (char)(Opt->FragmentRenderingEnabled && Opt->TransparentWaterEnabled && EffectsBase->AreThereOpticallyTransparentEffects());

RendData.InitFrameToRender(this);
Success = EffectsBase->InitFrameToRender(&RendData);
Substances = RendData.Substances;

CenterPixelSize = ComputeCenterPixelSize();
if (CenterPixelSize <= 0.0)
	{
	UserMessageOK("Initialize Frame", "Bad values. Center pixel size was computed as 0. Rendering aborted.");
	Success = 0;
	} // if
if (RealTimeFoliageWrite && ! TexRefVert)
	{
	if (TexRefVert = new VertexDEM())
		{
		TexRefVert->Lat = TexRefLat;
		TexRefVert->Lon = TexRefLon;
		TexRefVert->Elev = TexRefElev;
		#ifdef WCS_BUILD_VNS
		DefCoords->DegToCart(TexRefVert);
		#else // WCS_BUILD_VNS
		TexRefVert->DegToCart(PlanetRad);
		#endif // WCS_BUILD_VNS
		} // if
	else
		RealTimeFoliageWrite = NULL;
	} // if

return (Success);

} // Renderer::InitFrame

/*===========================================================================*/

void Renderer::PlotDemoRainbowFrags(void)
{
unsigned char MessyRed, MessyGreen, MessyBlue, OMessyRed;
long MessyRows, FirstMessyRow, MessyRow, MessyCol, MessyRowCt, MessyZip;
time_t MessyTime;
double MessyStep, PixRGB[3], DummyDbl, DummyNml[3];
rPixelFragment *PixFrag;

DummyDbl = DummyNml[0] = DummyNml[1] = DummyNml[2] = 0.0;

if (rPixelBlock && rPixelFragMap)
	{
	MessyStep = (double)Height / Width;
	MessyRows = Height / 10;
	GetTime(MessyTime);
	// casting from possibly 64-bit time_t to 32-bit not harmful as all we need are some unpredictable bits
	RendRand.Seed64((unsigned long)MessyTime, (unsigned long)MessyTime);
	OMessyRed = (unsigned char)(RendRand.GenPRN() * 255);

	for (MessyCol = 0; MessyCol < Width; MessyCol ++)
		{
		MessyRed = OMessyRed;
		MessyGreen = OMessyRed + 128;
		MessyBlue = OMessyRed + 255;
		FirstMessyRow = (long)(MessyCol * MessyStep - MessyRows / 2);  //lint !e653
		for (MessyRow = FirstMessyRow, MessyRowCt = 0; MessyRowCt < max(MessyRows, 10) && MessyRow < Height; MessyRowCt ++, MessyRow ++)
			{
			MessyRed += 10;
			MessyGreen += 15;
			MessyBlue += 20;
			if (MessyRow < 0)
				continue;
			MessyZip = MessyRow * Width + MessyCol;
			PixRGB[0] = MessyRed * (1.0 / 255.0);
			PixRGB[1] = MessyGreen * (1.0 / 255.0);
			PixRGB[2] = MessyBlue * (1.0 / 255.0);
			if (PixFrag = rPixelFragMap[MessyZip].PlotPixel(rPixelBlock, 0.0f, (unsigned char)128, ~0UL, ~0UL, 10, 
				WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_TERRAIN))
				{
				PixFrag->PlotPixel(rPixelBlock, PixRGB, DummyDbl, DummyNml);
				// DrawOffsetX, DrawOffsetY may not yet be calculated so best not to plot at this time
				//ScreenPixelPlotFragments(&rPixelFragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);
				} // if
			} // for
		} // for
	} // if

} // Renderer::PlotDemoRainbowFrags

/*===========================================================================*/

void Renderer::PlotProjectedNULLRegionFrags(void)
{
double PixRGB[3], DummyDbl, DummyNml[3];
rPixelFragment *PixFrag;
VertexDEM Vert;
long Row, Col, Zip;

PixRGB[0] = 195.0 / 255.0;
PixRGB[1] = 172.0 / 255.0;
PixRGB[2] = 128.0 / 255.0;
DummyDbl = DummyNml[2] = DummyNml[1] = DummyNml[0] = 0.0;

if (Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && Cam->Projected && Cam->Coords)
	{
	if (rPixelBlock && rPixelFragMap)
		{
		for (Row = Zip = 0; Row < Height; Row ++)
			{
			for (Col = 0; Col < Width; Col ++, Zip ++)
				{
				// unproject this pixel and see if it is a valid position to plot to
				Vert.ScrnXYZ[0] = Col + .5;
				Vert.ScrnXYZ[1] = Row + .5;
				Vert.ScrnXYZ[2] = Vert.Q = 0.0;
				if (! Cam->TestValidScreenPosition(&Vert, CenterPixelSize))
					{
					if (PixFrag = rPixelFragMap[Zip].PlotPixel(rPixelBlock, 0.0f, (unsigned char)255, ~0UL, ~0UL, 10, 
						WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_MASK))
						{
						PixFrag->PlotPixel(rPixelBlock, PixRGB, DummyDbl, DummyNml);
						// DrawOffsetX, DrawOffsetY may not yet be calculated so best not to plot at this time
						//ScreenPixelPlotFragments(&rPixelFragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);
						} // if
					} // if
				} // for
			} // for
		} // if
	} // if

} // Renderer::PlotProjectedNULLRegionFrags

/*===========================================================================*/

void Renderer::ClearProjectedNULLRegionFrags(void)
{
long Row, Col, Zip;
rPixelFragment *PixFrag;

if (Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && Cam->Projected && Cam->Coords)
	{
	if (rPixelBlock && rPixelFragMap)
		{
		for (Row = Zip = 0; Row < Height; Row ++)
			{
			for (Col = 0; Col < Width; Col ++, Zip ++)
				{
				if ((PixFrag = rPixelFragMap[Zip].FragList) && rPixelFragMap[Zip].UsedFrags > 0)
					{
					if (PixFrag->TestFlags(WCS_PIXELFRAG_FLAGBIT_OBJECTYPE) == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_MASK)
						{
						rPixelFragMap[Zip].FragList = PixFrag->Next;
						rPixelFragMap[Zip].UsedFrags --;
						} // if
					} // if
				} // for
			} // for
		} // if
	} // if

} // Renderer::ClearProjectedNULLRegionFrags

/*===========================================================================*/

// computes width of the center pixel of full image (all segments) in meters at one meter distance Z
double Renderer::ComputeCenterPixelSize(void)
{
double DistX, DistY, DistZ;
VertexDEM Vert[2];

Vert[0].ScrnXYZ[0] = Cam->CenterX + .5;
Vert[0].ScrnXYZ[1] = Cam->CenterY + .5;
Vert[0].ScrnXYZ[2] = Vert[0].Q = 1.0;
Cam->UnProjectVertexDEM(DefCoords, &Vert[0], EarthLatScaleMeters, PlanetRad, 0);	// 0 = don't need lat/lon
Vert[1].ScrnXYZ[0] = Cam->CenterX - .5;
Vert[1].ScrnXYZ[1] = Cam->CenterY - .5;
Vert[1].ScrnXYZ[2] = Vert[1].Q = 1.0;
Cam->UnProjectVertexDEM(DefCoords, &Vert[1], EarthLatScaleMeters, PlanetRad, 0);	// 0 = don't need lat/lon

if (Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
	{
	DistX = (Vert[0].Lon - Vert[1].Lon) * Cam->CamLonScale * EarthLatScaleMeters;
	DistY = (Vert[0].Lat - Vert[1].Lat) * EarthLatScaleMeters;
	DistZ = 0.0;
	} // if
else
	{
	DistX = Vert[0].XYZ[0] - Vert[1].XYZ[0];
	DistY = Vert[0].XYZ[1] - Vert[1].XYZ[1];
	DistZ = Vert[0].XYZ[2] - Vert[1].XYZ[2];
	} // if

return (sqrt(DistX * DistX + DistY * DistY + DistZ * DistZ));

} // Renderer::ComputeCenterPixelSize

/*===========================================================================*/

// RenderSuccess < 0 indicates that rendering will continue with a different Renderer, Scenarios forced reinit
void Renderer::Cleanup(bool KeepRaster, bool FromShadows, int RenderSuccess, bool OpenDiagnostic)
{
BufferNode *NextBuf;
NotifyTag Changes[2];

// reset this if rendering is done
if (! FromShadows)
	{
	if (RenderSuccess)
		ProcessFoliageList();
	DestroyFoliageList();
	AppMem_ClearDEMSizes();
	} // if

// post final elapsed render time
if (Master && ! FromShadows && RenderSuccess >= 0)
	Master->SetProcText((char*)(RenderSuccess ? "Render Completed.": "Render Init Error."));

// free all Effect GeoRasters
if (! FromShadows && EffectsBase)
	EffectsBase->FreeAll(RenderSuccess >= 0);
// free all vector segment data
if (! FromShadows && DBase)
	DBase->FreeVecSegmentData();
// clear medium-longevity image objects from memory
if (ImageBase)
	ImageBase->ClearImageRasters(WCS_RASTER_LONGEVITY_MEDIUM);

// delete stuff
#ifdef WCS_VECPOLY_EFFECTS
// must be before Quanta is destroyed
if (! FromShadows && EvalEffects)
	EvalEffects->DestroyPolygonList(NULL);
if (DEMCue && ! FromShadows)
	{
	// must be before Quanta is destroyed
	delete DEMCue;
	DEMCue = NULL;
	} // if
if (! FromShadows && QuantaCreated)
	RemoveQuantaAllocator();
if (! FromShadows)
	DEMMemoryCurrentSize = 0;
#else // WCS_VECPOLY_EFFECTS
if (DEMCue)
	delete DEMCue;
DEMCue = NULL;
#endif // WCS_VECPOLY_EFFECTS

// remove vertex and polygon edge objects
if (VertexRoot[0])
	delete VertexRoot[0];
if (VertexRoot[1])
	delete VertexRoot[1];
if (VertexRoot[2])
	delete VertexRoot[2];
if (EdgeRoot[0])
	delete EdgeRoot[0];
if (EdgeRoot[1])
	delete EdgeRoot[1];
if (EdgeRoot[2])
	delete EdgeRoot[2];
VertexRoot[0] = VertexRoot[1] = VertexRoot[2] = NULL;
EdgeRoot[0] = EdgeRoot[1] = EdgeRoot[2] = NULL;

if (PolyEdge1)
	AppMem_Free(PolyEdge1, PolyEdgeSize);
if (PolyEdge2)
	AppMem_Free(PolyEdge2, PolyEdgeSize);
PolyEdge1 = PolyEdge2 = NULL;
PolyEdgeSize = 0;

if (PixelWeight)
	AppMem_Free(PixelWeight, PixelWeightSize * sizeof (unsigned char));
if (PixelBitBytes)
	AppMem_Free(PixelBitBytes, 8 * PixelWeightSize * sizeof (unsigned char));
if (VertexWeight[0])
	AppMem_Free(VertexWeight[0], PixelWeightSize * sizeof (double));
if (VertexWeight[1])
	AppMem_Free(VertexWeight[1], PixelWeightSize * sizeof (double));
if (VertexWeight[2])
	AppMem_Free(VertexWeight[2], PixelWeightSize * sizeof (double));
PixelWeight = PixelBitBytes = NULL;
VertexWeight[0] = VertexWeight[1] = VertexWeight[2] = NULL;
PixelWeightSize = 0;

if (! FromShadows && RenderSuccess >= 0 && Buffers && Buffers->Buffer)
	SaveDisplayedBuffers(1);

if (! KeepRaster)
	{
	rFreePixelFragmentMap();
	if (Preview)
		{
		delete Preview;
		Preview = NULL;
		Pane = NULL;
		} // if
	while (Buffers)
		{
		NextBuf = Buffers->Next;
		delete Buffers;
		Buffers = NextBuf;
		} // if
	if (Rast)
		delete Rast;
	Rast = NULL;
	if (LastRast)
		delete LastRast;
	LastRast = NULL;
	Bitmap[2] = Bitmap[1] = Bitmap[0] = AABuf = TreeBitmap[2] = TreeBitmap[1] = TreeBitmap[0] = TreeAABuf = NULL;
	ZBuf = TreeZBuf = LatBuf = LonBuf = ElevBuf = ReflectionBuf = IllumBuf = SlopeBuf = AspectBuf = NormalBuf[2] = NormalBuf[1] = NormalBuf[0] = NULL;
	ObjectBuf = NULL;
	ExponentBuf = NULL;
	PlotBuf[2] = PlotBuf[1] = PlotBuf[0] = NULL;

	SetupWidth = SetupHeight = Width = Height = OSLowX = OSHighX = OSLowY = OSHighY = 0;
	ViewCamCopy = NULL;
	} // if
else if (Cam && (ViewCamCopy = new Camera))
	{
	ReportPixelFragResources();
	ViewCamCopy->Copy(ViewCamCopy, Cam);
	} // else if
else 
	{
	ReportPixelFragResources();
	ViewCamCopy = NULL;
	} // else

if (! FromShadows)
	{
	if (! IsCamView && ProjectBase && RenderSuccess)
		{
		// reset time and frame rate values
		ProjectBase->Interactive->SetActiveTimeAndRate(StashProjectTime, StashProjectFrameRate);

		// update application by time
		AppBase->UpdateProjectByTime();

		// send notification that time has changed even though we're just resetting it.
		Changes[0] = MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff);
		Changes[1] = NULL;
		ProjectBase->Interactive->GenerateNotify(Changes, NULL);
		} // if not from CamView

	// set display thresholds to defaults since diagnostics won't know any different when it opens
	Threshold[0] = 0;
	Threshold[1] = 100;
	if (Preview && RenderSuccess >= 0)
		{
		Preview->RenderDone(OpenDiagnostic);
		} // if
	AppBase->SetProcPri(0);
	LocalLog->EndModal();
	if (Master && RenderSuccess)
		{
		Master->GUIEndModal();
		//Master->Run = not needed I'm thinking
		Master->SetPause(0);
		} // if
	//GlobalApp->AppEx->GenerateNotify(ThawEvent, NULL);
	ApplyPostProc(NULL);
	} // if

} // Renderer::Cleanup

/*===========================================================================*/

#ifdef WCS_VECPOLY_EFFECTS
QuantaAllocator *Renderer::AddQuantaAllocator(void)
{
unsigned long MemCeiling;

if (! MyErrorFunctor)
	MyErrorFunctor = new RendererQuantaAllocatorErrorFunctor(this);
if (! MyErrorFunctor)
	return NULL;
		
if (! Quanta)
	{
	if (Quanta = new QuantaAllocator())
		{
		QuantaCreated = true;
		MaterialList::PrepareAllocation(Quanta, 100000, 25000); // prepare the class
		MatListWaterProperties::PrepareAllocation(Quanta, 5000, 5000); // prepare the class
		MatListVectorProperties::PrepareAllocation(Quanta, 5000, 5000); // prepare the class
		VectorNodeRenderData::PrepareAllocation(Quanta, 100000, 25000); // prepare the class
		VectorNodeList::PrepareAllocation(Quanta, 1000, 1000); // prepare the class
		VectorNodeLink::PrepareAllocation(Quanta, 200000, 50000); // prepare the class
		VectorNode::PrepareAllocation(Quanta, 100000, 25000); // prepare the class
		EffectJoeList::PrepareAllocation(Quanta, 100000, 25000); // prepare the class
		VectorPart::PrepareAllocation(Quanta, 1000, 1000); // prepare the class
		VectorPolygon::PrepareAllocation(Quanta, 50000, 20000); // prepare the class
		VectorPolygonList::PrepareAllocation(Quanta, 1000, 500); // prepare the class
		VectorPolygonListDouble::PrepareAllocation(Quanta, 50000, 20000); // prepare the class
		PolygonBoundingBox::PrepareAllocation(Quanta, 10000, 5000); // prepare the class
		PolygonEdgeList::PrepareAllocation(Quanta, 1000, 500); // prepare the class
		TfxDetail::PrepareAllocation(Quanta, 5000, 2000); // prepare the class

		Quanta->InstallCeilingErrorHandlerFunctor(MyErrorFunctor);
		MemCeiling = ProjectBase->Prefs.MemoryLimitsEnabled ? ProjectBase->Prefs.VecPolyMemoryLimit * 1000000: 0;
		Quanta->SetCombinedPoolCeilingBytes(MemCeiling); // 100megs normal, 75megs for testing RMNP_Profile23, 0=disabled
		} // if
	} // if
	
return (Quanta);

} // Renderer::AddQuantaAllocator

/*===========================================================================*/

void Renderer::RemoveQuantaAllocator(void)
{
unsigned long Lr, Lc;

// this is a static array so could be deleted in the program cleanup code on exit
// actually with the QuantaAllocator, it isn't necessary to delete these at all

if (Quanta)
	{
	for (Lr = 0; Lr < 129; ++Lr)
		{
		for (Lc = 0; Lc < 129; ++Lc)
			{
			#ifdef WCS_QUANTAALLOCATOR_DETAILED_CLEANUP
			if (SubNodes[Lr][Lc])
				{
				SubNodes[Lr][Lc]->DeleteLinksNoCrossCheck();
				delete SubNodes[Lr][Lc];
				} // if
			#endif // WCS_QUANTAALLOCATOR_DETAILED_CLEANUP
			SubNodes[Lr][Lc] = NULL;
			} // for
		} // for

	ReportQuantaResources();
	MaterialList::CleanupAllocation(); // clean up
	MatListWaterProperties::CleanupAllocation(); // clean up
	MatListVectorProperties::CleanupAllocation(); // clean up
	VectorNodeRenderData::CleanupAllocation(); // clean up
	VectorNodeList::CleanupAllocation(); // clean up
	VectorNodeLink::CleanupAllocation(); // clean up
	VectorNode::CleanupAllocation(); // clean up
	EffectJoeList::CleanupAllocation(); // clean up
	VectorPart::CleanupAllocation(); // clean up
	VectorPolygon::CleanupAllocation(); // clean up
	VectorPolygonList::CleanupAllocation(); // clean up
	VectorPolygonListDouble::CleanupAllocation(); // clean up
	PolygonBoundingBox::CleanupAllocation(); // clean up
	PolygonEdgeList::CleanupAllocation(); // clean up
	TfxDetail::CleanupAllocation(); // clean up
	delete Quanta;
	Quanta = NULL;
	QuantaCreated = false;
	} // if

if (MyErrorFunctor)
	{
	delete MyErrorFunctor;
	MyErrorFunctor = NULL;
	} // if
	
} // Renderer::RemoveQuantaAllocator

/*===========================================================================*/

void Renderer::ReportQuantaResources(void)
{

#ifdef _DEBUG
// tell us how many units of different types were actually called for vs what was allocated
char DebugStr[128];

//foo = (unsigned long)MaterialList::GetPoolCurrentItemsInUse();
sprintf(DebugStr, "%I64u / %lu MaterialList\n", MaterialList::GetPoolCurrentItemsInUse(), sizeof (class MaterialList));
OutputDebugString(DebugStr);
sprintf(DebugStr, "%I64u / %lu MatListWaterProperties\n", MatListWaterProperties::GetPoolCurrentItemsInUse(), sizeof (class MatListWaterProperties));
OutputDebugString(DebugStr);
sprintf(DebugStr, "%I64u / %lu MatListVectorProperties\n", MatListVectorProperties::GetPoolCurrentItemsInUse(), sizeof (class MatListVectorProperties));
OutputDebugString(DebugStr);
sprintf(DebugStr, "%I64u / %lu VectorNodeRenderData\n", VectorNodeRenderData::GetPoolCurrentItemsInUse(), sizeof (class VectorNodeRenderData));
OutputDebugString(DebugStr);
sprintf(DebugStr, "%I64u / %lu VectorNodeList\n", VectorNodeList::GetPoolCurrentItemsInUse(), sizeof (class VectorNodeList));
OutputDebugString(DebugStr);
sprintf(DebugStr, "%I64u / %lu VectorNodeLink\n", VectorNodeLink::GetPoolCurrentItemsInUse(), sizeof (class VectorNodeLink));
OutputDebugString(DebugStr);
sprintf(DebugStr, "%I64u / %lu VectorNode\n", VectorNode::GetPoolCurrentItemsInUse(), sizeof (class VectorNode));
OutputDebugString(DebugStr);
sprintf(DebugStr, "%I64u / %lu EffectJoeList\n", EffectJoeList::GetPoolCurrentItemsInUse(), sizeof (class EffectJoeList));
OutputDebugString(DebugStr);
sprintf(DebugStr, "%I64u / %lu VectorPart\n", VectorPart::GetPoolCurrentItemsInUse(), sizeof (class VectorPart));
OutputDebugString(DebugStr);
sprintf(DebugStr, "%I64u / %lu VectorPolygon\n", VectorPolygon::GetPoolCurrentItemsInUse(), sizeof (class VectorPolygon));
OutputDebugString(DebugStr);
sprintf(DebugStr, "%I64u / %lu VectorPolygonList\n", VectorPolygonList::GetPoolCurrentItemsInUse(), sizeof (class VectorPolygonList));
OutputDebugString(DebugStr);
sprintf(DebugStr, "%I64u / %lu VectorPolygonListDouble\n", VectorPolygonListDouble::GetPoolCurrentItemsInUse(), sizeof (class VectorPolygonListDouble));
OutputDebugString(DebugStr);
sprintf(DebugStr, "%I64u / %lu PolygonBoundingBox\n", PolygonBoundingBox::GetPoolCurrentItemsInUse(), sizeof (class PolygonBoundingBox));
OutputDebugString(DebugStr);
sprintf(DebugStr, "%I64u / %lu PolygonEdgeList\n", PolygonEdgeList::GetPoolCurrentItemsInUse(), sizeof (class PolygonEdgeList));
OutputDebugString(DebugStr);
sprintf(DebugStr, "%I64u / %lu TfxDetail\n", TfxDetail::GetPoolCurrentItemsInUse(), sizeof (class TfxDetail));
OutputDebugString(DebugStr);
sprintf(DebugStr, "%I64u Total QA Memory\n\n", Quanta->GetCurrentMemoryInUseBytes());
OutputDebugString(DebugStr);
#endif // _DEBUG

} // Renderer::ReportQuantaResources

/*===========================================================================*/

bool Renderer::FreeSomeQAResources(QuantaAllocator *ManagingAllocator, int DemandLevel)
{
long StashQNum, PolyCt, LowBoundCell, HighBoundCell;
unsigned long  TimeDiff, MapEntries, pLatEntriesM1;
RenderQItem *CurQ;
time_t EarliestTimeFound = 0, CurrentTime;
DEM *FoundOldest = NULL;
VectorPolygonListDouble *CurPolyList;
bool ItemFreed = false;

// see what the DEMQue has
if (DEMCue)
	{
	if (DEMCue->ActiveItems > 1)
		{
		GetTime(CurrentTime);
		// find DEMS that are not the current rendering DEM
		FoundOldest = (DEM *)0x01;
		while (FoundOldest && ! ItemFreed)
			{
			FoundOldest = NULL;
			for (CurQ = DEMCue->GetFirst(StashQNum); CurQ; CurQ = DEMCue->GetNext(StashQNum))
				{
				if (CurQ->QDEM && CurQ->QDEM != CurDEM)		// CurDEM is Renderer scope and the one being rendered
					{
					// we're looking for the last one used and not already freed
					if (CurQ->QDEM->VPData)
						{
						TimeDiff = (unsigned long)(CurrentTime - CurQ->QDEM->GetLastTouchedTime());
						if (TimeDiff > EarliestTimeFound)
							{
							EarliestTimeFound = TimeDiff;
							FoundOldest = CurQ->QDEM;
							} // if
						} // if
					} // if
				} // for
			if (FoundOldest)
				{
				if (FoundOldest->FreeVPData(true))
					ItemFreed = true;
				} // if
			} // while
		} // if
	if (! ItemFreed)
		{
		if (DEMCue->ActiveItems >= 0 && CurDEM && CurDEM->VPData)	// CurDEM is Renderer scope and the one being rendered
			{
			// we can try to free some cells that are not currently in use but have to be careful
			// since we might be calculating one while another is waiting in the cue to be drawn by another thread.
			MapEntries = CurDEM->VPDataMapEntries();
			pLatEntriesM1 = CurDEM->pLatEntries - 1;
			// the eight cells around a single cell are 
			// cell - 1 = S
			// cell + 1 = N
			// cell - (pLatEntries - 1) = W
			// cell + (pLatEntries - 1) = E
			// cell - (pLatEntries - 1) - 1 = SW
			// cell + (pLatEntries - 1) - 1 = SE
			// cell - (pLatEntries - 1) + 1 = NW
			// cell + (pLatEntries - 1) + 1 = NE
			if (CurPolyNumber >= 0 && LastPolyNumber >= 0)
				{
				LowBoundCell = min(CurPolyNumber, LastPolyNumber);
				HighBoundCell = max(CurPolyNumber, LastPolyNumber);
				} // if
			else if (CurPolyNumber >= 0)
				{
				LowBoundCell = HighBoundCell = CurPolyNumber;
				} // else if
			else if (LastPolyNumber >= 0)
				{
				LowBoundCell = HighBoundCell = LastPolyNumber;
				} // else if
			else
				{
				LowBoundCell = HighBoundCell = -1;
				} // else
			
			if (LowBoundCell >= 0)	// both will be non-negative then
				{
				LowBoundCell = LowBoundCell > (long)CurDEM->pLatEntries ? LowBoundCell - (long)pLatEntriesM1 - 1: 0;
				HighBoundCell = HighBoundCell < (long)MapEntries - (long)pLatEntriesM1 - 2 ? HighBoundCell + (long)pLatEntriesM1 + 1: (long)MapEntries - 1;
				} // if
			for (PolyCt = 0; PolyCt < (long)MapEntries; ++PolyCt)
				{
				if (PolyCt == LowBoundCell)
					{
					PolyCt = HighBoundCell;
					continue;
					} // if
				if (CurDEM->VPData[PolyCt])
					{
					// should be safe to delete this cell's data
					for (CurPolyList = CurDEM->VPData[PolyCt]; CurPolyList; CurPolyList = CurDEM->VPData[PolyCt])
						{
						CurDEM->VPData[PolyCt] = (VectorPolygonListDouble *)CurDEM->VPData[PolyCt]->NextPolygonList;
						CurPolyList->DeletePolygon();
						delete CurPolyList;
						} // for
					ItemFreed = true;
					} // if
				} // for
			if (! ItemFreed && LowBoundCell != HighBoundCell)
				{
				// do a more detailed check in the range of LowBoundCell to HighBoundCell
				for (PolyCt = LowBoundCell; PolyCt <= HighBoundCell; ++PolyCt)
					{
					if (CurDEM->VPData[PolyCt])
						{
						if (CurPolyNumber >= 0 && (
							(PolyCt >= CurPolyNumber - 1
							&& PolyCt <= CurPolyNumber + 1)
							|| (PolyCt >= CurPolyNumber - (long)pLatEntriesM1 - 1
							&& PolyCt <= CurPolyNumber - (long)pLatEntriesM1 + 1)
							|| (PolyCt >= CurPolyNumber + (long)pLatEntriesM1 - 1
							&& PolyCt <= CurPolyNumber + (long)pLatEntriesM1 + 1)))
							{
							continue;
							} // if
						if (LastPolyNumber >= 0 && (
							(PolyCt >= LastPolyNumber - 1
							&& PolyCt <= LastPolyNumber + 1)
							|| (PolyCt >= LastPolyNumber - (long)pLatEntriesM1 - 1
							&& PolyCt <= LastPolyNumber - (long)pLatEntriesM1 + 1)
							|| (PolyCt >= LastPolyNumber + (long)pLatEntriesM1 - 1
							&& PolyCt <= LastPolyNumber + (long)pLatEntriesM1 + 1)))
							{
							continue;
							} // if
						// should be safe to delete this cell's data
						for (CurPolyList = CurDEM->VPData[PolyCt]; CurPolyList; CurPolyList = CurDEM->VPData[PolyCt])
							{
							CurDEM->VPData[PolyCt] = (VectorPolygonListDouble *)CurDEM->VPData[PolyCt]->NextPolygonList;
							CurPolyList->DeletePolygon();
							delete CurPolyList;
							} // for
						ItemFreed = true;
						} // if
					} // for
				} // if
			} // else if
		} //  if
	} // if

return (ItemFreed);

} // Renderer::FreeSomeQAResources

/*===========================================================================*/

bool Renderer::FreeSomeDEMResources(void)
{
time_t EarliestTimeFound = 0, CurrentTime;
RenderQItem *CurQ;
DEM *FoundOldest = NULL;
unsigned long  TimeDiff;
long StashQNum;
bool ItemFreed = false;

if (DEMCue->ActiveItems > 1)
	{
	GetTime(CurrentTime);
	// find DEMS that are not the current rendering DEM
	for (CurQ = DEMCue->GetFirst(StashQNum); CurQ; CurQ = DEMCue->GetNext(StashQNum))
		{
		if (CurQ->QDEM && CurQ->QDEM != CurDEM)		// CurDEM is Renderer scope and the one being rendered
			{
			// we're looking for the last one used and not already freed
			if (CurQ->QDEM && CurQ->QDEM->RawMap)
				{
				TimeDiff = (unsigned long)(CurrentTime - CurQ->QDEM->GetLastTouchedTime());
				if (TimeDiff > EarliestTimeFound)
					{
					EarliestTimeFound = TimeDiff;
					FoundOldest = CurQ->QDEM;
					} // if
				} // if
			} // if
		} // for
	if (FoundOldest)
		{
		FoundOldest->FreeVPData(true);
		FoundOldest->FreeRawElevs();
		DEMMemoryCurrentSize -= FoundOldest->CalcFullMemoryReq();
		ItemFreed = true;
		} // if
	} // if

return (ItemFreed);

} // Renderer::FreeSomeDEMResources

#endif // WCS_VECPOLY_EFFECTS

/*===========================================================================*/

void Renderer::ReportPixelFragResources(void)
{
#ifdef _DEBUG
char DebugStr[128];
unsigned long FragsAllocated = 0, FragsUsed = 0;

if (rPixelBlock)
	{
	rPixelBlock->CountAllocatedFragments(FragsAllocated, FragsUsed);
	sprintf(DebugStr, "%lu / %lu Pixel Fragments Allocated/Used\n", FragsAllocated, FragsUsed);
	OutputDebugString(DebugStr);
	} // if
#endif // _DEBUG
} // Renderer::ReportPixelFragResources

/*===========================================================================*/

void Renderer::CollapseMap(void)
{
#ifdef WCS_BUILD_RTX
float ReplaceNULLElev, MinElevFound, MaxElevFound;
#endif // WCS_BUILD_RTX
long ArraySize;
#ifdef WCS_BUILD_RTX
long PixZip;
#endif // WCS_BUILD_RTX

if (rPixelFragMap)
	{
	ArraySize = Width * Height;
	#ifdef WCS_BUILD_RTX
	if (Exporter)
		{
		rPixelFragMap->CollapseMap(rPixelFragMap, ArraySize, Bitmap[0], Bitmap[1], Bitmap[2],
			ZBuf, AABuf, ReflectionBuf, NormalBuf, ExponentBuf,
			TreeBitmap[0], TreeBitmap[1], TreeBitmap[2], TreeAABuf, Exporter->FragmentCollapseType,
			Exporter->TransparentPixelsExist);
		if (ElevBuf)
			{
			ReplaceNULLElev = (float)-9999.0;
			MinElevFound = FLT_MAX;
			MaxElevFound = -FLT_MAX;
			for (PixZip = 0; PixZip < ArraySize; PixZip ++)
				{
				// any pixels that don't have terrain in them will have FLT_MAX for Z set by CollapseMap above
				// unless no NULLs are allowed and then we'll set the minimum elevation value.
				if (ZBuf[PixZip] < FLT_MAX)
					{
					ElevBuf[PixZip] = (float)(Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue - ZBuf[PixZip]);
					if (ElevBuf[PixZip] < MinElevFound)
						MinElevFound = ElevBuf[PixZip];
					if (ElevBuf[PixZip] > MaxElevFound)
						MaxElevFound = ElevBuf[PixZip];
					} // if
				else
					ElevBuf[PixZip] = ReplaceNULLElev;
				} // for
			if (! Exporter->AllowDEMNULL)
				{
				//ReplaceNULLElev = (float)(MinElevFound - 1.0);
				// find lowest elev value that is not NULL
				for (PixZip = 0; PixZip < ArraySize; PixZip ++)
					{
					// any pixels that don't have terrain in them will have -9999 for Z set above
					// if no NULLs are allowed replace this value with one closer to real terrain elevations
					if (ElevBuf[PixZip] == (float)-9999.0)
						ElevBuf[PixZip] = Exporter->ReplaceNULLElev;
					} // for
				} // if
			if (MaxElevFound > Exporter->MaxRenderedElevation)
				Exporter->MaxRenderedElevation = MaxElevFound;
			if (MinElevFound < Exporter->MinRenderedElevation)
				Exporter->MinRenderedElevation = MinElevFound;
			} // if
		} // if
	else
	#endif // WCS_BUILD_RTX
		{
		rPixelFragMap->CollapseMap(rPixelFragMap, ArraySize, Bitmap[0], Bitmap[1], Bitmap[2],
			ZBuf, AABuf, ReflectionBuf, NormalBuf, ExponentBuf);
		} // else
	} // if

} // Renderer::CollapseMap

/*===========================================================================*/

DrawingFenetre *Renderer::OpenPreview(void)
{
unsigned short PrefWidth, PrefHeight;

if (Pane)
	return (Pane);

PrefWidth = AppBase->WinSys->InquireDisplayWidth();
PrefHeight = AppBase->WinSys->InquireDisplayHeight();
PrefWidth  = (short)min(PrefWidth, Width);
PrefHeight = (short)min(PrefHeight, Height);

ProjectBase->SetWindowCoords('REND', 0, 0, PrefWidth, PrefHeight);
if (Preview = new RenderPreviewGUI(this, Master && Master->IsRunning()))
	{
	Preview->SetDrawingAreaSize(PrefWidth, PrefHeight);
	Preview->Open(ProjectBase);
	Pane = Preview;
	if (Master && Master->GetGUINativeWin())
		{
		Master->SetPreviewCheck(1);
		Master->SetPreview(1);
		} // if
	} // if

return (Pane);

} // Renderer::OpenPreview

/*===========================================================================*/

void Renderer::ClosePreview(void)
{

if (Master && Master->GetGUINativeWin())
	{
	Master->SetPreview(0);
	Master->SetPreviewCheck(0);
	} // if

if (Preview)
	{
	delete Preview;
	Preview = NULL;
	Pane = NULL;
	} // if

} // Renderer::ClosePreview

/*===========================================================================*/

void Renderer::UpdatePreview(void)
{
unsigned long  X, Y;
unsigned short SyncW, SyncH;
long TempZip, DestZip;
unsigned short Wide = 0;
unsigned char TransferTemp[3];

if (! PlotBuf[0])
	PlotBuf[0] = Bitmap[0];
if (! PlotBuf[1])
	PlotBuf[1] = Bitmap[1];
if (! PlotBuf[2])
	PlotBuf[2] = Bitmap[2];

if (Pane)
	{
	Pane->GetDrawingAreaSize(SyncW, SyncH);
	if (PlotBuf[0] && PlotBuf[1] && PlotBuf[2])
		{
		if ((DrawOffsetX > 0) && (DrawOffsetY > 0) || (Width <= SyncW) || (Height <= SyncH))
			{ // CamView limited region is in effect
			// resync window from bitmap
			for (Y = DrawOffsetY; Y < (unsigned long)(DrawOffsetY + Height); Y++)
				{
				for (X = DrawOffsetX; X < (unsigned long)(DrawOffsetX + Width); X++)
					{
					Wide = 0;
					//     <<<>>>
					// v Needs work v
					TempZip = Rast->RowZip[Y - DrawOffsetY] + X - DrawOffsetX;
					//use ThumbNailR ThumbNailG ThumbNailB
					DestZip = X - DrawOffsetX;
					ThumbNailR[DestZip] = PlotBuf[0][TempZip];
					ThumbNailG[DestZip] = PlotBuf[1][TempZip];
					ThumbNailB[DestZip] = PlotBuf[2][TempZip];
					if (ExponentBuf && DisplayBuffer == WCS_DIAGNOSTIC_RGB && ExponentBuf[TempZip])
						{
						TransferTemp[0] = ThumbNailR[DestZip];
						TransferTemp[1] = ThumbNailG[DestZip];
						TransferTemp[2] = ThumbNailB[DestZip];
						rPixelFragment::ExtractClippedExponentialColors(TransferTemp, ExponentBuf[TempZip]);
						ThumbNailR[DestZip] = TransferTemp[0];
						ThumbNailG[DestZip] = TransferTemp[1];
						ThumbNailB[DestZip] = TransferTemp[2];
						} // if
					Wide = (unsigned short)(1 + DestZip);
					} // for
				Pane->BGDrawLine24((unsigned short)DrawOffsetX, (unsigned short)Y, Wide, (unsigned char *)ThumbNailR, (unsigned char *)ThumbNailG, (unsigned char *)ThumbNailB);
				} // for
			Pane->SyncBackground((unsigned short)DrawOffsetX, (unsigned short)DrawOffsetY, (unsigned short)Width, (unsigned short)Height);
			} // if
		else
			{
			// resync window from bitmap
			if (Master)
				Master->SetProcText("Updating Preview");
			for (Y = 0; Y < SyncH; Y++)
				{
				for (X = 0; X <= (unsigned long)(SyncW); X++)
					{
					Wide = 0;
					TempZip = Rast->RowZip[Y - DrawOffsetY] + X - DrawOffsetX;
					//use ThumbNailR ThumbNailG ThumbNailB
					ThumbNailR[X] = PlotBuf[0][TempZip];
					ThumbNailG[X] = PlotBuf[1][TempZip];
					ThumbNailB[X] = PlotBuf[2][TempZip];
					if (ExponentBuf && DisplayBuffer == WCS_DIAGNOSTIC_RGB && ExponentBuf[TempZip])
						{
						TransferTemp[0] = ThumbNailR[X];
						TransferTemp[1] = ThumbNailG[X];
						TransferTemp[2] = ThumbNailB[X];
						rPixelFragment::ExtractClippedExponentialColors(TransferTemp, ExponentBuf[TempZip]);
						ThumbNailR[X] = TransferTemp[0];
						ThumbNailG[X] = TransferTemp[1];
						ThumbNailB[X] = TransferTemp[2];
						} // if
					Wide = (unsigned short)X;
					} // for
				Pane->BGDrawLine24(0, (unsigned short)Y, Wide, (unsigned char *)ThumbNailR, (unsigned char *)ThumbNailG, (unsigned char *)ThumbNailB);
				} // for
			Pane->SyncBackground(0, 0, Wide, SyncH - 1);
			} // else
		} // if
	} // if

} // Renderer::UpdatePreview

/*===========================================================================*/

void Renderer::DrawPreview(void)
{
unsigned char **TempBitMap, *RTemp = NULL, *GTemp = NULL, *BTemp = NULL;
long TempZip;
unsigned long  X, Y;
unsigned short SyncW, SyncH;
unsigned char TransferTemp[3];

// resync window from bitmap
if ((PlotFromFrags && rPixelFragMap) || (! PlotFromFrags && Bitmap[0] && Bitmap[1] && Bitmap[2]))
	{
	OpenPreview();
	if (Pane)
		{
		Pane->GetDrawingAreaSize(SyncW, SyncH);
		// Allocate contiguous block and divide up
		if (RTemp = (unsigned char *)AppMem_Alloc(((unsigned long)(SyncW + 1) * 3), APPMEM_CLEAR))
			{
			GTemp = &RTemp[(unsigned long)(SyncW + 1)];
			BTemp = &GTemp[(unsigned long)(SyncW + 1)];
			} // if
		else
			{
			// <<<>>> not sure what to do to fail gracefully here. return?
			} // else
		for (Y = 0; Y < SyncH && Y < (unsigned long)Height + DrawOffsetY; Y++)
			{
			if ((long)Y < DrawOffsetY)
				continue;
			for (X = 0; X <= (unsigned long)(SyncW) && X < (unsigned long)Width + DrawOffsetX; X++)
				{
				if ((long)X < DrawOffsetX)
					continue;
				TempZip = Rast->RowZip[Y - DrawOffsetY] + X - DrawOffsetX;
				if (PlotFromFrags)
					{
					rPixelFragMap[TempZip].CollapsePixel(RTemp[X], GTemp[X], BTemp[X]);
					} // if
				else
					{
					if (MultiBuffer && TreeZBuf[TempZip] < ZBuf[TempZip])
						{
						TempBitMap = TreeBitmap;
						} // if
					else
						{
						TempBitMap = Bitmap;
						} // else
					RTemp[X] = TempBitMap[0][TempZip];
					GTemp[X] = TempBitMap[1][TempZip];
					BTemp[X] = TempBitMap[2][TempZip];
					if (ExponentBuf && ExponentBuf[TempZip])
						{
						TransferTemp[0] = RTemp[X];
						TransferTemp[1] = GTemp[X];
						TransferTemp[2] = BTemp[X];
						rPixelFragment::ExtractClippedExponentialColors(TransferTemp, ExponentBuf[TempZip]);
						RTemp[X] = TransferTemp[0];
						GTemp[X] = TransferTemp[1];
						BTemp[X] = TransferTemp[2];
						} // if
					} // else
				} // for
			Pane->BGDrawLine24(0, (unsigned short)Y, (unsigned short)SyncW, RTemp, GTemp, BTemp);
			} // for
		if (RTemp)
			{
			Pane->SyncBackground(0, 0, (unsigned short)SyncW, (unsigned short)SyncH);
			AppMem_Free(RTemp, ((unsigned long)(SyncW + 1) * 3));
			RTemp = NULL;
			} // if
		} // if
	} // if

} // Renderer::DrawPreview

/*===========================================================================*/

void Renderer::ScreenPixelPlotFragments(rPixelHeader *Frag, long x, long y)
{
unsigned char Red, Green, Blue;

if (! Pane)
	return;

Frag->CollapsePixel(Red, Green, Blue);

//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//++DevCounter[17];
//#endif // FRANK or GARY

Pane->Plot24((int)x, (int)y, Red, Green, Blue);

} // Renderer::ScreenPixelPlotFragments 

/*===========================================================================*/

void Renderer::ScreenPixelPlot(unsigned char **PlotBitmap, long x, long y, long zip)
{
unsigned char RGB[3];

if (! Pane)
	return;

RGB[0] = PlotBitmap[0][zip];
RGB[1] = PlotBitmap[1][zip];
RGB[2] = PlotBitmap[2][zip];
if (ExponentBuf && DisplayBuffer == WCS_DIAGNOSTIC_RGB && ExponentBuf[zip])
	{
	rPixelFragment::ExtractClippedExponentialColors(RGB, ExponentBuf[zip]);
	} // if 

Pane->Plot24((int)x, (int)y, RGB[0], RGB[1], RGB[2]);

} // Renderer::ScreenPixelPlot 

/*===========================================================================*/

void Renderer::ScreenPixelPlotTwoBuf(unsigned char **BaseBitmap, float *BaseZBuf, unsigned char *BaseAABuf, 
	unsigned char **AltBitmap, float *AltZBuf, unsigned char *AltAABuf, long x, long y, long Zip)
{
unsigned char AltExists, BaseExists, AltPts, BasePts, MergePts;
unsigned char Red, Green, Blue;

if (! Pane)
	return;

AltExists = AltAABuf[Zip];
BaseExists = BaseAABuf[Zip];
if (AltExists && BaseExists)			// both alt and base at this pixel
	{
	if (AltZBuf[Zip] < BaseZBuf[Zip])		// alt is closer than base
		{
		if (AltExists < 255)			// alt buffer not full
			{
			// merge base data into alt data
			AltPts = AltExists;
			BasePts = AltPts + BaseExists > 255 ? 255 - AltPts: BaseExists;
			MergePts = AltPts + BasePts;
			Red = (UBYTE)((BaseBitmap[0][Zip] * BasePts + AltBitmap[0][Zip] * AltPts) / MergePts);
			Green = (UBYTE)((BaseBitmap[1][Zip] * BasePts + AltBitmap[1][Zip] * AltPts) / MergePts);
			Blue = (UBYTE)((BaseBitmap[2][Zip] * BasePts + AltBitmap[2][Zip] * AltPts) / MergePts);
			} // if
		else							// alt buffer full
			{
			// plot alt into base data
			Red = AltBitmap[0][Zip];
			Green = AltBitmap[1][Zip];
			Blue = AltBitmap[2][Zip];
			} // else
		} // if
	else								// base is closer than alt
		{
		if (BaseExists < 255)			// base buffer not full
			{
			// merge alt data into base data
			BasePts = BaseExists;
			AltPts = BasePts + AltExists > 255 ? 255 - BasePts: AltExists;
			MergePts = AltPts + BasePts;
			Red = (UBYTE)((BaseBitmap[0][Zip] * BasePts + AltBitmap[0][Zip] * AltPts) / MergePts);
			Green = (UBYTE)((BaseBitmap[1][Zip] * BasePts + AltBitmap[1][Zip] * AltPts) / MergePts);
			Blue = (UBYTE)((BaseBitmap[2][Zip] * BasePts + AltBitmap[2][Zip] * AltPts) / MergePts);
			} // if
		else
			{
			Red = Bitmap[0][Zip];
			Green = Bitmap[1][Zip];
			Blue = Bitmap[2][Zip];
			} // else
		} // else
	} // if
else if (AltExists)					// only alt exists at this pixel
	{
	// plot alt into base data
	Red = AltBitmap[0][Zip];
	Green = AltBitmap[1][Zip];
	Blue = AltBitmap[2][Zip];
	} // if
else
	{
	Red = Bitmap[0][Zip];
	Green = Bitmap[1][Zip];
	Blue = Bitmap[2][Zip];
	} // else

Pane->Plot24((int)x, (int)y, Red, Green, Blue);

} // Renderer::ScreenPixelPlotTwoBuf

/*===========================================================================*/

void Renderer::MergeZBufRGBPanels(unsigned char **BaseBitmap, float *BaseZBuf, unsigned char *BaseAABuf, 
	unsigned char **AltBitmap, float *AltZBuf, unsigned char *AltAABuf)
{
double MagicZBufFlip;
long Zip, MergeSize;
unsigned char AltExists, BaseExists, AltPts, BasePts, MergePts, AltPtCutoff;

if (Master)
	Master->SetProcText("Contemplating");

MagicZBufFlip = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_MAGICZ].CurValue;
AltPtCutoff = Cam->AAOptimizeReflection ? 0: MAGIC_TREEPT_CUTOFF;
MergeSize = Width * Height;

for (Zip = 0; Zip < MergeSize; Zip ++)
	{
	AltExists = AltAABuf[Zip];
	BaseExists = BaseAABuf[Zip];
	if (AltExists && BaseExists)			// both alt and base at this pixel
		{
		if (AltZBuf[Zip] < BaseZBuf[Zip])		// alt is closer than base
			{
			if (AltExists < 255)			// alt buffer not full
				{
				// merge base data into alt data
				AltPts = AltExists;
				BasePts = AltPts + BaseExists > 255 ? 255 - AltPts: BaseExists;
				MergePts = AltPts + BasePts;
				BaseBitmap[0][Zip] = (UBYTE)((BaseBitmap[0][Zip] * BasePts + AltBitmap[0][Zip] * AltPts) / MergePts);
				BaseBitmap[1][Zip] = (UBYTE)((BaseBitmap[1][Zip] * BasePts + AltBitmap[1][Zip] * AltPts) / MergePts);
				BaseBitmap[2][Zip] = (UBYTE)((BaseBitmap[2][Zip] * BasePts + AltBitmap[2][Zip] * AltPts) / MergePts);
				BaseAABuf[Zip] = MergePts;
				if (ReflectionBuf && ReflectionBuf[Zip])
					{
					ReflectionBuf[Zip] = (float)((ReflectionBuf[Zip] * BasePts) / MergePts);
					} // if
				else if (AltZBuf[Zip] < MagicZBufFlip && AltPts > AltPtCutoff)
					BaseZBuf[Zip] = AltZBuf[Zip];
				} // if
			else							// alt buffer full
				{
				// plot alt into base data
				BaseBitmap[0][Zip] = AltBitmap[0][Zip];
				BaseBitmap[1][Zip] = AltBitmap[1][Zip];
				BaseBitmap[2][Zip] = AltBitmap[2][Zip];
				BaseAABuf[Zip] = AltExists;
				BaseZBuf[Zip] = AltZBuf[Zip];
				if (ReflectionBuf)
					ReflectionBuf[Zip] = 0.0f;
				} // else
			} // if
		else								// base is closer than alt
			{
			if (BaseExists < 255)			// base buffer not full
				{
				// merge alt data into base data
				BasePts = BaseExists;
				AltPts = BasePts + AltExists > 255 ? 255 - BasePts: AltExists;
				MergePts = AltPts + BasePts;
				BaseBitmap[0][Zip] = (UBYTE)((BaseBitmap[0][Zip] * BasePts + AltBitmap[0][Zip] * AltPts) / MergePts);
				BaseBitmap[1][Zip] = (UBYTE)((BaseBitmap[1][Zip] * BasePts + AltBitmap[1][Zip] * AltPts) / MergePts);
				BaseBitmap[2][Zip] = (UBYTE)((BaseBitmap[2][Zip] * BasePts + AltBitmap[2][Zip] * AltPts) / MergePts);
				BaseAABuf[Zip] = MergePts;
				if (ReflectionBuf && ReflectionBuf[Zip])
					{
					ReflectionBuf[Zip] = (float)((ReflectionBuf[Zip] * BasePts) / MergePts);
					} // if
				} // if
			} // else
		} // if
	else if (AltExists)					// only alt exists at this pixel
		{
		// plot alt into base data
		BaseBitmap[0][Zip] = AltBitmap[0][Zip];
		BaseBitmap[1][Zip] = AltBitmap[1][Zip];
		BaseBitmap[2][Zip] = AltBitmap[2][Zip];
		BaseAABuf[Zip] = AltExists;
		BaseZBuf[Zip] = AltZBuf[Zip];
		} // if
	} // for Zip

} // Renderer::MergeZBufRGBPanels

/*===========================================================================*/

void Renderer::PlotPixelWithExponent(unsigned char **Bitmap, unsigned short *ExponentBuf, long PixZip, double *Value)
{
unsigned long TempRGB;
unsigned short TempExp;

ExponentBuf[PixZip] = 0;

TempExp = 0;
TempRGB = (unsigned long)(Value[0] * 255.0);
while (TempRGB & 0xffffff00)
	{
	TempExp ++;
	TempRGB >>= 1;
	} // while
Bitmap[0][PixZip] = (unsigned char)TempRGB;
ExponentBuf[PixZip] |= (TempExp << 10);

TempExp = 0;
TempRGB = (unsigned long)(Value[1] * 255.0);
while (TempRGB & 0xffffff00)
	{
	TempExp ++;
	TempRGB >>= 1;
	} // while
Bitmap[1][PixZip] = (unsigned char)TempRGB;
ExponentBuf[PixZip] |= (TempExp << 5);

TempExp = 0;
TempRGB = (unsigned long)(Value[2] * 255.0);
while (TempRGB & 0xffffff00)
	{
	TempExp ++;
	TempRGB >>= 1;
	} // while
Bitmap[2][PixZip] = (unsigned char)TempRGB;
ExponentBuf[PixZip] |= (TempExp);

} // Renderer::PlotPixelWithExponent

/*===========================================================================*/

void Renderer::GetPixelWithExponent(unsigned char **Bitmap, unsigned short *ExponentBuf, long PixZip, double *Value)
{
unsigned long TempRed, TempGreen, TempBlue;
unsigned short TempExp;

TempRed = Bitmap[0][PixZip];
TempExp = (ExponentBuf[PixZip] >> 10);
if (TempExp)
	TempRed <<= TempExp;

TempGreen = Bitmap[1][PixZip];
TempExp = ((ExponentBuf[PixZip] & 992) >> 5);
if (TempExp)
	TempGreen <<= TempExp;

TempBlue = Bitmap[2][PixZip];
TempExp = (ExponentBuf[PixZip] & 31);
if (TempExp)
	TempBlue <<= TempExp;

Value[0] = TempRed * (1.0 / 255.0);
Value[1] = TempGreen * (1.0 / 255.0);
Value[2] = TempBlue * (1.0 / 255.0);

} // Renderer::GetPixelWithExponent

/*===========================================================================*/

void Renderer::GetDisplayPixelWithExponent(unsigned char **Bitmap, unsigned short *ExponentBuf, long PixZip, unsigned char *Value)
{
unsigned long TempRed, TempGreen, TempBlue;
unsigned short TempExp;

TempRed = Bitmap[0][PixZip];
TempExp = (ExponentBuf[PixZip] >> 10);
if (TempExp)
	TempRed <<= TempExp;
if (TempRed > 255)
	TempRed = 255;

TempGreen = Bitmap[1][PixZip];
TempExp = ((ExponentBuf[PixZip] & 992) >> 5);
if (TempExp)
	TempGreen <<= TempExp;
if (TempGreen > 255)
	TempGreen = 255;

TempBlue = Bitmap[2][PixZip];
TempExp = (ExponentBuf[PixZip] & 31);
if (TempExp)
	TempBlue <<= TempExp;
if (TempBlue > 255)
	TempBlue = 255;

Value[0] = (unsigned char)TempRed;
Value[1] = (unsigned char)TempGreen;
Value[2] = (unsigned char)TempBlue;

} // Renderer::GetDisplayPixelWithExponent

/*===========================================================================*/

extern double TotalTime;
extern unsigned int TotalTicks;

void Renderer::LogElapsedTime(long CurFrame, bool ShowFrameTimeSummary, bool ShowJobTimeSummary, bool ShowCompletionPopup)
{
time_t NowSecs, Elapsed;
unsigned char ElapHrs, ElapMin, ElapSec;
char FrameStr[16], TotalFrames[16], BigStr[256];

if (StartSecs)
	{
	GetTime(NowSecs);
	UEndSecsFP = GetProcessTimeFP();

	sprintf(FrameStr, "%1d", CurFrame);
	sprintf(TotalFrames, "%1d", FramesToRender);

#ifdef WCS_BUILD_FRANK
#if _M_IX86_FP == 0
	sprintf(BigStr, "Compiled without SSE/SSE2\n");
#elif _M_IX86_FP == 1
	sprintf(BigStr, "Compiled for SSE");
#elif _M_IX86_FP == 2
	sprintf(BigStr, "Compiled for SSE2\n");
#else
	sprintf(BigStr, "Compiled with unknown /arch setting");
#endif // _M_IX86_FP
	LocalLog->PostStockError(WCS_LOG_MSG_NULL, BigStr);
#endif // WCS_BUILD_FRANK

#ifdef _OPENMP
	sprintf(BigStr, "OpenMP Enabled\n");
	LocalLog->PostStockError(WCS_LOG_MSG_NULL, BigStr);
#endif // _OPENMP

	if (GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("usertime_benchmark"))
		{
		double TimeDif, DecSec;
		TimeDif = UEndSecsFP - UStartSecsFP;
		DecSec = fmod(TimeDif, 60.0);
		ElapMin = (unsigned char)(((unsigned long)TimeDif / 60) % 60);
		ElapHrs = (unsigned char)((unsigned long)TimeDif / 3600);
		sprintf(BigStr, "User %s:  %02d:%02d:%07.4lf\n", FrameStr, ElapHrs, ElapMin, DecSec);
		LocalLog->PostStockError(WCS_LOG_MSG_TIME_ELAPSE, BigStr);
		} // if

	if (GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("profile_time_secs"))
		{
		sprintf(BigStr, "Profile secs: %lf\n", TotalTime);
		LocalLog->PostStockError(WCS_LOG_MSG_TIME_ELAPSE, BigStr);
		} // if

	if (GlobalApp->MainProj->Prefs.PrivateQueryConfigOpt("profile_time_ticks"))
		{
		sprintf(BigStr, "Profile ticks: %ld\n", TotalTicks);
		LocalLog->PostStockError(WCS_LOG_MSG_TIME_ELAPSE, BigStr);
		} // if

#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
	// all 50 counters are output because it's easier than always trying to enable/disable sets & remembering to do so.
	for (unsigned long DC = 0; DC < 50; DC++)
		{
		sprintf(BigStr, "DevCounter%d %I64u\n", DC, DevCounter[DC]);
		//LocalLog->PostStockError(WCS_LOG_MSG_NULL, BigStr);
		OutputDebugStr(BigStr);
		DevCounter[DC] = 0;
		} // for
	sprintf(BigStr, "Profile Time = %f\n", TotalTime);
	OutputDebugStr(BigStr);
	TotalTime = 0.0;
#endif // FRANK or GARY

	Elapsed = NowSecs - StartSecs;
	ElapSec = (unsigned char)(Elapsed % 60);
	ElapMin = (unsigned char)((Elapsed / 60) % 60);
	ElapHrs = (unsigned char)(Elapsed / 3600);

	if (ShowFrameTimeSummary)
		{
		sprintf(BigStr, "%s:  %02d:%02d:%02d", FrameStr, ElapHrs, ElapMin, ElapSec);
		LocalLog->PostStockError(WCS_LOG_MSG_TIME_ELAPSE, BigStr);
		} // if

	Elapsed = NowSecs - FirstSecs;
	ElapSec = (unsigned char)(Elapsed % 60);
	ElapMin = (unsigned char)((Elapsed / 60) % 60);
	ElapHrs = (unsigned char)(Elapsed / 3600);

	if(ShowJobTimeSummary)
		{
		sprintf(BigStr, "%s frames:  %02d:%02d:%02d", TotalFrames, ElapHrs, ElapMin, ElapSec);
		LocalLog->PostStockError(WCS_LOG_MSG_TOTAL_ELAPS, BigStr);
		} // if

	if(ShowCompletionPopup)
		{
		if (! GlobalApp->WinSys->InquireMinimized())
			{
			int SuppressUMOK = 0;

			SuppressUMOK = suppress_rendernotify;
			if (! SuppressUMOK && GlobalApp->MainProj->Prefs.PublicQueryConfigOptTrue("suppress_rendernotify"))
				{
				SuppressUMOK = 1;
				} // if
			if (!SuppressUMOK)
				{
				sprintf(BigStr, "Elapsed Time for frame %s:  %02d:%02d:%02d", FrameStr, ElapHrs, ElapMin, ElapSec);
				GlobalApp->Toaster->ShowNotification("Render Complete", BigStr, IDI_WCSNOSHADOW, false);
				} // if
			} // if
		} // if

	} // if 

} // Renderer::LogElapsedTime()

/*===========================================================================*/

void Renderer::SampleDiagnostics(long X, long Y, long MoveX, long MoveY)
{
DiagnosticData Data;
NotifyTag Changes[2];
long PixZip, SampleX, SampleY;

Data.RefLat = TexRefLat;
Data.RefLon = TexRefLon;
SampleX = X - DrawOffsetX;
SampleY = Y - DrawOffsetY;
Data.PixelX = X;
Data.PixelY = Y;
Data.MoveX = MoveX;
Data.MoveY = MoveY;
memset(&Data.ValueValid[0], 0, WCS_DIAGNOSTIC_NUMBUFFERS);
if (SampleX >= 0 && SampleY >= 0 && SampleX < Width && SampleY < Height)
	{
	PixZip = SampleY * Width + SampleX;
	if ((ZBuf && ZBuf[PixZip] < FLT_MAX) || ! ZBuf)
		{
		if (Bitmap[0] && Bitmap[1] && Bitmap[2])
			{
			Data.DataRGB[0] = Bitmap[0][PixZip];
			Data.DataRGB[1] = Bitmap[1][PixZip];
			Data.DataRGB[2] = Bitmap[2][PixZip];
			if (ExponentBuf && ExponentBuf[PixZip])
				rPixelFragment::ExtractClippedExponentialColors(Data.DataRGB, ExponentBuf[PixZip]);
			Data.ValueValid[WCS_DIAGNOSTIC_RGB] = 1;
			} // if
		if (AABuf)
			{
			Data.Alpha = AABuf[PixZip];
			Data.ValueValid[WCS_DIAGNOSTIC_ALPHA] = 1;
			} // if
		if (ZBuf)
			{
			Data.Value[WCS_DIAGNOSTIC_Z] = ZBuf[PixZip];
			Data.ValueValid[WCS_DIAGNOSTIC_Z] = 1;
			} // if
		if (ElevBuf)
			{
			Data.Value[WCS_DIAGNOSTIC_ELEVATION] = ElevBuf[PixZip];
			Data.ValueValid[WCS_DIAGNOSTIC_ELEVATION] = 1;
			} // if
		if (LatBuf)
			{
			Data.Value[WCS_DIAGNOSTIC_LATITUDE] = LatBuf[PixZip] + TexRefLat;
			Data.ValueValid[WCS_DIAGNOSTIC_LATITUDE] = 1;
			} // if
		if (LonBuf)
			{
			Data.Value[WCS_DIAGNOSTIC_LONGITUDE] = LonBuf[PixZip] + TexRefLon;
			Data.ValueValid[WCS_DIAGNOSTIC_LONGITUDE] = 1;
			} // if
		if (SlopeBuf)
			{
			Data.Value[WCS_DIAGNOSTIC_SLOPE] = SlopeBuf[PixZip];
			Data.ValueValid[WCS_DIAGNOSTIC_SLOPE] = 1;
			} // if
		if (AspectBuf)
			{
			Data.Value[WCS_DIAGNOSTIC_ASPECT] = AspectBuf[PixZip];
			Data.ValueValid[WCS_DIAGNOSTIC_ASPECT] = 1;
			} // if
		if (IllumBuf)
			{
			Data.Value[WCS_DIAGNOSTIC_ILLUMINATION] = IllumBuf[PixZip];
			Data.ValueValid[WCS_DIAGNOSTIC_ILLUMINATION] = 1;
			} // if
		if (ReflectionBuf)
			{
			Data.Value[WCS_DIAGNOSTIC_REFLECTION] = ReflectionBuf[PixZip];
			Data.ValueValid[WCS_DIAGNOSTIC_REFLECTION] = 1;
			} // if
		if (RelElBuf)
			{
			Data.Value[WCS_DIAGNOSTIC_RELEL] = RelElBuf[PixZip];
			Data.ValueValid[WCS_DIAGNOSTIC_RELEL] = 1;
			} // if
		if (NormalBuf[0])
			{
			Data.Value[WCS_DIAGNOSTIC_NORMALX] = NormalBuf[0][PixZip];
			Data.ValueValid[WCS_DIAGNOSTIC_NORMALX] = 1;
			} // if
		if (NormalBuf[1])
			{
			Data.Value[WCS_DIAGNOSTIC_NORMALY] = NormalBuf[1][PixZip];
			Data.ValueValid[WCS_DIAGNOSTIC_NORMALY] = 1;
			} // if
		if (NormalBuf[2])
			{
			Data.Value[WCS_DIAGNOSTIC_NORMALZ] = NormalBuf[2][PixZip];
			Data.ValueValid[WCS_DIAGNOSTIC_NORMALZ] = 1;
			} // if
		if (ObjectBuf && ObjectBuf[PixZip])
			{
			if (EffectsBase->IsEffectValid((GeneralEffect *)ObjectBuf[PixZip], 1)
				|| DBase->ValidateJoe(ObjectBuf[PixZip]))
				{
				Data.Object = ObjectBuf[PixZip];
				Data.ValueValid[WCS_DIAGNOSTIC_OBJECT] = 1;
				} // if
			} // if
		} // if
	} // if

Data.DisplayedBuffer = DisplayBuffer;
Data.ThresholdValid = ThresholdValid;
Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 
	MoveX || MoveY ? WCS_DIAGNOSTIC_ITEM_MOUSEDRAG: WCS_DIAGNOSTIC_ITEM_MOUSEDOWN, 0);
Changes[1] = NULL;

if (GlobalApp->GUIWins->CVG && Data.ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data.ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
	{
	if (Data.ValueValid[WCS_DIAGNOSTIC_ELEVATION])
		{
		GlobalApp->GUIWins->CVG->SetCursor(Data.Value[WCS_DIAGNOSTIC_LATITUDE], Data.Value[WCS_DIAGNOSTIC_LONGITUDE], Data.Value[WCS_DIAGNOSTIC_ELEVATION]);
		} // if
	else
		{
		GlobalApp->GUIWins->CVG->SetCursor(Data.Value[WCS_DIAGNOSTIC_LATITUDE], Data.Value[WCS_DIAGNOSTIC_LONGITUDE], 0.0);
		} // else
	GlobalApp->GUIWins->CVG->Draw();
	} // if
GlobalApp->AppEx->GenerateNotify(Changes, &Data);

} // Renderer::SampleDiagnostics

/*===========================================================================*/

void Renderer::EditDiagnosticObject(long X, long Y)
{
long PixZip, SampleX, SampleY;

SampleX = X - DrawOffsetX;
SampleY = Y - DrawOffsetY;
if (SampleX >= 0 && SampleY >= 0 && SampleX < Width && SampleY < Height)
	{
	PixZip = SampleY * Width + SampleX;
	if (ObjectBuf)
		{
		if (ObjectBuf[PixZip] && (EffectsBase->IsEffectValid((GeneralEffect *)ObjectBuf[PixZip], 1)
			|| DBase->ValidateJoe(ObjectBuf[PixZip])))
			(ObjectBuf[PixZip])->EditRAHost();
		} // if
	} // if

} // Renderer::EditDiagnosticObject

/*===========================================================================*/

void Renderer::SelectDiagnosticObject(long X, long Y)
{
long PixZip, SampleX, SampleY;

SampleX = X - DrawOffsetX;
SampleY = Y - DrawOffsetY;
if (SampleX >= 0 && SampleY >= 0 && SampleX < Width && SampleY < Height)
	{
	PixZip = SampleY * Width + SampleX;
	if (ObjectBuf)
		{
		if (ObjectBuf[PixZip] && (EffectsBase->IsEffectValid((GeneralEffect *)ObjectBuf[PixZip], 1)
			|| DBase->ValidateJoe(ObjectBuf[PixZip])))
			RasterAnimHost::SetActiveRAHost(ObjectBuf[PixZip]);
		} // if
	} // if

} // Renderer::SelectDiagnosticObject

/*===========================================================================*/

void Renderer::SetDisplayBuffer(unsigned char NewBuf, PostProcess *ProcessMe)
{
DiagnosticData Data;
NotifyTag Changes[2];

ThresholdValid = (
	NewBuf == WCS_DIAGNOSTIC_Z ||
	NewBuf == WCS_DIAGNOSTIC_ELEVATION ||
	NewBuf == WCS_DIAGNOSTIC_LATITUDE ||
	NewBuf == WCS_DIAGNOSTIC_LONGITUDE ||
	NewBuf == WCS_DIAGNOSTIC_SLOPE ||
	NewBuf == WCS_DIAGNOSTIC_ASPECT ||
	NewBuf == WCS_DIAGNOSTIC_ILLUMINATION ||
	NewBuf == WCS_DIAGNOSTIC_REFLECTION ||
	NewBuf == WCS_DIAGNOSTIC_RELEL ||
	NewBuf == WCS_DIAGNOSTIC_RGB);

switch (NewBuf)
	{
	case WCS_DIAGNOSTIC_ELEVATION:
		{
		if (ElevBuf && TreeBitmap[0])
			{
			DisplayBuffer = NewBuf;
			ConvertDiagnostics(1, &ElevBuf, WCS_DIAGNOSTIC_ELEVATION);
			PlotBuf[0] = TreeBitmap[0];
			PlotBuf[1] = TreeBitmap[0];
			PlotBuf[2] = TreeBitmap[0];
			} // if
		break;
		} // WCS_DIAGNOSTIC_ELEVATION
	case WCS_DIAGNOSTIC_LATITUDE:
		{
		if (LatBuf && TreeBitmap[0])
			{
			DisplayBuffer = NewBuf;
			ConvertDiagnostics(1, &LatBuf, WCS_DIAGNOSTIC_LATITUDE);
			PlotBuf[0] = TreeBitmap[0];
			PlotBuf[1] = TreeBitmap[0];
			PlotBuf[2] = TreeBitmap[0];
			} // if
		break;
		} // WCS_DIAGNOSTIC_LATITUDE
	case WCS_DIAGNOSTIC_LONGITUDE:
		{
		if (LonBuf && TreeBitmap[0])
			{
			DisplayBuffer = NewBuf;
			ConvertDiagnostics(1, &LonBuf, WCS_DIAGNOSTIC_LONGITUDE);
			PlotBuf[0] = TreeBitmap[0];
			PlotBuf[1] = TreeBitmap[0];
			PlotBuf[2] = TreeBitmap[0];
			} // if
		break;
		} // WCS_DIAGNOSTIC_LONGITUDE
	case WCS_DIAGNOSTIC_SLOPE:
		{
		if (SlopeBuf && TreeBitmap[0])
			{
			DisplayBuffer = NewBuf;
			ConvertDiagnostics(1, &SlopeBuf, WCS_DIAGNOSTIC_SLOPE);
			PlotBuf[0] = TreeBitmap[0];
			PlotBuf[1] = TreeBitmap[0];
			PlotBuf[2] = TreeBitmap[0];
			} // if
		break;
		} // WCS_DIAGNOSTIC_SLOPE
	case WCS_DIAGNOSTIC_ASPECT:
		{
		if (AspectBuf && TreeBitmap[0])
			{
			DisplayBuffer = NewBuf;
			ConvertDiagnostics(1, &AspectBuf, WCS_DIAGNOSTIC_ASPECT);
			PlotBuf[0] = TreeBitmap[0];
			PlotBuf[1] = TreeBitmap[0];
			PlotBuf[2] = TreeBitmap[0];
			} // if
		break;
		} // WCS_DIAGNOSTIC_ASPECT
	case WCS_DIAGNOSTIC_RELEL:
		{
		if (RelElBuf && TreeBitmap[0])
			{
			DisplayBuffer = NewBuf;
			ConvertDiagnostics(1, &RelElBuf, WCS_DIAGNOSTIC_RELEL);
			PlotBuf[0] = TreeBitmap[0];
			PlotBuf[1] = TreeBitmap[0];
			PlotBuf[2] = TreeBitmap[0];
			} // if
		break;
		} // WCS_DIAGNOSTIC_RELEL
	case WCS_DIAGNOSTIC_ILLUMINATION:
		{
		if (IllumBuf && TreeBitmap[0])
			{
			DisplayBuffer = NewBuf;
			ConvertDiagnostics(1, &IllumBuf, WCS_DIAGNOSTIC_ILLUMINATION);
			PlotBuf[0] = TreeBitmap[0];
			PlotBuf[1] = TreeBitmap[0];
			PlotBuf[2] = TreeBitmap[0];
			} // if
		break;
		} // WCS_DIAGNOSTIC_ILLUMINATION
	case WCS_DIAGNOSTIC_REFLECTION:
		{
		if (ReflectionBuf && TreeBitmap[0])
			{
			DisplayBuffer = NewBuf;
			ConvertDiagnostics(1, &ReflectionBuf, WCS_DIAGNOSTIC_REFLECTION);
			PlotBuf[0] = TreeBitmap[0];
			PlotBuf[1] = TreeBitmap[0];
			PlotBuf[2] = TreeBitmap[0];
			} // if
		break;
		} // WCS_DIAGNOSTIC_REFLECTION
	case WCS_DIAGNOSTIC_Z:
		{
		if (ZBuf && TreeBitmap[0])
			{
			DisplayBuffer = NewBuf;
			ConvertDiagnostics(1, &ZBuf, WCS_DIAGNOSTIC_Z);
			PlotBuf[0] = TreeBitmap[0];
			PlotBuf[1] = TreeBitmap[0];
			PlotBuf[2] = TreeBitmap[0];
			} // if
		break;
		} // WCS_DIAGNOSTIC_Z
	case WCS_DIAGNOSTIC_NORMALX:
	case WCS_DIAGNOSTIC_NORMALY:
	case WCS_DIAGNOSTIC_NORMALZ:
		{
		if (NormalBuf[0] && NormalBuf[1] && NormalBuf[2] && TreeBitmap[0] && TreeBitmap[1] && TreeBitmap[2])
			{
			DisplayBuffer = NewBuf;
			ConvertDiagnostics(3, NormalBuf, WCS_DIAGNOSTIC_NORMALX);
			PlotBuf[0] = TreeBitmap[0];
			PlotBuf[1] = TreeBitmap[1];
			PlotBuf[2] = TreeBitmap[2];
			} // if
		break;
		} // WCS_DIAGNOSTIC_NORMALX
	case WCS_DIAGNOSTIC_RGB:
		{
		if (Bitmap[0] && Bitmap[1] && Bitmap[2] && TreeBitmap[0] && TreeBitmap[1] && TreeBitmap[2])
			{
			DisplayBuffer = NewBuf;
			ConvertDiagnostics(3, (float **)Bitmap, WCS_DIAGNOSTIC_RGB, ProcessMe);
			PlotBuf[0] = TreeBitmap[0];
			PlotBuf[1] = TreeBitmap[1];
			PlotBuf[2] = TreeBitmap[2];
			} // if
		break;
		} // WCS_DIAGNOSTIC_RGB
	case WCS_DIAGNOSTIC_ALPHA:
		{
		if (AABuf)
			{
			DisplayBuffer = NewBuf;
			PlotBuf[0] = AABuf;
			PlotBuf[1] = AABuf;
			PlotBuf[2] = AABuf;
			} // if
		break;
		} // WCS_DIAGNOSTIC_ALPHA
	case WCS_DIAGNOSTIC_OBJECT:
		{
		if (ObjectBuf && TreeBitmap[0] && TreeBitmap[1] && TreeBitmap[2])
			{
			DisplayBuffer = NewBuf;
			ConvertDiagnostics(3, (float **)&ObjectBuf, WCS_DIAGNOSTIC_OBJECT);
			PlotBuf[0] = TreeBitmap[0];
			PlotBuf[1] = TreeBitmap[1];
			PlotBuf[2] = TreeBitmap[2];
			} // if
		break;
		} // WCS_DIAGNOSTIC_OBJECT
	} // switch

if (Pane)
	Pane->Clear();
UpdatePreview();

Data.ThresholdValid = ThresholdValid;
Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_UPDATETHRESHOLD, Threshold[0], Threshold[1]);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, &Data);

} // Renderer::SetDisplayBuffer

/*===========================================================================*/

void Renderer::SetDisplayThreshold(unsigned char LowThresh, unsigned char HighThresh)
{

if (DisplayBuffer == WCS_DIAGNOSTIC_RGB)
	{
	Threshold[0] = LowThresh;
	Threshold[1] = HighThresh;
	} // if
else
	{
	Threshold[0] = min(LowThresh, 99);
	Threshold[1] = max(LowThresh + 1, HighThresh);
	} // else

if (ThresholdValid)
	SetDisplayBuffer(DisplayBuffer);

} // Renderer::SetDisplayThreshold

/*===========================================================================*/

void Renderer::ConvertDiagnostics(int NumBuffers, float **InBuf, unsigned char BufType, PostProcess *ProcessMe)
{
long Ct, BufCt, MaxCt;
float ValMax, ValMin, ValScale, ValRange;
RasterAnimHost **ObjBuf;
unsigned char Mask, InvMask;//, TransferVal[3];

MaxCt = Width * Height;
for (BufCt = 0; BufCt < NumBuffers; BufCt ++)
	memset(TreeBitmap[BufCt], 0, Width * Height);
if (BufType == WCS_DIAGNOSTIC_RGB)
	{
	/*
	if (ExponentBuf)
		{
		for (Ct = 0; Ct < MaxCt; Ct ++)
			{
			GetDisplayPixelWithExponent(Bitmap, ExponentBuf, Ct, TransferVal);
			TreeBitmap[0][Ct] = TransferVal[0];
			TreeBitmap[1][Ct] = TransferVal[1];
			TreeBitmap[2][Ct] = TransferVal[2];
			} // for
		} // if
	else
	*/
		{
		memcpy(TreeBitmap[0], Bitmap[0], MaxCt);
		memcpy(TreeBitmap[1], Bitmap[1], MaxCt);
		memcpy(TreeBitmap[2], Bitmap[2], MaxCt);
		} // else
	if (Threshold[0] >= 12)
		{
		Mask = 0xff << (Threshold[0] / 12);
		InvMask = ~Mask;
		for (Ct = 0; Ct < MaxCt; Ct ++)
			{
			TreeBitmap[0][Ct] = (unsigned char)((Mask & Bitmap[0][Ct]) + ((InvMask & Bitmap[0][Ct]) * Threshold[1]) / 100);
			TreeBitmap[1][Ct] = (unsigned char)((Mask & Bitmap[1][Ct]) + ((InvMask & Bitmap[1][Ct]) * Threshold[1]) / 100);
			TreeBitmap[2][Ct] = (unsigned char)((Mask & Bitmap[2][Ct]) + ((InvMask & Bitmap[2][Ct]) * Threshold[1]) / 100);
			} // for
		} // if
	// handle post process previews
	if (GlobalApp->GUIWins->PPR && GlobalApp->GUIWins->PPR->PreviewEnabled && (ProcessMe || (ProcessMe = GlobalApp->GUIWins->PPR->GetActive())))
		{
		RenderPostProc(0, TreeBitmap, ProcessMe, 0);
		} // if
	} // if
else if (BufType == WCS_DIAGNOSTIC_OBJECT)
	{
	ObjBuf = (RasterAnimHost **)InBuf[0];
	for (Ct = 0; Ct < MaxCt; Ct ++)
		{
		if (ObjBuf[Ct])
			{
			RendRand.Seed64((unsigned long)ObjBuf[Ct], (unsigned long)ObjBuf[Ct]);
			TreeBitmap[0][Ct] = (unsigned char)(RendRand.GenPRN() * 255);
			TreeBitmap[1][Ct] = (unsigned char)(RendRand.GenPRN() * 255);
			TreeBitmap[2][Ct] = (unsigned char)(RendRand.GenPRN() * 255);
			} // if
		} // for
	} // if
else
	{
	if (Threshold[1] <= Threshold[0])
		{
		Threshold[0] = min(Threshold[0], 99);
		Threshold[1] = max(Threshold[0] + 1, Threshold[1]);
		} // if
	for (BufCt = 0; BufCt < NumBuffers; BufCt ++)
		{
		ValMin = FLT_MAX;
		ValMax = -FLT_MAX;
		for (Ct = 0; Ct < MaxCt; Ct ++)
			{
			if (ZBuf[Ct] < FLT_MAX)
				{
				if (InBuf[BufCt][Ct] < ValMin)
					ValMin = InBuf[BufCt][Ct];
				if (InBuf[BufCt][Ct] > ValMax)
					ValMax = InBuf[BufCt][Ct];
				} // if
			} // for
		if (ValMax > ValMin)
			{
			ValRange = ValMax - ValMin;
			if (ThresholdValid)
				{
				ValMin = ValMin + (ValRange * (float)Threshold[0]) / (float)100;
				ValMax = ValMin + (ValRange * (float)Threshold[1]) / (float)100;
				} // if
			if (ValMax > ValMin)
				{
				ValScale = (float)255.9 / (ValMax - ValMin);
				for (Ct = 0; Ct < MaxCt; Ct ++)
					{
					if (ZBuf[Ct] < FLT_MAX && InBuf[BufCt][Ct] > ValMin)
						{
						if (InBuf[BufCt][Ct] >= ValMax)
							TreeBitmap[BufCt][Ct] = 255;
						else
							TreeBitmap[BufCt][Ct] = (unsigned char)((InBuf[BufCt][Ct] - ValMin) * ValScale);
						} // if
					} // for
				} // if
			} // if
		} // for
	} // else

} // Renderer::ConvertDiagnostics

/*===========================================================================*/

void Renderer::SaveDisplayedBuffers(int SaveDefault)
{
ImageOutputEvent IOEvent;
char FileName[256], *Extension, *DefBuf;
BufferNode *CurBuf, *RootBuf;

#ifdef WCS_BUILD_DEMO
if (SaveDefault) return;
#endif // WCS_BUILD_DEMO
if (! PlotBuf[0])
	PlotBuf[0] = Bitmap[0];
if (! PlotBuf[1])
	PlotBuf[1] = Bitmap[1];
if (! PlotBuf[2])
	PlotBuf[2] = Bitmap[2];

strcpy(IOEvent.FileType, "PNG");
strcpy(IOEvent.OutBuffers[0], "RED");
strcpy(IOEvent.OutBuffers[1], "GREEN");
strcpy(IOEvent.OutBuffers[2], "BLUE");
// do this only if displayed buffers are RGB
if (ExponentBuf && DisplayBuffer == WCS_DIAGNOSTIC_RGB)
	strcpy(IOEvent.OutBuffers[3], "RGB EXPONENT");
if (DefBuf = ImageSaverLibrary::GetNextCodec(IOEvent.FileType, NULL))
	strcpy(IOEvent.Codec, DefBuf);
if (SaveDefault)
	IOEvent.PAF.SetPathAndName("WCSFrames:", APP_TLA"LastRender");
else
	IOEvent.PAF.SetPathAndName(ProjectBase->framepath, "");
IOEvent.AutoExtension = 1;
IOEvent.AutoDigits = 0;

if (SaveDefault || GetFileNamePtrn(1, "Save Displayed Image", (char *)IOEvent.PAF.GetPath(), (char *)IOEvent.PAF.GetName(), WCS_REQUESTER_WILDCARD, WCS_PATHANDFILE_NAME_LEN))
	{
	strcpy(ProjectBase->framepath, IOEvent.PAF.GetPath());
	strcpy(ProjectBase->framefile, IOEvent.PAF.GetName());
	strcpy(FileName, (char *)IOEvent.PAF.GetName());
	if (Extension = FindFileExtension(FileName))
		{
		if (DefBuf = ImageSaverLibrary::GetFormatFromExtension(Extension))
			{
			strcpy(IOEvent.FileType, DefBuf);
			IOEvent.AutoExtension = 0;
			} // if
		else
			strcat(ProjectBase->framefile, ".png");
		} // if
	else
		strcat(ProjectBase->framefile, ".png");

	// create a set of Buffer Nodes
	if (CurBuf = RootBuf = new BufferNode("RED", WCS_RASTER_BANDSET_BYTE))
		{
		CurBuf->Buffer = PlotBuf[0];
		if (CurBuf = CurBuf->AddBufferNode("GREEN", WCS_RASTER_BANDSET_BYTE))
			{
			CurBuf->Buffer = PlotBuf[1];
			if (CurBuf = CurBuf->AddBufferNode("BLUE", WCS_RASTER_BANDSET_BYTE))
				{
				CurBuf->Buffer = PlotBuf[2];
				if (ExponentBuf && DisplayBuffer == WCS_DIAGNOSTIC_RGB && (CurBuf = CurBuf->AddBufferNode("RGB EXPONENT", WCS_RASTER_BANDSET_SHORT)))
					{
					CurBuf->Buffer = ExponentBuf;
					} // if

				// this sets up some necessary format-specific allocations
				IOEvent.InitSequence(NULL, RootBuf, Width, Height);

				// prep to save
				for (CurBuf = RootBuf; CurBuf; CurBuf = CurBuf->Next)
					{
					if (CurBuf->Buffer)
						{
						CurBuf->PrepToSave(Opt, 0, Width, 0);
						} // if
					} // for

				// Save it
				IOEvent.SaveImage(NULL, RootBuf, Width, Height, 0, Opt);

				// Cleanup all prep work
				for (CurBuf = RootBuf; CurBuf; CurBuf = CurBuf->Next)
					{
					if (CurBuf->Buffer)
						{
						CurBuf->CleanupFromSave();
						} // if
					} // for
				} // if
			} // if

		while (RootBuf)
			{
			CurBuf = RootBuf->Next;
			delete RootBuf;
			RootBuf = CurBuf;
			} // if

		} // if
	} // if file name

} // Renderer::SaveDisplayedBuffers

/*===========================================================================*/

int Renderer::ApplyPostProc(PostProcess *ActivePostProc)
{
int DiagnosticsRGB;

DiagnosticsRGB = GlobalApp->GUIWins->RDG ? GlobalApp->GUIWins->RDG->DisplayBuffer == WCS_DIAGNOSTIC_RGB: 1;

if (! ActivePostProc)
	{
	if (GlobalApp->GUIWins->PPR && (ActivePostProc = GlobalApp->GUIWins->PPR->GetActive()))
		{
		if (! GlobalApp->GUIWins->PPR->PreviewEnabled)
			{
			// no harm done - just don't render it
			return (1);
			} // if
		} // if
	} // if

if (DiagnosticsRGB && ActivePostProc)
	{
	SetDisplayBuffer(WCS_DIAGNOSTIC_RGB, ActivePostProc);
	} // if

return (1);

} // Renderer::ApplyPostProc

/*===========================================================================*/

int Renderer::AddFoliageList(RealtimeFoliageData *FolDat)
{

if (! CurFolListDat)
	FolListDat = FolDat;
else
	CurFolListDat->Next = FolDat;

CurFolListDat = FolDat;

return (1);

} // Renderer::AddFoliageList

/*===========================================================================*/

int Renderer::AddLabelList(RealtimeFoliageData *LabelDat)
{

if (! CurLabelListDat)
	LabelListDat = LabelDat;
else
	CurLabelListDat->Next = LabelDat;

CurLabelListDat = LabelDat;

return (1);

} // Renderer::AddLabelList

/*===========================================================================*/

int Renderer::ProcessFoliageList(void)
{
double CellXYZ[3], XMax, XMin, YMax, YMin, ZMax, ZMin, DataVolume, VolumePerCell, CellRad,
	BoundsXMax, BoundsXMin, BoundsYMax, BoundsYMin, BoundsZMax, BoundsZMin;
RealtimeFoliageData *CurDat;
FILE *ffile, *ffile2;
long FileCt, NumCells, NewCells, XCells, YCells, ZCells, CellX, CellY, CellZ, ActualFiles, CellsWritePos, DatCt = 0, ListNum;
char FileName[512], BaseName[64], FileDescriptor[256], FileVersion = WCS_FOLIAGELIST_FILE_VERSION, *Ext;

if (!RealTimeFoliageWrite) return(0);

if (FolListDat)
	{
	// determine number, size and centers of data cells
	XMin = YMin = ZMin = FLT_MAX;
	XMax = YMax = ZMax = -FLT_MAX;
	CurDat = FolListDat;
	while (CurDat)
		{
		if (CurDat->XYZ[0] < XMin)
			XMin = CurDat->XYZ[0];
		if (CurDat->XYZ[1] < YMin)
			YMin = CurDat->XYZ[1];
		if (CurDat->XYZ[2] < ZMin)
			ZMin = CurDat->XYZ[2];
		if (CurDat->XYZ[0] > XMax)
			XMax = CurDat->XYZ[0];
		if (CurDat->XYZ[1] > YMax)
			YMax = CurDat->XYZ[1];
		if (CurDat->XYZ[2] > ZMax)
			ZMax = CurDat->XYZ[2];
		DatCt ++;
		CurDat = CurDat->Next;
		} // while
	} // if
if (LabelListDat)
	{
	// determine number, size and centers of data cells
	XMin = YMin = ZMin = FLT_MAX;
	XMax = YMax = ZMax = -FLT_MAX;
	CurDat = LabelListDat;
	while (CurDat)
		{
		if (CurDat->XYZ[0] < XMin)
			XMin = CurDat->XYZ[0];
		if (CurDat->XYZ[1] < YMin)
			YMin = CurDat->XYZ[1];
		if (CurDat->XYZ[2] < ZMin)
			ZMin = CurDat->XYZ[2];
		if (CurDat->XYZ[0] > XMax)
			XMax = CurDat->XYZ[0];
		if (CurDat->XYZ[1] > YMax)
			YMax = CurDat->XYZ[1];
		if (CurDat->XYZ[2] > ZMax)
			ZMax = CurDat->XYZ[2];
		DatCt ++;
		CurDat = CurDat->Next;
		} // while
	} // if
if (FolListDat || LabelListDat)
	{
	DataVolume = (XMax - XMin) * (YMax - YMin) * (ZMax - ZMin);

	// given some theoretical best number of trees per cell we make a first guess at the size of the cells
	// we start with the assumption that 1000 per cell is reasonable but we limit the number of cells to 20

	NumCells = DatCt / RealTimeFoliageWrite->StemsPerCell;
	NumCells = NumCells < 1 ? 1: NumCells > RealTimeFoliageWrite->NumFiles ? RealTimeFoliageWrite->NumFiles: NumCells;

	VolumePerCell = DataVolume / NumCells;
	if (VolumePerCell > 1.0)
		{
		CellRad = pow(VolumePerCell, 1.0 / 3.0);

		NewCellRadCalc:

		XCells = quicklongceil((XMax - XMin) / CellRad);
		YCells = quicklongceil((YMax - YMin) / CellRad);
		ZCells = quicklongceil((ZMax - ZMin) / CellRad);
		if (XCells <= 0)
			XCells = 1;
		if (YCells <= 0)
			YCells = 1;
		if (ZCells <= 0)
			ZCells = 1;
		NewCells = XCells * YCells * ZCells;

		if (NewCells > NumCells)
			{
			CellRad *= 2.0;
			goto NewCellRadCalc;
			} // while

		CellRad *= .5;
		NumCells = NewCells;
		} // if
	else
		{
		CellRad = .5;
		NumCells = 1;
		XCells = YCells = ZCells = 1;
		} // else

	XMin = (XMax + XMin) * .5 - XCells * CellRad;
	YMin = (YMax + YMin) * .5 - YCells * CellRad;
	ZMin = (ZMax + ZMin) * .5 - ZCells * CellRad;

	// lop off extension if provided -- we will add back as necessary
	if (Ext = FindFileExtension(RealTimeFoliageWrite->BaseName))
		{
		Ext[0] = NULL;
		} // if
	// write index file
	sprintf(BaseName, "%s.dat", RealTimeFoliageWrite->BaseName);
	strmfp(FileName, RealTimeFoliageWrite->DirName[0] ? RealTimeFoliageWrite->DirName: ProjectBase->dirname, BaseName);
	if (ffile = PROJ_fopen(FileName, "wb"))
		{
		// file descriptor
		sprintf(FileDescriptor, "%s Foliage List Index File", ProjectBase->projectname);
		fwrite((char *)FileDescriptor, strlen(FileDescriptor) + 1, 1, ffile);	// includes null terminator
		fputc('\n', ffile);	// so variable length file descriptor can be read by fgets
		// file version number
		fwrite((char *)&FileVersion, sizeof (char), 1, ffile);
		// number of files
		CellsWritePos = ftell(ffile);
		fwrite((char *)&NumCells, sizeof (long), 1, ffile);
		// reference XYZ
		#ifdef WCS_BUILD_RTX
		if (Exporter)
			{
			fwrite((char *)&TexRefVert->Lon, sizeof (double), 1, ffile);
			fwrite((char *)&TexRefVert->Lat, sizeof (double), 1, ffile);
			fwrite((char *)&TexRefVert->Elev, sizeof (double), 1, ffile);
			} // if
		else
		#endif // WCS_BUILD_RTX
			{
			fwrite((char *)&TexRefVert->XYZ[0], sizeof (double), 1, ffile);
			fwrite((char *)&TexRefVert->XYZ[1], sizeof (double), 1, ffile);
			fwrite((char *)&TexRefVert->XYZ[2], sizeof (double), 1, ffile);
			} // else
		// for each file
		// <<<>>> Should we use RealTimeFoliageWrite->NumFiles somehow here?
		ActualFiles = NumCells;
		for (CellX = FileCt = 0; CellX < XCells; CellX ++)
			{
			CellXYZ[0] = XMin + CellRad + CellX * CellRad * 2.0;
			BoundsXMin = CellXYZ[0] - CellRad;
			BoundsXMax = CellXYZ[0] + CellRad;

			for (CellY = 0; CellY < YCells; CellY ++)
				{
				CellXYZ[1] = YMin + CellRad + CellY * CellRad * 2.0;
				BoundsYMin = CellXYZ[1] - CellRad;
				BoundsYMax = CellXYZ[1] + CellRad;

				for (CellZ = 0; CellZ < ZCells; CellZ ++, FileCt ++)
					{
					CellXYZ[2] = ZMin + CellRad + CellZ * CellRad * 2.0;
					BoundsZMin = CellXYZ[2] - CellRad;
					BoundsZMax = CellXYZ[2] + CellRad;

					// check and see if there are any trees in the cell
					DatCt = 0;
					CurDat = FolListDat;
					for (ListNum = 0; ListNum < 2; ListNum ++)
						{
						while (CurDat)
							{
							#ifdef WCS_BUILD_RTX
							if (Exporter)
								{
								// they're all in -- it's only one file
								DatCt ++;
								} // if
							else
							#endif // WCS_BUILD_RTX
								{
								if (CurDat->XYZ[0] <= BoundsXMax && CurDat->XYZ[0] >= BoundsXMin &&
									CurDat->XYZ[1] <= BoundsYMax && CurDat->XYZ[1] >= BoundsYMin &&
									CurDat->XYZ[2] <= BoundsZMax && CurDat->XYZ[2] >= BoundsZMin)
									{
									DatCt ++;
									} // if
								} // else/dummy scope
							CurDat = CurDat->Next;
							} // while
						CurDat = LabelListDat;
						} // for
					
					// if there are trees
					if (DatCt)
						{
						// file name
						sprintf(BaseName, "%sFoliageList%d.dat", RealTimeFoliageWrite->BaseName, FileCt);
						fwrite((char *)BaseName, strlen(BaseName) + 1, 1, ffile);	// includes null terminator
						fputc('\n', ffile);	// so variable length file descriptor can be read by fgets
						// center XYZ
						fwrite((char *)&CellXYZ[0], sizeof (double), 1, ffile);
						fwrite((char *)&CellXYZ[1], sizeof (double), 1, ffile);
						fwrite((char *)&CellXYZ[2], sizeof (double), 1, ffile);
						// half cube cell dimension
						fwrite((char *)&CellRad, sizeof (double), 1, ffile);
						// number of trees in file
						fwrite((char *)&DatCt, sizeof (long), 1, ffile);
						// write tree file
						strmfp(FileName, RealTimeFoliageWrite->DirName[0] ? RealTimeFoliageWrite->DirName: ProjectBase->dirname, BaseName);
						if (ffile2 = PROJ_fopen(FileName, "wb"))
							{
							// write file descriptor
							fwrite((char *)BaseName, strlen(BaseName) + 1, 1, ffile2);
							fputc('\n', ffile2);	// so variable length file descriptor can be read by fgets
							// file version number
							fwrite((char *)&FileVersion, sizeof (char), 1, ffile2);
							CurDat = FolListDat;
							for (ListNum = 0; ListNum < 2; ListNum ++)
								{
								while (CurDat)
									{
									#ifdef WCS_BUILD_RTX
									if (Exporter)
										{
										// they're all in -- it's only one file
										CurDat->WriteFoliageRecord(ffile2);
										} // if
									else
									#endif // WCS_BUILD_RTX
										{
										// check and see if tree is in this cell
										if (CurDat->XYZ[0] <= BoundsXMax && CurDat->XYZ[0] >= BoundsXMin &&
											CurDat->XYZ[1] <= BoundsYMax && CurDat->XYZ[1] >= BoundsYMin &&
											CurDat->XYZ[2] <= BoundsZMax && CurDat->XYZ[2] >= BoundsZMin)
											{
											CurDat->WriteFoliageRecord(ffile2);
											} // if
										} // else/dummy scope
									CurDat = CurDat->Next;
									} // while
								CurDat = LabelListDat;
								} // for
							fclose(ffile2);
							} // if
						} // if
					else
						ActualFiles --;
					} // for
				} // for
			} // for
		if (ActualFiles != NumCells && CellsWritePos > 0)
			{
			if (! fseek(ffile, CellsWritePos, SEEK_SET))
				fwrite((char *)&ActualFiles, sizeof (long), 1, ffile);
			} // if
		fclose(ffile);
		} // if
	} // if

// test code to read the files just written
/*
strmfp(FileName, ProjectBase->dirname, "FoliageIndx.dat");
if (ffile = PROJ_fopen(FileName, "rb"))
	{
	// read file descriptor
	fgets(FileDescriptor, 256, ffile);
	// version
	fread((char *)&FileVersion, sizeof (char), 1, ffile);
	// number of files
	fread((char *)&NumCells, sizeof (long), 1, ffile);
	// reference XYZ
	fread((char *)&CellXYZ[0], sizeof (double), 1, ffile);
	fread((char *)&CellXYZ[1], sizeof (double), 1, ffile);
	fread((char *)&CellXYZ[2], sizeof (double), 1, ffile);
	// for each file
	for (FileCt = 0; FileCt < NumCells; FileCt ++)
		{
		// file name
		fgets(FileDescriptor, 256, ffile);
		// center XYZ
		fread((char *)&CellXYZ[0], sizeof (double), 1, ffile);
		fread((char *)&CellXYZ[1], sizeof (double), 1, ffile);
		fread((char *)&CellXYZ[2], sizeof (double), 1, ffile);
		// half cube cell dimension
		fread((char *)&CellRad, sizeof (double), 1, ffile);
		// number of trees in file
		if (fread((char *)&DatCt, sizeof (long), 1, ffile) != 1)
			break;
		} // for
	fclose(ffile);
	} // if index opened
*/
return (1);

} // Renderer::ProcessFoliageList


/*===========================================================================*/

void Renderer::DestroyFoliageList(void)
{
RealtimeFoliageData *CurDat;

while (FolListDat)
	{
	CurDat = FolListDat;
	FolListDat = FolListDat->Next;
	delete CurDat;
	} // if
while (LabelListDat)
	{
	CurDat = LabelListDat;
	LabelListDat = LabelListDat->Next;
	delete CurDat;
	} // if

} // Renderer::DestroyFoliageList

/*===========================================================================*/

void Renderer::SetRBounds(RasterBounds *RBounds, long CurWidth, long CurHeight, long PanoPanels, 
	long ImageSegments, int ConcatenateTiles)
{
double N, S, W, E, StashCenterX, StashCenterY;
long SaveWidth, SaveHeight, CenterXYModified = 0;
CoordSys *UnprojectSys;
VertexDEM Corner;

if (Cam)
	{
	#if (defined(WCS_RENDER_TILES) || defined(WCS_BUILD_RTX))
	if (XTiles > 1 || YTiles > 1)
		{
		if (ConcatenateTiles)
			{
			SaveWidth = CurWidth * XTiles;
			SaveHeight = CurHeight * YTiles;
			// set these to full image size position so world file writers unproject upper left of image correctly
			StashCenterX = Cam->CenterX;
			StashCenterY = Cam->CenterY;
			Cam->CenterX = TrueCenterX;
			Cam->CenterY = TrueCenterY;
			CenterXYModified = 1;
			} // if
		else
			{
			SaveWidth = CurWidth;
			SaveHeight = CurHeight;
			} // else
		} // if tiles
	else
	#endif // (defined(WCS_RENDER_TILES) || defined(WCS_BUILD_RTX))
		{
		SaveWidth = CurWidth * PanoPanels;
		SaveHeight = CurHeight * ImageSegments;
		// set these to full image size position so world file writers unproject upper left of image correctly
		StashCenterX = Cam->CenterX;
		StashCenterY = Cam->CenterY;
		Cam->CenterX = TrueCenterX;
		Cam->CenterY = TrueCenterY;
		CenterXYModified = 1;
		} // else

	UnprojectSys = DefCoords;
	#ifdef WCS_BUILD_VNS
	if (Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && Cam->Projected && Cam->Coords)
		{
		UnprojectSys = Cam->Coords;
		} // if
	#endif // WCS_BUILD_VNS
	// edge of UL (NW) corner pixel
	Corner.ScrnXYZ[0] = 0.0;
	Corner.ScrnXYZ[1] = 0.0;
	Corner.ScrnXYZ[2] = 1.0;
	Cam->UnProjectVertexDEM(UnprojectSys, &Corner, EarthLatScaleMeters, PlanetRad, 1);
	//N = Corner.Lat; 
	//W = Corner.Lon;
	N = Corner.xyz[1]; 
	W = Corner.xyz[0];

	// edge of LR (SE) corner pixel
	Corner.ScrnXYZ[0] = (double)SaveWidth;
	Corner.ScrnXYZ[1] = (double)SaveHeight;
	Corner.ScrnXYZ[2] = 1.0;
	Cam->UnProjectVertexDEM(UnprojectSys, &Corner, EarthLatScaleMeters, PlanetRad, 1);
	//S = Corner.Lat; 
	//E = Corner.Lon;
	S = Corner.xyz[1]; 
	E = Corner.xyz[0];

	RBounds->SetCoords(UnprojectSys);
	RBounds->SetOutsideBounds(N, S, W, E);
	RBounds->DeriveCoords(SaveHeight, SaveWidth);
	RBounds->NullReject = 1;
	RBounds->NullValue = -9999.0f;

	if (CenterXYModified)
		{
		Cam->CenterX = StashCenterX;
		Cam->CenterY = StashCenterY;
		} // if
	} // if

} // Renderer::SetRBounds

/*===========================================================================*/

void Renderer::CopySettings(Renderer *CopyFrom)
{

ExagerateElevLines = CopyFrom->ExagerateElevLines;
MaxFractalDepth = CopyFrom->MaxFractalDepth;
DefaultFrd = CopyFrom->DefaultFrd;
PhongTerrain = CopyFrom->PhongTerrain;
RasterTAsExist = CopyFrom->RasterTAsExist;
ShadowsExist = CopyFrom->ShadowsExist;
TerraffectorsExist = CopyFrom->TerraffectorsExist;
StreamsExist = CopyFrom->StreamsExist;
LakesExist = CopyFrom->LakesExist;
WaterExists = CopyFrom->WaterExists;
SnowsExist = CopyFrom->SnowsExist;
CmapsExist = CopyFrom->CmapsExist;
EcosExist = CopyFrom->EcosExist;
EcoRastersExist = CopyFrom->EcoRastersExist;
VolumetricAtmospheresExist = CopyFrom->VolumetricAtmospheresExist;
RenderBathos = CopyFrom->RenderBathos;
FieldInterval = CopyFrom->FieldInterval;
MoBlurInterval = CopyFrom->MoBlurInterval;
StashProjectTime = CopyFrom->StashProjectTime;
StashProjectFrameRate = CopyFrom->StashProjectFrameRate;
EarthLatScaleMeters = CopyFrom->EarthLatScaleMeters;
RefLonScaleMeters = CopyFrom->RefLonScaleMeters;
TexRefLat = CopyFrom->TexRefLat;
TexRefLon = CopyFrom->TexRefLon;
TexRefElev = CopyFrom->TexRefElev;
ElevDatum = CopyFrom->ElevDatum;
Exageration = CopyFrom->Exageration;
PlanetRad = CopyFrom->PlanetRad;
EarthRotation = CopyFrom->EarthRotation;
RenderTime = CopyFrom->RenderTime;
ShadowMinFolHt = CopyFrom->ShadowMinFolHt;
ShadowMapDistanceOffset = CopyFrom->ShadowMapDistanceOffset;
QMax = CopyFrom->QMax;
ZMergeDistance = CopyFrom->ZMergeDistance;
MinimumZ = CopyFrom->MinimumZ;
MaximumZ = CopyFrom->MaximumZ;

DefaultTerrainPar = CopyFrom->DefaultTerrainPar;
DefaultPlanetOpt = CopyFrom->DefaultPlanetOpt;
DefaultEnvironment = CopyFrom->DefaultEnvironment;
DefCoords = CopyFrom->DefCoords;

} // Renderer::CopySettings

/*===========================================================================*/
/*===========================================================================*/

VertexBase::VertexBase()
{

Q = ScrnXYZ[2] = ScrnXYZ[1] = ScrnXYZ[0] = XYZ[2] = XYZ[1] = XYZ[0] = xyz[2] = xyz[1] = xyz[0] = 0.0;

} // VertexBase::VertexBase

/*===========================================================================*/

void VertexBase::GetPosVector(VertexBase *Origin)
{

XYZ[0] -= Origin->XYZ[0];
XYZ[1] -= Origin->XYZ[1];
XYZ[2] -= Origin->XYZ[2];

} // VertexBase::GetPosVector

/*===========================================================================*/

void VertexBase::UnGetPosVector(VertexBase *Origin)
{

XYZ[0] += Origin->XYZ[0];
XYZ[1] += Origin->XYZ[1];
XYZ[2] += Origin->XYZ[2];

} // VertexBase::UnGetPosVector

/*===========================================================================*/

void VertexBase::CopyVBase(VertexBase *CopyFrom)
{

xyz[0] = CopyFrom->xyz[0];
xyz[1] = CopyFrom->xyz[1];
xyz[2] = CopyFrom->xyz[2];

XYZ[0] = CopyFrom->XYZ[0];
XYZ[1] = CopyFrom->XYZ[1];
XYZ[2] = CopyFrom->XYZ[2];

ScrnXYZ[0] = CopyFrom->ScrnXYZ[0];
ScrnXYZ[1] = CopyFrom->ScrnXYZ[1];
ScrnXYZ[2] = CopyFrom->ScrnXYZ[2];
Q = CopyFrom->Q;

} // VertexBase::CopyVBase

/*===========================================================================*/

void VertexBase::CopyXYZ(VertexBase *CopyFrom)
{

XYZ[0] = CopyFrom->XYZ[0];
XYZ[1] = CopyFrom->XYZ[1];
XYZ[2] = CopyFrom->XYZ[2];

} // VertexBase::CopyXYZ

/*===========================================================================*/

void VertexBase::CopyScrnXYZQ(VertexBase *CopyFrom)
{

ScrnXYZ[0] = CopyFrom->ScrnXYZ[0];
ScrnXYZ[1] = CopyFrom->ScrnXYZ[1];
ScrnXYZ[2] = CopyFrom->ScrnXYZ[2];
Q = CopyFrom->Q;

} // VertexBase::CopyScrnXYZQ

/*===========================================================================*/

void VertexBase::UnitVector(void)
{
double Length;

Length = sqrt(XYZ[0] * XYZ[0] + XYZ[1] * XYZ[1] + XYZ[2] * XYZ[2]);
if (Length > 0.0)
	{
	XYZ[0] /= Length;
	XYZ[1] /= Length;
	XYZ[2] /= Length;
	} // if

} // VertexBase::UnitVector

/*===========================================================================*/

void VertexBase::AddXYZ(VertexBase *Increment)
{

XYZ[0] += Increment->XYZ[0];
XYZ[1] += Increment->XYZ[1];
XYZ[2] += Increment->XYZ[2];

} // VertexBase::AddXYZ

/*===========================================================================*/

void VertexBase::MultiplyXYZ(double Increment)
{

XYZ[0] *= Increment;
XYZ[1] *= Increment;
XYZ[2] *= Increment;

} // VertexBase::MultiplyXYZ

/*===========================================================================*/

void VertexBase::RotateX(double Angle)
{
double NewY, NewZ, SinAngle, CosAngle;

Angle *= PiOver180;
/***
SinAngle = sin(Angle);
CosAngle = cos(Angle);
***/
sincos(Angle, &SinAngle, &CosAngle);
NewZ = XYZ[2] * CosAngle + XYZ[1] * SinAngle;
NewY = XYZ[1] * CosAngle - XYZ[2] * SinAngle;

XYZ[1] = NewY;
XYZ[2] = NewZ;

} // VertexBase::RotateX

/*===========================================================================*/

void VertexBase::RotateY(double Angle)
{
double NewX, NewZ, SinAngle, CosAngle;

Angle *= PiOver180;
/***
SinAngle = sin(Angle);
CosAngle = cos(Angle);
***/
sincos(Angle, &SinAngle, &CosAngle);
NewX = XYZ[0] * CosAngle + XYZ[2] * SinAngle;
NewZ = XYZ[2] * CosAngle - XYZ[0] * SinAngle;

XYZ[0] = NewX;
XYZ[2] = NewZ;

} // VertexBase::RotateY

/*===========================================================================*/

void VertexBase::RotateZ(double Angle)
{
double NewX, NewY, SinAngle, CosAngle;

Angle *= PiOver180;
/***
SinAngle = sin(Angle);
CosAngle = cos(Angle);
***/
sincos(Angle, &SinAngle, &CosAngle);
NewX = XYZ[0] * CosAngle + XYZ[1] * SinAngle;
NewY = XYZ[1] * CosAngle - XYZ[0] * SinAngle;

XYZ[0] = NewX;
XYZ[1] = NewY;

} // VertexBase::RotateZ

/*===========================================================================*/

// clockwise is positive angle looking from +y
double VertexBase::FindAngleYfromZ(void)
{
double Angle;

if (XYZ[2] == 0.0)
	{
	if (XYZ[0] == 0.0)
		Angle = 0.0;	// this is arbitrary, angle is actually undefined
	else if (XYZ[0] > 0.0)
		Angle = 90.0;
	else
		Angle = -90.0;
	} // if
else
	{
	Angle = atan(XYZ[0] / XYZ[2]) * PiUnder180;
	if (XYZ[2] < 0.0)
		Angle += 180.0;
	} // else

return (Angle);

} // VertexBase::FindAngleYfromZ

/*===========================================================================*/

// clockwise is positive angle looking from +y
// uses approximated atan for faster but less precise results, adequate for aspect, etc
double VertexBase::FindRoughAngleYfromZ(void)
{
double Angle;

if (XYZ[2] == 0.0)
	{
	if (XYZ[0] == 0.0)
		Angle = 0.0;	// this is arbitrary, angle is actually undefined
	else if (XYZ[0] > 0.0)
		Angle = 90.0;
	else
		Angle = -90.0;
	} // if
else
	{
	Angle = fastatanf((float)(XYZ[0] / XYZ[2])) * PiUnder180;
	//Angle = atan(XYZ[0] / XYZ[2]) * PiUnder180;
	if (XYZ[2] < 0.0)
		Angle += 180.0;
	} // else

return (Angle);

} // VertexBase::FindRoughAngleYfromZ

/*===========================================================================*/

// clockwise is positive angle looking from +x
double VertexBase::FindAngleXfromZ(void)
{
double Angle;

if (XYZ[2] == 0.0)
	{
	if (XYZ[1] == 0.0)
		Angle = 0.0;	// this is arbitrary, angle is actually undefined
	else if (XYZ[1] > 0.0)
		Angle = -90.0;
	else
		Angle = 90.0;
	} // if
else
	{
	Angle = -atan(XYZ[1] / XYZ[2]) * PiUnder180;
	if (XYZ[2] < 0.0)
		Angle += 180.0;
	} // else

return (Angle);

} // VertexBase::FindAngleXfromZ

/*===========================================================================*/

// clockwise is positive angle looking from +x
// uses approximated atan for faster but less precise results, adequate for aspect, etc
double VertexBase::FindRoughAngleXfromZ(void)
{
double Angle;

if (XYZ[2] == 0.0)
	{
	if (XYZ[1] == 0.0)
		Angle = 0.0;	// this is arbitrary, angle is actually undefined
	else if (XYZ[1] > 0.0)
		Angle = -90.0;
	else
		Angle = 90.0;
	} // if
else
	{
	//Angle = -atan(XYZ[1] / XYZ[2]) * (1.0 / PiOver180);
	Angle = -fastatanf((float)(XYZ[1] / XYZ[2])) * PiUnder180;
	if (XYZ[2] < 0.0)
		Angle += 180.0;
	} // else

return (Angle);

} // VertexBase::FindRoughAngleXfromZ

/*===========================================================================*/

// clockwise is positive angle looking from +x
double VertexBase::FindAngleXfromY(void)
{
double Angle;

if (XYZ[1] == 0.0)
	{
	if (XYZ[2] == 0.0)
		Angle = 0.0;	// this is arbitrary, angle is actually undefined
	else if (XYZ[2] > 0.0)
		Angle = 90.0;
	else
		Angle = -90.0;
	} // if
else
	{
	Angle = atan(XYZ[2] / XYZ[1]) * PiUnder180;
	if (XYZ[1] < 0.0)
		Angle += 180.0;
	} // else

return (Angle);

} // VertexBase::FindAngleXfromY

/*===========================================================================*/

// clockwise is positive angle looking from +z
double VertexBase::FindAngleZfromY(void)
{
double Angle;

if (XYZ[1] == 0.0)
	{
	if (XYZ[0] == 0.0)
		Angle = 0.0;	// this is arbitrary, angle is actually undefined
	else if (XYZ[0] > 0.0)
		Angle = -90.0;
	else
		Angle = 90.0;
	} // if
else
	{
	Angle = -atan(XYZ[0] / XYZ[1]) * PiUnder180;
	if (XYZ[1] < 0.0)
		Angle += 180.0;
	} // else

return (Angle);

} // VertexBase::FindAngleZfromY

/*===========================================================================*/

void VertexBase::SetDefaultViewVector(void)
{

XYZ[0] = XYZ[1] = 0.0;
XYZ[2] = 1.0;

} // VertexBase::SetDefaultViewVector

/*===========================================================================*/

void VertexBase::SetDefaultCamVerticalVector(void)
{

XYZ[0] = XYZ[2] = 0.0;
XYZ[1] = 1.0;

} // VertexBase::SetDefaultCamVerticalVector

/*===========================================================================*/

void VertexBase::ProjectPoint(Matx3x3 M)
{

ScrnXYZ[0] = XYZ[0] * M[0][0] + XYZ[1] * M[0][1] + XYZ[2] * M[0][2];
ScrnXYZ[1] = XYZ[0] * M[1][0] + XYZ[1] * M[1][1] + XYZ[2] * M[1][2];
ScrnXYZ[2] = XYZ[0] * M[2][0] + XYZ[1] * M[2][1] + XYZ[2] * M[2][2];

} // VertexBase::ProjectPoint

/*===========================================================================*/

void VertexBase::UnProjectPoint(Matx3x3 M)
{

XYZ[0] = ScrnXYZ[0] * M[0][0] + ScrnXYZ[1] * M[0][1] + ScrnXYZ[2] * M[0][2];
XYZ[1] = ScrnXYZ[0] * M[1][0] + ScrnXYZ[1] * M[1][1] + ScrnXYZ[2] * M[1][2];
XYZ[2] = ScrnXYZ[0] * M[2][0] + ScrnXYZ[1] * M[2][1] + ScrnXYZ[2] * M[2][2];

} // VertexBase::UnProjectPoint

/*===========================================================================*/

void VertexBase::Transform3DPoint(Matx4x4 M)
{
double BXYZ[3];

BXYZ[0] = XYZ[0] * M[0][0] + XYZ[1] * M[0][1] + XYZ[2] * M[0][2] + M[0][3];
BXYZ[1] = XYZ[0] * M[1][0] + XYZ[1] * M[1][1] + XYZ[2] * M[1][2] + M[1][3];
BXYZ[2] = XYZ[0] * M[2][0] + XYZ[1] * M[2][1] + XYZ[2] * M[2][2] + M[2][3];

XYZ[0] = BXYZ[0];
XYZ[1] = BXYZ[1];
XYZ[2] = BXYZ[2];

} // VertexBase::Transform3DPoint() 

/*===========================================================================*/

void VertexBase::Transform3DPointTo_xyz(Matx4x4 M)
{

xyz[0] = XYZ[0] * M[0][0] + XYZ[1] * M[0][1] + XYZ[2] * M[0][2] + M[0][3];
xyz[1] = XYZ[0] * M[1][0] + XYZ[1] * M[1][1] + XYZ[2] * M[1][2] + M[1][3];
xyz[2] = XYZ[0] * M[2][0] + XYZ[1] * M[2][1] + XYZ[2] * M[2][2] + M[2][3];

} // VertexBase::Transform3DPoint() 

/*===========================================================================*/

void VertexBase::InterpolateVertexBase(VertexBase *FromVtx, VertexBase *ToVtx, double LerpVal)
{

xyz[0] = FromVtx->xyz[0] + LerpVal * (ToVtx->xyz[0] - FromVtx->xyz[0]);
xyz[1] = FromVtx->xyz[1] + LerpVal * (ToVtx->xyz[1] - FromVtx->xyz[1]);
xyz[2] = FromVtx->xyz[2] + LerpVal * (ToVtx->xyz[2] - FromVtx->xyz[2]);

XYZ[0] = FromVtx->XYZ[0] + LerpVal * (ToVtx->XYZ[0] - FromVtx->XYZ[0]);
XYZ[1] = FromVtx->XYZ[1] + LerpVal * (ToVtx->XYZ[1] - FromVtx->XYZ[1]);
XYZ[2] = FromVtx->XYZ[2] + LerpVal * (ToVtx->XYZ[2] - FromVtx->XYZ[2]);

ScrnXYZ[0] = FromVtx->ScrnXYZ[0] + LerpVal * (ToVtx->ScrnXYZ[0] - FromVtx->ScrnXYZ[0]);
ScrnXYZ[1] = FromVtx->ScrnXYZ[1] + LerpVal * (ToVtx->ScrnXYZ[1] - FromVtx->ScrnXYZ[1]);
ScrnXYZ[2] = FromVtx->ScrnXYZ[2] + LerpVal * (ToVtx->ScrnXYZ[2] - FromVtx->ScrnXYZ[2]);

Q = FromVtx->Q + LerpVal * (ToVtx->Q - FromVtx->Q);

} // VertexBase::InterpolateVertexBase

/*===========================================================================*/
/*===========================================================================*/

VertexDEM::VertexDEM()
{

Elev = Lon = Lat = 0.0;
Flags = 0;

} // VertexDEM::VertexDEM

/*===========================================================================*/

void VertexDEM::CopyVDEM(VertexDEM *CopyFrom)
{

Lat = CopyFrom->Lat;
Lon = CopyFrom->Lon;
Elev = CopyFrom->Elev;
Flags = CopyFrom->Flags;
CopyVBase(CopyFrom);

} // VertexDEM::CopyVDEM

/*===========================================================================*/

#ifdef SPEED_MOD
void FASTCALL VertexDEM::CopyLatLon(VertexDEM *CopyFrom)
#else // SPEED_MOD
void VertexDEM::CopyLatLon(VertexDEM *CopyFrom)
#endif // SPEED_MOD
{

Lat = CopyFrom->Lat;
Lon = CopyFrom->Lon;
Elev = CopyFrom->Elev;

} // VertexDEM::CopyLatLon

/*===========================================================================*/

void VertexDEM::DegToCart(double PlanetRad)
{
double Radius, Rsq, Rxz, Ysq, TempLon = Lon;

Radius = PlanetRad + Elev;
XYZ[1] = Radius * sin(Lat * PiOver180);
Rxz = (Rsq = Radius * Radius) - (Ysq = XYZ[1] * XYZ[1]);
Rxz = (Rxz <= 0.0 ? 0.0 : sqrt(Rxz));
XYZ[0] = Rxz * sin(-Lon * PiOver180);
Rsq = Rsq - Ysq - XYZ[0] * XYZ[0];
XYZ[2] = (Rsq <= 0.0 ? 0.0 : sqrt(Rsq));

if (fabs(TempLon) > 180.0)
	{
	TempLon += 180;
	if (fabs(TempLon) >= 360.0)
		TempLon = fmod(TempLon, 360.0);	// retains the original sign
	if (TempLon < 0.0)
		TempLon += 360.0;
	TempLon -= 180.0;
	} // if
// replaced by above
//while (TempLon > 180.0)
//	TempLon -= 360.0;
//while (TempLon < -180)
//	TempLon += 360.0;

if (fabs(TempLon) < 90.0)
	XYZ[2] = -XYZ[2];

} // VertexDEM::DegToCart

/*===========================================================================*/

void VertexDEM::CartToDeg(double PlanetRad)
{
double Radius, Ysq, Rxz;

Radius = sqrt(XYZ[0] * XYZ[0] + (Ysq = XYZ[1] * XYZ[1]) + XYZ[2] * XYZ[2]);
if (Radius > 0.0)
	{
	Lat = asin(XYZ[1] / Radius) * PiUnder180;
	Rxz = sqrt(Radius * Radius - Ysq);
	if (Rxz != 0.0)
		{
		Lon = -asin(XYZ[0] / Rxz) * PiUnder180;
		if (XYZ[2] > 0.0)
			{
			Lon = 180.0 - Lon;
			if (Lon > 180.0)
				{
				Lon -= 360.0;
				} // if
			} // if
		} // if
	else
		Lon = 0.0;
	} // if
else
	{
	Lat = 0.0;
	Lon = 0.0;
	} // else

Elev = Radius - PlanetRad;

} // VertexDEM::CartToDeg

/*===========================================================================*/

void VertexDEM::CartToDeg(double SMajor, double SMinor)
{
double Radius, RadiusSq, Ysq, RxzSq, Flattening, EccSq, EccPrimeSq;
double Phi, Lambda, Height, NofPhi, SinPhi, CosPhi, SinSqPhi, X, Y, Z, TempX, TempY, TempZ, p, Theta, 
	SinCubedTheta, CosCubedTheta;

Flattening = (SMajor - SMinor) / SMajor;
EccSq = 2.0 * Flattening - Flattening * Flattening;
EccPrimeSq = (SMajor * SMajor - SMinor * SMinor) / (SMinor * SMinor);

TempX = XYZ[0];
TempY = XYZ[1];
TempZ = XYZ[2];
if (SMinor != 0.0)
	{
	// convert coords to ECEF
	X = -TempZ;
	Y = TempX;
	Z = TempY;
	// from Dana 1996
	if (X != 0.0 || Y != 0.0)
		{
		p = sqrt(X * X + Y * Y);
		Theta = atan(Z * SMajor / (p * SMinor));
		sincos(Theta, &SinCubedTheta, &CosCubedTheta);
		Phi = atan((Z + EccPrimeSq * SMinor * SinCubedTheta * SinCubedTheta * SinCubedTheta) /
			(p - EccSq * SMajor * CosCubedTheta * CosCubedTheta * CosCubedTheta));
		Lambda = atan2(Y, X);
		sincos(Phi, &SinPhi, &CosPhi);
		SinSqPhi = SinPhi * SinPhi;
		NofPhi = SMajor / sqrt(1.0 - EccSq * SinSqPhi);
		Height = (p / CosPhi) - NofPhi;
		Lat = Phi * PiUnder180;
		// this changes the sign to WCS convention
		Lon = -Lambda * PiUnder180;
		Elev = Height;
		} // if not on polar axis
	else
		{
		if (Z > 0.0)
			Lat = 90.0;
		else if (Z < 0.0)
			Lat = -90.0;
		else
			Lat = 0.0;
		Lon = 0.0;
		Elev = fabs(Z) - SMinor;
		} // else point is on polar axis
	} // if an ellipsoid
else
	{
	RadiusSq = (TempX * TempX + (Ysq = TempY * TempY) + TempZ * TempZ);
	Radius = sqrt(RadiusSq);
	if (Radius > 0.0)
		{
		Lat = asin(TempY / Radius) * PiUnder180;
		RxzSq = (RadiusSq - Ysq);
		if (RxzSq != 0.0)
			{
			Lon = -asin(TempX / sqrt(RxzSq)) * PiUnder180;
			if (TempZ > 0.0)
				Lon = 180.0 - Lon;
			} // if
		else
			Lon = 0.0;
		} // if
	else
		{
		Lat = 0.0;
		Lon = 0.0;
		} // else

	Elev = Radius - SMajor;
	} // else sphere

} // VertexDEM::CartToDeg

/*===========================================================================*/

void VertexDEM::RotateToHome(void)
{

RotateY(-Lon);
RotateX(90.0 - Lat);

} // VertexDEM::RotateToHome

/*===========================================================================*/

void VertexDEM::RotateFromHome(void)
{

RotateX(Lat - 90.0);
RotateY(Lon);

} // VertexDEM::RotateFromHome

/*===========================================================================*/

void VertexDEM::InterpolateVertexDEM(VertexDEM *FromVtx, VertexDEM *ToVtx, double LerpVal)
{

Lat = FromVtx->Lat + LerpVal * (ToVtx->Lat - FromVtx->Lat);
Lon = FromVtx->Lon + LerpVal * (ToVtx->Lon - FromVtx->Lon);
Elev = FromVtx->Elev + LerpVal * (ToVtx->Elev - FromVtx->Elev);

Flags = FromVtx->Flags;

InterpolateVertexBase(FromVtx, ToVtx, LerpVal);

} // VertexDEM::InterpolateVertexDEM

/*===========================================================================*/
/*===========================================================================*/

VertexCloud::VertexCloud()
{

//RGB[0] = RGB[1] = RGB[2];

} // VertexCloud::VertexCloud

/*===========================================================================*/

void VertexCloud::InterpolateVertexCloud(VertexCloud *FromVtx, VertexCloud *ToVtx, double LerpVal)
{

InterpolateVertexDEM(FromVtx, ToVtx, LerpVal);

} // VertexCloud::InterpolateVertexCloud

/*===========================================================================*/
/*===========================================================================*/

VertexData::VertexData()
{

Eco = NULL;
Lake = NULL;
Stream = NULL;
Vector = NULL;
Beach = NULL;
Map = NULL;

//TexDataInitialized = 0;
LatSeed = LonSeed = 0;
WaterElev = -FLT_MAX;
WaveHeight = WaterDepth = Displacement = oElev = oDisplacement = BeachLevel = 0.0;
VectorSlope = RelEl = VecOffsets[2] = VecOffsets[1] = VecOffsets[0] = 0.0f;
Frd = 0;
VectorType = 0;
#ifdef WCS_VECPOLY_EFFECTS
NodeData = NULL;	// this is owned by a VectorNode, do not delete it here
#endif // WCS_VECPOLY_EFFECTS

EdgeParent = NULL;

} // VertexData::VertexData

/*===========================================================================*/

VertexData::VertexData(PolygonEdge *NewEdgeParent)
{

Eco = NULL;
Lake = NULL;
Stream = NULL;
Vector = NULL;
Beach = NULL;
Map = NULL;

//TexDataInitialized = 0;
LatSeed = LonSeed = 0;
WaterElev = -FLT_MAX;
WaveHeight = WaterDepth = Displacement = oElev = oDisplacement = BeachLevel = 0.0;
VectorSlope = RelEl = VecOffsets[2] = VecOffsets[1] = VecOffsets[0] = 0.0f;
Frd = 0;
VectorType = 0;
#ifdef WCS_VECPOLY_EFFECTS
NodeData = NULL;	// this is owned by a VectorNode, do not delete it here
#endif // WCS_VECPOLY_EFFECTS

EdgeParent = NewEdgeParent;

} // VertexData::VertexData

/*===========================================================================*/

void VertexData::InterpolateVertexData(VertexData *FromVtx, VertexData *ToVtx, double LerpVal)
{

WaterElev = FromVtx->WaterElev + LerpVal * (ToVtx->WaterElev - FromVtx->WaterElev);
WaveHeight = FromVtx->WaveHeight + LerpVal * (ToVtx->WaveHeight - FromVtx->WaveHeight);
WaterDepth = FromVtx->WaterDepth + LerpVal * (ToVtx->WaterDepth - FromVtx->WaterDepth);
VectorSlope = FromVtx->VectorSlope + (float)LerpVal * (ToVtx->VectorSlope - FromVtx->VectorSlope);
RelEl = FromVtx->RelEl + (float)LerpVal * (ToVtx->RelEl - FromVtx->RelEl);
Displacement = FromVtx->Displacement + LerpVal * (ToVtx->Displacement - FromVtx->Displacement);
oElev = FromVtx->oElev + LerpVal * (ToVtx->oElev - FromVtx->oElev);
oDisplacement = FromVtx->oDisplacement + LerpVal * (ToVtx->oDisplacement - FromVtx->oDisplacement);
BeachLevel = FromVtx->BeachLevel + LerpVal * (ToVtx->BeachLevel - FromVtx->BeachLevel);

Eco = FromVtx->Eco;
Lake = FromVtx->Lake;
Stream = FromVtx->Stream;
Beach = FromVtx->Beach;
Vector = FromVtx->Vector;
Map = FromVtx->Map;
LatSeed = FromVtx->LatSeed;
LonSeed = FromVtx->LonSeed;
VectorType = FromVtx->VectorType;
Frd = FromVtx->Frd;
if (Vector)
	{
	VecOffsets[0] = FromVtx->VecOffsets[0] + (float)LerpVal * (ToVtx->VecOffsets[0] - FromVtx->VecOffsets[0]);
	VecOffsets[1] = FromVtx->VecOffsets[1] + (float)LerpVal * (ToVtx->VecOffsets[1] - FromVtx->VecOffsets[1]);
	VecOffsets[2] = FromVtx->VecOffsets[2] + (float)LerpVal * (ToVtx->VecOffsets[2] - FromVtx->VecOffsets[2]);
	} // if
	
#ifdef WCS_VECPOLY_EFFECTS
if (FromVtx->NodeData && ToVtx->NodeData)
	{
	if (NodeData || (NodeData = new VectorNodeRenderData()))
		{
		NodeData->NodeEnvironment = FromVtx->NodeData->NodeEnvironment;
		NodeData->RelEl = FromVtx->NodeData->RelEl + (float)LerpVal * (ToVtx->NodeData->RelEl - FromVtx->NodeData->RelEl);
		NodeData->Normal[2] = FromVtx->NodeData->Normal[2] + (float)LerpVal * (ToVtx->NodeData->Normal[2] - FromVtx->NodeData->Normal[2]);
		NodeData->Normal[1] = FromVtx->NodeData->Normal[1] + (float)LerpVal * (ToVtx->NodeData->Normal[1] - FromVtx->NodeData->Normal[1]);
		NodeData->Normal[0] = FromVtx->NodeData->Normal[0] + (float)LerpVal * (ToVtx->NodeData->Normal[0] - FromVtx->NodeData->Normal[0]);
		NodeData->Slope = FromVtx->NodeData->Slope + (float)LerpVal * (ToVtx->NodeData->Slope - FromVtx->NodeData->Slope);
		NodeData->Aspect = FromVtx->NodeData->Aspect + (float)LerpVal * (ToVtx->NodeData->Aspect - FromVtx->NodeData->Aspect);
		NodeData->TexDisplacement = FromVtx->NodeData->TexDisplacement + (float)LerpVal * (ToVtx->NodeData->TexDisplacement - FromVtx->NodeData->TexDisplacement);
		NodeData->TfxDisplacement = FromVtx->NodeData->TfxDisplacement + (float)LerpVal * (ToVtx->NodeData->TfxDisplacement - FromVtx->NodeData->TfxDisplacement);
		NodeData->BeachLevel = FromVtx->NodeData->BeachLevel + (float)LerpVal * (ToVtx->NodeData->BeachLevel - FromVtx->NodeData->BeachLevel);
		NodeData->Materials = NULL;
		NodeData->MaterialListOwned = true;
		} // if
	} // if
#endif // WCS_VECPOLY_EFFECTS

InterpolateVertexDEM(FromVtx, ToVtx, LerpVal);

} // VertexData::InterpolateVertexData

/*===========================================================================*/
/*===========================================================================*/

void PixelData::SetDefaults(void)
{

AltBacklightColor[2] = AltBacklightColor[1] = AltBacklightColor[0] = AltBacklightColorPct = Spec2RGBPct = SpecRGBPct = 
TranslucencyWt = SpecularWt2 = SpecularWt = OpacityUsed = OpticalDepth = WaterDepth = Reflectivity = TranslucencyExp = 
Translucency = SpecularExp2 = Specularity2 = SpecularExp = Specularity = Transparency = Luminosity = CloudShading = 
Illum = Spec2RGB[2] = Spec2RGB[1] = Spec2RGB[0] = SpecRGB[2] = SpecRGB[1] = SpecRGB[0] = RGB[2] = RGB[1] = RGB[0] = 0.0;

} // PixelData::SetDefaults

/*===========================================================================*/
/*===========================================================================*/

PolygonData::PolygonData()
{

Eco = NULL;
Lake = NULL;
Stream = NULL;
Beach = NULL;
Cmap = NULL;
Env = NULL;
Ground = NULL;
Snow = NULL;
Fnce = NULL;
Cloud = NULL;
CloudLayer = NULL;
Vector = NULL;
Object = NULL;
Plot3DRast = NULL;

LatSeed = LonSeed = 0;
PlotOffset3DX = PlotOffset3DY = 0;
Lat = Lon = Elev = WaterElev = Slope = Aspect = Z = Q = 
	OverstoryDissolve[1] = OverstoryDissolve[0] = UnderstoryDissolve[1] = UnderstoryDissolve[0] = 
	SnowCoverage = Area = RGB[2] = RGB[1] = RGB[0] = ShadowOffset = 0.0;
RelEl = VectorSlope = 0.0f;
TintFoliage = 0;	//TexDataInitialized = 0;
ShadeType = WCS_EFFECT_MATERIAL_SHADING_FLAT;
ShadowFlags = 0;
VectorType = RenderCmapColor = LuminousColor = 0;
FenceType = WCS_FENCEPIECE_SPAN;
ReceivedShadowIntensity = 1.0;
RenderPassWeight = 1.0;
VtxNum[2] = VtxNum[1] = VtxNum[0] = 0;
VertRefData[2] = VertRefData[1] = VertRefData[0] = 0;
Normal[1] = 1.0;
Normal[0] = Normal[2] = 0;
ViewVec[2] = 1.0;
ViewVec[0] = ViewVec[1] = 0;
// Normal[] is not initialized, nor is ViewVec[] because they are always calculated, 
// maybe some other stuff doesn't need it either

} // PolygonData::PolygonData

/*===========================================================================*/
/*===========================================================================*/

FoliagePlotData::FoliagePlotData()
{

WidthInPixels = HeightInPixels = TopElev = ElevGrad = Opacity = ColorImageOpacity = OrientationShading =
	MaxZOffset = TestPtrnOffset = 0.0;
ReplaceColor = NULL;
Poly = NULL;
Vert = NULL;
SourceRast = NULL;
RenderOccluded = ColorImage = Shade3D = 0;

} // FoliagePlotData::FoliagePlotData

/*===========================================================================*/
/*===========================================================================*/

bool RendererQuantaAllocatorErrorFunctor::operator() (QuantaAllocator *ManagingAllocator)
{

//UserMessageOK("QuantaAllocator", "Here, we will try to free some resources.", REQUESTER_ICON_STOP);

if (MyRend)
	{
	return (MyRend->FreeSomeQAResources(ManagingAllocator));
	} // if
	
return(false);

} // RendererQuantaAllocatorErrorFunctor::operator()
