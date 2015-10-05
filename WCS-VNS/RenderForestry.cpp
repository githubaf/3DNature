// RenderForestry.cpp
// Code for rendering foliage using forest industry style parameters
// Original code created on June 18, 2004 by Gary R. Huber
// Copyright 2004 3D Nature LLC. All rights reserved.

#include "stdafx.h"
#include "Application.h"
#include "Render.h"
#include "Ecotype.h"
#include "EffectsLib.h"
#include "UsefulSwap.h"
#include "Realtime.h"
#include "UsefulMath.h"
#include "Raster.h"

//extern WCSApp *GlobalApp;

//extern unsigned __int64 DevCounter[50];

#ifdef WCS_FORESTRY_WIZARD

// debugging stuff for forestry features
#ifdef WCS_BUILD_GARY
double AreaRendered, StemsRendered, AvgStemsRendered, PolysRendered, GroupStemsRendered[4], ImageStemsRendered[4][8],
	SumPRN, PRNAbove, PRNBelow;
long GroupCt, ImageCt, PRNCt, TinyPRN;
#endif // WCS_BUILD_GARY

ForestryRenderData::ForestryRenderData()
{

Eco = NULL;
Poly = NULL;
Vtx = NULL;
VertexWeights = NULL;
AvgNumStems = NormalizedDensity = RawDensity = MaxHeight = MinHeight = HeightRange = AgvHeight = 
	MaxDbh = MaxAge = MaxClosure = MaxStems = MaxBasalArea = 0.0;
MatCovg = 1.0;
SeedOffset = 0;
RenderAboveBeachLevel = false;
EcoDensityComputed = EcoSizeComputed = GroupDensityComputed = GroupSizeComputed = FolEffect = false;

} // ForestryRenderData::ForestryRenderData

/*===========================================================================*/
/*===========================================================================*/

