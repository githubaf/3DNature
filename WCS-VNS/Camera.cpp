// Camera.cpp
// For managing WCS Cameras
// Built from scratch on 03/23/99 by Gary R. Huber
// Copyright 1999 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "Conservatory.h"
#include "CameraEditGUI.h"
#include "Project.h"
#include "Types.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Log.h"
#include "GraphData.h"
#include "Raster.h"
#include "requester.h"
#include "Render.h"
#include "Database.h"
#include "Interactive.h"
#include "ViewGUI.h"
#include "Security.h"
#include "AppMem.h"
#include "Lists.h"
#include "UsefulUnit.h"
#include "UsefulMath.h"

/*===========================================================================*/

Camera::Camera()
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_CAMERA;
SetDefaults();

} // Camera::Camera

/*===========================================================================*/

Camera::Camera(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_CAMERA;
SetDefaults();

} // Camera::Camera

/*===========================================================================*/

Camera::Camera(RasterAnimHost *RAHost, EffectsLib *Library, Camera *Proto)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_CAMERA;
Prev = Library->LastCamera;
if (Library->LastCamera)
	{
	Library->LastCamera->Next = this;
	Library->LastCamera = this;
	} // if
else
	{
	Library->Cameras = Library->LastCamera = this;
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
	strcpy(NameBase, "Camera");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // Camera::Camera

/*===========================================================================*/

Camera::~Camera()
{

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->CPG && GlobalApp->GUIWins->CPG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->CPG;
		GlobalApp->GUIWins->CPG = NULL;
		} // if
	} // if


if (CamPos)
	delete CamPos;
if (TargPos)
	delete TargPos;
if (CamVertical)
	delete CamVertical;
if (CamRight)
	delete CamRight;
CamPos = TargPos = CamVertical = CamRight = NULL;

if (Smooth)
	delete Smooth;
Smooth = NULL;

if (Img)
	{
	if (Img->GetRaster())
		{
		Img->GetRaster()->RemoveAttribute(Img);
		} // if
	// LINT thinks this test is redundant but it ain't - it can be NULLed by RemoveAttribute()
	if (Img)
		delete Img;
	Img = NULL;
	} // if
	
} // Camera::~Camera

/*===========================================================================*/

void Camera::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_CAMERA_NUMANIMPAR] = {0.0, 0.0, 20.0, 0.0, 0.0, 10.0, 0.0, 0.0, 0.0, 45.0, .5, .5,
	0.0, 0.0 /* Calculated at runtime below, formerly 1.49598e12 */, 0.0, 50.0, 35.0, 100.0, 5.6, .5, 100.0, 0.0, 0.0, 1000.0, 1.0, 30.0};
double RangeDefaults[WCS_EFFECTS_CAMERA_NUMANIMPAR][3] = {
												90.0, -90.0, .0001,	// cam lat
												FLT_MAX, -FLT_MAX, .0001,	// cam lon
												FLT_MAX, -FLT_MAX, 1.0,	// cam elev
												90.0, -90.0, .0001,	// target lat
												FLT_MAX, -FLT_MAX, .0001,	// target lon
												FLT_MAX, -FLT_MAX, 1.0,	// target elev
												FLT_MAX, -FLT_MAX, 1.0,	// heading
												FLT_MAX, -FLT_MAX, 1.0,	// pitch
												FLT_MAX, -FLT_MAX, 1.0,	// bank
												179.9, 0.0001, 1.0,	// fov
												#ifdef WCS_BUILD_DEMO
												.5, .5, 0.0,	// center x
												.5, .5, 0.0,	// center y
												#else // WCS_BUILD_DEMO
												FLT_MAX, -FLT_MAX, .01,	// center x
												FLT_MAX, -FLT_MAX, .01,	// center y
												#endif // WCS_BUILD_DEMO
												FLT_MAX, 0.0, 1.0,	// z min
												FLT_MAX, 0.0, 1.0,	// z max
												FLT_MAX, 0.0, 1.0,	// magic z
												100000.0, 5.0, 1.0,	// focal length
												1000.0, 5.0, 1.0,	// film size
												FLT_MAX, 0.005, 1.0,	// focal dist
												100.0, 1.0, 1.0,	// fstop
												2.0, 0.0, .01,	// motion blur percent
												FLT_MAX, 0.0, 1.0,		// zbuf box blur offset
												FLT_MAX, 0.0, 0.0,		// ease in time, set increment to 0 to use project frame rate
												FLT_MAX, 0.0, 0.0,		// ease out time, set increment to 0 to use project frame rate
												FLT_MAX, 1.0, 1.0,		// view width
												FLT_MAX, 0.0, 1.0,		// stereo separation
												FLT_MAX, 1.0, 1.0,		// stereo convergence distance
												};
long Ct;

EffectDefault[WCS_EFFECTS_CAMERA_ANIMPAR_ZMAX] = ConvertToMeters(10.0, WCS_USEFUL_UNIT_AU); // 10 AU
CameraType = WCS_EFFECTS_CAMERATYPE_TARGETED;
RenderBeyondHorizon = DepthOfField = Orthographic = PanoCam = StereoCam = BoxFilter = MotionBlur =
	ZBufBoxFilter = FieldRender = FieldRenderPriority = VelocitySmoothing = AAOptimizeReflection = 
	BackgroundImageEnabled = 0;
BoxFilterSize = 1; 
AAPasses = 1; 
MaxDOFBlurRadius = 5;
PanoPanels = 1;
Img = NULL;
TargetObj = NULL;
CenterX = CenterY = 0.0;
HorScale = VertScale = 1.0;
CamPos = TargPos = CamVertical = CamRight = NULL;
Smooth = NULL;
LensDistortion = ApplyLensDistortion = 0;
CenterOnOrigin = 0;
Floating = InterElevFollow = 0;
CamLonScale = 0.0;
PlanWidth = PlanTranslateWidth = 0.0;
StereoConvergence = 1;
StereoPreviewChannel = StereoRenderChannel = Projected = 0;
CamHeading = CamPitch = CamBank = 0.0;
ProjectedEasting = ProjectedNorthing = 0.0;
Coords = NULL;

memset(ScrRotMatx, 0, sizeof(ScrRotMatx));	// quick way to init IEEE numbers to 0.0
memset(InvScrRotMatx, 0, sizeof(InvScrRotMatx));

for (Ct = 0; Ct < WCS_EFFECTS_CAMERA_NUMANIMPAR; Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for

for (Ct = 0; Ct < WCS_EFFECTS_CAMERA_NUMRESTOREVALUES; Ct ++)
	{
	RestoreVal[Ct] = 0.0;
	} // for
RestoreValid = 0;

AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_ZMIN].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_ZMAX].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_MAGICZ].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FOCALDIST].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_ZBUFBOXFILTOFFSET].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_STEREOSEPARATION].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_STEREOCONVERGENCE].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_EASEIN].SetMetricType(WCS_ANIMDOUBLE_METRIC_TIME);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_EASEOUT].SetMetricType(WCS_ANIMDOUBLE_METRIC_TIME);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_MOBLURPERCENT].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_EASEIN].SetNoNodes(1);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_EASEOUT].SetNoNodes(1);

IAUpVec2D[0] = IAUpVec2D[1] = 0.0;
IARightVec2D[0] = IARightVec2D[1] = 0.0;

} // Camera::SetDefaults

/*===========================================================================*/

void Camera::Copy(Camera *CopyTo, Camera *CopyFrom)
{
long Ct, Result = -1;
Raster *NewRast;
NotifyTag Changes[2];

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
			Result = UserMessageCustom("Copy Camera", "How do you wish to resolve Coordinate System name collisions?\n\nLink to existing Coordinate Systems, replace existing Systems, or create new Systems?",
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

if (CopyTo->Img)
	{
	if (CopyTo->Img->GetRaster())
		{
		CopyTo->Img->GetRaster()->RemoveAttribute(CopyTo->Img);
		} // if
	if (CopyTo->Img)
		delete CopyTo->Img;
	} // if
CopyTo->Img = NULL;
CopyTo->BackgroundImageEnabled = 0;
if (CopyFrom->Img)
	{
	if (CopyTo->Img = new RasterShell())
		{
		if (CopyFrom->Img->GetRaster())
			{
			if (NewRast = GlobalApp->CopyToImageLib->MatchNameMakeRaster(CopyFrom->Img->GetRaster()))
				{
				NewRast->AddAttribute(CopyTo->Img->GetType(), CopyTo->Img, CopyTo);
				CopyTo->BackgroundImageEnabled = CopyFrom->BackgroundImageEnabled;
				} // if
			} // if
		} // if
	} // if

Result = -1;
CopyTo->TargetObj = NULL;
if (CopyFrom->TargetObj)
	{
	if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
		{
		CopyTo->TargetObj = (Object3DEffect *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->TargetObj);
		} // if no need to make another copy, its all in the family
	else
		{
		if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->TargetObj->EffectType, CopyFrom->TargetObj->Name))
			{
			Result = UserMessageCustom("Copy Camera", "How do you wish to resolve Target Object name collisions?\n\nLink to existing Objects, replace existing Objects, or create new Objects?",
				"Link", "Create", "Overwrite", 1);
			} // if
		if (Result <= 0)
			{
			CopyTo->TargetObj = (Object3DEffect *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->TargetObj->EffectType, NULL, CopyFrom->TargetObj);
			} // if create new
		else if (Result == 1)
			{
			CopyTo->TargetObj = (Object3DEffect *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->TargetObj);
			} // if link to existing
		else if (CopyTo->TargetObj = (Object3DEffect *)GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->TargetObj->EffectType, CopyFrom->TargetObj->Name))
			{
			CopyTo->TargetObj->Copy(CopyTo->TargetObj, CopyFrom->TargetObj);
			Changes[0] = MAKE_ID(CopyTo->TargetObj->GetNotifyClass(), CopyTo->TargetObj->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, CopyTo->TargetObj);
			} // else if found and overwrite
		else
			{
			CopyTo->TargetObj = (Object3DEffect *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->TargetObj->EffectType, NULL, CopyFrom->TargetObj);
			} // else
		} // else better copy or overwrite it since its important to get just the right object
	} // if

CopyTo->RestoreValid = CopyFrom->RestoreValid;
CopyTo->CameraType = CopyFrom->CameraType;
CopyTo->RenderBeyondHorizon = CopyFrom->RenderBeyondHorizon;
CopyTo->DepthOfField = CopyFrom->DepthOfField;
CopyTo->Orthographic = CopyFrom->Orthographic;
CopyTo->PanoCam = CopyFrom->PanoCam;
CopyTo->BoxFilter = CopyFrom->BoxFilter;
CopyTo->MotionBlur = CopyFrom->MotionBlur;
CopyTo->ZBufBoxFilter = CopyFrom->ZBufBoxFilter;
CopyTo->FieldRender = CopyFrom->FieldRender;
CopyTo->FieldRenderPriority = CopyFrom->FieldRenderPriority;
CopyTo->VelocitySmoothing = CopyFrom->VelocitySmoothing;
CopyTo->AAOptimizeReflection = CopyFrom->AAOptimizeReflection;
CopyTo->BoxFilterSize = CopyFrom->BoxFilterSize; 
CopyTo->AAPasses = CopyFrom->AAPasses; 
CopyTo->MaxDOFBlurRadius = CopyFrom->MaxDOFBlurRadius;
CopyTo->LensDistortion = CopyFrom->LensDistortion;
CopyTo->CenterOnOrigin = CopyFrom->CenterOnOrigin;
CopyTo->Floating = CopyFrom->Floating;
CopyTo->InterElevFollow = CopyFrom->InterElevFollow;
CopyTo->BackgroundImageEnabled = CopyFrom->BackgroundImageEnabled;
CopyTo->ApplyLensDistortion = CopyFrom->ApplyLensDistortion;
CopyTo->VRSegments = CopyFrom->VRSegments;
CopyTo->PanoPanels = CopyFrom->PanoPanels;
CopyTo->StereoCam = CopyFrom->StereoCam;
CopyTo->StereoConvergence = CopyFrom->StereoConvergence;
CopyTo->StereoPreviewChannel = CopyFrom->StereoPreviewChannel;
CopyTo->StereoRenderChannel = CopyFrom->StereoRenderChannel;
CopyTo->Projected = CopyFrom->Projected;
CopyTo->CenterX = CopyFrom->CenterX;
CopyTo->CenterY = CopyFrom->CenterY;
CopyTo->HorScale = CopyFrom->HorScale;
CopyTo->VertScale = CopyFrom->VertScale;
CopyTo->CamLonScale = CopyFrom->CamLonScale;
CopyTo->PlanWidth = CopyFrom->PlanWidth;
CopyTo->PlanTranslateWidth = CopyFrom->PlanTranslateWidth;
CopyTo->ProjectedEasting = CopyFrom->ProjectedEasting;
CopyTo->ProjectedNorthing = CopyFrom->ProjectedNorthing;
CopyTo->IAUpVec2D[0] = CopyFrom->IAUpVec2D[0];
CopyTo->IAUpVec2D[1] = CopyFrom->IAUpVec2D[1];
CopyTo->IARightVec2D[0] = CopyFrom->IARightVec2D[0];
CopyTo->IARightVec2D[1] = CopyFrom->IARightVec2D[1];
for (Ct = 0; Ct < WCS_EFFECTS_CAMERA_NUMRESTOREVALUES; Ct ++)
	{
	CopyTo->RestoreVal[Ct] = CopyFrom->RestoreVal[Ct];
	} // for
if (CopyFrom->CamPos && (CopyTo->CamPos = new VertexDEM))
	{
	CopyTo->CamPos->CopyLatLon(CopyFrom->CamPos);
	CopyTo->CamPos->CopyXYZ(CopyFrom->CamPos);
	CopyTo->CamPos->CopyScrnXYZQ(CopyFrom->CamPos);
	} // if
if (CopyFrom->TargPos && (CopyTo->TargPos = new VertexDEM))
	{
	CopyTo->TargPos->CopyLatLon(CopyFrom->TargPos);
	CopyTo->TargPos->CopyXYZ(CopyFrom->TargPos);
	CopyTo->TargPos->CopyScrnXYZQ(CopyFrom->TargPos);
	} // if
if (CopyFrom->CamVertical && (CopyTo->CamVertical = new VertexDEM))
	{
	CopyTo->CamVertical->CopyLatLon(CopyFrom->CamVertical);
	CopyTo->CamVertical->CopyXYZ(CopyFrom->CamVertical);
	CopyTo->CamVertical->CopyScrnXYZQ(CopyFrom->CamVertical);
	} // if
if (CopyFrom->CamRight && (CopyTo->CamRight = new VertexDEM))
	{
	CopyTo->CamRight->CopyLatLon(CopyFrom->CamRight);
	CopyTo->CamRight->CopyXYZ(CopyFrom->CamRight);
	CopyTo->CamRight->CopyScrnXYZQ(CopyFrom->CamRight);
	} // if
