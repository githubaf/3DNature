// RenderShadow.cpp
// World Constrution Set code for initializing and rendering shadows.
// Written on 12/14/99 by Gary R. Huber.
// Copyright 1999 by Questar Productions. All rights reserved.

#include "stdafx.h"
#include "Render.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Points.h"
#include "Project.h"
#include "Interactive.h"
#include "Raster.h"
#include "Useful.h"
#include "MathSupport.h"
#include "Requester.h"
#include "RenderControlGUI.h"
#include "Lists.h"

#define WCS_LIGHT_UNCLAMPED_DISTANCE

#ifdef WCS_VECPOLY_EFFECTS
int Renderer::InitForShadows(WCSApp *AppSource, EffectsLib *EffectsSource, ImageLib *ImageSource, 
	Database *DBSource, Project *ProjectSource, MessageLog *LogSource, RenderInterface *MasterSource,
	RenderOpt *HostOpt, Renderer *HostRend, long FrameNum, double CurTime)
#else // WCS_VECPOLY_EFFECTS
int Renderer::InitForShadows(WCSApp *AppSource, EffectsLib *EffectsSource, ImageLib *ImageSource, 
	Database *DBSource, Project *ProjectSource, MessageLog *LogSource, RenderInterface *MasterSource,
	RenderOpt *HostOpt, Renderer *HostRend, long FrameNum)
#endif // WCS_VECPOLY_EFFECTS
{
Light *CurLight;
ShadowEffect *CurShadow;
CloudEffect *CurCloud;
Object3DEffect *CurObject;
JoeList *CurJoe;
VectorPoint *CurVtx;
Joe *SearchJoe;
NotifyTag Changes[2];
long CurVtxNum;
int Success = 1, DefaultShadowDone = 0;

if (Opt = new RenderOpt())
	{
	if (Cam = new Camera())
		{
		AppBase = AppSource;
		EffectsBase = EffectsSource;
		ImageBase = ImageSource;
		ProjectBase = ProjectSource;
		DBase = DBSource;
		LocalLog = LogSource;
		Master = MasterSource;
		Cam->RenderBeyondHorizon = 1;
		#ifdef WCS_VECPOLY_EFFECTS
		DEMCue = HostRend->DEMCue;
		EvalEffects = HostRend->EvalEffects;
		MyErrorFunctor->SetRend(this);
		#endif // WCS_VECPOLY_EFFECTS

		if (AppBase && EffectsBase && ImageBase && ProjectBase && DBase && LocalLog && HostOpt)
			{
			//StashProjectTime = ProjectBase->Interactive->GetActiveTime();
			//StashProjectFrameRate = ProjectBase->Interactive->GetFrameRate();
			ShadowMapInProgress = 1;
			CopySettings(HostRend);		// sets RenderTime too
			Opt->CopyEnabledSettings(Opt, HostOpt);
			Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].SetValue(HostOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue);
			Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_SIDEOVERSCAN].SetValue(.02);
			Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_BOTTOMOVERSCAN].SetValue(.02);
			Opt->RenderDiagnosticData = 0;
			IsCamView = Master ? 0: 1;
			if (Master)
				{
				Master->SetRenderer(this);
				Master->SetPreview(0);
				Master->SetFrameText("Shadows");
				} // if
				
			// for each light that casts shadows
			for (CurLight = (Light *)EffectsBase->GetListPtr(WCS_EFFECTSSUBCLASS_LIGHT); Success && CurLight; CurLight = (Light *)CurLight->Next)
				{
				if (CurLight->Enabled && CurLight->CastShadows)
					{
					ShadowLight = CurLight;
					if (HostOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] && HostOpt->TerrainEnabled)
						{
						DefaultShadowDone = 0;
						// for each shadow effect
						for (CurShadow = (ShadowEffect *)EffectsBase->GetListPtr(WCS_EFFECTSSUBCLASS_SHADOW); Success && CurShadow; CurShadow = (ShadowEffect *)CurShadow->Next)
							{
							if (CurShadow->Enabled && CurShadow->CastShadows)
								{
								// if there are Joes a bounded shadow map is created. If no Joes a shadow map for entire DEM set is created
								// but only for the first no-Joe shadow effect.
								if (CurShadow->Joes)
									{
									// for each vector
									for (CurJoe = CurShadow->Joes; Success && CurJoe; CurJoe = CurJoe->Next)
										{
										if (CurJoe->Me && ! CurJoe->Me->TestFlags(WCS_JOEFLAG_HASKIDS) && CurJoe->Me->GetFirstRealPoint() && CurJoe->Me->TestFlags(WCS_JOEFLAG_ACTIVATED))
											{
											#ifdef WCS_VECPOLY_EFFECTS
											Success = CreateShadowMap(CurLight, WCS_EFFECTSSUBCLASS_SHADOW, CurShadow, CurJoe->Me, NULL, 0, FrameNum, CurTime);
											#else // WCS_VECPOLY_EFFECTS
											Success = CreateShadowMap(CurLight, WCS_EFFECTSSUBCLASS_SHADOW, CurShadow, CurJoe->Me, NULL, 0, FrameNum);
											#endif // WCS_VECPOLY_EFFECTS
											} // if vector enabled
										} // for each vector
									} // if shadow effect enabled
								else if (! DefaultShadowDone)
									{
									#ifdef WCS_VECPOLY_EFFECTS
									Success = CreateShadowMap(CurLight, WCS_EFFECTSSUBCLASS_SHADOW, CurShadow, NULL, NULL, 0, FrameNum, CurTime);
									#else // WCS_VECPOLY_EFFECTS
									Success = CreateShadowMap(CurLight, WCS_EFFECTSSUBCLASS_SHADOW, CurShadow, NULL, NULL, 0, FrameNum);
									#endif // WCS_VECPOLY_EFFECTS
									DefaultShadowDone = 1;
									} // else if no Joes
								} // if
							// reinitialize or no object and fence shadows in next shadow map
							EffectsBase->InitBaseToRender();
							} // for each shadow effect
						} // if terrain shadows enabled
					// if cloud shadows
					if (HostOpt->CloudShadowsEnabled && HostOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_CLOUD])
						{
						// for each cloud model
						for (CurCloud = (CloudEffect *)EffectsBase->GetListPtr(WCS_EFFECTSSUBCLASS_CLOUD); Success && CurCloud; CurCloud = (CloudEffect *)CurCloud->Next)
							{
							if (CurCloud->RenderShadowMap)	// includes enabled state
								{
								#ifdef WCS_VECPOLY_EFFECTS
								Success = CreateShadowMap(CurLight, WCS_EFFECTSSUBCLASS_CLOUD, CurCloud, NULL, NULL, 0, FrameNum, CurTime);
								#else // WCS_VECPOLY_EFFECTS
								Success = CreateShadowMap(CurLight, WCS_EFFECTSSUBCLASS_CLOUD, CurCloud, NULL, NULL, 0, FrameNum);
								#endif // WCS_VECPOLY_EFFECTS
								} // if
							} // for each cloud
						} // if
					// if 3D Object shadows
					if (HostOpt->ObjectShadowsEnabled && HostOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D])
						{
						// for each 3d object that casts shadows or is self-shadowed
						for (CurObject = (Object3DEffect *)EffectsBase->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D); Success && CurObject; CurObject = (Object3DEffect *)CurObject->Next)
							{
							if ((CurObject->Joes || CurObject->GeographicInstance || (CurObject->Search && CurObject->Search->Enabled)) && CurObject->Enabled && CurObject->CastShadows)
								{
								// for each vector
								for (CurJoe = CurObject->Joes; Success && CurJoe; CurJoe = CurJoe->Next)
									{
									if (CurJoe->Me && ! CurJoe->Me->TestFlags(WCS_JOEFLAG_HASKIDS) && CurJoe->Me->GetFirstRealPoint() && CurJoe->Me->TestFlags(WCS_JOEFLAG_ACTIVATED))
										{
										for (CurVtxNum = 1, CurVtx = CurJoe->Me->GetFirstRealPoint(); Success && CurVtx; CurVtx = CurVtx->Next, CurVtxNum ++)
											{
											#ifdef WCS_VECPOLY_EFFECTS
											Success = CreateShadowMap(CurLight, WCS_EFFECTSSUBCLASS_OBJECT3D, CurObject, CurJoe->Me, CurVtx, CurVtxNum, FrameNum, CurTime);
											#else // WCS_VECPOLY_EFFECTS
											Success = CreateShadowMap(CurLight, WCS_EFFECTSSUBCLASS_OBJECT3D, CurObject, CurJoe->Me, CurVtx, CurVtxNum, FrameNum);
											#endif // WCS_VECPOLY_EFFECTS
											} // for each vector vertex
										} // if vector enabled
									} // for each vector
								if (CurObject->Search && CurObject->Search->Enabled)
									{
									// for each vector by search query
									SearchJoe = NULL;
									for (SearchJoe = DBase->GetNextByQuery(CurObject->Search, SearchJoe); SearchJoe; SearchJoe = DBase->GetNextByQuery(CurObject->Search, SearchJoe))
										{
										if (! SearchJoe->TestFlags(WCS_JOEFLAG_HASKIDS) && SearchJoe->GetFirstRealPoint() && SearchJoe->TestFlags(WCS_JOEFLAG_ACTIVATED))
											{
											for (CurVtxNum = 1, CurVtx = SearchJoe->GetFirstRealPoint(); Success && CurVtx; CurVtx = CurVtx->Next, CurVtxNum ++)
												{
												#ifdef WCS_VECPOLY_EFFECTS
												Success = CreateShadowMap(CurLight, WCS_EFFECTSSUBCLASS_OBJECT3D, CurObject, SearchJoe, CurVtx, CurVtxNum, FrameNum, CurTime);
												#else // WCS_VECPOLY_EFFECTS
												Success = CreateShadowMap(CurLight, WCS_EFFECTSSUBCLASS_OBJECT3D, CurObject, SearchJoe, CurVtx, CurVtxNum, FrameNum);
												#endif // WCS_VECPOLY_EFFECTS
												} // for each vector vertex
											} // if vector enabled
										} // for each vector
									} // if
								if (CurObject->GeographicInstance)
									#ifdef WCS_VECPOLY_EFFECTS
									Success = CreateShadowMap(CurLight, WCS_EFFECTSSUBCLASS_OBJECT3D, CurObject, NULL, NULL, 0, FrameNum, CurTime);
									#else // WCS_VECPOLY_EFFECTS
									Success = CreateShadowMap(CurLight, WCS_EFFECTSSUBCLASS_OBJECT3D, CurObject, NULL, NULL, 0, FrameNum);
									#endif // WCS_VECPOLY_EFFECTS
								} // if
							} // for 3D Object
						} // if
					// reinitialize or no object and fence shadows in next shadow map
					EffectsBase->InitBaseToRender();
					} // if light casts shadows
				} // for each light
			if (Master)
				{
				Master->SetFrameText("");
				} // if
			} // if
		#ifdef WCS_VECPOLY_EFFECTS
		if (HostRend->DEMCue)
			DEMCue = NULL;
		if (HostRend->EvalEffects)
			EvalEffects = NULL;
		MyErrorFunctor->SetRend(HostRend);
		#endif // WCS_VECPOLY_EFFECTS
		delete Cam;
		} // if
	delete Opt;
	} // if

