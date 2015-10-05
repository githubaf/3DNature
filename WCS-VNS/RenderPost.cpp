// RenderPost.cpp
// Processing functions that are called in the World Construction Set 
// rendering cycle after all objects are rendered. Includes backgrounds, reflections and depth of field.
// Built from scratch 11/12/99 by Gary R. Huber.
// Copyright 1999 by Questar Productions. All rights reserved.

#include "stdafx.h"
#if defined(_OPENMP)
#include <omp.h>
#endif // _OPENMP
#include "Render.h"
#include "EffectsLib.h"
#include "Requester.h"
#include "RenderControlGUI.h"
#include "Useful.h"
#include "Raster.h"
#include "AppMem.h"
#include "PixelManager.h"
#include "PostProcessEvent.h"
#include "Lists.h"
#include "Toolbar.h" // to ask if we're in "Animation Play" mode when we put up our progress indicator

//#include "Log.h"

//#define WCS_EXPERIMENTAL_REFLECTIONS
//#define WCS_ZONESAMPLED_REFLECTIONS
//#define WCS_SPEEDKILLER_REFLECTIONS
//#define WCS_PRETTYSLOW_REFLECTIONS
//#define WCS_LIMIT3FRAGS_REFLECTIONS
//#define WCS_COMPRESSEDFRAGS_REFLECTIONS
//#define WCS_REFLECTION_LARGEARRAY

#ifdef WCS_REFLECTION_LARGEARRAY
	#define ReflArrayWidth	5
	#define FReflArrayWidth	3
	#define ArrayHalfWidth	2
	#define FArrayHalfWidth 1
	double ReflWt[ReflArrayWidth][ReflArrayWidth] = {
		.1,		.29,	.4,		.29,	.1,
		.29,	.74,	.875,	.74,	.29,
		.4,		.875,	1.0,	.875,	.4,
		.29,	.74,	.875,	.74,	.29,
		.1,		.29,	.4,		.29,	.1	};
	double FReflWt[FReflArrayWidth][FReflArrayWidth] = {.24, .65, .24, .65, 1.0, .65, .24, .65, .24};
#else
	#define ReflArrayWidth	3
	#define FReflArrayWidth	3
	#define ArrayHalfWidth	1
	#define FArrayHalfWidth 1
	double ReflWt[ReflArrayWidth][ReflArrayWidth] = {.24, .65, .24, .65, 1.0, .65, .24, .65, .24};
	double FReflWt[FReflArrayWidth][FReflArrayWidth] = {.24, .65, .24, .65, 1.0, .65, .24, .65, .24};
#endif

#ifdef WCS_BUILD_DEMO
extern unsigned long MemFuck;
#endif // WCS_BUILD_DEMO