if (CopyFrom->Smooth && (CopyTo->Smooth = new SmoothPath))
	{
	if (! CopyTo->Smooth->Copy(CopyFrom->Smooth))
		{
		delete CopyTo->Smooth;
		CopyTo->Smooth = NULL;
		} // if
	} // if
CopyMatrix3x3(CopyTo->ScrRotMatx, CopyFrom->ScrRotMatx);
CopyMatrix3x3(CopyTo->InvScrRotMatx, CopyFrom->InvScrRotMatx);

GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // Camera::Copy

/*===========================================================================*/

char *Camera::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as a Camera Background! Remove anyway?");

} // Camera::OKRemoveRaster

/*===========================================================================*/

int Camera::RemoveRAHost(RasterAnimHost *RemoveMe)
{
NotifyTag Changes[2];
int Removed = 0;

if (Img && Img->GetRaster() == (Raster *)RemoveMe)
	{
	Img->GetRaster()->RemoveAttribute(Img);
	return (1);
	} // if

if (TargetObj == (Object3DEffect *)RemoveMe)
	{
	TargetObj = NULL;
	Removed = 1;
	} // if
else if (Coords == (CoordSys *)RemoveMe)
	{
	Coords = NULL;
	Removed = 1;
	} // if

if (Removed)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

	return (1);
	} // if

return (GeneralEffect::RemoveRAHost(RemoveMe));

} // Camera::RemoveRAHost

/*===========================================================================*/

void Camera::RemoveRaster(RasterShell *Shell)
{
NotifyTag Changes[2];

if (Img == Shell)
	Img = NULL;

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

} // Camera::RemoveRaster

/*===========================================================================*/

int Camera::SetRaster(Raster *NewRast)
{
NotifyTag Changes[2];

if (Img)
	{
	if (Img->GetRaster())
		{
		Img->GetRaster()->RemoveAttribute(Img);
		} // if
	} // if
if (! Img)
	{
	Img = new RasterShell;
	} // else
if (Img && NewRast)
	{
	NewRast->AddAttribute(Img->GetType(), Img, this);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	return (1);
	} // if
else
	{
	delete Img;		// delete it here since it wasn't added to a raster
	Img = NULL;
	return (0);
	} // else

} // Camera::SetRaster

/*===========================================================================*/

int Camera::SetTarget(Object3DEffect *NewTarget)
{
NotifyTag Changes[2];

TargetObj = NewTarget;
Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (1);

} // Camera::SetTarget

/*===========================================================================*/

int Camera::SetCoords(CoordSys *NewCoords)
{
NotifyTag Changes[2];

Coords = NewCoords;

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (1);

} // Camera::SetCoords

/*===========================================================================*/

ULONG Camera::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
unsigned long ImageID;
char TargetName[256];

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
					case WCS_EFFECTS_CAMERA_CAMLAT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_CAMLON:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_CAMELEV:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_TARGLAT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_TARGLON:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_TARGELEV:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_HEADING:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_PITCH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_V5BANK:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].Load(ffile, Size, ByteFlip);
						AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].ScaleValues(-1.0);
						break;
						}
					case WCS_EFFECTS_CAMERA_BANK:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_HFOV:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_CENTERX:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX].Load(ffile, Size, ByteFlip);
						#ifdef WCS_BUILD_DEMO
						if (AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX].CurValue != .5)
							AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX].SetValue(.5);
						#endif // WCS_BUILD_DEMO
						break;
						}
					case WCS_EFFECTS_CAMERA_CENTERY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY].Load(ffile, Size, ByteFlip);
						#ifdef WCS_BUILD_DEMO
						if (AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY].CurValue != .5)
							AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY].SetValue(.5);
						#endif // WCS_BUILD_DEMO
						break;
						}
					case WCS_EFFECTS_CAMERA_ZMIN:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_ZMIN].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_ZMAX:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_ZMAX].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_MAGICZ:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_MAGICZ].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_FOCALLENGTH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FOCALLENGTH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_FILMSIZE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FILMSIZE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_FOCALDIST:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FOCALDIST].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_FSTOP:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FSTOP].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_MOBLURPERCENT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_MOBLURPERCENT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_ZBUFBOXFILTOFFSET:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_ZBUFBOXFILTOFFSET].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_VIEWWIDTH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_STEREOSEPARATION:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_STEREOSEPARATION].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_STEREOCONVERGENCE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_STEREOCONVERGENCE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_CAMERATYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&CameraType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_RENDERBEYONDHORIZON:
						{
						BytesRead = ReadBlock(ffile, (char *)&RenderBeyondHorizon, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_BGIMAGEENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&BackgroundImageEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_DEPTHOFFIELD:
						{
						BytesRead = ReadBlock(ffile, (char *)&DepthOfField, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_ORTHOGRAPHIC:
						{
						BytesRead = ReadBlock(ffile, (char *)&Orthographic, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_PANOCAM:
						{
						BytesRead = ReadBlock(ffile, (char *)&PanoCam, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_LENSDISTORTION:
						{
						BytesRead = ReadBlock(ffile, (char *)&LensDistortion, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_CENTERONORIGIN:
						{
						BytesRead = ReadBlock(ffile, (char *)&CenterOnOrigin, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_BOXFILTER:
						{
						BytesRead = ReadBlock(ffile, (char *)&BoxFilter, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_MOTIONBLUR:
						{
						BytesRead = ReadBlock(ffile, (char *)&MotionBlur, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_ZBUFBOXFILTER:
						{
						BytesRead = ReadBlock(ffile, (char *)&ZBufBoxFilter, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_FIELDRENDER:
						{
						BytesRead = ReadBlock(ffile, (char *)&FieldRender, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_FIELDRENDERPRIORITY:
						{
						BytesRead = ReadBlock(ffile, (char *)&FieldRenderPriority, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_VELOCITYSMOOTHING:
						{
						BytesRead = ReadBlock(ffile, (char *)&VelocitySmoothing, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_AAOPTIMIZEREFLECTION:
						{
						BytesRead = ReadBlock(ffile, (char *)&AAOptimizeReflection, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_FLOATING:
						{
						BytesRead = ReadBlock(ffile, (char *)&Floating, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_INTERELEVFOLLOW:
						{
						BytesRead = ReadBlock(ffile, (char *)&InterElevFollow, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_STEREOCAM:
						{
						BytesRead = ReadBlock(ffile, (char *)&StereoCam, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_CONVERGENCE:
						{
						BytesRead = ReadBlock(ffile, (char *)&StereoConvergence, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_PREVIEWCHANNEL:
						{
						BytesRead = ReadBlock(ffile, (char *)&StereoPreviewChannel, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_RENDERCHANNEL:
						{
						BytesRead = ReadBlock(ffile, (char *)&StereoRenderChannel, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_PROJECTED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Projected, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_BOXFILTERSIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&BoxFilterSize, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_AAPASSES:
						{
						BytesRead = ReadBlock(ffile, (char *)&AAPasses, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_MAXDOFBLURRADIUS:
						{
						BytesRead = ReadBlock(ffile, (char *)&MaxDOFBlurRadius, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_VELOCITYSMOOTHINGEASEIN:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_EASEIN].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_VELOCITYSMOOTHINGEASEOUT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_EASEOUT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CAMERA_TARGETNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)TargetName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TargetName[0])
							{
							TargetObj = (Object3DEffect *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_OBJECT3D, TargetName);
							} // if
						break;
						}
					case WCS_EFFECTS_CAMERA_COORDSYSNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)TargetName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						#ifdef WCS_BUILD_VNS
						if (TargetName[0])
							{
							Coords = (CoordSys *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_COORDSYS, TargetName);
							} // if
						#endif // WCS_BUILD_VNS
						break;
						}
					case WCS_EFFECTS_IMAGEID:
						{
						BytesRead = ReadBlock(ffile, (char *)&ImageID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (ImageID > 0 && (Img = new RasterShell))
							{
							GlobalApp->LoadToImageLib->MatchRasterSetShell(ImageID, Img, this);
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

if (Floating)
	{
	// some early projects were built when we didn't implement separate variables
	// those projects need to be brought into line
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].SetFloating(1);
	} // if

return (TotalRead);

} // Camera::Load

/*===========================================================================*/

unsigned long Camera::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long ImageID;
unsigned long AnimItemTag[WCS_EFFECTS_CAMERA_NUMANIMPAR] = {WCS_EFFECTS_CAMERA_CAMLAT,
																 WCS_EFFECTS_CAMERA_CAMLON,
																 WCS_EFFECTS_CAMERA_CAMELEV,
																 WCS_EFFECTS_CAMERA_TARGLAT,
																 WCS_EFFECTS_CAMERA_TARGLON,
																 WCS_EFFECTS_CAMERA_TARGELEV,
																 WCS_EFFECTS_CAMERA_HEADING,
																 WCS_EFFECTS_CAMERA_PITCH,
																 WCS_EFFECTS_CAMERA_BANK,
																 WCS_EFFECTS_CAMERA_HFOV,
																 WCS_EFFECTS_CAMERA_CENTERX,
																 WCS_EFFECTS_CAMERA_CENTERY,
																 WCS_EFFECTS_CAMERA_ZMIN,
																 WCS_EFFECTS_CAMERA_ZMAX,
																 WCS_EFFECTS_CAMERA_MAGICZ,
																 WCS_EFFECTS_CAMERA_FOCALLENGTH,
																 WCS_EFFECTS_CAMERA_FILMSIZE,
																 WCS_EFFECTS_CAMERA_FOCALDIST,
																 WCS_EFFECTS_CAMERA_FSTOP,
																 WCS_EFFECTS_CAMERA_MOBLURPERCENT,
																 WCS_EFFECTS_CAMERA_ZBUFBOXFILTOFFSET,
																 WCS_EFFECTS_CAMERA_VELOCITYSMOOTHINGEASEIN,
																 WCS_EFFECTS_CAMERA_VELOCITYSMOOTHINGEASEOUT,
																 WCS_EFFECTS_CAMERA_VIEWWIDTH,
																 WCS_EFFECTS_CAMERA_STEREOSEPARATION,
																 WCS_EFFECTS_CAMERA_STEREOCONVERGENCE
																 };

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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_CAMERATYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&CameraType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_RENDERBEYONDHORIZON, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RenderBeyondHorizon)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_BGIMAGEENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&BackgroundImageEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_DEPTHOFFIELD, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DepthOfField)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_ORTHOGRAPHIC, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Orthographic)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_BOXFILTER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&BoxFilter)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_MOTIONBLUR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&MotionBlur)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_ZBUFBOXFILTER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ZBufBoxFilter)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_FIELDRENDER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FieldRender)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_FIELDRENDERPRIORITY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FieldRenderPriority)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_VELOCITYSMOOTHING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VelocitySmoothing)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_AAOPTIMIZEREFLECTION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AAOptimizeReflection)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_FLOATING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Floating)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_INTERELEVFOLLOW, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&InterElevFollow)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_STEREOCAM, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&StereoCam)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_CONVERGENCE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&StereoConvergence)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_PREVIEWCHANNEL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&StereoPreviewChannel)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_RENDERCHANNEL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&StereoRenderChannel)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_PROJECTED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Projected)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_PANOCAM, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PanoCam)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_LENSDISTORTION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LensDistortion)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_CENTERONORIGIN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&CenterOnOrigin)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_BOXFILTERSIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&BoxFilterSize)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_AAPASSES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&AAPasses)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_MAXDOFBLURRADIUS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&MaxDOFBlurRadius)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if (Img && (ImageID = Img->GetRasterID()) > 0)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_IMAGEID, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
		WCS_BLOCKTYPE_LONGINT, (char *)&ImageID)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

if (TargetObj)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_TARGETNAME, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(TargetObj->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)TargetObj->GetName())) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

#ifdef WCS_BUILD_VNS
if (Coords)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CAMERA_COORDSYSNAME, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Coords->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Coords->GetName())) == NULL)
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

} // Camera::Save

/*===========================================================================*/

void Camera::Edit(void)
{

DONGLE_INLINE_CHECK()
if (GlobalApp->GUIWins->CPG)
	{
	delete GlobalApp->GUIWins->CPG;
	}
GlobalApp->GUIWins->CPG = new CameraEditGUI(GlobalApp->AppEffects, GlobalApp->AppImages, this);
if (GlobalApp->GUIWins->CPG)
	{
	GlobalApp->GUIWins->CPG->Open(GlobalApp->MainProj);
	}

} // Camera::Edit

/*===========================================================================*/

char *CameraCritterNames[WCS_EFFECTS_CAMERA_NUMANIMPAR] = {"Camera Latitude (deg)", "Camera Longitude (deg)", 
	"Camera Elevation (m)", "Target Latitude (deg)", "Target Longitude (deg)", "Target Elevation (m)", "Heading (deg)", "Pitch (deg)", "Bank (deg)",
	"Field of View (deg)", "Offset X (%)", "Offset Y (%)", "Minimum Z (m)", "Maximum Z (m)", "Magic Z (m)", "Lens Focal Length (mm)", "Film Size (mm)",
	"Focal Distance (m)", "F Stop", "Motion Blur (%)", "Z Buf Box Filter Offset (m)", "Velocity Smoothing Ease In (sec)",
	"Velocity Smoothing Ease Out (sec)", "View Width (m)", "Stereo Separation (m)", "Stereo Convergence Distance (m)"};

/*===========================================================================*/

char *Camera::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (CameraCritterNames[Ct]);
	} // for
return ("");

} // Camera::GetCritterName

/*===========================================================================*/

long Camera::InitImageIDs(long &ImageID)
{
long NumImages = 0;

if (Img && Img->GetRaster())
	{
	Img->GetRaster()->SetID(ImageID++);
	NumImages ++;
	} // if
if (TargetObj)
	NumImages += TargetObj->InitImageIDs(ImageID);
if (Coords)
	NumImages += Coords->InitImageIDs(ImageID);
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // Camera::InitImageIDs

/*===========================================================================*/

int Camera::BuildFileComponentsList(EffectList **CoordSystems)
{
EffectList **ListPtr;

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

return (1);

} // Camera::BuildFileComponentsList

/*===========================================================================*/

char Camera::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_RASTER ||
	DropType == WCS_EFFECTSSUBCLASS_OBJECT3D
	#ifdef WCS_BUILD_VNS
	|| DropType == WCS_EFFECTSSUBCLASS_COORDSYS
	#endif // WCS_BUILD_VNS
	)
	return (1);

return (0);

} // Camera::GetRAHostDropOK

/*===========================================================================*/

int Camera::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
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
	if (this != (Camera *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (Camera *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_RASTER)
	{
	Success = SetRaster((Raster *)DropSource->DropSource);
	} // else if
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_OBJECT3D)
	{
	Success = SetTarget((Object3DEffect *)DropSource->DropSource);
	} // else if
#ifdef WCS_BUILD_VNS
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_COORDSYS)
	{
	Success = SetCoords((CoordSys *)DropSource->DropSource);
	} // else if
#endif // WCS_BUILD_VNS
else
	{
	Success = GeneralEffect::ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // Camera::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long Camera::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
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

Mask &= (WCS_RAHOST_ICONTYPE_RENDER | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // Camera::GetRAFlags

/*===========================================================================*/

void Camera::GetInterFlags(RasterAnimHostProperties *Prop, RasterAnimHost *FlagMe)
{
char Ct;

if (! FlagMe)
	{
	Prop->InterFlags = 0;
	return;
	} // if
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (FlagMe == GetAnimPtr(Ct))
		{
		if (Ct == WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT ||
			Ct == WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON ||
			Ct == WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV)
			{
			if (CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED && ! TargetObj)
				{
				Prop->InterFlags = (WCS_RAHOST_INTERBIT_CLICKTOPOS | 
									WCS_RAHOST_INTERBIT_MOVEX | WCS_RAHOST_INTERBIT_MOVEY | 
									WCS_RAHOST_INTERBIT_MOVEZ | WCS_RAHOST_INTERBIT_MOVEELEV);
				} // if
			else
				{
				Prop->InterFlags = 0;
				} // else
			return;
			} // if
		if (Ct == WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX ||
			Ct == WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY)
			{
			Prop->InterFlags = (WCS_RAHOST_INTERBIT_CLICKTOPOS | WCS_RAHOST_INTERBIT_MOVEX | WCS_RAHOST_INTERBIT_MOVEY);
			return;
			} // if
		} // if found
	} // for

Prop->InterFlags = (WCS_RAHOST_INTERBIT_CLICKTOPOS | 
					WCS_RAHOST_INTERBIT_MOVEX | WCS_RAHOST_INTERBIT_MOVEY | 
					WCS_RAHOST_INTERBIT_MOVEZ | WCS_RAHOST_INTERBIT_MOVEELEV |
					WCS_RAHOST_INTERBIT_SCALEY);

if (CameraType != WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
	Prop->InterFlags |= (WCS_RAHOST_INTERBIT_ROTATEZ);
if (CameraType != WCS_EFFECTS_CAMERATYPE_OVERHEAD)
	Prop->InterFlags |= (WCS_RAHOST_INTERBIT_ROTATEX | WCS_RAHOST_INTERBIT_ROTATEY);

} // Camera::GetInterFlags

/*===========================================================================*/

int Camera::ScaleMoveRotate(RasterAnimHost *MoveMe, DiagnosticData *Data, unsigned char Operation)
{
//char ElevText[256];
double NewVal, TerrainOffset = 0.0, TerrainElev;
PlanetOpt *DefPlanetOpt = NULL;

if (! MoveMe)
	{
	return (0);
	} // if
// Camera target group
if (MoveMe == GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT) ||
	MoveMe == GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON) ||
	MoveMe == GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV))
	{
	if (CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED && ! TargetObj)
		{
		if (Operation == WCS_RAHOST_INTERACTIVEOP_SETPOS || Operation == WCS_RAHOST_INTERACTIVEOP_SETPOSNOQUERY)
			{
			if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE])
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LATITUDE]);
			if (Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LONGITUDE]);
			if (Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION])
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].SetCurValue(Data->Value[WCS_DIAGNOSTIC_ELEVATION]);
			return (1);
			} // if 
		else if (Operation == WCS_RAHOST_INTERACTIVEOP_MOVEXYZ ||
			Operation == WCS_RAHOST_INTERACTIVEOP_MOVELATLONELEV)
			{
			Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] = 1;
			Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE] = 1;
			Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION] = 1;
			Data->Value[WCS_DIAGNOSTIC_LATITUDE] = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].CurValue;
			Data->Value[WCS_DIAGNOSTIC_LONGITUDE] = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].CurValue;
			Data->Value[WCS_DIAGNOSTIC_ELEVATION] = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].CurValue;
			GlobalApp->GUIWins->CVG->ScaleMotion(Data);
			if (InterElevFollow)
				{
				if (DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL))
					{
					TerrainElev = GlobalApp->MainProj->Interactive->ElevationPoint(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].CurValue,
						AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].CurValue);
					TerrainElev = CalcExag(TerrainElev, DefPlanetOpt);
					TerrainOffset = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].CurValue - TerrainElev;
					} // if
				} // if
			if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE])
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LATITUDE]);
			if (Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LONGITUDE]);
			if (Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION] || (InterElevFollow  && DefPlanetOpt))
				{
				if (InterElevFollow && DefPlanetOpt)
					{
					TerrainElev = GlobalApp->MainProj->Interactive->ElevationPoint(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].CurValue,
						AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].CurValue);
					TerrainElev = CalcExag(TerrainElev, DefPlanetOpt);
					AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].SetCurValue(TerrainOffset + TerrainElev);
					} // if
				else
					AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].SetCurValue(Data->Value[WCS_DIAGNOSTIC_ELEVATION]);
				} // if
			return (1);
			} // if 
		} // if
	return (0);
	} // if