Changes[1] = NULL;

for (CurShadow = (ShadowEffect *)EffectsBase->GetListPtr(WCS_EFFECTSSUBCLASS_SHADOW); Success && CurShadow; CurShadow = (ShadowEffect *)CurShadow->Next)
	{
	if (CurShadow->RegenMapFile < 0)
		{
		CurShadow->RegenMapFile = 0;
		Changes[0] = MAKE_ID(CurShadow->GetNotifyClass(), CurShadow->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, CurShadow->GetRAHostRoot());
		} // if
	} // for each shadow effect
for (CurCloud = (CloudEffect *)EffectsBase->GetListPtr(WCS_EFFECTSSUBCLASS_CLOUD); CurCloud; CurCloud = (CloudEffect *)CurCloud->Next)
	{
	if (CurCloud->RegenMapFile < 0)
		{
		CurCloud->RegenMapFile = 0;
		Changes[0] = MAKE_ID(CurCloud->GetNotifyClass(), CurCloud->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, CurCloud->GetRAHostRoot());
		} // if
	} // for each cloud
for (CurObject = (Object3DEffect *)EffectsBase->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D); CurObject; CurObject = (Object3DEffect *)CurObject->Next)
	{
	if (CurObject->RegenMapFile < 0)
		{
		CurObject->RegenMapFile = 0;
		Changes[0] = MAKE_ID(CurObject->GetNotifyClass(), CurObject->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, CurObject->GetRAHostRoot());
		} // if
	} // for 3D Object

return (Success);

} // Renderer::InitForShadows

/*===========================================================================*/