int Renderer::RenderReflections(void)
{
double IncidentAngle, ReflStrength, XInc, YInc, XInc5, YInc5, InvReflStr, SumPts, NormalComponent,
	LastZ, CurZ, Red, Grn, Blu;
VertexDEM ViewVec, ReflPos, ReflVec, ReflRay, CenterReflVec;
PixelData Pix;
BusyWin *BWDE = NULL;
long X, Y, Ct, PixZip, SourceZip, i, j, sX, sY, sZip;
int Success = 1, ReflDone, InterPts, TestTweeners;

if (rPixelFragMap)
	{
	switch (Opt->ReflectionType)
		{
		case WCS_REFLECTIONSTYLE_BEAMTRACE:
			{
			return (RenderReflections_BEAMTRACE(rPixelFragMap));
			} // WCS_REFLECTIONSTYLE_BEAMTRACE
		case WCS_REFLECTIONSTYLE_BTFRAGS:
		default:
			{
			int NumThreads = 1;
#if defined(_OPENMP)
			#pragma omp parallel
				{
				NumThreads = omp_get_num_threads();
				} // #pragma omp parallel
			if (NumThreads > 2)
				{
				#pragma omp parallel sections
					{
					#pragma omp section
					if (! RenderReflections_BTFRAGS(rPixelFragMap, 0, Width / 4, true))
						Success = 0;
					#pragma omp section
					if (! RenderReflections_BTFRAGS(rPixelFragMap, Width / 4, 2 * (Width / 4), false))
						Success = 0;
					#pragma omp section
					if (! RenderReflections_BTFRAGS(rPixelFragMap, 2 * (Width / 4), 3 * (Width / 4), false))
						Success = 0;
					#pragma omp section
					if (! RenderReflections_BTFRAGS(rPixelFragMap, 3 * (Width / 4), Width, false))
						Success = 0;
					} // #pragma omp parallel sections
				} // if
			else
#endif // _OPENMP
				{
				#pragma omp parallel sections
					{
					#pragma omp section
					if (! RenderReflections_BTFRAGS(rPixelFragMap, 0, Width / 2, true))
						Success = 0;
					#pragma omp section
					if (Success && (! RenderReflections_BTFRAGS(rPixelFragMap, Width / 2, Width, NumThreads == 1)))
						Success = 0;
					} // #pragma omp parallel sections
				} // else
			return (Success);
			} // default
		} // switch
	} // if

if (! ReflectionBuf)
	return (1);

if (IsCamView)
	{
	BWDE = new BusyWin("Reflections", Height, 'BWDE', 0);
	} // if
else
	{
	Master->ProcInit(Height, "Reflections");
	} // else

for (Ct = 0; Ct < 3; Ct ++)
	{
	for (Y = PixZip = 0; Y < Height; Y ++)
		{
		for (X = 0; X < Width; X ++, PixZip ++)
			{
			if (ReflectionBuf[PixZip])
				{
				SumPts = 0.0;
				NormalComponent = 0;
				for (i = -ArrayHalfWidth, sY = Y - ArrayHalfWidth; i <= ArrayHalfWidth; i ++, sY ++)
					{
					if (sY < 0 || sY >= Height)
						continue;
					sZip = sY * Width + X - ArrayHalfWidth;
					for (j = -ArrayHalfWidth, sX = X - ArrayHalfWidth; j <= ArrayHalfWidth; j ++, sZip ++, sX ++)
						{
						if (sX < 0 || sX >= Width)
							continue;
						if (ReflectionBuf[sZip])
							{
							SumPts += ReflWt[i + ArrayHalfWidth][j + ArrayHalfWidth] * ReflectionBuf[sZip];
							NormalComponent += (NormalBuf[Ct][sZip] * ReflWt[i + ArrayHalfWidth][j + ArrayHalfWidth] * ReflectionBuf[sZip]);
							} // if
						} // for
					} // for
				if (SumPts > 0.0)
					{
					NormalComponent /= SumPts;
					CloudZBuf[PixZip] = (float)(NormalComponent);
					} // if
				} // if
			} // for
		} // for
	for (Y = PixZip = 0; Y < Height; Y ++)
		{
		for (X = 0; X < Width; X ++, PixZip ++)
			{
			if (ReflectionBuf[PixZip])
				{
				NormalBuf[Ct][PixZip] = CloudZBuf[PixZip];
				} // if
			} // for
		} // for
	} // for
for (Y = PixZip = 0; Y < Height; Y ++)
	{
	for (X = 0; X < Width; X ++, PixZip ++)
		{
		if (ReflectionBuf[PixZip])
			{
			NormalComponent = sqrt(NormalBuf[0][PixZip] * NormalBuf[0][PixZip] + NormalBuf[1][PixZip] * NormalBuf[1][PixZip] + NormalBuf[2][PixZip] * NormalBuf[2][PixZip]);
			NormalBuf[0][PixZip] = (float)(NormalBuf[0][PixZip] / NormalComponent);
			NormalBuf[1][PixZip] = (float)(NormalBuf[1][PixZip] / NormalComponent);
			NormalBuf[2][PixZip] = (float)(NormalBuf[2][PixZip] / NormalComponent);
			} // if
		} // for
	} // for

Pix.SetDefaults();

for (Y = PixZip = 0; Y < Height; Y ++)
	{
	for (X = 0; X < Width; X ++, PixZip ++)
		{
		if (ReflectionBuf[PixZip])
			{
			ReflStrength = ReflectionBuf[PixZip];

			// cast a ray through the pixel to find the xyz coords of the reflecting point
			ReflPos.ScrnXYZ[0] = X + .5;
			ReflPos.ScrnXYZ[1] = Y + .5;
			ReflPos.ScrnXYZ[2] = ReflPos.Q = ZBuf[PixZip];

			// turn screen coords into world coord xyz
			Cam->UnProjectVertexDEM(DefCoords, &ReflPos, EarthLatScaleMeters, PlanetRad, 0);	// 1 = no lat/lon/elev

			// find the view vector for the camera from the pixel point, this is the reversed view vector for that pixel
			ViewVec.CopyXYZ(Cam->CamPos);
			ViewVec.GetPosVector(&ReflPos);
			ViewVec.UnitVector();

			Pix.Normal[0] = NormalBuf[0][PixZip];
			Pix.Normal[1] = NormalBuf[1][PixZip];
			Pix.Normal[2] = NormalBuf[2][PixZip];
			IncidentAngle = VectorAngle(Pix.Normal, ViewVec.XYZ);

			// diminish reflection strength using Fresnell equation
			//ReflStrength *= FresnelReflectionCoef(IncidentAngle, 1.33);		// IndexWater / IndexAir = 1.33 / 1.0);
			ReflStrength *= FresnelReflectionCoefWater(IncidentAngle);		// hardcoded to water

			if (ReflStrength > 0.0)
				{
				ReflDone = 0;
				// find the reflection ray
				ReflVec.XYZ[0] = Pix.Normal[0] * 2.0 * IncidentAngle;
				ReflVec.XYZ[1] = Pix.Normal[1] * 2.0 * IncidentAngle;
				ReflVec.XYZ[2] = Pix.Normal[2] * 2.0 * IncidentAngle;
				ReflVec.GetPosVector(&ViewVec);	// subtracts Pixel to camera vector
				ReflVec.UnitVector();

				// create a ray from the reflection point
				ReflRay.CopyXYZ(&ReflPos);
				ReflRay.AddXYZ(&ReflVec);
				Cam->ProjectVertexDEM(DefCoords, &ReflRay, EarthLatScaleMeters, PlanetRad, 0);	// 0 = no lat/lon/elev

				if (ReflRay.ScrnXYZ[2] > ReflPos.ScrnXYZ[2])
					{
					XInc = ReflRay.ScrnXYZ[0] - ReflPos.ScrnXYZ[0];
					YInc = ReflRay.ScrnXYZ[1] - ReflPos.ScrnXYZ[1];
					// note that by using Fresnell eq and z discrimination above we elliminate 
					// cases where view vector and reflection vector are in line which would make XInc and YInc 0
					if (fabs(YInc) > fabs(XInc))
						{
						// normalize increments to 1 pixel of y
						XInc /= fabs(YInc);
						ReflVec.XYZ[0] /= fabs(YInc);
						ReflVec.XYZ[1] /= fabs(YInc);
						ReflVec.XYZ[2] /= fabs(YInc);
						YInc = YInc > 0.0 ? 1.0: -1.0;
						} // if
					else
						{
						// normalize increments to 1 pixel of x
						YInc /= fabs(XInc);
						ReflVec.XYZ[0] /= fabs(XInc);
						ReflVec.XYZ[1] /= fabs(XInc);
						ReflVec.XYZ[2] /= fabs(XInc);
						XInc = XInc > 0.0 ? 1.0: -1.0;
						} // else
					// initialize the reflection ray, 
					// note that we won't be using the z component so it is not initialized
					XInc5 = 20.0 * XInc;
					YInc5 = 20.0 * YInc;
					ReflRay.ScrnXYZ[0] = ReflPos.ScrnXYZ[0];
					ReflRay.ScrnXYZ[1] = ReflPos.ScrnXYZ[1];
					ReflRay.ScrnXYZ[0] += XInc5;
					ReflRay.ScrnXYZ[1] += YInc5;
					LastZ = ReflPos.ScrnXYZ[2];

					// walk towards edge of image looking for a spot in the image where the pixel plotted there
					// makes a lesser angle with the surface normal than the idealized reflection ray.
					// Walk at  pixel interval, we'll come back and check intermediate pixels later if necessary.
					while (! ReflDone && ReflRay.ScrnXYZ[0] >= 0.0 && ReflRay.ScrnXYZ[0] < Width
						&& ReflRay.ScrnXYZ[1] >= 0.0 && ReflRay.ScrnXYZ[1] < Height)
						{
						TestTweeners = 0;
						// where is the current source pixel
						SourceZip = (int)ReflRay.ScrnXYZ[1] * Width + (int)ReflRay.ScrnXYZ[0];
						CurZ = ZBuf[SourceZip];
						// only reflect pixels if they are farther away, otherwise intervening objects like trees might reflect
						if (LastZ >= ReflPos.ScrnXYZ[2])
							{
							ReflRay.ScrnXYZ[2] = ReflRay.Q = LastZ;

							// unproject the source point into XYZ world space
							Cam->UnProjectVertexDEM(DefCoords, &ReflRay, EarthLatScaleMeters, PlanetRad, 0);

							// find vector from reflection point to the current source point
							ReflRay.GetPosVector(&ReflPos);

							// test angle against incident angle (which is the same as the reflection angle
							if (VectorAngle(ReflRay.XYZ, Pix.Normal) >= IncidentAngle)
								{
								TestTweeners = 1;
								} // if
							} // if
						if (! TestTweeners)
							{
							if (CurZ >= ReflPos.ScrnXYZ[2])
								{
								ReflRay.ScrnXYZ[2] = ReflRay.Q = CurZ;

								// unproject the source point into XYZ world space
								Cam->UnProjectVertexDEM(DefCoords, &ReflRay, EarthLatScaleMeters, PlanetRad, 0);

								// find vector from reflection point to the current source point
								ReflRay.GetPosVector(&ReflPos);

								// test angle against incident angle (which is the same as the reflection angle
								if (VectorAngle(ReflRay.XYZ, Pix.Normal) >= IncidentAngle)
									{
									TestTweeners = 1;
									} // if
								} // else
							} // if
						if (TestTweeners)
							{
							ReflRay.ScrnXYZ[0] -= 19 * XInc;
							ReflRay.ScrnXYZ[1] -= 19 * YInc;
							InterPts = 0;
							while (InterPts < 20 && ReflRay.ScrnXYZ[0] >= 0.0 && ReflRay.ScrnXYZ[0] < Width
								&& ReflRay.ScrnXYZ[1] >= 0.0 && ReflRay.ScrnXYZ[1] < Height)
								{
								// where is the current source pixel
								SourceZip = (int)ReflRay.ScrnXYZ[1] * Width + (int)ReflRay.ScrnXYZ[0];
								// only reflect pixels if they are farther away, otherwise intervening objects like trees might reflect
								if (ZBuf[SourceZip] >= ReflPos.ScrnXYZ[2])
									{
									ReflRay.ScrnXYZ[2] = ReflRay.Q = ZBuf[SourceZip];

									// unproject the source point into XYZ world space
									Cam->UnProjectVertexDEM(DefCoords, &ReflRay, EarthLatScaleMeters, PlanetRad, 0);

									// find vector from reflection point to the current source point
									ReflRay.GetPosVector(&ReflPos);

									// test angle against incident angle (which is the same as the reflection angle
									if (VectorAngle(ReflRay.XYZ, Pix.Normal) >= IncidentAngle)
										{
										Pix.RGB[0] = Bitmap[0][SourceZip];
										Pix.RGB[1] = Bitmap[1][SourceZip];
										Pix.RGB[2] = Bitmap[2][SourceZip];
										ReflDone = 1;
										break;
										} // if
									} // if
								ReflRay.ScrnXYZ[0] += XInc;
								ReflRay.ScrnXYZ[1] += YInc;
								InterPts ++;
								} // while
							} // if
						ReflRay.ScrnXYZ[0] += XInc5;
						ReflRay.ScrnXYZ[1] += YInc5;
						LastZ = CurZ;
						} // while

					// we either found a reflective source point or went off the edge of the image.
					// If off the edge go back 19 intervals and march forward until edge hit again or found reflector
					if (! ReflDone)
						{
						ReflRay.ScrnXYZ[0] -= 19 * XInc;
						ReflRay.ScrnXYZ[1] -= 19 * YInc;
						InterPts = 0;
						while (InterPts < 19 && ReflRay.ScrnXYZ[0] >= 0.0 && ReflRay.ScrnXYZ[0] < Width
							&& ReflRay.ScrnXYZ[1] >= 0.0 && ReflRay.ScrnXYZ[1] < Height)
							{
							// where is the current source pixel
							SourceZip = (int)ReflRay.ScrnXYZ[1] * Width + (int)ReflRay.ScrnXYZ[0];
							// only reflect pixels if they are farther away, otherwise intervening objects like trees might reflect
							if (ZBuf[SourceZip] >= ReflPos.ScrnXYZ[2])
								{
								ReflRay.ScrnXYZ[2] = ReflRay.Q = ZBuf[SourceZip];

								// unproject the source point into XYZ world space
								Cam->UnProjectVertexDEM(DefCoords, &ReflRay, EarthLatScaleMeters, PlanetRad, 0);

								// find vector from reflection point to the current source point
								ReflRay.GetPosVector(&ReflPos);

								// test angle against incident angle (which is the same as the reflection angle
								if (VectorAngle(ReflRay.XYZ, Pix.Normal) >= IncidentAngle)
									{
									Pix.RGB[0] = Bitmap[0][SourceZip];
									Pix.RGB[1] = Bitmap[1][SourceZip];
									Pix.RGB[2] = Bitmap[2][SourceZip];
									ReflDone = 1;
									break;
									} // if
								} // if
							ReflRay.ScrnXYZ[0] += XInc;
							ReflRay.ScrnXYZ[1] += YInc;
							InterPts ++;
							} // while
						} // if not done
					} // if
				if (! ReflDone)
					{
					// No reflective source pixel was found in the image so get the sky color along the reflection ray.
					// Sky color is additive so initialize the color to 0
					Pix.RGB[0] = Pix.RGB[1] = Pix.RGB[2] = 0.0;
					GetSkyColor(Pix.RGB, ReflVec.XYZ, ReflPos.XYZ);
					if (Pix.RGB[0] > 1.0)
						Pix.RGB[0] = 1.0;
					if (Pix.RGB[1] > 1.0)
						Pix.RGB[1] = 1.0;
					if (Pix.RGB[2] > 1.0)
						Pix.RGB[2] = 1.0;
					Pix.RGB[0] *= 255.99;
					Pix.RGB[1] *= 255.99;
					Pix.RGB[2] *= 255.99;
					ReflDone = 1;
					} // if reflecting back towards camera or out of image

				if (ReflStrength > 1.0)
					ReflStrength = 1.0;
				InvReflStr = 1.0 - ReflStrength;

				Bitmap[0][PixZip] = (unsigned char)(Pix.RGB[0] * ReflStrength + Bitmap[0][PixZip] * InvReflStr);
				Bitmap[1][PixZip] = (unsigned char)(Pix.RGB[1] * ReflStrength + Bitmap[1][PixZip] * InvReflStr);
				Bitmap[2][PixZip] = (unsigned char)(Pix.RGB[2] * ReflStrength + Bitmap[2][PixZip] * InvReflStr);
				ScreenPixelPlot(Bitmap, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
				} // if reflection strength
			ReflectionBuf[PixZip] = (float)(ReflStrength);
			} // if
		} // for

	if (IsCamView)
		{
		if (BWDE)
			{
			if(BWDE->Update(Y + 1))
				{
				Success = 0;
				break;
				} // if
			} // if
		} // if
	else
		{
		Master->ProcUpdate(Y + 1);
		if (! Master->IsRunning())
			{
			Success = 0;
			break;
			} // if
		} // else
	} // for

if (CloudBitmap[0] && CloudBitmap[1] && CloudBitmap[2])
	{
	for (Y = PixZip = 0; Y < Height; Y ++)
		{
		for (X = 0; X < Width; X ++, PixZip ++)
			{
			if (ReflectionBuf[PixZip])
				{
				SumPts = 0.0;
				Red = Grn = Blu = 0.0;
				for (i = -FArrayHalfWidth, sY = Y - FArrayHalfWidth; i <= FArrayHalfWidth; i ++, sY ++)
					{
					if (sY < 0 || sY >= Height)
						continue;
					sZip = sY * Width + X - FArrayHalfWidth;
					for (j = -FArrayHalfWidth, sX = X - FArrayHalfWidth; j <= FArrayHalfWidth; j ++, sZip ++, sX ++)
						{
						if (sX < 0 || sX >= Width)
							continue;
						if (ReflectionBuf[sZip])
							{
							SumPts += FReflWt[i + FArrayHalfWidth][j + FArrayHalfWidth] * ReflectionBuf[sZip];
							Red += (Bitmap[0][sZip] * FReflWt[i + FArrayHalfWidth][j + FArrayHalfWidth] * ReflectionBuf[sZip]);
							Grn += (Bitmap[1][sZip] * FReflWt[i + FArrayHalfWidth][j + FArrayHalfWidth] * ReflectionBuf[sZip]);
							Blu += (Bitmap[2][sZip] * FReflWt[i + FArrayHalfWidth][j + FArrayHalfWidth] * ReflectionBuf[sZip]);
							} // if
						} // for
					} // for
				if (SumPts > 0.0)
					{
					CloudBitmap[0][PixZip] = (unsigned char)(Red / SumPts);
					CloudBitmap[1][PixZip] = (unsigned char)(Grn / SumPts);
					CloudBitmap[2][PixZip] = (unsigned char)(Blu / SumPts);
					} // if
				else
					{
					CloudBitmap[0][PixZip] = Bitmap[0][PixZip];
					CloudBitmap[1][PixZip] = Bitmap[1][PixZip];
					CloudBitmap[2][PixZip] = Bitmap[2][PixZip];
					} // else
				} // if
			} // for
		} // for
	for (Y = PixZip = 0; Y < Height; Y ++)
		{
		for (X = 0; X < Width; X ++, PixZip ++)
			{
			if (ReflectionBuf[PixZip])
				{
				Bitmap[0][PixZip] = (unsigned char)(CloudBitmap[0][PixZip] * ReflectionBuf[PixZip] + Bitmap[0][PixZip] * (1.0 - ReflectionBuf[PixZip]));
				Bitmap[1][PixZip] = (unsigned char)(CloudBitmap[1][PixZip] * ReflectionBuf[PixZip] + Bitmap[1][PixZip] * (1.0 - ReflectionBuf[PixZip]));
				Bitmap[2][PixZip] = (unsigned char)(CloudBitmap[2][PixZip] * ReflectionBuf[PixZip] + Bitmap[2][PixZip] * (1.0 - ReflectionBuf[PixZip]));
				} // if
			} // for
		} // for
	UpdatePreview();
	} // if

if (Master)
	Master->ProcClear();
if (BWDE)
	delete BWDE;

return (Success);

} // Renderer::RenderReflections

/*===========================================================================*/

int Renderer::RenderReflections_BEAMTRACE(rPixelHeader *FragMap)
{
int Success = 1, ReflDone, InterPts, TestTweeners;
long X, Y, PixZip, SourceZip;
unsigned long TempRGB[3];
double IncidentAngle, ReflStrength, XInc, YInc, XInc5, YInc5, InvReflStr, LastZ, CurZ, OrigColor[3];
VertexDEM ViewVec, ReflPos, ReflVec, ReflRay, CenterReflVec;
PixelData Pix;
BusyWin *BWDE = NULL;
rPixelFragment *CurFrag;

if (! ReflectionBuf)
	return (1);

if (IsCamView)
	{
	BWDE = new BusyWin("Reflections", Height, 'BWDE', 0);
	} // if
else
	{
	Master->ProcInit(Height, "Reflections");
	} // else

Pix.SetDefaults();

for (Y = PixZip = 0; Y < Height; Y ++)
	{
	for (X = 0; X < Width; X ++, PixZip ++)
		{
		if (CurFrag = FragMap[PixZip].GetFirstReflective())
			{
			for ( ; CurFrag; CurFrag = CurFrag->Next)
				{
				if (CurFrag->Refl)
					{
					ReflStrength = CurFrag->Refl->Reflect * (1.0 / 255.0);

					// cast a ray through the pixel to find the xyz coords of the reflecting point
					ReflPos.ScrnXYZ[0] = X + .5;
					ReflPos.ScrnXYZ[1] = Y + .5;
					ReflPos.ScrnXYZ[2] = ReflPos.Q = CurFrag->ZBuf;

					// turn screen coords into world coord xyz
					Cam->UnProjectVertexDEM(DefCoords, &ReflPos, EarthLatScaleMeters, PlanetRad, 0);	// 1 = no lat/lon/elev

					// find the view vector for the camera from the pixel point, this is the reversed view vector for that pixel
					ViewVec.CopyXYZ(Cam->CamPos);
					ViewVec.GetPosVector(&ReflPos);
					ViewVec.UnitVector();

					Pix.Normal[0] = CurFrag->Refl->Normal[0];
					Pix.Normal[1] = CurFrag->Refl->Normal[1];
					Pix.Normal[2] = CurFrag->Refl->Normal[2];
					IncidentAngle = VectorAngle(Pix.Normal, ViewVec.XYZ);

					// diminish reflection strength using Fresnell equation
					//ReflStrength *= FresnelReflectionCoef(IncidentAngle, 1.33);		// IndexWater / IndexAir = 1.33 / 1.0);
					ReflStrength *= FresnelReflectionCoefWater(IncidentAngle);		// hardcoded to water

					if (ReflStrength > 0.0)
						{
						ReflDone = 0;
						// find the reflection ray
						ReflVec.XYZ[0] = Pix.Normal[0] * 2.0 * IncidentAngle;
						ReflVec.XYZ[1] = Pix.Normal[1] * 2.0 * IncidentAngle;
						ReflVec.XYZ[2] = Pix.Normal[2] * 2.0 * IncidentAngle;
						ReflVec.GetPosVector(&ViewVec);	// subtracts Pixel to camera vector
						ReflVec.UnitVector();

						// create a ray from the reflection point
						ReflRay.CopyXYZ(&ReflPos);
						ReflRay.AddXYZ(&ReflVec);
						Cam->ProjectVertexDEM(DefCoords, &ReflRay, EarthLatScaleMeters, PlanetRad, 0);	// 0 = no lat/lon/elev

						if (ReflRay.ScrnXYZ[2] > ReflPos.ScrnXYZ[2])
							{
							XInc = ReflRay.ScrnXYZ[0] - ReflPos.ScrnXYZ[0];
							YInc = ReflRay.ScrnXYZ[1] - ReflPos.ScrnXYZ[1];
							// note that by using Fresnell eq and z discrimination above we elliminate 
							// cases where view vector and reflection vector are in line which would make XInc and YInc 0
							if (fabs(YInc) > fabs(XInc))
								{
								// normalize increments to 1 pixel of y
								XInc /= fabs(YInc);
								ReflVec.XYZ[0] /= fabs(YInc);
								ReflVec.XYZ[1] /= fabs(YInc);
								ReflVec.XYZ[2] /= fabs(YInc);
								YInc = YInc > 0.0 ? 1.0: -1.0;
								} // if
							else
								{
								// normalize increments to 1 pixel of x
								YInc /= fabs(XInc);
								ReflVec.XYZ[0] /= fabs(XInc);
								ReflVec.XYZ[1] /= fabs(XInc);
								ReflVec.XYZ[2] /= fabs(XInc);
								XInc = XInc > 0.0 ? 1.0: -1.0;
								} // else
							// initialize the reflection ray, 
							// note that we won't be using the z component so it is not initialized
							XInc5 = 20.0 * XInc;
							YInc5 = 20.0 * YInc;
							ReflRay.ScrnXYZ[0] = ReflPos.ScrnXYZ[0];
							ReflRay.ScrnXYZ[1] = ReflPos.ScrnXYZ[1];
							ReflRay.ScrnXYZ[0] += XInc5;
							ReflRay.ScrnXYZ[1] += YInc5;
							LastZ = ReflPos.ScrnXYZ[2];

							// walk towards edge of image looking for a spot in the image where the pixel plotted there
							// makes a lesser angle with the surface normal than the idealized reflection ray.
							// Walk at  pixel interval, we'll come back and check intermediate pixels later if necessary.
							while (! ReflDone && ReflRay.ScrnXYZ[0] >= 0.0 && ReflRay.ScrnXYZ[0] < Width
								&& ReflRay.ScrnXYZ[1] >= 0.0 && ReflRay.ScrnXYZ[1] < Height)
								{
								TestTweeners = 0;
								// where is the current source pixel
								SourceZip = (int)ReflRay.ScrnXYZ[1] * Width + (int)ReflRay.ScrnXYZ[0];
								CurZ = ZBuf[SourceZip];
								// only reflect pixels if they are farther away, otherwise intervening objects like trees might reflect
								if (LastZ >= ReflPos.ScrnXYZ[2])
									{
									ReflRay.ScrnXYZ[2] = ReflRay.Q = LastZ;

									// unproject the source point into XYZ world space
									Cam->UnProjectVertexDEM(DefCoords, &ReflRay, EarthLatScaleMeters, PlanetRad, 0);

									// find vector from reflection point to the current source point
									ReflRay.GetPosVector(&ReflPos);

									// test angle against incident angle (which is the same as the reflection angle
									if (VectorAngle(ReflRay.XYZ, Pix.Normal) >= IncidentAngle)
										{
										TestTweeners = 1;
										} // if
									} // if
								if (! TestTweeners)
									{
									if (CurZ >= ReflPos.ScrnXYZ[2])
										{
										ReflRay.ScrnXYZ[2] = ReflRay.Q = CurZ;

										// unproject the source point into XYZ world space
										Cam->UnProjectVertexDEM(DefCoords, &ReflRay, EarthLatScaleMeters, PlanetRad, 0);

										// find vector from reflection point to the current source point
										ReflRay.GetPosVector(&ReflPos);

										// test angle against incident angle (which is the same as the reflection angle
										if (VectorAngle(ReflRay.XYZ, Pix.Normal) >= IncidentAngle)
											{
											TestTweeners = 1;
											} // if
										} // else
									} // if
								if (TestTweeners)
									{
									ReflRay.ScrnXYZ[0] -= 19 * XInc;
									ReflRay.ScrnXYZ[1] -= 19 * YInc;
									InterPts = 0;
									while (InterPts < 20 && ReflRay.ScrnXYZ[0] >= 0.0 && ReflRay.ScrnXYZ[0] < Width
										&& ReflRay.ScrnXYZ[1] >= 0.0 && ReflRay.ScrnXYZ[1] < Height)
										{
										// where is the current source pixel
										SourceZip = (int)ReflRay.ScrnXYZ[1] * Width + (int)ReflRay.ScrnXYZ[0];
										// only reflect pixels if they are farther away, otherwise intervening objects like trees might reflect
										if (ZBuf[SourceZip] >= ReflPos.ScrnXYZ[2] && ! ReflectionBuf[SourceZip])
											{
											ReflRay.ScrnXYZ[2] = ReflRay.Q = ZBuf[SourceZip];

											// unproject the source point into XYZ world space
											Cam->UnProjectVertexDEM(DefCoords, &ReflRay, EarthLatScaleMeters, PlanetRad, 0);

											// find vector from reflection point to the current source point
											ReflRay.GetPosVector(&ReflPos);

											// test angle against incident angle (which is the same as the reflection angle
											if (VectorAngle(ReflRay.XYZ, Pix.Normal) >= IncidentAngle)
												{
												TempRGB[0] = Bitmap[0][SourceZip];
												TempRGB[1] = Bitmap[1][SourceZip];
												TempRGB[2] = Bitmap[2][SourceZip];
												if (ExponentBuf && ExponentBuf[SourceZip])
													{
													rPixelFragment::ExtractUnclippedExponentialColors(TempRGB, ExponentBuf[SourceZip]);
													} // if
												Pix.RGB[0] = TempRGB[0];
												Pix.RGB[1] = TempRGB[1];
												Pix.RGB[2] = TempRGB[2];
												ReflDone = 1;
												break;
												} // if
											} // if
										ReflRay.ScrnXYZ[0] += XInc;
										ReflRay.ScrnXYZ[1] += YInc;
										InterPts ++;
										} // while
									} // if
								ReflRay.ScrnXYZ[0] += XInc5;
								ReflRay.ScrnXYZ[1] += YInc5;
								LastZ = CurZ;
								} // while

							// we either found a reflective source point or went off the edge of the image.
							// If off the edge go back 19 intervals and march forward until edge hit again or found reflector
							if (! ReflDone)
								{
								ReflRay.ScrnXYZ[0] -= 19 * XInc;
								ReflRay.ScrnXYZ[1] -= 19 * YInc;
								InterPts = 0;
								while (InterPts < 19 && ReflRay.ScrnXYZ[0] >= 0.0 && ReflRay.ScrnXYZ[0] < Width
									&& ReflRay.ScrnXYZ[1] >= 0.0 && ReflRay.ScrnXYZ[1] < Height)
									{
									// where is the current source pixel
									SourceZip = (int)ReflRay.ScrnXYZ[1] * Width + (int)ReflRay.ScrnXYZ[0];
									// only reflect pixels if they are farther away, otherwise intervening objects like trees might reflect
									if (ZBuf[SourceZip] >= ReflPos.ScrnXYZ[2] && ! ReflectionBuf[SourceZip])
										{
										ReflRay.ScrnXYZ[2] = ReflRay.Q = ZBuf[SourceZip];

										// unproject the source point into XYZ world space
										Cam->UnProjectVertexDEM(DefCoords, &ReflRay, EarthLatScaleMeters, PlanetRad, 0);

										// find vector from reflection point to the current source point
										ReflRay.GetPosVector(&ReflPos);

										// test angle against incident angle (which is the same as the reflection angle
										if (VectorAngle(ReflRay.XYZ, Pix.Normal) >= IncidentAngle)
											{
											TempRGB[0] = Bitmap[0][SourceZip];
											TempRGB[1] = Bitmap[1][SourceZip];
											TempRGB[2] = Bitmap[2][SourceZip];
											if (ExponentBuf && ExponentBuf[SourceZip])
												{
												rPixelFragment::ExtractUnclippedExponentialColors(TempRGB, ExponentBuf[SourceZip]);
												} // if
											Pix.RGB[0] = TempRGB[0];
											Pix.RGB[1] = TempRGB[1];
											Pix.RGB[2] = TempRGB[2];
											ReflDone = 1;
											break;
											} // if
										} // if
									ReflRay.ScrnXYZ[0] += XInc;
									ReflRay.ScrnXYZ[1] += YInc;
									InterPts ++;
									} // while
								} // if not done
							} // if
						if (! ReflDone)
							{
							// No reflective source pixel was found in the image so get the sky color along the reflection ray.
							// Sky color is additive so initialize the color to 0
							Pix.RGB[0] = Pix.RGB[1] = Pix.RGB[2] = 0.0;
							GetSkyColor(Pix.RGB, ReflVec.XYZ, ReflPos.XYZ);
							} // if reflecting back towards camera or out of image
						else
							{
							// color came from bitmap so must be divided by 255
							Pix.RGB[0] *= (1.0 / 255.0);
							Pix.RGB[1] *= (1.0 / 255.0);
							Pix.RGB[2] *= (1.0 / 255.0);
							} // else

						if (ReflStrength > 1.0)
							ReflStrength = 1.0;
						InvReflStr = 1.0 - ReflStrength;

						CurFrag->ExtractColor(OrigColor);
						OrigColor[0] = (Pix.RGB[0] * ReflStrength + OrigColor[0] * InvReflStr);
						OrigColor[1] = (Pix.RGB[1] * ReflStrength + OrigColor[1] * InvReflStr);
						OrigColor[2] = (Pix.RGB[2] * ReflStrength + OrigColor[2] * InvReflStr);
						CurFrag->PlotPixel(OrigColor);
						CurFrag->Refl->Reflect = (unsigned char)(ReflStrength * 255.99);
						} // if reflection strength
					} // if fragment is reflective
				} // for each fragment
			// replot pixel to screen
			ScreenPixelPlotFragments(&FragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);
			} // if reflections on this pixel
		} // for

	if (IsCamView)
		{
		if (BWDE)
			{
			if(BWDE->Update(Y + 1))
				{
				Success = 0;
				break;
				} // if
			} // if
		} // if
	else
		{
		Master->ProcUpdate(Y + 1);
		if (! Master->IsRunning())
			{
			Success = 0;
			break;
			} // if
		} // else
	} // for

if (Master)
	Master->ProcClear();
if (BWDE)
	delete BWDE;

return (Success);

} // Renderer::RenderReflections_BEAMTRACE

/*===========================================================================*/

#ifdef SPEED_MOD
int FASTCALL Renderer::RenderReflections_BTFRAGS(rPixelHeader *const FragMap, long StartCol, long EndCol, bool ShowProgress)
#else // SPEED_MOD
int Renderer::RenderReflections_BTFRAGS(rPixelHeader *const FragMap, long StartCol, long EndCol, bool ShowProgress)
#endif // SPEED_MOD
{
double IncidentAngle, ReflStrength, XInc, YInc, InvReflStr, PixelSize, InvPixelSize, OrigColor[3], 
	SumCovg, TempCovg, FragRGB[3], ZBandMinus, ZBandPlus, ScreenDistX, ScreenDistY, TotalScreenDist,
	ScreenDistStep, InverseReflPtZ, InverseReflRayZ, InverseZDiff, InverseZ, InverseZIncr, BandCenterZ, CovgWt, 
	LastBandCenterZ, NextBandCenterZ;
VertexDEM ViewVec, ReflPos, ReflVec, ReflRay, TempVec, TestRay;
PixelData Pix;
BusyWin *BWDE = NULL;
rPixelFragment *CurFrag, *TestFrag;
struct PixelFragSort *FragSortList;
#ifdef SPEED_MOD
double SumIncSq;
long XX;
#endif // SPEED_MOD
float TestFragZBuf;
unsigned char CurFragType;
int Success = 1, ReflDone, FragX, FragY, CloudFound, FragIsCloud, SkyFound, FragIsSky, NoMoreClouds;
const long MaxListFrags = 20;
long X, Y, PixZip, SourceZip, FragSortListCt, Ct;

//StartHiResTimer();

if (! ReflectionBuf)
	return (1);

if (ShowProgress)
	{
	if (IsCamView)
		{
		BWDE = new BusyWin("Reflections", EndCol - StartCol, 'BWDE', 0);
		} // if
	else
		{
		Master->ProcInit(EndCol - StartCol, "Reflections");
		} // else
	} // if
	
Pix.SetDefaults();

if (! (FragSortList = (struct PixelFragSort *)AppMem_Alloc(MaxListFrags * sizeof(struct PixelFragSort), APPMEM_CLEAR)))
	return (0);

for (X = StartCol; X < EndCol; X ++)
	{
	PixZip = X;
#ifdef SPEED_MOD
	XX = X + DrawOffsetX;
#endif // SPEED_MOD
	for (Y = 0; Y < Height; Y++, PixZip += Width)
		{
		if (CurFrag = FragMap[PixZip].GetFirstReflective())
			{
			for ( ; CurFrag; CurFrag = CurFrag->Next)
				{
				if (CurFrag->Refl)
					{
					CurFragType = CurFrag->TestFlags(WCS_PIXELFRAG_FLAGBIT_OBJECTYPE);
					ReflStrength = CurFrag->Refl->Reflect * (1.0 / 255.0);

					// cast a ray through the pixel to find the xyz coords of the reflecting point
					ReflPos.ScrnXYZ[0] = X + .5;
					ReflPos.ScrnXYZ[1] = Y + .5;
					ReflPos.ScrnXYZ[2] = ReflPos.Q = CurFrag->ZBuf;

					// turn screen coords into world coord xyz
					Cam->UnProjectVertexDEM(DefCoords, &ReflPos, EarthLatScaleMeters, PlanetRad, 0);	// 1 = no lat/lon/elev

					// find the view vector for the camera from the pixel point, this is the reversed view vector for that pixel
					ViewVec.CopyXYZ(Cam->CamPos);
					ViewVec.GetPosVector(&ReflPos);
					ViewVec.UnitVector();

					Pix.Normal[0] = CurFrag->Refl->Normal[0];
					Pix.Normal[1] = CurFrag->Refl->Normal[1];
					Pix.Normal[2] = CurFrag->Refl->Normal[2];
					IncidentAngle = VectorAngle(Pix.Normal, ViewVec.XYZ);

					// diminish reflection strength using Fresnell equation
					//ReflStrength *= FresnelReflectionCoef(IncidentAngle, 1.33);		// IndexWater / IndexAir = 1.33 / 1.0);
					ReflStrength *= FresnelReflectionCoefWater(IncidentAngle);		// hardcoded to water

					if (ReflStrength > 0.0)
						{
						double IncidentAngle2 = 2.0 * IncidentAngle;

						ReflDone = 0;
						// find the reflection ray
						ReflVec.XYZ[0] = Pix.Normal[0] * IncidentAngle2;
						ReflVec.XYZ[1] = Pix.Normal[1] * IncidentAngle2;
						ReflVec.XYZ[2] = Pix.Normal[2] * IncidentAngle2;
						ReflVec.GetPosVector(&ViewVec);	// subtracts Pixel to camera vector
						ReflVec.UnitVector();

						// create a ray from the reflection point
						ReflVec.XYZ[0] *= 10000000.0;
						ReflVec.XYZ[1] *= 10000000.0;
						ReflVec.XYZ[2] *= 10000000.0;
						ReflRay.CopyXYZ(&ReflPos);
						ReflRay.AddXYZ(&ReflVec);
						Cam->ProjectVertexDEM(DefCoords, &ReflRay, EarthLatScaleMeters, PlanetRad, 0);	// 0 = no lat/lon/elev

						SumCovg = 0.0;
						FragSortListCt = 0;
						if (ReflRay.ScrnXYZ[2] > ReflPos.ScrnXYZ[2])
							{
							PixelSize = CenterPixelSize * CurFrag->ZBuf;
							InvPixelSize = 1.0 / PixelSize;
							XInc = ReflRay.ScrnXYZ[0] - ReflPos.ScrnXYZ[0];
							YInc = ReflRay.ScrnXYZ[1] - ReflPos.ScrnXYZ[1];
							// note that by using Fresnell eq and z discrimination above we elliminate 
							// cases where view vector and reflection vector are in line which would make XInc and YInc 0
							if (fabs(YInc) > fabs(XInc))
								{
								// normalize increments to 1 pixel of y
								XInc /= fabs(YInc);
#ifdef SPEED_MOD
								SumIncSq = XInc * XInc;
#endif // SPEED_MOD
								YInc = YInc > 0.0 ? 1.0: -1.0;
								} // if
							else
								{
								// normalize increments to 1 pixel of x
								YInc /= fabs(XInc);
#ifdef SPEED_MOD
								SumIncSq = YInc * YInc;
#endif // SPEED_MOD
								XInc = XInc > 0.0 ? 1.0: -1.0;
								} // else

							// find ratio of one pixel of screen offset to total screen distance between two vertex projections
							ScreenDistX = ReflRay.ScrnXYZ[0] - ReflPos.ScrnXYZ[0];
							ScreenDistY = ReflRay.ScrnXYZ[1] - ReflPos.ScrnXYZ[1];
#ifndef SPEED_MOD
							TotalScreenDist = sqrt(ScreenDistX * ScreenDistX + ScreenDistY * ScreenDistY);
							ScreenDistStep = sqrt(XInc * XInc + YInc * YInc);
							ScreenDistStep /= TotalScreenDist;
#else // !SPEED_MOD
							TotalScreenDist = ScreenDistX * ScreenDistX + ScreenDistY * ScreenDistY;
							ScreenDistStep = sqrt((SumIncSq + 1.0) / TotalScreenDist);	// one of the increments squared will always be 1.0
#endif // !SPEED_MOD
							// find inverse of reflection point Z and the other projected vertex on the ray and their difference
							InverseReflPtZ = 1.0 / ReflPos.ScrnXYZ[2];
							InverseReflRayZ = 1.0 / ReflRay.ScrnXYZ[2];
							InverseZDiff = InverseReflRayZ - InverseReflPtZ;

							// initialize inverse Z
							InverseZ = InverseReflPtZ;
							InverseZIncr = InverseZDiff * ScreenDistStep;

							// initialize the reflection ray, 
							ReflRay.ScrnXYZ[0] = ReflPos.ScrnXYZ[0];
							ReflRay.ScrnXYZ[1] = ReflPos.ScrnXYZ[1];
							ReflRay.ScrnXYZ[0] += XInc;
							ReflRay.ScrnXYZ[1] += YInc;
							InverseZ += InverseZIncr;
							LastBandCenterZ = ReflPos.ScrnXYZ[2];

							SkyFound = 0;
							NoMoreClouds = 0;
							CloudFound = 0;

							// walk towards edge of image looking for a spot in the image where the pixel plotted there
							// makes a lesser angle with the surface normal than the idealized reflection ray.
							// Walk at  pixel interval.
							while (! ReflDone && ReflRay.ScrnXYZ[0] >= 0.0 && ReflRay.ScrnXYZ[0] < Width
								&& ReflRay.ScrnXYZ[1] >= 0.0 && ReflRay.ScrnXYZ[1] < Height 
								&& FragSortListCt < MaxListFrags && SumCovg < 1.0 && InverseZ >= 0.0)
								{
								FragX = (int)ReflRay.ScrnXYZ[0];
								FragY = (int)ReflRay.ScrnXYZ[1];
								BandCenterZ = 1.0 / InverseZ;
								ZBandMinus = BandCenterZ - LastBandCenterZ;
								NextBandCenterZ = InverseZ + InverseZIncr > 0.0 ? 1.0 / (InverseZ + InverseZIncr): 10000000.0;
								ZBandPlus = NextBandCenterZ > BandCenterZ ? NextBandCenterZ - BandCenterZ: ZBandMinus;
								LastBandCenterZ = BandCenterZ;

								SourceZip = FragY * Width + FragX;
								if (FragMap[SourceZip].UsedFrags)
									{
									// test first frag to see if it is less than the far Z range value
									TestFrag = FragMap[SourceZip].FragList;
									TestFragZBuf = TestFrag->ZBuf;
									if (TestFragZBuf <= BandCenterZ + ZBandPlus)
										{
										while (TestFrag && SumCovg < 1.0 && FragSortListCt < MaxListFrags)
											{
											FragIsCloud = 0;
											FragIsSky = 0;
											if ((! TestFrag->Refl || TestFrag->TestFlags(WCS_PIXELFRAG_FLAGBIT_OBJECTYPE) != CurFragType) && TestFrag->Alpha)
												{
												TestFragZBuf = TestFrag->ZBuf;
												if (TestFragZBuf >= BandCenterZ - ZBandMinus)
													{
													if (TestFragZBuf > BandCenterZ + ZBandPlus)
														break;
													if (TestFrag->TestFlags(WCS_PIXELFRAG_FLAGBIT_OBJECTYPE) == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_CLOUD)
														{
														if (NoMoreClouds)
															goto SkipCloud;
														CloudFound = 1;
														FragIsCloud = 1;
														} // if
													if (TestFrag->TestFlags(WCS_PIXELFRAG_FLAGBIT_OBJECTYPE) == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_SKY
														|| TestFrag->TestFlags(WCS_PIXELFRAG_FLAGBIT_OBJECTYPE) == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_CELESTIAL)
														{
														if (SkyFound)
															goto SkipCloud;
														SkyFound = 1;
														FragIsSky = 1;
														} // if
													// add fragment to list
													// weight coverage by fraction of distance from center of z band
													if (TestFragZBuf >= BandCenterZ)
														CovgWt = FragIsCloud || FragIsSky ? 1.0: 1.0 - (TestFragZBuf - BandCenterZ) / ZBandPlus;
													else
														CovgWt = FragIsCloud || FragIsSky ? 1.0: 1.0 - (BandCenterZ - TestFragZBuf) / ZBandMinus;
													SumCovg += AddReflFragEntry(FragSortList, FragSortListCt, CovgWt, TestFrag, ! (FragIsSky));
													} // if
												} // if
											SkipCloud: 
											// the test for incident angle is because at very low incident angles there may be a pixel at the water edge 
											// that can't reflect anything from pixels above it so let it reflect water bottom if there is any
											if (IncidentAngle > .002 && TestFrag->TestFlags(WCS_PIXELFRAG_FLAGBIT_LASTREFLECTABLE))
												break;
											TestFrag = TestFrag->Next;
											} // while
										} // if
									} // if
								ReflRay.ScrnXYZ[0] += XInc;
								ReflRay.ScrnXYZ[1] += YInc;
								if (CloudFound)
									NoMoreClouds = 1;
								InverseZ += InverseZIncr;
								} // while
							if (FragSortListCt > 0)
								{
								qsort((void *)FragSortList, (size_t)FragSortListCt, (size_t)(sizeof (struct PixelFragSort)), ComparePixelFragSort);
								Pix.RGB[0] = Pix.RGB[1] = Pix.RGB[2] = 0.0;
								SumCovg = 0.0;
								for (Ct = 0; Ct < FragSortListCt && SumCovg < 1.0; Ct ++)
									{
									FragSortList[Ct].Frag->ExtractColor(FragRGB);
									TempCovg = SumCovg;
									SumCovg += FragSortList[Ct].Covg;
									if (SumCovg > 1.0)
										{
										SumCovg = 1.0;
										FragSortList[Ct].Covg = 1.0 - TempCovg;
										} // if
									Pix.RGB[0] += FragRGB[0] * FragSortList[Ct].Covg;
									Pix.RGB[1] += FragRGB[1] * FragSortList[Ct].Covg;
									Pix.RGB[2] += FragRGB[2] * FragSortList[Ct].Covg;
									} // for
								if (SumCovg > 0.0)
									{
									if (SumCovg < 1.0)
										{
										SumCovg = 1.0 / SumCovg;
										Pix.RGB[0] *= SumCovg;
										Pix.RGB[1] *= SumCovg;
										Pix.RGB[2] *= SumCovg;
										} // if
									ReflDone = 1;
									} // if
								} // if
							} // if
						if (! ReflDone)
							{
							// No reflective source pixel was found in the image so get the sky color along the reflection ray.
							// Sky color is additive so initialize the color to 0
							Pix.RGB[0] = Pix.RGB[1] = Pix.RGB[2] = 0.0;
							GetSkyColor(Pix.RGB, ReflVec.XYZ, ReflPos.XYZ);
							} // if reflecting back towards camera or out of image

						if (ReflStrength > 1.0)
							ReflStrength = 1.0;
						InvReflStr = 1.0 - ReflStrength;

						CurFrag->ExtractColor(OrigColor);
						OrigColor[0] = (Pix.RGB[0] * ReflStrength + OrigColor[0] * InvReflStr);
						OrigColor[1] = (Pix.RGB[1] * ReflStrength + OrigColor[1] * InvReflStr);
						OrigColor[2] = (Pix.RGB[2] * ReflStrength + OrigColor[2] * InvReflStr);
						CurFrag->PlotPixel(OrigColor);
						CurFrag->Refl->Reflect = (unsigned char)(ReflStrength * 255.99);
						} // if reflection strength
					} // if fragment is reflective
				} // for each fragment
			// replot pixel to screen
			#pragma omp critical (OMP_CRITICAL_SCREENPIXPLOT)
#ifdef SPEED_MOD
			ScreenPixelPlotFragments(&FragMap[PixZip], XX, Y + DrawOffsetY);
#else // SPEED_MOD
			ScreenPixelPlotFragments(&FragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);
#endif // SPEED_MOD
			} // if reflections on this pixel
		} // for

	if (ShowProgress)
		{
		if (IsCamView)
			{
			if (BWDE)
				{
				if(BWDE->Update(X + 1))
					{
					Success = 0;
					break;
					} // if
				} // if
			} // if
		else
			{
			Master->ProcUpdate(X + 1);
			if (! Master->IsRunning())
				{
				Success = 0;
				break;
				} // if
			} // else
		} // if
	} // for

//double etime = StopHiResTimerSecs();
//char msg[80];
//sprintf(msg, "Reflection Time = %lf", etime);
//GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, msg);

AppMem_Free(FragSortList, MaxListFrags * sizeof (struct PixelFragSort));
if (ShowProgress)
	{
	if (Master)
		Master->ProcClear();
	if (BWDE)
		delete BWDE;
	} // if
	
return (Success);

} // Renderer::RenderReflections_BTFRAGS

/*===========================================================================*/

long Renderer::AddReflFragEntry(PixelFragSort *SortList, long CurEntry, rPixelFragment *AddFrag, double NewFragCovg)
{

SortList[CurEntry].Frag = AddFrag;
SortList[CurEntry].Z = AddFrag->ZBuf;
SortList[CurEntry].Covg = NewFragCovg * AddFrag->FetchMaskedCoverage();

return (++CurEntry);

} // Renderer::AddReflFragEntry

/*===========================================================================*/

double Renderer::AddReflFragEntry(PixelFragSort *SortList, long &CurEntry, double CovgWt, rPixelFragment *AddFrag,
	int ConsiderMask)
{
double Covg;

SortList[CurEntry].Frag = AddFrag;
SortList[CurEntry].Z = AddFrag->ZBuf;
if (ConsiderMask)
	SortList[CurEntry].Covg = Covg = CovgWt * AddFrag->FetchMaskedCoverage();
else
	SortList[CurEntry].Covg = Covg = CovgWt * AddFrag->Alpha * (1.0 / 255.0);
CurEntry ++;

return (Covg);

} // Renderer::AddReflFragEntry

/*===========================================================================*/

// sorts near to far
int ComparePixelFragSort(const void *elem1, const void *elem2)
{

return (
	((struct PixelFragSort *)elem1)->Z > ((struct PixelFragSort *)elem2)->Z ? 1:
	(((struct PixelFragSort *)elem1)->Z < ((struct PixelFragSort *)elem2)->Z ? -1: 0)
	);

} // ComparePixelFragSort

/*===========================================================================*/

void Renderer::SampleImageReflection(double AreaToSample, double PixCtrX, double PixCtrY, unsigned char *Bitmap[3], double SampleRGB[3])
{
double SampleRadius, SampleRadiusSq, PixOffsetX, PixOffsetY, PixOffsetSq, SumWt, SampleWt;
long X, Y, SampStartY, SampStartX, SampEndY, SampEndX, PixZip;

SampleRadius = AreaToSample * .5;
if (SampleRadius < 1.0)
	SampleRadius = 1.0;
else if (SampleRadius > 5.0)
	SampleRadius = 5.0;	// need some kind of practical limit to keep sample times from going extreme
SampleRadiusSq = SampleRadius * SampleRadius;
SampStartY = (long)WCS_floor(PixCtrY - SampleRadius);
SampStartX = (long)WCS_floor(PixCtrX - SampleRadius);
SampEndY = (long)WCS_ceil(PixCtrY + SampleRadius);
SampEndX = (long)WCS_ceil(PixCtrX + SampleRadius);

SampleRGB[0] = SampleRGB[1] = SampleRGB[2] = 0.0;
SumWt = 0.0;

for (Y = SampStartY; Y <= SampEndY; Y ++)
	{
	if (Y < 0)
		continue;
	if (Y >= Height)
		break;
	PixZip = Y * Width + SampStartX;
	PixOffsetY = Y + .5 - PixCtrY;
	PixOffsetY *= PixOffsetY;
	for (X = SampStartX; X <= SampEndX; X ++, PixZip ++)
		{
		if (X < 0)
			continue;
		if (X >= Width)
			break;
		if (ReflectionBuf[PixZip])
			continue;
		PixOffsetX = X + .5 - PixCtrX;
		PixOffsetSq = PixOffsetX * PixOffsetX + PixOffsetY;
		// a very simple boolean test that should be improved with a gaussian type weight distribution
		SampleWt = SampleRadiusSq > PixOffsetSq ? 1.0: 0.0;
		if (SampleWt > 0.0)
			{
			SampleRGB[0] += Bitmap[0][PixZip] * SampleWt;
			SampleRGB[1] += Bitmap[1][PixZip] * SampleWt;
			SampleRGB[2] += Bitmap[2][PixZip] * SampleWt;
			SumWt += SampleWt;
			} // if
		} // for
	} // for

if (SumWt > 0.0)
	{
	SumWt = 1.0 / SumWt;
	SampleRGB[0] *= SumWt;
	SampleRGB[1] *= SumWt;
	SampleRGB[2] *= SumWt;
	} // if

} // Renderer::SampleImageReflection

/*===========================================================================*/

int Renderer::RenderBackground(void)
{
double SampleXLow, SampleYLow, XInc, YInc, SampleWt, SampleColor[3], PixNormal[3];
Raster *BGRast;
BusyWin *BWDE = NULL;
rPixelFragment *PixFrag;
int Success = 1, Abort = 0, PixWt;
long X, Y, PixZip;

if (! Cam->Img || ! (BGRast = Cam->Img->GetRaster()) || ! (BGRast->GetEnabled()))
	return (1);

if (IsCamView)
	{
	BWDE = new BusyWin("Background", Height, 'BWDE', 0);
	} // if
else
	{
	Master->ProcInit(Height, "Background");
	} // else

XInc = 1.0 / Width;
YInc = 1.0 / Height;
PixNormal[0] = PixNormal[1] = PixNormal[2] = 0.0;

for (Y = PixZip = 0, SampleYLow = 0.0; Y < Height && ! Abort; Y ++, SampleYLow += YInc)
	{
	for (X = 0, SampleXLow = 0.0; X < Width; X ++, PixZip ++, SampleXLow += XInc)
		{
		if (rPixelFragMap || AABuf[PixZip] < 255)
			{
			SampleColor[0] = SampleColor[1] = SampleColor[2] = 0.0;
			// sample image without self-masking
			SampleWt = BGRast->SampleRangeByteDouble3(SampleColor, SampleXLow, SampleXLow + XInc, SampleYLow, SampleYLow + YInc, Abort, 0);
			// if image was not found and user chose not to select a new one, Abort will be set.
			if (Abort)
				{
				// disable so it doesn't happen again
				Cam->BackgroundImageEnabled = 0;
				break;
				} // if
			if (SampleWt > 0.0)
				{
				if (! rPixelFragMap)
					{
					// plot the colors just as they come from the raster function. They are diminished if there is an alpha channel
					CloudBitmap[0][PixZip] = (unsigned char)(SampleColor[0] * 255.99);
					CloudBitmap[1][PixZip] = (unsigned char)(SampleColor[1] * 255.99);
					CloudBitmap[2][PixZip] = (unsigned char)(SampleColor[2] * 255.99);
					CloudAABuf[PixZip] = (unsigned char)(SampleWt * 255.99);
					ScreenPixelPlotTwoBuf(Bitmap, ZBuf, AABuf, CloudBitmap, CloudZBuf, CloudAABuf, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
					} // if
				else
					{
					PixWt = (int)(SampleWt * 255.99);
					if (PixFrag = rPixelFragMap[PixZip].PlotPixel(rPixelBlock, (float)FLT_MAX, (unsigned char)PixWt, ~0UL, ~0UL, FragmentDepth, 
						(unsigned char)WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_BACKGROUND))
						{
						PixFrag->PlotPixel(rPixelBlock, SampleColor, 0.0, PixNormal);
						ScreenPixelPlotFragments(&rPixelFragMap[PixZip], X + DrawOffsetX, Y + DrawOffsetY);
						} // if
					} // if
				if (ObjectBuf && ! ObjectBuf[PixZip])
					{
					ObjectBuf[PixZip] = Cam;
					if (ObjTypeBuf)
						ObjTypeBuf[PixZip] = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_BACKGROUND;
					} // if
				} // if
			} // if
		} // for

	if (IsCamView)
		{
		if (BWDE)
			{
			if(BWDE->Update(Y + 1))
				{
				Success = 0;
				break;
				} // if
			} // if
		} // if
	else
		{
		Master->ProcUpdate(Y + 1);
		if (! Master->IsRunning())
			{
			Success = 0;
			break;
			} // if
		} // else
	} // for

if (Master)
	Master->ProcClear();
if (BWDE)
	delete BWDE;

return (Success);

} // Renderer::RenderBackground

/*===========================================================================*/

int Renderer::RenderDepthOfField(void)
{
int Success = 1;
long X, Y, PixZip, Sx, Sy, i, j, Szip, ImageSize, MaxCoC, MaxCoCp1;
unsigned long SumRed, SumGrn, SumBlu, ThreeColors[3];
unsigned short *BackupExponentBuf = NULL;
float Zmin, Zmax;
double FocalLength, FocalDist, FStop, CoC, Znear, Zp, MinZ, *xDist, Dist, FilmSize, FilmScale, Wt, SumWt, Vp, Vd,
	CoCzmin, CoCzmax;
BusyWin *BWDE = NULL;

// check to see if DOF is enabled in the camera
if (! Cam->DepthOfField)
	return (1);

// defined in Useful.h
StandardNormalDistributionInit();

FocalLength = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FOCALLENGTH].CurValue * (1.0 / 1000.0);	// convert mm to m
FocalDist = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FOCALDIST].CurValue;
if (FocalDist < 2.0 * FocalLength)
	FocalDist = 2.0 * FocalLength;
FilmSize = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FILMSIZE].CurValue * (1.0 / 1000.0);	// convert mm to m
FilmScale = Width / FilmSize;
FStop = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FSTOP].CurValue;
MaxCoCp1 = Cam->MaxDOFBlurRadius;
MaxCoC = MaxCoCp1 - 1;
ImageSize = Width * Height;
MinZ = FocalLength * 10.0;
Vp = (FocalLength * FocalDist) / (FocalDist - FocalLength);