// Center XY group
if (MoveMe == GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX) ||
	MoveMe == GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY))
	{
	if (Operation == WCS_RAHOST_INTERACTIVEOP_SETPOS || Operation == WCS_RAHOST_INTERACTIVEOP_SETPOSNOQUERY)
		{
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX].SetCurValue((double)Data->PixelX / Data->DimX);
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY].SetCurValue((double)Data->PixelY / Data->DimY);
		return (1);
		} // if 
	if (Operation == WCS_RAHOST_INTERACTIVEOP_MOVEXYZ)
		{
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX].SetCurValue(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX].CurValue + (double)Data->MoveX / Data->DimX);
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY].SetCurValue(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY].CurValue + (double)Data->MoveY / Data->DimY);
		return (1);
		} // if 
	return (0);
	} // if

// Camera group
if (Operation == WCS_RAHOST_INTERACTIVEOP_SETPOS)
	{
	if (Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION])
		{
		AnimDoubleTime TempADT;
		TempADT.SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
		TempADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
		TempADT.SetValue(Data->Value[WCS_DIAGNOSTIC_ELEVATION]);
		if (GetInputValue("Enter new elevation for Camera. Clear the field to leave at current elevation.", &TempADT))
		//sprintf(ElevText, "%f", Data->Value[WCS_DIAGNOSTIC_ELEVATION]);
		//if (GetInputString("Enter new elevation for Camera. Clear the field to leave at current elevation.", ":;*/?`#%", ElevText))
			{
			if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE])
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LATITUDE]);
			if (Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LONGITUDE]);
			if (TempADT.CurValue != 0.0)
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetCurValue(TempADT.CurValue);
				//AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetCurValue(atof(ElevText));
			} // if
		} // if
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_SETPOSNOQUERY)
	{
	if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE])
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LATITUDE]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LONGITUDE]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION])
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetCurValue(Data->Value[WCS_DIAGNOSTIC_ELEVATION]);
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_MOVEXYZ ||
	Operation == WCS_RAHOST_INTERACTIVEOP_MOVELATLONELEV)
	{
	Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] = 1;
	Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE] = 1;
	Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION] = 1;
	Data->Value[WCS_DIAGNOSTIC_LATITUDE] = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue;
	Data->Value[WCS_DIAGNOSTIC_LONGITUDE] = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue;
	Data->Value[WCS_DIAGNOSTIC_ELEVATION] = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue;
	GlobalApp->GUIWins->CVG->ScaleMotion(Data);
	if (InterElevFollow)
		{
		if (DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL))
			{
			TerrainElev = GlobalApp->MainProj->Interactive->ElevationPoint(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue,
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue);
			TerrainElev = CalcExag(TerrainElev, DefPlanetOpt);
			TerrainOffset = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue - TerrainElev;
			} // if
		} // if
	if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE])
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LATITUDE]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LONGITUDE]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION] || InterElevFollow)
		{
		if (InterElevFollow && DefPlanetOpt)
			{
			TerrainElev = GlobalApp->MainProj->Interactive->ElevationPoint(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue,
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue);
			TerrainElev = CalcExag(TerrainElev, DefPlanetOpt);
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetCurValue(TerrainOffset + TerrainElev);
			} // if
		else
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetCurValue(Data->Value[WCS_DIAGNOSTIC_ELEVATION]);
		} // if
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_SCALE)
	{
	// allow hfov or view width
	NewVal = Data->MoveY / 100.0;
	if (NewVal < 0.0)
		NewVal = 1.0 / (1.0 - NewVal);
	else
		NewVal += 1.0;
	if (NewVal < .01)
		NewVal = .01;
	// let animcritter do clamping
	if (CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC || Orthographic)
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].SetCurValue(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].CurValue * NewVal);
	else
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetCurValue(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue * NewVal);
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_SETSIZE)
	{
	if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALX])
		{
		if (CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC || Orthographic)
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALX]);
		else
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALX]);
		} // if
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_ROTATE)
	{
	if (CameraType != WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
		{
		// allow banking
		NewVal = Data->MoveZ * .5;
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].SetCurValue(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].CurValue + NewVal);
		if (CameraType != WCS_EFFECTS_CAMERATYPE_OVERHEAD)
			{
			// allow heading and pitch
			NewVal = Data->MoveX * .5;
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].SetCurValue(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].CurValue + NewVal);
			NewVal = Data->MoveY * .5;
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].SetCurValue(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].CurValue + NewVal);
			} // if
		return (1);
		} // if
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_SETROTATION)
	{
	if (CameraType != WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
		{
		// allow banking
		if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALZ])
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALZ]);
		if (CameraType != WCS_EFFECTS_CAMERATYPE_OVERHEAD)
			{
			// allow heading and pitch
			if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALX])
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALX]);
			if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALY])
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALY]);
			} // if
		return (1);
		} // if
	} // if 

return (0);	// return 0 if nothing changed

} // Camera::ScaleMoveRotate

/*===========================================================================*/

RasterAnimHost *Camera::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;

if (! Current)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Found)
		{
		if (Ct == WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT || Ct == WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON || Ct == WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV)
			{
			if (CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED)
				return (GetAnimPtr(Ct));
			} // if
		else if (Ct == WCS_EFFECTS_CAMERA_ANIMPAR_HEADING || Ct == WCS_EFFECTS_CAMERA_ANIMPAR_PITCH)
			{
			if (! (CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD || CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC))
				return (GetAnimPtr(Ct));
			} // else if
		else if (Ct == WCS_EFFECTS_CAMERA_ANIMPAR_BANK)
			{
			if (CameraType != WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
				return (GetAnimPtr(Ct));
			} // else if
		else
			{
			return (GetAnimPtr(Ct));
			} // else if
		} // if
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for
if (Found && TargetObj)
	return (TargetObj);
if (Current == TargetObj)
	Found = 1;
#ifdef WCS_BUILD_VNS
if (Found && Coords)
	return (Coords);
if (Current == Coords)
	Found = 1;
#endif // WCS_BUILD_VNS
if (Found && Img && Img->GetRaster())
	return (Img->GetRaster());

return (NULL);

} // Camera::GetRAHostChild

/*===========================================================================*/

RasterAnimHost *Camera::GetNextGroupSibling(RasterAnimHost *FindMyBrother)
{

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT))
	return (GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON))
	return (GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV))
	return (GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT))
	return (GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON))
	return (GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV))
	return (GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX))
	return (GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY))
	return (GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HEADING))
	return (GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_PITCH));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_PITCH))
	return (GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_BANK));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_BANK))
	return (GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HEADING));

return (NULL);

} // Camera::GetNextGroupSibling

/*===========================================================================*/

int Camera::GetDeletable(RasterAnimHost *Test)
{

if (Img && Test == Img->GetRaster())
	return (1);
if (Test == TargetObj)
	return (1);
if (Test == Coords)
	return (1);

return (0);

} // Camera::GetDeletable

/*===========================================================================*/

void Camera::CreateBankKeys(void)
{
double BankFactor, FrameRate, FrameTime, FirstKey, LastKey, PlanetRad, Heading, Deviation, Banking, TimeOffset;
VertexDEM BehindPos, CurPos, AheadPos;
RasterAnimHostProperties Prop;
AnimDoubleTime NewBank;
CoordSys *MyCoords;
long KeyFrameInterval, MaxFrame, NextKeyFrame, LastKeyFrame;
char MessageStr[64];

MyCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
PlanetRad = GlobalApp->AppEffects->GetPlanetRadius();
if ((FrameRate = GlobalApp->MainProj->Interactive->GetFrameRate()) <= 0.0)
	FrameRate = 30.0;
TimeOffset = 1.0 / 30.0;		// just an arbitrary amount of time

// we're gonna add nodes to this dummy AnimDoubleTime, then copy them in a whack to the bank channel
NewBank.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

// user can specify a frame interval, which if 0 means use positional key frame positions
strcpy(MessageStr, "At Position Key Frames");
if (GetInputString("Enter a key frame interval. Accept as is to create Bank keys at current positional key frames.", ":;*/?`#%-", MessageStr))
	{
	KeyFrameInterval = atoi(MessageStr);
	// user can specify a banking factor. framerate and animation speed will also have an effect.
	sprintf(MessageStr, "%.1f", 10.0);
	if (GetInputString("Enter a banking factor. Larger numbers create steeper banking.", ":;*/?`#%", MessageStr))
		{
		BankFactor = atof(MessageStr);
		GetKeyFrameRange(FirstKey, LastKey);
		if (KeyFrameInterval <= 0)
			{
			// use positional key frames
			Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
			Prop.ItemOperator = WCS_KEYOPERATION_CUROBJGROUP;
			LastKeyFrame = -2;
			// find each unique positional key frame
			while (1)	//lint !e716
				{
				NextKeyFrame = -1;
				Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / FrameRate;
				Prop.NewKeyNodeRange[0] = -DBL_MAX;
				Prop.NewKeyNodeRange[1] = DBL_MAX;
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetRAHostProperties(&Prop);
				if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
					NextKeyFrame = quickftol(Prop.NewKeyNodeRange[1] * FrameRate + .5);
				if (NextKeyFrame > LastKeyFrame)
					{
					FrameTime = NextKeyFrame / FrameRate;
					if (Prop.KeyNodeRange[1] < FirstKey + TimeOffset)
						FrameTime += TimeOffset;
					else if (Prop.KeyNodeRange[1] > LastKey - TimeOffset)
						FrameTime -= TimeOffset;
					CurPos.Lat = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetValue(0, FrameTime);
					CurPos.Lon = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetValue(0, FrameTime);
					CurPos.Elev = 0.0;
					BehindPos.Lat = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetValue(0, FrameTime - TimeOffset);
					BehindPos.Lon = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetValue(0, FrameTime - TimeOffset);
					BehindPos.Elev = 0.0;
					AheadPos.Lat = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetValue(0, FrameTime + TimeOffset);
					AheadPos.Lon = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetValue(0, FrameTime + TimeOffset);
					AheadPos.Elev = 0.0;
					// convert to cartesian
					#ifdef WCS_BUILD_VNS
					MyCoords->DegToCart(&CurPos);
					MyCoords->DegToCart(&BehindPos);
					MyCoords->DegToCart(&AheadPos);
					#else // WCS_BUILD_VNS
					CurPos.DegToCart(PlanetRad);
					BehindPos.DegToCart(PlanetRad);
					AheadPos.DegToCart(PlanetRad);
					#endif // WCS_BUILD_VNS
					// copy latitude and longitude of current position
					BehindPos.CopyLatLon(&CurPos);
					AheadPos.CopyLatLon(&CurPos);
					// rotate to home position
					CurPos.RotateToHome();
					BehindPos.RotateToHome();
					AheadPos.RotateToHome();
					// get positional vectors
					AheadPos.GetPosVector(&CurPos);
					CurPos.GetPosVector(&BehindPos);

					Heading = CurPos.FindAngleYfromZ();
					AheadPos.RotateY(-Heading);
					Deviation = AheadPos.FindAngleYfromZ();
					Banking = -Deviation * BankFactor;
					NewBank.AddNode(FrameTime, Banking, 0.0);

					LastKeyFrame = NextKeyFrame;
					} // if
				else
					break;
				} // while
			} // if
		else
			{
			// use a rigid frame interval
			MaxFrame = quickftol(LastKey * FrameRate + .5);
			for (NextKeyFrame = 1; NextKeyFrame < MaxFrame; NextKeyFrame += KeyFrameInterval)
				{
				FrameTime = NextKeyFrame / FrameRate;
				CurPos.Lat = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetValue(0, FrameTime);
				CurPos.Lon = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetValue(0, FrameTime);
				CurPos.Elev = 0.0;
				BehindPos.Lat = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetValue(0, FrameTime - TimeOffset);
				BehindPos.Lon = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetValue(0, FrameTime - TimeOffset);
				BehindPos.Elev = 0.0;
				AheadPos.Lat = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetValue(0, FrameTime + TimeOffset);
				AheadPos.Lon = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetValue(0, FrameTime + TimeOffset);
				AheadPos.Elev = 0.0;
				// convert to cartesian
				#ifdef WCS_BUILD_VNS
				MyCoords->DegToCart(&CurPos);
				MyCoords->DegToCart(&BehindPos);
				MyCoords->DegToCart(&AheadPos);
				#else // WCS_BUILD_VNS
				CurPos.DegToCart(PlanetRad);
				BehindPos.DegToCart(PlanetRad);
				AheadPos.DegToCart(PlanetRad);
				#endif // WCS_BUILD_VNS
				// copy latitude and longitude of current position
				BehindPos.CopyLatLon(&CurPos);
				AheadPos.CopyLatLon(&CurPos);
				// rotate to home position
				CurPos.RotateToHome();
				BehindPos.RotateToHome();
				AheadPos.RotateToHome();
				// get positional vectors
				AheadPos.GetPosVector(&CurPos);
				CurPos.GetPosVector(&BehindPos);

				Heading = CurPos.FindAngleYfromZ();
				AheadPos.RotateY(-Heading);
				Deviation = AheadPos.FindAngleYfromZ();
				Banking = -Deviation * BankFactor;
				NewBank.AddNode(FrameTime, Banking, 0.0);
				} // for
			} // else
		// copy the animsetings to the temporary anim double
		NewBank.CopyRangeDefaults(&AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK]);
		// copy the nodes back to the bank channel
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].Copy(&AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK], &NewBank);
		// send notification
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].NodeAdded();
		} // if banking factor
	} // if frame interval

} // Camera::CreateBankKeys