#ifdef WCS_VECPOLY_EFFECTS
int Renderer::CreateShadowMap(Light *CurLight, int ObjectType, GeneralEffect *CurObj, 
	Joe *CurJoe, VectorPoint *CurVtx, long VtxNum, long FrameNum, double CurTime)
#else // WCS_VECPOLY_EFFECTS
int Renderer::CreateShadowMap(Light *CurLight, int ObjectType, GeneralEffect *CurObj, 
	Joe *CurJoe, VectorPoint *CurVtx, long VtxNum, long FrameNum)
#endif // WCS_VECPOLY_EFFECTS
{
double XLimits[2], YLimits[2], ZLimits[2], LatBounds[2], LonBounds[2], ElevBounds[2];
ShadowMap3D *ShadowMapExists1, *ShadowMapExists2;
Object3DEffect *CurObject;
ShadowEffect *CurShadow;
CloudEffect *CurCloud;
ShadowMap3D *TempMap, *TempMap2;
PolygonData Poly;
int Success = 1, NeedToRender = 1, Band, Try;
float HiElev, LowElev;

switch (ObjectType)
	{
	case WCS_EFFECTSSUBCLASS_SHADOW:
		{
		CurShadow = (ShadowEffect *)CurObj;
		// find out if there is already a shadow map for this combination of things
		ShadowMapExists1 = CurLight->MatchShadow(CurObj, CurJoe, CurVtx, WCS_SHADOWTYPE_TERRAIN);
		ShadowMapExists2 = CurLight->MatchShadow(CurObj, CurJoe, CurVtx, WCS_SHADOWTYPE_FOLIAGE);
		// find out if current shadow map is up to date
		if (ShadowMapExists1)
			{
			NeedToRender = (EffectsBase->AnimateShadows() || CurLight->AnimateShadows() || (CurLight->Distant && EarthRotation != 0.0));
			} // if
		else if (CurShadow->UseMapFile && ! CurShadow->RegenMapFile)
			{
			if (! EffectsBase->AnimateShadows() && ! CurLight->AnimateShadows() && ! (CurLight->Distant && EarthRotation != 0.0))
				{
				if (TempMap = new ShadowMap3D())
					{
					TempMap->VP.Lat = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue;
					TempMap->VP.Lon = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].CurValue;
					#ifdef WCS_LIGHT_UNCLAMPED_DISTANCE
					TempMap->VP.Elev = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].CurValue;
					#else // WCS_LIGHT_UNCLAMPED_DISTANCE
					TempMap->VP.Elev = min(CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].CurValue, 100000000.0);
					#endif // WCS_LIGHT_UNCLAMPED_DISTANCE
					// loading shadow map compares XYZ of light to the one stored with the shadow map
					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&TempMap->VP);
					#else // WCS_BUILD_VNS
					TempMap->VP.DegToCart(PlanetRad);
					#endif // WCS_BUILD_VNS
					if (CurLight->AttemptLoadShadowMapFile(TempMap, CurShadow->ShadowMapWidth, CurShadow, CurJoe, CurVtx, VtxNum, WCS_SHADOWTYPE_TERRAIN)
						&& CurLight->AttemptLoadShadowMapFile(TempMap, CurShadow->ShadowMapWidth, CurShadow, CurJoe, CurVtx, VtxNum, WCS_SHADOWTYPE_FOLIAGE))
						NeedToRender = 0;
					delete TempMap;
					} // if
				} // if
			} // else if
		if (NeedToRender)
			{
			if ((TempMap = new ShadowMap3D()) && (TempMap2 = new ShadowMap3D()) && (TempMap2->Rast = new Raster()))
				{
				// remove old shadow map if obsolete
				if (ShadowMapExists1)
					CurLight->RemoveShadowMap(ShadowMapExists1);
				if (ShadowMapExists2)
					CurLight->RemoveShadowMap(ShadowMapExists2);

				// set render options as needed
				Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].SetValue(1.0);
				Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_SIDEOVERSCAN].SetValue(0.0);
				Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_BOTTOMOVERSCAN].SetValue(0.0);
				ShadowMinFolHt = CurShadow->AnimPar[WCS_EFFECTS_SHADOW_ANIMPAR_MINFOLHT].CurValue;
				
				// set shadow map width in render opt
				SetupWidth = Width = Opt->OutputImageWidth = CurShadow->ShadowMapWidth;

				// give map an arbitrary height for now
				SetupHeight = Height = Opt->OutputImageHeight = Opt->OutputImageWidth;

				// init Render Data
				RendData.InitFrameToRender(this);

				// set light position in camera
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetValue(CurLight->LightPos->Lat);
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetValue(CurLight->LightPos->Lon);
				#ifdef WCS_LIGHT_UNCLAMPED_DISTANCE
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetValue(CurLight->LightPos->Elev);
				#else // WCS_LIGHT_UNCLAMPED_DISTANCE
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetValue(min(CurLight->LightPos->Elev, 100000000.0));
				#endif // WCS_LIGHT_UNCLAMPED_DISTANCE

				// set camera target
				Cam->TargetObj = NULL;
				if (CurJoe)
					{
					LatBounds[0] = CurJoe->GetNorth();
					LatBounds[1] = CurJoe->GetSouth();
					LonBounds[0] = CurJoe->GetWest();
					LonBounds[1] = CurJoe->GetEast();
					CurJoe->GetElevRange(HiElev, LowElev);
					} // if
				else
					{
					DBase->GetDEMExtents(LatBounds[0], LatBounds[1], LonBounds[0], LonBounds[1]);
					DBase->GetDEMElevRange(HiElev, LowElev);
					} // else
				ElevBounds[0] = HiElev;
				ElevBounds[1] = LowElev;
				if (LatBounds[0] == LatBounds[1] || LonBounds[0] == LonBounds[1])
					{
					delete TempMap;
					delete TempMap2;
					break;
					} // if no coverage area
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].SetValue(TempMap->RefPos.Lat = TempMap2->RefPos.Lat = ((LatBounds[0] + LatBounds[1]) * .5)); // Optimize out divide. Was / 2.0
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].SetValue(TempMap->RefPos.Lon = TempMap2->RefPos.Lon = ((LonBounds[0] + LonBounds[1]) * .5));// Optimize out divide. Was / 2.0
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].SetValue(TempMap->RefPos.Elev = TempMap2->RefPos.Elev = ((ElevBounds[0] + ElevBounds[1]) * .5));// Optimize out divide. Was / 2.0
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetValue(90.0);

				// initialize camera
				Cam->InitToRender(Opt, NULL);
				Cam->CenterX = .5 * Width;
				Cam->CenterY = .5 * Height;
				Cam->InitFrameToRender(EffectsBase, &RendData);

				// set reference coords to the vector's center position in the shadow map.
				#ifdef WCS_BUILD_VNS
				DefCoords->DegToCart(&TempMap->RefPos);
				DefCoords->DegToCart(&TempMap2->RefPos);
				#else // WCS_BUILD_VNS
				TempMap->VP.DegToCart(PlanetRad);
				TempMap2->VP.DegToCart(PlanetRad);
				#endif // WCS_BUILD_VNS

				// project object bounds
				if (CurShadow->ProjectBounds(Cam, &RendData, CurJoe, LatBounds, LonBounds, ElevBounds, XLimits, YLimits, ZLimits))
					{
					Try = 0;
					while ((fabs(YLimits[0] - YLimits[1]) < 50.0 || fabs(XLimits[0] - XLimits[1]) < 50.0) && Try < 100)
						{
						Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue *= 0.5;
						Cam->InitFrameToRender(EffectsBase, &RendData);
						CurShadow->ProjectBounds(Cam, &RendData, CurJoe, LatBounds, LonBounds, ElevBounds, XLimits, YLimits, ZLimits);
						Try ++;
						} // while
					if (Try < 100)
						{
						// determine z offset for shadow mapping
						TempMap->ZOffset = TempMap2->ZOffset = ShadowMapDistanceOffset = RendData.ShadowMapDistanceOffset = ZLimits[1];
						// compute correct image height
						SetupHeight = Height = Opt->OutputImageHeight = (long)(Width * (YLimits[0] - YLimits[1]) / (XLimits[0] - XLimits[1])) + 1;
						Cam->CenterY = .5 * Height;
						// adjust camera projection scales, center X and Y
						Cam->HorScale *= (Width - 1) / (XLimits[0] - XLimits[1]);
						Cam->VertScale = Cam->HorScale;
						CurShadow->ProjectBounds(Cam, &RendData, CurJoe, LatBounds, LonBounds, ElevBounds, XLimits, YLimits, ZLimits);

						// adjust camera center X and Y
						Cam->CenterX += Cam->CenterX - (XLimits[0] + XLimits[1]) * .5; // Optimize out divide. Was / 2.0
						Cam->CenterY += Cam->CenterY - (YLimits[0] + YLimits[1]) * .5; // Optimize out divide. Was / 2.0

						// for testing
						//CurShadow->ProjectBounds(Cam, &RendData, CurJoe, LatBounds, LonBounds, ElevBounds, XLimits, YLimits, ZLimits);

						if (Success = SetImageSize(FALSE))
							{
							if (Success = AddBasicBufferNodes())
								{
								#ifdef WCS_TEST_EARLY_TERRAIN_INIT
								if (InitTerrainFactors())
								#endif // WCS_TEST_EARLY_TERRAIN_INIT
									{
									if (Success = AllocateBuffers())
										{
										if (Success = InitOncePerRender())	// not really necessary for 3d objects but essential for terrain
											{
											// set some odds and ends that ordinarily would be set in InitFrame()
											RendData.TexData.MetersPerDegLat = EarthLatScaleMeters;
											RendData.TexData.Datum = ElevDatum;
											RendData.TexData.Exageration = Exageration;
											QMax = PlanetRad + Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue;
											QMax = sqrt(QMax * QMax - PlanetRad * PlanetRad);
											SetDrawOffsets();
											GetTime(FirstSecs);	// in Useful.cpp
											SetMultiBuf(1);
											if (Master && Master->GetPreview())
												OpenPreview();
											#ifdef WCS_VECPOLY_EFFECTS
											if (Success = RenderShadow(FrameNum, CurTime))
											#else // WCS_VECPOLY_EFFECTS
											if (Success = RenderShadow(FrameNum))
											#endif // WCS_VECPOLY_EFFECTS
												{
												SetMultiBuf(0);
												// copy settings and buffers
												TempMap->MyEffect = TempMap2->MyEffect = CurShadow;
												TempMap->MyType = WCS_SHADOWTYPE_TERRAIN;
												TempMap2->MyType = WCS_SHADOWTYPE_FOLIAGE;
												TempMap->MyJoe = TempMap2->MyJoe = CurJoe;
												TempMap->MyVertex = TempMap2->MyVertex = CurVtx;
												TempMap->CenterX = TempMap2->CenterX = Cam->CenterX;
												TempMap->CenterY = TempMap2->CenterY = Cam->CenterY;
												TempMap->HorScale = TempMap2->HorScale = Cam->HorScale;
												TempMap->DistanceOffset = TempMap2->DistanceOffset = CurLight->LightPos->Elev - Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue;
												CopyMatrix3x3(TempMap->SolarRotMatx, Cam->ScrRotMatx);
												CopyMatrix3x3(TempMap2->SolarRotMatx, Cam->ScrRotMatx);
												TempMap->VP.CopyLatLon(Cam->CamPos);
												TempMap2->VP.CopyLatLon(Cam->CamPos);
												TempMap->VP.CopyXYZ(Cam->CamPos);
												TempMap2->VP.CopyXYZ(Cam->CamPos);
												TempMap->AABuf = (unsigned char *)Buffers->FindBuffer("ANTIALIAS", WCS_RASTER_BANDSET_BYTE);
												TempMap2->AABuf = (unsigned char *)Buffers->FindBuffer("FOLIAGE ANTIALIAS", WCS_RASTER_BANDSET_BYTE);
												TempMap->ZBuf = (float *)Buffers->FindBuffer("ZBUF", WCS_RASTER_BANDSET_FLOAT);
												TempMap2->ZBuf = (float *)Buffers->FindBuffer("FOLIAGE ZBUF", WCS_RASTER_BANDSET_FLOAT);
												TempMap2->Rast->Rows = Rast->Rows;
												TempMap2->Rast->Cols = Rast->Cols;
												TempMap2->Rast->ByteBandSize = Rast->ByteBandSize;
												TempMap2->Rast->FloatBandSize = Rast->FloatBandSize;
												TempMap2->Rast->ByteMap[0] = TempMap2->AABuf;
												TempMap2->Rast->FloatMap[0] = TempMap2->ZBuf;
												// delete all other buffers
												Rast->FreeAltByteBand();
												Rast->FreeAltFloatBand();
												for (Band = 0; Band < WCS_RASTER_MAX_BANDS; Band ++)
													{
													if (Rast->ByteMap[Band])
														{
														if (Rast->ByteMap[Band] != TempMap->AABuf && Rast->ByteMap[Band] != TempMap2->AABuf)
															{
															Rast->FreeByteBand(Band);
															} // if
														else if (Rast->ByteMap[Band] == TempMap2->AABuf)
															{
															Rast->ByteMap[Band] = NULL;
															} // else if
														} // if
													if (Rast->FloatMap[Band])
														{
														if (Rast->FloatMap[Band] != TempMap->ZBuf && Rast->FloatMap[Band] != TempMap2->ZBuf)
															{
															Rast->FreeFloatBand(Band);
															} // if
														else if (Rast->FloatMap[Band] == TempMap2->ZBuf)
															{
															Rast->FloatMap[Band] = NULL;
															} // else if
														} // if
													} // for
												// assign raster to shadow map
												TempMap->Rast = Rast;
												// NULL raster so it isn't deleted in cleanup
												Rast = NULL;

												// link shadow map to light
												CurLight->AddShadowMap(TempMap);
												CurLight->AddShadowMap(TempMap2);
												// cancel future regeneration
												CurShadow->RegenMapFile = -1;
												// save file
												if (CurShadow->UseMapFile)
													{
													// save shadow maps
													CurLight->SaveShadowMapFile(TempMap, CurShadow, CurJoe, CurVtx, VtxNum, WCS_SHADOWTYPE_TERRAIN);
													CurLight->SaveShadowMapFile(TempMap2, CurShadow, CurJoe, CurVtx, VtxNum, WCS_SHADOWTYPE_FOLIAGE);
													} // if
												} // if
											} // if
										} // if
									} // if
								} // if
							} // if
						} // if
					else
						{
						Success = 0;
						UserMessageOK("Terrain Shadow", "Unable to initialize shadow map. Shaded region too small or too far from light.");
						} // else
					} // if
				else
					{
					Success = 0;
					UserMessageOK("3D Object Shadow", "Unable to initialize shadow map. Shaded region too large or too close to light.");
					} // else
				if (! Success)
					{
					delete TempMap;
					delete TempMap2;
					} // if
				} // if
			Cleanup(false, true, 1, false);
			} // if need to render
		break;
		} // WCS_EFFECTSSUBCLASS_SHADOW
	case WCS_EFFECTSSUBCLASS_CLOUD:
		{
		CurCloud = (CloudEffect *)CurObj;
		// find out if there is already a shadow map for this combination of things
		ShadowMapExists1 = CurLight->MatchShadow(CurObj, CurJoe, CurVtx, WCS_SHADOWTYPE_CLOUDSM);
		// find out if current shadow map is up to date
		if (ShadowMapExists1)
			{
			NeedToRender = (CurCloud->AnimateCloudShadows() || CurLight->AnimateShadows() || (CurLight->Distant && EarthRotation != 0.0));
			} // if
		else if (CurCloud->UseMapFile && ! CurCloud->RegenMapFile)
			{
			if (! CurCloud->AnimateCloudShadows() && ! CurLight->AnimateShadows() && ! (CurLight->Distant && EarthRotation != 0.0))
				{
				if (TempMap = new ShadowMap3D())
					{
					TempMap->VP.Lat = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue;
					TempMap->VP.Lon = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].CurValue;
					#ifdef WCS_LIGHT_UNCLAMPED_DISTANCE
					TempMap->VP.Elev = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].CurValue;
					#else // WCS_LIGHT_UNCLAMPED_DISTANCE
					TempMap->VP.Elev = min(CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].CurValue, 100000000.0);
					#endif // WCS_LIGHT_UNCLAMPED_DISTANCE
					// loading shadow map compares XYZ of light to the one stored with the shadow map
					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&TempMap->VP);
					#else // WCS_BUILD_VNS
					TempMap->VP.DegToCart(PlanetRad);
					#endif // WCS_BUILD_VNS
					if (CurLight->AttemptLoadShadowMapFile(TempMap, CurCloud->ShadowMapWidth, CurCloud, CurJoe, CurVtx, VtxNum, WCS_SHADOWTYPE_CLOUDSM))
						NeedToRender = 0;
					delete TempMap;
					} // if
				} // if
			} // else if
		if (NeedToRender)
			{
			if (TempMap = new ShadowMap3D())
				{
				// remove old shadow map if obsolete
				if (ShadowMapExists1)
					CurLight->RemoveShadowMap(ShadowMapExists1);

				// set render options as needed
				Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].SetValue(1.0);
				Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_SIDEOVERSCAN].SetValue(0.0);
				Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_BOTTOMOVERSCAN].SetValue(0.0);
				
				// set shadow map width in render opt
				SetupWidth = Width = Opt->OutputImageWidth = CurCloud->ShadowMapWidth;

				// give map an arbitrary height for now
				SetupHeight = Height = Opt->OutputImageHeight = Opt->OutputImageWidth;

				// init Render Data
				RendData.InitFrameToRender(this);

				// set light position in camera
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetValue(CurLight->LightPos->Lat);
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetValue(CurLight->LightPos->Lon);
				#ifdef WCS_LIGHT_UNCLAMPED_DISTANCE
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetValue(CurLight->LightPos->Elev);
				#else // WCS_LIGHT_UNCLAMPED_DISTANCE
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetValue(min(CurLight->LightPos->Elev, 100000000.0));
				#endif // WCS_LIGHT_UNCLAMPED_DISTANCE

				// set camera target to center of cloud map
				Cam->TargetObj = NULL;
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].SetValue(TempMap->RefPos.Lat = CurCloud->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLAT].CurValue);
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].SetValue(TempMap->RefPos.Lon = CurCloud->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_CENTERLON].CurValue);
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].SetValue(TempMap->RefPos.Elev = CurCloud->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BASEELEV].CurValue);
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetValue(120.0);

				// initialize camera
				Cam->InitToRender(Opt, NULL);
				Cam->CenterX = .5 * Width;
				Cam->CenterY = .5 * Height;
				Cam->InitFrameToRender(EffectsBase, &RendData);

				// set reference coords to the vector's center position in the shadow map.
				#ifdef WCS_BUILD_VNS
				DefCoords->DegToCart(&TempMap->RefPos);
				#else // WCS_BUILD_VNS
				TempMap->RefPos.DegToCart(PlanetRad);
				#endif // WCS_BUILD_VNS

				// project object bounds
				if (CurCloud->ProjectBounds(Cam, &RendData, XLimits, YLimits, ZLimits))
					{
					Try = 0;
					while ((fabs(YLimits[0] - YLimits[1]) < 50.0 || fabs(XLimits[0] - XLimits[1]) < 50.0) && Try < 100)
						{
						Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue *= 0.5;
						Cam->InitFrameToRender(EffectsBase, &RendData);
						CurCloud->ProjectBounds(Cam, &RendData, XLimits, YLimits, ZLimits);
						Try ++;
						} // while
					if (Try < 100)
						{
						// determine z offset for shadow mapping
						TempMap->ZOffset = ShadowMapDistanceOffset = RendData.ShadowMapDistanceOffset = ZLimits[1];
						// compute correct image height
						SetupHeight = Height = Opt->OutputImageHeight = (long)(Width * (YLimits[0] - YLimits[1]) / (XLimits[0] - XLimits[1])) + 1;
						Cam->CenterY = .5 * Height;
						// adjust camera projection scales, center X and Y
						Cam->HorScale *= (Width - 1) / (XLimits[0] - XLimits[1]);
						Cam->VertScale = Cam->HorScale;
						CurCloud->ProjectBounds(Cam, &RendData, XLimits, YLimits, ZLimits);

						// adjust camera center X and Y
						Cam->CenterX += Cam->CenterX - (XLimits[0] + XLimits[1]) * .5; // Optimize out divide. Was / 2.0
						Cam->CenterY += Cam->CenterY - (YLimits[0] + YLimits[1]) * .5; // Optimize out divide. Was / 2.0

						// for testing
						//CurCloud->ProjectBounds(Cam, &RendData, XLimits, YLimits, ZLimits);

						if (Success = SetImageSize(FALSE))
							{
							if (Success = AddBasicBufferNodes())
								{
								#ifdef WCS_TEST_EARLY_TERRAIN_INIT
								if (InitTerrainFactors())
								#endif // WCS_TEST_EARLY_TERRAIN_INIT
									{
									if (Success = AllocateBuffers())
										{
										if (Success = InitOncePerRender())	// not really necessary for 3d objects but essential for terrain
											{
											// set some odds and ends that ordinarily would be set in InitFrame()
											RendData.TexData.MetersPerDegLat = EarthLatScaleMeters;
											RendData.TexData.Datum = ElevDatum;
											RendData.TexData.Exageration = Exageration;
											SetDrawOffsets();
											GetTime(FirstSecs);	// in Useful.cpp
											SetMultiBuf(1);
											if (Master && Master->GetPreview())
												OpenPreview();
											if (Success = RenderCloudShadow(CurCloud))
												{
												SetMultiBuf(0);
												// copy settings and buffers
												TempMap->MyEffect = CurCloud;
												TempMap->MyType = WCS_SHADOWTYPE_CLOUDSM;
												TempMap->MyJoe = CurJoe;
												TempMap->MyVertex = CurVtx;
												TempMap->CenterX = Cam->CenterX;
												TempMap->CenterY = Cam->CenterY;
												TempMap->HorScale = Cam->HorScale;
												TempMap->DistanceOffset = CurLight->LightPos->Elev - Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue;
												CopyMatrix3x3(TempMap->SolarRotMatx, Cam->ScrRotMatx);
												TempMap->VP.CopyLatLon(Cam->CamPos);
												TempMap->VP.CopyXYZ(Cam->CamPos);
												TempMap->AABuf = (unsigned char *)Buffers->FindBuffer("FOLIAGE ANTIALIAS", WCS_RASTER_BANDSET_BYTE);
												TempMap->ZBuf = (float *)Buffers->FindBuffer("FOLIAGE ZBUF", WCS_RASTER_BANDSET_FLOAT);
												// delete all other buffers
												Rast->FreeAltByteBand();
												Rast->FreeAltFloatBand();
												for (Band = 0; Band < WCS_RASTER_MAX_BANDS; Band ++)
													{
													if (Rast->ByteMap[Band])
														{
														if (Rast->ByteMap[Band] != TempMap->AABuf)
															{
															Rast->FreeByteBand(Band);
															} // if
														} // if
													if (Rast->FloatMap[Band])
														{
														if (Rast->FloatMap[Band] != TempMap->ZBuf)
															{
															Rast->FreeFloatBand(Band);
															} // if
														} // if
													} // for
												// assign raster to shadow map
												TempMap->Rast = Rast;
												// NULL raster so it isn't deleted in cleanup
												Rast = NULL;

												// link shadow map to light
												CurLight->AddShadowMap(TempMap);
												// cancel future regeneration
												CurCloud->RegenMapFile = -1;
												// save file
												if (CurCloud->UseMapFile)
													{
													// save shadow map
													CurLight->SaveShadowMapFile(TempMap, CurCloud, CurJoe, CurVtx, VtxNum, WCS_SHADOWTYPE_CLOUDSM);
													} // if
												} // if
											} // if
										} // if
									} // if
								} // if
							} // if
						} // if
					else
						{
						Success = 0;
						UserMessageOK("Cloud Shadow", "Unable to initialize shadow map. Shaded cloud model too small or too far from light.");
						} // else
					} // if
				else
					{
					Success = 0;
					UserMessageOK("3D Object Shadow", "Unable to initialize shadow map. Shaded cloud model too large or too close to light.");
					} // else
				if (! Success)
					{
					delete TempMap;
					} // if
				} // if
			Cleanup(false, true, 1, false);
			} // if need to render
		break;
		} // WCS_EFFECTSSUBCLASS_CLOUD
	case WCS_EFFECTSSUBCLASS_OBJECT3D:
		{
		CurObject = (Object3DEffect *)CurObj;
		ShadowMapExists1 = CurLight->MatchShadow(CurObj, CurJoe, CurVtx, WCS_SHADOWTYPE_3DOBJECT);
		// find out if current shadow map is up to date
		if (ShadowMapExists1)
			{
			NeedToRender = (CurObject->AnimateShadow3D() || CurLight->AnimateShadows() || (CurLight->Distant && EarthRotation != 0.0));
			} // if
		else if (CurObject->UseMapFile && ! CurObject->RegenMapFile)
			{
			if (! CurObject->AnimateShadow3D() && ! CurLight->AnimateShadows() && ! (CurLight->Distant && EarthRotation != 0.0))
				{
				if (TempMap = new ShadowMap3D())
					{
					TempMap->VP.Lat = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue;
					TempMap->VP.Lon = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].CurValue;
					#ifdef WCS_LIGHT_UNCLAMPED_DISTANCE
					TempMap->VP.Elev = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].CurValue;
					#else // WCS_LIGHT_UNCLAMPED_DISTANCE
					TempMap->VP.Elev = min(CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].CurValue, 100000000.0);
					#endif // WCS_LIGHT_UNCLAMPED_DISTANCE
					// loading shadow map compares XYZ of light to the one stored with the shadow map
					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&TempMap->VP);
					#else // WCS_BUILD_VNS
					TempMap->VP.DegToCart(PlanetRad);
					#endif // WCS_BUILD_VNS
					if (CurLight->AttemptLoadShadowMapFile(TempMap, CurObject->ShadowMapWidth, CurObject, CurJoe, CurVtx, VtxNum, WCS_SHADOWTYPE_3DOBJECT))
						NeedToRender = 0;
					delete TempMap;
					} // if
				} // if
			} // else if
		if (NeedToRender)
			{
			if (TempMap = new ShadowMap3D())
				{
				// remove old shadow map if obsolete
				if (ShadowMapExists1)
					CurLight->RemoveShadowMap(ShadowMapExists1);

				// set render options as needed
				Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].SetValue(1.0);
				Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_SIDEOVERSCAN].SetValue(0.0);
				Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_BOTTOMOVERSCAN].SetValue(0.0);

				// set shadow map width in render opt
				SetupWidth = Width = Opt->OutputImageWidth = CurObject->ShadowMapWidth;

				// give map an arbitrary height for now
				SetupHeight = Height = Opt->OutputImageHeight = Opt->OutputImageWidth;

				// init Render Data
				RendData.InitFrameToRender(this);

				// set light position in camera
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].SetValue(CurLight->LightPos->Lat);
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].SetValue(CurLight->LightPos->Lon);
				#ifdef WCS_LIGHT_UNCLAMPED_DISTANCE
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetValue(CurLight->LightPos->Elev);
				#else // WCS_LIGHT_UNCLAMPED_DISTANCE
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].SetValue(min(CurLight->LightPos->Elev, 100000000.0));
				#endif // WCS_LIGHT_UNCLAMPED_DISTANCE

				// set camera target
				// set reference coords to the object's ground position in the shadow map (actual position may vary).
				// Also fill in object's base latitude, longitude and elevation in passing
				CurObject->FindBasePosition(&RendData, &TempMap->RefPos, &Poly, CurJoe, CurVtx);
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].SetValue(TempMap->RefPos.Lat);
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].SetValue(TempMap->RefPos.Lon);
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].SetValue(TempMap->RefPos.Elev);

				Cam->TargetObj = NULL;
				Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetValue(90.0);

				// initialize camera
				Cam->InitToRender(Opt, NULL);
				Cam->CenterX = .5 * Width;
				Cam->CenterY = .5 * Height;
				Cam->InitFrameToRender(EffectsBase, &RendData);

				// project object bounds
				if (CurObject->ProjectBounds(Cam, &RendData, &TempMap->RefPos, &Poly, XLimits, YLimits, ZLimits))
					{
					Try = 0;
					while ((fabs(YLimits[0] - YLimits[1]) < 50.0 || fabs(XLimits[0] - XLimits[1]) < 50.0) && Try < 100)
						{
						Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue *= 0.5;
						Cam->InitFrameToRender(EffectsBase, &RendData);
						CurObject->ProjectBounds(Cam, &RendData, &TempMap->RefPos, &Poly, XLimits, YLimits, ZLimits);
						Try ++;
						} // while
					if (Try < 100)
						{
						// determine z offset for shadow mapping
						TempMap->ZOffset = ShadowMapDistanceOffset = RendData.ShadowMapDistanceOffset = ZLimits[1];
						// compute correct image height
						SetupHeight = Height = Opt->OutputImageHeight = (long)(Width * (YLimits[0] - YLimits[1]) / (XLimits[0] - XLimits[1])) + 1;
						Cam->CenterY = .5 * Height;
						// adjust camera projection scales, center X and Y
						Cam->HorScale *= (Width - 1) / (XLimits[0] - XLimits[1]);
						Cam->VertScale = Cam->HorScale;
						CurObject->ProjectBounds(Cam, &RendData, &TempMap->RefPos, &Poly, XLimits, YLimits, ZLimits);

						// adjust camera center X and Y
						Cam->CenterX += Cam->CenterX - (XLimits[0] + XLimits[1]) * .5; // Optimize out divide. Was / 2.0
						Cam->CenterY += Cam->CenterY - (YLimits[0] + YLimits[1]) * .5; // Optimize out divide. Was / 2.0

						// for testing
						//CurObject->ProjectBounds(Cam, &RendData, &TempMap->RefPos, XLimits, YLimits, ZLimits);

						if (Success = SetImageSize(FALSE))
							{
							if (Success = AddBasicBufferNodes())
								{
								#ifdef WCS_TEST_EARLY_TERRAIN_INIT
								if (InitTerrainFactors())
								#endif // WCS_TEST_EARLY_TERRAIN_INIT
									{
									if (Success = AllocateBuffers())
										{
										if (Success = InitOncePerRender())	// not really necessary for 3d objects but essential for terrain
											{
											SetDrawOffsets();
											GetTime(FirstSecs);	// in Useful.cpp
											SetMultiBuf(1);
											if (Master && Master->GetPreview())
												OpenPreview();
											if (Success = Render3DObject(&Poly, &TempMap->RefPos, CurObject, NULL, -1.0))
												{
												MergeZBufRGBPanels(Bitmap, ZBuf, AABuf, TreeBitmap, TreeZBuf, TreeAABuf);
												SetMultiBuf(0);
												// copy settings and buffers
												TempMap->MyType = WCS_SHADOWTYPE_3DOBJECT;
												TempMap->MyEffect = CurObject;
												TempMap->MyJoe = CurJoe;
												TempMap->MyVertex = CurVtx;
												TempMap->CenterX = Cam->CenterX;
												TempMap->CenterY = Cam->CenterY;
												TempMap->HorScale = Cam->HorScale;
												TempMap->DistanceOffset = CurLight->LightPos->Elev - Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue;
												CopyMatrix3x3(TempMap->SolarRotMatx, Cam->ScrRotMatx);
												TempMap->VP.CopyLatLon(Cam->CamPos);
												TempMap->VP.CopyXYZ(Cam->CamPos);
												TempMap->AABuf = (unsigned char *)Buffers->FindBuffer("ANTIALIAS", WCS_RASTER_BANDSET_BYTE);
												TempMap->ZBuf = (float *)Buffers->FindBuffer("ZBUF", WCS_RASTER_BANDSET_FLOAT);
												// delete all other buffers
												for (Band = 0; Band < WCS_RASTER_MAX_BANDS; Band ++)
													{
													if (Rast->ByteMap[Band] && Rast->ByteMap[Band] != TempMap->AABuf)
														{
														Rast->FreeByteBand(Band);
														} // if
													if (Rast->FloatMap[Band] && Rast->FloatMap[Band] != TempMap->ZBuf)
														{
														Rast->FreeFloatBand(Band);
														} // if
													} // for
												Rast->FreeAltByteBand();
												Rast->FreeAltFloatBand();
												// assign raster to shadow map
												TempMap->Rast = Rast;
												// NULL raster so it isn't deleted in cleanup
												Rast = NULL;

												// link shadow map to light
												CurLight->AddShadowMap(TempMap);
												// cancel future regeneration
												CurObject->RegenMapFile = -1;
												// save file
												if (CurObject->UseMapFile)
													{
													// save shadow map
													CurLight->SaveShadowMapFile(TempMap, CurObject, CurJoe, CurVtx, VtxNum, WCS_SHADOWTYPE_3DOBJECT);
													} // if
												} // if
											} // if
										} // if
									} // if
								} // if
							} // if
						} // if
					else
						{
						Success = 0;
						UserMessageOK("3D Object Shadow", "Unable to initialize shadow map. Shaded object too small or too far from light.");
						} // else
					} // if
				else
					{
					Success = 0;
					UserMessageOK("3D Object Shadow", "Unable to initialize shadow map. Shaded object too large or too close to light.");
					} // else
				if (! Success)
					delete TempMap;
				} // if
			Cleanup(false, true, 1, false);
			} // if need to render
		break;
		} // WCS_EFFECTSSUBCLASS_OBJECT3D
	} // switch