// determine the range of z values in the image
Zmin = FLT_MAX;
Zmax = -FLT_MAX;

for (PixZip = 0; PixZip < ImageSize; PixZip ++)
	{
	if (ZBuf[PixZip] < Zmin)
		Zmin = ZBuf[PixZip];
	if (ZBuf[PixZip] > Zmax)
		Zmax = ZBuf[PixZip];
	} // for

// find the circle of confusion for the two extremes

if (Zmin < MinZ)
	Zmin = (float)MinZ;
Vd = (FocalLength * Zmin) / (Zmin - FocalLength);
CoCzmin = (FilmScale * fabs(Vd - Vp) * FocalLength / (FStop * Vd)) * 0.5; // Optimize out divide. Was / 2.0
if (Zmax < MinZ)
	Zmax = (float)MinZ;
Vd = (FocalLength * Zmax) / (Zmax - FocalLength);
CoCzmax = (FilmScale * fabs(Vd - Vp) * FocalLength / (FStop * Vd)) * 0.5; // Optimize out divide. Was / 2.0

if (CoCzmin < MaxCoC && CoCzmax < MaxCoC)
	{
	MaxCoC = (long)WCS_ceil(max(CoCzmin, CoCzmax));
	MaxCoCp1 = MaxCoC + 1;
	} // if new smaller max circle of confusion size

