// RenderSky.cpp
// Code for rendering Skies, clouds, celestial objects in World Construction Set.
// Built from scratch 10/27/99 by Gary R. Huber.
// Copyright 1999 by Questar Productions. All rights reserved.

#include "stdafx.h"
#include "Render.h"
#include "EffectsLib.h"
#include "Requester.h"
#include "RenderControlGUI.h"
#include "Useful.h"
#include "Raster.h"
#include "AppMem.h"
#include "PixelManager.h"
#include "Lists.h"

#ifdef WCS_BUILD_DEMO
extern unsigned long MemFuck;
#endif // WCS_BUILD_DEMO

int Renderer::RenderSky(void)
{
double R, Rsq, C, C4;
Light *FirstLight, *CurLight;
Sky *FirstSky, *CurSky;
BusyWin *BWDE = NULL;
long X, Y, RowZip, PixZip;
int Success = 1;
PixelData Pix;

// no sky available for planimetric cameras, can't convert screen coords into world cartesian
if (Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
	return (1);

if (! DefCoords->Initialized)
	DefCoords->Initialize();
if (! DefCoords->Initialized)
	return (0);
	
FirstLight = (Light *)EffectsBase->GetListPtr(WCS_EFFECTSSUBCLASS_LIGHT);
FirstSky = (Sky *)EffectsBase->GetListPtr(WCS_EFFECTSSUBCLASS_SKY);
Pix.SetDefaults();

while (FirstSky)
	{
	if (FirstSky->Enabled)
		break;
	FirstSky = (Sky *)FirstSky->Next;
	} // while

if (! FirstSky)
	return (1);	// no enabled skies so nothing to do

// advance to the first enabled distant light
while (FirstLight)
	{
	if (FirstLight->Enabled && FirstLight->Distant)
		break;
	FirstLight = (Light *)FirstLight->Next;
	} // while

// seed random number generators
for (CurSky = FirstSky, X = 0; CurSky; CurSky = (Sky *)CurSky->Next, X += 17)
	{
	if (CurSky->Enabled)
		{
		if (CurSky->DitherTable || (CurSky->DitherTable = new double[Width]))
			{
			CurSky->Rand.Seed64(12345 + X, 54321 - X);
			} // if
		else
			return (0);
		} // if
	} // for

R = 4.0E5 + PlanetRad;
Rsq = R * R;	// this is the radius of the sky sphere squared
C = Cam->CamPos->XYZ[0] * Cam->CamPos->XYZ[0] + 
	Cam->CamPos->XYZ[1] * Cam->CamPos->XYZ[1] + 
	Cam->CamPos->XYZ[2] * Cam->CamPos->XYZ[2] - Rsq;
C4 = C * 4.0;

if (IsCamView)
	{
	BWDE = new BusyWin("Sky", Height, 'BWDE', 0);
	} // if
else
	{
	Master->ProcInit(Height, "Sky");
	} // else

for (Y = RowZip = 0; Y < Height; ++Y, RowZip += Width)
	{
	for (CurSky = FirstSky; CurSky; CurSky = (Sky *)CurSky->Next)
		{
		if (CurSky->Enabled)
			{
			CurSky->Rand.GenMultiPRN(Width, CurSky->DitherTable);
			} // if
		} // while
	#pragma omp parallel for private(CurSky, CurLight, PixZip) firstprivate(Pix)
	for (X = 0; X < Width; ++X)
		{
		double LightValue[3], SkyValue[3], ZenAngle, LightAngle, Dither, B, Determinant, T, OldWt;
		VertexDEM ViewVec, LightVec, SkyVec;
		rPixelFragment *PixFrag = NULL;
		float ZFlt;
		int ReplaceValues, PixWt;
		unsigned char FartherWt;
		
		PixZip = RowZip + X;
		if (rPixelFragMap || AABuf[PixZip] < 255)
			{
			// these values get modified in UnProjectVertexDEM() so must be reset each pixel
			ViewVec.ScrnXYZ[0] = X + .5;
			ViewVec.ScrnXYZ[1] = Y + .5;
			ViewVec.ScrnXYZ[2] = ViewVec.Q = 1.0;

			Pix.RGB[2] = Pix.RGB[1] = Pix.RGB[0] = 0.0;
			PixWt = 255;

			// turn screen coords into world coord xyz
			Cam->UnProjectVertexDEM(DefCoords, &ViewVec, EarthLatScaleMeters, PlanetRad, 0);	// 0 = don't need lat/lon

			// find the pos vector for the pixel point from the camera, this is the view vector for that pixel
			ViewVec.GetPosVector(Cam->CamPos);
			ViewVec.UnitVector();

			// find intercept of view vector with sphere of radius 100 km
			B = 2.0 * (ViewVec.XYZ[0] * Cam->CamPos->XYZ[0] + 
				ViewVec.XYZ[1] * Cam->CamPos->XYZ[1] + 
				ViewVec.XYZ[2] * Cam->CamPos->XYZ[2]);

			Determinant = B * B - C4;
			if (Determinant >= 0.0)
				{
				// solving the quadratic equation for the + root
				if ((T = (-B + sqrt(Determinant)) * 0.5) > 0.0)
					{
					// substitute T back in parametric ray equation to find what the actual intercept coords are
					SkyVec.XYZ[0] = ViewVec.XYZ[0] * T + Cam->CamPos->XYZ[0];
					SkyVec.XYZ[1] = ViewVec.XYZ[1] * T + Cam->CamPos->XYZ[1];
					SkyVec.XYZ[2] = ViewVec.XYZ[2] * T + Cam->CamPos->XYZ[2];
					#ifdef WCS_BUILD_VNS
					DefCoords->CartToDeg(&SkyVec);
					#else // WCS_BUILD_VNS
					SkyVec.CartToDeg(PlanetRad);
					#endif // WCS_BUILD_VNS
					Cam->ProjectVertexDEM(DefCoords, &SkyVec, EarthLatScaleMeters, PlanetRad, 0);
					ZFlt = (float)SkyVec.ScrnXYZ[2];

					// Determine what the angle cosine is between the view vector and the zenith.
					// This will return 1 for looking straight up and -1 for down and 0 for horizon.
					ZenAngle = VectorAngle(ViewVec.XYZ, Cam->CamPos->XYZ);

					// scale the cosine from 0 at zenith to 1 at nadir
					ZenAngle = .5 - .5 * ZenAngle;

					// loop through each sky
					for (CurSky = FirstSky; CurSky; CurSky = (Sky *)CurSky->Next)
						{
						if (CurSky->Enabled)
							{
							// get basic pixel color from sky gradient
							if (! CurSky->SkyGrad.GetBasicColor(SkyValue[0], SkyValue[1], SkyValue[2], ZenAngle))
								SkyValue[2] = SkyValue[1] = SkyValue[0] = 0.0;
							for (CurLight = FirstLight; CurLight; CurLight = (Light *)CurLight->Next)
								{
								if (CurLight->Enabled && CurLight->Distant)
									{
									// make a light vector from the camera
									LightVec.CopyXYZ(CurLight->LightPos);
									LightVec.GetPosVector(Cam->CamPos);

									// Determine what the angle cosine is between the view vector and the light.
									// This will return 1 for looking straight at the light and -1 for away from it.
									LightAngle = VectorAngle(ViewVec.XYZ, LightVec.XYZ);

									// scale the cosine from 0 toward light to 1 away from it
									LightAngle = .5 - .5 * LightAngle;

									if (CurSky->LightGrad.GetBasicColor(LightValue[0], LightValue[1], LightValue[2], LightAngle))
										{
										SkyValue[0] += LightValue[0] * CurLight->CompleteColor[0];
										SkyValue[1] += LightValue[1] * CurLight->CompleteColor[1];
										SkyValue[2] += LightValue[2] * CurLight->CompleteColor[2];
										} // if
									} // if
								} // for each light
							SkyValue[0] *= CurSky->AnimPar[WCS_EFFECTS_SKY_ANIMPAR_INTENSITY].CurValue;
							SkyValue[1] *= CurSky->AnimPar[WCS_EFFECTS_SKY_ANIMPAR_INTENSITY].CurValue;
							SkyValue[2] *= CurSky->AnimPar[WCS_EFFECTS_SKY_ANIMPAR_INTENSITY].CurValue;

							// apply dithering
							if (CurSky->AnimPar[WCS_EFFECTS_SKY_ANIMPAR_PIXELDITHER].CurValue > 0.0)
								{
								Dither = (.5 - CurSky->DitherTable[X]) * CurSky->AnimPar[WCS_EFFECTS_SKY_ANIMPAR_PIXELDITHER].CurValue;
								SkyValue[0] += (SkyValue[0] * Dither);
								SkyValue[1] += (SkyValue[1] * Dither);
								SkyValue[2] += (SkyValue[2] * Dither);
								} // if

							// we're keeping a running sum of all the sky contributions
							Pix.RGB[0] += SkyValue[0];
							Pix.RGB[1] += SkyValue[1];
							Pix.RGB[2] += SkyValue[2];

							} // if enabled
						} // for each sky

					// back out the background color based on the intensity of the sky color
					Pix.Illum = (Pix.RGB[0] + Pix.RGB[1] + Pix.RGB[2]) * (1.0 / 3.0);
					if (Pix.Illum > 1.0)
						Pix.Illum = 1.0;
					if (rPixelFragMap)
						{
						// get the color of fragments farther than the sky
						rPixelFragMap[PixZip].CollapseFartherPixel(ZFlt, SkyValue, FartherWt);
						if (FartherWt)
							{
							OldWt = 1.0 - Pix.Illum;
							if (OldWt > 0.0)
								{
								OldWt *= FartherWt * (1.0 / 255.0);
								Pix.RGB[0] += SkyValue[0] * OldWt;
								Pix.RGB[1] += SkyValue[1] * OldWt;
								Pix.RGB[2] += SkyValue[2] * OldWt;
								} // if
							} // if
						} // else
					else
						{
						if (CloudAABuf[PixZip])
							{
							OldWt = 1.0 - Pix.Illum;
							if (OldWt > 0.0)
								{
								OldWt *= CloudAABuf[PixZip] * (1.0 / 65025.0);
								Pix.RGB[0] += CloudBitmap[0][PixZip] * OldWt;
								Pix.RGB[1] += CloudBitmap[1][PixZip] * OldWt;
								Pix.RGB[2] += CloudBitmap[2][PixZip] * OldWt;
								} // if
							} // if
						} // if

					// has to come after we derive the background color
					if (rPixelFragMap)
						{
						//PixWt = 255;
						if (! (PixFrag = rPixelFragMap[PixZip].PlotPixel(rPixelBlock, ZFlt, (unsigned char)PixWt, ~0UL, ~0UL, FragmentDepth, 
							(unsigned char)WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_SKY)))
							{
							continue;
							} // if
						} // if

					// if there were stars or celestial objects rendered we want to add the sky to whatever was there
					if (rPixelFragMap)
						{
						ReplaceValues = (ZFlt <= rPixelFragMap[PixZip].GetFirstZ());
						} // else
					else
						{
						if (Pix.RGB[0] > 1.0)
							Pix.RGB[0] = 1.0;
						if (Pix.RGB[1] > 1.0)
							Pix.RGB[1] = 1.0;
						if (Pix.RGB[2] > 1.0)
							Pix.RGB[2] = 1.0;

						CloudBitmap[0][PixZip] = (unsigned char)(Pix.RGB[0] * 255.9);
						CloudBitmap[1][PixZip] = (unsigned char)(Pix.RGB[1] * 255.9);
						CloudBitmap[2][PixZip] = (unsigned char)(Pix.RGB[2] * 255.9);
						CloudAABuf[PixZip] = 255;
						if (ZFlt < CloudZBuf[PixZip])
							{
							CloudZBuf[PixZip] = ZFlt;
							if (ZFlt < ZBuf[PixZip])
								{
								ReplaceValues = 1;
								} // if
							} // if
						//ScreenPixelPlotTwoBuf(Bitmap, ZBuf, AABuf, CloudBitmap, CloudZBuf, CloudAABuf, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
						} // if
					if (ReplaceValues)
						{
						if (LatBuf)
							LatBuf[PixZip] = (float)(SkyVec.Lat - TexRefLat);
						if (LonBuf)
							LonBuf[PixZip] = (float)(SkyVec.Lon - TexRefLon);
						if (ElevBuf)
							ElevBuf[PixZip] = (float)SkyVec.Elev;
						if (IllumBuf)
							IllumBuf[PixZip] = (float)Pix.Illum;
						if (ObjectBuf && ! ObjectBuf[PixZip])
							{
							ObjectBuf[PixZip] = FirstSky;
							if (ObjTypeBuf)
								ObjTypeBuf[PixZip] = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_SKY;
							} // if
						} // if

					if (PixFrag)
						{
						PixFrag->PlotPixel(rPixelBlock, Pix.RGB, Pix.Reflectivity, Pix.Normal);
						rPixelFragMap[PixZip].SetLastFrag(PixFrag);
						//ScreenPixelPlotFragments(&rPixelFragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);
						} // if

					} // if
				} // if
			} // if aabuffer not full
		} // for x
		// end #pragma omp parallel for
	if (rPixelFragMap)
		{
		for (X = 0, PixZip = RowZip; X < Width; ++X, ++PixZip)
			{
			ScreenPixelPlotFragments(&rPixelFragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);
			} // for
		} // if
	else
		{
		for (X = 0, PixZip = RowZip; X < Width; ++X, ++PixZip)
			{
			ScreenPixelPlotTwoBuf(Bitmap, ZBuf, AABuf, CloudBitmap, CloudZBuf, CloudAABuf, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
			} // for
		} // else
	if (IsCamView)	// gotta have this distinction
		{
		if(BWDE->Update(Y + 1))
			{
			Success = 0;
			break;
			} // if
		} // if
	else
		{
		Master->ProcUpdate(Y + 1);	// calls Master->CheckAbort() - do M->CheckAbort() any time - it returns the Run variable
		if (! Master->IsRunning())
			{
			Success = 0;
			break;
			} // if
		} // else
	} // for y

if (Master)
	Master->ProcClear();
if (BWDE)
	delete BWDE;

return (Success);

} // Renderer::RenderSky

/*===========================================================================*/

// this is simply a less efficient version of RenderSky for getting the color along a single ray
// When doing this in batch like RenderSky from a stationary point, certain variables need only be calculated once
int Renderer::GetSkyColor(double PixColor[3], double ViewVec[3], double OriginPos[3])
{
int Found = 0;
long SkyCt;
double LightValue[3], SkyValue[3], ZenAngle, LightAngle, Dither, R, Rsq, B, C, C4, Determinant, T;
Light *FirstLight, *CurLight;
Sky *FirstSky, *CurSky;
VertexDEM LightVec, SkyVec;

// no sky available for planimetric cameras, can't convert screen coords into world cartesian
if (Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
	return (0);

FirstLight = (Light *)EffectsBase->GetListPtr(WCS_EFFECTSSUBCLASS_LIGHT);
FirstSky = (Sky *)EffectsBase->GetListPtr(WCS_EFFECTSSUBCLASS_SKY);

while (FirstSky)
	{
	if (FirstSky->Enabled)
		break;
	FirstSky = (Sky *)FirstSky->Next;
	} // while

if (! FirstSky)
	return (0);	// no enabled skies so nothing to do

// advance to the first enabled distant light
while (FirstLight)
	{
	if (FirstLight->Enabled && FirstLight->Distant)
		break;
	FirstLight = (Light *)FirstLight->Next;
	} // while

// seed random number generators
CurSky = FirstSky;
SkyCt = 0;
while (CurSky)
	{
	if (CurSky->Enabled)
		{
		CurSky->Rand.Seed64(12345 + SkyCt, 54321 - SkyCt);
		SkyCt += 17;
		} // if
	CurSky = (Sky *)CurSky->Next;
	} // while

R = 2.0E5 + PlanetRad;
Rsq = R * R;	// this is the radius of the sky sphere squared
C = OriginPos[0] * OriginPos[0] + 
	OriginPos[1] * OriginPos[1] + 
	OriginPos[2] * OriginPos[2] - Rsq;
C4 = C * 4.0;

// find intercept of view vector with sphere of radius 100 km
B = 2.0 * (ViewVec[0] * OriginPos[0] + 
	ViewVec[1] * OriginPos[1] + 
	ViewVec[2] * OriginPos[2]);

Determinant = B * B - C4;
if (Determinant >= 0.0)
	{
	// solving the quadratic equation for the + root
	if ((T = (-B + sqrt(Determinant)) * 0.5) > 0.0)
		{
		// substitute T back in parametric ray equation to find what the actual intercept coords are
		SkyVec.XYZ[0] = ViewVec[0] * T + OriginPos[0];
		SkyVec.XYZ[1] = ViewVec[1] * T + OriginPos[1];
		SkyVec.XYZ[2] = ViewVec[2] * T + OriginPos[2];
		#ifdef WCS_BUILD_VNS
		DefCoords->CartToDeg(&SkyVec);
		#else // WCS_BUILD_VNS
		SkyVec.CartToDeg(PlanetRad);
		#endif // WCS_BUILD_VNS
		Cam->ProjectVertexDEM(DefCoords, &SkyVec, EarthLatScaleMeters, PlanetRad, 0);

		// Determine what the angle cosine is between the view vector and the zenith.
		// This will return 1 for looking straight up and -1 for down and 0 for horizon.
		ZenAngle = VectorAngle(ViewVec, OriginPos);

		// scale the cosine from 0 at zenith to 1 at nadir
		ZenAngle = .5 - .5 * ZenAngle;

		// loop through each sky
		for (CurSky = FirstSky; CurSky; CurSky = (Sky *)CurSky->Next)
			{
			if (CurSky->Enabled)
				{
				// get basic pixel color from sky gradient
				if (! CurSky->SkyGrad.GetBasicColor(SkyValue[0], SkyValue[1], SkyValue[2], ZenAngle))
					SkyValue[0] = SkyValue[1] = SkyValue[2] = 0.0;
				for (CurLight = FirstLight; CurLight; CurLight = (Light *)CurLight->Next)
					{
					if (CurLight->Enabled && CurLight->Distant)
						{
						// make a light vector from the camera
						LightVec.CopyXYZ(CurLight->LightPos);
						FindPosVector(LightVec.XYZ, LightVec.XYZ, OriginPos);

						// Determine what the angle cosine is between the view vector and the light.
						// This will return 1 for looking straight at the light and -1 for away from it.
						LightAngle = VectorAngle(ViewVec, LightVec.XYZ);

						// scale the cosine from 0 toward light to 1 away from it
						LightAngle = .5 - .5 * LightAngle;

						if (CurSky->LightGrad.GetBasicColor(LightValue[0], LightValue[1], LightValue[2], LightAngle))
							{
							SkyValue[0] += LightValue[0] * CurLight->CompleteColor[0];
							SkyValue[1] += LightValue[1] * CurLight->CompleteColor[1];
							SkyValue[2] += LightValue[2] * CurLight->CompleteColor[2];
							} // if
						} // if
					} // for each light
				SkyValue[0] *= CurSky->AnimPar[WCS_EFFECTS_SKY_ANIMPAR_INTENSITY].CurValue;
				SkyValue[1] *= CurSky->AnimPar[WCS_EFFECTS_SKY_ANIMPAR_INTENSITY].CurValue;
				SkyValue[2] *= CurSky->AnimPar[WCS_EFFECTS_SKY_ANIMPAR_INTENSITY].CurValue;

				// apply dithering
				if (CurSky->AnimPar[WCS_EFFECTS_SKY_ANIMPAR_PIXELDITHER].CurValue > 0.0)
					{
					Dither = (.5 - CurSky->Rand.GenPRN()) * CurSky->AnimPar[WCS_EFFECTS_SKY_ANIMPAR_PIXELDITHER].CurValue;
					SkyValue[0] += (SkyValue[0] * Dither);
					SkyValue[1] += (SkyValue[1] * Dither);
					SkyValue[2] += (SkyValue[2] * Dither);
					} // if

				// we're keeping a running sum of all the sky contributions
				PixColor[0] += SkyValue[0];
				PixColor[1] += SkyValue[1];
				PixColor[2] += SkyValue[2];
				Found = 1;
				} // if enabled
			} // for each sky
		} // if
	} // if

return (Found);

} // Renderer::GetSkyColor

/*===========================================================================*/

// CloudSource will be passed if this is a cloud shadow rendering
int Renderer::RenderClouds(CloudEffect *CloudSource)
{
double ReducedLon, DistOffset, DistDif, MaxCovgDist, MaxShadeDist, LayerSpace, ElevDif[2];
VertexCloud *Vert[3];
CloudLayerData **LayerList, *CurLayer;
CloudEffect *CurCloud;
BusyWin *BWDE = NULL;
BusyWin *BWBE = NULL;
int Success = 1;
unsigned long int StashCurSteps, StashMaxSteps;
time_t StashStartSecs;
long Ct, LayerCt, MoreToDo, Row, Col, NumLayers, LayerNum,
	VertNumber, RowLat, ColLon;
char LayerStr[64], StashText[200];
PolygonData Poly;
#ifdef WCS_BUILD_DEMO
unsigned char MessyRed, MessyGreen, MessyBlue, OMessyRed;
long MessyRows, FirstMessyRow, MessyRow, MessyCol, MessyRowCt, MessyZip;
time_t MessyTime;
double MessyStep;
#endif // WCS_BUILD_DEMO

#ifdef WCS_BUILD_DEMO
if (ZBuf[0] > 1.0f || ZBuf[Width * Height - 1] > 1.0f)
	{
	MemFuck += 90768000;
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
	} // if
#endif // WCS_BUILD_DEMO

NumLayers = 0;
if (CloudSource)
	{
	if (CloudSource->RenderShadowMap)
		NumLayers = CloudSource->NumLayers;
	} // if
else
	{
	// count cloud layers
	CurCloud = (CloudEffect *)EffectsBase->GetListPtr(WCS_EFFECTSSUBCLASS_CLOUD);
	while (CurCloud)
		{
		if (CurCloud->RenderPlanar)
			NumLayers += CurCloud->NumLayers;
		CurCloud = (CloudEffect *)CurCloud->Next;
		} // while
	} // else

// nothing to render, go home
if (NumLayers == 0)
	return (1);

// create an array of cloud layers.
#ifdef WCS_BUILD_DEMO
if (! (LayerList = (CloudLayerData **)AppMem_Alloc(NumLayers * sizeof (CloudLayerData *) + MemFuck, APPMEM_CLEAR)))
#else // WCS_BUILD_DEMO
if (! (LayerList = (CloudLayerData **)AppMem_Alloc(NumLayers * sizeof (CloudLayerData *), APPMEM_CLEAR)))
#endif // WCS_BUILD_DEMO
	return (0);

for (Ct = 0; Ct < NumLayers; Ct ++)
	{
	if (! (LayerList[Ct] = new CloudLayerData()))
		{
		Success = 0;
		goto EndCloud;
		} // if
	} // for

// each layer entry needs a pointer back to the cloud and the elevation of the layer,
// along with layer density and illumination data
CurCloud = CloudSource ? CloudSource: (CloudEffect *)EffectsBase->GetListPtr(WCS_EFFECTSSUBCLASS_CLOUD);
Ct = 0;
while (CurCloud)
	{
	if ((CloudSource && CurCloud->RenderShadowMap) || (! CloudSource && CurCloud->RenderPlanar))
		{
		CurCloud->CovgProf.GetMinMaxDist(DistOffset, MaxCovgDist);	// DistOffset is just used as a dummy here
		CurCloud->ShadeProf.GetMinMaxDist(DistOffset, MaxShadeDist);	// DistOffset is just used as a dummy here
		if (CurCloud->NumLayers > 1)
			{
			DistDif = 2.0 / (CurCloud->NumLayers - 1);
			LayerSpace = CurCloud->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_THICKNESS].CurValue / (CurCloud->NumLayers - 1);
			for (LayerNum = 0; LayerNum < CurCloud->NumLayers; LayerNum ++, Ct ++)
				{
				LayerList[Ct]->Cloud = CurCloud;
				LayerList[Ct]->LayerNum = LayerNum;
				LayerList[Ct]->Elev = CurCloud->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BASEELEV].CurValue + LayerNum * LayerSpace;
				
				// evaluate density gradient at layer elevation (Density) and elevation + 1m (Density1Up)
				DistOffset = (double)LayerNum / (double)(CurCloud->NumLayers - 1);
				LayerList[Ct]->Shading = CurCloud->ShadeProf.GetValue(0, DistOffset) * CurCloud->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_SELFSHADING].CurValue;
				LayerList[Ct]->Cloud->ColorGrad.GetBasicColor(LayerList[Ct]->RGB[0], LayerList[Ct]->RGB[1], LayerList[Ct]->RGB[2], DistOffset);
				DistOffset = (double)(LayerNum + .5) / (double)(CurCloud->NumLayers);
				LayerList[Ct]->Density = CurCloud->DensityProf.GetValue(0, DistOffset) * CurCloud->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_DENSITY].CurValue;
				LayerList[Ct]->Coverage = CurCloud->CovgProf.GetValue(0, DistOffset) * CurCloud->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_COVERAGE].CurValue;
				
				LayerList[Ct]->Dist = LayerList[Ct]->Elev - CurCloud->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BASEELEV].CurValue;
				LayerList[Ct]->Above = (char)(Cam->CamPos->Elev >= LayerList[Ct]->Elev);
				} // for
			} // if
		else
			{
			LayerList[Ct]->Cloud = CurCloud;
			LayerSpace = 0.0;
			LayerList[Ct]->Elev = CurCloud->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_BASEELEV].CurValue;

			LayerList[Ct]->Coverage = CurCloud->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_COVERAGE].CurValue;
			LayerList[Ct]->Density = CurCloud->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_DENSITY].CurValue;
			LayerList[Ct]->Shading = CurCloud->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_SELFSHADING].CurValue;

			// evaluate color gradient at 0
			LayerList[Ct]->Cloud->ColorGrad.GetBasicColor(LayerList[Ct]->RGB[0], LayerList[Ct]->RGB[1], LayerList[Ct]->RGB[2], 0.0);
			LayerList[Ct]->Dist = 0.0;
			LayerList[Ct]->Above = (char)(Cam->CamPos->Elev >= LayerList[Ct]->Elev);
			Ct ++;
			} // else
		} // if
	if (CloudSource)
		break;
	CurCloud = (CloudEffect *)CurCloud->Next;
	} // while