return (Success);

} // Renderer::CreateShadowMap

/*===========================================================================*/

#ifdef WCS_VECPOLY_EFFECTS
int Renderer::RenderShadow(long FrameNum, double CurTime)
#else // WCS_VECPOLY_EFFECTS
int Renderer::RenderShadow(long FrameNum)
#endif // WCS_VECPOLY_EFFECTS
{
int Success = 1;

GetTime(StartSecs);
GetTime(FrameSecs);

// load foliage
if (Opt->FoliageEnabled || Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE])
	{
	Success = ImageBase->InitFoliageRasters(RenderTime, Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue, 
		Opt->FoliageEnabled, Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE],
		Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM], Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE], this);
	} // if
// render terrain, water, foliage
if (Success)
	{
	#ifdef WCS_VECPOLY_EFFECTS
	Success = RenderTerrain(FrameNum, 0, CurTime);
	#else // WCS_VECPOLY_EFFECTS
	Success = RenderTerrain(FrameNum, 0);
	#endif // WCS_VECPOLY_EFFECTS
	} // if
// clear short-longevity foliage from memory
ImageBase->ClearImageRasters(WCS_RASTER_LONGEVITY_SHORT);

return (Success);

} // Renderer::RenderShadow

/*===========================================================================*/

int Renderer::RenderCloudShadow(CloudEffect *CurCloud)
{
int Success;

GetTime(StartSecs);
GetTime(FrameSecs);

// render clouds
Success = RenderClouds(CurCloud);

return (Success);

} // Renderer::RenderCloudShadow

