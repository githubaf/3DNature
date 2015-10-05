// RenderMaterials.cpp
// Material and Lighting evaluators called during rendering
// Code for World Construction Set Renderer by Gary R. Huber, 9/3/99.
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "Application.h"
#include "Render.h"
#include "EffectsLib.h"
#include "Ecotype.h"
#include "Joe.h"
#include "Points.h"
#include "Useful.h"
#include "Raster.h"
#include "Requester.h"
#include "RenderControlGUI.h"
#include "AppMem.h"
#include "Interactive.h"
#include "Realtime.h"
#include "PixelManager.h"
#include "FontImage.h"
#include "Project.h"
#include "VectorPolygon.h"
#include "VectorNode.h"
#include "Lists.h"

//extern double TotalTime;

//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//extern unsigned __int64 DevCounter[50];
//#endif // FRANK or GARY

// define this to make strata twice the thickness of the normal version - this could be user controllable
//#define WCS_STRATA_SCALING

extern float DefaultAAKernX[];
extern float DefaultAAKernY[];
extern char *Attrib_TextSymbol;

#ifdef _OPENMP
// causes foliage plotting to be multithreaded
//#define OPENMP_FOLIAGE_PLOTTING
#endif // _OPENMP

// debugging stuff for forestry features
#ifdef WCS_FORESTRY_WIZARD
#ifdef WCS_BUILD_GARY
extern double AreaRendered, StemsRendered, AvgStemsRendered, PolysRendered, GroupStemsRendered[4], ImageStemsRendered[4][8];
extern long GroupCt, ImageCt;
#endif // WCS_BUILD_GARY
#endif // WCS_FORESTRY_WIZARD

#ifdef SPEED_MOD
void FASTCALL Renderer::IlluminatePixel(PixelData *Pix)
#else // SPEED_MOD
void Renderer::IlluminatePixel(PixelData *Pix)
#endif // SPEED_MOD
{
double CosAngle, Fade, Illum, Distance, Ratio, LuminosityInv, HazePercent, HazeSumPercent, HazeSumPercentInv, 
	SpecAngle, Specularity, Specularity2, TopAmbient, BotAmbient, PassedShadow, SpecAmbient,
	IllumSum[3], TempLight[3], Specular[3], TempSpec[3], HazeSum[3], LightVec[3], PixToLight[3], SpecVec[3], 
	InvViewVec[3], OrigColor[3], HazeStart, HazeEnd, HazeRange, HazeStartIntensity, HazeEndIntensity, HazeIntensityRange,
	BacklightIllum, LightVecLen, PixToLightLen;
	//WaterTransmission;
int FadeMisc;
Light *CurLight = EffectsBase->Lights;
Atmosphere *CurAtmo;

OrigColor[0] = Pix->RGB[0];
OrigColor[1] = Pix->RGB[1];
OrigColor[2] = Pix->RGB[2];
InvViewVec[0] = -Pix->ViewVec[0]; // ViewVec is pre-normalized, so InvViewVec will be as well
InvViewVec[1] = -Pix->ViewVec[1];
InvViewVec[2] = -Pix->ViewVec[2];

// If lights are disabled in Render Options then polygons will have the material color.
// If instead _each_ individual light is disabled each polygon will be its luminosity * polygon color.
if (Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT])
	{
	// if there are light textures set flag to evaluate again for this pixel
	if (Pix->TexData)
		{
		while (CurLight)
			{
			if (CurLight->Enabled)
				{
				if (CurLight->GetEnabledTexture(WCS_EFFECTS_LIGHT_TEXTURE_COLOR))
					CurLight->ColorEvaluated = 0;
				} // if
			CurLight = (Light *)CurLight->Next;
			} // while
		CurLight = EffectsBase->Lights;
		} // if

	// clouds are the only objects that sllow turning off miscellaneous shadow types
	FadeMisc = (Pix->PixelType != WCS_PIXELTYPE_CLOUD || (Pix->ShadowFlags & WCS_SHADOWTYPE_MISC));

	if (Pix->Luminosity < 1.0 || Pix->Specularity > 0.0 || Pix->Specularity2 > 0.0)
		{
		Specular[2] = Specular[1] = Specular[0] = IllumSum[2] = IllumSum[1] = IllumSum[0] = 0.0;

		//WaterTransmission = Pix->WaterDepth > 0.0 && Pix->OpticalDepth > 0.0 ? exp(-Pix->WaterDepth * 4.605 / Pix->OpticalDepth): 1.0;

		while (CurLight)
			{
			if (CurLight->Enabled && Pix->IgnoreLight != CurLight && CurLight->PassTest(Pix->Object))
				{
				Fade = 1.0;
				// find if in planet shadow
				PixToLight[0] = CurLight->LightPos->XYZ[0] - Pix->XYZ[0];
				PixToLight[1] = CurLight->LightPos->XYZ[1] - Pix->XYZ[1];
				PixToLight[2] = CurLight->LightPos->XYZ[2] - Pix->XYZ[2];
				PixToLightLen = UnitVectorMagnitude(PixToLight); // Unitize vector and record previous length for later use
				if (FadeMisc)
					{
					CosAngle = VectorAngleNoUnitize(PixToLight, CurLight->LightPosUnit); // both args are guaranteed unitized already
					if (CosAngle > CurLight->CosTangentAngle)
						{
						if ((Distance = PixToLightLen) >= CurLight->TangentDistance)
							{
							if (CosAngle < CurLight->CosTangentAnglePlus5Pct)
								{
								Ratio = (CosAngle - CurLight->CosTangentAngle) * CurLight->InvCosTangentAngleDifference;
								Fade = 1.0 - Ratio;
								} // if
							else
								Fade = 0.0;
							// replaced by above 11/14/02 GRH
							//Ratio = CosAngle * CurLight->InvCosTangentAngle;
							//if (Ratio > 1.0038)
							//	Fade = 0.0;
							//else
							//	Fade = 1.0 - (Ratio - 1.0) * (1.0 / .0038); // nee / .0038
							} // if
						} // if
					} // if
				if (Fade > 0.0)
					{
					// find the light's vector relative to the pixel or earth center if Distant light
					if (Pix->PixelType == WCS_PIXELTYPE_CELESTIAL || CurLight->LightType == WCS_EFFECTS_LIGHTTYPE_OMNI || 
						(CurLight->LightType == WCS_EFFECTS_LIGHTTYPE_SPOT && ! CurLight->Distant))
						{
						// vector must be from pixel to light
						LightVec[0] = CurLight->LightPos->XYZ[0] - Pix->XYZ[0];
						LightVec[1] = CurLight->LightPos->XYZ[1] - Pix->XYZ[1];
						LightVec[2] = CurLight->LightPos->XYZ[2] - Pix->XYZ[2];
						LightVecLen = UnitVectorMagnitude(LightVec); // could return 0.0
						// compensate for soft edge of spot cone
						if (FadeMisc && CurLight->LightType == WCS_EFFECTS_LIGHTTYPE_SPOT)
							{
							if(LightVecLen != 0.0)
								{
								CosAngle = VectorAngleNoUnitize(CurLight->LightAim->XYZ, LightVec);
								} // if
							else
								{
								CosAngle = 1.0; // VectorAngle returns 1.0 if length of either Vector <= 0.0
								} // else
							if (CosAngle < CurLight->MaxConeCosAngle)
								Fade = 0.0;
							else if (CosAngle < CurLight->MaxHotSpotCosAngle)
								{
								Fade *= ((CosAngle - CurLight->MaxConeCosAngle) * CurLight->InvConeEdgeCosAngle);
								} // else if in soft edge
							} // if spotlight
						} // else
					else
						{
						// vector has been pre-calculated to aim at the light from the target
						// Lightaim is pre-unitized, so we don't need to re-do it
						LightVec[0] = CurLight->LightAim->XYZ[0];
						LightVec[1] = CurLight->LightAim->XYZ[1];
						LightVec[2] = CurLight->LightAim->XYZ[2];
						LightVecLen = 1.0;
						} // else

					if (FadeMisc && ! CurLight->Distant && CurLight->FallOffExp != 0.0)
						{
						if ((Distance = PixToLightLen) >= CurLight->MaxIllumDist)
							Fade = 0.0;
						else if (Distance > 1.0)
							{
							Fade *= (1.0 / pow(Distance, CurLight->FallOffExp));
							} // if
						} // if
					if (Fade > 0.0)
						{
						if (Pix->ShadowFlags & (~(WCS_SHADOWTYPE_VOLUME | WCS_SHADOWTYPE_MISC)))
							{
							// diminish Fade by shadows, this will affect specular component as well as diffuse
							if ((PassedShadow = CurLight->EvaluateShadows(Pix)) < 1.0)
								{
								Fade *= (1.0 + Pix->ReceivedShadowIntensity * (PassedShadow - 1.0)); 
								} // if
							} // if
						//Fade *= WaterTransmission;

						if (Fade > 0.0)
							{
							if (Pix->Normal[0] != 0.0 || Pix->Normal[1] != 0.0 || Pix->Normal[2] != 0.0)
								{
								if(LightVecLen != 0.0)
									{
									Illum = VectorAngleNoUnitize(Pix->Normal, LightVec);
									} // if
								else
									Illum = 1.0;
								} // if
							else
								Illum = 1.0;
							if (Pix->PixelType == WCS_PIXELTYPE_CLOUD)
								{
								//if (same cloud not casting and receiving volumetric shadows)
								if (! (((CloudEffect *)Pix->Object)->CastShadows && 
									(((CloudEffect *)Pix->Object)->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_VOLUMETRIC || 
									((CloudEffect *)Pix->Object)->CastShadowStyle == WCS_EFFECTS_CLOUD_SHADOWSTYLE_COMBINATION)
									&& ((CloudEffect *)Pix->Object)->ReceiveShadowsVolumetric))
									{
									Illum = Illum * Pix->CloudShading + 1.0 - Pix->CloudShading;
									} // if
								else
									Illum = 1.0;	// volumetric shading will be supplied later
								} // if cloud
							else if (Pix->PixelType == WCS_PIXELTYPE_FOLIAGE)
								{
								// temporary testing
								if (Pix->CloudShading > 0.0 && Illum < 0.0)
									ScaleHSV(Pix->RGB, 0.0, 50.0 * Pix->CloudShading * (-Illum), 0.0);
								Illum += (1.0 - Illum) * Pix->CloudShading;
								} // else if foliage
							TempSpec[2] = TempSpec[1] = TempSpec[0] = 0.0;

							// compute specular component
							if (! Exporter && (Pix->Specularity > 0.0 || Pix->Specularity2 > 0.0) && Illum > 0.0)
								{
								// compute specular light reflection amount and color
								// Normal vector must be normalized, but it normally is
								SpecVec[0] = Pix->Normal[0] * 2.0 * Illum;
								SpecVec[1] = Pix->Normal[1] * 2.0 * Illum;
								SpecVec[2] = Pix->Normal[2] * 2.0 * Illum;
								// UnitVector is no longer needed here, as it was done, above
								//UnitVector(PixToLight);		// (formerly) required for the next operation to be correct
								FindPosVector(SpecVec, SpecVec, PixToLight);	// subtracts Pixel to light vector
								if ((SpecAngle = VectorAngleNoUnitize(SpecVec, Pix->ViewVec)) > 0.0)
									{
									if (Pix->Specularity > 0.0)
										Specularity = Pix->Specularity * SafePow(SpecAngle, Pix->SpecularExp);
									else
										Specularity = 0.0;
									if (Pix->Specularity2 > 0.0)
										Specularity2 = Pix->Specularity2 * SafePow(SpecAngle, Pix->SpecularExp2);
									else
										Specularity2 = 0.0;
									if ((Specularity *= Fade) > 0.0)
										{
										if (Pix->TexData && ! CurLight->ColorEvaluated)
											{
											CurLight->EvalColor(Pix);
											} // if
										if (Pix->SpecRGBPct > 0.0)
											{
											TempSpec[0] = Specularity * (CurLight->CompleteColor[0] * (1.0 - Pix->SpecRGBPct)
												+ Pix->SpecRGBPct * Pix->SpecRGB[0]);
											TempSpec[1] = Specularity * (CurLight->CompleteColor[1] * (1.0 - Pix->SpecRGBPct)
												+ Pix->SpecRGBPct * Pix->SpecRGB[1]);
											TempSpec[2] = Specularity * (CurLight->CompleteColor[2] * (1.0 - Pix->SpecRGBPct)
												+ Pix->SpecRGBPct * Pix->SpecRGB[2]);
											} // if
										else
											{
											TempSpec[0] = Specularity * CurLight->CompleteColor[0];
											TempSpec[1] = Specularity * CurLight->CompleteColor[1];
											TempSpec[2] = Specularity * CurLight->CompleteColor[2];
											} // else
										} // if
									if ((Specularity2 *= Fade) > 0.0)
										{
										if (Pix->Spec2RGBPct > 0.0)
											{
											TempSpec[0] += Specularity2 * (CurLight->CompleteColor[0] * (1.0 - Pix->Spec2RGBPct)
												+ Pix->Spec2RGBPct * Pix->Spec2RGB[0]);
											TempSpec[1] += Specularity2 * (CurLight->CompleteColor[1] * (1.0 - Pix->Spec2RGBPct)
												+ Pix->Spec2RGBPct * Pix->Spec2RGB[1]);
											TempSpec[2] += Specularity2 * (CurLight->CompleteColor[2] * (1.0 - Pix->Spec2RGBPct)
												+ Pix->Spec2RGBPct * Pix->Spec2RGB[2]);
											} // if
										else
											{
											TempSpec[0] += Specularity2 * CurLight->CompleteColor[0];
											TempSpec[1] += Specularity2 * CurLight->CompleteColor[1];
											TempSpec[2] += Specularity2 * CurLight->CompleteColor[2];
											} // else
										Specularity += Specularity2;
										} // if
									if (Specularity > 0.0)
										{
										if (Pix->Transparency > 0.0)
											{
											double InvSpec = 1.0 - Specularity;

											if (InvSpec <= 0.0)
												Pix->Transparency = 0.0;
											else if (InvSpec < 1.0)
												Pix->Transparency *= InvSpec;
											} // if
										} // if
									} // if
								else
									Specularity = 0.0;
								} // if
							else
								Specularity = 0.0;

							TempLight[0] = TempLight[1] = TempLight[2] = 0.0;
							// compute translucency amount
							if (Pix->Translucency > 0.0 && Illum < 0.0 && ! Exporter)
								{
								// compute translucency amount
								if(PixToLightLen == 0.0)
									{
									Illum = Pix->Translucency; // VectorAngle would return 1.0 for invalid input vectors and 1.0^n=1.0 for all n
									} // if
								else
									{
									Illum = Pix->Translucency * SafePow(VectorAngleNoUnitize(InvViewVec, PixToLight), Pix->TranslucencyExp);
									} // else
								} // if
							else if (Pix->PixelType == WCS_PIXELTYPE_CLOUD)
								{
								if(PixToLightLen == 0)
									{
									CosAngle = 1.0;
									} // if
								else
									{
									CosAngle = VectorAngleNoUnitize(InvViewVec, PixToLight);
									} // else
								if (CosAngle > 0.0)
									{
									//Illum += Pix->Translucency * SafePow(CosAngle, Pix->TranslucencyExp);
									BacklightIllum = Pix->Translucency * SafePow(CosAngle, Pix->TranslucencyExp);
									Illum += BacklightIllum * (1.0 - Pix->AltBacklightColorPct);
									BacklightIllum *= Fade * Pix->AltBacklightColorPct;
									TempLight[0] = BacklightIllum * Pix->AltBacklightColor[0];
									TempLight[1] = BacklightIllum * Pix->AltBacklightColor[1];
									TempLight[2] = BacklightIllum * Pix->AltBacklightColor[2];
									} // if
								} // else if cloud

							// illumination amount is relative to the cosine of the angle between light and surface normal vectors
							if (Illum > 0.0 || Specularity > 0.0)
								{
								if (Pix->TexData && ! CurLight->ColorEvaluated)
									{
									CurLight->EvalColor(Pix);
									} // if
								TempLight[0] += Illum * Fade * CurLight->CompleteColor[0];
								TempLight[1] += Illum * Fade * CurLight->CompleteColor[1];
								TempLight[2] += Illum * Fade * CurLight->CompleteColor[2];
								
								IllumSum[0] += TempLight[0];
								IllumSum[1] += TempLight[1];
								IllumSum[2] += TempLight[2];
								Specular[0] += TempSpec[0];
								Specular[1] += TempSpec[1];
								Specular[2] += TempSpec[2];
								} // if Illum > 0
							} // if Fade > 0
						} // if Fade > 0
					} // if light is less than 90 degrees around the globe from pixel
				} // if light enabled
			CurLight = (Light *)CurLight->Next;
			} // while

		if (Pix->Luminosity < 1.0)
			{
			LuminosityInv = 1.0 - Pix->Luminosity;
			Pix->RGB[0] = Pix->RGB[0] * (TempLight[0] = (IllumSum[0] * LuminosityInv + Pix->Luminosity)) + Specular[0];
			Pix->RGB[1] = Pix->RGB[1] * (TempLight[1] = (IllumSum[1] * LuminosityInv + Pix->Luminosity)) + Specular[1];
			Pix->RGB[2] = Pix->RGB[2] * (TempLight[2] = (IllumSum[2] * LuminosityInv + Pix->Luminosity)) + Specular[2];
			Pix->Illum = (TempLight[0] + Specular[0] + TempLight[1] + Specular[1] + TempLight[2] + Specular[2]) * (1.0 / 3.0);
			} // if
		else
			{
			Pix->RGB[0] += Specular[0];
			Pix->RGB[1] += Specular[1];
			Pix->RGB[2] += Specular[2];
			Pix->Illum = Pix->Luminosity + (Specular[0] + Specular[1] + Specular[2]) * (1.0 / 3.0);
			} // else
		} // if 
	if (Pix->ShadowFlags & WCS_SHADOWTYPE_VOLUME)
		{
		VertexDEM TempPt;

		TempPt.Lat = Pix->Lat;
		TempPt.Lon = Pix->Lon;
		TempPt.Elev = Pix->Elev;
		TempPt.XYZ[0] = Pix->XYZ[0];
		TempPt.XYZ[1] = Pix->XYZ[1];
		TempPt.XYZ[2] = Pix->XYZ[2];
		if (Pix->PixelType == WCS_PIXELTYPE_CLOUD)
			VolumetricPointShader(Pix->TexData, Pix, &TempPt, Pix->RGB, Pix->CloudShading);
		else
			VolumetricPointShader(Pix->TexData, Pix, &TempPt, Pix->RGB, Pix->ReceivedShadowIntensity);
		} // if
	} // if lights enabled
else
	Pix->Illum = 1.0;

if (Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE])
	{
	// compute ambient light factors, Pix->XYZ is in the direction of "up"
	if (Pix->Normal[0] != 0.0 || Pix->Normal[1] != 0.0 || Pix->Normal[2] != 0.0)
		TopAmbient = VectorAngle(Pix->Normal, Pix->XYZ) * .5 + .5;
	else
		TopAmbient = 1.0;
	BotAmbient = 1.0 - TopAmbient;

	// apply atmospheric effects, diminish pixel color and add haze and fog colors
	CurAtmo = EffectsBase->Atmospheres;
	HazeSumPercent = HazeSum[0] = HazeSum[1] = HazeSum[2] = 0.0;
	// this will boost the amount of ambient color added if surface is specular
	SpecAmbient = 1.0 + 2.0 * (Pix->Specularity + Pix->Specularity2);
	while (CurAtmo)
		{
		if (CurAtmo->Enabled)
			{
			if (Pix->PixelType != WCS_PIXELTYPE_CELESTIAL)
				{
				Pix->RGB[0] += OrigColor[0] * SpecAmbient * (TopAmbient * CurAtmo->CompleteTopAmbient[0] + BotAmbient * CurAtmo->CompleteBottomAmbient[0]);
				Pix->RGB[1] += OrigColor[1] * SpecAmbient * (TopAmbient * CurAtmo->CompleteTopAmbient[1] + BotAmbient * CurAtmo->CompleteBottomAmbient[1]);
				Pix->RGB[2] += OrigColor[2] * SpecAmbient * (TopAmbient * CurAtmo->CompleteTopAmbient[2] + BotAmbient * CurAtmo->CompleteBottomAmbient[2]);
				} // if
			if (CurAtmo->AtmosphereType != WCS_EFFECTS_ATMOSPHERETYPE_VOLUMETRIC)
				{
				if (Pix->PixelType != WCS_PIXELTYPE_CELESTIAL)
					{
					if (CurAtmo->HazeEnabled)
						{
						Distance = Pix->Q;
						if (Pix->PixelType == WCS_PIXELTYPE_CLOUD && CurAtmo->SeparateCloudHaze)
							{
							HazeStart = CurAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZESTART].CurValue;
							HazeEnd = CurAtmo->CloudHazeEndDistance;
							HazeRange = CurAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZERANGE].CurValue;
							HazeStartIntensity = CurAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZESTARTINTENSITY].CurValue;
							HazeEndIntensity = CurAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_CLOUDHAZEENDINTENSITY].CurValue;
							HazeIntensityRange = CurAtmo->CloudHazeIntensityRange;
							CurAtmo->EvalColor(Pix, WCS_EFFECTS_ATMOSPHERECOLORTYPE_CLOUDHAZE);
							TempLight[0] = CurAtmo->CompleteColor[0];	//CompleteCloudHazeColor[0];
							TempLight[1] = CurAtmo->CompleteColor[1];	//CompleteCloudHazeColor[1];
							TempLight[2] = CurAtmo->CompleteColor[2];	//CompleteCloudHazeColor[2];
							} // if
						else
							{
							HazeStart = CurAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].CurValue;
							HazeEnd = CurAtmo->HazeEndDistance;
							HazeRange = CurAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE].CurValue;
							HazeStartIntensity = CurAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY].CurValue;
							HazeEndIntensity = CurAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY].CurValue;
							HazeIntensityRange = CurAtmo->HazeIntensityRange;
							CurAtmo->EvalColor(Pix, WCS_EFFECTS_ATMOSPHERECOLORTYPE_HAZE);
							TempLight[0] = CurAtmo->CompleteColor[0];	//CompleteHazeColor[0];
							TempLight[1] = CurAtmo->CompleteColor[1];	//CompleteHazeColor[1];
							TempLight[2] = CurAtmo->CompleteColor[2];	//CompleteHazeColor[2];
							} // else
						if (Distance <= HazeStart)
							{
							HazePercent = HazeStartIntensity;
							} // if
						else if (Distance >= HazeEnd)
							{
							HazePercent = HazeEndIntensity;
							} // if
						else
							{
							HazePercent = (Distance - HazeStart) / HazeRange;
							if (CurAtmo->AtmosphereType == WCS_EFFECTS_ATMOSPHERETYPE_SLOWINCREASE)
								HazePercent *= HazePercent;
							else if (CurAtmo->AtmosphereType == WCS_EFFECTS_ATMOSPHERETYPE_FASTINCREASE)
								{
								HazePercent = 1.0 - HazePercent;
								HazePercent *= HazePercent;
								HazePercent = 1.0 - HazePercent;
								} // else if
							else if (CurAtmo->AtmosphereType == WCS_EFFECTS_ATMOSPHERETYPE_EXPONENTIAL)
								{
								HazePercent = 1.0 - exp(-HazePercent * 4.605);
								} // else if
							HazePercent = HazeStartIntensity + HazePercent * HazeIntensityRange;
							} // else
						if (HazePercent > 0.0)
							{
							if (CurAtmo->ActAsFilter)
								{
								Pix->RGB[0] = Pix->RGB[0] * (1.0 - HazePercent) + (Pix->RGB[0] * TempLight[0] * HazePercent);
								Pix->RGB[1] = Pix->RGB[1] * (1.0 - HazePercent) + (Pix->RGB[1] * TempLight[1] * HazePercent);
								Pix->RGB[2] = Pix->RGB[2] * (1.0 - HazePercent) + (Pix->RGB[2] * TempLight[2] * HazePercent);
								} // if
							else
								{
								HazeSumPercent += HazePercent;
								TempLight[0] *= HazePercent;
								TempLight[1] *= HazePercent;
								TempLight[2] *= HazePercent;
								// if haze is backlit, apply translucency
								if (Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT])
									{
									CurLight = EffectsBase->Lights;
									while (CurLight)
										{
										if (CurLight->Enabled && Pix->IgnoreLight != CurLight && CurLight->PassTest(CurAtmo))
											{
											// This is a whole independent new use of PixToLight and doesn't benefit from
											// the Length/Unitization optimizations used above
											// Though InvViewVec is already unitized, it costs us one sqrt and divide to unitize PixToLight
											// Which is the exact savings that VectorAngleNoUnitize gains us, so it's not worthwhile
											PixToLight[0] = CurLight->LightPos->XYZ[0] - Pix->XYZ[0];
											PixToLight[1] = CurLight->LightPos->XYZ[1] - Pix->XYZ[1];
											PixToLight[2] = CurLight->LightPos->XYZ[2] - Pix->XYZ[2];
											if ((CosAngle = VectorAngle(InvViewVec, PixToLight)) > 0.0)
												{
												Illum = HazePercent * SafePow(CosAngle, 4.0);
												if (Pix->TexData && ! CurLight->ColorEvaluated)
													{
													CurLight->EvalColor(Pix);
													} // if
												TempLight[0] += CurLight->CompleteColor[0] * Illum;
												TempLight[1] += CurLight->CompleteColor[1] * Illum;
												TempLight[2] += CurLight->CompleteColor[2] * Illum;
												} // if
											} // if
										CurLight = (Light *)CurLight->Next;
										} // while
									} // if lights enabled
								HazeSum[0] += TempLight[0];
								HazeSum[1] += TempLight[1];
								HazeSum[2] += TempLight[2];
								} // if
							} // if
						} // if haze enabled
					if (CurAtmo->FogEnabled)
						{
						Distance = Pix->Elev;
						if (Distance <= CurAtmo->FogLowElev)
							{
							HazePercent = CurAtmo->FogLowIntensity;
							} // if
						else if (Distance >= CurAtmo->FogHighElev)
							{
							HazePercent = CurAtmo->FogHighIntensity;
							} // if
						else
							{
							HazePercent = (Distance - CurAtmo->FogLowElev) / CurAtmo->FogRange;
							HazePercent = CurAtmo->FogLowIntensity + HazePercent * CurAtmo->FogIntensityRange;
							} // else
						if (HazePercent > 0.0)
							{
							HazeSumPercent += HazePercent;
							CurAtmo->EvalColor(Pix, WCS_EFFECTS_ATMOSPHERECOLORTYPE_FOG);
							HazeSum[0] += CurAtmo->CompleteColor[0] * HazePercent;	//CompleteFogColor[0] * HazePercent;
							HazeSum[1] += CurAtmo->CompleteColor[1] * HazePercent;	//CompleteFogColor[1] * HazePercent;
							HazeSum[2] += CurAtmo->CompleteColor[2] * HazePercent;	//CompleteFogColor[2] * HazePercent;
							} // if
						} // if fog enabled
					} // if not celestial
				} // if not volumetric
			else
				{
				// <<<>>>gh
				} // else volumetric
			} // if
		CurAtmo = (Atmosphere *)CurAtmo->Next;
		} // while

	if (HazeSumPercent > 0.0)
		{
		if (HazeSumPercent > 1.0)
			{
			double InvHazeSumPercent = 1.0 / HazeSumPercent;
			HazeSum[0] *= InvHazeSumPercent;
			HazeSum[1] *= InvHazeSumPercent;
			HazeSum[2] *= InvHazeSumPercent;
			HazeSumPercent = 1.0;
			} // if
		HazeSumPercentInv = 1.0 - HazeSumPercent;

		Pix->RGB[0] = Pix->RGB[0] * HazeSumPercentInv + HazeSum[0];
		Pix->RGB[1] = Pix->RGB[1] * HazeSumPercentInv + HazeSum[1];
		Pix->RGB[2] = Pix->RGB[2] * HazeSumPercentInv + HazeSum[2];
		if (Pix->Reflectivity)
			Pix->Reflectivity *= HazeSumPercentInv;
		} // if
	} // if atmospheres enabled

} // Renderer::IlluminatePixel

/*===========================================================================*/

double Renderer::IlluminateAtmoPixel(PixelData *Pix, Light *CurLight)
{
double CosAngle, Fade, Distance, PassedShadow, PixToLight[3], PixToLightLen, Ratio;

Fade = 1.0;
Distance = 0.0;

if (Pix->ShadowFlags & WCS_SHADOWTYPE_MISC)
	{
	// find pixel's distance around planet from light source
	PixToLight[0] = CurLight->LightPos->XYZ[0] - Pix->XYZ[0];
	PixToLight[1] = CurLight->LightPos->XYZ[1] - Pix->XYZ[1];
	PixToLight[2] = CurLight->LightPos->XYZ[2] - Pix->XYZ[2];
	PixToLightLen = UnitVectorMagnitude(PixToLight);
	CosAngle = VectorAngleNoUnitize(PixToLight, CurLight->LightPosUnit);
	if (CosAngle > CurLight->CosTangentAngle)
		{
		if ((Distance = PixToLightLen) >= CurLight->TangentDistance)
			{
			if (CosAngle < CurLight->CosTangentAnglePlus5Pct)
				{
				Ratio = (CosAngle - CurLight->CosTangentAngle) * CurLight->InvCosTangentAngleDifference;
				Fade = 1.0 - Ratio;
				} // if
			else
				Fade = 0.0;
			// replaced by above 11/14/02 GRH
			//Ratio = CosAngle * CurLight->InvCosTangentAngle;
			//if (Ratio > 1.0038)
			//	Fade = 0.0;
			//else
			//	Fade = 1.0 - (Ratio - 1.0) * (1.0 / .0038); // nee / .0038
			} // if
		} // if

	if (Fade > 0.0)
		{
		// find the light's vector relative to the pixel or earth center if Distant light
		if (CurLight->LightType == WCS_EFFECTS_LIGHTTYPE_OMNI || 
			(CurLight->LightType == WCS_EFFECTS_LIGHTTYPE_SPOT && ! CurLight->Distant))
			{
			// compensate for soft edge of spot cone
			if (CurLight->LightType == WCS_EFFECTS_LIGHTTYPE_SPOT)
				{
				if(PixToLightLen == 0.0)
					{
					CosAngle = 1.0;
					} // if
				else
					{
					CosAngle = VectorAngleNoUnitize(CurLight->LightAim->XYZ, PixToLight);
					} // else
				if (CosAngle < CurLight->MaxConeCosAngle)
					Fade = 0.0;
				else if (CosAngle < CurLight->MaxHotSpotCosAngle)
					{
					Fade *= ((CosAngle - CurLight->MaxConeCosAngle) * CurLight->InvConeEdgeCosAngle);
					} // else if in soft edge
				} // if spotlight
			} // else

		// ray from pixel to light is needed for falloff and specular calculations
		if (! CurLight->Distant && CurLight->FallOffExp != 0.0)
			{
			if (Distance == 0.0)
				Distance = PixToLightLen;
			if (Distance >= CurLight->MaxIllumDist)
				Fade = 0.0;
			else if (Distance > 1.0)
				{
				Fade /= pow(Distance, CurLight->FallOffExp);
				} // if
			} // if
		} // if light is less than 90 degrees around the globe from pixel
	} // if
if (Fade > 0.0 && (Pix->ShadowFlags & ~(WCS_SHADOWTYPE_VOLUME | WCS_SHADOWTYPE_MISC)))
	{
	// diminish Fade by shadows
	if ((PassedShadow = CurLight->EvaluateShadows(Pix)) < 1.0)
		{
		Fade *= (1.0 + Pix->ReceivedShadowIntensity * (PassedShadow - 1.0)); 
		} // if
	} // if Fade > 0

return (Fade);

} // Renderer::IlluminateAtmoPixel

/*===========================================================================*/

void Renderer::IlluminateExportPixel(PixelData *Pix, Light *CurLight)
{
double Illum, LuminosityInv, IllumSum[3];

if (Pix->Luminosity < 1.0)
	{
	// vector has been pre-calculated to aim at the light from the target
	if (Pix->Normal[0] != 0.0 || Pix->Normal[1] != 0.0 || Pix->Normal[2] != 0.0)
		Illum = VectorAngleNoUnitize(Pix->Normal, CurLight->LightAim->XYZ); // both args are already unitized
	else
		Illum = 1.0;

	// illumination amount is relative to the cosine of the angle between light and surface normal vectors
	if (Illum > 0.0)
		{
		IllumSum[0] = Illum * CurLight->CompleteColor[0];
		IllumSum[1] = Illum * CurLight->CompleteColor[1];
		IllumSum[2] = Illum * CurLight->CompleteColor[2];
		} // if Illum > 0
	else
		IllumSum[0] = IllumSum[1] = IllumSum[2] = 0.0;

	LuminosityInv = 1.0 - Pix->Luminosity;
	Pix->RGB[0] = Pix->RGB[0] * (IllumSum[0] * LuminosityInv + Pix->Luminosity);
	Pix->RGB[1] = Pix->RGB[1] * (IllumSum[1] * LuminosityInv + Pix->Luminosity);
	Pix->RGB[2] = Pix->RGB[2] * (IllumSum[2] * LuminosityInv + Pix->Luminosity);
	} // if

} // Renderer::IlluminateExportPixel

/*===========================================================================*/

// random number usage by index
// 0 - Foliage On/Off switch
// 1 - Foliage Selector
// 2 - Wt of vertex 0
// 3 - Wt of vertex 1
// 4 - Wt of vertex 2
// 5 - Density
// 6 - Height
// 7 - Flip X or Rotate X
// 8 - Rotate Y
// 9 - Rotate Z

/* archived and retired 3/27/08 GRH
int Renderer::RenderFoliage(PolygonData *Poly, VertexData *Vtx[3], Ecotype *Eco, double Opacity, 
	double MatCovg, double *VertexWeights, unsigned long SeedOffset, int FolEffect, bool RenderAboveBeachLevel)
{
double Random[10], Density, FolDens, FolHeight, MinHeight, FolHeightFactor, EnvHeightFactor, 
	ThirdVtxWt, Rotate[3], Value[3], CamFolVec[3], TexOpacity, ForshortenAngle, MinPixelSize;
#ifdef WCS_VECPOLY_EFFECTS
double FolMaterialWeight, BeachElev;
#endif // WCS_VECPOLY_EFFECTS
int Success = 1, LastTime, ItemSuppressed = 0;
FoliageLink *FolLink;
FoliageChainList *CurChain;
RasterAnimHost *ObjStash;
RealtimeFoliageData *FolListDat;
VertexDEM FolVtx;
bool FlipX, RenderThisItem = true;

FolPlotData.Poly = Poly;
FolPlotData.Vert = &FolVtx;
FolPlotData.Opacity = Opacity;

if (! ShadowMapInProgress && Opt->FoliageShadowsOnly)
	return (1);

// some stuff to test range and average of GenGauss()
//Density = 0.0;
//FolHeight = -FLT_MAX;
//MinHeight = FLT_MAX;
//for (LastTime = 0; LastTime < 1000000; LastTime ++)
//	{
//	FolDens = Eco->Rand.GenGauss();
//	Density += FolDens;
//	if (FolDens > FolHeight)
//		FolHeight = FolDens;
//	if (FolDens < MinHeight)
//		MinHeight = FolDens;
//	} // for
//Density /= 1000000.0;
//Average in one test was -.0007, max was 3.364, min was - 3.356


Eco->Rand.Seed64BitShift(Poly->LatSeed + SeedOffset, Poly->LonSeed + SeedOffset);
Value[2] = Value[1] = Value[0] = 0.0;

// either we do all this for one foliage chain if ecotype holds the density
// or we repeat it for each group

for (CurChain = Eco->ChainList; CurChain; CurChain = CurChain->Next)
	{
	// LastTime will tell us when to move on to the next chain
	LastTime = 0;
	// Calculate the actual density to be used in determining stems to grow.
	// Density value passed as MatCovg is the material coverage value.
	Density = MatCovg;
	if (! FolEffect)
		{
		// if there are thematic maps for density, evaluate them. Otherwise use the density directly.
		if (CurChain->DensityTheme && CurChain->DensityTheme->Eval(Value, Poly->Vector))
			{
			// see usage notes in else clause below
			if (Eco->ConstDensity)
				// thematic map has supplied density in stems/unit area. DensityNormalizer converts this to stems/sq meter
				//Density *= ((Eco->Rand.GenGauss() * (1.0 / 6.9282)) + 1.5) * Value[0] * Eco->DensityNormalizer * Poly->Area;
				// above formula was wrong and yielded values of 1 to 2, averaging 1.5. Modified 9/26/02 by GRH
				Density *= ((Eco->Rand.GenGauss() * (1.0 / 6.9282)) + 1.0) * Value[0] * Eco->DensityNormalizer * Poly->Area;
			else
				Density *= .01 * Value[0];	// thematic map has supplied density in %
			Density *= CurChain->EcoThemeDensFactor;	// factor compensates for the occasion when the foliage chain is for a 
														// single group but thematic map provides density for entire ecotype.
														// This can happen if height is specified by group and density by ecotype.
			} // if
		else
			{
			if (Eco->ConstDensity)
				{
				// use gaussian random value to get normal distribution of foliage, divide by 3.46410 and add 1 
				// so output is in range 0-2 but generally closer to 1.
				// Correct for polygon area.
				// Eco->DensityArea contains a density value normalized to 1 meter square
				// to scale to range of 0 to 2:
				//Density *= ((Eco->Rand.GenGauss() / 3.46410) + 1.0) * Eco->DensityPerSqMeter * Poly->Area;
				// to scale to range of .5 to 1.5 for more even distribution:
				//Density *= ((Eco->Rand.GenGauss() * (1.0 / 6.9282)) + 1.5) * CurChain->DensityPerSqMeter * Poly->Area;
				// above formula was wrong and yielded values of 1 to 2, averaging 1.5. Modified 9/26/02 by GRH
				Density *= ((Eco->Rand.GenGauss() * (1.0 / 6.9282)) + 1.0) * CurChain->DensityPerSqMeter * Poly->Area;
				} // if constant density ecotype
			else
				{
				Density *= CurChain->DensityPerSqMeter;	// the name is misleading when used for variable density foliage
				} // else
			} // else
		} // if

#ifdef WCS_FORESTRY_WIZARD
#ifdef WCS_BUILD_GARY
	AvgStemsRendered += Density;
	AreaRendered += Poly->Area;

	PolysRendered ++;
#endif // WCS_BUILD_GARY
#endif // WCS_FORESTRY_WIZARD

	if (Density > 0.0)
		{
		if (Density <= 1.0)
			LastTime = 1;
		// seed the random number generator with the polygon center location
		// we'll need potentially 10 random #s for each foliage
		// generate two to start, one to determine if a foliage is needed and one to determine which foliage
		Eco->Rand.GenMultiPRN(2, &Random[0]);
		// the granlarity of the random numbers is .00001 - 
		// this prohibits fine control over foliage density at low densities per polygon.
		// The solution is to add a fraction of that granularity if the random # is very small.
		if (Random[0] < .0001)
			Random[0] += (.00001 * Eco->Rand.GenPRN());
		while (Success && Random[0] < Density)
			{
			// pick a foliage to draw
			FolLink = CurChain->FoliageChain;
			while (Random[1] > FolLink->Density)
				{
				if (! (FolLink = FolLink->Next))
					{
					LastTime = 1;
					break;
					} // if
				} // while

			// Foliage may be in the list but disabled if the Image Object or 3D Object itself is disabled
			// The raster may disabled if it failed to load correctly.
			if (FolLink && FolLink->Enabled && ((FolLink->Rast && FolLink->Rast->Enabled) || (FolLink->Fol->Obj && FolLink->Fol->Obj->Enabled)))
				{
				// note that Random[4] is no longer used since 7/17/02.
				// Random[4] is now used in vertex-weighted distributions
				Eco->Rand.GenMultiPRN(8, &Random[2]);
				if (! FolEffect)
					{
					// determine the lat, lon and elev of the foliage position by doing a tricky
					// barycentric weighting of vertices based on 3 random #s.
					#ifdef WCS_USE_V5FOLIAGE_DENSITYMETHOD
					// this is the V5/VNS 1 method. Tended to create higher densities at polygon corners.
					Random[3] *= (1.0 - Random[2]);
					ThirdVtxWt = 1.0 - Random[2] - Random[3];
					if (Random[4] <= .33333333)
						{
						FolVtx.Lat = Vtx[0]->Lat * Random[2] + Vtx[1]->Lat * Random[3] + Vtx[2]->Lat * ThirdVtxWt;
						FolVtx.Lon = Vtx[0]->Lon * Random[2] + Vtx[1]->Lon * Random[3] + Vtx[2]->Lon * ThirdVtxWt;
						FolVtx.Elev = Vtx[0]->Elev * Random[2] + Vtx[1]->Elev * Random[3] + Vtx[2]->Elev * ThirdVtxWt;
						} // if
					else if (Random[4] >= .66666667)
						{
						FolVtx.Lat = Vtx[0]->Lat * ThirdVtxWt + Vtx[1]->Lat * Random[2] + Vtx[2]->Lat * Random[3];
						FolVtx.Lon = Vtx[0]->Lon * ThirdVtxWt + Vtx[1]->Lon * Random[2] + Vtx[2]->Lon * Random[3];
						FolVtx.Elev = Vtx[0]->Elev * ThirdVtxWt + Vtx[1]->Elev * Random[2] + Vtx[2]->Elev * Random[3];
						} // else if
					else
						{
						FolVtx.Lat = Vtx[0]->Lat * Random[3] + Vtx[1]->Lat * ThirdVtxWt + Vtx[2]->Lat * Random[2];
						FolVtx.Lon = Vtx[0]->Lon * Random[3] + Vtx[1]->Lon * ThirdVtxWt + Vtx[2]->Lon * Random[2];
						FolVtx.Elev = Vtx[0]->Elev * Random[3] + Vtx[1]->Elev * ThirdVtxWt + Vtx[2]->Elev * Random[2];
						} // else 
					#endif // WCS_USE_V5FOLIAGE_DENSITYMETHOD
					// this is better and simpler
					if (Random[2] + Random[3] > 1.0)
						{
						Random[2] = 1.0 - Random[2];
						Random[3] = 1.0 - Random[3];
						} // if
					ThirdVtxWt = 1 - Random[2] - Random[3];
					FolVtx.Lat = Vtx[0]->Lat * Random[2] + Vtx[1]->Lat * Random[3] + Vtx[2]->Lat * ThirdVtxWt;
					FolVtx.Lon = Vtx[0]->Lon * Random[2] + Vtx[1]->Lon * Random[3] + Vtx[2]->Lon * ThirdVtxWt;
					FolVtx.Elev = Vtx[0]->Elev * Random[2] + Vtx[1]->Elev * Random[3] + Vtx[2]->Elev * ThirdVtxWt;

					#ifdef WCS_VECPOLY_EFFECTS
					RenderThisItem = true;
					// If there is a beach involved in this polygon we need to find out if the foliage is above the beach line
					if (Vtx[0]->NodeData->BeachLevel < FLT_MAX || Vtx[1]->NodeData->BeachLevel < FLT_MAX || Vtx[2]->NodeData->BeachLevel < FLT_MAX)
						{
						BeachElev = (Vtx[0]->NodeData->BeachLevel == FLT_MAX ? FolVtx.Elev: Vtx[0]->NodeData->BeachLevel) * Random[2];
						BeachElev += (Vtx[1]->NodeData->BeachLevel == FLT_MAX ? FolVtx.Elev: Vtx[1]->NodeData->BeachLevel) * Random[3];
						BeachElev += (Vtx[2]->NodeData->BeachLevel == FLT_MAX ? FolVtx.Elev: Vtx[2]->NodeData->BeachLevel) * ThirdVtxWt;
						RenderThisItem = RenderAboveBeachLevel ? (FolVtx.Elev > BeachElev): (FolVtx.Elev <= BeachElev);
						} // if
						
					// VNS 3 allows foliage density to vary over the polygon based on vertex weights
					if (RenderThisItem && VertexWeights)
						{
						// weighting of this foliage position based on same weights used for calculating the positional coordinates
						FolMaterialWeight = VertexWeights[0] * Random[2] + VertexWeights[1] * Random[3] + VertexWeights[2] * ThirdVtxWt;
						// if a random number is above FolMaterialWeight, throw out this instance but count it as one done
						RenderThisItem = (Random[4] <= FolMaterialWeight);
						} // if
					#endif // WCS_VECPOLY_EFFECTS
					} // if
				else
					{
					FolVtx.Lat = Poly->Lat;
					FolVtx.Lon = Poly->Lon;
					FolVtx.Elev = Poly->Elev;
					} // else

				if (RenderThisItem)
					{
					// project the foliage base position
					Cam->ProjectVertexDEM(DefCoords, &FolVtx, EarthLatScaleMeters, PlanetRad, 1);	// 1 means convert vertex to cartesian

					// if there are any textures need to transfer texture data, process textures
					FolDens = 1.0;
					if (FolLink->EcoDensTex || FolLink->GrpDensTex || FolLink->FolDensTex || 
						FolLink->EcoHtTex || FolLink->GrpHtTex || FolLink->FolHtTex || FolLink->FolColorTex || FolLink->FolBacklightTex ||
						FolLink->GrpDensTheme || FolLink->FolDensTheme)
						{
						// transfer polygon data to TextureData
						// sets TexData.VDataVecOffsetsValid to 0
						RendData.TransferTextureData(Poly);

						// set foliage coordinates
						RendData.TexData.Elev = FolVtx.Elev;
						RendData.TexData.Latitude = FolVtx.Lat;
						RendData.TexData.Longitude = FolVtx.Lon;
						RendData.TexData.ZDist = FolVtx.ScrnXYZ[2];
						RendData.TexData.QDist = FolVtx.Q;
						RendData.TexData.VDEM[0] = &FolVtx;
						RendData.TexData.WaterDepth = Poly->WaterElev - FolVtx.Elev;
						RendData.TexData.VDataVecOffsetsValid = 0;

						// if there are any density textures we need to reduce the probability of this foliage rendering
						if (FolLink->EcoDensTex)
							{
							// evaluate ecotype density texture
							if ((TexOpacity = FolLink->EcoDensTex->Eval(Value, &RendData.TexData)) > 0.0)
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
							if ((TexOpacity = FolLink->GrpDensTex->Eval(Value, &RendData.TexData)) > 0.0)
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
							if ((TexOpacity = FolLink->FolDensTex->Eval(Value, &RendData.TexData)) > 0.0)
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
						if (Eco->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
							{
							if (FolLink->GrpHtTheme && FolLink->GrpHtTheme->Eval(Value, Poly->Vector))
								{
								FolHeight = Value[0];
								if (Eco->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
									{
									MinHeight = FolLink->Grp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue;
									if (MinHeight < FolHeight)
										{
										FolHeight = MinHeight + (FolHeight - MinHeight) * Random[6];
										} // if
									} // if
								else if (Eco->SecondHeightType == WCS_ECOTYPE_SECONDHT_RANGEPCT)
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
								if (Eco->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
									{
									MinHeight = Eco->AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue;
									if (MinHeight < FolHeight)
										{
										FolHeight = MinHeight + (FolHeight - MinHeight) * Random[6];
										} // if
									} // if
								else if (Eco->SecondHeightType == WCS_ECOTYPE_SECONDHT_RANGEPCT)
									{
									MinHeight = FolHeight - .01 * Eco->AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue;
									FolHeight = MinHeight + 2.0 * (FolHeight - MinHeight) * Random[6];
									} // else if
								else
									{
									MinHeight = FolHeight * .01 * Eco->AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue;
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
							if ((TexOpacity = FolLink->EcoHtTex->Eval(Value, &RendData.TexData)) > 0.0)
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
							if ((TexOpacity = FolLink->GrpHtTex->Eval(Value, &RendData.TexData)) > 0.0)
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
							if ((TexOpacity = FolLink->FolHtTex->Eval(Value, &RendData.TexData)) > 0.0)
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
						if (FolHeight > ShadowMinFolHt || (! ShadowMapInProgress && FolHeight > 0.0))
							{
							// Use factor in default Environment if no Environment specified for this polygon (could be Foliage or Ecosystem Effect).
							EnvHeightFactor = Poly->Env ? Poly->Env->AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_FOLIAGEHTFACT].CurValue: 
								DefaultEnvironment->AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_FOLIAGEHTFACT].CurValue;
							FolHeight *= EnvHeightFactor;
							// now paths diverge between Image Objects and 3D Objects
							if (FolLink->Fol->FoliageType == WCS_FOLIAGE_TYPE_OBJECT3D)
								{
								Rotate[0] = FolLink->Fol->RandomRotate[0] ? FolLink->Fol->Rotate[0] * 2.0 * (Random[7] - .5): 0.0;
								Rotate[1] = FolLink->Fol->RandomRotate[1] ? FolLink->Fol->Rotate[1] * 2.0 * (Random[8] - .5): 0.0;
								Rotate[2] = FolLink->Fol->RandomRotate[2] ? FolLink->Fol->Rotate[2] * 2.0 * (Random[9] - .5): 0.0;
								// render the object, pass the rotation set, height, shading options, position of base
								FolLink->Fol->Obj->Rand.Copy(&Eco->Rand);

								ItemSuppressed = 0;

								if (RealTimeFoliageWrite && RealTimeFoliageWrite->Include3DO && ! ShadowMapInProgress)
									{
									if (FolHeight >= RealTimeFoliageWrite->ConfigParams[WCS_REALTIME_CONFIG_MINHEIGHT].GetCurValue()
										&& FolHeight <= RealTimeFoliageWrite->ConfigParams[WCS_REALTIME_CONFIG_MAXHEIGHT].GetCurValue()
										&& FolVtx.ScrnXYZ[2] <= RealTimeFoliageWrite->ConfigParams[WCS_REALTIME_CONFIG_FARDIST].GetCurValue()
										&& FolVtx.ScrnXYZ[2] >= RealTimeFoliageWrite->ConfigParams[WCS_REALTIME_CONFIG_NEARDIST].GetCurValue())
										{
										if (FolVtx.ScrnXYZ[0] >= 0 && FolVtx.ScrnXYZ[0] < Width && FolVtx.ScrnXYZ[1] >= 0 && FolVtx.ScrnXYZ[1] < Height)
											{
											if (FolListDat = new RealtimeFoliageData())
												{
												unsigned short FolBits;
												if(FolHeight >= 1638)
													{
													FolListDat->Height = (unsigned short)FolHeight;
													FolBits = 0;
													} // if
												else if(FolHeight >= 163)
													{
													FolListDat->Height = (unsigned short)(FolHeight * 10.0);
													FolBits = 0x01;
													} // else if
												else if(FolHeight >= 16)
													{
													FolListDat->Height = (unsigned short)(FolHeight * 100.0);
													FolBits = 0x02;
													} // else if
												else
													{
													FolListDat->Height = (unsigned short)(FolHeight * 1000.0);
													FolBits = 0x03;
													} // else
												if(FolBits)
													{
													FolListDat->Height |= (FolBits << 14);
													} // if
												FolListDat->ElementID = -(short)EffectsBase->GetObjectID(FolLink->Fol->Obj->EffectType, FolLink->Fol->Obj);
												if (fabs(Rotate[0]) >= 360.0)
													Rotate[0] = fmod(Rotate[0], 360.0);	// retains the original sign
												if (Rotate[0] < 0.0)
													Rotate[0] += 360.0;

												if (fabs(Rotate[1]) >= 360.0)
													Rotate[1] = fmod(Rotate[1], 360.0);	// retains the original sign
												if (Rotate[1] < 0.0)
													Rotate[1] += 360.0;

												if (fabs(Rotate[2]) >= 360.0)
													Rotate[2] = fmod(Rotate[2], 360.0);	// retains the original sign
												if (Rotate[2] < 0.0)
													Rotate[2] += 360.0;
												FolListDat->TripleValue[0] = (unsigned char)(Rotate[0] * 255.99 / 360.0);
												FolListDat->TripleValue[1] = (unsigned char)(Rotate[1] * 255.99 / 360.0);
												FolListDat->TripleValue[2] = (unsigned char)(Rotate[2] * 255.99 / 360.0);
												#ifdef WCS_BUILD_RTX
												if (Exporter)
													{
													FolListDat->XYZ[0] = (float)(FolVtx.Lon - TexRefLon);
													FolListDat->XYZ[1] = (float)(FolVtx.Lat - TexRefLat);
													FolListDat->XYZ[2] = (float)(FolVtx.Elev - TexRefElev);
													} // if
												else
												#endif // WCS_BUILD_RTX
													{
													FolListDat->XYZ[0] = (float)(FolVtx.XYZ[0] - TexRefVert->XYZ[0]);
													FolListDat->XYZ[1] = (float)(FolVtx.XYZ[1] - TexRefVert->XYZ[1]);
													FolListDat->XYZ[2] = (float)(FolVtx.XYZ[2] - TexRefVert->XYZ[2]);
													} // else
												#ifdef WCS_BUILD_SX2
												// if this object is flagged as one that has click-to-query, set the flag and the pointers
												if (FolEffect)
													{
													if (((FoliageEffect *)Poly->Object)->IsClickQueryEnabled())
														{
														if (FolLink->Fol->Obj->IsClickQueryEnabled())
															FolListDat->MyEffect = FolLink->Fol->Obj;
														else
															FolListDat->MyEffect = (FoliageEffect *)Poly->Object;
														FolListDat->MyVec = Poly->Vector;
														FolListDat->BitInfo |= WCS_REALTIME_FOLDAT_BITINFO_CLICKQUERY;
														} // if
													else
														{
														FolListDat->MyVec = Poly->Vector;
														FolListDat->BitInfo |= WCS_REALTIME_FOLDAT_BITINFO_VECTORPRESENT;
														} // else
													} // if
												#endif // WCS_BUILD_SX2
												AddFoliageList(FolListDat);
												} // if FolListDat
											} // if
										ItemSuppressed = 1;
										} // if FolHeight
									#ifdef WCS_BUILD_RTX
									else if (Exporter)
										ItemSuppressed = (! Exporter->ExportTexture || Exporter->TextureFoliageType == WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_TERRAININFIRST);
									#endif // WCS_BUILD_RTX
									} // if RealTimeFoliageWrite

								if (! ItemSuppressed)
									{
									ObjStash = Poly->Object;
									Poly->Object = FolLink->Fol->Obj;
									Render3DObject(Poly, &FolVtx, FolLink->Fol->Obj, Rotate, FolHeight);
									Poly->Object = ObjStash;
									RendData.TexData.Object3D = NULL;
									} // if

								} // if 3D Object
							else
								{
								FlipX = (FolLink->Fol->FlipX && Random[7] >= .5);
								if (ShadowMapInProgress && ShadowLight->FlipFoliage)
									FlipX = ! FlipX;
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
									Value[0] = Poly->RGB[0];
									Value[1] = Poly->RGB[1];
									Value[2] = Poly->RGB[2];
									TexOpacity = 1.0;
									} // if
								else if (FolLink->FolColorTheme && FolLink->FolColorTheme->Eval(Value, Poly->Vector))
									{
									// colors will be in range 0-255 hopefully so we need to reduce to 0-1 and elliminate negatives
									if (Value[0] <= 0.0)
										Value[0] = 0.0;
									else
										Value[0] *= (1.0 / 255.0);
									if (Value[1] <= 0.0)
										Value[1] = 0.0;
									else
										Value[1] *= (1.0 / 255.0);
									if (Value[2] <= 0.0)
										Value[2] = 0.0;
									else
										Value[2] *= (1.0 / 255.0);
									TexOpacity = 1.0;
									} // else if
								else if (FolLink->FolColorTex)
									{
									if ((TexOpacity = FolLink->FolColorTex->Eval(Value, &RendData.TexData)) > 0.0)
										{
										if (TexOpacity < 1.0)
											{
											// if gray image blend texture color with replacement color
											if (! FolLink->ColorImage)
												{
												Value[0] += FolLink->Fol->Color.GetCompleteValue(0) * (1.0 - TexOpacity);
												Value[1] += FolLink->Fol->Color.GetCompleteValue(1) * (1.0 - TexOpacity);
												Value[2] += FolLink->Fol->Color.GetCompleteValue(2) * (1.0 - TexOpacity);
												} // else
											} // if
										} // if
									else if (! FolLink->ColorImage)
										{
										Value[0] = FolLink->Fol->Color.GetCompleteValue(0);
										Value[1] = FolLink->Fol->Color.GetCompleteValue(1);
										Value[2] = FolLink->Fol->Color.GetCompleteValue(2);
										} // else if gray image
									// else Value[] already set to 0's by texture evaluator
									} // if
								else
									{
									TexOpacity = 0.0;
									if (! FolLink->ColorImage)
										{
										Value[0] = FolLink->Fol->Color.GetCompleteValue(0);
										Value[1] = FolLink->Fol->Color.GetCompleteValue(1);
										Value[2] = FolLink->Fol->Color.GetCompleteValue(2);
										} // if gray image
									else
										{
										Value[0] = Value[1] = Value[2] = 0.0;
										} // else need to set Value[] to 0's
									} // else

								FolPlotData.OrientationShading = FolLink->Fol->AnimPar[WCS_FOLIAGE_ANIMPAR_ORIENTATIONSHADING].CurValue;
								if (FolLink->FolBacklightTex)
									{
									if ((TexOpacity = FolLink->FolBacklightTex->Eval(Value, &RendData.TexData)) > 0.0)
										{
										if (TexOpacity < 1.0)
											{
											// Value[0] has already been diminished by the texture's opacity
											FolPlotData.OrientationShading *= (1.0 - TexOpacity + Value[0]);
											} // if
										else
											{
											FolPlotData.OrientationShading *= Value[0];
											} // if
										} // if
									} // if

								// calculate the tree height in pixels based on size and distance or Z
								if (Cam->Orthographic || Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
									FolHeightFactor = Cam->VertScale;
								else if (Cam->ApplyLensDistortion)
									FolHeightFactor = Cam->VertScale / FolVtx.Q;
								else
									FolHeightFactor = Cam->VertScale / FolVtx.ScrnXYZ[2];
								FolPlotData.HeightInPixels = FolHeight * FolHeightFactor;

								// compute the tree width based on image proportions and pixel aspect
								FolPlotData.WidthInPixels = FolPlotData.HeightInPixels * FolLink->ImageWidthFactor / Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue;

								if (ShadowMapInProgress)
									{
									// forshorten foliage image based on angle of camera and vertical
									// at angle >= 90 no forshortening
									// maximum forshortening at angle of 20 degrees (arbitrary). We don't want the tree to completely disappear.
									FindPosVector(CamFolVec, Cam->CamPos->XYZ, FolVtx.XYZ);
									ForshortenAngle = VectorAngle(FolVtx.XYZ, CamFolVec);
									if (ForshortenAngle > 0.0)	// cos > 0 means angle less than 90 degrees
										{
										ForshortenAngle = sin(acos(ForshortenAngle));
										FolPlotData.HeightInPixels *= ForshortenAngle;
										if (FolPlotData.HeightInPixels < FolPlotData.WidthInPixels * .5)
											FolPlotData.HeightInPixels = FolPlotData.WidthInPixels * .5;
										} // if
									} // if

								// compute an elevation gradient
								FolPlotData.ElevGrad = -FolHeight / FolPlotData.HeightInPixels;
								FolPlotData.TopElev = FolVtx.Elev + FolHeight;

								if (Opt->RenderFromOverhead && FolVtx.ScrnXYZ[2] > 0.0)
									{
									#ifdef WCS_BUILD_RTX
									if (Exporter)
										FolVtx.ScrnXYZ[2] -= 200.0;
									else
									#endif // WCS_BUILD_RTX
										FolVtx.ScrnXYZ[2] -= 10.0;
									if (FolVtx.ScrnXYZ[2] < 0.0)
										FolVtx.ScrnXYZ[2] = 0.0;
									} // if

								FolPlotData.MaxZOffset = FolHeight * FolLink->ImageWidthFactor * .5;	// .25 was used in v4
								FolPlotData.TestPtrnOffset = Poly->Env ? Poly->Env->FoliageBlending: DefaultEnvironment->FoliageBlending;
								FolPlotData.TestPtrnOffset *= ZMergeDistance;
								MinPixelSize = Poly->Env ? Poly->Env->MinPixelSize: DefaultEnvironment->MinPixelSize;
								if (FolPlotData.WidthInPixels < MinPixelSize)
									FolPlotData.WidthInPixels = MinPixelSize;
								if (FolPlotData.HeightInPixels < MinPixelSize)
									FolPlotData.HeightInPixels = MinPixelSize;
								// paste the foliage image into the scene
								// pass Value[] for replacement color, 1 - TexOpacity for color image color opacity,
								//  Opacity for foliage opacity, FolLink for various values including raster,
								//  pasted image size and position, FolPlotData.ElevGrad, FolPlotData.TopElev...

								ItemSuppressed = 0;

								// made the height criterion a user variable
								// distance criterion are used during creation
								if (RealTimeFoliageWrite && RealTimeFoliageWrite->IncludeImage && ! ShadowMapInProgress)
									{
									if (FolHeight >= RealTimeFoliageWrite->ConfigParams[WCS_REALTIME_CONFIG_MINHEIGHT].GetCurValue() 
										&& FolHeight <= RealTimeFoliageWrite->ConfigParams[WCS_REALTIME_CONFIG_MAXHEIGHT].GetCurValue()
										&& FolVtx.ScrnXYZ[2] <= RealTimeFoliageWrite->ConfigParams[WCS_REALTIME_CONFIG_FARDIST].GetCurValue()
										&& FolVtx.ScrnXYZ[2] >= RealTimeFoliageWrite->ConfigParams[WCS_REALTIME_CONFIG_NEARDIST].GetCurValue())
										{
										if (FolVtx.ScrnXYZ[0] >= 0 && FolVtx.ScrnXYZ[0] < Width && FolVtx.ScrnXYZ[1] >= 0 && FolVtx.ScrnXYZ[1] < Height)
											{
											if (FolListDat = new RealtimeFoliageData())
												{
												unsigned short FolBits;

												if(FolHeight >= 1638)
													{
													FolListDat->Height = (unsigned short)FolHeight;
													FolBits = 0;
													} // if
												else if(FolHeight >= 163)
													{
													FolListDat->Height = (unsigned short)(FolHeight * 10.0);
													FolBits = 0x01;
													} // else if
												else if(FolHeight >= 16)
													{
													FolListDat->Height = (unsigned short)(FolHeight * 100.0);
													FolBits = 0x02;
													} // else if
												else
													{
													FolListDat->Height = (unsigned short)(FolHeight * 1000.0);
													FolBits = 0x03;
													} // else
												if(FolBits)
													{
													FolListDat->Height |= (FolBits << 14);
													} // if
													
												FolListDat->ElementID = (short)FolLink->Rast->GetID();
												FolListDat->BitInfo = FlipX ? WCS_REALTIME_FOLDAT_BITINFO_FLIPX: 0;
												if (FolLink->Shade3D)
													FolListDat->BitInfo |= WCS_REALTIME_FOLDAT_BITINFO_SHADE3D;
												if ((FolListDat->ImageColorOpacity = (unsigned char)((1.0 - TexOpacity) * 255.99)) < 255)
													{
													FolListDat->TripleValue[0] = (unsigned char)WCS_min(Value[0] * 255.99, 255.0);
													FolListDat->TripleValue[1] = (unsigned char)WCS_min(Value[1] * 255.99, 255.0);
													FolListDat->TripleValue[2] = (unsigned char)WCS_min(Value[2] * 255.99, 255.0);
													} // if
												else if (! FolLink->ColorImage)
													{
													FolListDat->ImageColorOpacity = 0;
													FolListDat->TripleValue[0] = (unsigned char)WCS_min(Value[0] * 255.99, 255.0);
													FolListDat->TripleValue[1] = (unsigned char)WCS_min(Value[1] * 255.99, 255.0);
													FolListDat->TripleValue[2] = (unsigned char)WCS_min(Value[2] * 255.99, 255.0);
													} // if gray image
												#ifdef WCS_BUILD_RTX
												if (Exporter)
													{
													FolListDat->XYZ[0] = (float)(FolVtx.Lon - TexRefLon);
													FolListDat->XYZ[1] = (float)(FolVtx.Lat - TexRefLat);
													FolListDat->XYZ[2] = (float)(FolVtx.Elev - TexRefElev);
													} // if
												else
												#endif // WCS_BUILD_RTX
													{
													FolListDat->XYZ[0] = (float)(FolVtx.XYZ[0] - TexRefVert->XYZ[0]);
													FolListDat->XYZ[1] = (float)(FolVtx.XYZ[1] - TexRefVert->XYZ[1]);
													FolListDat->XYZ[2] = (float)(FolVtx.XYZ[2] - TexRefVert->XYZ[2]);
													} // else
												#ifdef WCS_BUILD_SX2
												// if this object is flagged as one that has click-to-query, set the flag and the pointers
												if (FolEffect && ((FoliageEffect *)Poly->Object)->IsClickQueryEnabled())
													{
													FolListDat->MyEffect = (FoliageEffect *)Poly->Object;
													FolListDat->MyVec = Poly->Vector;
													FolListDat->BitInfo |= WCS_REALTIME_FOLDAT_BITINFO_CLICKQUERY;
													} // if
												#endif // WCS_BUILD_SX2
												AddFoliageList(FolListDat);
												} // if FolListDat
											else
												Success = 0;
											} // if
										ItemSuppressed = 1;
										} // if FolHeight
									#ifdef WCS_BUILD_RTX
									else if (Exporter)
										ItemSuppressed = (! Exporter->ExportTexture || Exporter->TextureFoliageType == WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_TERRAININFIRST);
									#endif // WCS_BUILD_RTX
									} // if RealTimeFoliageWrite

								if (! ItemSuppressed)
									{
									FolPlotData.SourceRast = FolLink->Rast;
									FolPlotData.ColorImageOpacity = 1.0 - TexOpacity;
									FolPlotData.ReplaceColor = Value;
									FolPlotData.RenderOccluded = FolLink->RenderOccluded;
									FolPlotData.Shade3D = FolLink->Shade3D;
									FolPlotData.ColorImage = FolLink->ColorImage;
	#ifdef WCS_FORESTRY_WIZARD
	#ifdef WCS_BUILD_GARY
									StemsRendered ++;
	#endif // WCS_BUILD_GARY
	#endif // WCS_FORESTRY_WIZARD
									if (FolVtx.ScrnXYZ[2] > MinimumZ && FolVtx.ScrnXYZ[2] <= MaximumZ)
										{
										if (FlipX)
											PlotFoliageReverse();
										else
											PlotFoliage();
										} // if
									} // if
								} // else it's an image Object
							} // if it's got some height
						} // if we pass the last density test
					} // if item not disallowed by vertex weighting
				} // if

			if (LastTime == 1)
				break;
			// we'll need some more random #s to see if we proceed with more foliage
			Eco->Rand.GenMultiPRN(2, &Random[0]);
			Density -= 1.0;
			if (Density <= 1.0)
				LastTime = 1;
			} // while
		} // if
	} // for

return (Success);

} // Renderer::RenderFoliage
*/
/*===========================================================================*/

int Renderer::RenderFoliageFileFoliage(PolygonData *Poly, VertexDEM *FolVtx, FoliagePreviewData *FPD, Ecotype *Ecotp, double Opacity, unsigned long SeedOffset)
{
double FolHeight, FolHeightFactor, EnvHeightFactor, 
	CamFolVec[3], ForshortenAngle, MinPixelSize, ImageWidthFactor;
int ColorImage, Success = 1, ItemSuppressed = 0;
char CurName[256];

if (! ShadowMapInProgress && Opt->FoliageShadowsOnly)
	return (1);

FolPlotData.Poly = Poly;
FolPlotData.Vert = FolVtx;
FolPlotData.Opacity = Opacity;

if (FPD->CurRast)
	{
	if (FPD->CurRast->Enabled)
		{
		if (! FPD->CurRast->LoadToRender(RenderTime, Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue, WCS_RASTER_INITFLAGS_IMAGELOADED | WCS_RASTER_INITFLAGS_FOLIAGELOADED | WCS_RASTER_INITFLAGS_FOLIAGEDOWNSAMPLED))
			{
			if (! UserMessageOKCAN(FPD->CurRast->GetUserName(), "Unable to load this Image Object!\nContinue without it?"))
				return (0);
			else
				{
				FPD->CurRast->SetEnabled(0);		// so it doesn't try again on each frame
				return (1);
				} // else
			} // if failed
		FolPlotData.OrientationShading = .25;	// the default value
		ColorImage = FPD->CurRast->GetIsColor();

		FolHeight = FPD->Height;
		if (FolHeight > ShadowMinFolHt || (! ShadowMapInProgress && FolHeight > 0.0))
			{
			// Use factor in default Environment if no Environment specified for this polygon (could be Foliage or Ecosystem Effect).
			EnvHeightFactor = DefaultEnvironment->AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_FOLIAGEHTFACT].CurValue;
			FolHeight *= EnvHeightFactor;

			// calculate the tree height in pixels based on size and distance or Z
			if (Cam->Orthographic || Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
				FolHeightFactor = Cam->VertScale;
			else if (Cam->ApplyLensDistortion)
				FolHeightFactor = Cam->VertScale / FolVtx->Q;
			else
				FolHeightFactor = Cam->VertScale / FolVtx->ScrnXYZ[2];
			FolPlotData.HeightInPixels = FolHeight * FolHeightFactor;

			// compute the tree width based on image proportions and pixel aspect
			ImageWidthFactor = (double)FPD->CurRast->Cols / (double)FPD->CurRast->Rows;
			FolPlotData.WidthInPixels = FolPlotData.HeightInPixels * ImageWidthFactor / Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue;

			if (ShadowMapInProgress)
				{
				// forshorten foliage image based on angle of camera and vertical
				// at angle >= 90 no forshortening
				// maximum forshortening at angle of 20 degrees (arbitrary). We don't want the tree to completely disappear.
				FindPosVector(CamFolVec, Cam->CamPos->XYZ, FolVtx->XYZ);
				ForshortenAngle = VectorAngle(FolVtx->XYZ, CamFolVec);
				if (ForshortenAngle > 0.0)	// cos > 0 means angle less than 90 degrees
					{
					ForshortenAngle = sin(acos(ForshortenAngle));
					FolPlotData.HeightInPixels *= ForshortenAngle;
					if (FolPlotData.HeightInPixels < FolPlotData.WidthInPixels * .5)
						FolPlotData.HeightInPixels = FolPlotData.WidthInPixels * .5;
					} // if
				} // if

			// compute an elevation gradient
			FolPlotData.ElevGrad = -FolHeight / FolPlotData.HeightInPixels;
			FolPlotData.TopElev = FolVtx->Elev + FolHeight;

			if (Opt->RenderFromOverhead && FolVtx->ScrnXYZ[2] > 0.0)
				{
				#ifdef WCS_BUILD_RTX
				if (Exporter)
					FolVtx->ScrnXYZ[2] -= 200.0;
				else
				#endif // WCS_BUILD_RTX
					FolVtx->ScrnXYZ[2] -= 10.0;
				if (FolVtx->ScrnXYZ[2] < 0.0)
					FolVtx->ScrnXYZ[2] = 0.0;
				} // if

			FolPlotData.MaxZOffset = FolHeight * ImageWidthFactor * .5;	// .25 was used in v4
			FolPlotData.TestPtrnOffset = Poly->Env ? Poly->Env->FoliageBlending: DefaultEnvironment->FoliageBlending;
			FolPlotData.TestPtrnOffset *= ZMergeDistance;
			MinPixelSize = Poly->Env ? Poly->Env->MinPixelSize: DefaultEnvironment->MinPixelSize;
			if (FolPlotData.WidthInPixels < MinPixelSize)
				FolPlotData.WidthInPixels = MinPixelSize;
			if (FolPlotData.HeightInPixels < MinPixelSize)
				FolPlotData.HeightInPixels = MinPixelSize;
			// paste the foliage image into the scene
			// pass Value[] for replacement color, 1 - TexOpacity for color image color opacity,
			//  Opacity for foliage opacity, FolLink for various values including raster,
			//  pasted image size and position, FolPlotData.ElevGrad, FolPlotData.TopElev...

			ItemSuppressed = 0;

			// made the height criterion a user variable
			// distance criterion are used during creation
			if (RealTimeFoliageWrite && RealTimeFoliageWrite->IncludeImage && ! ShadowMapInProgress)
				{
				if (FolHeight >= RealTimeFoliageWrite->ConfigParams[WCS_REALTIME_CONFIG_MINHEIGHT].GetCurValue() 
					&& FolHeight <= RealTimeFoliageWrite->ConfigParams[WCS_REALTIME_CONFIG_MAXHEIGHT].GetCurValue()
					&& FolVtx->ScrnXYZ[2] <= RealTimeFoliageWrite->ConfigParams[WCS_REALTIME_CONFIG_FARDIST].GetCurValue()
					&& FolVtx->ScrnXYZ[2] >= RealTimeFoliageWrite->ConfigParams[WCS_REALTIME_CONFIG_NEARDIST].GetCurValue())
					{
					if (FolVtx->ScrnXYZ[0] >= 0 && FolVtx->ScrnXYZ[0] < Width && FolVtx->ScrnXYZ[1] >= 0 && FolVtx->ScrnXYZ[1] < Height)
						{
						if (FolListDat = new RealtimeFoliageData())
							{
							unsigned short FolBits;

							if(FolHeight >= 1638)
								{
								FolListDat->Height = (unsigned short)FolHeight;
								FolBits = 0;
								} // if
							else if(FolHeight >= 163)
								{
								FolListDat->Height = (unsigned short)(FolHeight * 10.0);
								FolBits = 0x01;
								} // else if
							else if(FolHeight >= 16)
								{
								FolListDat->Height = (unsigned short)(FolHeight * 100.0);
								FolBits = 0x02;
								} // else if
							else
								{
								FolListDat->Height = (unsigned short)(FolHeight * 1000.0);
								FolBits = 0x03;
								} // else
							if(FolBits)
								{
								FolListDat->Height |= (FolBits << 14);
								} // if
								
							FolListDat->ElementID = (short)FPD->CurRast->GetID();
							FolListDat->BitInfo = FPD->FlipX ? WCS_REALTIME_FOLDAT_BITINFO_FLIPX: 0;
							if (FPD->Shade3D)
								FolListDat->BitInfo |= WCS_REALTIME_FOLDAT_BITINFO_SHADE3D;
							if ((FolListDat->ImageColorOpacity = (unsigned char)(FPD->ColorImageOpacity * 255.99)) < 255)
								{
								FolListDat->TripleValue[0] = (unsigned char)WCS_min(FPD->RGB[0] * 255.99, 255.0);
								FolListDat->TripleValue[1] = (unsigned char)WCS_min(FPD->RGB[1] * 255.99, 255.0);
								FolListDat->TripleValue[2] = (unsigned char)WCS_min(FPD->RGB[2] * 255.99, 255.0);
								} // if
							else if (! ColorImage)
								{
								FolListDat->ImageColorOpacity = 0;
								FolListDat->TripleValue[0] = (unsigned char)WCS_min(FPD->RGB[0] * 255.99, 255.0);
								FolListDat->TripleValue[1] = (unsigned char)WCS_min(FPD->RGB[1] * 255.99, 255.0);
								FolListDat->TripleValue[2] = (unsigned char)WCS_min(FPD->RGB[2] * 255.99, 255.0);
								} // if gray image
							#ifdef WCS_BUILD_RTX
							if (Exporter)
								{
								FolListDat->XYZ[0] = (float)(FolVtx->Lon - TexRefLon);
								FolListDat->XYZ[1] = (float)(FolVtx->Lat - TexRefLat);
								FolListDat->XYZ[2] = (float)(FolVtx->Elev - TexRefElev);
								} // if
							else
							#endif // WCS_BUILD_RTX
								{
								FolListDat->XYZ[0] = (float)(FolVtx->XYZ[0] - TexRefVert->XYZ[0]);
								FolListDat->XYZ[1] = (float)(FolVtx->XYZ[1] - TexRefVert->XYZ[1]);
								FolListDat->XYZ[2] = (float)(FolVtx->XYZ[2] - TexRefVert->XYZ[2]);
								} // else
							#ifdef WCS_BUILD_SX2
							// if this object is flagged as one that has click-to-query, set the flag and the pointers
							if (((FoliageEffect *)Poly->Object)->IsClickQueryEnabled())
								{
								FolListDat->MyEffect = (FoliageEffect *)Poly->Object;
								FolListDat->MyVec = Poly->Vector;
								FolListDat->BitInfo |= WCS_REALTIME_FOLDAT_BITINFO_CLICKQUERY;
								} // if
							#endif // WCS_BUILD_SX2
							AddFoliageList(FolListDat);
							} // if FolListDat
						else
							Success = 0;
						} // if
					ItemSuppressed = 1;
					} // if FolHeight
				#ifdef WCS_BUILD_RTX
				else if (Exporter)
					ItemSuppressed = (! Exporter->ExportTexture || Exporter->TextureFoliageType == WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_TERRAININFIRST);
				#endif // WCS_BUILD_RTX
				} // if RealTimeFoliageWrite

			if (! ItemSuppressed)
				{
				FolPlotData.SourceRast = FPD->CurRast;
				FolPlotData.ColorImageOpacity = FPD->ColorImageOpacity;
				FolPlotData.ReplaceColor = FPD->RGB;
				FolPlotData.RenderOccluded = Ecotp->RenderOccluded;
				FolPlotData.Shade3D = FPD->Shade3D;
				FolPlotData.ColorImage = ColorImage;
		#ifdef WCS_FORESTRY_WIZARD
		#ifdef WCS_BUILD_GARY
				StemsRendered ++;
		#endif // WCS_BUILD_GARY
		#endif // WCS_FORESTRY_WIZARD
				if (FPD->FlipX)
					PlotFoliageReverse();
				else
					PlotFoliage();
				} // if
			} // if height challenge is met
		} // if enabled
	} // if
else if (FPD->Object3D)
	{
	strcpy(CurName, FPD->Object3D->GetName());
	} // if

return (Success);

} // Renderer::RenderFoliageFileFoliage

/*===========================================================================*/

//void Renderer::PlotFoliage(FoliagePlotData *FolPlotData)
void Renderer::PlotFoliage(void)
{
double Dox, Doy, Dex, Dey, dX, dY, Sox, Soy, Cox, Coy, Cex, Cey, wtx, wty, wt, PixWt, MaxWt, ElPt,
	DownFromTop, DownFromTopSum, InFromLeft = 0.0, InFromLeftSum, DownFromTopIncr, 
	MidX = 0.0, Span = 0.0, ZSum, VertNormalWt, HorNormalWt, ZOffsetFactor,	ReplaceColor255[3];
PixelData Pix;
rPixelFragment *PixFrag = NULL;
MathAccellerators *mathTables = GlobalApp->MathTables;
float ZFlt;
long Px, Py, Pxp1, Pyp1, X, Y, DRows, DCols, DxStart, DyStart, PixVal[4], PixZip, SourceZip, i, j;
int FragBits, ReplaceValues;
#ifndef OPENMP_FOLIAGE_PLOTTING
VertexDEM PixelVtx;
#endif // OPENMP_FOLIAGE_PLOTTING
unsigned short NewBits, TotalBits, OldBits;
unsigned char FragFlags;

//CAProfResume();

//FolPlotData.SourceRast = FolPlotData.SourceRast->FindBestDownsampledVersion(1.0 / FolPlotData.WidthInPixels, 1.0 / FolPlotData.HeightInPixels);
FolPlotData.SourceRast = FolPlotData.SourceRast->FindBestDownsampledVersion(FolPlotData.WidthInPixels, FolPlotData.HeightInPixels);
FolPlotData.Vert->ScrnXYZ[2] -= ShadowMapDistanceOffset;

Pix.SetDefaults();
Pix.TexData = &RendData.TexData;
Pix.PixelType = WCS_PIXELTYPE_FOLIAGE;
FragFlags = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FOLIAGE;
if (FolPlotData.RenderOccluded
	#ifdef WCS_BUILD_RTX
	|| Exporter
	#endif // WCS_BUILD_RTX
	)
	FragFlags |= WCS_PIXELFRAG_FLAGBIT_RENDEROCCLUDED;
#ifdef WCS_BUILD_RTX
if (Exporter)
	FragFlags |= WCS_PIXELFRAG_FLAGBIT_HALFFRAGLIMITED;
#endif // WCS_BUILD_RTX
Pix.TexData->ObjectType = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FOLIAGE;
Pix.CloudShading = FolPlotData.OrientationShading;
Pix.TranslucencyExp = 1.0;
Pix.ShadowFlags = FolPlotData.Poly->ShadowFlags;
Pix.ReceivedShadowIntensity = FolPlotData.Poly->ReceivedShadowIntensity;
Pix.ShadowOffset = FolPlotData.Poly->ShadowOffset + FolPlotData.MaxZOffset;
Pix.Object = Pix.TexData->Object = FolPlotData.Poly->Object;
Dox = FolPlotData.Vert->ScrnXYZ[0] - .5 * FolPlotData.WidthInPixels;	//Dx;
Dex = FolPlotData.Vert->ScrnXYZ[0] + .5 * FolPlotData.WidthInPixels;	//Dx + Dw;
Doy = FolPlotData.Vert->ScrnXYZ[1] - FolPlotData.HeightInPixels;		//Dy;
Dey = FolPlotData.Vert->ScrnXYZ[1];							//Dy + Dh;

dX = (double)FolPlotData.SourceRast->Cols / FolPlotData.WidthInPixels;	//Dw;
dY = (double)FolPlotData.SourceRast->Rows / FolPlotData.HeightInPixels;	//Dh;

MaxWt = dX * dY;

Sox = (floor(Dox) - Dox) * dX;
Soy = (floor(Doy) - Doy) * dY;

DxStart = quicklongfloor(Dox); 
DyStart = quicklongfloor(Doy);
DCols = quickftol((ceil(Dex) - floor(Dox)));
DRows = quickftol((ceil(Dey) - floor(Doy)));

//++DevCounter[0];
//DevCounter[1] += DCols;
//DevCounter[2] += DRows;

DownFromTopIncr = 1.0 / (FolPlotData.SourceRast->Rows - 1);
ReplaceColor255[0] = FolPlotData.ReplaceColor[0] * 255.0;
ReplaceColor255[1] = FolPlotData.ReplaceColor[1] * 255.0;
ReplaceColor255[2] = FolPlotData.ReplaceColor[2] * 255.0;

//for (Y = DyStart, Coy = Soy, Cey = Soy + dY, j = 0, PixWt = ZSum = DownFromTopSum = InFromLeftSum = 0.0,
//	PixVal[3] = PixVal[2] = PixVal[1] = PixVal[0] = 0, ElPt = FolPlotData.TopElev;	//, Shade = ShadeStart;
//	j < DRows; j ++, Y ++, Coy += dY, Cey += dY, ElPt += FolPlotData.ElevGrad)	//, Shade += ShadeGrade)
Y = DyStart;
Coy = Soy, Cey = Soy + dY, PixWt = ZSum = DownFromTopSum = InFromLeftSum = 0.0;
PixVal[3] = PixVal[2] = PixVal[1] = PixVal[0] = 0;
ElPt = FolPlotData.TopElev;	//, Shade = ShadeStart;
//for (j = 0; j < DRows; ++j)
for (j = DRows; j != 0; --j)
	{
	if (Y < 0)
		{
		++Y, Coy += dY, Cey += dY, ElPt += FolPlotData.ElevGrad;	//, Shade += ShadeGrade)
		continue;
		} // if
	if (Y >= Height)
		break;
	#ifndef OPENMP_FOLIAGE_PLOTTING
	PixZip = Y * Width + DxStart;
	for (X = DxStart, Cox = Sox, Cex = Sox + dX, i = 0; i < DCols;
		i ++, X ++, Cox += dX, Cex += dX, PixVal[3] = PixVal[2] = PixVal[1] = PixVal[0] = 0, 
		PixWt = ZSum = DownFromTopSum = InFromLeftSum = 0.0, PixZip ++)
		{
		if (X < 0)
			continue;
		if (X >= Width)
			break;
	#else // OPENMP_FOLIAGE_PLOTTING
	#pragma omp parallel for schedule(dynamic, 5) private(Cox, Cex, PixVal, PixWt, ZSum, DownFromTopSum, InFromLeftSum, PixZip, Py, Pyp1, DownFromTop, wty, MidX, Span, Px, Pxp1, wtx, wt, SourceZip, InFromLeft, NewBits, OldBits, TotalBits, FragBits, ZOffsetFactor, ZFlt, PixFrag, VertNormalWt, HorNormalWt, ReplaceValues) firstprivate(Pix)
	for (i = 0; i < DCols; ++i)
		{
		VertexDEM PixelVtx;
		
		X = DxStart + i;
		if (X >= 0 && X < Width)
			{
			Cox = Sox + i * dX;
			Cex = Cox + dX;
			PixVal[0] = PixVal[1] = PixVal[2] = PixVal[3] = 0;
			PixWt = ZSum = DownFromTopSum = InFromLeftSum = 0.0;
			PixZip = Y * Width + DxStart + i;
	#endif // OPENMP_FOLIAGE_PLOTTING
			if (rPixelFragMap || (FolPlotData.Vert->ScrnXYZ[2] < ZBuf[PixZip] || AABuf[PixZip] < 255))
				{
				Py = quickftol(Coy);
				DownFromTop = (Py + .5 - Soy) * DownFromTopIncr;
				if (DownFromTop < 0.0)
					DownFromTop = 0.0;
				for (Pyp1 = Py + 1; Py < Cey && Py < FolPlotData.SourceRast->Rows; Py ++, Pyp1 ++, DownFromTop += DownFromTopIncr)
					{
					double invSpan;

					if (Py < 0)
						continue;
					wty = min((double)Pyp1, Cey) - max((double)Py, Coy);
					if (FolPlotData.SourceRast->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT])
						MidX = FolPlotData.SourceRast->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][Py];
					if (FolPlotData.SourceRast->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN])
						{
						Span = FolPlotData.SourceRast->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][Py] * .5;			// added the .5 on 3/9/96 
						invSpan = 1.0 / Span;
						} // if
					for (Px = quickftol(Cox), Pxp1 = quickftol(Cox + 1); Px < Cex && Px < FolPlotData.SourceRast->Cols; Px ++, Pxp1 ++)
						{
						if (Px < 0)
							continue;
						wtx = min((double)Pxp1, Cex) - max((double)Px, Cox);
						wt = wtx * wty;
						SourceZip = Py * FolPlotData.SourceRast->Cols + Px;
						if (FolPlotData.SourceRast->AltFloatMap)	// I think this test can be elliminated with V4
							wt *= FolPlotData.SourceRast->AltFloatMap[SourceZip];
						if (FolPlotData.ColorImage)
							{
							if (FolPlotData.SourceRast->Red[SourceZip] || FolPlotData.SourceRast->Green[SourceZip] || FolPlotData.SourceRast->Blue[SourceZip])
								{
								if (FolPlotData.Shade3D)
									{
									if (Span > 0.0)
										{
										//InFromLeft = (Px + .5 - MidX) / Span;		// Angle would not exceed .5 so cut span in half - see above
										InFromLeft = (Px + .5 - MidX) * invSpan;		// Angle would not exceed .5 so cut span in half - see above
										} // if
									if (InFromLeft > 1.0)
										InFromLeft = 1.0;
									else if (InFromLeft < -1.0)
										InFromLeft = -1.0;
									PixWt += wt;
									PixVal[0] = quickftol(PixVal[0] + wt * (unsigned int)FolPlotData.SourceRast->Red[SourceZip]);
									PixVal[1] = quickftol(PixVal[1] + wt * (unsigned int)FolPlotData.SourceRast->Green[SourceZip]);
									PixVal[2] = quickftol(PixVal[2] + wt * (unsigned int)FolPlotData.SourceRast->Blue[SourceZip]);
									if (FolPlotData.SourceRast->AltByteMap)
										PixVal[3] = quickftol(PixVal[3] + wt * (unsigned int)FolPlotData.SourceRast->AltByteMap[SourceZip]);
									DownFromTopSum += DownFromTop * wt;
									InFromLeftSum += InFromLeft * wt;
									} // if 3D shading
								else
									{
									PixWt += wt;
									PixVal[0] = quickftol(PixVal[0] + wt * (unsigned int)FolPlotData.SourceRast->Red[SourceZip]);
									PixVal[1] = quickftol(PixVal[1] + wt * (unsigned int)FolPlotData.SourceRast->Green[SourceZip]);
									PixVal[2] = quickftol(PixVal[2] + wt * (unsigned int)FolPlotData.SourceRast->Blue[SourceZip]);
									if (FolPlotData.SourceRast->AltByteMap)
										PixVal[3] = quickftol(PixVal[3] + wt * (unsigned int)FolPlotData.SourceRast->AltByteMap[SourceZip]);
									} // else no 3D shading
								} // if pixel colored
							} // if rgb
						else
							{
							if (FolPlotData.SourceRast->Red[SourceZip])
								{
								if (FolPlotData.Shade3D)
									{
									if (Span > 0.0)
										//InFromLeft = (Px + .5 - MidX) / Span;		// Angle would not exceed .5 so cut span in half - see above
										InFromLeft = (Px + .5 - MidX) * invSpan;		// Angle would not exceed .5 so cut span in half - see above
									if (InFromLeft > 1.0)
										InFromLeft = 1.0;
									else if (InFromLeft < -1.0)
										InFromLeft = -1.0;
									PixWt += wt;
									PixVal[0] = quickftol(PixVal[0] + wt * (unsigned int)FolPlotData.SourceRast->Red[SourceZip]);
									if (FolPlotData.SourceRast->AltByteMap)
										PixVal[3] = quickftol(PixVal[3] + wt * (unsigned int)FolPlotData.SourceRast->AltByteMap[SourceZip]);
									DownFromTopSum += DownFromTop * wt;
									InFromLeftSum += InFromLeft * wt;
									} // if 3D shading
								else
									{
									PixWt += wt;
									PixVal[0] = quickftol(PixVal[0] + wt * (unsigned int)FolPlotData.SourceRast->Red[SourceZip]);
									if (FolPlotData.SourceRast->AltByteMap)
										PixVal[3] = quickftol(PixVal[3] + wt * (unsigned int)FolPlotData.SourceRast->AltByteMap[SourceZip]);
									} // else
								} // if pixel is colored
							} // else gray
						} // for Px
					} // for Py

				// Plot pixel
				if (PixWt > 0.0)
					{
					double InvPixWt = 1.0 / PixWt;

					// common to if & else
					PixVal[0] = quickftol(PixVal[0] * InvPixWt);
					PixVal[3] = quickftol(PixVal[3] * InvPixWt);
					if (FolPlotData.ColorImage)
						{
						PixVal[1] = quickftol(PixVal[1] * InvPixWt);
						PixVal[2] = quickftol(PixVal[2] * InvPixWt);
						if (FolPlotData.ColorImageOpacity < 1.0)
							{
							PixVal[0] = quickftol(PixVal[0] * FolPlotData.ColorImageOpacity + ReplaceColor255[0]);
							PixVal[1] = quickftol(PixVal[1] * FolPlotData.ColorImageOpacity + ReplaceColor255[1]);
							PixVal[2] = quickftol(PixVal[2] * FolPlotData.ColorImageOpacity + ReplaceColor255[2]);
							} // if
						} // if
					else
						{
						PixVal[1] = quickftol(PixVal[0] * FolPlotData.ReplaceColor[1]);
						PixVal[2] = quickftol(PixVal[0] * FolPlotData.ReplaceColor[2]);
						PixVal[0] = quickftol(PixVal[0] * FolPlotData.ReplaceColor[0]);
						} // else
					if (FolPlotData.Shade3D)
						{
						DownFromTopSum *= InvPixWt;
						InFromLeftSum *= InvPixWt;
						} // if
					PixWt /= MaxWt;
					if (PixWt > 1.0)
						PixWt = 1.0;
					NewBits = (unsigned short)(PixWt * 255.9 * FolPlotData.Opacity);
					ZOffsetFactor = PixVal[3] * (1.0 / 255.0);
					// testing 8/11/02
					ZSum = FolPlotData.Vert->ScrnXYZ[2];
					/* testing 8/11/02 removed the z offset because it cause reflection problems
					ZSum = Vert->ScrnXYZ[2] - (MaxZOffset * ZOffsetFactor);
					*/
					if (ZSum < .01)
						ZSum = .01;	// don't let pixels go into negative z space, causes weirdness in 3d compositing
									// and unprojecting coordinates
					if (FragBits = NewBits)
						{
						ZFlt = (float)ZSum;
						if (rPixelFragMap)
							{
							if (FragBits > 255)
								FragBits = 255;
							if (! (PixFrag = rPixelFragMap[PixZip].PlotPixel(rPixelBlock, ZFlt, (unsigned char)FragBits, ~0UL, ~0UL, FragmentDepth, 
								FragFlags)))
								{
								continue;
								} // if
							} // if
						else
							{
							double NewWt;
							unsigned short PolyBits, RemBits, DifBits;

							OldBits = TreeAABuf[PixZip];
							PolyBits = AABuf[PixZip];
							if (ZFlt > TreeZBuf[PixZip])					// new tree farther than old tree
								{
								if (ZFlt - TreeZBuf[PixZip] < FolPlotData.TestPtrnOffset && NewBits + OldBits > 255)	// modified this block 8/2/98
									{
									NewWt = 1.0 - (.5 * (1.0 + (ZFlt - TreeZBuf[PixZip]) / FolPlotData.TestPtrnOffset));
									RemBits = 255 - OldBits;
									DifBits = NewBits - RemBits;
									NewBits = (unsigned short)(.5 + RemBits + DifBits * NewWt);
									OldBits = (unsigned short)(.5 + OldBits - DifBits * NewWt);
									if (! NewBits)
										continue;
									} // if
								else if (OldBits >= 255 || (ZFlt > ZBuf[PixZip] && PolyBits >= 255))
									continue;
								if (NewBits + OldBits > 255)
									NewBits = 255 - OldBits;
								TotalBits = NewBits + OldBits;
								} // if
							else
								{
								if (TreeZBuf[PixZip] - ZFlt < FolPlotData.TestPtrnOffset && NewBits + OldBits > 255)	// modified this block 8/2/98
									{
									NewWt = .5 * (1.0 + (TreeZBuf[PixZip] - ZFlt) / FolPlotData.TestPtrnOffset);
									RemBits = 255 - NewBits;
									DifBits = OldBits - RemBits;
									OldBits = (unsigned short)(.5 + RemBits + DifBits * NewWt);
									NewBits = (unsigned short)(.5 + NewBits - DifBits * NewWt);
									if (! NewBits)
										continue;
									} // if
								else if (ZFlt > ZBuf[PixZip] && PolyBits >= 255)
									continue;
								} // else
							} // if ! FragMap

						Pix.RGB[0] = PixVal[0] * (1.0 / 255.0);
						Pix.RGB[1] = PixVal[1] * (1.0 / 255.0);
						Pix.RGB[2] = PixVal[2] * (1.0 / 255.0);

						if (! ShadowMapInProgress)
							{
							PixelVtx.ScrnXYZ[0] = X + .5;
							PixelVtx.ScrnXYZ[1] = Y + .5;
							PixelVtx.ScrnXYZ[2] = PixelVtx.Q = ZSum;
							Cam->UnProjectVertexDEM(DefCoords, &PixelVtx, EarthLatScaleMeters, PlanetRad, LightTexturesExist);
							Pix.XYZ[0] = PixelVtx.XYZ[0];
							Pix.XYZ[1] = PixelVtx.XYZ[1];
							Pix.XYZ[2] = PixelVtx.XYZ[2];
							Pix.Q = PixelVtx.Q;
							Pix.Elev = ElPt;
							// view vector is used for specularity and for that it is best to have it point toward the camera
							Pix.ViewVec[0] = Cam->CamPos->XYZ[0] - Pix.XYZ[0];
							Pix.ViewVec[1] = Cam->CamPos->XYZ[1] - Pix.XYZ[1];
							Pix.ViewVec[2] = Cam->CamPos->XYZ[2] - Pix.XYZ[2];
							UnitVector(Pix.ViewVec); // Illumination code requires this to be unitized in advance to save time later
							//if (! Exporter)
							//	{
								// make surface normal point toward camera
								Pix.Normal[0] = Pix.ViewVec[0];
								Pix.Normal[1] = Pix.ViewVec[1];
								Pix.Normal[2] = Pix.ViewVec[2];
								// needs to be normalized since it will be put in Normal XYZ buffers
								// UnitVector(Pix.Normal); // no longer necessary, since ViewVec is pre-normalized
								// modify normal based on vertical and horizontal position of pixel in foliage image
								if (FolPlotData.Shade3D)
									{
									VertNormalWt = DownFromTopSum >= .75 ? 0.0: cos(DownFromTopSum * .666666 * Pi);
									HorNormalWt = (mathTables->ASinTab.Lookup(InFromLeftSum) * 2.0 / Pi);
									Pix.Normal[0] += Cam->CamVertical->XYZ[0] * VertNormalWt;
									Pix.Normal[1] += Cam->CamVertical->XYZ[1] * VertNormalWt;
									Pix.Normal[2] += Cam->CamVertical->XYZ[2] * VertNormalWt;
									Pix.Normal[0] += Cam->CamRight->XYZ[0] * HorNormalWt;
									Pix.Normal[1] += Cam->CamRight->XYZ[1] * HorNormalWt;
									Pix.Normal[2] += Cam->CamRight->XYZ[2] * HorNormalWt;
									UnitVector(Pix.Normal);
									// if its green foliage make it translucent
									if (Pix.RGB[1] > Pix.RGB[0] && Pix.RGB[1] > Pix.RGB[2])
										{
										Pix.Translucency = (1.0 - ZOffsetFactor) * .5;
										Pix.Translucency *= Pix.Translucency;
										} // if
									else
										Pix.Translucency = 0.0;
									} // if
							//	} // if
							//else
							//	{
								// make surface normal 0 which will prevent orientation shading
							//	Pix.Normal[0] = Pix.Normal[1] = Pix.Normal[2] = 0.0;
							//	} // else
							if (! LightTexturesExist)
								IlluminatePixel(&Pix);
							else
								{
								#pragma omp critical (OMP_CRITICAL_TEXTUREDATAINUSE)
									{
									RendData.TransferTextureData(&PixelVtx);
									IlluminatePixel(&Pix);
									} // end #pragma omp critical
								} // if
							} // if not shadow map

						ReplaceValues = 0;

						if (rPixelFragMap)
							{
							ReplaceValues = (ZFlt <= rPixelFragMap[PixZip].GetFirstZ());
							} // else
						else
							{
							if (Pix.RGB[0] >= 1.0)
								PixVal[0] = 255;
							else
								PixVal[0] = quickftol(Pix.RGB[0] * 255.0);
							if (Pix.RGB[1] >= 1.0)
								PixVal[1] = 255;
							else
								PixVal[1] = quickftol(Pix.RGB[1] * 255.0);
							if (Pix.RGB[2] >= 1.0)
								PixVal[2] = 255;
							else
								PixVal[2] = quickftol(Pix.RGB[2] * 255.0);


							if (ZFlt > TreeZBuf[PixZip])					// new tree farther than old tree
								{
								TreeBitmap[0][PixZip] = (unsigned char)((TreeBitmap[0][PixZip] * OldBits + PixVal[0] * NewBits) / TotalBits);
								TreeBitmap[1][PixZip] = (unsigned char)((TreeBitmap[1][PixZip] * OldBits + PixVal[1] * NewBits) / TotalBits);
								TreeBitmap[2][PixZip] = (unsigned char)((TreeBitmap[2][PixZip] * OldBits + PixVal[2] * NewBits) / TotalBits);
								TreeAABuf[PixZip] = (unsigned char)TotalBits;
								} // if ZFlt
							else										// new tree closer than old tree
								{
								if (OldBits)
									{
									if (NewBits + OldBits > 255)
										OldBits = 255 - NewBits;
									TotalBits = NewBits + OldBits;
									TreeBitmap[0][PixZip] = (unsigned char)((TreeBitmap[0][PixZip] * OldBits + PixVal[0] * NewBits) / TotalBits);
									TreeBitmap[1][PixZip] = (unsigned char)((TreeBitmap[1][PixZip] * OldBits + PixVal[1] * NewBits) / TotalBits);
									TreeBitmap[2][PixZip] = (unsigned char)((TreeBitmap[2][PixZip] * OldBits + PixVal[2] * NewBits) / TotalBits);
									TreeAABuf[PixZip] = (unsigned char)TotalBits;
									} // if
								else
									{
									TreeBitmap[0][PixZip] = (unsigned char)PixVal[0];
									TreeBitmap[1][PixZip] = (unsigned char)PixVal[1];
									TreeBitmap[2][PixZip] = (unsigned char)PixVal[2];
									TotalBits = NewBits;
									TreeAABuf[PixZip] = (unsigned char)TotalBits;
									} // else
								TreeZBuf[PixZip] = ZFlt;
								if (! ShadowMapInProgress && ZFlt < ZBuf[PixZip])
									{
									ReplaceValues = 1;
									} // if
								} // else ZFlt
							#ifndef OPENMP_FOLIAGE_PLOTTING
							ScreenPixelPlotTwoBuf(Bitmap, ZBuf, AABuf, TreeBitmap, TreeZBuf, TreeAABuf, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
							#endif // OPENMP_FOLIAGE_PLOTTING
							} // if
						if (ReplaceValues)
							{
							if (LatBuf)
								LatBuf[PixZip] = (float)(FolPlotData.Vert->Lat - TexRefLat);
							if (LonBuf)
								LonBuf[PixZip] = (float)(FolPlotData.Vert->Lon - TexRefLon);
							if (IllumBuf)
								IllumBuf[PixZip] = (float)Pix.Illum;
							if (RelElBuf)
								RelElBuf[PixZip] = (float)FolPlotData.Poly->RelEl;
							if (SlopeBuf)
								SlopeBuf[PixZip] = (float)FolPlotData.Poly->Slope;
							if (AspectBuf)
								AspectBuf[PixZip] = (float)FolPlotData.Poly->Aspect;
							if (rPixelFragMap || (! ReflectionBuf || ! ReflectionBuf[PixZip] || TotalBits >= 255))
								{
								if (NormalBuf[0])
									NormalBuf[0][PixZip] = (float)Pix.Normal[0];
								if (NormalBuf[1])
									NormalBuf[1][PixZip] = (float)Pix.Normal[1];
								if (NormalBuf[2])
									NormalBuf[2][PixZip] = (float)Pix.Normal[2];
								} // if
							if (ObjectBuf)
								ObjectBuf[PixZip] = FolPlotData.Poly->Object;
							if (ObjTypeBuf)
								ObjTypeBuf[PixZip] = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FOLIAGE;
							if (ElevBuf)
								ElevBuf[PixZip] = (float)ElPt;
							} // if

						if (PixFrag)
							{
							PixFrag->PlotPixel(rPixelBlock, Pix.RGB, Pix.Reflectivity, Pix.Normal);
							#ifndef OPENMP_FOLIAGE_PLOTTING
							ScreenPixelPlotFragments(&rPixelFragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);
							#endif // OPENMP_FOLIAGE_PLOTTING
							} // if
						} // if NewBits
					} // if PixWt
				} // if
		#ifdef OPENMP_FOLIAGE_PLOTTING
			} // if in image bounds (OpenMP only)
		#endif // OPENMP_FOLIAGE_PLOTTING
		} // for x
	#ifdef OPENMP_FOLIAGE_PLOTTING
	if (rPixelFragMap)
		{
		for (X = DxStart, i = 0, PixZip = Y * Width + DxStart; i < DCols;
			++i, ++X, ++PixZip)
			{
			if (X < 0)
				continue;
			if (X >= Width)
				break;
			ScreenPixelPlotFragments(&rPixelFragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);
			} // for
		} // if
	else
		{
		for (X = DxStart, i = 0, PixZip = Y * Width + DxStart; i < DCols;
			++i, ++X, ++PixZip)
			{
			if (X < 0)
				continue;
			if (X >= Width)
				break;
			ScreenPixelPlotTwoBuf(Bitmap, ZBuf, AABuf, TreeBitmap, TreeZBuf, TreeAABuf, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
			} // for
		} // else
	#endif // OPENMP_FOLIAGE_PLOTTING
	++Y, Coy += dY, Cey += dY, ElPt += FolPlotData.ElevGrad;	//, Shade += ShadeGrade)
	} // for y

//CAProfPause();
//TotalTime += StopHiResTimerSecs();

} // Renderer::PlotFoliage

/*===========================================================================*/

//void Renderer::PlotFoliageReverse(FoliagePlotData *FolPlotData)
void Renderer::PlotFoliageReverse()
{
double Dox, Doy, Dex, Dey, dX, dY, Sox, Soy, Cox, Coy, Cex, Cey,
	wtx, wty, wt, PixWt, MaxWt, ElPt,
	DownFromTop, DownFromTopSum, InFromLeft = 0.0, InFromLeftSum, DownFromTopIncr, 
	MidX = 0.0, Span = 0.0, ZSum, VertNormalWt, HorNormalWt, ZOffsetFactor,
	ReplaceColor255[3];
MathAccellerators *mathTables = GlobalApp->MathTables;
rPixelFragment *PixFrag = NULL;
float ZFlt;
long Px, Py, Pxp1, Pyp1, X, Y, DRows, DCols, DxStart, DyStart, PixVal[4],
	PixZip, SourceZip, i, j;
int FragBits, ReplaceValues;
unsigned short NewBits, TotalBits, OldBits;
unsigned char FragFlags;
PixelData Pix;
#ifndef OPENMP_FOLIAGE_PLOTTING
VertexDEM PixelVtx;
#endif // OPENMP_FOLIAGE_PLOTTING

//FolPlotData.SourceRast = FolPlotData.SourceRast->FindBestDownsampledVersion(1.0 / FolPlotData.WidthInPixels, 1.0 / FolPlotData.HeightInPixels);
FolPlotData.SourceRast = FolPlotData.SourceRast->FindBestDownsampledVersion(FolPlotData.WidthInPixels, FolPlotData.HeightInPixels);
FolPlotData.Vert->ScrnXYZ[2] -= ShadowMapDistanceOffset;

Pix.SetDefaults();
Pix.TexData = &RendData.TexData;
Pix.PixelType = WCS_PIXELTYPE_FOLIAGE;
FragFlags = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FOLIAGE;
if (FolPlotData.RenderOccluded
	#ifdef WCS_BUILD_RTX
	|| Exporter
	#endif // WCS_BUILD_RTX
	)
	FragFlags |= WCS_PIXELFRAG_FLAGBIT_RENDEROCCLUDED;
#ifdef WCS_BUILD_RTX
if (Exporter)
	FragFlags |= WCS_PIXELFRAG_FLAGBIT_HALFFRAGLIMITED;
#endif // WCS_BUILD_RTX
Pix.TexData->ObjectType = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FOLIAGE;
Pix.CloudShading = FolPlotData.OrientationShading;
Pix.TranslucencyExp = 1.0;
Pix.ShadowFlags = FolPlotData.Poly->ShadowFlags;
Pix.ReceivedShadowIntensity = FolPlotData.Poly->ReceivedShadowIntensity;
Pix.ShadowOffset = FolPlotData.Poly->ShadowOffset + FolPlotData.MaxZOffset;
Pix.Object = Pix.TexData->Object = FolPlotData.Poly->Object;
Dex = FolPlotData.Vert->ScrnXYZ[0] - .5 * FolPlotData.WidthInPixels;	//Dx;
Dox = FolPlotData.Vert->ScrnXYZ[0] + .5 * FolPlotData.WidthInPixels;	//Dx + Dw;
Doy = FolPlotData.Vert->ScrnXYZ[1] - FolPlotData.HeightInPixels;		//Dy;
Dey = FolPlotData.Vert->ScrnXYZ[1];							//Dy + Dh;

dX = (double)FolPlotData.SourceRast->Cols / FolPlotData.WidthInPixels;	//Dw;
dY = (double)FolPlotData.SourceRast->Rows / FolPlotData.HeightInPixels;	//Dh;

MaxWt = dX * dY;

Sox = (Dox - ceil(Dox)) * dX;
Soy = (floor(Doy) - Doy) * dY;

DxStart = quicklongfloor(Dox); 
DyStart = quicklongfloor(Doy);
DCols = quickftol(ceil(Dox) - floor(Dex));
DRows = quickftol(ceil(Dey) - floor(Doy));

DownFromTopIncr = 1.0 / (FolPlotData.SourceRast->Rows - 1);
ReplaceColor255[0] = FolPlotData.ReplaceColor[0] * 255;
ReplaceColor255[1] = FolPlotData.ReplaceColor[1] * 255;
ReplaceColor255[2] = FolPlotData.ReplaceColor[2] * 255;

for (Y = DyStart, Coy = Soy, Cey = Soy + dY, j = 0, PixWt = ZSum = DownFromTopSum = InFromLeftSum = 0.0,
	PixVal[3] = PixVal[2] = PixVal[1] = PixVal[0] = 0, ElPt = FolPlotData.TopElev;	//, Shade = ShadeStart;
	j < DRows; j ++, Y ++, Coy += dY, Cey += dY, ElPt += FolPlotData.ElevGrad)	//, Shade += ShadeGrade)
	{
	if (Y < 0)
		continue;
	if (Y >= Height)
		break;
	#ifndef OPENMP_FOLIAGE_PLOTTING
	PixZip = Y * Width + DxStart;
	for (X = DxStart, Cox = Sox, Cex = Sox + dX, i = 0; i < DCols;
		i ++, X --, Cox += dX, Cex += dX, PixVal[3] = PixVal[2] = PixVal[1] = PixVal[0] = 0, 
		PixWt = ZSum = DownFromTopSum = InFromLeftSum = 0.0, PixZip --)
		{
		if (X >= Width)
			continue;
		if (X < 0)
			break;
	#else // OPENMP_FOLIAGE_PLOTTING
	#pragma omp parallel for schedule(dynamic, 5) private(Cox, Cex, PixVal, PixWt, ZSum, DownFromTopSum, InFromLeftSum, PixZip, Py, Pyp1, DownFromTop, wty, MidX, Span, Px, Pxp1, wtx, wt, SourceZip, InFromLeft, NewBits, OldBits, TotalBits, FragBits, ZOffsetFactor, ZFlt, PixFrag, VertNormalWt, HorNormalWt, ReplaceValues) firstprivate(Pix)
	for (i = 0; i < DCols; ++i)
		{
		VertexDEM PixelVtx;
		
		X = DxStart - i;
		if (X >= 0 && X < Width)
			{
			Cox = Sox + i * dX;
			Cex = Cox + dX;
			PixVal[0] = PixVal[1] = PixVal[2] = PixVal[3] = 0;
			PixWt = ZSum = DownFromTopSum = InFromLeftSum = 0.0;
			PixZip = Y * Width + DxStart - i;
	#endif // OPENMP_FOLIAGE_PLOTTING
			if (rPixelFragMap || (FolPlotData.Vert->ScrnXYZ[2] < ZBuf[PixZip] || AABuf[PixZip] < 255))
				{
				Py = quickftol(Coy);
				DownFromTop = (Py + .5 - Soy) * DownFromTopIncr;
				if (DownFromTop < 0.0)
					DownFromTop = 0.0;
				for (Pyp1 = Py + 1; Py < Cey && Py < FolPlotData.SourceRast->Rows; Py ++, Pyp1 ++, DownFromTop += DownFromTopIncr)
					{
					if (Py < 0)
						continue;
					wty = min((double)Pyp1, Cey) - max((double)Py, Coy);
					if (FolPlotData.SourceRast->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT])
						MidX = FolPlotData.SourceRast->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][Py];
					if (FolPlotData.SourceRast->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN])
						Span = FolPlotData.SourceRast->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][Py] * .5;			// added the .5 on 3/9/96 
					for (Px = quickftol(Cox), Pxp1 = quickftol(Cox + 1); Px < Cex && Px < FolPlotData.SourceRast->Cols; Px ++, Pxp1 ++)
						{
						if (Px < 0)
							continue;
						wtx = min((double)Pxp1, Cex) - max((double)Px, Cox);
						wt = wtx * wty;
						SourceZip = Py * FolPlotData.SourceRast->Cols + Px;
						if (FolPlotData.SourceRast->AltFloatMap)	// I think this test can be elliminated with V4
							wt *= FolPlotData.SourceRast->AltFloatMap[SourceZip];
						if (FolPlotData.ColorImage)
							{
							if (FolPlotData.SourceRast->Red[SourceZip] || FolPlotData.SourceRast->Green[SourceZip] || FolPlotData.SourceRast->Blue[SourceZip])
								{
								if (FolPlotData.Shade3D)
									{
									if (Span > 0.0)
										InFromLeft = (MidX - Px - .5) / Span;		// Angle would not exceed .5 so cut span in half - see above
									if (InFromLeft > 1.0)
										InFromLeft = 1.0;
									else if (InFromLeft < -1.0)
										InFromLeft = -1.0;
									PixWt += wt;
									PixVal[0] = quickftol(PixVal[0] + wt * (unsigned int)FolPlotData.SourceRast->Red[SourceZip]);
									PixVal[1] = quickftol(PixVal[1] + wt * (unsigned int)FolPlotData.SourceRast->Green[SourceZip]);
									PixVal[2] = quickftol(PixVal[2] + wt * (unsigned int)FolPlotData.SourceRast->Blue[SourceZip]);
									if (FolPlotData.SourceRast->AltByteMap)
										PixVal[3] = quickftol(PixVal[3] + wt * (unsigned int)FolPlotData.SourceRast->AltByteMap[SourceZip]);
									DownFromTopSum += DownFromTop * wt;
									InFromLeftSum += InFromLeft * wt;
									} // if 3D shading
								else
									{
									PixWt += wt;
									PixVal[0] = quickftol(PixVal[0] + wt * (unsigned int)FolPlotData.SourceRast->Red[SourceZip]);
									PixVal[1] = quickftol(PixVal[1] + wt * (unsigned int)FolPlotData.SourceRast->Green[SourceZip]);
									PixVal[2] = quickftol(PixVal[2] + wt * (unsigned int)FolPlotData.SourceRast->Blue[SourceZip]);
									if (FolPlotData.SourceRast->AltByteMap)
										PixVal[3] = quickftol(PixVal[3] + wt * (unsigned int)FolPlotData.SourceRast->AltByteMap[SourceZip]);
									} // else no 3D shading
								} // if pixel colored
							} // if rgb
						else
							{
							if (FolPlotData.SourceRast->Red[SourceZip])
								{
								if (FolPlotData.Shade3D)
									{
									if (Span > 0.0)
										InFromLeft = (MidX - Px - .5) / Span;		// Angle would not exceed .5 so cut span in half - see above
									if (InFromLeft > 1.0)
										InFromLeft = 1.0;
									else if (InFromLeft < -1.0)
										InFromLeft = -1.0;
									PixWt += wt;
									PixVal[0] = quickftol(PixVal[0] + wt * (unsigned int)FolPlotData.SourceRast->Red[SourceZip]);
									if (FolPlotData.SourceRast->AltByteMap)
										PixVal[3] = quickftol(PixVal[3] + wt * (unsigned int)FolPlotData.SourceRast->AltByteMap[SourceZip]);
									DownFromTopSum += DownFromTop * wt;
									InFromLeftSum += InFromLeft * wt;
									} // if 3D shading
								else
									{
									PixWt += wt;
									PixVal[0] = quickftol(PixVal[0] + wt * (unsigned int)FolPlotData.SourceRast->Red[SourceZip]);
									if (FolPlotData.SourceRast->AltByteMap)
										PixVal[3] = quickftol(PixVal[3] + wt * (unsigned int)FolPlotData.SourceRast->AltByteMap[SourceZip]);
									} // else
								} // if pixel is colored
							} // else gray
						} // for Px
					} // for Py

				// Plot pixel
				if (PixWt > 0.0)
					{
					double InvPixWt = 1.0 / PixWt;	// F2_MOD

					if (FolPlotData.ColorImage)
						{
						PixVal[0] = quickftol(PixVal[0] * InvPixWt);
						PixVal[1] = quickftol(PixVal[1] * InvPixWt);
						PixVal[2] = quickftol(PixVal[2] * InvPixWt);
						PixVal[3] = quickftol(PixVal[3] * InvPixWt);
						if (FolPlotData.ColorImageOpacity < 1.0)
							{
							PixVal[0] = quickftol(PixVal[0] * FolPlotData.ColorImageOpacity + ReplaceColor255[0]);
							PixVal[1] = quickftol(PixVal[1] * FolPlotData.ColorImageOpacity + ReplaceColor255[1]);
							PixVal[2] = quickftol(PixVal[2] * FolPlotData.ColorImageOpacity + ReplaceColor255[2]);
							} // if
						} // if
					else
						{
						PixVal[0] = quickftol(PixVal[0] * InvPixWt);
						PixVal[3] = quickftol(PixVal[3] * InvPixWt);
						PixVal[1] = quickftol(PixVal[0] * FolPlotData.ReplaceColor[1]);
						PixVal[2] = quickftol(PixVal[0] * FolPlotData.ReplaceColor[2]);
						PixVal[0] = quickftol(PixVal[0] * FolPlotData.ReplaceColor[0]);
						} // else
					if (FolPlotData.Shade3D)
						{
						DownFromTopSum *= InvPixWt;
						InFromLeftSum *= InvPixWt;
						} // if
					PixWt /= MaxWt;
					if (PixWt > 1.0)
						PixWt = 1.0;
					NewBits = (unsigned short)(PixWt * 255.9 * FolPlotData.Opacity);
					ZOffsetFactor = PixVal[3] * (1.0 / 255.0);
					// testing 8/11/02
					ZSum = FolPlotData.Vert->ScrnXYZ[2];
					/* testing 8/11/02 removed the z offset because it cause reflection problems
					ZSum = Vert->ScrnXYZ[2] - (MaxZOffset * ZOffsetFactor);
					*/
					if (ZSum < .01)
						ZSum = .01;	// don't let pixels go into negative z space, causes weirdness in 3d compositing
									// and unprojecting coordinates
					if (FragBits = NewBits)
						{
						ZFlt = (float)ZSum;
						if (rPixelFragMap)
							{
							if (FragBits > 255)
								FragBits = 255;
							if (! (PixFrag = rPixelFragMap[PixZip].PlotPixel(rPixelBlock, ZFlt, (unsigned char)FragBits, ~0UL, ~0UL, FragmentDepth, 
								FragFlags)))
								{
								continue;
								} // if
							} // if
						else
							{
							double NewWt;
							unsigned short PolyBits, RemBits, DifBits;

							OldBits = TreeAABuf[PixZip];
							PolyBits = AABuf[PixZip];
							if (ZFlt > TreeZBuf[PixZip])					// new tree farther than old tree
								{
								if (ZFlt - TreeZBuf[PixZip] < FolPlotData.TestPtrnOffset && NewBits + OldBits > 255)	// modified this block 8/2/98
									{
									NewWt = 1.0 - (.5 * (1.0 + (ZFlt - TreeZBuf[PixZip]) / FolPlotData.TestPtrnOffset));
									RemBits = 255 - OldBits;
									DifBits = NewBits - RemBits;
									NewBits = (unsigned short)(.5 + RemBits + DifBits * NewWt);
									OldBits = (unsigned short)(.5 + OldBits - DifBits * NewWt);
									if (! NewBits)
										continue;
									} // if
								else if (OldBits >= 255 || (ZFlt > ZBuf[PixZip] && PolyBits >= 255))
									continue;
								if (NewBits + OldBits > 255)
									NewBits = 255 - OldBits;
								TotalBits = NewBits + OldBits;
								} // if
							else
								{
								if (TreeZBuf[PixZip] - ZFlt < FolPlotData.TestPtrnOffset && NewBits + OldBits > 255)	// modified this block 8/2/98
									{
									NewWt = .5 * (1.0 + (TreeZBuf[PixZip] - ZFlt) / FolPlotData.TestPtrnOffset);
									RemBits = 255 - NewBits;
									DifBits = OldBits - RemBits;
									OldBits = (unsigned short)(.5 + RemBits + DifBits * NewWt);
									NewBits = (unsigned short)(.5 + NewBits - DifBits * NewWt);
									if (! NewBits)
										continue;
									} // if
								else if (ZFlt > ZBuf[PixZip] && PolyBits >= 255)
									continue;
								} // else
							} // if ! FragMap

						Pix.RGB[0] = PixVal[0] * (1.0 / 255.0);
						Pix.RGB[1] = PixVal[1] * (1.0 / 255.0);
						Pix.RGB[2] = PixVal[2] * (1.0 / 255.0);

						if (! ShadowMapInProgress)
							{
							PixelVtx.ScrnXYZ[0] = X + .5;
							PixelVtx.ScrnXYZ[1] = Y + .5;
							PixelVtx.ScrnXYZ[2] = PixelVtx.Q = ZSum;
							Cam->UnProjectVertexDEM(DefCoords, &PixelVtx, EarthLatScaleMeters, PlanetRad, LightTexturesExist);
							Pix.XYZ[0] = PixelVtx.XYZ[0];
							Pix.XYZ[1] = PixelVtx.XYZ[1];
							Pix.XYZ[2] = PixelVtx.XYZ[2];
							Pix.Q = PixelVtx.Q;
							Pix.Elev = ElPt;
							// view vector is used for specularity and for that it is best to have it point toward the camera
							Pix.ViewVec[0] = Cam->CamPos->XYZ[0] - Pix.XYZ[0];
							Pix.ViewVec[1] = Cam->CamPos->XYZ[1] - Pix.XYZ[1];
							Pix.ViewVec[2] = Cam->CamPos->XYZ[2] - Pix.XYZ[2];
							UnitVector(Pix.ViewVec); // Illumination code requires this to be unitized in advance to save time later
							//if (! Exporter)
							//	{
								// make surface normal point toward camera
								Pix.Normal[0] = Pix.ViewVec[0];
								Pix.Normal[1] = Pix.ViewVec[1];
								Pix.Normal[2] = Pix.ViewVec[2];
								// needs to be normalized since it will be put in Normal XYZ buffers
								// UnitVector(Pix.Normal); // no longer necessary, since ViewVec is pre-normalized
								// modify normal based on vertical and horizontal position of pixel in foliage image
								if (FolPlotData.Shade3D)
									{
									VertNormalWt = DownFromTopSum >= .75 ? 0.0: cos(DownFromTopSum * .666666 * Pi);
									HorNormalWt = (mathTables->ASinTab.Lookup(InFromLeftSum) * 2.0 / Pi);
									Pix.Normal[0] += Cam->CamVertical->XYZ[0] * VertNormalWt;
									Pix.Normal[1] += Cam->CamVertical->XYZ[1] * VertNormalWt;
									Pix.Normal[2] += Cam->CamVertical->XYZ[2] * VertNormalWt;
									Pix.Normal[0] += Cam->CamRight->XYZ[0] * HorNormalWt;
									Pix.Normal[1] += Cam->CamRight->XYZ[1] * HorNormalWt;
									Pix.Normal[2] += Cam->CamRight->XYZ[2] * HorNormalWt;
									UnitVector(Pix.Normal);
									// if its green foliage make it translucent
									if (Pix.RGB[1] > Pix.RGB[0] && Pix.RGB[1] > Pix.RGB[2])
										{
										Pix.Translucency = (1.0 - ZOffsetFactor) * .5;
										Pix.Translucency *= Pix.Translucency;
										} // if
									else
										Pix.Translucency = 0.0;
									} // if
							//	} // if
							//else
							//	{
								// make surface normal 0 which will prevent orientation shading
							//	Pix.Normal[0] = Pix.Normal[1] = Pix.Normal[2] = 0.0;
							//	} // else
							if (! LightTexturesExist)
								IlluminatePixel(&Pix);
							else
								{
								#pragma omp critical (OMP_CRITICAL_TEXTUREDATAINUSE)
									{
									RendData.TransferTextureData(&PixelVtx);
									IlluminatePixel(&Pix);
									} // end #pragma omp critical
								} // if
							} // if not shadow map

						ReplaceValues = 0;

						if (rPixelFragMap)
							{
							ReplaceValues = (ZFlt <= rPixelFragMap[PixZip].GetFirstZ());
							} // else
						else
							{
							if (Pix.RGB[0] >= 1.0)
								PixVal[0] = 255;
							else
								PixVal[0] = quickftol(Pix.RGB[0] * 255.0);
							if (Pix.RGB[1] >= 1.0)
								PixVal[1] = 255;
							else
								PixVal[1] = quickftol(Pix.RGB[1] * 255.0);
							if (Pix.RGB[2] >= 1.0)
								PixVal[2] = 255;
							else
								PixVal[2] = quickftol(Pix.RGB[2] * 255.0);


							if (ZFlt > TreeZBuf[PixZip])					// new tree farther than old tree
								{
								TreeBitmap[0][PixZip] = (unsigned char)((TreeBitmap[0][PixZip] * OldBits + PixVal[0] * NewBits) / TotalBits);
								TreeBitmap[1][PixZip] = (unsigned char)((TreeBitmap[1][PixZip] * OldBits + PixVal[1] * NewBits) / TotalBits);
								TreeBitmap[2][PixZip] = (unsigned char)((TreeBitmap[2][PixZip] * OldBits + PixVal[2] * NewBits) / TotalBits);
								TreeAABuf[PixZip] = (unsigned char)TotalBits;
								} // if ZFlt
							else										// new tree closer than old tree
								{
								if (OldBits)
									{
									if (NewBits + OldBits > 255)
										OldBits = 255 - NewBits;
									TotalBits = NewBits + OldBits;
									TreeBitmap[0][PixZip] = (unsigned char)((TreeBitmap[0][PixZip] * OldBits + PixVal[0] * NewBits) / TotalBits);
									TreeBitmap[1][PixZip] = (unsigned char)((TreeBitmap[1][PixZip] * OldBits + PixVal[1] * NewBits) / TotalBits);
									TreeBitmap[2][PixZip] = (unsigned char)((TreeBitmap[2][PixZip] * OldBits + PixVal[2] * NewBits) / TotalBits);
									TreeAABuf[PixZip] = (unsigned char)TotalBits;
									} // if
								else
									{
									TreeBitmap[0][PixZip] = (unsigned char)PixVal[0];
									TreeBitmap[1][PixZip] = (unsigned char)PixVal[1];
									TreeBitmap[2][PixZip] = (unsigned char)PixVal[2];
									TotalBits = NewBits;
									TreeAABuf[PixZip] = (unsigned char)TotalBits;
									} // else
								TreeZBuf[PixZip] = ZFlt;
								if (! ShadowMapInProgress && ZFlt < ZBuf[PixZip])
									{
									ReplaceValues = 1;
									} // if
								} // else ZFlt
							#ifndef OPENMP_FOLIAGE_PLOTTING
							ScreenPixelPlotTwoBuf(Bitmap, ZBuf, AABuf, TreeBitmap, TreeZBuf, TreeAABuf, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
							#endif // OPENMP_FOLIAGE_PLOTTING
							} // if
						if (ReplaceValues)
							{
							if (LatBuf)
								LatBuf[PixZip] = (float)(FolPlotData.Vert->Lat - TexRefLat);
							if (LonBuf)
								LonBuf[PixZip] = (float)(FolPlotData.Vert->Lon - TexRefLon);
							if (IllumBuf)
								IllumBuf[PixZip] = (float)Pix.Illum;
							if (RelElBuf)
								RelElBuf[PixZip] = (float)FolPlotData.Poly->RelEl;
							if (SlopeBuf)
								SlopeBuf[PixZip] = (float)FolPlotData.Poly->Slope;
							if (AspectBuf)
								AspectBuf[PixZip] = (float)FolPlotData.Poly->Aspect;
							if (rPixelFragMap || (! ReflectionBuf || ! ReflectionBuf[PixZip] || TotalBits >= 255))
								{
								if (NormalBuf[0])
									NormalBuf[0][PixZip] = (float)Pix.Normal[0];
								if (NormalBuf[1])
									NormalBuf[1][PixZip] = (float)Pix.Normal[1];
								if (NormalBuf[2])
									NormalBuf[2][PixZip] = (float)Pix.Normal[2];
								} // if
							if (ObjectBuf)
								ObjectBuf[PixZip] = FolPlotData.Poly->Object;
							if (ObjTypeBuf)
								ObjTypeBuf[PixZip] = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FOLIAGE;
							if (ElevBuf)
								ElevBuf[PixZip] = (float)ElPt;
							} // if

						if (PixFrag)
							{
							PixFrag->PlotPixel(rPixelBlock, Pix.RGB, Pix.Reflectivity, Pix.Normal);
							#ifndef OPENMP_FOLIAGE_PLOTTING
							ScreenPixelPlotFragments(&rPixelFragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);
							#endif // OPENMP_FOLIAGE_PLOTTING
							} // if
						} // if NewBits
					} // if PixWt
				} // if
		#ifdef OPENMP_FOLIAGE_PLOTTING
			} // if in image bounds (OpenMP only)
		#endif // OPENMP_FOLIAGE_PLOTTING
		} // for x
	#ifdef OPENMP_FOLIAGE_PLOTTING
	if (rPixelFragMap)
		{
		for (X = DxStart, i = 0, PixZip = Y * Width + DxStart; i < DCols;
			++i, --X, --PixZip)
			{
			if (X < 0)
				continue;
			if (X >= Width)
				break;
			ScreenPixelPlotFragments(&rPixelFragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);
			} // for
		} // if
	else
		{
		for (X = DxStart, i = 0, PixZip = Y * Width + DxStart; i < DCols;
			++i, --X, --PixZip)
			{
			if (X < 0)
				continue;
			if (X >= Width)
				break;
			ScreenPixelPlotTwoBuf(Bitmap, ZBuf, AABuf, TreeBitmap, TreeZBuf, TreeAABuf, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
			} // for
		} // else
	#endif // OPENMP_FOLIAGE_PLOTTING
	//++Y; Coy += dY;	Cey += dY; ElPt += FolPlotData.ElevGrad;	//, Shade += ShadeGrade)
	} // for y

} // Renderer::PlotFoliageReverse

/*===========================================================================*/

int Renderer::ProcessOneFoliageExport(VertexDEM *Vert, Raster *SourceRast, AnimColorTime *ReplaceRGB, 
	int Shade3D, double TopElev, double ElevGrad, int CreateClipMap, Light *Lumens, int DoubleSample, long ActualWidth)
{
double Dox, Doy, Dex, Dey, dX, dY, Sox, Soy, Cox, Coy, Cex, Cey,
	wtx, wty, wt, PixWt, MaxWt, ElPt,
	DownFromTop, DownFromTopSum, InFromLeft = 0.0, InFromLeftSum, DownFromTopIncr, 
	MidX = 0.0, Span = 0.0, VertNormalWt, HorNormalWt,
	ReplaceColor[3], WidthInPixels, HeightInPixels;
MathAccellerators *mathTables = GlobalApp->MathTables;
long Px, Py, Pxp1, Pyp1, X, Y, DRows, DCols, DxStart, DyStart, PixVal[4],
	PixZip, SourceZip, i, j, ColorImage, HalfHeight;
int Success = 1;
PixelData Pix;
VertexDEM PixelVtx;
unsigned short NewBits;
unsigned char BGColor[3];

WidthInPixels = DoubleSample ? (Width - 2): Width;
HeightInPixels = DoubleSample ? (Height - 2) * .5: Height;	// width and height are defined in Renderer
HalfHeight = DoubleSample ? Height / 2: Height;

//SourceRast = SourceRast->FindBestDownsampledVersion(1.0 / WidthInPixels, 1.0 / HeightInPixels);
SourceRast = SourceRast->FindBestDownsampledVersion(WidthInPixels, HeightInPixels);

ColorImage = SourceRast->Red && SourceRast->Green && SourceRast->Blue;

Pix.SetDefaults();
Dox = DoubleSample ? 1.0: 0.0;	//Vert->ScrnXYZ[0] - .5 * WidthInPixels;	//Dx;
Dex = DoubleSample ? Width - Dox: WidthInPixels;	//Vert->ScrnXYZ[0] + .5 * WidthInPixels;	//Dx + Dw;
Doy = DoubleSample ? 1.0: 0.0;	//Vert->ScrnXYZ[1] - HeightInPixels;		//Dy;
Dey = HeightInPixels;	//Vert->ScrnXYZ[1];							//Dy + Dh;

dX = (double)SourceRast->Cols / WidthInPixels;	//Dw;
dY = (double)SourceRast->Rows / HeightInPixels;	//Dh;

MaxWt = dX * dY;

Sox = (WCS_floor(Dox) - Dox) * dX;
Soy = (WCS_floor(Doy) - Doy) * dY;

DxStart = quicklongfloor(Dox);
DyStart = quicklongfloor(Doy);
DCols = quickftol(WCS_ceil(Dex) - WCS_floor(Dox));
DRows = quickftol(WCS_ceil(Dey) - WCS_floor(Doy));

if ((fabs(SourceRast->AverageBand[0] - .5) < .001) && 
	(fabs(SourceRast->AverageBand[1] - .5) < .001) &&
	(fabs(SourceRast->AverageBand[2] - .5) < .001))
	SourceRast->ComputeAverageBands();

DownFromTopIncr = 1.0 / (SourceRast->Rows - 1);
if (ReplaceRGB)
	{
	ReplaceColor[0] = ReplaceRGB->GetClampedCompleteValue(0);
	ReplaceColor[1] = ReplaceRGB->GetClampedCompleteValue(1);
	ReplaceColor[2] = ReplaceRGB->GetClampedCompleteValue(2);
	BGColor[0] = (unsigned char)(ReplaceColor[0] * 255.0);
	BGColor[1] = (unsigned char)(ReplaceColor[1] * 255.0);
	BGColor[2] = (unsigned char)(ReplaceColor[2] * 255.0);
	} // if
else
	{
	ReplaceColor[0] = 127.0 * (1.0 / 255.0);
	ReplaceColor[1] = 162.0 * (1.0 / 255.0);
	ReplaceColor[2] = 95.0 * (1.0 / 255.0);
	BGColor[0] = (unsigned char)(SourceRast->AverageBand[0] * 255.0);
	BGColor[1] = (unsigned char)(SourceRast->AverageBand[1] * 255.0);
	BGColor[2] = (unsigned char)(SourceRast->AverageBand[2] * 255.0);
	} // else

// rather arbitary
Pix.Luminosity = .25;
PixelVtx.ScrnXYZ[0] = WidthInPixels * .5;
PixelVtx.ScrnXYZ[1] = HeightInPixels * .5;
PixelVtx.ScrnXYZ[2] = PixelVtx.Q = Vert->ScrnXYZ[2];
Cam->UnProjectVertexDEM(DefCoords, &PixelVtx, EarthLatScaleMeters, PlanetRad, 0);
Pix.Q = PixelVtx.Q;

if (Master)
	Master->ProcInit(DRows, "Processing Foliage Image");

for (Y = DyStart, Coy = Soy, Cey = Soy + dY, j = 0, PixWt = DownFromTopSum = InFromLeftSum = 0.0,
	PixVal[3] = PixVal[2] = PixVal[1] = PixVal[0] = 0, ElPt = TopElev;	//, Shade = ShadeStart;
	j < DRows; j ++, Y ++, Coy += dY, Cey += dY, ElPt += ElevGrad)	//, Shade += ShadeGrade)
	{
	// this needs to go here because of continue statements in this loop
	if (Master)
		{
		Master->ProcUpdate(j);
		if (! Master->IsRunning())
			{
			Success = 0;
			break;
			} // if
		} // if
	if (Y < 0)
		continue;
	if (Y >= Height)
		break;
	PixZip = Y * Width + DxStart;
	for (X = DxStart, Cox = Sox, Cex = Sox + dX, i = 0; i < DCols;
		i ++, X ++, Cox += dX, Cex += dX, PixVal[3] = PixVal[2] = PixVal[1] = PixVal[0] = 0, 
		PixWt = DownFromTopSum = InFromLeftSum = 0.0, PixZip ++)
		{
		if (X < 0)
			continue;
		if (X >= Width)
			break;
		Py = quickftol(Coy);
		DownFromTop = (Py + .5 - Soy) * DownFromTopIncr;
		if (DownFromTop < 0.0)
			DownFromTop = 0.0;
		for (Pyp1 = Py + 1; Py < Cey && Py < SourceRast->Rows; Py ++, Pyp1 ++, DownFromTop += DownFromTopIncr)
			{
			if (Py < 0)
				continue;
			wty = min((double)Pyp1, Cey) - max((double)Py, Coy);
			if (SourceRast->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT])
				MidX = SourceRast->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][Py];
			if (SourceRast->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN])
				Span = SourceRast->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][Py] * .5;			// added the .5 on 3/9/96 
			for (Px = quickftol(Cox), Pxp1 = quickftol(Cox + 1); Px < Cex && Px < SourceRast->Cols; Px ++, Pxp1 ++)
				{
				if (Px < 0)
					continue;
				wtx = min((double)Pxp1, Cex) - max((double)Px, Cox);
				wt = wtx * wty;
				SourceZip = Py * SourceRast->Cols + Px;
				if (SourceRast->AltFloatMap)	// I think this test can be elliminated with V4
					wt *= SourceRast->AltFloatMap[SourceZip];
				if (ColorImage)
					{
					if (SourceRast->Red[SourceZip] || SourceRast->Green[SourceZip] || SourceRast->Blue[SourceZip])
						{
						if (Span > 0.0)
							InFromLeft = (Px + .5 - MidX) / Span;		// Angle would not exceed .5 so cut span in half - see above
						if (InFromLeft > 1.0)
							InFromLeft = 1.0;
						else if (InFromLeft < -1.0)
							InFromLeft = -1.0;
						PixWt += wt;
						PixVal[0] = quickftol(PixVal[0] + wt * (unsigned int)SourceRast->Red[SourceZip]);
						PixVal[1] = quickftol(PixVal[1] + wt * (unsigned int)SourceRast->Green[SourceZip]);
						PixVal[2] = quickftol(PixVal[2] + wt * (unsigned int)SourceRast->Blue[SourceZip]);
						if (SourceRast->AltByteMap)
							PixVal[3] = quickftol(PixVal[3] + wt * (unsigned int)SourceRast->AltByteMap[SourceZip]);
						DownFromTopSum += DownFromTop * wt;
						InFromLeftSum += InFromLeft * wt;
						} // if pixel colored
					} // if rgb
				else
					{
					if (SourceRast->Red[SourceZip])
						{
						if (Span > 0.0)
							InFromLeft = (Px + .5 - MidX) / Span;		// Angle would not exceed .5 so cut span in half - see above
						if (InFromLeft > 1.0)
							InFromLeft = 1.0;
						else if (InFromLeft < -1.0)
							InFromLeft = -1.0;
						PixWt += wt;
						PixVal[0] = quickftol(PixVal[0] + wt * (unsigned int)SourceRast->Red[SourceZip]);
						if (SourceRast->AltByteMap)
							PixVal[3] = quickftol(PixVal[3] + wt * (unsigned int)SourceRast->AltByteMap[SourceZip]);
						DownFromTopSum += DownFromTop * wt;
						InFromLeftSum += InFromLeft * wt;
						} // if pixel is colored
					} // else gray
				} // for
			} // for

		// Plot pixel
		if (PixWt > 0.0)
			{
			double InvPixWt = 1.0 / PixWt;
			if (ColorImage)
				{
				PixVal[0] = quickftol(PixVal[0] * InvPixWt);
				PixVal[1] = quickftol(PixVal[1] * InvPixWt);
				PixVal[2] = quickftol(PixVal[2] * InvPixWt);
				PixVal[3] = quickftol(PixVal[3] * InvPixWt);
				} // if
			else
				{
				PixVal[0] = quickftol(PixVal[0] * InvPixWt);
				PixVal[3] = quickftol(PixVal[3] * InvPixWt);
				PixVal[1] = quickftol(PixVal[0] * ReplaceColor[1]);
				PixVal[2] = quickftol(PixVal[0] * ReplaceColor[2]);
				PixVal[0] = quickftol(PixVal[0] * ReplaceColor[0]);
				} // else
			DownFromTopSum *= InvPixWt;
			InFromLeftSum *= InvPixWt;

			PixWt /= MaxWt;
			if (PixWt > 1.0)
				PixWt = 1.0;
			NewBits = (unsigned short)(PixWt * 255.9);

			if (NewBits)
				{
				Pix.RGB[0] = PixVal[0] * (1.0 / 255.0);
				Pix.RGB[1] = PixVal[1] * (1.0 / 255.0);
				Pix.RGB[2] = PixVal[2] * (1.0 / 255.0);

				if (Shade3D)
					{
					Pix.XYZ[0] = PixelVtx.XYZ[0];
					Pix.XYZ[1] = PixelVtx.XYZ[1];
					Pix.XYZ[2] = PixelVtx.XYZ[2];
					Pix.Elev = ElPt;
					// make surface normal point toward camera
					Pix.Normal[0] = Cam->CamPos->XYZ[0] - Pix.XYZ[0];
					Pix.Normal[1] = Cam->CamPos->XYZ[1] - Pix.XYZ[1];
					Pix.Normal[2] = Cam->CamPos->XYZ[2] - Pix.XYZ[2];
					// needs to be normalized since it will be put in Normal XYZ buffers
					UnitVector(Pix.Normal);
					// modify normal based on vertical and horizontal position of pixel in foliage image

					VertNormalWt = DownFromTopSum >= .75 ? 0.0: cos(DownFromTopSum * .666666 * Pi);
					HorNormalWt = (mathTables->ASinTab.Lookup(InFromLeftSum) * 2.0 / Pi);
					//Pix.Normal[0] += Cam->CamVertical->XYZ[0] * VertNormalWt;
					//Pix.Normal[1] += Cam->CamVertical->XYZ[1] * VertNormalWt;
					//Pix.Normal[2] += Cam->CamVertical->XYZ[2] * VertNormalWt;
					//Pix.Normal[0] += Cam->CamRight->XYZ[0] * HorNormalWt;
					//Pix.Normal[1] += Cam->CamRight->XYZ[1] * HorNormalWt;
					//Pix.Normal[2] += Cam->CamRight->XYZ[2] * HorNormalWt;
					if (VertNormalWt > 1.0)
						VertNormalWt = 1.0;
					if (HorNormalWt > 1.0)
						HorNormalWt = 1.0;
					if (HorNormalWt < -1.0)
						HorNormalWt = -1.0;
					HorNormalWt *= ((1.0 - VertNormalWt) * .6 + .4);
					Pix.Normal[0] = Cam->CamVertical->XYZ[0] * VertNormalWt + Pix.Normal[0] * (1.0 - VertNormalWt);
					Pix.Normal[1] = Cam->CamVertical->XYZ[1] * VertNormalWt + Pix.Normal[1] * (1.0 - VertNormalWt);
					Pix.Normal[2] = Cam->CamVertical->XYZ[2] * VertNormalWt + Pix.Normal[2] * (1.0 - VertNormalWt);
					Pix.Normal[0] = Cam->CamRight->XYZ[0] * HorNormalWt + Pix.Normal[0] * (1.0 - fabs(HorNormalWt));
					Pix.Normal[1] = Cam->CamRight->XYZ[1] * HorNormalWt + Pix.Normal[1] * (1.0 - fabs(HorNormalWt));
					Pix.Normal[2] = Cam->CamRight->XYZ[2] * HorNormalWt + Pix.Normal[2] * (1.0 - fabs(HorNormalWt));
					UnitVector(Pix.Normal);
					Pix.Translucency = 0.0;

					// not ready for prime time yet
					IlluminateExportPixel(&Pix, Lumens);
					} // if

				if (Pix.RGB[0] >= 1.0)
					PixVal[0] = 255;
				else
					PixVal[0] = quickftol(Pix.RGB[0] * 255.0);
				if (Pix.RGB[1] >= 1.0)
					PixVal[1] = 255;
				else
					PixVal[1] = quickftol(Pix.RGB[1] * 255.0);
				if (Pix.RGB[2] >= 1.0)
					PixVal[2] = 255;
				else
					PixVal[2] = quickftol(Pix.RGB[2] * 255.0);

				Bitmap[0][PixZip] = (unsigned char)PixVal[0];
				Bitmap[1][PixZip] = (unsigned char)PixVal[1];
				Bitmap[2][PixZip] = (unsigned char)PixVal[2];
				if (CreateClipMap)
					AABuf[PixZip] = NewBits > 127 ? 255: 0;
				else
					AABuf[PixZip] = (unsigned char)NewBits;
				ScreenPixelPlot(Bitmap, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
				} // if NewBits
			} // if PixWt
		else
			{
			Bitmap[0][PixZip] = BGColor[0];
			Bitmap[1][PixZip] = BGColor[1];
			Bitmap[2][PixZip] = BGColor[2];
			} // else 
		} // for x
	} // for y

if (DoubleSample)
	{
	// mirror each row to the bottom of the image
	/* this was for an old flavor of Lightwave which isn't used anymore
	for (j = 0, Y = Height - 1; j < Height / 2; j ++, Y --)
		{
		SourceZip = j * Width;
		PixZip = Y * Width;
		for (X = 0; X < Width; X ++, PixZip ++, SourceZip ++)
			{
			Bitmap[0][PixZip] = Bitmap[0][SourceZip];
			Bitmap[1][PixZip] = Bitmap[1][SourceZip];
			Bitmap[2][PixZip] = Bitmap[2][SourceZip];
			AABuf[PixZip] = AABuf[SourceZip];
			ScreenPixelPlot(Bitmap, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
			} // for
		} // for
	*/
	// this is for GoogleEarth
	for (PixZip = 0; PixZip < Width; ++PixZip)
		{
		Bitmap[0][PixZip] = 0;
		Bitmap[1][PixZip] = 0;
		Bitmap[2][PixZip] = 0;
		AABuf[PixZip] = 0;
		} // for
	for (Y = 1, PixZip = Width, SourceZip = 2 * Width -1; Y < HalfHeight; ++Y, PixZip += Width, SourceZip += Width)
		{
		Bitmap[0][PixZip] = 0;
		Bitmap[1][PixZip] = 0;
		Bitmap[2][PixZip] = 0;
		AABuf[PixZip] = 0;
		Bitmap[0][SourceZip] = 0;
		Bitmap[1][SourceZip] = 0;
		Bitmap[2][SourceZip] = 0;
		AABuf[SourceZip] = 0;
		} // for
	for (Y = HalfHeight, PixZip = Width * HalfHeight; Y < Height; ++Y)
		{
		for (X = 0; X < Width; ++X, ++PixZip)
			{
			Bitmap[0][PixZip] = 0;
			Bitmap[1][PixZip] = 0;
			Bitmap[2][PixZip] = 0;
			AABuf[PixZip] = 0;
			} // for
		} // for
	} // if

if (Master)
	Master->ProcClear();
return (Success);

} // Renderer::ProcessOneFoliageExport

/*===========================================================================*/

int Renderer::RenderFoliage(FoliageVertexList *ListElement)
{
VertexDEM ObjVert;
PolygonData Poly;
FoliageEffect *Folg;

if (! ShadowMapInProgress && Opt->FoliageShadowsOnly)
	return (1);

if (Folg = ListElement->Fol)
	{
	if (! ShadowMapInProgress || (ShadowLight && ShadowLight->PassTest(Folg)))
		{
		Folg->FindBasePosition(&RendData, &ObjVert, &Poly, ListElement->Vec, ListElement->Point);
		Cam->ProjectVertexDEM(DefCoords, &ObjVert, EarthLatScaleMeters, PlanetRad, 1);
		if (ObjVert.ScrnXYZ[2] > 0.0)
			{
			if (ObjVert.ScrnXYZ[2] > MinimumZ && ObjVert.ScrnXYZ[2] <= MaximumZ)
				{
				if (ShadowsExist)
					EffectsBase->EvalShadows(&RendData, &Poly);

				if (! ShadowMapInProgress && Folg->Ecotp.DissolveEnabled
					&& ObjVert.ScrnXYZ[2] > Folg->Ecotp.DissolveDistance)
					{
					// opacity is the amount of dissolve
					if (Folg->Ecotp.DissolveDistance > 0.0)
						{
						Poly.OverstoryDissolve[0] = (ObjVert.ScrnXYZ[2] - Folg->Ecotp.DissolveDistance) / Folg->Ecotp.DissolveDistance;
						if (Poly.OverstoryDissolve[0] > 1.0)
							Poly.OverstoryDissolve[0] = 1.0;
						} // if
					else
						Poly.OverstoryDissolve[0] = 1.0;
					} // if
				if (! Folg->UseFoliageFile && ListElement->Vec && ListElement->Point)
					{
					// VertexData[] in RenderFoliage() is not needed for foliage effects since placement is explicit
					#ifdef WCS_FORESTRY_WIZARD
					// 1 = foliage effect, true = render above beach level (which is irrelevant for foliage effects anyway)
					if (! RenderForestryFoliage(&Poly, NULL, &Folg->Ecotp, 1.0 - Poly.OverstoryDissolve[0], 1.0, NULL, WCS_SEEDOFFSET_FOLIAGEEFFECT, 1, true))
						return (0);
					#endif // WCS_FORESTRY_WIZARD
					} // if
				#ifdef WCS_BUILD_SX2
				else if (Folg->UseFoliageFile && Folg->FoliageFileIndex.CellDat && ListElement->Point)
					{
					FoliagePreviewData FPD;

					// ListElement->Point is a RealtimeFoliageData pointer
					Folg->FindBasePosition(&RendData, &ObjVert, &Poly, NULL, ListElement->Point);
					// get the data about the foliage to render
					((RealtimeFoliageData *)ListElement->Point)->InterpretFoliageRecord(EffectsBase, ImageBase, &FPD);
					// Send specific data to the foliage renderer that bypasses all the other foliage decision making tree
					if (! RenderFoliageFileFoliage(&Poly, &ObjVert, &FPD, &Folg->Ecotp, 1.0 - Poly.OverstoryDissolve[0], WCS_SEEDOFFSET_FOLIAGEEFFECT))
						return (0);
					} // else
				#endif // WCS_BUILD_SX2
				} // if
			} // if
		} // if
	} // if

return (1);

} // Renderer::RenderFoliage

/*===========================================================================*/

// a modified form of this code was stolen and turned into ViewGUI::DrawWalls
int Renderer::RenderFence(FenceVertexList *ListElement)
{
#ifndef WCS_FENCE_LIMITED
double Area, Angle, RoofElev;
int Clockwise, Legal, BreakToOuterLoop, RoofEntity;
long NumRoofPts, FirstCt, NextCt, NextNextCt, TestCt, NumToTest, Illegal, PtOdd, PtLow, PtHigh;
VectorPoint **RoofPts, *PLink, *PLinkPrev;
#endif // WCS_FENCE_LIMITED
double PolySide[2][3], TopElev, BotElev;
#ifdef WCS_THEMATIC_MAP
double ThemeValue[3];
#endif // WCS_THEMATIC_MAP
unsigned long Flags;
long Ct;
int Success = 1;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexData *VtxPtr[3];
VertexData Vtx[4];
VertexBase TempVert;
PolygonData Poly;

if (MyAttr = (JoeCoordSys *)ListElement->Vec->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
	MyCoords = MyAttr->Coord;
else
	MyCoords = NULL;

RendData.TexData.VDataVecOffsetsValid = 0;

switch (ListElement->PieceType)
	{
	case WCS_FENCEPIECE_SPAN:
		{
		// 0 and 2 are top, 1 and 3 are bottom
		ListElement->PointA->ProjToDefDeg(MyCoords, &Vtx[0]);
		Vtx[1].CopyLatLon(&Vtx[0]);
		ListElement->PointB->ProjToDefDeg(MyCoords, &Vtx[2]);
		Vtx[3].CopyLatLon(&Vtx[2]);

		TopElev = ListElement->Fnce->AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANTOPELEV].CurValue;
		BotElev = ListElement->Fnce->AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANBOTELEV].CurValue;
		#ifdef WCS_THEMATIC_MAP
		if (ListElement->Fnce->GetEnabledTheme(WCS_EFFECTS_FENCE_THEME_SPANTOPELEV) &&
			ListElement->Fnce->GetTheme(WCS_EFFECTS_FENCE_THEME_SPANTOPELEV)->Eval(ThemeValue, ListElement->Vec))
			TopElev = ThemeValue[0];
		if (ListElement->Fnce->GetEnabledTheme(WCS_EFFECTS_FENCE_THEME_SPANBOTELEV) &&
			ListElement->Fnce->GetTheme(WCS_EFFECTS_FENCE_THEME_SPANBOTELEV)->Eval(ThemeValue, ListElement->Vec))
			BotElev = ThemeValue[0];
		#endif // WCS_THEMATIC_MAP

		#ifdef WCS_FENCE_LIMITED
		if (TopElev > 5.0)
			TopElev = 5.0;
		if (BotElev > 5.0)
			BotElev = 5.0;
		if (TopElev < -10.0)
			TopElev = -10.0;
		if (BotElev < -10.0)
			BotElev = -10.0;
		#endif // WCS_FENCE_LIMITED

		if (ListElement->Fnce->Absolute == WCS_EFFECT_RELATIVETOJOE)
			{
			Vtx[0].Elev = TopElev + RendData.ElevDatum + (Vtx[0].Elev - RendData.ElevDatum) * RendData.Exageration;
			Vtx[1].Elev = BotElev + RendData.ElevDatum + (Vtx[1].Elev - RendData.ElevDatum) * RendData.Exageration;
			Vtx[2].Elev = TopElev + RendData.ElevDatum + (Vtx[2].Elev - RendData.ElevDatum) * RendData.Exageration;
			Vtx[3].Elev = BotElev + RendData.ElevDatum + (Vtx[3].Elev - RendData.ElevDatum) * RendData.Exageration;
			} // if
		else if (ListElement->Fnce->Absolute == WCS_EFFECT_ABSOLUTE)
			{
			Vtx[0].Elev = TopElev;
			Vtx[1].Elev = BotElev;
			Vtx[2].Elev = TopElev;
			Vtx[3].Elev = BotElev;
			} // else if
		else
			{
			Flags = (WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED |
				WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED);
			RendData.Interactive->VertexDataPoint(&RendData, &Vtx[0], &Poly, Flags);
			Vtx[1].Elev = Vtx[0].Elev + BotElev;
			Vtx[0].Elev += TopElev;
			RendData.Interactive->VertexDataPoint(&RendData, &Vtx[2], &Poly, Flags);
			Vtx[3].Elev = Vtx[2].Elev + BotElev;
			Vtx[2].Elev += TopElev;
			} // else

		for (Ct = 0; Ct < 4; Ct ++)
			{
			// convert degrees to cartesian and project to screen
			Cam->ProjectVertexDEM(DefCoords, &Vtx[Ct], EarthLatScaleMeters, PlanetRad, 1);
			} // for

		Poly.Vector = ListElement->Vec;
		Poly.VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE;
		Poly.VectorType |= (ListElement->Fnce->ConnectToOrigin ? WCS_TEXTURE_VECTOREFFECTTYPE_CONNECTENDS: 0);
		// if not end connecting segment then PointA->Next will be PointB
		// if end connecting segment then PointA->Next will be NULL
		if (! ListElement->PointA->Next || ListElement->PointA->Next != ListElement->PointB)
			Poly.VectorType |= WCS_TEXTURE_VECTOREFFECTTYPE_SKIPFIRSTPOINT;
		Poly.Lat = ListElement->Lat;
		Poly.Lon = ListElement->Lon;
		Poly.Elev = ListElement->Elev;
		Poly.Z = (Vtx[0].ScrnXYZ[2] + Vtx[1].ScrnXYZ[2] + Vtx[2].ScrnXYZ[2] + Vtx[3].ScrnXYZ[2]) * 0.25; // Optimized out division. Was / 4.0
		Poly.Q = (Vtx[0].Q + Vtx[1].Q + Vtx[2].Q + Vtx[3].Q) * 0.25; // Optimized out division. Was / 4.0
		Poly.Slope = 90.0;

		Poly.LonSeed = (ULONG)((Poly.Lon - WCS_floor(Poly.Lon)) * ULONG_MAX);
		Poly.LatSeed = (ULONG)((Poly.Lat - WCS_floor(Poly.Lat)) * ULONG_MAX);

		// compute surface normal - it is same for both triangles
		FindPosVector(PolySide[0], Vtx[1].XYZ, Vtx[0].XYZ);
		FindPosVector(PolySide[1], Vtx[2].XYZ, Vtx[0].XYZ);
		SurfaceNormal(Poly.Normal, PolySide[1], PolySide[0]);

		// test visibility and reverse normal if necessary
		ZeroPoint3d(Poly.ViewVec);
		// find view vector
		for (Ct = 0; Ct < 4; Ct ++)
			{
			AddPoint3d(Poly.ViewVec, Vtx[Ct].XYZ);
			} // for
		Poly.ViewVec[0] *= (1.0 / 4.0);
		Poly.ViewVec[1] *= (1.0 / 4.0);
		Poly.ViewVec[2] *= (1.0 / 4.0);

		Poly.ViewVec[0] -= Cam->CamPos->XYZ[0]; 
		Poly.ViewVec[1] -= Cam->CamPos->XYZ[1]; 
		Poly.ViewVec[2] -= Cam->CamPos->XYZ[2]; 
		if (SurfaceVisible(Poly.Normal, Poly.ViewVec, 0))	// TRUE if surface faces away
			{
			NegateVector(Poly.Normal);
			} // if reverse normals

		TempVert.XYZ[0] = Poly.Normal[0];
		TempVert.XYZ[1] = Poly.Normal[1];
		TempVert.XYZ[2] = Poly.Normal[2];
		TempVert.RotateY(-Poly.Lon);
		TempVert.RotateX(90.0 - Poly.Lat);
		Poly.Aspect = TempVert.FindRoughAngleYfromZ();
		if (Poly.Aspect < 0.0)
			Poly.Aspect += 360.0;

		Poly.Beach = &ListElement->Fnce->SpanMat;
		Poly.Fnce = ListElement->Fnce;
		Poly.FenceType = WCS_FENCEPIECE_SPAN;
		if (ShadowsExist)
			EffectsBase->EvalShadows(&RendData, &Poly);

		VtxPtr[0] = &Vtx[0];
		VtxPtr[1] = &Vtx[2];
		VtxPtr[2] = &Vtx[1];
		for (Ct = 0; Ct < 2; Ct ++)
			{
			if (! (Success = InstigateTerrainPolygon(&Poly, VtxPtr)))
				break;
			VtxPtr[0] = &Vtx[2];
			VtxPtr[1] = &Vtx[1];
			VtxPtr[2] = &Vtx[3];
			} // for
		break;
		} // span
	#ifndef WCS_FENCE_LIMITED
	case WCS_FENCEPIECE_ROOF:
		{
		#ifdef WCS_FENCE_VECPOLY_ROOF
		// new VectorPolygon class features improved triangulation
		VectorPolygon *VecPoly, *CurPoly;
		VectorPolygonListDouble *VecPolyList, *CurVecPolyList, *ListOfHoles = NULL, **CurHoleListPtr;
		VectorNode *RoofTriangle[3];
		JoeList *CurHoleJL;
		bool InsufficientNodes = false;

		// some setup work
		Flags = (WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED |
			WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED);

		// get general roof elevation
		RoofElev = ListElement->Fnce->AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_ROOFELEV].CurValue;
		#ifdef WCS_THEMATIC_MAP
		if (ListElement->Fnce->GetEnabledTheme(WCS_EFFECTS_FENCE_THEME_ROOFELEV) &&
			ListElement->Fnce->GetTheme(WCS_EFFECTS_FENCE_THEME_ROOFELEV)->Eval(ThemeValue, ListElement->Vec))
			RoofElev = ThemeValue[0];
		#endif // WCS_THEMATIC_MAP

		// create a VectorPolygon from the Joe
		if (VecPoly = new VectorPolygon(DBase, ListElement->Fnce, ListElement->Vec, false, InsufficientNodes))
			{
			if (Success = VecPoly->ConvertToDefGeo())
				{
				// remove holes if necessary
				if (ListElement->Holes)
					{
					CurHoleListPtr = &ListOfHoles;
					for (CurHoleJL = ListElement->Holes; CurHoleJL; CurHoleJL = CurHoleJL->Next)
						{
						if (*CurHoleListPtr = new VectorPolygonListDouble())
							{
							if ((*CurHoleListPtr)->MakeVectorPolygon(DBase, ListElement->Fnce, CurHoleJL->Me))
								{
								if (Success = (*CurHoleListPtr)->ConvertToDefGeo())
									CurHoleListPtr = (VectorPolygonListDouble **)&(*CurHoleListPtr)->NextPolygonList;
								else
									break;
								} // if
							else
								{
								Success = 0;
								break;
								} // else
							} // if
						else
							{
							Success = 0;
							break;
							} // else
						} // for
					if (Success)
						{
						if (VecPoly->RemoveHoles(ListOfHoles))
							{
							if (! VecPoly->RemoveConnectingParts())
								Success = 0;
							} // if
						else
							Success = 0;
						} // if
					if (ListOfHoles)
						{
						// if the list persisted through the hole removal process then something probably went wrong.
						// remove the list.
						for (VectorPolygonListDouble *CurHoleList = ListOfHoles; ListOfHoles; CurHoleList = ListOfHoles)
							{
							ListOfHoles = (VectorPolygonListDouble *)ListOfHoles->NextPolygonList;
							CurHoleList->DeletePolygon();
							delete CurHoleList;
							} // for
						} // if
					} // if
				} // if
			if (Success)
				{
				// create a set of triangular polygons
				double PArea;
				bool BoolSuccess = true;
				
				// test for negativity and reverse if necessary
				if ((PArea = VecPoly->PolygonArea()) < 0)
					VecPoly->ReverseDirection();
				if (PArea != 0.0)
					{
					VecPolyList = VecPoly->Triangulate(BoolSuccess);
					if (Success = BoolSuccess)
						{
						// render and delete each triangle
						if (VecPoly->TotalNumNodes >= 3)
							{
							CurVecPolyList = NULL;
							CurPoly = VecPoly;
							} // if
						else
							{
							CurVecPolyList = VecPolyList;
							CurPoly = NULL;
							} // else
						for (; CurPoly || CurVecPolyList; CurVecPolyList = VecPolyList)
							{
							if (! CurPoly)
								CurPoly = CurVecPolyList->MyPolygon;
							RoofTriangle[0] = CurPoly->PolyFirstNode();
							RoofTriangle[1] = RoofTriangle[0]->NextNode;
							RoofTriangle[2] = RoofTriangle[1]->NextNode;

							// coords are already in DefDeg, just need to copy to Vtx[]
							RoofTriangle[0]->ProjToDefDeg(NULL, &Vtx[0]);
							RoofTriangle[1]->ProjToDefDeg(NULL, &Vtx[1]);
							RoofTriangle[2]->ProjToDefDeg(NULL, &Vtx[2]);

							if (ListElement->Fnce->Absolute == WCS_EFFECT_RELATIVETOJOE)
								{
								Vtx[0].Elev = RoofElev + 
									RendData.ElevDatum + (Vtx[0].Elev - RendData.ElevDatum) * RendData.Exageration;
								Vtx[1].Elev = RoofElev + 
									RendData.ElevDatum + (Vtx[1].Elev - RendData.ElevDatum) * RendData.Exageration;
								Vtx[2].Elev = RoofElev + 
									RendData.ElevDatum + (Vtx[2].Elev - RendData.ElevDatum) * RendData.Exageration;
								} // if
							else if (ListElement->Fnce->Absolute == WCS_EFFECT_ABSOLUTE)
								{
								Vtx[0].Elev = Vtx[1].Elev = Vtx[2].Elev = RoofElev;
								} // else if
							else
								{
								RendData.Interactive->VertexDataPoint(&RendData, &Vtx[0], &Poly, Flags);
								RendData.Interactive->VertexDataPoint(&RendData, &Vtx[1], &Poly, Flags);
								RendData.Interactive->VertexDataPoint(&RendData, &Vtx[2], &Poly, Flags);
								Vtx[0].Elev += RoofElev;
								Vtx[1].Elev += RoofElev;
								Vtx[2].Elev += RoofElev;
								} // else

							for (Ct = 0; Ct < 3; Ct ++)
								{
								// convert degrees to cartesian and project to screen
								Cam->ProjectVertexDEM(DefCoords, &Vtx[Ct], EarthLatScaleMeters, PlanetRad, 1);
								} // for

							Poly.Vector = ListElement->Vec;
							Poly.VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE;
							Poly.VectorType |= (ListElement->Fnce->ConnectToOrigin ? WCS_TEXTURE_VECTOREFFECTTYPE_CONNECTENDS: 0);
							Poly.Lat = ListElement->Lat;
							Poly.Lon = ListElement->Lon;
							Poly.Elev = ListElement->Elev;
							Poly.Z = (Vtx[0].ScrnXYZ[2] + Vtx[1].ScrnXYZ[2] + Vtx[2].ScrnXYZ[2]) * (1.0 / 3.0);
							Poly.Q = (Vtx[0].Q + Vtx[1].Q + Vtx[2].Q) * (1.0 / 3.0);
							Poly.Slope = 0.0;

							Poly.LonSeed = (ULONG)((Poly.Lon - WCS_floor(Poly.Lon)) * ULONG_MAX);
							Poly.LatSeed = (ULONG)((Poly.Lat - WCS_floor(Poly.Lat)) * ULONG_MAX);

							// compute surface normal - it is same for both triangles
							FindPosVector(PolySide[0], Vtx[1].XYZ, Vtx[0].XYZ);
							FindPosVector(PolySide[1], Vtx[2].XYZ, Vtx[0].XYZ);
							SurfaceNormal(Poly.Normal, PolySide[1], PolySide[0]);

							// test visibility and reverse normal if necessary
							ZeroPoint3d(Poly.ViewVec);
							// find view vector
							for (Ct = 0; Ct < 3; Ct ++)
								{
								AddPoint3d(Poly.ViewVec, Vtx[Ct].XYZ);
								} // for
							Poly.ViewVec[0] *= (1.0 / 3.0);
							Poly.ViewVec[1] *= (1.0 / 3.0);
							Poly.ViewVec[2] *= (1.0 / 3.0);

							Poly.ViewVec[0] -= Cam->CamPos->XYZ[0]; 
							Poly.ViewVec[1] -= Cam->CamPos->XYZ[1]; 
							Poly.ViewVec[2] -= Cam->CamPos->XYZ[2]; 
							if (SurfaceVisible(Poly.Normal, Poly.ViewVec, 0))	// TRUE if surface faces away
								{
								NegateVector(Poly.Normal);
								} // if reverse normals

							Poly.Beach = ListElement->Fnce->SeparateRoofMat ? &ListElement->Fnce->RoofMat: &ListElement->Fnce->SpanMat;
							Poly.Fnce = ListElement->Fnce;
							Poly.FenceType = WCS_FENCEPIECE_ROOF;
							if (ShadowsExist)
								EffectsBase->EvalShadows(&RendData, &Poly);

							VtxPtr[0] = &Vtx[0];
							VtxPtr[1] = &Vtx[2];
							VtxPtr[2] = &Vtx[1];
							if (! (Success = InstigateTerrainPolygon(&Poly, VtxPtr)))
								break;

							if (CurVecPolyList)
								{
								// advance pointer and delete the current working triangle
								VecPolyList = (VectorPolygonListDouble *)VecPolyList->NextPolygonList;
								CurVecPolyList->DeletePolygon();
								delete CurVecPolyList;
								} // if
							CurPoly = NULL;
							} // for
						} // if
					else if (UserMessageOKCAN((char *)ListElement->Vec->GetBestName(), "Error triangulating the roof for the Wall Component\nassigned to this vector. Vector may have faulty topology.\nContinue without it?"))
						Success = true;
					} // if
				} // if
			delete VecPoly;
			} // if
		#else // WCS_FENCE_VECPOLY_ROOF
		PLinkPrev = NULL;
		for (NumRoofPts = 0, PLink = ListElement->Vec->GetFirstRealPoint(); PLink; PLink = PLink->Next)
			{
			if (PLinkPrev && PLink->SamePoint(PLinkPrev))
				continue;
			if (! PLink->Next && PLink->SamePoint(ListElement->Vec->GetFirstRealPoint()))
				continue;
			NumRoofPts ++;
			PLinkPrev = PLink;
			} // for
		//NumRoofPts = ListElement->Vec->GetNumRealPoints();
		if (NumRoofPts >= 3 && (RoofPts = (VectorPoint **)AppMem_Alloc(NumRoofPts * sizeof (VectorPoint *), APPMEM_CLEAR)))
			{
			// find out if vector is clockwise or countercw
			Area = ListElement->Vec->ComputeAreaDegrees();
			Clockwise = (Area >= 0.0);
			// build list of vector points
			PLinkPrev = NULL;
			if (Clockwise)
				{
				for (Ct = 0, PLink = ListElement->Vec->GetFirstRealPoint(); Ct < NumRoofPts && PLink; PLink = PLink->Next)
					{
					if (PLinkPrev && PLink->SamePoint(PLinkPrev))
						continue;
					if (! PLink->Next && PLink->SamePoint(ListElement->Vec->GetFirstRealPoint()))
						continue;
					RoofPts[Ct ++] = PLink;
					PLinkPrev = PLink;
					} // for
				} // if
			else
				{
				for (Ct = NumRoofPts - 1, PLink = ListElement->Vec->GetFirstRealPoint(); Ct >= 0 && PLink; PLink = PLink->Next)
					{
					if (PLinkPrev && PLink->SamePoint(PLinkPrev))
						continue;
					if (! PLink->Next && PLink->SamePoint(ListElement->Vec->GetFirstRealPoint()))
						continue;
					RoofPts[Ct --] = PLink;
					PLinkPrev = PLink;
					} // for
				} // else
			Flags = (WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED |
				WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED);

			RoofElev = ListElement->Fnce->AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_ROOFELEV].CurValue;
			#ifdef WCS_THEMATIC_MAP
			if (ListElement->Fnce->GetEnabledTheme(WCS_EFFECTS_FENCE_THEME_ROOFELEV) &&
				ListElement->Fnce->GetTheme(WCS_EFFECTS_FENCE_THEME_ROOFELEV)->Eval(ThemeValue, ListElement->Vec))
				RoofElev = ThemeValue[0];
			#endif // WCS_THEMATIC_MAP

			NumToTest = NumRoofPts - 2;
			Illegal = 0;
			for (FirstCt = 0; Illegal < NumToTest && NumToTest > 0; )
				{
				BreakToOuterLoop = 0;
				if (RoofPts[FirstCt])
					{
					for (NextCt = FirstCt + 1 < NumRoofPts ? FirstCt + 1: 0; NextCt != FirstCt && ! BreakToOuterLoop; )
						{
						if (RoofPts[NextCt])
							{
							for (NextNextCt = NextCt + 1 < NumRoofPts ? NextCt + 1: 0; NextNextCt != FirstCt && NextNextCt != NextCt; )
								{
								if (RoofPts[NextNextCt])
									{
									Legal = 1;
									// test to see if it is a legal triangle
									// a roof triangle is legal and OK to draw if it fills clockwise space and
									// if no other point causes the triangle to be partly outside the roof outline
									// Is it clockwise?
									RoofPts[FirstCt]->ProjToDefDeg(MyCoords, &Vtx[0]);
									RoofPts[NextCt]->ProjToDefDeg(MyCoords, &Vtx[1]);
									RoofPts[NextNextCt]->ProjToDefDeg(MyCoords, &Vtx[2]);
									Vtx[1].XYZ[0] = -(Vtx[1].Lon - Vtx[0].Lon);
									Vtx[1].XYZ[2] = Vtx[1].Lat - Vtx[0].Lat;
									Vtx[2].XYZ[0] = -(Vtx[2].Lon - Vtx[0].Lon);
									Vtx[2].XYZ[2] = Vtx[2].Lat - Vtx[0].Lat;
									Angle = Vtx[1].FindAngleYfromZ();
									Vtx[2].RotateY(-Angle);
									if (Vtx[2].XYZ[0] >= 0.0)
										{
										// rotate back to original position
										Vtx[2].RotateY(Angle);
										Angle = Vtx[2].FindAngleYfromZ();
										Vtx[2].RotateY(-Angle);
										for (TestCt = NextNextCt + 1 < NumRoofPts ? NextNextCt + 1: 0; TestCt != FirstCt && TestCt != NextCt && TestCt != NextNextCt; )
											{
											if (RoofPts[TestCt])
												{
												RoofPts[TestCt]->ProjToDefDeg(MyCoords, &Vtx[1]);
												Vtx[1].XYZ[0] = -(Vtx[1].Lon - Vtx[0].Lon);
												Vtx[1].XYZ[2] = Vtx[1].Lat - Vtx[0].Lat;
												Vtx[1].RotateY(-Angle);
												if (Vtx[1].XYZ[2] > 0.0 && Vtx[1].XYZ[2] < Vtx[2].XYZ[2] && Vtx[1].XYZ[0] < 0.0)
													{
													Legal = 0;
													break;
													} // if
												} // if
											TestCt ++;
											if (TestCt >= NumRoofPts)
												TestCt = 0;
											} // for
										} // if clockwise triangle
									else
										Legal = 0;

									// if legal then fill it
									if (Legal)
										{
										RoofPts[FirstCt]->ProjToDefDeg(MyCoords, &Vtx[0]);
										RoofPts[NextCt]->ProjToDefDeg(MyCoords, &Vtx[1]);
										RoofPts[NextNextCt]->ProjToDefDeg(MyCoords, &Vtx[2]);

										if (ListElement->Fnce->Absolute == WCS_EFFECT_RELATIVETOJOE)
											{
											Vtx[0].Elev = RoofElev + 
												RendData.ElevDatum + (Vtx[0].Elev - RendData.ElevDatum) * RendData.Exageration;
											Vtx[1].Elev = RoofElev + 
												RendData.ElevDatum + (Vtx[1].Elev - RendData.ElevDatum) * RendData.Exageration;
											Vtx[2].Elev = RoofElev + 
												RendData.ElevDatum + (Vtx[2].Elev - RendData.ElevDatum) * RendData.Exageration;
											} // if
										else if (ListElement->Fnce->Absolute == WCS_EFFECT_ABSOLUTE)
											{
											Vtx[0].Elev = Vtx[1].Elev = Vtx[2].Elev = RoofElev;
											} // else if
										else
											{
											RendData.Interactive->VertexDataPoint(&RendData, &Vtx[0], &Poly, Flags);
											RendData.Interactive->VertexDataPoint(&RendData, &Vtx[1], &Poly, Flags);
											RendData.Interactive->VertexDataPoint(&RendData, &Vtx[2], &Poly, Flags);
											Vtx[0].Elev += RoofElev;
											Vtx[1].Elev += RoofElev;
											Vtx[2].Elev += RoofElev;
											} // else

										for (Ct = 0; Ct < 3; Ct ++)
											{
											// convert degrees to cartesian and project to screen
											Cam->ProjectVertexDEM(DefCoords, &Vtx[Ct], EarthLatScaleMeters, PlanetRad, 1);
											} // for

										Poly.Vector = ListElement->Vec;
										Poly.VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE;
										Poly.VectorType |= (ListElement->Fnce->ConnectToOrigin ? WCS_TEXTURE_VECTOREFFECTTYPE_CONNECTENDS: 0);
										Poly.Lat = ListElement->Lat;
										Poly.Lon = ListElement->Lon;
										Poly.Elev = ListElement->Elev;
										Poly.Z = (Vtx[0].ScrnXYZ[2] + Vtx[1].ScrnXYZ[2] + Vtx[2].ScrnXYZ[2]) * (1.0 / 3.0);
										Poly.Q = (Vtx[0].Q + Vtx[1].Q + Vtx[2].Q) * (1.0 / 3.0);
										Poly.Slope = 0.0;

										Poly.LonSeed = (ULONG)((Poly.Lon - WCS_floor(Poly.Lon)) * ULONG_MAX);
										Poly.LatSeed = (ULONG)((Poly.Lat - WCS_floor(Poly.Lat)) * ULONG_MAX);

										// compute surface normal - it is same for both triangles
										FindPosVector(PolySide[0], Vtx[1].XYZ, Vtx[0].XYZ);
										FindPosVector(PolySide[1], Vtx[2].XYZ, Vtx[0].XYZ);
										SurfaceNormal(Poly.Normal, PolySide[1], PolySide[0]);

										// test visibility and reverse normal if necessary
										ZeroPoint3d(Poly.ViewVec);
										// find view vector
										for (Ct = 0; Ct < 3; Ct ++)
											{
											AddPoint3d(Poly.ViewVec, Vtx[Ct].XYZ);
											} // for
										Poly.ViewVec[0] *= (1.0 / 3.0);
										Poly.ViewVec[1] *= (1.0 / 3.0);
										Poly.ViewVec[2] *= (1.0 / 3.0);

										Poly.ViewVec[0] -= Cam->CamPos->XYZ[0]; 
										Poly.ViewVec[1] -= Cam->CamPos->XYZ[1]; 
										Poly.ViewVec[2] -= Cam->CamPos->XYZ[2]; 
										if (SurfaceVisible(Poly.Normal, Poly.ViewVec, 0))	// TRUE if surface faces away
											{
											NegateVector(Poly.Normal);
											} // if reverse normals

										Poly.Beach = ListElement->Fnce->SeparateRoofMat ? &ListElement->Fnce->RoofMat: &ListElement->Fnce->SpanMat;
										Poly.Fnce = ListElement->Fnce;
										Poly.FenceType = WCS_FENCEPIECE_ROOF;
										if (ShadowsExist)
											EffectsBase->EvalShadows(&RendData, &Poly);

										VtxPtr[0] = &Vtx[0];
										VtxPtr[1] = &Vtx[2];
										VtxPtr[2] = &Vtx[1];
										if (! (Success = InstigateTerrainPolygon(&Poly, VtxPtr)))
											break;

										RoofPts[NextCt] = NULL;
										NumToTest --;
										Illegal = 0;
										BreakToOuterLoop = 1;
										break;
										} // if
									else
										{
										Illegal ++;
										BreakToOuterLoop = 1;
										break;
										} // else
									} // if
								NextNextCt ++;
								if (NextNextCt >= NumRoofPts)
									NextNextCt = 0;
								} // for
							} // if
						NextCt ++;
						if (NextCt >= NumRoofPts)
							NextCt = 0;
						} // for
					} // if
				FirstCt ++; 
				if (FirstCt >= NumRoofPts)
					FirstCt = 0;
				} // for
			AppMem_Free(RoofPts, NumRoofPts * sizeof (VectorPoint *));
			} // if list
		#endif // WCS_FENCE_VECPOLY_ROOF
		break;
		} // roof
	case WCS_FENCEPIECE_SKINFRAME:
		{
		// check to see if this is a roof type polygon by seeing if the area in plan view is non-zero
		Area = ListElement->Vec->ComputeAreaDegrees();
		RoofEntity = (fabs(Area) > 1.0E-15);

		if (! RoofEntity)
			{
			NumRoofPts = ListElement->Vec->GetNumRealPoints();
			if (RoofPts = (VectorPoint **)AppMem_Alloc(NumRoofPts * sizeof (VectorPoint *), 0))
				{
				// build list of vector points
				for (Ct = 0, PLink = ListElement->Vec->GetFirstRealPoint(); Ct < NumRoofPts && PLink; Ct ++, PLink = PLink->Next)
					{
					RoofPts[Ct] = PLink;
					} // for
				PtOdd = 0;
				PtLow = 1;
				PtHigh = RoofPts[0]->SamePoint(RoofPts[NumRoofPts - 1]) ? NumRoofPts - 2: NumRoofPts - 1;
				while (PtHigh > PtLow)
					{
					RoofPts[PtOdd]->ProjToDefDeg(MyCoords, &Vtx[0]);
					RoofPts[PtHigh]->ProjToDefDeg(MyCoords, &Vtx[1]);
					RoofPts[PtLow]->ProjToDefDeg(MyCoords, &Vtx[2]);

					Vtx[0].Elev = RendData.ElevDatum + (Vtx[0].Elev - RendData.ElevDatum) * RendData.Exageration;
					Vtx[1].Elev = RendData.ElevDatum + (Vtx[1].Elev - RendData.ElevDatum) * RendData.Exageration;
					Vtx[2].Elev = RendData.ElevDatum + (Vtx[2].Elev - RendData.ElevDatum) * RendData.Exageration;

					for (Ct = 0; Ct < 3; Ct ++)
						{
						// convert degrees to cartesian and project to screen
						Cam->ProjectVertexDEM(DefCoords, &Vtx[Ct], EarthLatScaleMeters, PlanetRad, 1);
						} // for

					Poly.Vector = ListElement->Vec;
					Poly.VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE;
					Poly.VectorType |= (ListElement->Fnce->ConnectToOrigin ? WCS_TEXTURE_VECTOREFFECTTYPE_CONNECTENDS: 0);
					Poly.Lat = ListElement->Lat;
					Poly.Lon = ListElement->Lon;
					Poly.Elev = ListElement->Elev;
					Poly.Z = (Vtx[0].ScrnXYZ[2] + Vtx[1].ScrnXYZ[2] + Vtx[2].ScrnXYZ[2]) / 3.0;
					Poly.Q = (Vtx[0].Q + Vtx[1].Q + Vtx[2].Q) / 3.0;
					Poly.Slope = 0.0;

					Poly.LonSeed = (ULONG)((Poly.Lon - WCS_floor(Poly.Lon)) * ULONG_MAX);
					Poly.LatSeed = (ULONG)((Poly.Lat - WCS_floor(Poly.Lat)) * ULONG_MAX);

					// compute surface normal - it is same for both triangles
					FindPosVector(PolySide[0], Vtx[1].XYZ, Vtx[0].XYZ);
					FindPosVector(PolySide[1], Vtx[2].XYZ, Vtx[0].XYZ);
					SurfaceNormal(Poly.Normal, PolySide[1], PolySide[0]);

					// test visibility and reverse normal if necessary
					ZeroPoint3d(Poly.ViewVec);
					// find view vector
					for (Ct = 0; Ct < 3; Ct ++)
						{
						AddPoint3d(Poly.ViewVec, Vtx[Ct].XYZ);
						} // for
					Poly.ViewVec[0] /= 3.0;
					Poly.ViewVec[1] /= 3.0;
					Poly.ViewVec[2] /= 3.0;

					Poly.ViewVec[0] -= Cam->CamPos->XYZ[0]; 
					Poly.ViewVec[1] -= Cam->CamPos->XYZ[1]; 
					Poly.ViewVec[2] -= Cam->CamPos->XYZ[2]; 
					if (SurfaceVisible(Poly.Normal, Poly.ViewVec, 0))	// TRUE if surface faces away
						{
						NegateVector(Poly.Normal);
						} // if reverse normals

					Poly.Beach = &ListElement->Fnce->SpanMat;
					Poly.Fnce = ListElement->Fnce;
					Poly.FenceType = WCS_FENCEPIECE_SPAN;

					VtxPtr[0] = &Vtx[0];
					VtxPtr[1] = &Vtx[2];
					VtxPtr[2] = &Vtx[1];
					if (! (Success = InstigateTerrainPolygon(&Poly, VtxPtr)))
						break;
					if (PtOdd > PtHigh)
						{
						PtOdd = PtLow;
						PtLow ++;
						} // if
					else
						{
						PtOdd = PtHigh;
						PtHigh --;
						} // else
					} // while
				AppMem_Free(RoofPts, NumRoofPts * sizeof (VectorPoint *));
				} // if list
			} // if not roof entity
		else
			{
			PLinkPrev = NULL;
			for (NumRoofPts = 0, PLink = ListElement->Vec->GetFirstRealPoint(); PLink; PLink = PLink->Next)
				{
				if (PLinkPrev && PLink->SamePoint(PLinkPrev))
					continue;
				if (! PLink->Next && PLink->SamePoint(ListElement->Vec->GetFirstRealPoint()))
					continue;
				NumRoofPts ++;
				PLinkPrev = PLink;
				} // for
			//NumRoofPts = ListElement->Vec->GetNumRealPoints();
			if (NumRoofPts >= 3 && (RoofPts = (VectorPoint **)AppMem_Alloc(NumRoofPts * sizeof (VectorPoint *), APPMEM_CLEAR)))
				{
				// find out if vector is clockwise or countercw
				Clockwise = (Area >= 0.0);
				// build list of vector points
				PLinkPrev = NULL;
				if (Clockwise)
					{
					for (Ct = 0, PLink = ListElement->Vec->GetFirstRealPoint(); Ct < NumRoofPts && PLink; PLink = PLink->Next)
						{
						if (PLinkPrev && PLink->SamePoint(PLinkPrev))
							continue;
						if (! PLink->Next && PLink->SamePoint(ListElement->Vec->GetFirstRealPoint()))
							continue;
						RoofPts[Ct ++] = PLink;
						PLinkPrev = PLink;
						} // for
					} // if
				else
					{
					for (Ct = NumRoofPts - 1, PLink = ListElement->Vec->GetFirstRealPoint(); Ct >= 0 && PLink; PLink = PLink->Next)
						{
						if (PLinkPrev && PLink->SamePoint(PLinkPrev))
							continue;
						if (! PLink->Next && PLink->SamePoint(ListElement->Vec->GetFirstRealPoint()))
							continue;
						RoofPts[Ct --] = PLink;
						PLinkPrev = PLink;
						} // for
					} // else

				NumToTest = NumRoofPts - 2;
				Illegal = 0;
				for (FirstCt = 0; Illegal < NumToTest && NumToTest > 0; )
					{
					BreakToOuterLoop = 0;
					if (RoofPts[FirstCt])
						{
						for (NextCt = FirstCt + 1 < NumRoofPts ? FirstCt + 1: 0; NextCt != FirstCt && ! BreakToOuterLoop; )
							{
							if (RoofPts[NextCt])
								{
								for (NextNextCt = NextCt + 1 < NumRoofPts ? NextCt + 1: 0; NextNextCt != FirstCt && NextNextCt != NextCt; )
									{
									if (RoofPts[NextNextCt])
										{
										Legal = 1;
										// test to see if it is a legal triangle
										// a roof triangle is legal and OK to draw if it fills clockwise space and
										// if no other point causes the triangle to be partly outside the roof outline
										// Is it clockwise?
										RoofPts[FirstCt]->ProjToDefDeg(MyCoords, &Vtx[0]);
										RoofPts[NextCt]->ProjToDefDeg(MyCoords, &Vtx[1]);
										RoofPts[NextNextCt]->ProjToDefDeg(MyCoords, &Vtx[2]);
										Vtx[1].XYZ[0] = -(Vtx[1].Lon - Vtx[0].Lon);
										Vtx[1].XYZ[2] = Vtx[1].Lat - Vtx[0].Lat;
										Vtx[2].XYZ[0] = -(Vtx[2].Lon - Vtx[0].Lon);
										Vtx[2].XYZ[2] = Vtx[2].Lat - Vtx[0].Lat;
										Angle = Vtx[1].FindAngleYfromZ();
										Vtx[2].RotateY(-Angle);
										if (Vtx[2].XYZ[0] >= 0.0)
											{
											// rotate back to original position
											Vtx[2].RotateY(Angle);
											Angle = Vtx[2].FindAngleYfromZ();
											Vtx[2].RotateY(-Angle);
											for (TestCt = NextNextCt + 1 < NumRoofPts ? NextNextCt + 1: 0; TestCt != FirstCt && TestCt != NextCt && TestCt != NextNextCt; )
												{
												if (RoofPts[TestCt])
													{
													RoofPts[TestCt]->ProjToDefDeg(MyCoords, &Vtx[1]);
													Vtx[1].XYZ[0] = -(Vtx[1].Lon - Vtx[0].Lon);
													Vtx[1].XYZ[2] = Vtx[1].Lat - Vtx[0].Lat;
													Vtx[1].RotateY(-Angle);
													if (Vtx[1].XYZ[2] > 0.0 && Vtx[1].XYZ[2] < Vtx[2].XYZ[2] && Vtx[1].XYZ[0] < 0.0)
														{
														Legal = 0;
														break;
														} // if
													} // if
												TestCt ++;
												if (TestCt >= NumRoofPts)
													TestCt = 0;
												} // for
											} // if clockwise triangle
										else
											Legal = 0;

										// if legal then fill it
										if (Legal)
											{
											RoofPts[FirstCt]->ProjToDefDeg(MyCoords, &Vtx[0]);
											RoofPts[NextCt]->ProjToDefDeg(MyCoords, &Vtx[1]);
											RoofPts[NextNextCt]->ProjToDefDeg(MyCoords, &Vtx[2]);

											Vtx[0].Elev = RendData.ElevDatum + (Vtx[0].Elev - RendData.ElevDatum) * RendData.Exageration;
											Vtx[1].Elev = RendData.ElevDatum + (Vtx[1].Elev - RendData.ElevDatum) * RendData.Exageration;
											Vtx[2].Elev = RendData.ElevDatum + (Vtx[2].Elev - RendData.ElevDatum) * RendData.Exageration;

											for (Ct = 0; Ct < 3; Ct ++)
												{
												// convert degrees to cartesian and project to screen
												Cam->ProjectVertexDEM(DefCoords, &Vtx[Ct], EarthLatScaleMeters, PlanetRad, 1);
												} // for

											Poly.Vector = ListElement->Vec;
											Poly.VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE;
											Poly.VectorType |= (ListElement->Fnce->ConnectToOrigin ? WCS_TEXTURE_VECTOREFFECTTYPE_CONNECTENDS: 0);
											Poly.Lat = ListElement->Lat;
											Poly.Lon = ListElement->Lon;
											Poly.Elev = ListElement->Elev;
											Poly.Z = (Vtx[0].ScrnXYZ[2] + Vtx[1].ScrnXYZ[2] + Vtx[2].ScrnXYZ[2]) * (1.0 / 3.0);
											Poly.Q = (Vtx[0].Q + Vtx[1].Q + Vtx[2].Q) * (1.0 / 3.0);
											Poly.Slope = 0.0;

											Poly.LonSeed = (ULONG)((Poly.Lon - WCS_floor(Poly.Lon)) * ULONG_MAX);
											Poly.LatSeed = (ULONG)((Poly.Lat - WCS_floor(Poly.Lat)) * ULONG_MAX);

											// compute surface normal - it is same for both triangles
											FindPosVector(PolySide[0], Vtx[1].XYZ, Vtx[0].XYZ);
											FindPosVector(PolySide[1], Vtx[2].XYZ, Vtx[0].XYZ);
											SurfaceNormal(Poly.Normal, PolySide[1], PolySide[0]);

											// test visibility and reverse normal if necessary
											ZeroPoint3d(Poly.ViewVec);
											// find view vector
											for (Ct = 0; Ct < 3; Ct ++)
												{
												AddPoint3d(Poly.ViewVec, Vtx[Ct].XYZ);
												} // for
											Poly.ViewVec[0] *= (1.0 / 3.0);
											Poly.ViewVec[1] *= (1.0 / 3.0);
											Poly.ViewVec[2] *= (1.0 / 3.0);

											Poly.ViewVec[0] -= Cam->CamPos->XYZ[0]; 
											Poly.ViewVec[1] -= Cam->CamPos->XYZ[1]; 
											Poly.ViewVec[2] -= Cam->CamPos->XYZ[2]; 
											if (SurfaceVisible(Poly.Normal, Poly.ViewVec, 0))	// TRUE if surface faces away
												{
												NegateVector(Poly.Normal);
												} // if reverse normals

											Poly.Beach = ListElement->Fnce->SeparateRoofMat ? &ListElement->Fnce->RoofMat: &ListElement->Fnce->SpanMat;
											Poly.Fnce = ListElement->Fnce;
											Poly.FenceType = WCS_FENCEPIECE_ROOF;
											if (ShadowsExist)
												EffectsBase->EvalShadows(&RendData, &Poly);

											VtxPtr[0] = &Vtx[0];
											VtxPtr[1] = &Vtx[2];
											VtxPtr[2] = &Vtx[1];
											if (! (Success = InstigateTerrainPolygon(&Poly, VtxPtr)))
												break;

											RoofPts[NextCt] = NULL;
											NumToTest --;
											Illegal = 0;
											BreakToOuterLoop = 1;
											break;
											} // if
										else
											{
											Illegal ++;
											BreakToOuterLoop = 1;
											break;
											} // else
										} // if
									NextNextCt ++;
									if (NextNextCt >= NumRoofPts)
										NextNextCt = 0;
									} // for
								} // if
							NextCt ++;
							if (NextCt >= NumRoofPts)
								NextCt = 0;
							} // for
						} // if
					FirstCt ++; 
					if (FirstCt >= NumRoofPts)
						FirstCt = 0;
					} // for
				AppMem_Free(RoofPts, NumRoofPts * sizeof (VectorPoint *));
				} // if list
			} // else roof entity
		break;
		} // skin frame
	#endif // WCS_FENCE_LIMITED
	} // switch

return (Success);

} // Renderer::RenderFence

/*===========================================================================*/

int Renderer::RenderLabel(LabelVertexList *ListElement)
{
VertexDEM LabelVert;
PolygonData Poly;
Label *Labl;

if (Labl = ListElement->Labl)
	{
	Labl->FindBasePosition(&RendData, &LabelVert, &Poly, ListElement->Vec, ListElement->Point);
	Cam->ProjectVertexDEM(DefCoords, &LabelVert, EarthLatScaleMeters, PlanetRad, 1);
	if (LabelVert.ScrnXYZ[2] > 0.0)
		{
		if (LabelVert.ScrnXYZ[2] > MinimumZ && LabelVert.ScrnXYZ[2] <= MaximumZ)
			{
			// don't know if we'll do shadows on labels or not
			//if (ShadowsExist)
			//	EffectsBase->EvalShadows(&RendData, &Poly);
			if (! RenderLabel(&Poly, &LabelVert, Labl, ListElement->Vec))
				return (0);
			} // if
		} // if
	} // if

return (1);

} // Renderer::RenderLabel

/*===========================================================================*/

int Renderer::RenderLabel(PolygonData *Poly, VertexDEM *LabelVtx, Label *Labl, Joe *CurVec)
{
double FlagHeight, FlagWidth, BorderWidth, PoleWidth, TopPoleWidth, PoleHeight, LabelScale, LabelTextScale, Slant, 
	FloatStart, FloatEnd, StartWt, EndWt, SlantWt, TextLineHeight, TextLineSpace, TextLetterSpace, TextOutlineSize, TextHeight, TextWidth,
	LineOffset, DummyColor[3], LabelWidth, LabelHeight, LabelHeightFactor, LabelPixelHeight, LabelPixelWidth,
	LabelElevGrad, LabelTopElev, MaxZOffset, TestMergeOffset, MinPixelSize, VertShift, HorShift, ThisLineWidth,
	TextBlockWidth, TextBlockHeight, MasterSize, Value[3], TexOpacity, FlagTextWidth, FlagTextHeight, FinalHorScale = 1.0,
	MaxTextWidth, MaxTextHeight, LabelBacklight = 1.0;
float FloatAlpha, EdgeAlpha, TempAlpha;
long LabelW, LabelH, TextBlockW, TextBlockH, TextBlockOffsetW, TextBlockOffsetH, X, Y, PixZip, DPixZip, NumPixels, NumFlagPixels, StartPixel, EndPixel, OrigStrLen, StrStart,
	ThisLinePixels, CurChar, TestChar, LastSpace, SolutionFound, SpaceFound, MaxLinePixels = 0, NumTextLines = 0, LeadingPixels, 
	KerningPixels, EdgingPixels, LinePixels, LineStartW, LineStartH, SumTextAlpha, ScaleMult, i, j, ImageID = 0;
Raster *LabelRast;
#ifdef WCS_BUILD_RTX
RealtimeFoliageData *FolListDat;
#endif // WCS_BUILD_RTX
unsigned char *LabelRedBuf = NULL, *LabelGrnBuf = NULL, *LabelBluBuf = NULL, *LabelAlphaBuf = NULL, *LabelEdgeBuf = NULL;
int Success = 1, OverheadView, RenderText = 0, UseLoResFont;
Foliage FolLink(NULL);
FontImage FI;
InternalRasterFont *IRF;
unsigned char Red, Green, Blue, OutlineRed, OutlineGreen, OutlineBlue, TempRed, TempGreen, TempBlue;
char MesgCopy[WCS_LABELTEXT_MAXLEN];

// project the vertex to see where it falls in the output image
Cam->ProjectVertexDEM(DefCoords, LabelVtx, EarthLatScaleMeters, PlanetRad, 1);	// 1 means convert vertex to cartesian
DummyColor[0] = DummyColor[1] = DummyColor[2] = .75;
Value[0] = Value[1] = Value[2] = 0.0;
// for exports only write out labels that have their bases inside the image
#ifdef WCS_BUILD_RTX
if (Exporter)
	{
	if (LabelVtx->ScrnXYZ[0] < 0 || LabelVtx->ScrnXYZ[0] >= Width || LabelVtx->ScrnXYZ[1] < 0 || LabelVtx->ScrnXYZ[1] >= Height)
		return (1);
	} // if
#endif // WCS_BUILD_RTX

// evaluate MasterSize
if (Labl->GetEnabledTheme(WCS_EFFECTS_LABEL_THEME_MASTERSIZE) && 
	Labl->Theme[WCS_EFFECTS_LABEL_THEME_MASTERSIZE]->Eval(Value, CurVec))
	MasterSize = Value[0] * .01;
else
	MasterSize = Labl->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_MASTERSIZE].CurValue;

if (Labl->GetEnabledTexture(WCS_EFFECTS_LABEL_TEXTURE_MASTERSIZE) && MasterSize > 0.0)
	{
	RendData.TransferTextureData(Poly);

	// set foliage coordinates
	RendData.TexData.Elev = LabelVtx->Elev;
	RendData.TexData.Latitude = LabelVtx->Lat;
	RendData.TexData.Longitude = LabelVtx->Lon;
	RendData.TexData.ZDist = LabelVtx->ScrnXYZ[2];
	RendData.TexData.QDist = LabelVtx->Q;
	RendData.TexData.VDEM[0] = LabelVtx;
	RendData.TexData.WaterDepth = Poly->WaterElev - LabelVtx->Elev;
	RendData.TexData.VDataVecOffsetsValid = 0;

	// evaluate master size texture
	if ((TexOpacity = Labl->GetTexRootPtr(WCS_EFFECTS_LABEL_TEXTURE_MASTERSIZE)->Eval(Value, &RendData.TexData)) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			// Value[0] has already been diminished by the texture's opacity
			MasterSize *= (1.0 - TexOpacity + Value[0]);
			} // if
		else
			MasterSize *= Value[0];
		} // if
	} // if

if (! ShadowMapInProgress && LabelVtx->ScrnXYZ[2] > 0.0 && MasterSize > 0.0)
	{
	#ifdef WCS_BUILD_RTX
	OverheadView = ! Exporter && (Cam->CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD || Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC);
	#else // WCS_BUILD_RTX
	OverheadView = (Cam->CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD || Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC);
	#endif // WCS_BUILD_RTX

	// determine the text string
	ReplaceStringTokens(MesgCopy, Labl->MesgText, CurVec, LabelVtx);
	OrigStrLen = (long)strlen(MesgCopy);

	UseLoResFont = Labl->HiResFont == WCS_EFFECTS_LABEL_HIRESFONT_ALWAYS ? 0: 
		Labl->HiResFont == WCS_EFFECTS_LABEL_HIRESFONT_NEVER ? 1: 
		OrigStrLen > 40 || OrigStrLen == 0;
	IRF = new InternalRasterFont(UseLoResFont);

	if (IRF && (LabelRast = new Raster))
		{
		FI.SetFont(IRF->FontDataPointer, (USHORT)IRF->InternalFontWidth, (USHORT)IRF->InternalFontHeight);

		TextLineHeight = MasterSize * Labl->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTLINEHEIGHT].CurValue;
		TextOutlineSize = Labl->OutlineEnabled ? MasterSize * Labl->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTOUTLINEWIDTH].CurValue: 0.0;
		TextLineSpace = MasterSize * Labl->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTLINESPACE].CurValue + TextOutlineSize;
		TextLetterSpace = MasterSize * Labl->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTLETTERSPACE].CurValue + TextOutlineSize;
		MaxTextWidth = MasterSize * Labl->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGWIDTH].CurValue;
		MaxTextHeight = MasterSize * Labl->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGHEIGHT].CurValue;

		LabelScale = LabelTextScale = (IRF->InternalFontHeight / 16) / TextLineHeight;	// pixels/meter
		EdgingPixels = Labl->OutlineEnabled ? quickftol(LabelScale * TextOutlineSize): 0;
		KerningPixels = quickftol(LabelScale * TextLetterSpace);
		LeadingPixels = quickftol(LabelScale * TextLineSpace);
		LinePixels = quickftol(LabelScale * TextLineHeight) + LeadingPixels;	// jump down by this much for each line

		// measure the width and height of the text string
		if (Labl->TextEnabled && OrigStrLen > 0 && TextLineHeight > 0.0)
			{
			// count the number of text lines
			StrStart = 0;
			for (CurChar = 0; CurChar <= OrigStrLen; CurChar ++)
				{
				if (MesgCopy[CurChar] == 10 || MesgCopy[CurChar] == 13 || MesgCopy[CurChar] == 0)
					{
					MesgCopy[CurChar] = 0;
					// measure length of text line
					ThisLinePixels = FI.TextWidth(&MesgCopy[StrStart], (unsigned char)KerningPixels);

					ThisLineWidth = (ThisLinePixels) / LabelScale;

					if ((Labl->FlagWidthStyle == WCS_EFFECTS_LABEL_FLAGSIZE_FLOATING ||
						Labl->FlagWidthStyle == WCS_EFFECTS_LABEL_FLAGSIZE_FIXED) &&
						Labl->WordWrapEnabled &&
						ThisLineWidth > MaxTextWidth)
						{
						SolutionFound = SpaceFound = 0;
						// step CurChar backwards testing zeros in all the blank spaces
						for (TestChar = CurChar - 1; TestChar > StrStart; TestChar --)
							{
							if (MesgCopy[TestChar] == ' ')
								{
								SpaceFound = 1;
								MesgCopy[TestChar] = 0;
								ThisLinePixels = FI.TextWidth(&MesgCopy[StrStart], (unsigned char)KerningPixels);
								ThisLineWidth = (ThisLinePixels) / LabelScale;
								if (ThisLineWidth <= MaxTextWidth)
									{
									CurChar = TestChar;
									SolutionFound = 1;
									break;
									} // if
								else
									{
									MesgCopy[TestChar] = ' ';
									LastSpace = TestChar;
									} // else
								} // if
							} // for
						if (SpaceFound && ! SolutionFound)
							{
							MesgCopy[LastSpace] = 0;
							CurChar = LastSpace;
							} // if
						} // if

					if (ThisLinePixels > MaxLinePixels)
						MaxLinePixels = ThisLinePixels;
					NumTextLines ++;
					StrStart = CurChar + 1;
					} // if
				} // for
			TextWidth = (MaxLinePixels + EdgingPixels + EdgingPixels) / LabelScale;
			TextHeight = NumTextLines * TextLineHeight + (NumTextLines - 1) * TextLineSpace + EdgingPixels + EdgingPixels;

			// adjust size based on width style
			if (Labl->FlagWidthStyle == WCS_EFFECTS_LABEL_FLAGSIZE_FLOATING)
				{
				FlagTextWidth = min(TextWidth, MaxTextWidth);;
				} // if
			else
				{
				FlagTextWidth = MaxTextWidth;
				if (Labl->FlagWidthStyle == WCS_EFFECTS_LABEL_FLAGSIZE_FIXEDFLOATTEXT)
					{
					// is text wider than fixed width?
					if (TextWidth > FlagTextWidth)
						{
						// rescale text so it is smaller
						FinalHorScale = FlagTextWidth / TextWidth;
						LabelScale *= (1.0 / FinalHorScale);			// pixels/meter
						TextHeight *= FinalHorScale;
						} // if
					} // if
				TextWidth = FlagTextWidth;
				} // else

			// now the height
			if (Labl->FlagHeightStyle == WCS_EFFECTS_LABEL_FLAGSIZE_FLOATING)
				{
				FlagTextHeight = min(TextHeight, MaxTextHeight);
				} // if
			else
				{
				FlagTextHeight =  MaxTextHeight;
				} // else
			} // if
		else
			{
			TextWidth = 0.0;
			TextHeight = 0.0;
			FlagTextWidth = 0.0;
			FlagTextHeight = 0.0;
			} // else

		// compute the size of the label in real world units including pole, flag and border
		FlagHeight = FlagTextHeight + TextLineHeight + TextLineSpace + 2.0 * TextOutlineSize;
		// seem to need some extra space width-wise
		FlagWidth = FlagTextWidth + TextLineHeight * 2 + TextLineSpace + 2.0 * TextOutlineSize;
		//TextBlockHeight = TextHeight + 2.0 * TextOutlineSize;
		//TextBlockWidth = TextWidth + 2.0 * TextOutlineSize;
		TextBlockHeight = FlagTextHeight + 2.0 * TextOutlineSize;
		TextBlockWidth = FlagTextWidth + 2.0 * TextOutlineSize;
		BorderWidth = Labl->BorderEnabled ? MasterSize * Labl->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_BORDERWIDTH].CurValue: 0.0;
		PoleWidth = (Labl->PoleEnabled && (! OverheadView || Labl->OverheadViewPole)) ? MasterSize * Labl->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_POLEWIDTH].CurValue: 0.0;
		TopPoleWidth = ((Labl->PoleEnabled && (! OverheadView || Labl->OverheadViewPole)) && Labl->PoleFullHeight && Labl->PolePosition != WCS_EFFECTS_LABEL_POLEPOSITION_CENTER) ? MasterSize * Labl->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_POLEWIDTH].CurValue: 0.0;
		PoleHeight = (Labl->PoleEnabled && (! OverheadView || Labl->OverheadViewPole)) ? MasterSize * Labl->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_POLEHEIGHT].CurValue: 0.0;

		// compute bitmap sizes
		// use same scale as for computing text sizes
		// LabelScale = pixels / meter

		// convert the real-world size to a pixel size with a uniform XY scale.
		LabelW = quickftol(WCS_ceil(FlagWidth * LabelScale) + 2 * WCS_ceil(BorderWidth * LabelScale) + WCS_ceil(TopPoleWidth * LabelScale));
		LabelH = quickftol(WCS_ceil(FlagHeight * LabelScale) + 2 * WCS_ceil(BorderWidth * LabelScale) + WCS_ceil(PoleHeight * LabelScale));
		TextBlockW = quicklongceil(TextBlockWidth * LabelScale);
		TextBlockH = quicklongceil(TextBlockHeight * LabelScale);
		TextBlockOffsetW = quickftol(WCS_ceil((FlagWidth - TextBlockWidth) * .5 * LabelScale) + WCS_ceil(BorderWidth * LabelScale) + WCS_ceil(TopPoleWidth * LabelScale));
		TextBlockOffsetH = quickftol(WCS_ceil((FlagHeight - TextBlockHeight) * .5 * LabelScale) + WCS_ceil(BorderWidth * LabelScale));
		if (TextBlockW + TextBlockOffsetW > LabelW)
			TextBlockOffsetW = LabelW - TextBlockW;
		if (TextBlockH + TextBlockOffsetH > LabelH)
			TextBlockOffsetH = LabelH - TextBlockH;
		LineStartW = quickftol(WCS_ceil(BorderWidth * LabelScale) + WCS_ceil(TopPoleWidth * LabelScale) + WCS_ceil((TextLineHeight * 2 + TextLineSpace) * LabelScale) * .5 + TextOutlineSize * LabelScale);
		LineStartH = quickftol(WCS_ceil(BorderWidth * LabelScale) + WCS_ceil((TextLineHeight + TextLineSpace) * LabelScale) * .5 + TextOutlineSize * LabelScale);
		LineStartW -= TextBlockOffsetW;
		LineStartH -= TextBlockOffsetH;

		// compute the actual size of the label
		LabelWidth = (LabelW / LabelScale);// * FinalHorScale;
		LabelHeight = (LabelH / LabelScale);// * FinalHorScale;

		// calculate the label height in pixels based on size and distance or Z
		if (Cam->Orthographic || Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
			LabelHeightFactor = Cam->VertScale;
		else if (Cam->ApplyLensDistortion)
			LabelHeightFactor = Cam->VertScale / LabelVtx->Q;
		else
			LabelHeightFactor = Cam->VertScale / LabelVtx->ScrnXYZ[2];
		LabelPixelHeight = LabelHeight * LabelHeightFactor;

		// compute the label width based on image proportions and pixel aspect
		LabelPixelWidth = LabelPixelHeight * ((double)LabelW / LabelH) / Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue;

		ScaleMult = 1;
		while (LabelW < LabelPixelWidth && LabelH < LabelPixelHeight && LabelW < 1000 && LabelH < 1000)
			{
			LabelW *= 2;
			LabelH *= 2;
			ScaleMult *= 2;
			} // while
		LabelScale *= ScaleMult;
		TextBlockOffsetW *= ScaleMult;
		TextBlockOffsetH *= ScaleMult;

		// create raster bitmaps
		LabelRast->Rows = LabelH;
		LabelRast->Cols = LabelW;

		if (TextBlockW > 0.0 && TextBlockH > 0.0)
			{
			LabelRedBuf = (unsigned char *)AppMem_Alloc(TextBlockW * TextBlockH, APPMEM_CLEAR);
			LabelGrnBuf = (unsigned char *)AppMem_Alloc(TextBlockW * TextBlockH, APPMEM_CLEAR);
			LabelBluBuf = (unsigned char *)AppMem_Alloc(TextBlockW * TextBlockH, APPMEM_CLEAR);
			LabelAlphaBuf = (unsigned char *)AppMem_Alloc(TextBlockW * TextBlockH, APPMEM_CLEAR);
			LabelEdgeBuf = (unsigned char *)AppMem_Alloc(TextBlockW * TextBlockH, APPMEM_CLEAR);
			RenderText = 1;
			} // if

		if ((! RenderText || (LabelRedBuf && LabelGrnBuf && LabelBluBuf && LabelAlphaBuf && LabelEdgeBuf)) && 
			LabelRast->AllocRGBBands() && LabelRast->AllocAltFloatBand(LabelH, LabelW))
			{
			// setup FontImage with dimensions of bitmaps
			if (RenderText)
				FI.SetOutput(TextBlockW, TextBlockH, LabelRedBuf, LabelGrnBuf, LabelBluBuf, LabelAlphaBuf, LabelEdgeBuf);

			// rasterize the flag, border and pole honoring transparency settings
			NumFlagPixels = quickftol(WCS_ceil(FlagHeight * LabelScale) + 2 * WCS_ceil(BorderWidth * LabelScale));
			if (NumFlagPixels > LabelH)
				NumFlagPixels = LabelH;
			if (Labl->FlagEnabled)
				{
				Red = (unsigned char)(Labl->CompleteFlagColor[0] * 255);
				Green = (unsigned char)(Labl->CompleteFlagColor[1] * 255);
				Blue = (unsigned char)(Labl->CompleteFlagColor[2] * 255);
				FloatAlpha = (float)(1.0 - Labl->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_FLAGTRANSPAR].CurValue);
				for (Y = PixZip = 0; Y < NumFlagPixels; Y ++)
					{
					for (X = 0; X < LabelW; X ++, PixZip ++)
						{
						LabelRast->Red[PixZip] = Red;
						LabelRast->Green[PixZip] = Green;
						LabelRast->Blue[PixZip] = Blue;
						LabelRast->AltFloatMap[PixZip] = FloatAlpha;
						} // for
					} // for
				} // if
			if (Labl->BorderEnabled)
				{
				Red = (unsigned char)(Labl->CompleteBorderColor[0] * 255);
				Green = (unsigned char)(Labl->CompleteBorderColor[1] * 255);
				Blue = (unsigned char)(Labl->CompleteBorderColor[2] * 255);
				if (! Red && ! Green && ! Blue)
					Red = Green = Blue = 1;
				FloatAlpha = (float)(1.0 - Labl->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_BORDERTRANSPAR].CurValue);
				NumPixels = quickftol(WCS_ceil(BorderWidth * LabelScale));
				if (NumPixels > LabelH)
					NumPixels = LabelH;
				for (Y = PixZip = 0; Y < NumPixels; Y ++)
					{
					for (X = 0; X < LabelW; X ++, PixZip ++)
						{
						LabelRast->Red[PixZip] = Red;
						LabelRast->Green[PixZip] = Green;
						LabelRast->Blue[PixZip] = Blue;
						LabelRast->AltFloatMap[PixZip] = FloatAlpha;
						} // for
					} // for
				for (Y = NumFlagPixels - NumPixels; Y < NumFlagPixels; Y ++)
					{
					PixZip = Y * LabelW;
					for (X = 0; X < LabelW; X ++, PixZip ++)
						{
						LabelRast->Red[PixZip] = Red;
						LabelRast->Green[PixZip] = Green;
						LabelRast->Blue[PixZip] = Blue;
						LabelRast->AltFloatMap[PixZip] = FloatAlpha;
						} // for
					} // for
				if (Labl->PoleEnabled && Labl->PoleFullHeight && Labl->PolePosition == WCS_EFFECTS_LABEL_POLEPOSITION_LEFT)
					NumPixels = quicklongceil((PoleWidth + BorderWidth) * LabelScale);
				else
					NumPixels = quicklongceil(BorderWidth * LabelScale);
				if (NumPixels > LabelW)
					NumPixels = LabelW;
				for (Y = 0; Y < NumFlagPixels; Y ++)
					{
					PixZip = Y * LabelW;
					for (X = 0; X < NumPixels; X ++, PixZip ++)
						{
						LabelRast->Red[PixZip] = Red;
						LabelRast->Green[PixZip] = Green;
						LabelRast->Blue[PixZip] = Blue;
						LabelRast->AltFloatMap[PixZip] = FloatAlpha;
						} // for
					} // for
				if (Labl->PoleEnabled && Labl->PoleFullHeight && Labl->PolePosition == WCS_EFFECTS_LABEL_POLEPOSITION_RIGHT)
					NumPixels = quicklongceil((PoleWidth + BorderWidth) * LabelScale);
				else
					NumPixels = quicklongceil(BorderWidth * LabelScale);
				if (NumPixels > LabelW)
					NumPixels = LabelW;
				for (Y = 0; Y < NumFlagPixels; Y ++)
					{
					PixZip = Y * LabelW + LabelW - NumPixels;
					for (X = LabelW - NumPixels; X < LabelW; X ++, PixZip ++)
						{
						LabelRast->Red[PixZip] = Red;
						LabelRast->Green[PixZip] = Green;
						LabelRast->Blue[PixZip] = Blue;
						LabelRast->AltFloatMap[PixZip] = FloatAlpha;
						} // for
					} // for
				} // if
			if (Labl->PoleEnabled && (! OverheadView || Labl->OverheadViewPole))
				{
				Red = (unsigned char)(Labl->CompletePoleColor[0] * 255);
				Green = (unsigned char)(Labl->CompletePoleColor[1] * 255);
				Blue = (unsigned char)(Labl->CompletePoleColor[2] * 255);
				if (! Red && ! Green && ! Blue)
					Red = Green = Blue = 1;
				FloatAlpha = (float)(1.0 - Labl->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_POLETRANSPAR].CurValue);
				NumPixels = Labl->PoleFullWidth ? LabelW: quicklongceil(PoleWidth * LabelScale);
				if (NumPixels > LabelW)
					NumPixels = LabelW;
				StartPixel = (Labl->PoleFullWidth || Labl->PolePosition == WCS_EFFECTS_LABEL_POLEPOSITION_LEFT) ? 0:
					Labl->PolePosition == WCS_EFFECTS_LABEL_POLEPOSITION_RIGHT ? LabelW - NumPixels: LabelW / 2 - NumPixels / 2;
				EndPixel = StartPixel + NumPixels;
				// top of the pole
				if (! Labl->PoleFullWidth && Labl->PoleFullHeight && (Labl->PolePosition == WCS_EFFECTS_LABEL_POLEPOSITION_LEFT ||
					Labl->PolePosition == WCS_EFFECTS_LABEL_POLEPOSITION_RIGHT))
					{
					for (Y = 0; Y < NumFlagPixels; Y ++)
						{
						X = StartPixel;
						PixZip = Y * LabelW + X;
						for (; X < EndPixel; X ++, PixZip ++)
							{
							LabelRast->Red[PixZip] = Red;
							LabelRast->Green[PixZip] = Green;
							LabelRast->Blue[PixZip] = Blue;
							LabelRast->AltFloatMap[PixZip] = FloatAlpha;
							} // for
						} // for
					} // if
				// now the base of the pole
				if (Labl->PoleBaseStyle == WCS_EFFECTS_LABEL_POLEBASESTYLE_SQUARE && 
					(Labl->PoleStyle == WCS_EFFECTS_LABEL_POLESTYLE_VERTICAL || 
					Labl->PolePosition != WCS_EFFECTS_LABEL_POLEPOSITION_CENTER))
					{
					for (Y = NumFlagPixels; Y < LabelH; Y ++)
						{
						X = StartPixel;
						PixZip = Y * LabelW + X;
						for (; X < EndPixel; X ++, PixZip ++)
							{
							LabelRast->Red[PixZip] = Red;
							LabelRast->Green[PixZip] = Green;
							LabelRast->Blue[PixZip] = Blue;
							LabelRast->AltFloatMap[PixZip] = FloatAlpha;
							} // for

						} // for
					} // if
				else
					{
					// is the pole tapered
					if (Labl->PoleBaseStyle == WCS_EFFECTS_LABEL_POLEBASESTYLE_TAPERED)
						Slant = NumPixels * .5 / (LabelH - NumFlagPixels);
					else
						Slant = 0.0;
					// is the pole angled?
					if (Labl->PoleStyle == WCS_EFFECTS_LABEL_POLESTYLE_ANGLED && 
						Labl->PoleBaseStyle == WCS_EFFECTS_LABEL_POLEBASESTYLE_TAPERED &&
						Labl->PolePosition != WCS_EFFECTS_LABEL_POLEPOSITION_CENTER)
						{
						LineOffset = LabelW * .5 / (LabelH - NumFlagPixels);
						if (Labl->PolePosition == WCS_EFFECTS_LABEL_POLEPOSITION_RIGHT)
							LineOffset = -LineOffset;
						} // if
					else
						LineOffset = 0.0;
					FloatStart = StartPixel + Slant * .5 + LineOffset * .5;
					FloatEnd = EndPixel - Slant * .5 + LineOffset * .5;
					for (Y = NumFlagPixels; Y < LabelH; Y ++)
						{
						StartPixel = quickftol(WCS_floor(FloatStart));
						EndPixel = quickftol(WCS_ceil(FloatEnd));
						X = StartPixel;
						PixZip = Y * LabelW + X;
						StartWt = 1.0 - (FloatStart - StartPixel);
						EndWt = 1.0 - (EndPixel - FloatEnd);
						for (; X < EndPixel && FloatEnd >= FloatStart; X ++, PixZip ++)
							{
							SlantWt = X > StartPixel ? 1.0: StartWt;
							SlantWt *= (X < EndPixel - 1) ? 1.0: EndWt;
							LabelRast->Red[PixZip] = Red;
							LabelRast->Green[PixZip] = Green;
							LabelRast->Blue[PixZip] = Blue;
							LabelRast->AltFloatMap[PixZip] = FloatAlpha * (float)SlantWt;
							} // for
						FloatStart += Slant;
						FloatStart += LineOffset;
						FloatEnd -= Slant;
						FloatEnd += LineOffset;
						} // for
					} // else
				} // if

			// rasterize the text and outline
			if (FlagTextHeight > 0.0)
				{
				StrStart = 0;
				Red = (unsigned char)(Labl->CompleteTextColor[0] * 255);
				Green = (unsigned char)(Labl->CompleteTextColor[1] * 255);
				Blue = (unsigned char)(Labl->CompleteTextColor[2] * 255);
				if (! Red && ! Green && ! Blue)
					Red = Green = Blue = 1;
				OutlineRed = (unsigned char)(Labl->CompleteOutlineColor[0] * 255);
				OutlineGreen = (unsigned char)(Labl->CompleteOutlineColor[1] * 255);
				OutlineBlue = (unsigned char)(Labl->CompleteOutlineColor[2] * 255);
				if (! OutlineRed && ! OutlineGreen && ! OutlineBlue)
					OutlineRed = OutlineGreen = OutlineBlue = 1;
				FloatAlpha = (float)(1.0 - Labl->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTTRANSPAR].CurValue);
				EdgeAlpha = (float)(1.0 - Labl->AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_OUTLINETRANSPAR].CurValue);
				for (CurChar = 0; CurChar <= OrigStrLen; CurChar ++)
					{
					if (MesgCopy[CurChar] == 0)
						{
						FI.PrintText(LineStartW, LineStartH, &MesgCopy[StrStart], Red, Green, Blue, 
							Labl->Justification == WCS_EFFECTS_LABEL_JUSTIFY_LEFT ? WCS_FONTIMAGE_JUSTIFY_LEFT:
							Labl->Justification == WCS_EFFECTS_LABEL_JUSTIFY_RIGHT ? WCS_FONTIMAGE_JUSTIFY_RIGHT: WCS_FONTIMAGE_JUSTIFY_CENTER, 
							WCS_FONTIMAGE_DRAWMODE_NORMAL, (char)LeadingPixels, (unsigned char)KerningPixels, (unsigned char)EdgingPixels, NULL);
						StrStart = CurChar + 1;
						LineStartH += LinePixels;
						} // if
					} // for
				if (Labl->OutlineEnabled)
					FI.OutlineText((unsigned char)EdgingPixels);

				//FI.DebugDump();

				// copy text raster into flag raster
				for (Y = PixZip = 0; Y < TextBlockH; Y ++)
					{
					for (X = 0; X < TextBlockW; X ++, PixZip ++)
						{
						TempAlpha = 0.0f;
						if (LabelAlphaBuf[PixZip])
							{
							if (LabelEdgeBuf[PixZip])
								{
								SumTextAlpha = LabelAlphaBuf[PixZip] + LabelEdgeBuf[PixZip];
								TempRed = (unsigned char)((Red * LabelAlphaBuf[PixZip] + OutlineRed * LabelEdgeBuf[PixZip]) / SumTextAlpha);
								TempGreen = (unsigned char)((Green * LabelAlphaBuf[PixZip] + OutlineGreen * LabelEdgeBuf[PixZip]) / SumTextAlpha);
								TempBlue = (unsigned char)((Blue * LabelAlphaBuf[PixZip] + OutlineBlue * LabelEdgeBuf[PixZip]) / SumTextAlpha);
								TempAlpha = (float)((FloatAlpha * LabelAlphaBuf[PixZip] + EdgeAlpha * LabelEdgeBuf[PixZip]) / (float)SumTextAlpha);
								} // if
							else
								{
								SumTextAlpha = LabelAlphaBuf[PixZip];
								TempRed = Red;
								TempGreen = Green;
								TempBlue = Blue;
								TempAlpha = (float)(FloatAlpha * LabelAlphaBuf[PixZip] * (1.0 / 255.0));
								} // else
							} // if
						else if (LabelEdgeBuf[PixZip])
							{
							SumTextAlpha = LabelEdgeBuf[PixZip];
							TempRed = OutlineRed;
							TempGreen = OutlineGreen;
							TempBlue = OutlineBlue;
							TempAlpha = (float)(EdgeAlpha * LabelEdgeBuf[PixZip] * (1.0 / 255.0));
							} // if
						if (LabelAlphaBuf[PixZip] || LabelEdgeBuf[PixZip])
							{
							for (j = 0; j < ScaleMult; j ++)
								{
								DPixZip = (Y * ScaleMult + j + TextBlockOffsetH) * LabelW + X * ScaleMult + TextBlockOffsetW;
								for (i = 0; i < ScaleMult; i ++, DPixZip ++)
									{
									// the amount of flag color that is used in the final mix is
									// flag alpha * (1 - edge + text alpha / 255)
									LabelRast->AltFloatMap[DPixZip] *= (float)(1.0 - SumTextAlpha / 255.0);
									LabelRast->Red[DPixZip] = (unsigned char)(TempRed * TempAlpha + LabelRast->Red[DPixZip] * (1.0 - TempAlpha));
									LabelRast->Green[DPixZip] = (unsigned char)(TempGreen * TempAlpha + LabelRast->Green[DPixZip] * (1.0 - TempAlpha));
									LabelRast->Blue[DPixZip] = (unsigned char)(TempBlue * TempAlpha + LabelRast->Blue[DPixZip] * (1.0 - TempAlpha));
									TempAlpha += LabelRast->AltFloatMap[DPixZip];
									LabelRast->AltFloatMap[DPixZip] = (float)(min(1.0, TempAlpha));
									} // for
								} // for
							} // if
						} // for
					} // for
				} // if

			//LabelRast->SaveImage(0);

			// if Exporter then we do not render the label into the bitmap but instead call a method on
			// the Exporter to save the Raster image. And we add the label to the label list, a separate foliage list
			// that will be concatenated with the regular foliage list when rendering is done
			#ifdef WCS_BUILD_RTX
			if (Exporter)
				{
				// pole position will be recorded in foliage data structure
				if (Labl->PoleEnabled)
					{
					// put the pole base at the vertex location
					if (Labl->PolePosition == WCS_EFFECTS_LABEL_POLEPOSITION_CENTER || 
						Labl->PoleStyle == WCS_EFFECTS_LABEL_POLESTYLE_ANGLED)
						{
						HorShift = 0.0;
						} // if
					else if (Labl->PolePosition == WCS_EFFECTS_LABEL_POLEPOSITION_LEFT)
						{
						HorShift = 1.0;
						} // if
					else
						{
						HorShift = -1.0;
						} // if
					} // if
				else
					{
					if (Labl->AnchorPoint == WCS_EFFECTS_LABEL_ANCHORPOINT_LOWERLEFT || 
						Labl->AnchorPoint == WCS_EFFECTS_LABEL_ANCHORPOINT_LEFTEDGE ||
						Labl->AnchorPoint == WCS_EFFECTS_LABEL_ANCHORPOINT_UPPERLEFT)
						{
						HorShift = 1.0;
						} // if
					else if (Labl->AnchorPoint == WCS_EFFECTS_LABEL_ANCHORPOINT_UPPERRIGHT || 
						Labl->AnchorPoint == WCS_EFFECTS_LABEL_ANCHORPOINT_RIGHTEDGE ||
						Labl->AnchorPoint == WCS_EFFECTS_LABEL_ANCHORPOINT_LOWERRIGHT)
						{
						HorShift = -1.0;
						} // if
					else
						{
						HorShift = 0.0;
						} // if
					} // else

				// add the raster to the image obj lib with a unique name
				ImageID = Exporter->SaveLabelImage(LabelRast, ImageBase, Labl);

				if (FolListDat = new RealtimeFoliageData())
					{
					unsigned short FolBits;

					if(LabelHeight >= 1638)
						{
						FolListDat->Height = (unsigned short)LabelHeight;
						FolBits = 0;
						} // if
					else if(LabelHeight >= 163)
						{
						FolListDat->Height = (unsigned short)(LabelHeight * 10.0);
						FolBits = 0x01;
						} // else if
					else if(LabelHeight >= 16)
						{
						FolListDat->Height = (unsigned short)(LabelHeight * 100.0);
						FolBits = 0x02;
						} // else if
					else
						{
						FolListDat->Height = (unsigned short)(LabelHeight * 1000.0);
						FolBits = 0x03;
						} // else
					if(FolBits)
						{
						FolListDat->Height |= (FolBits << 14);
						} // if
						
					FolListDat->ElementID = (short)ImageID;
					// use bit 3 to tell it is a label, bit 4 for left pole, bit 5 for right pole
					FolListDat->BitInfo = (WCS_REALTIME_FOLDAT_BITINFO_LABEL | 
						(HorShift > 0.0 ? WCS_REALTIME_FOLDAT_BITINFO_LEFTPOLE: HorShift < 0.0 ? 
						WCS_REALTIME_FOLDAT_BITINFO_RIGHTPOLE: 0));
					FolListDat->ImageColorOpacity = 255;
					FolListDat->XYZ[0] = (float)(LabelVtx->Lon - TexRefLon);
					FolListDat->XYZ[1] = (float)(LabelVtx->Lat - TexRefLat);
					FolListDat->XYZ[2] = (float)(LabelVtx->Elev - TexRefElev);
					#ifdef WCS_BUILD_SX2
					// if this object is flagged as one that has click-to-query, set the flag and the pointers
					if (Labl->IsClickQueryEnabled())
						{
						FolListDat->MyEffect = Labl;
						FolListDat->MyVec = CurVec;
						FolListDat->BitInfo |= WCS_REALTIME_FOLDAT_BITINFO_CLICKQUERY;
						} // if
					#endif // WCS_BUILD_SX2
					AddLabelList(FolListDat);
					} // if FolListDat
				} // if
			else
			#endif // WCS_BUILD_RTX
				{
				// compute an elevation gradient
				LabelElevGrad = -LabelHeight / LabelPixelHeight;
				LabelTopElev = LabelVtx->Elev + LabelHeight;

				if (OverheadView)
					{
					LabelVtx->ScrnXYZ[2] -= (100.0 + 10.0 * LabelHeight);
					if (LabelVtx->ScrnXYZ[2] < 0.0)
						LabelVtx->ScrnXYZ[2] = 0.0;
					} // if

				MaxZOffset = LabelHeight * ((double)LabelW / LabelH) * .5;	// .25 was used in v4
				TestMergeOffset = DefaultEnvironment->FoliageBlending;
				TestMergeOffset *= ZMergeDistance;
				MinPixelSize = DefaultEnvironment->MinPixelSize;
				if (LabelPixelWidth < MinPixelSize)
					LabelPixelWidth = MinPixelSize;
				if (LabelPixelHeight < MinPixelSize)
					LabelPixelHeight = MinPixelSize;

				// compute the pixel bounds of the label
				// base position is the bottom of the label raster, depending on the pole position it can be the right, left or center
				// of the label or one of the corners or edges.
				if ((Labl->PoleEnabled && (! OverheadView || Labl->OverheadViewPole)))
					{
					VertShift = 0.0;
					// put the pole base at the vertex location
					if (Labl->PolePosition == WCS_EFFECTS_LABEL_POLEPOSITION_CENTER || 
						Labl->PoleStyle == WCS_EFFECTS_LABEL_POLESTYLE_ANGLED)
						{
						HorShift = 0.0;
						} // if
					else if (Labl->PolePosition == WCS_EFFECTS_LABEL_POLEPOSITION_LEFT)
						{
						HorShift = LabelPixelWidth * .5;
						} // if
					else
						{
						HorShift = -LabelPixelWidth * .5;
						} // if
					} // if
				else
					{
					if (! OverheadView && 
						(Labl->AnchorPoint == WCS_EFFECTS_LABEL_ANCHORPOINT_CENTER || 
						Labl->AnchorPoint == WCS_EFFECTS_LABEL_ANCHORPOINT_LEFTEDGE ||
						Labl->AnchorPoint == WCS_EFFECTS_LABEL_ANCHORPOINT_RIGHTEDGE))
						{
						VertShift = LabelPixelHeight * .5;
						} // if
					else if (! OverheadView &&
						(Labl->AnchorPoint == WCS_EFFECTS_LABEL_ANCHORPOINT_UPPERLEFT || 
						Labl->AnchorPoint == WCS_EFFECTS_LABEL_ANCHORPOINT_TOPEDGE ||
						Labl->AnchorPoint == WCS_EFFECTS_LABEL_ANCHORPOINT_UPPERRIGHT))
						{
						VertShift = LabelPixelHeight;
						} // if
					else
						{
						VertShift = 0.0;
						} // if
					if (Labl->AnchorPoint == WCS_EFFECTS_LABEL_ANCHORPOINT_LOWERLEFT || 
						Labl->AnchorPoint == WCS_EFFECTS_LABEL_ANCHORPOINT_LEFTEDGE ||
						Labl->AnchorPoint == WCS_EFFECTS_LABEL_ANCHORPOINT_UPPERLEFT)
						{
						HorShift = LabelPixelWidth * .5;
						} // if
					else if (Labl->AnchorPoint == WCS_EFFECTS_LABEL_ANCHORPOINT_UPPERRIGHT || 
						Labl->AnchorPoint == WCS_EFFECTS_LABEL_ANCHORPOINT_RIGHTEDGE ||
						Labl->AnchorPoint == WCS_EFFECTS_LABEL_ANCHORPOINT_LOWERRIGHT)
						{
						HorShift = -LabelPixelWidth * .5;
						} // if
					else
						{
						HorShift = 0.0;
						} // if
					} // else
				LabelVtx->ScrnXYZ[0] += HorShift;
				LabelVtx->ScrnXYZ[1] += VertShift;

				// set up FoliagePlotData
				FolPlotData.RenderOccluded = (char)Labl->RenderOccluded;
				FolPlotData.ColorImage = TRUE;
				FolPlotData.Shade3D = FALSE;
				FolPlotData.Poly = Poly;
				FolPlotData.Vert = LabelVtx;
				FolPlotData.SourceRast = LabelRast;
				FolPlotData.WidthInPixels = LabelPixelWidth;
				FolPlotData.HeightInPixels = LabelPixelHeight;
				FolPlotData.ReplaceColor = DummyColor;
				FolPlotData.TopElev = LabelTopElev;
				FolPlotData.ElevGrad = LabelElevGrad;
				FolPlotData.Opacity = 1.0;
				FolPlotData.ColorImageOpacity = 1.0;
				FolPlotData.OrientationShading = LabelBacklight;
				FolPlotData.MaxZOffset = MaxZOffset;
				FolPlotData.TestPtrnOffset = TestMergeOffset;

				// special foliage bands are required even if not needed
				//if (Success = LabelRast->CreateFoliageBands())
					{
					// set flags on the raster so it doesn't try to load it in the foliage plotter
					LabelRast->SetInitFlags(WCS_RASTER_INITFLAGS_IMAGELOADED | WCS_RASTER_INITFLAGS_FOLIAGELOADED | WCS_RASTER_INITFLAGS_FOLIAGEDOWNSAMPLED | WCS_RASTER_INITFLAGS_DOWNSAMPLED);
					// paste the label into the image using the foliage plotting routine
					PlotFoliage();
					} // if
				} // else not Exporter
			} // if bitmaps allocated

		if (LabelRedBuf)
			AppMem_Free(LabelRedBuf, TextBlockW * TextBlockH);
		if (LabelGrnBuf)
			AppMem_Free(LabelGrnBuf, TextBlockW * TextBlockH);
		if (LabelBluBuf)
			AppMem_Free(LabelBluBuf, TextBlockW * TextBlockH);
		if (LabelAlphaBuf)
			AppMem_Free(LabelAlphaBuf, TextBlockW * TextBlockH);
		if (LabelEdgeBuf)
			AppMem_Free(LabelEdgeBuf, TextBlockW * TextBlockH);
		if (ImageID == 0)	// image has not been added to image library for use by exporter
			delete LabelRast;
		else
			LabelRast->FreeAllBands(WCS_RASTER_LONGEVITY_FORCEFREE);
		} // if Raster
	if (IRF)
		delete IRF;
	} // if In front of camera

return (Success);

} // Renderer::RenderLabel

/*===========================================================================*/

void Renderer::ReplaceStringTokens(char *MesgCopy, char *OrigMesg, Joe *CurVec, VertexDEM *CurVert)
{
double JoeVal, DVal, TargDist;
long LetterCt, OrigLen, AttribTokenLen, AttribBegun, AttribStart, AttribEnd, LastSegStart, UnitChar, UnitID, 
	PrecisionFound, NextLetter, FrameNum, FrameRate;
LayerStub *Stub;
char *JoeStr;
VertexDEM Begin, End;
bool italics = false;
char AttribName[256], TempStr[WCS_LABELTEXT_MAXLEN], DefaultDigits, TestLetter, Units[8];

OrigLen = (long)strlen(OrigMesg);
MesgCopy[0] = 0;

AttribTokenLen = (long)strlen(Attrib_TextSymbol);
AttribBegun = 0;
LastSegStart = 0;

for (LetterCt = 0; LetterCt <= OrigLen; LetterCt ++)
	{
	DefaultDigits = '2';	// default # decimal digits to print
	PrecisionFound = 0;
	if (! OrigMesg[LetterCt])
		{
		strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
		TempStr[LetterCt - LastSegStart] = 0;
		strcat_NeHeUpper(MesgCopy, TempStr, italics);
		break;
		} // if
	if (OrigMesg[LetterCt] == 10 || OrigMesg[LetterCt] == 13)
		{
		strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
		TempStr[LetterCt - LastSegStart] = 0;
		strcat_NeHeUpper(MesgCopy, TempStr, italics);
		strcat(MesgCopy, "\n");
		if (OrigMesg[LetterCt + 1] == 10 || OrigMesg[LetterCt + 1] == 13)
			LetterCt ++;
		LastSegStart = LetterCt + 1;
		italics = false;
		continue;
		} // if
	if (! strncmp(&OrigMesg[LetterCt], Attrib_TextSymbol, AttribTokenLen))
		{
		if (! AttribBegun)
			{
			AttribBegun = 1;
			AttribStart = LetterCt + AttribTokenLen;
			strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
			TempStr[LetterCt - LastSegStart] = 0;
			strcat(MesgCopy, TempStr);
			} // if
		else
			{
			AttribEnd = LetterCt;
			strncpy(AttribName, &OrigMesg[AttribStart], AttribEnd - AttribStart);
			AttribName[AttribEnd - AttribStart] = 0;
			AttribBegun = 0;
			LastSegStart = AttribEnd + AttribTokenLen;
			// find the attribute on this Joe, it might be text or numeric
			if (CurVec)
				{
				if (Stub = CurVec->CheckTextAttributeExistance(AttribName))
					{
					if (JoeStr = (char *)CurVec->GetTextAttributeValue(Stub))
						{
						strcat(MesgCopy, JoeStr);
						} // if
					} // if
				else if (Stub = CurVec->CheckIEEEAttributeExistance(AttribName))
					{
					if (JoeVal = CurVec->GetIEEEAttributeValue(Stub))
						{
						sprintf(TempStr, "%f", JoeVal);
						TrimZeros(TempStr);
						strcat(MesgCopy, TempStr);
						} // if
					} // if
				} // if
			} // else
		} // if text match
	else if (! strncmp(&OrigMesg[LetterCt], "&C", 2))
		{
		NextLetter = LetterCt + 2;
		TestLetter = OrigMesg[NextLetter];
		if ((TestLetter == 'B') || (TestLetter == 'C')
			|| (TestLetter == 'F') || (TestLetter == 'H') 
			|| (TestLetter == 'N') || (TestLetter == 'P') 
			|| (TestLetter == 'X') || (TestLetter == 'Y') 
			|| (TestLetter == 'Z') || (TestLetter == 'T'))
			{
			strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
			TempStr[LetterCt - LastSegStart] = 0;
			strcat(MesgCopy, TempStr);
			if (OrigMesg[NextLetter + 1] == '%')
				{
				NextLetter ++;
				if ((OrigMesg[NextLetter + 1] >= '0') && (OrigMesg[NextLetter + 1] <= '9'))
					{
					NextLetter ++;
					DefaultDigits = OrigMesg[NextLetter];
					PrecisionFound = 1;
					} // if
				} // if
			switch (TestLetter)
				{
				case 'B':	// bank
					{
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].GetCurValue();
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &CB
				case 'F':	// fov
					{
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].GetCurValue();
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &CF
				case 'H':	// heading
					{
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].GetCurValue();
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &CH
				case 'N':	// name
					{
					strcat_NeHeUpper(MesgCopy, Cam->GetName(), italics);
					LetterCt = NextLetter;
					break;
					} // &CN
				case 'P':	// pitch
					{
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].GetCurValue();
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &CP
				case 'X':	// longitude
					{
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetCurValue();
					if (GlobalApp->MainProj->Prefs.PosLonHemisphere == WCS_PROJPREFS_LONCONVENTION_POSEAST)
						DVal = -DVal;
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &CX
				case 'Y':	// latitude
					{
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetCurValue();
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &CY
				case 'Z':	// elevation
					{
					UnitID = WCS_USEFUL_UNIT_METER;
					// look for unit suffix
					if (PrecisionFound)
						{
						UnitChar = 0;
						while ((OrigMesg[NextLetter + 1] != 0) && (OrigMesg[NextLetter + 1] != 13) && (OrigMesg[NextLetter + 1] !=' ') && (OrigMesg[NextLetter + 1] != '&') && (UnitChar < 7))
							{
							NextLetter ++;
							Units[UnitChar ++] = OrigMesg[NextLetter];
							} // while
						Units[UnitChar] = 0;
						UnitID = MatchUnitSuffix(Units);
						if (UnitID < 0)
							UnitID = WCS_USEFUL_UNIT_METER;
						} // if
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetCurValue();
					if (UnitID != WCS_USEFUL_UNIT_METER)
						DVal = ConvertFromMeters(DVal, UnitID);
					sprintf(TempStr, "%.*f%s", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'), GetUnitSuffix(UnitID));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &CZ
				case 'C':	// Compass bearing
					{
					DVal = Cam->CamHeading;
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &CC
				case 'T':	// tilt (including targetting and pitch)
					{
					DVal = Cam->CamPitch;
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &CT
				default:
					break;
				} // switch
			LastSegStart = LetterCt + 1;
			} // if
		} // else if &C
	else if (! strncmp(&OrigMesg[LetterCt], "&F", 2))
		{
		NextLetter = LetterCt + 2;
		TestLetter = OrigMesg[NextLetter];
		if (TestLetter == 'N' || TestLetter == 'S')
			{
			strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
			TempStr[LetterCt - LastSegStart] = 0;
			strcat(MesgCopy, TempStr);
			FrameRate = (unsigned long)(Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue + 0.5);
			FrameNum = quickftol(FrameRate * RenderTime);
			switch (TestLetter)
				{
				case 'N':	// frame number
					{
					NextLetter ++;
					if ((OrigMesg[NextLetter] < '1') || (OrigMesg[NextLetter] > '9'))
						break;
					DefaultDigits = OrigMesg[NextLetter];
					sprintf(TempStr, "%0*u", DefaultDigits - '0', FrameNum);
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &FN
				case 'S':	// SMPTE type timecode
					{
					unsigned long h, m, s, f, t;
					if (FrameRate != 0)
						{
						f = (unsigned long)(FrameNum % FrameRate);
						t = (unsigned long)(FrameNum / FrameRate);	// this frames time in seconds at this frame rate
						h = t / 3600;
						m = (t % 3600) / 60;
						s = t % 60;
						sprintf(TempStr, "%02u:%02u:%02u:%02u", h, m, s, f);
						}
					else
						sprintf(TempStr, "00:00:00:00");
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &FS
				default:
					break;
				} // switch
			LastSegStart = LetterCt + 1;
			} // if
		} // else if &F
	else if (! strncmp(&OrigMesg[LetterCt], "&I", 2))
		{
		strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
		TempStr[LetterCt - LastSegStart] = 0;
		strcat_NeHeUpper(MesgCopy, TempStr, italics);
		LetterCt++;
		LastSegStart = LetterCt + 1;
		italics = !italics;
		} // else if &I
	else if (! strncmp(&OrigMesg[LetterCt], "&P", 2))
		{
		NextLetter = LetterCt + 2;
		TestLetter = OrigMesg[NextLetter];
		if (TestLetter == 'N')
			{
			strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
			TempStr[LetterCt - LastSegStart] = 0;
			strcat(MesgCopy, TempStr);
			strcat(MesgCopy, ProjectBase->projectname);
			LetterCt = NextLetter;
			LastSegStart = LetterCt + 1;
			} // if &PN
		} // else if &P
	else if (! strncmp(&OrigMesg[LetterCt], "&R", 2))
		{
		NextLetter = LetterCt + 2;
		TestLetter = OrigMesg[NextLetter];
		if (TestLetter == 'N' || TestLetter == 'D' || TestLetter == 'E' || TestLetter == 'T')
			{
			time_t ltime;
			struct tm *now;

			strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
			TempStr[LetterCt - LastSegStart] = 0;
			strcat(MesgCopy, TempStr);
			switch (TestLetter)
				{
				case 'N':	// render opts name
					{
					strcat(MesgCopy, Opt->GetName());
					LetterCt = NextLetter;
					break;
					} // &RN
				case 'D':	// render date & time
					{
					time(&ltime);
					now = localtime(&ltime);
					strftime(TempStr, sizeof(TempStr), "%c", now);
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &RD
				case 'E':	// render date only
					{
					time(&ltime);
					now = localtime(&ltime);
					strftime(TempStr, sizeof(TempStr), "%x", now);
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &RE
				case 'T':	// render time only
					{
					time(&ltime);
					now = localtime(&ltime);
					strftime(TempStr, sizeof(TempStr), "%X", now);
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &RT
				default:
					break;
				} // switch
			LastSegStart = LetterCt + 1;
			} // if
		} // else if &R
	else if (! strncmp(&OrigMesg[LetterCt], "&T", 2))
		{
		NextLetter = LetterCt + 2;
		TestLetter = OrigMesg[NextLetter];
		if ((TestLetter == 'X') || (TestLetter == 'Y')
			|| (TestLetter == 'Z') || (TestLetter == 'D'))
			{
			strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
			TempStr[LetterCt - LastSegStart] = 0;
			strcat(MesgCopy, TempStr);
			if (OrigMesg[NextLetter + 1] == '%')
				{
				NextLetter ++;
				if ((OrigMesg[NextLetter + 1] >= '0') && (OrigMesg[NextLetter + 1] <= '9'))
					{
					NextLetter ++;
					DefaultDigits = OrigMesg[NextLetter];
					PrecisionFound = 1;
					} // if
				} // if
			switch (TestLetter)
				{
				case 'X':	// longitude
					{
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].GetCurValue();
					if (GlobalApp->MainProj->Prefs.PosLonHemisphere == WCS_PROJPREFS_LONCONVENTION_POSEAST)
						DVal = -DVal;
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &TX
				case 'Y':	// latitude
					{
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetCurValue();
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &TY
				case 'Z':	// elevation
					{
					UnitID = WCS_USEFUL_UNIT_METER;
					// look for unit suffix
					if (PrecisionFound)
						{
						UnitChar = 0;
						while ((OrigMesg[NextLetter + 1] != 0) && (OrigMesg[NextLetter + 1] != 13) && (OrigMesg[NextLetter + 1] !=' ') && (OrigMesg[NextLetter + 1] != '&') && (UnitChar < 7))
							{
							NextLetter ++;
							Units[UnitChar ++] = OrigMesg[NextLetter];
							} // while
						Units[UnitChar] = 0;
						UnitID = MatchUnitSuffix(Units);
						if (UnitID < 0)
							UnitID = WCS_USEFUL_UNIT_METER;
						} // if
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].GetCurValue();
					if (UnitID != WCS_USEFUL_UNIT_METER)
						DVal = ConvertFromMeters(DVal, UnitID);
					sprintf(TempStr, "%.*f%s", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'), GetUnitSuffix(UnitID));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &TZ
				case 'D':	// distance
					{
					UnitID = WCS_USEFUL_UNIT_METER;
					// look for unit suffix
					if (PrecisionFound)
						{
						UnitChar = 0;
						while ((OrigMesg[NextLetter + 1] != 0) && (OrigMesg[NextLetter + 1] != 13) && (OrigMesg[NextLetter + 1] !=' ') && (OrigMesg[NextLetter + 1] != '&') && (UnitChar < 7))
							{
							NextLetter ++;
							Units[UnitChar ++] = OrigMesg[NextLetter];
							} // while
						Units[UnitChar] = 0;
						UnitID = MatchUnitSuffix(Units);
						if (UnitID < 0)
							UnitID = WCS_USEFUL_UNIT_METER;
						} // if
					TargDist = 0;
					if(Cam->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED)
						{
						Begin.Lat = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetCurValue();
						Begin.Lon = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetCurValue();
						Begin.Elev = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetCurValue();
						End.Lat = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetCurValue();
						End.Lon = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].GetCurValue();
						End.Elev = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].GetCurValue();
						#ifdef WCS_BUILD_VNS
						DefCoords->DegToCart(&Begin);
						DefCoords->DegToCart(&End);
						#else // WCS_BUILD_VNS
						Begin.DegToCart(PlanetRad);
						End.DegToCart(PlanetRad);
						#endif // WCS_BUILD_VNS
						TargDist = PointDistance(Begin.XYZ, End.XYZ);
						} // if
					DVal = TargDist;
					if (UnitID != WCS_USEFUL_UNIT_METER)
						DVal = ConvertFromMeters(DVal, UnitID);
					sprintf(TempStr, "%.*f%s", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'), GetUnitSuffix(UnitID));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &TD
				default:
					break;
				} // switch
			LastSegStart = LetterCt + 1;
			} // if
		} // else if &T
	else if (! strncmp(&OrigMesg[LetterCt], "&U", 2))
		{
		NextLetter = LetterCt + 2;
		TestLetter = OrigMesg[NextLetter];
		if (TestLetter == 'N' || TestLetter == 'E')
			{
			strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
			TempStr[LetterCt - LastSegStart] = 0;
			strcat(MesgCopy, TempStr);
			switch (TestLetter)
				{
				case 'N':	// Name
					{
					strcat(MesgCopy, ProjectBase->UserName);
					LetterCt = NextLetter;
					break;
					} // &UN
				case 'E':	// EMail
					{
					strcat(MesgCopy, ProjectBase->UserEmail);
					LetterCt = NextLetter;
					break;
					} // &UE
				default:
					break;
				} // switch
			LastSegStart = LetterCt + 1;
			} // if
		} // else if &U
	else if (! strncmp(&OrigMesg[LetterCt], "&V", 2))
		{
		NextLetter = LetterCt + 2;
		TestLetter = OrigMesg[NextLetter];
		if ((TestLetter == 'X') || (TestLetter == 'Y')
			|| (TestLetter == 'Z') || (TestLetter == 'D') 
			|| (TestLetter == 'N') || (TestLetter == 'L'))
			{
			strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
			TempStr[LetterCt - LastSegStart] = 0;
			strcat(MesgCopy, TempStr);
			if (OrigMesg[NextLetter + 1] == '%')
				{
				NextLetter ++;
				if ((OrigMesg[NextLetter + 1] >= '0') && (OrigMesg[NextLetter + 1] <= '9'))
					{
					NextLetter ++;
					DefaultDigits = OrigMesg[NextLetter];
					PrecisionFound = 1;
					} // if
				} // if
			switch (TestLetter)
				{
				case 'X':	// longitude
					{
					DVal = CurVert->Lon;
					if (GlobalApp->MainProj->Prefs.PosLonHemisphere == WCS_PROJPREFS_LONCONVENTION_POSEAST)
						DVal = -DVal;
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &VX
				case 'Y':	// latitude
					{
					DVal = CurVert->Lat;
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &VY
				case 'Z':	// elevation
					{
					UnitID = WCS_USEFUL_UNIT_METER;
					// look for unit suffix
					if (PrecisionFound)
						{
						UnitChar = 0;
						while ((OrigMesg[NextLetter + 1] != 0) && (OrigMesg[NextLetter + 1] != 13) && (OrigMesg[NextLetter + 1] !=' ') && (OrigMesg[NextLetter + 1] != '&') && (UnitChar < 7))
							{
							NextLetter ++;
							Units[UnitChar ++] = OrigMesg[NextLetter];
							} // while
						Units[UnitChar] = 0;
						UnitID = MatchUnitSuffix(Units);
						if (UnitID < 0)
							UnitID = WCS_USEFUL_UNIT_METER;
						} // if
					DVal = CurVert->Elev;
					if (UnitID != WCS_USEFUL_UNIT_METER)
						DVal = ConvertFromMeters(DVal, UnitID);
					sprintf(TempStr, "%.*f%s", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'), GetUnitSuffix(UnitID));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &VZ
				case 'D':	// distance
					{
					UnitID = WCS_USEFUL_UNIT_METER;
					// look for unit suffix
					if (PrecisionFound)
						{
						UnitChar = 0;
						while ((OrigMesg[NextLetter + 1] != 0) && (OrigMesg[NextLetter + 1] != 13) && (OrigMesg[NextLetter + 1] !=' ') && (OrigMesg[NextLetter + 1] != '&') && (UnitChar < 7))
							{
							NextLetter ++;
							Units[UnitChar ++] = OrigMesg[NextLetter];
							} // while
						Units[UnitChar] = 0;
						UnitID = MatchUnitSuffix(Units);
						if (UnitID < 0)
							UnitID = WCS_USEFUL_UNIT_METER;
						} // if
					TargDist = 0;
					Begin.Lat = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetCurValue();
					Begin.Lon = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetCurValue();
					Begin.Elev = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetCurValue();
					End.Lat = CurVert->Lat;
					End.Lon = CurVert->Lon;
					End.Elev = CurVert->Elev;
					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&Begin);
					DefCoords->DegToCart(&End);
					#else // WCS_BUILD_VNS
					Begin.DegToCart(PlanetRad);
					End.DegToCart(PlanetRad);
					#endif // WCS_BUILD_VNS
					TargDist = PointDistance(Begin.XYZ, End.XYZ);
					DVal = TargDist;
					if (UnitID != WCS_USEFUL_UNIT_METER)
						DVal = ConvertFromMeters(DVal, UnitID);
					sprintf(TempStr, "%.*f%s", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'), GetUnitSuffix(UnitID));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &VD
				case 'N':	// Name
					{
					strcat(MesgCopy, CurVec->FileName() ? CurVec->FileName(): CurVec->GetBestName());
					LetterCt = NextLetter;
					break;
					} // &VN
				case 'L':	// Label
					{
					strcat(MesgCopy, CurVec->Name() ? CurVec->Name(): CurVec->GetBestName());
					LetterCt = NextLetter;
					break;
					} // &VL
				default:
					break;
				} // switch
			LastSegStart = LetterCt + 1;
			} // if
		} // else if &V
	else if (! strncmp(&OrigMesg[LetterCt], "&G", 2))
		{
		NextLetter = LetterCt + 2;
		TestLetter = OrigMesg[NextLetter];
		if ((TestLetter == 'Z'))
			{
			strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
			TempStr[LetterCt - LastSegStart] = 0;
			strcat(MesgCopy, TempStr);
			if (OrigMesg[NextLetter + 1] == '%')
				{
				NextLetter ++;
				if ((OrigMesg[NextLetter + 1] >= '0') && (OrigMesg[NextLetter + 1] <= '9'))
					{
					NextLetter ++;
					DefaultDigits = OrigMesg[NextLetter];
					PrecisionFound = 1;
					} // if
				} // if
			switch (TestLetter)
				{
				case 'Z':	// ground elevation
					{
					unsigned long Flags;
					VertexData VertData;
					PolygonData Poly;

					UnitID = WCS_USEFUL_UNIT_METER;
					// look for unit suffix
					if (PrecisionFound)
						{
						UnitChar = 0;
						while ((OrigMesg[NextLetter + 1] != 0) && (OrigMesg[NextLetter + 1] != 13) && (OrigMesg[NextLetter + 1] !=' ') && (OrigMesg[NextLetter + 1] != '&') && (UnitChar < 7))
							{
							NextLetter ++;
							Units[UnitChar ++] = OrigMesg[NextLetter];
							} // while
						Units[UnitChar] = 0;
						UnitID = MatchUnitSuffix(Units);
						if (UnitID < 0)
							UnitID = WCS_USEFUL_UNIT_METER;
						} // if
					VertData.Lat = CurVert->Lat;
					VertData.Lon = CurVert->Lat;
					VertData.Elev = CurVert->Elev;
					Flags = (WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED |
						WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED | 
						WCS_VERTEXDATA_FLAG_WATERELEV | WCS_VERTEXDATA_FLAG_WATERDEPTH | WCS_VERTEXDATA_FLAG_LAKEAPPLIED |
						WCS_VERTEXDATA_FLAG_STREAMAPPLIED | WCS_VERTEXDATA_FLAG_WAVEHEIGHT);
					ProjectBase->Interactive->VertexDataPoint(&RendData, &VertData, &Poly, Flags);
					DVal = VertData.Elev;
					if (UnitID != WCS_USEFUL_UNIT_METER)
						DVal = ConvertFromMeters(DVal, UnitID);
					sprintf(TempStr, "%.*f%s", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'), GetUnitSuffix(UnitID));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &GZ
				default:
					break;
				} // switch
			LastSegStart = LetterCt + 1;
			} // if
		} // else if &G
	} // for

} // Renderer::ReplaceStringTokens

/*===========================================================================*/

int Renderer::Render3DObject(Object3DVertexList *ListElement)
{
VertexDEM ObjVert;
PolygonData Poly;
Object3DEffect *Object3D;

if (Object3D = ListElement->Obj)
	{
	Object3D->FindBasePosition(&RendData, &ObjVert, &Poly, ListElement->Vec, ListElement->Point);
	Cam->ProjectVertexDEM(DefCoords, &ObjVert, EarthLatScaleMeters, PlanetRad, 1);
	if (! Render3DObject(&Poly, &ObjVert, Object3D, NULL, -1.0))
		return (0);
	} // if

return (1);

} // Renderer::Render3DObject

/*===========================================================================*/

int Renderer::Render3DObject(PolygonData *Poly, VertexDEM *BaseVtx, Object3DEffect *Object3D, 
	double ExtraRotation[3], double ScaleHeight)
{
double MinX, MinY, MaxX, MaxY;
double *NormalStash = NULL;
unsigned long int StashCurSteps, StashMaxSteps;
long Ct, VertCt, VisiblePolys, poly, p0, p1, p2, BusyWinBaseCt, aCt, bCt, cCt, dCt, qCt, NVerts, Obj3DNumVerts;
short Success = 1, ReverseNormals, RenderPass, MaxPasses, CreateRenderRast, FragmentPerObject, FragmentPerPass;
Vertex3D *Vtx[3];
struct RenderPolygon3D *PolySort;
Raster *RenderRast = NULL;
BusyWin *BWDE = NULL;
char StashText[202], NameStr[WCS_EFFECT_MAXNAMELENGTH + 24];

CreateRenderRast = (! rPixelFragMap || Object3D->FragmentOptimize != WCS_EFFECTS_OBJECT3D_OPTIMIZE_PERPIXEL);
FragmentPerObject = (CreateRenderRast && (! rPixelFragMap || Object3D->FragmentOptimize == WCS_EFFECTS_OBJECT3D_OPTIMIZE_PEROBJECT));
FragmentPerPass = (CreateRenderRast && ! FragmentPerObject && (! rPixelFragMap || Object3D->FragmentOptimize == WCS_EFFECTS_OBJECT3D_OPTIMIZE_PERPASS));

RendData.TexData.VDataVecOffsetsValid = 0;

if (! IsCamView && Master)
	{
	Master->GetProcSetup(StashCurSteps, StashMaxSteps, StashText);
	} // if

if ((Object3D->Vertices && Object3D->Polygons && Object3D->NameTable) || Object3D->OpenInputFile(NULL, FALSE, FALSE, FALSE))
	{
	Obj3DNumVerts = Object3D->NumVertices;
	if (! CreateRenderRast || (RenderRast = new Raster))
		{
		sprintf(NameStr, "%s%s", Object3D->Name, ShadowMapInProgress ? " Shadow": "");
		MaxPasses = ShadowMapInProgress ? 1: Object3D->AAPasses;
		Poly->RenderPassWeight = 1.0 / MaxPasses;
		if (IsCamView)
			{
			BWDE = new BusyWin(NameStr, MaxPasses * Object3D->NumPolys, 'BWDE', 0);
			} // if
		else if (Master)
			{
			Master->ProcInit(MaxPasses * Object3D->NumPolys, NameStr);
			} // else

		if (Object3D->Transform(NULL, 0, &RendData, Poly, BaseVtx, ExtraRotation, ScaleHeight))
			{
			// transform surface normals if they come from a file
			if (! Object3D->CalcNormals)
				{
				// create an array, stash original normals
				if (NormalStash = (double *)AppMem_Alloc(Obj3DNumVerts * 3 * sizeof (double), 0))
					{
					for (Ct = VertCt = 0; Ct < Obj3DNumVerts; Ct ++)
						{
						NormalStash[VertCt ++] = Object3D->Vertices[Ct].Normal[0];
						NormalStash[VertCt ++] = Object3D->Vertices[Ct].Normal[1];
						NormalStash[VertCt ++] = Object3D->Vertices[Ct].Normal[2];
						} // for
					// transform surface normals
					Object3D->TransformNormals();
					} // if
				} // if

			// project all the object vertices
// F2_NOTE: This loop sped up about 50% on Falcon with OMP enabled.  However, my random rotations of my 3D objects came out
// differently than that of the non-OMP build
//#pragma omp parallel for private(Ct) shared(Obj3DNumVerts)
			for (Ct = 0; Ct < Obj3DNumVerts; ++Ct)
				{
				#ifdef WCS_BUILD_VNS
				DefCoords->CartToDeg(&Object3D->Vertices[Ct]);
				#else // WCS_BUILD_VNS
				Object3D->Vertices[Ct].CartToDeg(PlanetRad);
				#endif // WCS_BUILD_VNS
				Cam->ProjectVertexDEM(DefCoords, &Object3D->Vertices[Ct], EarthLatScaleMeters, PlanetRad, 0);
				} // for

			// create an array of simple structures to use for depth sorting
			if (PolySort = (struct RenderPolygon3D *)AppMem_Alloc(Object3D->NumPolys * sizeof (struct RenderPolygon3D), 0))
				{
				// cull out-of-image polygons
				MinX = MinY = FLT_MAX;
				MaxX = MaxY = -FLT_MAX;

				for (Ct = 0, VisiblePolys = 0; Ct < Object3D->NumPolys; Ct ++)
					{
					//if it's invisible skip it
					if (Object3D->NameTable[Object3D->Polygons[Ct].Material].Mat->Shading == WCS_EFFECT_MATERIAL_SHADING_INVISIBLE ||
						! Object3D->NameTable[Object3D->Polygons[Ct].Material].Mat->Enabled)
						continue;
					PolySort[VisiblePolys].Poly = &Object3D->Polygons[Ct];
					PolySort[VisiblePolys].qqq = 0.0;
					aCt = bCt = cCt = dCt = qCt = 0;
					NVerts = Object3D->Polygons[Ct].NumVerts;
					for (p0 = 0; p0 < NVerts; p0 ++)
						{
						if (Object3D->Vertices[Object3D->Polygons[Ct].VertRef[p0]].ScrnXYZ[2] <= 0.0)
							qCt ++;
						PolySort[VisiblePolys].qqq += Object3D->Vertices[Object3D->Polygons[Ct].VertRef[p0]].ScrnXYZ[2];
						if (Object3D->Vertices[Object3D->Polygons[Ct].VertRef[p0]].ScrnXYZ[0] < 0)
							aCt ++;
						if (Object3D->Vertices[Object3D->Polygons[Ct].VertRef[p0]].ScrnXYZ[0] >= Width)
							bCt ++;
						if (Object3D->Vertices[Object3D->Polygons[Ct].VertRef[p0]].ScrnXYZ[1] < 0)
							cCt ++;
						if (Object3D->Vertices[Object3D->Polygons[Ct].VertRef[p0]].ScrnXYZ[1] >= Height)
							dCt ++;
						} // for
					if (aCt >= NVerts || bCt >= NVerts || cCt >= NVerts || dCt >= NVerts || qCt >= NVerts)
						continue;
					// normalize the z value
					PolySort[VisiblePolys].qqq /= NVerts;
					// check for bounds revision
					for (p0 = 0; p0 < NVerts; p0 ++)
						{
						if (Object3D->Vertices[Object3D->Polygons[Ct].VertRef[p0]].ScrnXYZ[0] < MinX)
							MinX = Object3D->Vertices[Object3D->Polygons[Ct].VertRef[p0]].ScrnXYZ[0];
						if (Object3D->Vertices[Object3D->Polygons[Ct].VertRef[p0]].ScrnXYZ[0] > MaxX)
							MaxX = Object3D->Vertices[Object3D->Polygons[Ct].VertRef[p0]].ScrnXYZ[0];
						if (Object3D->Vertices[Object3D->Polygons[Ct].VertRef[p0]].ScrnXYZ[1] < MinY)
							MinY = Object3D->Vertices[Object3D->Polygons[Ct].VertRef[p0]].ScrnXYZ[1];
						if (Object3D->Vertices[Object3D->Polygons[Ct].VertRef[p0]].ScrnXYZ[1] > MaxY)
							MaxY = Object3D->Vertices[Object3D->Polygons[Ct].VertRef[p0]].ScrnXYZ[1];
						} // for
					VisiblePolys ++;
					} // for

				// now we know we if have some polygons to render
				if (VisiblePolys > 0)
					{
					MinX = floor(max(MinX, 0.0) - .5);
					MinY = floor(max(MinY, 0.0) - .5);
					MaxX = ceil(min(MaxX, Width - 1) + .5);
					MaxY = ceil(min(MaxY, Height - 1) + .5);
					if (RenderRast)
						{
						RenderRast->Cols = quickftol(MaxX - MinX);
						RenderRast->Rows = quickftol(MaxY - MinY);
						} // if
					if (! RenderRast || (RenderRast->AllocRender3DBands() && RenderRast->AllocPixelFragmentMap()))
						{
						qsort((void *)PolySort, (size_t)VisiblePolys, (size_t)(sizeof (struct RenderPolygon3D)), CompareRenderPolygon3D);
						// set each polygon un-normalized
						for (Ct = 0; Ct < VisiblePolys; Ct ++)
							{
							PolySort[Ct].Poly->Normalized = 0;
							} // for
						// one-time polygon initializations
						Poly->BaseMat.Covg[0] = 1.0;
						Poly->ShadowFlags = Object3D->ShadowFlags;
						Poly->ReceivedShadowIntensity = Object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SHADOWINTENS].CurValue;
						Poly->ShadowOffset = Object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SHADOWOFFSET].CurValue;
						Poly->Plot3DRast = RenderRast;
						Poly->PlotOffset3DX = quickftol(MinX);
						Poly->PlotOffset3DY = quickftol(MinY);

						// for however many antialias passes or until user cancels
						for (RenderPass = 0; Success && RenderPass < MaxPasses; RenderPass ++)
							{
							BusyWinBaseCt = RenderPass * Object3D->NumPolys + Object3D->NumPolys - VisiblePolys + 1;
							for (Ct = 0; Ct < VisiblePolys; Ct ++)
								{
								ReverseNormals = 0;
								Poly->BaseMat.Mat[0] = Object3D->NameTable[PolySort[Ct].Poly->Material].Mat;
								
								if (! ShadowMapInProgress)
									{
									// shade type depends on whether or not using pre-computed surface normals
									Poly->ShadeType = Object3D->CalcNormals ? (char)Poly->BaseMat.Mat[0]->Shading: WCS_EFFECT_MATERIAL_SHADING_PHONG;

									// compute surface and vertex normals
									if (RenderPass == 0)
										{
										PolySort[Ct].Poly->Normalize(Object3D->Vertices);
										if (Poly->BaseMat.Mat[0]->FlipNormal)
											NegateVector(PolySort[Ct].Poly->Normal);
										} // if
									// copy normals to PolygonData
									Poly->Normal[0] = PolySort[Ct].Poly->Normal[0];
									Poly->Normal[1] = PolySort[Ct].Poly->Normal[1];
									Poly->Normal[2] = PolySort[Ct].Poly->Normal[2];
									if (Poly->ShadeType == WCS_EFFECT_MATERIAL_SHADING_PHONG && Object3D->CalcNormals)
										{
										for (p0 = 0; p0 < PolySort[Ct].Poly->NumVerts; p0 ++)
											{
											Object3D->Vertices[PolySort[Ct].Poly->VertRef[p0]].Normalized = 0;
											Object3D->Vertices[PolySort[Ct].Poly->VertRef[p0]].Normalize(Object3D->Polygons, Object3D->Vertices, PolySort[Ct].Poly, Object3D->NameTable);
											} // for
										} // if phong
									// test for backside viewing and reverse normals if necessary
									ZeroPoint3d(Poly->ViewVec);
									// find view vector
									for (p0 = 0; p0 < PolySort[Ct].Poly->NumVerts; p0 ++)
										{
										AddPoint3d(Poly->ViewVec, Object3D->Vertices[PolySort[Ct].Poly->VertRef[p0]].XYZ);
										} // for
									Poly->ViewVec[0] -= (PolySort[Ct].Poly->NumVerts * Cam->CamPos->XYZ[0]); 
									Poly->ViewVec[1] -= (PolySort[Ct].Poly->NumVerts * Cam->CamPos->XYZ[1]); 
									Poly->ViewVec[2] -= (PolySort[Ct].Poly->NumVerts * Cam->CamPos->XYZ[2]); 
									if (ReverseNormals = SurfaceVisible(Poly->Normal, Poly->ViewVec, 0))	// TRUE if surface faces away
										{
										if (Poly->BaseMat.Mat[0]->DoubleSided)
											{
											NegateVector(Poly->Normal);
											if (Poly->ShadeType == WCS_EFFECT_MATERIAL_SHADING_PHONG)
												{
												for (p0 = 0; p0 < PolySort[Ct].Poly->NumVerts; p0 ++)
													{
													NegateVector(Object3D->Vertices[PolySort[Ct].Poly->VertRef[p0]].Normal);
													} // for
												} // if 
											} // if double-sided
										else
											continue;
										} // if reverse normals
									} // if ! shadow mapping

								p0 = 0;
								p1 = 1;
								p2 = PolySort[Ct].Poly->NumVerts - 1;
								poly = 0;
								while (p2 > p1)
									{
									// assign vertex pointers
									Vtx[0] = &Object3D->Vertices[PolySort[Ct].Poly->VertRef[p0]];
									Vtx[1] = &Object3D->Vertices[PolySort[Ct].Poly->VertRef[p1]];
									Vtx[2] = &Object3D->Vertices[PolySort[Ct].Poly->VertRef[p2]];
									// apply antialias pass offset to screen coords
									Vtx[0]->ScrnXYZ[0] += DefaultAAKernX[RenderPass];
									Vtx[1]->ScrnXYZ[0] += DefaultAAKernX[RenderPass];
									Vtx[2]->ScrnXYZ[0] += DefaultAAKernX[RenderPass];
									Vtx[0]->ScrnXYZ[1] += DefaultAAKernY[RenderPass];
									Vtx[1]->ScrnXYZ[1] += DefaultAAKernY[RenderPass];
									Vtx[2]->ScrnXYZ[1] += DefaultAAKernY[RenderPass];
									Poly->VtxNum[0] = PolySort[Ct].Poly->VertRef[p0];
									Poly->VtxNum[1] = PolySort[Ct].Poly->VertRef[p1];
									Poly->VtxNum[2] = PolySort[Ct].Poly->VertRef[p2];
									Poly->VertRefData[0] = PolySort[Ct].Poly->FindVertRefDataHead(Poly->VtxNum[0]);
									Poly->VertRefData[1] = PolySort[Ct].Poly->FindVertRefDataHead(Poly->VtxNum[1]);
									Poly->VertRefData[2] = PolySort[Ct].Poly->FindVertRefDataHead(Poly->VtxNum[2]);

									RenderPolygon(Poly, (VertexBase **)Vtx, WCS_POLYGONTYPE_3DOBJECT);

									// remove antialias pass offset from screen coords
									Vtx[0]->ScrnXYZ[0] -= DefaultAAKernX[RenderPass];
									Vtx[1]->ScrnXYZ[0] -= DefaultAAKernX[RenderPass];
									Vtx[2]->ScrnXYZ[0] -= DefaultAAKernX[RenderPass];
									Vtx[0]->ScrnXYZ[1] -= DefaultAAKernY[RenderPass];
									Vtx[1]->ScrnXYZ[1] -= DefaultAAKernY[RenderPass];
									Vtx[2]->ScrnXYZ[1] -= DefaultAAKernY[RenderPass];

									if (poly %2)
										{
										p0 = p2;
										p2 --;
										} // if
									else
										{
										p0 = p1;
										p1 ++;
										} // else
									poly ++;
									} // while p2 > p1
								// put normals back to original orientation
								if (ReverseNormals && Poly->ShadeType == WCS_EFFECT_MATERIAL_SHADING_PHONG)
									{
									for (p0 = 0; p0 < PolySort[Ct].Poly->NumVerts; p0 ++)
										{
										NegateVector(Object3D->Vertices[PolySort[Ct].Poly->VertRef[p0]].Normal);
										} // for
									} // if ReverseNormals
								if (IsCamView)
									{
									if (BWDE)
										{
										if(BWDE->Update(Ct + BusyWinBaseCt))
											{
											Success = 0;
											RenderPass = MaxPasses - 1;
											break;
											} // if
										} // if
									} // if
								else
									{
									Master->ProcUpdate(Ct + BusyWinBaseCt);
									if (! Master->IsRunning())
										{
										Success = 0;
										RenderPass = MaxPasses - 1;
										break;
										} // if
									} // else
								} // for each polygon
							if (RenderRast)
								{
								if (FragmentPerPass)
									Composite3DObjectPass(Object3D, RenderRast, Poly->RenderPassWeight, MinX, MinY);
								else
									RenderRast->CopyRenderMapMerge(1.0 / (RenderPass + 1.0));
								RenderRast->ClearPixelFragMap();
								} // if
							} // for each render pass
						if (RenderRast && FragmentPerObject)
							Composite3DObject(Object3D, RenderRast, MinX, MinY);
						} // if rasters initialized
					} // if at least one polygon in the image
				AppMem_Free(PolySort, Object3D->NumPolys * sizeof (struct RenderPolygon3D));
				} // if PolySort memory allocated
			// restore surface normals if they come from the file, this object may be reused without reloading
			if (! Object3D->CalcNormals && NormalStash)
				{
				for (Ct = VertCt = 0; Ct < Obj3DNumVerts; Ct ++)
					{
					Object3D->Vertices[Ct].Normal[0] = NormalStash[VertCt ++];
					Object3D->Vertices[Ct].Normal[1] = NormalStash[VertCt ++];
					Object3D->Vertices[Ct].Normal[2] = NormalStash[VertCt ++];
					} // for
				AppMem_Free(NormalStash, Obj3DNumVerts * 3 * sizeof (double));
				} // if
			} // if Transform - returns 0 if no vertices
		if (BWDE)
			delete BWDE;
		} // if Rasters created
	if (RenderRast)
		delete RenderRast;
	} // if object loaded

if (! IsCamView && Master)
	{
	if (strlen(StashText) > 0)  //lint !e645
		StashText[strlen(StashText) - 1] = 0;
	else
		StashText[0] = StashText[1] = 0;
	Master->RestoreProcSetup(StashCurSteps, StashMaxSteps, &StashText[1]);
	} // if

return (Success);

} // Renderer::Render3DObject

/*===========================================================================*/

int Renderer::Composite3DObjectPass(Object3DEffect *Object, Raster *SourceRast, double PassWt, double DxStart, double DyStart)
{
double dSourceNormal[3], SourceColor[3];
rPixelFragment *PixFrag;
long DpZip, SpZip, PixWt, X, Y, i, j;
float SourceZ, SourceReflect, SourceNormal[3];
unsigned char Flags, SourceCoverage;

// D stands for destination, S is for Source, p is for pixel, o is for origin, p1 is for +1, Int is for interval, Covg is for coverage

if (SourceRast->Cols <= 0 || SourceRast->Rows <= 0 || ! rPixelFragMap || ! SourceRast->rPixelBlock || ! SourceRast->rPixelBlock->FragMap)
	return (0);

dSourceNormal[0] = dSourceNormal[1] = dSourceNormal[2] = 0.0;
Flags = (Object->RenderOccluded 
	#ifdef WCS_BUILD_RTX
	|| Exporter
	#endif // WCS_BUILD_RTX
	) ? WCS_PIXELFRAG_FLAGBIT_RENDEROCCLUDED: 0;
Flags |= WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_3DOBJECT;

for (Y = quickftol(DyStart), j = 0; j < SourceRast->Rows; j ++, Y ++)
	{
	if (Y < 0)
		continue;
	if (Y >= Height)
		break;
	DpZip = Y * Width + quickftol(DxStart);
	SpZip = j * SourceRast->Cols;
	for (X = quickftol(DxStart), i = 0; i < SourceRast->Cols; i ++, X ++, DpZip ++, SpZip ++)
		{
		if (X < 0)
			continue;
		if (X >= Width)
			break;
		SourceRast->rPixelBlock->FragMap[SpZip].CollapsePixel(SourceColor, SourceZ, SourceCoverage,
			SourceReflect, SourceNormal);

		if (SourceCoverage > 0)
			{
			PixWt = (int)WCS_ceil(SourceCoverage * PassWt);
			if (PixWt > 255)
				PixWt = 255;
			if (PixFrag = rPixelFragMap[DpZip].PlotPixel(rPixelBlock, SourceZ, (unsigned char)PixWt, ~0UL, ~0UL, FragmentDepth, 
				Flags))
				{
				dSourceNormal[0] = SourceNormal[0];
				dSourceNormal[1] = SourceNormal[1];
				dSourceNormal[2] = SourceNormal[2];
				PixFrag->PlotPixel(rPixelBlock, SourceColor, (double)SourceReflect, dSourceNormal);
				ScreenPixelPlotFragments(&rPixelFragMap[DpZip], X + DrawOffsetX, Y + DrawOffsetY);
				} // if
			} // if
		} // for
	} // for

return (1);

} // Renderer::Composite3DObjectPass

/*===========================================================================*/

void Renderer::Composite3DObject(Object3DEffect *Object, Raster *SourceRast, double DxStart, double DyStart)
{
double PixWt, NewWt, TestMergeOffset;
float ZSum;
long X, Y, PixZip, SourceZip, i, j;
unsigned short OldBits, NewBits, PolyBits, TotalBits, RemBits, DifBits;
unsigned long TopPixCovg, BotPixCovg;
rPixelFragment *PixFrag;
double PixColor[3], PixNormal[3];
unsigned char Flags;
unsigned long TempRed, TempGreen, TempBlue;
unsigned short TempExp;

TopPixCovg = BotPixCovg = (unsigned long)~0;
PixNormal[0] = PixNormal[1] = PixNormal[2] = 0.0;
Flags = (Object->RenderOccluded 
	#ifdef WCS_BUILD_RTX
	|| Exporter
	#endif // WCS_BUILD_RTX
	) ? WCS_PIXELFRAG_FLAGBIT_RENDEROCCLUDED: 0;
Flags |= WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_3DOBJECT;

TestMergeOffset = DefaultEnvironment->FoliageBlending;
TestMergeOffset *= ZMergeDistance;

for (Y = quickftol(DyStart), j = 0; j < SourceRast->Rows; j ++, Y ++)
	{
	if (Y < 0)
		continue;
	if (Y >= Height)
		break;
	PixZip = Y * Width + quickftol(DxStart);
	SourceZip = j * SourceRast->Cols;
	for (X = quickftol(DxStart), i = 0; i < SourceRast->Cols; i ++, X ++, PixZip ++, SourceZip ++)
		{
		if (X < 0)
			continue;
		if (X >= Width)
			break;
		if ((PixWt = SourceRast->AABuf[SourceZip]) > 0)
			{
			OldBits = TreeAABuf[PixZip];
			PolyBits = AABuf[PixZip];
			NewBits = (unsigned short)(PixWt);
			ZSum = SourceRast->ZBuf[SourceZip];
			if (NewBits)
				{
				if (! rPixelFragMap)
					{
					TempRed = (SourceRast->ExponentBuf[SourceZip] >> 10) ? 255: SourceRast->Red[SourceZip];
					TempGreen = ((SourceRast->ExponentBuf[SourceZip] & 992) >> 5) ? 255: SourceRast->Green[SourceZip];
					TempBlue = (SourceRast->ExponentBuf[SourceZip] & 31) ? 255: SourceRast->Blue[SourceZip];

					if (ZSum > TreeZBuf[PixZip])					// new farther than old
						{
						if (ZSum - TreeZBuf[PixZip] < TestMergeOffset && NewBits + OldBits > 255)
							{
							NewWt = 1.0 - (.5 * (1.0 + (ZSum - TreeZBuf[PixZip]) / TestMergeOffset));
							RemBits = 255 - OldBits;
							DifBits = NewBits - RemBits;
							NewBits = (unsigned short)(RemBits + DifBits * NewWt);
							OldBits = (unsigned short)(OldBits - DifBits * NewWt);
							if (! NewBits)
								continue;
							} // if
						else if (OldBits >= 255 || (ZSum > ZBuf[PixZip] && PolyBits >= 255))
							continue;
						if (NewBits + OldBits > 255)
							NewBits = 255 - OldBits;
						TotalBits = NewBits + OldBits;
						TreeBitmap[0][PixZip] = (TreeBitmap[0][PixZip] * OldBits + (unsigned char)TempRed * NewBits) / TotalBits;
						TreeBitmap[1][PixZip] = (TreeBitmap[1][PixZip] * OldBits + (unsigned char)TempGreen * NewBits) / TotalBits;
						TreeBitmap[2][PixZip] = (TreeBitmap[2][PixZip] * OldBits + (unsigned char)TempBlue * NewBits) / TotalBits;
						TreeAABuf[PixZip] = (unsigned char)TotalBits;
						} // if ZSum
					else										// new closer than old
						{
						if (TreeZBuf[PixZip] - ZSum < TestMergeOffset && NewBits + OldBits > 255)
							{
							NewWt = .5 * (1.0 + (TreeZBuf[PixZip] - ZSum) / TestMergeOffset);
							RemBits = 255 - NewBits;
							DifBits = OldBits - RemBits;
							OldBits = (unsigned short)(RemBits + DifBits * NewWt);
							NewBits = (unsigned short)(NewBits - DifBits * NewWt);
							if (! NewBits)
								continue;
							} // if
						else if (ZSum > ZBuf[PixZip] && PolyBits >= 255)
							continue;
						if (OldBits)
							{
							if (NewBits + OldBits > 255)
								OldBits = 255 - NewBits;
							TotalBits = NewBits + OldBits;
							TreeBitmap[0][PixZip] = (TreeBitmap[0][PixZip] * OldBits + (unsigned char)TempRed * NewBits) / TotalBits;
							TreeBitmap[1][PixZip] = (TreeBitmap[1][PixZip] * OldBits + (unsigned char)TempGreen * NewBits) / TotalBits;
							TreeBitmap[2][PixZip] = (TreeBitmap[2][PixZip] * OldBits + (unsigned char)TempBlue * NewBits) / TotalBits;
							TreeAABuf[PixZip] = (unsigned char)TotalBits;
							} // if
						else
							{
							TreeBitmap[0][PixZip] = (unsigned char)TempRed;
							TreeBitmap[1][PixZip] = (unsigned char)TempGreen;
							TreeBitmap[2][PixZip] = (unsigned char)TempBlue;
							TreeAABuf[PixZip] = (unsigned char)NewBits;
							} // else
						TreeZBuf[PixZip] = (float)ZSum;
						if (! rPixelFragMap)
							ScreenPixelPlotTwoBuf(Bitmap, ZBuf, AABuf, TreeBitmap, TreeZBuf, TreeAABuf, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
						} // else ZSum
					} // if
				else
					{
					if (PixFrag = rPixelFragMap[PixZip].PlotPixel(rPixelBlock, (float)ZSum, (unsigned char)NewBits, TopPixCovg, BotPixCovg, FragmentDepth, 
						Flags))
						{
						TempRed = SourceRast->Red[SourceZip];
						TempExp = (SourceRast->ExponentBuf[SourceZip] >> 10);
						if (TempExp)
							TempRed <<= TempExp;

						TempGreen = SourceRast->Green[SourceZip];
						TempExp = ((SourceRast->ExponentBuf[SourceZip] & 992) >> 5);
						if (TempExp)
							TempGreen <<= TempExp;

						TempBlue = SourceRast->Blue[SourceZip];
						TempExp = (SourceRast->ExponentBuf[SourceZip] & 31);
						if (TempExp)
							TempBlue <<= TempExp;

						PixColor[0] = TempRed * (1.0 / 255.0);
						PixColor[1] = TempGreen * (1.0 / 255.0);
						PixColor[2] = TempBlue * (1.0 / 255.0);
						PixNormal[0] = SourceRast->NormalBuf[0][SourceZip];
						PixNormal[1] = SourceRast->NormalBuf[1][SourceZip];
						PixNormal[2] = SourceRast->NormalBuf[2][SourceZip];
						PixFrag->PlotPixel(rPixelBlock, PixColor, (double)SourceRast->ReflectionBuf[SourceZip], PixNormal);
						ScreenPixelPlotFragments(&rPixelFragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);
						} // if
					} // if
				} // if NewBits
			} // if PixWt
		} // for x
	} // for y

} // Renderer::Composite3DObject() 

/*===========================================================================*/
/*===========================================================================*/

int AnimMaterialGradient::GetRenderMaterial(TwinMaterials *MatTwin, double Pos)
{
double LastPos, Diff;
GradientCritter *CurGrad = Grad, *LastGrad = NULL;

MatTwin->Mat[0] = MatTwin->Mat[1] = NULL;
MatTwin->Covg[0] = MatTwin->Covg[1] = 0.0;

while (CurGrad)
	{
	if (CurGrad->Position.CurValue == Pos)
		{
		if (MatTwin->Mat[0] = (MaterialEffect *)CurGrad->GetThing())
			{
			MatTwin->Covg[0] = 1.0;
			return (1);
			} // if
		// returns 0 if the material is invalid
		return (0);
		} // if
	if (CurGrad->Position.CurValue > Pos)
		{
		if (LastGrad)
			{
			MatTwin->Mat[0] = (MaterialEffect *)LastGrad->GetThing();
			MatTwin->Mat[1] = (MaterialEffect *)CurGrad->GetThing();
			if (MatTwin->Mat[0] && MatTwin->Mat[1])
				{
				if ((Diff = CurGrad->Position.CurValue - LastPos) > 0.0)
					{
					switch (CurGrad->BlendStyle)
						{
						case WCS_GRADIENTCRITTER_BLENDSTYLE_SHARP:	// 0% overlap
							{
							LastPos = CurGrad->Position.CurValue;
							break;
							} // 
						case WCS_GRADIENTCRITTER_BLENDSTYLE_SOFT:	// 10% overlap
							{
							LastPos = LastPos + .9 * Diff;
							Diff *= .1;
							break;
							} // 
						case WCS_GRADIENTCRITTER_BLENDSTYLE_QUARTER:	// 25% overlap
							{
							LastPos = LastPos + .75 * Diff;
							Diff *= .25;
							break;
							} // 
						case WCS_GRADIENTCRITTER_BLENDSTYLE_HALF:	// 50% overlap
							{
							LastPos = LastPos + .5 * Diff;
							Diff *= .5;
							break;
							} // 
						case WCS_GRADIENTCRITTER_BLENDSTYLE_FULL:	// 100% overlap
						case WCS_GRADIENTCRITTER_BLENDSTYLE_FASTINCREASE:	// 100% overlap
						case WCS_GRADIENTCRITTER_BLENDSTYLE_SLOWINCREASE:	// 100% overlap
						case WCS_GRADIENTCRITTER_BLENDSTYLE_SCURVE:	// 100% overlap
							{
							// nothing to do
							break;
							} // 
						} // switch
					if (Pos > LastPos)
						{
						MatTwin->Covg[1] = (Pos - LastPos) / Diff;
						if (CurGrad->BlendStyle == WCS_GRADIENTCRITTER_BLENDSTYLE_SLOWINCREASE)
							{
							MatTwin->Covg[1] *= MatTwin->Covg[1];
							} // if
						else if (CurGrad->BlendStyle == WCS_GRADIENTCRITTER_BLENDSTYLE_FASTINCREASE)
							{
							MatTwin->Covg[1] = 1.0 - MatTwin->Covg[1];
							MatTwin->Covg[1] *= MatTwin->Covg[1];
							MatTwin->Covg[1] = 1.0 - MatTwin->Covg[1];
							} // if
						else if (CurGrad->BlendStyle == WCS_GRADIENTCRITTER_BLENDSTYLE_SCURVE)
							{
							MatTwin->Covg[1] *= MatTwin->Covg[1] * (3.0 - 2.0 * MatTwin->Covg[1]);	// same as PERLIN_s_curve()
							} // if
						MatTwin->Covg[0] = 1.0 - MatTwin->Covg[1];
						} // if
					else
						{
						MatTwin->Mat[1] = NULL;
						MatTwin->Covg[0] = 1.0;
						} // else
					} // if
				else
					{
					MatTwin->Mat[0] = MatTwin->Mat[1];
					MatTwin->Mat[1] = NULL;
					MatTwin->Covg[0] = 1.0;
					} // else
				return (1);
				} // if
			if (MatTwin->Mat[0])
				{
				MatTwin->Covg[0] = 1.0;
				return (1);
				} // if
			if (MatTwin->Mat[1])
				{
				MatTwin->Mat[0] = MatTwin->Mat[1];
				MatTwin->Mat[1] = NULL;
				MatTwin->Covg[0] = 1.0;
				return (1);
				} // if
			return (0);
			} // if
		else
			{
			if (MatTwin->Mat[0] = (MaterialEffect *)CurGrad->GetThing())
				{
				MatTwin->Covg[0] = 1.0;
				return (1);
				} // if
			return (0);
			} // else
		} // if
	LastGrad = CurGrad;
	LastPos = CurGrad->Position.CurValue;
	CurGrad = CurGrad->Next;
	} // if

// only found one critter
if (LastGrad)
	{
	if (MatTwin->Mat[0] = (MaterialEffect *)LastGrad->GetThing())
		{
		MatTwin->Covg[0] = 1.0;
		return (1);
		} // if
	} // if

// couldn't find a valid gradient critter
return (0);

} // AnimMaterialGradient::GetRenderMaterial

/*===========================================================================*/
/*===========================================================================*/

void MaterialEffect::Evaluate(PixelData *Pix, double MaterialWeight)
{
double TexOpacity, Value[3], BaseMatColor[3], SpecColor[3], TempVal, OrigWt, FoamWt, SpecColorWt, CriticalAmpMax, CriticalAmp, MaxElev, MinElev,
	SpecularWt = 0.0, SpecularWt2 = 0.0, TranslucencyWt = 0.0, StrataLineWt = 1.0, DiffuseIntensity = 1.0;

// fills in color, reflectivity, luminosity, speculariy, transparency, 
//  translucency, specular exponent, translucency exponent

OrigWt = MaterialWeight;
Value[2] = Value[1] = Value[0] = 0.0;

BaseMatColor[0] = DiffuseColor.GetCompleteValue(0);
BaseMatColor[1] = DiffuseColor.GetCompleteValue(1);
BaseMatColor[2] = DiffuseColor.GetCompleteValue(2);

// calculate transparency first and adjust MaterialWeight by material opacity
if (ApproveParam(WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY))
	{
	// transparency
	if ((TempVal = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue) > 0.0)
		{
		if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY))
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY]->Eval(Value, Pix->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					TempVal *= (1.0 - TexOpacity + Value[0]);
					} // if
				else
					TempVal *= Value[0];
				} // if
			} // if
		Pix->Transparency += TempVal * MaterialWeight;
		// this is what makes ecosystems and snow not cover ground completely if transparency is > 0
		if (Pix->PixelType == WCS_PIXELTYPE_TERRAIN)
			MaterialWeight *= (1.0 - TempVal);		// multiply material weight by opacity
		} // if

	} // if

if (ApproveParam(WCS_EFFECTS_MATERIAL_ANIMPAR_DIFFUSEINTENSITY))
	{
	// diffuse color intensity
	if ((DiffuseIntensity = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DIFFUSEINTENSITY].CurValue) > 0.0)
		{
		if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSEINTENSITY))
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSEINTENSITY]->Eval(Value, Pix->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					DiffuseIntensity *= (1.0 - TexOpacity + Value[0]);
					} // if
				else
					DiffuseIntensity *= Value[0];
				} // if
			} // if
		} // if
	} // if

// strata lines and colors
if (Strata && Strata->Enabled && ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_STRATA))
	{
	MaxElev = max(Pix->TexData->TElevRange[0], Pix->TexData->TElevRange[1]);
	MaxElev = max(MaxElev, Pix->TexData->TElevRange[2]);
	MaxElev = max(MaxElev, Pix->TexData->TElevRange[3]);
	MinElev = min(Pix->TexData->TElevRange[0], Pix->TexData->TElevRange[1]);
	MinElev = min(MinElev, Pix->TexData->TElevRange[2]);
	MinElev = min(MinElev, Pix->TexData->TElevRange[3]);
	if (Strata->ColorStrata)
		{
		// strata colors are always initialized in ComputeTextureColor()
		StrataLineWt = Strata->ComputeTextureColor(MaxElev - MinElev, BaseMatColor, Pix->TexData);
		// no strata lines so set this weighting factor to default
		if (! Strata->LinesEnabled)
			StrataLineWt = 1.0;
		} // if
	// no bump mapping so evaluate strata lines like in v4
	else if (Strata->LinesEnabled)
		{
		StrataLineWt = Strata->ComputeTexture(MaxElev - MinElev, Pix->TexData);
		} // else
	} // if

// foam
if (Foam && Foam->Enabled && ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_FOAM))
	{
	// find foam coverage
	if ((FoamWt = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_FOAMCOVERAGE].CurValue) > 0.0)
		{
		if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_FOAMCOVERAGE))
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_FOAMCOVERAGE]->Eval(Value, Pix->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					FoamWt *= (1.0 - TexOpacity + Value[0]);
					} // if
				else
					FoamWt *= Value[0];
				} // if
			} // if
		if (FoamWt > 0.0)
			{
			// based on wave height and wave height range for this material determine a foam cutoff
			if (WaveAmpRange > 0.0)
				{
				CriticalAmp = (sin(FoamWt * Pi + HalfPi) + 1.0) * .5;
				CriticalAmpMax = CriticalAmp * WaveAmpRange;
				CriticalAmp = (CriticalAmp - .1) * WaveAmpRange;
				CriticalAmpMax += MinWaveAmp;
				CriticalAmp += MinWaveAmp;
				if (Pix->TexData->WaveHeight > CriticalAmp)
					{
					if (Pix->TexData->WaveHeight >= CriticalAmpMax)
						{
						FoamWt = 1.0;
						} // if
					else
						{
						FoamWt = (Pix->TexData->WaveHeight - CriticalAmp) / (CriticalAmpMax - CriticalAmp);
						} // else
					} // if
				else
					{
					FoamWt = 0.0;
					} // else
				} // if
			if (FoamWt >0.0)
				{
				Foam->Evaluate(Pix, FoamWt * OrigWt);
				MaterialWeight -= (FoamWt * OrigWt);
				} // if
			} // if
		} // if
	} // if

// diffuse color
// evaluate thematic map, this will override strata colors
if (GetEnabledTheme(WCS_EFFECTS_MATERIAL_THEME_DIFFUSECOLOR) &&
	Theme[WCS_EFFECTS_MATERIAL_THEME_DIFFUSECOLOR]->Eval(Value, Pix->TexData->Vec))
	{
	BaseMatColor[0] = Value[0];
	BaseMatColor[1] = Value[1];
	BaseMatColor[2] = Value[2];
	// colors will be in range 0-255 hopefully so we need to reduce to 0-1 and elliminate negatives
	if (BaseMatColor[0] <= 0.0)
		BaseMatColor[0] = 0.0;
	else
		BaseMatColor[0] *= (1.0 / 255.0);
	if (BaseMatColor[1] <= 0.0)
		BaseMatColor[1] = 0.0;
	else
		BaseMatColor[1] *= (1.0 / 255.0);
	if (BaseMatColor[2] <= 0.0)
		BaseMatColor[2] = 0.0;
	else
		BaseMatColor[2] *= (1.0 / 255.0);
	} // if

if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
	{
	if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR]->Eval(Value, Pix->TexData)) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			TexOpacity = 1.0 - TexOpacity;
			BaseMatColor[0] = BaseMatColor[0] * TexOpacity + Value[0];
			BaseMatColor[1] = BaseMatColor[1] * TexOpacity + Value[1];
			BaseMatColor[2] = BaseMatColor[2] * TexOpacity + Value[2];
			} // if
		else
			{
			BaseMatColor[0] = Value[0];
			BaseMatColor[1] = Value[1];
			BaseMatColor[2] = Value[2];
			} // else
		} // if
	} // if
DiffuseIntensity *= MaterialWeight * StrataLineWt;
Pix->RGB[0] += DiffuseIntensity * BaseMatColor[0];
Pix->RGB[1] += DiffuseIntensity * BaseMatColor[1];
Pix->RGB[2] += DiffuseIntensity * BaseMatColor[2];

// luminosity
if ((TempVal = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].CurValue) > 0.0)
	{
	if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_LUMINOSITY))
		{
		if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_LUMINOSITY]->Eval(Value, Pix->TexData)) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				TempVal *= (1.0 - TexOpacity + Value[0]);
				} // if
			else
				TempVal *= Value[0];
			} // if
		} // if
	Pix->Luminosity += TempVal * MaterialWeight;
	} // if

// specularity
if ((TempVal = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY].CurValue) > 0.0)
	{
	if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARITY))
		{
		if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARITY]->Eval(Value, Pix->TexData)) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				TempVal *= (1.0 - TexOpacity + Value[0]);
				} // if
			else
				TempVal *= Value[0];
			} // if
		} // if
	SpecularWt = TempVal * MaterialWeight * StrataLineWt;
	Pix->Specularity += SpecularWt;
	} // if

// specular exponent
if (SpecularWt > 0.0)
	{
	// exponent is always > 0 so don't test for it
	TempVal = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].CurValue;
	if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULAREXP))
		{
		if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULAREXP]->Eval(Value, Pix->TexData)) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				TempVal *= (1.0 - TexOpacity + Value[0]);
				} // if
			else
				TempVal *= Value[0];
			} // if
		} // if
	// we want to weight the exponents by the % of the material and the specularity
	Pix->SpecularExp = (Pix->SpecularExp * Pix->SpecularWt + TempVal * SpecularWt);
	Pix->SpecularWt += SpecularWt;
	Pix->SpecularExp /= Pix->SpecularWt;
	} // if

if (ApproveParam(WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY2))
	{
	// specularity 2
	if ((TempVal = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY2].CurValue) > 0.0)
		{
		if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARITY2))
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARITY2]->Eval(Value, Pix->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					TempVal *= (1.0 - TexOpacity + Value[0]);
					} // if
				else
					TempVal *= Value[0];
				} // if
			} // if
		SpecularWt2 = TempVal * MaterialWeight * StrataLineWt;
		Pix->Specularity2 += SpecularWt2;
		} // if

	// specular exponent
	if (SpecularWt2 > 0.0)
		{
		// exponent is always > 0 so don't test for it
		TempVal = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP2].CurValue;
		if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULAREXP2))
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULAREXP2]->Eval(Value, Pix->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					TempVal *= (1.0 - TexOpacity + Value[0]);
					} // if
				else
					TempVal *= Value[0];
				} // if
			} // if
		// we want to weight the exponents by the % of the material and the specularity
		Pix->SpecularExp2 = (Pix->SpecularExp2 * Pix->SpecularWt2 + TempVal * SpecularWt2);
		Pix->SpecularWt2 += SpecularWt2;
		Pix->SpecularExp2 /= Pix->SpecularWt2;
		} // if
	} // if

if (SpecularWt > 0.0)
	{
	// alternate specular color weight
	if (ApproveParam(WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARCOLORPCT))
		{
		if ((SpecColorWt = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARCOLORPCT].CurValue) > 0.0)
			{
			if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLORPCT))
				{
				if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLORPCT]->Eval(Value, Pix->TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						SpecColorWt *= (1.0 - TexOpacity + Value[0]);
						} // if
					else
						SpecColorWt *= Value[0];
					} // if
				} // if
			SpecColorWt *= MaterialWeight;
			} // if

		if (SpecColorWt > 0.0)
			{
			SpecColor[0] = SpecularColor.GetCompleteValue(0);
			SpecColor[1] = SpecularColor.GetCompleteValue(1);
			SpecColor[2] = SpecularColor.GetCompleteValue(2);
			if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR))
				{
				if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR]->Eval(Value, Pix->TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						TexOpacity = 1.0 - TexOpacity;
						SpecColor[0] = SpecColor[0] * TexOpacity + Value[0];
						SpecColor[1] = SpecColor[1] * TexOpacity + Value[1];
						SpecColor[2] = SpecColor[2] * TexOpacity + Value[2];
						} // if
					else
						{
						SpecColor[0] = Value[0];
						SpecColor[1] = Value[1];
						SpecColor[2] = Value[2];
						} // else
					} // if
				} // if
			Pix->SpecRGB[0] += SpecColorWt * SpecColor[0];
			Pix->SpecRGB[1] += SpecColorWt * SpecColor[1];
			Pix->SpecRGB[2] += SpecColorWt * SpecColor[2];
			Pix->SpecRGBPct += SpecColorWt;
			} // if
		} // if
	} // if

if (SpecularWt2 > 0.0)
	{
	// alternate specular color weight
	if (ApproveParam(WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARCOLORPCT2))
		{
		if ((SpecColorWt = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARCOLORPCT2].CurValue) > 0.0)
			{
			if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLORPCT2))
				{
				if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLORPCT2]->Eval(Value, Pix->TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						SpecColorWt *= (1.0 - TexOpacity + Value[0]);
						} // if
					else
						SpecColorWt *= Value[0];
					} // if
				} // if
			SpecColorWt *= MaterialWeight;
			} // if

		if (SpecColorWt > 0.0)
			{
			SpecColor[0] = SpecularColor2.GetCompleteValue(0);
			SpecColor[1] = SpecularColor2.GetCompleteValue(1);
			SpecColor[2] = SpecularColor2.GetCompleteValue(2);
			if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR2))
				{
				if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR2]->Eval(Value, Pix->TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						TexOpacity = 1.0 - TexOpacity;
						SpecColor[0] = SpecColor[0] * TexOpacity + Value[0];
						SpecColor[1] = SpecColor[1] * TexOpacity + Value[1];
						SpecColor[2] = SpecColor[2] * TexOpacity + Value[2];
						} // if
					else
						{
						SpecColor[0] = Value[0];
						SpecColor[1] = Value[1];
						SpecColor[2] = Value[2];
						} // else
					} // if
				} // if
			Pix->Spec2RGB[0] += SpecColorWt * SpecColor[0];
			Pix->Spec2RGB[1] += SpecColorWt * SpecColor[1];
			Pix->Spec2RGB[2] += SpecColorWt * SpecColor[2];
			Pix->Spec2RGBPct += SpecColorWt;
			} // if
		} // if
	} // if

if (ApproveParam(WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMINANCE))
	{
	// translucency
	if ((TempVal = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMINANCE].CurValue) > 0.0)
		{
		if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMINANCE))
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMINANCE]->Eval(Value, Pix->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					TempVal *= (1.0 - TexOpacity + Value[0]);
					} // if
				else
					TempVal *= Value[0];
				} // if
			} // if
		TranslucencyWt = TempVal * MaterialWeight;
		Pix->Translucency += TranslucencyWt;
		} // if

	// translucency exponent
	if (TranslucencyWt > 0.0)
		{
		// exponent is always > 0 so don't test for it
		TempVal = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMEXP].CurValue;
		if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMEXP))
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMEXP]->Eval(Value, Pix->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					TempVal *= (1.0 - TexOpacity + Value[0]);
					} // if
				else
					TempVal *= Value[0];
				} // if
			} // if
		// we want to weight the exponents by the % of the material and the translucency
		Pix->TranslucencyExp = (Pix->TranslucencyExp * Pix->TranslucencyWt + TempVal * TranslucencyWt);
		Pix->TranslucencyWt += TranslucencyWt;
		Pix->TranslucencyExp /= Pix->TranslucencyWt;
		} // if
	} // if 3D object

if (ApproveParam(WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY))
	{
	// reflectivity
	if ((TempVal = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY].CurValue) > 0.0)
		{
		if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_REFLECTIVITY))
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_REFLECTIVITY]->Eval(Value, Pix->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					TempVal *= (1.0 - TexOpacity + Value[0]);
					} // if
				else
					TempVal *= Value[0];
				} // if
			} // if
		Pix->Reflectivity += TempVal * MaterialWeight;
		} // if
	} // if

if (ApproveParam(WCS_EFFECTS_MATERIAL_ANIMPAR_OPTICALDEPTH))
	{
	// optical depth
	if ((TempVal = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_OPTICALDEPTH].CurValue) > 0.0)
		{
		if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_OPTICALDEPTH))
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_OPTICALDEPTH]->Eval(Value, Pix->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					TempVal *= (1.0 - TexOpacity + Value[0]);
					} // if
				else
					TempVal *= Value[0];
				} // if
			} // if
		Pix->OpticalDepth += TempVal * MaterialWeight;
		} // if
	} // if

/* here's how these equations look if the texture replaces the animvalue as in v4 rather than modifying it
if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_REFLECTIVITY))
	{
	if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_REFLECTIVITY]->Eval(Value, Pix)) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			TempVal = Value[0] + AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY].CurValue * (1.0 - TexOpacity);
			} // if
		else
			TempVal = Value[0];
		} // if
	else
		TempVal = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY].CurValue;
	} // if
else
	TempVal = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY].CurValue;
Pix->Reflectivity += TempVal * MaterialWeight;
*/

Pix->OpacityUsed += MaterialWeight;

} // MaterialEffect::Evaluate

/*===========================================================================*/

void MaterialEffect::EvaluateTransparency(PixelData *Pix, double MaterialWeight)
{
double TexOpacity, Value[3], TempVal;

// fills in transparency only 

// calculate transparency first and adjust MaterialWeight by material opacity if it is terrain
if (ApproveParam(WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY))
	{
	// transparency
	if ((TempVal = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue) > 0.0)
		{
		if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY))
			{
			Value[2] = Value[1] = Value[0] = 0.0;
			if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY]->Eval(Value, Pix->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					TempVal *= (1.0 - TexOpacity + Value[0]);
					} // if
				else
					TempVal *= Value[0];
				} // if
			} // if
		Pix->Transparency += TempVal * MaterialWeight;
		// this is what makes ecosystems and snow not cover ground completely if transparency is > 0
		if (Pix->PixelType == WCS_PIXELTYPE_TERRAIN)
			MaterialWeight *= (1.0 - TempVal);		// multiply material weight by opacity
		} // if

	} // if

Pix->OpacityUsed += MaterialWeight;

} // MaterialEffect::EvaluateTransparency

/*===========================================================================*/

int MaterialStrata::EvaluateStrataBump(PixelData *Pix, VertexDEM *StratumNormal, double MaterialWeight)
{
double StrataLineWt = 1.0, ElevRange, ElevStash, StrataLineWt2, StrataDiff, Value[3], BumpIntensity, TexOpacity,
	MaxElev, MinElev;

if ((BumpIntensity = AnimPar[WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_BUMPINTENSITY].CurValue) > 0.0)
	{
	if (GetEnabledTexture(WCS_EFFECTS_MATERIALSTRATA_TEXTURE_BUMPINTENSITY))
		{
		Value[0] = Value[1] = Value[2] = 0.0;
		if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIALSTRATA_TEXTURE_BUMPINTENSITY]->Eval(Value, Pix->TexData)) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				BumpIntensity *= (1.0 - TexOpacity + Value[0]);
				} // if
			else
				BumpIntensity *= Value[0];
			} // if
		} // if
	BumpIntensity *= MaterialWeight * 2.0;
	} // if

if (BumpIntensity != 0.0)
	{
	MaxElev = max(Pix->TexData->TElevRange[0], Pix->TexData->TElevRange[1]);
	MaxElev = max(MaxElev, Pix->TexData->TElevRange[2]);
	MaxElev = max(MaxElev, Pix->TexData->TElevRange[3]);
	MinElev = min(Pix->TexData->TElevRange[0], Pix->TexData->TElevRange[1]);
	MinElev = min(MinElev, Pix->TexData->TElevRange[2]);
	MinElev = min(MinElev, Pix->TexData->TElevRange[3]);
	ElevRange = MaxElev - MinElev;
	ElevStash = Pix->TexData->Elev;

	// evaluate the strata over two intervals, the nominal interval for this pixel, evaluated above,
	// and an interval higher.
	StrataLineWt = ComputeTexture(ElevRange, Pix->TexData);
	// raise the sampled elevation by one sample interval.
	Pix->TexData->Elev += fabs(ElevRange);
	StrataLineWt2 = ComputeTexture(ElevRange, Pix->TexData);
	// put the elevation back where it was
	Pix->TexData->Elev = ElevStash;

	// find the difference, if there is none then there is no change to the surface normal
	if ((StrataDiff = StrataLineWt - StrataLineWt2) != 0.0)
		{
		// find the surface normal of the stratum plane
		// for now assume the normal is straight up, user should be able to control bump strength
		StratumNormal->XYZ[0] = StratumNormal->XYZ[2] = 0.0;
		StratumNormal->XYZ[1] = 1.0 * StrataDiff * BumpIntensity;
		StratumNormal->Lat = Pix->Lat;
		StratumNormal->Lon = Pix->Lon;
		StratumNormal->RotateFromHome();
		return (1);
		} // if
	} // if bump intensity

return (0);

} // MaterialStrata::EvaluateStrataBump

/*===========================================================================*/

// return value is boolean telling whether bump map had any effect
int MaterialEffect::EvaluateTerrainBump(RenderData *Rend, PixelData *Pix, double *NormalAdd, double MaterialWeight, 
	double PixWidth, double PlanetRad)
{
double RotY, RotX, TexAmp[3], Value[3], BumpIntensity, TexOpacity;
int BumpDone = 0;

// Given the pixel's surface normal as Pix->Normal[].
// We want to sample the bump texture at two right angle positions from the pixel center.
// Pixel center is given as Pix->XYZ[].
// The new sample positions lie on vectors emanating from the pixel center and lie in the plane normal to the
// pixel's surface normal.
// To find these two vectors we find the rotation angle between the surface normal and one of the coordinate system axes.
// We rotate the vector around first the Y axis, then the X so that it aligns with the +Z axis.
// We create two vertices at offsets to the origin along the +X and +Y axes, then counter-rotate them back to the
// orientation of the surface normal. Offset distance should be the width of a pixel in screen space.
// The two new sample points are then the two vectors added to the vertex coords of the pixel center.

if (MaterialWeight > 0.0)
	{
	NormalAdd[2] = NormalAdd[1] = NormalAdd[0] = 0.0;
	Value[2] = Value[1] = Value[0] = 0.0;

	if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP))
		{
		if ((BumpIntensity = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_BUMPINTENSITY].CurValue) > 0.0)
			{
			if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMPINTENSITY))
				{
				if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_BUMPINTENSITY]->Eval(Value, Pix->TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						BumpIntensity *= (1.0 - TexOpacity + Value[0]);
						} // if
					else
						BumpIntensity *= Value[0];
					} // if
				} // if
			BumpIntensity *= MaterialWeight * 2.0;
			} // if

		if (BumpIntensity != 0.0)
			{
			Rend->BumpVert[0].XYZ[0] = Pix->Normal[0];
			Rend->BumpVert[0].XYZ[1] = Pix->Normal[1];
			Rend->BumpVert[0].XYZ[2] = Pix->Normal[2];
			RotY = Rend->BumpVert[0].FindRoughAngleYfromZ();
			Rend->BumpVert[0].RotateY(-RotY);
			RotX = Rend->BumpVert[0].FindRoughAngleXfromZ();
			Rend->BumpVert[1].XYZ[0] = PixWidth;
			Rend->BumpVert[1].XYZ[1] = Rend->BumpVert[1].XYZ[2] = 0.0;
			Rend->BumpVert[2].XYZ[1] = PixWidth;
			Rend->BumpVert[2].XYZ[0] = Rend->BumpVert[2].XYZ[2] = 0.0;
			Rend->BumpVert[1].RotateX(RotX);
			Rend->BumpVert[1].RotateY(RotY);
			Rend->BumpVert[2].RotateX(RotX);
			Rend->BumpVert[2].RotateY(RotY);

			// Some of the pixel's texture data may be needed in the vertex
			Rend->ReverseTransferTextureData(&Rend->BumpSampleVert, Pix->TexData);
			// back up pixel's texture data
			Rend->BumpBackupTextureData.CopyData(Pix->TexData);
			// sets TexData.VDataVecOffsetsValid to 0
			Rend->TransferTextureDataRange(&Rend->BumpSampleVert, Pix->TexData, PixWidth);

			// Once the sample positions are found, the bump texture is sampled 
			// at those locations and at the pixel center location.
			// Pixel center first.
			Rend->BumpSampleVert.XYZ[0] = Pix->XYZ[0];
			Rend->BumpSampleVert.XYZ[1] = Pix->XYZ[1];
			Rend->BumpSampleVert.XYZ[2] = Pix->XYZ[2];
			#ifdef WCS_BUILD_VNS
			Rend->DefCoords->CartToDeg(&Rend->BumpSampleVert);
			#else // WCS_BUILD_VNS
			Rend->BumpSampleVert.CartToDeg(Rend->PlanetRad);
			#endif // WCS_BUILD_VNS

			TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_BUMP]->Eval(Value, &Rend->TexData);
			TexAmp[0] = Value[0];

			Rend->BumpSampleVert.XYZ[0] = Pix->XYZ[0] + Rend->BumpVert[1].XYZ[0];
			Rend->BumpSampleVert.XYZ[1] = Pix->XYZ[1] + Rend->BumpVert[1].XYZ[1];
			Rend->BumpSampleVert.XYZ[2] = Pix->XYZ[2] + Rend->BumpVert[1].XYZ[2];
			Rend->TexData.VDataVecOffsetsValid = 0;
			#ifdef WCS_BUILD_VNS
			Rend->DefCoords->CartToDeg(&Rend->BumpSampleVert);
			#else // WCS_BUILD_VNS
			Rend->BumpSampleVert.CartToDeg(Rend->PlanetRad);
			#endif // WCS_BUILD_VNS

			TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_BUMP]->Eval(Value, &Rend->TexData);
			TexAmp[1] = Value[0];

			Rend->BumpSampleVert.XYZ[0] = Pix->XYZ[0] + Rend->BumpVert[2].XYZ[0];
			Rend->BumpSampleVert.XYZ[1] = Pix->XYZ[1] + Rend->BumpVert[2].XYZ[1];
			Rend->BumpSampleVert.XYZ[2] = Pix->XYZ[2] + Rend->BumpVert[2].XYZ[2];
			Rend->TexData.VDataVecOffsetsValid = 0;
			#ifdef WCS_BUILD_VNS
			Rend->DefCoords->CartToDeg(&Rend->BumpSampleVert);
			#else // WCS_BUILD_VNS
			Rend->BumpSampleVert.CartToDeg(Rend->PlanetRad);
			#endif // WCS_BUILD_VNS

			TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_BUMP]->Eval(Value, &Rend->TexData);
			TexAmp[2] = Value[0];

			// The differences in intensity are multiplied by a bump scalar and by the 3D vector which defines the offset
			// direction of the sample point from the pixel center. Signs are reversed so that a decrease in bump texture
			// in a vector's direction causes the normal to be moved in that direction. Offset vectors are normalized before
			// multiplying by bump intensity.

			TexAmp[1] = TexAmp[0] - TexAmp[1];
			TexAmp[2] = TexAmp[0] - TexAmp[2];

			TexAmp[1] *= BumpIntensity;
			TexAmp[2] *= BumpIntensity;

			PixWidth = 1.0 / PixWidth;
			Rend->BumpVert[1].MultiplyXYZ(PixWidth * TexAmp[1]);

			Rend->BumpVert[2].MultiplyXYZ(PixWidth * TexAmp[2]);

			// The two resultant vectors are added
			
			NormalAdd[0] = Rend->BumpVert[1].XYZ[0] + Rend->BumpVert[2].XYZ[0];
			NormalAdd[1] = Rend->BumpVert[1].XYZ[1] + Rend->BumpVert[2].XYZ[1];
			NormalAdd[2] = Rend->BumpVert[1].XYZ[2] + Rend->BumpVert[2].XYZ[2];
			BumpDone = 1;
			// restore pixel's texture data
			// restores TexData->VDataVecOffsetsValid
			Pix->TexData->CopyData(&Rend->BumpBackupTextureData);
			} // if bump intensity
		} // if

	if (ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_STRATA) && Strata && Strata->Enabled && Strata->BumpLines)
		{
		if (Strata->EvaluateStrataBump(Pix, &Rend->BumpVert[0], MaterialWeight))
			{
			NormalAdd[0] += Rend->BumpVert[0].XYZ[0];
			NormalAdd[1] += Rend->BumpVert[0].XYZ[1];
			NormalAdd[2] += Rend->BumpVert[0].XYZ[2];
			BumpDone = 1;
			} // if
		} // if
	} // if material weight

Pix->OpacityUsed += MaterialWeight;

return (BumpDone);

} // MaterialEffect::EvaluateTerrainBump

/*===========================================================================*/

// return value is boolean telling whether bump map had any effect
int MaterialEffect::Evaluate3DBump(RenderData *Rend, PixelData *Pix, double *NormalAdd, double MaterialWeight, 
	double PixWidth, Object3DEffect *Object)
{
double RotY, RotX, TexAmp[3], Value[3], BumpIntensity, TexOpacity, ScaledPixWidth;
int BumpDone = 0;

// Given the pixel's surface normal as Pix->Normal[].
// We want to sample the bump texture at two right angle positions from the pixel center.
// Pixel center is given as Pix->XYZ[].
// The new sample positions lie on vectors emanating from the pixel center and lie in the plane normal to the
// pixel's surface normal.
// To find these two vectors we find the rotation angle between the surface normal and one of the coordinate system axes.
// We rotate the vector around first the Y axis, then the X so that it aligns with the +Z axis.
// We create two vertices at offsets to the origin along the +X and +Y axes, then counter-rotate them back to the
// orientation of the surface normal. Offset distance should be the width of a pixel in screen space.
// The two new sample points are then the two vectors added to the vertex coords of the pixel center.

if (MaterialWeight > 0.0)
	{
	NormalAdd[0] = NormalAdd[1] = NormalAdd[2] = 0.0;
	Value[0] = Value[1] = Value[2] = 0.0;

	if ((BumpIntensity = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_BUMPINTENSITY].CurValue) > 0.0)
		{
		if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMPINTENSITY))
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_BUMPINTENSITY]->Eval(Value, Pix->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					BumpIntensity *= (1.0 - TexOpacity + Value[0]);
					} // if
				else
					BumpIntensity *= Value[0];
				} // if
			} // if
		BumpIntensity *= MaterialWeight * 2.0;
		} // if

	if (BumpIntensity != 0.0)
		{
		Rend->BumpVert[0].XYZ[0] = Pix->Normal[0];
		Rend->BumpVert[0].XYZ[1] = Pix->Normal[1];
		Rend->BumpVert[0].XYZ[2] = Pix->Normal[2];
		RotY = Rend->BumpVert[0].FindRoughAngleYfromZ();
		Rend->BumpVert[0].RotateY(-RotY);
		RotX = Rend->BumpVert[0].FindRoughAngleXfromZ();
		Rend->BumpVert[1].XYZ[0] = PixWidth;
		Rend->BumpVert[1].XYZ[1] = Rend->BumpVert[1].XYZ[2] = 0.0;
		Rend->BumpVert[2].XYZ[1] = PixWidth;
		Rend->BumpVert[2].XYZ[0] = Rend->BumpVert[2].XYZ[2] = 0.0;
		Rend->BumpVert[1].RotateX(RotX);
		Rend->BumpVert[1].RotateY(RotY);
		Rend->BumpVert[2].RotateX(RotX);
		Rend->BumpVert[2].RotateY(RotY);

		// find length of PixWidth in object space
		Rend->BumpSampleVert.XYZ[0] = Pix->XYZ[0];
		Rend->BumpSampleVert.XYZ[1] = Pix->XYZ[1];
		Rend->BumpSampleVert.XYZ[2] = Pix->XYZ[2];
		Rend->BumpSampleVert.Transform3DPointTo_xyz(Object->InvWorldMatx);
		Rend->BumpSampleVert.XYZ[0] = Pix->XYZ[0] + Rend->BumpVert[1].XYZ[0];
		Rend->BumpSampleVert.XYZ[1] = Pix->XYZ[1] + Rend->BumpVert[1].XYZ[1];
		Rend->BumpSampleVert.XYZ[2] = Pix->XYZ[2] + Rend->BumpVert[1].XYZ[2];
		Rend->BumpSampleVert.Transform3DPoint(Object->InvWorldMatx);
		FindPosVector(Rend->BumpSampleVert.XYZ, Rend->BumpSampleVert.xyz, Rend->BumpSampleVert.XYZ);
		ScaledPixWidth = VectorMagnitude(Rend->BumpSampleVert.XYZ);

		// back up pixel's texture data
		Rend->BumpBackupTextureData.CopyData(Pix->TexData);
		Rend->Transfer3DTextureDataRange(&Rend->BumpSampleVert, PixWidth, ScaledPixWidth);

		// Once the sample positions are found, the bump texture is sampled 
		// at those locations and at the pixel center location.
		// Pixel center first.
		Rend->BumpSampleVert.XYZ[0] = Pix->XYZ[0];
		Rend->BumpSampleVert.XYZ[1] = Pix->XYZ[1];
		Rend->BumpSampleVert.XYZ[2] = Pix->XYZ[2];
		// because textures can be applied in world space
		#ifdef WCS_BUILD_VNS
		Rend->DefCoords->CartToDeg(&Rend->BumpSampleVert);
		#else // WCS_BUILD_VNS
		Rend->BumpSampleVert.CartToDeg(Rend->PlanetRad);
		#endif // WCS_BUILD_VNS
		// transform coordinates to object space, InvWorldMatx includes local matrix
		Rend->BumpSampleVert.Transform3DPointTo_xyz(Object->InvWorldMatx);

		TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_BUMP]->Eval(Value, &Rend->TexData);
		TexAmp[0] = Value[0];

		Rend->BumpSampleVert.XYZ[0] = Pix->XYZ[0] + Rend->BumpVert[1].XYZ[0];
		Rend->BumpSampleVert.XYZ[1] = Pix->XYZ[1] + Rend->BumpVert[1].XYZ[1];
		Rend->BumpSampleVert.XYZ[2] = Pix->XYZ[2] + Rend->BumpVert[1].XYZ[2];
		// because textures can be applied in world space
		#ifdef WCS_BUILD_VNS
		Rend->DefCoords->CartToDeg(&Rend->BumpSampleVert);
		#else // WCS_BUILD_VNS
		Rend->BumpSampleVert.CartToDeg(Rend->PlanetRad);
		#endif // WCS_BUILD_VNS
		// transform coordinates to object space, InvWorldMatx includes local matrix
		Rend->BumpSampleVert.Transform3DPointTo_xyz(Object->InvWorldMatx);

		TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_BUMP]->Eval(Value, &Rend->TexData);
		TexAmp[1] = Value[0];

		Rend->BumpSampleVert.XYZ[0] = Pix->XYZ[0] + Rend->BumpVert[2].XYZ[0];
		Rend->BumpSampleVert.XYZ[1] = Pix->XYZ[1] + Rend->BumpVert[2].XYZ[1];
		Rend->BumpSampleVert.XYZ[2] = Pix->XYZ[2] + Rend->BumpVert[2].XYZ[2];
		// because textures can be applied in world space
		#ifdef WCS_BUILD_VNS
		Rend->DefCoords->CartToDeg(&Rend->BumpSampleVert);
		#else // WCS_BUILD_VNS
		Rend->BumpSampleVert.CartToDeg(Rend->PlanetRad);
		#endif // WCS_BUILD_VNS
		// transform coordinates to object space, InvWorldMatx includes local matrix
		Rend->BumpSampleVert.Transform3DPointTo_xyz(Object->InvWorldMatx);

		TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_BUMP]->Eval(Value, &Rend->TexData);
		TexAmp[2] = Value[0];

		// The differences in intensity are multiplied by a bump scalar and by the 3D vector which defines the offset
		// direction of the sample point from the pixel center. Signs are reversed so that a decrease in bump texture
		// in a vector's direction causes the normal to be moved in that direction. Offset vectors are normalized before
		// multiplying by bump intensity.

		TexAmp[1] = TexAmp[0] - TexAmp[1];
		TexAmp[2] = TexAmp[0] - TexAmp[2];

		TexAmp[1] *= BumpIntensity;
		TexAmp[2] *= BumpIntensity;

		PixWidth = 1.0 / PixWidth;
		Rend->BumpVert[1].MultiplyXYZ(PixWidth * TexAmp[1]);

		Rend->BumpVert[2].MultiplyXYZ(PixWidth * TexAmp[2]);

		// The two resultant vectors are added
		
		NormalAdd[0] = Rend->BumpVert[1].XYZ[0] + Rend->BumpVert[2].XYZ[0];
		NormalAdd[1] = Rend->BumpVert[1].XYZ[1] + Rend->BumpVert[2].XYZ[1];
		NormalAdd[2] = Rend->BumpVert[1].XYZ[2] + Rend->BumpVert[2].XYZ[2];
		BumpDone = 1;
		// restore pixel's texture data
		Pix->TexData->CopyData(&Rend->BumpBackupTextureData);
		} // if bump intensity

	if (ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_STRATA) && Strata && Strata->Enabled && Strata->BumpLines)
		{
		if (Strata->EvaluateStrataBump(Pix, &Rend->BumpVert[0], MaterialWeight))
			{
			NormalAdd[0] += Rend->BumpVert[0].XYZ[0];
			NormalAdd[1] += Rend->BumpVert[0].XYZ[1];
			NormalAdd[2] += Rend->BumpVert[0].XYZ[2];
			BumpDone = 1;
			} // if
		} // if
	} // if material weight

Pix->OpacityUsed += MaterialWeight;

return (BumpDone);

} // MaterialEffect::Evaluate3DBump

/*===========================================================================*/

double MaterialEffect::EvaluateOpacity(PixelData *Pix)
{
double TempVal, Value[3], TexOpacity;

// evaluate material transparency
if (ApproveParam(WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY))
	{
	// transparency
	if ((TempVal = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue) > 0.0)
		{
		if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY))
			{
			Value[0] = Value[1] = Value[2] = 0.0;
			if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY]->Eval(Value, Pix->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					TempVal *= (1.0 - TexOpacity + Value[0]);
					} // if
				else
					TempVal *= Value[0];
				} // if
			} // if
		} // if
	return (1.0 - TempVal);
	} // if

return (1.0);

} // MaterialEffect::EvaluateOpacity

/*===========================================================================*/

// careful, for speed reasons the texture's availability must be checked before calling this
// since it requires that texture data be transferred from a vertex.
double MaterialEffect::EvaluateDisplacement(TextureData *TexData)
{
double Value[3], TexOpacity;

// displacement
Value[0] = Value[1] = Value[2] = 0.0;
if ((TexOpacity = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_DISPLACEMENT]->Eval(Value, TexData)) > 0.0)
	{
	// Value[0] has already been diminished by the texture's opacity
	Value[0] *= AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DISPLACEMENT].CurValue;
	return (Value[0]);
	} // if

return (0.0);

} // MaterialEffect::EvaluateDisplacement

/*===========================================================================*/
/*===========================================================================*/

void Ecotype::EvaluateDissolve(PixelData *Pix, double DissolveWeight)
{
double Value[3], TexOpacity;

if (GetEnabledTexture(WCS_ECOTYPE_TEXTURE_DISSOLVECOLOR))
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	if ((TexOpacity = TexRoot[WCS_ECOTYPE_TEXTURE_DISSOLVECOLOR]->Eval(Value, Pix->TexData)) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			TexOpacity = 1.0 - TexOpacity;
			Value[0] += DissolveColor.GetCompleteValue(0) * TexOpacity;
			Value[1] += DissolveColor.GetCompleteValue(1) * TexOpacity;
			Value[2] += DissolveColor.GetCompleteValue(2) * TexOpacity;
			} // if
		} // if
	else
		{
		Value[0] = DissolveColor.GetCompleteValue(0);
		Value[1] = DissolveColor.GetCompleteValue(1);
		Value[2] = DissolveColor.GetCompleteValue(2);
		} // else
	} // if
else
	{
	Value[0] = DissolveColor.GetCompleteValue(0);
	Value[1] = DissolveColor.GetCompleteValue(1);
	Value[2] = DissolveColor.GetCompleteValue(2);
	} // else
Pix->RGB[0] += Value[0] * DissolveWeight;
Pix->RGB[1] += Value[1] * DissolveWeight;
Pix->RGB[2] += Value[2] * DissolveWeight;

Pix->OpacityUsed += DissolveWeight;

} // Ecotype::EvaluateDissolve

/*===========================================================================*/
/*===========================================================================*/

double SnowEffect::EvaluateCoverage(RenderData *Rend, PolygonData *Poly)
{
double Value[3], TexOpacity, Feathering, SnowLine, SnowMin, SnowMax, SnowElevFac, SnowSlopeFac, 
	SnowRelFac, SnowBlend, Fact1, Fact2, AspectRad;

AspectRad = DEG2RAD(Poly->Aspect);

SnowLine = Eco.Line
	+ Eco.Skew * Poly->Slope * cos(AspectRad - Eco.SkewAzimuth)	// skew has been normalized by dividing by 45 degrees
    + Eco.AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_RELEL].CurValue * Poly->RelEl;
if (GlobalGradientsEnabled)
	SnowLine -= (fabs(Poly->Lat) - fabs(AnimPar[WCS_EFFECTS_SNOW_ANIMPAR_GLOBALREFLAT].CurValue)) * GlobalGradient;

// first test for snow range matching terrain characteristics
if (Poly->Elev >= SnowLine)
	{
	if (Poly->Slope >= Eco.MinSlope &&	// both in degrees
		Poly->Slope <= Eco.MaxSlope)
		{
		if (Poly->RelEl >= Eco.MinRelEl &&
			Poly->RelEl <= Eco.MaxRelEl)
			{
			// see if feathering exists
			if ((Feathering = AnimPar[WCS_EFFECTS_SNOW_ANIMPAR_FEATHERING].CurValue) > 0.0)
				{
				if (GetEnabledTexture(WCS_EFFECTS_SNOW_TEXTURE_FEATHERING))
					{
					Value[0] = Value[1] = Value[2] = 0.0;
					if ((TexOpacity = TexRoot[WCS_EFFECTS_SNOW_TEXTURE_FEATHERING]->Eval(Value, Rend->TransferTextureData(Poly))) > 0.0)
						{
						if (TexOpacity < 1.0)
							{
							// Value[0] has already been diminished by the texture's opacity
							Feathering *= (1.0 - TexOpacity + Value[0]);
							} // if
						else
							Feathering *= Value[0];
						} // if
					} // if

				// use the feathering value to modify coverage by terrain characteristics
				if (Feathering > 0.0)
					{
					// Calculate Snow Elev Factor
					SnowElevFac = ((Poly->Elev - SnowLine) * 0.01); // Range: 100m,  // Optimized out division. Was / 100.0
					if (SnowElevFac > 1.0)
						SnowElevFac = 1.0;

					// Calculate Snow Slope Factor
					SnowMax = Eco.MaxSlope;
					SnowMin = Eco.MinSlope;
					if (SnowMin > 0.0)
						SnowSlopeFac = ((SnowMax - Poly->Slope) / ((SnowMax - SnowMin) * 0.25)); // Range: SnowSlopeRange/4
					else
						{
						Fact1 = SnowMax - Poly->Slope;
						Fact2 = Poly->Slope - SnowMin;
						SnowSlopeFac = (min(Fact1, Fact2) / ((SnowMax - SnowMin) * 0.25)); // SnowSlopeRange/4
						} // else
					if (SnowSlopeFac > 1.0)
						SnowSlopeFac = 1.0;

					// Calculate Snow RelEl Factor
					SnowMin = Eco.MinRelEl;
					SnowMax = Eco.MaxRelEl;
					Fact1 = fabs(SnowMax - Poly->RelEl);
					Fact2 = fabs(Poly->RelEl - SnowMin);
					SnowRelFac = (min(Fact1, Fact2) / ((SnowMax - SnowMin) * 0.25)); // Range: SnowRelRange/4
					if (SnowRelFac > 1.0)
						SnowRelFac = 1.0;

					// SnowBlend will now be the Snow Blending amount, from 0.0 to 1.0
					SnowBlend = SnowRelFac * SnowSlopeFac * SnowElevFac; // Calculate unadjusted blend amount

					// SnowFeatherInverse is snow edge sharpness, [1.0,0.0]
					// 1.0: hard edge
					// 0.0: Maximum feathering

					// Adjust by Sharpness amount:
					// Add 1/10th of amount, then convert amount into range of 1.0-10.0 and multiply
					// Sharpness values of 0.0 will be a no-op, values of 1.0 will guarantee
					// a result > 1.0 which will clip to 1.0 below.
					// SnowBlend of 0.0 is no snow, 1.0 is full Snow
					//SnowBlend += (SnowFeatherInverse * .10);
					//SnowBlend *= (1.0 + (9.0 * SnowFeatherInverse));
					SnowBlend += (1.0 - Feathering);
					if (SnowBlend < 0.0)
						SnowBlend = 0.0;
					if (SnowBlend > 1.0)
						SnowBlend = 1.0;
					return (SnowBlend);
					} // if
				} // if
			// no feathering
			return (1.0);
			} // if RelEl within range
		} // if slope within range
	} // if elev greater than snowline

return (0.0);

} // SnowEffect::EvaluateCoverage

/*===========================================================================*/

double SnowEffect::EvaluateCoverage(RenderData *Rend, VectorNode *CurNode)
{
double Value[3], TexOpacity, Feathering, SnowLine, SnowMin, SnowMax, SnowElevFac, SnowSlopeFac, 
	SnowRelFac, SnowBlend, Fact1, Fact2, AspectRad;

AspectRad = DEG2RAD(CurNode->NodeData->Aspect);

SnowLine = Eco.Line
	+ Eco.Skew * CurNode->NodeData->Slope * cos(AspectRad - Eco.SkewAzimuth)	// skew has been normalized by dividing by 45 degrees
    + Eco.AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_RELEL].CurValue * CurNode->NodeData->RelEl;
if (GlobalGradientsEnabled)
	SnowLine -= (fabs(CurNode->Lat) - fabs(AnimPar[WCS_EFFECTS_SNOW_ANIMPAR_GLOBALREFLAT].CurValue)) * GlobalGradient;

// first test for snow range matching terrain characteristics
if (CurNode->Elev >= SnowLine)
	{
	if (CurNode->NodeData->Slope >= Eco.MinSlope &&	// both in degrees
		CurNode->NodeData->Slope <= Eco.MaxSlope)
		{
		if (CurNode->NodeData->RelEl >= Eco.MinRelEl &&
			CurNode->NodeData->RelEl <= Eco.MaxRelEl)
			{
			// see if feathering exists
			if ((Feathering = AnimPar[WCS_EFFECTS_SNOW_ANIMPAR_FEATHERING].CurValue) > 0.0)
				{
				if (GetEnabledTexture(WCS_EFFECTS_SNOW_TEXTURE_FEATHERING))
					{
					Value[0] = Value[1] = Value[2] = 0.0;
					if ((TexOpacity = TexRoot[WCS_EFFECTS_SNOW_TEXTURE_FEATHERING]->Eval(Value, Rend->TransferTextureData(CurNode, NULL, NULL))) > 0.0)
						{
						if (TexOpacity < 1.0)
							{
							// Value[0] has already been diminished by the texture's opacity
							Feathering *= (1.0 - TexOpacity + Value[0]);
							} // if
						else
							Feathering *= Value[0];
						} // if
					} // if

				// use the feathering value to modify coverage by terrain characteristics
				if (Feathering > 0.0)
					{
					// Calculate Snow Elev Factor
					SnowElevFac = ((CurNode->Elev - SnowLine) * 0.01); // Range: 100m,  // Optimized out division. Was / 100.0
					if (SnowElevFac > 1.0)
						SnowElevFac = 1.0;

					// Calculate Snow Slope Factor
					SnowMax = Eco.MaxSlope;
					SnowMin = Eco.MinSlope;
					if (SnowMin > 0.0 && SnowMax < 90.0)
						{
						Fact1 = SnowMax - CurNode->NodeData->Slope;
						Fact2 = CurNode->NodeData->Slope - SnowMin;
						SnowSlopeFac = (min(Fact1, Fact2) / ((SnowMax - SnowMin) * 0.25)); // SnowSlopeRange/4
						} // else
					else if (SnowMin > 0.0)
						SnowSlopeFac = ((CurNode->NodeData->Slope - SnowMin) / (90.0 - SnowMin * 0.25)); // Range: SnowSlopeRange/4
					else if (SnowMax < 90.0)
						SnowSlopeFac = ((SnowMax - CurNode->NodeData->Slope) / (SnowMax * 0.25)); // Range: SnowSlopeRange/4
					else
						SnowSlopeFac = 1.0;
					if (SnowSlopeFac > 1.0)
						SnowSlopeFac = 1.0;

					// Calculate Snow RelEl Factor
					SnowMin = Eco.MinRelEl;
					SnowMax = Eco.MaxRelEl;
					Fact1 = fabs(SnowMax - CurNode->NodeData->RelEl);
					Fact2 = fabs(CurNode->NodeData->RelEl - SnowMin);
					SnowRelFac = (min(Fact1, Fact2) / ((SnowMax - SnowMin) * 0.25)); // Range: SnowRelRange/4
					if (SnowRelFac > 1.0)
						SnowRelFac = 1.0;

					// SnowBlend will now be the Snow Blending amount, from 0.0 to 1.0
					SnowBlend = SnowRelFac * SnowSlopeFac * SnowElevFac; // Calculate unadjusted blend amount

					// SnowFeatherInverse is snow edge sharpness, [1.0,0.0]
					// 1.0: hard edge
					// 0.0: Maximum feathering

					// Adjust by Sharpness amount:
					// Add 1/10th of amount, then convert amount into range of 1.0-10.0 and multiply
					// Sharpness values of 0.0 will be a no-op, values of 1.0 will guarantee
					// a result > 1.0 which will clip to 1.0 below.
					// SnowBlend of 0.0 is no snow, 1.0 is full Snow
					//SnowBlend += (SnowFeatherInverse * .10);
					//SnowBlend *= (1.0 + (9.0 * SnowFeatherInverse));
					SnowBlend += (1.0 - Feathering);
					if (SnowBlend < 0.0)
						SnowBlend = 0.0;
					if (SnowBlend > 1.0)
						SnowBlend = 1.0;
					return (SnowBlend);
					} // if
				} // if
			// no feathering
			return (1.0);
			} // if RelEl within range
		} // if slope within range
	} // if elev greater than snowline

return (0.0);

} // SnowEffect::EvaluateCoverage

/*===========================================================================*/

double SnowEffect::EvaluateCoverage(RenderData *Rend, PixelData *Pix)
{
double Value[3], TexOpacity, Feathering, SnowLine, SnowMin, SnowMax, SnowElevFac, SnowSlopeFac, 
	SnowRelFac, SnowBlend, Fact1, Fact2, AspectRad;

AspectRad = DEG2RAD(Pix->TexData->Aspect);

SnowLine = Eco.Line
	+ Eco.Skew * Pix->TexData->Slope * cos(AspectRad - Eco.SkewAzimuth)	// skew has been normalized by dividing by 45 degrees
    + Eco.AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_RELEL].CurValue * Pix->TexData->RelElev;
if (GlobalGradientsEnabled)
	SnowLine -= (fabs(Pix->Lat) - fabs(AnimPar[WCS_EFFECTS_SNOW_ANIMPAR_GLOBALREFLAT].CurValue)) * GlobalGradient;

// first test for snow range matching terrain characteristics
if (Pix->Elev >= SnowLine)
	{
	if (Pix->TexData->Slope >= Eco.MinSlope &&	// both in degrees
		Pix->TexData->Slope <= Eco.MaxSlope)
		{
		if (Pix->TexData->RelElev >= Eco.MinRelEl &&
			Pix->TexData->RelElev <= Eco.MaxRelEl)
			{
			// see if feathering exists
			if ((Feathering = AnimPar[WCS_EFFECTS_SNOW_ANIMPAR_FEATHERING].CurValue) > 0.0)
				{
				if (GetEnabledTexture(WCS_EFFECTS_SNOW_TEXTURE_FEATHERING))
					{
					Value[2] = Value[1] = Value[0] = 0.0;
					if ((TexOpacity = TexRoot[WCS_EFFECTS_SNOW_TEXTURE_FEATHERING]->Eval(Value, Pix->TexData)) > 0.0)
						{
						if (TexOpacity < 1.0)
							{
							// Value[0] has already been diminished by the texture's opacity
							Feathering *= (1.0 - TexOpacity + Value[0]);
							} // if
						else
							Feathering *= Value[0];
						} // if
					} // if

				// use the feathering value to modify coverage by terrain characteristics
				if (Feathering > 0.0)
					{
					// Calculate Snow Elev Factor
					SnowElevFac = ((Pix->Elev - SnowLine) * 0.01); // Range: 100m,  // Optimized out division. Was / 100.0
					if (SnowElevFac > 1.0)
						SnowElevFac = 1.0;

					// Calculate Snow Slope Factor
					SnowMax = Eco.MaxSlope;
					SnowMin = Eco.MinSlope;
					if (SnowMin > 0.0 && SnowMax < 90.0)
						{
						Fact1 = SnowMax - Pix->TexData->Slope;
						Fact2 = Pix->TexData->Slope - SnowMin;
						SnowSlopeFac = (min(Fact1, Fact2) / ((SnowMax - SnowMin) * 0.25)); // SnowSlopeRange/4
						} // else
					else if (SnowMin > 0.0)
						SnowSlopeFac = ((Pix->TexData->Slope - SnowMin) / (90.0 - SnowMin * 0.25)); // Range: SnowSlopeRange/4
					else if (SnowMax < 90.0)
						SnowSlopeFac = ((SnowMax - Pix->TexData->Slope) / (SnowMax * 0.25)); // Range: SnowSlopeRange/4
					else
						SnowSlopeFac = 1.0;
					if (SnowSlopeFac > 1.0)
						SnowSlopeFac = 1.0;

					// Calculate Snow RelEl Factor
					SnowMin = Eco.MinRelEl;
					SnowMax = Eco.MaxRelEl;
					Fact1 = fabs(SnowMax - Pix->TexData->RelElev);
					Fact2 = fabs(Pix->TexData->RelElev - SnowMin);
					SnowRelFac = (min(Fact1, Fact2) / ((SnowMax - SnowMin) * 0.25)); // Range: SnowRelRange/4
					if (SnowRelFac > 1.0)
						SnowRelFac = 1.0;

					// SnowBlend will now be the Snow Blending amount, from 0.0 to 1.0
					SnowBlend = SnowRelFac * SnowSlopeFac * SnowElevFac; // Calculate unadjusted blend amount

					// SnowFeatherInverse is snow edge sharpness, [1.0,0.0]
					// 1.0: hard edge
					// 0.0: Maximum feathering

					// Adjust by Sharpness amount:
					// Add 1/10th of amount, then convert amount into range of 1.0-10.0 and multiply
					// Sharpness values of 0.0 will be a no-op, values of 1.0 will guarantee
					// a result > 1.0 which will clip to 1.0 below.
					// SnowBlend of 0.0 is no snow, 1.0 is full Snow
					//SnowBlend += (SnowFeatherInverse * .10);
					//SnowBlend *= (1.0 + (9.0 * SnowFeatherInverse));
					SnowBlend += (1.0 - Feathering);
					if (SnowBlend < 0.0)
						SnowBlend = 0.0;
					else if (SnowBlend > 1.0)
						SnowBlend = 1.0;
					return (SnowBlend);
					} // if
				} // if
			// no feathering
			return (1.0);
			} // if RelEl within range
		} // if slope within range
	} // if elev greater than snowline

return (0.0);

} // SnowEffect::EvaluateCoverage

/*===========================================================================*/
/*===========================================================================*/

int EnvironmentEffect::EvalEcosystem(PolygonData *Poly)
{
double EcoLine, AspectRad;
EffectList *CurEffect = Ecosystems;
EcosystemEffect *CurEco;

AspectRad = DEG2RAD(Poly->Aspect);

while (CurEffect)
	{
	if (CurEco = (EcosystemEffect *)CurEffect->Me)
		{
		if (CurEco->Enabled)
			{
			EcoLine = CurEco->Line
				- CurEco->Skew * Poly->Slope * cos(AspectRad - CurEco->SkewAzimuth)	// skew has been normalized by dividing by 45 degrees
				+ CurEco->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_RELEL].CurValue * Poly->RelEl;
			if (GlobalGradientsEnabled)
				EcoLine -= (fabs(Poly->Lat) - fabs(AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_GLOBALREFLAT].CurValue)) * GlobalGradient;

			// test for eco range matching terrain characteristics
			if (Poly->Elev <= EcoLine)
				{
				if (Poly->Slope >= CurEco->MinSlope &&	// both in degrees
					Poly->Slope <= CurEco->MaxSlope)
					{
					if (Poly->RelEl >= CurEco->MinRelEl &&
						Poly->RelEl <= CurEco->MaxRelEl)
						{
						Poly->Eco = CurEco;
						return (1);
						} // if in relel bounds
					} // if in slope bounds
				} // if in elevation bounds
			} // if
		} // if eco effect
	CurEffect = CurEffect->Next;
	} // while

return (0);

} // EnvironmentEffect::EvalEcosystem

/*===========================================================================*/

bool EnvironmentEffect::EvalEcosystem(RenderData *Rend, VectorNode *CurNode, double &SumOfAllCoverages, double &PlowedSnow)
{
double EcoLine, AspectRad, AccumulatedWt = 0.0;
EffectList *CurEffect = Ecosystems;
EcosystemEffect *CurEco;
TwinMaterials MatTwin;
bool Done = false, Success = true;

AspectRad = DEG2RAD(CurNode->NodeData->Aspect);

while (CurEffect && AccumulatedWt < 1.0)
	{
	if (CurEco = (EcosystemEffect *)CurEffect->Me)
		{
		if (CurEco->Enabled)
			{
			EcoLine = CurEco->Line
				- CurEco->Skew * CurNode->NodeData->Slope * cos(AspectRad - CurEco->SkewAzimuth)	// skew has been normalized by dividing by 45 degrees
				+ CurEco->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_RELEL].CurValue * CurNode->NodeData->RelEl;
			if (GlobalGradientsEnabled)
				EcoLine -= (fabs(CurNode->Lat) - fabs(AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_GLOBALREFLAT].CurValue)) * GlobalGradient;

			// test for eco range matching terrain characteristics
			if (CurNode->Elev <= EcoLine)
				{
				if (CurNode->NodeData->Slope >= CurEco->MinSlope &&	// both in degrees
					CurNode->NodeData->Slope <= CurEco->MaxSlope)
					{
					if (CurNode->NodeData->RelEl >= CurEco->MinRelEl &&
						CurNode->NodeData->RelEl <= CurEco->MaxRelEl)
						{
						if (CurEco->EcoMat.GetRenderMaterial(&MatTwin, CurEco->GetRenderMatGradientPos(Rend, NULL, CurNode, NULL)))
							{
							if (MatTwin.Mat[0])
								{
								if (CurEco->PlowSnow)
									PlowedSnow += MatTwin.Covg[0];
								AccumulatedWt += MatTwin.Covg[0] * (1.0 - MatTwin.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
								if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[0], CurEco, NULL, (float)MatTwin.Covg[0]))
									{
									Success = false;
									break;
									} // if
								} // if
							if (MatTwin.Mat[1])
								{
								if (CurEco->PlowSnow)
									PlowedSnow += MatTwin.Covg[1];
								AccumulatedWt += MatTwin.Covg[1] * (1.0 - MatTwin.Mat[1]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
								if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[1], CurEco, NULL, (float)MatTwin.Covg[1]))
									{
									Success = false;
									break;
									} // if
								} // if
							Done = true;
							} // if
						} // if in relel bounds
					} // if in slope bounds
				} // if in elevation bounds
			} // if
		} // if eco effect
	CurEffect = CurEffect->Next;
	} // while

SumOfAllCoverages += AccumulatedWt;

return (Success);

} // EnvironmentEffect::EvalEcosystem

/*===========================================================================*/
/*===========================================================================*/

double EcosystemEffect::GetRenderMatGradientPos(RenderData *Rend, PolygonData *Poly)
{
double Value[3], TempVal, TexOpacity;

// evaluate thematic map
if (GetEnabledTheme(WCS_EFFECTS_ECOSYSTEM_THEME_MATDRIVER) &&
	Theme[WCS_EFFECTS_ECOSYSTEM_THEME_MATDRIVER]->Eval(Value, Poly->Vector))
	{
	TempVal = Value[0] * .01;
	} // if
else
	TempVal = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MATDRIVER].CurValue;
// gradient position
if (GetEnabledTexture(WCS_EFFECTS_ECOSYSTEM_TEXTURE_MATDRIVER))
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	if ((TexOpacity = TexRoot[WCS_EFFECTS_ECOSYSTEM_TEXTURE_MATDRIVER]->Eval(Value, Rend->TransferTextureData(Poly))) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			// Value[0] has already been diminished by the texture's opacity
			TempVal = Value[0] + TempVal * (1.0 - TexOpacity);
			} // if
		else
			TempVal = Value[0];
		} // if
	} // if

return (TempVal);

} // EcosystemEffect::GetRenderMatGradientPos

/*===========================================================================*/

double EcosystemEffect::GetRenderMatGradientPos(RenderData *Rend, Joe *RefVec, VectorNode *CurNode, MaterialList *RefMat)
{
double Value[3], TempVal, TexOpacity;

// evaluate thematic map
if (GetEnabledTheme(WCS_EFFECTS_ECOSYSTEM_THEME_MATDRIVER) &&
	Theme[WCS_EFFECTS_ECOSYSTEM_THEME_MATDRIVER]->Eval(Value, RefVec))
	{
	TempVal = Value[0] * .01;
	} // if
else
	TempVal = AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MATDRIVER].CurValue;
// gradient position
if (GetEnabledTexture(WCS_EFFECTS_ECOSYSTEM_TEXTURE_MATDRIVER))
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	if ((TexOpacity = TexRoot[WCS_EFFECTS_ECOSYSTEM_TEXTURE_MATDRIVER]->Eval(Value, Rend->TransferTextureData(CurNode, RefVec, RefMat))) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			// Value[0] has already been diminished by the texture's opacity
			TempVal = Value[0] + TempVal * (1.0 - TexOpacity);
			} // if
		else
			TempVal = Value[0];
		} // if
	} // if

return (TempVal);

} // EcosystemEffect::GetRenderMatGradientPos

/*===========================================================================*/
/*===========================================================================*/

double Fence::GetRenderMatGradientPos(RenderData *Rend, PolygonData *Poly)
{
double Value[3], TempVal, TexOpacity;
ThematicMap *CurTheme;
RootTexture *CurTex;
AnimDoubleTime *CurAnim;

if (Poly->FenceType == WCS_FENCEPIECE_SPAN)
	{
	CurTheme = GetEnabledTheme(WCS_EFFECTS_FENCE_THEME_SPANMATDRIVER);
	CurTex = GetEnabledTexture(WCS_EFFECTS_FENCE_TEXTURE_SPANMATDRIVER);
	CurAnim = &AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_SPANMATDRIVER];
	} // if
else
	{
	CurTheme = GetEnabledTheme(WCS_EFFECTS_FENCE_THEME_ROOFMATDRIVER);
	CurTex = GetEnabledTexture(WCS_EFFECTS_FENCE_TEXTURE_ROOFMATDRIVER);
	CurAnim = &AnimPar[WCS_EFFECTS_FENCE_ANIMPAR_ROOFMATDRIVER];
	} // else

	// evaluate thematic map
if (CurTheme && CurTheme->Eval(Value, Poly->Vector))
	{
	TempVal = Value[0] * .01;
	} // if
else
	TempVal = CurAnim->CurValue;
// gradient position
if (CurTex)
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	if ((TexOpacity = CurTex->Eval(Value, Rend->TransferTextureData(Poly))) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			// Value[0] has already been diminished by the texture's opacity
			TempVal = Value[0] + TempVal * (1.0 - TexOpacity);
			} // if
		else
			TempVal = Value[0];
		} // if
	} // if

return (TempVal);

} // Fence::GetRenderMatGradientPos

/*===========================================================================*/
/*===========================================================================*/

double MaterialEffect::EvalWaves(RenderData *Rend, VertexData *Vert)
{
double WaveAmp = 0.0;
EffectList *CurWave = Waves;

while (CurWave)
	{
	if (CurWave->Me->Enabled)
		WaveAmp += ((WaveEffect *)CurWave->Me)->EvalHeight(Rend, Vert);
	CurWave = CurWave->Next;
	} // while
// if displacement and texture evaluate them
if (AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DISPLACEMENT].CurValue > 0.0)
	{
	if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_DISPLACEMENT))
		WaveAmp += EvaluateDisplacement(Rend->TransferTextureData(Vert));
	} // if

return (WaveAmp);

} // MaterialEffect::EvalWaves

/*===========================================================================*/

double MaterialEffect::EvalWaves(RenderData *Rend, VectorNode *CurNode, Joe *RefVec, MaterialList *RefMat)
{
double WaveAmp = 0.0;
EffectList *CurWave = Waves;

while (CurWave)
	{
	if (CurWave->Me->Enabled)
		WaveAmp += ((WaveEffect *)CurWave->Me)->EvalHeight(Rend, CurNode, RefVec, RefMat);
	CurWave = CurWave->Next;
	} // while
// if displacement and texture evaluate them
if (AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DISPLACEMENT].CurValue > 0.0)
	{
	if (GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_DISPLACEMENT))
		WaveAmp += EvaluateDisplacement(Rend->TransferTextureData(CurNode, RefVec, RefMat));
	} // if

return (WaveAmp);

} // MaterialEffect::EvalWaves

/*===========================================================================*/
/*
int LakeEffect::EvaluateFoam(PixelData *Pix)
{
double Value[3], TexOpacity, Coverage, RefHeight, TransitionHeight = .25;
// calculate foam % from texture, wave height, water depth and wave reference height and depth
// evaluate foam color/texture and reduce by foam %
// increment Pix->OpacityUsed by foam %

if ((Coverage = WaterMat.AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_FOAMCOVERAGE].CurValue) > 0.0)
	{
	// take into account water depth, wave height
	RefHeight = WaterMat.AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WAVEREFHEIGHT].CurValue;

	// adjust reference wave height based on water depth, no adjustment if water depth > ref depth
	if (Pix->TexData->WaterDepth < WaterMat.AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WAVEREFDEPTH].CurValue)
		{
		RefHeight *= Pix->TexData->WaterDepth / WaterMat.AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WAVEREFDEPTH].CurValue;
		TransitionHeight *= Pix->TexData->WaterDepth / WaterMat.AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WAVEREFDEPTH].CurValue;
		} // if
	// there is a fixed .25 meter transition from no foam to full foam, adjusted by water depth
	// if wave height is between (RefHeight - TransitionHeight) and (RefHeight) diminish the coverage value
	if (Pix->TexData->WaveHeight > RefHeight - TransitionHeight)
		{
		if (Pix->TexData->WaveHeight < RefHeight)
			{
			// TransitionHeight must be > 0 in order to pass two conditional tests bracketing this equation
			Coverage *= (Pix->TexData->WaveHeight - (RefHeight - TransitionHeight)) / TransitionHeight;
			} // if
		} // if
	else
		return (0);

	// see if there is a texture to diminish coverage
	if (WaterMat.GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_FOAMCOVERAGE))
		{
		if ((TexOpacity = WaterMat.TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_FOAMCOVERAGE]->Eval(Value, Pix->TexData)) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				Coverage *= (1.0 - TexOpacity + Value[0]);
				} // if
			else
				Coverage *= Value[0];
			} // if
		} // if

	// if there's still some amount of foam after all that
	if (Coverage > 0.0)
		{
		// reduce coverage by any used opacity in the pixel
		Coverage *= (1.0 - Pix->OpacityUsed);

		if (WaterMat.GetEnabledTexture(WCS_EFFECTS_MATERIAL_TEXTURE_FOAMCOLOR))
			{
			if ((TexOpacity = WaterMat.TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_FOAMCOLOR]->Eval(Value, Pix->TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					TexOpacity = 1.0 - TexOpacity;
					Value[0] += WaterMat.FoamColor.GetCompleteValue(0) * TexOpacity;
					Value[1] += WaterMat.FoamColor.GetCompleteValue(1) * TexOpacity;
					Value[2] += WaterMat.FoamColor.GetCompleteValue(2) * TexOpacity;
					} // if
				} // if
			else
				{
				Value[0] = WaterMat.FoamColor.GetCompleteValue(0);
				Value[1] = WaterMat.FoamColor.GetCompleteValue(1);
				Value[2] = WaterMat.FoamColor.GetCompleteValue(2);
				} // else
			} // if
		else
			{
			Value[0] = WaterMat.FoamColor.GetCompleteValue(0);
			Value[1] = WaterMat.FoamColor.GetCompleteValue(1);
			Value[2] = WaterMat.FoamColor.GetCompleteValue(2);
			} // else

		// set foam color and opacity used
		Pix->RGB[0] += Value[0] * Coverage;
		Pix->RGB[1] += Value[1] * Coverage;
		Pix->RGB[2] += Value[2] * Coverage;
		Pix->OpacityUsed += Coverage;
		return (1);
		} // if
	} // if

// return 0 if no foam coverage
return (0);

} // LakeEffect::EvaluateFoam
*/
/*===========================================================================*/

double StreamEffect::GetRenderWaterMatGradientPos(RenderData *Rend, VertexData *Vert)
{
double Value[3], TempVal, TexOpacity;

// gradient position
if (GetEnabledTexture(WCS_EFFECTS_STREAM_TEXTURE_WATERMATDRIVER))
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	if ((TexOpacity = TexRoot[WCS_EFFECTS_STREAM_TEXTURE_WATERMATDRIVER]->Eval(Value, Rend->TransferTextureData(Vert))) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			// Value[0] has already been diminished by the texture's opacity
			TempVal = Value[0] + AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_WATERMATDRIVER].CurValue * (1.0 - TexOpacity);
			} // if
		else
			TempVal = Value[0];
		} // if
	else
		TempVal = AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_WATERMATDRIVER].CurValue;
	} // if
else
	TempVal = AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_WATERMATDRIVER].CurValue;

return (TempVal);

} // StreamEffect::GetRenderWaterMatGradientPos

/*===========================================================================*/

double StreamEffect::GetRenderWaterMatGradientPos(RenderData *Rend, Joe *RefVec, VectorNode *CurNode, MaterialList *RefMat)
{
double Value[3], TempVal, TexOpacity;

// gradient position
if (GetEnabledTexture(WCS_EFFECTS_STREAM_TEXTURE_WATERMATDRIVER))
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	if ((TexOpacity = TexRoot[WCS_EFFECTS_STREAM_TEXTURE_WATERMATDRIVER]->Eval(Value, Rend->TransferTextureData(CurNode, RefVec, RefMat))) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			// Value[0] has already been diminished by the texture's opacity
			TempVal = Value[0] + AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_WATERMATDRIVER].CurValue * (1.0 - TexOpacity);
			} // if
		else
			TempVal = Value[0];
		} // if
	else
		TempVal = AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_WATERMATDRIVER].CurValue;
	} // if
else
	TempVal = AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_WATERMATDRIVER].CurValue;

return (TempVal);

} // StreamEffect::GetRenderWaterMatGradientPos

/*===========================================================================*/

double StreamEffect::GetRenderWaterMatGradientPos(RenderData *Rend, PolygonData *Poly)
{
double Value[3], TempVal, TexOpacity;

// gradient position
if (GetEnabledTexture(WCS_EFFECTS_STREAM_TEXTURE_WATERMATDRIVER))
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	if ((TexOpacity = TexRoot[WCS_EFFECTS_STREAM_TEXTURE_WATERMATDRIVER]->Eval(Value, Rend->TransferTextureData(Poly))) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			// Value[0] has already been diminished by the texture's opacity
			TempVal = Value[0] + AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_WATERMATDRIVER].CurValue * (1.0 - TexOpacity);
			} // if
		else
			TempVal = Value[0];
		} // if
	else
		TempVal = AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_WATERMATDRIVER].CurValue;
	} // if
else
	TempVal = AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_WATERMATDRIVER].CurValue;

return (TempVal);

} // StreamEffect::GetRenderWaterMatGradientPos

/*===========================================================================*/

double StreamEffect::GetRenderBeachMatGradientPos(RenderData *Rend, PolygonData *Poly)
{
double Value[3], TempVal, TexOpacity;

// gradient position
if (GetEnabledTexture(WCS_EFFECTS_STREAM_TEXTURE_BEACHMATDRIVER))
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	if ((TexOpacity = TexRoot[WCS_EFFECTS_STREAM_TEXTURE_BEACHMATDRIVER]->Eval(Value, Rend->TransferTextureData(Poly))) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			// Value[0] has already been diminished by the texture's opacity
			TempVal = Value[0] + AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHMATDRIVER].CurValue * (1.0 - TexOpacity);
			} // if
		else
			TempVal = Value[0];
		} // if
	else
		TempVal = AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHMATDRIVER].CurValue;
	} // if
else
	TempVal = AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHMATDRIVER].CurValue;

return (TempVal);

} // StreamEffect::GetRenderBeachMatGradientPos

/*===========================================================================*/

double StreamEffect::GetRenderBeachMatGradientPos(RenderData *Rend, Joe *RefVec, VectorNode *CurNode, MaterialList *RefMat)
{
double Value[3], TempVal, TexOpacity;

// gradient position
if (GetEnabledTexture(WCS_EFFECTS_STREAM_TEXTURE_BEACHMATDRIVER))
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	if ((TexOpacity = TexRoot[WCS_EFFECTS_STREAM_TEXTURE_BEACHMATDRIVER]->Eval(Value, Rend->TransferTextureData(CurNode, RefVec, RefMat))) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			// Value[0] has already been diminished by the texture's opacity
			TempVal = Value[0] + AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHMATDRIVER].CurValue * (1.0 - TexOpacity);
			} // if
		else
			TempVal = Value[0];
		} // if
	else
		TempVal = AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHMATDRIVER].CurValue;
	} // if
else
	TempVal = AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_BEACHMATDRIVER].CurValue;

return (TempVal);

} // StreamEffect::GetRenderBeachMatGradientPos

/*===========================================================================*/

double LakeEffect::GetRenderWaterMatGradientPos(RenderData *Rend, VertexData *Vert)
{
double Value[3], TempVal, TexOpacity;

// evaluate thematic map
if (GetEnabledTheme(WCS_EFFECTS_LAKE_THEME_WATERMATDRIVER) &&
	Theme[WCS_EFFECTS_LAKE_THEME_WATERMATDRIVER]->Eval(Value, Vert->Vector))
	{
	TempVal = Value[0] * .01;
	} // if
else
	TempVal = AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_WATERMATDRIVER].CurValue;
// gradient position
if (GetEnabledTexture(WCS_EFFECTS_LAKE_TEXTURE_WATERMATDRIVER))
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	if ((TexOpacity = TexRoot[WCS_EFFECTS_LAKE_TEXTURE_WATERMATDRIVER]->Eval(Value, Rend->TransferTextureData(Vert))) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			// Value[0] has already been diminished by the texture's opacity
			TempVal = Value[0] + TempVal * (1.0 - TexOpacity);
			} // if
		else
			TempVal = Value[0];
		} // if
	} // if

return (TempVal);

} // LakeEffect::GetRenderWaterMatGradientPos

/*===========================================================================*/

double LakeEffect::GetRenderWaterMatGradientPos(RenderData *Rend, Joe *RefVec, VectorNode *CurNode, MaterialList *RefMat)
{
double Value[3], TempVal, TexOpacity;

// evaluate thematic map
if (GetEnabledTheme(WCS_EFFECTS_LAKE_THEME_WATERMATDRIVER) &&
	Theme[WCS_EFFECTS_LAKE_THEME_WATERMATDRIVER]->Eval(Value, RefVec))
	{
	TempVal = Value[0] * .01;
	} // if
else
	TempVal = AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_WATERMATDRIVER].CurValue;
// gradient position
if (GetEnabledTexture(WCS_EFFECTS_LAKE_TEXTURE_WATERMATDRIVER))
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	if ((TexOpacity = TexRoot[WCS_EFFECTS_LAKE_TEXTURE_WATERMATDRIVER]->Eval(Value, Rend->TransferTextureData(CurNode, RefVec, RefMat))) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			// Value[0] has already been diminished by the texture's opacity
			TempVal = Value[0] + TempVal * (1.0 - TexOpacity);
			} // if
		else
			TempVal = Value[0];
		} // if
	} // if

return (TempVal);

} // LakeEffect::GetRenderWaterMatGradientPos

/*===========================================================================*/

double LakeEffect::GetRenderWaterMatGradientPos(RenderData *Rend, PolygonData *Poly)
{
double Value[3], TempVal, TexOpacity;

// evaluate thematic map
if (GetEnabledTheme(WCS_EFFECTS_LAKE_THEME_WATERMATDRIVER) &&
	Theme[WCS_EFFECTS_LAKE_THEME_WATERMATDRIVER]->Eval(Value, Poly->Vector))
	{
	TempVal = Value[0] * .01;
	} // if
else
	TempVal = AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_WATERMATDRIVER].CurValue;
// gradient position
if (GetEnabledTexture(WCS_EFFECTS_LAKE_TEXTURE_WATERMATDRIVER))
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	if ((TexOpacity = TexRoot[WCS_EFFECTS_LAKE_TEXTURE_WATERMATDRIVER]->Eval(Value, Rend->TransferTextureData(Poly))) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			// Value[0] has already been diminished by the texture's opacity
			TempVal = Value[0] + TempVal * (1.0 - TexOpacity);
			} // if
		else
			TempVal = Value[0];
		} // if
	} // if

return (TempVal);

} // LakeEffect::GetRenderWaterMatGradientPos

/*===========================================================================*/

double LakeEffect::GetRenderBeachMatGradientPos(RenderData *Rend, PolygonData *Poly)
{
double Value[3], TempVal, TexOpacity;

// evaluate thematic map
if (GetEnabledTheme(WCS_EFFECTS_LAKE_THEME_BEACHMATDRIVER) &&
	Theme[WCS_EFFECTS_LAKE_THEME_BEACHMATDRIVER]->Eval(Value, Poly->Vector))
	{
	TempVal = Value[0] * .01;
	} // if
else
	TempVal = AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHMATDRIVER].CurValue;
// gradient position
if (GetEnabledTexture(WCS_EFFECTS_LAKE_TEXTURE_BEACHMATDRIVER))
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	if ((TexOpacity = TexRoot[WCS_EFFECTS_LAKE_TEXTURE_BEACHMATDRIVER]->Eval(Value, Rend->TransferTextureData(Poly))) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			// Value[0] has already been diminished by the texture's opacity
			TempVal = Value[0] + TempVal * (1.0 - TexOpacity);
			} // if
		else
			TempVal = Value[0];
		} // if
	} // if

return (TempVal);

} // LakeEffect::GetRenderBeachMatGradientPos

/*===========================================================================*/

double LakeEffect::GetRenderBeachMatGradientPos(RenderData *Rend, Joe *RefVec, VectorNode *CurNode, MaterialList *RefMat)
{
double Value[3], TempVal, TexOpacity;

// evaluate thematic map
if (GetEnabledTheme(WCS_EFFECTS_LAKE_THEME_BEACHMATDRIVER) &&
	Theme[WCS_EFFECTS_LAKE_THEME_BEACHMATDRIVER]->Eval(Value, RefVec))
	{
	TempVal = Value[0] * .01;
	} // if
else
	TempVal = AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_BEACHMATDRIVER].CurValue;
// gradient position
if (GetEnabledTexture(WCS_EFFECTS_LAKE_TEXTURE_BEACHMATDRIVER))
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	if ((TexOpacity = TexRoot[WCS_EFFECTS_LAKE_TEXTURE_BEACHMATDRIVER]->Eval(Value, Rend->TransferTextureData(CurNode, RefVec, RefMat))) > 0.0)
		{
		if (TexOpacity < 1.0)
			{
			// Value[0] has already been diminished by the texture's opacity
			TempVal = Value[0] + TempVal * (1.0 - TexOpacity);
			} // if
		else
			TempVal = Value[0];
		} // if
	} // if

return (TempVal);

} // LakeEffect::GetRenderBeachMatGradientPos

/*===========================================================================*/
/*===========================================================================*/

double WaveSource::EvalHeight(RenderData *Rend, VertexData *Vert, double RelX, double RelY, double CurTime, double EnvTimeOffset)
{
double WaveAmp, DistX, DistY, Dist, Phase, Frequency, WaveTime, t0, EnvStartDist, EnvFirstDist, EnvLastDist, EnvDist, 
	EnvAmp, TempVal, EnvWidth, Value[3], TexOpacity;

if (AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE].CurValue > 0.0)
	{
	// this could really be calculated once per time slice since it doesn't depend on any local variables
	Frequency = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_VELOCITY].CurValue / AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH].CurValue;
	Value[0] = Value[1] = Value[2] = 0.0;

	// calculate vertex distance from wave source
	DistX = RelX - AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETX].CurValue;
	DistY = RelY - AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETY].CurValue;
	Dist = sqrt(DistX * DistX + DistY * DistY);

	if (EnvelopeEnabled)
		{
		EnvDist = Dist;
		// this is for static envelopes used for making falloff from the source point
		if (AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPEVELOCITY].CurValue == 0.0)
			{
			if (Envelope.GetMinMaxDist(EnvFirstDist, EnvLastDist) && EnvLastDist > 0.0)
				{
				EnvWidth = EnvLastDist - EnvFirstDist;
				if (Dist > EnvLastDist)
					{
					if (RepeatEnvelopeAfter)
						EnvDist = EnvFirstDist + fmod((Dist - EnvFirstDist), EnvWidth);
					} // if
				else if (Dist < EnvFirstDist)
					{
					if (RepeatEnvelopeBefore)
						EnvDist = EnvLastDist - fmod((EnvLastDist - Dist), EnvWidth);
					} // if
				} // if

			if ((EnvAmp = Envelope.GetValue(0, EnvDist)) > 0.0)
				{
				TempVal = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_PHASE].CurValue;
				// analyze phase texture if one exists
				if (GetEnabledTexture(WCS_EFFECTS_WAVESOURCE_TEXTURE_PHASE))
					{
					if ((TexOpacity = TexRoot[WCS_EFFECTS_WAVESOURCE_TEXTURE_PHASE]->Eval(Value, Rend->TransferTextureData(Vert))) > 0.0)
						{
						if (TexOpacity < 1.0)
							{
							// Value[0] has already been diminished by the texture's opacity
							TempVal *= (1.0 - TexOpacity + Value[0]);
							} // if
						else
							TempVal *= Value[0];
						} // if
					} // if
				Phase = ((CurTime * Frequency) - (Dist / AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH].CurValue) + 
					TempVal) * TwoPi;
				WaveAmp = sin(Phase) * EnvAmp;
				} // if
			else
				return (0.0);
			} // if
		// this is for moving envelopes like would occur if a rock were dropped in the water
		else
			{
			t0 = EnvTimeOffset + AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPESTARTTIME].CurValue;
			WaveTime = CurTime - t0;
			EnvStartDist = WaveTime * AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPEVELOCITY].CurValue;

			if (Envelope.GetMinMaxDist(EnvFirstDist, EnvWidth) && EnvWidth > 0.0)
				{
				EnvFirstDist = EnvStartDist;
				EnvLastDist = EnvFirstDist - EnvWidth;
				if (Dist > EnvFirstDist)
					{
					if (RepeatEnvelopeBefore)
						EnvDist = EnvFirstDist - (EnvLastDist + fmod((Dist - EnvLastDist), EnvWidth));
					else
						EnvDist = 0.0;
					} // if
				else if (Dist < EnvLastDist)
					{
					if (RepeatEnvelopeAfter)
						EnvDist = fmod((EnvFirstDist - Dist), EnvWidth);
					else
						EnvDist = EnvWidth;
					} // if
				else
					EnvDist = EnvFirstDist - Dist;
				} // if
			if ((EnvAmp = Envelope.GetValue(0, EnvDist)) > 0.0)
				{
				TempVal = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_PHASE].CurValue;
				// analyze phase texture if one exists
				if (GetEnabledTexture(WCS_EFFECTS_WAVESOURCE_TEXTURE_PHASE))
					{
					if ((TexOpacity = TexRoot[WCS_EFFECTS_WAVESOURCE_TEXTURE_PHASE]->Eval(Value, Rend->TransferTextureData(Vert))) > 0.0)
						{
						if (TexOpacity < 1.0)
							{
							// Value[0] has already been diminished by the texture's opacity
							TempVal *= (1.0 - TexOpacity + Value[0]);
							} // if
						else
							TempVal *= Value[0];
						} // if
					} // if
				Phase = ((WaveTime * Frequency) - (Dist / AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH].CurValue) + 
					TempVal) * TwoPi;
				WaveAmp = sin(Phase) * EnvAmp;
				} // if
			else
				return (0.0);
			} // else
		} // if
	else
		{
		TempVal = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_PHASE].CurValue;
		// analyze phase texture if one exists
		if (GetEnabledTexture(WCS_EFFECTS_WAVESOURCE_TEXTURE_PHASE))
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_WAVESOURCE_TEXTURE_PHASE]->Eval(Value, Rend->TransferTextureData(Vert))) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					TempVal *= (1.0 - TexOpacity + Value[0]);
					} // if
				else
					TempVal *= Value[0];
				} // if
			} // if
		Phase = ((CurTime * Frequency) - (Dist / AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH].CurValue) + 
			TempVal) * TwoPi;
		WaveAmp = sin(Phase);
		} // else

	// scale the result and return
	TempVal = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE].CurValue;
	// analyze amplitude texture if one exists
	if (GetEnabledTexture(WCS_EFFECTS_WAVESOURCE_TEXTURE_AMPLITUDE))
		{
		if ((TexOpacity = TexRoot[WCS_EFFECTS_WAVESOURCE_TEXTURE_AMPLITUDE]->Eval(Value, Rend->TransferTextureData(Vert))) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				TempVal *= (1.0 - TexOpacity + Value[0]);
				} // if
			else
				TempVal *= Value[0];
			} // if
		} // if
	return (WaveAmp * TempVal);
	} // if amplitude > 0

return (0.0);

} // WaveSource::EvalHeight

/*===========================================================================*/

double WaveSource::EvalHeight(RenderData *Rend, VectorNode *CurNode, Joe *RefVec, MaterialList *RefMat, double RelX, double RelY, double CurTime, double EnvTimeOffset)
{
double WaveAmp, DistX, DistY, Dist, Phase, Frequency, WaveTime, t0, EnvStartDist, EnvFirstDist, EnvLastDist, EnvDist, 
	EnvAmp, TempVal, EnvWidth, Value[3], TexOpacity;

if (AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE].CurValue > 0.0)
	{
	// this could really be calculated once per time slice since it doesn't depend on any local variables
	Frequency = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_VELOCITY].CurValue / AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH].CurValue;
	Value[0] = Value[1] = Value[2] = 0.0;

	// calculate vertex distance from wave source
	DistX = RelX - AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETX].CurValue;
	DistY = RelY - AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETY].CurValue;
	Dist = sqrt(DistX * DistX + DistY * DistY);

	if (EnvelopeEnabled)
		{
		EnvDist = Dist;
		// this is for static envelopes used for making falloff from the source point
		if (AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPEVELOCITY].CurValue == 0.0)
			{
			if (Envelope.GetMinMaxDist(EnvFirstDist, EnvLastDist) && EnvLastDist > 0.0)
				{
				EnvWidth = EnvLastDist - EnvFirstDist;
				if (Dist > EnvLastDist)
					{
					if (RepeatEnvelopeAfter)
						EnvDist = EnvFirstDist + fmod((Dist - EnvFirstDist), EnvWidth);
					} // if
				else if (Dist < EnvFirstDist)
					{
					if (RepeatEnvelopeBefore)
						EnvDist = EnvLastDist - fmod((EnvLastDist - Dist), EnvWidth);
					} // if
				} // if

			if ((EnvAmp = Envelope.GetValue(0, EnvDist)) > 0.0)
				{
				TempVal = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_PHASE].CurValue;
				// analyze phase texture if one exists
				if (GetEnabledTexture(WCS_EFFECTS_WAVESOURCE_TEXTURE_PHASE))
					{
					if ((TexOpacity = TexRoot[WCS_EFFECTS_WAVESOURCE_TEXTURE_PHASE]->Eval(Value, Rend->TransferTextureData(CurNode, RefVec, RefMat))) > 0.0)
						{
						if (TexOpacity < 1.0)
							{
							// Value[0] has already been diminished by the texture's opacity
							TempVal *= (1.0 - TexOpacity + Value[0]);
							} // if
						else
							TempVal *= Value[0];
						} // if
					} // if
				Phase = ((CurTime * Frequency) - (Dist / AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH].CurValue) + 
					TempVal) * TwoPi;
				WaveAmp = sin(Phase) * EnvAmp;
				} // if
			else
				return (0.0);
			} // if
		// this is for moving envelopes like would occur if a rock were dropped in the water
		else
			{
			t0 = EnvTimeOffset + AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPESTARTTIME].CurValue;
			WaveTime = CurTime - t0;
			EnvStartDist = WaveTime * AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPEVELOCITY].CurValue;

			if (Envelope.GetMinMaxDist(EnvFirstDist, EnvWidth) && EnvWidth > 0.0)
				{
				EnvFirstDist = EnvStartDist;
				EnvLastDist = EnvFirstDist - EnvWidth;
				if (Dist > EnvFirstDist)
					{
					if (RepeatEnvelopeBefore)
						EnvDist = EnvFirstDist - (EnvLastDist + fmod((Dist - EnvLastDist), EnvWidth));
					else
						EnvDist = 0.0;
					} // if
				else if (Dist < EnvLastDist)
					{
					if (RepeatEnvelopeAfter)
						EnvDist = fmod((EnvFirstDist - Dist), EnvWidth);
					else
						EnvDist = EnvWidth;
					} // if
				else
					EnvDist = EnvFirstDist - Dist;
				} // if
			if ((EnvAmp = Envelope.GetValue(0, EnvDist)) > 0.0)
				{
				TempVal = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_PHASE].CurValue;
				// analyze phase texture if one exists
				if (GetEnabledTexture(WCS_EFFECTS_WAVESOURCE_TEXTURE_PHASE))
					{
					if ((TexOpacity = TexRoot[WCS_EFFECTS_WAVESOURCE_TEXTURE_PHASE]->Eval(Value, Rend->TransferTextureData(CurNode, RefVec, RefMat))) > 0.0)
						{
						if (TexOpacity < 1.0)
							{
							// Value[0] has already been diminished by the texture's opacity
							TempVal *= (1.0 - TexOpacity + Value[0]);
							} // if
						else
							TempVal *= Value[0];
						} // if
					} // if
				Phase = ((WaveTime * Frequency) - (Dist / AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH].CurValue) + 
					TempVal) * TwoPi;
				WaveAmp = sin(Phase) * EnvAmp;
				} // if
			else
				return (0.0);
			} // else
		} // if
	else
		{
		TempVal = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_PHASE].CurValue;
		// analyze phase texture if one exists
		if (GetEnabledTexture(WCS_EFFECTS_WAVESOURCE_TEXTURE_PHASE))
			{
			if ((TexOpacity = TexRoot[WCS_EFFECTS_WAVESOURCE_TEXTURE_PHASE]->Eval(Value, Rend->TransferTextureData(CurNode, RefVec, RefMat))) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					TempVal *= (1.0 - TexOpacity + Value[0]);
					} // if
				else
					TempVal *= Value[0];
				} // if
			} // if
		Phase = ((CurTime * Frequency) - (Dist / AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH].CurValue) + 
			TempVal) * TwoPi;
		WaveAmp = sin(Phase);
		} // else

	// scale the result and return
	TempVal = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE].CurValue;
	// analyze amplitude texture if one exists
	if (GetEnabledTexture(WCS_EFFECTS_WAVESOURCE_TEXTURE_AMPLITUDE))
		{
		if ((TexOpacity = TexRoot[WCS_EFFECTS_WAVESOURCE_TEXTURE_AMPLITUDE]->Eval(Value, Rend->TransferTextureData(CurNode, RefVec, RefMat))) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				TempVal *= (1.0 - TexOpacity + Value[0]);
				} // if
			else
				TempVal *= Value[0];
			} // if
		} // if
	return (WaveAmp * TempVal);
	} // if amplitude > 0

return (0.0);

} // WaveSource::EvalHeight

/*===========================================================================*/

double WaveSource::EvalCloudWaveHeight(CloudEffect *Cld, VertexCloud *Vert, TextureData *TexData, double RelX, double RelY, double CurTime)
{
double WaveAmp, DistX, DistY, Dist, Phase, Frequency, TempVal,
	Value[3], TexOpacity;

if (AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE].CurValue > 0.0)
	{
	// this could really be calculated once per time slice since it doesn't depend on any local variables
	Frequency = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_VELOCITY].CurValue / AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH].CurValue;
	Value[0] = Value[1] = Value[2] = 0.0;

	// calculate vertex distance from wave source
	DistX = RelX - AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETX].CurValue;
	DistY = RelY - AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETY].CurValue;
	Dist = sqrt(DistX * DistX + DistY * DistY);

	TempVal = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_PHASE].CurValue;
	// analyze phase texture if one exists
	if (GetEnabledTexture(WCS_EFFECTS_WAVESOURCE_TEXTURE_PHASE))
		{
		if ((TexOpacity = TexRoot[WCS_EFFECTS_WAVESOURCE_TEXTURE_PHASE]->Eval(Value, Cld->TransferTextureData(Vert, TexData))) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				TempVal *= (1.0 - TexOpacity + Value[0]);
				} // if
			else
				TempVal *= Value[0];
			} // if
		} // if
	Phase = ((CurTime * Frequency) - (Dist / AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH].CurValue) + 
		TempVal) * TwoPi;
	WaveAmp = sin(Phase);

	// scale the result and return
	TempVal = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE].CurValue;
	// analyze amplitude texture if one exists
	if (GetEnabledTexture(WCS_EFFECTS_WAVESOURCE_TEXTURE_AMPLITUDE))
		{
		if ((TexOpacity = TexRoot[WCS_EFFECTS_WAVESOURCE_TEXTURE_AMPLITUDE]->Eval(Value, Cld->TransferTextureData(Vert, TexData))) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				TempVal *= (1.0 - TexOpacity + Value[0]);
				} // if
			else
				TempVal *= Value[0];
			} // if
		} // if
	return (WaveAmp * TempVal);
	} // if amplitude > 0

return (0.0);

} // WaveSource::EvalCloudWaveHeight

/*===========================================================================*/

double WaveSource::EvalSampleCloudWaveHeight(double RelX, double RelY, double CurTime)
{
double WaveAmp, DistX, DistY, Dist, Phase, Frequency, TempVal;

if (AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE].CurValue > 0.0)
	{
	// this could really be calculated once per time slice since it doesn't depend on any local variables
	Frequency = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_VELOCITY].CurValue / AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH].CurValue;

	// calculate vertex distance from wave source
	DistX = RelX - AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETX].CurValue;
	DistY = RelY - AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETY].CurValue;
	Dist = sqrt(DistX * DistX + DistY * DistY);

	TempVal = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_PHASE].CurValue;
	// no analyze phase texture for samples
	Phase = ((CurTime * Frequency) - (Dist / AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH].CurValue) + 
		TempVal) * TwoPi;
	WaveAmp = sin(Phase);

	// scale the result and return
	TempVal = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE].CurValue;
	// no analyze amplitude texture for samples
	return (WaveAmp * TempVal);
	} // if amplitude > 0

return (0.0);

} // WaveSource::EvalSampleCloudWaveHeight

/*===========================================================================*/
/*===========================================================================*/

double WaveEffect::EvalHeight(RenderData *Rend, VertexData *Vert)
{
double WaveAmp = 0.0, TempAmp, RelX, RelY, lonscale, DistX, DistY, DistZ, EnvTimeOffset = 0.0, Value[3], TexOpacity;
long PtCt;
short WaveNum, SourceCt;
WaveSource *CurWave;
JoeList *CurJoe;
VectorPoint *PtLink;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM MyVert;

if (AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_AMPLITUDE].CurValue > 0.0 && WaveSourceArray)
	{
	// compute vertex offset from wave effect coords
	lonscale = cos(Vert->Lat * PiOver180) * Rend->EarthLatScaleMeters;
	RelY = (Vert->Lat - AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].CurValue) * Rend->EarthLatScaleMeters;
	RelX = (AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue - Vert->Lon) * lonscale;
	Value[2] = Value[1] = Value[0] = 0.0;

	if (CurJoe = Joes)
		{
		while (CurJoe)
			{
			if (CurJoe->Me)
				{
				TempAmp = 0.0;
				if (WaveEffectType == WCS_WAVE_WAVETYPE_POINT)
					{
					// apply sources at vector vertices
					if (PtLink = CurJoe->Me->GetFirstRealPoint())
						{
						if (MyAttr = (JoeCoordSys *)CurJoe->Me->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
							MyCoords = MyAttr->Coord;
						else
							MyCoords = NULL;
						for (PtCt = 0; PtLink; PtCt ++, PtLink = PtLink->Next)
							{
							if (PtLink->ProjToDefDeg(MyCoords, &MyVert))
								{
								RelY = (Vert->Lat - MyVert.Lat) * Rend->EarthLatScaleMeters;
								RelX = (MyVert.Lon - Vert->Lon) * lonscale;
								
								// seed the random number generator based on vertex number
								Rand.Seed64BitShift(PtCt * 1371 + WCS_SEEDOFFSET_WAVE, PtCt * 897 + WCS_SEEDOFFSET_WAVE);

								// determine a random envelope start time
								if (RandomizeEnvStart)
									{
									if (RandomizeType == WCS_WAVE_RANDOMIZETYPE_POISSON)
										{
										EnvTimeOffset = 2.0 * (Rand.GenPRN() - .5) * AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_ENVSTARTRANGE].CurValue;
										} // if
									else
										{
										EnvTimeOffset = Rand.GenGauss() * AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_ENVSTARTRANGE].CurValue;
										} // else
									} // if
								// add per vertex delay
								EnvTimeOffset += PtCt * AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_PERVERTEXENVDELAY].CurValue;

								// roll dice to see which wave sources get evaluated
								WaveNum = (short)(Rand.GenPRN() * NumSources);
								for (SourceCt = 0; SourceCt < WaveSourcesPerVertex; SourceCt ++, WaveNum ++)
									{
									if (WaveNum >= NumSources)
										WaveNum = 0;
									TempAmp += WaveSourceArray[WaveNum]->EvalHeight(Rend, Vert, RelX, RelY, Rend->RenderTime, EnvTimeOffset);
									} // for
								} // if
							} // for
						} // if
					} // if point style
				else
					{
					// find out if vertex coords are within joe outline
					if (CurJoe->Me->SimpleContained(Vert->Lat, Vert->Lon))
						{
						// evaluate each wave source
						CurWave = WaveSources;
						while (CurWave)
							{
							if (CurWave->Enabled)
								TempAmp += CurWave->EvalHeight(Rend, Vert, RelX, RelY, Rend->RenderTime, 0.0);
							CurWave = CurWave->Next;
							} // while
						// reduce amplitude if there is a gradient profile
						if (UseGradient)
							{
							// this undoubtedly isn't the most efficient way to get a distance but it
							// doesn't require georasters and it is more accurate than using georasters.
							DistX = fabs(CurJoe->Me->MinDistToPoint(Vert->Lat, Vert->Lon, Vert->Elev, Rend->EarthLatScaleMeters, 1, 0.0, 1.0, DistX, DistY, DistZ));
							TempAmp *= ADProf.GetValue(0, DistX);
							} // if
						} // if
					} // else line style
				WaveAmp += TempAmp;
				} // if
			CurJoe = CurJoe->Next;
			} // while
		} // if Joes
	else
		{
		// no Joes, treat same as if it is inbounds but no profile gradient
		// evaluate each wave source
		CurWave = WaveSources;
		while (CurWave)
			{
			if (CurWave->Enabled)
				WaveAmp += CurWave->EvalHeight(Rend, Vert, RelX, RelY, Rend->RenderTime, 0.0);
			CurWave = CurWave->Next;
			} // while
		} // else no Joes

	// scale wave amp by amplitude factor
	TempAmp = AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_AMPLITUDE].CurValue;
	// analyze amplitude texture if one exists
	if (GetEnabledTexture(WCS_EFFECTS_WAVE_TEXTURE_AMPLITUDE))
		{
		if ((TexOpacity = TexRoot[WCS_EFFECTS_WAVE_TEXTURE_AMPLITUDE]->Eval(Value, Rend->TransferTextureData(Vert))) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				TempAmp *= (1.0 - TexOpacity + Value[0]);
				} // if
			else
				TempAmp *= Value[0];
			} // if
		} // if
	return (WaveAmp * TempAmp);
	} // if amplitude > 0

return (0.0);

} // WaveEffect::EvalHeight

/*===========================================================================*/

double WaveEffect::EvalHeight(RenderData *Rend, VectorNode *CurNode, Joe *RefVec, MaterialList *RefMat)
{
double WaveAmp = 0.0, TempAmp, RelX, RelY, lonscale, DistX, DistY, DistZ, EnvTimeOffset = 0.0, Value[3], TexOpacity;
long PtCt;
short WaveNum, SourceCt;
WaveSource *CurWave;
JoeList *CurJoe;
VectorPoint *PtLink;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM MyVert;

if (AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_AMPLITUDE].CurValue > 0.0 && WaveSourceArray)
	{
	// compute vertex offset from wave effect coords
	lonscale = cos(CurNode->Lat * PiOver180) * Rend->EarthLatScaleMeters;
	RelY = (CurNode->Lat - AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].CurValue) * Rend->EarthLatScaleMeters;
	RelX = (AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue - CurNode->Lon) * lonscale;
	Value[0] = Value[1] = Value[2] = 0.0;

	if (CurJoe = Joes)
		{
		while (CurJoe)
			{
			if (CurJoe->Me)
				{
				TempAmp = 0.0;
				if (WaveEffectType == WCS_WAVE_WAVETYPE_POINT)
					{
					// apply sources at vector vertices
					if (PtLink = CurJoe->Me->GetFirstRealPoint())
						{
						if (MyAttr = (JoeCoordSys *)CurJoe->Me->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
							MyCoords = MyAttr->Coord;
						else
							MyCoords = NULL;
						for (PtCt = 0; PtLink; PtCt ++, PtLink = PtLink->Next)
							{
							if (PtLink->ProjToDefDeg(MyCoords, &MyVert))
								{
								RelY = (CurNode->Lat - MyVert.Lat) * Rend->EarthLatScaleMeters;
								RelX = (MyVert.Lon - CurNode->Lon) * lonscale;
								
								// seed the random number generator based on vertex number
								Rand.Seed64BitShift(PtCt * 1371 + WCS_SEEDOFFSET_WAVE, PtCt * 897 + WCS_SEEDOFFSET_WAVE);

								// determine a random envelope start time
								if (RandomizeEnvStart)
									{
									if (RandomizeType == WCS_WAVE_RANDOMIZETYPE_POISSON)
										{
										EnvTimeOffset = 2.0 * (Rand.GenPRN() - .5) * AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_ENVSTARTRANGE].CurValue;
										} // if
									else
										{
										EnvTimeOffset = Rand.GenGauss() * AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_ENVSTARTRANGE].CurValue;
										} // else
									} // if
								// add per vertex delay
								EnvTimeOffset += PtCt * AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_PERVERTEXENVDELAY].CurValue;

								// roll dice to see which wave sources get evaluated
								WaveNum = (short)(Rand.GenPRN() * NumSources);
								for (SourceCt = 0; SourceCt < WaveSourcesPerVertex; SourceCt ++, WaveNum ++)
									{
									if (WaveNum >= NumSources)
										WaveNum = 0;
									TempAmp += WaveSourceArray[WaveNum]->EvalHeight(Rend, CurNode, RefVec, RefMat, RelX, RelY, Rend->RenderTime, EnvTimeOffset);
									} // for
								} // if
							} // for
						} // if
					} // if point style
				else
					{
					// find out if vertex coords are within joe outline
					if (CurJoe->Me->SimpleContained(CurNode->Lat, CurNode->Lon))
						{
						// evaluate each wave source
						CurWave = WaveSources;
						while (CurWave)
							{
							if (CurWave->Enabled)
								TempAmp += CurWave->EvalHeight(Rend, CurNode, RefVec, RefMat, RelX, RelY, Rend->RenderTime, 0.0);
							CurWave = CurWave->Next;
							} // while
						// reduce amplitude if there is a gradient profile
						if (UseGradient)
							{
							// this undoubtedly isn't the most efficient way to get a distance but it
							// doesn't require georasters and it is more accurate than using georasters.
							DistX = fabs(CurJoe->Me->MinDistToPoint(CurNode->Lat, CurNode->Lon, CurNode->Elev, Rend->EarthLatScaleMeters, 1, 0.0, 1.0, DistX, DistY, DistZ));
							TempAmp *= ADProf.GetValue(0, DistX);
							} // if
						} // if
					} // else line style
				WaveAmp += TempAmp;
				} // if
			CurJoe = CurJoe->Next;
			} // while
		} // if Joes
	else
		{
		// no Joes, treat same as if it is inbounds but no profile gradient
		// evaluate each wave source
		CurWave = WaveSources;
		while (CurWave)
			{
			if (CurWave->Enabled)
				WaveAmp += CurWave->EvalHeight(Rend, CurNode, RefVec, RefMat, RelX, RelY, Rend->RenderTime, 0.0);
			CurWave = CurWave->Next;
			} // while
		} // else no Joes

	// scale wave amp by amplitude factor
	TempAmp = AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_AMPLITUDE].CurValue;
	// analyze amplitude texture if one exists
	if (GetEnabledTexture(WCS_EFFECTS_WAVE_TEXTURE_AMPLITUDE))
		{
		if ((TexOpacity = TexRoot[WCS_EFFECTS_WAVE_TEXTURE_AMPLITUDE]->Eval(Value, Rend->TransferTextureData(CurNode, RefVec, RefMat))) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// Value[0] has already been diminished by the texture's opacity
				TempAmp *= (1.0 - TexOpacity + Value[0]);
				} // if
			else
				TempAmp *= Value[0];
			} // if
		} // if
	return (WaveAmp * TempAmp);
	} // if amplitude > 0

return (0.0);

} // WaveEffect::EvalHeight

/*===========================================================================*/
/*===========================================================================*/

double MaterialStrata::ComputeTexture(double ElevRange, TextureData *TexData)
{
double ElPt, FirstFraction, LastFraction, LastEl, OrigEl, SumSamp, Value[3];
long TexEl, SumEl = 0, MacroSum, FirstPass = 1, FirstTexEl, LastTexEl;

#ifdef WCS_STRATA_SCALING
// changing the strata spacing by 2
ElevRange *= 0.5;  // Optimized out division. Was /= 2.0 
ElPt = TexData->Elev * .5;  // Optimized out division. Was / 2.0
#else // WCS_STRATA_SCALING
ElPt = TexData->Elev;
#endif // WCS_STRATA_SCALING

ElPt += (TexData->Latitude * AnimPar[WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_NORTHDIP].CurValue);
ElPt += (TexData->Longitude * AnimPar[WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_WESTDIP].CurValue);
if (GetEnabledTexture(WCS_EFFECTS_MATERIALSTRATA_TEXTURE_DEFORMATION))
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	TexRoot[WCS_EFFECTS_MATERIALSTRATA_TEXTURE_DEFORMATION]->Eval(Value, TexData);
	ElPt -= (AnimPar[WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_DEFORMSCALE].CurValue * Value[0]);
	} // if
ElPt += MakeNoise(50, fabs(TexData->Longitude), fabs(TexData->Latitude));
// reverse noise map orientation so features don't
// correspond with darkness noise below 
ElPt += 100000.0;	// negative numbers make poor array indices 
while (ElPt < 1200.0)
	ElPt += 100000.0;	// negative numbers make poor array indices 

OrigEl = ElPt;

if (ElevRange < -100.0)
	ElevRange = -100.0;
else if (ElevRange > 100.0)
	ElevRange = 100.0;

RepeatTex:

LastEl = ElPt + ElevRange;

if (ElevRange == 0.0)
	{
	TexEl = quickftol(WCS_ceil(ElPt));
	TexEl %= 1200;		// 1200 values in the array StrataTex[] 

	SumEl = StrataTex[TexEl];
	}
else if (ElevRange < 0.0)
	{
	TexEl = FirstTexEl = quickftol(WCS_ceil(ElPt));
	FirstFraction = 1.0 - (TexEl - ElPt);
	TexEl %= 1200;		// 1200 values in the array StrataTex[] 

	SumEl += quickftol(StrataTex[TexEl] * FirstFraction);
	TexEl --;
	ElPt -= (1.0 + FirstFraction);
	SumSamp = FirstFraction;

	LastTexEl = quickftol(WCS_ceil(LastEl));
	if (LastTexEl != FirstTexEl)
		{
		for (; ElPt>=LastEl; TexEl--, ElPt-=1.0)
			// The equal test is important since LastFraction can be 0 
			{
			if (TexEl < 0)
				TexEl = 1199;
			SumEl += StrataTex[TexEl];
			SumSamp += 1.0;
			} // for

		if (TexEl < 0)
			TexEl = 1199;
		LastFraction = LastTexEl - LastEl;
		SumEl += quickftol(StrataTex[TexEl] * LastFraction);
		SumSamp += LastFraction;
		} // if 

	SumEl = quickftol(SumEl / SumSamp);
	} // else if proceed in top down direction 
else
	{
	TexEl = FirstTexEl = quickftol(WCS_floor(ElPt));
	FirstFraction = 1.0 - (ElPt - TexEl);
	TexEl %= 1200;		// 1200 values in the array StrataTex[] 

	SumEl += quickftol(StrataTex[TexEl] * FirstFraction);
	TexEl ++;
	ElPt += (1.0 + FirstFraction);
	SumSamp = FirstFraction;

	LastTexEl = quickftol(WCS_floor(LastEl));
	if (LastTexEl != FirstTexEl)
		{
		for (; ElPt<=LastEl; TexEl++, ElPt+=1.0)
			// The equal test is important since LastFraction can be 0 
			{
			if (TexEl > 1199)
				TexEl = 0;
			SumEl += StrataTex[TexEl];
			SumSamp += 1.0;
			} // for

		if (TexEl > 1199)
			TexEl = 0;
		LastFraction = LastEl - LastTexEl;
		SumEl += quickftol((StrataTex[TexEl] * LastFraction));
		SumSamp += LastFraction;
		} // if 

	SumEl = quickftol(SumEl / SumSamp);
	} // else proceed bottom upwards 

// MicroStrata have about half the amplitude and double the frequency 

if (FirstPass)
	{
	ElPt = 2.0875423 * OrigEl;
	ElevRange *= 2.0875423;
	MacroSum = SumEl;
	SumEl = 0;
	FirstPass = 0;
	goto RepeatTex;
	} // if */

SumEl = quickftol(SumEl * .5 + MacroSum * .5);	//lint !e530
SumEl = (SumEl - 128) * 2;		// Compensating for the limited range of texture values

// returns value from 0 to 1 

return ((SumEl + MakeNoise((255 - SumEl) / 2, fabs(TexData->Latitude), fabs(TexData->Longitude))) * (1.0 / 255.0));

} // MaterialStrata::ComputeTexture() 

/*===========================================================================*/

double MaterialStrata::ComputeTextureColor(double ElevRange, double SetRGB[3], TextureData *TexData)
{
double ElPt, FirstFraction, LastFraction, LastEl, OrigEl, SumSamp, MacroSum[4], Value[3], SumRed = 0, SumGrn = 0, SumBlu = 0;
long TexEl, SumEl = 0, FirstPass = 1, FirstTexEl, LastTexEl;

ElPt = TexData->Elev;
ElPt += (TexData->Latitude * AnimPar[WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_NORTHDIP].CurValue);
ElPt += (TexData->Longitude * AnimPar[WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_WESTDIP].CurValue);
if (GetEnabledTexture(WCS_EFFECTS_MATERIALSTRATA_TEXTURE_DEFORMATION))
	{
	Value[0] = Value[1] = Value[2] = 0.0;
	TexRoot[WCS_EFFECTS_MATERIALSTRATA_TEXTURE_DEFORMATION]->Eval(Value, TexData);
	ElPt -= (AnimPar[WCS_EFFECTS_MATERIALSTRATA_ANIMPAR_DEFORMSCALE].CurValue * Value[0]);
	} // if
ElPt += MakeNoise(50, fabs(TexData->Longitude), fabs(TexData->Latitude));
// reverse noise map orientation so features don't
// correspond with darkness noise below 
ElPt += 100000.0;	// negative numbers make poor array indices 
while (ElPt < 1200.0)
	ElPt += 100000.0;	// negative numbers make poor array indices 

OrigEl = ElPt;

if (ElevRange < 0.0)
	{
	if (ElevRange < -100.0)
		ElevRange = -100.0;
	} // if
else if (ElevRange > 100.0)
	ElevRange = 100.0;

RepeatTex:

LastEl = ElPt + ElevRange;

if (ElevRange == 0.0)
	{
	TexEl = quickftol(WCS_ceil(ElPt));
	TexEl %= 1200;		// 1200 values in the array StrataTex[] 

	SumRed = StrataColor[StrataCol[TexEl]].GetCompleteValue(0);
	SumGrn = StrataColor[StrataCol[TexEl]].GetCompleteValue(1);
	SumBlu = StrataColor[StrataCol[TexEl]].GetCompleteValue(2);
	SumEl = StrataTex[TexEl];
	} // if
else if (ElevRange < 0.0)
	{
	TexEl = FirstTexEl = quickftol(WCS_ceil(ElPt));
	FirstFraction = 1.0 - (TexEl - ElPt);
	TexEl %= 1200;		// 1200 values in the array StrataTex[] 

	SumRed += (StrataColor[StrataCol[TexEl]].GetCompleteValue(0) * FirstFraction);
	SumGrn += (StrataColor[StrataCol[TexEl]].GetCompleteValue(1) * FirstFraction);
	SumBlu += (StrataColor[StrataCol[TexEl]].GetCompleteValue(2) * FirstFraction);
	SumEl += quickftol(StrataTex[TexEl] * FirstFraction);
	TexEl --;
	ElPt -= (1.0 + FirstFraction);
	SumSamp = FirstFraction;

	LastTexEl = quickftol(WCS_ceil(LastEl));
	if (LastTexEl != FirstTexEl)
		{
		for (; ElPt>=LastEl; TexEl--, ElPt-=1.0)
			// The equal test is important since LastFraction can be 0 
			{
			if (TexEl < 0)
				TexEl = 1199;
			SumRed += StrataColor[StrataCol[TexEl]].GetCompleteValue(0);
			SumGrn += StrataColor[StrataCol[TexEl]].GetCompleteValue(1);
			SumBlu += StrataColor[StrataCol[TexEl]].GetCompleteValue(2);
			SumEl += StrataTex[TexEl];
			SumSamp += 1.0;
			} // for

		if (TexEl < 0)
			TexEl = 1199;
		LastFraction = LastTexEl - LastEl;
		SumRed += (StrataColor[StrataCol[TexEl]].GetCompleteValue(0) * LastFraction);
		SumGrn += (StrataColor[StrataCol[TexEl]].GetCompleteValue(1) * LastFraction);
		SumBlu += (StrataColor[StrataCol[TexEl]].GetCompleteValue(2) * LastFraction);
		SumEl += quickftol(StrataTex[TexEl] * LastFraction);
		SumSamp += LastFraction;
		} // if 

		// temp scope to create multiplicative inverse variable
		{ //lint !e539
		double SumSampInv = 1.0 / SumSamp; // for multiplicative inverse, below
		SumRed *= SumSampInv;
		SumGrn *= SumSampInv;
		SumBlu *= SumSampInv;
		SumEl = quickftol(SumEl * SumSampInv);
		} 
	} // else if proceed in top down direction 
else
	{
	TexEl = FirstTexEl = quickftol(WCS_floor(ElPt));
	FirstFraction = 1.0 - (ElPt - TexEl);
	TexEl %= 1200;		// 1200 values in the array StrataTex[] 

	SumRed += (StrataColor[StrataCol[TexEl]].GetCompleteValue(0) * FirstFraction);
	SumGrn += (StrataColor[StrataCol[TexEl]].GetCompleteValue(1) * FirstFraction);
	SumBlu += (StrataColor[StrataCol[TexEl]].GetCompleteValue(2) * FirstFraction);
	SumEl += quickftol(StrataTex[TexEl] * FirstFraction);
	TexEl ++;
	ElPt += (1.0 + FirstFraction);
	SumSamp = FirstFraction;

	LastTexEl = quickftol(WCS_floor(LastEl));
	if (LastTexEl != FirstTexEl)
		{
		for (; ElPt<=LastEl; TexEl++, ElPt+=1.0)
			// The equal test is important since LastFraction can be 0 
			{
			if (TexEl > 1199)
				TexEl = 0;
			SumRed += StrataColor[StrataCol[TexEl]].GetCompleteValue(0);
			SumGrn += StrataColor[StrataCol[TexEl]].GetCompleteValue(1);
			SumBlu += StrataColor[StrataCol[TexEl]].GetCompleteValue(2);
			SumEl += StrataTex[TexEl];
			SumSamp += 1.0;
			} // for ... 

		if (TexEl > 1199)
			TexEl = 0;
		LastFraction = LastEl - LastTexEl;
		SumRed += (StrataColor[StrataCol[TexEl]].GetCompleteValue(0) * LastFraction);
		SumGrn += (StrataColor[StrataCol[TexEl]].GetCompleteValue(1) * LastFraction);
		SumBlu += (StrataColor[StrataCol[TexEl]].GetCompleteValue(2) * LastFraction);
		SumEl += quickftol(StrataTex[TexEl] * LastFraction);
		SumSamp += LastFraction;
		} // if 

		// temp scope to create multiplicative inverse variable
		{ //lint !e539
		double SumSampInv = 1.0 / SumSamp; // for multiplicative inverse, below
		SumRed *= SumSampInv;
		SumGrn *= SumSampInv;
		SumBlu *= SumSampInv;
		SumEl = quickftol(SumEl * SumSampInv);
		} // temp scope
	} // else proceed bottom upwards 

// MicroStrata have about half the amplitude and double the frequency 

if (FirstPass)
	{
	ElPt = 2.0875423 * OrigEl;
	ElevRange *= 2.0875423;
	MacroSum[0] = SumEl;
	MacroSum[1] = SumRed;
	MacroSum[2] = SumGrn;
	MacroSum[3] = SumBlu;
	SumEl = 0;
	SumRed = SumGrn = SumBlu = 0.0;
	FirstPass = 0;
	goto RepeatTex;
	} // if 

SumEl = quickftol(SumEl * .5 + MacroSum[0] * .5);	//lint !e530
SumEl = (SumEl - 128) * 2;		// Compensating for the limited range of texture values

SetRGB[0] = (SumRed * .5 + MacroSum[1] * .5);
SetRGB[1] = (SumGrn * .5 + MacroSum[2] * .5);
SetRGB[2] = (SumBlu * .5 + MacroSum[3] * .5);

// returns value from 0 to 1 
return ((SumEl + MakeNoise((255 - SumEl) / 2, fabs(TexData->Latitude), fabs(TexData->Longitude))) * (1.0 / 255.0));

} // MaterialStrata::ComputeTextureColor() 

/*===========================================================================*/