// sort layer list in order of increasing distance from camera elevation
MoreToDo = 1;
while (MoreToDo)
	{
	MoreToDo = 0;
	for (Ct = 0; Ct < NumLayers - 1; Ct ++)
		{
		ElevDif[0] = fabs(LayerList[Ct]->Elev - Cam->CamPos->Elev);
		ElevDif[1] = fabs(LayerList[Ct + 1]->Elev - Cam->CamPos->Elev);
		if (ElevDif[1] < ElevDif[0])
			{
			swmem(&LayerList[Ct], &LayerList[Ct + 1], sizeof (CloudLayerData *));
			MoreToDo = 1;
			} // if
		} // for
	} // while

if (IsCamView)
	{
	BWDE = new BusyWin((char *)(ShadowMapInProgress ? "Cloud Shadows": "Cloud Models"), NumLayers, 'BWDE', 0);
	} // if
else
	{
	Master->GetFrameSetup(StashCurSteps, StashMaxSteps, StashStartSecs, StashText);
	Master->FrameTextInit((char*)(ShadowMapInProgress ? "Cloud Shadows": "Cloud Models"));
	Master->FrameGaugeInit(NumLayers);
	} // else

// allocate an array of VertexCloud for each Cloud
CurCloud = CloudSource ? CloudSource: (CloudEffect *)EffectsBase->GetListPtr(WCS_EFFECTSSUBCLASS_CLOUD);
while (CurCloud)
	{
	if ((CloudSource && CurCloud->RenderShadowMap) || (! CloudSource && CurCloud->RenderPlanar))
		{
		if (! CurCloud->Vertices)
			{
			if (! CurCloud->AllocVertices())
				{
				Success = 0;
				goto EndCloud;
				} // if
			// evaluate density at each vertex and put value (0-1) in vertex->xyz.
			// xyz[0] has the normalized amplitude for waves
			// xyz[1] has the intensity factor based on falloff and vectors
			CurCloud->EvalVertices(RenderTime, EarthLatScaleMeters);
			} // if
		} // if
	if (CloudSource)
		break;
	CurCloud = (CloudEffect *)CurCloud->Next;
	} // while

// clear needed buffers
ClearCloudBuffers();

// render each layer
for (LayerCt = 0; Success && LayerCt < NumLayers; LayerCt ++)
	{
	// lock onto the current layer
	CurLayer = LayerList[LayerCt];

	// render layer
	Poly.Cloud = CurLayer->Cloud;
	Poly.Object = CurLayer->Cloud;
	Poly.CloudLayer = CurLayer;

	// set cloud density texture Z Center to the distance offset of the layer
	//if (CurLayer->Cloud->CovgTextureExists)
	//	{
	//	CurLayer->Cloud->TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->OffsetTexCenter(2, CurLayer->Dist);
	//	} // if

	if (IsCamView)
		{
		// F2_NOTE: I don't ever recall seeing this BusyWin... it's the one below it that I normally see.
		// Do we really need both in this same loop?
		BWBE = new BusyWin("Projecting", CurLayer->Cloud->Cols, 'BWBE', 0);
		} // if
	else if (Master)
		{
		Master->ProcInit(CurLayer->Cloud->Cols, "Projecting");
		} // else

	for (Col = VertNumber = 0; Col < CurLayer->Cloud->Cols; Col ++)
		{
		for (Row = 0; Row < CurLayer->Cloud->Rows; Row ++, VertNumber ++)
			{
			CurLayer->Cloud->Vertices[VertNumber].Elev = CurLayer->Elev;
			Cam->ProjectVertexDEM(DefCoords, &CurLayer->Cloud->Vertices[VertNumber], EarthLatScaleMeters, PlanetRad, 1);
			} // for
		if (IsCamView)	// gotta have this distinction
			{
			if(BWBE->Update(Col + 1))
				{
				Success = 0;
				break;
				} // if
			} // if
		else if (Master)
			{
			Master->ProcUpdate(Col + 1);	// calls Master->CheckAbort() - do M->CheckAbort() any time - it returns the Run variable
			if (! Master->IsRunning())
				{
				Success = 0;
				break;
				} // if
			} // else
		} // for

	if (BWBE)
		delete BWBE;

	if (! Success)
		break;
		
	// start a new process for rendering the polygons
	sprintf(LayerStr, "Model %d of %d", LayerCt + 1, NumLayers);
	if (IsCamView)
		{
		BWBE = new BusyWin(LayerStr, CurLayer->Cloud->Cols - 1, 'BWBE', 0);
		} // if
	else if (Master)
		{
		Master->ProcInit(CurLayer->Cloud->Cols - 1, LayerStr);
		} // else

	// render in the correct order - outward from the camera's position
	ReducedLon = Cam->CamPos->Lon;
	if (fabs(ReducedLon) >= 360.0)
		ReducedLon = fmod(ReducedLon, 360.0);	// retains the original sign
	if (ReducedLon < 0.0)
		ReducedLon += 360.0;
	//while (ReducedLon >= 360.0)
	//	ReducedLon -= 360.0;
	//while (ReducedLon < 0.0)
	//	ReducedLon += 360.0;
	if (fabs(CurLayer->Cloud->HighLon - ReducedLon) < 180.0)
		ColLon = quickftol((CurLayer->Cloud->HighLon - ReducedLon) / CurLayer->Cloud->LonStep);
	else
		ColLon = quickftol((ReducedLon - CurLayer->Cloud->HighLon) / CurLayer->Cloud->LonStep);

	RowLat = quickftol((Cam->CamPos->Lat - CurLayer->Cloud->LowLat) / CurLayer->Cloud->LatStep);
	if (ColLon < 0)
		ColLon = 0;
	else if (ColLon > CurLayer->Cloud->Cols - 1)
		ColLon = CurLayer->Cloud->Cols - 1;
	if (RowLat < 0)
		RowLat = 0;
	else if (RowLat > CurLayer->Cloud->Rows - 1)
		RowLat = CurLayer->Cloud->Rows - 1;

	Poly.ShadowFlags = CurLayer->Cloud->GetShadowFlags();
	Poly.ShadowOffset = 50.0;
	Poly.ReceivedShadowIntensity = CurLayer->Cloud->AnimPar[WCS_EFFECTS_CLOUD_ANIMPAR_RECVSHADOWINTENS].CurValue;

	Ct = 0;
	for (Col = ColLon; Success && Col < CurLayer->Cloud->Cols - 1; Col ++)
		{
		VertNumber = Col * CurLayer->Cloud->Rows + RowLat;
		for (Row = RowLat; Row < CurLayer->Cloud->Rows - 1; Row ++, VertNumber ++)
			{                                             //   _
			// render two polygons at very VertNumber station !\|	VertNumber is vertex at lower left (southwest)
			Vert[0] = &CurLayer->Cloud->Vertices[VertNumber];
			Vert[1] = &CurLayer->Cloud->Vertices[VertNumber + CurLayer->Cloud->Rows];
			Vert[2] = &CurLayer->Cloud->Vertices[VertNumber + 1];
			if (! (Success = RenderPolygon(&Poly, (VertexBase **)Vert, WCS_POLYGONTYPE_CLOUD)))
				break;
			Vert[0] = &CurLayer->Cloud->Vertices[VertNumber + CurLayer->Cloud->Rows + 1];
			Vert[1] = &CurLayer->Cloud->Vertices[VertNumber + 1];
			Vert[2] = &CurLayer->Cloud->Vertices[VertNumber + CurLayer->Cloud->Rows];
			if (! (Success = RenderPolygon(&Poly, (VertexBase **)Vert, WCS_POLYGONTYPE_CLOUD)))
				break;
			} // for

		VertNumber = Col * CurLayer->Cloud->Rows + RowLat - 1;
		for (Row = RowLat - 1; Row >= 0; Row --, VertNumber --)
			{
			Vert[0] = &CurLayer->Cloud->Vertices[VertNumber];
			Vert[1] = &CurLayer->Cloud->Vertices[VertNumber + CurLayer->Cloud->Rows];
			Vert[2] = &CurLayer->Cloud->Vertices[VertNumber + 1];
			if (! (Success = RenderPolygon(&Poly, (VertexBase **)Vert, WCS_POLYGONTYPE_CLOUD)))
				break;
			Vert[0] = &CurLayer->Cloud->Vertices[VertNumber + CurLayer->Cloud->Rows + 1];
			Vert[1] = &CurLayer->Cloud->Vertices[VertNumber + 1];
			Vert[2] = &CurLayer->Cloud->Vertices[VertNumber + CurLayer->Cloud->Rows];
			if (! (Success = RenderPolygon(&Poly, (VertexBase **)Vert, WCS_POLYGONTYPE_CLOUD)))
				break;
			} // for

		if (IsCamView)
			{
			if(BWBE->Update(++ Ct))
				{
				Success = 0;
				break;
				} // if
			} // if
		else if (Master)
			{
			Master->ProcUpdate(++ Ct);
			if (! Master->IsRunning())
				{
				Success = 0;
				break;
				} // if
			} // else
		} // for

	for (Col = ColLon - 1; Success && Col >= 0; Col --)
		{
		VertNumber = Col * CurLayer->Cloud->Rows + RowLat;
		for (Row = RowLat; Row < CurLayer->Cloud->Rows - 1; Row ++, VertNumber ++)
			{
			Vert[0] = &CurLayer->Cloud->Vertices[VertNumber];
			Vert[1] = &CurLayer->Cloud->Vertices[VertNumber + CurLayer->Cloud->Rows];
			Vert[2] = &CurLayer->Cloud->Vertices[VertNumber + 1];
			if (! (Success = RenderPolygon(&Poly, (VertexBase **)Vert, WCS_POLYGONTYPE_CLOUD)))
				break;
			Vert[0] = &CurLayer->Cloud->Vertices[VertNumber + CurLayer->Cloud->Rows + 1];
			Vert[1] = &CurLayer->Cloud->Vertices[VertNumber + 1];
			Vert[2] = &CurLayer->Cloud->Vertices[VertNumber + CurLayer->Cloud->Rows];
			if (! (Success = RenderPolygon(&Poly, (VertexBase **)Vert, WCS_POLYGONTYPE_CLOUD)))
				break;
			} // for

		VertNumber = Col * CurLayer->Cloud->Rows + RowLat - 1;
		for (Row = RowLat - 1; Row >= 0; Row --, VertNumber --)
			{
			Vert[0] = &CurLayer->Cloud->Vertices[VertNumber];
			Vert[1] = &CurLayer->Cloud->Vertices[VertNumber + CurLayer->Cloud->Rows];
			Vert[2] = &CurLayer->Cloud->Vertices[VertNumber + 1];
			if (! (Success = RenderPolygon(&Poly, (VertexBase **)Vert, WCS_POLYGONTYPE_CLOUD)))
				break;
			Vert[0] = &CurLayer->Cloud->Vertices[VertNumber + CurLayer->Cloud->Rows + 1];
			Vert[1] = &CurLayer->Cloud->Vertices[VertNumber + 1];
			Vert[2] = &CurLayer->Cloud->Vertices[VertNumber + CurLayer->Cloud->Rows];
			if (! (Success = RenderPolygon(&Poly, (VertexBase **)Vert, WCS_POLYGONTYPE_CLOUD)))
				break;
			} // for

		if (IsCamView)
			{
			if(BWBE->Update(++ Ct))
				{
				Success = 0;
				break;
				} // if
			} // if
		else if (Master)
			{
			Master->ProcUpdate(++ Ct);
			if (! Master->IsRunning())
				{
				Success = 0;
				break;
				} // if
			} // else
		} // for

	if (BWBE)
		delete BWBE;

	// update total clouds gauge
	if (IsCamView)	// gotta have this distinction
		{
		if(BWDE->Update(LayerCt + 1))
			{
			Success = 0;
			//if (CurLayer->Cloud->CovgTextureExists)
			//	{
			//	CurLayer->Cloud->TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->OffsetTexCenter(2, -CurLayer->Dist);
			//	} // if
			break;
			} // if
		} // if
	else if (Master)
		{
		Master->FrameGaugeUpdate(LayerCt + 1);	// calls Master->CheckAbort() - do M->CheckAbort() any time - it returns the Run variable
		if (! Master->IsRunning())
			{
			Success = 0;
			//if (CurLayer->Cloud->CovgTextureExists)
			//	{
			//	CurLayer->Cloud->TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->OffsetTexCenter(2, -CurLayer->Dist);
			//	} // if
			break;
			} // if
		} // else
	//if (CurLayer->Cloud->CovgTextureExists)
	//	{
	//	CurLayer->Cloud->TexRoot[WCS_EFFECTS_CLOUD_TEXTURE_COVERAGE]->OffsetTexCenter(2, -CurLayer->Dist);
	//	} // if
	} // for each layer