if (ExponentBuf && ! (BackupExponentBuf = (unsigned short *)AppMem_Alloc(Width * Height * sizeof (unsigned short), APPMEM_CLEAR)))
	Success = 0;

#ifdef WCS_BUILD_DEMO
if (Success && (xDist = (double *)AppMem_Alloc(MaxCoCp1 * MaxCoCp1 * sizeof (double) + MemFuck, 0)))
#else // WCS_BUILD_DEMO
if (Success && (xDist = (double *)AppMem_Alloc(MaxCoCp1 * MaxCoCp1 * sizeof (double), 0)))
#endif // WCS_BUILD_DEMO
	{
	if (IsCamView)
		{
		BWDE = new BusyWin("Depth of Field", Height, 'BWDE', 0);
		} // if
	else
		{
		Master->ProcInit(Height, "Depth of Field");
		} // else
	// initialize distance table
	for (Sy = 0; Sy <= MaxCoC; Sy ++)
		{
		for (Sx = 0; Sx <= MaxCoC; Sx ++)
			{
			xDist[Sy * MaxCoCp1 + Sx] = sqrt((double)(Sx * Sx + Sy * Sy));	//lint !e790
			} // for
		} // for

	for (Y = 0, PixZip = 0; Y < Height; Y ++)
		{
		for (X = 0; X < Width; X ++, PixZip ++)
			{
			Zp = ZBuf[PixZip];
			SumWt = 0.0;
			SumRed = SumGrn = SumBlu = 0;
			// check each pixel in max circle of confusion to see if it contributes
			for (j = -MaxCoC; j <= MaxCoC; j ++)
				{
				if ((Sy = Y + j) < 0)
					continue;
				if (Sy >= Height)
					break;
				Szip = Sy * Width + X - MaxCoC;
				for (i = -MaxCoC; i <= MaxCoC; i ++, Szip ++)
					{
					if ((Sx = X + i) < 0)
						continue;
					if (Sx >= Width)
						break;
					// use the closer of two z values for distance to point
					Znear = min(Zp, ZBuf[Szip]);
					// watch out for unrealistically low z values - probably due to foliage z offset
					if (Znear < MinZ)
						Znear = MinZ;
					// Vd is the focal distance of distance Znear
					Vd = (FocalLength * Znear) / (Znear - FocalLength);
					// calculate the circle of confusion radius and limit it to max set by user
					if ((CoC = (FilmScale * fabs(Vd - Vp) * FocalLength / (FStop * Vd)) * 0.5) > MaxCoC) // Optimize out divide. Was / 2.0
						CoC = MaxCoC;
					// does this pixel's circle of confusion overlap the pixel we're modifying?
					if ((Dist = xDist[abs(j) * MaxCoCp1 + abs(i)]) <= CoC)
						{
						// normalize the pixel offset distance and look up wt in table
						//Wt = (Pi * CoC * CoC);
						Wt = CoC;
						if (Wt < .005)
							Wt = .005;
						Wt = StandardNormalDistribution(2.33 * Dist / CoC) / min(1.0, Wt);
						SumWt += Wt;
						ThreeColors[0] = Bitmap[0][Szip];
						ThreeColors[1] = Bitmap[1][Szip];
						ThreeColors[2] = Bitmap[2][Szip];
						if (ExponentBuf && ExponentBuf[Szip])
							rPixelFragment::ExtractUnclippedExponentialColors(ThreeColors, ExponentBuf[Szip]);
						SumRed += (long)(Wt * ThreeColors[0]);
						SumGrn += (long)(Wt * ThreeColors[1]);
						SumBlu += (long)(Wt * ThreeColors[2]);
						} // if
					} // for
				} // for
			if (SumWt > 0.0)
				{
				ThreeColors[0] = (unsigned long)(SumRed / SumWt);
				ThreeColors[1] = (unsigned long)(SumGrn / SumWt);
				ThreeColors[2] = (unsigned long)(SumBlu / SumWt);
				if (BackupExponentBuf)
					rPixelFragment::ExtractExponentialColors(ThreeColors, BackupExponentBuf[PixZip]);
				TreeBitmap[0][PixZip] = (unsigned char)(ThreeColors[0]);
				TreeBitmap[1][PixZip] = (unsigned char)(ThreeColors[1]);
				TreeBitmap[2][PixZip] = (unsigned char)(ThreeColors[2]);
				} // if
			} // for
		if (IsCamView)
			{
			if (BWDE)
				{
				if(BWDE->Update(Y + 1))
					{
					Success = 0;
					break;
					} // if
				} // if
			} // if
		else
			{
			Master->ProcUpdate(Y + 1);
			if (!Master->IsRunning())
				{
				Success = 0;
				break;
				} // if
			} // else
		} // for

	memcpy(Bitmap[0], TreeBitmap[0], ImageSize);
	memcpy(Bitmap[1], TreeBitmap[1], ImageSize);
	memcpy(Bitmap[2], TreeBitmap[2], ImageSize);
	if (ExponentBuf && BackupExponentBuf)
		memcpy(ExponentBuf, BackupExponentBuf, ImageSize * sizeof (unsigned short));

	AppMem_Free(xDist, MaxCoCp1 * MaxCoCp1 * sizeof (double));
	if (Master)
		Master->ProcClear();
	if(BWDE)
		delete BWDE;
	BWDE = NULL;
	} // if