/*===========================================================================*/

// this is used by export camera path
// return -1 when no more unique key frame positions found
long Camera::GetNextMotionKeyFrame(long LastFrame, double FrameRate)
{
long TotalKeys, NextFrame, NextCamPosFrame = -1, NextTargPosFrame = -1, NextHPBFrame = -1;
RasterAnimHostProperties Prop;

TotalKeys = 0;
TotalKeys += AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetNumNodes(0);
TotalKeys += AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetNumNodes(0);
TotalKeys += AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetNumNodes(0);
TotalKeys += AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].GetNumNodes(0);
TotalKeys += AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].GetNumNodes(0);
TotalKeys += AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].GetNumNodes(0);
TotalKeys += AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetNumNodes(0);
TotalKeys += AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].GetNumNodes(0);
TotalKeys += AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].GetNumNodes(0);

if (TotalKeys > 0)
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
	Prop.ItemOperator = WCS_KEYOPERATION_CUROBJGROUP;

	Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastFrame +.5) / FrameRate;
	Prop.NewKeyNodeRange[0] = -DBL_MAX;
	Prop.NewKeyNodeRange[1] = DBL_MAX;
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetRAHostProperties(&Prop);
	if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
		NextCamPosFrame = quickftol(Prop.NewKeyNodeRange[1] * FrameRate + .5);

	Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastFrame +.5) / FrameRate;
	Prop.NewKeyNodeRange[0] = -DBL_MAX;
	Prop.NewKeyNodeRange[1] = DBL_MAX;
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetRAHostProperties(&Prop);
	if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
		NextTargPosFrame = quickftol(Prop.NewKeyNodeRange[1] * FrameRate + .5);

	Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastFrame +.5) / FrameRate;
	Prop.NewKeyNodeRange[0] = -DBL_MAX;
	Prop.NewKeyNodeRange[1] = DBL_MAX;
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].GetRAHostProperties(&Prop);
	if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
		NextHPBFrame = quickftol(Prop.NewKeyNodeRange[1] * FrameRate + .5);

	if (NextCamPosFrame > LastFrame || NextTargPosFrame > LastFrame || NextHPBFrame > LastFrame)
		{
		NextFrame = LONG_MAX;
		if (NextCamPosFrame > LastFrame && NextCamPosFrame >= 0)
			NextFrame = NextCamPosFrame;
		if (NextTargPosFrame > LastFrame && NextTargPosFrame >= 0)
			NextFrame = min(NextTargPosFrame, NextFrame);
		if (NextHPBFrame > LastFrame && NextHPBFrame >= 0)
			NextFrame = min(NextHPBFrame, NextFrame);

		return (NextFrame);
		} // while
	return (-1);
	} // if
else
	{
	// no key frames
	return (LastFrame < 0 ? 0: -1);
	} // else

} // Camera::GetNextMotionKeyFrame

/*===========================================================================*/

GraphNode *Camera::GetNearestMotionNode(double KeyTime)
{
double DistDiff1, DistDiff = FLT_MAX;
GraphNode *Node, *Found = NULL;

if (Node = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].FindNearestSiblingNode(KeyTime, NULL))
	{
	DistDiff = fabs(KeyTime - Node->GetDistance());
	Found = Node;
	} // if
if (Node = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].FindNearestSiblingNode(KeyTime, NULL))
	{
	DistDiff1 = fabs(KeyTime - Node->GetDistance());
	if (DistDiff1 < DistDiff)
		{
		DistDiff = DistDiff1;
		Found = Node;
		} // if
	} // if
if (Node = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].FindNearestSiblingNode(KeyTime, NULL))
	{
	DistDiff1 = fabs(KeyTime - Node->GetDistance());
	if (DistDiff1 < DistDiff)
		{
		DistDiff = DistDiff1;
		Found = Node;
		} // if
	} // if

return (Found);

} // Camera::GetNearestMotionNode

/*===========================================================================*/

void Camera::CaptureRestoreValues(double Restore[WCS_EFFECTS_CAMERA_NUMRESTOREVALUES])
{

Restore[0] = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue;
Restore[1] = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].CurValue;
Restore[2] = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].CurValue;
Restore[3] = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].CurValue;
Restore[4] = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].CurValue;
Restore[5] = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue;
Restore[6] = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue;
Restore[7] = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue;
Restore[8] = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].CurValue;
Restore[9] = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].CurValue;
Restore[10] = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].CurValue;
Restore[11] = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX].CurValue;
Restore[12] = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY].CurValue;

} // Camera::CaptureRestoreValues

/*===========================================================================*/

void Camera::RestoreRestoreValues(double Restore[WCS_EFFECTS_CAMERA_NUMRESTOREVALUES])
{

AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetValue(Restore[0]);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].SetValue(Restore[1]);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].SetValue(Restore[2]);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].SetValue(Restore[3]);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].SetValue(Restore[4]);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetValue(Restore[5]);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetValue(Restore[6]);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetValue(Restore[7]);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].SetValue(Restore[8]);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].SetValue(Restore[9]);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].SetValue(Restore[10]);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX].SetValue(Restore[11]);
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY].SetValue(Restore[12]);

} // Camera::RestoreRestoreValues

/*===========================================================================*/

void Camera::SetFloating(char NewFloating)
{
double TargElev, NSDist, WEDist;
Database *CurDB;
NotifyTag Changes[2];
RasterAnimHostProperties Prop;
DEMBounds CurBounds;
PlanetOpt *DefPlanetOpt;

if (NewFloating)
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS;
	Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATED;
	GetRAHostProperties(&Prop);
	if (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
		{
		if (! UserMessageOKCAN("Set Camera Floating", "Keyframes exist for this Camera. They will be removed\n if the Camera is set to float. Remove key frames?"))
			{
			Floating = 0;
			return;
			} // if
		} // if
	Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
	Prop.FrameOperator = WCS_KEYOPERATION_ALLKEYS;
	Prop.KeyframeOperation = WCS_KEYOPERATION_DELETE;
	Prop.TypeNumber = WCS_EFFECTSSUBCLASS_CAMERA;
	Prop.PropMask = WCS_RAHOST_MASKBIT_KEYRANGE;
	SetRAHostProperties(&Prop);
	if ((CurDB = GlobalApp->AppDB) && (DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, NULL)))
		{
		// stash restore coords if the current coords are suspected of being valid
		if (AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue != 0.0 || AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue != 0.0 || AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue != 0.0)
			{
			CaptureRestoreValues(RestoreVal);
			RestoreValid = 1;
			} // if
		// get some relative sizes
		if (CurDB->FillDEMBounds(&CurBounds))
			{
			CurBounds.LowElev = CalcExag(CurBounds.LowElev, DefPlanetOpt);
			CurBounds.HighElev = CalcExag(CurBounds.HighElev, DefPlanetOpt);
			TargElev = (CurBounds.LowElev + CurBounds.HighElev) * 0.5;
			} // if
		else
			{
			CurDB->GetBounds(CurBounds.North, CurBounds.South, CurBounds.East, CurBounds.West);
			TargElev = 0.0;
			CurBounds.HighElev = CurBounds.LowElev = 2000.0;
			//CurBounds.North = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y) + .5;
			//CurBounds.South = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y) - .5;
			//CurBounds.West = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X) + .5;
			//CurBounds.East = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X) - .5;
			//TargElev = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Z);
			} // if
		NSDist = fabs(CurBounds.North - CurBounds.South) * LatScale(GlobalApp->AppEffects->GetPlanetRadius());
		WEDist = fabs(CurBounds.West - CurBounds.East) * LonScale(GlobalApp->AppEffects->GetPlanetRadius(), (CurBounds.North + CurBounds.South) * 0.5);
		// set values
		if (CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD || CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
			{
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetValue((CurBounds.North + CurBounds.South) * 0.5);
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetValue((CurBounds.East + CurBounds.West) * 0.5);
			if (CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetValue(CurBounds.HighElev + 5000.0);
			else
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetValue(CurBounds.HighElev + NSDist * 2.0);
			} // if
		else
			{
			if (fabs(CurBounds.North - CurBounds.South) > 10.0) // huge range, locate Camera 1d south of Target
				{
				double CamLatTemp = (CurBounds.North + CurBounds.South) * 0.5;
				CamLatTemp -= 1.0;
				if (CamLatTemp < 90.0) CamLatTemp = 90.0;
				} // if
			else if ((CurBounds.North + CurBounds.South) * 0.5 > 0.0)
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetValue(min(CurBounds.North, CurBounds.South));
			else
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetValue(max(CurBounds.North, CurBounds.South));
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetValue((CurBounds.East + CurBounds.West) * 0.5);
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetValue(CurBounds.HighElev + NSDist * .25);
			} // if
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].SetValue(0.0);
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].SetValue(0.0);
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].SetValue(0.0);
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetValue(45.0);
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].SetValue(max(NSDist, WEDist));
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX].SetValue(.5);
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY].SetValue(.5);
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].SetValue((CurBounds.North + CurBounds.South) * 0.5);
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].SetValue((CurBounds.East + CurBounds.West) * 0.5);
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].SetValueNotify(TargElev);
		if (CameraType == WCS_EFFECTS_CAMERATYPE_UNTARGETED)
			{
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].SetValue(30.0);
			} // if
		} // if

	Floating = 1;
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY].SetFloating(1);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].SetFloating(1);

	// if this is the first time the camera is set floating we grab 
	// the new coords and stuff them in the restore values
	if (! RestoreValid)
		{
		CaptureRestoreValues(RestoreVal);
		RestoreValid = 1;
		} // if
	} // if enable floating
else
	{
	Floating = 0;
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetFloating(0);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetFloating(0);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetFloating(0);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].SetFloating(0);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].SetFloating(0);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].SetFloating(0);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].SetFloating(0);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].SetFloating(0);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].SetFloating(0);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetFloating(0);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX].SetFloating(0);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY].SetFloating(0);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].SetFloating(0);
	} // else if unfloat

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

} // Camera::SetFloating

/*===========================================================================*/

void Camera::Zoom(float ZoomDirectionAndAmount)
{
double GroundElev, ReducedElev, ViewWidth, TempVal, Circumference, NSDist, NewValue;
float Amount;
PlanetOpt *DefPlanetOpt;
DEMBounds CurBounds;
NotifyTag Changes[2];

CaptureRestoreValues(RestoreVal);

Amount = fabs(ZoomDirectionAndAmount);
if (Amount > 1.0)
	Amount = 1.0; // gotta clamp this for sanity right now
if (Amount == 0.0) return; // error checking

if (ZoomDirectionAndAmount > 0) // zoom in
	{
	if ((TempVal = (1.0 + Amount * .5 - Amount)) != 0.0)
		{
		if (CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC || Orthographic)
			{
			// halve view width
			NewValue = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].CurValue * TempVal;
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].SetValue(NewValue);
			} // if
		else if (CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD)
			{
			// halve distance to ground
			GroundElev = GlobalApp->MainProj->Interactive->ElevationPoint(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue,
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue);
			if (DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, NULL))
				GroundElev = CalcExag(GroundElev, DefPlanetOpt);
			ReducedElev = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue - GroundElev;
			NewValue = GroundElev + ReducedElev * TempVal;
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetValue(NewValue);
			} // else if overhead
		else
			{
			// halve fov
			ViewWidth = tan(PiOver180 * AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue * 0.5);
			NewValue = ViewWidth * TempVal;
			NewValue = 2.0 * atan(NewValue) / PiOver180;
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetValue(NewValue);
			} // else
		} // if
	} // if