EndCloud:

if (LayerList)
	{
	for (Ct = 0; Ct < NumLayers; Ct ++)
		{
		if (LayerList[Ct])
			delete LayerList[Ct];
		} // for
	AppMem_Free(LayerList, NumLayers * sizeof (CloudLayerData *));
	} // if
CurCloud = (CloudEffect *)EffectsBase->GetListPtr(WCS_EFFECTSSUBCLASS_CLOUD);
while (CurCloud)
	{
	if (! CurCloud->VolumeSub)
		CurCloud->RemoveVertices();
	CurCloud = (CloudEffect *)CurCloud->Next;
	} // while
if (Master)
	{
	Master->ProcClear();
	Master->RestoreFrameSetup(StashCurSteps, StashMaxSteps, StashStartSecs, &StashText[1]);
	} // if
if (BWDE)
	delete BWDE;

return (Success);

} // Renderer::RenderClouds

/*===========================================================================*/

int Renderer::RenderStars(void)
{
unsigned short LatSeed, LonSeed;
int Success = 1, Abort;
long NumStars, StarfieldCt, StarCt, X, Y, PixZip;
double R, Rsq, B, C, C4, Determinant, T, MinLat, MaxLat, MinLon, MaxLon, CurMinLon, CurMaxLon, CurLon, CurLat, 
	CurBaseLat, CurBaseLon, HighBlockDensityFactor, LowBlockDensityFactor, HighDensityLat, DensityFactorDecrement, 
	Discard, SizeInPixels, Brightness, BrightnessFactor, TwinkleAmp, TwinkleTime, TwinkleSum, StarColor[3], Fact1, Fact2,
	CurMaxLonDiff;
StarFieldEffect *CurStar, *FirstStar;
VertexDEM StarPos, ViewVec;
BusyWin *BWDE = NULL;

FirstStar = EffectsBase->StarField;
while (FirstStar)
	{
	if (FirstStar->Enabled)
		break;
	FirstStar = (StarFieldEffect *)FirstStar->Next;
	} // while

StarfieldCt = 0;
CurStar = EffectsBase->StarField;
while (CurStar)
	{
	if (CurStar->Enabled)
		StarfieldCt ++;
	CurStar = (StarFieldEffect *)CurStar->Next;
	} // while

if (FirstStar)
	{
	if (IsCamView)
		{
		BWDE = new BusyWin("Stars", StarfieldCt, 'BWDE', 0);
		} // if
	else
		{
		Master->ProcInit(StarfieldCt, "Stars");
		} // else

	MinLat = MinLon = DBL_MAX;
	MaxLat = MaxLon = -DBL_MAX;

	// Note equations for ray/sphere intercept are found in Craig A. Lindley, "Practical Ray-tracing in C"
	R = 1.0E9;
	Rsq = 1.0E18;	// this is the radius of the star sphere squared
	C = Cam->CamPos->XYZ[0] * Cam->CamPos->XYZ[0] + 
		Cam->CamPos->XYZ[1] * Cam->CamPos->XYZ[1] + 
		Cam->CamPos->XYZ[2] * Cam->CamPos->XYZ[2] - Rsq;
	C4 = C * 4.0;

	for (Y = 0; Y < Height; Y += 2)
		{
		PixZip = Y * Width;
		for (X = 0; X < Width; X += 2, PixZip += 2)
			{
			if (AABuf[PixZip] < 255)
				{
				// these values get modified in UnProjectVertexDEM() so must be reset each pixel
				ViewVec.ScrnXYZ[0] = X + .5;
				ViewVec.ScrnXYZ[1] = Y + .5;
				ViewVec.ScrnXYZ[2] = ViewVec.Q = 1.0;

				// turn screen coords into world coord xyz
				Cam->UnProjectVertexDEM(DefCoords, &ViewVec, EarthLatScaleMeters, PlanetRad, 0);	// 0 = don't need lat/lon

				// find the pos vector for the pixel point from the camera, this is the view vector for that pixel
				ViewVec.GetPosVector(Cam->CamPos);
				ViewVec.UnitVector();

				// find intercept of view vector with sphere of radius 1 million km
				B = 2.0 * (ViewVec.XYZ[0] * Cam->CamPos->XYZ[0] + 
					ViewVec.XYZ[1] * Cam->CamPos->XYZ[1] + 
					ViewVec.XYZ[2] * Cam->CamPos->XYZ[2]);

				Determinant = B * B - C4;
				if (Determinant >= 0.0)
					{
					// solving the quadratic equation for the + root
					if ((T = (-B + sqrt(Determinant)) * 0.5) > 0.0)
						{
						// substitute T back in parametric ray equation to find what the actual intercept coords are
						ViewVec.XYZ[0] = ViewVec.XYZ[0] * T + Cam->CamPos->XYZ[0];
						ViewVec.XYZ[1] = ViewVec.XYZ[1] * T + Cam->CamPos->XYZ[1];
						ViewVec.XYZ[2] = ViewVec.XYZ[2] * T + Cam->CamPos->XYZ[2];
						#ifdef WCS_BUILD_VNS
						DefCoords->CartToDeg(&ViewVec);
						#else // WCS_BUILD_VNS
						ViewVec.CartToDeg(PlanetRad);
						#endif // WCS_BUILD_VNS
						if (ViewVec.Lat < MinLat)
							MinLat = ViewVec.Lat;
						if (ViewVec.Lat > MaxLat)
							MaxLat = ViewVec.Lat;
						if (ViewVec.Lon < MinLon)
							MinLon = ViewVec.Lon;
						if (ViewVec.Lon > MaxLon)
							MaxLon = ViewVec.Lon;
						} // if positive root is positive
					} // if
				} // if
			} // for
		} // for

	if (MaxLat > MinLat && MaxLon > MinLon)
		{
		MaxLat += 90.0;
		MinLat += 90.0;
		MaxLat = 20.0 * WCS_ceil(MaxLat * 0.05) - 90.0;
		MinLat = 20.0 * WCS_floor(MinLat * 0.05) - 90.0;
		
		for (CurStar = FirstStar, StarfieldCt = 0; Success && CurStar; CurStar = (StarFieldEffect *)CurStar->Next)
			{
			if (CurStar->Enabled && CurStar->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_DENSITY].CurValue > 0.0
				&& CurStar->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_SIZEFACTOR].CurValue > 0.0
				&& CurStar->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_INTENSITY].CurValue > 0.0)
				{
				// subtract out the longitude rotation
				CurMaxLon = MaxLon - CurStar->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_LONGITUDEROT].CurValue + EarthRotation;
				CurMinLon = MinLon - CurStar->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_LONGITUDEROT].CurValue + EarthRotation;
				// bump up or down to nearest 20 degree interval, this can result in overlap
				CurMaxLon = 20.0 * WCS_ceil(CurMaxLon * 0.05);
				CurMinLon = 20.0 * WCS_floor(CurMinLon * 0.05);
				// make longitude + for rng seeding
				CurMaxLonDiff = CurMaxLon - CurMinLon;
				if (fabs(CurMinLon) >= 360.0)
					CurMinLon = fmod(CurMinLon, 360.0);
				if (CurMinLon < 0.0)
					CurMinLon += 360.0;
				CurMaxLon = CurMaxLonDiff + CurMinLon;
				// replaced by above
				//while (CurMinLon < 0.0)
				//	{
				//	CurMinLon += 360.0;
				//	CurMaxLon += 360.0;
				//	} // while
				// don't want to double the star count in any area, remove overlap
				if (CurMaxLon - CurMinLon > 360.0)
					CurMaxLon = CurMinLon + 360.0;

				// set Abort to 0 for each star effect
				Abort = 0;
				for (CurLat = MinLat + 10.0, CurBaseLat = MinLat; Success && CurLat < MaxLat && ! Abort; CurLat += 20.0, CurBaseLat += 20.0)
					{
					for (CurLon = CurMinLon + 10.0, CurBaseLon = CurMinLon; Success && CurLon < CurMaxLon && ! Abort; CurLon += 20.0, CurBaseLon += 20.0)
						{
						LatSeed = (unsigned short)(CurLat + .5);
						LonSeed = (unsigned short)(CurLon >= 360.0 ? CurLon - 360.0 + .5: CurLon + .5);
						LatSeed *= 15;
						LonSeed *= 17;
						LatSeed = (unsigned short)(LatSeed + CurStar->RandomSeed);
						LonSeed = (unsigned short)(LonSeed + CurStar->RandomSeed);

						Fact1 = fabs(CurLat - 10.0);
						Fact2 = fabs(CurLat + 10.0);
						HighBlockDensityFactor = min(Fact1, Fact2);
						HighDensityLat = HighBlockDensityFactor;
						HighBlockDensityFactor = cos(HighBlockDensityFactor * PiOver180);
						Fact1 = fabs(CurLat - 10.0);
						Fact2 = fabs(CurLat + 10.0);
						LowBlockDensityFactor = max(Fact1, Fact2);
						LowBlockDensityFactor = cos(LowBlockDensityFactor * PiOver180);
						// this is the % density reduction for every degree change in latitude
						DensityFactorDecrement = (HighBlockDensityFactor - LowBlockDensityFactor) / (20.0 * HighBlockDensityFactor);

						NumStars = (long)(CurStar->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_DENSITY].CurValue * 333.0
							* HighBlockDensityFactor);

						CurStar->Rand.Seed64(LatSeed, LonSeed);
						for (StarCt = 0; Success && StarCt < NumStars && ! Abort; StarCt ++)
							{
							StarPos.Lat = CurStar->Rand.GenPRN() * 20.0 + CurBaseLat;
							Discard = (fabs(StarPos.Lat) - HighDensityLat) * DensityFactorDecrement;
							if (CurStar->Rand.GenPRN() >= Discard)
								{
								StarPos.Lon = CurStar->Rand.GenPRN() * 20.0 + CurBaseLon;
								StarPos.Lon += CurStar->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_LONGITUDEROT].CurValue;
								StarPos.Lon -= EarthRotation;
								StarPos.Elev = R - PlanetRad;
								
								// before applying rotation determine other random properties
								SizeInPixels = CurStar->Rand.GenPRN() * CurStar->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_SIZEFACTOR].CurValue;
								SizeInPixels *= 2.0;
								Brightness = CurStar->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_INTENSITY].CurValue;
								BrightnessFactor = CurStar->Rand.GenPRN();
								BrightnessFactor *= BrightnessFactor;
								BrightnessFactor = 1.0 - BrightnessFactor;
								Brightness -= Brightness * BrightnessFactor * CurStar->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_INTENSITYRANGE].CurValue;
								CurStar->ColorGrad.GetBasicColor(StarColor[0], StarColor[1], StarColor[2], CurStar->Rand.GenPRN());
								if ((TwinkleAmp = CurStar->AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_TWINKLEAMP].CurValue) > 0.0)
									{
									TwinkleTime = CurStar->Rand.GenPRN() * 100.0 + RenderTime;
									TwinkleSum = sin(TwinkleTime * 1.21);	// 5.2 sec period: TwoPi / 5.2 = 1.21
									TwinkleSum += sin(TwinkleTime * 2.09);	// 3.0 sec period: TwoPi / 3.0 = 2.09
									TwinkleSum += sin(TwinkleTime * 3.31);	// 1.9 sec period: TwoPi / 1.9 = 3.31
									TwinkleSum += sin(TwinkleTime * 5.71);	// 1.1 sec period: TwoPi / 1.1 = 5.71
									TwinkleSum = (TwinkleSum * 0.25) * TwinkleAmp;
									if (TwinkleSum > 0.0)
										TwinkleSum = 0.0;
									Brightness += Brightness * TwinkleSum;
									} // if
								if (Brightness > 0.0)
									{
									Cam->ProjectVertexDEM(DefCoords, &StarPos, EarthLatScaleMeters, PlanetRad, 1);
									if (StarPos.ScrnXYZ[2] > 0.0 && StarPos.ScrnXYZ[0] > -200.0 && StarPos.ScrnXYZ[0] < Width + 200.0 && 
										StarPos.ScrnXYZ[1] > -200.0 && StarPos.ScrnXYZ[1] < Height + 200.0)
										{
										Success = PlotCelest(&StarPos, SizeInPixels, Brightness, 0.0, StarColor, 
											(CurStar->Img ? CurStar->Img->GetRaster(): NULL), CurStar, NULL, 0, 0, Abort, 
											(unsigned char)WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_STAR);
										// if image was not found and user chose not to select a new one, Abort will be set.
										if (Abort)
											{
											// disable so it doesn't happen again
											CurStar->Enabled = 0;
											} // if
										} // if in bounds
									} // if
								} // if
							} // for
						} // for
					} // for
				} // if

			if (IsCamView)
				{
				if(BWDE->Update(++ StarfieldCt))
					{
					Success = 0;
					break;
					} // if
				} // if
			else
				{
				Master->ProcUpdate(++ StarfieldCt);
				if (! Master->IsRunning())
					{
					Success = 0;
					break;
					} // if
				} // else
			} // for
		} // if

	if (Master)
		Master->ProcClear();
	if (BWDE)
		delete BWDE;
	BWDE = NULL;
	} // if

return (Success);

} // Renderer::RenderStars

/*===========================================================================*/