else
	Success = 0;

if (BackupExponentBuf)
	AppMem_Free(BackupExponentBuf, Width * Height * sizeof (unsigned short));

return (Success);

} // Renderer::RenderDepthOfField

/*===========================================================================*/

int Renderer::RenderBoxFilter(void)
{
int Success = 1;
long X, Y, j, PixZip, Retrieve, ZBRetrieve, Current, NonCurrent, Zalias, Apply;
unsigned long ThreeColors[3], Sum[3];
UBYTE *BackupLine[2][3];
unsigned short *BackupExpon[2];
float ZBufDist;
BusyWin *BWDE = NULL;

if (IsCamView)
	{
	BWDE = new BusyWin("Box Filter", Height * Cam->BoxFilterSize, 'BWDE', 0);
	} // if
else
	{
	Master->ProcInit(Height * Cam->BoxFilterSize, "Box Filter");
	} // else

Zalias = Cam->ZBufBoxFilter;
ZBufDist = (float)Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_ZBUFBOXFILTOFFSET].CurValue;
#ifdef WCS_BUILD_DEMO
BackupLine[0][0] = (UBYTE *)AppMem_Alloc(Width + MemFuck, 0);
BackupLine[0][1] = (UBYTE *)AppMem_Alloc(Width, 0);
#else // WCS_BUILD_DEMO
BackupLine[0][0] = (UBYTE *)AppMem_Alloc(Width, 0);
BackupLine[0][1] = (UBYTE *)AppMem_Alloc(Width, 0);
#endif // WCS_BUILD_DEMO
BackupLine[0][2] = (UBYTE *)AppMem_Alloc(Width, 0);
BackupLine[1][0] = (UBYTE *)AppMem_Alloc(Width, 0);
BackupLine[1][1] = (UBYTE *)AppMem_Alloc(Width, 0);
BackupLine[1][2] = (UBYTE *)AppMem_Alloc(Width, 0);
if (ExponentBuf)
	{
	BackupExpon[0] = (unsigned short *)AppMem_Alloc(Width * sizeof (unsigned short), 0);
	BackupExpon[1] = (unsigned short *)AppMem_Alloc(Width * sizeof (unsigned short), 0);
	} // if