/*===========================================================================*/

ShadowMap3D::ShadowMap3D()
{

ZeroMatrix3x3(SolarRotMatx);
CenterX = CenterY = 0.0;
HorScale = 1.0;
MyType = 0;
Rast = NULL;
ZOffset = DistanceOffset = 0.0;
Next = NULL;
MyJoe = NULL;
MyVertex = NULL;
MyEffect = NULL;
AABuf = NULL;
ZBuf = NULL;

} // ShadowMap3D::ShadowMap3D

/*===========================================================================*/

ShadowMap3D::~ShadowMap3D()
{

if (Rast)
	delete Rast;
Rast = NULL;

} // ShadowMap3D::~ShadowMap3D

/*===========================================================================*/

int ShadowMap3D::ProjectPoint(double PtXYZ[3], double ScrnCoords[3])
{
double InvScrnTwo;

FindPosVector(ScrnCoords, PtXYZ, VP.XYZ);
RotatePoint(ScrnCoords, SolarRotMatx);

// if perspective divide by Z
InvScrnTwo = 1.0 / ScrnCoords[2];
ScrnCoords[0] = CenterX + HorScale * ScrnCoords[0] * InvScrnTwo;
ScrnCoords[1] = CenterY - HorScale * ScrnCoords[1] * InvScrnTwo;
ScrnCoords[2] -= ZOffset;

if (ScrnCoords[0] >= 0 && ScrnCoords[0] < Rast->Cols && ScrnCoords[1] >= 0 && ScrnCoords[1] < Rast->Rows)
	{
	return (1);
	} // if

return (0);

} // ShadowMap3D::ProjectPoint