int Renderer::RenderCelestial(void)
{
double CelestColor[3], SizeInPixels, Dummy;
int Success = 1, Abort;
unsigned long int StashCurSteps, StashMaxSteps;
time_t StashStartSecs;
long CelestCt, NumCelest = 0;
CelestialEffect *CurCelest;
EffectList *CurLight;
BusyWin *BWDE = NULL;
char StashText[200];
VertexDEM CelestPos, HVec0, HVec1;

CurCelest = EffectsBase->Celestial;
while (CurCelest)
	{
	if (CurCelest->Enabled && CurCelest->Img && CurCelest->Img->GetRaster())
		{
		if (CurLight = CurCelest->Lights)
			{
			while (CurLight)
				{
				if (CurLight->Me && CurLight->Me->Enabled)
					{
					NumCelest ++;
					} // if
				CurLight = CurLight->Next;
				} // while
			} // if
		else
			{
			NumCelest ++;
			} // else
		} // if
	CurCelest = (CelestialEffect *)CurCelest->Next;
	} // while

if (NumCelest)
	{
	if (IsCamView)
		{
		BWDE = new BusyWin("Celestial Objects", NumCelest, 'BWDE', 0);
		} // if
	else if (Master)
		{
		Master->GetFrameSetup(StashCurSteps, StashMaxSteps, StashStartSecs, StashText);
		Master->FrameTextInit("Celestial Objects");
		Master->FrameGaugeInit(NumCelest);
		} // else

	CurCelest = EffectsBase->Celestial;
	CelestCt = 0;
	while (CurCelest && Success)
		{
		if (CurCelest->Enabled && CurCelest->Img && CurCelest->Img->GetRaster())
			{

			// set Abort to 0 for each celestial object
			Abort = 0;
			if (CurCelest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_TRANSPARENCY].CurValue < 1.0)
				{
				CelestColor[0] = CurCelest->Color.GetCompleteValue(0);
				CelestColor[1] = CurCelest->Color.GetCompleteValue(1);
				CelestColor[2] = CurCelest->Color.GetCompleteValue(2);
				if (CurLight = CurCelest->Lights)
					{
					while (CurLight && Success)
						{
						if (CurLight->Me && CurLight->Me->Enabled)
							{
							CelestPos.Lat = ((Light *)CurLight->Me)->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue;
							CelestPos.Lon = ((Light *)CurLight->Me)->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].CurValue;
							CelestPos.Elev = ((Light *)CurLight->Me)->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].CurValue;
							if (((Light *)CurLight->Me)->Distant)
								CelestPos.Lon += EarthRotation;

							Cam->ProjectVertexDEM(DefCoords, &CelestPos, EarthLatScaleMeters, PlanetRad, 1);	// 1 means convert lat/lon to cart

							// test to see if in plot range
							if (CelestPos.ScrnXYZ[2] > 0.0 && CelestPos.ScrnXYZ[0] > -10000.0 && CelestPos.ScrnXYZ[0] < Width + 10000.0 && 
								CelestPos.ScrnXYZ[1] > -10000.0 && CelestPos.ScrnXYZ[1] < Height + 10000.0)
								{
								// find the vector parallel to screen and horizontal to the right
								HVec0.CopyScrnXYZQ(&CelestPos);
								HVec1.CopyScrnXYZQ(&CelestPos);
								HVec1.ScrnXYZ[0] += 1.0;
								Cam->UnProjectVertexDEM(DefCoords, &HVec0, EarthLatScaleMeters, PlanetRad, 0);	// 0 means don't nead lat/lon
								Cam->UnProjectVertexDEM(DefCoords, &HVec1, EarthLatScaleMeters, PlanetRad, 0);
								HVec1.GetPosVector(&HVec0);
								HVec1.UnitVector();

								// find intersection of ray from center of object in direction HVec1 with sphere of object radius
								if (RaySphereIntersect(CurCelest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_RADIUS].CurValue,
									CelestPos.XYZ, HVec1.XYZ, CelestPos.XYZ, HVec0.XYZ, Dummy))
									{
									Cam->ProjectVertexDEM(DefCoords, &HVec0, EarthLatScaleMeters, PlanetRad, 0);	// 0 means already in cartesian
									SizeInPixels = 2.0 * sqrt((HVec0.ScrnXYZ[0] - CelestPos.ScrnXYZ[0]) * (HVec0.ScrnXYZ[0] - CelestPos.ScrnXYZ[0]) +
										(HVec0.ScrnXYZ[1] - CelestPos.ScrnXYZ[1]) * (HVec0.ScrnXYZ[1] - CelestPos.ScrnXYZ[1]));
									SizeInPixels *= CurCelest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_SIZEFACTOR].CurValue;

									Success = PlotCelest(&CelestPos, SizeInPixels, 1.0, 
										CurCelest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_TRANSPARENCY].CurValue, CelestColor, 
										CurCelest->Img->GetRaster(), CurCelest, (Light *)CurLight->Me, CurCelest->ShowPhase, 1, Abort,
										(unsigned char)WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_CELESTIAL);
									// if image was not found and user chose not to select a new one, Abort will be set.
									if (Abort)
										{
										// disable so it doesn't happen again
										CurCelest->Enabled = 0;
										break;
										} // if
									} // if
								} // if
							CelestCt ++;
							} // if
						CurLight = CurLight->Next;
						} // while
					} // if
				else
					{
					CelestPos.Lat = CurCelest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE].CurValue;
					CelestPos.Lon = CurCelest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE].CurValue - EarthRotation;
					CelestPos.Elev = CurCelest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE].CurValue;
					
					Cam->ProjectVertexDEM(DefCoords, &CelestPos, EarthLatScaleMeters, PlanetRad, 1);

					// test to see if in plot range
					if (CelestPos.ScrnXYZ[2] > 0.0 && CelestPos.ScrnXYZ[0] > -10000.0 && CelestPos.ScrnXYZ[0] < Width + 10000.0 && 
						CelestPos.ScrnXYZ[1] > -10000.0 && CelestPos.ScrnXYZ[1] < Height + 10000.0)
						{
						// find the vector parallel to screen and horizontal to the right
						HVec0.CopyScrnXYZQ(&CelestPos);
						HVec1.CopyScrnXYZQ(&CelestPos);
						HVec1.ScrnXYZ[0] += 1.0;
						Cam->UnProjectVertexDEM(DefCoords, &HVec0, EarthLatScaleMeters, PlanetRad, 0);	// 0 means don't nead lat/lon
						Cam->UnProjectVertexDEM(DefCoords, &HVec1, EarthLatScaleMeters, PlanetRad, 0);
						HVec1.GetPosVector(&HVec0);
						HVec1.UnitVector();

						// find intersection of ray from center of object in direction HVec1 with sphere of object radius
						if (RaySphereIntersect(CurCelest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_RADIUS].CurValue,
							CelestPos.XYZ, HVec1.XYZ, CelestPos.XYZ, HVec0.XYZ, Dummy))
							{
							Cam->ProjectVertexDEM(DefCoords, &HVec0, EarthLatScaleMeters, PlanetRad, 0);	// 0 means already in cartesian
							SizeInPixels = 2.0 * sqrt((HVec0.ScrnXYZ[0] - CelestPos.ScrnXYZ[0]) * (HVec0.ScrnXYZ[0] - CelestPos.ScrnXYZ[0]) +
								(HVec0.ScrnXYZ[1] - CelestPos.ScrnXYZ[1]) * (HVec0.ScrnXYZ[1] - CelestPos.ScrnXYZ[1]));
							SizeInPixels *= CurCelest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_SIZEFACTOR].CurValue;

							Success = PlotCelest(&CelestPos, SizeInPixels, 1.0, 
								CurCelest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_TRANSPARENCY].CurValue, CelestColor, 
								CurCelest->Img->GetRaster(), CurCelest, NULL, CurCelest->ShowPhase, 1, Abort,
								(unsigned char)WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_CELESTIAL);
							// if image was not found and user chose not to select a new one, Abort will be set.
							if (Abort)
								{
								// disable so it doesn't happen again
								CurCelest->Enabled = 0;
								break;
								} // if
							} // if
						} // if
					CelestCt ++;
					} // else
				} // if

			if (IsCamView)
				{
				if (BWDE)
					{
					if(BWDE->Update(CelestCt))
						{
						Success = 0;
						break;
						} // if
					} // if
				} // if
			else if (Master)
				{
				Master->FrameGaugeUpdate(CelestCt);
				if (! Master->IsRunning())
					{
					Success = 0;
					break;
					} // if
				} // else
			} // if
		CurCelest = (CelestialEffect *)CurCelest->Next;
		} // while

	if (Master)
		Master->RestoreFrameSetup(StashCurSteps, StashMaxSteps, StashStartSecs, &StashText[1]);
	if(BWDE)
		delete BWDE;
	BWDE = NULL;
	} // if

return (Success);

} // Renderer::RenderCelestial

/*===========================================================================*/

int Renderer::PlotCelest(VertexDEM *Vert, double WidthInPixels, double Brightness, double Transparency, 
	double CelestColor[3], Raster *SourceRast, GeneralEffect *CurObject, Light *PosSource, int ShowPhase, int DoGauge, 
	int &Abort, unsigned char PlotFlags)
{
int Success = 1, ReplaceValues, ColorImage;
double HeightInPixels, XStart, YStart, XEnd, YEnd, XDbl, YDbl, XCovg, YCovg, Opacity, NewWt, SumWt, ReplaceColor[3],
	SampleXLow, SampleXHigh, SampleYLow, SampleYHigh, SampleWt, Dummy, /*AtmoColor[3], AtmoOpticalDepth[3],*/
	PixNormal[3];
long X, Y, XInt, YInt, PixZip, HtCt;
float ZFlt;
VertexDEM ViewVec;
PixelData Pix;
CelestialEffect *CurCelest;
BusyWin *BWDE = NULL;

//ReplaceColor[0] = CelestColor[0] * 255.99 * Brightness;
//ReplaceColor[1] = CelestColor[1] * 255.99 * Brightness;
//ReplaceColor[2] = CelestColor[2] * 255.99 * Brightness;
ReplaceColor[0] = CelestColor[0] * Brightness;
ReplaceColor[1] = CelestColor[1] * Brightness;
ReplaceColor[2] = CelestColor[2] * Brightness;
PixNormal[0] = PixNormal[1] = PixNormal[2] = 0.0;
Opacity = 1.0 - Transparency;
Pix.SetDefaults();
Pix.PixelType = WCS_PIXELTYPE_CELESTIAL;
Pix.Object = CurObject;
Pix.IgnoreLight = PosSource;
Pix.ShadowFlags = 0;
if (WidthInPixels <= 0.0)
	return (1);

if (SourceRast)
	{
	HeightInPixels = ((double)SourceRast->Rows / SourceRast->Cols) * Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue * WidthInPixels;
	ColorImage = SourceRast->GetIsColor();
	// this will return NULL if image couldn't be loaded, in which case we'll just plot a colored square
	//SourceRast = SourceRast->FindBestDownsampledVersion(1.0 / WidthInPixels, 1.0 / HeightInPixels);
	SourceRast = SourceRast->FindBestDownsampledVersion(WidthInPixels, HeightInPixels);
	} // if
else
	{
	HeightInPixels = Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue * WidthInPixels;
	} // else

XStart = Vert->ScrnXYZ[0] - .5 * WidthInPixels;
YStart = Vert->ScrnXYZ[1] - .5 * HeightInPixels;
XEnd = Vert->ScrnXYZ[0] + .5 * WidthInPixels;
YEnd = Vert->ScrnXYZ[1] + .5 * HeightInPixels;
XInt = (long)WCS_floor(XStart);
YInt = (long)WCS_floor(YStart);
ZFlt = (float)Vert->ScrnXYZ[2];

if (DoGauge)
	{
	if (IsCamView)
		{
		BWDE = new BusyWin(CurObject->GetName(), (unsigned long)HeightInPixels + 1, 'BWDE', 0);
		} // if
	else
		{
		Master->ProcInit((unsigned long)HeightInPixels + 1, CurObject->GetName());
		} // else
	} // if

for (Y = YInt, YDbl = YStart, HtCt = 0; Y < YEnd && ! Abort; Y ++, YDbl = WCS_floor(YDbl + 1))
	{
	if (Y < 0)
		{
		++ HtCt;
		continue;
		} // if
	if (Y >= Height)
		break;
	YCovg = min(Y + 1, YEnd) - YDbl;
	for (X = XInt, XDbl = XStart; X < XEnd; X ++, XDbl = WCS_floor(XDbl + 1))
		{
		if (X < 0)
			continue;
		if (X >= Width)
			break;
		PixZip = Y * Width + X;
		if (AABuf[PixZip] < 255)
			{
			XCovg = min(X + 1, XEnd) - XDbl;
			NewWt = XCovg * YCovg * Opacity * 255.99;
			if (SourceRast)
				{
				// convert sample corners to 0-1 range
				SampleXLow = (XDbl - XStart) / WidthInPixels;
				SampleXHigh = (XDbl + XCovg - XStart) / WidthInPixels;
				SampleYLow = (YDbl - YStart) / HeightInPixels;
				SampleYHigh = (YDbl + YCovg - YStart) / HeightInPixels;
				// for each output pixel or portion thereof find the color from the raster image
				Pix.RGB[0] = Pix.RGB[1] = Pix.RGB[2] = 0.0;
				// sample image with self-masking
				SampleWt = SourceRast->SampleRangeByteDouble3(Pix.RGB, SampleXLow, SampleXHigh, SampleYLow, SampleYHigh, Abort, 1);
				// if image was not found and user chose not to select a new one, Abort will be set.
				if (Abort)
					{
					break;
					} // if
				NewWt *= SampleWt;
				if (NewWt <= 0.0)
					continue;
				if (ColorImage)
					{
					//Pix.RGB[0] *= 255.99 * Brightness / SampleWt;
					//Pix.RGB[1] *= 255.99 * Brightness / SampleWt;
					//Pix.RGB[2] *= 255.99 * Brightness / SampleWt;
					Pix.RGB[0] *= Brightness / SampleWt;
					Pix.RGB[1] *= Brightness / SampleWt;
					Pix.RGB[2] *= Brightness / SampleWt;
					} // else
				else
					{
					Pix.RGB[0] *= ReplaceColor[0] / SampleWt;
					Pix.RGB[1] *= ReplaceColor[1] / SampleWt;
					Pix.RGB[2] *= ReplaceColor[2] / SampleWt;
					} // else
				} // if
			else
				{
				Pix.RGB[0] = ReplaceColor[0];
				Pix.RGB[1] = ReplaceColor[1];
				Pix.RGB[2] = ReplaceColor[2];
				} // else

			if ((SourceRast && ShowPhase) || VolumetricAtmospheresExist)
				{
				CurCelest = (CelestialEffect *)CurObject;
				ViewVec.ScrnXYZ[0] = X + .5;
				ViewVec.ScrnXYZ[1] = Y + .5;
				ViewVec.ScrnXYZ[2] = ViewVec.Q = 1.0;

				// turn screen coords into world coord xyz
				Cam->UnProjectVertexDEM(DefCoords, &ViewVec, EarthLatScaleMeters, PlanetRad, 0);	// 0 = don't need lat/lon

				// find the pos vector for the pixel point from the camera, this is the view vector for that pixel
				ViewVec.GetPosVector(Cam->CamPos);
				ViewVec.UnitVector();

				RaySphereIntersect(CurCelest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_RADIUS].CurValue *
					CurCelest->AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_SIZEFACTOR].CurValue,
					Cam->CamPos->XYZ, ViewVec.XYZ, Vert->XYZ, Pix.XYZ, Dummy);
				} // if

			if (SourceRast && ShowPhase)
				{
				FindPosVector(Pix.Normal, Pix.XYZ, Vert->XYZ);
				UnitVector(Pix.Normal);
				FindPosVector(Pix.ViewVec, Cam->CamPos->XYZ, Pix.XYZ);
				Pix.OpacityUsed = NewWt;
				IlluminatePixel(&Pix);
				} // if

			if (! rPixelFragMap)
				{
				Pix.RGB[0] = Pix.RGB[0] >= 1.0 ? 255.0: Pix.RGB[0] * 255.0;
				Pix.RGB[1] = Pix.RGB[1] >= 1.0 ? 255.0: Pix.RGB[1] * 255.0;
				Pix.RGB[2] = Pix.RGB[2] >= 1.0 ? 255.0: Pix.RGB[2] * 255.0;

				if (ZFlt <= CloudZBuf[PixZip])
					{
					ReplaceValues = 1;
					if (NewWt < 255.0)
						{
						if (NewWt + CloudAABuf[PixZip] > 255.99)
							CloudAABuf[PixZip] = (unsigned char)(255.99 - NewWt);
						SumWt = NewWt + CloudAABuf[PixZip];
						CloudBitmap[0][PixZip] = (unsigned char)((Pix.RGB[0] * NewWt + CloudBitmap[0][PixZip] * CloudAABuf[PixZip]) / SumWt);
						CloudBitmap[1][PixZip] = (unsigned char)((Pix.RGB[1] * NewWt + CloudBitmap[1][PixZip] * CloudAABuf[PixZip]) / SumWt);
						CloudBitmap[2][PixZip] = (unsigned char)((Pix.RGB[2] * NewWt + CloudBitmap[2][PixZip] * CloudAABuf[PixZip]) / SumWt);
						} // if need to compute a blend factor
					else
						{
						SumWt = NewWt + CloudAABuf[PixZip];
						CloudBitmap[0][PixZip] = (unsigned char)(Pix.RGB[0]);
						CloudBitmap[1][PixZip] = (unsigned char)(Pix.RGB[1]);
						CloudBitmap[2][PixZip] = (unsigned char)(Pix.RGB[2]);
						} // just a straight replacement
					} // if
				else
					{
					// new pixel is farther away and we know that AABuf is not full so blend the values
					ReplaceValues = 0;
					if (NewWt + CloudAABuf[PixZip] > 255.99)
						NewWt = 255.99 -  CloudAABuf[PixZip];
					SumWt = NewWt + CloudAABuf[PixZip];
					CloudBitmap[0][PixZip] = (unsigned char)((Pix.RGB[0] * NewWt + CloudBitmap[0][PixZip] * CloudAABuf[PixZip]) / SumWt);
					CloudBitmap[1][PixZip] = (unsigned char)((Pix.RGB[1] * NewWt + CloudBitmap[1][PixZip] * CloudAABuf[PixZip]) / SumWt);
					CloudBitmap[2][PixZip] = (unsigned char)((Pix.RGB[2] * NewWt + CloudBitmap[2][PixZip] * CloudAABuf[PixZip]) / SumWt);
					} // else need to blend for sure
				SumWt = WCS_ceil(SumWt);
				if (SumWt >= 254.5)
					CloudAABuf[PixZip] = 255;
				else
					CloudAABuf[PixZip] = (unsigned char)(SumWt);
				} // if
			else
				{
				ReplaceValues = (ZFlt <= rPixelFragMap[PixZip].GetFirstZ());
				} // else
			if (ReplaceValues)
				{
				if (! rPixelFragMap)
					CloudZBuf[PixZip] = ZFlt;
				if (LatBuf)
					LatBuf[PixZip] = (float)(Vert->Lat - TexRefLat);
				if (LonBuf)
					LonBuf[PixZip] = (float)(Vert->Lon - TexRefLon);
				if (ElevBuf)
					ElevBuf[PixZip] = (float)Vert->Elev;
				if (NormalBuf[0])
					NormalBuf[0][PixZip] = (float)Pix.Normal[0];
				if (NormalBuf[1])
					NormalBuf[1][PixZip] = (float)Pix.Normal[1];
				if (NormalBuf[2])
					NormalBuf[2][PixZip] = (float)Pix.Normal[2];
				if (ObjectBuf)
					ObjectBuf[PixZip] = CurObject;
				if (ObjTypeBuf)
					ObjTypeBuf[PixZip] = (PlotFlags & 0x0f);
				} // if
			if (rPixelFragMap)
				{
				rPixelFragment *PixFrag;
				int PixWt;

				PixWt = (int)WCS_ceil(NewWt);
				if (PixWt > 255)
					PixWt = 255;
				if (PixFrag = rPixelFragMap[PixZip].PlotPixel(rPixelBlock, ZFlt, (unsigned char)PixWt, ~0UL, ~0UL, FragmentDepth, 
					PlotFlags))
					{
					PixFrag->PlotPixel(rPixelBlock, Pix.RGB, 0.0, PixNormal);
					ScreenPixelPlotFragments(&rPixelFragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);
					} // if
				} // if
			if (! rPixelFragMap)
				ScreenPixelPlotTwoBuf(Bitmap, ZBuf, AABuf, CloudBitmap, CloudZBuf, CloudAABuf, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
			} // if
		} // for X

	if (DoGauge)
		{
		if (IsCamView)
			{
			if (BWDE)
				{
				if(BWDE->Update(++ HtCt))
					{
					Success = 0;
					break;
					} // if
				} // if
			} // if
		else
			{
			Master->ProcUpdate(++ HtCt);
			if (! Master->IsRunning())
				{
				Success = 0;
				break;
				} // if
			} // else
		} // if
	} // for Y 