else if (ZoomDirectionAndAmount == -FLT_MAX) // zoom way out
	{ // "Amount" variable is disregarded
	if (CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
		{
		// zoom out to coverage of whole earth, preferably centered in longitude on terrain
		if (DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, NULL))
			{
			Circumference = LatScale(GlobalApp->AppEffects->GetPlanetRadius()) * 360.0;
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].SetValue(Circumference);
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetValue(0.0);
			if (GlobalApp->AppDB->FillDEMBounds(&CurBounds))
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetValue((CurBounds.East + CurBounds.West) * 0.5);
			else
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetValue(0.0);
			} // if
		} // if plan
	else if (CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD)
		{
		// raise camera elev to great heights based on size of terrain
		if (DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, NULL))
			{
			if (GlobalApp->AppDB->FillDEMBounds(&CurBounds))
				{
				NSDist = fabs(CurBounds.North - CurBounds.South) * LatScale(GlobalApp->AppEffects->GetPlanetRadius());
				GroundElev = GlobalApp->MainProj->Interactive->ElevationPoint(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue,
					AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue);
				GroundElev = CalcExag(GroundElev, DefPlanetOpt);
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetValue(GroundElev + NSDist * 10.0);
				} // if
			} // if
		} // else if overhead
	else if (! Orthographic)
		{
		// set fov wide, but maximum (180) seems excessive and confusing to many users
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetValue(90.0);	// value will be clipped to allowable range
		} // else if !ortho
	// don't know what to do for orthographic
	} // else if zoom way out
else if (ZoomDirectionAndAmount < 0) // zoom out
	{
	if ((TempVal = (1.0 + Amount * .5 - Amount)) != 0.0)
		{
		if (CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC || Orthographic)
			{
			// double view width
			NewValue = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].CurValue / TempVal;
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].SetValue(NewValue);
			} // if plan or ortho
		else if (CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD)
			{
			// double distance to ground
			GroundElev = GlobalApp->MainProj->Interactive->ElevationPoint(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue,
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue);
			if (DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, NULL))
				GroundElev = CalcExag(GroundElev, DefPlanetOpt);
			ReducedElev = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue - GroundElev;
			NewValue = GroundElev + ReducedElev / TempVal;
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetValue(NewValue);
			} // else if overhead
		else
			{
			// double fov
			ViewWidth = tan(PiOver180 * AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue * 0.5);
			NewValue = ViewWidth / TempVal;
			NewValue = 2.0 * atan(NewValue) / PiOver180;
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetValue(NewValue);
			} // else
		} // if
	} // else zoom out

RestoreValid = 1;
SetFloating(0);
Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

} // Camera::Zoom

/*===========================================================================*/

void  Camera::ZoomBoxProxy(Camera *TheCameraToZoom, long OldWidth, long OldHeight, long NewWidth, long NewHeight, long NewCenterX, long NewCenterY)
{
NotifyTag Changes[2];
double TempStash[WCS_EFFECTS_CAMERA_NUMRESTOREVALUES];

if (TheCameraToZoom)
	{
	// zoom the proxy camera and capture an array of values
	ZoomBox(OldWidth, OldHeight, NewWidth, NewHeight, NewCenterX, NewCenterY);
	CaptureRestoreValues(TempStash);
	// capture the camera to modify's values for possible later restoration
	TheCameraToZoom->CaptureRestoreValues(TheCameraToZoom->RestoreVal);
	// replace modify camera's values with the ones generated by the zoom
	TheCameraToZoom->RestoreRestoreValues(TempStash);
	// restore the proxy's values so they can be used again
	RestoreRestoreValues(RestoreVal);
	TheCameraToZoom->RestoreValid = 1;
	TheCameraToZoom->SetFloating(0);
	Changes[0] = MAKE_ID(TheCameraToZoom->GetNotifyClass(), TheCameraToZoom->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, TheCameraToZoom->GetRAHostRoot());
	} // if

} // Camera::ZoomBoxProxy

/*===========================================================================*/

// this f() assumes that the camera is already initialized for the view. It uses vertex unprojection.
void  Camera::ZoomBox(long OldWidth, long OldHeight, long NewWidth, long NewHeight, long NewCenterX, long NewCenterY)
{
double WidthRatio, HeightRatio, Distance, XAngle, YAngle, GroundElev, EarthCenter[3];
PlanetOpt *DefPlanetOpt;
VertexDEM Vert, NearestPosIntersection, TempVec, TempVec2;
DEMBounds CurBounds;
NotifyTag Changes[2];

if (DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, NULL))
	{
	CaptureRestoreValues(RestoreVal);

	if (CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
		{
		Vert.ScrnXYZ[0] = NewCenterX;
		Vert.ScrnXYZ[1] = NewCenterY;
		Vert.ScrnXYZ[2] = 1.0;
		UnProjectVertexDEM(GlobalApp->AppEffects->FetchDefaultCoordSys(), &Vert, LatScale(GlobalApp->AppEffects->GetPlanetRadius()), 
			GlobalApp->AppEffects->GetPlanetRadius(), 1);
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetValue(Vert.Lat);
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetValue(Vert.Lon);
		if (OldWidth > 0 && OldHeight > 0)
			{
			WidthRatio = (double)NewWidth / (double)OldWidth;
			HeightRatio = (double)NewHeight / (double)OldHeight;
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].SetValue(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].CurValue * max(WidthRatio, HeightRatio));
			} // if
		} // if
	else if (CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD)
		{
		EarthCenter[0] = EarthCenter[1] = EarthCenter[2] = 0.0;
		// project where the ray through the new center intersects a sphere at low terrain elevation
		Vert.ScrnXYZ[0] = NewCenterX;
		Vert.ScrnXYZ[1] = NewCenterY;
		Vert.ScrnXYZ[2] = 1.0;
		UnProjectVertexDEM(GlobalApp->AppEffects->FetchDefaultCoordSys(), &Vert, LatScale(GlobalApp->AppEffects->GetPlanetRadius()), 
			GlobalApp->AppEffects->GetPlanetRadius(), 1);
		Vert.GetPosVector(CamPos);
		Vert.UnitVector();
		GlobalApp->AppDB->FillDEMBounds(&CurBounds); // if this fails it fills values of 0 which is what we want for default anyway

		CurBounds.LowElev = CalcExag(CurBounds.LowElev, DefPlanetOpt);
		// camera must be above the elevation of the lowest terrain
		if (AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue > CurBounds.LowElev)
			{
			// intersect the sphere using the view ray through the new view center. 
			// Distance is not used but must be supplied as a double by reference
			if (RaySphereIntersect(CurBounds.LowElev + GlobalApp->AppEffects->GetPlanetRadius(), 
				CamPos->XYZ, Vert.XYZ, EarthCenter, NearestPosIntersection.XYZ, Distance))
				{
				// transform cartesian to geographic
				#ifdef WCS_BUILD_VNS
				GlobalApp->AppEffects->FetchDefaultCoordSys()->CartToDeg(&NearestPosIntersection);
				#else // WCS_BUILD_VNS
				NearestPosIntersection.DegToCart(GlobalApp->AppEffects->GetPlanetRadius());
				#endif // WCS_BUILD_VNS
				// set the new values of lat/lon
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetValue(NearestPosIntersection.Lat);
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetValue(NearestPosIntersection.Lon);
				} // if
			} // if

		if (OldWidth > 0 && OldHeight > 0)
			{
			WidthRatio = (double)NewWidth / (double)OldWidth;
			HeightRatio = (double)NewHeight / (double)OldHeight;
			if (Orthographic)
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].SetValue(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].CurValue * max(WidthRatio, HeightRatio));
			else
				{
				GroundElev = GlobalApp->MainProj->Interactive->ElevationPoint(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue,
					AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue);
				GroundElev = CalcExag(GroundElev, DefPlanetOpt);
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetValue(GroundElev + (AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue - GroundElev) * max(WidthRatio, HeightRatio));
				//AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetValue(2.0 * atan(max(WidthRatio, HeightRatio) * tan(PiOver180 * AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue / 2.0)) / PiOver180);
				} // else
			} // if
		} // else if
	else
		{
		//double AngleLeft, AngleRight, AngleCenter;

		// determine a more accurate new view center to use for unprojecting
		// need to compensate for parallax
		// this is commented out because it seems to over-compensate or compensate in the wrong direction
		//AngleLeft = atan((((NewCenterX - NewWidth / 2.0 - (OldWidth / 2.0)) / (OldWidth / 2.0))) * tan(PiOver180 * AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue / 2.0)) / PiOver180;
		//AngleRight = atan((((NewCenterX + NewWidth / 2.0 - (OldWidth / 2.0)) / (OldWidth / 2.0))) * tan(PiOver180 * AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue / 2.0)) / PiOver180;
		//AngleCenter = (AngleLeft + AngleRight) / 2.0;
		//NewCenterX = tan(AngleCenter * PiOver180) * OldWidth / 2.0 + OldWidth / 2.0;

		// project a ray through the new view center, find its heading and pitch rotation relative to current view vector
		Vert.ScrnXYZ[0] = NewCenterX;
		Vert.ScrnXYZ[1] = NewCenterY;
		Vert.ScrnXYZ[2] = 1.0;
		UnProjectVertexDEM(GlobalApp->AppEffects->FetchDefaultCoordSys(), &Vert, LatScale(GlobalApp->AppEffects->GetPlanetRadius()), 
			GlobalApp->AppEffects->GetPlanetRadius(), 1);

		Vert.GetPosVector(CamPos);
		Vert.UnitVector();

		// here's the fun part - rotate this thing every which way
		// rotate the existing view vector to its normal position
		TempVec.CopyXYZ(TargPos);
		TempVec.RotateY(-CamPos->Lon);
		TempVec.RotateX(90.0 - CamPos->Lat);
		// apply heading and pitch rotations
		// find angle that vector makes around y with +z axis
		YAngle = TempVec.FindAngleYfromZ();
		// rotate around y by -y angle
		TempVec.RotateY(-YAngle);
		// find angle that vector makes around x with +z axis
		XAngle = TempVec.FindAngleXfromZ();
		// rotate around y by -x angle
		TempVec.RotateX(-XAngle);

		// rotate the new view vector to its normal position
		TempVec2.CopyXYZ(&Vert);
		TempVec2.RotateY(-CamPos->Lon);
		TempVec2.RotateX(90.0 - CamPos->Lat);
		// rotate around y by -y angle
		TempVec2.RotateY(-YAngle);
		// rotate around y by -x angle
		TempVec2.RotateX(-XAngle);

		// the remaining y and x rotation angles are the heading and pitch modifiers
		YAngle = TempVec2.FindAngleYfromZ();
		// rotate around y by -y angle
		TempVec2.RotateY(-YAngle);
		// find angle that vector makes around x with +z axis
		XAngle = TempVec2.FindAngleXfromZ();

		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].SetValue(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].CurValue + YAngle);
		AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].SetValue(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].CurValue + XAngle);

		if (OldWidth > 0 && OldHeight > 0)
			{
			WidthRatio = (double)NewWidth / (double)OldWidth;
			HeightRatio = (double)NewHeight / (double)OldHeight;
			if (Orthographic)
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].SetValue(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].CurValue * max(WidthRatio, HeightRatio));
			else
				AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetValue(2.0 * atan(max(WidthRatio, HeightRatio) * tan(PiOver180 * AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue / 2.0)) / PiOver180);
			} // if
		} // else
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX].SetValue(.5);
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY].SetValue(.5);

	RestoreValid = 1;
	SetFloating(0);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

} // Camera::ZoomBox

/*===========================================================================*/

void Camera::RestoreZoom(void)
{
long Ct;
double TempVal[WCS_EFFECTS_CAMERA_NUMRESTOREVALUES];
NotifyTag Changes[2];

if (RestoreValid)
	{
	CaptureRestoreValues(TempVal);

	RestoreRestoreValues(RestoreVal);

	for (Ct = 0; Ct < WCS_EFFECTS_CAMERA_NUMRESTOREVALUES; Ct ++)
		{
		RestoreVal[Ct] = TempVal[Ct];
		} // for

	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

} // Camera::RestoreZoom

/*===========================================================================*/

void Camera::RestoreZoomProxy(Camera *TheCameraToRestore)
{
double TempVal[WCS_EFFECTS_CAMERA_NUMRESTOREVALUES];
NotifyTag Changes[2];
long Ct;

CaptureRestoreValues(RestoreVal);

TheCameraToRestore->CaptureRestoreValues(TempVal);
TheCameraToRestore->RestoreRestoreValues(RestoreVal);

for (Ct = 0; Ct < WCS_EFFECTS_CAMERA_NUMRESTOREVALUES; Ct ++)
	{
	TheCameraToRestore->RestoreVal[Ct] = TempVal[Ct];
	} // for
TheCameraToRestore->RestoreValid = 1;

Changes[0] = MAKE_ID(TheCameraToRestore->GetNotifyClass(), TheCameraToRestore->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, TheCameraToRestore->GetRAHostRoot());

} // Camera::RestoreZoomProxy

/*===========================================================================*/

// CamView may pass NULL for Buffers
int Camera::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{
double HFOV;

if (Smooth)
	{
	delete Smooth;
	Smooth = NULL;
	} // if

if (! CamPos)
	{
	if (! (CamPos = new VertexDEM()))
		return (0);
	} // if
if (! TargPos)
	{
	if (! (TargPos = new VertexDEM()))
		return (0);
	} // if
if (! CamVertical)
	{
	if (! (CamVertical = new VertexDEM()))
		return (0);
	} // if
if (! CamRight)
	{
	if (! (CamRight = new VertexDEM()))
		return (0);
	} // if

if (PanoCam)
	{
	HFOV = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].GetValue(0, 0.0);
	if ((PanoPanels = quickftol(360.0 / HFOV)) < 3)
		PanoPanels = 3;
	} // if
else
	PanoPanels = 1;

ApplyLensDistortion = ((CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC) || PanoCam || Orthographic) ? 0: LensDistortion;

return (1);

} // Camera::InitToRender

/*===========================================================================*/

void Camera::GetTargetPosition(RenderData *Rend)
{

if (CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED)
	{
	if (TargetObj)
		{
		// translate object origin's lat/lon/elev + xyz offset into global cartesian coordinates
		// the same as would be done to render the object
		if (! TargetObj->FindCurrentCartesian(Rend, TargPos, CenterOnOrigin))
			{
			TargPos->XYZ[0] = TargPos->XYZ[1] = TargPos->XYZ[2] = 0.0;
			} // if
		} // if
	else
		{
		TargPos->Lat = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].CurValue;
		TargPos->Lon = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].CurValue - Rend->EarthRotation;
		TargPos->Elev = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].CurValue;
		#ifdef WCS_BUILD_VNS
		Rend->DefCoords->DegToCart(TargPos);
		#else // WCS_BUILD_VNS
		TargPos->DegToCart(Rend->PlanetRad);
		#endif // WCS_BUILD_VNS
		} // else
	} // else if untargeted

} // Camera::GetTargetPosition

/*===========================================================================*/