/*===========================================================================*/

double ShadowMap3D::SampleWeighted(long SampleCell, double X, double Y)
{
double InterpVal = 0.0, XOff, YOff, XInvOff, YInvOff, wt[4], val[4];
long Row, Col;

Col = quickftol(X);
Row = quickftol(Y);

XOff = X - Col;
YOff = Y - Row;

XInvOff = 1.0 - XOff;
YInvOff = 1.0 - YOff;

if (Col >= Rast->Cols - 1)
	{
	if (Row >= Rast->Rows)
		{
		InterpVal = AABuf[SampleCell];
		}
	else
		{
		wt[0] = YInvOff;
		val[0] = AABuf[SampleCell];
		wt[1] = YOff;
		val[1] = AABuf[SampleCell + Rast->Cols];
		InterpVal = (wt[0] * val[0] + wt[1] * val[1]);
		} // else
	} // if
else if (Row >= Rast->Rows - 1)
	{
	wt[0] = XInvOff;
	val[0] = AABuf[SampleCell];
	wt[1] = XOff;
	val[1] = AABuf[SampleCell + 1];
	InterpVal = (wt[0] * val[0] + wt[1] * val[1]);
	} // else if
else
	{
	wt[0] = YInvOff * XInvOff;
	val[0] = AABuf[SampleCell];
	wt[1] = YOff * XInvOff;
	val[1] = AABuf[SampleCell + Rast->Cols];
	wt[2] = YOff * XOff;
	val[2] = AABuf[SampleCell + Rast->Cols + 1];
	wt[3] = XOff * YInvOff;
	val[3] = AABuf[SampleCell + 1];
	InterpVal = (wt[0] * val[0] + wt[1] * val[1] + wt[2] * val[2] + wt[3] * val[3]);
	} // else

return (InterpVal);

} // ShadowMap3D::SampleWeighted

/*===========================================================================*/