if (Master && DoGauge)
	Master->ProcClear();
if (BWDE)
	delete BWDE;
BWDE = NULL;

return (Success);

} // Renderer::PlotCelest

/*===========================================================================*/

int Renderer::RenderVolumetrics(int BeforeReflections)
{

if (EffectsBase->AtmoBase.SpeedBoost > 1)
	return (RenderVolumetricsSpeedBoost(BeforeReflections, EffectsBase->AtmoBase.SpeedBoost));

return (RenderVolumetricsNoBoost(BeforeReflections));

} // Renderer::RenderVolumetrics

/*===========================================================================*/

int Renderer::RenderVolumetricsSpeedBoost(int BeforeReflections, int BoostRate)
{
double InterpInterval, InterpIntervalY, InterpIntervalX, InterpY, InterpX, InvInterpX, InvInterpY, Transmission, FragColor[3], SumColor[3], 
	InterpColor[4][3], InterpTransmission[4], EndFragDist, MinDist, MaxDist,  
	Dist1, Dist2, Dist3, Dist4, ThreshScale, AdaptiveThreshDist, AdaptiveTransDist, TransThresh;
unsigned long TopMask, BotMask, TestAlpha;
long NumXSamples, NumYSamples, X, Y, PrevX, PrevY, XSample, YSample, LastX, LastY, InterpZip, InterpZipY, InterpZipX, FragPlot, 
	ComputeVolumetrics, Adaptive, TempAdaptive, FirstFrag;
int Success = 1;
AnimDoubleTime *TransmissionStash[2], *CurTrans, *PrevTrans;
AnimColorTime *ColorStash[2], *CurColor, *PrevColor;
float *FinalT[2], *CurFinalT, *PrevFinalT;
VolumetricSubstance *CurSub;
rPixelFragment *CurFrag, *LastFrag;
BusyWin *BWDE = NULL;
VertexDEM ViewVec, FragVert, SubVert;
TextureData TexData;
PixelData PixData;

if (! rPixelFragMap && ! (Bitmap[0] && Bitmap[1] && Bitmap[2] && ZBuf))
	return (1);

// see if there is anything to do in this pass
ComputeVolumetrics = 0;
CurSub = Substances;
while (CurSub)
	{
	if (CurSub->GetRenderVolumetric() && CurSub->GetBeforeReflections() == BeforeReflections)
		ComputeVolumetrics = 1;
	CurSub = CurSub->NextSub;
	} // while
if (! ComputeVolumetrics)
	return (1);

if (IsCamView)
	{
	BWDE = new BusyWin("Volumetrics", Height, 'BWDE', 0);
	} // if
else
	{
	Master->ProcInit(Height, "Volumetrics");
	} // else

PixData.TexData = &TexData;
if (BoostRate > 5)
	BoostRate = 5;

NumXSamples = (Width - 1) / BoostRate + 1;
if ((Width - 1) % BoostRate)
	NumXSamples ++;
NumYSamples = (Height - 1) / BoostRate + 1;
if ((Height - 1) % BoostRate)
	NumYSamples ++;
LastX = Width + ((Width - 1) % BoostRate ? BoostRate: 0);
LastY = Height + ((Height - 1) % BoostRate ? BoostRate: 0);
InterpInterval = 1.0 / BoostRate;
ThreshScale = (100 - EffectsBase->AtmoBase.AdaptiveThreshold) * .01;
TransThresh = EffectsBase->AtmoBase.AdaptiveThreshold * .01;

TransmissionStash[0] = TransmissionStash[1] = NULL;
ColorStash[0] = ColorStash[1] = NULL;
FinalT[0] = FinalT[1] = NULL;

if ((TransmissionStash[0] = new AnimDoubleTime[NumXSamples]) &&
	(TransmissionStash[1] = new AnimDoubleTime[NumXSamples]) &&
	(ColorStash[0] = new AnimColorTime[NumXSamples]) &&
	(ColorStash[1] = new AnimColorTime[NumXSamples]) &&
	(FinalT[0] = (float *)AppMem_Alloc(NumXSamples * sizeof (float), APPMEM_CLEAR)) &&
	(FinalT[1] = (float *)AppMem_Alloc(NumXSamples * sizeof (float), APPMEM_CLEAR)))
	{
	for (XSample = 0; XSample < NumXSamples; XSample ++)
		{
		TransmissionStash[0][XSample].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
		TransmissionStash[1][XSample].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
		ColorStash[0][XSample].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
		ColorStash[1][XSample].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
		} // for
	CurTrans = TransmissionStash[0];
	PrevTrans = TransmissionStash[1];
	CurColor = ColorStash[0];
	PrevColor = ColorStash[1];
	CurFinalT = FinalT[0];
	PrevFinalT = FinalT[1];
	for (Y = PrevY = YSample = 0; Y < LastY && YSample < NumYSamples; Y += BoostRate, YSample ++)
		{
		if (Y < Height)
			InterpIntervalY = InterpInterval;
		else
			{
			Y = Height - 1;
			InterpIntervalY = 1.0 / (Y - PrevY);
			}
		for (X = PrevX = XSample = 0; X < LastX && XSample < NumXSamples; X += BoostRate, XSample ++)
			{
			if (X < Width)
				{
				if (X > 0)
					InterpIntervalX = InterpInterval;
				else
					InterpIntervalX = 1.0;
				} // if
			else
				{
				X = Width - 1;
				InterpIntervalX = 1.0 / (X - PrevX);
				} // if
			SampleVolumetricRay(X, Y, &CurTrans[XSample], &CurColor[XSample], &CurFinalT[XSample],
				&TexData, &PixData, &ViewVec, &FragVert, &SubVert, BeforeReflections);

			// interpolate
			// fill in spaces
			if (Y > 0)
				{
				if (PrevTrans[XSample].Graph[0] || CurTrans[XSample].Graph[0] || 
					(XSample > 0 && (PrevTrans[XSample - 1].Graph[0] || CurTrans[XSample - 1].Graph[0])))
					{
					Adaptive = 0;
					Dist2 = PrevTrans[XSample].Graph[0] ? PrevTrans[XSample].Graph[0]->GetFirstDist(): 0.0;
					Dist4 = CurTrans[XSample].Graph[0] ? CurTrans[XSample].Graph[0]->GetFirstDist(): 0.0;
					if (XSample > 0)
						{
						Dist1 = PrevTrans[XSample - 1].Graph[0] ? PrevTrans[XSample - 1].Graph[0]->GetFirstDist(): 0.0;
						Dist3 = CurTrans[XSample - 1].Graph[0] ? CurTrans[XSample - 1].Graph[0]->GetFirstDist(): 0.0;
						MaxDist = max(Dist2, Dist4);
						MaxDist = max(MaxDist, Dist1);
						MaxDist = max(MaxDist, Dist3);
						MinDist = min(Dist2, Dist4);
						MinDist = min(MinDist, Dist1);
						MinDist = min(MinDist, Dist3);
						} // if
					else
						{
						MaxDist = max(Dist2, Dist4);
						MinDist = min(Dist2, Dist4);
						} // else
					AdaptiveThreshDist = MaxDist * ThreshScale;
					if (MinDist <= AdaptiveThreshDist)
						Adaptive = 1;
					else
						{
						Dist2 = PrevFinalT[XSample];
						Dist4 = CurFinalT[XSample];
						if (XSample > 0)
							{
							Dist1 = PrevFinalT[XSample - 1];
							Dist3 = CurFinalT[XSample - 1];
							MaxDist = max(Dist2, Dist4);
							MaxDist = max(MaxDist, Dist1);
							MaxDist = max(MaxDist, Dist3);
							MinDist = min(Dist2, Dist4);
							MinDist = min(MinDist, Dist1);
							MinDist = min(MinDist, Dist3);
							} // if
						else
							{
							MaxDist = max(Dist2, Dist4);
							MinDist = min(Dist2, Dist4);
							} // else
						AdaptiveTransDist = MaxDist - MinDist;
						if (AdaptiveTransDist >= TransThresh)
							Adaptive = 1;
						} // else check transmission difference
					// fill in spaces above
					for (InterpZipY = PrevY + 1, InterpY = InterpIntervalY; InterpZipY < Y; InterpZipY ++, InterpY += InterpIntervalY)
						{
						InvInterpY = 1.0 - InterpY;
						for (InterpZipX = X > 0 ? PrevX + 1: 0, InterpX = InterpIntervalX; InterpZipX <= X; InterpZipX ++, InterpX += InterpIntervalX)
							{
							//if (InterpZipX < 0)
							//	continue;
							InterpZip = InterpZipY * Width + InterpZipX;
							TempAdaptive = 0;
							if (! Adaptive)
								{
								// might have roundoff error
								if ((InvInterpX = 1.0 - InterpX) < 0.0)
									InvInterpX = 0.0;
								// for each fragment in InterpZip find corresponding transmission and color
								TopMask = BotMask = 0UL;
								TestAlpha = 0;
								FragPlot = 0;
								FirstFrag = 1;
								// for each fragment in InterpZip find corresponding transmission and color
								if (rPixelFragMap)
									{
									CurFrag = rPixelFragMap[InterpZip].FragList;
									} // if
								else
									{
									CurFrag = NULL;
									} // else
								while ((CurFrag || ! rPixelFragMap))
									{
									// find distance to sample
									if (CurFrag)
										{
										FragVert.ScrnXYZ[2] = FragVert.Q = CurFrag->ZBuf;
										} // if
									else
										{
										FragVert.ScrnXYZ[2] = FragVert.Q = ZBuf[InterpZip];
										} // else
									// unproject z to actual distance
									Cam->UnProjectVertexDEM(DefCoords, &FragVert, EarthLatScaleMeters, PlanetRad, 0);	// 0 = don't need lat/lon

									// find the pos vector for the pixel point from the camera, this is the view vector for that pixel
									FragVert.GetPosVector(Cam->CamPos);
									EndFragDist = VectorMagnitude(FragVert.XYZ);
									if (FirstFrag && (EndFragDist <= AdaptiveThreshDist))
										{
										TempAdaptive = 1;
										break;
										} // if

									// analyze corner points for transmission and color
									InterpTransmission[0] = XSample <= 0 ? 1.0: PrevTrans[XSample - 1].Graph[0] ? PrevTrans[XSample - 1].Graph[0]->GetValue(EndFragDist): 1.0;
									InterpTransmission[1] = PrevTrans[XSample].Graph[0] ? PrevTrans[XSample].Graph[0]->GetValue(EndFragDist): 1.0;
									InterpTransmission[2] = XSample <= 0 ? 1.0: CurTrans[XSample - 1].Graph[0] ? CurTrans[XSample - 1].Graph[0]->GetValue(EndFragDist): 1.0;
									InterpTransmission[3] = CurTrans[XSample].Graph[0] ? CurTrans[XSample].Graph[0]->GetValue(EndFragDist): 1.0;
									Transmission = InterpTransmission[0] * InvInterpX * InvInterpY
										+ InterpTransmission[1] * InterpX * InvInterpY
										+ InterpTransmission[2] * InvInterpX * InterpY
										+ InterpTransmission[3] * InterpX * InterpY;

									if (Transmission < .9999)	// allow for a bit of math imprecision
										{
										if (CurFrag)
											CurFrag->ExtractColor(FragColor);
										else
											{
											FragColor[0] = Bitmap[0][InterpZip] * (1.0 / 255.0);
											FragColor[1] = Bitmap[1][InterpZip] * (1.0 / 255.0);
											FragColor[2] = Bitmap[2][InterpZip] * (1.0 / 255.0);
											} // else

										InterpColor[0][0] = XSample <= 0 ? 0.0: PrevColor[XSample - 1].Graph[0] ? PrevColor[XSample - 1].Graph[0]->GetValue(EndFragDist): 0.0;
										InterpColor[1][0] = PrevColor[XSample].Graph[0] ? PrevColor[XSample].Graph[0]->GetValue(EndFragDist): 0.0;
										InterpColor[2][0] = XSample <= 0 ? 0.0: CurColor[XSample - 1].Graph[0] ? CurColor[XSample - 1].Graph[0]->GetValue(EndFragDist): 0.0;
										InterpColor[3][0] = CurColor[XSample].Graph[0] ? CurColor[XSample].Graph[0]->GetValue(EndFragDist): 0.0;

										InterpColor[0][1] = XSample <= 0 ? 0.0: PrevColor[XSample - 1].Graph[1] ? PrevColor[XSample - 1].Graph[1]->GetValue(EndFragDist): 0.0;
										InterpColor[1][1] = PrevColor[XSample].Graph[1] ? PrevColor[XSample].Graph[1]->GetValue(EndFragDist): 0.0;
										InterpColor[2][1] = XSample <= 0 ? 0.0: CurColor[XSample - 1].Graph[1] ? CurColor[XSample - 1].Graph[1]->GetValue(EndFragDist): 0.0;
										InterpColor[3][1] = CurColor[XSample].Graph[1] ? CurColor[XSample].Graph[1]->GetValue(EndFragDist): 0.0;

										InterpColor[0][2] = XSample <= 0 ? 0.0: PrevColor[XSample - 1].Graph[2] ? PrevColor[XSample - 1].Graph[2]->GetValue(EndFragDist): 0.0;
										InterpColor[1][2] = PrevColor[XSample].Graph[2] ? PrevColor[XSample].Graph[2]->GetValue(EndFragDist): 0.0;
										InterpColor[2][2] = XSample <= 0 ? 0.0: CurColor[XSample - 1].Graph[2] ? CurColor[XSample - 1].Graph[2]->GetValue(EndFragDist): 0.0;
										InterpColor[3][2] = CurColor[XSample].Graph[2] ? CurColor[XSample].Graph[2]->GetValue(EndFragDist): 0.0;

										SumColor[0] = InterpColor[0][0] * InvInterpX * InvInterpY
											+ InterpColor[1][0] * InterpX * InvInterpY
											+ InterpColor[2][0] * InvInterpX * InterpY
											+ InterpColor[3][0] * InterpX * InterpY;
										SumColor[1] = InterpColor[0][1] * InvInterpX * InvInterpY
											+ InterpColor[1][1] * InterpX * InvInterpY
											+ InterpColor[2][1] * InvInterpX * InterpY
											+ InterpColor[3][1] * InterpX * InterpY;
										SumColor[2] = InterpColor[0][2] * InvInterpX * InvInterpY
											+ InterpColor[1][2] * InterpX * InvInterpY
											+ InterpColor[2][2] * InvInterpX * InterpY
											+ InterpColor[3][2] * InterpX * InterpY;

										FragColor[0] = SumColor[0] + FragColor[0] * Transmission;
										FragColor[1] = SumColor[1] + FragColor[1] * Transmission;
										FragColor[2] = SumColor[2] + FragColor[2] * Transmission;
										} // if
									if (CurFrag)
										{
										FirstFrag = 0;
										if (Transmission < .9999)
											{
											CurFrag->PlotPixel(FragColor);
											if (CurFrag->Refl)
												CurFrag->Refl->Reflect = (unsigned short)(CurFrag->Refl->Reflect * Transmission);
											FragPlot = 1;
											} // if
										LastFrag = CurFrag;
										while (CurFrag = CurFrag->Next)
											{
											// test fragment visibility
											if (CurFrag->TestVisible(LastFrag, TopMask, BotMask, TestAlpha))
												break;
											LastFrag = CurFrag;
											} // while
										} // if
									else
										{
										if (Transmission < .9999)
											{
											if (FragColor[0] > 1.0)
												FragColor[0] = 1.0;
											if (FragColor[1] > 1.0)
												FragColor[1] = 1.0;
											if (FragColor[2] > 1.0)
												FragColor[2] = 1.0;
											Bitmap[0][InterpZip] = (unsigned char)(FragColor[0] * 255.9);
											Bitmap[1][InterpZip] = (unsigned char)(FragColor[1] * 255.9);
											Bitmap[2][InterpZip] = (unsigned char)(FragColor[2] * 255.9);
											if (ReflectionBuf && ReflectionBuf[InterpZip])
												ReflectionBuf[InterpZip] = (float)(ReflectionBuf[InterpZip] * Transmission);
											ScreenPixelPlot(Bitmap, InterpZipX + DrawOffsetX, InterpZipY + DrawOffsetY, InterpZip);
											} // if
										break;
										} // else
									} // while
								if (FragPlot)
									ScreenPixelPlotFragments(&rPixelFragMap[InterpZip], InterpZipX + DrawOffsetX, InterpZipY + DrawOffsetY);	//lint !e794
								} // if ! adaptive
							if (Adaptive || TempAdaptive)
								{
								SampleVolumetricRay(InterpZipX, InterpZipY, NULL, NULL, NULL,
									&TexData, &PixData, &ViewVec, &FragVert, &SubVert, BeforeReflections);
								} // else adaptive
							} // for
						} // for
					} // if need to interpolate
				} // if
			if (XSample > 0)
				{
				if (CurTrans[XSample - 1].Graph[0] || CurTrans[XSample].Graph[0])
					{
					Adaptive = 0;
					Dist1 = CurTrans[XSample - 1].Graph[0] ? CurTrans[XSample - 1].Graph[0]->GetFirstDist(): 0.0;
					Dist2 = CurTrans[XSample].Graph[0] ? CurTrans[XSample].Graph[0]->GetFirstDist(): 0.0;
					MaxDist = max(Dist1, Dist2);
					MinDist = min(Dist1, Dist2);
					AdaptiveThreshDist = MaxDist * ThreshScale;
					if (MinDist <= AdaptiveThreshDist)
						Adaptive = 1;
					else
						{
						Dist1 = CurFinalT[XSample - 1];
						Dist2 = CurFinalT[XSample];
						MaxDist = max(Dist1, Dist2);
						MinDist = min(Dist1, Dist2);
						AdaptiveTransDist = MaxDist - MinDist;
						if (AdaptiveTransDist >= TransThresh)
							Adaptive = 1;
						} // else
					// fill in spaces between
					InterpZipY = Y;
					InterpY = 1.0;
					for (InterpZipX = PrevX + 1, InterpX = InterpIntervalX; InterpZipX < X; InterpZipX ++, InterpX += InterpIntervalX)
						{
						InterpZip = InterpZipY * Width + InterpZipX;
						TempAdaptive = 0;
						if (! Adaptive)
							{
							InvInterpX = 1.0 - InterpX;
							TopMask = BotMask = 0UL;
							TestAlpha = 0;
							FragPlot = 0;
							FirstFrag = 1;
							// for each fragment in InterpZip find corresponding transmission and color
							if (rPixelFragMap)
								{
								CurFrag = rPixelFragMap[InterpZip].FragList;
								} // if
							else
								{
								CurFrag = NULL;
								} // else
							while ((CurFrag || ! rPixelFragMap))
								{
								// find distance to sample
								if (CurFrag)
									{
									FragVert.ScrnXYZ[2] = FragVert.Q = CurFrag->ZBuf;
									} // if
								else
									{
									FragVert.ScrnXYZ[2] = FragVert.Q = ZBuf[InterpZip];
									} // else
								// unproject z to actual distance
								Cam->UnProjectVertexDEM(DefCoords, &FragVert, EarthLatScaleMeters, PlanetRad, 0);	// 0 = don't need lat/lon

								// find the pos vector for the pixel point from the camera, this is the view vector for that pixel
								FragVert.GetPosVector(Cam->CamPos);
								EndFragDist = VectorMagnitude(FragVert.XYZ);
								if (FirstFrag && (EndFragDist <= AdaptiveThreshDist))
									{
									TempAdaptive = 1;
									break;
									} // if

								// analyze corner points for transmission and color
								InterpTransmission[0] = CurTrans[XSample - 1].Graph[0] ? CurTrans[XSample - 1].Graph[0]->GetValue(EndFragDist): 1.0;
								InterpTransmission[1] = CurTrans[XSample].Graph[0] ? CurTrans[XSample].Graph[0]->GetValue(EndFragDist): 1.0;
								Transmission = InterpTransmission[0] * InvInterpX + InterpTransmission[1] * InterpX;

								if (Transmission < 1.0)
									{
									if (CurFrag)
										CurFrag->ExtractColor(FragColor);
									else
										{
										FragColor[0] = Bitmap[0][InterpZip] * (1.0 / 255.0);
										FragColor[1] = Bitmap[1][InterpZip] * (1.0 / 255.0);
										FragColor[2] = Bitmap[2][InterpZip] * (1.0 / 255.0);
										} // else

									InterpColor[0][0] = CurColor[XSample - 1].Graph[0] ? CurColor[XSample - 1].Graph[0]->GetValue(EndFragDist): 0.0;
									InterpColor[1][0] = CurColor[XSample].Graph[0] ? CurColor[XSample].Graph[0]->GetValue(EndFragDist): 0.0;
									InterpColor[0][1] = CurColor[XSample - 1].Graph[1] ? CurColor[XSample - 1].Graph[1]->GetValue(EndFragDist): 0.0;
									InterpColor[1][1] = CurColor[XSample].Graph[1] ? CurColor[XSample].Graph[1]->GetValue(EndFragDist): 0.0;
									InterpColor[0][2] = CurColor[XSample - 1].Graph[2] ? CurColor[XSample - 1].Graph[2]->GetValue(EndFragDist): 0.0;
									InterpColor[1][2] = CurColor[XSample].Graph[2] ? CurColor[XSample].Graph[2]->GetValue(EndFragDist): 0.0;
									SumColor[0] = InterpColor[0][0] * InvInterpX + InterpColor[1][0] * InterpX;
									SumColor[1] = InterpColor[0][1] * InvInterpX + InterpColor[1][1] * InterpX;
									SumColor[2] = InterpColor[0][2] * InvInterpX + InterpColor[1][2] * InterpX;

									FragColor[0] = SumColor[0] + FragColor[0] * Transmission;
									FragColor[1] = SumColor[1] + FragColor[1] * Transmission;
									FragColor[2] = SumColor[2] + FragColor[2] * Transmission;
									} // if
								if (CurFrag)
									{
									FirstFrag = 0;
									if (Transmission < 1.0)
										{
										CurFrag->PlotPixel(FragColor);
										if (CurFrag->Refl)
											CurFrag->Refl->Reflect = (unsigned short)(CurFrag->Refl->Reflect * Transmission);
										FragPlot = 1;
										} // if
									LastFrag = CurFrag;
									while (CurFrag = CurFrag->Next)
										{
										// test fragment visibility
										if (CurFrag->TestVisible(LastFrag, TopMask, BotMask, TestAlpha))
											break;
										LastFrag = CurFrag;
										} // while
									} // if
								else
									{
									if (Transmission < 1.0)
										{
										if (FragColor[0] > 1.0)
											FragColor[0] = 1.0;
										if (FragColor[1] > 1.0)
											FragColor[1] = 1.0;
										if (FragColor[2] > 1.0)
											FragColor[2] = 1.0;
										Bitmap[0][InterpZip] = (unsigned char)(FragColor[0] * 255.9);
										Bitmap[1][InterpZip] = (unsigned char)(FragColor[1] * 255.9);
										Bitmap[2][InterpZip] = (unsigned char)(FragColor[2] * 255.9);
										if (ReflectionBuf && ReflectionBuf[InterpZip])
											ReflectionBuf[InterpZip] = (float)(ReflectionBuf[InterpZip] * Transmission);
										ScreenPixelPlot(Bitmap, InterpZipX + DrawOffsetX, InterpZipY + DrawOffsetY, InterpZip);
										} // if
									break;
									} // else
								} // while
							if (FragPlot)
								ScreenPixelPlotFragments(&rPixelFragMap[InterpZip], InterpZipX + DrawOffsetX, InterpZipY + DrawOffsetY);	//lint !e794
							} // if ! adaptive
						if (Adaptive || TempAdaptive)
							{
							SampleVolumetricRay(InterpZipX, InterpZipY, NULL, NULL, NULL,
								&TexData, &PixData, &ViewVec, &FragVert, &SubVert, BeforeReflections);
							} // else
						} // for
					} // if need to interpolate
				} // if
			PrevX = X;
			} // for
		swmem(&CurTrans, &PrevTrans, sizeof (AnimDouble *));
		swmem(&CurColor, &PrevColor, sizeof (AnimColor *));
		swmem(&CurFinalT, &PrevFinalT, sizeof (float *));
		if (IsCamView)	// gotta have this distinction
			{
			if(BWDE->Update(Y + 1))
				{
				Success = 0;
				break;
				} // if
			} // if
		else
			{
			Master->ProcUpdate(Y + 1);	// calls Master->CheckAbort() - do M->CheckAbort() any time - it returns the Run variable
			if (! Master->IsRunning())
				{
				Success = 0;
				break;
				} // if
			} // else
		PrevY = Y;
		} // for
	} // if

if (Master)
	Master->ProcClear();
if (BWDE)
	delete BWDE;

if (TransmissionStash[0])
	delete [] TransmissionStash[0];
if (TransmissionStash[1])
	delete [] TransmissionStash[1];
if (ColorStash[0])
	delete [] ColorStash[0];
if (ColorStash[1])
	delete [] ColorStash[1];
if (FinalT[0])
	AppMem_Free(FinalT[0], NumXSamples * sizeof (float));
if (FinalT[1])
	AppMem_Free(FinalT[1], NumXSamples * sizeof (float));

return (Success);

} // Renderer::RenderVolumetricsSpeedBoost