int Camera::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{
double Heading, Pitch, Bank, PanoHeading, StereoOffset, VecLen, FirstX, FirstY, FirstZ, PrevTime, PrevTimeX, PrevTimeY, PrevTimeZ, RenderTime;
int NodeX, NodeY, NodeZ, JiggleABit, GoBackward, GoForward, StereoSide;
VertexDEM TempVec;
Matx3x3 PanoMatx, LocalMatx, WorldMatx, TempMatx;

Heading = Pitch = Bank = PanoHeading = StereoOffset = 0.0;
CamHeading = CamPitch = CamBank = 0.0;

// Modify camera position based on smoothed path.
// Set new position in AnimPar[].CurValue.
// Don't use smoothing if from CamView.
if (VelocitySmoothing && (! Rend->IsCamView || Rend->IsProcessing))
	{
	if (! Smooth)
		{
		if (! (Smooth = new SmoothPath(&AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON], &AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT],
			&AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV], AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_EASEIN].CurValue, AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_EASEOUT].CurValue, 
			Rend->FrameRate, 1, Rend->EarthLatScaleMeters)))
			return (0);
		else if (Smooth->ConstructError)
			return (0);
		} // if need to init
	Smooth->GetSmoothPoint(AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue, AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue,
			AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue, Rend->RenderTime);
	} // if smooth camera path

// find camera position
CamPos->Lat = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue;
CamPos->Lon = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue - Rend->EarthRotation;
CamPos->Elev = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue;

#ifdef WCS_BUILD_VNS
Rend->DefCoords->DegToCart(CamPos);
#else // WCS_BUILD_VNS
CamPos->DegToCart(Rend->PlanetRad);
#endif // WCS_BUILD_VNS

// find target position and view vector
if (CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED)
	{
	if (TargetObj)
		{
		// translate object origin's lat/lon/elev + xyz offset into global cartesian coordinates
		// the same as would be done to render the object
		if (! TargetObj->FindCurrentCartesian(Rend, TargPos, CenterOnOrigin))
			return (0);
		} // if
	else
		{
		TargPos->Lat = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].CurValue;
		TargPos->Lon = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].CurValue - Rend->EarthRotation;
		TargPos->Elev = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].CurValue;
		#ifdef WCS_BUILD_VNS
		Rend->DefCoords->DegToCart(TargPos);
		#else // WCS_BUILD_VNS
		TargPos->DegToCart(Rend->PlanetRad);
		#endif // WCS_BUILD_VNS
		} // else
	// convert target position to a view vector
	TargPos->GetPosVector(CamPos);
	TargPos->Lat = CamPos->Lat;
	TargPos->Lon = CamPos->Lon;
	TargPos->Elev = CamPos->Elev;
	TargPos->RotateToHome();
	Heading = TargPos->FindAngleYfromZ();
	TargPos->RotateY(-Heading);
	Pitch = TargPos->FindAngleXfromZ();
	Heading += AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].CurValue;
	Pitch += AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].CurValue;
	} // if targeted
else if (CameraType == WCS_EFFECTS_CAMERATYPE_ALIGNED)
	{
	// find camera position an instant before the current time and an instant after.
	// the vector between the two positions, if they are different, is the view vector.
	RenderTime = Rend->RenderTime;
	JiggleABit = GoBackward = GoForward = 1;

	RepeatTest:

	if (Smooth)
		{
		Smooth->GetSmoothPoint(TargPos->Lon, TargPos->Lat,
				TargPos->Elev, RenderTime - WCS_RENDERTIME_WEEBIT);
		#ifdef WCS_BUILD_VNS
		Rend->DefCoords->DegToCart(TargPos);
		#else // WCS_BUILD_VNS
		TargPos->DegToCart(Rend->PlanetRad);
		#endif // WCS_BUILD_VNS
		FirstX = TargPos->XYZ[0];
		FirstY = TargPos->XYZ[1];
		FirstZ = TargPos->XYZ[2];
		Smooth->GetSmoothPoint(TargPos->Lon, TargPos->Lat,
				TargPos->Elev, RenderTime + WCS_RENDERTIME_WEEBIT);
		#ifdef WCS_BUILD_VNS
		Rend->DefCoords->DegToCart(TargPos);
		#else // WCS_BUILD_VNS
		TargPos->DegToCart(Rend->PlanetRad);
		#endif // WCS_BUILD_VNS
		TargPos->XYZ[0] -= FirstX;
		TargPos->XYZ[1] -= FirstY;
		TargPos->XYZ[2] -= FirstZ;
		} // if smooth path
	else
		{
		TargPos->Lat = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetValue(0, RenderTime - WCS_RENDERTIME_WEEBIT);
		TargPos->Lon = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetValue(0, RenderTime - WCS_RENDERTIME_WEEBIT);
		TargPos->Elev = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetValue(0, RenderTime - WCS_RENDERTIME_WEEBIT);
		#ifdef WCS_BUILD_VNS
		Rend->DefCoords->DegToCart(TargPos);
		#else // WCS_BUILD_VNS
		TargPos->DegToCart(Rend->PlanetRad);
		#endif // WCS_BUILD_VNS
		FirstX = TargPos->XYZ[0];
		FirstY = TargPos->XYZ[1];
		FirstZ = TargPos->XYZ[2];
		TargPos->Lat = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetValue(0, RenderTime + WCS_RENDERTIME_WEEBIT);
		TargPos->Lon = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetValue(0, RenderTime + WCS_RENDERTIME_WEEBIT);
		TargPos->Elev = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetValue(0, RenderTime + WCS_RENDERTIME_WEEBIT);
		#ifdef WCS_BUILD_VNS
		Rend->DefCoords->DegToCart(TargPos);
		#else // WCS_BUILD_VNS
		TargPos->DegToCart(Rend->PlanetRad);
		#endif // WCS_BUILD_VNS
		TargPos->XYZ[0] -= FirstX;
		TargPos->XYZ[1] -= FirstY;
		TargPos->XYZ[2] -= FirstZ;
		} // else not smooth path

	// Check to see that the view vector points somewhere, otherwise treat as overhead camera.
	// Need to do something different if camera is stationary for part of path.
	if (TargPos->XYZ[0] == 0.0 && TargPos->XYZ[1] == 0.0 && TargPos->XYZ[2] == 0.0)
		{
		// first just try jiggling the position a wee bit, camera may be at apex of a symmetric arc
		if (JiggleABit == 1)
			{
			if (RenderTime >= WCS_RENDERTIME_WEEBIT)
				RenderTime = Rend->RenderTime - WCS_RENDERTIME_WEEBIT;
			else
				RenderTime = Rend->RenderTime + WCS_RENDERTIME_WEEBIT;
			JiggleABit = 2;
			goto RepeatTest;
			} // if
		else if (JiggleABit == 2)
			{
			RenderTime = Rend->RenderTime;
			JiggleABit = 0;
			} // else if already tried jiggling position

		// find most recent time at which camera position was different
		if (GoBackward)
			{
			NodeX = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetNextDist(PrevTimeX, 0, RenderTime);
			NodeY = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetNextDist(PrevTimeY, 0, RenderTime);
			NodeZ = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetNextDist(PrevTimeZ, 0, RenderTime);
			if (NodeX || NodeY || NodeZ)
				{
				PrevTime = RenderTime;
				if (NodeX)
					PrevTime = PrevTimeX;
				if (NodeY)
					{
					if (PrevTime < RenderTime)
						PrevTime = max(PrevTimeY, PrevTime);
					else
						PrevTime = PrevTimeY;
					} // if
				if (NodeZ)
					{
					if (PrevTime < RenderTime)
						PrevTime = max(PrevTimeZ, PrevTime);
					else
						PrevTime = PrevTimeZ;
					} // if
				RenderTime = PrevTime;
				goto RepeatTest;
				} // if
			else
				{
				GoBackward = 0;
				RenderTime = Rend->RenderTime;
				} // else time to try forward
			} // if

		// if that failed find next time at which camera position was different
		if (GoForward)
			{
			NodeX = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetNextDist(PrevTimeX, 1, RenderTime);
			NodeY = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetNextDist(PrevTimeY, 1, RenderTime);
			NodeZ = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetNextDist(PrevTimeZ, 1, RenderTime);
			if (NodeX || NodeY || NodeZ)
				{
				PrevTime = RenderTime;
				if (NodeX)
					PrevTime = PrevTimeX;
				if (NodeY)
					{
					if (PrevTime > RenderTime)
						PrevTime = min(PrevTimeY, PrevTime);
					else
						PrevTime = PrevTimeY;
					} // if
				if (NodeZ)
					{
					if (PrevTime > RenderTime)
						PrevTime = min(PrevTimeZ, PrevTime);
					else
						PrevTime = PrevTimeZ;
					} // if
				RenderTime = PrevTime;
				goto RepeatTest;
				} // if
			else
				GoForward = 0;
			} // if

		// if all else fails align camera to vertical
		TargPos->GetPosVector(CamPos);
		} // if stationary

	TargPos->Lat = CamPos->Lat;
	TargPos->Lon = CamPos->Lon;
	TargPos->Elev = CamPos->Elev;
	TargPos->RotateToHome();
	Heading = TargPos->FindAngleYfromZ();
	TargPos->RotateY(-Heading);
	Pitch = TargPos->FindAngleXfromZ();
	Heading += AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].CurValue;
	Pitch += AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].CurValue;
	} // else if aligned to path
else if (CameraType == WCS_EFFECTS_CAMERATYPE_UNTARGETED)
	{
	Heading = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].CurValue;
	Pitch = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].CurValue;
	} // else if overhead or planimetric
else if (CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD || CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
	{
	Pitch = 90.0;
	} // else if overhead or planimetric
else
	{
	// must be some newfangled camera type we don't know about - bail out quick before it bites us!
	return (0);
	} // else	

Bank = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].CurValue;

CamVertical->XYZ[0] = CamVertical->XYZ[2] = 0.0;
CamVertical->XYZ[1] = 1.0;
CamRight->XYZ[1] = CamRight->XYZ[2] = 0.0;
CamRight->XYZ[0] = 1.0;
TargPos->XYZ[0] = TargPos->XYZ[1] = 0.0;
TargPos->XYZ[2] = 1.0;

if (PanoCam)
	{
	// rotate the camera by x degrees around its vertical axis
	PanoHeading = Rend->PanoPanel * 360.0 / PanoPanels;
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetValue(360.0 / PanoPanels);
	} // if 
else if (StereoCam)
	{
	if (Rend->IsCamView)
		StereoSide = (StereoPreviewChannel == WCS_CAMERA_STEREOCHANNEL_LEFT ? -1: (StereoPreviewChannel == WCS_CAMERA_STEREOCHANNEL_RIGHT ? 1: 0));
	else
		StereoSide = Rend->StereoSide;
	StereoOffset = StereoSide * .5 * AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_STEREOSEPARATION].CurValue;
	if (StereoConvergence)
		{
		// rotate the camera by x degrees around its vertical axis
		PanoHeading = -atan(StereoOffset / AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_STEREOCONVERGENCE].CurValue);
		} // if
	} // if 

BuildRotationMatrix(0.0, -PanoHeading * PiOver180, 0.0, PanoMatx);
BuildRotationMatrix(-Pitch * PiOver180, -Heading * PiOver180, -Bank * PiOver180, LocalMatx);
BuildRotationMatrix(-(CamPos->Lat - 90.0) * PiOver180, -CamPos->Lon * PiOver180, 0.0, WorldMatx);

Multiply3x3Matrices(LocalMatx, PanoMatx, TempMatx);
Multiply3x3Matrices(WorldMatx, TempMatx, InvScrRotMatx);

RotatePoint(TargPos->XYZ, InvScrRotMatx);
RotatePoint(CamVertical->XYZ, InvScrRotMatx);
RotatePoint(CamRight->XYZ, InvScrRotMatx);

BuildInvRotationMatrix(0.0, -PanoHeading * PiOver180, 0.0, PanoMatx);
BuildInvRotationMatrix(-Pitch * PiOver180, -Heading * PiOver180, -Bank * PiOver180, LocalMatx);
BuildInvRotationMatrix(-(CamPos->Lat - 90.0) * PiOver180, -CamPos->Lon * PiOver180, 0.0, WorldMatx);

Multiply3x3Matrices(LocalMatx, WorldMatx, TempMatx);
Multiply3x3Matrices(PanoMatx, TempMatx, ScrRotMatx);

// stereo offset
CamPos->XYZ[0] += StereoOffset * CamRight->XYZ[0];
CamPos->XYZ[1] += StereoOffset * CamRight->XYZ[1];
CamPos->XYZ[2] += StereoOffset * CamRight->XYZ[2];

// create a 2D vector for interactive view manipulation
IAUpVec2D[0] = IAUpVec2D[1] = IARightVec2D[0] = IARightVec2D[1] = 0.0;
if (CameraType != WCS_EFFECTS_CAMERATYPE_OVERHEAD && CameraType != WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
	{
	TempVec.CopyXYZ(TargPos);
	TempVec.RotateY(-CamPos->Lon);
	TempVec.RotateX(90.0 - CamPos->Lat);
	IAUpVec2D[0] = TempVec.XYZ[0];
	IAUpVec2D[1] = TempVec.XYZ[2];
	} // if
else if (fabs(IAUpVec2D[0]) < .01 && fabs(IAUpVec2D[1]) < .01)
	{
	TempVec.CopyXYZ(CamVertical);
	TempVec.RotateY(-CamPos->Lon);
	TempVec.RotateX(90.0 - CamPos->Lat);
	IAUpVec2D[0] = TempVec.XYZ[0];
	IAUpVec2D[1] = TempVec.XYZ[2];
	} // else
VecLen = sqrt(IAUpVec2D[0] * IAUpVec2D[0] + IAUpVec2D[1] * IAUpVec2D[1]);
if (VecLen > 0.0)
	{
	IAUpVec2D[0] /= VecLen;	// this will scale the longitude component of interactive movement
	IAUpVec2D[1] /= VecLen;	// this will scale the latitude component of interactive movement
	IARightVec2D[0] = IAUpVec2D[0];
	IARightVec2D[1] = IAUpVec2D[1];
	RotatePoint(IARightVec2D, 90.0);
	IAUpVec2D[0] = -IAUpVec2D[0];
	IARightVec2D[0] = -IARightVec2D[0];
	} // if

CamLonScale = cos(CamPos->Lat * PiOver180);

#ifdef WCS_BUILD_VNS
if (CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && Projected && Coords)
	{
	TempVec.Lat = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue;
	TempVec.Lon = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue;
	TempVec.Elev = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue;
	Coords->DefDegToProj(&TempVec);
	ProjectedEasting = TempVec.xyz[0];
	ProjectedNorthing = TempVec.xyz[1];
	} // if
#endif // WCS_BUILD_VNS

SetScales(Rend->SetupWidth, Rend->PixelAspect);

if (CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
	{
	double TempX1, TempX2;

	TempVec.Lat = 0.0;
	TempVec.Lon = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue + 180.0;
	TempVec.Elev = 0.0;
	ProjectVertexDEM(Rend->DefCoords, &TempVec, Rend->EarthLatScaleMeters, Rend->PlanetRad, 0);
	TempX1 = TempVec.ScrnXYZ[0];
	TempVec.Lon = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue - 180.0;
	ProjectVertexDEM(Rend->DefCoords, &TempVec, Rend->EarthLatScaleMeters, Rend->PlanetRad, 0);
	TempX2 = TempVec.ScrnXYZ[0];
	PlanWidth = fabs(TempX1 - TempX2);
	TempVec.Lon = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue - 178.0;
	ProjectVertexDEM(Rend->DefCoords, &TempVec, Rend->EarthLatScaleMeters, Rend->PlanetRad, 0);
	TempX2 = TempVec.ScrnXYZ[0];
	PlanTranslateWidth = fabs(TempX1 - TempX2);
	} // if

// preserve settings for use by Post process text overlay
CamHeading = Heading;
CamPitch = Pitch;
CamBank = Bank;

return (GeneralEffect::InitFrameToRender(Lib, Rend));

} // Camera::InitFrameToRender

/*===========================================================================*/

void Camera::SetScales(long ImageWidth, double Aspect)
{

if (CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC || (CameraType != WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && Orthographic))
	HorScale = ImageWidth / AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].CurValue;		//(double)ImageWidth / (AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue * 2.0 * tan(.5 * AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue * PiOver180));
else
	HorScale = (double)ImageWidth / (2.0 * tan(.5 * AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue * PiOver180));
VertScale = HorScale * Aspect;

} // Camera::SetScales