int Renderer::RenderForestryFoliage(PolygonData *Poly, VertexData *Vtx[3], Ecotype *Eco, double RenderOpacity, 
	double MatCovg, double *VertexWeights, unsigned long SeedOffset, int FolEffect, bool RenderAboveBeachLevel)
{
ForestryRenderData ForDat;
int Success = 1;

if (! ShadowMapInProgress && Opt->FoliageShadowsOnly)
	return (1);

ForDat.Eco = Eco;
ForDat.Poly = Poly;
ForDat.Vtx = Vtx;
ForDat.VertexWeights = VertexWeights;
ForDat.SeedOffset = SeedOffset;
ForDat.FolEffect = FolEffect ? true: false;
ForDat.RenderOpacity = RenderOpacity;
ForDat.RenderAboveBeachLevel = RenderAboveBeachLevel;
Eco->Rand.Seed64BitShift(Poly->LatSeed + SeedOffset, Poly->LonSeed + SeedOffset);

if (Eco->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
	{
	Eco->ComputeEcoSize(&ForDat);
	} // if
if (! FolEffect)
	{
	ForDat.MatCovg = MatCovg;
	if (Eco->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
		{
		// compute ForDat.NormalizedDensity
		Eco->ComputeEcoDensity(&ForDat);
		// average number of stems per polygon of this size
		if (Eco->ConstDensity == WCS_FOLIAGE_DENSITY_CONSTANT)
			ForDat.AvgNumStems = ForDat.NormalizedDensity * Poly->Area;	// Area is always positive calculated by SolveTriangleArea()
		else
			ForDat.AvgNumStems = ForDat.NormalizedDensity;
		// when there are multiple polygon materials, need to reduce # stems by amount of material coverage
		ForDat.AvgNumStems *= MatCovg;
		#ifdef WCS_BUILD_GARY
		AreaRendered += Poly->Area;
		AvgStemsRendered += ForDat.AvgNumStems;
		PolysRendered ++;
		#endif // WCS_BUILD_GARY
		} // if
	} // if
else
	ForDat.AvgNumStems = 1.0;

RenderForestryEcotype(&ForDat);

return (Success);

} // Renderer::RenderForestryFoliage

/*===========================================================================*/

/*
// usage of random numbers for foliage
Random[0] - added to GroupSum to determine if a stem should be drawn
Random[1] - vertex location randomization, used again for determining if another stem should be drawn
Random[2] - vertex location randomization
Random[3] - tests to see if textures have elliminated a stem
Random[4] - determines which foliage in a group to draw
Random[5] - determines height within minimum and maximum ends of height range
Random[6] - image FlipX state, random 3d object X rotation
Random[7] - random 3d object Y rotation
Random[8] - random 3d object Z rotation
Random[9] - used to determine if an item's positional weighting turns it off

*/

int Renderer::RenderForestryEcotype(ForestryRenderData *ForDat)
{
double Random[10], GroupSum = 0.0, SumGroupDensity = 0.0, SumFoliageDensity, InstanceTexturedDensity, MaxStemsThisGroup, 
	ThirdVtxWt, SumImageFinder, TexOpacity, FolHeight, Value[3];
#ifdef WCS_VECPOLY_EFFECTS
double FolMaterialWeight, BeachElev;
#endif // WCS_VECPOLY_EFFECTS
FoliageGroup *CurGrp, *StrtGrp;
Foliage *CurFol;
int Success = 1, GroupNum = 1;
VertexDEM FolVtx;
#ifdef WCS_VECPOLY_EFFECTS
bool RenderThisItem = true;
#endif // WCS_VECPOLY_EFFECTS

#ifdef WCS_BUILD_GARY
GroupCt = 0;
#endif // WCS_BUILD_GARY

Value[2] = Value[1] = Value[0] = 0.0;

// compute a group percentage for each group taking into account thematic maps but not textures
// added to replace below 3/27/08 GRH
for (CurGrp = ForDat->Eco->FolGrp; CurGrp; CurGrp = CurGrp->Next)
	{
	if (CurGrp->Enabled)
		{
		CurGrp->GroupSizeComputed = CurGrp->GroupDensityComputed = 0;
		if (ForDat->Eco->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
			{
			CurGrp->ComputeGroupSize(ForDat);
			// calculated ForDat->HeightRange, ForDat->MinHeight, ForDat->MaxHeight, ForDat->AgvHeight
			} // if
		else
			{
			CurGrp->ComputeGroupRelativeSize(ForDat);
			// calculated CurGrp->RelativeGroupSize
			} // else
		if (ForDat->Eco->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
			{
			CurGrp->ComputeGroupDensity(ForDat);	
			// calculated ForDat->NormalizedDensity, ForDat->RawDensity
			CurGrp->GroupPercentage = 1.0;
			CurGrp->NormalizedDensity *= ForDat->MatCovg;
			} // if
		else
			{
			CurGrp->ComputeGroupRelativeDensity(ForDat);
			// calculated CurGrp->RelativeGroupDensity
			SumGroupDensity += CurGrp->RelativeGroupDensity;
			} // else
		} // if
	else
		{
		CurGrp->GroupPercentage = 0.0;
		} // else
	} // for
if (ForDat->Eco->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
	{
	if (SumGroupDensity > 0.0)
		{
		SumGroupDensity = 1.0 / SumGroupDensity;
		for (CurGrp = ForDat->Eco->FolGrp; CurGrp; CurGrp = CurGrp->Next)
			{
			if (CurGrp->Enabled)
				CurGrp->GroupPercentage = CurGrp->RelativeGroupDensity * SumGroupDensity;
			} // for
		} // if
	else
		return (1);
	} // if
/* removed and replaced by above 3/27/08 GRH
// For ecotypes that have density by group just set each percentage to 1
if (ForDat->Eco->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
	{
	for (CurGrp = ForDat->Eco->FolGrp; CurGrp; CurGrp = CurGrp->Next)
		{
		if (CurGrp->Enabled)
			{
			CurGrp->GroupPercentage = 1.0;
			CurGrp->ComputeGroupRelativeSize(ForDat);
			} // if
		else
			CurGrp->GroupPercentage = 0.0;
		} // for
	} // if
else
	{
	// if there are thematic maps for any of the group %'s then they need to be evaluated and new
	// relative percentages calculated for each group
	for (CurGrp = ForDat->Eco->FolGrp; CurGrp; CurGrp = CurGrp->Next)
		{
		if (CurGrp->Enabled)
			{
			CurGrp->ComputeGroupRelativeDensity(ForDat);
			SumGroupDensity += CurGrp->RelativeGroupDensity;
			CurGrp->ComputeGroupRelativeSize(ForDat);
			} // if
		} // for
	if (SumGroupDensity > 0.0)
		{
		SumGroupDensity = 1.0 / SumGroupDensity;
		for (CurGrp = ForDat->Eco->FolGrp; CurGrp; CurGrp = CurGrp->Next)
			{
			if (CurGrp->Enabled)
				CurGrp->GroupPercentage = CurGrp->RelativeGroupDensity * SumGroupDensity;
			else
				CurGrp->GroupPercentage = 0.0;
			} // for
		} // if
	else
		return (1);
	} // else
*/
// if foliage effect, choose a group to render from. Only going to render one unit.
if (ForDat->FolEffect)
	{
	Random[0] = ForDat->Eco->Rand.GenPRN();
	// the granularity of the random numbers is .00001 - 
	// this prohibits fine control over foliage density at low densities per polygon.
	// The solution is to add a fraction of that granularity if the random # is very small.
	if (Random[0] < .0001)
		Random[0] += (.00001 * ForDat->Eco->Rand.GenPRN());
	// find the Foliage to draw
	SumImageFinder = 0.0;
	for (StrtGrp = ForDat->Eco->FolGrp; StrtGrp && StrtGrp->Next; StrtGrp = StrtGrp->Next, GroupNum ++)
		{
		SumImageFinder += StrtGrp->GroupPercentage;
		if (SumImageFinder >= Random[0])
			break;
		#ifdef WCS_BUILD_GARY
		GroupCt ++;
		#endif // WCS_BUILD_GARY
		} // while
	} // if
else
	StrtGrp = ForDat->Eco->FolGrp;

// F2_NOTE: This would be an optimum point to parallize the code.  DevCounter is hit over 500k times for Railway project.
for (CurGrp = StrtGrp; CurGrp && Success; CurGrp = CurGrp->Next, GroupNum ++)
	{
	//++DevCounter[46];
	if (CurGrp->GroupPercentage > 0.0)
		{
		// copy data over from group into common data structure for ready access. Added 3/27/08 GRH
		if (ForDat->Eco->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
			{
			ForDat->NormalizedDensity = CurGrp->NormalizedDensity;
			ForDat->RawDensity = CurGrp->RawDensity;
			if (ForDat->Eco->ConstDensity == WCS_FOLIAGE_DENSITY_CONSTANT)
				ForDat->AvgNumStems = ForDat->NormalizedDensity * ForDat->Poly->Area;	// Area is always positive calculated by SolveTriangleArea()
			else
				ForDat->AvgNumStems = ForDat->NormalizedDensity;
			} // if
		if (ForDat->Eco->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
			{
			ForDat->HeightRange = CurGrp->HeightRange;
			ForDat->MinHeight = CurGrp->MinHeight;
			ForDat->MaxHeight = CurGrp->MaxHeight;
			ForDat->AgvHeight = CurGrp->AgvHeight;
			} // if
		// removed and replaced by above 3/27/08 GRH
		//ForDat->GroupDensityComputed = ForDat->GroupSizeComputed = false;
		// if there are thematic maps for any of the foliage %'s then they need to be evaluated and new
		// relative percentages calculated for each foliage
		SumFoliageDensity = 0.0;
		for (CurFol = CurGrp->Fol; CurFol; CurFol = CurFol->Next)
			{
			if (CurFol->Enabled)
				{
				CurFol->ComputeFoliageRelativeDensity(ForDat);
				SumFoliageDensity += CurFol->RelativeFoliageDensity;
				CurFol->ComputeFoliageRelativeSize(ForDat);
				} // if
			} // for
		if (SumFoliageDensity > 0.0)
			{
			SumFoliageDensity = 1.0 / SumFoliageDensity;
			for (CurFol = CurGrp->Fol; CurFol; CurFol = CurFol->Next)
				{
				if (CurFol->Enabled)
					CurFol->FoliagePercentage = CurFol->RelativeFoliageDensity * SumFoliageDensity;
				else
					CurFol->FoliagePercentage = 0.0;
				} // for

			if (! ForDat->FolEffect)
				{
				// by reseeding the PRNG for each group with a unique value we can get the same trees drawn
				// even if an earlier group becomes disabled or the number of instances of an earlier group changes
				if (GroupNum > 1)
					ForDat->Eco->Rand.Seed64BitShift(ForDat->Poly->LatSeed + ForDat->SeedOffset * GroupNum, ForDat->Poly->LonSeed + ForDat->SeedOffset * GroupNum);
				// if density is in groups we need to compute the untextured density now
				// if textures apply they will be calculated below using vertex locations of actual stems
				// removed and replaced by above 3/27/08 GRH
				//if (ForDat->Eco->AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
				//	{
				//	CurGrp->ComputeGroupDensity(ForDat);
				//	// average number of stems per polygon of this size
				//	if (ForDat->Eco->ConstDensity == WCS_FOLIAGE_DENSITY_CONSTANT)
				//		ForDat->AvgNumStems = ForDat->NormalizedDensity * ForDat->Poly->Area;	// Area is always positive calculated by SolveTriangleArea()
				//	else
				//		ForDat->AvgNumStems = ForDat->NormalizedDensity;
				//	} // if
				Random[0] = ForDat->Eco->Rand.GenPRN();
				// the granularity of the random numbers is .00001 - 
				// this prohibits fine control over foliage density at low densities per polygon.
				// The solution is to add a fraction of that granularity if the random # is very small.
				if (Random[0] < .0001)
					Random[0] += (.00001 * ForDat->Eco->Rand.GenPRN());

				/* <<<>>> removed and replaced 7/22/04
				if (ForDat->Eco->ConstDensity == WCS_FOLIAGE_DENSITY_CONSTANT)
					{
					// render a number of stems of that group until random numbers exceed the average per group
					// for each group - multiply the group percentage times the average number of stems
					// this method resulted in understating the number of stems in low constant density situations
					MaxStemsThisGroup = CurGrp->GroupPercentage * ForDat->AvgNumStems * .5;
					} // if
				else
					{
					// variable density requres a slightly different method to make it come out with the right density
					MaxStemsThisGroup = CurGrp->GroupPercentage * ForDat->AvgNumStems;
					} // else
				*/
				// this is the old method from VNS 1 and it seems to work very well. Tested it on 7/22/04
				MaxStemsThisGroup = ((ForDat->Eco->Rand.GenGauss() * (1.0 / 6.9282)) + 1.0) * CurGrp->GroupPercentage * ForDat->AvgNumStems;

				GroupSum = Random[0];
				} // if
			else
				{
				GroupSum = 0.0;
				MaxStemsThisGroup = 1.0;
				} // else
			while (GroupSum <= MaxStemsThisGroup)
				{
				// compute group size if needed before textures are initialized for this foliage vertex location
				// removed and replaced by above 3/27/08 GRH
				//if (! ForDat->GroupSizeComputed)
				//	{
				//	if (ForDat->Eco->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
				//		{
				//		CurGrp->ComputeGroupSize(ForDat);
				//		} // if
				//	ForDat->GroupSizeComputed = true;
				//	} // if

				ForDat->Eco->Rand.GenMultiPRN(9, &Random[1]);
				// calculate a random position for this foliage stem within the polygon
				if (! ForDat->FolEffect)
					{
					// method for evenly distributing points over a triangle adopted for VNS 2/WCS 6
					if (Random[1] + Random[2] > 1.0)
						{
						Random[1] = 1.0 - Random[1];
						Random[2] = 1.0 - Random[2];
						} // if
					ThirdVtxWt = 1 - Random[1] - Random[2];
					FolVtx.Lat = ForDat->Vtx[0]->Lat * Random[1] + ForDat->Vtx[1]->Lat * Random[2] + ForDat->Vtx[2]->Lat * ThirdVtxWt;
					FolVtx.Lon = ForDat->Vtx[0]->Lon * Random[1] + ForDat->Vtx[1]->Lon * Random[2] + ForDat->Vtx[2]->Lon * ThirdVtxWt;
					FolVtx.Elev = ForDat->Vtx[0]->Elev * Random[1] + ForDat->Vtx[1]->Elev * Random[2] + ForDat->Vtx[2]->Elev * ThirdVtxWt;

					#ifdef WCS_VECPOLY_EFFECTS
					RenderThisItem = true;
					// If there is a beach involved in this polygon we need to find out if the foliage is above the beach line
					if (ForDat->Vtx[0]->NodeData->BeachLevel < FLT_MAX || ForDat->Vtx[1]->NodeData->BeachLevel < FLT_MAX || ForDat->Vtx[2]->NodeData->BeachLevel < FLT_MAX)
						{
						BeachElev = (ForDat->Vtx[0]->NodeData->BeachLevel == FLT_MAX ? FolVtx.Elev: ForDat->Vtx[0]->NodeData->BeachLevel) * Random[1];
						BeachElev += (ForDat->Vtx[1]->NodeData->BeachLevel == FLT_MAX ? FolVtx.Elev: ForDat->Vtx[1]->NodeData->BeachLevel) * Random[2];
						BeachElev += (ForDat->Vtx[2]->NodeData->BeachLevel == FLT_MAX ? FolVtx.Elev: ForDat->Vtx[2]->NodeData->BeachLevel) * ThirdVtxWt;
						RenderThisItem = ForDat->RenderAboveBeachLevel ? (FolVtx.Elev > BeachElev): (FolVtx.Elev <= BeachElev);
						} // if
						
					// VNS 3 allows foliage density to vary over the polygon based on vertex weights
					if (RenderThisItem && ForDat->VertexWeights)
						{
						// weighting of this foliage position based on same weights used for calculating the positional coordinates
						FolMaterialWeight = ForDat->VertexWeights[0] * Random[1] + ForDat->VertexWeights[1] * Random[2] + ForDat->VertexWeights[2] * ThirdVtxWt;
						// if a random number is above FolMaterialWeight, throw out this instance but count it as one done
						RenderThisItem = (Random[9] <= FolMaterialWeight);
						} // if
					#endif // WCS_VECPOLY_EFFECTS
					} // if
				else
					{
					FolVtx.Lat = ForDat->Poly->Lat;
					FolVtx.Lon = ForDat->Poly->Lon;
					FolVtx.Elev = ForDat->Poly->Elev;
					} // else

				#ifdef WCS_VECPOLY_EFFECTS
				if (RenderThisItem)
				#endif // WCS_VECPOLY_EFFECTS
					{
					// project the foliage base position
					Cam->ProjectVertexDEM(DefCoords, &FolVtx, EarthLatScaleMeters, PlanetRad, 1);	// 1 means convert vertex to cartesian

					// in case textures exist
					RendData.TransferTextureData(ForDat->Poly);
					// set foliage coordinates
					RendData.TexData.Elev = FolVtx.Elev;
					RendData.TexData.Latitude = FolVtx.Lat;
					RendData.TexData.Longitude = FolVtx.Lon;
					RendData.TexData.ZDist = FolVtx.ScrnXYZ[2];
					RendData.TexData.QDist = FolVtx.Q;
					RendData.TexData.VDEM[0] = &FolVtx;
					RendData.TexData.WaterDepth = ForDat->Poly->WaterElev - FolVtx.Elev;
					RendData.TexData.VDataVecOffsetsValid = 0;

					// if there are ecotype or group textures calculate them and multiply together
					// test to see if the second RN falls below the product
					InstanceTexturedDensity = 1.0;
					if (! ForDat->FolEffect)
						{
						if (ForDat->Eco->EcoDensityTex)
							{
							if ((TexOpacity = ForDat->Eco->EcoDensityTex->Eval(Value, &RendData.TexData)) > 0.0)
								{
								if (TexOpacity < 1.0)
									{
									// Value[0] has already been diminished by the texture's opacity
									InstanceTexturedDensity *= (1.0 - TexOpacity + Value[0]);
									} // if
								else
									InstanceTexturedDensity *= Value[0];
								} // if
							} // if
						if (CurGrp->GroupDensityTex)
							{
							if ((TexOpacity = CurGrp->GroupDensityTex->Eval(Value, &RendData.TexData)) > 0.0)
								{
								if (TexOpacity < 1.0)
									{
									// Value[0] has already been diminished by the texture's opacity
									InstanceTexturedDensity *= (1.0 - TexOpacity + Value[0]);
									} // if
								else
									InstanceTexturedDensity *= Value[0];
								} // if
							} // if
						} // if

					// test to see if ecotype or group textures wiped out this instance
					if (Random[3] <= InstanceTexturedDensity)
						{
						// find the Foliage to draw
						SumImageFinder = 0.0;
						#ifdef WCS_BUILD_GARY
						ImageCt = 0;
						#endif // WCS_BUILD_GARY
						for (CurFol = CurGrp->Fol; CurFol && CurFol->Next; CurFol = CurFol->Next)
							{
							SumImageFinder += CurFol->FoliagePercentage;
							if (SumImageFinder >= Random[4])
								break;
							#ifdef WCS_BUILD_GARY
							ImageCt ++;
							#endif // WCS_BUILD_GARY
							} // while

						if (CurFol)
							{
							// evaluate foliage density texture
							if (! ForDat->FolEffect && CurFol->FoliageDensityTex)
								{
								if ((TexOpacity = CurFol->FoliageDensityTex->Eval(Value, &RendData.TexData)) > 0.0)
									{
									if (TexOpacity < 1.0)
										{
										// Value[0] has already been diminished by the texture's opacity
										InstanceTexturedDensity *= (1.0 - TexOpacity + Value[0]);
										} // if
									else
										InstanceTexturedDensity *= Value[0];
									} // if
								} // if

							// test it again taking foliage texture into account
							if (Random[3] <= InstanceTexturedDensity)
								{
								// looks like we're really going to render this thing so now we have to figure out how big
								FolHeight = ForDat->MinHeight + ForDat->HeightRange * Random[5];
								// if absolute size is in ecotype, reduce by group size
								if (ForDat->Eco->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
									FolHeight *= CurGrp->RelativeGroupSize;
								// reduce by individual foliage size
								FolHeight *= CurFol->RelativeFoliageSize;

								if (ForDat->Eco->EcoSizeTex)
									{
									if ((TexOpacity = ForDat->Eco->EcoSizeTex->Eval(Value, &RendData.TexData)) > 0.0)
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
								if (CurGrp->GroupSizeTex)
									{
									if ((TexOpacity = CurGrp->GroupSizeTex->Eval(Value, &RendData.TexData)) > 0.0)
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
								if (CurFol->FoliageSizeTex)
									{
									if ((TexOpacity = CurFol->FoliageSizeTex->Eval(Value, &RendData.TexData)) > 0.0)
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

								// textured height has been found
								// store some diagnostic data
								#ifdef WCS_BUILD_GARY
								StemsRendered ++;
								if (GroupCt < 4)
									{
									GroupStemsRendered[GroupCt] ++;
									if (ImageCt < 8)
										ImageStemsRendered[GroupCt][ImageCt] ++;
									} // if
								#endif // WCS_BUILD_GARY

								if (FolHeight > ShadowMinFolHt || (! ShadowMapInProgress && FolHeight > 0.0))
									{
									if (! (Success = OneStepCloserToRenderingFoliage(ForDat, CurFol, &FolVtx, Random, FolHeight)))
										break;
									} // if
								} // if
							} // if
						} // if
					} // if item wasn't ignored
					
				// see if we need to draw another one
				if (ForDat->FolEffect || GroupSum >= MaxStemsThisGroup)
					break;
				if (ForDat->Eco->ConstDensity == WCS_FOLIAGE_DENSITY_CONSTANT)
					{
					// this replaced below 7/22/04
					MaxStemsThisGroup -= 1.0;
					/*
					Random[0] = ForDat->Eco->Rand.GenPRN();
					// see note near top of function for explanation
					if (Random[0] < .0001)
						Random[0] += (.00001 * ForDat->Eco->Rand.GenPRN());
					GroupSum += Random[0];
					if (GroupSum > MaxStemsThisGroup)
						{
						Diff = (GroupSum - MaxStemsThisGroup);
						Random[1] = ForDat->Eco->Rand.GenPRN();
						if (Random[1] <= Diff)
							GroupSum = MaxStemsThisGroup;
						} // if
					*/
					} // if
				else if (MaxStemsThisGroup > 1.0)
					{
					MaxStemsThisGroup -= 1.0;
					/* not needed anymore 7/22/04
					Random[0] = ForDat->Eco->Rand.GenPRN();
					// see note near top of function for explanation
					if (Random[0] < .0001)
						Random[0] += (.00001 * ForDat->Eco->Rand.GenPRN());
					GroupSum = Random[0];
					*/
					} // else
				else
					break;
				} // while
			} // if
		} // if
	if (ForDat->FolEffect)
		break;
	#ifdef WCS_BUILD_GARY
	GroupCt ++;
	#endif // WCS_BUILD_GARY
	} // for

return (Success);

} // Renderer::RenderForestryEcotype

/*===========================================================================*/

int Renderer::OneStepCloserToRenderingFoliage(ForestryRenderData *ForDat, Foliage *FolToRender, VertexDEM *FolVtx, 
	double *Random, double FolHeight)
{
double EnvHeightFactor, Rotate[3], Value[3], TexValue[3], TexOpacity, FolHeightFactor,
	CamFolVec[3], ForshortenAngle, MinPixelSize;
RasterAnimHost *ObjStash;
RealtimeFoliageData *FolListDat;
int ItemSuppressed = 0, Success = 1, FlipX;

Value[2] = Value[1] = Value[0] = 0.0;
TexValue[2] = TexValue[1] = TexValue[0] = 0.0;

FolPlotData.Poly = ForDat->Poly;
FolPlotData.Vert = FolVtx;
FolPlotData.Opacity = ForDat->RenderOpacity;

// Use factor in default Environment if no Environment specified for this polygon (could be Foliage or Ecosystem Effect).
EnvHeightFactor = ForDat->Poly->Env ? ForDat->Poly->Env->AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_FOLIAGEHTFACT].CurValue: 
	DefaultEnvironment->AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_FOLIAGEHTFACT].CurValue;
FolHeight *= EnvHeightFactor;
// now paths diverge between Image Objects and 3D Objects
if (FolToRender->FoliageType == WCS_FOLIAGE_TYPE_OBJECT3D)
	{
	if (FolToRender->Obj)
		{
		Rotate[0] = FolToRender->RandomRotate[0] ? FolToRender->Rotate[0] * 2.0 * (Random[6] - .5): 0.0;
		Rotate[1] = FolToRender->RandomRotate[1] ? FolToRender->Rotate[1] * 2.0 * (Random[7] - .5): 0.0;
		Rotate[2] = FolToRender->RandomRotate[2] ? FolToRender->Rotate[2] * 2.0 * (Random[8] - .5): 0.0;
		// render the object, pass the rotation set, height, shading options, position of base
		FolToRender->Obj->Rand.Copy(&ForDat->Eco->Rand);

		if (! ShadowMapInProgress)
			{
			if (FolToRender->Obj->ShadowsOnly)
				ItemSuppressed = 1;
			else if (RealTimeFoliageWrite && RealTimeFoliageWrite->Include3DO)
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
							FolListDat->ElementID = -(short)EffectsBase->GetObjectID(FolToRender->Obj->EffectType, FolToRender->Obj);
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
							if (ForDat->FolEffect)
								{
								if (((FoliageEffect *)ForDat->Poly->Object)->IsClickQueryEnabled())
									{
									if (FolToRender->Obj->IsClickQueryEnabled())
										FolListDat->MyEffect = FolToRender->Obj;
									else
										FolListDat->MyEffect = (FoliageEffect *)ForDat->Poly->Object;
									FolListDat->MyVec = ForDat->Poly->Vector;
									FolListDat->BitInfo |= WCS_REALTIME_FOLDAT_BITINFO_CLICKQUERY;
									} // if
								else
									{
									FolListDat->MyVec = ForDat->Poly->Vector;
									FolListDat->BitInfo |= WCS_REALTIME_FOLDAT_BITINFO_VECTORPRESENT;
									} // else
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
			} // if not making shadows
			
		if (! ItemSuppressed)
			{
			ObjStash = ForDat->Poly->Object;
			ForDat->Poly->Object = FolToRender->Obj;
			Render3DObject(ForDat->Poly, FolVtx, FolToRender->Obj, Rotate, FolHeight);
			ForDat->Poly->Object = ObjStash;
			RendData.TexData.Object3D = NULL;
			} // if
		} // if
	} // if 3D Object
else
	{
	if (FolToRender->Rast)
		{
		FlipX = (FolToRender->FlipX && Random[6] >= .5);
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
		if (ForDat->Poly->TintFoliage)
			{
			Value[0] = ForDat->Poly->RGB[0];
			Value[1] = ForDat->Poly->RGB[1];
			Value[2] = ForDat->Poly->RGB[2];
			TexOpacity = 1.0;
			} // if
		else if (FolToRender->FoliageColorTheme && FolToRender->FoliageColorTheme->Eval(Value, ForDat->Poly->Vector))
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
		else if (FolToRender->FoliageColorTex)
			{
			if ((TexOpacity = FolToRender->FoliageColorTex->Eval(Value, &RendData.TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// if gray image blend texture color with replacement color
					if (! FolToRender->ColorImage)
						{
						Value[0] += FolToRender->Color.GetCompleteValue(0) * (1.0 - TexOpacity);
						Value[1] += FolToRender->Color.GetCompleteValue(1) * (1.0 - TexOpacity);
						Value[2] += FolToRender->Color.GetCompleteValue(2) * (1.0 - TexOpacity);
						} // else
					} // if
				} // if
			else if (! FolToRender->ColorImage)
				{
				Value[0] = FolToRender->Color.GetCompleteValue(0);
				Value[1] = FolToRender->Color.GetCompleteValue(1);
				Value[2] = FolToRender->Color.GetCompleteValue(2);
				} // else if gray image
			// else Value[] already set to 0's by texture evaluator
			} // if
		else
			{
			TexOpacity = 0.0;
			if (! FolToRender->ColorImage)
				{
				Value[0] = FolToRender->Color.GetCompleteValue(0);
				Value[1] = FolToRender->Color.GetCompleteValue(1);
				Value[2] = FolToRender->Color.GetCompleteValue(2);
				} // if gray image
			else
				{
				Value[0] = Value[1] = Value[2] = 0.0;
				} // else need to set Value[] to 0's
			} // else

		FolPlotData.OrientationShading = FolToRender->AnimPar[WCS_FOLIAGE_ANIMPAR_ORIENTATIONSHADING].CurValue;
		if (FolToRender->FoliageBacklightTex)
			{
			if ((TexOpacity = FolToRender->FoliageBacklightTex->Eval(TexValue, &RendData.TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					FolPlotData.OrientationShading *= (1.0 - TexOpacity + TexValue[0]);
					} // if
				else
					{
					FolPlotData.OrientationShading *= TexValue[0];
					} // if
				} // if
			} // if

		// calculate the tree height in pixels based on size and distance or Z
		if (Cam->Orthographic || Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
			FolHeightFactor = Cam->VertScale;
		else if (Cam->ApplyLensDistortion)
			FolHeightFactor = Cam->VertScale / FolVtx->Q;
		else
			FolHeightFactor = Cam->VertScale / FolVtx->ScrnXYZ[2];
		FolPlotData.HeightInPixels = FolHeight * FolHeightFactor;

		// compute the tree width based on image proportions and pixel aspect
		FolPlotData.WidthInPixels = FolPlotData.HeightInPixels * FolToRender->ImageWidthFactor / Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue;

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

		FolPlotData.MaxZOffset = FolHeight * FolToRender->ImageWidthFactor * .5;	// .25 was used in v4
		FolPlotData.TestPtrnOffset = ForDat->Poly->Env ? ForDat->Poly->Env->FoliageBlending: DefaultEnvironment->FoliageBlending;
		FolPlotData.TestPtrnOffset *= ZMergeDistance;
		MinPixelSize = ForDat->Poly->Env ? ForDat->Poly->Env->MinPixelSize: DefaultEnvironment->MinPixelSize;
		if (FolPlotData.WidthInPixels < MinPixelSize)
			FolPlotData.WidthInPixels = MinPixelSize;
		if (FolPlotData.HeightInPixels < MinPixelSize)
			FolPlotData.HeightInPixels = MinPixelSize;
		// paste the foliage image into the scene
		// pass Value[] for replacement color, 1 - TexOpacity for color image color opacity,
		//  Opacity for foliage opacity, FolToRender for various values including raster,
		//  pasted image size and position, FolPlotData.ElevGrad, FolPlotData.TopElev...

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
							
						FolListDat->ElementID = (short)FolToRender->Rast->GetID();
						FolListDat->BitInfo = FlipX ? WCS_REALTIME_FOLDAT_BITINFO_FLIPX: 0;
						if (FolToRender->Shade3D)
							FolListDat->BitInfo |= WCS_REALTIME_FOLDAT_BITINFO_SHADE3D;
						if ((FolListDat->ImageColorOpacity = (unsigned char)((1.0 - TexOpacity) * 255.99)) < 255)
							{
							FolListDat->TripleValue[0] = (unsigned char)WCS_min(Value[0] * 255.99, 255.0);
							FolListDat->TripleValue[1] = (unsigned char)WCS_min(Value[1] * 255.99, 255.0);
							FolListDat->TripleValue[2] = (unsigned char)WCS_min(Value[2] * 255.99, 255.0);
							} // if
						else if (! FolToRender->ColorImage)
							{
							FolListDat->ImageColorOpacity = 0;
							FolListDat->TripleValue[0] = (unsigned char)WCS_min(Value[0] * 255.99, 255.0);
							FolListDat->TripleValue[1] = (unsigned char)WCS_min(Value[1] * 255.99, 255.0);
							FolListDat->TripleValue[2] = (unsigned char)WCS_min(Value[2] * 255.99, 255.0);
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
						if (ForDat->FolEffect && ((FoliageEffect *)ForDat->Poly->Object)->IsClickQueryEnabled())
							{
							FolListDat->MyEffect = (FoliageEffect *)ForDat->Poly->Object;
							FolListDat->MyVec = ForDat->Poly->Vector;
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
			FolPlotData.SourceRast = FolToRender->Rast;
			FolPlotData.ColorImageOpacity = 1.0 - TexOpacity;
			FolPlotData.ReplaceColor = Value;
			FolPlotData.RenderOccluded = FolToRender->RenderOccluded;
			FolPlotData.Shade3D = FolToRender->Shade3D;
			FolPlotData.ColorImage = FolToRender->ColorImage;
			if (FolVtx->ScrnXYZ[2] > MinimumZ && FolVtx->ScrnXYZ[2] <= MaximumZ)
				{
				if (FlipX)
					PlotFoliageReverse();
				else
					PlotFoliage();
				} // if
			} // if
		} // if
	} // else it's an image Object

return (Success);

} // Renderer::OneStepCloserToRenderingFoliage

/*===========================================================================*/

void FoliageGroup::ComputeGroupRelativeDensity(ForestryRenderData *ForDat)
{
double Value[3];

Value[2] = Value[1] = Value[0] = 0.0;

if (GroupDensityTheme && GroupDensityTheme->Eval(Value, ForDat->Poly->Vector))
	{
	// relative densities are given in percent
	RelativeGroupDensity = Value[0] * .01;
	} // if 
else
	RelativeGroupDensity = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].CurValue * .01;

} // FoliageGroup::ComputeGroupRelativeDensity

/*===========================================================================*/

void Foliage::ComputeFoliageRelativeDensity(ForestryRenderData *ForDat)
{
double Value[3];

Value[2] = Value[1] = Value[0] = 0.0;

if (FoliageDensityTheme && FoliageDensityTheme->Eval(Value, ForDat->Poly->Vector))
	{
	// relative densities are given in percent
	RelativeFoliageDensity = Value[0] * .01;
	} // if 
else
	RelativeFoliageDensity = AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].CurValue;

} // Foliage::ComputeFoliageRelativeDensity

/*===========================================================================*/

void FoliageGroup::ComputeGroupRelativeSize(ForestryRenderData *ForDat)
{
double Value[3];

Value[2] = Value[1] = Value[0] = 0.0;

if (GroupSizeTheme && GroupSizeTheme->Eval(Value, ForDat->Poly->Vector))
	{
	// relative sizes are given in percent
	RelativeGroupSize = Value[0] * .01;
	} // if 
else
	RelativeGroupSize = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].CurValue * .01;

} // FoliageGroup::ComputeGroupRelativeSize

/*===========================================================================*/

void Foliage::ComputeFoliageRelativeSize(ForestryRenderData *ForDat)
{
double Value[3];

Value[2] = Value[1] = Value[0] = 0.0;

if (FoliageSizeTheme && FoliageSizeTheme->Eval(Value, ForDat->Poly->Vector))
	{
	// relative sizes are given in percent
	RelativeFoliageSize = Value[0] * .01;
	} // if 
else
	RelativeFoliageSize = AnimPar[WCS_FOLIAGE_ANIMPAR_HEIGHT].CurValue;

} // Foliage::ComputeFoliageRelativeSize

/*===========================================================================*/

void Ecotype::ComputeEcoSize(ForestryRenderData *ForDat)
{

if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_HEIGHT)
	{
	ComputeEcoSizeFromHt(ForDat);
	} // else
else if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
	{
	ComputeEcoSizeFromDbh(ForDat);
	} // else
else if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
	{
	ComputeEcoSizeFromAge(ForDat);
	} // else
else // (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE)
	{
	ComputeEcoSizeFromClosure(ForDat);
	} // if

ForDat->EcoSizeComputed = true;

} // Ecotype::ComputeEcoSize

/*===========================================================================*/

void Ecotype::ComputeEcoDensity(ForestryRenderData *ForDat)
{

// get a number for density
if (DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA)
	{
	ComputeEcoDensityFromStems(ForDat);
	} // else
else if (DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA)
	{
	ComputeEcoDensityFromBasalArea(ForDat);
	} // else
else // (DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
	{
	ComputeEcoDensityFromClosure(ForDat);
	} // if

// normalize the density to /meter**2
if (ConstDensity == WCS_FOLIAGE_DENSITY_CONSTANT)
	{
	if (DensityMethod != WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
		{
		ForDat->NormalizedDensity = ForDat->RawDensity * DensityNormalizer;
		} // if
	else
		{
		// for closure, density is already in / sq meter
		ForDat->NormalizedDensity = ForDat->RawDensity;
		} // else
	} // if
else
	{
	ForDat->NormalizedDensity = ForDat->RawDensity * .01;
	} // variable density

ForDat->EcoDensityComputed = true;

} // Ecotype::ComputeEcoDensity

/*===========================================================================*/

void Ecotype::ComputeEcoSizeFromHt(ForestryRenderData *ForDat)
{
double MinHt, Value[3];

ComputeEcoHeight(ForDat);

Value[2] = Value[1] = Value[0] = 0.0;
// is there a thematic map for min height?
if (EcoMinHeightTheme && EcoMinHeightTheme->Eval(Value, ForDat->Poly->Vector))
	{
	MinHt = Value[0];
	} // if
else
	MinHt = AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue;

if (SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
	{
	ForDat->MinHeight = MinHt;
	if (ForDat->MinHeight > ForDat->MaxHeight)
		swmem(&ForDat->MinHeight, &ForDat->MaxHeight, sizeof (double));
	ForDat->HeightRange = ForDat->MaxHeight - ForDat->MinHeight;
	} // if
else if (SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
	{
	ForDat->MinHeight = ForDat->MaxHeight * MinHt * .01;
	ForDat->HeightRange = ForDat->MaxHeight - ForDat->MinHeight;
	} // if
else // (SecondHeightType == WCS_ECOTYPE_SECONDHT_RANGEPCT)
	{
	ForDat->HeightRange = ForDat->MaxHeight * MinHt * .01;
	ForDat->MinHeight = ForDat->MaxHeight - ForDat->HeightRange;
	ForDat->MaxHeight += ForDat->HeightRange;
	} // if

ForDat->AgvHeight = ForDat->MinHeight + ForDat->HeightRange * .5;

} // Ecotype::ComputeEcoSizeFromHt

/*===========================================================================*/

void Ecotype::ComputeEcoSizeFromDbh(ForestryRenderData *ForDat)
{
double MinHt, Value[3];

ComputeEcoDbh(ForDat);
// look up height in Dbh graph
ForDat->MaxHeight = DBHCurve.GetValue(0, ForDat->MaxDbh);

Value[2] = Value[1] = Value[0] = 0.0;
// is there a thematic map for min height?
if (EcoMinHeightTheme && EcoMinHeightTheme->Eval(Value, ForDat->Poly->Vector))
	{
	MinHt = Value[0];
	} // if
else
	MinHt = AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue;

if (SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
	{
	ForDat->MinHeight = DBHCurve.GetValue(0, MinHt);
	if (ForDat->MinHeight > ForDat->MaxHeight)
		swmem(&ForDat->MinHeight, &ForDat->MaxHeight, sizeof (double));
	ForDat->HeightRange = ForDat->MaxHeight - ForDat->MinHeight;
	} // if
else if (SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
	{
	ForDat->MinHeight = ForDat->MaxHeight * MinHt * .01;
	ForDat->HeightRange = ForDat->MaxHeight - ForDat->MinHeight;
	} // if
else // (SecondHeightType == WCS_ECOTYPE_SECONDHT_RANGEPCT)
	{
	ForDat->HeightRange = ForDat->MaxHeight * MinHt * .01;
	ForDat->MinHeight = ForDat->MaxHeight - ForDat->HeightRange;
	ForDat->MaxHeight += ForDat->HeightRange;
	} // if

ForDat->AgvHeight = ForDat->MinHeight + ForDat->HeightRange * .5;

} // Ecotype::ComputeEcoSizeFromDbh

/*===========================================================================*/

void Ecotype::ComputeEcoSizeFromAge(ForestryRenderData *ForDat)
{
double MinHt, Value[3];

ComputeEcoAge(ForDat);
// look up height in Age graph
ForDat->MaxHeight = AgeCurve.GetValue(0, ForDat->MaxAge);

Value[2] = Value[1] = Value[0] = 0.0;
// is there a thematic map for min height?
if (EcoMinHeightTheme && EcoMinHeightTheme->Eval(Value, ForDat->Poly->Vector))
	{
	MinHt = Value[0];
	} // if
else
	MinHt = AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue;

if (SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
	{
	ForDat->MinHeight = AgeCurve.GetValue(0, MinHt);
	if (ForDat->MinHeight > ForDat->MaxHeight)
		swmem(&ForDat->MinHeight, &ForDat->MaxHeight, sizeof (double));
	ForDat->HeightRange = ForDat->MaxHeight - ForDat->MinHeight;
	} // if
else if (SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
	{
	ForDat->MinHeight = ForDat->MaxHeight * MinHt * .01;
	ForDat->HeightRange = ForDat->MaxHeight - ForDat->MinHeight;
	} // if
else // (SecondHeightType == WCS_ECOTYPE_SECONDHT_RANGEPCT)
	{
	ForDat->HeightRange = ForDat->MaxHeight * MinHt * .01;
	ForDat->MinHeight = ForDat->MaxHeight - ForDat->HeightRange;
	ForDat->MaxHeight += ForDat->HeightRange;
	} // if

ForDat->AgvHeight = ForDat->MinHeight + ForDat->HeightRange * .5;

} // Ecotype::ComputeEcoSizeFromAge

/*===========================================================================*/

void Ecotype::ComputeEcoSizeFromClosure(ForestryRenderData *ForDat)
{
double MinHt, Value[3];

// compute normalized density, reduced by any texture if necessary
if (! ForDat->EcoDensityComputed)
	ComputeEcoDensity(ForDat);
ComputeEcoClosure(ForDat);

// normalized density is per square meter
// closure is in % 0-1
// need average image height / width (h/r) for all images in the ecotype

// each image takes up an area of pi * radius**2

// since we know the density and closure we can derive the average area per tree

// if we have density as number of stems per sq meter, let us pretend we have an area of 10000 sq meter (1 ha)
// in that area we have S = D * 10000 stems.
// if the crown closure is C the area covered by S trees is A = 10000 * C
// The area covered by each tree is TA = A / S
// The radius of each tree is R = sqrt(TA / pi)
// The width of each tree is W = 2 * R
// The height of each tree is H = W * h/r

// example
// D = .02, C = .5, h/r = 2
// S = 200, A = 5000
// TA = 25
// R = 2.82m
// W = 5.64m
// H = 11.28m = 36ft

if (ForDat->NormalizedDensity > 0.0)
	{
	ForDat->MaxHeight = 2.0 * sqrt(ForDat->MaxClosure / (ForDat->NormalizedDensity * Pi)) * AvgImageHeightWidthRatio;

	Value[2] = Value[1] = Value[0] = 0.0;
	// is there a thematic map for min height?
	if (EcoMinHeightTheme && EcoMinHeightTheme->Eval(Value, ForDat->Poly->Vector))
		{
		MinHt = Value[0];
		} // if
	else
		MinHt = AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue;

	if (SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
		{
		ForDat->MinHeight = 2.0 * sqrt(MinHt / (ForDat->NormalizedDensity * Pi)) * AvgImageHeightWidthRatio;
		if (ForDat->MinHeight > ForDat->MaxHeight)
			swmem(&ForDat->MinHeight, &ForDat->MaxHeight, sizeof (double));
		ForDat->HeightRange = ForDat->MaxHeight - ForDat->MinHeight;
		} // if
	else if (SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
		{
		ForDat->MinHeight = ForDat->MaxHeight * MinHt * .01;
		ForDat->HeightRange = ForDat->MaxHeight - ForDat->MinHeight;
		} // if
	else // (SecondHeightType == WCS_ECOTYPE_SECONDHT_RANGEPCT)
		{
		ForDat->HeightRange = ForDat->MaxHeight * MinHt * .01;
		ForDat->MinHeight = ForDat->MaxHeight - ForDat->HeightRange;
		ForDat->MaxHeight += ForDat->HeightRange;
		} // if

	ForDat->AgvHeight = ForDat->MinHeight + ForDat->HeightRange * .5;
	} // if
else
	ForDat->AgvHeight = ForDat->MaxHeight = ForDat->MinHeight = ForDat->HeightRange = 0.0;

} // Ecotype::ComputeEcoSizeFromClosure

/*===========================================================================*/

void Ecotype::ComputeEcoDensityFromStems(ForestryRenderData *ForDat)
{

ComputeEcoStems(ForDat);
ForDat->RawDensity = ForDat->MaxStems;

} // Ecotype::ComputeEcoDensityFromStems

/*===========================================================================*/

void Ecotype::ComputeEcoDensityFromBasalArea(ForestryRenderData *ForDat)
{
double R;

ComputeEcoBasalArea(ForDat);
ComputeEcoDbh(ForDat);

// assuming that Dbh is in meters and basal area is in a user specified units
// typically dbh is in inches, basal area in square feet / area
// we have a unit specifier for area so basal area to the same units as diameter

// R = DBH * .5
// density/unit area = BA / R * R * Pi
if (ForDat->MaxDbh > 0.0)
	{
	R = ForDat->MaxDbh * .5;
	// BasalAreaUnitConversion converts area to square meters since R is in meters
	ForDat->RawDensity = ForDat->MaxBasalArea * BasalAreaUnitConversion / (R * R * Pi);
	} // if
else
	ForDat->RawDensity = 0.0;

} // Ecotype::ComputeEcoDensityFromBasalArea

/*===========================================================================*/

void Ecotype::ComputeEcoDensityFromClosure(ForestryRenderData *ForDat)
{
double TA;

if (! ForDat->EcoSizeComputed)
	ComputeEcoSize(ForDat);
ComputeEcoClosure(ForDat);

if (ForDat->MaxClosure > 0.0 && AvgImageHeightWidthRatio > 0.0 && ForDat->AgvHeight > 0.0)
	{
	// D = .02, C = .5, h/r = 2
	// S = 200, A = 5000
	// TA = 25
	// R = 2.82m
	// W = 5.64m
	// H = 11.28m = 36ft

	// H = 2 * sqrt(C / (D * Pi)) * h/r
	// (H / (2 * (h/r)))**2 = C / (D * Pi)
	// (D * Pi) = C / (H / (2 * (h/r)))**2
	// D = C / (Pi * (H / (2 * (h/r)))**2)

	TA = ForDat->AgvHeight / (2.0 * AvgImageHeightWidthRatio);
	ForDat->RawDensity = ForDat->MaxClosure / (Pi * TA * TA);
	} // if
else
	ForDat->RawDensity = 0.0;

} // Ecotype::ComputeEcoDensityFromClosure

/*===========================================================================*/

void Ecotype::ComputeEcoHeight(ForestryRenderData *ForDat)
{
double Value[3];

Value[2] = Value[1] = Value[0] = 0.0;

// is there a thematic map for height?
if (EcoHeightTheme && EcoHeightTheme->Eval(Value, ForDat->Poly->Vector))
	{
	ForDat->MaxHeight = Value[0];
	} // if
else
	ForDat->MaxHeight = AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue;

} // Ecotype::ComputeEcoHeight

/*===========================================================================*/

void Ecotype::ComputeEcoDbh(ForestryRenderData *ForDat)
{
double Value[3];

Value[2] = Value[1] = Value[0] = 0.0;

// is there a thematic map for Dbh?
if (EcoDbhTheme && EcoDbhTheme->Eval(Value, ForDat->Poly->Vector))
	{
	ForDat->MaxDbh = Value[0];
	} // if
else
	ForDat->MaxDbh = AnimPar[WCS_ECOTYPE_ANIMPAR_DBH].CurValue;

} // Ecotype::ComputeEcoDbh

/*===========================================================================*/

void Ecotype::ComputeEcoAge(ForestryRenderData *ForDat)
{
double Value[3];

Value[2] = Value[1] = Value[0] = 0.0;

// is there a thematic map for age?
if (EcoAgeTheme && EcoAgeTheme->Eval(Value, ForDat->Poly->Vector))
	{
	ForDat->MaxAge = Value[0];
	} // if
else
	ForDat->MaxAge = AnimPar[WCS_ECOTYPE_ANIMPAR_AGE].CurValue;

} // Ecotype::ComputeEcoAge

/*===========================================================================*/

void Ecotype::ComputeEcoClosure(ForestryRenderData *ForDat)
{
double Value[3];

Value[2] = Value[1] = Value[0] = 0.0;

// is there a thematic map for closure?
if (EcoClosureTheme && EcoClosureTheme->Eval(Value, ForDat->Poly->Vector))
	{
	ForDat->MaxClosure = Value[0];
	} // if
else
	ForDat->MaxClosure = AnimPar[WCS_ECOTYPE_ANIMPAR_CROWNCLOSURE].CurValue;

} // Ecotype::ComputeEcoClosure

/*===========================================================================*/

void Ecotype::ComputeEcoStems(ForestryRenderData *ForDat)
{
double Value[3];

Value[2] = Value[1] = Value[0] = 0.0;

// is there a thematic map for stems?
if (EcoStemsTheme && EcoStemsTheme->Eval(Value, ForDat->Poly->Vector))
	{
	ForDat->MaxStems = Value[0];
	} // if
else
	ForDat->MaxStems = AnimPar[WCS_ECOTYPE_ANIMPAR_DENSITY].CurValue;

} // Ecotype::ComputeEcoStems

/*===========================================================================*/

void Ecotype::ComputeEcoBasalArea(ForestryRenderData *ForDat)
{
double Value[3];

Value[2] = Value[1] = Value[0] = 0.0;

// is there a thematic map for basal area?
if (EcoBasalAreaTheme && EcoBasalAreaTheme->Eval(Value, ForDat->Poly->Vector))
	{
	ForDat->MaxBasalArea = Value[0];
	} // if
else
	ForDat->MaxBasalArea = AnimPar[WCS_ECOTYPE_ANIMPAR_BASALAREA].CurValue;

} // Ecotype::ComputeEcoBasalArea

/*===========================================================================*/

void FoliageGroup::ComputeGroupSize(ForestryRenderData *ForDat)
{

if (! GroupSizeComputed)
	{
	if (ForDat->Eco->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_HEIGHT)
		{
		ComputeGroupSizeFromHt(ForDat);
		} // else
	else if (ForDat->Eco->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
		{
		ComputeGroupSizeFromDbh(ForDat);
		} // else
	else if (ForDat->Eco->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
		{
		ComputeGroupSizeFromAge(ForDat);
		} // else
	else // (ForDat->Eco->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE)
		{
		ComputeGroupSizeFromClosure(ForDat);
		} // if
	} // if
	
GroupSizeComputed = true;

} // FoliageGroup::ComputeGroupSize

/*===========================================================================*/

void FoliageGroup::ComputeGroupDensity(ForestryRenderData *ForDat)
{

if (! GroupDensityComputed)
	{
	// get a number for density
	if (ForDat->Eco->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA)
		{
		ComputeGroupDensityFromStems(ForDat);
		} // else
	else if (ForDat->Eco->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA)
		{
		ComputeGroupDensityFromBasalArea(ForDat);
		} // else
	else // (ForDat->Eco->DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
		{
		ComputeGroupDensityFromClosure(ForDat);
		} // if

	// normalize the density to /meter**2
	if (ForDat->Eco->ConstDensity == WCS_FOLIAGE_DENSITY_CONSTANT)
		{
		if (ForDat->Eco->DensityMethod != WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
			{
			NormalizedDensity = RawDensity * ForDat->Eco->DensityNormalizer;
			} // if
		else
			{
			// for closure, density is already in / sq meter
			NormalizedDensity = RawDensity;
			} // else
		} // if
	else
		{
		NormalizedDensity = RawDensity * .01;
		} // variable density
	} // if
	
GroupDensityComputed = true;

} // FoliageGroup::ComputeGroupDensity

/*===========================================================================*/

void FoliageGroup::ComputeGroupSizeFromHt(ForestryRenderData *ForDat)
{
double MinHt, Value[3];

ComputeGroupHeight(ForDat);

Value[2] = Value[1] = Value[0] = 0.0;

// is there a thematic map for min height?
if (GroupMinHeightTheme && GroupMinHeightTheme->Eval(Value, ForDat->Poly->Vector))
	{
	MinHt = Value[0];
	} // if
else
	MinHt = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue;

if (ForDat->Eco->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
	{
	MinHeight = MinHt;
	if (MinHeight > MaxHeight)
		swmem(&MinHeight, &MaxHeight, sizeof (double));
	HeightRange = MaxHeight - MinHeight;
	} // if
else if (ForDat->Eco->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
	{
	MinHeight = MaxHeight * MinHt * .01;
	HeightRange = MaxHeight - MinHeight;
	} // if
else // (ForDat->Eco->SecondHeightType == WCS_ECOTYPE_SECONDHT_RANGEPCT)
	{
	HeightRange = MaxHeight * MinHt * .01;
	MinHeight = MaxHeight - HeightRange;
	MaxHeight += HeightRange;
	} // if

AgvHeight = MinHeight + HeightRange * .5;

} // FoliageGroup::ComputeGroupSizeFromHt

/*===========================================================================*/

void FoliageGroup::ComputeGroupSizeFromDbh(ForestryRenderData *ForDat)
{
double MinHt, Value[3];

ComputeGroupDbh(ForDat);
// look up height in Dbh graph
MaxHeight = DBHCurve.GetValue(0, MaxDbh);

Value[2] = Value[1] = Value[0] = 0.0;

// is there a thematic map for min height?
if (GroupMinHeightTheme && GroupMinHeightTheme->Eval(Value, ForDat->Poly->Vector))
	{
	MinHt = Value[0];
	} // if
else
	MinHt = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue;

if (ForDat->Eco->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
	{
	MinHeight = DBHCurve.GetValue(0, MinHt);
	if (MinHeight > MaxHeight)
		swmem(&MinHeight, &MaxHeight, sizeof (double));
	HeightRange = MaxHeight - MinHeight;
	} // if
else if (ForDat->Eco->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
	{
	MinHeight = MaxHeight * MinHt * .01;
	HeightRange = MaxHeight - MinHeight;
	} // if
else // (ForDat->Eco->SecondHeightType == WCS_ECOTYPE_SECONDHT_RANGEPCT)
	{
	HeightRange = MaxHeight * MinHt * .01;
	MinHeight = MaxHeight - HeightRange;
	MaxHeight += HeightRange;
	} // if

AgvHeight = MinHeight + HeightRange * .5;

} // FoliageGroup::ComputeGroupSizeFromDbh

/*===========================================================================*/

void FoliageGroup::ComputeGroupSizeFromAge(ForestryRenderData *ForDat)
{
double MinHt, Value[3];

ComputeGroupAge(ForDat);
// look up height in Age graph
MaxHeight = AgeCurve.GetValue(0, MaxAge);

Value[2] = Value[1] = Value[0] = 0.0;

// is there a thematic map for min height?
if (GroupMinHeightTheme && GroupMinHeightTheme->Eval(Value, ForDat->Poly->Vector))
	{
	MinHt = Value[0];
	} // if
else
	MinHt = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue;

if (ForDat->Eco->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
	{
	MinHeight = AgeCurve.GetValue(0, MinHt);
	if (MinHeight > MaxHeight)
		swmem(&MinHeight, &MaxHeight, sizeof (double));
	HeightRange = MaxHeight - MinHeight;
	} // if
else if (ForDat->Eco->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
	{
	MinHeight = MaxHeight * MinHt * .01;
	HeightRange = MaxHeight - MinHeight;
	} // if
else // (ForDat->Eco->SecondHeightType == WCS_ECOTYPE_SECONDHT_RANGEPCT)
	{
	HeightRange = MaxHeight * MinHt * .01;
	MinHeight = MaxHeight - HeightRange;
	MaxHeight += HeightRange;
	} // if

AgvHeight = MinHeight + HeightRange * .5;

} // FoliageGroup::ComputeGroupSizeFromAge

/*===========================================================================*/

void FoliageGroup::ComputeGroupSizeFromClosure(ForestryRenderData *ForDat)
{
double MinHt, Value[3];

// compute normalized density, reduced by any texture if necessary
if (! GroupDensityComputed)
	ComputeGroupDensity(ForDat);
ComputeGroupClosure(ForDat);

// normalized density is per square meter
// closure is in % 0-1
// need average image height / width (h/r) for all images in the ecotype

// each image takes up an area of pi * radius**2

// since we know the density and closure we can derive the average area per tree

// if we have density as number of stems per sq meter, let us pretend we have an area of 10000 sq meter (1 ha)
// in that area we have S = D * 10000 stems.
// if the crown closure is C the area covered by S trees is A = 10000 * C
// The area covered by each tree is TA = A / S
// The radius of each tree is R = sqrt(TA / pi)
// The width of each tree is W = 2 * R
// The height of each tree is H = W * h/r

// example
// D = .02, C = .5, h/r = 2
// S = 200, A = 5000
// TA = 25
// R = 2.82m
// W = 5.64m
// H = 11.28m = 36ft

if (NormalizedDensity > 0.0)
	{
	MaxHeight = 2.0 * sqrt(MaxClosure / (NormalizedDensity * Pi)) * AvgImageHeightWidthRatio;

	Value[2] = Value[1] = Value[0] = 0.0;

	// is there a thematic map for min height?
	if (GroupMinHeightTheme && GroupMinHeightTheme->Eval(Value, ForDat->Poly->Vector))
		{
		MinHt = Value[0];
		} // if
	else
		MinHt = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue;

	if (ForDat->Eco->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
		{
		MinHeight = 2.0 * sqrt(MinHt / (NormalizedDensity * Pi)) * AvgImageHeightWidthRatio;
		if (MinHeight > MaxHeight)
			swmem(&MinHeight, &MaxHeight, sizeof (double));
		HeightRange = MaxHeight - MinHeight;
		} // if
	else if (ForDat->Eco->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
		{
		MinHeight = MaxHeight * MinHt * .01;
		HeightRange = MaxHeight - MinHeight;
		} // if
	else // (ForDat->Eco->SecondHeightType == WCS_ECOTYPE_SECONDHT_RANGEPCT)
		{
		HeightRange = MaxHeight * MinHt * .01;
		MinHeight = MaxHeight - HeightRange;
		MaxHeight += HeightRange;
		} // if

	AgvHeight = MinHeight + HeightRange * .5;
	} // if
else
	AgvHeight = MaxHeight = MinHeight = HeightRange = 0.0;

} // FoliageGroup::ComputeGroupSizeFromClosure

/*===========================================================================*/

void FoliageGroup::ComputeGroupDensityFromStems(ForestryRenderData *ForDat)
{

ComputeGroupStems(ForDat);
RawDensity = MaxStems;

} // FoliageGroup::ComputeGroupDensityFromStems

/*===========================================================================*/

void FoliageGroup::ComputeGroupDensityFromBasalArea(ForestryRenderData *ForDat)
{
double R;

ComputeGroupBasalArea(ForDat);
ComputeGroupDbh(ForDat);

// assuming that Dbh is in meters and basal area is in a user specified units
// typically dbh is in inches, basal area in square feet / area
// we have a unit specifier for area so basal area to the same units as diameter

// R = DBH * .5
// density/unit area = BA / R * R * Pi
if (MaxDbh > 0.0)
	{
	R = MaxDbh * .5;
	// BasalAreaUnitConversion converts area to square meters since R is in meters
	RawDensity = MaxBasalArea * ForDat->Eco->BasalAreaUnitConversion / (R * R * Pi);
	} // if
else
	RawDensity = 0.0;

} // FoliageGroup::ComputeGroupDensityFromBasalArea

/*===========================================================================*/

void FoliageGroup::ComputeGroupDensityFromClosure(ForestryRenderData *ForDat)
{
double TA;

if (! GroupSizeComputed)
	ComputeGroupSize(ForDat);
ComputeGroupClosure(ForDat);

if (MaxClosure > 0.0 && AvgImageHeightWidthRatio > 0.0 && AgvHeight > 0.0)
	{
	// D = .02, C = .5, h/r = 2
	// S = 200, A = 5000
	// TA = 25
	// R = 2.82m
	// W = 5.64m
	// H = 11.28m = 36ft

	// H = 2 * sqrt(C / (D * Pi)) * h/r
	// (H / (2 * (h/r)))**2 = C / (D * Pi)
	// (D * Pi) = C / (H / (2 * (h/r)))**2
	// D = C / (Pi * (H / (2 * (h/r)))**2)

	TA = AgvHeight / (2.0 * AvgImageHeightWidthRatio);
	RawDensity = MaxClosure / (Pi * TA * TA);
	} // if
else
	RawDensity = 0.0;

} // FoliageGroup::ComputeGroupDensityFromClosure

/*===========================================================================*/

void FoliageGroup::ComputeGroupHeight(ForestryRenderData *ForDat)
{
double Value[3];

Value[2] = Value[1] = Value[0] = 0.0;

// is there a thematic map for height?
if (GroupHeightTheme && GroupHeightTheme->Eval(Value, ForDat->Poly->Vector))
	{
	MaxHeight = Value[0];
	} // if
else
	MaxHeight = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].CurValue;

} // FoliageGroup::ComputeGroupHeight

/*===========================================================================*/

void FoliageGroup::ComputeGroupDbh(ForestryRenderData *ForDat)
{
double Value[3];

Value[2] = Value[1] = Value[0] = 0.0;

// is there a thematic map for Dbh?
if (GroupDbhTheme && GroupDbhTheme->Eval(Value, ForDat->Poly->Vector))
	{
	MaxDbh = Value[0];
	} // if
else
	MaxDbh = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DBH].CurValue;

} // FoliageGroup::ComputeGroupDbh

/*===========================================================================*/

void FoliageGroup::ComputeGroupAge(ForestryRenderData *ForDat)
{
double Value[3];

Value[2] = Value[1] = Value[0] = 0.0;

// is there a thematic map for age?
if (GroupAgeTheme && GroupAgeTheme->Eval(Value, ForDat->Poly->Vector))
	{
	MaxAge = Value[0];
	} // if
else
	MaxAge = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_AGE].CurValue;

} // FoliageGroup::ComputeGroupAge

/*===========================================================================*/

void FoliageGroup::ComputeGroupClosure(ForestryRenderData *ForDat)
{
double Value[3];

Value[2] = Value[1] = Value[0] = 0.0;

// is there a thematic map for closure?
if (GroupClosureTheme && GroupClosureTheme->Eval(Value, ForDat->Poly->Vector))
	{
	MaxClosure = Value[0];
	} // if
else
	MaxClosure = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_CROWNCLOSURE].CurValue;

} // FoliageGroup::ComputeGroupClosure

/*===========================================================================*/

void FoliageGroup::ComputeGroupStems(ForestryRenderData *ForDat)
{
double Value[3];

Value[2] = Value[1] = Value[0] = 0.0;

// is there a thematic map for stems?
if (GroupStemsTheme && GroupStemsTheme->Eval(Value, ForDat->Poly->Vector))
	{
	MaxStems = Value[0];
	} // if
else
	MaxStems = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].CurValue;

} // FoliageGroup::ComputeGroupStems

/*===========================================================================*/

void FoliageGroup::ComputeGroupBasalArea(ForestryRenderData *ForDat)
{
double Value[3];

Value[2] = Value[1] = Value[0] = 0.0;

// is there a thematic map for basal area?
if (GroupBasalAreaTheme && GroupBasalAreaTheme->Eval(Value, ForDat->Poly->Vector))
	{
	MaxBasalArea = Value[0];
	} // if
else
	MaxBasalArea = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_BASALAREA].CurValue;

} // FoliageGroup::ComputeGroupBasalArea

/*===========================================================================*/

#endif // WCS_FORESTRY_WIZARD