/*===========================================================================*/

void Renderer::SampleVolumetricRay(long X, long Y, AnimDoubleTime *CurTrans, AnimColorTime *CurColor, float *CurFinalT,
	TextureData *TexData, PixelData *PixData, VertexDEM *ViewVec, VertexDEM *FragVert, VertexDEM *SubVert, 
	int BeforeReflections)
{
double Transmission, TotalTau, FragColor[3], TempColor[3], SumColor[3], 
	StartFragDist, EndFragDist, MinDist, MaxDist, SegSubLen, Tau, Shading;
unsigned long TopMask, BotMask, TestAlpha;
long PixZip, FragPlot, ComputeVolumetrics;
VolumetricSubstance *CurSub;
rPixelFragment *CurFrag, *LastFrag;

Transmission = 1.0;
PixZip = Y * Width + X;
if (CurTrans)
	{
	CurTrans->ReleaseNodes();
	CurColor->ReleaseNodes();
	*CurFinalT = 1.0f;
	} // if
// analyze current pixel
// texture screen coords
TexData->PixelX[0] = (double)X + SegmentOffsetX - (double)Width * Cam->PanoPanels * .5;
TexData->PixelX[1] = (double)(X + 1) + SegmentOffsetX - (double)Width * Cam->PanoPanels * .5;
TexData->PixelY[0] = -((double)Y + SegmentOffsetY) + (double)Height * Opt->RenderImageSegments * .5;
TexData->PixelY[1] = -((double)(Y + 1) + SegmentOffsetY) + (double)Height * Opt->RenderImageSegments * .5;
TexData->PixelUnityX[0] = TexData->PixelX[0] / (Width * Cam->PanoPanels);
TexData->PixelUnityX[1] = TexData->PixelX[1] / (Width * Cam->PanoPanels);
TexData->PixelUnityY[0] = TexData->PixelY[0] / (Height * Opt->RenderImageSegments);
TexData->PixelUnityY[1] = TexData->PixelY[1] / (Height * Opt->RenderImageSegments);

// define pixel ray
ViewVec->ScrnXYZ[0] = X + .5;
ViewVec->ScrnXYZ[1] = Y + .5;
ViewVec->ScrnXYZ[2] = ViewVec->Q = 1.0;

// turn screen coords into world coord xyz
Cam->UnProjectVertexDEM(DefCoords, ViewVec, EarthLatScaleMeters, PlanetRad, 0);	// 0 = don't need lat/lon

// find the pos vector for the pixel point from the camera, this is the view vector for that pixel
ViewVec->GetPosVector(Cam->CamPos);
ViewVec->UnitVector();

// build an on/off list that includes all volumetric things
// determine an initial sample interval for each volumetric thing
CurSub = Substances;
ComputeVolumetrics = 0;
while (CurSub)
	{
	CurSub->DeleteRayList();
	CurSub->CurRayList = NULL;
	if (CurSub->GetRenderVolumetric() && CurSub->GetBeforeReflections() == BeforeReflections)
		{
		if (CurSub->RayList = CurSub->BuildRayList(DefCoords, PlanetRad, Cam->CamPos->XYZ, ViewVec->XYZ))
			{
			ComputeVolumetrics = 1;
			CurSub->CurRayList = CurSub->RayList;
			} // if
		} // if
	CurSub = CurSub->NextSub;
	} // while

if (ComputeVolumetrics)
	{
	FragPlot = 0;
	// for each visible fragment if fragments or the closest z distance if not frags
	if (rPixelFragMap)
		{
		CurFrag = rPixelFragMap[PixZip].FragList;
		} // if
	else
		{
		CurFrag = NULL;
		} // else

	// walk along ray segment and at specified intervals
	TotalTau = 0.0;
	SumColor[0] = SumColor[1] = SumColor[2] = 0.0;
	TopMask = BotMask = 0UL;
	TestAlpha = 0;
	EndFragDist = 0.0;
	FragVert->ScrnXYZ[0] = X + .5;
	FragVert->ScrnXYZ[1] = Y + .5;
	while ((CurFrag || ! rPixelFragMap))// && TotalTau < 6.9077)
		{
		// see if we need any more volume evaluation or if we just need to update the rest of the fragments in the chain
		if (TotalTau < 6.9077)
			{
			StartFragDist = EndFragDist;
			if (CurFrag)
				{
				FragVert->ScrnXYZ[2] = FragVert->Q = CurFrag->ZBuf;
				} // if
			else
				{
				FragVert->ScrnXYZ[2] = FragVert->Q = ZBuf[PixZip];
				} // else
			// unproject z to actual distance
			Cam->UnProjectVertexDEM(DefCoords, FragVert, EarthLatScaleMeters, PlanetRad, 0);	// 0 = don't need lat/lon

			// find the pos vector for the pixel point from the camera, this is the view vector for that pixel
			FragVert->GetPosVector(Cam->CamPos);
			EndFragDist = VectorMagnitude(FragVert->XYZ);

			// for each thing that is "on" for all or part of the ray segment
			CurSub = Substances;
			while (CurSub && TotalTau < 6.9077)
				{
				PixData->ShadowFlags = CurSub->GetShadowFlags();
				PixData->ReceivedShadowIntensity = CurSub->GetShadowIntenstiy();
				while (CurSub->CurRayList && CurSub->CurRayList->SegStartDist < EndFragDist && TotalTau < 6.9077)
					{
					if (! CurSub->CurRayList->SegEvaluated)
						{
						// evaluate density and color including shading for the segment at the center of segment
						SubVert->XYZ[0] = CurSub->CurRayList->SegMidXYZ[0];
						SubVert->XYZ[1] = CurSub->CurRayList->SegMidXYZ[1];
						SubVert->XYZ[2] = CurSub->CurRayList->SegMidXYZ[2];
						// project to screen coords and convert to lat/lon - textures may use z distance
						#ifdef WCS_BUILD_VNS
						DefCoords->CartToDeg(SubVert);
						#else // WCS_BUILD_VNS
						SubVert->CartToDeg(PlanetRad);
						#endif // WCS_BUILD_VNS
						Cam->ProjectVertexDEM(DefCoords, SubVert, EarthLatScaleMeters, PlanetRad, 0);
						// evaluate volumetric optical depth and color
						// set the values CurSub->CurRayList->SegOpticalDepth and CurSub->CurRayList->SegColor[3].
						CurSub->PointSampleVolumetric(CurSub->CurRayList, SubVert, TexData, Shading);
						// get shading %, modify color
						if (CurSub->CurRayList->SegOpticalDepth < FLT_MAX)
							{
							if ((Shading > 0.0 && (PixData->ShadowFlags & WCS_SHADOWTYPE_VOLUME)) || 
								(PixData->ReceivedShadowIntensity > 0.0 && (PixData->ShadowFlags & ~WCS_SHADOWTYPE_VOLUME)))
								{
								IlluminateVolumetric(CurSub, TexData, PixData, SubVert, CurSub->CurRayList->SegColor, Shading);
								} // if
							else
								{
								CurSub->CurRayList->SegColor[0] *= (1.0 - Shading);
								CurSub->CurRayList->SegColor[1] *= (1.0 - Shading);
								CurSub->CurRayList->SegColor[2] *= (1.0 - Shading);
								} // else
							} // if
						CurSub->CurRayList->SegEvaluated = 1;
						} // if
					// augment attenuation and color
					// keep a running accumulation of pass-through attenuation and color which will be added 
					//   to attenuated fragment color.
					// find length of this substances contribution to this fragment segment
					if (CurSub->CurRayList->SegOpticalDepth < FLT_MAX)
						{
						MinDist = max(StartFragDist, CurSub->CurRayList->SegStartDist);
						MaxDist = min(EndFragDist, CurSub->CurRayList->SegEndDist);
						SegSubLen = MaxDist - MinDist;
						// diminish the transmission to 1% at optical depth distance
						Tau = SegSubLen * 4.605 / CurSub->CurRayList->SegOpticalDepth;

						// if the exponent argument gets too large it causes float underflow and contributes negligibly
						// 6.9077 corresponds with a transmission result of .001 which is tiny
						if (Tau > 6.9077)
							Transmission = 0.0;
						else
							Transmission = exp(-Tau);
						// multiply transmission by fragment weight and color
						// sum fragment contributions
						TempColor[0] = CurSub->CurRayList->SegColor[0] * (1.0 - Transmission);
						TempColor[1] = CurSub->CurRayList->SegColor[1] * (1.0 - Transmission);
						TempColor[2] = CurSub->CurRayList->SegColor[2] * (1.0 - Transmission);
						Transmission = exp(-TotalTau);
						SumColor[0] += TempColor[0] * Transmission;
						SumColor[1] += TempColor[1] * Transmission;
						SumColor[2] += TempColor[2] * Transmission;
						TotalTau += Tau;
						} // if

					// increment segments if segment end is less than fragment end
					if (CurSub->CurRayList->SegEndDist <= EndFragDist)
						{
						if (CurSub->CurRayList->SegEndDist >= CurSub->CurRayList->RayOffDist)
							CurSub->CurRayList = CurSub->CurRayList->Next;
						else
							{
							CurSub->CurRayList->SegStartDist = CurSub->CurRayList->SegEndDist;
							CurSub->CurRayList->SegEndDist = WCS_min(CurSub->CurRayList->SegEndDist + CurSub->CurRayList->SampleRate, CurSub->CurRayList->RayOffDist);
							CurSub->CurRayList->SegMidXYZ[0] += CurSub->CurRayList->SampleRate * ViewVec->XYZ[0];
							CurSub->CurRayList->SegMidXYZ[1] += CurSub->CurRayList->SampleRate * ViewVec->XYZ[1];
							CurSub->CurRayList->SegMidXYZ[2] += CurSub->CurRayList->SampleRate * ViewVec->XYZ[2];
							CurSub->CurRayList->SegEvaluated = 0;
							} // else
						} // if
					else
						break;
					} // while
				CurSub = CurSub->NextSub;
				} // while
			} // if
		if (TotalTau > 0.0)
			{
			if (CurFrag)
				CurFrag->ExtractColor(FragColor);
			else
				{
				FragColor[0] = Bitmap[0][PixZip] * (1.0 / 255.0);
				FragColor[1] = Bitmap[1][PixZip] * (1.0 / 255.0);
				FragColor[2] = Bitmap[2][PixZip] * (1.0 / 255.0);
				} // else

			if (TotalTau > 6.9077)
				Transmission = 0.0;
			else
				Transmission = exp(-TotalTau);
			FragColor[0] = SumColor[0] + FragColor[0] * Transmission;
			FragColor[1] = SumColor[1] + FragColor[1] * Transmission;
			FragColor[2] = SumColor[2] + FragColor[2] * Transmission;
			} // if
		else
			Transmission = 1.0;
		// add graph nodes even if 
		if (CurTrans)
			{
			CurTrans->AddNode(EndFragDist, Transmission, 0.0);
			CurColor->AddNode(EndFragDist, SumColor, 0.0);
			} // if

		if (CurFrag)
			{
			if (TotalTau > 0.0)
				{
				CurFrag->PlotPixel(FragColor);
				if (CurFrag->Refl)
					CurFrag->Refl->Reflect = (unsigned short)(CurFrag->Refl->Reflect * Transmission);
				FragPlot = 1;
				} // if
			LastFrag = CurFrag;
			while (CurFrag = CurFrag->Next)
				{
				// test fragment visibility
				if (CurFrag->TestVisible(LastFrag, TopMask, BotMask, TestAlpha))
					break;
				LastFrag = CurFrag;
				} // while
			} // if
		else
			{
			if (TotalTau > 0.0)
				{
				if (FragColor[0] > 1.0)
					FragColor[0] = 1.0;
				if (FragColor[1] > 1.0)
					FragColor[1] = 1.0;
				if (FragColor[2] > 1.0)
					FragColor[2] = 1.0;
				Bitmap[0][PixZip] = (unsigned char)(FragColor[0] * 255.9);
				Bitmap[1][PixZip] = (unsigned char)(FragColor[1] * 255.9);
				Bitmap[2][PixZip] = (unsigned char)(FragColor[2] * 255.9);
				if (ReflectionBuf && ReflectionBuf[PixZip])
					ReflectionBuf[PixZip] = (float)(ReflectionBuf[PixZip] * Transmission);
				ScreenPixelPlot(Bitmap, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
				} // if
			break;
			} // else
		} // while
	if (FragPlot)
		ScreenPixelPlotFragments(&rPixelFragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);	//lint !e794
	if (CurTrans)
		{
		CurTrans->SetNodeLinearAll(1);
		CurColor->SetNodeLinearAll(1);
		*CurFinalT = (float)Transmission;
		} // if
	} // if at least one volumetric substance found along pixel's ray path

} // Renderer::SampleVolumetricRay

/*===========================================================================*/

int Renderer::RenderVolumetricsNoBoost(int BeforeReflections)
{
double StartFragDist, EndFragDist, MinDist, MaxDist, SegSubLen, Tau, Transmission, TotalTau, 
	SumColor[3], TempColor[3], FragColor[3], Shading;
long X, Y, PixZip, ComputeVolumetrics, FragPlot;
int Success = 1;
unsigned long TopMask, BotMask, TestAlpha;
VolumetricSubstance *CurSub;
rPixelFragment *CurFrag, *LastFrag;
BusyWin *BWDE = NULL;
VertexDEM ViewVec, FragVert, SubVert;
TextureData TexData;
PixelData PixData;

if (! rPixelFragMap && ! (Bitmap[0] && Bitmap[1] && Bitmap[2] && ZBuf))
	return (1);

// see if there is anything to do in this pass
ComputeVolumetrics = 0;
CurSub = Substances;
while (CurSub)
	{
	if (CurSub->GetRenderVolumetric() && CurSub->GetBeforeReflections() == BeforeReflections)
		ComputeVolumetrics = 1;
	CurSub = CurSub->NextSub;
	} // while
if (! ComputeVolumetrics)
	return (1);

if (IsCamView)
	{
	BWDE = new BusyWin("Volumetrics", Height, 'BWDE', 0);
	} // if
else
	{
	Master->ProcInit(Height, "Volumetrics");
	} // else

PixData.TexData = &TexData;

// for each screen pixel
for (Y = PixZip = 0; Y < Height; Y ++)
	{
	for (X = 0; X < Width; X ++, PixZip ++)
		{
		// texture screen coords
		TexData.PixelX[0] = (double)X + SegmentOffsetX - Width * Cam->PanoPanels * .5;
		TexData.PixelX[1] = (double)(X + 1) + SegmentOffsetX - Width * Cam->PanoPanels * .5;
		TexData.PixelY[0] = -((double)Y + SegmentOffsetY) + Height * Opt->RenderImageSegments * .5;
		TexData.PixelY[1] = -((double)(Y + 1) + SegmentOffsetY) + Height * Opt->RenderImageSegments * .5;
		TexData.PixelUnityX[0] = TexData.PixelX[0] / (Width * Cam->PanoPanels);
		TexData.PixelUnityX[1] = TexData.PixelX[1] / (Width * Cam->PanoPanels);
		TexData.PixelUnityY[0] = TexData.PixelY[0] / (Height * Opt->RenderImageSegments);
		TexData.PixelUnityY[1] = TexData.PixelY[1] / (Height * Opt->RenderImageSegments);

		// define pixel ray
		ViewVec.ScrnXYZ[0] = X + .5;
		ViewVec.ScrnXYZ[1] = Y + .5;
		ViewVec.ScrnXYZ[2] = ViewVec.Q = 1.0;

		// turn screen coords into world coord xyz
		Cam->UnProjectVertexDEM(DefCoords, &ViewVec, EarthLatScaleMeters, PlanetRad, 0);	// 0 = don't need lat/lon

		// find the pos vector for the pixel point from the camera, this is the view vector for that pixel
		ViewVec.GetPosVector(Cam->CamPos);
		ViewVec.UnitVector();

		// build an on/off list that includes all volumetric things
		// determine an initial sample interval for each volumetric thing
		CurSub = Substances;
		ComputeVolumetrics = 0;
		while (CurSub)
			{
			CurSub->DeleteRayList();
			CurSub->CurRayList = NULL;
			if (CurSub->GetRenderVolumetric() && CurSub->GetBeforeReflections() == BeforeReflections)
				{
				if (CurSub->RayList = CurSub->BuildRayList(DefCoords, PlanetRad, Cam->CamPos->XYZ, ViewVec.XYZ))
					{
					ComputeVolumetrics = 1;
					CurSub->CurRayList = CurSub->RayList;
					} // if
				} // if
			CurSub = CurSub->NextSub;
			} // while
		if (ComputeVolumetrics)
			{
			FragPlot = 0;
			// for each visible fragment if fragments or the closest z distance if not frags
			if (rPixelFragMap)
				{
				CurFrag = rPixelFragMap[PixZip].FragList;
				} // if
			else
				{
				CurFrag = NULL;
				} // else

			// walk along ray segment and at specified intervals
			TotalTau = 0.0;
			SumColor[0] = SumColor[1] = SumColor[2] = 0.0;
			TopMask = BotMask = 0UL;
			TestAlpha = 0;
			EndFragDist = 0.0;
			FragVert.ScrnXYZ[0] = X + .5;
			FragVert.ScrnXYZ[1] = Y + .5;
			while ((CurFrag || ! rPixelFragMap))// && TotalTau < 6.9077)
				{
				// see if we need any more volume evaluation or if we just need to update the rest of the fragments in the chain
				if (TotalTau < 6.9077)
					{
					StartFragDist = EndFragDist;
					if (CurFrag)
						{
						FragVert.ScrnXYZ[2] = FragVert.Q = CurFrag->ZBuf;
						} // if
					else
						{
						FragVert.ScrnXYZ[2] = FragVert.Q = ZBuf[PixZip];
						} // else
					// unproject z to actual distance
					Cam->UnProjectVertexDEM(DefCoords, &FragVert, EarthLatScaleMeters, PlanetRad, 0);	// 0 = don't need lat/lon

					// find the pos vector for the pixel point from the camera, this is the view vector for that pixel
					FragVert.GetPosVector(Cam->CamPos);
					EndFragDist = VectorMagnitude(FragVert.XYZ);

					// for each thing that is "on" for all or part of the ray segment
					CurSub = Substances;
					while (CurSub && TotalTau < 6.9077)
						{
						PixData.ShadowFlags = CurSub->GetShadowFlags();
						PixData.ReceivedShadowIntensity = CurSub->GetShadowIntenstiy();
						while (CurSub->CurRayList && CurSub->CurRayList->SegStartDist < EndFragDist && TotalTau < 6.9077)
							{
							if (! CurSub->CurRayList->SegEvaluated)
								{
								// evaluate density and color including shading for the segment at the center of segment
								SubVert.XYZ[0] = CurSub->CurRayList->SegMidXYZ[0];
								SubVert.XYZ[1] = CurSub->CurRayList->SegMidXYZ[1];
								SubVert.XYZ[2] = CurSub->CurRayList->SegMidXYZ[2];
								// project to screen coords and convert to lat/lon - textures may use z distance
								#ifdef WCS_BUILD_VNS
								DefCoords->CartToDeg(&SubVert);
								#else // WCS_BUILD_VNS
								SubVert.CartToDeg(PlanetRad);
								#endif // WCS_BUILD_VNS
								Cam->ProjectVertexDEM(DefCoords, &SubVert, EarthLatScaleMeters, PlanetRad, 0);
								// evaluate volumetric optical depth and color
								// set the values CurSub->CurRayList->SegOpticalDepth and CurSub->CurRayList->SegColor[3].
								CurSub->PointSampleVolumetric(CurSub->CurRayList, &SubVert, &TexData, Shading);
								// get shading %, modify color
								if (CurSub->CurRayList->SegOpticalDepth < FLT_MAX)
									{
									if ((Shading > 0.0 && (PixData.ShadowFlags & WCS_SHADOWTYPE_VOLUME)) || 
										(PixData.ReceivedShadowIntensity > 0.0 && (PixData.ShadowFlags & ~WCS_SHADOWTYPE_VOLUME)))
										{
										IlluminateVolumetric(CurSub, &TexData, &PixData, &SubVert, CurSub->CurRayList->SegColor, Shading);
										} // if
									else
										{
										CurSub->CurRayList->SegColor[0] *= (1.0 - Shading);
										CurSub->CurRayList->SegColor[1] *= (1.0 - Shading);
										CurSub->CurRayList->SegColor[2] *= (1.0 - Shading);
										} // else
									} // if
								CurSub->CurRayList->SegEvaluated = 1;
								} // if
							// augment attenuation and color
							// keep a running accumulation of pass-through attenuation and color which will be added 
							//   to attenuated fragment color.
							// find length of this substances contribution to this fragment segment
							if (CurSub->CurRayList->SegOpticalDepth < FLT_MAX)
								{
								MinDist = max(StartFragDist, CurSub->CurRayList->SegStartDist);
								MaxDist = min(EndFragDist, CurSub->CurRayList->SegEndDist);
								SegSubLen = MaxDist - MinDist;
								// diminish the transmission to 1% at optical depth distance
								Tau = SegSubLen * 4.605 / CurSub->CurRayList->SegOpticalDepth;

								// if the exponent argument gets too large it causes float underflow and contributes negligibly
								// 6.9077 corresponds with a transmission result of .001 which is tiny
								if (Tau > 6.9077)
									Transmission = 0.0;
								else
									Transmission = exp(-Tau);
								// multiply transmission by fragment weight and color
								// sum fragment contributions
								TempColor[0] = CurSub->CurRayList->SegColor[0] * (1.0 - Transmission);
								TempColor[1] = CurSub->CurRayList->SegColor[1] * (1.0 - Transmission);
								TempColor[2] = CurSub->CurRayList->SegColor[2] * (1.0 - Transmission);
								Transmission = exp(-TotalTau);
								SumColor[0] += TempColor[0] * Transmission;
								SumColor[1] += TempColor[1] * Transmission;
								SumColor[2] += TempColor[2] * Transmission;
								TotalTau += Tau;
								} // if

							// increment segments if segment end is less than fragment end
							if (CurSub->CurRayList->SegEndDist <= EndFragDist)
								{
								if (CurSub->CurRayList->SegEndDist >= CurSub->CurRayList->RayOffDist)
									CurSub->CurRayList = CurSub->CurRayList->Next;
								else
									{
									CurSub->CurRayList->SegStartDist = CurSub->CurRayList->SegEndDist;
									CurSub->CurRayList->SegEndDist = WCS_min(CurSub->CurRayList->SegEndDist + CurSub->CurRayList->SampleRate, CurSub->CurRayList->RayOffDist);
									CurSub->CurRayList->SegMidXYZ[0] += CurSub->CurRayList->SampleRate * ViewVec.XYZ[0];
									CurSub->CurRayList->SegMidXYZ[1] += CurSub->CurRayList->SampleRate * ViewVec.XYZ[1];
									CurSub->CurRayList->SegMidXYZ[2] += CurSub->CurRayList->SampleRate * ViewVec.XYZ[2];
									CurSub->CurRayList->SegEvaluated = 0;
									} // else
								} // if
							else
								break;
							} // while
						CurSub = CurSub->NextSub;
						} // while
					} // if
				if (TotalTau > 0.0)
					{
					if (CurFrag)
						CurFrag->ExtractColor(FragColor);
					else
						{
						FragColor[0] = Bitmap[0][PixZip] * (1.0 / 255.0);
						FragColor[1] = Bitmap[1][PixZip] * (1.0 / 255.0);
						FragColor[2] = Bitmap[2][PixZip] * (1.0 / 255.0);
						} // else

					if (TotalTau > 6.9077)
						Transmission = 0.0;
					else
						Transmission = exp(-TotalTau);
					FragColor[0] = SumColor[0] + FragColor[0] * Transmission;
					FragColor[1] = SumColor[1] + FragColor[1] * Transmission;
					FragColor[2] = SumColor[2] + FragColor[2] * Transmission;
					} // if

				if (CurFrag)
					{
					if (TotalTau > 0.0)
						{
						CurFrag->PlotPixel(FragColor);
						if (CurFrag->Refl)
							CurFrag->Refl->Reflect = (unsigned short)(CurFrag->Refl->Reflect * Transmission);
						FragPlot = 1;
						} // if
					LastFrag = CurFrag;
					while (CurFrag = CurFrag->Next)
						{
						// test fragment visibility
						if (CurFrag->TestVisible(LastFrag, TopMask, BotMask, TestAlpha))
							break;
						LastFrag = CurFrag;
						} // while
					} // if
				else
					{
					if (TotalTau > 0.0)
						{
						if (FragColor[0] > 1.0)
							FragColor[0] = 1.0;
						if (FragColor[1] > 1.0)
							FragColor[1] = 1.0;
						if (FragColor[2] > 1.0)
							FragColor[2] = 1.0;
						Bitmap[0][PixZip] = (unsigned char)(FragColor[0] * 255.9);
						Bitmap[1][PixZip] = (unsigned char)(FragColor[1] * 255.9);
						Bitmap[2][PixZip] = (unsigned char)(FragColor[2] * 255.9);
						if (ReflectionBuf && ReflectionBuf[PixZip])
							ReflectionBuf[PixZip] = (float)(ReflectionBuf[PixZip] * Transmission);
						ScreenPixelPlot(Bitmap, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
						} // if
					break;
					} // else
				} // while
			if (FragPlot)
				ScreenPixelPlotFragments(&rPixelFragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);	//lint !e794
			} // if at least one volumetric substance found along pixel's ray path
		} // for X

	if (IsCamView)	// gotta have this distinction
		{
		if(BWDE->Update(Y + 1))
			{
			Success = 0;
			break;
			} // if
		} // if
	else
		{
		Master->ProcUpdate(Y + 1);	// calls Master->CheckAbort() - do M->CheckAbort() any time - it returns the Run variable
		if (! Master->IsRunning())
			{
			Success = 0;
			break;
			} // if
		} // else
	} // for Y

if (Master)
	Master->ProcClear();
if (BWDE)
	delete BWDE;

return (Success);

} // Renderer::RenderVolumetrics

/*===========================================================================*/

double Renderer::IlluminateVolumetric(VolumetricSubstance *BacklightSub, 
	TextureData *TexData, PixelData *PixData, VertexDEM *PointPos, double PointRGB[3], double ShadingAllowed)
{
double OutRGB[3], IllumColor[3], LightVec[3], CamToPixVec[3], Tau, TotalTau, ShadowIllumination, LightDist, DummyDbl, MinDist, MaxDist, 
	SegSubLen, Transmission, BacklightPct, AngleBacklightPct, TotalBacklightPct, BacklightExp, AltColorBacklightPct,
	LightAngle, StraightBacklightPct, LightColorBacklightPct,
	BacklightColor[3];
int ComputeVolumetrics, StashType;
Light *CurLight;
VolumetricSubstance *CurSub;
RasterAnimHost *StashObject;
VertexDEM SubVert;

StashType = TexData->ObjectType;
StashObject = TexData->Object;

if (Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT])
	{
	OutRGB[0] = OutRGB[1] = OutRGB[2] = 0.0;
	// StraightBacklightPct accounts for forward scattering of light through atmospherics towards the point of interest
	StraightBacklightPct = 1.0;
	if (BacklightSub)
		StraightBacklightPct += BacklightSub->GetBacklightPct(); 
	
	// for each light
	CurLight = EffectsBase->Lights;
	while (CurLight)
		{
		if (CurLight->Enabled)
			{
			if (CurLight->GetEnabledTexture(WCS_EFFECTS_LIGHT_TEXTURE_COLOR))
				CurLight->ColorEvaluated = 0;
			// if illuminate volumetrics
			if (CurLight->IllumAtmosphere && PixData->IgnoreLight != CurLight && (! StashObject || CurLight->PassTest(StashObject)))
				{
				ComputeVolumetrics = 0;
				Transmission = 1.0;
				// find the falloff for the light at the point including shadows
				PixData->XYZ[0] = PointPos->XYZ[0];
				PixData->XYZ[1] = PointPos->XYZ[1];
				PixData->XYZ[2] = PointPos->XYZ[2];
				CamToPixVec[0] = PointPos->XYZ[0] - Cam->CamPos->XYZ[0];
				CamToPixVec[1] = PointPos->XYZ[1] - Cam->CamPos->XYZ[1];
				CamToPixVec[2] = PointPos->XYZ[2] - Cam->CamPos->XYZ[2];
				// avoid this call if possible - like if user has waived all rights to recieve shadows
				// from earth's umbra, spotlight cones or light falloff and shadow maps
				// if CurSub is NULL then this is not a volumetric sample and shadows have already been done elsewhere
				if (BacklightSub && (PixData->ShadowFlags & ~WCS_SHADOWTYPE_VOLUME))
					ShadowIllumination = IlluminateAtmoPixel(PixData, CurLight);
				else
					ShadowIllumination = 1.0;
				// if light > 0
				if (ShadowIllumination > 0.0)
					{
					IllumColor[0] = IllumColor[1] = IllumColor[2] = 0.0;
					if (! CurLight->ColorEvaluated && PixData->TexData)
						{
						// pixel texture data is used in light texture evaluation
						// PointPos contains XYZ, lat, lon, elev
						PixData->TexData->TexRefLon = TexRefLon;
						PixData->TexData->TexRefLat = TexRefLat;
						PixData->TexData->TexRefElev = TexRefElev;
						PixData->TexData->MetersPerDegLon = RefLonScaleMeters;
						PixData->TexData->MetersPerDegLat = EarthLatScaleMeters;
						PixData->TexData->VDEM[0] = PointPos;
						PixData->TexData->VDEM[1] = PixData->TexData->VDEM[2] = NULL;
						PixData->TexData->VData[0] = PixData->TexData->VData[1] = PixData->TexData->VData[2] = NULL;
						PixData->TexData->PData = NULL;
						PixData->TexData->Object = StashObject;
						PixData->TexData->ObjectType = StashType;
						PixData->TexData->ZDist = PointPos->ScrnXYZ[2];
						PixData->TexData->QDist = PointPos->Q;
						PixData->TexData->Elev = PointPos->Elev;
						PixData->TexData->Latitude = PointPos->Lat;
						PixData->TexData->Longitude = PointPos->Lon;

						CurLight->EvalColor(PixData);
						} // if
					// cast ray from vertex to light
					LightVec[0] = CurLight->LightPos->XYZ[0] - PointPos->XYZ[0];
					LightVec[1] = CurLight->LightPos->XYZ[1] - PointPos->XYZ[1];
					LightVec[2] = CurLight->LightPos->XYZ[2] - PointPos->XYZ[2];

					if (PixData->ShadowFlags & WCS_SHADOWTYPE_VOLUME)
						{
						TotalTau = 0.0;
						LightDist = VectorMagnitude(LightVec);
						UnitVector(LightVec);
						// build ray list
						CurSub = Substances;
						while (CurSub)
							{
							CurSub->DeleteShadowRayList();
							CurSub->CurShadowRayList = NULL;
							if (CurSub->GetCastVolumetricShadows())
								{
								if (CurSub->ShadowRayList = CurSub->BuildRayList(DefCoords, PlanetRad, PointPos->XYZ, LightVec))
									{
									ComputeVolumetrics = 1;
									CurSub->CurShadowRayList = CurSub->ShadowRayList;
									} // if
								} // if
							CurSub = CurSub->NextSub;
							} // while
						if (ComputeVolumetrics)
							{
							// walk ray list evaluating density
							// calculate the light color reaching the point
							// add light color to running total
							CurSub = Substances;
							while (CurSub && TotalTau < 6.9077)
								{
								while (CurSub->CurShadowRayList && CurSub->CurShadowRayList->SegStartDist < LightDist && TotalTau < 6.9077)
									{
									if (! CurSub->CurShadowRayList->SegEvaluated)
										{
										// evaluate density and color including shading for the segment at the center of segment
										SubVert.XYZ[0] = CurSub->CurShadowRayList->SegMidXYZ[0];
										SubVert.XYZ[1] = CurSub->CurShadowRayList->SegMidXYZ[1];
										SubVert.XYZ[2] = CurSub->CurShadowRayList->SegMidXYZ[2];
										// project to screen coords and convert to lat/lon - textures may use z distance
										#ifdef WCS_BUILD_VNS
										DefCoords->CartToDeg(&SubVert);
										#else // WCS_BUILD_VNS
										SubVert.CartToDeg(PlanetRad);
										#endif // WCS_BUILD_VNS
										Cam->ProjectVertexDEM(DefCoords, &SubVert, EarthLatScaleMeters, PlanetRad, 0);
										// evaluate volumetric optical depth and color
										// set the values CurSub->CurShadowRayList->SegOpticalDepth and CurSub->CurShadowRayList->SegColor[3].
										CurSub->PointSampleVolumetric(CurSub->CurShadowRayList, &SubVert, TexData, DummyDbl);
										CurSub->CurShadowRayList->SegEvaluated = 1;
										} // if
									// find length of this substances contribution to this fragment segment
									if (CurSub->CurShadowRayList->SegOpticalDepth < FLT_MAX)
										{
										MinDist = max(0.0, CurSub->CurShadowRayList->SegStartDist);
										MaxDist = min(LightDist, CurSub->CurShadowRayList->SegEndDist);
										SegSubLen = MaxDist - MinDist;
										// diminish the transmission to 1% at optical depth distance
										Tau = SegSubLen * 4.605 / CurSub->CurShadowRayList->SegOpticalDepth;
										TotalTau += Tau;
										} // if

									// increment segments if segment end is less than light distance
									if (CurSub->CurShadowRayList->SegEndDist < LightDist)
										{
										if (CurSub->CurShadowRayList->SegEndDist >= CurSub->CurShadowRayList->RayOffDist)
											CurSub->CurShadowRayList = CurSub->CurShadowRayList->Next;
										else
											{
											CurSub->CurShadowRayList->SegStartDist = CurSub->CurShadowRayList->SegEndDist;
											CurSub->CurShadowRayList->SegEndDist = WCS_min(CurSub->CurShadowRayList->SegEndDist + CurSub->CurShadowRayList->SampleRate, CurSub->CurShadowRayList->RayOffDist);
											CurSub->CurShadowRayList->SegMidXYZ[0] += CurSub->CurShadowRayList->SampleRate * LightVec[0];
											CurSub->CurShadowRayList->SegMidXYZ[1] += CurSub->CurShadowRayList->SampleRate * LightVec[1];
											CurSub->CurShadowRayList->SegMidXYZ[2] += CurSub->CurShadowRayList->SampleRate * LightVec[2];
											CurSub->CurShadowRayList->SegEvaluated = 0;
											} // else
										} // if
									else
										break;
									} // while
								CurSub = CurSub->NextSub;
								} // while

							if (TotalTau <= 6.9077)
								{
								Transmission = exp(-TotalTau);
								} // if not fully opaque
							else
								{
								Transmission = 0.0;
								} // else
							} // if compute volumetrics
						} // if apply volumetrics
					// Illumination is already compensated for shadow intensity
					// get the amount of backlight
					if (Transmission > 0.0)
						{
						if (BacklightSub && ((BacklightPct = BacklightSub->GetBacklightPct()) > 0.0) && 
							((LightAngle = VectorAngle(CamToPixVec, LightVec)) > 0.0))
							{
							BacklightExp = BacklightSub->GetBacklightExp();
							AngleBacklightPct = SafePow(LightAngle, BacklightExp);
							TotalBacklightPct = BacklightPct * (1.0 + AngleBacklightPct);
							Transmission *= (1.0 + TotalBacklightPct);

							// get the amount and color of alternate backlight
							AltColorBacklightPct = BacklightSub->GetAltBacklightColorPct() * AngleBacklightPct;
							LightColorBacklightPct = 1.0 - AltColorBacklightPct;
							BacklightColor[0] = BacklightSub->GetBacklightColor(0);
							BacklightColor[1] = BacklightSub->GetBacklightColor(1);
							BacklightColor[2] = BacklightSub->GetBacklightColor(2);
							IllumColor[0] = BacklightColor[0] * AltColorBacklightPct +
								PointRGB[0] * CurLight->CompleteColor[0] * LightColorBacklightPct;
							IllumColor[1] = BacklightColor[1] * AltColorBacklightPct +
								PointRGB[1] * CurLight->CompleteColor[1] * LightColorBacklightPct;
							IllumColor[2] = BacklightColor[2] * AltColorBacklightPct +
								PointRGB[2] * CurLight->CompleteColor[2] * LightColorBacklightPct;
							} // if
						else
							{
							Transmission *= StraightBacklightPct;
							IllumColor[0] = PointRGB[0] * CurLight->CompleteColor[0];
							IllumColor[1] = PointRGB[1] * CurLight->CompleteColor[1];
							IllumColor[2] = PointRGB[2] * CurLight->CompleteColor[2];
							} // else
						IllumColor[0] *= Transmission;
						IllumColor[1] *= Transmission;
						IllumColor[2] *= Transmission;
						} // if
					// contribution from this light
					OutRGB[0] += (IllumColor[0] * ShadingAllowed + PointRGB[0] * (1.0 - ShadingAllowed)) * ShadowIllumination;
					OutRGB[1] += (IllumColor[1] * ShadingAllowed + PointRGB[1] * (1.0 - ShadingAllowed)) * ShadowIllumination;
					OutRGB[2] += (IllumColor[2] * ShadingAllowed + PointRGB[2] * (1.0 - ShadingAllowed)) * ShadowIllumination;
					} // if illumination > 0
				} // if
			} // if
		CurLight = (Light *)CurLight->Next;
		} // while
	PointRGB[0] = OutRGB[0];
	PointRGB[1] = OutRGB[1];
	PointRGB[2] = OutRGB[2];
	return (1);
	} // if

return (0);

} // Renderer::IlluminateVolumetric

/*===========================================================================*/

void Renderer::VolumetricPointShader(TextureData *TexData, PixelData *PixData, 
	VertexDEM *PointPos, double PointRGB[3], double ShadingAllowed)
{
double LightVec[3], Tau, TotalTau, LightDist, DummyDbl, MinDist, MaxDist, SegSubLen, Transmission;
VertexDEM SubVert;
Light *CurLight;
VolumetricSubstance *CurSub;
RasterAnimHost *StashObject;
int ComputeVolumetrics;

if (! Substances)
	return;

Transmission = ShadingAllowed;
StashObject = PixData->Object;

// for each light
CurLight = EffectsBase->Lights;
while (CurLight)
	{
	if (CurLight->Enabled)
		{
		// if illuminate volumetrics
		if (CurLight->IllumAtmosphere && PixData->IgnoreLight != CurLight && (! StashObject || CurLight->PassTest(StashObject)))
			{
			TotalTau = 0.0;
			// cast ray from vertex to light
			LightVec[0] = CurLight->LightPos->XYZ[0] - PointPos->XYZ[0];
			LightVec[1] = CurLight->LightPos->XYZ[1] - PointPos->XYZ[1];
			LightVec[2] = CurLight->LightPos->XYZ[2] - PointPos->XYZ[2];
			LightDist = VectorMagnitude(LightVec);
			UnitVector(LightVec);
			// build ray list
			CurSub = Substances;
			ComputeVolumetrics = 0;
			while (CurSub)
				{
				CurSub->DeleteShadowRayList();
				CurSub->CurShadowRayList = NULL;
				if (CurSub->GetCastVolumetricShadows())
					{
					if (CurSub->ShadowRayList = CurSub->BuildRayList(DefCoords, PlanetRad, PointPos->XYZ, LightVec))
						{
						ComputeVolumetrics = 1;
						CurSub->CurShadowRayList = CurSub->ShadowRayList;
						} // if
					} // if
				CurSub = CurSub->NextSub;
				} // while
			if (ComputeVolumetrics)
				{
				// walk ray list evaluating density
				// calculate the light color reaching the point
				// add light color to running total
				CurSub = Substances;
				while (CurSub && TotalTau < 6.9077)
					{
					while (CurSub->CurShadowRayList && CurSub->CurShadowRayList->SegStartDist < LightDist && TotalTau < 6.9077)
						{
						if (! CurSub->CurShadowRayList->SegEvaluated)
							{
							// evaluate density and color including shading for the segment at the center of segment
							SubVert.XYZ[0] = CurSub->CurShadowRayList->SegMidXYZ[0];
							SubVert.XYZ[1] = CurSub->CurShadowRayList->SegMidXYZ[1];
							SubVert.XYZ[2] = CurSub->CurShadowRayList->SegMidXYZ[2];
							// project to screen coords and convert to lat/lon - textures may use z distance
							#ifdef WCS_BUILD_VNS
							DefCoords->CartToDeg(&SubVert);
							#else // WCS_BUILD_VNS
							SubVert.CartToDeg(PlanetRad);
							#endif // WCS_BUILD_VNS
							Cam->ProjectVertexDEM(DefCoords, &SubVert, EarthLatScaleMeters, PlanetRad, 0);
							// evaluate volumetric optical depth and color
							// set the values CurSub->CurShadowRayList->SegOpticalDepth and CurSub->CurShadowRayList->SegColor[3].
							CurSub->PointSampleVolumetric(CurSub->CurShadowRayList, &SubVert, TexData, DummyDbl);
							CurSub->CurShadowRayList->SegEvaluated = 1;
							} // if
						// find length of this substances contribution to this fragment segment
						if (CurSub->CurShadowRayList->SegOpticalDepth < FLT_MAX)
							{
							MinDist = max(0.0, CurSub->CurShadowRayList->SegStartDist);
							MaxDist = min(LightDist, CurSub->CurShadowRayList->SegEndDist);
							SegSubLen = MaxDist - MinDist;
							// diminish the transmission to 1% at optical depth distance
							Tau = SegSubLen * 4.605 / CurSub->CurShadowRayList->SegOpticalDepth;
							TotalTau += Tau;
							} // if

						// increment segments if segment end is less than light distance
						if (CurSub->CurShadowRayList->SegEndDist < LightDist)
							{
							if (CurSub->CurShadowRayList->SegEndDist >= CurSub->CurShadowRayList->RayOffDist)
								CurSub->CurShadowRayList = CurSub->CurShadowRayList->Next;
							else
								{
								CurSub->CurShadowRayList->SegStartDist = CurSub->CurShadowRayList->SegEndDist;
								CurSub->CurShadowRayList->SegEndDist = WCS_min(CurSub->CurShadowRayList->SegEndDist + CurSub->CurShadowRayList->SampleRate, CurSub->CurShadowRayList->RayOffDist);
								CurSub->CurShadowRayList->SegMidXYZ[0] += CurSub->CurShadowRayList->SampleRate * LightVec[0];
								CurSub->CurShadowRayList->SegMidXYZ[1] += CurSub->CurShadowRayList->SampleRate * LightVec[1];
								CurSub->CurShadowRayList->SegMidXYZ[2] += CurSub->CurShadowRayList->SampleRate * LightVec[2];
								CurSub->CurShadowRayList->SegEvaluated = 0;
								} // else
							} // if
						else
							break;
						} // while
					CurSub = CurSub->NextSub;
					} // while

				if (TotalTau <= 6.9077)
					{
					Transmission *= exp(-TotalTau);
					} // if
				else
					{
					Transmission = 0.0;
					break;
					} // if
				} // if
			} // if
		} // if
	CurLight = (Light *)CurLight->Next;
	} // while

// Transmission already reduced by the amount of shading sllowed
PointRGB[0] = PointRGB[0] * (1.0 - ShadingAllowed) + Transmission * PointRGB[0];
PointRGB[1] = PointRGB[1] * (1.0 - ShadingAllowed) + Transmission * PointRGB[1];
PointRGB[2] = PointRGB[2] * (1.0 - ShadingAllowed) + Transmission * PointRGB[2];

} // Renderer::VolumetricPointShader