else
	{
	BackupExpon[0] = BackupExpon[1] = NULL;
	} // else

if (BackupLine[0][0] && BackupLine[0][1] && BackupLine[0][2] && BackupLine[1][0] && BackupLine[1][1] && BackupLine[1][2]
	&& BackupExpon[0] && BackupExpon[1])
	{
	for (j = 0, Current = 0; j < Cam->BoxFilterSize; j ++)
		{
		memcpy(BackupLine[Current][0], &Bitmap[0][0], Width);
		memcpy(BackupLine[Current][1], &Bitmap[1][0], Width);
		memcpy(BackupLine[Current][2], &Bitmap[2][0], Width);
		if (ExponentBuf)
			memcpy(BackupExpon[Current], &ExponentBuf[0], Width * sizeof (unsigned short));
		for (Y = 1; Y < Height - 1; Y ++)
			{
			Current = 1 - Current;
			NonCurrent = ! Current;
			PixZip = Width * Y;
			memcpy(BackupLine[Current][0], &Bitmap[0][PixZip], Width);
			memcpy(BackupLine[Current][1], &Bitmap[1][PixZip], Width);
			memcpy(BackupLine[Current][2], &Bitmap[2][PixZip], Width);
			if (ExponentBuf)
				memcpy(BackupExpon[Current], &ExponentBuf[PixZip], Width * sizeof (unsigned short));
			for (X = 1, PixZip ++; X < Width - 1; X ++, PixZip ++)
				{
				Apply = 0;
				Retrieve = X - 1;
				ZBRetrieve = PixZip - 1 - Width;
				if (fabs( ZBuf[PixZip] - ZBuf[ZBRetrieve]) > ZBufDist)
					Apply = 1;
				ThreeColors[0] = BackupLine[NonCurrent][0][Retrieve];
				ThreeColors[1] = BackupLine[NonCurrent][1][Retrieve];
				ThreeColors[2] = BackupLine[NonCurrent][2][Retrieve];
				if (ExponentBuf && BackupExpon[NonCurrent][Retrieve])
					rPixelFragment::ExtractUnclippedExponentialColors(ThreeColors, BackupExpon[NonCurrent][Retrieve]);
				Sum[0] = ThreeColors[0];
				Sum[1] = ThreeColors[1];
				Sum[2] = ThreeColors[2];
				Retrieve ++;
				ZBRetrieve ++;

				if (fabs( ZBuf[PixZip] - ZBuf[ZBRetrieve]) > ZBufDist)
					Apply = 1;
				ThreeColors[0] = BackupLine[NonCurrent][0][Retrieve];
				ThreeColors[1] = BackupLine[NonCurrent][1][Retrieve];
				ThreeColors[2] = BackupLine[NonCurrent][2][Retrieve];
				if (ExponentBuf && BackupExpon[NonCurrent][Retrieve])
					rPixelFragment::ExtractUnclippedExponentialColors(ThreeColors, BackupExpon[NonCurrent][Retrieve]);
				Sum[0] += 2 * ThreeColors[0];
				Sum[1] += 2 * ThreeColors[1];
				Sum[2] += 2 * ThreeColors[2];
				Retrieve ++;
				ZBRetrieve ++;

				if (fabs( ZBuf[PixZip] - ZBuf[ZBRetrieve]) > ZBufDist)
					Apply = 1;
				ThreeColors[0] = BackupLine[NonCurrent][0][Retrieve];
				ThreeColors[1] = BackupLine[NonCurrent][1][Retrieve];
				ThreeColors[2] = BackupLine[NonCurrent][2][Retrieve];
				if (ExponentBuf && BackupExpon[NonCurrent][Retrieve])
					rPixelFragment::ExtractUnclippedExponentialColors(ThreeColors, BackupExpon[NonCurrent][Retrieve]);
				Sum[0] += ThreeColors[0];
				Sum[1] += ThreeColors[1];
				Sum[2] += ThreeColors[2];

				Retrieve = X - 1;
				ZBRetrieve = PixZip - 1;
				if (fabs( ZBuf[PixZip] - ZBuf[ZBRetrieve]) > ZBufDist)
					Apply = 1;
				ThreeColors[0] = BackupLine[Current][0][Retrieve];
				ThreeColors[1] = BackupLine[Current][1][Retrieve];
				ThreeColors[2] = BackupLine[Current][2][Retrieve];
				if (ExponentBuf && BackupExpon[Current][Retrieve])
					rPixelFragment::ExtractUnclippedExponentialColors(ThreeColors, BackupExpon[Current][Retrieve]);
				Sum[0] += 2 * ThreeColors[0];
				Sum[1] += 2 * ThreeColors[1];
				Sum[2] += 2 * ThreeColors[2];
				Retrieve ++;
				ZBRetrieve ++;

				if (fabs( ZBuf[PixZip] - ZBuf[ZBRetrieve]) > ZBufDist)
					Apply = 1;
				ThreeColors[0] = BackupLine[Current][0][Retrieve];
				ThreeColors[1] = BackupLine[Current][1][Retrieve];
				ThreeColors[2] = BackupLine[Current][2][Retrieve];
				if (ExponentBuf && BackupExpon[Current][Retrieve])
					rPixelFragment::ExtractUnclippedExponentialColors(ThreeColors, BackupExpon[Current][Retrieve]);
				Sum[0] += 3 * ThreeColors[0];
				Sum[1] += 3 * ThreeColors[1];
				Sum[2] += 3 * ThreeColors[2];
				Retrieve ++;
				ZBRetrieve ++;

				if (fabs( ZBuf[PixZip] - ZBuf[ZBRetrieve]) > ZBufDist)
					Apply = 1;
				ThreeColors[0] = BackupLine[Current][0][Retrieve];
				ThreeColors[1] = BackupLine[Current][1][Retrieve];
				ThreeColors[2] = BackupLine[Current][2][Retrieve];
				if (ExponentBuf && BackupExpon[Current][Retrieve])
					rPixelFragment::ExtractUnclippedExponentialColors(ThreeColors, BackupExpon[Current][Retrieve]);
				Sum[0] += 2 * ThreeColors[0];
				Sum[1] += 2 * ThreeColors[1];
				Sum[2] += 2 * ThreeColors[2];

				Retrieve = PixZip - 1 + Width;
				ZBRetrieve = Retrieve;
				if (fabs( ZBuf[PixZip] - ZBuf[ZBRetrieve]) > ZBufDist)
					Apply = 1;
				ThreeColors[0] = Bitmap[0][Retrieve];
				ThreeColors[1] = Bitmap[1][Retrieve];
				ThreeColors[2] = Bitmap[2][Retrieve];
				if (ExponentBuf && ExponentBuf[Retrieve])
					rPixelFragment::ExtractUnclippedExponentialColors(ThreeColors, ExponentBuf[Retrieve]);
				Sum[0] += ThreeColors[0];
				Sum[1] += ThreeColors[1];
				Sum[2] += ThreeColors[2];
				Retrieve ++;
				ZBRetrieve ++;

				if (fabs( ZBuf[PixZip] - ZBuf[ZBRetrieve]) > ZBufDist)
					Apply = 1;
				ThreeColors[0] = Bitmap[0][Retrieve];
				ThreeColors[1] = Bitmap[1][Retrieve];
				ThreeColors[2] = Bitmap[2][Retrieve];
				if (ExponentBuf && ExponentBuf[Retrieve])
					rPixelFragment::ExtractUnclippedExponentialColors(ThreeColors, ExponentBuf[Retrieve]);
				Sum[0] += 2 * ThreeColors[0];
				Sum[1] += 2 * ThreeColors[1];
				Sum[2] += 2 * ThreeColors[2];
				Retrieve ++;
				ZBRetrieve ++;

				if (fabs( ZBuf[PixZip] - ZBuf[ZBRetrieve]) > ZBufDist)
					Apply = 1;
				ThreeColors[0] = Bitmap[0][Retrieve];
				ThreeColors[1] = Bitmap[1][Retrieve];
				ThreeColors[2] = Bitmap[2][Retrieve];
				if (ExponentBuf && ExponentBuf[Retrieve])
					rPixelFragment::ExtractUnclippedExponentialColors(ThreeColors, ExponentBuf[Retrieve]);
				Sum[0] += ThreeColors[0];
				Sum[1] += ThreeColors[1];
				Sum[2] += ThreeColors[2];
				
				if (! Zalias || Apply)
					{
					Sum[0] /= 15;
					Sum[1] /= 15;
					Sum[2] /= 15;
					if (ExponentBuf)
						rPixelFragment::ExtractExponentialColors(Sum, ExponentBuf[PixZip]);
					Bitmap[0][PixZip]=(UBYTE)(Sum[0]);
					Bitmap[1][PixZip]=(UBYTE)(Sum[1]);
					Bitmap[2][PixZip]=(UBYTE)(Sum[2]);
					ScreenPixelPlot(Bitmap, X + DrawOffsetX, Y + DrawOffsetY, PixZip);
					} // if   
				} // for x=0... 
			if (IsCamView)
				{
				if (BWDE)
					{
					if(BWDE->Update(j * Height + Y + 1))
						{
						Success = 0;
						break;
						} // if
					} // if
				} // if
			else
				{
				Master->ProcUpdate(j * Height + Y + 1);
				if (! Master->IsRunning())
					{
					Success = 0;
					break;
					} // if
				} // else
			} // for y=0... 
		} // for j=0 
	} // if 