/*===========================================================================*/

void Camera::ProjectVertexDEM(CoordSys *MyCoords, VertexDEM *Vert, double EarthLatScaleMeters, double PlanetRad, char ConvertVtx)
{
double XYZ[3], TempLon, AbsZ;

// convert lon/lat/elev to XYZ
if (ConvertVtx)
	#ifdef WCS_BUILD_VNS
	MyCoords->DegToCart(Vert);
	#else // WCS_BUILD_VNS
	Vert->DegToCart(PlanetRad);
	#endif // WCS_BUILD_VNS

if (CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
	{
	#ifdef WCS_BUILD_VNS
	if (Projected && Coords)
		{
		VertexDEM TempVert;

		TempVert.CopyLatLon(Vert);
		Coords->DefDegToProj(&TempVert);
		Vert->ScrnXYZ[0] = TempVert.xyz[0] - ProjectedEasting;
		Vert->ScrnXYZ[1] = TempVert.xyz[1] - ProjectedNorthing;
		if (Coords->GetGeographic())
			{
			Vert->ScrnXYZ[0] = -Vert->ScrnXYZ[0];
			TempLon = Vert->ScrnXYZ[0];
			if (fabs(TempLon) > 180.0)
				{
				TempLon += 180.0;
				if (fabs(TempLon) > 360.0)
					TempLon = fmod(TempLon, 360.0);	// retains the original sign
				if (TempLon < 0.0)
					TempLon += 360.0;
				TempLon -= 180.0;
				Vert->ScrnXYZ[0] = TempLon;
				} // if
			Vert->ScrnXYZ[0] *= CamLonScale * EarthLatScaleMeters;
			Vert->ScrnXYZ[1] *= EarthLatScaleMeters;
			} // if
		}	// convert lon/lat/elev to x/y/z
	else
	#endif // WCS_BUILD_VNS
		{
		TempLon = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue - Vert->Lon;
		if (fabs(TempLon) > 180.0)
			{
			TempLon += 180.0;
			if (fabs(TempLon) > 360.0)
				TempLon = fmod(TempLon, 360.0);	// retains the original sign
			if (TempLon < 0.0)
				TempLon += 360.0;
			TempLon -= 180.0;
			} // if
		Vert->ScrnXYZ[0] = TempLon * CamLonScale * EarthLatScaleMeters;
		Vert->ScrnXYZ[1] = (Vert->Lat - AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue) * EarthLatScaleMeters;
		} // else
	Vert->ScrnXYZ[2] = Vert->Q = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue - Vert->Elev;
	} // if
else
	{
	XYZ[0] = Vert->XYZ[0];
	XYZ[1] = Vert->XYZ[1];
	XYZ[2] = Vert->XYZ[2];

	// convert XYZ to Screen coords via projection matrix
	Vert->GetPosVector(CamPos);
	Vert->ProjectPoint(ScrRotMatx);
	Vert->XYZ[0] = XYZ[0];
	Vert->XYZ[1] = XYZ[1];
	Vert->XYZ[2] = XYZ[2];

	// compute distance to vertex
	Vert->Q = sqrt(Vert->ScrnXYZ[0] * Vert->ScrnXYZ[0] + Vert->ScrnXYZ[1] * Vert->ScrnXYZ[1] + Vert->ScrnXYZ[2] * Vert->ScrnXYZ[2]);

	// if perspective divide by Z
	if (PanoCam)
		{
		AbsZ = fabs(Vert->ScrnXYZ[2]);
		Vert->ScrnXYZ[0] /= AbsZ;
		Vert->ScrnXYZ[1] /= AbsZ;
		} // if
	else if (! Orthographic)
		{
		if (ApplyLensDistortion)
			{
			Vert->ScrnXYZ[0] /= Vert->Q;
			Vert->ScrnXYZ[1] /= Vert->Q;
			} // if
		else
			{
			AbsZ = fabs(Vert->ScrnXYZ[2]);
			Vert->ScrnXYZ[0] /= AbsZ;
			Vert->ScrnXYZ[1] /= AbsZ;
			} // else
		} // if
	} // else

// shift and scale the screen coordinate values
Vert->ScrnXYZ[0] = CenterX + Vert->ScrnXYZ[0] * HorScale;
Vert->ScrnXYZ[1] = CenterY - Vert->ScrnXYZ[1] * VertScale;
if (Vert->ScrnXYZ[2] < 0.0)
	Vert->Q = -Vert->Q;

} // Camera::ProjectVertexDEM

/*===========================================================================*/

void Camera::UnProjectVertexDEM(CoordSys *MyCoords, VertexDEM *Vert, double EarthLatScaleMeters, double PlanetRad, char ConvertVtx)
{
double TempX, TempY, TempZ;

TempX = Vert->ScrnXYZ[0];
TempY = Vert->ScrnXYZ[1];
TempZ = Vert->ScrnXYZ[2];

// correct for center and scales
Vert->ScrnXYZ[0] = (Vert->ScrnXYZ[0] - CenterX) / HorScale;
Vert->ScrnXYZ[1] = (CenterY - Vert->ScrnXYZ[1]) / VertScale;

if (CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
	{
	// convert x/y/z to lon/lat/elev
	Vert->xyz[2] = Vert->Elev = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue - Vert->ScrnXYZ[2];
	if (Projected && Coords)
		{
		if (Coords->GetGeographic())
			{
			Vert->ScrnXYZ[1] /= EarthLatScaleMeters;
			Vert->ScrnXYZ[0] /= (CamLonScale * EarthLatScaleMeters);
			Vert->ScrnXYZ[0] = -Vert->ScrnXYZ[0];
			} // if
		Vert->xyz[0] = Vert->ScrnXYZ[0] + ProjectedEasting;
		Vert->xyz[1] = Vert->ScrnXYZ[1] + ProjectedNorthing;
		Coords->ProjToDefDeg(Vert);
		} // if
	else
		{
		Vert->xyz[0] = Vert->Lon = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue - Vert->ScrnXYZ[0] / (CamLonScale * EarthLatScaleMeters);
		Vert->xyz[1] = Vert->Lat = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue + Vert->ScrnXYZ[1] / EarthLatScaleMeters;
		#ifdef WCS_BUILD_VNS
		MyCoords->DegToCart(Vert);
		#else // WCS_BUILD_VNS
		Vert->DegToCart(PlanetRad);
		#endif // WCS_BUILD_VNS
		} // else
	} // if
else
	{
	if (Vert->Q == Vert->ScrnXYZ[2])		// special value to tell us that we don't know q, it needs to be calculated
		Vert->Q = sqrt(Vert->ScrnXYZ[0] * Vert->ScrnXYZ[0] + Vert->ScrnXYZ[1] * Vert->ScrnXYZ[1] + Vert->ScrnXYZ[2] * Vert->ScrnXYZ[2]);

	if (PanoCam)
		{
		Vert->ScrnXYZ[0] *= Vert->ScrnXYZ[2];
		Vert->ScrnXYZ[1] *= Vert->ScrnXYZ[2];
		} // if
	else if (! Orthographic)
		{
		if (ApplyLensDistortion)
			{
			Vert->ScrnXYZ[0] *= Vert->Q;
			Vert->ScrnXYZ[1] *= Vert->Q;
			} // if
		else
			{
			Vert->ScrnXYZ[0] *= Vert->ScrnXYZ[2];
			Vert->ScrnXYZ[1] *= Vert->ScrnXYZ[2];
			} // else
		} // if


	// convert Screen coords to XYZ via inverse projection matrix
	Vert->UnProjectPoint(InvScrRotMatx);
	Vert->UnGetPosVector(CamPos);

	if (ConvertVtx)
		#ifdef WCS_BUILD_VNS
		MyCoords->CartToDeg(Vert);
		#else // WCS_BUILD_VNS
		Vert->CartToDeg(PlanetRad);
		#endif // WCS_BUILD_VNS
	} // else

// putem back like ya foundem
Vert->ScrnXYZ[0] = TempX;
Vert->ScrnXYZ[1] = TempY;
Vert->ScrnXYZ[2] = TempZ;

} // Camera::UnProjectVertexDEM

/*===========================================================================*/

void Camera::UnProjectProjectedVertexDEM(VertexDEM *Vert, double EarthLatScaleMeters)
{
double TempX, TempY, TempZ;

TempX = Vert->ScrnXYZ[0];
TempY = Vert->ScrnXYZ[1];
TempZ = Vert->ScrnXYZ[2];

// correct for center and scales
Vert->ScrnXYZ[0] = (Vert->ScrnXYZ[0] - CenterX) / HorScale;
Vert->ScrnXYZ[1] = (CenterY - Vert->ScrnXYZ[1]) / VertScale;

if (CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
	{
	// convert x/y/z to lon/lat/elev
	if (Projected && Coords)
		{
		if (Coords->GetGeographic())
			{
			Vert->ScrnXYZ[1] /= EarthLatScaleMeters;
			Vert->ScrnXYZ[0] /= (CamLonScale * EarthLatScaleMeters);
			Vert->ScrnXYZ[0] = -Vert->ScrnXYZ[0];
			} // if
		Vert->xyz[0] = Vert->ScrnXYZ[0] + ProjectedEasting;
		Vert->xyz[1] = Vert->ScrnXYZ[1] + ProjectedNorthing;
		} // if
	} // if

// putem back like ya foundem
Vert->ScrnXYZ[0] = TempX;
Vert->ScrnXYZ[1] = TempY;
Vert->ScrnXYZ[2] = TempZ;

} // Camera::UnProjectVertexDEM

/*===========================================================================*/

int Camera::TestValidScreenPosition(VertexDEM *Vert, double CompareValue)
{
double TempX, TempY, TempZ, StashX, StashY;
int LocationValid = 1;

TempX = Vert->ScrnXYZ[0];
TempY = Vert->ScrnXYZ[1];
TempZ = Vert->ScrnXYZ[2];

// correct for center and scales
Vert->ScrnXYZ[0] = (Vert->ScrnXYZ[0] - CenterX) / HorScale;
Vert->ScrnXYZ[1] = (CenterY - Vert->ScrnXYZ[1]) / VertScale;

if (CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
	{
	// convert x/y/z to lon/lat/elev, return value indicates success if non-zero, error indicates invalid screen coords
	if (Projected && Coords)
		{
		Vert->xyz[0] = StashX = Vert->ScrnXYZ[0] + ProjectedEasting;
		Vert->xyz[1] = StashY = Vert->ScrnXYZ[1] + ProjectedNorthing;
		if (LocationValid = Coords->ProjToDeg(Vert))
			{
			// now check to see if reverse gives original input values
			Coords->DegToProj(Vert);
			if (fabs(StashX - Vert->xyz[0]) > CompareValue || fabs(StashY - Vert->xyz[1]) > CompareValue)
				LocationValid = 0;
			} // if
		} // if
	} // if

Vert->ScrnXYZ[0] = TempX;
Vert->ScrnXYZ[1] = TempY;
Vert->ScrnXYZ[2] = TempZ;

return (LocationValid);

} // Camera::TestValidScreenPosition

/*===========================================================================*/

void Camera::ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds)
{
double ShiftWE, ShiftNS, ShiftUD, TempVal;
GraphNode *CurNode;

ShiftWE = ((CurBounds->West + CurBounds->East) - (OldBounds->West - OldBounds->East)) * 0.5;
ShiftNS = ((CurBounds->North + CurBounds->South) - (OldBounds->North + OldBounds->South)) * 0.5;
ShiftUD = ((CurBounds->HighElev + CurBounds->LowElev) - (OldBounds->HighElev + OldBounds->LowElev)) * 0.5;

// camera position
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetValue(
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue + ShiftNS);
if (CurNode = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetFirstNode(0))
	{
	TempVal = CurNode->GetValue() + ShiftNS;
	if (TempVal > 90.0)
		TempVal = 90.0;
	if (TempVal < -90.0)
		TempVal = -90.0;
	CurNode->SetValue(TempVal);
	while (CurNode = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetNextNode(0, CurNode))
		{
		TempVal = CurNode->GetValue() + ShiftNS;
		if (TempVal > 90.0)
			TempVal = 90.0;
		if (TempVal < -90.0)
			TempVal = -90.0;
		CurNode->SetValue(TempVal);
		} // while
	} // if
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetValue(
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue + ShiftWE);
if (CurNode = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetFirstNode(0))
	{
	CurNode->SetValue(CurNode->GetValue() + ShiftWE);
	while (CurNode = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetNextNode(0, CurNode))
		{
		CurNode->SetValue(CurNode->GetValue() + ShiftWE);
		} // while
	} // if
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetValue(
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue + ShiftUD);
if (CurNode = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetFirstNode(0))
	{
	CurNode->SetValue(CurNode->GetValue() + ShiftUD);
	while (CurNode = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetNextNode(0, CurNode))
		{
		CurNode->SetValue(CurNode->GetValue() + ShiftUD);
		} // while
	} // if

// target
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].SetValue(
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].CurValue + ShiftNS);
if (CurNode = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetFirstNode(0))
	{
	TempVal = CurNode->GetValue() + ShiftNS;
	if (TempVal > 90.0)
		TempVal = 90.0;
	if (TempVal < -90.0)
		TempVal = -90.0;
	CurNode->SetValue(TempVal);
	while (CurNode = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetNextNode(0, CurNode))
		{
		TempVal = CurNode->GetValue() + ShiftNS;
		if (TempVal > 90.0)
			TempVal = 90.0;
		if (TempVal < -90.0)
			TempVal = -90.0;
		CurNode->SetValue(TempVal);
		} // while
	} // if
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].SetValue(
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].CurValue + ShiftWE);
if (CurNode = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].GetFirstNode(0))
	{
	CurNode->SetValue(CurNode->GetValue() + ShiftWE);
	while (CurNode = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].GetNextNode(0, CurNode))
		{
		CurNode->SetValue(CurNode->GetValue() + ShiftWE);
		} // while
	} // if
AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].SetValue(
	AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].CurValue + ShiftUD);
if (CurNode = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].GetFirstNode(0))
	{
	CurNode->SetValue(CurNode->GetValue() + ShiftUD);
	while (CurNode = AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].GetNextNode(0, CurNode))
		{
		CurNode->SetValue(CurNode->GetValue() + ShiftUD);
		} // while
	} // if

} // Camera::ScaleToDEMBounds

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int Camera::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
Camera *CurrentCamera = NULL;
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
						} // if material
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
					else if (! strnicmp(ReadBuf, "Camera", 8))
						{
						if (CurrentCamera = new Camera(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentCamera->Load(ffile, Size, ByteFlip)) == Size)
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

if (Success == 1 && CurrentCamera)
	{
	if (EffectsLib::LoadQueries && OldBoundsLoaded && GlobalApp->AppDB->FillDEMBounds(&CurBounds))
		{
		if (UserMessageYN("Load Camera", "Do you wish the loaded Camera's position\n to be scaled to current DEM bounds?"))
			{
			CurrentCamera->ScaleToDEMBounds(&OldBounds, &CurBounds);
			} // if
		} // if
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentCamera);
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

} // Camera::LoadObject

/*===========================================================================*/

int Camera::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
#ifdef WCS_BUILD_VNS
EffectList *CurEffect, *CoordsList = NULL;
#endif // WCS_BUILD_VNS
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

#ifdef WCS_BUILD_VNS
if (BuildFileComponentsList(&CoordsList))
	{
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

	while (CoordsList)
		{
		CurEffect = CoordsList;
		CoordsList = CoordsList->Next;
		delete CurEffect;
		} // while
	} // if
#endif // WCS_BUILD_VNS

// Camera
strcpy(StrBuf, "Camera");
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
			} // if Camera saved 
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

} // Camera::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::Camera_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
Camera *Current;

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
						if (Current = new Camera(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (Camera *)FindDuplicateByName(Current->EffectType, Current))
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

} // EffectsLib::Camera_Load()

/*===========================================================================*/

ULONG EffectsLib::Camera_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
Camera *Current;

Current = Cameras;
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
	Current = (Camera *)Current->Next;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // EffectsLib::Camera_Save()

/*===========================================================================*/

SmoothPath::SmoothPath()
{

Smooth[0] = Smooth[1] = Smooth[2] = NULL;
FrameTime = 0.0;
NumFrames = 0;
ConstructError = 0;

} // SmoothPath::SmoothPath

/*===========================================================================*/

int SmoothPath::Copy(SmoothPath *CopyFrom)
{

FrameTime = CopyFrom->FrameTime;
NumFrames = CopyFrom->NumFrames;

if (CopyFrom->Smooth[0])
	{
	if (Smooth[0] = (double *)AppMem_Alloc(NumFrames * sizeof (double), 0))
		memcpy(Smooth[0], CopyFrom->Smooth[0], NumFrames * sizeof (double));
	else 
		return (0);
	} // if
if (CopyFrom->Smooth[1])
	{
	if (Smooth[1] = (double *)AppMem_Alloc(NumFrames * sizeof (double), 0))
		memcpy(Smooth[1], CopyFrom->Smooth[1], NumFrames * sizeof (double));
	else 
		return (0);
	} // if
if (CopyFrom->Smooth[2])
	{
	if (Smooth[2] = (double *)AppMem_Alloc(NumFrames * sizeof (double), 0))
		memcpy(Smooth[2], CopyFrom->Smooth[2], NumFrames * sizeof (double));
	else
		return (0);
	} // if

return (1);

} // SmoothPath::Copy

/*===========================================================================*/

SmoothPath::SmoothPath(AnimDoubleTime *AnimX, AnimDoubleTime *AnimY, AnimDoubleTime *AnimZ, 
	double EaseInTime, double EaseOutTime, double FrameRate, int GeoCoordVariable, double LatScaleMeters)
{
AnimDoubleTime *Path[3];
double FirstDummy, SecondDummy, Offset, DistX, DistY, DistZ, VBar, CurVel, LastVel, VelInc, CurDist, 
	CurTime, TotalEaseTime, MaxTime = 0.0;
double *TempPath[3], *Distance, *PathLen;
long Ct, Traveled, LastPt, NextPt;

ConstructError = 0;
Smooth[0] = Smooth[1] = Smooth[2] = NULL;
Path[0] = AnimX;
Path[1] = AnimY;
Path[2] = AnimZ;
FrameTime = 0.0;

// find number of frames in animated path
Path[0]->GetMinMaxDist(FirstDummy, SecondDummy);
if (SecondDummy > MaxTime)
	MaxTime = SecondDummy;
Path[1]->GetMinMaxDist(FirstDummy, SecondDummy);
if (SecondDummy > MaxTime)
	MaxTime = SecondDummy;
if (Path[2])
	{
	Path[2]->GetMinMaxDist(FirstDummy, SecondDummy);
	if (SecondDummy > MaxTime)
		MaxTime = SecondDummy;
	} // if

if (MaxTime <= 0.0)
	{
	return;
	} // if

//limit ease in and out to max time
if ((TotalEaseTime = EaseInTime + EaseOutTime) > MaxTime)
	{
	EaseInTime *= MaxTime / TotalEaseTime;
	EaseOutTime *= MaxTime / TotalEaseTime;
	} // if

// convert time to frames
NumFrames = quickftol(MaxTime * FrameRate);

// multiply by 100
NumFrames *= 100;

// add 1 for frame 0
NumFrames ++;

// no smoothing necessary if path is very short
if (NumFrames > 2)
	{
	// create table of values for splined position
	Smooth[0] = (double *)AppMem_Alloc(NumFrames * sizeof (double), 0);
	Smooth[1] = (double *)AppMem_Alloc(NumFrames * sizeof (double), 0);
	Smooth[2] = (double *)AppMem_Alloc(NumFrames * sizeof (double), 0);
	TempPath[0] = (double *)AppMem_Alloc(NumFrames * sizeof (double), 0);
	TempPath[1] = (double *)AppMem_Alloc(NumFrames * sizeof (double), 0);
	TempPath[2] = (double *)AppMem_Alloc(NumFrames * sizeof (double), 0);
	PathLen = (double *)AppMem_Alloc(NumFrames * sizeof (double), 0);
	Distance = (double *)AppMem_Alloc(NumFrames * sizeof (double), 0);
	if (Smooth[0] && Smooth[1] && Smooth[2] && TempPath[0] && TempPath[1] && TempPath[2] && PathLen && Distance)
		{
		FrameTime = 1.0 / (FrameRate * 100.0);
		// determine splined values at each interval
		for (Ct = 0, CurTime = 0.0; Ct < NumFrames; Ct ++, CurTime += FrameTime)
			{
			TempPath[0][Ct] = Path[0]->GetValue(0, CurTime);
			TempPath[1][Ct] = Path[1]->GetValue(0, CurTime);
			if (Path[2])
				TempPath[2][Ct] = Path[2]->GetValue(0, CurTime);
			else
				TempPath[2][Ct] = 0.0;
			} // for

		// find length of path
		PathLen[0] = 0.0;
		for (Ct = 1; Ct < NumFrames; Ct ++)
			{
			DistX = TempPath[0][Ct] - TempPath[0][Ct - 1];
			DistY = TempPath[1][Ct] - TempPath[1][Ct - 1];
			DistZ = TempPath[2][Ct] - TempPath[2][Ct - 1];
			if (GeoCoordVariable)
				{
				// convert to meters
				DistY *= LatScaleMeters;
				DistX *= LatScaleMeters * cos(((TempPath[1][Ct] + TempPath[1][Ct - 1]) * 0.5) * PiOver180);
				} // if
			PathLen[Ct] = PathLen[Ct - 1] + sqrt(DistX * DistX + DistY * DistY + DistZ * DistZ);
			} // for

		// if there is no path length then we must exit now or crash later
		if (PathLen[NumFrames - 1] <= 0.0)
			goto NoSmoothingNecessary;

		// find average velocity exclusive of ease in/out
		VBar = PathLen[NumFrames - 1] / (MaxTime - (EaseInTime + EaseOutTime) * 0.5);

		// calculate a distance along path to each "frame"
		CurDist = CurTime = CurVel = 0.0;
		Ct = 0;
		if (EaseInTime > 0.0)
			{
			VelInc = VBar / (EaseInTime * FrameRate * 100.0);
			for ( ; CurTime <= EaseInTime && Ct < NumFrames; Ct ++, CurTime += FrameTime)
				{
				Distance[Ct] = CurDist;
				LastVel = CurVel;
				CurVel += VelInc;
				if (CurVel > VBar)
					CurVel = VBar;
				CurDist += ((CurVel + LastVel) * 0.5) * FrameTime;
				} // for
			} // if ease in
		CurVel = VBar;
		for ( ; CurTime <= MaxTime - EaseOutTime && Ct < NumFrames; Ct ++, CurTime += FrameTime)
			{
			Distance[Ct] = CurDist;
			CurDist += VBar * FrameTime;
			} // for
		if (EaseOutTime > 0.0)
			{
			VelInc = VBar / (EaseOutTime * FrameRate * 100.0);
			for ( ; Ct < NumFrames; Ct ++, CurTime += FrameTime)
				{
				Distance[Ct] = CurDist;
				LastVel = CurVel;
				CurVel -= VelInc;
				if (CurVel < 0.0)
					CurVel = 0.0;
				CurDist += ((CurVel + LastVel) / 2.0) * FrameTime;
				} // for
			} // if ease out
		// in case of roundoff error causing a missed frame at end
		if (Ct < NumFrames)
			{
			for ( ; Ct < NumFrames; Ct ++)
				{
				Distance[Ct] = Distance[Ct - 1];
				} // for
			} // for

		// fill values in smooth table with interpolated values
		Traveled = 0;
		LastPt = 0;
		NextPt = 1;

		while (Traveled < NumFrames)
			{
			while (NextPt < NumFrames - 1 && Distance[Traveled] > PathLen[NextPt])
				{
				LastPt ++;
				NextPt ++;
				} // while
			if (Distance[Traveled] >= PathLen[NextPt])
				{
				Smooth[0][Traveled] = TempPath[0][NextPt];
				Smooth[1][Traveled] = TempPath[1][NextPt];
				Smooth[2][Traveled] = TempPath[2][NextPt];
				} // if
			else
				{
				Offset = (Distance[Traveled] - PathLen[LastPt]) / (PathLen[NextPt] - PathLen[LastPt]);
				Smooth[0][Traveled] = TempPath[0][LastPt] + (TempPath[0][NextPt] - TempPath[0][LastPt]) * Offset;
				Smooth[1][Traveled] = TempPath[1][LastPt] + (TempPath[1][NextPt] - TempPath[1][LastPt]) * Offset;
				Smooth[2][Traveled] = TempPath[2][LastPt] + (TempPath[2][NextPt] - TempPath[2][LastPt]) * Offset;
				} // else
			Traveled ++;
			} // while
		} // if
	else
		ConstructError = 1;
	if (PathLen)
		AppMem_Free(PathLen, NumFrames * sizeof (double));
	if (Distance)
		AppMem_Free(Distance, NumFrames * sizeof (double));
	if (TempPath[0])
		AppMem_Free(TempPath[0], NumFrames * sizeof (double));
	if (TempPath[1])
		AppMem_Free(TempPath[1], NumFrames * sizeof (double));
	if (TempPath[2])
		AppMem_Free(TempPath[2], NumFrames * sizeof (double));
	} // if

return;

NoSmoothingNecessary:

if (PathLen)
	AppMem_Free(PathLen, NumFrames * sizeof (double));
if (Distance)
	AppMem_Free(Distance, NumFrames * sizeof (double));
if (TempPath[0])
	AppMem_Free(TempPath[0], NumFrames * sizeof (double));
if (TempPath[1])
	AppMem_Free(TempPath[1], NumFrames * sizeof (double));
if (TempPath[2])
	AppMem_Free(TempPath[2], NumFrames * sizeof (double));
if (Smooth[0])
	AppMem_Free(Smooth[0], NumFrames * sizeof (double));
if (Smooth[1])
	AppMem_Free(Smooth[1], NumFrames * sizeof (double));
if (Smooth[2])
	AppMem_Free(Smooth[2], NumFrames * sizeof (double));
Smooth[0] = Smooth[1] = Smooth[2] = NULL;
FrameTime = 0.0;
NumFrames = 0;

} // SmoothPath::SmoothPath

/*===========================================================================*/

SmoothPath::~SmoothPath()
{

if (Smooth[0])
	AppMem_Free(Smooth[0], NumFrames * sizeof (double));
if (Smooth[1])
	AppMem_Free(Smooth[1], NumFrames * sizeof (double));
if (Smooth[2])
	AppMem_Free(Smooth[2], NumFrames * sizeof (double));

} // SmoothPath::~SmoothPath

/*===========================================================================*/

int SmoothPath::GetSmoothPoint(double &GetX, double &GetY, double &GetZ, double Time)
{
double SampleTime, Offset;
long SampleLow;

if (Smooth[0] && Smooth[1] && Smooth[2])
	{
	SampleTime = Time / FrameTime;
	SampleLow = quicklongfloor(SampleTime);

	if (SampleLow < 0)
		SampleLow = 0;
	else if (SampleLow >= NumFrames)
		SampleLow = NumFrames - 1;

	if (SampleLow < NumFrames - 1 && (Offset = SampleTime - SampleLow) > 0.0)
		{
		GetX = Smooth[0][SampleLow] + Offset * (Smooth[0][SampleLow + 1] - Smooth[0][SampleLow]);
		GetY = Smooth[1][SampleLow] + Offset * (Smooth[1][SampleLow + 1] - Smooth[1][SampleLow]);
		GetZ = Smooth[2][SampleLow] + Offset * (Smooth[2][SampleLow + 1] - Smooth[2][SampleLow]);
		} // if
	else
		{
		GetX = Smooth[0][SampleLow];
		GetY = Smooth[1][SampleLow];
		GetZ = Smooth[2][SampleLow];
		} // else
	return (1);
	} // if

return (0);

} // SmoothPath::GetSmoothPoint

/*===========================================================================*/

int Camera::AddDerivedPopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags)
{

PMA->AddPopMenuItem("Set Floating", "FLOAT", 1);

return(1);
} // Camera::AddDerivedPopMenus

/*===========================================================================*/

int Camera::HandlePopMenuSelection(void *Action)
{
// only one action known currently
SetFloating(1);
return(1);
} // Camera::HandlePopMenuSelection