if (BackupLine[0][0])
	AppMem_Free(BackupLine[0][0], Width);
if (BackupLine[0][1])
	AppMem_Free(BackupLine[0][1], Width);
if (BackupLine[0][2])
	AppMem_Free(BackupLine[0][2], Width);
if (BackupLine[1][0])
	AppMem_Free(BackupLine[1][0], Width);
if (BackupLine[1][1])
	AppMem_Free(BackupLine[1][1], Width);
if (BackupLine[1][2])
	AppMem_Free(BackupLine[1][2], Width);
if (BackupExpon[0])
	AppMem_Free(BackupExpon[0], Width * sizeof (unsigned short));
if (BackupExpon[1])
	AppMem_Free(BackupExpon[1], Width * sizeof (unsigned short));

if (Master)
	Master->ProcClear();
if (BWDE)
	delete BWDE;

return (Success);

} // Renderer::RenderBoxFilter

/*===========================================================================*/

int Renderer::RenderPostProc(int PreReflection, unsigned char **OptionalBitmaps, PostProcess *ProcessMe, long FrameNum)
{
int Success = 1, Redraw;
BusyWin *BWDE = NULL;
EffectList *Post;
PostProcessEvent *Proc;

// don't put up progress UI if we're in animation play mode as it hides animation stop button
if (IsCamView)
	{
	if(!GlobalApp->MCP->QueryCurrentlyPlaying())
		{
		BWDE = new BusyWin("Post Processing", Height, 'BWDE', 0);
		} // if
	} // if
else
	{
	Master->ProcInit(Height, "Post Processing");
	} // else

if (ProcessMe)
	{
	if (ProcessMe->Enabled)
		Success = ProcessMe->RenderPostProc(&RendData, Buffers, NULL, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, FALSE);
	} // if
else if (Opt->Post && (Success = Opt->RenderPostProc(PreReflection, &RendData, Buffers, PreReflection ? rPixelBlock: NULL, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, TRUE)))
	{
	Post = Opt->Post;
	Redraw = 0;
	while (Post && ! Redraw)
		{
		if (Post->Me && Post->Me->Enabled && PreReflection == ((PostProcess *)Post->Me)->BeforeReflection)
			{
			Proc = ((PostProcess *)Post->Me)->Events;
			while (Proc)
				{
				if (Proc->Enabled)
					{
					Redraw = 1;
					break;
					} // if
				Proc = Proc->Next;
				} // while
			} // if
		Post = Post->Next;
		} // while
	if (Redraw)
		{
		if (PlotFromFrags && Pane)
			DrawPreview();
		else
			UpdatePreview();
		} // if
	} // else if

if (Master)
	Master->ProcClear();
if (BWDE)
	delete BWDE;

return (Success);

} // Renderer::RenderPostProc

/*===========================================================================*/
