// Texture.cpp
// Stuff for the new material and ecosystem ground textures.
// Built from scratch on 9/11/98 by Gary R. Huber.
// Copyright 1998 by Questar productions. All rights reserved.

#include "stdafx.h"
#include "MathSupport.h"
#include "TrigTable.h"
#include "Texture.h"
#include "Raster.h"
#include "Application.h"
#include "Useful.h"
#include "Random.h"
#include "Noise.h"
#include "CellularBasis.h"
#include "Conservatory.h"
#include "Interactive.h"
#include "Joe.h"
#include "Project.h"
#include "Requester.h"
#include "EffectsLib.h"
#include "DragnDropListGUI.h"
#include "Toolbar.h"
#include "Render.h"
#include "TextureEditGUI.h"
#include "ImageInputFormat.h"
#include "IncludeExcludeTypeList.h"
#include "AppMem.h"
#include "Lists.h"

PRNGX RootTexture::TextureRand;

// This doesn't seem to be referred to anywhere anymore
//#define WCS_CREATE_TEXTURERNGTABLE

// this is a static member of RootTexture
char *RootTexture::TextureNames[WCS_TEXTURE_NUMTYPES] = {"Planar Image", "Cylindrical Image", "Spherical Image",
	/*"Cubic Image",*/ "Front Projection", "Environment Map", "UV Mapped Image", "Color per Vertex", "Stripes", "Soft Stripes", "Single Stripe", 
	"Single Soft Stripe",
	"Wood", "Marble", "Brick", 
	"Dots",
	/*"Veins", "Crust", "Underwater",*/
	"Fractal Noise", "MultiFractal", "Hybrid MultiFractal", /*"Ridged MultiFractal", "Hetero Terrain",*/ "Turbulence",
	"F1 Cell Basis", "F2 Cell Basis", "F2mF1 Cell Basis", /*"F3mF1 Cell Basis",*/
	"F1 Manhattan", "F2 Manhattan", "F2mF1 Manhattan", "Birdshot",
	"Add", "Subtract", "Multiply", "Composite", "Contrast", "Darken", "Lighten", 
	"Levels", "HSV Merge",
	"Skew", "Bell Curve", "Square Wave",
	"Sawtooth", "Steps", "Linear", "Gamma", "Bias", "Gain", "Custom Curve",
	"Maximum", "Maximum Switch", "Minimum", "Minimum Switch", "Threshold", "Terrain Parameter", "Gradient",
	"Include/Exclude", "Object Type"
	};
// this is a static member of RootTexture
char *RootTexture::UserNames[WCS_TEXTURE_NUMTYPES] = {"Planar Image", "Cylindrical Image", "Spherical Image",
	/*"Cubic Image",*/ "Front Projection", "Environment Map", "UV Mapped Image", "Color per Vertex", "Stripes", "Soft Stripes", "Single Stripe", 
	"Single Soft Stripe",
	"Wood", "Marble", "Brick", 
	"Dots",
	/*"Veins", "Crust", "Underwater",*/
	"Fractal Noise", "MultiFractal", "Hybrid MultiFractal", /*"Ridged MultiFractal", "Hetero Terrain",*/ "Turbulence",
	"F1 Cell Basis", "F2 Cell Basis", "F2mF1 Cell Basis", /*"F3mF1 Cell Basis",*/
	"F1 Manhattan", "F2 Manhattan", "F2mF1 Manhattan", "Pebbles",
	"Add", "Subtract", "Multiply", "Composite", "Contrast", "Darken", "Lighten", 
	"Levels", "HSV Merge",
	"Skew", "Bell Curve", "Square Wave",
	"Sawtooth", "Steps", "Linear", "Gamma", "Bias", "Gain", "Custom Curve",
	"Maximum", "Maximum Switch", "Minimum", "Minimum Switch", "Threshold", "Dynamic Parameter", "Gradient",
	"Include/Exclude", "Object Type"
	};
// <<<>>> ADD_NEW_TEXTURE

// this is a static member of RootTexture
// true indicates it's used as a remap, false indicates it is not
bool RootTexture::TextureRemapType[WCS_TEXTURE_NUMTYPES] = 
	{
	// bitmaps
	false, // WCS_TEXTURE_TYPE_PLANARIMAGE,
	false, // WCS_TEXTURE_TYPE_CYLINDRICALIMAGE,
	false, // WCS_TEXTURE_TYPE_SPHERICALIMAGE,
//	WCS_TEXTURE_TYPE_CUBICIMAGE,
	false, // WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE,
	false, // WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE,
	// vertex maps
	false, // WCS_TEXTURE_TYPE_UVW,
	false, // WCS_TEXTURE_TYPE_COLORPERVERTEX,
	// patterns
	false, // WCS_TEXTURE_TYPE_STRIPES,
	false, // WCS_TEXTURE_TYPE_SOFTSTRIPES,
	false, // WCS_TEXTURE_TYPE_SINGLESTRIPE,
	false, // WCS_TEXTURE_TYPE_SINGLESOFTSTRIPE,
	false, // WCS_TEXTURE_TYPE_WOODGRAIN,
	false, // WCS_TEXTURE_TYPE_MARBLE,
	false, // WCS_TEXTURE_TYPE_BRICK,
	false, // WCS_TEXTURE_TYPE_DOTS,
//	WCS_TEXTURE_TYPE_VEINS,
//	WCS_TEXTURE_TYPE_CRUST,
//	WCS_TEXTURE_TYPE_UNDERWATER,
	// fractal
	false, // WCS_TEXTURE_TYPE_FRACTALNOISE,
	false, // WCS_TEXTURE_TYPE_MULTIFRACTALNOISE,
	false, // WCS_TEXTURE_TYPE_HYBRIDMULTIFRACTALNOISE,
//	WCS_TEXTURE_TYPE_RIDGEDMULTIFRACTALNOISE,
//	WCS_TEXTURE_TYPE_HETEROTERRAINNOISE,
	false, // WCS_TEXTURE_TYPE_TURBULENCE,
	// cell basis
	false, // WCS_TEXTURE_TYPE_F1CELLBASIS,
	false, // WCS_TEXTURE_TYPE_F2CELLBASIS,
	false, // WCS_TEXTURE_TYPE_F2MF1CELLBASIS,
//	WCS_TEXTURE_TYPE_F3MF1CELLBASIS,
	false, // WCS_TEXTURE_TYPE_F1MANHATTAN,
	false, // WCS_TEXTURE_TYPE_F2MANHATTAN,
	false, // WCS_TEXTURE_TYPE_F2MF1MANHATTAN,
	false, // WCS_TEXTURE_TYPE_BIRDSHOT,
	// mathematical
	false, // WCS_TEXTURE_TYPE_ADD,
	false, // WCS_TEXTURE_TYPE_SUBTRACT,
	false, // WCS_TEXTURE_TYPE_MULTIPLY,
	false, // WCS_TEXTURE_TYPE_COMPOSITE,
	true, // WCS_TEXTURE_TYPE_CONTRAST,
	true, // WCS_TEXTURE_TYPE_DARKEN,
	true, // WCS_TEXTURE_TYPE_LIGHTEN,
	true, // WCS_TEXTURE_TYPE_LEVELS,
	false, // WCS_TEXTURE_TYPE_HSVMERGE,
	true, // WCS_TEXTURE_TYPE_SKEW,
	true, // WCS_TEXTURE_TYPE_BELLCURVE,
	true, // WCS_TEXTURE_TYPE_SQUAREWAVE,
	true, // WCS_TEXTURE_TYPE_SAWTOOTH,
	true, // WCS_TEXTURE_TYPE_STEP,
	true, // WCS_TEXTURE_TYPE_SLOPE,
	true, // WCS_TEXTURE_TYPE_GAMMA,
	true, // WCS_TEXTURE_TYPE_BIAS,
	true, // WCS_TEXTURE_TYPE_GAIN,
	true, // WCS_TEXTURE_TYPE_CUSTOMCURVE,
	false, // WCS_TEXTURE_TYPE_MAXIMUM,
	false, // WCS_TEXTURE_TYPE_MAXIMUMSWITCH,
	false, // WCS_TEXTURE_TYPE_MINIMUM,
	false, // WCS_TEXTURE_TYPE_MINIMUMSWITCH,
	false, // WCS_TEXTURE_TYPE_THRESHOLD,
	// parameters
	false, // WCS_TEXTURE_TYPE_TERRAINPARAM,
	// no inputs
	false, // WCS_TEXTURE_TYPE_GRADIENT,
	false, // WCS_TEXTURE_TYPE_INCLUDEEXCLUDE,
	false, // WCS_TEXTURE_TYPE_INCLUDEEXCLUDETYPE,
// <<<>>> ADD_NEW_TEXTURE
	// WCS_TEXTURE_NUMTYPES
	}; // texture types


char *RootTexture::CoordSpaceNames[WCS_TEXTURE_MAX_COORDSPACES] = {"Object Cartesian", "World Cartesian", 
	"Project Referenced Cartesian", "Vector Aligned", "Georeferenced Image", "Render Image Unity Scale", 
	"Render Image Unity, Ignore Z", "Render Image Pixels", "Render Image Pixels, Ignore Z", "Object Vertex UVW Map", 
	"Object Vertex Colors", "None"};

float *RootTexture::CubePreviewData;
float *RootTexture::SpherePreviewData;
double TextureAA::TextureAARNGTable[WCS_TEXTUREAA_MAX_SAMPLES * 3];
char TextureAA::TableInitialized;

//char TextureTypeString[64];

// this is a static member of RootTexture
char RootTexture::IdentifyTextureType(char *TypeStr)
{
char Ct;

for (Ct = 0; Ct < WCS_TEXTURE_NUMTYPES; Ct ++)
	{
	if (! strcmp(TypeStr, TextureNames[Ct]))
		return (Ct);
	} // for

return (-1);

} // RootTexture::IdentifyTextureType

/*===========================================================================*/
/*===========================================================================*/

TextureData::TextureData()
{

SampleType = WCS_SAMPLETEXTURE_LIVEDATA;
MetersPerDegLat = MetersPerDegLon = EARTHLATSCALE_METERS;
TexRefLat = TexRefLon = TexRefElev = 0.0;
TLowX = THighX = TLowY = THighY = TLowZ = THighZ = 0.0;
Elev = Slope = Aspect = Latitude = Longitude = Illumination = ZDist = QDist = WaterDepth = WaveHeight = 
	Reflectivity = Normal[2] = Normal[1] = Normal[0] = CamAimVec[2] = CamAimVec[1] = CamAimVec[0] = 0.0;
RelElev = VectorSlope = 0.0f;
TLatRange[3] = TLatRange[2] = TLatRange[1] = TLatRange[0] = 0.0;
TLonRange[3] = TLonRange[2] = TLonRange[1] = TLonRange[0] = 0.0;
TElevRange[3] = TElevRange[2] = TElevRange[1] = TElevRange[0] = 0.0;
PixelX[1] = PixelX[0] = PixelY[1] = PixelY[0] = 0.0;
VecOffset[2] = VecOffset[1] = VecOffset[0] = 0.0;
VecOffsetPixel[5] = VecOffsetPixel[4] = VecOffsetPixel[3] = 
	VecOffsetPixel[2] = VecOffsetPixel[1] = VecOffsetPixel[0] = 0.0;
VectorEffectType = VectorOffsetsComputed = VDataVecOffsetsValid = 0;
InputSuppliedInOutput = 0;
ExagerateElevLines = 0;
IsSampleThumbnail = 0;
ThumbnailX = ThumbnailY = 0.0;
Datum = 0.0;
Exageration = 1.0;
Object = NULL;
RGB[0] = RGB[1] = RGB[2] = 0.0;
Vec = NULL;
ObjectType = 0;
VDEM[0] = VDEM[1] = VDEM[2] = NULL;
VData[0] = VData[1] = VData[2] = NULL;
PData = NULL;
VtxPixCornerWt[0][0] = VtxPixCornerWt[0][1] = VtxPixCornerWt[0][2] = 0.0;
VtxPixCornerWt[1][0] = VtxPixCornerWt[1][1] = VtxPixCornerWt[1][2] = 0.0;
VtxPixCornerWt[2][0] = VtxPixCornerWt[2][1] = VtxPixCornerWt[2][2] = 0.0;
VtxPixCornerWt[3][0] = VtxPixCornerWt[3][1] = VtxPixCornerWt[3][2] = 0.0;
VtxWt[2] = VtxWt[1] = VtxWt[0] = 0.0;
VtxNum[2] = VtxNum[1] = VtxNum[0] = 0;
VertRefData[2] = VertRefData[1] = VertRefData[0] = 0;
Object3D = NULL;
PixelWidth = ObjectPixelWidth = 0.0;

} // TextureData::TextureData

/*===========================================================================*/
/*===========================================================================*/

TextureExtractionData::TextureExtractionData(TextureData *Data)
{

LowX = Data->TLowX;
HighX = Data->THighX;
LowY = Data->TLowY;
HighY = Data->THighY;
LowZ = Data->TLowZ;
HighZ = Data->THighZ;
LatRange[0] = Data->TLatRange[0];
LatRange[1] = Data->TLatRange[1];
LatRange[2] = Data->TLatRange[2];
LatRange[3] = Data->TLatRange[3];
LonRange[0] = Data->TLonRange[0];
LonRange[1] = Data->TLonRange[1];
LonRange[2] = Data->TLonRange[2];
LonRange[3] = Data->TLonRange[3];
ElevRange[0] = Data->TElevRange[0];
ElevRange[1] = Data->TElevRange[1];
ElevRange[2] = Data->TElevRange[2];
ElevRange[3] = Data->TElevRange[3];

} // TextureExtractionData::TextureExtractionData

/*===========================================================================*/
/*===========================================================================*/

void TextureAA::CreateTextureAASamplePRNTable(void)
{
double Temp[3], DistX, DistY, DistZ, Dist2D, Dist3D;
long Ct, TryAgain;
//FILE *ffile;
//char filename[256];

RootTexture::TextureRand.Seed64(123456, 654321);

for (Ct = 0; Ct < WCS_TEXTUREAA_MAX_SAMPLES * 3; Ct += 3)
	{
	TryAgain = 1;
	while (TryAgain)
		{
		TryAgain = 0;
		RootTexture::TextureRand.GenMultiPRN(3, Temp);
		if (Ct > 2)
			{
			DistX = Temp[0] - TextureAARNGTable[Ct - 3];
			DistY = Temp[1] - TextureAARNGTable[Ct - 2];
			DistZ = Temp[2] - TextureAARNGTable[Ct - 1];
			Dist2D = DistX * DistX + DistY * DistY;
			Dist3D = Dist2D + DistZ * DistZ;
			if (Dist2D > .5 && Dist3D > .75)
				{
				if (Ct > 5)
					{
					DistX = Temp[0] - TextureAARNGTable[Ct - 6];
					DistY = Temp[1] - TextureAARNGTable[Ct - 5];
					DistZ = Temp[2] - TextureAARNGTable[Ct - 4];
					Dist2D = DistX * DistX + DistY * DistY;
					Dist3D = Dist2D + DistZ * DistZ;
					if (Dist2D > .08 && Dist3D > .12)
						{
						if (Ct > 8)
							{
							DistX = Temp[0] - TextureAARNGTable[Ct - 9];
							DistY = Temp[1] - TextureAARNGTable[Ct - 8];
							DistZ = Temp[2] - TextureAARNGTable[Ct - 7];
							Dist2D = DistX * DistX + DistY * DistY;
							Dist3D = Dist2D + DistZ * DistZ;
							if (Dist2D <= .0128 || Dist3D <= .0192)
								TryAgain = 1;
							} // if
						} // if
					else
						TryAgain = 1;
					} // if
				} // i
			else
				TryAgain = 1;
			} // if
		} // while
	TextureAARNGTable[Ct] = Temp[0];
	TextureAARNGTable[Ct + 1] = Temp[1];
	TextureAARNGTable[Ct + 2] = Temp[2];
	} // for

TableInitialized = 1;

/*
// write out a file of Random Number triples
strmfp(filename, "WCSProjects:", "TextureAATable.txt");

if (ffile = PROJ_fopen(filename, "w"))
	{
	for (Ct = 0; Ct < WCS_TEXTUREAA_MAX_SAMPLES * 3; Ct += 3)
		{
		fprintf(ffile, "%f, %f, %f,\n", TextureAARNGTable[Ct], TextureAARNGTable[Ct + 1], TextureAARNGTable[Ct + 2]); 
		} // for
	fclose(ffile);
	} // if
*/

} // TextureAA::CreateTextureAASamplePRNTable

/*===========================================================================*/

TextureAA::TextureAA()
{

memset(BinCount, 0, sizeof BinCount);
memset(BinSum, 0, sizeof BinSum);

} // TextureAA::TextureAA

/*===========================================================================*/

void TextureAA::Init(Texture *AAMe)
{
//long Ct, BinNum;
//double LowX, LowY, LowZ, Output[3], BinInterval, BinCenter;

if (! TableInitialized)
	CreateTextureAASamplePRNTable();

/* not currently needed - leave in for future reference
for (Ct = 0; Ct < WCS_TEXTUREAA_PRESAMPLES; Ct ++)
	{
	LowX = RootTexture::TextureRand.GenPRN();
	LowY = RootTexture::TextureRand.GenPRN();
	if (! AAMe->ThreeD && WCS_TEXTURETYPE_IS3DOPTIONAL(AAMe->TexType))
		{
		LowZ = 0.0;
		} // if
	else
		{
		LowZ = RootTexture::TextureRand.GenPRN();
		} // else
	AAMe->Eval(Output, LowX, LowX, LowY, LowY, LowZ, LowZ, 0, 0, NULL);
	if (AAMe->ApplyToColor)
		{
		if ((BinNum = (int)(Output[0] * WCS_TEXTUREAA_SAMPLEBINS)) >= WCS_TEXTUREAA_SAMPLEBINS)
			BinNum = WCS_TEXTUREAA_SAMPLEBINS - 1;
		BinCount[BinNum][0] ++;
		if ((BinNum = (int)(Output[1] * WCS_TEXTUREAA_SAMPLEBINS)) >= WCS_TEXTUREAA_SAMPLEBINS)
			BinNum = WCS_TEXTUREAA_SAMPLEBINS - 1;
		BinCount[BinNum][1] ++;
		if ((BinNum = (int)(Output[2] * WCS_TEXTUREAA_SAMPLEBINS)) >= WCS_TEXTUREAA_SAMPLEBINS)
			BinNum = WCS_TEXTUREAA_SAMPLEBINS - 1;
		BinCount[BinNum][2] ++;
		} // if
	else
		{
		if ((BinNum = (int)(Output[0] * WCS_TEXTUREAA_SAMPLEBINS)) >= WCS_TEXTUREAA_SAMPLEBINS)
			BinNum = WCS_TEXTUREAA_SAMPLEBINS - 1;
		BinCount[BinNum][0] ++;
		} // else
	} // for

BinInterval = 1.0 / WCS_TEXTUREAA_SAMPLEBINS;
BinCenter = BinInterval / 2.0;

if (AAMe->ApplyToColor)
	{
	BinSum[0][0] = (float)(BinCount[0][0] * BinCenter);
	BinSum[0][1] = (float)(BinCount[0][1] * BinCenter);
	BinSum[0][2] = (float)(BinCount[0][2] * BinCenter);
	for (Ct = 1, BinCenter += BinInterval; Ct < WCS_TEXTUREAA_SAMPLEBINS; Ct ++, BinCenter += BinInterval)
		{
		BinSum[Ct][0] = (float)(BinSum[Ct - 1][0] + BinCount[Ct][0] * BinCenter);
		BinCount[Ct][0] += BinCount[Ct - 1][0];
		BinSum[Ct][1] = (float)(BinSum[Ct - 1][1] + BinCount[Ct][1] * BinCenter);
		BinCount[Ct][1] += BinCount[Ct - 1][1];
		BinSum[Ct][2] = (float)(BinSum[Ct - 1][2] + BinCount[Ct][2] * BinCenter);
		BinCount[Ct][2] += BinCount[Ct - 1][2];
		} // for
	} // if
else
	{
	BinSum[0][0] = (float)(BinCount[0][0] * BinCenter);
	for (Ct = 1, BinCenter += BinInterval; Ct < WCS_TEXTUREAA_SAMPLEBINS; Ct ++, BinCenter += BinInterval)
		{
		BinSum[Ct][0] = (float)(BinSum[Ct - 1][0] + BinCount[Ct][0] * BinCenter);
		BinCount[Ct][0] += BinCount[Ct - 1][0];
		} // for
	} // else
*/

} // TextureAA::TextureAA

/*===========================================================================*/
/* ccurrently not used
inline void TextureAA::ProcessSamplePoints(double *Output, double MinR, double MaxR, double MinG, double MaxG, double MinB, double MaxB)
{
double SampleRange[3];
int CountRange[3], TestBin[2];

TestBin[0] = (int)(MinR * (WCS_TEXTUREAA_SAMPLEBINS)) - 1;
if ((TestBin[1] = (int)(MaxR * WCS_TEXTUREAA_SAMPLEBINS)) >= WCS_TEXTUREAA_SAMPLEBINS)
	TestBin[1] = WCS_TEXTUREAA_SAMPLEBINS - 1;
CountRange[0] = BinCount[TestBin[1]][0] - (TestBin[0] >= 0 ? BinCount[TestBin[0]][0]: 0);
SampleRange[0] = BinSum[TestBin[1]][0] - (TestBin[0] >= 0 ? BinSum[TestBin[0]][0]: 0.0);
TestBin[0] = (int)(MinG * (WCS_TEXTUREAA_SAMPLEBINS)) - 1;
if ((TestBin[1] = (int)(MaxG * WCS_TEXTUREAA_SAMPLEBINS)) >= WCS_TEXTUREAA_SAMPLEBINS)
	TestBin[1] = WCS_TEXTUREAA_SAMPLEBINS - 1;
CountRange[1] = BinCount[TestBin[1]][1] - (TestBin[0] >= 0 ? BinCount[TestBin[0]][1]: 0);
SampleRange[1] = BinSum[TestBin[1]][1] - (TestBin[0] >= 0 ? BinSum[TestBin[0]][1]: 0.0);
TestBin[0] = (int)(MinB * (WCS_TEXTUREAA_SAMPLEBINS)) - 1;
if ((TestBin[1] = (int)(MaxB * WCS_TEXTUREAA_SAMPLEBINS)) >= WCS_TEXTUREAA_SAMPLEBINS)
	TestBin[1] = WCS_TEXTUREAA_SAMPLEBINS - 1;
CountRange[2] = BinCount[TestBin[1]][2] - (TestBin[0] >= 0 ? BinCount[TestBin[0]][2]: 0);
SampleRange[2] = BinSum[TestBin[1]][2] - (TestBin[0] >= 0 ? BinSum[TestBin[0]][2]: 0.0);

if (CountRange[0])
	Output[0] = SampleRange[0] / CountRange[0];
else
	Output[0] = MinR;
if (CountRange[1])
	Output[1] = SampleRange[1] / CountRange[1];
else
	Output[1] = MinG;
if (CountRange[2])
	Output[2] = SampleRange[2] / CountRange[2];
else
	Output[2] = MinB;

} // TextureAA::ProcessSamplePoints
*/
/*===========================================================================*/

void RootTextureParent::Copy(RootTextureParent *CopyTo, RootTextureParent *CopyFrom)
{
long Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (CopyTo->GetTexRootPtr(Ct))
		{
		delete CopyTo->GetTexRootPtr(Ct);		// removes links to images
		CopyTo->SetTexRootPtr(Ct, NULL);
		} // if
	if (CopyFrom->GetTexRootPtr(Ct))
		{
		CopyTo->NewRootTexture(Ct);
		//CopyTo->SetTexRootPtr(Ct, new RootTexture(CopyTo, CopyFrom->GetTexRootPtr(Ct)->ApplyToEcosys, CopyFrom->GetTexRootPtr(Ct)->ApplyToColor, CopyFrom->GetTexRootPtr(Ct)->ApplyToDisplace));
		if (CopyTo->GetTexRootPtr(Ct))
			{
			CopyTo->GetTexRootPtr(Ct)->Copy(CopyTo->GetTexRootPtr(Ct), CopyFrom->GetTexRootPtr(Ct));
			} // if
		} // if
	} // for

} // RootTextureParent::Copy

/*===========================================================================*/

long RootTextureParent::GetTexNumberFromAddr(RootTexture **Addr)
{
long Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtrAddr(Ct) == Addr)
		{
		return (Ct);
		} // if
	} // for

return (-1);

} // RootTextureParent::GetTexNumberFromAddr

/*===========================================================================*/

void RootTextureParent::DeleteAllTextures(void)
{
long Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		delete GetTexRootPtr(Ct);
		SetTexRootPtr(Ct, NULL);
		} // if
	} // for

} // RootTextureParent::DeleteAllTextures

/*===========================================================================*/

int RootTextureParent::AreThereEnabledTextures(void)
{
long Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetEnabled())
		{
		return (1);
		} // if
	} // for

return (0);

} // RootTextureParent::AreThereEnabledTextures

/*===========================================================================*/
/*===========================================================================*/
// Notes:
// Textures can be applied to either 3D Object Material Effects or to Ecosystem ground polygons
// Textures can be applied to the color channel of a material or to any of the other material properties
//  including specularity, specular exponent, luminosity, transparency, translucency,
//  translucency exponent or bump.
// For an ecosystem, textures can be applied to the color, bump, reflectivity (specularity),
//  smoothness (specular exponent), transparency (optical depth in the case of water), luminosity 
//  or foliage (foliage will be applied according to the texture values).
// Depending on whether the texture is appied to an ecosystem or to a material, certain values
//  in the Texture object may not be available to the user. For instance the LatLonBounds only makes
//  sense for textures applied to the terrain (as ecosystems).
// In addition certain values are only applicable to those textures applied as colors to the material
//  or ecosystem. The obvious case is the Palette numbers of colors.
// Still other special availability cases are determined by whether the user selects a raster image
//  or a procedural for the type of a texture.
// The texture root stores a pointer to a linked list of textures. The list can be as long as desired
//  and will be evaluated from the head of the list downward until either the end of the list or until
//  the opacity exceeds 100%.
// Like LightWave, the opacity, reduced by any alpha channel or self-alpha, is applied to the remainder
//  of the unused opacity when a texture is evaluated in the chain. We'll see how this works and change
//  it later if necessary to achieve certain effects.
// The Opacity channel, if specified, is just the head of another texture chain. The only difference is
//  that it obviously is not a color application and so color palettes are not available.

RootTexture::RootTexture()
: RasterAnimHost(NULL)
{

SetDefaults();

} // RootTexture::RootTexture

/*===========================================================================*/

RootTexture::RootTexture(RasterAnimHost *RAHost)
: RasterAnimHost(RAHost)
{

SetDefaults();

} // RootTexture::RootTexture

/*===========================================================================*/

RootTexture::RootTexture(RasterAnimHost *RAHost, unsigned char EcosysSource, unsigned char ColorSource, unsigned char DisplaceSource)
: RasterAnimHost(RAHost)
{

SetDefaults();
ApplyToEcosys = EcosysSource;
ApplyToColor = ColorSource;
ApplyToDisplace = DisplaceSource;
PreviewDirection = ApplyToEcosys ? 2: 5;

} // RootTexture::RootTexture

/*===========================================================================*/

void RootTexture::SetDefaults(void)
{
double SizeRangeDefaults[3] = {1000000.0, 0.01, .5};

PreviewSize.SetDefaults(this, 0, 2.8);
PreviewSize.SetRangeDefaults(SizeRangeDefaults);
PreviewSize.SetNoNodes(1);

Tex = NULL;
Name[0] = 0;
ApplyToEcosys = 0;
ApplyToColor = 0;
ApplyToDisplace = 0;
ShowSize = WCS_TEXTURE_SHOW_SIZE;
ShowRootNail = 0;
ShowCompChildNail = 1;
ShowComponentNail = 0;
PreviewDirection = 5;
Enabled = 1;
PreviewSize.SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);

} // RootTexture::SetDefaults

/*===========================================================================*/

RootTexture::~RootTexture()
{
Texture *TempTex;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->TXG && GlobalApp->GUIWins->TXG->GetRootTexture() == this)
		{
		delete GlobalApp->GUIWins->TXG;
		GlobalApp->GUIWins->TXG = NULL;
		} // if
	} // if

while (Tex)
	{
	TempTex = Tex;
	Tex = Tex->Next;
	delete TempTex;
	} // while

} // RootTexture::~RootTexture

/*===========================================================================*/

void RootTexture::DestroyPreviewSubjects(void)
{

if (CubePreviewData)
	AppMem_Free(CubePreviewData, WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE * sizeof (float) * 4);
CubePreviewData = NULL;
if (SpherePreviewData)
	AppMem_Free(SpherePreviewData, WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE * sizeof (float) * 4);
SpherePreviewData = NULL;

} // RootTexture::DestroyPreviewSubjects

/*===========================================================================*/

void RootTexture::Copy(RootTexture *CopyTo, RootTexture *CopyFrom)
{
Texture *CurrentFrom = CopyFrom->Tex, **ToPtr, *NextTex;
RasterAnimHost *ToRoot;
RasterAnimHostProperties Prop;

PreviewSize.Copy(&CopyTo->PreviewSize, &CopyFrom->PreviewSize);
ToRoot = CopyTo->GetRAHostRoot();

if (ToRoot && CopyTo->RAParent && (CopyFrom->RAParent || ToRoot != this))
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
	ToRoot->GetRAHostProperties(&Prop);
	CopyTo->ApplyToEcosys = (Prop.TypeNumber != WCS_EFFECTSSUBCLASS_MATERIAL && Prop.TypeNumber != WCS_EFFECTSSUBCLASS_POSTPROC);
	CopyTo->RAParent->GetTextureApplication(CopyTo, CopyTo->ApplyToColor, CopyTo->ApplyToDisplace);
	} // if
else
	{
	CopyTo->ApplyToEcosys = CopyFrom->ApplyToEcosys;
	CopyTo->ApplyToColor = CopyFrom->ApplyToColor;
	CopyTo->ApplyToDisplace = CopyFrom->ApplyToDisplace;
	} // else

// delete existing textures
while (CopyTo->Tex)
	{
	NextTex = CopyTo->Tex;
	CopyTo->Tex = CopyTo->Tex->Next;
	delete NextTex;
	} // while

ToPtr = &CopyTo->Tex;

while (CurrentFrom)
	{
	if (CurrentFrom->GetApplicationLegal(CopyTo->ApplyToEcosys))
		{
		if (*ToPtr = NewTexture(CopyTo, CurrentFrom, CurrentFrom->TexType, WCS_TEXTURE_ROOT))
			{
			(*ToPtr)->Copy(*ToPtr, CurrentFrom);
			} // if
		ToPtr = &(*ToPtr)->Next;
		} // if
	else
		UserMessageOK("Copy Texture", "Not all texture components can be applied\n to the destination object type.\nSome components will not be copied.");
	CurrentFrom = CurrentFrom->Next;
	} // while

strcpy(CopyTo->Name, CopyFrom->Name);
CopyTo->ShowSize = CopyFrom->ShowSize;
CopyTo->ShowRootNail = CopyFrom->ShowRootNail;
CopyTo->ShowCompChildNail = CopyFrom->ShowCompChildNail;
CopyTo->ShowComponentNail = CopyFrom->ShowComponentNail;
CopyTo->PreviewDirection = CopyFrom->PreviewDirection;
CopyTo->Enabled = CopyFrom->Enabled;
RasterAnimHost::Copy(CopyTo, CopyFrom);

} // RootTexture::Copy

/*===========================================================================*/

unsigned char RootTexture::GetCoordSpaceNumberFromName(char *CoordSpaceName)
{
unsigned char SpaceNumber;

for (SpaceNumber = 0; SpaceNumber < WCS_TEXTURE_MAX_COORDSPACES; SpaceNumber ++)
	{
	if (! strcmp(CoordSpaceName, CoordSpaceNames[SpaceNumber]))
		return (SpaceNumber);
	} // for

return ((unsigned char)(-1));

} // RootTexture::GetCoordSpaceNumberFromName

/*===========================================================================*/

void RootTexture::OffsetTexCenter(int Channel, double Offset)
{
Texture *Current = Tex;

while (Current)
	{
	Current->OffsetTexCenter(Channel, Offset);
	Current = Current->Next;
	} // while

} // RootTexture::OffsetTexCenter

/*===========================================================================*/

// does the texture need to be rendered for exporting 3d objects
int RootTexture::IsNeedRendered(Object3DEffect *Object3D, int MultipleUVMappingsOK, unsigned char &PrevMapUsed)
{
unsigned char CurUVMap = 0;
Texture *CurTex;
int NeedRendered = 0, UVFound = 0, SameMap = 1;

// criteria are: 1 enabled UV texture only, no other non-UV enabled textures, one UV map in object
if (CurTex = Tex)
	{
	while (CurTex)
		{
		if (CurTex->Enabled)
			{
			if (CurTex->TexType == WCS_TEXTURE_TYPE_UVW)
				{
				if (Object3D->GetVertexMapNumber(WCS_VERTREFDATA_MAPTYPE_UVW, CurTex->MapName, CurUVMap))
					CurUVMap ++;
				if (PrevMapUsed)
					SameMap = (PrevMapUsed == CurUVMap);
				else
					PrevMapUsed = CurUVMap;
				if (! SameMap && ! MultipleUVMappingsOK)
					{
					NeedRendered = 1;
					break;
					} // if
				} // if
			else
				{
				NeedRendered = 1;
				break;
				} // if
			} // if
		CurTex = CurTex->Next;
		} // if
	} // if

return (NeedRendered);

} // RootTexture::IsNeedRendered

/*===========================================================================*/

int RootTexture::IsTextureTileable(double &TileWidth, double &TileHeight, double &TileCenterX, double &TileCenterY)
{
double CurTileWidth, CurTileHeight, CurTileCenterX, CurTileCenterY;
Texture *Current = Tex;
int OneTileDone = 0, Tileable = 1, SizeDoesMatter;

TileWidth = TileHeight = 1.0;
TileCenterX = TileCenterY = 0.0;

while (Current && Tileable)
	{
	if (Tileable = Current->IsTextureTileable(CurTileWidth, CurTileHeight, CurTileCenterX, CurTileCenterY, SizeDoesMatter))
		{
		if (SizeDoesMatter)
			{
			if (OneTileDone)
				{
				if (fabs(CurTileCenterX - TileCenterX) <= .01 && fabs(CurTileCenterY - TileCenterY) <= .01)
					{
					// find common size
					if (fabs(CurTileWidth - TileWidth) > .01 || fabs(CurTileHeight - TileHeight) > .01)
						{
						TileWidth = WCS_lcm(TileWidth, CurTileWidth, .05);
						TileHeight = WCS_lcm(TileHeight, CurTileHeight, .05);
						} // if
					} // if
				else
					Tileable = 0;
				} // if
			else
				{
				TileCenterX = CurTileCenterX;
				TileCenterY = CurTileCenterY;
				TileWidth = CurTileWidth;
				TileHeight = CurTileHeight;
				OneTileDone = 1;
				} // else
			} // if
		} // if
	Current = Current->Next;
	} // while

return (Tileable);

} // RootTexture::IsTextureTileable

/*===========================================================================*/

Texture *RootTexture::MatchRasterShell(RasterShell *Shell)
{
Texture *Current = Tex, *Test;

while (Current)
	{
	if (Test = Current->MatchRasterShell(Shell))
		return (Test);
	Current = Current->Next;
	} // while

return (NULL);

} // RootTexture::MatchRasterShell

/*===========================================================================*/

int RootTexture::SetToTime(double Time)
{
long Found = 0;
Texture *Current = Tex;

while (Current)
	{
	if (Current->SetToTime(Time))
		Found = 1;
	Current->BuildTextureRotationMatrix(TRUE);
	Current = Current->Next;
	} // while

return (Found);

} // RootTexture::SetToTime

/*===========================================================================*/

int RootTexture::GetRAHostAnimated(void)
{
Texture *Current = Tex;

while (Current)
	{
	if (Current->GetRAHostAnimated())
		return (1);
	Current = Current->Next;
	} // while

return (0);

} // RootTexture::GetRAHostAnimated

/*===========================================================================*/

bool RootTexture::GetRAHostAnimatedInclVelocity(void)
{
Texture *Current = Tex;

while (Current)
	{
	if (Current->GetRAHostAnimatedInclVelocity())
		return (true);
	Current = Current->Next;
	} // while

return (false);

} // RootTexture::GetRAHostAnimatedInclVelocity

/*===========================================================================*/

int RootTexture::IsAnimated(void)
{
Texture *Current = Tex;

while (Current)
	{
	if (Current->Enabled && Current->IsAnimated())
		return (1);
	Current = Current->Next;
	} // while

return (0);

} // RootTexture::IsAnimated

/*===========================================================================*/

char RootTexture::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_TEXTURE)
	return (1);

return (0);

} // RootTexture::GetRAHostDropOK

/*===========================================================================*/

int RootTexture::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char WinNum, QueryStr[256], NameStr[128];
int QueryResult, Success = 0;
long Ct, NumListItems = 0;
RasterAnimHostProperties Prop;
RasterAnimHost *TargetList[30];
Texture *CurTex;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	if (this != (RootTexture *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (RootTexture *)DropSource->DropSource);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
	{
	if (! Tex)
		{
		if (CurTex = AddNewTexture((Texture *)DropSource->DropSource, ((Texture *)DropSource->DropSource)->GetTexType()))
			{
			CurTex->Copy(CurTex, (Texture *)DropSource->DropSource);
			Changes[0] = MAKE_ID(CurTex->GetNotifyClass(), CurTex->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, CurTex->GetRAHostRoot());
			Success = 1;
			} // if
		} // if
	else
		{
		strcpy(QueryStr, "Add a new Texture or replace an existing one of the same type?");
		if (QueryResult = UserMessageCustom(NameStr, QueryStr, "Add New", "Cancel", "Replace", 0))
			{
			if (QueryResult == 1)
				{
				if (CurTex = AddNewTexture((Texture *)DropSource->DropSource, ((Texture *)DropSource->DropSource)->GetTexType()))
					{
					CurTex->Copy(CurTex, (Texture *)DropSource->DropSource);
					Changes[0] = MAKE_ID(CurTex->GetNotifyClass(), CurTex->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, CurTex->GetRAHostRoot());
					Success = 1;
					} // if
				} // if
			else if (QueryResult == 2)
				{
				for (Ct = 0, CurTex = Tex; CurTex; CurTex = CurTex->Next)
					{
					if (CurTex->GetTexType() == ((Texture *)DropSource->DropSource)->GetTexType())
						{
						TargetList[Ct] = CurTex;
						Ct ++;
						} // if
					} // for
				NumListItems = Ct;
				if (! NumListItems)
					UserMessageOK(NameStr, "No textures of the same type were found.");
				} // else if
			} // if
		} // else
	} // else if

if (NumListItems > 0)
	{
	if ((WinNum = GlobalApp->MCP->GetAvailableDragnDropListNumber()) >= 0)
		{
		GlobalApp->GUIWins->DDL[WinNum] = new DragnDropListGUI(WinNum, DropSource, TargetList, NumListItems, NULL, NULL);
		if(GlobalApp->GUIWins->DDL[WinNum])
			{
			Success = -1;
			GlobalApp->GUIWins->DDL[WinNum]->Open(GlobalApp->MainProj);
			}
		} // if
	else
		UserMessageOK("Open Drag 'n Drop List", "All List windows are in use.\nTry dropping on a target deeper in the hierarchy\n and closer to your desired target.");
	} // if

return (Success);

} // RootTexture::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long RootTexture::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_ROOTTEXTURE | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // RootTexture::GetRAFlags

/*===========================================================================*/

void RootTexture::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = RAParent ? RAParent->GetCritterName(this): (char*)"";
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
	Prop->Path = "Texture";
	Prop->Ext = "rtx";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_POPCLASS)
	{
	GetPopClassFlags(Prop);
	} // get pop-up menu classes

} // RootTexture::GetRAHostProperties

/*===========================================================================*/

int RootTexture::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_ENABLED)
		{
		Enabled = (Prop->Flags & WCS_RAHOST_FLAGBIT_ENABLED) ? 1: 0;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
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
	return(SaveObject(Prop->fFile));
	} // if

return (Success);

} // RootTexture::SetRAHostProperties

/*===========================================================================*/

int RootTexture::BuildFileComponentsList(EffectList **Coords)
{
Texture *Current = Tex;

while (Current)
	{
	if (! Current->BuildFileComponentsList(Coords))
		return (0);
	Current = Current->Next;
	} // while

return (1);

} // RootTexture::BuildFileComponentsList

/*===========================================================================*/

int RootTexture::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil)
{

AnimAffil = NULL;

if (ChildA)
	{
	if (ChildA == &PreviewSize)
		{
		AnimAffil = (AnimCritter *)ChildA;
		return (1);
		} // if
	} // if


return (0);

} // RootTexture::GetAffiliates

/*===========================================================================*/

int RootTexture::GetPopClassFlags(RasterAnimHostProperties *Prop)
{
AnimCritter *AnimAffil = NULL;

Prop->PopClassFlags = 0;
Prop->PopExistsFlags = 0;
Prop->PopEnabledFlags = 0;

if (GetAffiliates(Prop->ChildA, Prop->ChildB, AnimAffil))
	{
	return (RasterAnimHost::GetPopClassFlags(Prop, AnimAffil, NULL, NULL));
	} // if

return (0);

} // RootTexture::GetPopClassFlags

/*===========================================================================*/

int RootTexture::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil))
	{
	return (RasterAnimHost::AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, NULL, NULL));
	} // if

return(0);

} // RootTexture::AddSRAHBasePopMenus

/*===========================================================================*/

int RootTexture::HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil))
	{
	return (RasterAnimHost::HandleSRAHPopMenuSelection(Action, AnimAffil, NULL, NULL, NULL, NULL));
	} // if

return(0);

} // RootTexture::HandleSRAHPopMenuSelection

/*===========================================================================*/

RasterAnimHost *RootTexture::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Found = 0;
Texture *CurTex = Tex;

if (! Current)
	Found = 1;

while (CurTex)
	{
	if (Found)
		return (CurTex);
	if (Current == CurTex)
		Found = 1;
	CurTex = CurTex->Next;
	} // while

return (NULL);

} // RootTexture::GetRAHostChild

/*===========================================================================*/

int RootTexture::GetDeletable(RasterAnimHost *Test)
{
Texture *CurTex = Tex;

while (CurTex)
	{
	if (Test == CurTex)
		return (1);
	CurTex = CurTex->Next;
	} // while

return (0);

} // RootTexture::GetDeletable

/*===========================================================================*/

long RootTexture::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Found = 0;
Texture *CurTex = Tex;

while (CurTex)
	{
	if (CurTex->GetKeyFrameRange(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	CurTex = CurTex->Next;
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

} // RootTexture::GetKeyFrameRange

/*===========================================================================*/

char *RootTexture::GetCritterName(RasterAnimHost *TestMe)
{

return ("");

} // RootTexture::GetCritterName

/*===========================================================================*/

void RootTexture::EditRAHost(void)
{
AnimColorTime *DefaultColor = NULL;
RasterAnimHostProperties Prop;

DONGLE_INLINE_CHECK()

if (RAParent)
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
	RAParent->GetRAHostProperties(&Prop);
	if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_MATERIAL)
		DefaultColor = &((MaterialEffect *)RAParent)->DiffuseColor;
	else if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_LIGHT)
		DefaultColor = &((Light *)RAParent)->Color;
	} // if

if(GlobalApp->GUIWins->TXG)
	{
	delete GlobalApp->GUIWins->TXG;
	}
GlobalApp->GUIWins->TXG = new TextureEditGUI(GlobalApp->AppImages, RAParent, this, DefaultColor);
if(GlobalApp->GUIWins->TXG)
	{
	GlobalApp->GUIWins->TXG->Open(GlobalApp->MainProj);
	}

} // RootTexture::EditRAHost

/*===========================================================================*/

int RootTexture::ConfigureForEcosystem(void)
{
Texture *Current = Tex;

ApplyToEcosys = 1;
PreviewDirection = 2;

while (Current)
	{
	if (! Current->ConfigureForEcosystem())
		return (0);
	Current = Current->Next;
	} // while

return (1);

} // RootTexture::ConfigureForEcosystem

/*===========================================================================*/

int RootTexture::ConfigureForObject3D(void)
{
Texture *Current = Tex;

ApplyToEcosys = 0;
PreviewDirection = 5;

while (Current)
	{
	if (! Current->ConfigureForObject3D())
		return (0);
	Current = Current->Next;
	} // while

return (1);

} // RootTexture::ConfigureForObject3D

/*===========================================================================*/

long RootTexture::InitImageIDs(long &ImageID)
{
Texture *Current = Tex;
long ImagesFound = 0;

while (Current)
	{
	ImagesFound += Current->InitImageIDs(ImageID);
	Current = Current->Next;
	} // while

return (ImagesFound);

} // RootTexture::InitImageIDs

/*===========================================================================*/

long RootTexture::SetColorList(short *ColorList, long &ListPos)
{
Texture *Current = Tex;
long ColorsFound = 0;

while (Current)
	{
	ColorsFound += Current->SetColorList(ColorList, ListPos);
	Current = Current->Next;
	} // while

return (ColorsFound);

} // RootTexture::SetColorList

/*===========================================================================*/

int RootTexture::InitAAChain(void)
{
Texture *Current = Tex;

while (Current)
	{
	if (! Current->InitAAChain())
		return (0);
	Current = Current->Next;
	} // while

return (1);

} // RootTexture::InitAAChain

/*===========================================================================*/

int RootTexture::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{
Texture *Current = Tex;

while (Current)
	{
	if (! Current->InitFrameToRender(Lib, Rend))
		return (0);
	Current = Current->Next;
	} // while

return (1);

} // RootTexture::InitFrameToRender

/*===========================================================================*/

int RootTexture::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{
Texture *Current = Tex;

while (Current)
	{
	if (! Current->InitToRender(Opt, Buffers))
		return (0);
	Current = Current->Next;
	} // while

return (1);

} // RootTexture::InitToRender

/*===========================================================================*/

void RootTexture::ResolveLoadLinkages(EffectsLib *Lib)
{
Texture *Current = Tex;

while (Current)
	{
	Current->ResolveLoadLinkages(Lib);
	Current = Current->Next;
	} // while

} // RootTexture::ResolveLoadLinkages

/*===========================================================================*/

void RootTexture::PropagateCoordSpace(unsigned char NewSpace)
{
Texture *Current = Tex;

while (Current)
	{
	Current->PropagateCoordSpace(NewSpace);
	Current = Current->Next;
	} // while

} // RootTexture::PropagateCoordSpace

/*===========================================================================*/

int RootTexture::AdjustTextureOrder(Texture *MoveMe, int Direction)
{
Texture *Current = Tex, *Last = NULL, *LastLast = NULL;

while (Current)
	{
	if (Current == MoveMe)
		{
		if (Direction > 0)
			{
			if (Current->Next)
				{
				if (Last)
					{
					Last->Next = Current->Next;
					Current->Next = Current->Next->Next;
					Last->Next->Next = Current;
					} // if
				else
					{
					Tex = Current->Next;
					Current->Next = Current->Next->Next;
					Tex->Next = Current;
					} // else
				return (1);
				} // if
			else
				{
				return (0);
				} // else
			} // if
		else
			{
			if (Last)
				{
				if (LastLast)
					{
					Last->Next = Current->Next;
					Current->Next = Last;
					LastLast->Next = Current;
					} // if
				else
					{
					Last->Next = Current->Next;
					Current->Next = Last;
					Tex = Current;
					} // else
				return (1);
				} // if
			else
				{
				return (0);
				} // else
			} // else
		} // if
	LastLast = Last;
	Last = Current;
	Current = Current->Next;
	} // while

return (0);

} // RootTexture::AdjustTextureOrder

/*===========================================================================*/

Texture *RootTexture::AddNewTexture(AnimColorTime *DefaultColor)
{
Texture *Current = Tex, *Last = NULL;
NotifyTag Changes[2];

while (Current)
	{
	Last = Current;
	Current = Current->Next;
	} // while

if (Last)
	{
	Current = Last->Next = new FractalNoiseTexture(this, WCS_TEXTURE_ROOT, ApplyToEcosys, ApplyToColor, NULL, DefaultColor);
	} // if
else
	{
	Current = Tex = new FractalNoiseTexture(this, WCS_TEXTURE_ROOT, ApplyToEcosys, ApplyToColor, NULL, DefaultColor);
	} // else

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (Current);

} // RootTexture::AddNewTexture

/*===========================================================================*/

Texture *RootTexture::AddNewTexture(Texture *Proto, unsigned char TexType)
{
Texture *Current = Tex, *Last = NULL;
NotifyTag Changes[2];

while (Current)
	{
	Last = Current;
	Current = Current->Next;
	} // while

if (Last)
	{
	Current = Last->Next = NewTexture(this, Proto, TexType, WCS_TEXTURE_ROOT);
	} // if
else
	{
	Current = Tex = NewTexture(this, Proto, TexType, WCS_TEXTURE_ROOT);
	} // else
if (! Proto)
	{
	Current->ApplyToEcosys = ApplyToEcosys;
	Current->ApplyToColor = ApplyToColor;
	} // if

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (Current);

} // RootTexture::AddNewTexture

/*===========================================================================*/

Texture *RootTexture::RemoveTexture(Texture *RemoveMe)
{
Texture *Current = Tex, *Last = NULL, *RemovedParent;

while (Current)
	{
	if (Current == RemoveMe)
		{
		if (Last)
			{
			Last->Next = Current->Next;
			delete Current;
			return (Last);
			} // if
		else
			{
			Tex = Current->Next;
			delete Current;
			return (Tex);
			} // else
		} // if
	if (RemovedParent = Current->RemoveTexture(RemoveMe))
		{
		return (RemovedParent);
		} // else if
	Last = Current;
	Current = Current->Next;
	} // while

return (RemoveMe);

} // RootTexture::RemoveTexture

/*===========================================================================*/

int RootTexture::RemoveRAHost(RasterAnimHost *RemoveMe)
{
Texture *CurTex = Tex, *PrevTex = NULL;
NotifyTag Changes[2];

while (CurTex)
	{
	if (CurTex == (Texture *)RemoveMe)
		{
		if (PrevTex)
			PrevTex->Next = CurTex->Next;
		else
			Tex = CurTex->Next;

		delete CurTex;

		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

		return (1);
		} // if
	PrevTex = CurTex;
	CurTex = CurTex->Next;
	} // while

return (0);

} // RootTexture::RemoveRAHost

/*===========================================================================*/

Texture *RootTexture::GetValidTexture(Texture *TestActive)
{
Texture *CurTex = Tex;

while (CurTex)
	{
	if (CurTex == TestActive)
		return (TestActive);
	if (CurTex->FindTexture(TestActive))
		return (TestActive);
	CurTex = CurTex->Next;
	} // while

return (Tex);

} // RootTexture::GetValidTexture

/*===========================================================================*/

int RootTexture::VertexUVWAvailable(void)
{
RasterAnimHost *Root;
RasterAnimHostProperties Prop;

Root = GetRAHostRoot();
Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
Root->GetRAHostProperties(&Prop);
if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_MATERIAL)
	return (((MaterialEffect *)Root)->VertexUVWAvailable());

return (0);

} // RootTexture::VertexUVWAvailable

/*===========================================================================*/

int RootTexture::VertexCPVAvailable(void)
{
RasterAnimHost *Root;
RasterAnimHostProperties Prop;

Root = GetRAHostRoot();
Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
Root->GetRAHostProperties(&Prop);
if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_MATERIAL)
	return (((MaterialEffect *)Root)->VertexColorsAvailable());

return (0);

} // RootTexture::VertexCPVAvailable

/*===========================================================================*/

// this is a static member of RootTexture
float *RootTexture::LoadPreviewFile(char PreviewType)
{
float **LoadToPtr = NULL;
char *Name, filename[256], temppath[256], Success = 0;
FILE *ffile;
#ifdef BYTEORDER_BIGENDIAN
long Ct;
#endif // BYTEORDER_BIGENDIAN

if (PreviewType == WCS_TEXTURE_PREVIEW_CUBE)
	{
	LoadToPtr = &CubePreviewData;
	Name = "PreviewCube.bin";
	} // if
else if (PreviewType == WCS_TEXTURE_PREVIEW_SPHERE)
	{
	LoadToPtr = &SpherePreviewData;
	Name = "PreviewSphere.bin";
	} // if

if (LoadToPtr)
	{
	if (! *LoadToPtr)
		*LoadToPtr = (float *)AppMem_Alloc(WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE * sizeof (float) * 4, 0);
	if (*LoadToPtr)
		{
		GlobalApp->GetProgDir(filename, 254);
		strmfp(temppath, filename, "Tools");
		strmfp(filename, temppath, Name);
		if (ffile = PROJ_fopen(filename, "rb"))
			{
			Success = (char)(fread(*LoadToPtr, WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE * sizeof (float), 4, ffile) == 4);
			#ifdef BYTEORDER_BIGENDIAN
			if (Success)
				{
				for (Ct = 0; Ct < WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE * 4; Ct ++)
					{
					SimpleEndianFlip32F(&(*LoadToPtr)[Ct], &(*LoadToPtr)[Ct]);
					} // for
				} // if
			#endif // BYTEORDER_BIGENDIAN
			fclose(ffile);
			} // if
		} // if
	} // if

if (Success)
	return (*LoadToPtr);
return (NULL);

} // RootTexture::LoadPreviewFile

/*===========================================================================*/

int RootTexture::EvalSampleInit(Raster *PlotRast, TextureSampleData *Samp)
{
long ReadyGo;

Samp->PreviewDataPtr = Samp->XData = Samp->YData = Samp->ZData = Samp->IllumData = NULL;
Samp->Running = 0;
Samp->SampleInc = Samp->PreviewSize / (WCS_RASTER_TNAIL_SIZE - 1);
Samp->SampleStart = Samp->PreviewSize * 0.5;

if (! PlotRast->ThumbnailValid())
	ReadyGo = (long)PlotRast->AllocThumbnail();
else
	{
	ReadyGo = 1;
	//PlotRast->ClearThumbnail();
	} // else
Samp->Thumb = PlotRast->Thumb;

Samp->TexData.IsSampleThumbnail = 1;
Samp->TexData.ThumbnailX = Samp->TexData.ThumbnailY = 0.0;

if (ReadyGo)
	{
	BuildTextureRotationMatrices();

	Samp->StartX = Samp->StartY = Samp->StartZ = Samp->XHInc = Samp->XVInc = Samp->YHInc = Samp->YVInc = Samp->ZHInc = Samp->ZVInc = 0.0;

	switch (Samp->PreviewDirection)
		{
		case WCS_TEXTURE_PREVIEW_PLUSX:
			{
			if (ApplyToEcosys)
				{
				// -y left, +z top
				Samp->StartY = -Samp->SampleStart;
				Samp->YHInc = Samp->SampleInc;
				Samp->StartZ = Samp->SampleStart;
				Samp->ZVInc = -Samp->SampleInc;
				} // if
			else
				{
				// -z left, +y top
				Samp->StartY = Samp->SampleStart;
				Samp->YVInc = -Samp->SampleInc;
				Samp->StartZ = -Samp->SampleStart;
				Samp->ZHInc = Samp->SampleInc;
				} // else
			break;
			} // + X
		case WCS_TEXTURE_PREVIEW_PLUSY:
			{
			if (ApplyToEcosys)
				{
				// +x left, +z top
				Samp->StartX = Samp->SampleStart;
				Samp->XHInc = -Samp->SampleInc;
				Samp->StartZ = Samp->SampleStart;
				Samp->ZVInc = -Samp->SampleInc;
				} // if
			else
				{
				// -x left, +z top
				Samp->StartX = -Samp->SampleStart;
				Samp->XHInc = Samp->SampleInc;
				Samp->StartZ = Samp->SampleStart;
				Samp->ZVInc = -Samp->SampleInc;
				} // else
			break;
			} // + Y
		case WCS_TEXTURE_PREVIEW_PLUSZ:
			{
			if (ApplyToEcosys)
				{
				// -x left, +y top
				Samp->StartX = -Samp->SampleStart;
				Samp->XHInc = Samp->SampleInc;
				Samp->StartY = Samp->SampleStart;
				Samp->YVInc = -Samp->SampleInc;
				} // if
			else
				{
				// +x left, +y top
				Samp->StartX = Samp->SampleStart;
				Samp->XHInc = -Samp->SampleInc;
				Samp->StartY = Samp->SampleStart;
				Samp->YVInc = -Samp->SampleInc;
				} // else
			break;
			} // + Z
		case WCS_TEXTURE_PREVIEW_MINUSX:
			{
			if (ApplyToEcosys)
				{
				// +y left, +z top
				Samp->StartY = Samp->SampleStart;
				Samp->YHInc = -Samp->SampleInc;
				Samp->StartZ = Samp->SampleStart;
				Samp->ZVInc = -Samp->SampleInc;
				} // if
			else
				{
				// +z left, +y top
				Samp->StartY = Samp->SampleStart;
				Samp->YVInc = -Samp->SampleInc;
				Samp->StartZ = Samp->SampleStart;
				Samp->ZHInc = -Samp->SampleInc;
				} // else
			break;
			} // - X
		case WCS_TEXTURE_PREVIEW_MINUSY:
			{
			if (ApplyToEcosys)
				{
				// -x left, +z top
				Samp->StartX = -Samp->SampleStart;
				Samp->XHInc = Samp->SampleInc;
				Samp->StartZ = Samp->SampleStart;
				Samp->ZVInc = -Samp->SampleInc;
				} // if
			else
				{
				// +x left, +z top
				Samp->StartX = Samp->SampleStart;
				Samp->XHInc = -Samp->SampleInc;
				Samp->StartZ = Samp->SampleStart;
				Samp->ZVInc = -Samp->SampleInc;
				} // else
			break;
			} // - Y
		case WCS_TEXTURE_PREVIEW_MINUSZ:
			{
			if (ApplyToEcosys)
				{
				// +x left, +y top
				Samp->StartX = Samp->SampleStart;
				Samp->XHInc = -Samp->SampleInc;
				Samp->StartY = Samp->SampleStart;
				Samp->YVInc = -Samp->SampleInc;
				} // if
			else
				{
				// -x left, +y top
				Samp->StartX = -Samp->SampleStart;
				Samp->XHInc = Samp->SampleInc;
				Samp->StartY = Samp->SampleStart;
				Samp->YVInc = -Samp->SampleInc;
				} // else
			break;
			} // - Z
		case WCS_TEXTURE_PREVIEW_CUBE:
			{
			if (! CubePreviewLoaded())
				LoadPreviewFile(WCS_TEXTURE_PREVIEW_CUBE);
			if (! CubePreviewLoaded())
				{
				UserMessageOK("Texture Preview", "Unable to load Cube Preview file!");
				return (0);
				} // if
			Samp->PreviewDataPtr = CubePreviewData;
			break;
			} // Cube
		case WCS_TEXTURE_PREVIEW_SPHERE:
			{
			if (! SpherePreviewLoaded())
				LoadPreviewFile(WCS_TEXTURE_PREVIEW_SPHERE);
			if (! SpherePreviewLoaded())
				{
				UserMessageOK("Texture Preview", "Unable to load Sphere Preview file!");
				return (0);
				} // if
			Samp->PreviewDataPtr = SpherePreviewData;
			break;
			} // Cube
		} // switch

	if (! Samp->PreviewDataPtr)
		{
		Samp->TexData.SampleType = WCS_SAMPLETEXTURE_FLAT;
		Samp->XInc = fabs(Samp->XVInc) > fabs(Samp->XHInc) ? Samp->XVInc: Samp->XHInc;
		Samp->YInc = fabs(Samp->YVInc) > fabs(Samp->YHInc) ? Samp->YVInc: Samp->YHInc;
		Samp->ZInc = fabs(Samp->ZVInc) > fabs(Samp->ZHInc) ? Samp->ZVInc: Samp->ZHInc;
		} // if not cube or sphere
	else
		{
		Samp->TexData.SampleType = WCS_SAMPLETEXTURE_PROJECTED;
		Samp->XData = Samp->PreviewDataPtr;
		Samp->YData = &Samp->PreviewDataPtr[WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE];
		Samp->ZData = &Samp->PreviewDataPtr[2 * WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE];
		Samp->IllumData = &Samp->PreviewDataPtr[3 * WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE];
		} // else
	Samp->y = Samp->zip = 0;
	Samp->Running = 1;
	return (1);
	} // if

return (0);

} // RootTexture::EvalSampleInit

/*===========================================================================*/

// returns 0 if more to do, 1 if done
int RootTexture::EvalOneSampleLine(TextureSampleData *Samp)
{
double SumOpacity, NewOpacity, OldOpacity, OpacityFactor, Illum, Value[3];
long x, CurTexType;
Texture *CurTex;
int StrataBlend;

// clear the next row of data
memset(&Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip], 0, WCS_RASTER_TNAIL_SIZE);
memset(&Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip], 0, WCS_RASTER_TNAIL_SIZE);
memset(&Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip], 0, WCS_RASTER_TNAIL_SIZE);
Samp->TexData.ThumbnailX = 0.0;

if (! Samp->PreviewDataPtr)
	{
	for (x = 0, Samp->TexData.TLowX = Samp->StartX, Samp->TexData.TLowY = Samp->StartY, Samp->TexData.TLowZ = Samp->StartZ; 
		x < WCS_RASTER_TNAIL_SIZE; 
		x ++, Samp->zip ++, Samp->TexData.TLowX += Samp->XHInc, Samp->TexData.TLowY += Samp->YHInc, Samp->TexData.TLowZ += Samp->ZHInc)
		{
		CurTex = Tex;
		SumOpacity = 0.0;
		while (CurTex && SumOpacity < 1.0)
			{
			if (CurTex->Enabled)
				{
				CurTexType = CurTex->GetTexType();
				Samp->TexData.THighX = Samp->TexData.TLowX + Samp->XInc;
				Samp->TexData.THighY = Samp->TexData.TLowY + Samp->YInc;
				Samp->TexData.THighZ = Samp->TexData.TLowZ + Samp->ZInc;
				Value[0] = Value[1] = Value[2] = 0.0;
				StrataBlend = 0;	//! WCS_TEXTURETYPE_BLENDILLEGAL(CurTexType);
				if ((NewOpacity = CurTex->Eval(Value, &Samp->TexData, TRUE, StrataBlend)) > 0.0)
					{
					OldOpacity = SumOpacity;
					if ((SumOpacity += NewOpacity) > 1.0)
						{
						OpacityFactor = (1.0 - OldOpacity) / NewOpacity;
						NewOpacity = 1.0 - OldOpacity;
						SumOpacity = 1.0;
						Value[0] *= OpacityFactor;
						Value[1] *= OpacityFactor;
						Value[2] *= OpacityFactor;
						} // if
					if (ApplyToColor)
						{
						if (Value[WCS_RASTER_IMAGE_BAND_RED] > 1.0)
							Value[WCS_RASTER_IMAGE_BAND_RED] = 1.0;
						if (Value[WCS_RASTER_IMAGE_BAND_GREEN] > 1.0)
							Value[WCS_RASTER_IMAGE_BAND_GREEN] = 1.0;
						if (Value[WCS_RASTER_IMAGE_BAND_BLUE] > 1.0)
							Value[WCS_RASTER_IMAGE_BAND_BLUE] = 1.0;
						Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip] += (UBYTE)(255.99 * Value[WCS_RASTER_IMAGE_BAND_RED]);
						Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip] += (UBYTE)(255.99 * Value[WCS_RASTER_IMAGE_BAND_GREEN]);
						Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip] += (UBYTE)(255.99 * Value[WCS_RASTER_IMAGE_BAND_BLUE]);
						} // if
					else
						{
						if (Value[WCS_RASTER_IMAGE_BAND_RED] > 1.0)
							Value[WCS_RASTER_IMAGE_BAND_RED] = 1.0;
						Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip] += (UBYTE)(255.99 * Value[WCS_RASTER_IMAGE_BAND_RED]);
						Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip] =
							Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip] = Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip];
						} // else
					} // if > 0
				} // if
			CurTex = CurTex->Next;
			} // while
		Samp->TexData.ThumbnailX += 1.0 / (WCS_RASTER_TNAIL_SIZE - 1);
		} // for
	Samp->StartX += Samp->XVInc;
	Samp->StartY += Samp->YVInc;
	Samp->StartZ += Samp->ZVInc;
	} // if not cube or sphere
else
	{
	for (x = 0; x < WCS_RASTER_TNAIL_SIZE; x ++, Samp->zip ++)
		{
		Illum = Samp->IllumData[Samp->zip];
		if (Illum > .01)
			{
			CurTex = Tex;
			SumOpacity = 0.0;
			Samp->TexData.TLowX = Samp->XData[Samp->zip] * Samp->PreviewSize;
			Samp->TexData.THighX = Samp->XData[Samp->zip + WCS_RASTER_TNAIL_SIZE + 1] * Samp->PreviewSize;
			if (ApplyToEcosys)
				{
				Samp->TexData.TLowZ = Samp->YData[Samp->zip] * Samp->PreviewSize;
				Samp->TexData.TLowY = Samp->ZData[Samp->zip] * Samp->PreviewSize;
				Samp->TexData.THighZ = Samp->YData[Samp->zip + WCS_RASTER_TNAIL_SIZE + 1] * Samp->PreviewSize;
				Samp->TexData.THighY = Samp->ZData[Samp->zip + WCS_RASTER_TNAIL_SIZE + 1] * Samp->PreviewSize;
				} // if
			else
				{
				Samp->TexData.TLowY = Samp->YData[Samp->zip] * Samp->PreviewSize;
				Samp->TexData.TLowZ = Samp->ZData[Samp->zip] * Samp->PreviewSize;
				Samp->TexData.THighY = Samp->YData[Samp->zip + WCS_RASTER_TNAIL_SIZE + 1] * Samp->PreviewSize;
				Samp->TexData.THighZ = Samp->ZData[Samp->zip + WCS_RASTER_TNAIL_SIZE + 1] * Samp->PreviewSize;
				} // else
			while (CurTex && SumOpacity < 1.0)
				{
				if (CurTex->Enabled)
					{
					CurTexType = CurTex->GetTexType();
					Value[0] = Value[1] = Value[2] = 0.0;
					StrataBlend = 0;	//! WCS_TEXTURETYPE_BLENDILLEGAL(CurTexType);
					if ((NewOpacity = CurTex->Eval(Value, &Samp->TexData, TRUE, StrataBlend)) > 0.0)
						{
						OldOpacity = SumOpacity;
						if ((SumOpacity += NewOpacity) > 1.0)
							{
							OpacityFactor = (1.0 - OldOpacity) / NewOpacity;
							NewOpacity = 1.0 - OldOpacity;
							SumOpacity = 1.0;
							Value[0] *= OpacityFactor;
							Value[1] *= OpacityFactor;
							Value[2] *= OpacityFactor;
							} // if
						if (ApplyToColor)
							{
							if (Value[WCS_RASTER_IMAGE_BAND_RED] > 1.0)
								Value[WCS_RASTER_IMAGE_BAND_RED] = 1.0;
							if (Value[WCS_RASTER_IMAGE_BAND_GREEN] > 1.0)
								Value[WCS_RASTER_IMAGE_BAND_GREEN] = 1.0;
							if (Value[WCS_RASTER_IMAGE_BAND_BLUE] > 1.0)
								Value[WCS_RASTER_IMAGE_BAND_BLUE] = 1.0;
							Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip] += (UBYTE)(255.99 * Value[WCS_RASTER_IMAGE_BAND_RED] * Illum);
							Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip] += (UBYTE)(255.99 * Value[WCS_RASTER_IMAGE_BAND_GREEN] * Illum);
							Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip] += (UBYTE)(255.99 * Value[WCS_RASTER_IMAGE_BAND_BLUE] * Illum);
							} // if
						else
							{
							if (Value[WCS_RASTER_IMAGE_BAND_RED] > 1.0)
								Value[WCS_RASTER_IMAGE_BAND_RED] = 1.0;
							Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip] += (UBYTE)(255.99 * Value[WCS_RASTER_IMAGE_BAND_RED] * Illum);
							Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip] =
								Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip] = Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip];
							} // else
						} // if > 0
					} // if
				CurTex = CurTex->Next;
				} // while
			} // if pixel worth sampling
		Samp->TexData.ThumbnailX += 1.0 / (WCS_RASTER_TNAIL_SIZE - 1);
		} // for
	} // else

Samp->y ++;
Samp->TexData.ThumbnailY += 1.0 / (WCS_RASTER_TNAIL_SIZE - 1);

if (Samp->y < WCS_RASTER_TNAIL_SIZE)
	return (0);

Samp->Running = 0;
return (1);

} // RootTexture::EvalOneSampleLine

/*===========================================================================*/

double RootTexture::Eval(double Output[3], TextureData *Data)
{
double SumOpacity, NewOpacity, OldOpacity, OpacityFactor, Value[3], OrigValue[3];
Texture *CurTex;
long CurTexType;
int StrataBlend, InputSuppliedInOutput;

OrigValue[0] = Output[0];
OrigValue[1] = Output[1];
OrigValue[2] = Output[2];

InputSuppliedInOutput = Data->InputSuppliedInOutput;
Data->InputSuppliedInOutput = 0;

SumOpacity = Output[0] = Output[1] = Output[2] = 0.0;
CurTex = Tex;
while (CurTex && SumOpacity < 1.0)
	{
	if (CurTex->Enabled)
		{
		CurTexType = CurTex->GetTexType();
		//if (StrataBlend = ! WCS_TEXTURETYPE_BLENDILLEGAL(CurTexType))
		if (StrataBlend = (InputSuppliedInOutput && ! WCS_TEXTURETYPE_BLENDILLEGAL(CurTexType)))
			{
			Value[0] = OrigValue[0];
			Value[1] = OrigValue[1];
			Value[2] = OrigValue[2];
			} // if
		if ((NewOpacity = CurTex->Eval(Value, Data, TRUE, StrataBlend)) > 0.0)
			{
			OldOpacity = SumOpacity;
			if ((SumOpacity += NewOpacity) > 1.0)
				{
				OpacityFactor = (1.0 - OldOpacity) / NewOpacity;
				NewOpacity = 1.0 - OldOpacity;
				SumOpacity = 1.0;
				Value[0] *= OpacityFactor;
				Value[1] *= OpacityFactor;
				Value[2] *= OpacityFactor;
				} // if
			if (ApplyToColor)
				{
				Output[0] += Value[WCS_RASTER_IMAGE_BAND_RED];
				Output[1] += Value[WCS_RASTER_IMAGE_BAND_GREEN];
				Output[2] += Value[WCS_RASTER_IMAGE_BAND_BLUE];
				} // if
			else
				{
				Output[0] += Value[WCS_RASTER_IMAGE_BAND_RED];
				Output[1] = Output[2] = Output[0];
				} // else
			} // if > 0
		} // if
	CurTex = CurTex->Next;
	} // while

return (SumOpacity);

} // RootTexture::Eval

/*===========================================================================*/

// <<<>>>gh this version needs to become obsolete
/*
double RootTexture::Eval(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, TextureData *Data)
{
double SumOpacity;

Output[0] = Output[1] = Output[2] = SumOpacity = 0.0;

return (SumOpacity);

} // RootTexture::Eval
*/
/*===========================================================================*/

void RootTexture::BuildTextureRotationMatrices(void)
{
Texture *Current = Tex;

while (Current)
	{
	Current->BuildTextureRotationMatrix(TRUE);
	Current = Current->Next;
	} // while

} // RootTexture::BuildTextureRotationMatrices

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int RootTexture::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[64];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
RootTexture *CurrentTex = NULL;
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
					else if (! strnicmp(ReadBuf, "RootTex", 8))
						{
						if (CurrentTex = new RootTexture(NULL, ApplyToEcosys, ApplyToColor, ApplyToDisplace))
							{
							if ((BytesRead = CurrentTex->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if Root Texture
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

if (Success == 1 && CurrentTex)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentTex);
	delete CurrentTex;
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

} // RootTexture::LoadObject

/*===========================================================================*/

int RootTexture::SaveObject(FILE *ffile)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
EffectList *CurEffect, *Coords = NULL;
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

if (BuildFileComponentsList(&Coords))
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

	while (Coords)
		{
		CurEffect = Coords;
		Coords = Coords->Next;
		delete CurEffect;
		} // while
	} // if

// RootTexture
strcpy(StrBuf, "RootTex");
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
			} // if RootTexture saved 
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

} // RootTexture::SaveObject

/*===========================================================================*/
/*===========================================================================*/

Texture::Texture(RasterAnimHost *RAHost, unsigned char SetTexType)
: RasterAnimHost(RAHost), ColorGrad(this, 0)
{

SetDefaults(SetTexType, NULL);

} // Texture::Texture

/*===========================================================================*/

Texture::Texture(RasterAnimHost *RAHost, unsigned char SetTexType, unsigned char EcosysSource, unsigned char ColorSource, AnimColorTime *DefaultColor)
: RasterAnimHost(RAHost), ColorGrad(this, EcosysSource)
{

SetDefaults(SetTexType, ColorSource ? DefaultColor: NULL);
ApplyToEcosys = EcosysSource;
ApplyToColor = ColorSource;

} // Texture::Texture

/*===========================================================================*/

Texture::~Texture()
{
int Ct;
Texture *TempTex;

if (Img)
	{
	if (Img->GetRaster())
		{
		Img->GetRaster()->RemoveAttribute(Img);
		} // if
	delete Img;
	Img = NULL;
	} // if

switch (TexType)
	{
	case WCS_TEXTURE_TYPE_WOODGRAIN:
		{
		((WoodGrainTexture *)this)->DeleteSpecificResources();
		break;
		} // WCS_TEXTURE_TYPE_WOODGRAIN
	case WCS_TEXTURE_TYPE_MARBLE:
		{
		((MarbleTexture *)this)->DeleteSpecificResources();
		break;
		} // WCS_TEXTURE_TYPE_MARBLE
	case WCS_TEXTURE_TYPE_FRACTALNOISE:
		{
		((FractalNoiseTexture *)this)->DeleteSpecificResources();
		break;
		} // WCS_TEXTURE_TYPE_FRACTALNOISE
	case WCS_TEXTURE_TYPE_MULTIFRACTALNOISE:
		{
		((MultiFractalNoiseTexture *)this)->DeleteSpecificResources();
		break;
		} // WCS_TEXTURE_TYPE_MULTIFRACTALNOISE
	case WCS_TEXTURE_TYPE_HYBRIDMULTIFRACTALNOISE:
		{
		((HybridMultiFractalNoiseTexture *)this)->DeleteSpecificResources();
		break;
		} // WCS_TEXTURE_TYPE_HYBRIDMULTIFRACTALNOISE
	case WCS_TEXTURE_TYPE_RIDGEDMULTIFRACTALNOISE:
		{
		((RidgedMultiFractalNoiseTexture *)this)->DeleteSpecificResources();
		break;
		} // WCS_TEXTURE_TYPE_RIDGEDMULTIFRACTALNOISE
	case WCS_TEXTURE_TYPE_HETEROTERRAINNOISE:
		{
		((HeteroTerrainNoiseTexture *)this)->DeleteSpecificResources();
		break;
		} // WCS_TEXTURE_TYPE_HETEROTERRAINNOISE
	case WCS_TEXTURE_TYPE_TURBULENCE:
		{
		((TurbulenceTexture *)this)->DeleteSpecificResources();
		break;
		} // WCS_TEXTURE_TYPE_TURBULENCE
	case WCS_TEXTURE_TYPE_F1CELLBASIS:
		{
		((F1CellBasisTexture *)this)->DeleteSpecificResources();
		break;
		} // WCS_TEXTURE_TYPE_F1CELLBASIS
	case WCS_TEXTURE_TYPE_F2CELLBASIS:
		{
		((F2CellBasisTexture *)this)->DeleteSpecificResources();
		break;
		} // WCS_TEXTURE_TYPE_F2CELLBASIS
	case WCS_TEXTURE_TYPE_F2MF1CELLBASIS:
		{
		((F2mF1CellBasisTexture *)this)->DeleteSpecificResources();
		break;
		} // WCS_TEXTURE_TYPE_F2MF1CELLBASIS
	case WCS_TEXTURE_TYPE_F3MF1CELLBASIS:
		{
		((F3mF1CellBasisTexture *)this)->DeleteSpecificResources();
		break;
		} // WCS_TEXTURE_TYPE_F3MF1CELLBASIS
	case WCS_TEXTURE_TYPE_F1MANHATTAN:
		{
		((F1ManhattanTexture *)this)->DeleteSpecificResources();
		break;
		} // WCS_TEXTURE_TYPE_F1MANHATTAN
	case WCS_TEXTURE_TYPE_F2MANHATTAN:
		{
		((F2ManhattanTexture *)this)->DeleteSpecificResources();
		break;
		} // WCS_TEXTURE_TYPE_F2MANHATTAN
	case WCS_TEXTURE_TYPE_F2MF1MANHATTAN:
		{
		((F2mF1ManhattanTexture *)this)->DeleteSpecificResources();
		break;
		} // WCS_TEXTURE_TYPE_F2MF1MANHATTAN
	case WCS_TEXTURE_TYPE_BIRDSHOT:
		{
		((BirdshotTexture *)this)->DeleteSpecificResources();
		break;
		} // WCS_TEXTURE_TYPE_BIRDSHOT
	case WCS_TEXTURE_TYPE_CUSTOMCURVE:
		{
		((CustomCurveTexture *)this)->DeleteSpecificResources();
		break;
		} // WCS_TEXTURE_TYPE_CUSTOMCURVE
// <<<>>> ADD_NEW_TEXTURE if resources are allocated especially for this element class
	} // switch

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	if (Tex[Ct])
		{
		TempTex = Tex[Ct];
		Tex[Ct] = NULL;
		delete TempTex;	// this will be recursive but what's the option?
		} // if
	} // for

if (InclExcl)
	delete InclExcl;
InclExcl = NULL;
if (InclExclType)
	delete InclExclType;
InclExclType = NULL;

if (AASampler)
	delete AASampler;
AASampler = NULL;

} // Texture::~Texture

/*===========================================================================*/

void Texture::SetDefaults(unsigned char SetTexType, AnimColorTime *DefaultColor)
{
double FalloffRangeDefaults[3] = {10000.0, 0.0, 1.0};
double SizeRangeDefaults[3] = {FLT_MAX, 0.00001, .5};
double FeatherRangeDefaults[3] = {10.0, 0.0, 1.0};
int Ct;
GradientCritter *GradNode;

Next = Prev = NULL;
AASampler = NULL;
ApplyToEcosys = 0;
ApplyToColor = 0;
AbsoluteOutput = 0;

CoordSpace = GetDefaultCoordSpace(SetTexType);

TexType = SetTexType;
Enabled = 1;
if (WCS_TEXTURETYPE_ISBITMAP(SetTexType))
	Antialias = 1;
else
	Antialias = 0;
SelfOpacity = 0;
AASamples = 2;
TexAxis = 2;
Rotated = 0;

Quantize = 0;

// obsolete
PalColor[0] = 0;
PalColor[1] = 0;

if (GradNode = ColorGrad.AddNode(0.0))
	{
	if (GradNode->GetThing() && DefaultColor)
		{
		((ColorTextureThing *)GradNode->GetThing())->Color.Copy(&((ColorTextureThing *)GradNode->GetThing())->Color, DefaultColor);
		} // if
	} // if
ColorGrad.AddNode(1.0);	// this will default to a random color

ImageNeg = ReverseWidth = ReverseHeight = AlphaOnly = 0;
PixelBlend = 1;
TileWidth = TileHeight = FlipWidth = FlipHeight = 0;
WrapWidth = WrapHeight = 100.0;
Misc = 0;
ThreeD = 1;
MapName[0] = 0;

Img = NULL;
Rast = NULL;
Coords = NULL;
InclExcl = NULL;
InclExclType = NULL;
North = South = West = East = LatRange = LonRange = 0.0;	// for use by renderer

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = 0;
	ParamName[Ct] = "";
	Tex[Ct] = NULL;
	} // for

for (Ct = 0; Ct < 3; Ct ++)
	{
	TexSize[Ct].SetDefaults(this, (char)(Ct), 1.0);
	TexCenter[Ct].SetDefaults(this, (char)(Ct + 3), 0.0);
	TexFalloff[Ct].SetDefaults(this, (char)(Ct + 6), 0.0);
	TexVelocity[Ct].SetDefaults(this, (char)(Ct + 9), 0.0);
	TexRotation[Ct].SetDefaults(this, (char)(Ct + 12), 0.0);
	TexFalloff[Ct].SetRangeDefaults(FalloffRangeDefaults);
	TexSize[Ct].SetRangeDefaults(SizeRangeDefaults);
	TexSize[Ct].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	TexCenter[Ct].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	TexVelocity[Ct].SetMetricType(WCS_ANIMDOUBLE_METRIC_VELOCITY);
	TexRotation[Ct].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
	TexVelocity[Ct].SetNoNodes(1);
	TSize[Ct] = TexSize[Ct].CurValue;
	TCenter[Ct] = TexCenter[Ct].CurValue;
	TFalloff[Ct] = TexFalloff[Ct].CurValue;
	TRotation[Ct] = TexRotation[Ct].CurValue;
	} // for
TexFeather.SetDefaults(this, 19, 0.0);
TexFeather.SetRangeDefaults(FeatherRangeDefaults);
for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), 0.0);
	} // for

} // Texture::SetDefaults

/*===========================================================================*/

void Texture::SetDimensionUnits(void)
{
unsigned char Dimension = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;
double InputRangeDefaults[3] = {FLT_MAX, -FLT_MAX, 1.0};

if (TexType == WCS_TEXTURE_TYPE_TERRAINPARAM)
	{
	switch (Misc)
		{
		case WCS_TEXTURE_TERRAINPARAM_ELEV:
			{
			Dimension = WCS_ANIMDOUBLE_METRIC_HEIGHT;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_ELEV
		case WCS_TEXTURE_TERRAINPARAM_RELELEV:
			{
			Dimension = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_RELELEV
		case WCS_TEXTURE_TERRAINPARAM_SLOPE:
			{
			InputRangeDefaults[1] = 0.0;
			InputRangeDefaults[0] = 90.0;
			Dimension = WCS_ANIMDOUBLE_METRIC_ANGLE;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_SLOPE
		case WCS_TEXTURE_TERRAINPARAM_ASPECT:
			{
			InputRangeDefaults[1] = 0.0;
			InputRangeDefaults[0] = 360.0;
			Dimension = WCS_ANIMDOUBLE_METRIC_ANGLE;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_ASPECT
		case WCS_TEXTURE_TERRAINPARAM_NORTHDEV:
			{
			InputRangeDefaults[1] = 0.0;
			InputRangeDefaults[0] = 180.0;
			Dimension = WCS_ANIMDOUBLE_METRIC_ANGLE;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_NORTHDEV
		case WCS_TEXTURE_TERRAINPARAM_EASTDEV:
			{
			InputRangeDefaults[1] = 0.0;
			InputRangeDefaults[0] = 180.0;
			Dimension = WCS_ANIMDOUBLE_METRIC_ANGLE;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_EASTDEV
		case WCS_TEXTURE_TERRAINPARAM_LATITUDE:
			{
			InputRangeDefaults[1] = -90.0;
			InputRangeDefaults[0] = 90.0;
			Dimension = WCS_ANIMDOUBLE_METRIC_LATITUDE;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_LATITUDE
		case WCS_TEXTURE_TERRAINPARAM_LONGITUDE:
			{
			Dimension = WCS_ANIMDOUBLE_METRIC_LONGITUDE;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_LONGITUDE
		case WCS_TEXTURE_TERRAINPARAM_ILLUMINATION:
			{
			InputRangeDefaults[1] = 0.0;
			InputRangeDefaults[0] = 100000.0;
			Dimension = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_ILLUMINATION
		case WCS_TEXTURE_TERRAINPARAM_ZDISTANCE:
			{
			InputRangeDefaults[1] = 0.0;
			Dimension = WCS_ANIMDOUBLE_METRIC_DISTANCE;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_ZDISTANCE
		case WCS_TEXTURE_TERRAINPARAM_WATERDEPTH:
			{
			Dimension = WCS_ANIMDOUBLE_METRIC_HEIGHT;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_WATERDEPTH
		case WCS_TEXTURE_TERRAINPARAM_VECTORSLOPE:
			{
			InputRangeDefaults[1] = 0.0;
			InputRangeDefaults[0] = 1000.0;
			Dimension = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_VECTORSLOPE
		case WCS_TEXTURE_TERRAINPARAM_RED:
		case WCS_TEXTURE_TERRAINPARAM_GREEN:
		case WCS_TEXTURE_TERRAINPARAM_BLUE:
			{
			InputRangeDefaults[1] = 0.0;
			InputRangeDefaults[0] = 100000.0;
			Dimension = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_RED
		case WCS_TEXTURE_TERRAINPARAM_HUE:
			{
			InputRangeDefaults[1] = 0.0;
			InputRangeDefaults[0] = 360.0;
			Dimension = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_HUE
		case WCS_TEXTURE_TERRAINPARAM_SATURATION:
			{
			InputRangeDefaults[1] = 0.0;
			InputRangeDefaults[0] = 100000.0;
			Dimension = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_SATURATION
		case WCS_TEXTURE_TERRAINPARAM_VALUE:
			{
			InputRangeDefaults[1] = 0.0;
			InputRangeDefaults[0] = 100000.0;
			Dimension = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_VALUE
		case WCS_TEXTURE_TERRAINPARAM_LUMINOSITY:
			{
			InputRangeDefaults[1] = 0.0;
			InputRangeDefaults[0] = 100000.0;
			Dimension = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_LUMINOSITY
		case WCS_TEXTURE_TERRAINPARAM_REFLECTIVITY:
			{
			InputRangeDefaults[1] = 0.0;
			InputRangeDefaults[0] = 100000.0;
			Dimension = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_REFLECTIVITY
		case WCS_TEXTURE_TERRAINPARAM_NORMALFROMCAMERA:
			{
			InputRangeDefaults[1] = 0.0;
			InputRangeDefaults[0] = 90.0;
			Dimension = WCS_ANIMDOUBLE_METRIC_ANGLE;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_NORMALFROMCAMERA
		default:
			{
			InputRangeDefaults[1] = 0.0;
			InputRangeDefaults[0] = 90.0;
			break;
			} // default
		} // switch
	TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetRangeDefaults(InputRangeDefaults);
	TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetRangeDefaults(InputRangeDefaults);
	TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetMetricType(Dimension);
	TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetMetricType(Dimension);
	} // if

} // Texture::SetDimensionUnits

/*===========================================================================*/

unsigned char Texture::GetDefaultCoordSpace(unsigned char SetTexType)
{
RasterAnimHostProperties Prop;
RasterAnimHost *Root;

if (WCS_TEXTURETYPE_ISMATHEMATICAL(SetTexType))
	return (WCS_TEXTURE_COORDSPACE_NONE);

switch (SetTexType)
	{
	case WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE:
		{
		return (WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE_NOZ);
		} // WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE
	case WCS_TEXTURE_TYPE_UVW:
		{
		return (WCS_TEXTURE_COORDSPACE_VERTEX_UVW);
		} // WCS_TEXTURE_TYPE_UVW
	case WCS_TEXTURE_TYPE_COLORPERVERTEX:
		{
		return (WCS_TEXTURE_COORDSPACE_VERTEX_COLORPERVERTEX);
		} // WCS_TEXTURE_TYPE_COLORPERVERTEX
	case WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE:
	case WCS_TEXTURE_TYPE_INCLUDEEXCLUDE:
	case WCS_TEXTURE_TYPE_INCLUDEEXCLUDETYPE:
		{
		return (WCS_TEXTURE_COORDSPACE_NONE);
		} // WCS_TEXTURE_TYPE_COLORPERVERTEX
	} // switch

Root = GetRAHostRoot();
Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
Root->GetRAHostProperties(&Prop);

switch (Prop.TypeNumber)
	{
	case WCS_EFFECTSSUBCLASS_MATERIAL:
	case WCS_EFFECTSSUBCLASS_GENERATOR:
		{
		return (WCS_TEXTURE_COORDSPACE_OBJECT_CARTESIAN);
		} // WCS_EFFECTSSUBCLASS_MATERIAL
	case WCS_EFFECTSSUBCLASS_POSTPROC:
		{
		return (WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE_NOZ);
		} // 
	default:
		{
		return (WCS_TEXTURE_COORDSPACE_PROJECT_GEOGRXYZ);
		} // 
	} // switch
// <<<>>> ADD_NEW_TEXTURE
//return (WCS_TEXTURE_COORDSPACE_PROJECT_GEOGRXYZ); // to appease compilers that don't realize you will always hit one of the cases above.
} // Texture::GetDefaultCoordSpace

/*===========================================================================*/

void Texture::CopyAndTransfer(Texture *CopyFrom)
{
int Ct;

Rast = NULL;
Coords = NULL;

if (CopyFrom)
	{
	Enabled = CopyFrom->Enabled;
	//ApplyToEcosys = CopyFrom->ApplyToEcosys;
	//ApplyToColor = CopyFrom->ApplyToColor;
	for (Ct = 0; Ct < 3; Ct ++)
		{
		TexCenter[Ct].Copy((AnimCritter *)&TexCenter[Ct], (AnimCritter *)&CopyFrom->TexCenter[Ct]);
		TexFalloff[Ct].Copy((AnimCritter *)&TexFalloff[Ct], (AnimCritter *)&CopyFrom->TexFalloff[Ct]);
		TexVelocity[Ct].Copy((AnimCritter *)&TexVelocity[Ct], (AnimCritter *)&CopyFrom->TexVelocity[Ct]);
		TexSize[Ct].Copy((AnimCritter *)&TexSize[Ct], (AnimCritter *)&CopyFrom->TexSize[Ct]);
		TexRotation[Ct].Copy((AnimCritter *)&TexRotation[Ct], (AnimCritter *)&CopyFrom->TexRotation[Ct]);
		TSize[Ct] = CopyFrom->TSize[Ct];
		TCenter[Ct] = CopyFrom->TCenter[Ct];
		TFalloff[Ct] = CopyFrom->TFalloff[Ct];
		TRotation[Ct] = CopyFrom->TRotation[Ct];
		} // for
	if (GetTextureTypeClass() == CopyFrom->GetTextureTypeClass())
		{
		SelfOpacity = CopyFrom->SelfOpacity;
		for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
			{
			// don't encourage propagation of obsolescence
			if (Ct == WCS_TEXTURE_COLOR1 || Ct == WCS_TEXTURE_COLOR2)
				continue;
			if (GetTextureParamType(Ct) == CopyFrom->GetTextureParamType(Ct))
				{
				if (Ct < WCS_TEXTURE_MAXPARAMS)
					{
					TexParam[Ct].Copy((AnimCritter *)&TexParam[Ct], (AnimCritter *)&CopyFrom->TexParam[Ct]);
					} // if
				if (CopyFrom->Tex[Ct])
					{
					Tex[Ct] = CopyFrom->Tex[Ct];
					CopyFrom->Tex[Ct] = NULL;
					Tex[Ct]->Prev = this;
					Tex[Ct]->RAParent = this;
					} // if
//				UseTexture[Ct] = CopyFrom->UseTexture[Ct];
				} // if
			} // for
		//GeoReg.Copy(&GeoReg, &CopyFrom->GeoReg);
		TexFeather.Copy((AnimCritter *)&TexFeather, (AnimCritter *)&CopyFrom->TexFeather);
		if (CopyFrom->Img)
			{
			Img = CopyFrom->Img;
			CopyFrom->Img = NULL;
			Img->SetHost(this);
			} // if
		TexAxis = CopyFrom->TexAxis;

		if (InclExcl)
			{
			if (InclExclType)
				delete InclExclType;
			InclExclType = NULL;
			if (CopyFrom->InclExcl)
				{
				delete InclExcl;
				InclExcl = CopyFrom->InclExcl;
				CopyFrom->InclExcl = NULL;
				} // if
			} // if
		if (InclExclType)
			{
			if (InclExcl)
				delete InclExcl;
			InclExcl = NULL;
			if (CopyFrom->InclExclType)
				{
				delete InclExclType;
				InclExclType = CopyFrom->InclExclType;
				CopyFrom->InclExclType = NULL;
				} // if
			} // if
		ThreeD = WCS_TEXTURETYPE_IS3DOPTIONAL(TexType) ? CopyFrom->ThreeD: 1;
		Antialias = WCS_TEXTURETYPE_USEANTIALIAS(TexType) ? CopyFrom->Antialias: 0;
		AASamples = WCS_TEXTURETYPE_USESUMTABLEANTIALIAS(TexType) ? CopyFrom->AASamples: 2;
		Quantize = CopyFrom->Quantize;

		strcpy(MapName, CopyFrom->MapName);
		SetCoordSpace(ApproveCoordSpace(CopyFrom->CoordSpace) ? CopyFrom->CoordSpace: GetDefaultCoordSpace(TexType), FALSE);
		ImageNeg = CopyFrom->ImageNeg;
		ReverseWidth = CopyFrom->ReverseWidth;
		ReverseHeight = CopyFrom->ReverseHeight;
		AlphaOnly = CopyFrom->AlphaOnly;
		PixelBlend = CopyFrom->PixelBlend;
		TileWidth = CopyFrom->TileWidth;
		TileHeight = CopyFrom->TileHeight;
		FlipWidth = CopyFrom->FlipWidth;
		FlipHeight = CopyFrom->FlipHeight;
		WrapWidth = CopyFrom->WrapWidth;
		WrapHeight = CopyFrom->WrapHeight;
		AbsoluteOutput = CopyFrom->AbsoluteOutput;
		Misc = CopyFrom->Misc;
		} // if
	PalColor[0] = CopyFrom->PalColor[0];
	PalColor[1] = CopyFrom->PalColor[1];
	ColorGrad.Copy(&ColorGrad, &CopyFrom->ColorGrad);
	ColorGrad.SetTexturePrev(this);
	SetDimensionUnits();
	} // if


} // Texture::CopyAndTransfer

/*===========================================================================*/

void Texture::Copy(Texture *CopyTo, Texture *CopyFrom)
{
double RangeDefaults[3] = {100.0, 0.0, 1.0};
double CurrentValue, TempValue, Multiplier;
Raster *NewRast;
RasterAttribute *MyAttr;
CoordSys *MyCoords;
RasterAnimHostProperties Prop;
Texture *DelTex;
int Ct;
int CopyOutputAsIs = 0, DestParamNumber, SourceParamNumber;
unsigned char MetricType;

CopyTo->Rast = NULL;
CopyTo->Coords = NULL;

DestParamNumber = CopyTo->FindTextureNumberInParent();
SourceParamNumber = CopyFrom->FindTextureNumberInParent();
if (CopyTo->RAParent && CopyFrom->RAParent)
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
	CopyTo->RAParent->GetRAHostProperties(&Prop);
	if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ROOTTEXTURE)
		{
		CopyTo->ApplyToColor = ((RootTexture *)CopyTo->RAParent)->ApplyToColor;
		CopyTo->ApplyToEcosys = ((RootTexture *)CopyTo->RAParent)->ApplyToEcosys;
		} // if
	else if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_COLORTEXTURE)
		{
		CopyTo->ApplyToColor = 1;
		CopyTo->ApplyToEcosys = ((ColorTextureThing *)CopyTo->RAParent)->ApplyToEcosys;
		} // else
	else if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
		{
		// take into account that some textures use color texture children in parameters
		//CopyTo->ApplyToColor = 0;
		CopyTo->ApplyToColor = ((Texture *)CopyTo->RAParent)->IsColorChild(DestParamNumber);
		CopyTo->ApplyToEcosys = ((Texture *)CopyTo->RAParent)->ApplyToEcosys;
		} // else
	} // if
else
	{
	CopyTo->ApplyToEcosys = CopyFrom->ApplyToEcosys;
	CopyTo->ApplyToColor = CopyFrom->ApplyToColor;
	CopyOutputAsIs = 1;
	} // else

CopyTo->Enabled = CopyFrom->Enabled;
CopyTo->Antialias = CopyFrom->Antialias;
CopyTo->SelfOpacity = CopyFrom->SelfOpacity;
CopyTo->AASamples = CopyFrom->AASamples;
CopyTo->AbsoluteOutput = CopyFrom->AbsoluteOutput;
for (Ct = 0; Ct < 3; Ct ++)
	{
	TexCenter[Ct].Copy((AnimCritter *)&CopyTo->TexCenter[Ct], (AnimCritter *)&CopyFrom->TexCenter[Ct]);
	TexFalloff[Ct].Copy((AnimCritter *)&CopyTo->TexFalloff[Ct], (AnimCritter *)&CopyFrom->TexFalloff[Ct]);
	TexVelocity[Ct].Copy((AnimCritter *)&CopyTo->TexVelocity[Ct], (AnimCritter *)&CopyFrom->TexVelocity[Ct]);
	TexSize[Ct].Copy((AnimCritter *)&CopyTo->TexSize[Ct], (AnimCritter *)&CopyFrom->TexSize[Ct]);
	TexRotation[Ct].Copy((AnimCritter *)&CopyTo->TexRotation[Ct], (AnimCritter *)&CopyFrom->TexRotation[Ct]);
	CopyTo->TSize[Ct] = CopyFrom->TSize[Ct];
	CopyTo->TCenter[Ct] = CopyFrom->TCenter[Ct];
	CopyTo->TFalloff[Ct] = CopyFrom->TFalloff[Ct];
	CopyTo->TRotation[Ct] = CopyFrom->TRotation[Ct];
	} // for

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	if (Ct < WCS_TEXTURE_MAXPARAMS)
		{
		CopyTo->TexParam[Ct].Copy((AnimCritter *)&CopyTo->TexParam[Ct], (AnimCritter *)&CopyFrom->TexParam[Ct]);
		if (Ct == WCS_TEXTURE_HIGH)
			{
			if (! CopyOutputAsIs)
				{
				if (WCS_TEXTURE_INPUTPARAM(DestParamNumber))
					{
					// if source is not an input parameter or if it is a different one from the destination
					if ((! WCS_TEXTURE_INPUTPARAM(SourceParamNumber)) ||
						(! ((WCS_TEXTURE_SIZE(DestParamNumber) && WCS_TEXTURE_SIZE(SourceParamNumber)) ||
						(WCS_TEXTURE_CENTER(DestParamNumber) && WCS_TEXTURE_CENTER(SourceParamNumber)) ||
						(WCS_TEXTURE_FALLOFF(DestParamNumber) && WCS_TEXTURE_FALLOFF(SourceParamNumber)) ||
						(WCS_TEXTURE_ROTATION(DestParamNumber) && WCS_TEXTURE_ROTATION(SourceParamNumber)))))
						{
						// set metric and defaults for appropriate type
						// we can only get here if parent is a texture
						if (((Texture *)CopyTo->RAParent)->GetAbsoluteOutputRange(DestParamNumber, RangeDefaults, CurrentValue, Multiplier, MetricType))
							{
							CopyTo->ParamFlags[WCS_TEXTURE_LOW] &= ~WCS_TEXTURE_PARAMPERCENT;
							CopyTo->ParamFlags[WCS_TEXTURE_HIGH] &= ~WCS_TEXTURE_PARAMPERCENT;
							CopyTo->TexParam[WCS_TEXTURE_LOW].SetRangeDefaults(RangeDefaults);
							CopyTo->TexParam[WCS_TEXTURE_LOW].SetMultiplier(Multiplier);
							CopyTo->TexParam[WCS_TEXTURE_LOW].SetMetricType(MetricType);
							TempValue = CurrentValue == 0.0 ? -1.0: WCS_floor(CurrentValue - .1 * CurrentValue);
							CopyTo->TexParam[WCS_TEXTURE_LOW].SetValue(TempValue);
							CopyTo->TexParam[WCS_TEXTURE_HIGH].SetRangeDefaults(RangeDefaults);
							CopyTo->TexParam[WCS_TEXTURE_HIGH].SetMultiplier(Multiplier);
							CopyTo->TexParam[WCS_TEXTURE_HIGH].SetMetricType(MetricType);
							TempValue = CurrentValue == 0.0 ? 1.0: WCS_ceil(CurrentValue + .1 * CurrentValue);
							CopyTo->TexParam[WCS_TEXTURE_HIGH].SetValue(TempValue);
							CopyTo->AbsoluteOutput = 1;
							} // if
						} // if
					} // if
				else
					{
					if (WCS_TEXTURE_INPUTPARAM(SourceParamNumber))
						{
						// set to normal defaults
						CopyTo->ParamFlags[WCS_TEXTURE_LOW] |= WCS_TEXTURE_PARAMPERCENT;
						CopyTo->ParamFlags[WCS_TEXTURE_HIGH] |= WCS_TEXTURE_PARAMPERCENT;
						CopyTo->TexParam[WCS_TEXTURE_LOW].SetRangeDefaults(RangeDefaults);
						CopyTo->TexParam[WCS_TEXTURE_LOW].SetMultiplier(1.0);
						CopyTo->TexParam[WCS_TEXTURE_LOW].SetMetricType(WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS);
						CopyTo->TexParam[WCS_TEXTURE_LOW].SetValue(0.0);
						CopyTo->TexParam[WCS_TEXTURE_HIGH].SetRangeDefaults(RangeDefaults);
						CopyTo->TexParam[WCS_TEXTURE_HIGH].SetMultiplier(1.0);
						CopyTo->TexParam[WCS_TEXTURE_HIGH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS);
						CopyTo->TexParam[WCS_TEXTURE_HIGH].SetValue(100.0);
						CopyTo->AbsoluteOutput = 0;
						} // if
					} // else
				} // if
			} // if
		} // if
	if (CopyTo->Tex[Ct])
		{
		DelTex = CopyTo->Tex[Ct];
		CopyTo->Tex[Ct] = NULL;
		delete DelTex;
		} // if
	// colors 1 and 2 are obsolete
	if (CopyFrom->Tex[Ct] && Ct != WCS_TEXTURE_COLOR1 && Ct != WCS_TEXTURE_COLOR2)
		{
		if (CopyFrom->Tex[Ct]->GetApplicationLegal(CopyTo->ApplyToEcosys))
			{
			if (CopyTo->Tex[Ct] = RootTexture::NewTexture(CopyTo, CopyFrom->Tex[Ct], CopyFrom->Tex[Ct]->TexType, Ct))
				{
				CopyTo->Tex[Ct]->Prev = CopyTo;
				CopyTo->Tex[Ct]->Copy(CopyTo->Tex[Ct], CopyFrom->Tex[Ct]);
				} // if
			} // if
		else
			UserMessageOK("Copy Texture", "Not all texture components can be applied\n to the destination object type.\nSome components will not be copied.");
		} // if
	} // for
//GeoReg.Copy(&CopyTo->GeoReg, &CopyFrom->GeoReg);
TexFeather.Copy((AnimCritter *)&CopyTo->TexFeather, (AnimCritter *)&CopyFrom->TexFeather);
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
if (CopyFrom->Img)
	{
	if (CopyTo->Img = new RasterShell())
		{
		if (CopyFrom->Img->GetRaster())
			{
			if (NewRast = GlobalApp->CopyToImageLib->MatchNameMakeRaster(CopyFrom->Img->GetRaster()))
				{
				NewRast->AddAttribute(CopyTo->Img->GetType(), CopyTo->Img, CopyTo);
				if ((MyAttr = CopyFrom->Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
					{
					if (((GeoRefShell *)MyAttr->GetShell())->Host)
						{
						if (MyCoords = GlobalApp->CopyToEffectsLib->CompareMakeCoordSys((CoordSys *)((GeoRefShell *)MyAttr->GetShell())->Host, TRUE))
							MyCoords->AddRaster(NewRast);
						} // if
					}  // if
				} // if
			} // if
		} // if
	} // if
CopyTo->TexAxis = CopyFrom->TexAxis;

if (CopyTo->InclExcl)
	delete CopyTo->InclExcl;
CopyTo->InclExcl = NULL;
if (CopyFrom->InclExcl && (CopyTo->InclExcl = new IncludeExcludeList(CopyTo)))
	{
	CopyTo->InclExcl->Copy(CopyTo->InclExcl, CopyFrom->InclExcl);
	} // if
if (CopyTo->InclExclType)
	delete CopyTo->InclExclType;
CopyTo->InclExclType = NULL;
if (CopyFrom->InclExclType && (CopyTo->InclExclType = new IncludeExcludeTypeList(CopyTo)))
	{
	CopyTo->InclExclType->Copy(CopyTo->InclExclType, CopyFrom->InclExclType);
	} // if

CopyTo->PalColor[0] = CopyFrom->PalColor[0];
CopyTo->PalColor[1] = CopyFrom->PalColor[1];
ColorGrad.Copy(&CopyTo->ColorGrad, &CopyFrom->ColorGrad);
CopyTo->ColorGrad.SetTexturePrev(CopyTo);
if (CopyTo->TexType == WCS_TEXTURE_TYPE_CUSTOMCURVE && CopyFrom->TexType == WCS_TEXTURE_TYPE_CUSTOMCURVE && 
	((CustomCurveTexture *)CopyTo)->CurveADP && ((CustomCurveTexture *)CopyFrom)->CurveADP)
	((CustomCurveTexture *)CopyTo)->CurveADP->Copy(((CustomCurveTexture *)CopyTo)->CurveADP, ((CustomCurveTexture *)CopyFrom)->CurveADP);

CopyTo->RepairApplyToColor(CopyFrom->ApplyToColor);

if (CopyTo->RAParent)
	CopyTo->CoordSpace = CopyTo->ApproveCoordSpace(CopyFrom->CoordSpace) ? CopyFrom->CoordSpace: CopyTo->GetDefaultCoordSpace(CopyTo->TexType);
else
	CopyTo->CoordSpace = CopyFrom->CoordSpace;
CopyTo->ImageNeg = CopyFrom->ImageNeg;
CopyTo->ReverseWidth = CopyFrom->ReverseWidth;
CopyTo->ReverseHeight = CopyFrom->ReverseHeight;
CopyTo->AlphaOnly = CopyFrom->AlphaOnly;
CopyTo->PixelBlend = CopyFrom->PixelBlend;
CopyTo->Quantize = CopyFrom->Quantize;
CopyTo->TileWidth = CopyFrom->TileWidth;
CopyTo->TileHeight = CopyFrom->TileHeight;
CopyTo->FlipWidth = CopyFrom->FlipWidth;
CopyTo->FlipHeight = CopyFrom->FlipHeight;
CopyTo->WrapWidth = CopyFrom->WrapWidth;
CopyTo->WrapHeight = CopyFrom->WrapHeight;
CopyTo->Misc = CopyFrom->Misc;
CopyTo->ThreeD = CopyFrom->ThreeD;
strcpy(CopyTo->MapName, CopyFrom->MapName);
RasterAnimHost::Copy(CopyTo, CopyFrom);

CopyTo->InitNoise();
CopyTo->InitBasis();

} // Texture::Copy

/*===========================================================================*/

void Texture::RepairApplyToColor(int ParentApplyColor)
{
GradientCritter *GradNode, *FirstNode, *LastNode;
ColorTextureThing *ColTex;
Texture *DelTex;

if (ApplyToColor && ! ParentApplyColor)
	{
	// move value textures to color
	if (Tex[WCS_TEXTURE_LOW])
		{
		if (GradNode = ColorGrad.AddNode(0.0, 0))
			{
			if (ColTex = (ColorTextureThing *)GradNode->GetThing())
				{
				if (ColTex->Tex)
					{
					DelTex = ColTex->Tex;
					ColTex->Tex = NULL;
					delete DelTex;
					} // if
				ColTex->Tex = Tex[WCS_TEXTURE_LOW];
				Tex[WCS_TEXTURE_LOW] = NULL;
				ColTex->Tex->ApplyToColor = 1;
				ColTex->Tex->RAParent = ColTex;
				ColTex->Tex->Prev = this;
				ColTex->Tex->RepairApplyToColor(ParentApplyColor);
				} // if
			} // if
		} // if
	if (Tex[WCS_TEXTURE_HIGH] && WCS_TEXTURETYPE_ISTWOVALUE(TexType))
		{
		if (GradNode = ColorGrad.AddNode(1.0, 0))
			{
			if (ColTex = (ColorTextureThing *)GradNode->GetThing())
				{
				if (ColTex->Tex)
					{
					DelTex = ColTex->Tex;
					ColTex->Tex = NULL;
					delete DelTex;
					} // if
				ColTex->Tex = Tex[WCS_TEXTURE_HIGH];
				Tex[WCS_TEXTURE_HIGH] = NULL;
				ColTex->Tex->ApplyToColor = 1;
				ColTex->Tex->RAParent = ColTex;
				ColTex->Tex->Prev = this;
				ColTex->Tex->RepairApplyToColor(ParentApplyColor);
				} // if
			} // if
		} // if
	} // if
else if (! ApplyToColor && ParentApplyColor)
	{
	// move color textures to value
	if (Tex[WCS_TEXTURE_LOW])
		{
		DelTex = Tex[WCS_TEXTURE_LOW];
		Tex[WCS_TEXTURE_LOW] = NULL;
		delete DelTex;
		} // if
	if (GradNode = ColorGrad.GetNextNode(NULL))
		{
		if (ColTex = (ColorTextureThing *)GradNode->GetThing())
			{
			if (ColTex->Tex)
				{
				Tex[WCS_TEXTURE_LOW] = ColTex->Tex;
				ColTex->Tex = NULL;
				Tex[WCS_TEXTURE_LOW]->ApplyToColor = 0;
				Tex[WCS_TEXTURE_LOW]->RAParent = this;
				Tex[WCS_TEXTURE_LOW]->Prev = this;
				Tex[WCS_TEXTURE_LOW]->RepairApplyToColor(ParentApplyColor);
				} // if
			} // if
		FirstNode = LastNode = GradNode;
		while (GradNode = ColorGrad.GetNextNode(GradNode))
			{
			LastNode = GradNode;
			} // if
		GradNode = FirstNode;
		while (GradNode = ColorGrad.GetNextNode(GradNode))
			{
			if (! (WCS_TEXTURETYPE_ISTWOVALUE(TexType)) || GradNode != LastNode)
				{
				if (ColTex = (ColorTextureThing *)GradNode->GetThing())
					{
					if (ColTex->Tex)
						{
						DelTex = ColTex->Tex;
						ColTex->Tex = NULL;
						delete DelTex;
						} // if
					} // if
				} // if
			} // if
		if (LastNode != GradNode && WCS_TEXTURETYPE_ISTWOVALUE(TexType))
			{
			if (ColTex = (ColorTextureThing *)LastNode->GetThing())
				{
				if (ColTex->Tex)
					{
					Tex[WCS_TEXTURE_HIGH] = ColTex->Tex;
					ColTex->Tex = NULL;
					Tex[WCS_TEXTURE_HIGH]->ApplyToColor = 0;
					Tex[WCS_TEXTURE_HIGH]->RAParent = this;
					Tex[WCS_TEXTURE_HIGH]->Prev = this;
					Tex[WCS_TEXTURE_HIGH]->RepairApplyToColor(ParentApplyColor);
					} // if
				} // if
			} // if
		} // if
	} // else if

} // Texture::RepairApplyToColor

/*===========================================================================*/

// this is called by DragnDropListGUI
Texture *Texture::NewTexture(long ParamNum, Texture *Proto, unsigned char NewTexType)
{
NotifyTag Changes[2];

// colors 1 & 2 are obsolete
if (ParamNum < WCS_TEXTURE_MAXPARAMTEXTURES && ParamNum != WCS_TEXTURE_COLOR1 && ParamNum != WCS_TEXTURE_COLOR2)
	{
	if (TextureAvailable(ParamNum))
		{
		if (Tex[ParamNum])
			delete Tex[ParamNum];
		Tex[ParamNum] = RootTexture::NewTexture(this, Proto, Proto ? Proto->GetTexType(): NewTexType, ParamNum);
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if

	return (Tex[ParamNum]);
	} // if

return (NULL);

} // Texture::NewTexture

/*===========================================================================*/

// static so it can be called from Textures
Texture *RootTexture::NewTexture(RasterAnimHost *RAHost, Texture *Proto, unsigned char NewTexType, long ParamNumber)
{
Texture *NewTex = NULL;

if (Proto)
	NewTexType = Proto->TexType;

switch (NewTexType)
	{
	case WCS_TEXTURE_TYPE_PLANARIMAGE:
		{
		NewTex = (Texture *)new PlanarImageTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_CYLINDRICALIMAGE:
		{
		NewTex = (Texture *)new CylindricalImageTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_SPHERICALIMAGE:
		{
		NewTex = (Texture *)new SphericalImageTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_CUBICIMAGE:
		{
		NewTex = (Texture *)new CubicImageTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE:
		{
		NewTex = (Texture *)new FrontProjectionTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE:
		{
		NewTex = (Texture *)new EnvironmentMapTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_UVW:
		{
		NewTex = (Texture *)new UVImageTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_COLORPERVERTEX:
		{
		NewTex = (Texture *)new ColorPerVertexTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_STRIPES:
		{
		NewTex = (Texture *)new StripeTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_SOFTSTRIPES:
		{
		NewTex = (Texture *)new SoftStripeTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_SINGLESTRIPE:
		{
		NewTex = (Texture *)new SingleStripeTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_SINGLESOFTSTRIPE:
		{
		NewTex = (Texture *)new SingleSoftStripeTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_WOODGRAIN:
		{
		NewTex = (Texture *)new WoodGrainTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_BRICK:
		{
		NewTex = (Texture *)new BrickTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_DOTS:
		{
		NewTex = (Texture *)new DotsTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_MARBLE:
		{
		NewTex = (Texture *)new MarbleTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_VEINS:
		{
		NewTex = (Texture *)new VeinTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_CRUST:
		{
		NewTex = (Texture *)new CrustTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_UNDERWATER:
		{
		NewTex = (Texture *)new UnderwaterTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_FRACTALNOISE:
		{
		NewTex = (Texture *)new FractalNoiseTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_MULTIFRACTALNOISE:
		{
		NewTex = (Texture *)new MultiFractalNoiseTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_HYBRIDMULTIFRACTALNOISE:
		{
		NewTex = (Texture *)new HybridMultiFractalNoiseTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_RIDGEDMULTIFRACTALNOISE:
		{
		NewTex = (Texture *)new RidgedMultiFractalNoiseTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_HETEROTERRAINNOISE:
		{
		NewTex = (Texture *)new HeteroTerrainNoiseTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_TURBULENCE:
		{
		NewTex = (Texture *)new TurbulenceTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_F1CELLBASIS:
		{
		NewTex = (Texture *)new F1CellBasisTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_F2CELLBASIS:
		{
		NewTex = (Texture *)new F2CellBasisTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_F2MF1CELLBASIS:
		{
		NewTex = (Texture *)new F2mF1CellBasisTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_F3MF1CELLBASIS:
		{
		NewTex = (Texture *)new F3mF1CellBasisTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_F1MANHATTAN:
		{
		NewTex = (Texture *)new F1ManhattanTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_F2MANHATTAN:
		{
		NewTex = (Texture *)new F2ManhattanTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_F2MF1MANHATTAN:
		{
		NewTex = (Texture *)new F2mF1ManhattanTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_BIRDSHOT:
		{
		NewTex = (Texture *)new BirdshotTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_ADD:
		{
		NewTex = (Texture *)new AddTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_COMPOSITE:
		{
		NewTex = (Texture *)new CompositeTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_CONTRAST:
		{
		NewTex = (Texture *)new ContrastTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_DARKEN:
		{
		NewTex = (Texture *)new DarkenTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_LIGHTEN:
		{
		NewTex = (Texture *)new LightenTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_LEVELS:
		{
		NewTex = (Texture *)new LevelsTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_SKEW:
		{
		NewTex = (Texture *)new SkewTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_BELLCURVE:
		{
		NewTex = (Texture *)new BellCurveTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_SQUAREWAVE:
		{
		NewTex = (Texture *)new SquareWaveTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_SAWTOOTH:
		{
		NewTex = (Texture *)new SawtoothTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_STEP:
		{
		NewTex = (Texture *)new StepTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_SLOPE:
		{
		NewTex = (Texture *)new SlopeTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_GAMMA:
		{
		NewTex = (Texture *)new GammaTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_BIAS:
		{
		NewTex = (Texture *)new BiasTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_GAIN:
		{
		NewTex = (Texture *)new GainTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_CUSTOMCURVE:
		{
		NewTex = (Texture *)new CustomCurveTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_MAXIMUM:
		{
		NewTex = (Texture *)new MaximumTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_MAXIMUMSWITCH:
		{
		NewTex = (Texture *)new MaximumSwitchTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_MINIMUM:
		{
		NewTex = (Texture *)new MinimumTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_MINIMUMSWITCH:
		{
		NewTex = (Texture *)new MinimumSwitchTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_MULTIPLY:
		{
		NewTex = (Texture *)new MultiplyTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_SUBTRACT:
		{
		NewTex = (Texture *)new SubtractTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_THRESHOLD:
		{
		NewTex = (Texture *)new ThresholdTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_TERRAINPARAM:
		{
		NewTex = (Texture *)new TerrainParameterTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_GRADIENT:
		{
		NewTex = (Texture *)new GradientTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_INCLUDEEXCLUDE:
		{
		NewTex = (Texture *)new IncludeExcludeTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_INCLUDEEXCLUDETYPE:
		{
		NewTex = (Texture *)new IncludeExcludeTypeTexture(RAHost, ParamNumber);
		break;
		} // 
	case WCS_TEXTURE_TYPE_HSVMERGE:
		{
		NewTex = (Texture *)new HSVMergeTexture(RAHost, ParamNumber);
		break;
		} // WCS_TEXTURE_TYPE_HSVMERGE
	} // switch
// <<<>>> ADD_NEW_TEXTURE

return (NewTex);

} // RootTexture::NewTexture

/*===========================================================================*/

void Texture::PropagateCoordSpace(unsigned char SpaceNumber)
{
int Ct;
GradientCritter *CurGrad = NULL;

CoordSpace = SpaceNumber;

while (CurGrad = ColorGrad.GetNextNode(CurGrad))
	{
	if (CurGrad->GetThing() && ((ColorTextureThing *)CurGrad->GetThing())->Tex)
		((ColorTextureThing *)CurGrad->GetThing())->Tex->PropagateCoordSpace(SpaceNumber);
	} // while

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	if (Tex[Ct])
		Tex[Ct]->PropagateCoordSpace(SpaceNumber);
	} // for

} // Texture::PropagateCoordSpace

/*===========================================================================*/

void Texture::SetCoordSpace(unsigned char NewSpace, int SendNotify)
{
NotifyTag Changes[2];
char **VertMapNames = NULL;
int MapType, Ct;

if (ApproveCoordSpace(NewSpace))
	{
	CoordSpace = NewSpace;
	if (CoordSpace == WCS_TEXTURE_COORDSPACE_VERTEX_UVW || CoordSpace == WCS_TEXTURE_COORDSPACE_VERTEX_COLORPERVERTEX)
		{
		MapType = CoordSpace == WCS_TEXTURE_COORDSPACE_VERTEX_UVW ? 0: 1;
		if (VertMapNames = GetVertexMapNames(MapType))
			{
			SetVertexMap(VertMapNames[0]);
			for (Ct = 0; VertMapNames[Ct]; Ct ++)
				{
				} // for
			AppMem_Free(VertMapNames, (Ct + 1) * sizeof (char *));
			} // if
		} // if
	if (SendNotify)
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if

} // Texture::SetCoordSpace

/*===========================================================================*/

long Texture::FindTextureOwnerType(void)
{
int RootFound = 0;
RasterAnimHost *Owner;
RasterAnimHostProperties OwnerProp;

Owner = this;
while (Owner)
	{
	Owner->GetRAHostProperties(&OwnerProp);
	if (RootFound)
		return (OwnerProp.TypeNumber);
	if (OwnerProp.TypeNumber == WCS_RAHOST_OBJTYPE_ROOTTEXTURE)
		RootFound = 1;
	Owner = Owner->RAParent;
	} // while

return (0);

} // Texture::FindTextureOwnerType

/*===========================================================================*/

RootTexture *Texture::GetRoot(void)
{
RasterAnimHost *Owner;
RasterAnimHostProperties OwnerProp;

Owner = this;
while (Owner)
	{
	Owner->GetRAHostProperties(&OwnerProp);
	if (OwnerProp.TypeNumber == WCS_RAHOST_OBJTYPE_ROOTTEXTURE)
		return ((RootTexture *)Owner);
	Owner = Owner->RAParent;
	} // while

return (NULL);

} // Texture::GetRoot

/*===========================================================================*/

int Texture::ApproveCoordSpace(unsigned char TestSpace)
{
long OwnerType;
RasterAnimHost *Root;
RootTexture *TexRoot;
RasterAnimHostProperties Prop;

Root = GetRAHostRoot();
Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
Root->GetRAHostProperties(&Prop);

OwnerType = FindTextureOwnerType();
TexRoot = GetRoot();

switch (TestSpace)
	{
	case WCS_TEXTURE_COORDSPACE_NONE:
		{
		if (! WCS_TEXTURETYPE_ISMATHEMATICAL(TexType)
			&& TexType != WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE && TexType != WCS_TEXTURE_TYPE_INCLUDEEXCLUDE
			&& TexType != WCS_TEXTURE_TYPE_INCLUDEEXCLUDETYPE)
			return (0);
		break;
		} // WCS_TEXTURE_COORDSPACE_NONE
	case WCS_TEXTURE_COORDSPACE_OBJECT_CARTESIAN:
		{
		if (TexType == WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE || TexType == WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE
			|| TexType == WCS_TEXTURE_TYPE_UVW || TexType == WCS_TEXTURE_TYPE_COLORPERVERTEX)
			return (0);
		if (TexRoot && Prop.TypeNumber == WCS_EFFECTSSUBCLASS_OBJECT3D && ((Object3DEffect *)Root)->ObjectCartesianLegal(TexRoot))
			return (1);
		return (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_MATERIAL || Prop.TypeNumber == WCS_EFFECTSSUBCLASS_GENERATOR);
		} // WCS_TEXTURE_COORDSPACE_OBJECT_CARTESIAN
	case WCS_TEXTURE_COORDSPACE_GLOBAL_CARTESIAN:
		{
		if (TexType == WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE || TexType == WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE
			|| TexType == WCS_TEXTURE_TYPE_UVW || TexType == WCS_TEXTURE_TYPE_COLORPERVERTEX
			|| Prop.TypeNumber == WCS_EFFECTSSUBCLASS_GENERATOR
			|| Prop.TypeNumber == WCS_EFFECTSSUBCLASS_TERRAFFECTOR
			|| Prop.TypeNumber == WCS_EFFECTSSUBCLASS_RASTERTA
			|| Prop.TypeNumber == WCS_EFFECTSSUBCLASS_TERRAINPARAM)
			return (0);
		break;
		} // WCS_TEXTURE_COORDSPACE_GLOBAL_CARTESIAN
	case WCS_TEXTURE_COORDSPACE_PROJECT_GEOGRXYZ:
		{
		if (TexType == WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE || TexType == WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE
			|| TexType == WCS_TEXTURE_TYPE_UVW || TexType == WCS_TEXTURE_TYPE_COLORPERVERTEX 
			|| Prop.TypeNumber == WCS_EFFECTSSUBCLASS_GENERATOR)
			return (0);
		break;
		} // WCS_TEXTURE_COORDSPACE_PROJECT_GEOGRXYZ
	case WCS_TEXTURE_COORDSPACE_VECTOR_ALIGNED:
		{
		if (TexType == WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE || TexType == WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE
			|| TexType == WCS_TEXTURE_TYPE_UVW || TexType == WCS_TEXTURE_TYPE_COLORPERVERTEX
			|| Prop.TypeNumber == WCS_EFFECTSSUBCLASS_POSTPROC || Prop.TypeNumber == WCS_EFFECTSSUBCLASS_GENERATOR
			|| Prop.TypeNumber == WCS_EFFECTSSUBCLASS_RASTERTA
			|| Prop.TypeNumber == WCS_EFFECTSSUBCLASS_TERRAINPARAM
			|| Prop.TypeNumber == WCS_EFFECTSSUBCLASS_CLOUD)
			return (0);
		return (! WCS_TEXTURETYPE_ALIGNTOVECTORILLEGAL(TexType));
		} // WCS_TEXTURE_COORDSPACE_VECTOR_ALIGNED
	case WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED:
		{
		if (TexType == WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE || TexType == WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE
			|| TexType == WCS_TEXTURE_TYPE_UVW || TexType == WCS_TEXTURE_TYPE_COLORPERVERTEX)
			return (0);
		return (! WCS_TEXTURETYPE_LATLONBNDSILLEGAL(TexType));
		} // WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED
	case WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE_NOZ:
		{
		if (TexType == WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE
			|| TexType == WCS_TEXTURE_TYPE_UVW || TexType == WCS_TEXTURE_TYPE_COLORPERVERTEX
			|| TexType == WCS_TEXTURE_TYPE_CYLINDRICALIMAGE || TexType == WCS_TEXTURE_TYPE_SPHERICALIMAGE
			|| Prop.TypeNumber == WCS_EFFECTSSUBCLASS_GENERATOR
			|| Prop.TypeNumber == WCS_EFFECTSSUBCLASS_TERRAFFECTOR
			|| Prop.TypeNumber == WCS_EFFECTSSUBCLASS_RASTERTA
			|| Prop.TypeNumber == WCS_EFFECTSSUBCLASS_TERRAINPARAM
			|| OwnerType == WCS_RAHOST_OBJTYPE_ECOTYPE || OwnerType == WCS_RAHOST_OBJTYPE_FOLIAGEGROUP || OwnerType == WCS_RAHOST_OBJTYPE_FOLIAGE
			|| OwnerType == WCS_EFFECTSSUBCLASS_ECOSYSTEM || OwnerType == WCS_EFFECTSSUBCLASS_SNOW || OwnerType == WCS_EFFECTSSUBCLASS_GROUND
			|| OwnerType == WCS_EFFECTSSUBCLASS_LAKE || OwnerType == WCS_EFFECTSSUBCLASS_STREAM || OwnerType == WCS_EFFECTSSUBCLASS_WAVE
			|| OwnerType == WCS_EFFECTSSUBCLASS_ENVIRONMENT || OwnerType == WCS_EFFECTSSUBCLASS_OBJECT3D || OwnerType == WCS_EFFECTSSUBCLASS_SHADOW
			|| OwnerType == WCS_RAHOST_OBJTYPE_WAVESOURCE)
			return (0);
		break;
		} // WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE_NOZ
	case WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE:
	case WCS_TEXTURE_COORDSPACE_IMAGE_PIXELUNITS:
	case WCS_TEXTURE_COORDSPACE_IMAGE_PIXELUNITS_NOZ:
		{
		if (TexType == WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE || TexType == WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE
			|| TexType == WCS_TEXTURE_TYPE_UVW || TexType == WCS_TEXTURE_TYPE_COLORPERVERTEX
			|| TexType == WCS_TEXTURE_TYPE_CYLINDRICALIMAGE || TexType == WCS_TEXTURE_TYPE_SPHERICALIMAGE
			|| Prop.TypeNumber == WCS_EFFECTSSUBCLASS_GENERATOR
			|| Prop.TypeNumber == WCS_EFFECTSSUBCLASS_TERRAFFECTOR
			|| Prop.TypeNumber == WCS_EFFECTSSUBCLASS_RASTERTA
			|| Prop.TypeNumber == WCS_EFFECTSSUBCLASS_TERRAINPARAM
			|| OwnerType == WCS_RAHOST_OBJTYPE_ECOTYPE || OwnerType == WCS_RAHOST_OBJTYPE_FOLIAGEGROUP || OwnerType == WCS_RAHOST_OBJTYPE_FOLIAGE
			|| OwnerType == WCS_EFFECTSSUBCLASS_ECOSYSTEM || OwnerType == WCS_EFFECTSSUBCLASS_SNOW || OwnerType == WCS_EFFECTSSUBCLASS_GROUND
			|| OwnerType == WCS_EFFECTSSUBCLASS_LAKE || OwnerType == WCS_EFFECTSSUBCLASS_STREAM || OwnerType == WCS_EFFECTSSUBCLASS_WAVE
			|| OwnerType == WCS_EFFECTSSUBCLASS_ENVIRONMENT || OwnerType == WCS_EFFECTSSUBCLASS_OBJECT3D || OwnerType == WCS_EFFECTSSUBCLASS_SHADOW
			|| OwnerType == WCS_RAHOST_OBJTYPE_WAVESOURCE)
			return (0);
		break;
		} // WCS_TEXTURE_COORDSPACE_IMAGE_PIXELUNITS_NOZ
	case WCS_TEXTURE_COORDSPACE_VERTEX_UVW:
		{
		if (TexType == WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE || TexType == WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE
			|| TexType == WCS_TEXTURE_TYPE_COLORPERVERTEX)
			return (0);
		return (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_MATERIAL && ((MaterialEffect *)Root)->VertexUVWAvailable());
		} // WCS_TEXTURE_COORDSPACE_VERTEX_UVW
	case WCS_TEXTURE_COORDSPACE_VERTEX_COLORPERVERTEX:
		{
		if (TexType == WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE || TexType == WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE
			|| TexType == WCS_TEXTURE_TYPE_UVW)
			return (0);
		return (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_MATERIAL && ((MaterialEffect *)Root)->VertexColorsAvailable()
			&& TexType == WCS_TEXTURE_TYPE_COLORPERVERTEX);
		} // WCS_TEXTURE_COORDSPACE_VERTEX_COLORPERVERTEX
	} // switch
// <<<>>> ADD_NEW_TEXTURE - is the texture space legal for the texture type

return (1);

} // Texture::ApproveCoordSpace

/*===========================================================================*/

Texture *Texture::MatchRasterShell(RasterShell *Shell)
{
Texture *Test;
long Ct;

if (Img && Img == Shell)
	return (this);

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	if (Tex[Ct] && (Test = Tex[Ct]->MatchRasterShell(Shell)))
		{
		return (Test);
		} // if
	} // for

return (NULL);

} // Texture::MatchRasterShell

/*===========================================================================*/

char *Texture::OKRemoveRaster(void)
{

return ("Image Object is used as a Texture! Remove anyway?");

} // Texture::OKRemoveRaster

/*===========================================================================*/

void Texture::RemoveRaster(RasterShell *Shell)
{
long Ct;
NotifyTag Changes[2];

Rast = NULL;
Coords = NULL;

if (Img && Img == Shell)
	{
	Img = NULL;
	if (CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
		CoordSpace = GetDefaultCoordSpace(TexType);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	return;
	} // if

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	if (Tex[Ct])
		{
		Tex[Ct]->RemoveRaster(Shell);
		} // if
	} // for

} // Texture::RemoveRaster

/*===========================================================================*/

void Texture::SetRaster(Raster *NewRast)
{
NotifyTag Changes[2];

Rast = NULL;
Coords = NULL;

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
	} // if
else
	{
	delete Img;		// delete it here since it wasn't added to a raster
	Img = NULL;
	} // else

} // Texture::SetRaster

/*===========================================================================*/

char *Texture::GetTextureName(long TexNumber)
{

if (TexNumber < WCS_TEXTURE_MAXPARAMTEXTURES)
	{
	if (TexNumber != WCS_TEXTURE_COLOR1 && TexNumber != WCS_TEXTURE_COLOR2)
		{
		if (! ApplyToColor || (TexNumber != WCS_TEXTURE_LOW && TexNumber != WCS_TEXTURE_HIGH))
			{
			return (ParamName[TexNumber]);
			} // if
		} // if
	} // if

return ("");

} // Texture::GetTextureName

/*===========================================================================*/

void Texture::SetVertexMap(char *NewMap)
{
NotifyTag Changes[2];

strncpy(MapName, NewMap, WCS_UVMAP_MAXNAMELENGTH);
MapName[WCS_UVMAP_MAXNAMELENGTH - 1] = 0;
Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

} // Texture::SetVertexMap

/*===========================================================================*/

char **Texture::GetVertexMapNames(int MapType)
{
RasterAnimHostProperties Prop;
RasterAnimHost *Root;

Root = GetRAHostRoot();
Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
Root->GetRAHostProperties(&Prop);
if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_MATERIAL)
	{
	return (((MaterialEffect *)Root)->GetVertexMapNames(MapType));
	} // if

return (NULL);

} // Texture::GetNextVertexMapName

/*===========================================================================*/

int Texture::ApproveInclExclClass(long MyClass)
{

if (MyClass == WCS_EFFECTSSUBCLASS_OBJECT3D ||
	MyClass == WCS_EFFECTSSUBCLASS_LAKE ||
	MyClass == WCS_EFFECTSSUBCLASS_ECOSYSTEM ||
	MyClass == WCS_EFFECTSSUBCLASS_CLOUD ||
	MyClass == WCS_EFFECTSSUBCLASS_CMAP ||
	MyClass == WCS_EFFECTSSUBCLASS_FOLIAGE ||
	MyClass == WCS_EFFECTSSUBCLASS_STREAM ||
	//MyClass == WCS_EFFECTSSUBCLASS_MATERIAL ||
	MyClass == WCS_EFFECTSSUBCLASS_CELESTIAL ||
	MyClass == WCS_EFFECTSSUBCLASS_STARFIELD ||
	MyClass == WCS_EFFECTSSUBCLASS_GROUND ||
	//MyClass == WCS_EFFECTSSUBCLASS_SNOW ||
	MyClass == WCS_EFFECTSSUBCLASS_SKY ||
	//MyClass == WCS_EFFECTSSUBCLASS_ATMOSPHERE ||
	MyClass == WCS_EFFECTSSUBCLASS_FENCE ||
	MyClass == WCS_EFFECTSSUBCLASS_LABEL)
	return (1);

return (0);

} // Texture::ApproveInclExclClass

/*===========================================================================*/

int Texture::RemoveRAHost(RasterAnimHost *RemoveMe)
{
char Ct;
NotifyTag Changes[2];
int Removed = 0;

Rast = NULL;
Coords = NULL;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	if (RemoveMe == Tex[Ct])
		{
		delete Tex[Ct];
		Tex[Ct] = NULL;
		Removed = 1;
		} // if
	} // for

if (Removed)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

return (Removed);

} // Texture::RemoveRAHost

/*===========================================================================*/

char Texture::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_TEXTURE
	|| DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORGRADIENT
	|| (ApplyToColor && (DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME
	|| DropType == WCS_RAHOST_OBJTYPE_COLORTEXTURE))
	|| DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	return (1);
return (0);

} // Texture::GetRAHostDropOK

/*===========================================================================*/

int Texture::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char WinNum, QueryStr[256], NameStr[128];
int QueryResult, Success = 0;
long Ct, ApprovedCt, NumListItems = 0;
RasterAnimHostProperties Prop;
RasterAnimHost *TargetList[31];
long TargetListItems[31];
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (Texture *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s or add as a child texture?", DropSource->Name, DropSource->Type, NameStr);
		if (QueryResult = UserMessageCustom(NameStr, QueryStr, "Copy", "Cancel", "Add", 1))
			{
			if (QueryResult == 1)
				{
				if (TexType == ((Texture *)DropSource->DropSource)->GetTexType())
					{
					Copy(this, (Texture *)DropSource->DropSource);
					// re-configure texture
					Success = 1;
					Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
					} // if
				else
					{
					UserMessageOK(NameStr, "Source texture component is of a different type than\n the target and cannot be copied over the target.");
					Success = 1;
					} // else
				} // if
			else
				{
				for (Ct = ApprovedCt = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES && Ct < 30; Ct ++)
					{
					if (TextureAvailable(Ct))
						{
						if (Ct != WCS_TEXTURE_COLOR1 && Ct != WCS_TEXTURE_COLOR2)
							{
							if (! ApplyToColor || (Ct != WCS_TEXTURE_LOW && Ct != WCS_TEXTURE_HIGH))
								{
								TargetListItems[ApprovedCt] = Ct;
								TargetList[ApprovedCt ++] = Tex[Ct];
								} // if
							} // if
						} // if
					} // for
				TargetList[ApprovedCt ++] = &ColorGrad;
				NumListItems = ApprovedCt;
				} // else if
			} // if
		else
			Success = 0;
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORGRADIENT
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_COLORTEXTURE)
	{
	Success = ColorGrad.ProcessRAHostDragDrop(DropSource);
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	{
	Success = -1;
	for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
		{
		if (ParamAvailable(Ct) && (! ApplyToColor || (Ct != WCS_TEXTURE_LOW && Ct != WCS_TEXTURE_HIGH)))
			TargetList[Ct] = &TexParam[Ct];
		} // for
	NumListItems = Ct;
	} // if

if (NumListItems > 0)
	{
	if ((WinNum = GlobalApp->MCP->GetAvailableDragnDropListNumber()) >= 0)
		{
		GlobalApp->GUIWins->DDL[WinNum] = new DragnDropListGUI(WinNum, DropSource, TargetList, NumListItems, NULL, this, TargetListItems);
		if(GlobalApp->GUIWins->DDL[WinNum])
			{
			GlobalApp->GUIWins->DDL[WinNum]->Open(GlobalApp->MainProj);
			}
		} // if
	else
		UserMessageOK("Open Drag 'n Drop List", "All List windows are in use.\nTry dropping on a target deeper in the hierarchy\n and closer to your desired target.");
	} // if

return (Success);

} // Texture::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long Texture::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_TEXTURE | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // Texture::GetRAFlags

/*===========================================================================*/

void Texture::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
//	sprintf(TextureTypeString, "%s", RootTexture::UserNames[TexType]);
	Prop->Name = RootTexture::UserNames[TexType];
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPE)
	{
	Prop->Type = "(Texture Component)";
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
	Prop->Path = "Texture";
	Prop->Ext = "tex";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_POPCLASS)
	{
	GetPopClassFlags(Prop);
	} // get pop-up menu classes

} // Texture::GetRAHostProperties

/*===========================================================================*/

int Texture::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_ENABLED)
		{
		Enabled = (Prop->Flags & WCS_RAHOST_FLAGBIT_ENABLED) ? 1: 0;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
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
	return (-2);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_SAVEFILE)
	{
	return (-2);
	} // if

return (Success);

} // Texture::SetRAHostProperties

/*===========================================================================*/

int Texture::BuildFileComponentsList(EffectList **Coords)
{
#ifdef WCS_BUILD_VNS
EffectList **ListPtr;
RasterAttribute *MyAttr;

if (Img && Img->GetRaster())
	{
	if ((MyAttr = Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
		{
		if (((GeoRefShell *)MyAttr->GetShell())->Host)
			{
			ListPtr = Coords;
			while (*ListPtr)
				{
				if ((*ListPtr)->Me == ((GeoRefShell *)MyAttr->GetShell())->Host)
					break;
				ListPtr = &(*ListPtr)->Next;
				} // if
			if (! (*ListPtr))
				{
				if (*ListPtr = new EffectList())
					(*ListPtr)->Me = (CoordSys *)((GeoRefShell *)MyAttr->GetShell())->Host;
				else
					return (0);
				} // if
			} // if
		}  // if
	} // if
#endif // WCS_BUILD_VNS

return (1);

} // Texture::BuildFileComponentsList

/*===========================================================================*/

int Texture::GetAbsoluteOutputRange(long ParamNumber, double RangeDefaults[3], double &CurrentValue, double &Multiplier, unsigned char &MetricType)
{

if (WCS_TEXTURE_SIZE(ParamNumber))
	{
	TexSize[0].GetRangeDefaults(RangeDefaults);
	MetricType = TexSize[0].GetMetricType();
	Multiplier = TexSize[0].GetMultiplier();
	CurrentValue = (ParamNumber == WCS_TEXTURE_SIZE1) ? TexSize[0].CurValue: (ParamNumber == WCS_TEXTURE_SIZE2) ? TexSize[1].CurValue: TexSize[2].CurValue;
	return (1);
	} // if
else if (WCS_TEXTURE_CENTER(ParamNumber))
	{
	TexCenter[0].GetRangeDefaults(RangeDefaults);
	MetricType = TexCenter[0].GetMetricType();
	Multiplier = TexCenter[0].GetMultiplier();
	CurrentValue = (ParamNumber == WCS_TEXTURE_SIZE1) ? TexCenter[0].CurValue: (ParamNumber == WCS_TEXTURE_SIZE2) ? TexCenter[1].CurValue: TexCenter[2].CurValue;
	return (1);
	} // if
else if (WCS_TEXTURE_FALLOFF(ParamNumber))
	{
	TexFalloff[0].GetRangeDefaults(RangeDefaults);
	MetricType = TexFalloff[0].GetMetricType();
	Multiplier = TexFalloff[0].GetMultiplier();
	CurrentValue = (ParamNumber == WCS_TEXTURE_SIZE1) ? TexFalloff[0].CurValue: (ParamNumber == WCS_TEXTURE_SIZE2) ? TexFalloff[1].CurValue: TexFalloff[2].CurValue;
	return (1);
	} // if
else if (WCS_TEXTURE_ROTATION(ParamNumber))
	{
	TexRotation[0].GetRangeDefaults(RangeDefaults);
	MetricType = TexRotation[0].GetMetricType();
	Multiplier = TexRotation[0].GetMultiplier();
	CurrentValue = (ParamNumber == WCS_TEXTURE_SIZE1) ? TexRotation[0].CurValue: (ParamNumber == WCS_TEXTURE_SIZE2) ? TexRotation[1].CurValue: TexRotation[2].CurValue;
	return (1);
	} // if

return (0);

} // Texture::GetAbsoluteOutputRange

/*===========================================================================*/

void Texture::SetupForInputParamTexture(long ParamNumber)
{
double CurrentValue, TempValue, Multiplier, RangeDefaults[3];
unsigned char MetricType;
RasterAnimHostProperties Prop;

if (RAParent)
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
	RAParent->GetRAHostProperties(&Prop);
	if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
		{
		if (((Texture *)RAParent)->GetAbsoluteOutputRange(ParamNumber, RangeDefaults, CurrentValue, Multiplier, MetricType))
			{
			ParamFlags[WCS_TEXTURE_LOW] &= ~WCS_TEXTURE_PARAMPERCENT;
			ParamFlags[WCS_TEXTURE_HIGH] &= ~WCS_TEXTURE_PARAMPERCENT;
			TexParam[WCS_TEXTURE_LOW].SetRangeDefaults(RangeDefaults);
			TexParam[WCS_TEXTURE_LOW].SetMultiplier(Multiplier);
			TexParam[WCS_TEXTURE_LOW].SetMetricType(MetricType);
			TempValue = CurrentValue == 0.0 ? -1.0: WCS_floor(CurrentValue - .1 * CurrentValue);
			TexParam[WCS_TEXTURE_LOW].SetValue(TempValue);
			TexParam[WCS_TEXTURE_HIGH].SetRangeDefaults(RangeDefaults);
			TexParam[WCS_TEXTURE_HIGH].SetMultiplier(Multiplier);
			TexParam[WCS_TEXTURE_HIGH].SetMetricType(MetricType);
			TempValue = CurrentValue == 0.0 ? 1.0: WCS_ceil(CurrentValue + .1 * CurrentValue);
			TexParam[WCS_TEXTURE_HIGH].SetValue(TempValue);
			AbsoluteOutput = 1;
			} // if
		} // if
	} // if

} // Texture::SetupForInputParamTexture

/*===========================================================================*/

int Texture::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil)
{
long Ct;

AnimAffil = NULL;

if (ChildA)
	{
	for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
		{
		if (ChildA == &TexParam[Ct])
			{
			AnimAffil = (AnimCritter *)ChildA;
			return (1);
			} // if
		} // for
	for (Ct = 0; Ct < 3; Ct ++)
		{
		if (ChildA == &TexCenter[Ct] || ChildA == &TexFalloff[Ct] || ChildA == &TexVelocity[Ct] || ChildA == &TexRotation[Ct] || ChildA == &TexSize[Ct])
			{
			AnimAffil = (AnimCritter *)ChildA;
			return (1);
			} // if
		} // for
	if (ChildA == &TexFeather)
		{
		AnimAffil = (AnimCritter *)ChildA;
		return (1);
		} // if
	} // if

return (0);

} // Texture::GetAffiliates

/*===========================================================================*/

int Texture::GetPopClassFlags(RasterAnimHostProperties *Prop)
{
AnimCritter *AnimAffil = NULL;

Prop->PopClassFlags = 0;
Prop->PopExistsFlags = 0;
Prop->PopEnabledFlags = 0;

if (GetAffiliates(Prop->ChildA, Prop->ChildB, AnimAffil))
	{
	return (RasterAnimHost::GetPopClassFlags(Prop, AnimAffil, NULL, NULL));
	} // if

return (0);

} // Texture::GetPopClassFlags

/*===========================================================================*/

int Texture::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil))
	{
	return (RasterAnimHost::AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, NULL, NULL));
	} // if

return(0);

} // Texture::AddSRAHBasePopMenus

/*===========================================================================*/

int Texture::HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil))
	{
	return (RasterAnimHost::HandleSRAHPopMenuSelection(Action, AnimAffil, NULL, NULL, NULL, NULL));
	} // if

return(0);

} // Texture::HandleSRAHPopMenuSelection

/*===========================================================================*/

RasterAnimHost *Texture::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;

if (! Current)
	{
	if (ApplyToColor)
		return (&ColorGrad);
	else
		Found = 1;
	} // if
if (Current == &ColorGrad)
	Found = 1;
for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	if (ApplyToColor && (Ct == WCS_TEXTURE_LOW || Ct == WCS_TEXTURE_HIGH))
		continue;
	if (Found && ParamAvailable(Ct))
		return (&TexParam[Ct]);
	if (Current == &TexParam[Ct])
		Found = 1;
	} // for
if (! ApplyToEcosys || (CoordSpace != WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED && TexType != WCS_TEXTURE_TYPE_TERRAINPARAM))
	{
	for (Ct = 0; Ct < 3; Ct ++)
		{
		if (Found)
			return (&TexSize[Ct]);
		if (Current == &TexSize[Ct])
			Found = 1;
		} // for
	for (Ct = 0; Ct < 3; Ct ++)
		{
		if (Found)
			return (&TexCenter[Ct]);
		if (Current == &TexCenter[Ct])
			Found = 1;
		} // for
	for (Ct = 0; Ct < 3; Ct ++)
		{
		if (Found)
			return (&TexFalloff[Ct]);
		if (Current == &TexFalloff[Ct])
			Found = 1;
		} // for
	for (Ct = 0; Ct < 3; Ct ++)
		{
		if (Found)
			return (&TexVelocity[Ct]);
		if (Current == &TexVelocity[Ct])
			Found = 1;
		} // for
	for (Ct = 0; Ct < 3; Ct ++)
		{
		if (Found)
			return (&TexRotation[Ct]);
		if (Current == &TexRotation[Ct])
			Found = 1;
		} // for
	} // if
else if (CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
	{
	if (Found)
		return (&TexFeather);
	if (Current == &TexFeather)
		Found = 1;
	} // if
for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	if (Found && Tex[Ct])
		return (Tex[Ct]);
	if (Current == Tex[Ct])
		Found = 1;
	} // for
if (Found && InclExcl)
	return (InclExcl);
if (InclExcl && Current == InclExcl)
	Found = 1;
if (Found && InclExclType)
	return (InclExclType);
if (InclExclType && Current == InclExclType)
	Found = 1;
if (Found && Img && Img->GetRaster())
	return (Img->GetRaster());

return (NULL);

} // Texture::GetRAHostChild

/*===========================================================================*/

RasterAnimHost *Texture::GetNextGroupSibling(RasterAnimHost *FindMyBrother)
{

if (FindMyBrother == &TexSize[0])
	return (&TexSize[1]);
if (FindMyBrother == &TexSize[1])
	return (&TexSize[2]);
if (FindMyBrother == &TexSize[2])
	return (&TexSize[0]);

if (FindMyBrother == &TexCenter[0])
	return (&TexCenter[1]);
if (FindMyBrother == &TexCenter[1])
	return (&TexCenter[2]);
if (FindMyBrother == &TexCenter[2])
	return (&TexCenter[0]);

if (FindMyBrother == &TexFalloff[0])
	return (&TexFalloff[1]);
if (FindMyBrother == &TexFalloff[1])
	return (&TexFalloff[2]);
if (FindMyBrother == &TexFalloff[2])
	return (&TexFalloff[0]);

if (FindMyBrother == &TexVelocity[0])
	return (&TexVelocity[1]);
if (FindMyBrother == &TexVelocity[1])
	return (&TexVelocity[2]);
if (FindMyBrother == &TexVelocity[2])
	return (&TexVelocity[0]);

if (FindMyBrother == &TexRotation[0])
	return (&TexRotation[1]);
if (FindMyBrother == &TexRotation[1])
	return (&TexRotation[2]);
if (FindMyBrother == &TexRotation[2])
	return (&TexRotation[0]);

return (NULL);

} // Texture::GetNextGroupSibling

/*===========================================================================*/

int Texture::GetDeletable(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	if (Test == Tex[Ct])
		return (1);
	} // for

return (0);

} // Texture::GetDeletable

/*===========================================================================*/

long Texture::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Ct, Found = 0;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	if (ApplyToColor && (Ct == WCS_TEXTURE_LOW || Ct == WCS_TEXTURE_HIGH))
		continue;
	if (ParamAvailable(Ct) && TexParam[Ct].GetMinMaxDist(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // for
for (Ct = 0; Ct < 3; Ct ++)
	{
	if (TexSize[Ct].GetMinMaxDist(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	if (TexCenter[Ct].GetMinMaxDist(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	if (TexFalloff[Ct].GetMinMaxDist(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	if (TexVelocity[Ct].GetMinMaxDist(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	if (TexRotation[Ct].GetMinMaxDist(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // for
if (TexFeather.GetMinMaxDist(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	if (Ct == WCS_TEXTURE_COLOR1 || Ct == WCS_TEXTURE_COLOR2)
		continue;
	if (Tex[Ct] && Tex[Ct]->GetKeyFrameRange(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // while

if (ApplyToColor && ColorGrad.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if

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

} // Texture::GetKeyFrameRange

/*===========================================================================*/

char *Texture::GetCritterName(RasterAnimHost *TestMe)
{
long Ct;

if (TestMe == &ColorGrad)
	return ("");
for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	if (TestMe == &TexParam[Ct])
		{
		return (ParamName[Ct]);
		} // if
	} // for
for (Ct = 0; Ct < 3; Ct ++)
	{
	if (TestMe == &TexSize[Ct])
		{
		return (Ct == 0 ? (char*)"X Size (m)": Ct == 1 ? (char*)"Y Size (m)": (char*)"Z Size (m)");
		} // if
	if (TestMe == &TexCenter[Ct])
		{
		return (Ct == 0 ? (char*)"X Center (m)": Ct == 1 ? (char*)"Y Center (m)": (char*)"Z Center (m)");
		} // if
	if (TestMe == &TexFalloff[Ct])
		{
		return (Ct == 0 ? (char*)"X Falloff (%/m)": Ct == 1 ? (char*)"Y Falloff (%/m)": (char*)"Z Falloff (%/m)");
		} // if
	if (TestMe == &TexVelocity[Ct])
		{
		return (Ct == 0 ? (char*)"X Velocity (m/sec)": Ct == 1 ? (char*)"Y Velocity (m/sec)": (char*)"Z Velocity (m/sec)");
		} // if
	if (TestMe == &TexRotation[Ct])
		{
		return (Ct == 0 ? (char*)"X Rotation (deg)": Ct == 1 ? (char*)"Y Rotation (deg)": (char*)"Z Rotation (deg)");
		} // if
	} // for
if (TestMe == &TexFeather)
	{
	return ("Feathering");
	} // if

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	if (Tex[Ct] && TestMe == Tex[Ct])
		{
		return (ParamName[Ct]);
		} // if
	} // while

return ("");

} // Texture::GetCritterName

/*===========================================================================*/

int Texture::SetToTime(double Time)
{
long Ct, Found = 0, initNoise = 0, initBasis = 0;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	if (ParamAvailable(Ct))
		{
		if (TexParam[Ct].SetToTime(Time))
			{
			Found = 1;
			if (ReInitNoise(Ct))
				{
				initNoise = 1;
				} // if
			if (ReInitBasis(Ct))
				{
				initBasis = 1;
				} // if
			} // if
		} // if
	} // for
for (Ct = 0; Ct < 3; Ct ++)
	{
	if (TexCenter[Ct].SetToTime(Time) ||
		TexFalloff[Ct].SetToTime(Time) ||
		TexVelocity[Ct].SetToTime(Time) ||
		TexSize[Ct].SetToTime(Time) ||
		TexRotation[Ct].SetToTime(Time))
		Found = 1;
	} // for
if (TexFeather.SetToTime(Time))
	Found = 1;
if (ColorGrad.SetToTime(Time))
	Found = 1;
for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	if (Tex[Ct])
		{
		if (Tex[Ct]->SetToTime(Time))
			Found = 1;
		} // if
	} // for
if (initNoise)
	{
	InitNoise();
	} // if
if (initBasis)
	{
	InitBasis();
	} // if

return (Found);

} // Texture::SetToTime

/*===========================================================================*/

int Texture::IsAnimated(void)
{
int Ct;
GradientCritter *CurGrad = NULL;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; ++Ct)
	{
	if (TexParam[Ct].GetNumNodes(0) > 1)
		return (1);
	} // if

for (Ct = 0; Ct < 3; ++Ct)
	{
	if (TexSize[Ct].GetNumNodes(0) > 1)
		return (1);
	if (TexCenter[Ct].GetNumNodes(0) > 1)
		return (1);
	if (TexRotation[Ct].GetNumNodes(0) > 1)
		return (1);
	if (TexVelocity[Ct].GetNumNodes(0) > 1)
		return (1);
	if (TexFalloff[Ct].GetNumNodes(0) > 1)
		return (1);
	if (TexVelocity[Ct].CurValue != 0.0)
		return (1);
	} // if

if (Img && Img->GetRaster() && Img->GetRaster()->GetEnabled() && (Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_SEQUENCE) || Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE)))
	return (1);
	
while (CurGrad = ColorGrad.GetNextNode(CurGrad))
	{
	if (CurGrad->GetThing() && ((ColorTextureThing *)CurGrad->GetThing())->Tex && 
		((ColorTextureThing *)CurGrad->GetThing())->Tex->IsAnimated())
		{
		return (1);
		} // if
	} // while

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; ++Ct)
	{
	if (Tex[Ct] && Tex[Ct]->Enabled && Tex[Ct]->IsAnimated())
		return (1);
	} // for

return (0);

} // Texture::IsAnimated

/*===========================================================================*/

int Texture::GetRAHostAnimated(void)
{
int Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; ++Ct)
	{
	if (TexParam[Ct].GetRAHostAnimated())
		return (1);
	} // if
for (Ct = 0; Ct < 3; ++Ct)
	{
	if (TexSize[Ct].GetRAHostAnimated())
		return (1);
	if (TexCenter[Ct].GetRAHostAnimated())
		return (1);
	if (TexRotation[Ct].GetRAHostAnimated())
		return (1);
	if (TexVelocity[Ct].GetRAHostAnimated())
		return (1);
	if (TexFalloff[Ct].GetRAHostAnimated())
		return (1);
	} // if
for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; ++Ct)
	{
	if (Tex[Ct] && Tex[Ct]->GetRAHostAnimated())
		return (1);
	} // for
if (ColorGrad.GetRAHostAnimated())
	return (1);

return (0);

} // Texture::GetRAHostAnimated

/*===========================================================================*/

bool Texture::GetRAHostAnimatedInclVelocity(void)
{
int Ct;

if (GetRAHostAnimated())
	return (true);
for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; ++Ct)
	{
	if (Tex[Ct] && Tex[Ct]->GetRAHostAnimatedInclVelocity())
		return (true);
	} // for
if (ColorGrad.AnimateMaterials())
	return (true);
if (TexVelocity[0].CurValue != 0.0 || TexVelocity[1].CurValue != 0.0 || TexVelocity[2].CurValue != 0.0)
	return (true);
	
return (false);

} // Texture::GetRAHostAnimated

/*===========================================================================*/

int Texture::InitAAChain(void)
{
int Test;
GradientCritter *CurGrad = NULL;

if (! InitAAConditional())
	return (0);

while (CurGrad = ColorGrad.GetNextNode(CurGrad))
	{
	if (CurGrad->GetThing() && ((ColorTextureThing *)CurGrad->GetThing())->Tex && 
		! ((ColorTextureThing *)CurGrad->GetThing())->Tex->InitAAChain())
		{
		return (0);
		} // if
	} // while

for (Test = 0; Test < WCS_TEXTURE_MAXPARAMTEXTURES; Test ++)
	{
	if (Tex[Test] && ! Tex[Test]->InitAAChain())
		{
		return (0);
		} // if
	} // for

return (1);

} // Texture::InitAAChain

/*===========================================================================*/

int Texture::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{
int Test;
GradientCritter *CurGrad = NULL;

if (! InitGeoReg())
	return (0);

BuildTextureRotationMatrix(FALSE);

while (CurGrad = ColorGrad.GetNextNode(CurGrad))
	{
	if (CurGrad->GetThing() && ((ColorTextureThing *)CurGrad->GetThing())->Tex && 
		! ((ColorTextureThing *)CurGrad->GetThing())->Tex->InitFrameToRender(Lib, Rend))
		{
		return (0);
		} // if
	} // while

for (Test = 0; Test < WCS_TEXTURE_MAXPARAMTEXTURES; Test ++)
	{
	if (Tex[Test] && ! Tex[Test]->InitFrameToRender(Lib, Rend))
		{
		return (0);
		} // if
	} // for

return (1);

} // Texture::InitFrameToRender

/*===========================================================================*/

int Texture::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{
int Test;
GradientCritter *CurGrad = NULL;
RasterAnimHost *Root;

if (Buffers)
	{
	RasterAnimHostProperties Prop;

	Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
	Root = GetRAHostRoot();
	Root->GetRAHostProperties(&Prop);
	if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_POSTPROC)
		{
		if (TexType == WCS_TEXTURE_TYPE_TERRAINPARAM)
			{
			switch (Misc)
				{
				case WCS_TEXTURE_TERRAINPARAM_ELEV:
					{
					if (! Buffers->AddBufferNode("ELEVATION", WCS_RASTER_BANDSET_FLOAT))
						return (0);
					break;
					} // WCS_TEXTURE_TERRAINPARAM_ELEV
				case WCS_TEXTURE_TERRAINPARAM_RELELEV:
					{
					if (! Buffers->AddBufferNode("RELATIVE ELEVATION", WCS_RASTER_BANDSET_FLOAT))
						return (0);
					break;
					} // WCS_TEXTURE_TERRAINPARAM_RELELEV
				case WCS_TEXTURE_TERRAINPARAM_SLOPE:
					{
					if (! Buffers->AddBufferNode("SLOPE", WCS_RASTER_BANDSET_FLOAT))
						return (0);
					break;
					} // WCS_TEXTURE_TERRAINPARAM_SLOPE
				case WCS_TEXTURE_TERRAINPARAM_NORTHDEV:
				case WCS_TEXTURE_TERRAINPARAM_EASTDEV:
				case WCS_TEXTURE_TERRAINPARAM_ASPECT:
					{
					if (! Buffers->AddBufferNode("ASPECT", WCS_RASTER_BANDSET_FLOAT))
						return (0);
					break;
					} // WCS_TEXTURE_TERRAINPARAM_NORTHDEV
				case WCS_TEXTURE_TERRAINPARAM_LATITUDE:
					{
					if (! Buffers->AddBufferNode("LATITUDE", WCS_RASTER_BANDSET_FLOAT))
						return (0);
					break;
					} // WCS_TEXTURE_TERRAINPARAM_LATITUDE
				case WCS_TEXTURE_TERRAINPARAM_LONGITUDE:
					{
					if (! Buffers->AddBufferNode("LONGITUDE", WCS_RASTER_BANDSET_FLOAT))
						return (0);
					break;
					} // WCS_TEXTURE_TERRAINPARAM_LONGITUDE
				case WCS_TEXTURE_TERRAINPARAM_ILLUMINATION:
					{
					if (! Buffers->AddBufferNode("ILLUMINATION", WCS_RASTER_BANDSET_FLOAT))
						return (0);
					break;
					} // WCS_TEXTURE_TERRAINPARAM_ILLUMINATION
				case WCS_TEXTURE_TERRAINPARAM_ZDISTANCE:
					{
					if (! Buffers->AddBufferNode("ZBUF", WCS_RASTER_BANDSET_FLOAT))
						return (0);
					break;
					} // WCS_TEXTURE_TERRAINPARAM_ZDISTANCE
				case WCS_TEXTURE_TERRAINPARAM_REFLECTIVITY:
					{
					if (! Buffers->AddBufferNode("REFLECTION", WCS_RASTER_BANDSET_FLOAT))
						return (0);
					break;
					} // WCS_TEXTURE_TERRAINPARAM_REFLECTIVITY
				case WCS_TEXTURE_TERRAINPARAM_NORMALFROMCAMERA:
					{
					if (! Buffers->AddBufferNode("SURFACE NORMAL X", WCS_RASTER_BANDSET_FLOAT))
						return (0);
					if (! Buffers->AddBufferNode("SURFACE NORMAL Y", WCS_RASTER_BANDSET_FLOAT))
						return (0);
					if (! Buffers->AddBufferNode("SURFACE NORMAL Z", WCS_RASTER_BANDSET_FLOAT))
						return (0);
					break;
					} // WCS_TEXTURE_TERRAINPARAM_NORMALFROMCAMERA
				} // switch
			} // if
		else if (TexType == WCS_TEXTURE_TYPE_INCLUDEEXCLUDE)
			{
			if (! Buffers->AddBufferNode("OBJECT", WCS_RASTER_BANDSET_FLOAT))
				return (0);
			} // if
		else if (TexType == WCS_TEXTURE_TYPE_INCLUDEEXCLUDETYPE)
			{
			if (! Buffers->AddBufferNode("OBJECT TYPE", WCS_RASTER_BANDSET_BYTE))
				return (0);
			} // if
		else if (CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
			{
			if (! Buffers->AddBufferNode("LATITUDE", WCS_RASTER_BANDSET_FLOAT))
				return (0);
			if (! Buffers->AddBufferNode("LONGITUDE", WCS_RASTER_BANDSET_FLOAT))
				return (0);
			} // if
		} // if
	} // if

while (CurGrad = ColorGrad.GetNextNode(CurGrad))
	{
	if (CurGrad->GetThing() && ((ColorTextureThing *)CurGrad->GetThing())->Tex && 
		! ((ColorTextureThing *)CurGrad->GetThing())->Tex->InitToRender(Opt, Buffers))
		{
		return (0);
		} // if
	} // while

for (Test = 0; Test < WCS_TEXTURE_MAXPARAMTEXTURES; Test ++)
	{
	if (Tex[Test] && ! Tex[Test]->InitToRender(Opt, Buffers))
		{
		return (0);
		} // if
	} // for

return (1);

} // Texture::InitToRender

/*===========================================================================*/

int Texture::InitAAConditional(void)
{

if (Antialias && WCS_TEXTURETYPE_USESUMTABLEANTIALIAS(TexType))
	{
	if (! InitAA())
		return (0);
	} // if
else if (AASampler)
	{
	delete AASampler;
	AASampler = NULL;
	} // else if
return (1);

} // Texture::InitAAConditional

/*===========================================================================*/

TextureAA *Texture::InitAA(void)
{

if (! AASampler)
	AASampler = new TextureAA();
if (AASampler)
	{
	AASampler->Init(this);
	} // if

return (AASampler);

} // Texture::InitAA

/*===========================================================================*/

void Texture::ResolveLoadLinkages(EffectsLib *Lib)
{
int Test;
GradientCritter *CurGrad = NULL;

if (InclExcl)
	InclExcl->ResolveLoadLinkages(Lib);

while (CurGrad = ColorGrad.GetNextNode(CurGrad))
	{
	if (CurGrad->GetThing() && ((ColorTextureThing *)CurGrad->GetThing())->Tex) 
		{
		((ColorTextureThing *)CurGrad->GetThing())->Tex->ResolveLoadLinkages(Lib);
		} // if
	} // while

for (Test = 0; Test < WCS_TEXTURE_MAXPARAMTEXTURES; Test ++)
	{
	if (Tex[Test])
		{
		Tex[Test]->ResolveLoadLinkages(Lib);
		} // if
	} // for

} // Texture::ResolveLoadLinkages

/*===========================================================================*/

void Texture::SetBounds(double LatRange[2], double LonRange[2])
{
RasterAttribute *MyAttr;

if (Img && Img->GetRaster())
	{
	if ((MyAttr = Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
		{
		((GeoRefShell *)MyAttr->GetShell())->SetBounds(LatRange, LonRange);
		} // if
	} // if

/*
NotifyTag Changes[2];

// this will set new texture lat/lon bounds

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
	if (LatRange[1] < LatRange[0])
		swmem(&LatRange[0], &LatRange[1], sizeof (double));
	if (LonRange[1] < LonRange[0])
		swmem(&LonRange[0], &LonRange[1], sizeof (double));
	// if bounds appear to wrap more than halfway around earth then probably 
	// want to take the smaller arc
	if (fabs(LonRange[1] - LonRange[0]) > 180.0)
		{
		LonRange[1] -= 360.0;
		} // if
	GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(LatRange[1]);
	GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(LatRange[0]);
	GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(LonRange[1]);
	GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(LonRange[0]);
	Changes[0] = MAKE_ID(GeoReg.GetNotifyClass(), GeoReg.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GeoReg.GetRAHostRoot());
	} // if
*/
} // Texture::SetBounds

/*===========================================================================*/

Texture *Texture::RemoveTexture(Texture *RemoveMe)
{
int Ct;
Texture *RemovedParent, *DelTex;
GradientCritter *CurGrad = NULL;

while (CurGrad = ColorGrad.GetNextNode(CurGrad))
	{
	if (CurGrad->GetThing() && ((ColorTextureThing *)CurGrad->GetThing())->Tex)
		{
		if (RemoveMe == ((ColorTextureThing *)CurGrad->GetThing())->Tex)
			{
			DelTex = ((ColorTextureThing *)CurGrad->GetThing())->Tex;
			((ColorTextureThing *)CurGrad->GetThing())->Tex = NULL;
			delete DelTex;
			return (this);
			} // if
		if (RemovedParent = ((ColorTextureThing *)CurGrad->GetThing())->Tex->RemoveTexture(RemoveMe))
			return (RemovedParent);
		} // if
	} // while

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	if (Tex[Ct])
		{
		if (Tex[Ct] == RemoveMe)
			{
			DelTex = Tex[Ct];
			Tex[Ct] = NULL;
			delete DelTex;
			return (this);
			} // if
		if (RemovedParent = Tex[Ct]->RemoveTexture(RemoveMe))
			return (RemovedParent);
		} // if
	} // for

return (NULL);

} // Texture::RemoveTexture

/*===========================================================================*/

int Texture::FindTextureNumberInParent(void)
{
int Test;
GradientCritter *CurGrad = NULL;

if (Prev)
	{
	while (CurGrad = Prev->ColorGrad.GetNextNode(CurGrad))
		{
		if (CurGrad->GetThing() && this == ((ColorTextureThing *)CurGrad->GetThing())->Tex)
			return (WCS_TEXTURE_COLOR1);
		} // while
	for (Test = 0; Test < WCS_TEXTURE_MAXPARAMTEXTURES; Test ++)
		{
		if (this == Prev->Tex[Test])
			{
			return (Test);
			} // if
		} // for
	} // if

return (WCS_TEXTURE_ROOT);

} // Texture::FindTextureNumberInParent

/*===========================================================================*/

Texture *Texture::FindTexture(Texture *FindMe)
{
int Ct;
GradientCritter *CurGrad = NULL;

while (CurGrad = ColorGrad.GetNextNode(CurGrad))
	{
	if (CurGrad->GetThing())
		{
		if (((ColorTextureThing *)CurGrad->GetThing())->Tex)
			{
			if (FindMe == ((ColorTextureThing *)CurGrad->GetThing())->Tex ||
				((ColorTextureThing *)CurGrad->GetThing())->Tex->FindTexture(FindMe))
				return (FindMe);
			} // if
		} // if
	} // while
for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	if (Tex[Ct])
		{
		if (Tex[Ct] == FindMe || Tex[Ct]->FindTexture(FindMe))
			return (FindMe);
		} // if
	} // for

return (NULL);

} // Texture::FindTexture

/*===========================================================================*/

int Texture::GetApplicationLegal(unsigned char Ecosys)
{

if (Ecosys)
	return (! WCS_TEXTURETYPE_ECOILLEGAL(TexType));
else
	return (! WCS_TEXTURETYPE_OBJECT3DILLEGAL(TexType));

} // Texture::GetApplicationLegal

/*===========================================================================*/

int Texture::ConfigureForEcosystem(void)
{
int Test;
GradientCritter *CurGrad = NULL;

if (WCS_TEXTURETYPE_ECOILLEGAL(TexType))
	return (0);
ApplyToEcosys = 1;

while (CurGrad = ColorGrad.GetNextNode(CurGrad))
	{
	if (CurGrad->GetThing() && ((ColorTextureThing *)CurGrad->GetThing())->Tex && 
		! ((ColorTextureThing *)CurGrad->GetThing())->Tex->ConfigureForEcosystem())
		{
		return (0);
		} // if
	} // while

for (Test = 0; Test < WCS_TEXTURE_MAXPARAMTEXTURES; Test ++)
	{
	if (Tex[Test] && ! Tex[Test]->ConfigureForEcosystem())
		{
		return (0);
		} // if
	} // for

return (1);

} // Texture::ConfigureForEcosystem

/*===========================================================================*/

int Texture::ConfigureForObject3D(void)
{
int Test;
GradientCritter *CurGrad = NULL;

if (WCS_TEXTURETYPE_OBJECT3DILLEGAL(TexType))
	return (0);
ApplyToEcosys = 0;

while (CurGrad = ColorGrad.GetNextNode(CurGrad))
	{
	if (CurGrad->GetThing() && ((ColorTextureThing *)CurGrad->GetThing())->Tex && 
		! ((ColorTextureThing *)CurGrad->GetThing())->Tex->ConfigureForObject3D())
		{
		return (0);
		} // if
	} // while

for (Test = 0; Test < WCS_TEXTURE_MAXPARAMTEXTURES; Test ++)
	{
	if (Tex[Test] && ! Tex[Test]->ConfigureForObject3D())
		{
		return (0);
		} // if
	} // for

return (1);

} // Texture::ConfigureForObject3D

/*===========================================================================*/

int Texture::SetSizeToFitMaterial(void)
{
RasterAnimHost *RootOp;
RasterAnimHostProperties Prop;
double XRange[2], YRange[2], ZRange[2];

RootOp = GetRAHostRoot();
Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
RootOp->GetRAHostProperties(&Prop);
if (Prop.TypeNumber >= WCS_EFFECTSSUBCLASS_LAKE && Prop.TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
	{
	if (((GeneralEffect *)RootOp)->GetMaterialBoundsXYZ(XRange, YRange, ZRange))
		{
		// set centers on all three axes
		TexCenter[0].SetValue((XRange[0] + XRange[1]) * .5);
		TexCenter[1].SetValue((YRange[0] + YRange[1]) * .5);
		TexCenter[2].SetValue((ZRange[0] + ZRange[1]) * .5);

		// set size on all three axes making sure not to set size to 0 or less
		TexSize[0].SetValue(XRange[0] - XRange[1]);
		TexSize[1].SetValue(YRange[0] - YRange[1]);
		TexSize[2].SetValue(ZRange[0] - ZRange[1]);
		
		// one notify should be sufficient
		TexSize[0].ValueChanged();
		return (1);
		} // if
	} // if

return (0);

} //Texture::SetSizeToFitMaterial

/*===========================================================================*/

int Texture::UsesSeed(void)
{
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	if (ParamAvailable(Ct))
		{
		if (GetTextureParamType(Ct) == WCS_TEXTURE_PARAMTYPE_SEED)
			{
			return (1);
			} // if
		} // if
	} // for

return (0);

} // Texture::UsesSeed

/*===========================================================================*/

void Texture::OffsetTexCenter(int Channel, double Offset)
{
int Test;
GradientCritter *CurGrad = NULL;

if (TexType >= WCS_TEXTURE_TYPE_WOODGRAIN && TexType <= WCS_TEXTURE_TYPE_BIRDSHOT)
	TexCenter[Channel].SetValue(TexCenter[Channel].CurValue + Offset);

while (CurGrad = ColorGrad.GetNextNode(CurGrad))
	{
	if (CurGrad->GetThing() && ((ColorTextureThing *)CurGrad->GetThing())->Tex)
		((ColorTextureThing *)CurGrad->GetThing())->Tex->OffsetTexCenter(Channel, Offset);
	} // while

for (Test = 0; Test < WCS_TEXTURE_MAXPARAMTEXTURES; Test ++)
	{
	if (Tex[Test])
		Tex[Test]->OffsetTexCenter(Channel, Offset);
	} // for

} // Texture::OffsetTexCenter

/*===========================================================================*/

int Texture::IsTextureTileable(double &dTileWidth, double &dTileHeight, double &dTileCenterX, double &dTileCenterY, 
	int &SizeDoesMatter)
{
double CurTileWidth, CurTileHeight, CurTileCenterX, CurTileCenterY;
int OneTileDone = 0, Test, Tileable = 1, TempSizeDoesMatter;
GradientCritter *CurGrad = NULL;

if (! WCS_TEXTURETYPE_USESIZE(TexType))
	{
	SizeDoesMatter = 0;
	return (1);
	} // if

SizeDoesMatter = 1;

// ennumerate the texture types that are not tileable under any circumstances
if (! WCS_TEXTURETYPE_TILEABLE(TexType))
	return (0);

// texture must be aligned to vector
if (WCS_TEXTUREPARAM_USECOORDSPACE(TexType) && CoordSpace != WCS_TEXTURE_COORDSPACE_VECTOR_ALIGNED)
	return (0);

// axis must be Y or Z
if (WCS_TEXTURETYPE_USEAXIS(TexType) && TexAxis == 0)
	return (0);

// if rotation legal, must not rotate texture on x
if (WCS_TEXTURETYPE_USEROTATION(TexType) && fabs(TexRotation[0].CurValue) > .001)
	return (0);

// if falloff legal, must not falloff texture on y or z
if (fabs(TexFalloff[1].CurValue) > .001 || fabs(TexFalloff[2].CurValue) > .001)
	return (0);

// y axis for aligned texture is in direction of vector
// z axis is vertical
dTileWidth = TexSize[1].CurValue;
dTileHeight = TexSize[2].CurValue;
dTileCenterX = TexCenter[1].CurValue;
dTileCenterY = TexCenter[2].CurValue;

// if image, must double size if flipX, Y
if (WCS_TEXTURETYPE_ISBITMAP(TexType))
	{
	if (FlipWidth)
		dTileWidth *= 2.0;
	if (FlipHeight)
		dTileHeight *= 2.0;
	} // if

while ((CurGrad = ColorGrad.GetNextNode(CurGrad)) && Tileable)
	{
	if (CurGrad->GetThing() && ((ColorTextureThing *)CurGrad->GetThing())->Tex)
		{
		if (Tileable = ((ColorTextureThing *)CurGrad->GetThing())->Tex->IsTextureTileable(CurTileWidth, CurTileHeight, CurTileCenterX, CurTileCenterY, TempSizeDoesMatter))
			{
			if (TempSizeDoesMatter)
				{
				if (OneTileDone)
					{
					if (fabs(CurTileCenterX - dTileCenterX) <= .01 && fabs(CurTileCenterY - dTileCenterY) <= .01)
						{
						// find common size
						if (fabs(CurTileWidth - dTileWidth) > .01 || fabs(CurTileHeight - dTileHeight) > .01)
							{
							dTileWidth = WCS_lcm(dTileWidth, CurTileWidth, .05);
							dTileHeight = WCS_lcm(dTileHeight, CurTileHeight, .05);
							} // if
						} // if
					else
						Tileable = 0;
					} // if
				else
					{
					dTileCenterX = CurTileCenterX;
					dTileCenterY = CurTileCenterY;
					dTileWidth = CurTileWidth;
					dTileHeight = CurTileHeight;
					OneTileDone = 1;
					} // else
				} // if
			} // if
		} // if
	} // while

for (Test = 0; Test < WCS_TEXTURE_MAXPARAMTEXTURES && Tileable; Test ++)
	{
	if (Tex[Test])
		{
		if (Tileable = Tex[Test]->IsTextureTileable(CurTileWidth, CurTileHeight, CurTileCenterX, CurTileCenterY, TempSizeDoesMatter))
			{
			if (TempSizeDoesMatter)
				{
				if (OneTileDone)
					{
					if (fabs(CurTileCenterX - dTileCenterX) <= .01 && fabs(CurTileCenterY - dTileCenterY) <= .01)
						{
						// find common size
						if (fabs(CurTileWidth - dTileWidth) > .01 || fabs(CurTileHeight - dTileHeight) > .01)
							{
							dTileWidth = WCS_lcm(dTileWidth, CurTileWidth, .05);
							dTileHeight = WCS_lcm(dTileHeight, CurTileHeight, .05);
							} // if
						} // if
					else
						Tileable = 0;
					} // if
				else
					{
					dTileCenterX = CurTileCenterX;
					dTileCenterY = CurTileCenterY;
					dTileWidth = CurTileWidth;
					dTileHeight = CurTileHeight;
					OneTileDone = 1;
					} // else
				} // if
			} // if
		} // if
	} // for

return (Tileable);

} // Texture::IsTextureTileable

/*===========================================================================*/

long Texture::InitImageIDs(long &ImageID)
{
long Test, ImagesFound = 0;

if (Img && Img->GetRaster())
	{
	Img->GetRaster()->SetID(ImageID++);
	ImagesFound = 1;
	} // if

for (Test = 0; Test < WCS_TEXTURE_MAXPARAMTEXTURES; Test ++)
	{
	if (Tex[Test])
		{
		ImagesFound += Tex[Test]->InitImageIDs(ImageID);
		} // if
	} // for
ImagesFound += ColorGrad.InitImageIDs(ImageID);

return (ImagesFound);

} // Texture::InitImageIDs

/*===========================================================================*/

long Texture::SetColorList(short *ColorList, long &ListPos)
{
long Test, ColorsFound = 0;

if (ApplyToColor)
	{
	if (ColorList)
		ColorList[ListPos++] = PalColor[0];
	ColorsFound ++;
	if (ColorList)
		ColorList[ListPos++] = PalColor[1];
	ColorsFound ++;
	} // if

for (Test = 0; Test < WCS_TEXTURE_MAXPARAMTEXTURES; Test ++)
	{
	if (Tex[Test])
		{
		ColorsFound += Tex[Test]->SetColorList(ColorList, ListPos);
		} // if
	} // for

return (ColorsFound);

} // Texture::SetColorList

/*===========================================================================*/

void Texture::BuildTextureRotationMatrix(int EvalChildren)
{
int Ct;
GradientCritter *CurGrad = NULL;


if (TextureRotated())
	{
	BuildRotationMatrix(TexRotation[0].CurValue * PiOver180, TexRotation[1].CurValue * PiOver180, TexRotation[2].CurValue * PiOver180, RotMatx);
	Rotated = 1;
	} // if
else
	Rotated = 0;

if (EvalChildren)
	{
	while (CurGrad = ColorGrad.GetNextNode(CurGrad))
		{
		if (CurGrad->GetThing())
			{
			if (((ColorTextureThing *)CurGrad->GetThing())->Tex)
				{
				((ColorTextureThing *)CurGrad->GetThing())->Tex->BuildTextureRotationMatrix(EvalChildren);
				} // if
			} // if
		} // while
	for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
		{
		if (Tex[Ct])
			Tex[Ct]->BuildTextureRotationMatrix(EvalChildren);
		} // for
	} // if

} // Texture::BuildTextureRotationMatrix

/*===========================================================================*/

void Texture::BuildTextureRotationMatrixT(void)
{
GradientCritter *CurGrad = NULL;


if (TextureRotatedT())
	{
	BuildRotationMatrix(TRotation[0] * PiOver180, TRotation[1] * PiOver180, TRotation[2] * PiOver180, RotMatx);
	Rotated = 1;
	} // if
else
	Rotated = 0;

} // Texture::BuildTextureRotationMatrixT

/*===========================================================================*/

// inline functions 
void Texture::AdjustSampleForVelocity(double &LowX, double &HighX, double &LowY, double &HighY, double &LowZ, double &HighZ)
{
double Time, DistOffset;

Time = GlobalApp->MainProj->Interactive->GetActiveTime();

// this should really be an integral of the time velocity curve if velocity is animated.
// beter to animate texture center to change the velocity over time
DistOffset = TexVelocity[0].CurValue * Time;
LowX -= DistOffset;
HighX -= DistOffset;
DistOffset = TexVelocity[1].CurValue * Time;
LowY -= DistOffset;
HighY -= DistOffset;
DistOffset = TexVelocity[2].CurValue * Time;
LowZ -= DistOffset;
HighZ -= DistOffset;

} // Texture::AdjustSampleForVelocity

/*===========================================================================*/

void Texture::ReCenterMoveRotate(double &LowX, double &HighX, double &LowY, double &HighY, double &LowZ, double &HighZ)
{

LowX -= TCenter[0];
HighX -= TCenter[0];
LowY -= TCenter[1];
HighY -= TCenter[1];
LowZ -= TCenter[2];
HighZ -= TCenter[2];

if (TextureMoving())
	{
	AdjustSampleForVelocity(LowX, HighX, LowY, HighY, LowZ, HighZ);
	} // if

if (Rotated)
	{
	RotatePoint(LowX, LowY, LowZ, RotMatx);
	RotatePoint(HighX, HighY, HighZ, RotMatx);
	} // if

} // Texture::ReCenterMoveRotate

/*===========================================================================*/

// not used in V2
void Texture::ReSize(double &LowX, double &HighX, double &LowY, double &HighY, double &LowZ, double &HighZ)
{

LowX /= TexSize[0].CurValue;
HighX /= TexSize[0].CurValue;
LowY /= TexSize[1].CurValue;
HighY /= TexSize[1].CurValue;
LowZ /= TexSize[2].CurValue;
HighZ /= TexSize[2].CurValue;

} // Texture::ReSize

/*===========================================================================*/

void Texture::ReSizeT(double &LowX, double &HighX, double &LowY, double &HighY, double &LowZ, double &HighZ)
{

LowX /= TSize[0];
HighX /= TSize[0];
LowY /= TSize[1];
HighY /= TSize[1];
LowZ /= TSize[2];
HighZ /= TSize[2];

} // Texture::ReSizeT

/*===========================================================================*/

double Texture::MaxSampleSize(double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ)
{
double A, B;

A = fabs(HighX - LowX);
B = fabs(HighY - LowY);
B = max(A, B);
A = fabs(HighZ - LowZ);
return (max(A, B));

} // Texture::MaxSampleSize

/*===========================================================================*/

double Texture::FractalSetup(double MaxOctaves, double &LowX, double &HighX, double &LowY, double &HighY, double &LowZ, double &HighZ)
{
double Octaves, SampleWidth;

LowX -= TCenter[0] + TexParam[WCS_TEXTURE_FRACTALNOISE_INPUTSEED].CurValue;
HighX -= TCenter[0] + TexParam[WCS_TEXTURE_FRACTALNOISE_INPUTSEED].CurValue;
LowY -= TCenter[1] + TexParam[WCS_TEXTURE_FRACTALNOISE_INPUTSEED].CurValue;
HighY -= TCenter[1] + TexParam[WCS_TEXTURE_FRACTALNOISE_INPUTSEED].CurValue;
LowZ -= TCenter[2] + TexParam[WCS_TEXTURE_FRACTALNOISE_INPUTSEED].CurValue;
HighZ -= TCenter[2] + TexParam[WCS_TEXTURE_FRACTALNOISE_INPUTSEED].CurValue;

if (TextureMoving())
	{
	AdjustSampleForVelocity(LowX, HighX, LowY, HighY, LowZ, HighZ);
	} // if

if (Rotated)
	{
	RotatePoint(LowX, LowY, LowZ, RotMatx);
	RotatePoint(HighX, HighY, HighZ, RotMatx);
	} // if

ReSizeT(LowX, HighX, LowY, HighY, LowZ, HighZ);

SampleWidth = MaxSampleSize(LowX, HighX, LowY, HighY, LowZ, HighZ);
if (SampleWidth > 0.0)
	{
	if (SampleWidth < 2.0)
		{
		Octaves = 1.0 + log(1.0 / SampleWidth) * INVLOG2;
		Octaves = min(Octaves, MaxOctaves);
		} // if
	else
		Octaves = 0.0;
	} // if
else
	Octaves = MaxOctaves;

return (Octaves);

} // Texture::FractalSetup

/*===========================================================================*/

double Texture::CellBasisSetup(double MaxOctaves, double &LowX, double &HighX, double &LowY, double &HighY, double &LowZ, double &HighZ)
{
double Octaves, SampleWidth;

ReCenterMoveRotate(LowX, HighX, LowY, HighY, LowZ, HighZ);
ReSizeT(LowX, HighX, LowY, HighY, LowZ, HighZ);

SampleWidth = MaxSampleSize(LowX, HighX, LowY, HighY, LowZ, HighZ);
if (SampleWidth > 0.0)
	{
	if (SampleWidth < 2.0)
		{
		Octaves = 1.0 + log(1.0 / SampleWidth) * INVLOG2;
		Octaves = min(Octaves, MaxOctaves);
		} // if
	else
		Octaves = 0.0;
	} // if
else
	Octaves = MaxOctaves;

return (Octaves);

} // Texture::CellBasisSetup

/*===========================================================================*/

double Texture::AnalyzeWrapup(double Output[3], double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], double Value, TextureData *Data, int EvalChildren)
{

// apply blending and strata functions to Value
if (EvalChildren)
	EvalStrataBlend(Value, Data);

// scale the output to the appropriate range
if (ApplyToColor)
	{
	// <<<>>>gh Data is NULL for samples unless we change things, and texture coords must be passed in it!
	ColorGrad.Analyze(Output, Value, Data, EvalChildren);
	} // if
else
	{
	Output[0] = Input[WCS_TEXTURE_LOW][0] + (Input[WCS_TEXTURE_HIGH][0] - Input[WCS_TEXTURE_LOW][0]) * Value;
	Output[1] = 0.0;
	Output[2] = 0.0;
	} // else

return (Value);

} // Texture::AnalyzeWrapup

/*===========================================================================*/

double Texture::AnalyzeImageWrapup(double Output[3], double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], double Value, TextureData *Data, int EvalChildren)
{

// apply blending and strata functions to Value
if (EvalChildren)
	{
	EvalStrataBlend(Output[0], Data);
	EvalStrataBlend(Output[1], Data);
	EvalStrataBlend(Output[2], Data);
	} // if

// scale the output to the appropriate range
if (! ApplyToColor)
	{
	Output[1] = 0.0;
	Output[2] = 0.0;
	} // else

if (Value > 1.0)
	Value = 1.0;
else if (Value < 0.0)
	Value = 0.0;
if (AlphaOnly)
	{
	if (! ImageNeg)
		Output[2] = Output[1] = Output[0] = Value;
	else
		Output[2] = Output[1] = Output[0] = 1.0 - Value;
	} // if
	
return (Value);

} // Texture::AnalyzeImageWrapup

/*===========================================================================*/

int AnimColorGradient::Analyze(double Output[3], double Pos, TextureData *Data, int EvalChildren)
{
double LastPos = 0.0, Diff, Covg[2], TexOpacity[2], TexOutput[2][3];
ColorTextureThing *Thing[2];
GradientCritter *CurGrad = Grad, *LastGrad = NULL;
long CurTexType;
int StrataBlend;

Thing[1] = Thing[0] = NULL;
Output[2] = Output[1] = Output[0] = 0.0;

while (CurGrad)
	{
	if (CurGrad->Position.CurValue == Pos)
		{
		if (Thing[0] = (ColorTextureThing *)CurGrad->GetThing())
			{
			Output[0] = Thing[0]->Color.GetCompleteValue(0);
			Output[1] = Thing[0]->Color.GetCompleteValue(1);
			Output[2] = Thing[0]->Color.GetCompleteValue(2);
			goto FinalAnalysis;
			} // if
		// returns 0 if the color is invalid
		return (0);
		} // if
	if (CurGrad->Position.CurValue > Pos)
		{
		if (LastGrad)
			{
			Thing[0] = (ColorTextureThing *)LastGrad->GetThing();
			Thing[1] = (ColorTextureThing *)CurGrad->GetThing();
			if (Thing[0] && Thing[1])
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
						Covg[1] = (Pos - LastPos) / Diff;
						if (CurGrad->BlendStyle == WCS_GRADIENTCRITTER_BLENDSTYLE_SLOWINCREASE)
							{
							Covg[1] *= Covg[1];
							} // if
						else if (CurGrad->BlendStyle == WCS_GRADIENTCRITTER_BLENDSTYLE_FASTINCREASE)
							{
							Covg[1] = 1.0 - Covg[1];
							Covg[1] *= Covg[1];
							Covg[1] = 1.0 - Covg[1];
							} // if
						else if (CurGrad->BlendStyle == WCS_GRADIENTCRITTER_BLENDSTYLE_SCURVE)
							{
							Covg[1] *= Covg[1] * (3.0 - 2.0 * Covg[1]);	// same as PERLIN_s_curve()
							} // if
						Covg[0] = 1.0 - Covg[1];
						} // if
					else
						{
						Thing[1] = NULL;
						} // else
					} // if
				else
					{
					Thing[0] = Thing[1];
					Thing[1] = NULL;
					} // else
				goto FinalAnalysis;
				} // if
			if (Thing[0])
				{
				goto FinalAnalysis;
				} // if
			if (Thing[1])
				{
				Thing[0] = Thing[1];
				Thing[1] = NULL;
				goto FinalAnalysis;
				} // if
			return (0);
			} // if
		else
			{
			if (Thing[0] = (ColorTextureThing *)CurGrad->GetThing())
				{
				goto FinalAnalysis;
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
	if (Thing[0] = (ColorTextureThing *)LastGrad->GetThing())
		{
		goto FinalAnalysis;
		} // if
	} // if

// couldn't find a valid gradient critter
return (0);

FinalAnalysis:
// see if there are any textures we need to consider

TexOpacity[0] = TexOpacity[1] = 0.0;
if (EvalChildren)
	{
	if (Thing[0]->Tex && Thing[0]->Tex->Enabled)
		{
		if (Thing[1] && Thing[1]->Tex && Thing[1]->Tex->Enabled)
			{
			//#pragma omp parallel sections private(CurTexType, StrataBlend)
				{
				//#pragma omp section
					{
					CurTexType = Thing[0]->Tex->GetTexType();
					StrataBlend = 0;//(! WCS_TEXTURETYPE_BLENDILLEGAL(CurTexType));
					TexOutput[0][0] = TexOutput[0][1] = TexOutput[0][2] = 0.0;
					TexOpacity[0] = Thing[0]->Tex->Eval(TexOutput[0], Data, EvalChildren, StrataBlend);
					} // #pragma omp section
				//#pragma omp section
					{
					CurTexType = Thing[1]->Tex->GetTexType();
					StrataBlend = 0;//(! WCS_TEXTURETYPE_BLENDILLEGAL(CurTexType));
					TexOutput[1][0] = TexOutput[1][1] = TexOutput[1][2] = 0.0;
					TexOpacity[1] = Thing[1]->Tex->Eval(TexOutput[1], Data, EvalChildren, StrataBlend);
					} // #pragma omp section
				} // #pragma omp parallel sections
			} // if
		else
			{
			CurTexType = Thing[0]->Tex->GetTexType();
			StrataBlend = 0;//(! WCS_TEXTURETYPE_BLENDILLEGAL(CurTexType));
			TexOutput[0][0] = TexOutput[0][1] = TexOutput[0][2] = 0.0;
			TexOpacity[0] = Thing[0]->Tex->Eval(TexOutput[0], Data, EvalChildren, StrataBlend);
			} // else
		} // if
	else if (Thing[1] && Thing[1]->Tex && Thing[1]->Tex->Enabled)
		{
		CurTexType = Thing[1]->Tex->GetTexType();
		StrataBlend = 0;//(! WCS_TEXTURETYPE_BLENDILLEGAL(CurTexType));
		TexOutput[1][0] = TexOutput[1][1] = TexOutput[1][2] = 0.0;
		TexOpacity[1] = Thing[1]->Tex->Eval(TexOutput[1], Data, EvalChildren, StrataBlend);
		} // if
	} // if
// blend texture colors with basic color
if (TexOpacity[0] > 0.0)
	{
	if (TexOpacity[0] < 1.0)
		{
		TexOpacity[0] = 1.0 - TexOpacity[0];
		Output[0] = TexOutput[0][0] + Thing[0]->Color.GetCompleteValue(0) * TexOpacity[0];
		Output[1] = TexOutput[0][1] + Thing[0]->Color.GetCompleteValue(1) * TexOpacity[0];
		Output[2] = TexOutput[0][2] + Thing[0]->Color.GetCompleteValue(2) * TexOpacity[0];
		} // if
	else
		{
		Output[0] = TexOutput[0][0];
		Output[1] = TexOutput[0][1];
		Output[2] = TexOutput[0][2];
		} // else
	} // if
else
	{
	Output[0] = Thing[0]->Color.GetCompleteValue(0);
	Output[1] = Thing[0]->Color.GetCompleteValue(1);
	Output[2] = Thing[0]->Color.GetCompleteValue(2);
	} // else
if (Thing[1])
	{
	if (TexOpacity[1] > 0.0)
		{
		if (TexOpacity[1] < 1.0)
			{
			TexOpacity[1] = 1.0 - TexOpacity[1];
			TexOutput[1][0] = TexOutput[1][0] + Thing[1]->Color.GetCompleteValue(0) * TexOpacity[1];
			TexOutput[1][1] = TexOutput[1][1] + Thing[1]->Color.GetCompleteValue(1) * TexOpacity[1];
			TexOutput[1][2] = TexOutput[1][2] + Thing[1]->Color.GetCompleteValue(2) * TexOpacity[1];
			} // if
		} // if
	else
		{
		TexOutput[1][0] = Thing[1]->Color.GetCompleteValue(0);
		TexOutput[1][1] = Thing[1]->Color.GetCompleteValue(1);
		TexOutput[1][2] = Thing[1]->Color.GetCompleteValue(2);
		} // else
	Output[0] = Output[0] * Covg[0] + TexOutput[1][0] * Covg[1];
	Output[1] = Output[1] * Covg[0] + TexOutput[1][1] * Covg[1];
	Output[2] = Output[2] * Covg[0] + TexOutput[1][2] * Covg[1];
	} // if

return (1);

} // AnimColorGradient::Analyze

/*===========================================================================*/

int Texture::InitGeoReg(void)
{
int BoundsType;
double DegLatPerCell, DegLonPerCell;
GeoRegister *GeoReg;
RasterAttribute *MyAttr;

Rast = NULL;
Coords = NULL;

if (CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
	{
	if (Img && Img->GetRaster())
		{
		if ((MyAttr = Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
			{
			GeoReg = &((GeoRefShell *)MyAttr->GetShell())->GeoReg;
			Coords = (CoordSys *)((GeoRefShell *)MyAttr->GetShell())->Host;
			BoundsType = ((GeoRefShell *)MyAttr->GetShell())->BoundsType;

			North = GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue;
			South = GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
			West = GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue;
			East = GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue;

			if (Rast = Img->GetRaster()->Rows && Img->GetRaster()->Cols ? Img->GetRaster(): NULL)
				{
				if (BoundsType == WCS_GEOREFSHELL_BOUNDSTYPE_CENTERS)
					{
					DegLatPerCell = (North - South) / (Rast->Rows - 1);	//lint !e413
					DegLonPerCell = (West - East) / (Rast->Cols - 1);	//lint !e413
					South -= DegLatPerCell * .5;
					North += DegLatPerCell * .5;
					East -= DegLonPerCell * .5;
					West += DegLonPerCell * .5;
					} // if
				} // if
			// these values are in units of the projection, either degrees or meters
			// measured from south and west
			LatRange = North - South;
			LonRange = East - West;

			} // if
		} // if
	return (Rast ? 1: 0);
	} // if lat/lon bnds

return (1);

} // Texture::InitGeoReg

/*===========================================================================*/

double Texture::AdjustSampleForLatLonBounds(double &LowX, double &HighX, double &LowY, double &HighY, double &LowZ, double &HighZ, 
	TextureData *Data, TextureExtractionData *Extraction)
{
double Opacity = 1.0, FeatherWeight, D1, D2, XDiff;
char InPt[4], Ct, WrapEarth;

if (! Rast || LatRange == 0.0 || LonRange == 0.0)
	{
	if (! InitGeoReg())
		return (0.0);
	} // if

WrapEarth = fabs(LonRange) >= 359.99 && (! Coords || Coords->Geographic);

// scale lat/lon to bounds
if (LatRange != 0.0 && LonRange != 0.0)
	{
	LowZ = HighZ = 0.0;

	// sort latitudes
	if (! Data->SampleType)
		{
		if (Coords)
			{
			VertexDEM Vert;

			LowY = LowX = FLT_MAX;
			HighY = HighX = -FLT_MAX;
			for (Ct = 0; Ct < 4; Ct ++)
				{
				Vert.Lat = Vert.xyz[1] = Extraction->LatRange[Ct];
				Vert.Lon = Vert.xyz[0] = Extraction->LonRange[Ct];

				if (! Coords->DefDegToProj(&Vert))
					{
					return (0.0);
					} // if projection error

				Vert.xyz[1] = (Vert.xyz[1] - South) / LatRange;
				// we need these coords from the left edge
				Vert.xyz[0] = (Vert.xyz[0] - West) / LonRange;
				if (LowY > Vert.xyz[1])
					LowY = Vert.xyz[1];
				if (LowX > Vert.xyz[0])
					LowX = Vert.xyz[0];
				if (HighY < Vert.xyz[1])
					HighY = Vert.xyz[1];
				if (HighX < Vert.xyz[0])
					HighX = Vert.xyz[0];
				} // for
			} // if
		else
			{
			LowY = min(Extraction->LatRange[0], Extraction->LatRange[1]);
			LowY = min(LowY, Extraction->LatRange[2]);
			LowY = min(LowY, Extraction->LatRange[3]);
			HighY = max(Extraction->LatRange[0], Extraction->LatRange[1]);
			HighY = max(HighY, Extraction->LatRange[2]);
			HighY = max(HighY, Extraction->LatRange[3]);

			LowY = (LowY - South) /  LatRange;
			HighY = (HighY - South) /  LatRange;
			} // if
		if (WrapEarth && (LowX < 0.0 || HighX > 1.0))
			{
			XDiff = HighX - LowX;
			if (LowX < 0.0 && HighX < 0.0)
				{
				LowX = LowX - WCS_floor(LowX);
				HighX = LowX + XDiff;
				} // if
			else if (LowX > 1.0 && HighX > 1.0)
				{
				HighX = HighX - WCS_floor(HighX);
				LowX = HighX - XDiff;
				} // if
			else
				{
				if (LowX < 0.0)
					{
					LowX = LowX - WCS_floor(LowX);
					//HighX = LowX + XDiff;		// removed 7/13/05 - seems to be unnecessary and in fact erroneous -gh
					} // if
				if (HighX > 1.0)
					{
					HighX = HighX - WCS_floor(HighX);
					//LowX = HighX - XDiff;		// removed 7/13/05 - seems to be unnecessary and in fact erroneous -gh
					} // if
				} // else
			} // if
		} // if
	else if (LowY < HighY)
		Swap64(LowY, HighY);
	InPt[0] = (LowY >= 0.0 && LowY <= 1.0);
	InPt[1] = (HighY >= 0.0 && HighY <= 1.0);
	if (InPt[0] || InPt[1])
		{
		// sort longitudes
		if (! Data->SampleType)
			{
			if (! Coords)
				{
				double invLonRange = 1.0 / LonRange;

				// reverse low and high x because x increases to east, lon incr. to west
				LowX = max(Extraction->LonRange[0], Extraction->LonRange[1]);
				LowX = max(LowX, Extraction->LonRange[2]);
				LowX = max(LowX, Extraction->LonRange[3]);
				HighX = min(Extraction->LonRange[0], Extraction->LonRange[1]);
				HighX = min(HighX, Extraction->LonRange[2]);
				HighX = min(HighX, Extraction->LonRange[3]);

				LowX = (LowX - West) *  invLonRange;
				HighX = (HighX - West) *  invLonRange;
				} // if
			} // if
		else if (LowX < HighX)
			Swap64(LowX, HighX);
		InPt[2] = (LowX >= 0.0 && LowX <= 1.0);
		InPt[3] = (HighX >= 0.0 && HighX <= 1.0);
		if (InPt[2] || InPt[3])
			{
			if (! InPt[0])
				{
				Opacity *= (HighY / (HighY - LowY));
				LowY = 0.0;
				} // if
			else if (! InPt[1])
				{
				Opacity *= ((1.0 - LowY) / (HighY - LowY));
				HighY = 1.0;
				} // else
			if (! InPt[2])
				{
				Opacity *= (HighX / (HighX - LowX));
				LowX = 0.0;
				} // if
			else if (! InPt[3])
				{
				Opacity *= ((1.0 - LowX) / (HighX - LowX));
				HighX = 1.0;
				} // else
			if (TexFeather.CurValue > 0.0)
				{
				D1 = 2.0 * (((LowX + HighX) * .5) - .5);
				D2 = 2.0 * (((LowY + HighY) * .5) - .5);
				FeatherWeight = (10.0 - TexFeather.CurValue) * (1.0 - sqrt(D1 * D1 + D2 * D2));
				FeatherWeight = FeatherWeight <= 0.0 ? 0.0: FeatherWeight >= 1.0 ? 1.0: FeatherWeight;//sqrt(LonRange);
				Opacity *= FeatherWeight;
				} // if
			} // if
		else
			Opacity = 0.0;
		} // if
	else
		Opacity = 0.0;
	} // if
else
	Opacity = 0.0;

return (Opacity);

} // Texture::AdjustSampleForLatLonBounds

/*===========================================================================*/

int Texture::ComputeVectorOffsets(TextureData *Data)
{
double JoeElev;
float CurOffsets[3];
int OffEnd = 0, SplineLatLon, SplineElev, ConnectEnds, SkipFirst;

if (Data->VData[0])
	{
	SplineLatLon = (Data->VectorEffectType & WCS_TEXTURE_VECTOREFFECTTYPE_SPLINELATLON) ? 1: 0;
	SplineElev = (Data->VectorEffectType & WCS_TEXTURE_VECTOREFFECTTYPE_SPLINEELEV) ? 1: 0;
	ConnectEnds = (Data->VectorEffectType & WCS_TEXTURE_VECTOREFFECTTYPE_CONNECTENDS) ? 1: 0;
	SkipFirst = (Data->VectorEffectType & WCS_TEXTURE_VECTOREFFECTTYPE_SKIPFIRSTPOINT) ? 1: 0;
	Data->Vec->MinDistToPoint(Data->VData[0]->Lat, Data->VData[0]->Lon, Data->VData[0]->Elev, Data->MetersPerDegLat, Data->Exageration, 
		Data->Datum, SplineLatLon, SplineElev, ConnectEnds, SkipFirst, OffEnd, JoeElev, CurOffsets, NULL);
		Data->VData[0]->VecOffsets[0] = CurOffsets[0];
		Data->VData[0]->VecOffsets[1] = CurOffsets[1];
		Data->VData[0]->VecOffsets[2] = (float)(Data->VData[0]->Elev - JoeElev);
	if (Data->VData[1] && Data->VData[2])
		{
		Data->Vec->MinDistToPoint(Data->VData[1]->Lat, Data->VData[1]->Lon, Data->VData[1]->Elev, Data->MetersPerDegLat, Data->Exageration, 
			Data->Datum, SplineLatLon, SplineElev, ConnectEnds, SkipFirst, OffEnd, JoeElev, CurOffsets, NULL);
			Data->VData[1]->VecOffsets[0] = CurOffsets[0];
			Data->VData[1]->VecOffsets[1] = CurOffsets[1];
			Data->VData[1]->VecOffsets[2] = (float)(Data->VData[1]->Elev - JoeElev);
		Data->Vec->MinDistToPoint(Data->VData[2]->Lat, Data->VData[2]->Lon, Data->VData[2]->Elev, Data->MetersPerDegLat, Data->Exageration, 
			Data->Datum, SplineLatLon, SplineElev, ConnectEnds, SkipFirst, OffEnd, JoeElev, CurOffsets, NULL);
			Data->VData[2]->VecOffsets[0] = CurOffsets[0];
			Data->VData[2]->VecOffsets[1] = CurOffsets[1];
			Data->VData[2]->VecOffsets[2] = (float)(Data->VData[2]->Elev - JoeElev);
		} // if
	Data->VDataVecOffsetsValid = 1;
	return (1);
	} // if

return (0);

} // Texture::ComputeVectorOffsets

/*===========================================================================*/

// the way it used to be done
double Texture::AdjustSampleForAlignToVector(double &LowX, double &HighX, double &LowY, double &HighY, double &LowZ, double &HighZ, 
	TextureData *Data, TextureExtractionData *Extraction)
{
double Opacity = 1.0, DistX, DistY, DistZ;
int Ct;

if (Data->IsSampleThumbnail)
	{
	return (1.0);
	} // if
else if (! Data->Vec)
	return (0.0);

// determine distance to vector, along vector, and elevation difference for all the LatRange and LonRange values in Data
// x may be + for right of vector or - for left of vector
// y is always +
// z may be + or -

if (Data->VectorOffsetsComputed)
	{
	LowX = Data->VecOffsetPixel[0];
	HighX = Data->VecOffsetPixel[1];
	LowY = Data->VecOffsetPixel[2];
	HighY = Data->VecOffsetPixel[3];
	LowZ = Data->VecOffsetPixel[4];
	HighZ = Data->VecOffsetPixel[5];
	} // if
else if (Data->VectorEffectType & WCS_TEXTURE_VECTOREFFECTTYPE_LINE)
	{
	double JoeElev;
	float DistXYZ[3];
	int OffEnd = 0;

	LowX = FLT_MAX;
	HighX = -FLT_MAX;
	LowY = FLT_MAX;
	HighY = -FLT_MAX;
	LowZ = FLT_MAX;
	HighZ = -FLT_MAX;
	for (Ct = 0; Ct < 4; Ct ++)
		{
		// use the new V6 function for consistency
		Data->Vec->MinDistToPoint(Extraction->LatRange[Ct], Extraction->LonRange[Ct], Extraction->ElevRange[Ct], Data->MetersPerDegLat, 
			Data->Exageration, Data->Datum, 
			Data->VectorEffectType & WCS_TEXTURE_VECTOREFFECTTYPE_SPLINELATLON, 
			Data->VectorEffectType & WCS_TEXTURE_VECTOREFFECTTYPE_SPLINEELEV, 
			Data->VectorEffectType & WCS_TEXTURE_VECTOREFFECTTYPE_CONNECTENDS, 
			Data->VectorEffectType & WCS_TEXTURE_VECTOREFFECTTYPE_SKIPFIRSTPOINT, 
			OffEnd, 
			JoeElev, DistXYZ, NULL);
		DistX = DistXYZ[0];
		DistY = DistXYZ[1];
		DistZ = DistXYZ[2];
//		the old function call
//		Data->Vec->MinDistToPoint(Extraction->LatRange[Ct], Extraction->LonRange[Ct], Extraction->ElevRange[Ct], Data->MetersPerDegLat,
//			0, Data->Datum, Data->Exageration, DistX, DistY, DistZ);
		if (DistX < LowX)
			Data->VecOffsetPixel[0] = LowX = DistX;
		if (DistX > HighX)
			Data->VecOffsetPixel[1] = HighX = DistX;
		if (DistY < LowY)
			Data->VecOffsetPixel[2] = LowY = DistY;
		if (DistY > HighY)
			Data->VecOffsetPixel[3] = HighY = DistY;
		if (DistZ < LowZ)
			Data->VecOffsetPixel[4] = LowZ = DistZ;
		if (DistZ > HighZ)
			Data->VecOffsetPixel[5] = HighZ = DistZ;
		} // for
	Data->VectorOffsetsComputed = 1;
	} // if line effect
else
	{
	LowX = FLT_MAX;
	HighX = -FLT_MAX;
	Data->VecOffsetPixel[2] = LowY = 0.0;
	Data->VecOffsetPixel[3] = HighY = 0.0;
	LowZ = FLT_MAX;
	HighZ = -FLT_MAX;
	for (Ct = 0; Ct < 4; Ct ++)
		{
		Data->Vec->MinDistToPoint(Extraction->LatRange[Ct], Extraction->LonRange[Ct], Extraction->ElevRange[Ct], Data->MetersPerDegLat,
			1, Data->Datum, Data->Exageration, DistX, DistY, DistZ);
		if (DistX < LowX)
			Data->VecOffsetPixel[0] = LowX = DistX;
		if (DistX > HighX)
			Data->VecOffsetPixel[1] = HighX = DistX;
		if (DistZ < LowZ)
			Data->VecOffsetPixel[4] = LowZ = DistZ;
		if (DistZ > HighZ)
			Data->VecOffsetPixel[5] = HighZ = DistZ;
		} // for
	Data->VectorOffsetsComputed = 1;
	} //else area effect

return (Opacity);

} // Texture::AdjustSampleForAlignToVector

/*===========================================================================*/

double Texture::ObtainCylindricalX(double LowX, double HighX, double LowZ, double HighZ, double &PtXLow, double &PtXHigh)
{
double TempWrap, Wrap, Len, Opacity = 1.0;

Wrap = WrapWidth * (1.0 / 100.0);

if (Wrap <= 0.0)
	return (0.0);

if ((Len = LowX * LowX + LowZ * LowZ) > 0.0)
	{
	if (LowX > 0.0)
		PtXLow = TwoPi - GlobalApp->MathTables->ACosTab.Lookup(LowZ / sqrt(Len));
	else
		PtXLow = GlobalApp->MathTables->ACosTab.Lookup(LowZ / sqrt(Len));
	} // if
else
	PtXLow = Pi;

if ((Len = HighX * HighX + HighZ * HighZ) > 0.0)
	{
	if (HighX > 0.0)
		PtXHigh = TwoPi - GlobalApp->MathTables->ACosTab.Lookup(HighZ / sqrt(Len));
	else
		PtXHigh = GlobalApp->MathTables->ACosTab.Lookup(HighZ / sqrt(Len));
	} // if
else
	PtXHigh = Pi;

TempWrap = 1.0 / (TwoPi * Wrap);
PtXLow = (PtXLow - Pi * (1.0 - Wrap)) * TempWrap;
PtXHigh = (PtXHigh - Pi * (1.0 - Wrap)) * TempWrap;

if ((PtXLow < 0.0 && PtXHigh < 0.0) || (PtXLow > 1.0 && PtXHigh > 1.0))
	return (0.0);

if (PtXLow > PtXHigh)
	Swap64(PtXLow, PtXHigh);

if (PtXLow != PtXHigh)
	{
	if (PtXLow < 0.0)
		{
		Opacity *= (PtXHigh / (PtXHigh - PtXLow));
		PtXLow = 0.0;
		} // if
	if (PtXHigh > 1.0)
		{
		Opacity *= ((1.0 - PtXLow) / (PtXHigh - PtXLow));
		PtXHigh = 1.0;
		} // if
	} // if

return (Opacity);

} // Texture::ObtainCylindricalX

/*===========================================================================*/

double Texture::ObtainSphericalY(double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double &PtYLow, double &PtYHigh)
{
double TempWrap, Wrap, Len, Opacity = 1.0;

Wrap = WrapHeight * (1.0 / 100.0);

if (Wrap <= 0.0)
	return (0.0);

if ((Len = LowX * LowX + LowY * LowY + LowZ * LowZ) > 0.0)
	PtYLow = HalfPi + GlobalApp->MathTables->ASinTab.Lookup(LowY / sqrt(Len));
else
	PtYLow = HalfPi;
if ((Len = HighX * HighX + HighY * HighY + HighZ * HighZ) > 0.0)
	PtYHigh = HalfPi + GlobalApp->MathTables->ASinTab.Lookup(HighY / sqrt(Len));
else
	PtYHigh = HalfPi;

TempWrap = 1.0 / (Pi * Wrap);
PtYLow = (PtYLow - HalfPi * (1.0 - Wrap)) * TempWrap;
PtYHigh = (PtYHigh - HalfPi * (1.0 - Wrap)) * TempWrap;

if ((PtYLow < 0.0 && PtYHigh < 0.0) || (PtYLow > 1.0 && PtYHigh > 1.0))
	return (0.0);

if (PtYLow > PtYHigh)
	Swap64(PtYLow, PtYHigh);

if (PtYLow != PtYHigh)
	{
	if (PtYLow < 0.0)
		{
		Opacity *= (PtYHigh / (PtYHigh - PtYLow));
		PtYLow = 0.0;
		} // if
	if (PtYHigh > 1.0)
		{
		Opacity *= ((1.0 - PtYLow) / (PtYHigh - PtYLow));
		PtYHigh = 1.0;
		} // if
	} // if

return (Opacity);

} // Texture::ObtainSphericalY

/*===========================================================================*/

int Texture::EvalSampleInit(Raster *PlotRast, TextureSampleData *Samp)
{
double Value[3];
long x, ReadyGo;
unsigned char ByteVal;

Samp->PreviewDataPtr = Samp->XData = Samp->YData = Samp->ZData = Samp->IllumData = NULL;
Samp->Running = 0;
Samp->SampleInc = Samp->PreviewSize / (WCS_RASTER_TNAIL_SIZE - 1);
Samp->SampleStart = Samp->PreviewSize * .5;

if (! PlotRast->ThumbnailValid())
	ReadyGo = (long)PlotRast->AllocThumbnail();
else
	ReadyGo = 1;
Samp->Thumb = PlotRast->Thumb;

Samp->TexData.IsSampleThumbnail = 1;
Samp->TexData.ThumbnailX = Samp->TexData.ThumbnailY = 0.0;

if (ReadyGo && Enabled)
	{
	if (WCS_TEXTURETYPE_ISCURVE(TexType))
		{
		double inc, dtempx;
		long oldx, oldy = 0, tempx, tempy;

		PlotRast->ClearThumbnail();

		// paint in a gradient on both axes
		for (x = 0; x < WCS_RASTER_TNAIL_SIZE; x ++)
			{
			ByteVal = (unsigned char)(255.0 * ((double)x / (WCS_RASTER_TNAIL_SIZE - 1)));
			for (Samp->y = WCS_RASTER_TNAIL_SIZE - 6; Samp->y < WCS_RASTER_TNAIL_SIZE; Samp->y ++)
				{
				Samp->zip = Samp->y * WCS_RASTER_TNAIL_SIZE + x;
				Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip] = ByteVal;
				Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip] = ByteVal;
				Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip] = ByteVal;
				} // for
			} // for
		for (Samp->y = 0; Samp->y < WCS_RASTER_TNAIL_SIZE; Samp->y ++)
			{
			ByteVal = 255 - (unsigned char)(255.0 * ((double)Samp->y / (WCS_RASTER_TNAIL_SIZE - 1)));
			for (x = 0; x < 4; x ++)
				{
				Samp->zip = Samp->y * WCS_RASTER_TNAIL_SIZE + x;
				Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip] = ByteVal;
				Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip] = ByteVal;
				Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip] = ByteVal;
				} // for
			} // for

		oldx = -1;
		Samp->TexData.TLowX = Samp->TexData.TLowY = Samp->TexData.TLowZ = Samp->TexData.THighX = Samp->TexData.THighY = Samp->TexData.THighZ = 0.0;
		for (x = 0; x < WCS_RASTER_TNAIL_SIZE; x ++)
			{
			Value[0] = (double)x / (double)(WCS_RASTER_TNAIL_SIZE - 1);
			Value[1] = Value[2] = Value[0];
			Eval(Value, &Samp->TexData, FALSE, TRUE);
			Samp->y = quickftol((1.0 - Value[0]) * (WCS_RASTER_TNAIL_SIZE - 4));		// invert since thumbnail y is 0 at top
			Samp->zip = Samp->y * WCS_RASTER_TNAIL_SIZE + x;
			Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip] = 255;
			Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip] = 128;
			Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip] = 255;
			if (oldx >= 0)
				{
				if (abs(Samp->y - oldy) > 1)
					{
					inc = 1.0 / (double)(abs(Samp->y - oldy));	// inc always +
					if (Samp->y > oldy)
						{
						for (tempy = oldy + 1, dtempx = oldx + inc; tempy < Samp->y; tempy ++, dtempx += inc)
							{
							tempx = quickftol(dtempx);
							Samp->zip = tempy * WCS_RASTER_TNAIL_SIZE + tempx;
							Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip] = 255;
							Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip] = 128;
							Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip] = 255;
							} // for
						} // if
					else
						{
						for (tempy = oldy - 1, dtempx = oldx + inc; tempy > Samp->y; tempy --, dtempx += inc)
							{
							tempx = quickftol(dtempx);
							Samp->zip = tempy * WCS_RASTER_TNAIL_SIZE + tempx;
							Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip] = 255;
							Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip] = 128;
							Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip] = 255;
							} // for
						} // if
					} // if
				} // if
			oldx = x, oldy = Samp->y;
			} // for
		return (2);
		} // if
	else
		{
		BuildTextureRotationMatrix(Samp->AndChildren);

		Samp->StartX = Samp->StartY = Samp->StartZ = Samp->XHInc = Samp->XVInc = Samp->YHInc = Samp->YVInc = Samp->ZHInc = Samp->ZVInc = 0.0;

		switch (Samp->PreviewDirection)
			{
			case WCS_TEXTURE_PREVIEW_PLUSX:
				{
				if (ApplyToEcosys)
					{
					// -y left, +z top
					Samp->StartY = -Samp->SampleStart;
					Samp->YHInc = Samp->SampleInc;
					Samp->StartZ = Samp->SampleStart;
					Samp->ZVInc = -Samp->SampleInc;
					} // if
				else
					{
					// -z left, +y top
					Samp->StartY = Samp->SampleStart;
					Samp->YVInc = -Samp->SampleInc;
					Samp->StartZ = -Samp->SampleStart;
					Samp->ZHInc = Samp->SampleInc;
					} // else
				break;
				} // + X
			case WCS_TEXTURE_PREVIEW_PLUSY:
				{
				if (ApplyToEcosys)
					{
					// +x left, +z top
					Samp->StartX = Samp->SampleStart;
					Samp->XHInc = -Samp->SampleInc;
					Samp->StartZ = Samp->SampleStart;
					Samp->ZVInc = -Samp->SampleInc;
					} // if
				else
					{
					// -x left, +z top
					Samp->StartX = -Samp->SampleStart;
					Samp->XHInc = Samp->SampleInc;
					Samp->StartZ = Samp->SampleStart;
					Samp->ZVInc = -Samp->SampleInc;
					} // else
				break;
				} // + Y
			case WCS_TEXTURE_PREVIEW_PLUSZ:
				{
				if (ApplyToEcosys)
					{
					// -x left, +y top
					Samp->StartX = -Samp->SampleStart;
					Samp->XHInc = Samp->SampleInc;
					Samp->StartY = Samp->SampleStart;
					Samp->YVInc = -Samp->SampleInc;
					} // if
				else
					{
					// +x left, +y top
					Samp->StartX = Samp->SampleStart;
					Samp->XHInc = -Samp->SampleInc;
					Samp->StartY = Samp->SampleStart;
					Samp->YVInc = -Samp->SampleInc;
					} // else
				break;
				} // + Z
			case WCS_TEXTURE_PREVIEW_MINUSX:
				{
				if (ApplyToEcosys)
					{
					// +y left, +z top
					Samp->StartY = Samp->SampleStart;
					Samp->YHInc = -Samp->SampleInc;
					Samp->StartZ = Samp->SampleStart;
					Samp->ZVInc = -Samp->SampleInc;
					} // if
				else
					{
					// +z left, +y top
					Samp->StartY = Samp->SampleStart;
					Samp->YVInc = -Samp->SampleInc;
					Samp->StartZ = Samp->SampleStart;
					Samp->ZHInc = -Samp->SampleInc;
					} // else
				break;
				} // - X
			case WCS_TEXTURE_PREVIEW_MINUSY:
				{
				if (ApplyToEcosys)
					{
					// -x left, +z top
					Samp->StartX = -Samp->SampleStart;
					Samp->XHInc = Samp->SampleInc;
					Samp->StartZ = Samp->SampleStart;
					Samp->ZVInc = -Samp->SampleInc;
					} // if
				else
					{
					// +x left, +z top
					Samp->StartX = Samp->SampleStart;
					Samp->XHInc = -Samp->SampleInc;
					Samp->StartZ = Samp->SampleStart;
					Samp->ZVInc = -Samp->SampleInc;
					} // else
				break;
				} // - Y
			case WCS_TEXTURE_PREVIEW_MINUSZ:
				{
				if (ApplyToEcosys)
					{
					// +x left, +y top
					Samp->StartX = Samp->SampleStart;
					Samp->XHInc = -Samp->SampleInc;
					Samp->StartY = Samp->SampleStart;
					Samp->YVInc = -Samp->SampleInc;
					} // if
				else
					{
					// -x left, +y top
					Samp->StartX = -Samp->SampleStart;
					Samp->XHInc = Samp->SampleInc;
					Samp->StartY = Samp->SampleStart;
					Samp->YVInc = -Samp->SampleInc;
					} // else
				break;
				} // - Z
			case WCS_TEXTURE_PREVIEW_CUBE:
				{
				if (! RootTexture::CubePreviewLoaded())
					RootTexture::LoadPreviewFile(WCS_TEXTURE_PREVIEW_CUBE);
				if (! RootTexture::CubePreviewLoaded())
					{
					UserMessageOK("Texture Preview", "Unable to load Cube Preview file!");
					return (0);
					} // if
				Samp->PreviewDataPtr = RootTexture::CubePreviewData;
				break;
				} // Cube
			case WCS_TEXTURE_PREVIEW_SPHERE:
				{
				if (! RootTexture::SpherePreviewLoaded())
					RootTexture::LoadPreviewFile(WCS_TEXTURE_PREVIEW_SPHERE);
				if (! RootTexture::SpherePreviewLoaded())
					{
					UserMessageOK("Texture Preview", "Unable to load Sphere Preview file!");
					return (0);
					} // if
				Samp->PreviewDataPtr = RootTexture::SpherePreviewData;
				break;
				} // Cube
			} // switch

		if (! Samp->PreviewDataPtr)
			{
			Samp->TexData.SampleType = WCS_SAMPLETEXTURE_FLAT;
			Samp->XInc = fabs(Samp->XVInc) > fabs(Samp->XHInc) ? Samp->XVInc: Samp->XHInc;
			Samp->YInc = fabs(Samp->YVInc) > fabs(Samp->YHInc) ? Samp->YVInc: Samp->YHInc;
			Samp->ZInc = fabs(Samp->ZVInc) > fabs(Samp->ZHInc) ? Samp->ZVInc: Samp->ZHInc;
			} // if not cube or sphere
		else
			{
			Samp->TexData.SampleType = WCS_SAMPLETEXTURE_PROJECTED;
			Samp->XData = Samp->PreviewDataPtr;
			Samp->YData = &Samp->PreviewDataPtr[WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE];
			Samp->ZData = &Samp->PreviewDataPtr[2 * WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE];
			Samp->IllumData = &Samp->PreviewDataPtr[3 * WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE];
			} // else
		} // else
	Samp->y = Samp->zip = 0;
	Samp->Running = 1;
	return (1);
	} // if

return (0);

} // Texture::EvalSampleInit

/*===========================================================================*/

int Texture::EvalOneSampleLine(TextureSampleData *Samp)
{
double Illum, Value[3];
long x, CurTexType;
int StrataBlend, rVal;

// clear the next row of data
memset(&Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip], 0, WCS_RASTER_TNAIL_SIZE);
memset(&Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip], 0, WCS_RASTER_TNAIL_SIZE);
memset(&Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip], 0, WCS_RASTER_TNAIL_SIZE);
Samp->TexData.ThumbnailX = 0.0;

if (! Samp->PreviewDataPtr)
	{
	CurTexType = GetTexType();
	StrataBlend = 0;	//(! Prev && ! WCS_TEXTURETYPE_BLENDILLEGAL(CurTexType));
	for (x = 0, Samp->TexData.TLowX = Samp->StartX, Samp->TexData.TLowY = Samp->StartY, Samp->TexData.TLowZ = Samp->StartZ; 
		x < WCS_RASTER_TNAIL_SIZE; 
		x ++, Samp->zip ++, Samp->TexData.TLowX += Samp->XHInc, Samp->TexData.TLowY += Samp->YHInc, Samp->TexData.TLowZ += Samp->ZHInc)
		{
		Samp->TexData.THighX = Samp->TexData.TLowX + Samp->XInc;
		Samp->TexData.THighY = Samp->TexData.TLowY + Samp->YInc;
		Samp->TexData.THighZ = Samp->TexData.TLowZ + Samp->ZInc;
		Value[0] = Value[1] = Value[2] = 0.0;
		if ((Eval(Value, &Samp->TexData, Samp->AndChildren, StrataBlend)) > 0.0)
			{
			if (ApplyToColor)
				{
				if (Value[WCS_RASTER_IMAGE_BAND_RED] > 1.0)
					Value[WCS_RASTER_IMAGE_BAND_RED] = 1.0;
				if (Value[WCS_RASTER_IMAGE_BAND_GREEN] > 1.0)
					Value[WCS_RASTER_IMAGE_BAND_GREEN] = 1.0;
				if (Value[WCS_RASTER_IMAGE_BAND_BLUE] > 1.0)
					Value[WCS_RASTER_IMAGE_BAND_BLUE] = 1.0;
				Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip] += (UBYTE)(255.99 * Value[WCS_RASTER_IMAGE_BAND_RED]);
				Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip] += (UBYTE)(255.99 * Value[WCS_RASTER_IMAGE_BAND_GREEN]);
				Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip] += (UBYTE)(255.99 * Value[WCS_RASTER_IMAGE_BAND_BLUE]);
				} // if
			else
				{
				if (AbsoluteOutput)
					{
					if (TexParam[WCS_TEXTURE_HIGH].CurValue != TexParam[WCS_TEXTURE_LOW].CurValue)
						Value[WCS_RASTER_IMAGE_BAND_RED] = 255.99 * (Value[WCS_RASTER_IMAGE_BAND_RED] - TexParam[WCS_TEXTURE_LOW].CurValue) / (TexParam[WCS_TEXTURE_HIGH].CurValue - TexParam[WCS_TEXTURE_LOW].CurValue);
					else
						Value[WCS_RASTER_IMAGE_BAND_RED] = 0.0;
					} // if
				else
					{
					Value[WCS_RASTER_IMAGE_BAND_RED] *= 255.99;
					} // else
				if (Value[WCS_RASTER_IMAGE_BAND_RED] > 255.0)
					Value[WCS_RASTER_IMAGE_BAND_RED] = 255.0;
				Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip] += (UBYTE)(Value[WCS_RASTER_IMAGE_BAND_RED]);
				Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip] =
					Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip] = Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip];
				} // else
			} // if > 0
		Samp->TexData.ThumbnailX += 1.0 / (WCS_RASTER_TNAIL_SIZE - 1);
		} // for
	Samp->StartX += Samp->XVInc;
	Samp->StartY += Samp->YVInc;
	Samp->StartZ += Samp->ZVInc;
	} // if not cube or sphere
else
	{
	CurTexType = GetTexType();
	StrataBlend = 0;	//(! Prev && ! WCS_TEXTURETYPE_BLENDILLEGAL(CurTexType));
	for (x = 0; x < WCS_RASTER_TNAIL_SIZE; x ++, Samp->zip ++)
		{
		Illum = Samp->IllumData[Samp->zip];
		if (Illum > .01)
			{
			Samp->TexData.TLowX = Samp->XData[Samp->zip] * Samp->PreviewSize;
			Samp->TexData.THighX = Samp->XData[Samp->zip + WCS_RASTER_TNAIL_SIZE + 1] * Samp->PreviewSize;
			if (ApplyToEcosys)
				{
				Samp->TexData.TLowZ = Samp->YData[Samp->zip] * Samp->PreviewSize;
				Samp->TexData.TLowY = Samp->ZData[Samp->zip] * Samp->PreviewSize;
				Samp->TexData.THighZ = Samp->YData[Samp->zip + WCS_RASTER_TNAIL_SIZE + 1] * Samp->PreviewSize;
				Samp->TexData.THighY = Samp->ZData[Samp->zip + WCS_RASTER_TNAIL_SIZE + 1] * Samp->PreviewSize;
				} // if
			else
				{
				Samp->TexData.TLowY = Samp->YData[Samp->zip] * Samp->PreviewSize;
				Samp->TexData.TLowZ = Samp->ZData[Samp->zip] * Samp->PreviewSize;
				Samp->TexData.THighY = Samp->YData[Samp->zip + WCS_RASTER_TNAIL_SIZE + 1] * Samp->PreviewSize;
				Samp->TexData.THighZ = Samp->ZData[Samp->zip + WCS_RASTER_TNAIL_SIZE + 1] * Samp->PreviewSize;
				} // else
			Value[0] = Value[1] = Value[2] = 0.0;
			if ((Eval(Value, &Samp->TexData, Samp->AndChildren, StrataBlend)) > 0.0)
				{
				if (ApplyToColor)
					{
					if (Value[WCS_RASTER_IMAGE_BAND_RED] > 1.0)
						Value[WCS_RASTER_IMAGE_BAND_RED] = 1.0;
					if (Value[WCS_RASTER_IMAGE_BAND_GREEN] > 1.0)
						Value[WCS_RASTER_IMAGE_BAND_GREEN] = 1.0;
					if (Value[WCS_RASTER_IMAGE_BAND_BLUE] > 1.0)
						Value[WCS_RASTER_IMAGE_BAND_BLUE] = 1.0;
					Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip] += (UBYTE)(255.99 * Value[WCS_RASTER_IMAGE_BAND_RED] * Illum);
					Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip] += (UBYTE)(255.99 * Value[WCS_RASTER_IMAGE_BAND_GREEN] * Illum);
					Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip] += (UBYTE)(255.99 * Value[WCS_RASTER_IMAGE_BAND_BLUE] * Illum);
					} // if
				else
					{
					if (AbsoluteOutput)
						{
						if (TexParam[WCS_TEXTURE_HIGH].CurValue != TexParam[WCS_TEXTURE_LOW].CurValue)
							Value[WCS_RASTER_IMAGE_BAND_RED] = 255.99 * (Value[WCS_RASTER_IMAGE_BAND_RED] - TexParam[WCS_TEXTURE_LOW].CurValue) / (TexParam[WCS_TEXTURE_HIGH].CurValue - TexParam[WCS_TEXTURE_LOW].CurValue);
						else
							Value[WCS_RASTER_IMAGE_BAND_RED] = 0.0;
						} // if
					else
						{
						Value[WCS_RASTER_IMAGE_BAND_RED] *= 255.99;
						} // else
					if (Value[WCS_RASTER_IMAGE_BAND_RED] > 255.0)
						Value[WCS_RASTER_IMAGE_BAND_RED] = 255.0;
					Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip] += (UBYTE)(Value[WCS_RASTER_IMAGE_BAND_RED] * Illum);
					Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip] =
						Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip] = Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip];
					} // else
				} // if > 0
			} // if pixel worth sampling
		Samp->TexData.ThumbnailX += 1.0 / (WCS_RASTER_TNAIL_SIZE - 1);
		} // for
	} // else

Samp->y ++;
Samp->TexData.ThumbnailY += 1.0 / (WCS_RASTER_TNAIL_SIZE - 1);

if (Samp->y < WCS_RASTER_TNAIL_SIZE)
	rVal = 0;
else
	{
	Samp->Running = 0;
	rVal = 1;
	} // else

return (rVal);

} // Texture::EvalOneSampleLine

/*===========================================================================*/

double Texture::Eval(double Output[3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Opacity, ParamOpacity, SOp, ParamVal[WCS_TEXTURE_MAXPARAMTEXTURES][3], Falloff[3], AvgDist, 
	sLowX, sHighX, sLowY, sHighY, sLowZ, sHighZ, SOpSum, XRange, YRange, ZRange,
	/*MaxR, MinR, MaxG, MinG, MaxB, MinB,*/ tLowX, tLowY, tLowZ, tHighX, tHighY, tHighZ;
double TestValue[3];
unsigned long Ct;
int TexRotated = 0, ParamCt;
TextureExtractionData Extraction(Data);

if (EvalChildren && Tex[WCS_TEXTURE_OPACITY] && Tex[WCS_TEXTURE_OPACITY]->Enabled)
	{
	// LINT complains about ParamVal being uninitialized. It gets initialized by the Eval function.
	Opacity = Tex[WCS_TEXTURE_OPACITY]->Eval(ParamVal[WCS_TEXTURE_OPACITY], Data, TRUE, FALSE);
	if (Opacity < 1.0)
		{
		Opacity = (TexParam[WCS_TEXTURE_OPACITY].CurValue * (1.0 / 100.0)) * (1.0 - Opacity) + ParamVal[WCS_TEXTURE_OPACITY][0];
		} // if
	else
		{
		Opacity = ParamVal[WCS_TEXTURE_OPACITY][0];
		} // else
	} // if
else
	{
	Opacity = TexParam[WCS_TEXTURE_OPACITY].CurValue * (1.0 / 100.0);
	} // else

if (Opacity <= 0.0)
	{
	Output[0] = Output[1] = Output[2] = 0.0;
	return (0.0);
	} // if

if (! Data->IsSampleThumbnail)
	{
	if ((Opacity = Opacity * ExtractTexDataValues(Data, &Extraction)) <= 0.0)
		{
		Output[0] = Output[1] = Output[2] = 0.0;
		return (0.0);
		} // if
	} // if
sLowX = Extraction.LowX;
sHighX = Extraction.HighX;
sLowY = Extraction.LowY;
sHighY = Extraction.HighY;
sLowZ = Extraction.LowZ;
sHighZ = Extraction.HighZ;

// center
for (Ct = WCS_TEXTURE_MAXPARAMS + 7, ParamCt = 0; ParamCt < 3; Ct ++, ParamCt ++)
	{
	if (EvalChildren && Tex[Ct] && Tex[Ct]->Enabled)
		{
		ParamOpacity = Tex[Ct]->Eval(TestValue, Data, TRUE, FALSE);
		if (ParamOpacity < 1.0)
			TCenter[ParamCt] = TestValue[0] + TexCenter[ParamCt].CurValue * (1.0 - ParamOpacity);
		else
			TCenter[ParamCt] = TestValue[0];
		} // if
	else
		{
		TCenter[ParamCt] = TexCenter[ParamCt].CurValue;
		} // else
	} // for
// falloff
for (Ct = WCS_TEXTURE_MAXPARAMS + 10, ParamCt = 0; ParamCt < 3; Ct ++, ParamCt ++)
	{
	if (EvalChildren && Tex[Ct] && Tex[Ct]->Enabled)
		{
		ParamOpacity = Tex[Ct]->Eval(TestValue, Data, TRUE, FALSE);
		if (ParamOpacity < 1.0)
			TFalloff[ParamCt] = TestValue[0] + TexFalloff[ParamCt].CurValue * (1.0 - ParamOpacity);
		else
			TFalloff[ParamCt] = TestValue[0];
		} // if
	else
		{
		TFalloff[ParamCt] = TexFalloff[ParamCt].CurValue;
		} // else
	} // for

if (TFalloff[0] > 0.0 || TFalloff[1] > 0.0 || TFalloff[2] > 0.0)
	{
	if (CoordSpace != WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
		{
		AvgDist = ((sLowX + sHighX) * .5) - TCenter[0];
		Falloff[0] = AvgDist * TFalloff[0] * (1.0 / 100.0);
		AvgDist = ((sLowY + sHighY) * .5) - TCenter[1];
		Falloff[1] = AvgDist * TFalloff[1] * (1.0 / 100.0);
		AvgDist = ((sLowZ + sHighZ) * .5) - TCenter[2];
		Falloff[2] = AvgDist * TFalloff[2] * (1.0 / 100.0);
		if (Falloff[0] >= 1.0 || Falloff[1] >= 1.0 || Falloff[2] >= 1.0)
			{
			Opacity = 0.0;
			} // if
		else
			{
			Falloff[0] = sqrt(Falloff[0] * Falloff[0] + Falloff[1] * Falloff[1] + Falloff[2] * Falloff[2]);
			if (Falloff[0] >= 1.0)
				{
				Opacity = 0.0;
				} // if
			else
				{
				Opacity *= (1.0 - Falloff[0]);
				} // else
			} // else
		} // if
	} // if

// param values and colors
if (Opacity > 0.0)
	{
	// this is very slow - too little work in most iterations of the loop
	//#pragma omp parallel for private(ParamOpacity)
	for (Ct = WCS_TEXTURE_OPACITY + 1; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
		{
		if (ParamAvailable(Ct))
			{
			if (EvalChildren && Tex[Ct] && Tex[Ct]->Enabled)
				{
				ParamOpacity = Tex[Ct]->Eval(ParamVal[Ct], Data, TRUE, FALSE);
				if (ParamOpacity < 1.0)
					{
					// very few applications are color: HSVMerge, for instance
					if (! Tex[Ct]->ApplyToColor)
						{
						if (ParamFlags[Ct] & WCS_TEXTURE_PARAMPERCENT)
							ParamVal[Ct][0] += ((TexParam[Ct].CurValue * (1.0 / 100.0)) * (1.0 - ParamOpacity));
						else
							ParamVal[Ct][0] += ((TexParam[Ct].CurValue) * (1.0 - ParamOpacity));
						} // if
					} // if
				} // if
			else
				{
				if (ParamFlags[Ct] & WCS_TEXTURE_PARAMPERCENT)
					ParamVal[Ct][2] = ParamVal[Ct][1] = ParamVal[Ct][0] = TexParam[Ct].CurValue * (1.0 / 100.0);
				else
					ParamVal[Ct][2] = ParamVal[Ct][1] = ParamVal[Ct][0] = TexParam[Ct].CurValue;
				} // else
			} // if
		} // for

	// size
	for (Ct = WCS_TEXTURE_MAXPARAMS + 4, ParamCt = 0; ParamCt < 3; Ct ++, ParamCt ++)
		{
		if (EvalChildren && Tex[Ct] && Tex[Ct]->Enabled)
			{
			ParamOpacity = Tex[Ct]->Eval(TestValue, Data, TRUE, FALSE);
			if (ParamOpacity < 1.0)
				TSize[ParamCt] = TestValue[0] + TexSize[ParamCt].CurValue * (1.0 - ParamOpacity);
			else
				TSize[ParamCt] = TestValue[0];
			} // if
		else
			{
			TSize[ParamCt] = TexSize[ParamCt].CurValue;
			} // else
		} // for
	// rotation
	for (Ct = WCS_TEXTURE_MAXPARAMS + 13, ParamCt = 0; ParamCt < 3; Ct ++, ParamCt ++)
		{
		if (EvalChildren && Tex[Ct] && Tex[Ct]->Enabled)
			{
			ParamOpacity = Tex[Ct]->Eval(TestValue, Data, TRUE, FALSE);
			if (ParamOpacity < 1.0)
				TRotation[ParamCt] = TestValue[0] + TexRotation[ParamCt].CurValue * (1.0 - ParamOpacity);
			else
				TRotation[ParamCt] = TestValue[0];
			TexRotated = 1;
			} // if
		else
			{
			TRotation[ParamCt] = TexRotation[ParamCt].CurValue;
			} // else
		} // for
	if (TexRotated)
		BuildTextureRotationMatrixT();

	if (StrataBlend)
		{
		ParamVal[WCS_TEXTURE_LOW][0] = Output[0];
		ParamVal[WCS_TEXTURE_LOW][1] = Output[1];
		ParamVal[WCS_TEXTURE_LOW][2] = Output[2];
		ParamVal[WCS_TEXTURE_COLOR1][0] = Output[0];
		ParamVal[WCS_TEXTURE_COLOR1][1] = Output[1];
		ParamVal[WCS_TEXTURE_COLOR1][2] = Output[2];
		} // if

	// analyze the derived texture using a virtual function
	if (! AASampler || WCS_SinglePoint3D(sLowX, sHighX, sLowY, sHighY, sLowZ, sHighZ))
		{
		SOp = Analyze(Output, sLowX, sHighX, sLowY, sHighY, sLowZ, sHighZ, ParamVal, Data, EvalChildren, StrataBlend);
		} // if
	else
		{
		double SumOutput0, SumOutput1, SumOutput2;
		int BaseRNG;

		SumOutput0 = SumOutput1 = SumOutput2 = 0.0;
		SOpSum = 0.0;
		XRange = sHighX - sLowX;
		YRange = sHighY - sLowY;
		ZRange = sHighZ - sLowZ;
		//MaxR = 0.0; MinR = 1.0;
		//MaxG = 0.0; MinG = 1.0;
		//MaxB = 0.0; MinB = 1.0;
		if (! Data->SampleType)
			RootTexture::TextureRand.Seed64((unsigned long)Data->PixelX[0], (unsigned long)Data->PixelY[0] * 47);
		BaseRNG = (int)(RootTexture::TextureRand.GenPRN() * 48.0);
		//#pragma omp parallel for private(tLowX, tLowY, tLowZ, tHighX, tHighY, tHighZ) reduction(+: SOpSum, SumOutput0, SumOutput1, SumOutput2)
		for (Ct = 0; Ct < AASamples; Ct ++)
			{
			tLowX = TextureAA::TextureAARNGTable[BaseRNG + Ct] * XRange + sLowX;
			tLowY = TextureAA::TextureAARNGTable[BaseRNG + Ct + 1] * YRange + sLowY;
			tLowZ = TextureAA::TextureAARNGTable[BaseRNG + Ct + 2] * ZRange + sLowZ;
			tHighX = TextureAA::TextureAARNGTable[BaseRNG + Ct + 3] * XRange + sLowX;
			tHighY = TextureAA::TextureAARNGTable[BaseRNG + Ct + 4] * YRange + sLowY;
			tHighZ = TextureAA::TextureAARNGTable[BaseRNG + Ct + 5] * ZRange + sLowZ;
			SOpSum += Analyze(Output, tLowX, tHighX, tLowY, tHighY, tLowZ, tHighZ, ParamVal, Data, EvalChildren, StrataBlend);
			SumOutput0 += Output[0];
			SumOutput1 += Output[1];
			SumOutput2 += Output[2];
			//if (Output[0] > MaxR)
			//	MaxR = Output[0];
			//if (Output[0] < MinR)
			//	MinR = Output[0];
			//if (Output[1] > MaxG)
			//	MaxG = Output[1];
			//if (Output[1] < MinG)
			//	MinG = Output[1];
			//if (Output[2] > MaxB)
			//	MaxB = Output[2];
			//if (Output[2] < MinB)
			//	MinB = Output[2];
			} // for
		Output[0] = SumOutput0 / AASamples;
		Output[1] = SumOutput1 / AASamples;
		Output[2] = SumOutput2 / AASamples;
		SOp = SOpSum / AASamples;
		//AASampler->ProcessSamplePoints(Output, MinR, MaxR, MinG, MaxG, MinB, MaxB);
		} // else

	// fill values in Value[], multiply by opacity
	if (SelfOpacity && SOp < 1.0)
		{
		Opacity *= SOp;
		} // if
	Output[0] *= Opacity;
	Output[1] *= Opacity;
	Output[2] *= Opacity;
	} // if
else
	Output[2] = Output[1] = Output[0] = 0.0;

return (Opacity);

} // Texture::Eval

/*===========================================================================*/

void Texture::EvalStrataBlend(double &Value, TextureData *Data)
{
double BlendValue[3], BlendOpacity;

if (Tex[WCS_TEXTURE_STRATAFUNC] && Tex[WCS_TEXTURE_STRATAFUNC]->Enabled)
	{
	BlendValue[0] = Value;
	BlendValue[1] = BlendValue[2] = 0.0;	// not necessary but good form
	if ((BlendOpacity = Tex[WCS_TEXTURE_STRATAFUNC]->Eval(BlendValue, Data, TRUE, TRUE)) < 1.0)
		{
		Value = BlendValue[0] + ((1.0 - BlendOpacity) * Value);
		} // if
	else
		{
		Value = BlendValue[0];
		} // else
	} // if
if (Tex[WCS_TEXTURE_BLENDINGFUNC] && Tex[WCS_TEXTURE_BLENDINGFUNC]->Enabled)
	{
	BlendValue[0] = Value;
	BlendValue[2] = BlendValue[1] = 0.0;	// not necessary but good form
	if ((BlendOpacity = Tex[WCS_TEXTURE_BLENDINGFUNC]->Eval(BlendValue, Data, TRUE, TRUE)) < 1.0)
		{
		Value = BlendValue[0] + ((1.0 - BlendOpacity) * Value);
		} // if
	else
		{
		Value = BlendValue[0];
		} // else
	} // if

} // Texture::EvalStrataBlend

/*===========================================================================*/

double Texture::ExtractTexDataValues(TextureData *Data, TextureExtractionData *Extraction)
{
double RangeX[4], RangeY[4], RangeZ[4], TempOpacity, DataOffset, rVal = 0.0;
ObjectPerVertexMap *VertexMap;
long MapIndex[3];
unsigned char MapType, MapNumber;

// this generates some optimal assembly language - don't ask :)
goto etdv;

r1:
rVal = 1.0;

exit:
return (rVal);

etdv:
if (Data->VDEM[0] && Data->VDEM[1] && Data->VDEM[2])
	{
	switch (CoordSpace)
		{
		case WCS_TEXTURE_COORDSPACE_NONE:
			{
			//if (TexType == WCS_TEXTURE_TYPE_TERRAINPARAM)
			//	{
			//	return (ExtractDynamicValues(Data));
			//	} // if
			goto r1;
			} // WCS_TEXTURE_COORDSPACE_NONE
		case WCS_TEXTURE_COORDSPACE_OBJECT_CARTESIAN:
			{
			RangeX[0] = (Data->VtxPixCornerWt[0][0] * Data->VDEM[0]->xyz[0] + Data->VtxPixCornerWt[0][1] * Data->VDEM[1]->xyz[0] + Data->VtxPixCornerWt[0][2] * Data->VDEM[2]->xyz[0]);
			RangeY[0] = (Data->VtxPixCornerWt[0][0] * Data->VDEM[0]->xyz[1] + Data->VtxPixCornerWt[0][1] * Data->VDEM[1]->xyz[1] + Data->VtxPixCornerWt[0][2] * Data->VDEM[2]->xyz[1]);
			RangeZ[0] = (Data->VtxPixCornerWt[0][0] * Data->VDEM[0]->xyz[2] + Data->VtxPixCornerWt[0][1] * Data->VDEM[1]->xyz[2] + Data->VtxPixCornerWt[0][2] * Data->VDEM[2]->xyz[2]);

			RangeX[1] = (Data->VtxPixCornerWt[1][0] * Data->VDEM[0]->xyz[0] + Data->VtxPixCornerWt[1][1] * Data->VDEM[1]->xyz[0] + Data->VtxPixCornerWt[1][2] * Data->VDEM[2]->xyz[0]);
			RangeY[1] = (Data->VtxPixCornerWt[1][0] * Data->VDEM[0]->xyz[1] + Data->VtxPixCornerWt[1][1] * Data->VDEM[1]->xyz[1] + Data->VtxPixCornerWt[1][2] * Data->VDEM[2]->xyz[1]);
			RangeZ[1] = (Data->VtxPixCornerWt[1][0] * Data->VDEM[0]->xyz[2] + Data->VtxPixCornerWt[1][1] * Data->VDEM[1]->xyz[2] + Data->VtxPixCornerWt[1][2] * Data->VDEM[2]->xyz[2]);

			RangeX[2] = (Data->VtxPixCornerWt[2][0] * Data->VDEM[0]->xyz[0] + Data->VtxPixCornerWt[2][1] * Data->VDEM[1]->xyz[0] + Data->VtxPixCornerWt[2][2] * Data->VDEM[2]->xyz[0]);
			RangeY[2] = (Data->VtxPixCornerWt[2][0] * Data->VDEM[0]->xyz[1] + Data->VtxPixCornerWt[2][1] * Data->VDEM[1]->xyz[1] + Data->VtxPixCornerWt[2][2] * Data->VDEM[2]->xyz[1]);
			RangeZ[2] = (Data->VtxPixCornerWt[2][0] * Data->VDEM[0]->xyz[2] + Data->VtxPixCornerWt[2][1] * Data->VDEM[1]->xyz[2] + Data->VtxPixCornerWt[2][2] * Data->VDEM[2]->xyz[2]);

			RangeX[3] = (Data->VtxPixCornerWt[3][0] * Data->VDEM[0]->xyz[0] + Data->VtxPixCornerWt[3][1] * Data->VDEM[1]->xyz[0] + Data->VtxPixCornerWt[3][2] * Data->VDEM[2]->xyz[0]);
			RangeY[3] = (Data->VtxPixCornerWt[3][0] * Data->VDEM[0]->xyz[1] + Data->VtxPixCornerWt[3][1] * Data->VDEM[1]->xyz[1] + Data->VtxPixCornerWt[3][2] * Data->VDEM[2]->xyz[1]);
			RangeZ[3] = (Data->VtxPixCornerWt[3][0] * Data->VDEM[0]->xyz[2] + Data->VtxPixCornerWt[3][1] * Data->VDEM[1]->xyz[2] + Data->VtxPixCornerWt[3][2] * Data->VDEM[2]->xyz[2]);
			break;
			} // WCS_TEXTURE_COORDSPACE_OBJECT_CARTESIAN
		case WCS_TEXTURE_COORDSPACE_GLOBAL_CARTESIAN:
			{
			RangeX[0] = (Data->VtxPixCornerWt[0][0] * Data->VDEM[0]->XYZ[0] + Data->VtxPixCornerWt[0][1] * Data->VDEM[1]->XYZ[0] + Data->VtxPixCornerWt[0][2] * Data->VDEM[2]->XYZ[0]);
			RangeY[0] = (Data->VtxPixCornerWt[0][0] * Data->VDEM[0]->XYZ[1] + Data->VtxPixCornerWt[0][1] * Data->VDEM[1]->XYZ[1] + Data->VtxPixCornerWt[0][2] * Data->VDEM[2]->XYZ[1]);
			RangeZ[0] = (Data->VtxPixCornerWt[0][0] * Data->VDEM[0]->XYZ[2] + Data->VtxPixCornerWt[0][1] * Data->VDEM[1]->XYZ[2] + Data->VtxPixCornerWt[0][2] * Data->VDEM[2]->XYZ[2]);

			RangeX[1] = (Data->VtxPixCornerWt[1][0] * Data->VDEM[0]->XYZ[0] + Data->VtxPixCornerWt[1][1] * Data->VDEM[1]->XYZ[0] + Data->VtxPixCornerWt[1][2] * Data->VDEM[2]->XYZ[0]);
			RangeY[1] = (Data->VtxPixCornerWt[1][0] * Data->VDEM[0]->XYZ[1] + Data->VtxPixCornerWt[1][1] * Data->VDEM[1]->XYZ[1] + Data->VtxPixCornerWt[1][2] * Data->VDEM[2]->XYZ[1]);
			RangeZ[1] = (Data->VtxPixCornerWt[1][0] * Data->VDEM[0]->XYZ[2] + Data->VtxPixCornerWt[1][1] * Data->VDEM[1]->XYZ[2] + Data->VtxPixCornerWt[1][2] * Data->VDEM[2]->XYZ[2]);

			RangeX[2] = (Data->VtxPixCornerWt[2][0] * Data->VDEM[0]->XYZ[0] + Data->VtxPixCornerWt[2][1] * Data->VDEM[1]->XYZ[0] + Data->VtxPixCornerWt[2][2] * Data->VDEM[2]->XYZ[0]);
			RangeY[2] = (Data->VtxPixCornerWt[2][0] * Data->VDEM[0]->XYZ[1] + Data->VtxPixCornerWt[2][1] * Data->VDEM[1]->XYZ[1] + Data->VtxPixCornerWt[2][2] * Data->VDEM[2]->XYZ[1]);
			RangeZ[2] = (Data->VtxPixCornerWt[2][0] * Data->VDEM[0]->XYZ[2] + Data->VtxPixCornerWt[2][1] * Data->VDEM[1]->XYZ[2] + Data->VtxPixCornerWt[2][2] * Data->VDEM[2]->XYZ[2]);

			RangeX[3] = (Data->VtxPixCornerWt[3][0] * Data->VDEM[0]->XYZ[0] + Data->VtxPixCornerWt[3][1] * Data->VDEM[1]->XYZ[0] + Data->VtxPixCornerWt[3][2] * Data->VDEM[2]->XYZ[0]);
			RangeY[3] = (Data->VtxPixCornerWt[3][0] * Data->VDEM[0]->XYZ[1] + Data->VtxPixCornerWt[3][1] * Data->VDEM[1]->XYZ[1] + Data->VtxPixCornerWt[3][2] * Data->VDEM[2]->XYZ[1]);
			RangeZ[3] = (Data->VtxPixCornerWt[3][0] * Data->VDEM[0]->XYZ[2] + Data->VtxPixCornerWt[3][1] * Data->VDEM[1]->XYZ[2] + Data->VtxPixCornerWt[3][2] * Data->VDEM[2]->XYZ[2]);
			break;
			} // WCS_TEXTURE_COORDSPACE_GLOBAL_CARTESIAN
		case WCS_TEXTURE_COORDSPACE_PROJECT_GEOGRXYZ:
		case WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED:
			{
			Extraction->LatRange[0] = (Data->VtxPixCornerWt[0][0] * Data->VDEM[0]->Lat + Data->VtxPixCornerWt[0][1] * Data->VDEM[1]->Lat + Data->VtxPixCornerWt[0][2] * Data->VDEM[2]->Lat);
			Extraction->LonRange[0] = (Data->VtxPixCornerWt[0][0] * Data->VDEM[0]->Lon + Data->VtxPixCornerWt[0][1] * Data->VDEM[1]->Lon + Data->VtxPixCornerWt[0][2] * Data->VDEM[2]->Lon);
			Extraction->ElevRange[0] = (Data->VtxPixCornerWt[0][0] * Data->VDEM[0]->Elev + Data->VtxPixCornerWt[0][1] * Data->VDEM[1]->Elev + Data->VtxPixCornerWt[0][2] * Data->VDEM[2]->Elev);

			Extraction->LatRange[1] = (Data->VtxPixCornerWt[1][0] * Data->VDEM[0]->Lat + Data->VtxPixCornerWt[1][1] * Data->VDEM[1]->Lat + Data->VtxPixCornerWt[1][2] * Data->VDEM[2]->Lat);
			Extraction->LonRange[1] = (Data->VtxPixCornerWt[1][0] * Data->VDEM[0]->Lon + Data->VtxPixCornerWt[1][1] * Data->VDEM[1]->Lon + Data->VtxPixCornerWt[1][2] * Data->VDEM[2]->Lon);
			Extraction->ElevRange[1] = (Data->VtxPixCornerWt[1][0] * Data->VDEM[0]->Elev + Data->VtxPixCornerWt[1][1] * Data->VDEM[1]->Elev + Data->VtxPixCornerWt[1][2] * Data->VDEM[2]->Elev);

			Extraction->LatRange[2] = (Data->VtxPixCornerWt[2][0] * Data->VDEM[0]->Lat + Data->VtxPixCornerWt[2][1] * Data->VDEM[1]->Lat + Data->VtxPixCornerWt[2][2] * Data->VDEM[2]->Lat);
			Extraction->LonRange[2] = (Data->VtxPixCornerWt[2][0] * Data->VDEM[0]->Lon + Data->VtxPixCornerWt[2][1] * Data->VDEM[1]->Lon + Data->VtxPixCornerWt[2][2] * Data->VDEM[2]->Lon);
			Extraction->ElevRange[2] = (Data->VtxPixCornerWt[2][0] * Data->VDEM[0]->Elev + Data->VtxPixCornerWt[2][1] * Data->VDEM[1]->Elev + Data->VtxPixCornerWt[2][2] * Data->VDEM[2]->Elev);

			Extraction->LatRange[3] = (Data->VtxPixCornerWt[3][0] * Data->VDEM[0]->Lat + Data->VtxPixCornerWt[3][1] * Data->VDEM[1]->Lat + Data->VtxPixCornerWt[3][2] * Data->VDEM[2]->Lat);
			Extraction->LonRange[3] = (Data->VtxPixCornerWt[3][0] * Data->VDEM[0]->Lon + Data->VtxPixCornerWt[3][1] * Data->VDEM[1]->Lon + Data->VtxPixCornerWt[3][2] * Data->VDEM[2]->Lon);
			Extraction->ElevRange[3] = (Data->VtxPixCornerWt[3][0] * Data->VDEM[0]->Elev + Data->VtxPixCornerWt[3][1] * Data->VDEM[1]->Elev + Data->VtxPixCornerWt[3][2] * Data->VDEM[2]->Elev);

			if (CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
				{
				if ((TempOpacity = AdjustSampleForLatLonBounds(Extraction->LowX, Extraction->HighX, Extraction->LowY, Extraction->HighY, Extraction->LowZ, Extraction->HighZ, Data, Extraction)) <= 0.0)
					goto exit;
				rVal = TempOpacity;
				goto exit;
				} // if
			// not GEOREFERENCED, then it is GEOGRXYZ
			RangeX[0] = (Data->TexRefLon - Extraction->LonRange[0]) * Data->MetersPerDegLon;
			RangeY[0] = (Extraction->LatRange[0] - Data->TexRefLat) * Data->MetersPerDegLat;
			RangeZ[0] = Extraction->ElevRange[0] - Data->TexRefElev;

			RangeX[1] = (Data->TexRefLon - Extraction->LonRange[1]) * Data->MetersPerDegLon;
			RangeY[1] = (Extraction->LatRange[1] - Data->TexRefLat) * Data->MetersPerDegLat;
			RangeZ[1] = Extraction->ElevRange[1] - Data->TexRefElev;

			RangeX[2] = (Data->TexRefLon - Extraction->LonRange[2]) * Data->MetersPerDegLon;
			RangeY[2] = (Extraction->LatRange[2] - Data->TexRefLat) * Data->MetersPerDegLat;
			RangeZ[2] = Extraction->ElevRange[2] - Data->TexRefElev;

			RangeX[3] = (Data->TexRefLon - Extraction->LonRange[3]) * Data->MetersPerDegLon;
			RangeY[3] = (Extraction->LatRange[3] - Data->TexRefLat) * Data->MetersPerDegLat;
			RangeZ[3] = Extraction->ElevRange[3] - Data->TexRefElev;
			break;
			} // WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED
		case WCS_TEXTURE_COORDSPACE_VECTOR_ALIGNED:
			{
			if (Data->Vec)
				{
				if (Data->VData[0] && Data->VData[1] && Data->VData[2] && (Data->VDataVecOffsetsValid || ComputeVectorOffsets(Data)))
					{
					RangeX[0] = (Data->VtxPixCornerWt[0][0] * Data->VData[0]->VecOffsets[0] + Data->VtxPixCornerWt[0][1] * Data->VData[1]->VecOffsets[0] + Data->VtxPixCornerWt[0][2] * Data->VData[2]->VecOffsets[0]);
					RangeY[0] = (Data->VtxPixCornerWt[0][0] * Data->VData[0]->VecOffsets[1] + Data->VtxPixCornerWt[0][1] * Data->VData[1]->VecOffsets[1] + Data->VtxPixCornerWt[0][2] * Data->VData[2]->VecOffsets[1]);
					RangeZ[0] = (Data->VtxPixCornerWt[0][0] * Data->VData[0]->VecOffsets[2] + Data->VtxPixCornerWt[0][1] * Data->VData[1]->VecOffsets[2] + Data->VtxPixCornerWt[0][2] * Data->VData[2]->VecOffsets[2]);

					RangeX[1] = (Data->VtxPixCornerWt[1][0] * Data->VData[0]->VecOffsets[0] + Data->VtxPixCornerWt[1][1] * Data->VData[1]->VecOffsets[0] + Data->VtxPixCornerWt[1][2] * Data->VData[2]->VecOffsets[0]);
					RangeY[1] = (Data->VtxPixCornerWt[1][0] * Data->VData[0]->VecOffsets[1] + Data->VtxPixCornerWt[1][1] * Data->VData[1]->VecOffsets[1] + Data->VtxPixCornerWt[1][2] * Data->VData[2]->VecOffsets[1]);
					RangeZ[1] = (Data->VtxPixCornerWt[1][0] * Data->VData[0]->VecOffsets[2] + Data->VtxPixCornerWt[1][1] * Data->VData[1]->VecOffsets[2] + Data->VtxPixCornerWt[1][2] * Data->VData[2]->VecOffsets[2]);

					RangeX[2] = (Data->VtxPixCornerWt[2][0] * Data->VData[0]->VecOffsets[0] + Data->VtxPixCornerWt[2][1] * Data->VData[1]->VecOffsets[0] + Data->VtxPixCornerWt[2][2] * Data->VData[2]->VecOffsets[0]);
					RangeY[2] = (Data->VtxPixCornerWt[2][0] * Data->VData[0]->VecOffsets[1] + Data->VtxPixCornerWt[2][1] * Data->VData[1]->VecOffsets[1] + Data->VtxPixCornerWt[2][2] * Data->VData[2]->VecOffsets[1]);
					RangeZ[2] = (Data->VtxPixCornerWt[2][0] * Data->VData[0]->VecOffsets[2] + Data->VtxPixCornerWt[2][1] * Data->VData[1]->VecOffsets[2] + Data->VtxPixCornerWt[2][2] * Data->VData[2]->VecOffsets[2]);

					RangeX[3] = (Data->VtxPixCornerWt[3][0] * Data->VData[0]->VecOffsets[0] + Data->VtxPixCornerWt[3][1] * Data->VData[1]->VecOffsets[0] + Data->VtxPixCornerWt[3][2] * Data->VData[2]->VecOffsets[0]);
					RangeY[3] = (Data->VtxPixCornerWt[3][0] * Data->VData[0]->VecOffsets[1] + Data->VtxPixCornerWt[3][1] * Data->VData[1]->VecOffsets[1] + Data->VtxPixCornerWt[3][2] * Data->VData[2]->VecOffsets[1]);
					RangeZ[3] = (Data->VtxPixCornerWt[3][0] * Data->VData[0]->VecOffsets[2] + Data->VtxPixCornerWt[3][1] * Data->VData[1]->VecOffsets[2] + Data->VtxPixCornerWt[3][2] * Data->VData[2]->VecOffsets[2]);
					} // if
				else
					{
					Extraction->LatRange[0] = (Data->VtxPixCornerWt[0][0] * Data->VDEM[0]->Lat + Data->VtxPixCornerWt[0][1] * Data->VDEM[1]->Lat + Data->VtxPixCornerWt[0][2] * Data->VDEM[2]->Lat);
					Extraction->LonRange[0] = (Data->VtxPixCornerWt[0][0] * Data->VDEM[0]->Lon + Data->VtxPixCornerWt[0][1] * Data->VDEM[1]->Lon + Data->VtxPixCornerWt[0][2] * Data->VDEM[2]->Lon);
					Extraction->ElevRange[0] = (Data->VtxPixCornerWt[0][0] * Data->VDEM[0]->Elev + Data->VtxPixCornerWt[0][1] * Data->VDEM[1]->Elev + Data->VtxPixCornerWt[0][2] * Data->VDEM[2]->Elev);

					Extraction->LatRange[1] = (Data->VtxPixCornerWt[1][0] * Data->VDEM[0]->Lat + Data->VtxPixCornerWt[1][1] * Data->VDEM[1]->Lat + Data->VtxPixCornerWt[1][2] * Data->VDEM[2]->Lat);
					Extraction->LonRange[1] = (Data->VtxPixCornerWt[1][0] * Data->VDEM[0]->Lon + Data->VtxPixCornerWt[1][1] * Data->VDEM[1]->Lon + Data->VtxPixCornerWt[1][2] * Data->VDEM[2]->Lon);
					Extraction->ElevRange[1] = (Data->VtxPixCornerWt[1][0] * Data->VDEM[0]->Elev + Data->VtxPixCornerWt[1][1] * Data->VDEM[1]->Elev + Data->VtxPixCornerWt[1][2] * Data->VDEM[2]->Elev);

					Extraction->LatRange[2] = (Data->VtxPixCornerWt[2][0] * Data->VDEM[0]->Lat + Data->VtxPixCornerWt[2][1] * Data->VDEM[1]->Lat + Data->VtxPixCornerWt[2][2] * Data->VDEM[2]->Lat);
					Extraction->LonRange[2] = (Data->VtxPixCornerWt[2][0] * Data->VDEM[0]->Lon + Data->VtxPixCornerWt[2][1] * Data->VDEM[1]->Lon + Data->VtxPixCornerWt[2][2] * Data->VDEM[2]->Lon);
					Extraction->ElevRange[2] = (Data->VtxPixCornerWt[2][0] * Data->VDEM[0]->Elev + Data->VtxPixCornerWt[2][1] * Data->VDEM[1]->Elev + Data->VtxPixCornerWt[2][2] * Data->VDEM[2]->Elev);

					Extraction->LatRange[3] = (Data->VtxPixCornerWt[3][0] * Data->VDEM[0]->Lat + Data->VtxPixCornerWt[3][1] * Data->VDEM[1]->Lat + Data->VtxPixCornerWt[3][2] * Data->VDEM[2]->Lat);
					Extraction->LonRange[3] = (Data->VtxPixCornerWt[3][0] * Data->VDEM[0]->Lon + Data->VtxPixCornerWt[3][1] * Data->VDEM[1]->Lon + Data->VtxPixCornerWt[3][2] * Data->VDEM[2]->Lon);
					Extraction->ElevRange[3] = (Data->VtxPixCornerWt[3][0] * Data->VDEM[0]->Elev + Data->VtxPixCornerWt[3][1] * Data->VDEM[1]->Elev + Data->VtxPixCornerWt[3][2] * Data->VDEM[2]->Elev);

					rVal = AdjustSampleForAlignToVector(Extraction->LowX, Extraction->HighX, Extraction->LowY, Extraction->HighY, Extraction->LowZ, Extraction->HighZ, Data, Extraction);
					goto exit;
					} // else
				} // if
			else
				goto exit;
			break;
			} // WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED
		case WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE:
			{
			Extraction->LowX = Data->PixelUnityX[0];
			Extraction->HighX = Data->PixelUnityX[1];
			Extraction->LowY = Data->PixelUnityY[0];
			Extraction->HighY = Data->PixelUnityY[1];
			Extraction->LowZ = Extraction->HighZ = Data->ZDist;
			goto r1;
			} // WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE
		case WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE_NOZ:
			{
			Extraction->LowX = Data->PixelUnityX[0];
			Extraction->HighX = Data->PixelUnityX[1];
			Extraction->LowY = Data->PixelUnityY[0];
			Extraction->HighY = Data->PixelUnityY[1];
			Extraction->LowZ = Extraction->HighZ = 0.0;
			goto r1;
			} // WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE_NOZ
		case WCS_TEXTURE_COORDSPACE_IMAGE_PIXELUNITS:
			{
			Extraction->LowX = Data->PixelX[0];
			Extraction->HighX = Data->PixelX[1];
			Extraction->LowY = Data->PixelY[0];
			Extraction->HighY = Data->PixelY[1];
			Extraction->LowZ = Extraction->HighZ = Data->ZDist;
			goto r1;
			} // WCS_TEXTURE_COORDSPACE_IMAGE_PIXELUNITS
		case WCS_TEXTURE_COORDSPACE_IMAGE_PIXELUNITS_NOZ:
			{
			Extraction->LowX = Data->PixelX[0];
			Extraction->HighX = Data->PixelX[1];
			Extraction->LowY = Data->PixelY[0];
			Extraction->HighY = Data->PixelY[1];
			Extraction->LowZ = Extraction->HighZ = 0.0;
			goto r1;
			} // WCS_TEXTURE_COORDSPACE_IMAGE_PIXELUNITS_NOZ
		case WCS_TEXTURE_COORDSPACE_VERTEX_UVW:
			{
			MapType = WCS_VERTREFDATA_MAPTYPE_UVW;
			if (Data->Object3D && (VertexMap = Data->Object3D->MatchVertexMap(MapType, MapName, MapNumber)))
				{
				MapIndex[0] = Data->VertRefData[0] ? Data->VertRefData[0]->GetMapIndex(MapType, MapNumber, Data->VtxNum[0]): Data->VtxNum[0];
				MapIndex[1] = Data->VertRefData[1] ? Data->VertRefData[1]->GetMapIndex(MapType, MapNumber, Data->VtxNum[1]): Data->VtxNum[1];
				MapIndex[2] = Data->VertRefData[2] ? Data->VertRefData[2]->GetMapIndex(MapType, MapNumber, Data->VtxNum[2]): Data->VtxNum[2];
				if (VertexMap->CoordsValid[MapIndex[0]] && VertexMap->CoordsValid[MapIndex[1]] && VertexMap->CoordsValid[MapIndex[2]])
					{
					RangeX[0] = (Data->VtxPixCornerWt[0][0] * VertexMap->CoordsArray[0][MapIndex[0]] + 
						Data->VtxPixCornerWt[0][1] * VertexMap->CoordsArray[0][MapIndex[1]] + 
						Data->VtxPixCornerWt[0][2] * VertexMap->CoordsArray[0][MapIndex[2]]) - .5;
					RangeY[0] = (Data->VtxPixCornerWt[0][0] * VertexMap->CoordsArray[1][MapIndex[0]] + 
						Data->VtxPixCornerWt[0][1] * VertexMap->CoordsArray[1][MapIndex[1]] + 
						Data->VtxPixCornerWt[0][2] * VertexMap->CoordsArray[1][MapIndex[2]]) - .5;
					RangeZ[0] = (Data->VtxPixCornerWt[0][0] * VertexMap->CoordsArray[2][MapIndex[0]] + 
						Data->VtxPixCornerWt[0][1] * VertexMap->CoordsArray[2][MapIndex[1]] + 
						Data->VtxPixCornerWt[0][2] * VertexMap->CoordsArray[2][MapIndex[2]]) - .5;

					RangeX[1] = (Data->VtxPixCornerWt[1][0] * VertexMap->CoordsArray[0][MapIndex[0]] + 
						Data->VtxPixCornerWt[1][1] * VertexMap->CoordsArray[0][MapIndex[1]] + 
						Data->VtxPixCornerWt[1][2] * VertexMap->CoordsArray[0][MapIndex[2]]) - .5;
					RangeY[1] = (Data->VtxPixCornerWt[1][0] * VertexMap->CoordsArray[1][MapIndex[0]] + 
						Data->VtxPixCornerWt[1][1] * VertexMap->CoordsArray[1][MapIndex[1]] + 
						Data->VtxPixCornerWt[1][2] * VertexMap->CoordsArray[1][MapIndex[2]]) - .5;
					RangeZ[1] = (Data->VtxPixCornerWt[1][0] * VertexMap->CoordsArray[2][MapIndex[0]] + 
						Data->VtxPixCornerWt[1][1] * VertexMap->CoordsArray[2][MapIndex[1]] + 
						Data->VtxPixCornerWt[1][2] * VertexMap->CoordsArray[2][MapIndex[2]]) - .5;

					RangeX[2] = (Data->VtxPixCornerWt[2][0] * VertexMap->CoordsArray[0][MapIndex[0]] + 
						Data->VtxPixCornerWt[2][1] * VertexMap->CoordsArray[0][MapIndex[1]] + 
						Data->VtxPixCornerWt[2][2] * VertexMap->CoordsArray[0][MapIndex[2]]) - .5;
					RangeY[2] = (Data->VtxPixCornerWt[2][0] * VertexMap->CoordsArray[1][MapIndex[0]] + 
						Data->VtxPixCornerWt[2][1] * VertexMap->CoordsArray[1][MapIndex[1]] + 
						Data->VtxPixCornerWt[2][2] * VertexMap->CoordsArray[1][MapIndex[2]]) - .5;
					RangeZ[2] = (Data->VtxPixCornerWt[2][0] * VertexMap->CoordsArray[2][MapIndex[0]] + 
						Data->VtxPixCornerWt[2][1] * VertexMap->CoordsArray[2][MapIndex[1]] + 
						Data->VtxPixCornerWt[2][2] * VertexMap->CoordsArray[2][MapIndex[2]]) - .5;

					RangeX[3] = (Data->VtxPixCornerWt[3][0] * VertexMap->CoordsArray[0][MapIndex[0]] + 
						Data->VtxPixCornerWt[3][1] * VertexMap->CoordsArray[0][MapIndex[1]] + 
						Data->VtxPixCornerWt[3][2] * VertexMap->CoordsArray[0][MapIndex[2]]) - .5;
					RangeY[3] = (Data->VtxPixCornerWt[3][0] * VertexMap->CoordsArray[1][MapIndex[0]] + 
						Data->VtxPixCornerWt[3][1] * VertexMap->CoordsArray[1][MapIndex[1]] + 
						Data->VtxPixCornerWt[3][2] * VertexMap->CoordsArray[1][MapIndex[2]]) - .5;
					RangeZ[3] = (Data->VtxPixCornerWt[3][0] * VertexMap->CoordsArray[2][MapIndex[0]] + 
						Data->VtxPixCornerWt[3][1] * VertexMap->CoordsArray[2][MapIndex[1]] + 
						Data->VtxPixCornerWt[3][2] * VertexMap->CoordsArray[2][MapIndex[2]]) - .5;
					} // if
				else
					goto exit;
				} // if
			else
				goto exit;
			break;
			} // WCS_TEXTURE_COORDSPACE_VERTEX_UVW
		case WCS_TEXTURE_COORDSPACE_VERTEX_COLORPERVERTEX:
			{
			MapType = WCS_VERTREFDATA_MAPTYPE_COLORPERVERTEX;
			if (Data->Object3D && (VertexMap = Data->Object3D->MatchVertexMap(MapType, MapName, MapNumber)))
				{
				MapIndex[0] = Data->VertRefData[0] ? Data->VertRefData[0]->GetMapIndex(MapType, MapNumber, Data->VtxNum[0]): Data->VtxNum[0];
				MapIndex[1] = Data->VertRefData[1] ? Data->VertRefData[1]->GetMapIndex(MapType, MapNumber, Data->VtxNum[1]): Data->VtxNum[1];
				MapIndex[2] = Data->VertRefData[2] ? Data->VertRefData[2]->GetMapIndex(MapType, MapNumber, Data->VtxNum[2]): Data->VtxNum[2];
				if (VertexMap->CoordsValid[MapIndex[0]] && VertexMap->CoordsValid[MapIndex[1]] && VertexMap->CoordsValid[MapIndex[2]])
					{
					RangeX[0] = (Data->VtxPixCornerWt[0][0] * VertexMap->CoordsArray[0][MapIndex[0]] + 
						Data->VtxPixCornerWt[0][1] * VertexMap->CoordsArray[0][MapIndex[1]] + 
						Data->VtxPixCornerWt[0][2] * VertexMap->CoordsArray[0][MapIndex[2]]);
					RangeY[0] = (Data->VtxPixCornerWt[0][0] * VertexMap->CoordsArray[1][MapIndex[0]] + 
						Data->VtxPixCornerWt[0][1] * VertexMap->CoordsArray[1][MapIndex[1]] + 
						Data->VtxPixCornerWt[0][2] * VertexMap->CoordsArray[1][MapIndex[2]]);
					RangeZ[0] = (Data->VtxPixCornerWt[0][0] * VertexMap->CoordsArray[2][MapIndex[0]] + 
						Data->VtxPixCornerWt[0][1] * VertexMap->CoordsArray[2][MapIndex[1]] + 
						Data->VtxPixCornerWt[0][2] * VertexMap->CoordsArray[2][MapIndex[2]]);

					RangeX[1] = (Data->VtxPixCornerWt[1][0] * VertexMap->CoordsArray[0][MapIndex[0]] + 
						Data->VtxPixCornerWt[1][1] * VertexMap->CoordsArray[0][MapIndex[1]] + 
						Data->VtxPixCornerWt[1][2] * VertexMap->CoordsArray[0][MapIndex[2]]);
					RangeY[1] = (Data->VtxPixCornerWt[1][0] * VertexMap->CoordsArray[1][MapIndex[0]] + 
						Data->VtxPixCornerWt[1][1] * VertexMap->CoordsArray[1][MapIndex[1]] + 
						Data->VtxPixCornerWt[1][2] * VertexMap->CoordsArray[1][MapIndex[2]]);
					RangeZ[1] = (Data->VtxPixCornerWt[1][0] * VertexMap->CoordsArray[2][MapIndex[0]] + 
						Data->VtxPixCornerWt[1][1] * VertexMap->CoordsArray[2][MapIndex[1]] + 
						Data->VtxPixCornerWt[1][2] * VertexMap->CoordsArray[2][MapIndex[2]]);

					RangeX[2] = (Data->VtxPixCornerWt[2][0] * VertexMap->CoordsArray[0][MapIndex[0]] + 
						Data->VtxPixCornerWt[2][1] * VertexMap->CoordsArray[0][MapIndex[1]] + 
						Data->VtxPixCornerWt[2][2] * VertexMap->CoordsArray[0][MapIndex[2]]);
					RangeY[2] = (Data->VtxPixCornerWt[2][0] * VertexMap->CoordsArray[1][MapIndex[0]] + 
						Data->VtxPixCornerWt[2][1] * VertexMap->CoordsArray[1][MapIndex[1]] + 
						Data->VtxPixCornerWt[2][2] * VertexMap->CoordsArray[1][MapIndex[2]]);
					RangeZ[2] = (Data->VtxPixCornerWt[2][0] * VertexMap->CoordsArray[2][MapIndex[0]] + 
						Data->VtxPixCornerWt[2][1] * VertexMap->CoordsArray[2][MapIndex[1]] + 
						Data->VtxPixCornerWt[2][2] * VertexMap->CoordsArray[2][MapIndex[2]]);

					RangeX[3] = (Data->VtxPixCornerWt[3][0] * VertexMap->CoordsArray[0][MapIndex[0]] + 
						Data->VtxPixCornerWt[3][1] * VertexMap->CoordsArray[0][MapIndex[1]] + 
						Data->VtxPixCornerWt[3][2] * VertexMap->CoordsArray[0][MapIndex[2]]);
					RangeY[3] = (Data->VtxPixCornerWt[3][0] * VertexMap->CoordsArray[1][MapIndex[0]] + 
						Data->VtxPixCornerWt[3][1] * VertexMap->CoordsArray[1][MapIndex[1]] + 
						Data->VtxPixCornerWt[3][2] * VertexMap->CoordsArray[1][MapIndex[2]]);
					RangeZ[3] = (Data->VtxPixCornerWt[3][0] * VertexMap->CoordsArray[2][MapIndex[0]] + 
						Data->VtxPixCornerWt[3][1] * VertexMap->CoordsArray[2][MapIndex[1]] + 
						Data->VtxPixCornerWt[3][2] * VertexMap->CoordsArray[2][MapIndex[2]]);

					Extraction->LowX = Extraction->HighX = (RangeX[0] + RangeX[1] + RangeX[2] + RangeX[3]) * .25; 
					Extraction->LowY = Extraction->HighY = (RangeY[0] + RangeY[1] + RangeY[2] + RangeY[3]) * .25; 
					Extraction->LowZ = Extraction->HighZ = (RangeZ[0] + RangeZ[1] + RangeZ[2] + RangeZ[3]) * .25; 
					goto r1;
					} // if
				else
					goto exit;
				} // if
			else
				goto exit;
			} // WCS_TEXTURE_COORDSPACE_VERTEX_COLORPERVERTEX
		} // switch
	// sort max/min
	Extraction->LowX = min(RangeX[0], RangeX[1]);
	Extraction->LowX = min(Extraction->LowX, RangeX[2]);
	Extraction->LowX = min(Extraction->LowX, RangeX[3]);
	Extraction->LowY = min(RangeY[0], RangeY[1]);
	Extraction->LowY = min(Extraction->LowY, RangeY[2]);
	Extraction->LowY = min(Extraction->LowY, RangeY[3]);
	Extraction->LowZ = min(RangeZ[0], RangeZ[1]);
	Extraction->LowZ = min(Extraction->LowZ, RangeZ[2]);
	Extraction->LowZ = min(Extraction->LowZ, RangeZ[3]);
	Extraction->HighX = max(RangeX[0], RangeX[1]);
	Extraction->HighX = max(Extraction->HighX, RangeX[2]);
	Extraction->HighX = max(Extraction->HighX, RangeX[3]);
	Extraction->HighY = max(RangeY[0], RangeY[1]);
	Extraction->HighY = max(Extraction->HighY, RangeY[2]);
	Extraction->HighY = max(Extraction->HighY, RangeY[3]);
	Extraction->HighZ = max(RangeZ[0], RangeZ[1]);
	Extraction->HighZ = max(Extraction->HighZ, RangeZ[2]);
	Extraction->HighZ = max(Extraction->HighZ, RangeZ[3]);
	goto r1;
	} // if
else if (Data->VDEM[0])
	{
	switch (CoordSpace)
		{
		case WCS_TEXTURE_COORDSPACE_NONE:
			{
			//if (TexType == WCS_TEXTURE_TYPE_TERRAINPARAM)
			//	{
			//	return (ExtractDynamicValues(Data));
			//	} // if
			goto r1;
			} // WCS_TEXTURE_COORDSPACE_NONE
		case WCS_TEXTURE_COORDSPACE_OBJECT_CARTESIAN:
			{
			Extraction->LowX = Extraction->HighX = Data->VDEM[0]->xyz[0];
			Extraction->LowY = Extraction->HighY = Data->VDEM[0]->xyz[1];
			Extraction->LowZ = Extraction->HighZ = Data->VDEM[0]->xyz[2];
			break;
			} // WCS_TEXTURE_COORDSPACE_OBJECT_CARTESIAN
		case WCS_TEXTURE_COORDSPACE_GLOBAL_CARTESIAN:
			{
			Extraction->LowX = Extraction->HighX = Data->VDEM[0]->XYZ[0];
			Extraction->LowY = Extraction->HighY = Data->VDEM[0]->XYZ[1];
			Extraction->LowZ = Extraction->HighZ = Data->VDEM[0]->XYZ[2];
			break;
			} // WCS_TEXTURE_COORDSPACE_GLOBAL_CARTESIAN
		case WCS_TEXTURE_COORDSPACE_PROJECT_GEOGRXYZ:
		case WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED:
			{
			Extraction->LatRange[0] = Extraction->LatRange[1] = Extraction->LatRange[2] = Extraction->LatRange[3] = Data->VDEM[0]->Lat;
			Extraction->LonRange[0] = Extraction->LonRange[1] = Extraction->LonRange[2] = Extraction->LonRange[3] = Data->VDEM[0]->Lon;
			Extraction->ElevRange[0] = Extraction->ElevRange[1] = Extraction->ElevRange[2] = Extraction->ElevRange[3] = Data->VDEM[0]->Elev;

			if (CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
				{
				if ((TempOpacity = AdjustSampleForLatLonBounds(Extraction->LowX, Extraction->HighX, Extraction->LowY, Extraction->HighY, Extraction->LowZ, Extraction->HighZ, Data, Extraction)) <= 0.0)
					goto exit;
				rVal = TempOpacity;
				goto exit;
				} // if
			// not GEOREFERENCED, then it is GEOGRXYZ
			Extraction->LowX = Extraction->HighX = (Data->TexRefLon - Extraction->LonRange[0]) * Data->MetersPerDegLon;
			Extraction->LowY = Extraction->HighY = (Extraction->LatRange[0] - Data->TexRefLat) * Data->MetersPerDegLat;
			Extraction->LowZ = Extraction->HighZ = Extraction->ElevRange[0] - Data->TexRefElev;
			break;
			} // WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED
		case WCS_TEXTURE_COORDSPACE_VECTOR_ALIGNED:
			{
			if (Data->Vec)
				{
				if (Data->VData[0] && (Data->VDataVecOffsetsValid || ComputeVectorOffsets(Data)))
					{
					Extraction->LowX = Extraction->HighX = Data->VData[0]->VecOffsets[0];
					Extraction->LowY = Extraction->HighY = Data->VData[0]->VecOffsets[1];
					Extraction->LowZ = Extraction->HighZ = Data->VData[0]->VecOffsets[2];
					} // if
				else
					{
					Extraction->LatRange[0] = Extraction->LatRange[1] = Extraction->LatRange[2] = Extraction->LatRange[3] = Data->VDEM[0]->Lat;
					Extraction->LonRange[0] = Extraction->LonRange[1] = Extraction->LonRange[2] = Extraction->LonRange[3] = Data->VDEM[0]->Lon;
					Extraction->ElevRange[0] = Extraction->ElevRange[1] = Extraction->ElevRange[2] = Extraction->ElevRange[3] = Data->VDEM[0]->Elev;

					rVal = AdjustSampleForAlignToVector(Extraction->LowX, Extraction->HighX, Extraction->LowY, Extraction->HighY, Extraction->LowZ, Extraction->HighZ, Data, Extraction);
					goto exit;
					} // else
				} // if
			else
				goto exit;
			break;
			} // WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED
		case WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE:
			{
			Extraction->LowX = Data->PixelUnityX[0];
			Extraction->HighX = Data->PixelUnityX[1];
			Extraction->LowY = Data->PixelUnityY[0];
			Extraction->HighY = Data->PixelUnityY[1];
			Extraction->LowZ = Extraction->HighZ = Data->ZDist;
			goto r1;
			} // WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE
		case WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE_NOZ:
			{
			Extraction->LowX = Data->PixelUnityX[0];
			Extraction->HighX = Data->PixelUnityX[1];
			Extraction->LowY = Data->PixelUnityY[0];
			Extraction->HighY = Data->PixelUnityY[1];
			Extraction->LowZ = Extraction->HighZ = 0.0;
			goto r1;
			} // WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE_NOZ
		case WCS_TEXTURE_COORDSPACE_IMAGE_PIXELUNITS:
			{
			Extraction->LowX = Data->PixelX[0];
			Extraction->HighX = Data->PixelX[1];
			Extraction->LowY = Data->PixelY[0];
			Extraction->HighY = Data->PixelY[1];
			Extraction->LowZ = Extraction->HighZ = Data->ZDist;
			goto r1;
			} // WCS_TEXTURE_COORDSPACE_IMAGE_PIXELUNITS
		case WCS_TEXTURE_COORDSPACE_IMAGE_PIXELUNITS_NOZ:
			{
			Extraction->LowX = Data->PixelX[0];
			Extraction->HighX = Data->PixelX[1];
			Extraction->LowY = Data->PixelY[0];
			Extraction->HighY = Data->PixelY[1];
			Extraction->LowZ = Extraction->HighZ = 0.0;
			goto r1;
			} // WCS_TEXTURE_COORDSPACE_IMAGE_PIXELUNITS_NOZ
		case WCS_TEXTURE_COORDSPACE_VERTEX_UVW:
			{
			MapType = WCS_VERTREFDATA_MAPTYPE_UVW;
			if (Data->Object3D && (VertexMap = Data->Object3D->MatchVertexMap(MapType, MapName, MapNumber)))
				{
				MapIndex[0] = Data->VertRefData[0] ? Data->VertRefData[0]->GetMapIndex(MapType, MapNumber, Data->VtxNum[0]): Data->VtxNum[0];
				MapIndex[1] = Data->VertRefData[1] ? Data->VertRefData[1]->GetMapIndex(MapType, MapNumber, Data->VtxNum[1]): Data->VtxNum[1];
				MapIndex[2] = Data->VertRefData[2] ? Data->VertRefData[2]->GetMapIndex(MapType, MapNumber, Data->VtxNum[2]): Data->VtxNum[2];
				if (VertexMap->CoordsValid[MapIndex[0]] && VertexMap->CoordsValid[MapIndex[1]] && VertexMap->CoordsValid[MapIndex[2]])
					{
					Extraction->LowX = Extraction->HighX = (Data->VtxWt[0] * VertexMap->CoordsArray[0][MapIndex[0]] + 
						Data->VtxWt[1] * VertexMap->CoordsArray[0][MapIndex[1]] + 
						Data->VtxWt[2] * VertexMap->CoordsArray[0][MapIndex[2]]) - .5;
					Extraction->LowY = Extraction->HighY = (Data->VtxWt[0] * VertexMap->CoordsArray[1][MapIndex[0]] + 
						Data->VtxWt[1] * VertexMap->CoordsArray[1][MapIndex[1]] + 
						Data->VtxWt[2] * VertexMap->CoordsArray[1][MapIndex[2]]) - .5;
					Extraction->LowZ = Extraction->HighZ = (Data->VtxWt[0] * VertexMap->CoordsArray[2][MapIndex[0]] + 
						Data->VtxWt[1] * VertexMap->CoordsArray[2][MapIndex[1]] + 
						Data->VtxWt[2] * VertexMap->CoordsArray[2][MapIndex[2]]) - .5;
					goto r1;
					} // if
				} // if
			goto exit;
			} // WCS_TEXTURE_COORDSPACE_VERTEX_UVW
		case WCS_TEXTURE_COORDSPACE_VERTEX_COLORPERVERTEX:
			{
			MapType = WCS_VERTREFDATA_MAPTYPE_COLORPERVERTEX;
			if (Data->Object3D && (VertexMap = Data->Object3D->MatchVertexMap(MapType, MapName, MapNumber)))
				{
				MapIndex[0] = Data->VertRefData[0] ? Data->VertRefData[0]->GetMapIndex(MapType, MapNumber, Data->VtxNum[0]): Data->VtxNum[0];
				MapIndex[1] = Data->VertRefData[1] ? Data->VertRefData[1]->GetMapIndex(MapType, MapNumber, Data->VtxNum[1]): Data->VtxNum[1];
				MapIndex[2] = Data->VertRefData[2] ? Data->VertRefData[2]->GetMapIndex(MapType, MapNumber, Data->VtxNum[2]): Data->VtxNum[2];
				if (VertexMap->CoordsValid[MapIndex[0]] && VertexMap->CoordsValid[MapIndex[1]] && VertexMap->CoordsValid[MapIndex[2]])
					{
					Extraction->LowX = Extraction->HighX = (Data->VtxWt[0] * VertexMap->CoordsArray[0][MapIndex[0]] + 
						Data->VtxWt[1] * VertexMap->CoordsArray[0][MapIndex[1]] + 
						Data->VtxWt[2] * VertexMap->CoordsArray[0][MapIndex[2]]);
					Extraction->LowY = Extraction->HighY = (Data->VtxWt[0] * VertexMap->CoordsArray[1][MapIndex[0]] + 
						Data->VtxWt[1] * VertexMap->CoordsArray[1][MapIndex[1]] + 
						Data->VtxWt[2] * VertexMap->CoordsArray[1][MapIndex[2]]);
					Extraction->LowZ = Extraction->HighZ = (Data->VtxWt[0] * VertexMap->CoordsArray[2][MapIndex[0]] + 
						Data->VtxWt[1] * VertexMap->CoordsArray[2][MapIndex[1]] + 
						Data->VtxWt[2] * VertexMap->CoordsArray[2][MapIndex[2]]);
					goto r1;
					} // if
				} // if
			goto exit;
			} // WCS_TEXTURE_COORDSPACE_VERTEX_COLORPERVERTEX
		} // switch
	if (Data->PixelWidth > 0.0)
		{
		if (CoordSpace == WCS_TEXTURE_COORDSPACE_OBJECT_CARTESIAN)
			DataOffset = Data->ObjectPixelWidth;
		else
			DataOffset = Data->PixelWidth;
		Extraction->LowX -= DataOffset;
		Extraction->LowY -= DataOffset;
		Extraction->LowZ -= DataOffset;
		Extraction->HighX += DataOffset;
		Extraction->HighY += DataOffset;
		Extraction->HighZ += DataOffset;
		} // if
	goto r1;
	} // else if
else if (Data->PData)
	{
	switch (CoordSpace)
		{
		case WCS_TEXTURE_COORDSPACE_NONE:
			{
			//if (TexType == WCS_TEXTURE_TYPE_TERRAINPARAM)
			//	{
			//	return (ExtractDynamicValues(Data));
			//	} // if
			goto r1;
			} // WCS_TEXTURE_COORDSPACE_NONE
		case WCS_TEXTURE_COORDSPACE_VECTOR_ALIGNED:
		case WCS_TEXTURE_COORDSPACE_PROJECT_GEOGRXYZ:
		case WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED:
			{
			Extraction->LatRange[0] = Extraction->LatRange[1] = Extraction->LatRange[2] = Extraction->LatRange[3] = Data->PData->Lat;
			Extraction->LonRange[0] = Extraction->LonRange[1] = Extraction->LonRange[2] = Extraction->LonRange[3] = Data->PData->Lon;
			Extraction->ElevRange[0] = Extraction->ElevRange[1] = Extraction->ElevRange[2] = Extraction->ElevRange[3] = Data->PData->Elev;

			if (CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
				{
				if ((TempOpacity = AdjustSampleForLatLonBounds(Extraction->LowX, Extraction->HighX, Extraction->LowY, Extraction->HighY, Extraction->LowZ, Extraction->HighZ, Data, Extraction)) <= 0.0)
					goto exit;
				rVal = TempOpacity;
				goto exit;
				} // if
			if (CoordSpace == WCS_TEXTURE_COORDSPACE_VECTOR_ALIGNED)
				{
				rVal = AdjustSampleForAlignToVector(Extraction->LowX, Extraction->HighX, Extraction->LowY, Extraction->HighY, Extraction->LowZ, Extraction->HighZ, Data, Extraction);
				goto exit;
				} // if
			// not GEOREFERENCED, then it is GEOGRXYZ
			Extraction->LowX = Extraction->HighX = (Data->TexRefLon - Extraction->LonRange[0]) * Data->MetersPerDegLon;
			Extraction->LowY = Extraction->HighY = (Extraction->LatRange[0] - Data->TexRefLat) * Data->MetersPerDegLat;
			Extraction->LowZ = Extraction->HighZ = Extraction->ElevRange[0] - Data->TexRefElev;
			break;
			} // WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED
		case WCS_TEXTURE_COORDSPACE_OBJECT_CARTESIAN:
		case WCS_TEXTURE_COORDSPACE_GLOBAL_CARTESIAN:
		case WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE:
		case WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE_NOZ:
		case WCS_TEXTURE_COORDSPACE_IMAGE_PIXELUNITS:
		case WCS_TEXTURE_COORDSPACE_IMAGE_PIXELUNITS_NOZ:
		case WCS_TEXTURE_COORDSPACE_VERTEX_UVW:
		case WCS_TEXTURE_COORDSPACE_VERTEX_COLORPERVERTEX:
			{
			// not supported when only polygon data present
			goto exit;
			} // WCS_TEXTURE_COORDSPACE_VERTEX_UVW
		} // switch
	if (Data->PixelWidth > 0.0)
		{
		Extraction->LowX -= Data->PixelWidth;
		Extraction->LowY -= Data->PixelWidth;
		Extraction->LowZ -= Data->PixelWidth;
		Extraction->HighX += Data->PixelWidth;
		Extraction->HighY += Data->PixelWidth;
		Extraction->HighZ += Data->PixelWidth;
		} // if
	goto r1;
	} // else if
else
	{
	switch (CoordSpace)
		{
		case WCS_TEXTURE_COORDSPACE_NONE:
			{
			//if (TexType == WCS_TEXTURE_TYPE_TERRAINPARAM)
			//	{
			//	return (ExtractDynamicValues(Data));
			//	} // if
			goto r1;
			} // WCS_TEXTURE_COORDSPACE_NONE
		case WCS_TEXTURE_COORDSPACE_VECTOR_ALIGNED:
		case WCS_TEXTURE_COORDSPACE_PROJECT_GEOGRXYZ:
		case WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED:
			{
			Extraction->LatRange[0] = Extraction->LatRange[1] = Extraction->LatRange[2] = Extraction->LatRange[3] = Data->Latitude;
			Extraction->LonRange[0] = Extraction->LonRange[1] = Extraction->LonRange[2] = Extraction->LonRange[3] = Data->Longitude;
			Extraction->ElevRange[0] = Extraction->ElevRange[1] = Extraction->ElevRange[2] = Extraction->ElevRange[3] = Data->Elev;

			if (CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
				{
				if ((TempOpacity = AdjustSampleForLatLonBounds(Extraction->LowX, Extraction->HighX, Extraction->LowY, Extraction->HighY, Extraction->LowZ, Extraction->HighZ, Data, Extraction)) <= 0.0)
					goto exit;
				rVal = TempOpacity;
				goto exit;
				} // if
			if (CoordSpace == WCS_TEXTURE_COORDSPACE_VECTOR_ALIGNED)
				{
				rVal = AdjustSampleForAlignToVector(Extraction->LowX, Extraction->HighX, Extraction->LowY, Extraction->HighY, Extraction->LowZ, Extraction->HighZ, Data, Extraction);
				goto exit;
				} // if
			// not GEOREFERENCED, then it is GEOGRXYZ
			Extraction->LowX = Extraction->HighX = (Data->TexRefLon - Extraction->LonRange[0]) * Data->MetersPerDegLon;
			Extraction->LowY = Extraction->HighY = (Extraction->LatRange[0] - Data->TexRefLat) * Data->MetersPerDegLat;
			Extraction->LowZ = Extraction->HighZ = Extraction->ElevRange[0] - Data->TexRefElev;
			break;
			} // WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED
		case WCS_TEXTURE_COORDSPACE_OBJECT_CARTESIAN:
		case WCS_TEXTURE_COORDSPACE_GLOBAL_CARTESIAN:
		case WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE:
		case WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE_NOZ:
		case WCS_TEXTURE_COORDSPACE_IMAGE_PIXELUNITS:
		case WCS_TEXTURE_COORDSPACE_IMAGE_PIXELUNITS_NOZ:
		case WCS_TEXTURE_COORDSPACE_VERTEX_UVW:
		case WCS_TEXTURE_COORDSPACE_VERTEX_COLORPERVERTEX:
			{
			// not supported when no data source present
			goto exit;
			} // WCS_TEXTURE_COORDSPACE_VERTEX_UVW
		} // switch
	if (Data->PixelWidth > 0.0)
		{
		Extraction->LowX -= Data->PixelWidth;
		Extraction->LowY -= Data->PixelWidth;
		Extraction->LowZ -= Data->PixelWidth;
		Extraction->HighX += Data->PixelWidth;
		Extraction->HighY += Data->PixelWidth;
		Extraction->HighZ += Data->PixelWidth;
		} // if
	goto r1;
	} // else

//return (0.0);

} // Texture::ExtractTexDataValues

/*===========================================================================*/
/*
double Texture::ExtractDynamicValues(TextureData *Data)
{

if (! Data->IsSampleThumbnail && ! Data->SampleType)
	{
	switch (Misc)
		{
		case WCS_TEXTURE_TERRAINPARAM_ELEV:
			{
			if (Data->VDEM[0] && Data->VDEM[1] && Data->VDEM[2])
				{
				Data->Elev = (Data->VtxWt[0] * Data->VDEM[0]->Elev + 
					Data->VtxWt[1] * Data->VDEM[1]->Elev + 
					Data->VtxWt[2] * Data->VDEM[2]->Elev);
				} // if
			else if (Data->VDEM[0])
				{
				Data->Elev = Data->VDEM[0]->Elev;
				} // if
			else if (Data->PData)
				{
				Data->Elev = Data->PData->Elev;
				} // if
			else
				return (0.0);
			break;
			} // WCS_TEXTURE_TERRAINPARAM_ELEV
		case WCS_TEXTURE_TERRAINPARAM_RELELEV:
			{
			if (Data->VData[0] && Data->VData[1] && Data->VData[2])
				{
				Data->RelElev = (float)(Data->VtxWt[0] * Data->VData[0]->RelEl + 
					Data->VtxWt[1] * Data->VData[1]->RelEl + 
					Data->VtxWt[2] * Data->VData[2]->RelEl);
				} // if
			else if (Data->VData[0])
				{
				Data->RelElev = Data->VData[0]->RelEl;
				} // if
			else
				return (0.0);
			break;
			} // WCS_TEXTURE_TERRAINPARAM_RELELEV
		case WCS_TEXTURE_TERRAINPARAM_SLOPE:
			{
			if (Data->PData)
				{
				Data->Slope = Data->PData->Slope;
				} // if
			else
				return (0.0);
			break;
			} // WCS_TEXTURE_TERRAINPARAM_SLOPE
		case WCS_TEXTURE_TERRAINPARAM_NORTHDEV:
		case WCS_TEXTURE_TERRAINPARAM_EASTDEV:
		case WCS_TEXTURE_TERRAINPARAM_ASPECT:
			{
			if (Data->PData)
				{
				Data->Aspect = Data->PData->Aspect;
				} // if
			else
				return (0.0);
			break;
			} // WCS_TEXTURE_TERRAINPARAM_EASTDEV
		case WCS_TEXTURE_TERRAINPARAM_LATITUDE:
			{
			if (Data->VDEM[0] && Data->VDEM[1] && Data->VDEM[2])
				{
				Data->Latitude = (Data->VtxWt[0] * Data->VDEM[0]->Lat + 
					Data->VtxWt[1] * Data->VDEM[1]->Lat + 
					Data->VtxWt[2] * Data->VDEM[2]->Lat);
				} // if
			else if (Data->VDEM[0])
				{
				Data->Latitude = Data->VDEM[0]->Lat;
				} // if
			else if (Data->PData)
				{
				Data->Latitude = Data->PData->Lat;
				} // if
			else
				return (0.0);
			break;
			} // WCS_TEXTURE_TERRAINPARAM_LATITUDE
		case WCS_TEXTURE_TERRAINPARAM_LONGITUDE:
			{
			if (Data->VDEM[0] && Data->VDEM[1] && Data->VDEM[2])
				{
				Data->Longitude = (Data->VtxWt[0] * Data->VDEM[0]->Lon + 
					Data->VtxWt[1] * Data->VDEM[1]->Lon + 
					Data->VtxWt[2] * Data->VDEM[2]->Lon);
				} // if
			else if (Data->VDEM[0])
				{
				Data->Longitude = Data->VDEM[0]->Lon;
				} // if
			else if (Data->PData)
				{
				Data->Longitude = Data->PData->Lon;
				} // if
			else
				return (0.0);
			break;
			} // WCS_TEXTURE_TERRAINPARAM_LONGITUDE
		case WCS_TEXTURE_TERRAINPARAM_WATERDEPTH:
			{
			if (Data->VData[0] && Data->VData[1] && Data->VData[2])
				{
				Data->WaterDepth = (Data->VtxWt[0] * Data->VData[0]->WaterDepth + 
					Data->VtxWt[1] * Data->VData[1]->WaterDepth + 
					Data->VtxWt[2] * Data->VData[2]->WaterDepth);
				} // if
			else if (Data->VData[0])
				{
				Data->WaterDepth = Data->VData[0]->WaterDepth;
				} // if
			else
				return (0.0);
			break;
			} // WCS_TEXTURE_TERRAINPARAM_WATERDEPTH
		case WCS_TEXTURE_TERRAINPARAM_VECTORSLOPE:
			{
			if (Data->VData[0] && Data->VData[1] && Data->VData[2])
				{
				Data->VectorSlope = (float)(Data->VtxWt[0] * Data->VData[0]->VectorSlope + 
					Data->VtxWt[1] * Data->VData[1]->VectorSlope + 
					Data->VtxWt[2] * Data->VData[2]->VectorSlope);
				} // if
			else if (Data->VData[0])
				{
				Data->VectorSlope = Data->VData[0]->VectorSlope;
				} // if
			else if (Data->PData)
				{
				Data->VectorSlope = Data->PData->VectorSlope;
				} // if
			else
				return (0.0);
			break;
			} // WCS_TEXTURE_TERRAINPARAM_VECTORSLOPE
		} // switch
	} // else if

return (1.0);

} // Texture::ExtractDynamicValues

*/
/*===========================================================================*/
/*===========================================================================*/

PlanarImageTexture::PlanarImageTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_PLANARIMAGE)
{

SetDefaults(ParamNumber);

} // PlanarImageTexture::PlanarImageTexture

/*===========================================================================*/

PlanarImageTexture::PlanarImageTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_PLANARIMAGE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // PlanarImageTexture::PlanarImageTexture

/*===========================================================================*/

void PlanarImageTexture::SetDefaults(long ParamNumber)
{
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	0, 0,	// 1
	0, 0,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	0, 0,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",	// 0
	"", "",	// 1
	"", "",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",	// 9
	"", "",	// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_PLANARIMAGE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // PlanarImageTexture::SetDefaults

/*===========================================================================*/

char PlanarImageTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // PlanarImageTexture::GetTextureParamType

/*===========================================================================*/

double PlanarImageTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double PtXLow, PtXHigh, PtYLow, PtYHigh, Value, TempValue, TempXLow, TempXHigh, TempYLow, TempYHigh, TempOutput[3],
	wt, wtx, SumWt, PtXRange, PtYRange;
Raster *MyRast;
long IntervalNumberLowX, IntervalNumberHighX, IntervalNumberLowY, IntervalNumberHighY;
int Abort = 0;
char DontSample, DontSample2, LLBounds = 0;

Value = Output[0] = Output[1] = Output[2] = 0.0;
if (! Img || ! (MyRast = Img->GetRaster()))
	{
	return (0.0);
	} // if
if (Data->IsSampleThumbnail && MyRast->ImageManagerEnabled 
	&& MyRast->GetAverageColor(0) == .5
	&& MyRast->GetAverageColor(1) == .5
	&& MyRast->GetAverageColor(2) == .5
	&& MyRast->GetAverageCoverage() == 1.0)
	{
	// shares standin
	MyRast = GlobalApp->AppImages->FetchStandInRast();
	} // if

if (CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
	{
//	if ((TempOpacity = AdjustSampleForLatLonBounds(LowX, HighX, LowY, HighY, LowZ, HighZ, Data, Extraction)) <= 0.0)
//		return (0.0);
	LLBounds = 1;
	} // if
else
	{
	//TempOpacity = 1.0;
	LowX -= TCenter[0];
	HighX -= TCenter[0];
	LowY -= TCenter[1];
	HighY -= TCenter[1];
	LowZ -= TCenter[2];
	HighZ -= TCenter[2];

	if (TextureMoving())
		{
		AdjustSampleForVelocity(LowX, HighX, LowY, HighY, LowZ, HighZ);
		} // if

	if (Rotated)
		{
		RotatePoint(LowX, LowY, LowZ, RotMatx);
		RotatePoint(HighX, HighY, HighZ, RotMatx);
		} // if

	LowX /= TSize[0];
	HighX /= TSize[0];
	LowY /= TSize[1];
	HighY /= TSize[1];
	LowZ /= TSize[2];
	HighZ /= TSize[2];
	} // else not lat/lon bounds

// removed the restirction on align to vector's axis alignemtn 11/5/00 to 
// allow things like highway retaining walls to be textured with image aligned to x axis 
// or road stripes with image aligned to y
if (TexAxis == 2 || CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
	{
	PtXLow = LowX;
	PtXHigh = HighX;
	PtYLow = LowY;
	PtYHigh = HighY;
	} // else
else if (TexAxis == 0)
	{
	if (ApplyToEcosys)
		{
		PtXLow = LowY;
		PtXHigh = HighY;
		PtYLow = LowZ;
		PtYHigh = HighZ;
		} // if
	else
		{
		PtXLow = LowZ;
		PtXHigh = HighZ;
		PtYLow = LowY;
		PtYHigh = HighY;
		} // else
	} // if
else
	{
	PtXLow = LowX;
	PtXHigh = HighX;
	PtYLow = LowZ;
	PtYHigh = HighZ;
	} // else if

if (PtXLow > PtXHigh)
	Swap64(PtXLow, PtXHigh);
if (PtYLow > PtYHigh)
	Swap64(PtYLow, PtYHigh);

// without antialiasing
if (! Antialias || WCS_SinglePoint2D(PtXLow, PtXHigh, PtYLow, PtYHigh))
	{
	PtXLow = (PtXLow + PtXHigh) * .5;
	PtYLow = (PtYLow + PtYHigh) * .5;
	if (! LLBounds)
		{
		PtXLow += .5;		// shift to image coord space so that the texture center 0,0 is at the image center
		PtYLow += .5;
		} // if

	if (PtXLow < 0.0 || PtXLow > 1.0)
		{
		if (IntervalNumberLowX = (long)floor(PtXLow))
			{
			if (TileWidth && CoordSpace != WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
				{
				PtXLow -= IntervalNumberLowX;
				if (FlipWidth && IntervalNumberLowX % 2)
					{
					PtXLow = 1.0 - PtXLow;
					} // if
				} // if
			else
				{
				return (0.0);
				} // else if
			} // if
		} // if
	if (PtYLow < 0.0 || PtYLow > 1.0)
		{
		if (IntervalNumberLowY = (long)floor(PtYLow))
			{
			if (TileHeight && CoordSpace != WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
				{
				PtYLow -= IntervalNumberLowY;
				if (FlipHeight && IntervalNumberLowY % 2)
					{
					PtYLow = 1.0 - PtYLow;
					} // if
				} // if
			else
				{
				return (0.0);
				} // else if
			} // if
		} // if
	if (ReverseWidth)
		{
		PtXLow = 1.0 - PtXLow;
		} // if
	if (ReverseHeight)
		{
		PtYLow = 1.0 - PtYLow;
		} // if
	if (PixelBlend)
		Value = MyRast->SampleBlendDouble3(Output, PtXLow, 1.0 - PtYLow, Abort);	// 1 - PtY flips image right-side up
	else
		Value = MyRast->SampleByteDouble3(Output, PtXLow, 1.0 - PtYLow, Abort);	// 1 - PtY flips image right-side up
	// if image was not found and user chose not to select a new one, Abort will be set.
	if (Abort)
		{
		// disable so it doesn't happen again
		Enabled = 0;
		} // if
	} // if
else
	{
	if (! LLBounds)
		{
		PtXLow += .5;		// shift to image coord space so that the texture center 0,0 is at the image center
		PtXHigh += .5;		// shift to image coord space so that the texture center 0,0 is at the image center
		PtYLow += .5;
		PtYHigh += .5;
		} // if
	IntervalNumberLowX = (int)floor(PtXLow);
	IntervalNumberHighX = (int)floor(PtXHigh);
	IntervalNumberLowY = (int)floor(PtYLow);
	IntervalNumberHighY = (int)floor(PtYHigh);
	if (IntervalNumberLowX == IntervalNumberHighX)
		{
		if (IntervalNumberLowX)
			{
			if (TileWidth && CoordSpace != WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
				{
				PtXLow -= IntervalNumberLowX;
				PtXHigh -= IntervalNumberHighX;
				if (FlipWidth && IntervalNumberLowX % 2)
					{
					PtXLow = 1.0 - PtXLow;
					PtXHigh = 1.0 - PtXHigh;
					Swap64(PtXLow, PtXHigh);
					} // if
				} // if
			else
				{
				return (0.0);
				} // else if
			} // if
		if (ReverseWidth)
			{
			PtXLow = 1.0 - PtXLow;
			PtXHigh = 1.0 - PtXHigh;
			Swap64(PtXLow, PtXHigh);
			} // if
		if (IntervalNumberLowY == IntervalNumberHighY)
			{
			if (IntervalNumberLowY)
				{
				if (TileHeight && CoordSpace != WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
					{
					PtYLow -= IntervalNumberLowY;
					PtYHigh -= IntervalNumberHighY;
					if (FlipHeight && IntervalNumberLowY % 2)
						{
						PtYLow = 1.0 - PtYLow;
						PtYHigh = 1.0 - PtYHigh;
						Swap64(PtYLow, PtYHigh);
						} // if
					} // if
				else
					{
					return (0.0);
					} // else if
				} // if
			if (ReverseHeight)
				{
				PtYLow = 1.0 - PtYLow;
				PtYHigh = 1.0 - PtYHigh;
				Swap64(PtYLow, PtYHigh);
				} // if
			Value = MyRast->SampleRangeByteDouble3(Output, PtXLow, PtXHigh, 1.0 - PtYHigh, 1.0 - PtYLow, Abort);	// 1 - PtY flips image right-side up
			// if image was not found and user chose not to select a new one, Abort will be set.
			if (Abort)
				{
				// disable so it doesn't happen again
				Enabled = 0;
				} // if
			} // if same x and y
		else
			{
			// two passes of y
			// low y
			TempOutput[0] = TempOutput[1] = TempOutput[2] = 0.0;
			SumWt = Value = TempValue = 0.0;
			DontSample = 0;
			PtYRange = PtYHigh - PtYLow;
			TempYLow = PtYLow - IntervalNumberLowY;
			TempYHigh = 1.0;
			wt = (1.0 - TempYLow) / PtYRange;
			SumWt += wt;
			if (IntervalNumberLowY)
				{
				if (TileHeight && CoordSpace != WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
					{
					if (FlipHeight && IntervalNumberLowY % 2)
						{
						TempYLow = 1.0 - TempYLow;
						TempYHigh = 0.0;
						Swap64(TempYLow, TempYHigh);
						} // if
					} // if
				else
					{
					DontSample = 1;
					} // else
				} // if
			if (! DontSample)
				{
				if (ReverseHeight)
					{
					TempYLow = 1.0 - TempYLow;
					TempYHigh = 1.0 - TempYHigh;
					Swap64(TempYLow, TempYHigh);
					} // if
				TempValue = MyRast->SampleRangeByteDouble3(TempOutput, PtXLow, PtXHigh, 1.0 - TempYHigh, 1.0 - TempYLow, Abort);	// 1 - PtY flips image right-side up
				Output[0] = TempOutput[0] * wt;
				Output[1] = TempOutput[1] * wt;
				Output[2] = TempOutput[2] * wt;
				Value = TempValue * wt;
				// if image was not found and user chose not to select a new one, Abort will be set.
				if (Abort)
					{
					// disable so it doesn't happen again
					Enabled = 0;
					} // if
				} // if
			// high y
			TempOutput[0] = TempOutput[1] = TempOutput[2] = 0.0;
			TempValue = 0.0;
			DontSample = 0;
			TempYHigh = PtYHigh - IntervalNumberHighY;
			TempYLow = 0.0;
			wt = TempYHigh / PtYRange;
			SumWt += wt;
			if (IntervalNumberHighY)
				{
				if (TileHeight && CoordSpace != WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
					{
					if (FlipHeight && IntervalNumberHighY % 2)
						{
						TempYHigh = 1.0 - TempYHigh;
						TempYLow = 1.0;
						Swap64(TempYHigh, TempYLow);
						} // if
					} // if
				else
					{
					DontSample = 1;
					} // else
				} // if
			if (! DontSample && ! Abort)
				{
				if (ReverseHeight)
					{
					TempYLow = 1.0 - TempYLow;
					TempYHigh = 1.0 - TempYHigh;
					Swap64(TempYLow, TempYHigh);
					} // if
				TempValue = MyRast->SampleRangeByteDouble3(TempOutput, PtXLow, PtXHigh, 1.0 - TempYHigh, 1.0 - TempYLow, Abort);	// 1 - PtY flips image right-side up
				Output[0] += TempOutput[0] * wt;
				Output[1] += TempOutput[1] * wt;
				Output[2] += TempOutput[2] * wt;
				Value += TempValue * wt;
				// if image was not found and user chose not to select a new one, Abort will be set.
				if (Abort)
					{
					// disable so it doesn't happen again
					Enabled = 0;
					} // if
				} // if
			if (SumWt < 1.0)
				{
				SumWt = 1.0 - SumWt;
				Output[0] += (MyRast->GetAverageColor(0) * MyRast->GetAverageCoverage() * SumWt);
				Output[1] += (MyRast->GetAverageColor(1) * MyRast->GetAverageCoverage() * SumWt);
				Output[2] += (MyRast->GetAverageColor(2) * MyRast->GetAverageCoverage() * SumWt);
				Value += MyRast->GetAverageCoverage() * SumWt;
				} // if
			} // else same x, different y
		} // if same x
	else
		{
		if (IntervalNumberLowY == IntervalNumberHighY)
			{
			if (IntervalNumberLowY)
				{
				if (TileHeight && CoordSpace != WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
					{
					PtYLow -= IntervalNumberLowY;
					PtYHigh -= IntervalNumberHighY;
					if (FlipHeight && IntervalNumberLowY % 2)
						{
						PtYLow = 1.0 - PtYLow;
						PtYHigh = 1.0 - PtYHigh;
						Swap64(PtYLow, PtYHigh);
						} // if
					} // if
				else
					{
					return (0.0);
					} // else if
				} // if
			if (ReverseHeight)
				{
				PtYLow = 1.0 - PtYLow;
				PtYHigh = 1.0 - PtYHigh;
				Swap64(PtYLow, PtYHigh);
				} // if
			// two passes of x
			// low x
			TempOutput[0] = TempOutput[1] = TempOutput[2] = 0.0;
			SumWt = Value = TempValue = 0.0;
			DontSample = 0;
			PtXRange = PtXHigh - PtXLow;
			TempXLow = PtXLow - IntervalNumberLowX;
			TempXHigh = 1.0;
			wt = (1.0 - TempXLow) / PtXRange;
			SumWt += wt;
			if (IntervalNumberLowX)
				{
				if (TileWidth && CoordSpace != WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
					{
					if (FlipWidth && IntervalNumberLowX % 2)
						{
						TempXLow = 1.0 - TempXLow;
						TempXHigh = 0.0;
						Swap64(TempXLow, TempXHigh);
						} // if
					} // if
				else
					{
					DontSample = 1;
					} // else
				} // if
			if (! DontSample)
				{
				if (ReverseWidth)
					{
					TempXLow = 1.0 - TempXLow;
					TempXHigh = 1.0 - TempXHigh;
					Swap64(TempXLow, TempXHigh);
					} // if
				TempValue = MyRast->SampleRangeByteDouble3(TempOutput, TempXLow, TempXHigh, 1.0 - PtYHigh, 1.0 - PtYLow, Abort);	// 1 - PtY flips image right-side up
				Output[0] = TempOutput[0] * wt;
				Output[1] = TempOutput[1] * wt;
				Output[2] = TempOutput[2] * wt;
				Value = TempValue * wt;
				// if image was not found and user chose not to select a new one, Abort will be set.
				if (Abort)
					{
					// disable so it doesn't happen again
					Enabled = 0;
					} // if
				} // if
			// high x
			TempOutput[0] = TempOutput[1] = TempOutput[2] = 0.0;
			TempValue = 0.0;
			DontSample = 0;
			TempXHigh = PtXHigh - IntervalNumberHighX;
			TempXLow = 0.0;
			wt = TempXHigh / PtXRange;
			SumWt += wt;
			if (IntervalNumberHighX)
				{
				if (TileWidth && CoordSpace != WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
					{
					if (FlipWidth && IntervalNumberHighX % 2)
						{
						TempXHigh = 1.0 - TempXHigh;
						TempXLow = 1.0;
						Swap64(TempXHigh, TempXLow);
						} // if
					} // if
				else
					{
					DontSample = 1;
					} // else
				} // if
			if (! DontSample && ! Abort)
				{
				if (ReverseWidth)
					{
					TempXLow = 1.0 - TempXLow;
					TempXHigh = 1.0 - TempXHigh;
					Swap64(TempXLow, TempXHigh);
					} // if
				TempValue = MyRast->SampleRangeByteDouble3(TempOutput, TempXLow, TempXHigh, 1.0 - PtYHigh, 1.0 - PtYLow, Abort);	// 1 - PtY flips image right-side up
				Output[0] += TempOutput[0] * wt;
				Output[1] += TempOutput[1] * wt;
				Output[2] += TempOutput[2] * wt;
				Value += TempValue * wt;
				// if image was not found and user chose not to select a new one, Abort will be set.
				if (Abort)
					{
					// disable so it doesn't happen again
					Enabled = 0;
					} // if
				} // if
			if (SumWt < 1.0)
				{
				SumWt = 1.0 - SumWt;
				Output[0] += (MyRast->GetAverageColor(0) * MyRast->GetAverageCoverage() * SumWt);
				Output[1] += (MyRast->GetAverageColor(1) * MyRast->GetAverageCoverage() * SumWt);
				Output[2] += (MyRast->GetAverageColor(2) * MyRast->GetAverageCoverage() * SumWt);
				Value += MyRast->GetAverageCoverage() * SumWt;
				} // if
			} // if different x, same y
		else
			{
			// low x
			TempOutput[0] = TempOutput[1] = TempOutput[2] = 0.0;
			SumWt = Value = TempValue = 0.0;
			DontSample = 0;
			PtXRange = PtXHigh - PtXLow;
			TempXLow = PtXLow - IntervalNumberLowX;
			TempXHigh = 1.0;
			wtx = (1.0 - TempXLow) / PtXRange;
			if (IntervalNumberLowX)
				{
				if (TileWidth && CoordSpace != WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
					{
					if (FlipWidth && IntervalNumberLowX % 2)
						{
						TempXLow = 1.0 - TempXLow;
						TempXHigh = 0.0;
						Swap64(TempXLow, TempXHigh);
						} // if
					} // if
				else
					{
					DontSample = 1;
					} // else
				} // if
			if (ReverseWidth)
				{
				TempXLow = 1.0 - TempXLow;
				TempXHigh = 1.0 - TempXHigh;
				Swap64(TempXLow, TempXHigh);
				} // if
			// low y
			DontSample2 = 0;
			PtYRange = PtYHigh - PtYLow;
			TempYLow = PtYLow - IntervalNumberLowY;
			TempYHigh = 1.0;
			wt = wtx * (1.0 - TempYLow) / PtYRange;
			SumWt += wt;
			if (IntervalNumberLowY)
				{
				if (TileHeight && CoordSpace != WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
					{
					if (FlipHeight && IntervalNumberLowY % 2)
						{
						TempYLow = 1.0 - TempYLow;
						TempYHigh = 0.0;
						Swap64(TempYLow, TempYHigh);
						} // if
					} // if
				else
					{
					DontSample2 = 1;
					} // else
				} // if
			if (! DontSample || ! DontSample2)
				{
				if (ReverseHeight)
					{
					TempYLow = 1.0 - TempYLow;
					TempYHigh = 1.0 - TempYHigh;
					Swap64(TempYLow, TempYHigh);
					} // if
				TempValue = MyRast->SampleRangeByteDouble3(TempOutput, TempXLow, TempXHigh, 1.0 - TempYHigh, 1.0 - TempYLow, Abort);	// 1 - PtY flips image right-side up
				Output[0] = TempOutput[0] * wt;
				Output[1] = TempOutput[1] * wt;
				Output[2] = TempOutput[2] * wt;
				Value = TempValue * wt;
				// if image was not found and user chose not to select a new one, Abort will be set.
				if (Abort)
					{
					// disable so it doesn't happen again
					Enabled = 0;
					} // if
				} // if
			// high y
			TempOutput[0] = TempOutput[1] = TempOutput[2] = 0.0;
			TempValue = 0.0;
			DontSample2 = 0;
			TempYHigh = PtYHigh - IntervalNumberHighY;
			TempYLow = 0.0;
			wt = wtx * TempYHigh / PtYRange;
			SumWt += wt;
			if (IntervalNumberHighY)
				{
				if (TileHeight && CoordSpace != WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
					{
					if (FlipHeight && IntervalNumberHighY % 2)
						{
						TempYHigh = 1.0 - TempYHigh;
						TempYLow = 1.0;
						Swap64(TempYHigh, TempYLow);
						} // if
					} // if
				else
					{
					DontSample2 = 1;
					} // else
				} // if
			if ((! DontSample || ! DontSample2) && ! Abort)
				{
				if (ReverseHeight)
					{
					TempYLow = 1.0 - TempYLow;
					TempYHigh = 1.0 - TempYHigh;
					Swap64(TempYLow, TempYHigh);
					} // if
				TempValue = MyRast->SampleRangeByteDouble3(TempOutput, TempXLow, TempXHigh, 1.0 - TempYHigh, 1.0 - TempYLow, Abort);	// 1 - PtY flips image right-side up
				Output[0] += TempOutput[0] * wt;
				Output[1] += TempOutput[1] * wt;
				Output[2] += TempOutput[2] * wt;
				Value += TempValue * wt;
				// if image was not found and user chose not to select a new one, Abort will be set.
				if (Abort)
					{
					// disable so it doesn't happen again
					Enabled = 0;
					} // if
				} // if

			// high x
			TempOutput[0] = TempOutput[1] = TempOutput[2] = 0.0;
			TempValue = 0.0;
			DontSample = 0;
			TempXHigh = PtXHigh - IntervalNumberHighX;
			TempXLow = 0.0;
			wtx = TempXHigh / PtXRange;
			if (IntervalNumberHighX)
				{
				if (TileWidth && CoordSpace != WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
					{
					if (FlipWidth && IntervalNumberHighX % 2)
						{
						TempXHigh = 1.0 - TempXHigh;
						TempXLow = 1.0;
						Swap64(TempXHigh, TempXLow);
						} // if
					} // if
				else
					{
					DontSample = 1;
					} // else
				} // if
			if (ReverseWidth)
				{
				TempXLow = 1.0 - TempXLow;
				TempXHigh = 1.0 - TempXHigh;
				Swap64(TempXLow, TempXHigh);
				} // if
			// low y
			DontSample2 = 0;
			PtYRange = PtYHigh - PtYLow;
			TempYLow = PtYLow - IntervalNumberLowY;
			TempYHigh = 1.0;
			wt = wtx * (1.0 - TempYLow) / PtYRange;
			SumWt += wt;
			if (IntervalNumberLowY)
				{
				if (TileHeight && CoordSpace != WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
					{
					if (FlipHeight && IntervalNumberLowY % 2)
						{
						TempYLow = 1.0 - TempYLow;
						TempYHigh = 0.0;
						Swap64(TempYLow, TempYHigh);
						} // if
					} // if
				else
					{
					DontSample2 = 1;
					} // else
				} // if
			if ((! DontSample || ! DontSample2) && ! Abort)
				{
				if (ReverseHeight)
					{
					TempYLow = 1.0 - TempYLow;
					TempYHigh = 1.0 - TempYHigh;
					Swap64(TempYLow, TempYHigh);
					} // if
				TempValue = MyRast->SampleRangeByteDouble3(TempOutput, TempXLow, TempXHigh, 1.0 - TempYHigh, 1.0 - TempYLow, Abort);	// 1 - PtY flips image right-side up
				Output[0] += TempOutput[0] * wt;
				Output[1] += TempOutput[1] * wt;
				Output[2] += TempOutput[2] * wt;
				Value += TempValue * wt;
				// if image was not found and user chose not to select a new one, Abort will be set.
				if (Abort)
					{
					// disable so it doesn't happen again
					Enabled = 0;
					} // if
				} // if
			// high y
			TempOutput[0] = TempOutput[1] = TempOutput[2] = 0.0;
			TempValue = 0.0;
			DontSample2 = 0;
			TempYHigh = PtYHigh - IntervalNumberHighY;
			TempYLow = 0.0;
			wt = wtx * TempYHigh / PtYRange;
			SumWt += wt;
			if (IntervalNumberHighY)
				{
				if (TileHeight && CoordSpace != WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
					{
					if (FlipHeight && IntervalNumberHighY % 2)
						{
						TempYHigh = 1.0 - TempYHigh;
						TempYLow = 1.0;
						Swap64(TempYHigh, TempYLow);
						} // if
					} // if
				else
					{
					DontSample2 = 1;
					} // else
				} // if
			if ((! DontSample || ! DontSample2) && ! Abort)
				{
				if (ReverseHeight)
					{
					TempYLow = 1.0 - TempYLow;
					TempYHigh = 1.0 - TempYHigh;
					Swap64(TempYLow, TempYHigh);
					} // if
				TempValue = MyRast->SampleRangeByteDouble3(TempOutput, TempXLow, TempXHigh, 1.0 - TempYHigh, 1.0 - TempYLow, Abort);	// 1 - PtY flips image right-side up
				Output[0] += TempOutput[0] * wt;
				Output[1] += TempOutput[1] * wt;
				Output[2] += TempOutput[2] * wt;
				Value += TempValue * wt;
				// if image was not found and user chose not to select a new one, Abort will be set.
				if (Abort)
					{
					// disable so it doesn't happen again
					Enabled = 0;
					} // if
				} // if

			if (SumWt < 1.0)
				{
				SumWt = 1.0 - SumWt;
				Output[0] += (MyRast->GetAverageColor(0) * MyRast->GetAverageCoverage() * SumWt);
				Output[1] += (MyRast->GetAverageColor(1) * MyRast->GetAverageCoverage() * SumWt);
				Output[2] += (MyRast->GetAverageColor(2) * MyRast->GetAverageCoverage() * SumWt);
				Value += MyRast->GetAverageCoverage() * SumWt;
				} // if
			} // else different x, different y
		} // else different x
	} // else need to sample range of image

if (ImageNeg)
	{
	Output[0] = 1.0 - Output[0];
	Output[1] = 1.0 - Output[1];
	Output[2] = 1.0 - Output[2];
	} // if
if (! ApplyToColor)
	{
	Output[0] = (Output[0] + Output[1] + Output[2]) * (1.0 / 3.0);
	Output[1] = Output[2] = Output[0];
	} // if

return (AnalyzeImageWrapup(Output, Input, Value, Data, EvalChildren));
//return (Value);

} // PlanarImageTexture::Analyze

/*===========================================================================*/

UVImageTexture::UVImageTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_UVW)
{

SetDefaults(ParamNumber);

} // UVImageTexture::UVImageTexture

/*===========================================================================*/

UVImageTexture::UVImageTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_UVW, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // UVImageTexture::UVImageTexture

/*===========================================================================*/

void UVImageTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	0, 0,	// 1
	0, 0,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	0, 0,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 12
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",	// 0
	"", "",	// 1
	"", "",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",	// 9
	"", "",	// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_UVW;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // UVImageTexture::SetDefaults

/*===========================================================================*/

char UVImageTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // UVImageTexture::GetTextureParamType

/*===========================================================================*/

double UVImageTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double PtXLow, PtXHigh, PtYLow, PtYHigh, Value, TempValue, TempXLow, TempXHigh, TempYLow, TempYHigh, TempOutput[3],
	wt, wtx, SumWt, PtXRange, PtYRange, TempOpacity;
long IntervalNumberLowX, IntervalNumberHighX, IntervalNumberLowY, IntervalNumberHighY;
int Abort = 0;
char DontSample, DontSample2, LLBounds = 0;
Raster *MyRast;

Value = Output[0] = Output[1] = Output[2] = 0.0;
if (! Img || ! (MyRast = Img->GetRaster()))
	{
	return (0.0);
	} // if
if (Data->IsSampleThumbnail && MyRast->ImageManagerEnabled 
	&& MyRast->GetAverageColor(0) == .5
	&& MyRast->GetAverageColor(1) == .5
	&& MyRast->GetAverageColor(2) == .5
	&& MyRast->GetAverageCoverage() == 1.0)
	{
	// shares standin
	MyRast = GlobalApp->AppImages->FetchStandInRast();
	} // if

//if (CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
//	{
//	if ((TempOpacity = AdjustSampleForLatLonBounds(LowX, HighX, LowY, HighY, LowZ, HighZ, Data, Extraction)) <= 0.0)
//		return (0.0);
//	LLBounds = 1;
//	} // if
//else
//	{
	TempOpacity = 1.0;
	//LowX -= TCenter[0];
	//HighX -= TCenter[0];
	//LowY -= TCenter[1];
	//HighY -= TCenter[1];
	//LowZ -= TCenter[2];
	//HighZ -= TCenter[2];

	//if (TextureMoving())
	//	{
	//	AdjustSampleForVelocity(LowX, HighX, LowY, HighY, LowZ, HighZ);
	//	} // if

	//if (Rotated)
	//	{
	//	RotatePoint(LowX, LowY, LowZ, RotMatx);
	//	RotatePoint(HighX, HighY, HighZ, RotMatx);
	//	} // if

//	LowX /= TSize[0];
//	HighX /= TSize[0];
//	LowY /= TSize[1];
//	HighY /= TSize[1];
//	LowZ /= TSize[2];
//	HighZ /= TSize[2];
//	} // else not lat/lon bounds

// removed the restirction on align to vector's axis alignemtn 11/5/00 to 
// allow things like highway retaining walls to be textured with image aligned to x axis 
// or road stripes with image aligned to y
//if (TexAxis == 2 || CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
//	{
	PtXLow = LowX;
	PtXHigh = HighX;
	PtYLow = LowY;
	PtYHigh = HighY;
//	} // else
/*
else if (TexAxis == 0)
	{
	if (ApplyToEcosys)
		{
		PtXLow = LowY;
		PtXHigh = HighY;
		PtYLow = LowZ;
		PtYHigh = HighZ;
		} // if
	else
		{
		PtXLow = LowZ;
		PtXHigh = HighZ;
		PtYLow = LowY;
		PtYHigh = HighY;
		} // else
	} // if
else
	{
	PtXLow = LowX;
	PtXHigh = HighX;
	PtYLow = LowZ;
	PtYHigh = HighZ;
	} // else if
*/
if (PtXLow > PtXHigh)
	Swap64(PtXLow, PtXHigh);
if (PtYLow > PtYHigh)
	Swap64(PtYLow, PtYHigh);

// without antialiasing
if (! Antialias || WCS_SinglePoint2D(PtXLow, PtXHigh, PtYLow, PtYHigh))
	{
	PtXLow = (PtXLow + PtXHigh) * .5;
	PtYLow = (PtYLow + PtYHigh) * .5;
	if (! LLBounds)
		{
		PtXLow += .5;		// shift to image coord space so that the texture center 0,0 is at the image center
		PtYLow += .5;
		} // if

	if (PtXLow < 0.0 || PtXLow > 1.0)
		{
		if (IntervalNumberLowX = quicklongfloor(PtXLow))
			{
			if (TileWidth)
				{
				PtXLow -= IntervalNumberLowX;
				if (FlipWidth && IntervalNumberLowX % 2)
					{
					PtXLow = 1.0 - PtXLow;
					} // if
				} // if
			else
				{
				return (0.0);
				} // else if
			} // if
		} // if
	if (PtYLow < 0.0 || PtYLow > 1.0)
		{
		if (IntervalNumberLowY = quicklongfloor(PtYLow))
			{
			if (TileHeight)
				{
				PtYLow -= IntervalNumberLowY;
				if (FlipHeight && IntervalNumberLowY % 2)
					{
					PtYLow = 1.0 - PtYLow;
					} // if
				} // if
			else
				{
				return (0.0);
				} // else if
			} // if
		} // if
	if (ReverseWidth)
		{
		PtXLow = 1.0 - PtXLow;
		} // if
	if (ReverseHeight)
		{
		PtYLow = 1.0 - PtYLow;
		} // if
	if (PixelBlend)
		Value = MyRast->SampleBlendDouble3(Output, PtXLow, 1.0 - PtYLow, Abort);	// 1 - PtY flips image right-side up
	else
		Value = MyRast->SampleByteDouble3(Output, PtXLow, 1.0 - PtYLow, Abort);	// 1 - PtY flips image right-side up
	// if image was not found and user chose not to select a new one, Abort will be set.
	if (Abort)
		{
		// disable so it doesn't happen again
		Enabled = 0;
		} // if
	} // if
else
	{
	if (! LLBounds)
		{
		PtXLow += .5;		// shift to image coord space so that the texture center 0,0 is at the image center
		PtXHigh += .5;		// shift to image coord space so that the texture center 0,0 is at the image center
		PtYLow += .5;
		PtYHigh += .5;
		} // if
	IntervalNumberLowX = quicklongfloor(PtXLow);
	IntervalNumberHighX = quicklongfloor(PtXHigh);
	IntervalNumberLowY = quicklongfloor(PtYLow);
	IntervalNumberHighY = quicklongfloor(PtYHigh);
	if (IntervalNumberLowX == IntervalNumberHighX)
		{
		if (IntervalNumberLowX)
			{
			if (TileWidth)
				{
				PtXLow -= IntervalNumberLowX;
				PtXHigh -= IntervalNumberHighX;
				if (FlipWidth && IntervalNumberLowX % 2)
					{
					PtXLow = 1.0 - PtXLow;
					PtXHigh = 1.0 - PtXHigh;
					Swap64(PtXLow, PtXHigh);
					} // if
				} // if
			else
				{
				return (0.0);
				} // else if
			} // if
		if (ReverseWidth)
			{
			PtXLow = 1.0 - PtXLow;
			PtXHigh = 1.0 - PtXHigh;
			Swap64(PtXLow, PtXHigh);
			} // if
		if (IntervalNumberLowY == IntervalNumberHighY)
			{
			if (IntervalNumberLowY)
				{
				if (TileHeight)
					{
					PtYLow -= IntervalNumberLowY;
					PtYHigh -= IntervalNumberHighY;
					if (FlipHeight && IntervalNumberLowY % 2)
						{
						PtYLow = 1.0 - PtYLow;
						PtYHigh = 1.0 - PtYHigh;
						Swap64(PtYLow, PtYHigh);
						} // if
					} // if
				else
					{
					return (0.0);
					} // else if
				} // if
			if (ReverseHeight)
				{
				PtYLow = 1.0 - PtYLow;
				PtYHigh = 1.0 - PtYHigh;
				Swap64(PtYLow, PtYHigh);
				} // if
			Value = MyRast->SampleRangeByteDouble3(Output, PtXLow, PtXHigh, 1.0 - PtYHigh, 1.0 - PtYLow, Abort);	// 1 - PtY flips image right-side up
			// if image was not found and user chose not to select a new one, Abort will be set.
			if (Abort)
				{
				// disable so it doesn't happen again
				Enabled = 0;
				} // if
			} // if same x and y
		else
			{
			// two passes of y
			// low y
			TempOutput[0] = TempOutput[1] = TempOutput[2] = 0.0;
			SumWt = Value = TempValue = 0.0;
			DontSample = 0;
			PtYRange = PtYHigh - PtYLow;
			TempYLow = PtYLow - IntervalNumberLowY;
			TempYHigh = 1.0;
			wt = (1.0 - TempYLow) / PtYRange;
			SumWt += wt;
			if (IntervalNumberLowY)
				{
				if (TileHeight)
					{
					if (FlipHeight && IntervalNumberLowY % 2)
						{
						TempYLow = 1.0 - TempYLow;
						TempYHigh = 0.0;
						Swap64(TempYLow, TempYHigh);
						} // if
					} // if
				else
					{
					DontSample = 1;
					} // else
				} // if
			if (! DontSample)
				{
				if (ReverseHeight)
					{
					TempYLow = 1.0 - TempYLow;
					TempYHigh = 1.0 - TempYHigh;
					Swap64(TempYLow, TempYHigh);
					} // if
				TempValue = MyRast->SampleRangeByteDouble3(TempOutput, PtXLow, PtXHigh, 1.0 - TempYHigh, 1.0 - TempYLow, Abort);	// 1 - PtY flips image right-side up
				Output[0] = TempOutput[0] * wt;
				Output[1] = TempOutput[1] * wt;
				Output[2] = TempOutput[2] * wt;
				Value = TempValue * wt;
				// if image was not found and user chose not to select a new one, Abort will be set.
				if (Abort)
					{
					// disable so it doesn't happen again
					Enabled = 0;
					} // if
				} // if
			// high y
			TempOutput[0] = TempOutput[1] = TempOutput[2] = 0.0;
			TempValue = 0.0;
			DontSample = 0;
			TempYHigh = PtYHigh - IntervalNumberHighY;
			TempYLow = 0.0;
			wt = TempYHigh / PtYRange;
			SumWt += wt;
			if (IntervalNumberHighY)
				{
				if (TileHeight)
					{
					if (FlipHeight && IntervalNumberHighY % 2)
						{
						TempYHigh = 1.0 - TempYHigh;
						TempYLow = 1.0;
						Swap64(TempYHigh, TempYLow);
						} // if
					} // if
				else
					{
					DontSample = 1;
					} // else
				} // if
			if (! DontSample && ! Abort)
				{
				if (ReverseHeight)
					{
					TempYLow = 1.0 - TempYLow;
					TempYHigh = 1.0 - TempYHigh;
					Swap64(TempYLow, TempYHigh);
					} // if
				TempValue = MyRast->SampleRangeByteDouble3(TempOutput, PtXLow, PtXHigh, 1.0 - TempYHigh, 1.0 - TempYLow, Abort);	// 1 - PtY flips image right-side up
				Output[0] += TempOutput[0] * wt;
				Output[1] += TempOutput[1] * wt;
				Output[2] += TempOutput[2] * wt;
				Value += TempValue * wt;
				// if image was not found and user chose not to select a new one, Abort will be set.
				if (Abort)
					{
					// disable so it doesn't happen again
					Enabled = 0;
					} // if
				} // if
			if (SumWt < 1.0)
				{
				SumWt = 1.0 - SumWt;
				Output[0] += (MyRast->GetAverageColor(0) * MyRast->GetAverageCoverage() * SumWt);
				Output[1] += (MyRast->GetAverageColor(1) * MyRast->GetAverageCoverage() * SumWt);
				Output[2] += (MyRast->GetAverageColor(2) * MyRast->GetAverageCoverage() * SumWt);
				Value += MyRast->GetAverageCoverage() * SumWt;
				} // if
			} // else same x, different y
		} // if same x
	else
		{
		if (IntervalNumberLowY == IntervalNumberHighY)
			{
			if (IntervalNumberLowY)
				{
				if (TileHeight)
					{
					PtYLow -= IntervalNumberLowY;
					PtYHigh -= IntervalNumberHighY;
					if (FlipHeight && IntervalNumberLowY % 2)
						{
						PtYLow = 1.0 - PtYLow;
						PtYHigh = 1.0 - PtYHigh;
						Swap64(PtYLow, PtYHigh);
						} // if
					} // if
				else
					{
					return (0.0);
					} // else if
				} // if
			if (ReverseHeight)
				{
				PtYLow = 1.0 - PtYLow;
				PtYHigh = 1.0 - PtYHigh;
				Swap64(PtYLow, PtYHigh);
				} // if
			// two passes of x
			// low x
			TempOutput[0] = TempOutput[1] = TempOutput[2] = 0.0;
			SumWt = Value = TempValue = 0.0;
			DontSample = 0;
			PtXRange = PtXHigh - PtXLow;
			TempXLow = PtXLow - IntervalNumberLowX;
			TempXHigh = 1.0;
			wt = (1.0 - TempXLow) / PtXRange;
			SumWt += wt;
			if (IntervalNumberLowX)
				{
				if (TileWidth)
					{
					if (FlipWidth && IntervalNumberLowX % 2)
						{
						TempXLow = 1.0 - TempXLow;
						TempXHigh = 0.0;
						Swap64(TempXLow, TempXHigh);
						} // if
					} // if
				else
					{
					DontSample = 1;
					} // else
				} // if
			if (! DontSample)
				{
				if (ReverseWidth)
					{
					TempXLow = 1.0 - TempXLow;
					TempXHigh = 1.0 - TempXHigh;
					Swap64(TempXLow, TempXHigh);
					} // if
				TempValue = MyRast->SampleRangeByteDouble3(TempOutput, TempXLow, TempXHigh, 1.0 - PtYHigh, 1.0 - PtYLow, Abort);	// 1 - PtY flips image right-side up
				Output[0] = TempOutput[0] * wt;
				Output[1] = TempOutput[1] * wt;
				Output[2] = TempOutput[2] * wt;
				Value = TempValue * wt;
				// if image was not found and user chose not to select a new one, Abort will be set.
				if (Abort)
					{
					// disable so it doesn't happen again
					Enabled = 0;
					} // if
				} // if
			// high x
			TempOutput[0] = TempOutput[1] = TempOutput[2] = 0.0;
			TempValue = 0.0;
			DontSample = 0;
			TempXHigh = PtXHigh - IntervalNumberHighX;
			TempXLow = 0.0;
			wt = TempXHigh / PtXRange;
			SumWt += wt;
			if (IntervalNumberHighX)
				{
				if (TileWidth)
					{
					if (FlipWidth && IntervalNumberHighX % 2)
						{
						TempXHigh = 1.0 - TempXHigh;
						TempXLow = 1.0;
						Swap64(TempXHigh, TempXLow);
						} // if
					} // if
				else
					{
					DontSample = 1;
					} // else
				} // if
			if (! DontSample && ! Abort)
				{
				if (ReverseWidth)
					{
					TempXLow = 1.0 - TempXLow;
					TempXHigh = 1.0 - TempXHigh;
					Swap64(TempXLow, TempXHigh);
					} // if
				TempValue = MyRast->SampleRangeByteDouble3(TempOutput, TempXLow, TempXHigh, 1.0 - PtYHigh, 1.0 - PtYLow, Abort);	// 1 - PtY flips image right-side up
				Output[0] += TempOutput[0] * wt;
				Output[1] += TempOutput[1] * wt;
				Output[2] += TempOutput[2] * wt;
				Value += TempValue * wt;
				// if image was not found and user chose not to select a new one, Abort will be set.
				if (Abort)
					{
					// disable so it doesn't happen again
					Enabled = 0;
					} // if
				} // if
			if (SumWt < 1.0)
				{
				SumWt = 1.0 - SumWt;
				Output[0] += (MyRast->GetAverageColor(0) * MyRast->GetAverageCoverage() * SumWt);
				Output[1] += (MyRast->GetAverageColor(1) * MyRast->GetAverageCoverage() * SumWt);
				Output[2] += (MyRast->GetAverageColor(2) * MyRast->GetAverageCoverage() * SumWt);
				Value += MyRast->GetAverageCoverage() * SumWt;
				} // if
			} // if different x, same y
		else
			{
			// low x
			TempOutput[0] = TempOutput[1] = TempOutput[2] = 0.0;
			SumWt = Value = TempValue = 0.0;
			DontSample = 0;
			PtXRange = PtXHigh - PtXLow;
			TempXLow = PtXLow - IntervalNumberLowX;
			TempXHigh = 1.0;
			wtx = (1.0 - TempXLow) / PtXRange;
			if (IntervalNumberLowX)
				{
				if (TileWidth)
					{
					if (FlipWidth && IntervalNumberLowX % 2)
						{
						TempXLow = 1.0 - TempXLow;
						TempXHigh = 0.0;
						Swap64(TempXLow, TempXHigh);
						} // if
					} // if
				else
					{
					DontSample = 1;
					} // else
				} // if
			if (ReverseWidth)
				{
				TempXLow = 1.0 - TempXLow;
				TempXHigh = 1.0 - TempXHigh;
				Swap64(TempXLow, TempXHigh);
				} // if
			// low y
			DontSample2 = 0;
			PtYRange = PtYHigh - PtYLow;
			TempYLow = PtYLow - IntervalNumberLowY;
			TempYHigh = 1.0;
			wt = wtx * (1.0 - TempYLow) / PtYRange;
			SumWt += wt;
			if (IntervalNumberLowY)
				{
				if (TileHeight)
					{
					if (FlipHeight && IntervalNumberLowY % 2)
						{
						TempYLow = 1.0 - TempYLow;
						TempYHigh = 0.0;
						Swap64(TempYLow, TempYHigh);
						} // if
					} // if
				else
					{
					DontSample2 = 1;
					} // else
				} // if
			if (! DontSample || ! DontSample2)
				{
				if (ReverseHeight)
					{
					TempYLow = 1.0 - TempYLow;
					TempYHigh = 1.0 - TempYHigh;
					Swap64(TempYLow, TempYHigh);
					} // if
				TempValue = MyRast->SampleRangeByteDouble3(TempOutput, TempXLow, TempXHigh, 1.0 - TempYHigh, 1.0 - TempYLow, Abort);	// 1 - PtY flips image right-side up
				Output[0] = TempOutput[0] * wt;
				Output[1] = TempOutput[1] * wt;
				Output[2] = TempOutput[2] * wt;
				Value = TempValue * wt;
				// if image was not found and user chose not to select a new one, Abort will be set.
				if (Abort)
					{
					// disable so it doesn't happen again
					Enabled = 0;
					} // if
				} // if
			// high y
			TempOutput[0] = TempOutput[1] = TempOutput[2] = 0.0;
			TempValue = 0.0;
			DontSample2 = 0;
			TempYHigh = PtYHigh - IntervalNumberHighY;
			TempYLow = 0.0;
			wt = wtx * TempYHigh / PtYRange;
			SumWt += wt;
			if (IntervalNumberHighY)
				{
				if (TileHeight)
					{
					if (FlipHeight && IntervalNumberHighY % 2)
						{
						TempYHigh = 1.0 - TempYHigh;
						TempYLow = 1.0;
						Swap64(TempYHigh, TempYLow);
						} // if
					} // if
				else
					{
					DontSample2 = 1;
					} // else
				} // if
			if ((! DontSample || ! DontSample2) && ! Abort)
				{
				if (ReverseHeight)
					{
					TempYLow = 1.0 - TempYLow;
					TempYHigh = 1.0 - TempYHigh;
					Swap64(TempYLow, TempYHigh);
					} // if
				TempValue = MyRast->SampleRangeByteDouble3(TempOutput, TempXLow, TempXHigh, 1.0 - TempYHigh, 1.0 - TempYLow, Abort);	// 1 - PtY flips image right-side up
				Output[0] += TempOutput[0] * wt;
				Output[1] += TempOutput[1] * wt;
				Output[2] += TempOutput[2] * wt;
				Value += TempValue * wt;
				// if image was not found and user chose not to select a new one, Abort will be set.
				if (Abort)
					{
					// disable so it doesn't happen again
					Enabled = 0;
					} // if
				} // if

			// high x
			TempOutput[0] = TempOutput[1] = TempOutput[2] = 0.0;
			TempValue = 0.0;
			DontSample = 0;
			TempXHigh = PtXHigh - IntervalNumberHighX;
			TempXLow = 0.0;
			wtx = TempXHigh / PtXRange;
			if (IntervalNumberHighX)
				{
				if (TileWidth)
					{
					if (FlipWidth && IntervalNumberHighX % 2)
						{
						TempXHigh = 1.0 - TempXHigh;
						TempXLow = 1.0;
						Swap64(TempXHigh, TempXLow);
						} // if
					} // if
				else
					{
					DontSample = 1;
					} // else
				} // if
			if (ReverseWidth)
				{
				TempXLow = 1.0 - TempXLow;
				TempXHigh = 1.0 - TempXHigh;
				Swap64(TempXLow, TempXHigh);
				} // if
			// low y
			DontSample2 = 0;
			PtYRange = PtYHigh - PtYLow;
			TempYLow = PtYLow - IntervalNumberLowY;
			TempYHigh = 1.0;
			wt = wtx * (1.0 - TempYLow) / PtYRange;
			SumWt += wt;
			if (IntervalNumberLowY)
				{
				if (TileHeight)
					{
					if (FlipHeight && IntervalNumberLowY % 2)
						{
						TempYLow = 1.0 - TempYLow;
						TempYHigh = 0.0;
						Swap64(TempYLow, TempYHigh);
						} // if
					} // if
				else
					{
					DontSample2 = 1;
					} // else
				} // if
			if ((! DontSample || ! DontSample2) && ! Abort)
				{
				if (ReverseHeight)
					{
					TempYLow = 1.0 - TempYLow;
					TempYHigh = 1.0 - TempYHigh;
					Swap64(TempYLow, TempYHigh);
					} // if
				TempValue = MyRast->SampleRangeByteDouble3(TempOutput, TempXLow, TempXHigh, 1.0 - TempYHigh, 1.0 - TempYLow, Abort);	// 1 - PtY flips image right-side up
				Output[0] += TempOutput[0] * wt;
				Output[1] += TempOutput[1] * wt;
				Output[2] += TempOutput[2] * wt;
				Value += TempValue * wt;
				// if image was not found and user chose not to select a new one, Abort will be set.
				if (Abort)
					{
					// disable so it doesn't happen again
					Enabled = 0;
					} // if
				} // if
			// high y
			TempOutput[0] = TempOutput[1] = TempOutput[2] = 0.0;
			TempValue = 0.0;
			DontSample2 = 0;
			TempYHigh = PtYHigh - IntervalNumberHighY;
			TempYLow = 0.0;
			wt = wtx * TempYHigh / PtYRange;
			SumWt += wt;
			if (IntervalNumberHighY)
				{
				if (TileHeight)
					{
					if (FlipHeight && IntervalNumberHighY % 2)
						{
						TempYHigh = 1.0 - TempYHigh;
						TempYLow = 1.0;
						Swap64(TempYHigh, TempYLow);
						} // if
					} // if
				else
					{
					DontSample2 = 1;
					} // else
				} // if
			if ((! DontSample || ! DontSample2) && ! Abort)
				{
				if (ReverseHeight)
					{
					TempYLow = 1.0 - TempYLow;
					TempYHigh = 1.0 - TempYHigh;
					Swap64(TempYLow, TempYHigh);
					} // if
				TempValue = MyRast->SampleRangeByteDouble3(TempOutput, TempXLow, TempXHigh, 1.0 - TempYHigh, 1.0 - TempYLow, Abort);	// 1 - PtY flips image right-side up
				Output[0] += TempOutput[0] * wt;
				Output[1] += TempOutput[1] * wt;
				Output[2] += TempOutput[2] * wt;
				Value += TempValue * wt;
				// if image was not found and user chose not to select a new one, Abort will be set.
				if (Abort)
					{
					// disable so it doesn't happen again
					Enabled = 0;
					} // if
				} // if

			if (SumWt < 1.0)
				{
				SumWt = 1.0 - SumWt;
				Output[0] += (MyRast->GetAverageColor(0) * MyRast->GetAverageCoverage() * SumWt);
				Output[1] += (MyRast->GetAverageColor(1) * MyRast->GetAverageCoverage() * SumWt);
				Output[2] += (MyRast->GetAverageColor(2) * MyRast->GetAverageCoverage() * SumWt);
				Value += MyRast->GetAverageCoverage() * SumWt;
				} // if
			} // else different x, different y
		} // else different x
	} // else need to sample range of image

if (ImageNeg)
	{
	Output[0] = 1.0 - Output[0];
	Output[1] = 1.0 - Output[1];
	Output[2] = 1.0 - Output[2];
	} // if
if (! ApplyToColor)
	{
	Output[0] = (Output[0] + Output[1] + Output[2]) * (1.0 / 3.0);
	Output[1] = Output[2] = Output[0];
	} // if

return (AnalyzeImageWrapup(Output, Input, Value * TempOpacity, Data, EvalChildren));
//return (Value * TempOpacity);

} // UVImageTexture::Analyze

/*===========================================================================*/

ColorPerVertexTexture::ColorPerVertexTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_COLORPERVERTEX)
{

SetDefaults(ParamNumber);

} // ColorPerVertexTexture::ColorPerVertexTexture

/*===========================================================================*/

ColorPerVertexTexture::ColorPerVertexTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_COLORPERVERTEX, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // ColorPerVertexTexture::ColorPerVertexTexture

/*===========================================================================*/

void ColorPerVertexTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	0, 0,	// 1
	0, 0,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	0, 0,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",	// 0
	"", "",	// 1
	"", "",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",	// 9
	"", "",	// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_COLORPERVERTEX;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // ColorPerVertexTexture::SetDefaults

/*===========================================================================*/

char ColorPerVertexTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // ColorPerVertexTexture::GetTextureParamType

/*===========================================================================*/

double ColorPerVertexTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{

Output[0] = LowX;
Output[1] = LowY;
Output[2] = LowZ;

return (AnalyzeImageWrapup(Output, Input, 1.0, Data, EvalChildren));
//return (1.0);

} // ColorPerVertexTexture::Analyze

/*===========================================================================*/

CylindricalImageTexture::CylindricalImageTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_CYLINDRICALIMAGE)
{

SetDefaults(ParamNumber);

} // CylindricalImageTexture::CylindricalImageTexture

/*===========================================================================*/

CylindricalImageTexture::CylindricalImageTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_CYLINDRICALIMAGE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // CylindricalImageTexture::CylindricalImageTexture

/*===========================================================================*/

void CylindricalImageTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	0, 0,	// 1
	0, 0,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	0, 0,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",	// 0
	"", "",	// 1
	"", "",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",	// 9
	"", "",	// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_CYLINDRICALIMAGE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // CylindricalImageTexture::SetDefaults

/*===========================================================================*/

char CylindricalImageTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // CylindricalImageTexture::GetTextureParamType

/*===========================================================================*/

double CylindricalImageTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double PtXLow, PtXHigh, PtYLow, PtYHigh, X1, X2, Z1, Z2, Value, TempValue, TempYLow, TempYHigh, TempOutput[3],
	wt, SumWt, PtYRange, TempOpacity;
long IntervalNumberLowY, IntervalNumberHighY, DontSample;
int Abort = 0;
Raster *MyRast;

Value = Output[0] = Output[1] = Output[2] = 0.0;
if (! Img || ! (MyRast = Img->GetRaster()))
	{
	return (0.0);
	} // if
if (Data->IsSampleThumbnail && MyRast->ImageManagerEnabled 
	&& MyRast->GetAverageColor(0) == .5
	&& MyRast->GetAverageColor(1) == .5
	&& MyRast->GetAverageColor(2) == .5
	&& MyRast->GetAverageCoverage() == 1.0)
	{
	// shares standin
	MyRast = GlobalApp->AppImages->FetchStandInRast();
	} // if

LowX -= TCenter[0];
HighX -= TCenter[0];
LowY -= TCenter[1];
HighY -= TCenter[1];
LowZ -= TCenter[2];
HighZ -= TCenter[2];

if (TextureMoving())
	{
	AdjustSampleForVelocity(LowX, HighX, LowY, HighY, LowZ, HighZ);
	} // if

if (Rotated)
	{
	RotatePoint(LowX, LowY, LowZ, RotMatx);
	RotatePoint(HighX, HighY, HighZ, RotMatx);
	} // if

LowX /= TSize[0];
HighX /= TSize[0];
LowY /= TSize[1];
HighY /= TSize[1];
LowZ /= TSize[2];
HighZ /= TSize[2];

if (Data->SampleType != WCS_SAMPLETEXTURE_FLAT)
	{
	if (TexAxis == 2)
		{
		X1 = LowX;
		X2 = HighX;
		PtYLow = LowZ;
		PtYHigh = HighZ;
		Z1 = LowY;
		Z2 = HighY;
		} // else
	else if (TexAxis == 0)
		{
		X1 = LowZ;
		X2 = HighZ;
		PtYLow = LowX;
		PtYHigh = HighX;
		Z1 = LowY;
		Z2 = HighY;
		} // if
	else
		{
		X1 = LowX;
		X2 = HighX;
		PtYLow = LowY;
		PtYHigh = HighY;
		Z1 = LowZ;
		Z2 = HighZ;
		} // else if
	if ((TempOpacity = ObtainCylindricalX(X1, X2, Z1, Z2, PtXLow, PtXHigh)) <= 0)
		{
		return (0.0);
		} // if
	PtYLow += .5;
	PtYHigh += .5;
	} // if from renderer
else
	{
	if (TexAxis == 2)
		{
		PtXLow = LowX;
		PtXHigh = HighX;
		PtYLow = LowY;
		PtYHigh = HighY;
		} // else
	else if (TexAxis == 0)
		{
		PtXLow = LowZ;
		PtXHigh = HighZ;
		PtYLow = LowY;
		PtYHigh = HighY;
		} // if
	else
		{
		PtXLow = LowX;
		PtXHigh = HighX;
		PtYLow = LowZ;
		PtYHigh = HighZ;
		} // else if
	if (PtXLow > PtXHigh)
		Swap64(PtXLow, PtXHigh);
	if (PtYLow > PtYHigh)
		Swap64(PtYLow, PtYHigh);
	TempOpacity = 1.0;
	PtXLow += .5;
	PtXHigh += .5;
	PtYLow += .5;
	PtYHigh += .5;
	if (PtXLow < 0.0 || PtXHigh > 1.0)
		return (0.0);
	} // else for sample preview

if (! Antialias || WCS_SinglePoint2D(PtXLow, PtXHigh, PtYLow, PtYHigh))
	{
	PtXLow = (PtXLow + PtXHigh) * .5;
	PtYLow = (PtYLow + PtYHigh) * .5;

	if (PtYLow < 0.0 || PtYLow > 1.0)
		{
		if (IntervalNumberLowY = quicklongfloor(PtYLow))
			{
			if (TileHeight && CoordSpace != WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
				{
				PtYLow -= IntervalNumberLowY;
				if (FlipHeight && IntervalNumberLowY % 2)
					{
					PtYLow = 1.0 - PtYLow;
					} // if
				} // if
			else
				{
				return (0.0);
				} // else if
			} // if
		} // if
	if (ReverseWidth)
		{
		PtXLow = 1.0 - PtXLow;
		} // if
	if (ReverseHeight)
		{
		PtYLow = 1.0 - PtYLow;
		} // if
	if (PixelBlend)
		Value = MyRast->SampleBlendDouble3(Output, PtXLow, 1.0 - PtYLow, Abort);	// 1 - PtY flips image right-side up
	else
		Value = MyRast->SampleByteDouble3(Output, PtXLow, 1.0 - PtYLow, Abort);	// 1 - PtY flips image right-side up
	// if image was not found and user chose not to select a new one, Abort will be set.
	if (Abort)
		{
		// disable so it doesn't happen again
		Enabled = 0;
		} // if
	} // if
else
	{
	IntervalNumberLowY = quicklongfloor(PtYLow);
	IntervalNumberHighY = quicklongfloor(PtYHigh);

	if (ReverseWidth)
		{
		PtXLow = 1.0 - PtXLow;
		PtXHigh = 1.0 - PtXHigh;
		Swap64(PtXLow, PtXHigh);
		} // if
	if (IntervalNumberLowY == IntervalNumberHighY)
		{
		if (IntervalNumberLowY)
			{
			if (TileHeight)
				{
				PtYLow -= IntervalNumberLowY;
				PtYHigh -= IntervalNumberHighY;
				if (FlipHeight && IntervalNumberLowY % 2)
					{
					PtYLow = 1.0 - PtYLow;
					PtYHigh = 1.0 - PtYHigh;
					Swap64(PtYLow, PtYHigh);
					} // if
				} // if
			else
				{
				return (0.0);
				} // else if
			} // if
		if (ReverseHeight)
			{
			PtYLow = 1.0 - PtYLow;
			PtYHigh = 1.0 - PtYHigh;
			Swap64(PtYLow, PtYHigh);
			} // if
		Value = MyRast->SampleRangeByteDouble3(Output, PtXLow, PtXHigh, 1.0 - PtYHigh, 1.0 - PtYLow, Abort);	// 1 - PtY flips image right-side up
		// if image was not found and user chose not to select a new one, Abort will be set.
		if (Abort)
			{
			// disable so it doesn't happen again
			Enabled = 0;
			} // if
		} // if same x and y
	else
		{
		// two passes of y
		// low y
		TempOutput[0] = TempOutput[1] = TempOutput[2] = 0.0;
		SumWt = Value = TempValue = 0.0;
		DontSample = 0;
		PtYRange = PtYHigh - PtYLow;
		TempYLow = PtYLow - IntervalNumberLowY;
		TempYHigh = 1.0;
		wt = (1.0 - TempYLow) / PtYRange;
		SumWt += wt;
		if (IntervalNumberLowY)
			{
			if (TileHeight)
				{
				if (FlipHeight && IntervalNumberLowY % 2)
					{
					TempYLow = 1.0 - TempYLow;
					TempYHigh = 0.0;
					Swap64(TempYLow, TempYHigh);
					} // if
				} // if
			else
				{
				DontSample = 1;
				} // else
			} // if
		if (! DontSample)
			{
			if (ReverseHeight)
				{
				TempYLow = 1.0 - TempYLow;
				TempYHigh = 1.0 - TempYHigh;
				Swap64(TempYLow, TempYHigh);
				} // if
			TempValue = MyRast->SampleRangeByteDouble3(TempOutput, PtXLow, PtXHigh, 1.0 - TempYHigh, 1.0 - TempYLow, Abort);	// 1 - PtY flips image right-side up
			Output[0] = TempOutput[0] * wt;
			Output[1] = TempOutput[1] * wt;
			Output[2] = TempOutput[2] * wt;
			Value = TempValue * wt;
			// if image was not found and user chose not to select a new one, Abort will be set.
			if (Abort)
				{
				// disable so it doesn't happen again
				Enabled = 0;
				} // if
			} // if
		// high y
		TempOutput[0] = TempOutput[1] = TempOutput[2] = 0.0;
		TempValue = 0.0;
		DontSample = 0;
		TempYHigh = PtYHigh - IntervalNumberHighY;
		TempYLow = 0.0;
		wt = TempYHigh / PtYRange;
		SumWt += wt;
		if (IntervalNumberHighY)
			{
			if (TileHeight)
				{
				if (FlipHeight && IntervalNumberHighY % 2)
					{
					TempYHigh = 1.0 - TempYHigh;
					TempYLow = 1.0;
					Swap64(TempYHigh, TempYLow);
					} // if
				} // if
			else
				{
				DontSample = 1;
				} // else
			} // if
		if (! DontSample && ! Abort)
			{
			if (ReverseHeight)
				{
				TempYLow = 1.0 - TempYLow;
				TempYHigh = 1.0 - TempYHigh;
				Swap64(TempYLow, TempYHigh);
				} // if
			TempValue = MyRast->SampleRangeByteDouble3(TempOutput, PtXLow, PtXHigh, 1.0 - TempYHigh, 1.0 - TempYLow, Abort);	// 1 - PtY flips image right-side up
			Output[0] += TempOutput[0] * wt;
			Output[1] += TempOutput[1] * wt;
			Output[2] += TempOutput[2] * wt;
			Value += TempValue * wt;
			// if image was not found and user chose not to select a new one, Abort will be set.
			if (Abort)
				{
				// disable so it doesn't happen again
				Enabled = 0;
				} // if
			} // if
		if (SumWt < 1.0)
			{
			SumWt = 1.0 - SumWt;
			Output[0] += (MyRast->GetAverageColor(0) * MyRast->GetAverageCoverage() * SumWt);
			Output[1] += (MyRast->GetAverageColor(1) * MyRast->GetAverageCoverage() * SumWt);
			Output[2] += (MyRast->GetAverageColor(2) * MyRast->GetAverageCoverage() * SumWt);
			Value += MyRast->GetAverageCoverage() * SumWt;
			} // if
		} // else same x, different y
	} // else need to sample range of image

if (ImageNeg)
	{
	Output[0] = 1.0 - Output[0];
	Output[1] = 1.0 - Output[1];
	Output[2] = 1.0 - Output[2];
	} // if
if (! ApplyToColor)
	{
	Output[0] = (Output[0] + Output[1] + Output[2]) * (1.0 / 3.0);
	Output[1] = Output[2] = Output[0];
	} // if

return (AnalyzeImageWrapup(Output, Input, Value * TempOpacity, Data, EvalChildren));
//return (Value * TempOpacity);

} // CylindricalImageTexture::Analyze

/*===========================================================================*/

SphericalImageTexture::SphericalImageTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_SPHERICALIMAGE)
{

SetDefaults(ParamNumber);

} // SphericalImageTexture::SphericalImageTexture

/*===========================================================================*/

SphericalImageTexture::SphericalImageTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_SPHERICALIMAGE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // SphericalImageTexture::SphericalImageTexture

/*===========================================================================*/

void SphericalImageTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	0, 0,	// 1
	0, 0,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	0, 0,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",	// 0
	"", "",	// 1
	"", "",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",	// 9
	"", "",	// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_SPHERICALIMAGE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // SphericalImageTexture::SetDefaults

/*===========================================================================*/

char SphericalImageTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // SphericalImageTexture::GetTextureParamType

/*===========================================================================*/

double SphericalImageTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double PtXLow, PtXHigh, PtYLow, PtYHigh, X1, X2, Y1, Y2, Z1, Z2, Value, TempOpacity, TempOpacity2;
int Abort = 0;
Raster *MyRast;

Value = Output[0] = Output[1] = Output[2] = 0.0;
if (! Img || ! (MyRast = Img->GetRaster()))
	{
	return (0.0);
	} // if
if (Data->IsSampleThumbnail && MyRast->ImageManagerEnabled 
	&& MyRast->GetAverageColor(0) == .5
	&& MyRast->GetAverageColor(1) == .5
	&& MyRast->GetAverageColor(2) == .5
	&& MyRast->GetAverageCoverage() == 1.0)
	{
	// shares standin
	MyRast = GlobalApp->AppImages->FetchStandInRast();
	} // if

LowX -= TCenter[0];
HighX -= TCenter[0];
LowY -= TCenter[1];
HighY -= TCenter[1];
LowZ -= TCenter[2];
HighZ -= TCenter[2];

if (TextureMoving())
	{
	AdjustSampleForVelocity(LowX, HighX, LowY, HighY, LowZ, HighZ);
	} // if

if (Rotated)
	{
	RotatePoint(LowX, LowY, LowZ, RotMatx);
	RotatePoint(HighX, HighY, HighZ, RotMatx);
	} // if

LowX /= TSize[0];
HighX /= TSize[0];
LowY /= TSize[1];
HighY /= TSize[1];
LowZ /= TSize[2];
HighZ /= TSize[2];

if (Data->SampleType != WCS_SAMPLETEXTURE_FLAT)
	{
	if (TexAxis == 2)
		{
		X1 = LowX;
		X2 = HighX;
		Y1 = LowZ;
		Y2 = HighZ;
		Z1 = LowY;
		Z2 = HighY;
		} // else
	else if (TexAxis == 0)
		{
		X1 = LowZ;
		X2 = HighZ;
		Y1 = LowX;
		Y2 = HighX;
		Z1 = LowY;
		Z2 = HighY;
		} // if
	else
		{
		X1 = LowX;
		X2 = HighX;
		Y1 = LowY;
		Y2 = HighY;
		Z1 = LowZ;
		Z2 = HighZ;
		} // else if
	if ((TempOpacity = ObtainCylindricalX(X1, X2, Z1, Z2, PtXLow, PtXHigh)) <= 0)
		{
		return (0.0);
		} // if
	if ((TempOpacity2 = ObtainSphericalY(X1, X2, Y1, Y2, Z1, Z2, PtYLow, PtYHigh)) <= 0)
		{
		return (0.0);
		} // if
	TempOpacity *= TempOpacity2;
	} // if from renderer
else
	{
	if (TexAxis == 2)
		{
		PtXLow = LowX;
		PtXHigh = HighX;
		PtYLow = LowY;
		PtYHigh = HighY;
		} // else
	else if (TexAxis == 0)
		{
		PtXLow = LowZ;
		PtXHigh = HighZ;
		PtYLow = LowY;
		PtYHigh = HighY;
		} // if
	else
		{
		PtXLow = LowX;
		PtXHigh = HighX;
		PtYLow = LowZ;
		PtYHigh = HighZ;
		} // else if
	if (PtXLow > PtXHigh)
		Swap64(PtXLow, PtXHigh);
	if (PtYLow > PtYHigh)
		Swap64(PtYLow, PtYHigh);
	TempOpacity = 1.0;
	PtXLow += .5;
	PtXHigh += .5;
	PtYLow += .5;
	PtYHigh += .5;
	if (PtXLow < 0.0 || PtXHigh > 1.0 || PtYLow < 0.0 || PtYHigh > 1.0)
		return (0.0);
	} // else for sample preview - treat as planar image


if (! Antialias || WCS_SinglePoint2D(PtXLow, PtXHigh, PtYLow, PtYHigh))
	{
	PtXLow = (PtXLow + PtXHigh) * .5;
	PtYLow = (PtYLow + PtYHigh) * .5;

	if (ReverseWidth)
		{
		PtXLow = 1.0 - PtXLow;
		} // if
	if (ReverseHeight)
		{
		PtYLow = 1.0 - PtYLow;
		} // if
	if (PixelBlend)
		Value = MyRast->SampleBlendDouble3(Output, PtXLow, 1.0 - PtYLow, Abort);	// 1 - PtY flips image right-side up
	else
		Value = MyRast->SampleByteDouble3(Output, PtXLow, 1.0 - PtYLow, Abort);	// 1 - PtY flips image right-side up
	// if image was not found and user chose not to select a new one, Abort will be set.
	if (Abort)
		{
		// disable so it doesn't happen again
		Enabled = 0;
		} // if
	} // if
else
	{
	if (ReverseWidth)
		{
		PtXLow = 1.0 - PtXLow;
		PtXHigh = 1.0 - PtXHigh;
		Swap64(PtXLow, PtXHigh);
		} // if
	if (ReverseHeight)
		{
		PtYLow = 1.0 - PtYLow;
		PtYHigh = 1.0 - PtYHigh;
		Swap64(PtYLow, PtYHigh);
		} // if
	Value = MyRast->SampleRangeByteDouble3(Output, PtXLow, PtXHigh, 1.0 - PtYHigh, 1.0 - PtYLow, Abort);	// 1 - PtY flips image right-side up
	// if image was not found and user chose not to select a new one, Abort will be set.
	if (Abort)
		{
		// disable so it doesn't happen again
		Enabled = 0;
		} // if
	} // else need to sample range of image

if (ImageNeg)
	{
	Output[0] = 1.0 - Output[0];
	Output[1] = 1.0 - Output[1];
	Output[2] = 1.0 - Output[2];
	} // if
if (! ApplyToColor)
	{
	Output[0] = (Output[0] + Output[1] + Output[2]) * (1.0 / 3.0);
	Output[1] = Output[2] = Output[0];
	} // if

return (AnalyzeImageWrapup(Output, Input, Value * TempOpacity, Data, EvalChildren));
//return (Value * TempOpacity);

} // SphericalImageTexture::Analyze

/*===========================================================================*/

CubicImageTexture::CubicImageTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_CUBICIMAGE)
{

SetDefaults(ParamNumber);

} // CubicImageTexture::CubicImageTexture

/*===========================================================================*/

CubicImageTexture::CubicImageTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_CUBICIMAGE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // CubicImageTexture::CubicImageTexture

/*===========================================================================*/

void CubicImageTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	0, 0,	// 1
	0, 0,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	0, 0,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",	// 0
	"", "",	// 1
	"", "",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",	// 9
	"", "",	// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_CUBICIMAGE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // CubicImageTexture::SetDefaults

/*===========================================================================*/

char CubicImageTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // CubicImageTexture::GetTextureParamType

/*===========================================================================*/

FrontProjectionTexture::FrontProjectionTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE)
{

SetDefaults(ParamNumber);

} // FrontProjectionTexture::FrontProjectionTexture

/*===========================================================================*/

FrontProjectionTexture::FrontProjectionTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // FrontProjectionTexture::FrontProjectionTexture

/*===========================================================================*/

void FrontProjectionTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	0, 0,	// 1
	0, 0,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	0, 0,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",	// 0
	"", "",	// 1
	"", "",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",	// 9
	"", "",	// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_FRONTPROJECTIONIMAGE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // FrontProjectionTexture::SetDefaults

/*===========================================================================*/

char FrontProjectionTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // FrontProjectionTexture::GetTextureParamType

/*===========================================================================*/

double FrontProjectionTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double PtXLow, PtXHigh, PtYLow, PtYHigh, Value;
int Abort = 0;
Raster *MyRast;

Value = Output[0] = Output[1] = Output[2] = 0.0;
if (! Img || ! (MyRast = Img->GetRaster()))
	{
	return (0.0);
	} // if
if (Data->IsSampleThumbnail && MyRast->ImageManagerEnabled 
	&& MyRast->GetAverageColor(0) == .5
	&& MyRast->GetAverageColor(1) == .5
	&& MyRast->GetAverageColor(2) == .5
	&& MyRast->GetAverageCoverage() == 1.0)
	{
	// shares standin
	MyRast = GlobalApp->AppImages->FetchStandInRast();
	} // if

if (! Data->SampleType)
	{
	//PtXLow = Data->PixelX[0];
	//PtXHigh = Data->PixelX[1];
	//PtYLow = Data->PixelY[0];
	//PtYHigh = Data->PixelY[1];
	PtXLow = LowX + .5;
	PtXHigh = HighX + .5;
	PtYLow = -HighY + .5;
	PtYHigh = -LowY + .5;
	} // if
else
	{
	PtXLow = LowX;
	PtXHigh = HighX;
	PtYLow = LowY;
	PtYHigh = HighY;
	if (PtXLow > PtXHigh)
		Swap64(PtXLow, PtXHigh);
	if (PtYLow > PtYHigh)
		Swap64(PtYLow, PtYHigh);
	PtYLow = 1.0 - PtYLow;
	PtYHigh = 1.0 - PtYHigh;
	} // else
if (PtXLow < 0.0 || PtXLow > 1.0 || PtXHigh < 0.0 || PtXHigh > 1.0 || PtYLow < 0.0 || PtYLow > 1.0 || PtYHigh < 0.0 || PtYHigh > 1.0)
	return (0.0); 

if (ReverseWidth)
	{
	PtXLow = 1.0 - PtXLow;
	PtXHigh = 1.0 - PtXHigh;
	Swap64(PtXLow, PtXHigh);
	} // if
if (ReverseHeight)
	{
	PtYLow = 1.0 - PtYLow;
	PtYHigh = 1.0 - PtYHigh;
	Swap64(PtYLow, PtYHigh);
	} // if
Value = MyRast->SampleRangeByteDouble3(Output, PtXLow, PtXHigh, PtYLow, PtYHigh, Abort);
// if image was not found and user chose not to select a new one, Abort will be set.
if (Abort)
	{
	// disable so it doesn't happen again
	Enabled = 0;
	} // if

if (ImageNeg)
	{
	Output[0] = 1.0 - Output[0];
	Output[1] = 1.0 - Output[1];
	Output[2] = 1.0 - Output[2];
	} // if
if (! ApplyToColor)
	{
	Output[0] = (Output[0] + Output[1] + Output[2]) * (1.0 / 3.0);
	Output[1] = Output[2] = Output[0];
	} // if

return (AnalyzeImageWrapup(Output, Input, Value, Data, EvalChildren));
//return (Value);

} // FrontProjectionTexture::Analyze

/*===========================================================================*/

EnvironmentMapTexture::EnvironmentMapTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE)
{

SetDefaults(ParamNumber);

} // EnvironmentMapTexture::EnvironmentMapTexture

/*===========================================================================*/

EnvironmentMapTexture::EnvironmentMapTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // EnvironmentMapTexture::EnvironmentMapTexture

/*===========================================================================*/

void EnvironmentMapTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	0, 0,	// 1
	0, 0,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	0, 0,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",	// 0
	"", "",	// 1
	"", "",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",	// 9
	"", "",	// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_ENVIRONMENTMAPIMAGE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // EnvironmentMapTexture::SetDefaults

/*===========================================================================*/

char EnvironmentMapTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // EnvironmentMapTexture::GetTextureParamType

/*===========================================================================*/

double EnvironmentMapTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double PtXLow, PtXHigh, PtYLow, PtYHigh, X1, X2, Y1, Y2, Z1, Z2, Value, TempOpacity, Latitude, Longitude, IncidentAngle;
int Abort = 0;
Raster *MyRast;
VertexDEM ViewVec, ReflVec, TempVec;

Value = Output[0] = Output[1] = Output[2] = 0.0;
if (! Img || ! (MyRast = Img->GetRaster()))
	{
	return (0.0);
	} // if
if (Data->IsSampleThumbnail && MyRast->ImageManagerEnabled 
	&& MyRast->GetAverageColor(0) == .5
	&& MyRast->GetAverageColor(1) == .5
	&& MyRast->GetAverageColor(2) == .5
	&& MyRast->GetAverageCoverage() == 1.0)
	{
	// shares standin
	MyRast = GlobalApp->AppImages->FetchStandInRast();
	} // if

if (Rotated)
	{
	RotatePoint(LowX, LowY, LowZ, RotMatx);
	RotatePoint(HighX, HighY, HighZ, RotMatx);
	} // if

if (Data->SampleType != WCS_SAMPLETEXTURE_FLAT)
	{
	// take pixel vertices and compute a reflection vector for each
	// vector - should be in world coordinates with geographic orientation
	// find reflection vector in world coordinates first
	
	ViewVec.XYZ[0] = Data->CamAimVec[0];
	ViewVec.XYZ[1] = Data->CamAimVec[1];
	ViewVec.XYZ[2] = Data->CamAimVec[2];
	//NegateVector(ViewVec.XYZ);
	ViewVec.UnitVector();

	IncidentAngle = VectorAngle(Data->Normal, ViewVec.XYZ);

	// find the reflection ray
	ReflVec.XYZ[0] = Data->Normal[0] * 2.0 * IncidentAngle;
	ReflVec.XYZ[1] = Data->Normal[1] * 2.0 * IncidentAngle;
	ReflVec.XYZ[2] = Data->Normal[2] * 2.0 * IncidentAngle;
	ReflVec.GetPosVector(&ViewVec);	// subtracts Pixel to camera vector
	ReflVec.UnitVector();

	// create a rotation matrix that rotates the reflection vector to the geographic north pole
	ReflVec.RotateY(-Data->Longitude);
	ReflVec.RotateX(90.0 - Data->Latitude);
	Longitude = ReflVec.FindRoughAngleYfromZ();
	ReflVec.RotateY(-Longitude);
	Latitude = -ReflVec.FindRoughAngleYfromZ();
	if (Longitude < 0.0)
		Longitude += 360.0;
	else if (Longitude > 360.0)
		Longitude -= 360.0;

	LowX = HighX = Longitude * (1.0 / 360.0);
	//LowY = HighY = (Latitude + 90.0) * (1.0 / 180.0);
	// alternative sinusoidal mapping
	LowY = 1.0 - cos(Latitude * PiOver180);
	if (Latitude < 0.0)
		LowY = -LowY;
	LowY = HighY = LowY * .5 + .5;
	X1 = LowX;
	X2 = HighX;
	Y1 = LowY;
	Y2 = HighY;
	Z1 = LowZ;
	Z2 = HighZ;
	TempOpacity = 1.0;
	PtXLow = PtXHigh = X1;
	PtYLow = PtYHigh = Y1;
	} // if from renderer
else
	{
	if (TexAxis == 2)
		{
		PtXLow = LowX;
		PtXHigh = HighX;
		PtYLow = LowY;
		PtYHigh = HighY;
		} // else
	else if (TexAxis == 0)
		{
		PtXLow = LowZ;
		PtXHigh = HighZ;
		PtYLow = LowY;
		PtYHigh = HighY;
		} // if
	else
		{
		PtXLow = LowX;
		PtXHigh = HighX;
		PtYLow = LowZ;
		PtYHigh = HighZ;
		} // else if
	if (PtXLow > PtXHigh)
		Swap64(PtXLow, PtXHigh);
	if (PtYLow > PtYHigh)
		Swap64(PtYLow, PtYHigh);
	TempOpacity = 1.0;
	PtXLow += .5;
	PtXHigh += .5;
	PtYLow += .5;
	PtYHigh += .5;
	if (PtXLow < 0.0 || PtXHigh > 1.0 || PtYLow < 0.0 || PtYHigh > 1.0)
		return (0.0);
	} // else for sample preview - treat as planar image


if (! Antialias || WCS_SinglePoint2D(PtXLow, PtXHigh, PtYLow, PtYHigh))
	{
	PtXLow = (PtXLow + PtXHigh) * .5;
	PtYLow = (PtYLow + PtYHigh) * .5;

	if (ReverseWidth)
		{
		PtXLow = 1.0 - PtXLow;
		} // if
	if (ReverseHeight)
		{
		PtYLow = 1.0 - PtYLow;
		} // if
	if (PixelBlend)
		Value = MyRast->SampleBlendDouble3(Output, PtXLow, 1.0 - PtYLow, Abort);	// 1 - PtY flips image right-side up
	else
		Value = MyRast->SampleByteDouble3(Output, PtXLow, 1.0 - PtYLow, Abort);	// 1 - PtY flips image right-side up
	// if image was not found and user chose not to select a new one, Abort will be set.
	if (Abort)
		{
		// disable so it doesn't happen again
		Enabled = 0;
		} // if
	} // if
else
	{
	if (ReverseWidth)
		{
		PtXLow = 1.0 - PtXLow;
		PtXHigh = 1.0 - PtXHigh;
		Swap64(PtXLow, PtXHigh);
		} // if
	if (ReverseHeight)
		{
		PtYLow = 1.0 - PtYLow;
		PtYHigh = 1.0 - PtYHigh;
		Swap64(PtYLow, PtYHigh);
		} // if
	Value = MyRast->SampleRangeByteDouble3(Output, PtXLow, PtXHigh, 1.0 - PtYHigh, 1.0 - PtYLow, Abort);	// 1 - PtY flips image right-side up
	// if image was not found and user chose not to select a new one, Abort will be set.
	if (Abort)
		{
		// disable so it doesn't happen again
		Enabled = 0;
		} // if
	} // else need to sample range of image

if (ImageNeg)
	{
	Output[0] = 1.0 - Output[0];
	Output[1] = 1.0 - Output[1];
	Output[2] = 1.0 - Output[2];
	} // if
if (! ApplyToColor)
	{
	Output[0] = (Output[0] + Output[1] + Output[2]) * (1.0 / 3.0);
	Output[1] = Output[2] = Output[0];
	} // if

return (AnalyzeImageWrapup(Output, Input, Value * TempOpacity, Data, EvalChildren));
//return (Value * TempOpacity);

} // EnvironmentMapTexture::Analyze

/*===========================================================================*/

StripeTexture::StripeTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_STRIPES)
{

SetDefaults(ParamNumber);

} // StripeTexture::StripeTexture

/*===========================================================================*/

StripeTexture::StripeTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_STRIPES, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // StripeTexture::StripeTexture

/*===========================================================================*/

void StripeTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	0, 0,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	0, 0	// 12
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",	// 0
	"Low (%) ", "High (%) ",	// 1
	"Center (%) ", "Width (%) ",	// 3
	"", "",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"", "",		// 9
	"Color ", "Color ",	// 11
	"", ""	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 0.0, 50.0, 0.0, 0.0, 0.0, 100.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_STRIPES;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // StripeTexture::SetDefaults

/*===========================================================================*/

char StripeTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_WIDTH,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	0, 0
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // StripeTexture::GetTextureParamType

/*===========================================================================*/

double StripeTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Value, Offset, Temp, SampleWidth, TempValue;

ReCenterMoveRotate(LowX, HighX, LowY, HighY, LowZ, HighZ);
//SampleWidth = MaxSampleSize(LowX, HighX, LowY, HighY, LowZ, HighZ);

Value = Input[WCS_TEXTURE_STRIPE_CONTRAST][0];
if (TexAxis == 0)
	{
	SampleWidth = fabs(HighX - LowX);
	Offset = TSize[0] * Input[WCS_TEXTURE_STRIPE_CENTER][0];
	LowX += Offset;
	HighX += Offset;
	Temp = LowX / TSize[0];
	SampleWidth /= TSize[0];
	} // if
else if (TexAxis == 1)
	{
	SampleWidth = fabs(HighY - LowY);
	Offset = TSize[1] * Input[WCS_TEXTURE_STRIPE_CENTER][0];
	LowY += Offset;
	HighY += Offset;
	Temp = LowY / TSize[1];
	SampleWidth /= TSize[1];
	} // else if
else
	{
	SampleWidth = fabs(HighZ - LowZ);
	Offset = TSize[2] * Input[WCS_TEXTURE_STRIPE_CENTER][0];
	LowZ += Offset;
	HighZ += Offset;
	Temp = LowZ / TSize[2];
	SampleWidth /= TSize[2];
	} // else
Temp += (.5 * Input[WCS_TEXTURE_STRIPE_WIDTH][0]);
if (SampleWidth >= .5)
	{
	Value *= Input[WCS_TEXTURE_STRIPE_WIDTH][0];
	} // if
else if (SampleWidth > .25)
	{
	TempValue = InvertSmoothPulse(Temp, Input[WCS_TEXTURE_STRIPE_WIDTH][0], SampleWidth);
	Value *= (((.5 - SampleWidth) * 4.0) * TempValue + ((SampleWidth - .25) * 4.0) * Input[WCS_TEXTURE_STRIPE_WIDTH][0]);
	} // if
else if (SampleWidth > 0.0)
	{
	Value *= InvertSmoothPulse(Temp, Input[WCS_TEXTURE_STRIPE_WIDTH][0], SampleWidth);
	} // if
else if (Temp - floor(Temp) > Input[WCS_TEXTURE_STRIPE_WIDTH][0])
	Value = 0.0;
Value += Input[WCS_TEXTURE_STRIPE_BRIGHTNESS][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // StripeTexture::Analyze

/*===========================================================================*/

SoftStripeTexture::SoftStripeTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_SOFTSTRIPES)
{

SetDefaults(ParamNumber);

} // SoftStripeTexture::SoftStripeTexture

/*===========================================================================*/

SoftStripeTexture::SoftStripeTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_SOFTSTRIPES, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // SoftStripeTexture::SoftStripeTexture

/*===========================================================================*/

void SoftStripeTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	0, 0,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",	// 0
	"Low (%) ", "High (%) ",	// 1
	"Center (%) ", "Width (%) ",	// 3
	"", "",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"", "",		// 9
	"Color ", "Color ",	// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 0.0, 50.0, 0.0, 0.0, 0.0, 100.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_SOFTSTRIPES;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // SoftStripeTexture::SetDefaults

/*===========================================================================*/

char SoftStripeTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_WIDTH,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // SoftStripeTexture::GetTextureParamType

/*===========================================================================*/

double SoftStripeTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Value, Offset, Temp;

ReCenterMoveRotate(LowX, HighX, LowY, HighY, LowZ, HighZ);

Value = Input[WCS_TEXTURE_STRIPE_CONTRAST][0];
if (TexAxis == 0)
	{
	Offset = TSize[0] * Input[WCS_TEXTURE_STRIPE_CENTER][0];
	LowX += Offset;
	HighX += Offset;
	Temp = LowX / TSize[0];
	} // if
else if (TexAxis == 1)
	{
	Offset = TSize[1] * Input[WCS_TEXTURE_STRIPE_CENTER][0];
	LowY += Offset;
	HighY += Offset;
	Temp = LowY / TSize[1];
	} // else if
else
	{
	Offset = TSize[2] * Input[WCS_TEXTURE_STRIPE_CENTER][0];
	LowZ += Offset;
	HighZ += Offset;
	Temp = LowZ / TSize[2];
	} // else
Temp += (.5 * Input[WCS_TEXTURE_STRIPE_WIDTH][0]);
if ((Temp = Temp - WCS_floor(Temp)) > Input[WCS_TEXTURE_STRIPE_WIDTH][0])
	{
	Value = 0.0;
	} // if
else
	{
	Value *= (((-cos((Temp / Input[WCS_TEXTURE_STRIPE_WIDTH][0]) * TwoPi)) * .5) + .5);
	} // else
Value += Input[WCS_TEXTURE_STRIPE_BRIGHTNESS][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // SoftStripeTexture::Analyze

/*===========================================================================*/

SingleStripeTexture::SingleStripeTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_SINGLESTRIPE)
{

SetDefaults(ParamNumber);

} // SingleStripeTexture::SingleStripeTexture

/*===========================================================================*/

SingleStripeTexture::SingleStripeTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_SINGLESTRIPE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // SingleStripeTexture::SingleStripeTexture

/*===========================================================================*/

void SingleStripeTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	0, 0	// 12
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",	// 0
	"Low (%) ", "High (%) ",	// 1
	"Center (%) ", "Width (%) ",	// 3
	"Length (%) ", "Offset (m) ",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"", "",		// 9
	"Color ", "Color ",	// 11
	"", ""	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 50.0, 100.0, 100.0, 0.0, 0.0, 100.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
double OffsetRangeDefaults[3] = {1000000.0, -1000000.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_SINGLESTRIPE_OFFSET].SetRangeDefaults(OffsetRangeDefaults);

TexType = WCS_TEXTURE_TYPE_SINGLESTRIPE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // SingleStripeTexture::SetDefaults

/*===========================================================================*/

char SingleStripeTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_WIDTH,
	WCS_TEXTURE_PARAMTYPE_HEIGHT, WCS_TEXTURE_PARAMTYPE_OFFSET,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	0, 0
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // SingleStripeTexture::GetTextureParamType

/*===========================================================================*/

double SingleStripeTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Value, ValueX, ValueY, TempX, TempY, SampleWidth, c, t;

ReCenterMoveRotate(LowX, HighX, LowY, HighY, LowZ, HighZ);
//SampleWidth = MaxSampleSize(LowX, HighX, LowY, HighY, LowZ, HighZ);

Value = Input[WCS_TEXTURE_STRIPE_CONTRAST][0];
if (TexAxis == 0)
	{
	SampleWidth = fabs(HighX - LowX);
	TempX = min(LowX, HighX) - Input[WCS_TEXTURE_SINGLESTRIPE_OFFSET][0];
	TempX /= TSize[0];
	TempX += Input[WCS_TEXTURE_STRIPE_CENTER][0];
	TempY = min(LowY, HighY);
	TempY /= TSize[1];
	} // if
else if (TexAxis == 1)
	{
	SampleWidth = fabs(HighY - LowY);
	TempX = min(LowY, HighY) - Input[WCS_TEXTURE_SINGLESTRIPE_OFFSET][0];
	TempX /= TSize[1];
	TempX += Input[WCS_TEXTURE_STRIPE_CENTER][0];
	TempY = min(LowX, HighX);
	TempY /= TSize[0];
	} // else if
else
	{
	SampleWidth = fabs(HighZ - LowZ);
	TempX = min(LowZ, HighZ) - Input[WCS_TEXTURE_SINGLESTRIPE_OFFSET][0];
	TempX /= TSize[2];
	TempX += Input[WCS_TEXTURE_STRIPE_CENTER][0];
	TempY = min(LowY, HighY);
	TempY /= TSize[1];
	} // else
if (TempX > -SampleWidth && TempX < 1.0 + SampleWidth)
	{
	if (TempX >= 0.0)
		{
		if (TempX <= Input[WCS_TEXTURE_STRIPE_WIDTH][0])
			{
			ValueX = 1.0;
			} // if
		else if (TempX <= (t = Input[WCS_TEXTURE_STRIPE_WIDTH][0] + SampleWidth))
			{
			c = (t - TempX) / SampleWidth;
			ValueX = PERLIN_s_curve(c);
			} // if
		else
			ValueX = ValueY = 0.0;
		} // if
	else
		{
		c = 1.0 - TempX / SampleWidth;
		ValueX = PERLIN_s_curve(c);
		} // else
	TempY -= WCS_floor(TempY);
	if (TempY <= Input[WCS_TEXTURE_SINGLESTRIPE_LENGTH][0])
		ValueY = 1.0;
	else
		ValueY = 0.0;
	} // if
else
	ValueX = ValueY = 0.0;

Value *= (ValueX * ValueY);
Value += Input[WCS_TEXTURE_STRIPE_BRIGHTNESS][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // SingleStripeTexture::Analyze

/*===========================================================================*/

SingleSoftStripeTexture::SingleSoftStripeTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_SINGLESOFTSTRIPE)
{

SetDefaults(ParamNumber);

} // SingleSoftStripeTexture::SingleSoftStripeTexture

/*===========================================================================*/

SingleSoftStripeTexture::SingleSoftStripeTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_SINGLESOFTSTRIPE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // SingleSoftStripeTexture::SingleSoftStripeTexture

/*===========================================================================*/

void SingleSoftStripeTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",	// 0
	"Low (%) ", "High (%) ",	// 1
	"Center (%) ", "Width (%) ",	// 3
	"Length (%) ", "Offset (m) ",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"", "",		// 9
	"Color ", "Color ",	// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 50.0, 100.0, 100.0, 0.0, 0.0, 100.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
double OffsetRangeDefaults[3] = {10000.0, -10000.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_SINGLESTRIPE_OFFSET].SetRangeDefaults(OffsetRangeDefaults);

TexType = WCS_TEXTURE_TYPE_SINGLESOFTSTRIPE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // SingleSoftStripeTexture::SetDefaults

/*===========================================================================*/

char SingleSoftStripeTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_WIDTH,
	WCS_TEXTURE_PARAMTYPE_HEIGHT, WCS_TEXTURE_PARAMTYPE_OFFSET,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // SingleSoftStripeTexture::GetTextureParamType

/*===========================================================================*/

double SingleSoftStripeTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Value, ValueX, ValueY, TempX, TempY;

ReCenterMoveRotate(LowX, HighX, LowY, HighY, LowZ, HighZ);

Value = Input[WCS_TEXTURE_STRIPE_CONTRAST][0];
if (TexAxis == 0)
	{
	TempX = min(LowX, HighX) - Input[WCS_TEXTURE_SINGLESTRIPE_OFFSET][0];
	TempX /= TSize[0];
	TempX += Input[WCS_TEXTURE_STRIPE_CENTER][0];
	TempY = min(LowY, HighY);
	TempY /= TSize[1];
	} // if
else if (TexAxis == 1)
	{
	TempX = min(LowY, HighY) - Input[WCS_TEXTURE_SINGLESTRIPE_OFFSET][0];
	TempX /= TSize[1];
	TempX += Input[WCS_TEXTURE_STRIPE_CENTER][0];
	TempY = min(LowX, HighX);
	TempY /= TSize[0];
	} // else if
else
	{
	TempX = min(LowZ, HighZ) - Input[WCS_TEXTURE_SINGLESTRIPE_OFFSET][0];
	TempX /= TSize[2];
	TempX += Input[WCS_TEXTURE_STRIPE_CENTER][0];
	TempY = min(LowY, HighY);
	TempY /= TSize[1];
	} // else
if (TempX > 0.0 && TempX < Input[WCS_TEXTURE_STRIPE_WIDTH][0])
	{
	ValueX = ((-cos((TempX / Input[WCS_TEXTURE_STRIPE_WIDTH][0]) * TwoPi)) * .5) + .5;
	TempY -= WCS_floor(TempY);
	if (TempY > 0.0 && TempY < Input[WCS_TEXTURE_SINGLESTRIPE_LENGTH][0])
		{
		ValueY = 1.0;
		} // if
	else
		ValueY = 0.0;
	} // if
else
	ValueX = ValueY = 0.0;

Value *= (ValueX * ValueY);
Value += Input[WCS_TEXTURE_STRIPE_BRIGHTNESS][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // SingleSoftStripeTexture::Analyze

/*===========================================================================*/

WoodGrainTexture::WoodGrainTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_WOODGRAIN)
{

SetDefaults(ParamNumber);
InitNoise();

} // WoodGrainTexture::WoodGrainTexture

/*===========================================================================*/

WoodGrainTexture::WoodGrainTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_WOODGRAIN, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);
InitNoise();

} // WoodGrainTexture::WoodGrainTexture

/*===========================================================================*/

void WoodGrainTexture::DeleteSpecificResources(void)
{

if (Noise)
	delete Noise;
Noise = NULL;

} // WoodGrainTexture::DeleteSpecificResources

/*===========================================================================*/

void WoodGrainTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",	// 0
	"Low (%) ", "High (%) ",	// 1
	"Frequencies ", "Turbulence (%) ",	// 3
	"Ring Width (%) ", "Sharpness ",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"", "",	// 9
	"Color ", "Color ",	// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 2.0, 10.0, 2.0, 3.0, 0.0, 100.0, 0.0, 0.0};
double OctaveRangeDefaults[3] = {10.0, 0.0, 1.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

Noise = NULL;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_WOODGRAIN_FREQUENCIES].SetRangeDefaults(OctaveRangeDefaults);

TexType = WCS_TEXTURE_TYPE_WOODGRAIN;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // WoodGrainTexture::SetDefaults

/*===========================================================================*/

char WoodGrainTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_FREQUENCIES, WCS_TEXTURE_PARAMTYPE_TURBULENCE,
	WCS_TEXTURE_PARAMTYPE_SPACING, WCS_TEXTURE_PARAMTYPE_SHARPNESS,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // WoodGrainTexture::GetTextureParamType

/*===========================================================================*/

double WoodGrainTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Value, PtXLow, PtXHigh, PtYLow, PtYHigh, PtZLow, PtZHigh, Offset, Radius, Phase, HalfLambda;
NoiseVector Point;

ReCenterMoveRotate(LowX, HighX, LowY, HighY, LowZ, HighZ);
ReSizeT(LowX, HighX, LowY, HighY, LowZ, HighZ);

if (TexAxis == 2)
	{
	PtXLow = LowX;
	PtXHigh = HighX;
	PtYLow = LowY;
	PtYHigh = HighY;
	PtZLow = LowZ;
	PtZHigh = HighZ;
	} // if
else if (TexAxis == 0)
	{
	PtXLow = LowZ;
	PtXHigh = HighZ;
	PtYLow = LowY;
	PtYHigh = HighY;
	PtZLow = LowX;
	PtZHigh = HighX;
	} // else if
else
	{
	PtXLow = LowX;
	PtXHigh = HighX;
	PtYLow = LowZ;
	PtYHigh = HighZ;
	PtZLow = LowY;
	PtZHigh = HighY;
	} // else

Value = Input[WCS_TEXTURE_WOODGRAIN_CONTRAST][0];
if (Input[WCS_TEXTURE_WOODGRAIN_RINGSPACING][0] > 0.0 && Input[WCS_TEXTURE_WOODGRAIN_RINGSHARPNESS][0] > 0.0)
	{
	if (Input[WCS_TEXTURE_WOODGRAIN_TURBULENCE][0] > 0.0 && Noise)
		{
		Point.x = (PtXLow + PtXHigh) * 2.0;
		Point.y = (PtYLow + PtYHigh) * 2.0;
		Point.z = (PtZLow + PtZHigh) * .5;
		Offset = (Noise->EvaluateNoise(Point, 0, 0) * .5 + .5) * Input[WCS_TEXTURE_WOODGRAIN_TURBULENCE][0];
		PtXLow += Offset;
		PtXHigh += Offset;
		PtYLow += Offset;
		PtYHigh += Offset;

		//Point.x = (PtXLow + PtXHigh) * 2.0;
		//Point.y = 0.0;
		//Point.z = (PtZLow + PtZHigh) * .5;
		//Offset = (Noise->EvaluateNoise(Point, 0, 0) * .5 + .5) * Input[WCS_TEXTURE_WOODGRAIN_TURBULENCE][0];
		//PtXLow += Offset;
		//PtXHigh += Offset;
		//Point.x = 0.0;
		//Point.y = (PtYLow + PtYHigh) * 2.0;
		//Offset = (Noise->EvaluateNoise(Point, 0, 0) * .5 + .5) * Input[WCS_TEXTURE_WOODGRAIN_TURBULENCE][0];
		//PtYLow += Offset;
		//PtYHigh += Offset;
		} // if
	Radius = sqrt(PtXLow * PtXLow + PtYLow * PtYLow);

	Radius /= Input[WCS_TEXTURE_WOODGRAIN_RINGSPACING][0];
	Radius -= WCS_floor(Radius);

	HalfLambda = 3.0 / Input[WCS_TEXTURE_WOODGRAIN_RINGSHARPNESS][0];
	if (Radius > 1.0 - HalfLambda)
		{
		Phase = (Radius - (1.0 - HalfLambda)) / HalfLambda;
		Value *= (-cos(Phase * Pi) * .5 + .5);
		} // if
	else
		Value = 0.0;
	} // if

Value += Input[WCS_TEXTURE_WOODGRAIN_BRIGHTNESS][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // WoodGrainTexture::Analyze

/*===========================================================================*/

void WoodGrainTexture::InitNoise(void)
{
double Args[3];

if (Noise)
	delete Noise;
Args[2] = .5;
Args[1] = 2.0;
Args[0] = TexParam[WCS_TEXTURE_WOODGRAIN_FREQUENCIES].CurValue;

Noise = new fBmNoise(Args, 3);

} // WoodGrainTexture::InitNoise

/*===========================================================================*/

MarbleTexture::MarbleTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_MARBLE)
{

SetDefaults(ParamNumber);
InitNoise();

} // MarbleTexture::MarbleTexture

/*===========================================================================*/

MarbleTexture::MarbleTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_MARBLE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);
InitNoise();

} // MarbleTexture::MarbleTexture

/*===========================================================================*/

void MarbleTexture::DeleteSpecificResources(void)
{

if (Noise)
	delete Noise;
Noise = NULL;

} // MarbleTexture::DeleteSpecificResources

/*===========================================================================*/

void MarbleTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",	// 0
	"Low (%) ", "High (%) ",	// 1
	"Frequencies ", "Turbulence (%) ",	// 3
	"Vein Width (%) ", "Sharpness (%) ",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"", "",	// 9
	"Color ", "Color ",	// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 5.0, 50.0, 30.0, 5.0, 0.0, 100.0, 0.0, 0.0};
double OctaveRangeDefaults[3] = {10.0, 0.0, 1.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

Noise = NULL;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_WOODGRAIN_FREQUENCIES].SetRangeDefaults(OctaveRangeDefaults);

TexType = WCS_TEXTURE_TYPE_MARBLE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // MarbleTexture::SetDefaults

/*===========================================================================*/

char MarbleTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_FREQUENCIES, WCS_TEXTURE_PARAMTYPE_TURBULENCE,
	WCS_TEXTURE_PARAMTYPE_SPACING, WCS_TEXTURE_PARAMTYPE_SHARPNESS,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // MarbleTexture::GetTextureParamType

/*===========================================================================*/

double MarbleTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Value, PtXLow, PtXHigh, PtYLow, PtYHigh, PtZLow, PtZHigh, Phase, Lambda;
NoiseVector Point;

ReCenterMoveRotate(LowX, HighX, LowY, HighY, LowZ, HighZ);
ReSizeT(LowX, HighX, LowY, HighY, LowZ, HighZ);

if (TexAxis == 2)
	{
	PtXLow = LowX;
	PtXHigh = HighX;
	PtYLow = LowY;
	PtYHigh = HighY;
	PtZLow = LowZ;
	PtZHigh = HighZ;
	} // if
else if (TexAxis == 0)
	{
	PtXLow = LowZ;
	PtXHigh = HighZ;
	PtYLow = LowY;
	PtYHigh = HighY;
	PtZLow = LowX;
	PtZHigh = HighX;
	} // else if
else
	{
	PtXLow = LowX;
	PtXHigh = HighX;
	PtYLow = LowZ;
	PtYHigh = HighZ;
	PtZLow = LowY;
	PtZHigh = HighY;
	} // else

Value = Input[WCS_TEXTURE_WOODGRAIN_CONTRAST][0];
if (Input[WCS_TEXTURE_WOODGRAIN_RINGSPACING][0] > 0.0 && Input[WCS_TEXTURE_WOODGRAIN_RINGSHARPNESS][0] > 0.0)
	{
	if (Input[WCS_TEXTURE_WOODGRAIN_TURBULENCE][0] > 0.0 && Noise)
		{
		Point.x = (PtXLow + PtXHigh) * .5;
		Point.y = (PtYLow + PtYHigh) * .5;
		Point.z = (PtZLow + PtZHigh) * .5;
		PtZLow += (Noise->EvaluateNoise(Point, 0, 0) * .5) * Input[WCS_TEXTURE_WOODGRAIN_TURBULENCE][0];
		} // if
	PtZLow /= Input[WCS_TEXTURE_WOODGRAIN_RINGSPACING][0];
	PtZLow -= WCS_floor(PtZLow);

	Lambda = 3.0 / Input[WCS_TEXTURE_WOODGRAIN_RINGSHARPNESS][0];
	if (PtZLow > 1.0 - Lambda)
		{
		Phase = (PtZLow - (1.0 - Lambda)) / Lambda;
		Value *= (-cos(Phase * TwoPi) * .5 + .5);
		} // if
	else
		Value = 0.0;
	} // if

Value += Input[WCS_TEXTURE_WOODGRAIN_BRIGHTNESS][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // MarbleTexture::Analyze

/*===========================================================================*/

void MarbleTexture::InitNoise(void)
{
double Args[3];

if (Noise)
	delete Noise;
Args[2] = .5;
Args[1] = 2.0;
Args[0] = TexParam[WCS_TEXTURE_WOODGRAIN_FREQUENCIES].CurValue;

Noise = new fBmNoise(Args, 3);

} // MarbleTexture::InitNoise

/*===========================================================================*/

BrickTexture::BrickTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_BRICK)
{

SetDefaults(ParamNumber);

} // BrickTexture::BrickTexture

/*===========================================================================*/

BrickTexture::BrickTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_BRICK, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // BrickTexture::BrickTexture

/*===========================================================================*/

void BrickTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	0, 0,	// 8
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 10
	0, 0	// 12
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",	// 0
	"Low (%) ", "High (%) ",	// 1
	"RowShift (%) ", "",	// 3
	"Brick Gap (%) ", "Layer Gap (%) ",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"", "",	// 9
	"Color ", "Color ",	// 11
	"", ""	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 50.0, 0.0, 10.0, 20.0, 0.0, 100.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
double SizeRangeDefaults[3] = {FLT_MAX, 0.001, .5};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexSize[1].SetDefaults(this, 1, .5);		// set brick height to .5 and resets range defaults
TexSize[1].SetRangeDefaults(SizeRangeDefaults);		// reset range defaults

TexType = WCS_TEXTURE_TYPE_BRICK;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // BrickTexture::SetDefaults

/*===========================================================================*/

char BrickTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_SHIFT, 0,
	WCS_TEXTURE_PARAMTYPE_WIDTH, WCS_TEXTURE_PARAMTYPE_WIDTH,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	0, 0
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // BrickTexture::GetTextureParamType

/*===========================================================================*/

double BrickTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Value;
long Row, Col, AlignCol;

ReCenterMoveRotate(LowX, HighX, LowY, HighY, LowZ, HighZ);
ReSizeT(LowX, HighX, LowY, HighY, LowZ, HighZ);

Value = Input[WCS_TEXTURE_BRICK_CONTRAST][0];
if (TexAxis == 0)
	{
	Row = quicklongfloor(LowZ + .5);
	if (Row % 2)
		LowY -= Input[WCS_TEXTURE_BRICK_ROWSHIFT][0];
	} // if
else if (TexAxis == 1)
	{
	Row = quicklongfloor(LowZ + .5);
	if (Row % 2)
		LowX -= Input[WCS_TEXTURE_BRICK_ROWSHIFT][0];
	} // if
else
	{
	Row = quicklongfloor(LowY + .5);
	if (Row % 2)
		LowX -= Input[WCS_TEXTURE_BRICK_ROWSHIFT][0];
	} // if
if (Quantize)
	{
	if (TexAxis == 0)
		{
		Col = quicklongfloor(LowY + .5);
		AlignCol = quicklongfloor(LowX + .5);
		} // if
	else if (TexAxis == 1)
		{
		Col = quicklongfloor(LowX + .5);
		AlignCol = quicklongfloor(LowY + .5);
		} // if
	else
		{
		Col = quicklongfloor(LowX + .5);
		AlignCol = quicklongfloor(LowZ + .5);
		} // if
	if (ThreeD)
		{
		RootTexture::TextureRand.Seed64(Row + AlignCol, Col + AlignCol);
		Value *= RootTexture::TextureRand.GenPRN();
		} // if
	else
		{
		RootTexture::TextureRand.Seed64(Row, Col);
		Value *= RootTexture::TextureRand.GenPRN();
		} // else
	} // if
else
	{
	LowX -= floor(LowX + .5);
	LowY -= floor(LowY + .5);
	LowZ -= floor(LowZ + .5);
	if (ThreeD)
		{
		if (TexAxis == 0)
			{
			if (fabs(LowZ) > .5 - Input[WCS_TEXTURE_BRICK_YMORTARWIDTH][0] * .5 ||
				fabs(LowY) > .5 - Input[WCS_TEXTURE_BRICK_XMORTARWIDTH][0] * .5 ||
				fabs(LowX) > .5 - Input[WCS_TEXTURE_BRICK_XMORTARWIDTH][0] * .5)
				Value = 0.0;
			} // if
		else if (TexAxis == 1)
			{
			if (fabs(LowZ) > .5 - Input[WCS_TEXTURE_BRICK_YMORTARWIDTH][0] * .5 ||
				fabs(LowX) > .5 - Input[WCS_TEXTURE_BRICK_XMORTARWIDTH][0] * .5 ||
				fabs(LowY) > .5 - Input[WCS_TEXTURE_BRICK_XMORTARWIDTH][0] * .5)
				Value = 0.0;
			} // if
		else
			{
			if (fabs(LowY) > .5 - Input[WCS_TEXTURE_BRICK_YMORTARWIDTH][0] * .5 ||
				fabs(LowX) > .5 - Input[WCS_TEXTURE_BRICK_XMORTARWIDTH][0] * .5 ||
				fabs(LowZ) > .5 - Input[WCS_TEXTURE_BRICK_XMORTARWIDTH][0] * .5)
				Value = 0.0;
			} // if
		} // if
	else if (TexAxis == 0)
		{
		if (fabs(LowY) > .5 - Input[WCS_TEXTURE_BRICK_XMORTARWIDTH][0] * .5 ||
			fabs(LowZ) > .5 - Input[WCS_TEXTURE_BRICK_YMORTARWIDTH][0] * .5)
			Value = 0.0;
		} // else
	else if (TexAxis == 1)
		{
		if (fabs(LowX) > .5 - Input[WCS_TEXTURE_BRICK_XMORTARWIDTH][0] * .5 ||
			fabs(LowZ) > .5 - Input[WCS_TEXTURE_BRICK_YMORTARWIDTH][0] * .5)
			Value = 0.0;
		} // else
	else
		{
		if (fabs(LowX) > .5 - Input[WCS_TEXTURE_BRICK_XMORTARWIDTH][0] * .5 ||
			fabs(LowY) > .5 - Input[WCS_TEXTURE_BRICK_YMORTARWIDTH][0] * .5)
			Value = 0.0;
		} // else
	} // else
Value += Input[WCS_TEXTURE_BRICK_BRIGHTNESS][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // BrickTexture::Analyze

/*===========================================================================*/

DotsTexture::DotsTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_BRICK)
{

SetDefaults(ParamNumber);

} // DotsTexture::DotsTexture

/*===========================================================================*/

DotsTexture::DotsTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_DOTS, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // DotsTexture::DotsTexture

/*===========================================================================*/

void DotsTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	0, 0,	// 8
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 10
	0, 0	// 12
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",	// 0
	"Low (%) ", "High (%) ",	// 1
	"RowShift (%) ", "",	// 3
	"Coverage (%) ", "Density (%) ",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"", "",	// 9
	"Color ", "Color ",	// 11
	"", ""	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 0.0, 0.0, 30.0, 100.0, 0.0, 100.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (unsigned char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_DOTS;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // DotsTexture::SetDefaults

/*===========================================================================*/

char DotsTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_SHIFT, 0,
	WCS_TEXTURE_PARAMTYPE_COVERAGE, WCS_TEXTURE_PARAMTYPE_COVERAGE,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	0, 0
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // DotsTexture::GetTextureParamType

/*===========================================================================*/

double DotsTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Value, Dist;
long Row, Col, ZCol;

ReCenterMoveRotate(LowX, HighX, LowY, HighY, LowZ, HighZ);
ReSizeT(LowX, HighX, LowY, HighY, LowZ, HighZ);

Value = Input[WCS_TEXTURE_DOTS_CONTRAST][0];
Row = quicklongfloor(LowY + .5);
if (Row % 2)
	{
	LowX -= Input[WCS_TEXTURE_DOTS_ROWSHIFT][0];
	LowZ -= Input[WCS_TEXTURE_DOTS_ROWSHIFT][0];
	} // if
Col = quicklongfloor(LowX + .5);
if (Quantize)
	{
	ZCol = quicklongfloor(LowZ + .5);
	if (ThreeD)
		{
		RootTexture::TextureRand.Seed64(Row + ZCol, Col + ZCol);
		Value *= RootTexture::TextureRand.GenPRN();
		} // if
	else if (TexAxis == 0)
		{
		RootTexture::TextureRand.Seed64(Row + ZCol, ZCol);
		Value *= RootTexture::TextureRand.GenPRN();
		} // else
	else if (TexAxis == 1)
		{
		RootTexture::TextureRand.Seed64(ZCol, Col + ZCol);
		Value *= RootTexture::TextureRand.GenPRN();
		} // else
	else
		{
		RootTexture::TextureRand.Seed64(Row, Col);
		Value *= RootTexture::TextureRand.GenPRN();
		} // else
	} // if
else
	{
	LowX -= WCS_floor(LowX + .5);
	LowY -= WCS_floor(LowY + .5);
	LowZ -= WCS_floor(LowZ + .5);
	RootTexture::TextureRand.Seed64(Row, Col);
	if (RootTexture::TextureRand.GenPRN() <= Input[WCS_TEXTURE_DOTS_DENSITY][0])
		{
		if (ThreeD)
			{
			Dist = sqrt(LowX * LowX + LowY * LowY + LowZ * LowZ) * (1.0 / 1.26);	// cube root of 2 rounded up to assure total coverage
			} // if
		else if (TexAxis == 0)
			{
			Dist = sqrt(LowY * LowY + LowZ * LowZ) * (1.0 / 1.415);	// sqrt of 2 rounded up to assure total coverage
			} // else
		else if (TexAxis == 1)
			{
			Dist = sqrt(LowX * LowX + LowZ * LowZ) * (1.0 / 1.415);	// sqrt of 2 rounded up to assure total coverage
			} // else
		else
			{
			Dist = sqrt(LowX * LowX + LowY * LowY) * (1.0 / 1.415);	// sqrt of 2 rounded up to assure total coverage
			} // else
		if (Dist > Input[WCS_TEXTURE_DOTS_COVERAGE][0])
			Value = 0.0;
		} // if
	else
		Value = 0.0;
	} // else
Value += Input[WCS_TEXTURE_DOTS_BRIGHTNESS][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // DotsTexture::Analyze

/*===========================================================================*/

VeinTexture::VeinTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_VEINS)
{

SetDefaults(ParamNumber);

} // VeinTexture::VeinTexture

/*===========================================================================*/

VeinTexture::VeinTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_VEINS, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // VeinTexture::VeinTexture

/*===========================================================================*/

void VeinTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 14
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 2
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 4
	0, 0,	// 6
	0, 0,	// 8
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 10
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 12
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",	// 14
	"Low (%) ", "High (%) ",	// 0
	"Octaves ", "Coverage (%) ",	// 2
	"Ledge Level (%) ", "Ledge Width (%) ",	// 4
	"", "",	// 6
	"", "",	// 8
	"Color ", "Color ",	// 10
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 1.0, 100.0, 50.0, 20.0, 0.0, 0.0, 30.0, 0.0};
double OctaveRangeDefaults[3] = {10.0, 1.0, 1.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_VEIN_OCTAVES].SetRangeDefaults(OctaveRangeDefaults);

TexType = WCS_TEXTURE_TYPE_VEINS;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // VeinTexture::SetDefaults

/*===========================================================================*/

char VeinTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_OCTAVES, WCS_TEXTURE_PARAMTYPE_COVERAGE,
	WCS_TEXTURE_PARAMTYPE_LEDGELEVEL, WCS_TEXTURE_PARAMTYPE_LEDGEWIDTH,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // VeinTexture::GetTextureParamType

/*===========================================================================*/

CrustTexture::CrustTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_CRUST)
{

SetDefaults(ParamNumber);

} // CrustTexture::CrustTexture

/*===========================================================================*/

CrustTexture::CrustTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_CRUST, EcosysSource, ColorSource, DefaultColor)
{
SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // CrustTexture::CrustTexture

/*===========================================================================*/

void CrustTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 14
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 2
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 4
	0, 0,	// 6
	0, 0,	// 8
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 10
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 12
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",	// 14
	"Low (%) ", "High (%) ",	// 0
	"Octaves ", "Coverage (%) ",	// 2
	"Ledge Level (%) ", "Ledge Width (%) ",	// 4
	"", "",	// 6
	"", "",		// 8
	"Color ", "Color ",	// 10
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 1.0, 100.0, 50.0, 20.0, 0.0, 0.0, 30.0, 0.0};
double OctaveRangeDefaults[3] = {10.0, 1.0, 1.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_CRUST_OCTAVES].SetRangeDefaults(OctaveRangeDefaults);

TexType = WCS_TEXTURE_TYPE_CRUST;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // CrustTexture::SetDefaults

/*===========================================================================*/

char CrustTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_OCTAVES, WCS_TEXTURE_PARAMTYPE_COVERAGE,
	WCS_TEXTURE_PARAMTYPE_LEDGELEVEL, WCS_TEXTURE_PARAMTYPE_LEDGEWIDTH,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // CrustTexture::GetTextureParamType

/*===========================================================================*/

UnderwaterTexture::UnderwaterTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_UNDERWATER)
{

SetDefaults(ParamNumber);

} // UnderwaterTexture::UnderwaterTexture

/*===========================================================================*/

UnderwaterTexture::UnderwaterTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_UNDERWATER, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // UnderwaterTexture::UnderwaterTexture

/*===========================================================================*/

void UnderwaterTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL,	// 3
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 5
	0, 0,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",	// 0
	"Low (%) ", "High (%) ",	// 1
	"Wave Sources ", "Wave Length (m) ",	// 3
	"Speed (m/s) ", " Sharpness (%) ",	// 5
	"", "",	// 7
	"", "",		// 9
	"Color ", "Color ",	// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 3.0, 1.0, 0.5, 50.0, 0.0, 0.0, 0.0, 0.0};
double OctaveRangeDefaults[3] = {10.0, 1.0, 1.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_UNDERWATER_WAVES].SetRangeDefaults(OctaveRangeDefaults);

TexType = WCS_TEXTURE_TYPE_UNDERWATER;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // UnderwaterTexture::SetDefaults

/*===========================================================================*/

char UnderwaterTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_SOURCES, WCS_TEXTURE_PARAMTYPE_LENGTH,
	WCS_TEXTURE_PARAMTYPE_SPEED, WCS_TEXTURE_PARAMTYPE_SHARPNESS,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // UnderwaterTexture::GetTextureParamType

/*===========================================================================*/

FractalNoiseTexture::FractalNoiseTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_FRACTALNOISE)
{

SetDefaults(ParamNumber);
InitNoise();

} // FractalNoiseTexture::FractalNoiseTexture

/*===========================================================================*/

FractalNoiseTexture::FractalNoiseTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_FRACTALNOISE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);
InitNoise();

} // FractalNoiseTexture::FractalNoiseTexture

/*===========================================================================*/

void FractalNoiseTexture::DeleteSpecificResources(void)
{

if (Noise)
	delete Noise;
Noise = NULL;

} // FractalNoiseTexture::DeleteSpecificResources

/*===========================================================================*/

void FractalNoiseTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Low (%) ", "High (%) ",			// 1
	"Octaves ", "Input Seed ",	// 3
	"Roughness (%) ", "Lacunarity (%) ",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 6.0, 1.0, 50.0, 51.0, 0.0, 100.0, 0.0, 0.0};
double OctaveRangeDefaults[3] = {10.0, 1.0, 1.0};
double LacunarityRangeDefaults[3] = {100.0, 1.0, 1.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
double RandDefaults[3] = {10000000.0, 0.0, 1.0};
long Ct;

Noise = NULL;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_FRACTALNOISE_OCTAVES].SetRangeDefaults(OctaveRangeDefaults);
TexParam[WCS_TEXTURE_FRACTALNOISE_LACUNARITY].SetRangeDefaults(LacunarityRangeDefaults);
TexParam[WCS_TEXTURE_FRACTALNOISE_INPUTSEED].SetRangeDefaults(RandDefaults);

TexType = WCS_TEXTURE_TYPE_FRACTALNOISE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // FractalNoiseTexture::SetDefaults

/*===========================================================================*/

char FractalNoiseTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_OCTAVES, WCS_TEXTURE_PARAMTYPE_SEED,
	WCS_TEXTURE_PARAMTYPE_ROUGHNESS, WCS_TEXTURE_PARAMTYPE_LACUNARITY,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // FractalNoiseTexture::GetTextureParamType

/*===========================================================================*/

double FractalNoiseTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Value, Args[3];
NoiseVector Point;

if ((Args[0] = FractalSetup(Input[WCS_TEXTURE_FRACTALNOISE_OCTAVES][0], LowX, HighX, LowY, HighY, LowZ, HighZ)) > 0.0)
	{
	Value = Input[WCS_TEXTURE_FRACTALNOISE_CONTRAST][0];
	if (Noise)
		{
		Point.x = LowX;
		Point.y = LowY;
		Point.z = LowZ;
		if (! (Tex[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS] && Tex[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS]->Enabled)
			&& ! (Tex[WCS_TEXTURE_FRACTALNOISE_LACUNARITY] && Tex[WCS_TEXTURE_FRACTALNOISE_LACUNARITY]->Enabled))
			{
			Value *= (Noise->EvaluateNoise(Point, Args, 1) * .5 + .5);
			} // if
		else
			{
			Args[2] = 1.0 - Input[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS][0];
			Args[1] = Input[WCS_TEXTURE_FRACTALNOISE_LACUNARITY][0] * 4;
			Value *= (Noise->EvaluateNoise(Point, Args, 3) * .5 + .5);
			} // else
		} // if
	else
		Value = 0.0;
	Value += Input[WCS_TEXTURE_FRACTALNOISE_BRIGHTNESS][0];
	} // if
else
	Value = Input[WCS_TEXTURE_FRACTALNOISE_BRIGHTNESS][0] + .5 * Input[WCS_TEXTURE_FRACTALNOISE_CONTRAST][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // FractalNoiseTexture::Analyze

/*===========================================================================*/

void FractalNoiseTexture::InitNoise(void)
{
double Args[3];

if (Noise)
	delete Noise;
Args[2] = 1.0 - (TexParam[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS].CurValue * (1.0 / 100.0));
Args[1] = TexParam[WCS_TEXTURE_FRACTALNOISE_LACUNARITY].CurValue * (1.0 / 25.0);
Args[0] = TexParam[WCS_TEXTURE_FRACTALNOISE_OCTAVES].CurValue;

Noise = new fBmNoise(Args, 3);

} // FractalNoiseTexture::InitNoise

/*===========================================================================*/

MultiFractalNoiseTexture::MultiFractalNoiseTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_MULTIFRACTALNOISE)
{

SetDefaults(ParamNumber);
InitNoise();

} // MultiFractalNoiseTexture::MultiFractalNoiseTexture

/*===========================================================================*/

MultiFractalNoiseTexture::MultiFractalNoiseTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_MULTIFRACTALNOISE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);
InitNoise();

} // MultiFractalNoiseTexture::MultiFractalNoiseTexture

/*===========================================================================*/

void MultiFractalNoiseTexture::DeleteSpecificResources(void)
{

if (Noise)
	delete Noise;
Noise = NULL;

} // MultiFractalNoiseTexture::DeleteSpecificResources

/*===========================================================================*/

void MultiFractalNoiseTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	WCS_TEXTURE_PARAMAVAIL, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Low (%) ", "High (%) ",			// 1
	"Octaves ", "Input Seed ",	// 3
	"Roughness (%) ", "Lacunarity (%) ",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"Offset ", "",		// 9
	"Color ", "Color ",		// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 6.0, 1.0, 50.0, 51.0, 0.0, 100.0, 1.0, 0.0};
double OctaveRangeDefaults[3] = {10.0, 1.0, 1.0};
double LacunarityRangeDefaults[3] = {100.0, 1.0, 1.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
double RandDefaults[3] = {10000000.0, 0.0, 1.0};
long Ct;

Noise = NULL;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_FRACTALNOISE_OCTAVES].SetRangeDefaults(OctaveRangeDefaults);
TexParam[WCS_TEXTURE_FRACTALNOISE_LACUNARITY].SetRangeDefaults(LacunarityRangeDefaults);
TexParam[WCS_TEXTURE_FRACTALNOISE_INPUTSEED].SetRangeDefaults(RandDefaults);

TexType = WCS_TEXTURE_TYPE_MULTIFRACTALNOISE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // MultiFractalNoiseTexture::SetDefaults

/*===========================================================================*/

char MultiFractalNoiseTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_OCTAVES, WCS_TEXTURE_PARAMTYPE_SEED,
	WCS_TEXTURE_PARAMTYPE_ROUGHNESS, WCS_TEXTURE_PARAMTYPE_LACUNARITY,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	WCS_TEXTURE_PARAMTYPE_OFFSET, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // MultiFractalNoiseTexture::GetTextureParamType

/*===========================================================================*/

double MultiFractalNoiseTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Value, Args[3];
NoiseVector Point;

if ((Args[0] = FractalSetup(Input[WCS_TEXTURE_FRACTALNOISE_OCTAVES][0], LowX, HighX, LowY, HighY, LowZ, HighZ)) > 0.0)
	{
	Value = Input[WCS_TEXTURE_FRACTALNOISE_CONTRAST][0];
	if (Noise)
		{
		Point.x = LowX;
		Point.y = LowY;
		Point.z = LowZ;
		if (! (Tex[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS] && Tex[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS]->Enabled)
			&& ! (Tex[WCS_TEXTURE_FRACTALNOISE_LACUNARITY] && Tex[WCS_TEXTURE_FRACTALNOISE_LACUNARITY]->Enabled))
			{
			Value *= (Noise->EvaluateNoise(Point, Args, 1) * .5 + .5);
			} // if
		else
			{
			Args[2] = 1.0 - Input[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS][0];
			Args[1] = Input[WCS_TEXTURE_FRACTALNOISE_LACUNARITY][0] * 4;
			Value *= (Noise->EvaluateNoise(Point, Args, 3) * .5 + .5);
			} // else
		} // if
	else
		Value = 0.0;
	Value += Input[WCS_TEXTURE_FRACTALNOISE_BRIGHTNESS][0];
	} // if
else
	Value = Input[WCS_TEXTURE_FRACTALNOISE_BRIGHTNESS][0] + .5 * Input[WCS_TEXTURE_FRACTALNOISE_CONTRAST][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // MultiFractalNoiseTexture::Analyze

/*===========================================================================*/

void MultiFractalNoiseTexture::InitNoise(void)
{
double Args[4];

if (Noise)
	delete Noise;
Args[3] = TexParam[WCS_TEXTURE_MULTIFRACTALNOISE_OFFSET].CurValue;
Args[2] = 1.0 - (TexParam[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS].CurValue  * (1.0 / 100.0));
Args[1] = TexParam[WCS_TEXTURE_FRACTALNOISE_LACUNARITY].CurValue  * (1.0 / 25.0);
Args[0] = TexParam[WCS_TEXTURE_FRACTALNOISE_OCTAVES].CurValue;

Noise = new MultiFractal(Args, 4);

} // MultiFractalNoiseTexture::InitNoise

/*===========================================================================*/

HybridMultiFractalNoiseTexture::HybridMultiFractalNoiseTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_HYBRIDMULTIFRACTALNOISE)
{

SetDefaults(ParamNumber);
InitNoise();

} // HybridMultiFractalNoiseTexture::HybridMultiFractalNoiseTexture

/*===========================================================================*/

HybridMultiFractalNoiseTexture::HybridMultiFractalNoiseTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_HYBRIDMULTIFRACTALNOISE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);
InitNoise();

} // HybridMultiFractalNoiseTexture::HybridMultiFractalNoiseTexture

/*===========================================================================*/

void HybridMultiFractalNoiseTexture::DeleteSpecificResources(void)
{

if (Noise)
	delete Noise;
Noise = NULL;

} // HybridMultiFractalNoiseTexture::DeleteSpecificResources

/*===========================================================================*/

void HybridMultiFractalNoiseTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	WCS_TEXTURE_PARAMAVAIL, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Low (%) ", "High (%) ",			// 1
	"Octaves ", "Input Seed ",	// 3
	"Roughness (%) ", "Lacunarity (%) ",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"Offset ", "",		// 9
	"Color ", "Color ",		// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 6.0, 1.0, 50.0, 51.0, 0.0, 100.0, 1.0, 0.0};
double OctaveRangeDefaults[3] = {10.0, 1.0, 1.0};
double LacunarityRangeDefaults[3] = {100.0, 1.0, 1.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
double RandDefaults[3] = {10000000.0, 0.0, 1.0};
long Ct;

Noise = NULL;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_FRACTALNOISE_OCTAVES].SetRangeDefaults(OctaveRangeDefaults);
TexParam[WCS_TEXTURE_FRACTALNOISE_LACUNARITY].SetRangeDefaults(LacunarityRangeDefaults);
TexParam[WCS_TEXTURE_FRACTALNOISE_INPUTSEED].SetRangeDefaults(RandDefaults);

TexType = WCS_TEXTURE_TYPE_HYBRIDMULTIFRACTALNOISE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // HybridMultiFractalNoiseTexture::SetDefaults

/*===========================================================================*/

char HybridMultiFractalNoiseTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_OCTAVES, WCS_TEXTURE_PARAMTYPE_SEED,
	WCS_TEXTURE_PARAMTYPE_ROUGHNESS, WCS_TEXTURE_PARAMTYPE_LACUNARITY,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	WCS_TEXTURE_PARAMTYPE_OFFSET, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // HybridMultiFractalNoiseTexture::GetTextureParamType

/*===========================================================================*/

double HybridMultiFractalNoiseTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Value, Args[3];
NoiseVector Point;

if ((Args[0] = FractalSetup(Input[WCS_TEXTURE_FRACTALNOISE_OCTAVES][0], LowX, HighX, LowY, HighY, LowZ, HighZ)) > 0.0)
	{
	Value = Input[WCS_TEXTURE_FRACTALNOISE_CONTRAST][0];
	if (Noise)
		{
		Point.x = LowX;
		Point.y = LowY;
		Point.z = LowZ;
		if (! (Tex[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS] && Tex[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS]->Enabled)
			&& ! (Tex[WCS_TEXTURE_FRACTALNOISE_LACUNARITY] && Tex[WCS_TEXTURE_FRACTALNOISE_LACUNARITY]->Enabled))
			{
			Value *= (Noise->EvaluateNoise(Point, Args, 1) * .333333);
			} // if
		else
			{
			Args[2] = 1.0 - Input[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS][0];
			Args[1] = Input[WCS_TEXTURE_FRACTALNOISE_LACUNARITY][0] * 4;
			Value *= (Noise->EvaluateNoise(Point, Args, 3) * .333333);
			} // else
		} // if
	else
		Value = 0.0;
	Value += Input[WCS_TEXTURE_FRACTALNOISE_BRIGHTNESS][0];
	} // if
else
	Value = Input[WCS_TEXTURE_FRACTALNOISE_BRIGHTNESS][0] + .5 * Input[WCS_TEXTURE_FRACTALNOISE_CONTRAST][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // HybridMultiFractalNoiseTexture::Analyze

/*===========================================================================*/

void HybridMultiFractalNoiseTexture::InitNoise(void)
{
double Args[4];

if (Noise)
	delete Noise;
Args[3] = TexParam[WCS_TEXTURE_MULTIFRACTALNOISE_OFFSET].CurValue;
Args[2] = 1.0 - (TexParam[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS].CurValue  * (1.0 / 100.0));
Args[1] = TexParam[WCS_TEXTURE_FRACTALNOISE_LACUNARITY].CurValue  * (1.0 / 25.0);
Args[0] = TexParam[WCS_TEXTURE_FRACTALNOISE_OCTAVES].CurValue;

Noise = new HybridMultiFractal(Args, 4);

} // HybridMultiFractalNoiseTexture::InitNoise

/*===========================================================================*/

RidgedMultiFractalNoiseTexture::RidgedMultiFractalNoiseTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_RIDGEDMULTIFRACTALNOISE)
{

SetDefaults(ParamNumber);
InitNoise();

} // RidgedMultiFractalNoiseTexture::RidgedMultiFractalNoiseTexture

/*===========================================================================*/

RidgedMultiFractalNoiseTexture::RidgedMultiFractalNoiseTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_RIDGEDMULTIFRACTALNOISE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);
InitNoise();

} // RidgedMultiFractalNoiseTexture::RidgedMultiFractalNoiseTexture

/*===========================================================================*/

void RidgedMultiFractalNoiseTexture::DeleteSpecificResources(void)
{

if (Noise)
	delete Noise;
Noise = NULL;

} // RidgedMultiFractalNoiseTexture::DeleteSpecificResources

/*===========================================================================*/

void RidgedMultiFractalNoiseTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Low (%) ", "High (%) ",			// 1
	"Octaves ", "Input Seed ",	// 3
	"Roughness (%) ", "Lacunarity (%) ",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"Offset ", "Gain (%) ",		// 9
	"Color ", "Color ",		// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 6.0, 1.0, 50.0, 51.0, 0.0, 100.0, 1.0, 0.0};
double OctaveRangeDefaults[3] = {10.0, 1.0, 1.0};
double LacunarityRangeDefaults[3] = {100.0, 1.0, 1.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
double RandDefaults[3] = {10000000.0, 0.0, 1.0};
long Ct;

Noise = NULL;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_FRACTALNOISE_OCTAVES].SetRangeDefaults(OctaveRangeDefaults);
TexParam[WCS_TEXTURE_FRACTALNOISE_LACUNARITY].SetRangeDefaults(LacunarityRangeDefaults);
TexParam[WCS_TEXTURE_FRACTALNOISE_INPUTSEED].SetRangeDefaults(RandDefaults);

TexType = WCS_TEXTURE_TYPE_RIDGEDMULTIFRACTALNOISE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // RidgedMultiFractalNoiseTexture::SetDefaults

/*===========================================================================*/

char RidgedMultiFractalNoiseTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_OCTAVES, WCS_TEXTURE_PARAMTYPE_SEED,
	WCS_TEXTURE_PARAMTYPE_ROUGHNESS, WCS_TEXTURE_PARAMTYPE_LACUNARITY,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	WCS_TEXTURE_PARAMTYPE_OFFSET, WCS_TEXTURE_PARAMTYPE_GAIN,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // RidgedMultiFractalNoiseTexture::GetTextureParamType

/*===========================================================================*/

double RidgedMultiFractalNoiseTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Value, Args[5];
NoiseVector Point;

if ((Args[0] = FractalSetup(Input[WCS_TEXTURE_FRACTALNOISE_OCTAVES][0], LowX, HighX, LowY, HighY, LowZ, HighZ)) > 0.0)
	{
	Value = Input[WCS_TEXTURE_FRACTALNOISE_CONTRAST][0];
	if (Noise)
		{
		Point.x = LowX;
		Point.y = LowY;
		Point.z = LowZ;
		if (! (Tex[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS] && Tex[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS]->Enabled)
			&& ! (Tex[WCS_TEXTURE_FRACTALNOISE_LACUNARITY] && Tex[WCS_TEXTURE_FRACTALNOISE_LACUNARITY]->Enabled)
			&& ! (Tex[WCS_TEXTURE_RIDGEDMULTIFRACTALNOISE_GAIN] && Tex[WCS_TEXTURE_RIDGEDMULTIFRACTALNOISE_GAIN]->Enabled))
			{
			Value *= (Noise->EvaluateNoise(Point, Args, 1));
			} // if
		else
			{
			Args[4] = Input[WCS_TEXTURE_RIDGEDMULTIFRACTALNOISE_GAIN][0];
			Args[3] = Input[WCS_TEXTURE_MULTIFRACTALNOISE_OFFSET][0];
			Args[2] = 1.0 - Input[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS][0];
			Args[1] = Input[WCS_TEXTURE_FRACTALNOISE_LACUNARITY][0] * 4;
			Value *= (Noise->EvaluateNoise(Point, Args, 5));
			} // else
		} // if
	else
		Value = 0.0;
	Value += Input[WCS_TEXTURE_FRACTALNOISE_BRIGHTNESS][0];
	} // if
else
	Value = Input[WCS_TEXTURE_FRACTALNOISE_BRIGHTNESS][0] + .5 * Input[WCS_TEXTURE_FRACTALNOISE_CONTRAST][0];
Value = PERLIN_clamp(Value, 0.0, 1.0);

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // RidgedMultiFractalNoiseTexture::Analyze

/*===========================================================================*/

void RidgedMultiFractalNoiseTexture::InitNoise(void)
{
double Args[5];

if (Noise)
	delete Noise;
Args[4] = TexParam[WCS_TEXTURE_RIDGEDMULTIFRACTALNOISE_GAIN].CurValue  * (1.0 / 100.0);
Args[3] = TexParam[WCS_TEXTURE_MULTIFRACTALNOISE_OFFSET].CurValue;
Args[2] = 1.0 - (TexParam[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS].CurValue  * (1.0 / 100.0));
Args[1] = TexParam[WCS_TEXTURE_FRACTALNOISE_LACUNARITY].CurValue  * (1.0 / 25.0);
Args[0] = TexParam[WCS_TEXTURE_FRACTALNOISE_OCTAVES].CurValue;

Noise = new RidgedMultiFractal(Args, 5);

} // RidgedMultiFractalNoiseTexture::InitNoise

/*===========================================================================*/

HeteroTerrainNoiseTexture::HeteroTerrainNoiseTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_HETEROTERRAINNOISE)
{

SetDefaults(ParamNumber);
InitNoise();

} // HeteroTerrainNoiseTexture::HeteroTerrainNoiseTexture

/*===========================================================================*/

HeteroTerrainNoiseTexture::HeteroTerrainNoiseTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_HETEROTERRAINNOISE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);
InitNoise();

} // HeteroTerrainNoiseTexture::HeteroTerrainNoiseTexture

/*===========================================================================*/

void HeteroTerrainNoiseTexture::DeleteSpecificResources(void)
{

if (Noise)
	delete Noise;
Noise = NULL;

} // HeteroTerrainNoiseTexture::DeleteSpecificResources

/*===========================================================================*/

void HeteroTerrainNoiseTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	WCS_TEXTURE_PARAMAVAIL, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Low (%) ", "High (%) ",			// 1
	"Octaves ", "Input Seed ",	// 3
	"Roughness (%) ", "Lacunarity (%) ",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"Offset ", "",		// 9
	"Color ", "Color ",		// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 6.0, 1.0, 50.0, 51.0, 0.0, 100.0, 1.0, 0.0};
double OctaveRangeDefaults[3] = {10.0, 1.0, 1.0};
double LacunarityRangeDefaults[3] = {100.0, 1.0, 1.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
double RandDefaults[3] = {10000000.0, 0.0, 1.0};
long Ct;

Noise = NULL;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_FRACTALNOISE_OCTAVES].SetRangeDefaults(OctaveRangeDefaults);
TexParam[WCS_TEXTURE_FRACTALNOISE_LACUNARITY].SetRangeDefaults(LacunarityRangeDefaults);
TexParam[WCS_TEXTURE_FRACTALNOISE_INPUTSEED].SetRangeDefaults(RandDefaults);

TexType = WCS_TEXTURE_TYPE_HETEROTERRAINNOISE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // HeteroTerrainNoiseTexture::SetDefaults

/*===========================================================================*/

char HeteroTerrainNoiseTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_OCTAVES, WCS_TEXTURE_PARAMTYPE_SEED,
	WCS_TEXTURE_PARAMTYPE_ROUGHNESS, WCS_TEXTURE_PARAMTYPE_LACUNARITY,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	WCS_TEXTURE_PARAMTYPE_OFFSET, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // HeteroTerrainNoiseTexture::GetTextureParamType

/*===========================================================================*/

double HeteroTerrainNoiseTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Value, Args[3];
NoiseVector Point;

if ((Args[0] = FractalSetup(Input[WCS_TEXTURE_FRACTALNOISE_OCTAVES][0], LowX, HighX, LowY, HighY, LowZ, HighZ)) > 0.0)
	{
	Value = Input[WCS_TEXTURE_FRACTALNOISE_CONTRAST][0];
	if (Noise)
		{
		Point.x = LowX;
		Point.y = LowY;
		Point.z = LowZ;
		if (! (Tex[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS] && Tex[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS]->Enabled)
			&& ! (Tex[WCS_TEXTURE_FRACTALNOISE_LACUNARITY] && Tex[WCS_TEXTURE_FRACTALNOISE_LACUNARITY]->Enabled))
			{
			Value *= (Noise->EvaluateNoise(Point, Args, 1) * .5 + .5);
			} // if
		else
			{
			Args[2] = 1.0 - Input[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS][0];
			Args[1] = Input[WCS_TEXTURE_FRACTALNOISE_LACUNARITY][0] * 4;
			Value *= (Noise->EvaluateNoise(Point, Args, 3) * .5 + .5);
			} // else
		} // if
	else
		Value = 0.0;
	Value += Input[WCS_TEXTURE_FRACTALNOISE_BRIGHTNESS][0];
	} // if
else
	Value = Input[WCS_TEXTURE_FRACTALNOISE_BRIGHTNESS][0] + .5 * Input[WCS_TEXTURE_FRACTALNOISE_CONTRAST][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // HeteroTerrainNoiseTexture::Analyze

/*===========================================================================*/

void HeteroTerrainNoiseTexture::InitNoise(void)
{
double Args[4];

if (Noise)
	delete Noise;
Args[3] = TexParam[WCS_TEXTURE_MULTIFRACTALNOISE_OFFSET].CurValue;
Args[2] = 1.0 - (TexParam[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS].CurValue  * (1.0 / 100.0));
Args[1] = TexParam[WCS_TEXTURE_FRACTALNOISE_LACUNARITY].CurValue  * (1.0 / 25.0);
Args[0] = TexParam[WCS_TEXTURE_FRACTALNOISE_OCTAVES].CurValue;

Noise = new HeteroTerrain(Args, 4);

} // HeteroTerrainNoiseTexture::InitNoise

/*===========================================================================*/

TurbulenceTexture::TurbulenceTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_TURBULENCE)
{

SetDefaults(ParamNumber);
InitNoise();

} // TurbulenceTexture::TurbulenceTexture

/*===========================================================================*/

TurbulenceTexture::TurbulenceTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_TURBULENCE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);
InitNoise();

} // TurbulenceTexture::TurbulenceTexture

/*===========================================================================*/

void TurbulenceTexture::DeleteSpecificResources(void)
{

if (Noise)
	delete Noise;
Noise = NULL;

} // TurbulenceTexture::DeleteSpecificResources

/*===========================================================================*/

void TurbulenceTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Low (%) ", "High (%) ",			// 1
	"Octaves ", "Input Seed ",	// 3
	"Roughness (%) ", "Lacunarity (%) ",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 6.0, 1.0, 50.0, 50.0, 0.0, 100.0, 0.0, 0.0};
double OctaveRangeDefaults[3] = {10.0, 1.0, 1.0};
double LacunarityRangeDefaults[3] = {100.0, 1.0, 1.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
double RandDefaults[3] = {10000000.0, 0.0, 1.0};
long Ct;

Noise = NULL;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_FRACTALNOISE_OCTAVES].SetRangeDefaults(OctaveRangeDefaults);
TexParam[WCS_TEXTURE_FRACTALNOISE_LACUNARITY].SetRangeDefaults(LacunarityRangeDefaults);
TexParam[WCS_TEXTURE_FRACTALNOISE_INPUTSEED].SetRangeDefaults(RandDefaults);

TexType = WCS_TEXTURE_TYPE_TURBULENCE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // TurbulenceTexture::SetDefaults

/*===========================================================================*/

char TurbulenceTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_OCTAVES, WCS_TEXTURE_PARAMTYPE_SEED,
	WCS_TEXTURE_PARAMTYPE_ROUGHNESS, WCS_TEXTURE_PARAMTYPE_LACUNARITY,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // TurbulenceTexture::GetTextureParamType

/*===========================================================================*/

double TurbulenceTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Value, Args[3];
NoiseVector Point;

if ((Args[0] = FractalSetup(Input[WCS_TEXTURE_FRACTALNOISE_OCTAVES][0], LowX, HighX, LowY, HighY, LowZ, HighZ)) > 0.0)
	{
	Value = Input[WCS_TEXTURE_FRACTALNOISE_CONTRAST][0];
	if (Noise)
		{
		Point.x = LowX;
		Point.y = LowY;
		Point.z = LowZ;
		if (! (Tex[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS] && Tex[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS]->Enabled)
			&& ! (Tex[WCS_TEXTURE_FRACTALNOISE_LACUNARITY] && Tex[WCS_TEXTURE_FRACTALNOISE_LACUNARITY]->Enabled))
			{
			Value *= fabs(Noise->EvaluateNoise(Point, Args, 1) * .6666666);
			} // if
		else
			{
			Args[2] = 1.0 - Input[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS][0];
			Args[1] = Input[WCS_TEXTURE_FRACTALNOISE_LACUNARITY][0] * 4;
			Value *= fabs(Noise->EvaluateNoise(Point, Args, 3) * .6666666);
			} // else
		} // if
	else
		Value = 0.0;
	Value += Input[WCS_TEXTURE_FRACTALNOISE_BRIGHTNESS][0];
	} // if
else
	Value = Input[WCS_TEXTURE_FRACTALNOISE_BRIGHTNESS][0] + .5 * Input[WCS_TEXTURE_FRACTALNOISE_CONTRAST][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // TurbulenceTexture::Analyze

/*===========================================================================*/

void TurbulenceTexture::InitNoise(void)
{
double Args[3];

if (Noise)
	delete Noise;
Args[2] = 1.0 - (TexParam[WCS_TEXTURE_FRACTALNOISE_ROUGHNESS].CurValue  * (1.0 / 100.0));
Args[1] = TexParam[WCS_TEXTURE_FRACTALNOISE_LACUNARITY].CurValue  * (1.0 / 25.0);
Args[0] = TexParam[WCS_TEXTURE_FRACTALNOISE_OCTAVES].CurValue;

Noise = new fBmTurbulentNoise(Args, 3);

} // TurbulenceTexture::InitNoise

/*===========================================================================*/

F1CellBasisTexture::F1CellBasisTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_F1CELLBASIS)
{

SetDefaults(ParamNumber);
InitBasis();

} // F1CellBasisTexture::F1CellBasisTexture

/*===========================================================================*/

F1CellBasisTexture::F1CellBasisTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_F1CELLBASIS, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);
InitBasis();

} // F1CellBasisTexture::F1CellBasisTexture

/*===========================================================================*/

void F1CellBasisTexture::DeleteSpecificResources(void)
{

if (Basis)
	delete Basis;
Basis = NULL;

} // F1CellBasisTexture::DeleteSpecificResources

/*===========================================================================*/

void F1CellBasisTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL, 0,	// 3
	0, 0,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Low (%) ", "High (%) ",			// 1
	"Octaves ", "",	// 3
	"", "",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 1.0, 0.0, 0.0, 0.0, 0.0, 100.0, 0.0, 0.0};
double OctaveRangeDefaults[3] = {10.0, 1.0, 1.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

Basis = NULL;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_CELLBASIS_OCTAVES].SetRangeDefaults(OctaveRangeDefaults);

TexType = WCS_TEXTURE_TYPE_F1CELLBASIS;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // F1CellBasisTexture::SetDefaults

/*===========================================================================*/

char F1CellBasisTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_CELLOCTAVES, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // F1CellBasisTexture::GetTextureParamType

/*===========================================================================*/

double F1CellBasisTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
unsigned long Seed[2];
double ClosePt[4];
double Value, Args[1], Point[3];

if ((Args[0] = CellBasisSetup(Input[WCS_TEXTURE_CELLBASIS_OCTAVES][0], LowX, HighX, LowY, HighY, LowZ, HighZ)) > 0.0)
	{
	Value = Input[WCS_TEXTURE_CELLBASIS_CONTRAST][0];
	if (Basis)
		{
		if (! ThreeD)
			{
			if (TexAxis == 0)
				{
				Point[0] = LowY;
				Point[1] = LowZ;
				Point[2] = LowX;
				} // if
			else if (TexAxis == 1)
				{
				Point[0] = LowX;
				Point[1] = LowZ;
				Point[2] = LowY;
				} // else if
			else
				{
				Point[0] = LowX;
				Point[1] = LowY;
				Point[2] = LowZ;
				} // else
			} // if
		else
			{
			Point[0] = LowX;
			Point[1] = LowY;
			Point[2] = LowZ;
			} // switch
		if (Quantize)
			{
			ClosePt[0] = ClosePt[1] = ClosePt[2] = ClosePt[3] = 0.0;
			Basis->Evaluate(Point, Args, 1, ClosePt);
			Seed[0] = (ULONG)(ClosePt[0] * ULONG_MAX);
			Seed[1] = (ULONG)(ClosePt[1] * ULONG_MAX);
			CellularBasis::CellBasisRand.Seed64BitShift(Seed[0], Seed[1]);
			Value *= CellularBasis::CellBasisRand.GenPRN();
			} // if
		else
			Value *= (Basis->Evaluate(Point, Args, 1));
		} // if
	else
		Value = 0.0;
	Value += Input[WCS_TEXTURE_CELLBASIS_BRIGHTNESS][0];
	} // if
else
	Value = Input[WCS_TEXTURE_CELLBASIS_BRIGHTNESS][0] + .5 * Input[WCS_TEXTURE_CELLBASIS_CONTRAST][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // F1CellBasisTexture::Analyze

/*===========================================================================*/

void F1CellBasisTexture::InitBasis(void)
{
double Args[4];

if (Basis)
	delete Basis;
Args[0] = TexParam[WCS_TEXTURE_CELLBASIS_OCTAVES].CurValue;		// octaves
Args[1] = ThreeD ? 3.0: 2.0;	// dimensions
Args[2] = 1.0;	// points needed
Args[3] = 3.0;	// avg points per cell
Basis = new F1CellBasis3(Args, 4);

} // F1CellBasisTexture::InitBasis

/*===========================================================================*/

F2CellBasisTexture::F2CellBasisTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_F2CELLBASIS)
{

SetDefaults(ParamNumber);
InitBasis();

} // F2CellBasisTexture::F2CellBasisTexture

/*===========================================================================*/

F2CellBasisTexture::F2CellBasisTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_F2CELLBASIS, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);
InitBasis();

} // F2CellBasisTexture::F2CellBasisTexture

/*===========================================================================*/

void F2CellBasisTexture::DeleteSpecificResources(void)
{

if (Basis)
	delete Basis;
Basis = NULL;

} // F2CellBasisTexture::DeleteSpecificResources

/*===========================================================================*/

void F2CellBasisTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL, 0,	// 3
	0, 0,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Low (%) ", "High (%) ",			// 1
	"Octaves ", "",	// 3
	"", "",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 1.0, 0.0, 0.0, 0.0, 0.0, 100.0, 0.0, 0.0};
double OctaveRangeDefaults[3] = {10.0, 1.0, 1.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

Basis = NULL;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_CELLBASIS_OCTAVES].SetRangeDefaults(OctaveRangeDefaults);

TexType = WCS_TEXTURE_TYPE_F2CELLBASIS;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // F2CellBasisTexture::SetDefaults

/*===========================================================================*/

char F2CellBasisTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_CELLOCTAVES, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // F2CellBasisTexture::GetTextureParamType

/*===========================================================================*/

double F2CellBasisTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
unsigned long Seed[2];
double ClosePt[4];
double Value, Args[1], Point[3];

if ((Args[0] = CellBasisSetup(Input[WCS_TEXTURE_CELLBASIS_OCTAVES][0], LowX, HighX, LowY, HighY, LowZ, HighZ)) > 0.0)
	{
	Value = Input[WCS_TEXTURE_CELLBASIS_CONTRAST][0];
	if (Basis)
		{
		if (! ThreeD)
			{
			if (TexAxis == 0)
				{
				Point[0] = LowY;
				Point[1] = LowZ;
				Point[2] = LowX;
				} // if
			else if (TexAxis == 1)
				{
				Point[0] = LowX;
				Point[1] = LowZ;
				Point[2] = LowY;
				} // else if
			else
				{
				Point[0] = LowX;
				Point[1] = LowY;
				Point[2] = LowZ;
				} // else
			} // if
		else
			{
			Point[0] = LowX;
			Point[1] = LowY;
			Point[2] = LowZ;
			} // switch
		if (Quantize)
			{
			ClosePt[0] = ClosePt[1] = ClosePt[2] = ClosePt[3] = 0.0;
			Basis->Evaluate(Point, Args, 1, ClosePt);
			Seed[0] = (ULONG)(ClosePt[0] * ULONG_MAX);
			Seed[1] = (ULONG)(ClosePt[1] * ULONG_MAX);
			CellularBasis::CellBasisRand.Seed64BitShift(Seed[0], Seed[1]);
			Value *= CellularBasis::CellBasisRand.GenPRN();
			} // if
		else
			Value *= (Basis->Evaluate(Point, Args, 1));
		} // if
	else
		Value = 0.0;
	Value += Input[WCS_TEXTURE_CELLBASIS_BRIGHTNESS][0];
	} // if
else
	Value = Input[WCS_TEXTURE_CELLBASIS_BRIGHTNESS][0] + .5 * Input[WCS_TEXTURE_CELLBASIS_CONTRAST][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // F2CellBasisTexture::Analyze

/*===========================================================================*/

void F2CellBasisTexture::InitBasis(void)
{
double Args[4];

if (Basis)
	delete Basis;
Args[0] = TexParam[WCS_TEXTURE_CELLBASIS_OCTAVES].CurValue;		// octaves
Args[1] = ThreeD ? 3.0: 2.0;	// dimensions
Args[2] = 2.0;	// points needed
Args[3] = 4.0;	// avg points per cell
Basis = new F2CellBasis3(Args, 4);

} // F2CellBasisTexture::InitBasis

/*===========================================================================*/

F2mF1CellBasisTexture::F2mF1CellBasisTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_F2MF1CELLBASIS)
{

SetDefaults(ParamNumber);
InitBasis();

} // F2mF1CellBasisTexture::F2mF1CellBasisTexture

/*===========================================================================*/

F2mF1CellBasisTexture::F2mF1CellBasisTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_F2MF1CELLBASIS, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);
InitBasis();

} // F2mF1CellBasisTexture::F2mF1CellBasisTexture

/*===========================================================================*/

void F2mF1CellBasisTexture::DeleteSpecificResources(void)
{

if (Basis)
	delete Basis;
Basis = NULL;

} // F2mF1CellBasisTexture::DeleteSpecificResources

/*===========================================================================*/

void F2mF1CellBasisTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL, 0,	// 3
	0, 0,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Low (%) ", "High (%) ",			// 1
	"Octaves ", "",	// 3
	"", "",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 1.0, 0.0, 0.0, 0.0, 0.0, 100.0, 0.0, 0.0};
double OctaveRangeDefaults[3] = {10.0, 1.0, 1.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

Basis = NULL;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_CELLBASIS_OCTAVES].SetRangeDefaults(OctaveRangeDefaults);

TexType = WCS_TEXTURE_TYPE_F2MF1CELLBASIS;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // F2mF1CellBasisTexture::SetDefaults

/*===========================================================================*/

char F2mF1CellBasisTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_CELLOCTAVES, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // F2mF1CellBasisTexture::GetTextureParamType

/*===========================================================================*/

double F2mF1CellBasisTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
unsigned long Seed[2];
double ClosePt[4];
double Value, Args[1], Point[3];

if ((Args[0] = CellBasisSetup(Input[WCS_TEXTURE_CELLBASIS_OCTAVES][0], LowX, HighX, LowY, HighY, LowZ, HighZ)) > 0.0)
	{
	Value = Input[WCS_TEXTURE_CELLBASIS_CONTRAST][0];
	if (Basis)
		{
		if (! ThreeD)
			{
			if (TexAxis == 0)
				{
				Point[0] = LowY;
				Point[1] = LowZ;
				Point[2] = LowX;
				} // if
			else if (TexAxis == 1)
				{
				Point[0] = LowX;
				Point[1] = LowZ;
				Point[2] = LowY;
				} // else if
			else
				{
				Point[0] = LowX;
				Point[1] = LowY;
				Point[2] = LowZ;
				} // else
			} // if
		else
			{
			Point[0] = LowX;
			Point[1] = LowY;
			Point[2] = LowZ;
			} // switch
		if (Quantize)
			{
			ClosePt[0] = ClosePt[1] = ClosePt[2] = ClosePt[3] = 0.0;
			Basis->Evaluate(Point, Args, 1, ClosePt);
			Seed[0] = (ULONG)(ClosePt[0] * ULONG_MAX);
			Seed[1] = (ULONG)(ClosePt[1] * ULONG_MAX);
			CellularBasis::CellBasisRand.Seed64BitShift(Seed[0], Seed[1]);
			Value *= CellularBasis::CellBasisRand.GenPRN();
			} // if
		else
			Value *= (Basis->Evaluate(Point, Args, 1));
		} // if
	else
		Value = 0.0;
	Value += Input[WCS_TEXTURE_CELLBASIS_BRIGHTNESS][0];
	} // if
else
	Value = Input[WCS_TEXTURE_CELLBASIS_BRIGHTNESS][0] + .5 * Input[WCS_TEXTURE_CELLBASIS_CONTRAST][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // F2mF1CellBasisTexture::Analyze

/*===========================================================================*/

void F2mF1CellBasisTexture::InitBasis(void)
{
double Args[4];

if (Basis)
	delete Basis;
Args[0] = TexParam[WCS_TEXTURE_CELLBASIS_OCTAVES].CurValue;		// octaves
Args[1] = ThreeD ? 3.0: 2.0;	// dimensions
Args[2] = 2.0;	// points needed
Args[3] = 4.0;	// avg points per cell
Basis = new F2mF1CellBasis3(Args, 4);

} // F2mF1CellBasisTexture::InitBasis

/*===========================================================================*/

F3mF1CellBasisTexture::F3mF1CellBasisTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_F3MF1CELLBASIS)
{

SetDefaults(ParamNumber);
InitBasis();

} // F3mF1CellBasisTexture::F3mF1CellBasisTexture

/*===========================================================================*/

F3mF1CellBasisTexture::F3mF1CellBasisTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_F3MF1CELLBASIS, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);
InitBasis();

} // F3mF1CellBasisTexture::F3mF1CellBasisTexture

/*===========================================================================*/

void F3mF1CellBasisTexture::DeleteSpecificResources(void)
{

if (Basis)
	delete Basis;
Basis = NULL;

} // F3mF1CellBasisTexture::DeleteSpecificResources

/*===========================================================================*/

void F3mF1CellBasisTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL, 0,	// 3
	0, 0,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Low (%) ", "High (%) ",			// 1
	"Octaves ", "",	// 3
	"", "",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 1.0, 0.0, 0.0, 0.0, 0.0, 100.0, 0.0, 0.0};
double OctaveRangeDefaults[3] = {10.0, 1.0, 1.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

Basis = NULL;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_CELLBASIS_OCTAVES].SetRangeDefaults(OctaveRangeDefaults);

TexType = WCS_TEXTURE_TYPE_F3MF1CELLBASIS;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // F3mF1CellBasisTexture::SetDefaults

/*===========================================================================*/

char F3mF1CellBasisTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_CELLOCTAVES, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // F3mF1CellBasisTexture::GetTextureParamType

/*===========================================================================*/

double F3mF1CellBasisTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
unsigned long Seed[2];
double ClosePt[4];
double Value, Args[1], Point[3];

if ((Args[0] = CellBasisSetup(Input[WCS_TEXTURE_CELLBASIS_OCTAVES][0], LowX, HighX, LowY, HighY, LowZ, HighZ)) > 0.0)
	{
	Value = Input[WCS_TEXTURE_CELLBASIS_CONTRAST][0];
	if (Basis)
		{
		if (! ThreeD)
			{
			if (TexAxis == 0)
				{
				Point[0] = LowY;
				Point[1] = LowZ;
				Point[2] = LowX;
				} // if
			else if (TexAxis == 1)
				{
				Point[0] = LowX;
				Point[1] = LowZ;
				Point[2] = LowY;
				} // else if
			else
				{
				Point[0] = LowX;
				Point[1] = LowY;
				Point[2] = LowZ;
				} // else
			} // if
		else
			{
			Point[0] = LowX;
			Point[1] = LowY;
			Point[2] = LowZ;
			} // switch
		if (Quantize)
			{
			ClosePt[0] = ClosePt[1] = ClosePt[2] = ClosePt[3] = 0.0;
			Basis->Evaluate(Point, Args, 1, ClosePt);
			Seed[0] = (ULONG)(ClosePt[0] * ULONG_MAX);
			Seed[1] = (ULONG)(ClosePt[1] * ULONG_MAX);
			CellularBasis::CellBasisRand.Seed64BitShift(Seed[0], Seed[1]);
			Value *= CellularBasis::CellBasisRand.GenPRN();
			} // if
		else
			Value *= (Basis->Evaluate(Point, Args, 1));
		} // if
	else
		Value = 0.0;
	Value += Input[WCS_TEXTURE_CELLBASIS_BRIGHTNESS][0];
	} // if
else
	Value = Input[WCS_TEXTURE_CELLBASIS_BRIGHTNESS][0] + .5 * Input[WCS_TEXTURE_CELLBASIS_CONTRAST][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // F3mF1CellBasisTexture::Analyze

/*===========================================================================*/

void F3mF1CellBasisTexture::InitBasis(void)
{
double Args[4];

if (Basis)
	delete Basis;
Args[0] = TexParam[WCS_TEXTURE_CELLBASIS_OCTAVES].CurValue;		// octaves
Args[1] = ThreeD ? 3.0: 2.0;	// dimensions
Args[2] = 3.0;	// points needed
Args[3] = 6.0;	// avg points per cell
Basis = new F3mF1CellBasis3(Args, 4);

} // F3mF1CellBasisTexture::InitBasis

/*===========================================================================*/

F1ManhattanTexture::F1ManhattanTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_F1MANHATTAN)
{

SetDefaults(ParamNumber);
InitBasis();

} // F1ManhattanTexture::F1ManhattanTexture

/*===========================================================================*/

F1ManhattanTexture::F1ManhattanTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_F1MANHATTAN, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);
InitBasis();

} // F1ManhattanTexture::F1ManhattanTexture

/*===========================================================================*/

void F1ManhattanTexture::DeleteSpecificResources(void)
{

if (Basis)
	delete Basis;
Basis = NULL;

} // F1ManhattanTexture::DeleteSpecificResources

/*===========================================================================*/

void F1ManhattanTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL, 0,	// 3
	0, 0,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Low (%) ", "High (%) ",			// 1
	"Octaves ", "",	// 3
	"", "",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 1.0, 0.0, 0.0, 0.0, 0.0, 100.0, 0.0, 0.0};
double OctaveRangeDefaults[3] = {10.0, 1.0, 1.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

Basis = NULL;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_MANHATTAN_OCTAVES].SetRangeDefaults(OctaveRangeDefaults);

TexType = WCS_TEXTURE_TYPE_F1MANHATTAN;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // F1ManhattanTexture::SetDefaults

/*===========================================================================*/

char F1ManhattanTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_CELLOCTAVES, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // F1ManhattanTexture::GetTextureParamType

/*===========================================================================*/

double F1ManhattanTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
unsigned long Seed[2];
double ClosePt[4];
double Value, Args[1], Point[3];

if ((Args[0] = CellBasisSetup(Input[WCS_TEXTURE_MANHATTAN_OCTAVES][0], LowX, HighX, LowY, HighY, LowZ, HighZ)) > 0.0)
	{
	Value = Input[WCS_TEXTURE_MANHATTAN_CONTRAST][0];
	if (Basis)
		{
		if (! ThreeD)
			{
			if (TexAxis == 0)
				{
				Point[0] = LowY;
				Point[1] = LowZ;
				Point[2] = LowX;
				} // if
			else if (TexAxis == 1)
				{
				Point[0] = LowX;
				Point[1] = LowZ;
				Point[2] = LowY;
				} // else if
			else
				{
				Point[0] = LowX;
				Point[1] = LowY;
				Point[2] = LowZ;
				} // else
			} // if
		else
			{
			Point[0] = LowX;
			Point[1] = LowY;
			Point[2] = LowZ;
			} // switch
		if (Quantize)
			{
			ClosePt[0] = ClosePt[1] = ClosePt[2] = ClosePt[3] = 0.0;
			Basis->Evaluate(Point, Args, 1, ClosePt);
			Seed[0] = (ULONG)(ClosePt[0] * ULONG_MAX);
			Seed[1] = (ULONG)(ClosePt[1] * ULONG_MAX);
			CellularBasis::CellBasisRand.Seed64BitShift(Seed[0], Seed[1]);
			Value *= CellularBasis::CellBasisRand.GenPRN();
			} // if
		else
			Value *= (Basis->Evaluate(Point, Args, 1));
		} // if
	else
		Value = 0.0;
	Value += Input[WCS_TEXTURE_MANHATTAN_BRIGHTNESS][0];
	} // if
else
	Value = Input[WCS_TEXTURE_MANHATTAN_BRIGHTNESS][0] + .5 * Input[WCS_TEXTURE_MANHATTAN_CONTRAST][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // F1ManhattanTexture::Analyze

/*===========================================================================*/

void F1ManhattanTexture::InitBasis(void)
{
double Args[4];

if (Basis)
	delete Basis;
Args[0] = TexParam[WCS_TEXTURE_MANHATTAN_OCTAVES].CurValue;		// octaves
Args[1] = ThreeD ? 3.0: 2.0;	// dimensions
Args[2] = 1.0;	// points needed
Args[3] = 4.0;	// avg points per cell
Basis = new F1Manhattan3(Args, 4);

} // F1ManhattanTexture::InitBasis

/*===========================================================================*/

F2ManhattanTexture::F2ManhattanTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_F2MANHATTAN)
{

SetDefaults(ParamNumber);
InitBasis();

} // F2ManhattanTexture::F2ManhattanTexture

/*===========================================================================*/

F2ManhattanTexture::F2ManhattanTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_F2MANHATTAN, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);
InitBasis();

} // F2ManhattanTexture::F2ManhattanTexture

/*===========================================================================*/

void F2ManhattanTexture::DeleteSpecificResources(void)
{

if (Basis)
	delete Basis;
Basis = NULL;

} // F2ManhattanTexture::DeleteSpecificResources

/*===========================================================================*/

void F2ManhattanTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL, 0,	// 3
	0, 0,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Low (%) ", "High (%) ",			// 1
	"Octaves ", "",	// 3
	"", "",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 1.0, 0.0, 0.0, 0.0, 0.0, 100.0, 0.0, 0.0};
double OctaveRangeDefaults[3] = {10.0, 1.0, 1.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

Basis = NULL;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_MANHATTAN_OCTAVES].SetRangeDefaults(OctaveRangeDefaults);

TexType = WCS_TEXTURE_TYPE_F2MANHATTAN;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // F2ManhattanTexture::SetDefaults

/*===========================================================================*/

char F2ManhattanTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_CELLOCTAVES, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // F2ManhattanTexture::GetTextureParamType

/*===========================================================================*/

double F2ManhattanTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
unsigned long Seed[2];
double ClosePt[4];
double Value, Args[1], Point[3];

if ((Args[0] = CellBasisSetup(Input[WCS_TEXTURE_MANHATTAN_OCTAVES][0], LowX, HighX, LowY, HighY, LowZ, HighZ)) > 0.0)
	{
	Value = Input[WCS_TEXTURE_MANHATTAN_CONTRAST][0];
	if (Basis)
		{
		if (! ThreeD)
			{
			if (TexAxis == 0)
				{
				Point[0] = LowY;
				Point[1] = LowZ;
				Point[2] = LowX;
				} // if
			else if (TexAxis == 1)
				{
				Point[0] = LowX;
				Point[1] = LowZ;
				Point[2] = LowY;
				} // else if
			else
				{
				Point[0] = LowX;
				Point[1] = LowY;
				Point[2] = LowZ;
				} // else
			} // if
		else
			{
			Point[0] = LowX;
			Point[1] = LowY;
			Point[2] = LowZ;
			} // switch
		if (Quantize)
			{
			ClosePt[0] = ClosePt[1] = ClosePt[2] = ClosePt[3] = 0.0;
			Basis->Evaluate(Point, Args, 1, ClosePt);
			Seed[0] = (ULONG)(ClosePt[0] * ULONG_MAX);
			Seed[1] = (ULONG)(ClosePt[1] * ULONG_MAX);
			CellularBasis::CellBasisRand.Seed64BitShift(Seed[0], Seed[1]);
			Value *= CellularBasis::CellBasisRand.GenPRN();
			} // if
		else
			Value *= (Basis->Evaluate(Point, Args, 1));
		} // if
	else
		Value = 0.0;
	Value += Input[WCS_TEXTURE_MANHATTAN_BRIGHTNESS][0];
	} // if
else
	Value = Input[WCS_TEXTURE_MANHATTAN_BRIGHTNESS][0] + .5 * Input[WCS_TEXTURE_MANHATTAN_CONTRAST][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // F2ManhattanTexture::Analyze

/*===========================================================================*/

void F2ManhattanTexture::InitBasis(void)
{
double Args[4];

if (Basis)
	delete Basis;
Args[0] = TexParam[WCS_TEXTURE_MANHATTAN_OCTAVES].CurValue;		// octaves
Args[1] = ThreeD ? 3.0: 2.0;	// dimensions
Args[2] = 2.0;	// points needed
Args[3] = 5.0;	// avg points per cell
Basis = new F2Manhattan3(Args, 4);

} // F2ManhattanTexture::InitBasis

/*===========================================================================*/

F2mF1ManhattanTexture::F2mF1ManhattanTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_F2MF1MANHATTAN)
{

SetDefaults(ParamNumber);
InitBasis();

} // F2mF1ManhattanTexture::F2mF1ManhattanTexture

/*===========================================================================*/

F2mF1ManhattanTexture::F2mF1ManhattanTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_F2MF1MANHATTAN, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);
InitBasis();

} // F2mF1ManhattanTexture::F2mF1ManhattanTexture

/*===========================================================================*/

void F2mF1ManhattanTexture::DeleteSpecificResources(void)
{

if (Basis)
	delete Basis;
Basis = NULL;

} // F2mF1ManhattanTexture::DeleteSpecificResources

/*===========================================================================*/

void F2mF1ManhattanTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL, 0,	// 3
	0, 0,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Low (%) ", "High (%) ",			// 1
	"Octaves ", "",	// 3
	"", "",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 1.0, 0.0, 0.0, 0.0, 0.0, 100.0, 0.0, 0.0};
double OctaveRangeDefaults[3] = {10.0, 1.0, 1.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

Basis = NULL;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_MANHATTAN_OCTAVES].SetRangeDefaults(OctaveRangeDefaults);

TexType = WCS_TEXTURE_TYPE_F2MF1MANHATTAN;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // F2mF1ManhattanTexture::SetDefaults

/*===========================================================================*/

char F2mF1ManhattanTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_CELLOCTAVES, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // F2mF1ManhattanTexture::GetTextureParamType

/*===========================================================================*/

double F2mF1ManhattanTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
unsigned long Seed[2];
double ClosePt[4];
double Value, Args[1], Point[3];

if ((Args[0] = CellBasisSetup(Input[WCS_TEXTURE_MANHATTAN_OCTAVES][0], LowX, HighX, LowY, HighY, LowZ, HighZ)) > 0.0)
	{
	Value = Input[WCS_TEXTURE_MANHATTAN_CONTRAST][0];
	if (Basis)
		{
		if (! ThreeD)
			{
			if (TexAxis == 0)
				{
				Point[0] = LowY;
				Point[1] = LowZ;
				Point[2] = LowX;
				} // if
			else if (TexAxis == 1)
				{
				Point[0] = LowX;
				Point[1] = LowZ;
				Point[2] = LowY;
				} // else if
			else
				{
				Point[0] = LowX;
				Point[1] = LowY;
				Point[2] = LowZ;
				} // else
			} // if
		else
			{
			Point[0] = LowX;
			Point[1] = LowY;
			Point[2] = LowZ;
			} // switch
		if (Quantize)
			{
			ClosePt[0] = ClosePt[1] = ClosePt[2] = ClosePt[3] = 0.0;
			Basis->Evaluate(Point, Args, 1, ClosePt);
			Seed[0] = (ULONG)(ClosePt[0] * ULONG_MAX);
			Seed[1] = (ULONG)(ClosePt[1] * ULONG_MAX);
			CellularBasis::CellBasisRand.Seed64BitShift(Seed[0], Seed[1]);
			Value *= CellularBasis::CellBasisRand.GenPRN();
			} // if
		else
			Value *= (Basis->Evaluate(Point, Args, 1));
		} // if
	else
		Value = 0.0;
	Value += Input[WCS_TEXTURE_MANHATTAN_BRIGHTNESS][0];
	} // if
else
	Value = Input[WCS_TEXTURE_MANHATTAN_BRIGHTNESS][0] + .5 * Input[WCS_TEXTURE_MANHATTAN_CONTRAST][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // F2mF1ManhattanTexture::Analyze

/*===========================================================================*/

void F2mF1ManhattanTexture::InitBasis(void)
{
double Args[4];

if (Basis)
	delete Basis;
Args[0] = TexParam[WCS_TEXTURE_MANHATTAN_OCTAVES].CurValue;		// octaves
Args[1] = ThreeD ? 3.0: 2.0;	// dimensions
Args[2] = 2.0;	// points needed
Args[3] = 5.0;	// avg points per cell
Basis = new F2mF1Manhattan3(Args, 4);

} // F2mF1ManhattanTexture::InitBasis

/*===========================================================================*/

BirdshotTexture::BirdshotTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_BIRDSHOT)
{

SetDefaults(ParamNumber);
InitNoise();
InitBasis();

} // BirdshotTexture::BirdshotTexture

/*===========================================================================*/

BirdshotTexture::BirdshotTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_BIRDSHOT, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);
InitNoise();
InitBasis();

} // BirdshotTexture::BirdshotTexture

/*===========================================================================*/

void BirdshotTexture::DeleteSpecificResources(void)
{

if (Noise)
	delete Noise;
Noise = NULL;
if (Basis)
	delete Basis;
Basis = NULL;

} // BirdshotTexture::DeleteSpecificResources

/*===========================================================================*/

void BirdshotTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 5
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Low (%) ", "High (%) ",			// 1
	"Pebble Size (%) ", "Density (%) ",	// 3
	"Coverage (%) ", "Cell Size (%) ",	// 5
	"Brightness (%) ", "Contrast (%) ",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 50.0, 50.0, 50.0, 20.0, 0.0, 100.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

Noise = NULL;
Basis = NULL;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_BIRDSHOT;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // BirdshotTexture::SetDefaults

/*===========================================================================*/

char BirdshotTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_COVERAGE,
	WCS_TEXTURE_PARAMTYPE_COVERAGE, WCS_TEXTURE_PARAMTYPE_SIZE,
	WCS_TEXTURE_PARAMTYPE_BRIGHTNESS, WCS_TEXTURE_PARAMTYPE_CONTRAST,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // BirdshotTexture::GetTextureParamType

/*===========================================================================*/

double BirdshotTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
unsigned long Seed[2];
double Value, Args[4], InPt[4], OutPt[4], ShotRadius, ShotRadSq, Dist;
NoiseVector Point;
int NumArgs = 0;

if (Tex[WCS_TEXTURE_CELLBASIS_OCTAVES] && Tex[WCS_TEXTURE_CELLBASIS_OCTAVES]->Enabled)
	{
	Args[0] = 1.0;	// octaves
	Args[1] = ThreeD ? 3.0: 2.0;	// dimensions
	Args[2] = 1.0;	// points needed
	Args[3] = TexParam[WCS_TEXTURE_BIRDSHOT_DENSITY].CurValue  * (1.0 / 50.0);	// avg points per cell
	NumArgs = 4;
	} // if
if ((ShotRadius = Input[WCS_TEXTURE_BIRDSHOT_SHOTSIZE][0] * .5) < 0.005)
	ShotRadius = .005;
if (Input[WCS_TEXTURE_BIRDSHOT_CELLSIZE][0] <= 0.01)
	Input[WCS_TEXTURE_BIRDSHOT_CELLSIZE][0] = .01;
ShotRadSq = ShotRadius * ShotRadius;

ReCenterMoveRotate(LowX, HighX, LowY, HighY, LowZ, HighZ);
ReSizeT(LowX, HighX, LowY, HighY, LowZ, HighZ);

Value = Input[WCS_TEXTURE_FRACTALNOISE_CONTRAST][0];
// scale to cell size
LowX /= Input[WCS_TEXTURE_BIRDSHOT_CELLSIZE][0];
LowY /= Input[WCS_TEXTURE_BIRDSHOT_CELLSIZE][0];
LowZ /= Input[WCS_TEXTURE_BIRDSHOT_CELLSIZE][0];

// find closest feature point
// initialize OutPt in case no closest point is found in the search radius
if (! ThreeD)
	{
	if (TexAxis == 0)
		{
		InPt[0] = OutPt[0] = LowY;
		InPt[1] = OutPt[1] = LowZ;
		InPt[2] = OutPt[2] = LowX;
		} // if
	else if (TexAxis == 1)
		{
		InPt[0] = OutPt[0] = LowX;
		InPt[1] = OutPt[1] = LowZ;
		InPt[2] = OutPt[2] = LowY;
		} // else if
	else
		{
		InPt[0] = OutPt[0] = LowX;
		InPt[1] = OutPt[1] = LowY;
		InPt[2] = OutPt[2] = LowZ;
		} // else
	} // if
else
	{
	InPt[0] = LowX;
	InPt[1] = LowY;
	InPt[2] = LowZ;
	} // switch
InPt[3] = OutPt[3] = 0.0;

if ((Dist = Basis->Evaluate(InPt, Args, NumArgs, OutPt)) < ShotRadius)
	{
	// scale closest feature point up from cell size
	OutPt[0] *= Input[WCS_TEXTURE_BIRDSHOT_CELLSIZE][0];
	OutPt[1] *= Input[WCS_TEXTURE_BIRDSHOT_CELLSIZE][0];
	OutPt[2] *= Input[WCS_TEXTURE_BIRDSHOT_CELLSIZE][0];

	// evaluate noise at feature point
	if (Noise)
		{
		Point.x = OutPt[0];
		Point.y = OutPt[1];
		Point.z = OutPt[2];
		if ((Noise->EvaluateNoise(Point, 0, 0) * .5 + .5) <= Input[WCS_TEXTURE_BIRDSHOT_COVERAGE][0])
			{
			if (Quantize)
				{
				Seed[0] = (ULONG)(OutPt[0] * ULONG_MAX);
				Seed[1] = (ULONG)(OutPt[1] * ULONG_MAX);
				CellularBasis::CellBasisRand.Seed64BitShift(Seed[0], Seed[1]);
				Value *= CellularBasis::CellBasisRand.GenPRN();
				} // if
			else
				Value *= (sqrt(ShotRadSq - Dist * Dist) / ShotRadius);
			} // if
		else
			Value = 0.0;
		} // if
	else
		Value = 0.0;
	} // if within shot radius
else
	Value = 0.0;

Value += Input[WCS_TEXTURE_FRACTALNOISE_BRIGHTNESS][0];
if (Value > 1.0)
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // BirdshotTexture::Analyze

/*===========================================================================*/

void BirdshotTexture::InitNoise(void)
{
double Args[3];

if (Noise)
	delete Noise;
Args[2] = .5;	// roughness
Args[1] = 2.0;	// lacunarity
Args[0] = 1.0;	// octaves

Noise = new fBmNoise(Args, 3);

} // BirdshotTexture::InitNoise

/*===========================================================================*/

void BirdshotTexture::InitBasis(void)
{
double Args[4];

if (Basis)
	delete Basis;
Args[0] = 1.0;		// octaves
Args[1] = ThreeD ? 3.0: 2.0;	// dimensions
Args[2] = 1.0;	// points needed
Args[3] = TexParam[WCS_TEXTURE_BIRDSHOT_DENSITY].CurValue  * (1.0 / 25.0);	// avg points per cell
Basis = new F1CellBasis3(Args, 4);

} // BirdshotTexture::InitBasis

/*===========================================================================*/

AddTexture::AddTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_ADD)
{

SetDefaults(ParamNumber);

} // AddTexture::AddTexture

/*===========================================================================*/

AddTexture::AddTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_ADD, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // AddTexture::AddTexture

/*===========================================================================*/

void AddTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	0, 0,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value A (%) ", "Value B (%) ",			// 1
	"", "",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 40.0, 20.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_ADD;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // AddTexture::SetDefaults

/*===========================================================================*/

char AddTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // AddTexture::GetTextureParamType

/*===========================================================================*/

double AddTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{

if (ApplyToColor)
	{
	ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
	ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR2], 1.0, Data, EvalChildren);
	Output[0] = Input[WCS_TEXTURE_COLOR1][0] + Input[WCS_TEXTURE_COLOR2][0];
	Output[1] = Input[WCS_TEXTURE_COLOR1][1] + Input[WCS_TEXTURE_COLOR2][1];
	Output[2] = Input[WCS_TEXTURE_COLOR1][2] + Input[WCS_TEXTURE_COLOR2][2];
	if (Output[0] > 1.0)
		Output[0] = 1.0;
	if (Output[1] > 1.0)
		Output[1] = 1.0;
	if (Output[2] > 1.0)
		Output[2] = 1.0;
	} // if
else
	{
	Output[0] = Input[WCS_TEXTURE_LOW][0] + Input[WCS_TEXTURE_HIGH][0];
	Output[1] = 0.0;
	Output[2] = 0.0;
	if (Output[0] > 1.0)
		Output[0] = 1.0;
	} // else

return (1.0);

} // AddTexture::Analyze

/*===========================================================================*/

SubtractTexture::SubtractTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_SUBTRACT)
{

SetDefaults(ParamNumber);

} // SubtractTexture::SubtractTexture

/*===========================================================================*/

SubtractTexture::SubtractTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_SUBTRACT, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // SubtractTexture::SubtractTexture

/*===========================================================================*/

void SubtractTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	0, 0,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value A (%) ", "Value B (%) ",			// 1
	"", "",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 80.0, 40.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_SUBTRACT;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // SubtractTexture::SetDefaults

/*===========================================================================*/

char SubtractTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // SubtractTexture::GetTextureParamType

/*===========================================================================*/

double SubtractTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{

if (ApplyToColor)
	{
	ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
	ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR2], 1.0, Data, EvalChildren);
	Output[0] = Input[WCS_TEXTURE_COLOR1][0] - Input[WCS_TEXTURE_COLOR2][0];
	Output[1] = Input[WCS_TEXTURE_COLOR1][1] - Input[WCS_TEXTURE_COLOR2][1];
	Output[2] = Input[WCS_TEXTURE_COLOR1][2] - Input[WCS_TEXTURE_COLOR2][2];
	if (Output[0] < 0.0)
		Output[0] = 0.0;
	if (Output[1] < 0.0)
		Output[1] = 0.0;
	if (Output[2] < 0.0)
		Output[2] = 0.0;
	} // if
else
	{
	Output[0] = Input[WCS_TEXTURE_LOW][0] - Input[WCS_TEXTURE_HIGH][0];
	Output[1] = 0.0;
	Output[2] = 0.0;
	if (Output[0] < 0.0)
		Output[0] = 0.0;
	} // else

return (1.0);

} // SubtractTexture::Analyze

/*===========================================================================*/

MultiplyTexture::MultiplyTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_MULTIPLY)
{

SetDefaults(ParamNumber);

} // MultiplyTexture::MultiplyTexture

/*===========================================================================*/

MultiplyTexture::MultiplyTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_MULTIPLY, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // MultiplyTexture::MultiplyTexture

/*===========================================================================*/

void MultiplyTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	0, 0,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value A (%) ", "Value B (%) ",			// 1
	"", "",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 80.0, 40.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_MULTIPLY;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // MultiplyTexture::SetDefaults

/*===========================================================================*/

char MultiplyTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // MultiplyTexture::GetTextureParamType

/*===========================================================================*/

double MultiplyTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{

if (ApplyToColor)
	{
	ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
	ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR2], 1.0, Data, EvalChildren);
	Output[0] = Input[WCS_TEXTURE_COLOR1][0] * Input[WCS_TEXTURE_COLOR2][0];
	Output[1] = Input[WCS_TEXTURE_COLOR1][1] * Input[WCS_TEXTURE_COLOR2][1];
	Output[2] = Input[WCS_TEXTURE_COLOR1][2] * Input[WCS_TEXTURE_COLOR2][2];
	if (Output[0] > 1.0)
		Output[0] = 1.0;
	if (Output[1] > 1.0)
		Output[1] = 1.0;
	if (Output[2] > 1.0)
		Output[2] = 1.0;
	} // if
else
	{
	Output[0] = Input[WCS_TEXTURE_LOW][0] * Input[WCS_TEXTURE_HIGH][0];
	Output[1] = 0.0;
	Output[2] = 0.0;
	if (Output[0] > 1.0)
		Output[0] = 1.0;
	} // else

return (1.0);

} // MultiplyTexture::Analyze

/*===========================================================================*/

CompositeTexture::CompositeTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_COMPOSITE)
{

SetDefaults(ParamNumber);

} // CompositeTexture::CompositeTexture

/*===========================================================================*/

CompositeTexture::CompositeTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_COMPOSITE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // CompositeTexture::CompositeTexture

/*===========================================================================*/

void CompositeTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 12
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value A (%) ", "Value B (%) ",			// 1
	"Mask (%) ", "",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 25.0, 75.0, 75.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_COMPOSITE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // CompositeTexture::SetDefaults

/*===========================================================================*/

char CompositeTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_MASK, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // CompositeTexture::GetTextureParamType

/*===========================================================================*/

double CompositeTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Inverse, Value;

Value = Input[WCS_TEXTURE_COMPOSITE_MASK][0];

// apply Remap Functions to Value
if (EvalChildren)
	EvalStrataBlend(Value, Data);

if (ApplyToColor)
	{
	Inverse = 1.0 - Value;
	ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
	ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR2], 1.0, Data, EvalChildren);
	Output[0] = Input[WCS_TEXTURE_COLOR1][0] * Inverse + Input[WCS_TEXTURE_COLOR2][0] * Value;
	Output[1] = Input[WCS_TEXTURE_COLOR1][1] * Inverse + Input[WCS_TEXTURE_COLOR2][1] * Value;
	Output[2] = Input[WCS_TEXTURE_COLOR1][2] * Inverse + Input[WCS_TEXTURE_COLOR2][2] * Value;
	} // if
else
	{
	Output[0] = Input[WCS_TEXTURE_LOW][0] * (1.0 - Value) + Input[WCS_TEXTURE_HIGH][0]  * Value;
	Output[1] = 0.0;
	Output[2] = 0.0;
	} // else

return (Value);

} // CompositeTexture::Analyze

/*===========================================================================*/

ContrastTexture::ContrastTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_CONTRAST)
{

SetDefaults(ParamNumber);

} // ContrastTexture::ContrastTexture

/*===========================================================================*/

ContrastTexture::ContrastTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_CONTRAST, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // ContrastTexture::ContrastTexture

/*===========================================================================*/

void ContrastTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, 0,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value (%) ", "",			// 1
	"Contrast (%) ", "",	// 3
	"Median (%) ", "",	// 5
	"", "",	// 7
	"", "",		// 9
	"Color ", "",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 50.0, 0.0, 0.0, 75.0, 50.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_CONTRAST;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // ContrastTexture::SetDefaults

/*===========================================================================*/

char ContrastTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, 0,
	WCS_TEXTURE_PARAMTYPE_CONTRAST, 0,
	WCS_TEXTURE_PARAMTYPE_MEDIAN, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, 0,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // ContrastTexture::GetTextureParamType

/*===========================================================================*/

double ContrastTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Contrast;

if (ApplyToColor)
	{
	if (! StrataBlend)
		ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
	if (Input[WCS_TEXTURE_CONTRAST_CONTRAST][0] >= .5)
		{
		Contrast = 2.0 * (Input[WCS_TEXTURE_CONTRAST_CONTRAST][0] - .5);
		if (Input[WCS_TEXTURE_COLOR1][0] >= Input[WCS_TEXTURE_CONTRAST_MEDIAN][0])
			{
			Output[0] = Input[WCS_TEXTURE_COLOR1][0] + (1.0 - Input[WCS_TEXTURE_COLOR1][0]) * Contrast;
			} // if
		else
			{
			Output[0] = Input[WCS_TEXTURE_COLOR1][0] - Input[WCS_TEXTURE_COLOR1][0] * Contrast;
			} // else
		if (Input[WCS_TEXTURE_COLOR1][1] >= Input[WCS_TEXTURE_CONTRAST_MEDIAN][0])
			{
			Output[1] = Input[WCS_TEXTURE_COLOR1][1] + (1.0 - Input[WCS_TEXTURE_COLOR1][1]) * Contrast;
			} // if
		else
			{
			Output[1] = Input[WCS_TEXTURE_COLOR1][1] - Input[WCS_TEXTURE_COLOR1][1] * Contrast;
			} // else
		if (Input[WCS_TEXTURE_COLOR1][2] >= Input[WCS_TEXTURE_CONTRAST_MEDIAN][0])
			{
			Output[2] = Input[WCS_TEXTURE_COLOR1][2] + (1.0 - Input[WCS_TEXTURE_COLOR1][2]) * Contrast;
			} // if
		else
			{
			Output[2] = Input[WCS_TEXTURE_COLOR1][2] - Input[WCS_TEXTURE_COLOR1][2] * Contrast;
			} // else
		} // if
	else
		{
		Contrast = 2.0 * Input[WCS_TEXTURE_CONTRAST_CONTRAST][0];
		if (Input[WCS_TEXTURE_COLOR1][0] >= Input[WCS_TEXTURE_CONTRAST_MEDIAN][0])
			{
			Output[0] = Input[WCS_TEXTURE_CONTRAST_MEDIAN][0] + (Input[WCS_TEXTURE_COLOR1][0] - Input[WCS_TEXTURE_CONTRAST_MEDIAN][0]) * Contrast;
			} // if
		else
			{
			Output[0] = Input[WCS_TEXTURE_CONTRAST_MEDIAN][0] - (Input[WCS_TEXTURE_CONTRAST_MEDIAN][0] - Input[WCS_TEXTURE_COLOR1][0]) * Contrast;
			} // else
		if (Input[WCS_TEXTURE_COLOR1][1] >= Input[WCS_TEXTURE_CONTRAST_MEDIAN][0])
			{
			Output[1] = Input[WCS_TEXTURE_CONTRAST_MEDIAN][0] + (Input[WCS_TEXTURE_COLOR1][1] - Input[WCS_TEXTURE_CONTRAST_MEDIAN][0]) * Contrast;
			} // if
		else
			{
			Output[1] = Input[WCS_TEXTURE_CONTRAST_MEDIAN][0] - (Input[WCS_TEXTURE_CONTRAST_MEDIAN][0] - Input[WCS_TEXTURE_COLOR1][1]) * Contrast;
			} // else
		if (Input[WCS_TEXTURE_COLOR1][2] >= Input[WCS_TEXTURE_CONTRAST_MEDIAN][0])
			{
			Output[2] = Input[WCS_TEXTURE_CONTRAST_MEDIAN][0] + (Input[WCS_TEXTURE_COLOR1][2] - Input[WCS_TEXTURE_CONTRAST_MEDIAN][0]) * Contrast;
			} // if
		else
			{
			Output[2] = Input[WCS_TEXTURE_CONTRAST_MEDIAN][0] - (Input[WCS_TEXTURE_CONTRAST_MEDIAN][0] - Input[WCS_TEXTURE_COLOR1][2]) * Contrast;
			} // else
		} // else
	} // if
else
	{
	if (Input[WCS_TEXTURE_CONTRAST_CONTRAST][0] >= .5)
		{
		Contrast = 2.0 * (Input[WCS_TEXTURE_CONTRAST_CONTRAST][0] - .5);
		if (Input[WCS_TEXTURE_LOW][0] >= Input[WCS_TEXTURE_CONTRAST_MEDIAN][0])
			{
			Output[0] = Input[WCS_TEXTURE_LOW][0] + (1.0 - Input[WCS_TEXTURE_LOW][0]) * Contrast;
			} // if
		else
			{
			Output[0] = Input[WCS_TEXTURE_LOW][0] - Input[WCS_TEXTURE_LOW][0] * Contrast;
			} // else
		} // if
	else
		{
		Contrast = 2.0 * Input[WCS_TEXTURE_CONTRAST_CONTRAST][0];
		if (Input[WCS_TEXTURE_LOW][0] >= Input[WCS_TEXTURE_CONTRAST_MEDIAN][0])
			{
			Output[0] = Input[WCS_TEXTURE_CONTRAST_MEDIAN][0] + (Input[WCS_TEXTURE_LOW][0] - Input[WCS_TEXTURE_CONTRAST_MEDIAN][0]) * Contrast;
			} // if
		else
			{
			Output[0] = Input[WCS_TEXTURE_CONTRAST_MEDIAN][0] - (Input[WCS_TEXTURE_CONTRAST_MEDIAN][0] - Input[WCS_TEXTURE_LOW][0]) * Contrast;
			} // else
		} // else
	Output[1] = 0.0;
	Output[2] = 0.0;
	} // else

return (1.0);

} // ContrastTexture::Analyze

/*===========================================================================*/

DarkenTexture::DarkenTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_DARKEN)
{

SetDefaults(ParamNumber);

} // DarkenTexture::DarkenTexture

/*===========================================================================*/

DarkenTexture::DarkenTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_DARKEN, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // DarkenTexture::DarkenTexture

/*===========================================================================*/

void DarkenTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, 0,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value (%) ", "",			// 1
	"Mask (%) ", "",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",		// 9
	"Color ", "",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 100.0, 0.0, 75.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_DARKEN;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // DarkenTexture::SetDefaults

/*===========================================================================*/

char DarkenTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, 0,
	WCS_TEXTURE_PARAMTYPE_MASK, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, 0,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // DarkenTexture::GetTextureParamType

/*===========================================================================*/

double DarkenTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{

if (ApplyToColor)
	{
	if (! StrataBlend)
		ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
	Output[0] = Input[WCS_TEXTURE_COLOR1][0] * Input[WCS_TEXTURE_DARKEN_MASK][0];
	Output[1] = Input[WCS_TEXTURE_COLOR1][1] * Input[WCS_TEXTURE_DARKEN_MASK][0];
	Output[2] = Input[WCS_TEXTURE_COLOR1][2] * Input[WCS_TEXTURE_DARKEN_MASK][0];
	} // if
else
	{
	Output[0] = Input[WCS_TEXTURE_LOW][0] * Input[WCS_TEXTURE_DARKEN_MASK][0];
	Output[1] = 0.0;
	Output[2] = 0.0;
	} // else

return (1.0);

} // DarkenTexture::Analyze

/*===========================================================================*/

LightenTexture::LightenTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_LIGHTEN)
{

SetDefaults(ParamNumber);

} // LightenTexture::LightenTexture

/*===========================================================================*/

LightenTexture::LightenTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_LIGHTEN, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // LightenTexture::LightenTexture

/*===========================================================================*/

void LightenTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, 0,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value (%) ", "",			// 1
	"Mask (%) ", "",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",		// 9
	"Color ", "",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 0.0, 25.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_LIGHTEN;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // LightenTexture::SetDefaults

/*===========================================================================*/

char LightenTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, 0,
	WCS_TEXTURE_PARAMTYPE_MASK, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, 0,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // LightenTexture::GetTextureParamType

/*===========================================================================*/

double LightenTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{

if (ApplyToColor)
	{
	if (! StrataBlend)
		ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
	Output[0] = Input[WCS_TEXTURE_COLOR1][0] + (1.0 - Input[WCS_TEXTURE_COLOR1][0]) * Input[WCS_TEXTURE_LIGHTEN_MASK][0];
	Output[1] = Input[WCS_TEXTURE_COLOR1][1] + (1.0 - Input[WCS_TEXTURE_COLOR1][1]) * Input[WCS_TEXTURE_LIGHTEN_MASK][0];
	Output[2] = Input[WCS_TEXTURE_COLOR1][2] + (1.0 - Input[WCS_TEXTURE_COLOR1][2]) * Input[WCS_TEXTURE_LIGHTEN_MASK][0];
	} // if
else
	{
	Output[0] = Input[WCS_TEXTURE_LOW][0] + (1.0 - Input[WCS_TEXTURE_LOW][0]) * Input[WCS_TEXTURE_LIGHTEN_MASK][0];
	Output[1] = 0.0;
	Output[2] = 0.0;
	} // else

return (1.0);

} // LightenTexture::Analyze

/*===========================================================================*/

LevelsTexture::LevelsTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_LEVELS)
{

SetDefaults(ParamNumber);

} // LevelsTexture::LevelsTexture

/*===========================================================================*/

LevelsTexture::LevelsTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_LEVELS, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // LevelsTexture::LevelsTexture

/*===========================================================================*/

void LevelsTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, 0,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value (%) ", "",			// 1
	"Low (%) ", "High (%) ",	// 3
	"Mid (%) ", "",	// 5
	"", "",	// 7
	"", "",		// 9
	"Color ", "",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 0.0, 0.0, 100.0, 50.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_LEVELS;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // LevelsTexture::SetDefaults

/*===========================================================================*/

char LevelsTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_MEDIAN, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, 0,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // LevelsTexture::GetTextureParamType

/*===========================================================================*/

double LevelsTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double OldMid;

if (ApplyToColor)
	{
	if (! StrataBlend)
		ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);

	if (Input[WCS_TEXTURE_COLOR1][0] <= Input[WCS_TEXTURE_LEVELS_LOW][0])
		Output[0] = 0.0;
	else if (Input[WCS_TEXTURE_COLOR1][0] >= Input[WCS_TEXTURE_LEVELS_HIGH][0])
		Output[0] = 1.0;
	else
		{
		OldMid = Input[WCS_TEXTURE_LEVELS_LOW][0] + (Input[WCS_TEXTURE_LEVELS_HIGH][0] - Input[WCS_TEXTURE_LEVELS_LOW][0]) * Input[WCS_TEXTURE_LEVELS_MID][0];
		if (Input[WCS_TEXTURE_COLOR1][0] > OldMid)
			Output[0] = .5 + .5 * (Input[WCS_TEXTURE_COLOR1][0] - OldMid) / (Input[WCS_TEXTURE_LEVELS_HIGH][0] - OldMid);
		else if (Input[WCS_TEXTURE_COLOR1][0] < OldMid)
			Output[0] = .5 - .5 * (OldMid - Input[WCS_TEXTURE_COLOR1][0]) / (OldMid - Input[WCS_TEXTURE_LEVELS_LOW][0]);
		else
			Output[0] = .5;
		} // else
	if (Input[WCS_TEXTURE_COLOR1][1] <= Input[WCS_TEXTURE_LEVELS_LOW][0])
		Output[1] = 0.0;
	else if (Input[WCS_TEXTURE_COLOR1][1] >= Input[WCS_TEXTURE_LEVELS_HIGH][0])
		Output[1] = 1.0;
	else
		{
		OldMid = Input[WCS_TEXTURE_LEVELS_LOW][0] + (Input[WCS_TEXTURE_LEVELS_HIGH][0] - Input[WCS_TEXTURE_LEVELS_LOW][0]) * Input[WCS_TEXTURE_LEVELS_MID][0];
		if (Input[WCS_TEXTURE_COLOR1][1] > OldMid)
			Output[1] = .5 + .5 * (Input[WCS_TEXTURE_COLOR1][1] - OldMid) / (Input[WCS_TEXTURE_LEVELS_HIGH][0] - OldMid);
		else if (Input[WCS_TEXTURE_COLOR1][1] < OldMid)
			Output[1] = .5 - .5 * (OldMid - Input[WCS_TEXTURE_COLOR1][1]) / (OldMid - Input[WCS_TEXTURE_LEVELS_LOW][0]);
		else
			Output[1] = .5;
		} // else
	if (Input[WCS_TEXTURE_COLOR1][2] <= Input[WCS_TEXTURE_LEVELS_LOW][0])
		Output[2] = 0.0;
	else if (Input[WCS_TEXTURE_COLOR1][2] >= Input[WCS_TEXTURE_LEVELS_HIGH][0])
		Output[2] = 1.0;
	else
		{
		OldMid = Input[WCS_TEXTURE_LEVELS_LOW][0] + (Input[WCS_TEXTURE_LEVELS_HIGH][0] - Input[WCS_TEXTURE_LEVELS_LOW][0]) * Input[WCS_TEXTURE_LEVELS_MID][0];
		if (Input[WCS_TEXTURE_COLOR1][2] > OldMid)
			Output[2] = .5 + .5 * (Input[WCS_TEXTURE_COLOR1][2] - OldMid) / (Input[WCS_TEXTURE_LEVELS_HIGH][0] - OldMid);
		else if (Input[WCS_TEXTURE_COLOR1][2] < OldMid)
			Output[2] = .5 - .5 * (OldMid - Input[WCS_TEXTURE_COLOR1][2]) / (OldMid - Input[WCS_TEXTURE_LEVELS_LOW][0]);
		else
			Output[2] = .5;
		} // else
	} // if
else
	{
	if (Input[WCS_TEXTURE_LOW][0] <= Input[WCS_TEXTURE_LEVELS_LOW][0])
		Output[0] = 0.0;
	else if (Input[WCS_TEXTURE_LOW][0] >= Input[WCS_TEXTURE_LEVELS_HIGH][0])
		Output[0] = 1.0;
	else
		{
		OldMid = Input[WCS_TEXTURE_LEVELS_LOW][0] + (Input[WCS_TEXTURE_LEVELS_HIGH][0] - Input[WCS_TEXTURE_LEVELS_LOW][0]) * Input[WCS_TEXTURE_LEVELS_MID][0];
		if (Input[WCS_TEXTURE_LOW][0] > OldMid)
			Output[0] = .5 + .5 * (Input[WCS_TEXTURE_LOW][0] - OldMid) / (Input[WCS_TEXTURE_LEVELS_HIGH][0] - OldMid);
		else if (Input[WCS_TEXTURE_LOW][0] < OldMid)
			Output[0] = .5 - .5 * (OldMid - Input[WCS_TEXTURE_LOW][0]) / (OldMid - Input[WCS_TEXTURE_LEVELS_LOW][0]);
		else
			Output[0] = .5;
		} // else
	Output[1] = 0.0;
	Output[2] = 0.0;
	} // else

return (1.0);

} // LevelsTexture::Analyze

/*===========================================================================*/

HSVMergeTexture::HSVMergeTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_HSVMERGE)
{

SetDefaults(ParamNumber);

} // HSVMergeTexture::HSVMergeTexture

/*===========================================================================*/

HSVMergeTexture::HSVMergeTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_HSVMERGE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // HSVMergeTexture::HSVMergeTexture

/*===========================================================================*/

void HSVMergeTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	0, 0,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value (%) ", "",			// 1
	"Hue (%) ", "Saturation (%) ",	// 3
	"Value (%) ", "",	// 5
	"", "",	// 7
	"", "",		// 9
	"", "",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 0.0, 0.0, 50.0, 50.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_HSVMERGE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // HSVMergeTexture::SetDefaults

/*===========================================================================*/

char HSVMergeTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_SAMPLE, WCS_TEXTURE_PARAMTYPE_SAMPLE,
	WCS_TEXTURE_PARAMTYPE_SAMPLE, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, 0,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // HSVMergeTexture::GetTextureParamType

/*===========================================================================*/

double HSVMergeTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double RGB[3], HSV[3], Hue, Sat;

// hue
if (Tex[WCS_TEXTURE_HSVMERGE_HUE] && Tex[WCS_TEXTURE_HSVMERGE_HUE]->Enabled)
	{
	RGB[0] = Input[WCS_TEXTURE_HSVMERGE_HUE][0];
	RGB[1] = Input[WCS_TEXTURE_HSVMERGE_HUE][1];
	RGB[2] = Input[WCS_TEXTURE_HSVMERGE_HUE][2];
	RGBtoHSV(HSV, RGB);
	Hue = HSV[0];
	} // if
else
	Hue = Input[WCS_TEXTURE_HSVMERGE_HUE][0] * 360.0;	// expressed in percent, expand by * 360

// saturation
if (Tex[WCS_TEXTURE_HSVMERGE_SAT] && Tex[WCS_TEXTURE_HSVMERGE_SAT]->Enabled)
	{
	RGB[0] = Input[WCS_TEXTURE_HSVMERGE_SAT][0];
	RGB[1] = Input[WCS_TEXTURE_HSVMERGE_SAT][1];
	RGB[2] = Input[WCS_TEXTURE_HSVMERGE_SAT][2];
	RGBtoHSV(HSV, RGB);
	Sat = HSV[1];
	} // if
else
	Sat = Input[WCS_TEXTURE_HSVMERGE_SAT][0] * 100.0;	// expressed in percent

// value
if (Tex[WCS_TEXTURE_HSVMERGE_VAL] && Tex[WCS_TEXTURE_HSVMERGE_VAL]->Enabled)
	{
	RGB[0] = Input[WCS_TEXTURE_HSVMERGE_VAL][0];
	RGB[1] = Input[WCS_TEXTURE_HSVMERGE_VAL][1];
	RGB[2] = Input[WCS_TEXTURE_HSVMERGE_VAL][2];
	RGBtoHSV(HSV, RGB);
	} // if
else
	HSV[2] = Input[WCS_TEXTURE_HSVMERGE_VAL][0] * 100.0;	// expressed in percent

HSV[0] = Hue;
HSV[1] = Sat;

HSVtoRGB(HSV, Output);

if (! ApplyToColor)
	{
	Output[1] = 0.0;
	Output[2] = 0.0;
	} // else

return (1.0);

} // HSVMergeTexture::Analyze

/*===========================================================================*/

SkewTexture::SkewTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_SKEW)
{

SetDefaults(ParamNumber);

} // SkewTexture::SkewTexture

/*===========================================================================*/

SkewTexture::SkewTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_SKEW, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // SkewTexture::SkewTexture

/*===========================================================================*/

void SkewTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 5
	0, 0,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, 0,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value (%) ", "",			// 1
	"Mask (%) ", "",	// 3
	"Median In (%) ", "Median Out (%) ",	// 5
	"", "",	// 7
	"", "",		// 9
	"Color ", "",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 50.0, 0.0, 50.0, 0.0, 50.0, 25.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_SKEW;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // SkewTexture::SetDefaults

/*===========================================================================*/

char SkewTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, 0,
	WCS_TEXTURE_PARAMTYPE_MASK, 0,
	WCS_TEXTURE_PARAMTYPE_MEDIAN, WCS_TEXTURE_PARAMTYPE_MEDIAN,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, 0,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // SkewTexture::GetTextureParamType

/*===========================================================================*/

double SkewTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Offset;

if (ApplyToColor)
	{
	if (! StrataBlend)
		ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
	if (Input[WCS_TEXTURE_COLOR1][0] != 0.0 && Input[WCS_TEXTURE_COLOR1][0] != 1.0)
		{
		if (Input[WCS_TEXTURE_COLOR1][0] >= Input[WCS_TEXTURE_SKEW_MEDIANIN][0])
			{
			Offset = (Input[WCS_TEXTURE_COLOR1][0] - Input[WCS_TEXTURE_SKEW_MEDIANIN][0]) / (1.0 - Input[WCS_TEXTURE_SKEW_MEDIANIN][0]);
			Output[0] = Input[WCS_TEXTURE_SKEW_MEDIANOUT][0] + (1.0 - Input[WCS_TEXTURE_SKEW_MEDIANOUT][0]) * Offset;
			Output[0] = Output[0] * Input[WCS_TEXTURE_SKEW_MASK][0] + Input[WCS_TEXTURE_COLOR1][0] * (1.0 - Input[WCS_TEXTURE_SKEW_MASK][0]);
			} // if
		else
			{
			Offset = (Input[WCS_TEXTURE_SKEW_MEDIANIN][0] - Input[WCS_TEXTURE_COLOR1][0]) / (Input[WCS_TEXTURE_SKEW_MEDIANIN][0]);
			Output[0] = Input[WCS_TEXTURE_SKEW_MEDIANOUT][0] - Input[WCS_TEXTURE_SKEW_MEDIANOUT][0] * Offset;
			Output[0] = Output[0] * Input[WCS_TEXTURE_SKEW_MASK][0] + Input[WCS_TEXTURE_COLOR1][0] * (1.0 - Input[WCS_TEXTURE_SKEW_MASK][0]);
			} // else
		} // if
	else
		Output[0] = Input[WCS_TEXTURE_COLOR1][0];
	if (Input[WCS_TEXTURE_COLOR1][1] != 0.0 && Input[WCS_TEXTURE_COLOR1][1] != 1.0)
		{
		if (Input[WCS_TEXTURE_COLOR1][1] >= Input[WCS_TEXTURE_SKEW_MEDIANIN][0])
			{
			Offset = (Input[WCS_TEXTURE_COLOR1][1] - Input[WCS_TEXTURE_SKEW_MEDIANIN][0]) / (1.0 - Input[WCS_TEXTURE_SKEW_MEDIANIN][0]);
			Output[1] = Input[WCS_TEXTURE_SKEW_MEDIANOUT][0] + (1.0 - Input[WCS_TEXTURE_SKEW_MEDIANOUT][0]) * Offset;
			Output[1] = Output[1] * Input[WCS_TEXTURE_SKEW_MASK][0] + Input[WCS_TEXTURE_COLOR1][1] * (1.0 - Input[WCS_TEXTURE_SKEW_MASK][0]);
			} // if
		else
			{
			Offset = (Input[WCS_TEXTURE_SKEW_MEDIANIN][0] - Input[WCS_TEXTURE_COLOR1][1]) / (Input[WCS_TEXTURE_SKEW_MEDIANIN][0]);
			Output[1] = Input[WCS_TEXTURE_SKEW_MEDIANOUT][0] - Input[WCS_TEXTURE_SKEW_MEDIANOUT][0] * Offset;
			Output[1] = Output[1] * Input[WCS_TEXTURE_SKEW_MASK][0] + Input[WCS_TEXTURE_COLOR1][1] * (1.0 - Input[WCS_TEXTURE_SKEW_MASK][0]);
			} // else
		} // if
	else
		Output[1] = Input[WCS_TEXTURE_COLOR1][1];
	if (Input[WCS_TEXTURE_COLOR1][2] != 0.0 && Input[WCS_TEXTURE_COLOR1][2] != 1.0)
		{
		if (Input[WCS_TEXTURE_COLOR1][2] >= Input[WCS_TEXTURE_SKEW_MEDIANIN][0])
			{
			Offset = (Input[WCS_TEXTURE_COLOR1][2] - Input[WCS_TEXTURE_SKEW_MEDIANIN][0]) / (1.0 - Input[WCS_TEXTURE_SKEW_MEDIANIN][0]);
			Output[2] = Input[WCS_TEXTURE_SKEW_MEDIANOUT][0] + (1.0 - Input[WCS_TEXTURE_SKEW_MEDIANOUT][0]) * Offset;
			Output[2] = Output[2] * Input[WCS_TEXTURE_SKEW_MASK][0] + Input[WCS_TEXTURE_COLOR1][2] * (1.0 - Input[WCS_TEXTURE_SKEW_MASK][0]);
			} // if
		else
			{
			Offset = (Input[WCS_TEXTURE_SKEW_MEDIANIN][0] - Input[WCS_TEXTURE_COLOR1][2]) / (Input[WCS_TEXTURE_SKEW_MEDIANIN][0]);
			Output[2] = Input[WCS_TEXTURE_SKEW_MEDIANOUT][0] - Input[WCS_TEXTURE_SKEW_MEDIANOUT][0] * Offset;
			Output[2] = Output[2] * Input[WCS_TEXTURE_SKEW_MASK][0] + Input[WCS_TEXTURE_COLOR1][2] * (1.0 - Input[WCS_TEXTURE_SKEW_MASK][0]);
			} // else
		} // if
	else
		Output[2] = Input[WCS_TEXTURE_COLOR1][2];
	} // if
else
	{
	if (Input[WCS_TEXTURE_LOW][0] != 0.0 && Input[WCS_TEXTURE_LOW][0] != 1.0)
		{
		if (Input[WCS_TEXTURE_LOW][0] >= Input[WCS_TEXTURE_SKEW_MEDIANIN][0])
			{
			Offset = (Input[WCS_TEXTURE_LOW][0] - Input[WCS_TEXTURE_SKEW_MEDIANIN][0]) / (1.0 - Input[WCS_TEXTURE_SKEW_MEDIANIN][0]);
			Output[0] = Input[WCS_TEXTURE_SKEW_MEDIANOUT][0] + (1.0 - Input[WCS_TEXTURE_SKEW_MEDIANOUT][0]) * Offset;
			Output[0] = Output[0] * Input[WCS_TEXTURE_SKEW_MASK][0] + Input[WCS_TEXTURE_LOW][0] * (1.0 - Input[WCS_TEXTURE_SKEW_MASK][0]);
			} // if
		else
			{
			Offset = (Input[WCS_TEXTURE_SKEW_MEDIANIN][0] - Input[WCS_TEXTURE_LOW][0]) / (Input[WCS_TEXTURE_SKEW_MEDIANIN][0]);
			Output[0] = Input[WCS_TEXTURE_SKEW_MEDIANOUT][0] - Input[WCS_TEXTURE_SKEW_MEDIANOUT][0] * Offset;
			Output[0] = Output[0] * Input[WCS_TEXTURE_SKEW_MASK][0] + Input[WCS_TEXTURE_LOW][0] * (1.0 - Input[WCS_TEXTURE_SKEW_MASK][0]);
			} // else
		} // if
	else
		Output[0] = Input[WCS_TEXTURE_LOW][0];
	Output[1] = 0.0;
	Output[2] = 0.0;
	} // else

return (1.0);

} // SkewTexture::Analyze

/*===========================================================================*/

BellCurveTexture::BellCurveTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_BELLCURVE)
{

SetDefaults(ParamNumber);

} // BellCurveTexture::BellCurveTexture

/*===========================================================================*/

BellCurveTexture::BellCurveTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_BELLCURVE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // BellCurveTexture::BellCurveTexture

/*===========================================================================*/

void BellCurveTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 5
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, 0,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value (%) ", "Repeat ",			// 1
	"Amplitude (%) ", "Shift (%) ",	// 3
	"Frequency ", "Phase (%) ",	// 5
	"Phase Scale ", "Skew (%) ",	// 7
	"Lower Clamp (%) ", "Upper Clamp (%) ",		// 9
	"Color ", "",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 50.0, 0.0, 50.0, 50.0, 1.0, 0.0, 1.0, 50.0, 0.0, 100.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_BELLCURVE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // BellCurveTexture::SetDefaults

/*===========================================================================*/

char BellCurveTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_REPEATFLAG,
	WCS_TEXTURE_PARAMTYPE_AMPLITUDE, WCS_TEXTURE_PARAMTYPE_SHIFT,
	WCS_TEXTURE_PARAMTYPE_FREQUENCIES, WCS_TEXTURE_PARAMTYPE_PHASE,
	WCS_TEXTURE_PARAMTYPE_PHASESCALE, WCS_TEXTURE_PARAMTYPE_SKEW,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, 0,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // BellCurveTexture::GetTextureParamType

/*===========================================================================*/

double BellCurveTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double L, Tx, Px;
long W, Ct;

if (Input[WCS_TEXTURE_BELLCURVE_FREQUENCY][0] > 0.0)
	{
	L = 1.0 / Input[WCS_TEXTURE_BELLCURVE_FREQUENCY][0];
	Tx = L * Input[WCS_TEXTURE_BELLCURVE_PHASE][0] * Input[WCS_TEXTURE_BELLCURVE_PHASESCALE][0];

	if (ApplyToColor)
		{
		if (! StrataBlend)
			ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
		for (Ct = 0; Ct < 3; Ct ++)
			{
			Px = (Input[WCS_TEXTURE_COLOR1][Ct] - Tx) / L;
			W = quicklongfloor(Px);

			if (abs(W) <= (int)Input[WCS_TEXTURE_BELLCURVE_REPEAT][0])
				{
				Px -= W;
				if (Input[WCS_TEXTURE_BELLCURVE_SKEW][0] == .5)
					{
					Output[Ct] = (-cos(Px * TwoPi));
					} // if
				else if (Px == Input[WCS_TEXTURE_BELLCURVE_SKEW][0])
					{
					Output[Ct] = 1.0;
					} // else if
				else if (Px < Input[WCS_TEXTURE_BELLCURVE_SKEW][0])
					{
					Output[Ct] = (-cos(((.5 * Px) / Input[WCS_TEXTURE_BELLCURVE_SKEW][0]) * TwoPi));
					} // else if
				else
					{
					Output[Ct] = (-cos((1.0 - ((1.0 - Px) * .5) / (1.0 - Input[WCS_TEXTURE_BELLCURVE_SKEW][0])) * TwoPi));
					} // else
				} // if
			else
				{
				Output[Ct] = -1.0;
				} // else
			Output[Ct] *= Input[WCS_TEXTURE_BELLCURVE_AMPLITUDE][0];
			Output[Ct] += Input[WCS_TEXTURE_BELLCURVE_SHIFT][0];
			if (Output[Ct] > Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0])
				Output[Ct] = Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0];
			else if (Output[Ct] < Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0])
				Output[Ct] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
			} // for
		} // if
	else
		{
		Px = (Input[WCS_TEXTURE_LOW][0] - Tx) / L;
		W = quicklongfloor(Px);

		if (abs(W) <= (int)Input[WCS_TEXTURE_BELLCURVE_REPEAT][0])
			{
			Px -= W;
			if (Input[WCS_TEXTURE_BELLCURVE_SKEW][0] == .5)
				{
				Output[0] = (-cos(Px * TwoPi));
				} // if
			else if (Px == Input[WCS_TEXTURE_BELLCURVE_SKEW][0])
				{
				Output[0] = 1.0;
				} // else if
			else if (Px < Input[WCS_TEXTURE_BELLCURVE_SKEW][0])
				{
				Output[0] = -cos(((.5 * Px) / Input[WCS_TEXTURE_BELLCURVE_SKEW][0]) * TwoPi);
				} // else if
			else
				{
				Output[0] = -cos((1.0 - ((1.0 - Px) * .5) / (1.0 - Input[WCS_TEXTURE_BELLCURVE_SKEW][0])) * TwoPi);
				} // else
			} // if
		else
			{
			Output[0] = -1.0;
			} // else
		Output[0] *= Input[WCS_TEXTURE_BELLCURVE_AMPLITUDE][0];
		Output[0] += Input[WCS_TEXTURE_BELLCURVE_SHIFT][0];
		if (Output[0] > Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0])
			Output[0] = Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0];
		else if (Output[0] < Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0])
			Output[0] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
		Output[1] = Output[2] = 0.0;
		} // else
	} // if
else
	{
	Output[0] = Output[1] = Output[2] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
	} // else

return (1.0);

} // BellCurveTexture::Analyze

/*===========================================================================*/

SquareWaveTexture::SquareWaveTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_SQUAREWAVE)
{

SetDefaults(ParamNumber);

} // SquareWaveTexture::SquareWaveTexture

/*===========================================================================*/

SquareWaveTexture::SquareWaveTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_SQUAREWAVE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // SquareWaveTexture::SquareWaveTexture

/*===========================================================================*/

void SquareWaveTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 5
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, 0,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value (%) ", "Repeat ",			// 1
	"Amplitude (%) ", "Shift (%) ",	// 3
	"Frequency ", "Phase (%) ",	// 5
	"Phase Scale ", "Width (%) ",	// 7
	"Lower Clamp (%) ", "Upper Clamp (%) ",		// 9
	"Color ", "",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 50.0, 0.0, 50.0, 50.0, 1.0, 0.0, 1.0, 50.0, 0.0, 100.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_SQUAREWAVE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // SquareWaveTexture::SetDefaults

/*===========================================================================*/

char SquareWaveTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_REPEATFLAG,
	WCS_TEXTURE_PARAMTYPE_AMPLITUDE, WCS_TEXTURE_PARAMTYPE_SHIFT,
	WCS_TEXTURE_PARAMTYPE_FREQUENCIES, WCS_TEXTURE_PARAMTYPE_PHASE,
	WCS_TEXTURE_PARAMTYPE_PHASESCALE, WCS_TEXTURE_PARAMTYPE_WIDTH,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, 0,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // SquareWaveTexture::GetTextureParamType

/*===========================================================================*/

double SquareWaveTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double L, Tx, Px;
long W, Ct;

if (Input[WCS_TEXTURE_BELLCURVE_FREQUENCY][0] > 0.0)
	{
	L = 1.0 / Input[WCS_TEXTURE_BELLCURVE_FREQUENCY][0];
	Tx = L * Input[WCS_TEXTURE_BELLCURVE_PHASE][0] * Input[WCS_TEXTURE_BELLCURVE_PHASESCALE][0];

	if (ApplyToColor)
		{
		if (! StrataBlend)
			ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
		for (Ct = 0; Ct < 3; Ct ++)
			{
			Px = (Input[WCS_TEXTURE_COLOR1][Ct] - Tx) / L;
			W = quicklongfloor(Px);

			if (abs(W) <= (int)Input[WCS_TEXTURE_BELLCURVE_REPEAT][0])
				{
				Px -= W;
				if (Px <= .5 - Input[WCS_TEXTURE_SQUAREWAVE_WIDTH][0] * .5 || Px > .5 + Input[WCS_TEXTURE_SQUAREWAVE_WIDTH][0] * .5)
					{
					Output[Ct] = -1.0;
					} // if
				else
					{
					Output[Ct] = 1.0;
					} // else if
				} // if
			else
				{
				Output[Ct] = -1.0;
				} // else
			Output[Ct] *= Input[WCS_TEXTURE_BELLCURVE_AMPLITUDE][0];
			Output[Ct] += Input[WCS_TEXTURE_BELLCURVE_SHIFT][0];
			if (Output[Ct] > Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0])
				Output[Ct] = Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0];
			else if (Output[Ct] < Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0])
				Output[Ct] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
			} // for
		} // if
	else
		{
		Px = (Input[WCS_TEXTURE_LOW][0] - Tx) / L;
		W = quicklongfloor(Px);

		if (abs(W) <= (int)Input[WCS_TEXTURE_BELLCURVE_REPEAT][0])
			{
			Px -= W;
			if (Px <= .5 - Input[WCS_TEXTURE_SQUAREWAVE_WIDTH][0] * .5 || Px > .5 + Input[WCS_TEXTURE_SQUAREWAVE_WIDTH][0] * .5)
				{
				Output[0] = -1.0;
				} // if
			else
				{
				Output[0] = 1.0;
				} // else if
			} // if
		else
			{
			Output[0] = -1.0;
			} // else
		Output[0] *= Input[WCS_TEXTURE_BELLCURVE_AMPLITUDE][0];
		Output[0] += Input[WCS_TEXTURE_BELLCURVE_SHIFT][0];
		if (Output[0] > Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0])
			Output[0] = Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0];
		else if (Output[0] < Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0])
			Output[0] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
		Output[1] = Output[2] = 0.0;
		} // else
	} // if
else
	{
	Output[0] = Output[1] = Output[2] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
	} // else

return (1.0);

} // SquareWaveTexture::Analyze

/*===========================================================================*/

SawtoothTexture::SawtoothTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_SAWTOOTH)
{

SetDefaults(ParamNumber);

} // SawtoothTexture::SawtoothTexture

/*===========================================================================*/

SawtoothTexture::SawtoothTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_SAWTOOTH, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // SawtoothTexture::SawtoothTexture

/*===========================================================================*/

void SawtoothTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 5
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, 0,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value (%) ", "Repeat ",			// 1
	"Amplitude (%) ", "Shift (%) ",	// 3
	"Frequency ", "Phase (%) ",	// 5
	"Phase Scale ", "Skew (%) ",	// 7
	"Lower Clamp (%) ", "Upper Clamp (%) ",		// 9
	"Color ", "",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 50.0, 0.0, 50.0, 50.0, 1.0, 0.0, 1.0, 50.0, 0.0, 100.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_SAWTOOTH;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // SawtoothTexture::SetDefaults

/*===========================================================================*/

char SawtoothTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_REPEATFLAG,
	WCS_TEXTURE_PARAMTYPE_AMPLITUDE, WCS_TEXTURE_PARAMTYPE_SHIFT,
	WCS_TEXTURE_PARAMTYPE_FREQUENCIES, WCS_TEXTURE_PARAMTYPE_PHASE,
	WCS_TEXTURE_PARAMTYPE_PHASESCALE, WCS_TEXTURE_PARAMTYPE_SKEW,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, 0,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // SawtoothTexture::GetTextureParamType

/*===========================================================================*/

double SawtoothTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double L, Tx, Px;
long W, Ct;

if (Input[WCS_TEXTURE_BELLCURVE_FREQUENCY][0] > 0.0)
	{
	L = 1.0 / Input[WCS_TEXTURE_BELLCURVE_FREQUENCY][0];
	Tx = L * Input[WCS_TEXTURE_BELLCURVE_PHASE][0] * Input[WCS_TEXTURE_BELLCURVE_PHASESCALE][0];

	if (ApplyToColor)
		{
		if (! StrataBlend)
			ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
		for (Ct = 0; Ct < 3; Ct ++)
			{
			Px = (Input[WCS_TEXTURE_COLOR1][Ct] - Tx) / L;
			W = quicklongfloor(Px);

			if (abs(W) <= (int)Input[WCS_TEXTURE_BELLCURVE_REPEAT][0])
				{
				Px -= W;
				if (Px == Input[WCS_TEXTURE_BELLCURVE_SKEW][0])
					{
					Output[Ct] = 1.0;
					} // else if
				else if (Px < Input[WCS_TEXTURE_BELLCURVE_SKEW][0])
					{
					Output[Ct] = -1.0 + 2.0 * Px / Input[WCS_TEXTURE_BELLCURVE_SKEW][0];
					} // else if
				else
					{
					Output[Ct] = -1.0 + 2.0 * (1.0 - Px) / (1.0 - Input[WCS_TEXTURE_BELLCURVE_SKEW][0]);
					} // else
				} // if
			else
				{
				Output[Ct] = -1.0;
				} // else
			Output[Ct] *= Input[WCS_TEXTURE_BELLCURVE_AMPLITUDE][0];
			Output[Ct] += Input[WCS_TEXTURE_BELLCURVE_SHIFT][0];
			if (Output[Ct] > Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0])
				Output[Ct] = Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0];
			else if (Output[Ct] < Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0])
				Output[Ct] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
			} // for
		} // if
	else
		{
		Px = (Input[WCS_TEXTURE_LOW][0] - Tx) / L;
		W = quicklongfloor(Px);

		if (abs(W) <= (int)Input[WCS_TEXTURE_BELLCURVE_REPEAT][0])
			{
			Px -= W;
			if (Px == Input[WCS_TEXTURE_BELLCURVE_SKEW][0])
				{
				Output[0] = 1.0;
				} // else if
			else if (Px < Input[WCS_TEXTURE_BELLCURVE_SKEW][0])
				{
				Output[0] = -1.0 + 2.0 * Px / Input[WCS_TEXTURE_BELLCURVE_SKEW][0];
				} // else if
			else
				{
				Output[0] = -1.0 + 2.0 * (1.0 - Px) / (1.0 - Input[WCS_TEXTURE_BELLCURVE_SKEW][0]);
				} // else
			} // if
		else
			{
			Output[0] = -1.0;
			} // else
		Output[0] *= Input[WCS_TEXTURE_BELLCURVE_AMPLITUDE][0];
		Output[0] += Input[WCS_TEXTURE_BELLCURVE_SHIFT][0];
		if (Output[0] > Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0])
			Output[0] = Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0];
		else if (Output[0] < Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0])
			Output[0] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
		Output[1] = Output[2] = 0.0;
		} // else
	} // if
else
	{
	Output[0] = Output[1] = Output[2] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
	} // else

return (1.0);

} // SawtoothTexture::Analyze

/*===========================================================================*/

StepTexture::StepTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_STEP)
{

SetDefaults(ParamNumber);

} // StepTexture::StepTexture

/*===========================================================================*/

StepTexture::StepTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_STEP, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // StepTexture::StepTexture

/*===========================================================================*/

void StepTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 5
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 7
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, 0,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value (%) ", "",			// 1
	"Amplitude (%) ", "Shift (%) ",	// 3
	"Steps ", "Phase (%) ",	// 5
	"Phase Scale ", "Skew (%) ",	// 7
	"Lower Clamp (%) ", "Upper Clamp (%) ",		// 9
	"Color ", "",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 50.0, 0.0, 50.0, 50.0, 1.0, 0.0, 1.0, 50.0, 0.0, 100.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_STEP;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // StepTexture::SetDefaults

/*===========================================================================*/

char StepTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, 0,
	WCS_TEXTURE_PARAMTYPE_AMPLITUDE, WCS_TEXTURE_PARAMTYPE_SHIFT,
	WCS_TEXTURE_PARAMTYPE_FREQUENCIES, WCS_TEXTURE_PARAMTYPE_PHASE,
	WCS_TEXTURE_PARAMTYPE_PHASESCALE, WCS_TEXTURE_PARAMTYPE_SKEW,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, 0,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // StepTexture::GetTextureParamType

/*===========================================================================*/

double StepTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double InvS, S, Tx, Px;
long W, Ct;

if (Input[WCS_TEXTURE_STEP_STEPS][0] > 0.0)
	{
	InvS = Input[WCS_TEXTURE_STEP_STEPS][0];
	S = 1.0 / Input[WCS_TEXTURE_STEP_STEPS][0];
	Tx = Input[WCS_TEXTURE_BELLCURVE_PHASE][0] * Input[WCS_TEXTURE_BELLCURVE_PHASESCALE][0];

	if (ApplyToColor)
		{
		if (! StrataBlend)
			ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
		for (Ct = 0; Ct < 3; Ct ++)
			{
			Px = (Input[WCS_TEXTURE_COLOR1][Ct] - Tx);
			W = quicklongfloor(Px);

			Px -= W;
			if (Px == Input[WCS_TEXTURE_BELLCURVE_SKEW][0])
				{
				Output[Ct] = 1.0;
				} // else if
			else if (Px < Input[WCS_TEXTURE_BELLCURVE_SKEW][0])
				{
				Output[Ct] = -1.0 + 2.0 * S * WCS_floor((.5 * S + Px / Input[WCS_TEXTURE_BELLCURVE_SKEW][0]) * InvS);
				} // else if
			else
				{
				Output[Ct] = -1.0 + 2.0 * S * WCS_floor((.5 * S + (1.0 - Px) / (1.0 - Input[WCS_TEXTURE_BELLCURVE_SKEW][0])) * InvS);
				} // else
			Output[Ct] *= Input[WCS_TEXTURE_BELLCURVE_AMPLITUDE][0];
			Output[Ct] += Input[WCS_TEXTURE_BELLCURVE_SHIFT][0];
			if (Output[Ct] > Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0])
				Output[Ct] = Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0];
			else if (Output[Ct] < Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0])
				Output[Ct] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
			} // for
		} // if
	else
		{
		Px = (Input[WCS_TEXTURE_LOW][0] - Tx);
		W = quicklongfloor(Px);

		Px -= W;
		if (Px == Input[WCS_TEXTURE_BELLCURVE_SKEW][0])
			{
			Output[0] = 1.0;
			} // else if
		else if (Px < Input[WCS_TEXTURE_BELLCURVE_SKEW][0])
			{
			Output[0] = -1.0 + 2.0 * S * WCS_floor((.5 * S + Px / Input[WCS_TEXTURE_BELLCURVE_SKEW][0]) * InvS);
			} // else if
		else
			{
			Output[0] = -1.0 + 2.0 * S * WCS_floor((.5 * S + (1.0 - Px) / (1.0 - Input[WCS_TEXTURE_BELLCURVE_SKEW][0])) * InvS);
			} // else
		Output[0] *= Input[WCS_TEXTURE_BELLCURVE_AMPLITUDE][0];
		Output[0] += Input[WCS_TEXTURE_BELLCURVE_SHIFT][0];
		if (Output[0] > Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0])
			Output[0] = Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0];
		else if (Output[0] < Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0])
			Output[0] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
		Output[1] = Output[2] = 0.0;
		} // else
	} // if
else
	{
	Output[2] = Output[1] = Output[0] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
	} // else

return (1.0);

} // StepTexture::Analyze

/*===========================================================================*/

SlopeTexture::SlopeTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_SLOPE)
{

SetDefaults(ParamNumber);

} // SlopeTexture::SlopeTexture

/*===========================================================================*/

SlopeTexture::SlopeTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_SLOPE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // SlopeTexture::SlopeTexture

/*===========================================================================*/

void SlopeTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 5
	WCS_TEXTURE_PARAMAVAIL, 0,	// 7
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, 0,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value (%) ", "",			// 1
	"Amplitude (%) ", "Shift (%) ",	// 3
	"Frequency ", "Phase (%) ",	// 5
	"Phase Scale ", "",	// 7
	"Lower Clamp (%) ", "Upper Clamp (%) ",		// 9
	"Color ", "",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 50.0, 0.0, 50.0, 50.0, 1.0, 0.0, 1.0, 0.0, 0.0, 100.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_SLOPE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // SlopeTexture::SetDefaults

/*===========================================================================*/

char SlopeTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, 0,
	WCS_TEXTURE_PARAMTYPE_AMPLITUDE, WCS_TEXTURE_PARAMTYPE_SHIFT,
	WCS_TEXTURE_PARAMTYPE_FREQUENCIES, WCS_TEXTURE_PARAMTYPE_PHASE,
	WCS_TEXTURE_PARAMTYPE_PHASESCALE, 0,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, 0,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // SlopeTexture::GetTextureParamType

/*===========================================================================*/

double SlopeTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double S, Tx, Px;
long W, Ct;

if (Input[WCS_TEXTURE_BELLCURVE_FREQUENCY][0] > 0.0)
	{
	S = 1.0 / Input[WCS_TEXTURE_BELLCURVE_FREQUENCY][0];
	Tx = Input[WCS_TEXTURE_BELLCURVE_PHASE][0] * Input[WCS_TEXTURE_BELLCURVE_PHASESCALE][0];

	if (ApplyToColor)
		{
		if (! StrataBlend)
			ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
		for (Ct = 0; Ct < 3; Ct ++)
			{
			Px = Input[WCS_TEXTURE_COLOR1][Ct];
			W = quicklongfloor(Px);

			Px -= W;
			if (Px <= Tx)
				{
				Output[Ct] = -1.0;
				} // if
			else if (Px >= Tx + S)
				{
				Output[Ct] = 1.0;
				} // else if
			else
				{
				Output[Ct] = -1.0 + 2.0 * (Px - Tx) / S;
				} // else
			Output[Ct] *= Input[WCS_TEXTURE_BELLCURVE_AMPLITUDE][0];
			Output[Ct] += Input[WCS_TEXTURE_BELLCURVE_SHIFT][0];
			if (Output[Ct] > Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0])
				Output[Ct] = Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0];
			else if (Output[Ct] < Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0])
				Output[Ct] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
			} // for
		} // if
	else
		{
		Px = Input[WCS_TEXTURE_LOW][0];
		W = quicklongfloor(Px);

		Px -= W;
		if (Px <= Tx)
			{
			Output[0] = -1.0;
			} // if
		else if (Px >= Tx + S)
			{
			Output[0] = 1.0;
			} // else if
		else
			{
			Output[0] = -1.0 + 2.0 * (Px - Tx) / S;
			} // else
		Output[0] *= Input[WCS_TEXTURE_BELLCURVE_AMPLITUDE][0];
		Output[0] += Input[WCS_TEXTURE_BELLCURVE_SHIFT][0];
		if (Output[0] > Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0])
			Output[0] = Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0];
		else if (Output[0] < Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0])
			Output[0] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
		Output[1] = Output[2] = 0.0;
		} // else
	} // if
else
	{
	Output[0] = Output[1] = Output[2] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
	} // else

return (1.0);

} // SlopeTexture::Analyze

/*===========================================================================*/

GammaTexture::GammaTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_GAMMA)
{

SetDefaults(ParamNumber);

} // GammaTexture::GammaTexture

/*===========================================================================*/

GammaTexture::GammaTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_GAMMA, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // GammaTexture::GammaTexture

/*===========================================================================*/

void GammaTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, 0,	// 5
	0, 0,	// 7
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, 0,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value (%) ", "",			// 1
	"Amplitude (%) ", "Shift (%) ",	// 3
	"Gamma ", "",	// 5
	"", "",	// 7
	"Lower Clamp (%) ", "Upper Clamp (%) ",		// 9
	"Color ", "",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 50.0, 0.0, 50.0, 50.0, 2.0, 0.0, 0.0, 0.0, 0.0, 100.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
double GammaRangeDefaults[3] = {100.0, 0.1, 0.1};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_GAMMA_GAMMA].SetRangeDefaults(GammaRangeDefaults);

TexType = WCS_TEXTURE_TYPE_GAMMA;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // GammaTexture::SetDefaults

/*===========================================================================*/

char GammaTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, 0,
	WCS_TEXTURE_PARAMTYPE_AMPLITUDE, WCS_TEXTURE_PARAMTYPE_SHIFT,
	WCS_TEXTURE_PARAMTYPE_GAMMA, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, 0,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // GammaTexture::GetTextureParamType

/*===========================================================================*/

double GammaTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
long Ct;

if (Input[WCS_TEXTURE_GAMMA_GAMMA][0] < .1)
	Input[WCS_TEXTURE_GAMMA_GAMMA][0] = .1;
if (ApplyToColor)
	{
	if (! StrataBlend)
		ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
	for (Ct = 0; Ct < 3; Ct ++)
		{
		Output[Ct] = -1.0 + 2.0 * SafePow(Input[WCS_TEXTURE_COLOR1][Ct], 1.0 / Input[WCS_TEXTURE_GAMMA_GAMMA][0]);
		Output[Ct] *= Input[WCS_TEXTURE_BELLCURVE_AMPLITUDE][0];
		Output[Ct] += Input[WCS_TEXTURE_BELLCURVE_SHIFT][0];
		if (Output[Ct] > Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0])
			Output[Ct] = Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0];
		else if (Output[Ct] < Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0])
			Output[Ct] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
		} // for
	} // if
else
	{
	Output[0] = -1.0 + 2.0 * SafePow(Input[WCS_TEXTURE_LOW][0], 1.0 / Input[WCS_TEXTURE_GAMMA_GAMMA][0]);
	Output[0] *= Input[WCS_TEXTURE_BELLCURVE_AMPLITUDE][0];
	Output[0] += Input[WCS_TEXTURE_BELLCURVE_SHIFT][0];
	if (Output[0] > Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0])
		Output[0] = Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0];
	else if (Output[0] < Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0])
		Output[0] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
	Output[1] = Output[2] = 0.0;
	} // else

return (1.0);

} // GammaTexture::Analyze

/*===========================================================================*/

BiasTexture::BiasTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_BIAS)
{

SetDefaults(ParamNumber);

} // BiasTexture::BiasTexture

/*===========================================================================*/

BiasTexture::BiasTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_BIAS, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // BiasTexture::BiasTexture

/*===========================================================================*/

void BiasTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 5
	0, 0,	// 7
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, 0,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value (%) ", "",			// 1
	"Amplitude (%) ", "Shift (%) ",	// 3
	"Bias (%) ", "",	// 5
	"", "",	// 7
	"Lower Clamp (%) ", "Upper Clamp (%) ",		// 9
	"Color ", "",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 50.0, 0.0, 50.0, 50.0, 25.0, 0.0, 0.0, 0.0, 0.0, 100.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
double MedianRangeDefaults[3] = {99.0, 1.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_BIAS_BIAS].SetRangeDefaults(MedianRangeDefaults);

TexType = WCS_TEXTURE_TYPE_BIAS;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // BiasTexture::SetDefaults

/*===========================================================================*/

char BiasTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, 0,
	WCS_TEXTURE_PARAMTYPE_AMPLITUDE, WCS_TEXTURE_PARAMTYPE_SHIFT,
	WCS_TEXTURE_PARAMTYPE_BIAS, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, 0,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // BiasTexture::GetTextureParamType

/*===========================================================================*/

double BiasTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
long Ct;


if (Input[WCS_TEXTURE_BIAS_BIAS][0] < .01)
	Input[WCS_TEXTURE_BIAS_BIAS][0] = .01;
else if (Input[WCS_TEXTURE_BIAS_BIAS][0] > .99)
	Input[WCS_TEXTURE_BIAS_BIAS][0] = .99;
if (ApplyToColor)
	{
	if (! StrataBlend)
		ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
	for (Ct = 0; Ct < 3; Ct ++)
		{
		// This could be * -1.4426952
		Output[Ct] = -1.0 + 2.0 * SafePow(Input[WCS_TEXTURE_COLOR1][Ct], log(Input[WCS_TEXTURE_BIAS_BIAS][0]) * (1.0 / -.69314718));//log(.5));
		Output[Ct] *= Input[WCS_TEXTURE_BELLCURVE_AMPLITUDE][0];
		Output[Ct] += Input[WCS_TEXTURE_BELLCURVE_SHIFT][0];
		if (Output[Ct] > Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0])
			Output[Ct] = Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0];
		else if (Output[Ct] < Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0])
			Output[Ct] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
		} // for
	} // if
else
	{
	// This could be * -1.4426952
	Output[0] = -1.0 + 2.0 * SafePow(Input[WCS_TEXTURE_LOW][0], log(Input[WCS_TEXTURE_BIAS_BIAS][0]) * (1.0 / -.69314718));//log(.5));
	Output[0] *= Input[WCS_TEXTURE_BELLCURVE_AMPLITUDE][0];
	Output[0] += Input[WCS_TEXTURE_BELLCURVE_SHIFT][0];
	if (Output[0] > Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0])
		Output[0] = Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0];
	else if (Output[0] < Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0])
		Output[0] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
	Output[1] = Output[2] = 0.0;
	} // else

return (1.0);

} // BiasTexture::Analyze

/*===========================================================================*/

GainTexture::GainTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_GAIN)
{

SetDefaults(ParamNumber);

} // GainTexture::GainTexture

/*===========================================================================*/

GainTexture::GainTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_GAIN, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // GainTexture::GainTexture

/*===========================================================================*/

void GainTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 5
	0, 0,	// 7
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, 0,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value (%) ", "",			// 1
	"Amplitude (%) ", "Shift (%) ",	// 3
	"Gain (%) ", "",	// 5
	"", "",	// 7
	"Lower Clamp (%) ", "Upper Clamp (%) ",		// 9
	"Color ", "",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 50.0, 0.0, 50.0, 50.0, 25.0, 0.0, 0.0, 0.0, 0.0, 100.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
double MedianRangeDefaults[3] = {99.0, 1.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_GAIN_GAIN].SetRangeDefaults(MedianRangeDefaults);

TexType = WCS_TEXTURE_TYPE_GAIN;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // GainTexture::SetDefaults

/*===========================================================================*/

char GainTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, 0,
	WCS_TEXTURE_PARAMTYPE_AMPLITUDE, WCS_TEXTURE_PARAMTYPE_SHIFT,
	WCS_TEXTURE_PARAMTYPE_GAIN, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, 0,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // GainTexture::GetTextureParamType

/*===========================================================================*/

double GainTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
long Ct;


if (Input[WCS_TEXTURE_GAIN_GAIN][0] < .01)
	Input[WCS_TEXTURE_GAIN_GAIN][0] = .01;
else if (Input[WCS_TEXTURE_GAIN_GAIN][0] > .99)
	Input[WCS_TEXTURE_GAIN_GAIN][0] = .99;
if (ApplyToColor)
	{
	if (! StrataBlend)
		ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
	for (Ct = 0; Ct < 3; Ct ++)
		{
		if (Input[WCS_TEXTURE_COLOR1][Ct] < .5)
			Output[Ct] = -1.0 + SafePow(2.0 * Input[WCS_TEXTURE_COLOR1][Ct], log(1.0 - Input[WCS_TEXTURE_GAIN_GAIN][0]) * (1.0 / -.69314718));//log(Input[WCS_TEXTURE_GAIN_MEDIAN][0])); // This could be * -1.4426952
		else
			Output[Ct] = 1.0 - SafePow(2.0 - 2.0 * Input[WCS_TEXTURE_COLOR1][Ct], log(1.0 - Input[WCS_TEXTURE_GAIN_GAIN][0]) * (1.0 / -.69314718));//log(Input[WCS_TEXTURE_GAIN_MEDIAN][0])); // This could be * -1.4426952
		Output[Ct] *= Input[WCS_TEXTURE_BELLCURVE_AMPLITUDE][0];
		Output[Ct] += Input[WCS_TEXTURE_BELLCURVE_SHIFT][0];
		if (Output[Ct] > Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0])
			Output[Ct] = Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0];
		else if (Output[Ct] < Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0])
			Output[Ct] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
		} // for
	} // if
else
	{
	if (Input[WCS_TEXTURE_LOW][0] < .5)
		Output[0] = -1.0 + SafePow(2.0 * Input[WCS_TEXTURE_LOW][0], log(1.0 - Input[WCS_TEXTURE_GAIN_GAIN][0]) * (1.0 / -.69314718));//log(Input[WCS_TEXTURE_GAIN_MEDIAN][0])); // This could be * -1.4426952
	else
		Output[0] = 1.0 - SafePow(2.0 - 2.0 * Input[WCS_TEXTURE_LOW][0], log(1.0 - Input[WCS_TEXTURE_GAIN_GAIN][0]) * (1.0 / -.69314718));//log(Input[WCS_TEXTURE_GAIN_MEDIAN][0])); // This could be * -1.4426952
	Output[0] *= Input[WCS_TEXTURE_BELLCURVE_AMPLITUDE][0];
	Output[0] += Input[WCS_TEXTURE_BELLCURVE_SHIFT][0];
	if (Output[0] > Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0])
		Output[0] = Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0];
	else if (Output[0] < Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0])
		Output[0] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
	Output[1] = Output[2] = 0.0;
	} // else

return (1.0);

} // GainTexture::Analyze

/*===========================================================================*/

CustomCurveTexture::CustomCurveTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_CUSTOMCURVE)
{

SetDefaults(ParamNumber);

} // CustomCurveTexture::CustomCurveTexture

/*===========================================================================*/

CustomCurveTexture::CustomCurveTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_CUSTOMCURVE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // CustomCurveTexture::CustomCurveTexture

/*===========================================================================*/

void CustomCurveTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, 0,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	0, 0,	// 5
	0, 0,	// 7
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, 0,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value (%) ", "",			// 1
	"Amplitude (%) ", "Shift (%) ",	// 3
	"", "",	// 5
	"", "",	// 7
	"Lower Clamp (%) ", "Upper Clamp (%) ",		// 9
	"Color ", "",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 50.0, 0.0, 50.0, 50.0, 0.0, 0.0, 0.0, 0.0, 0.0, 100.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;
GraphNode *Node;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
if (CurveADP = new AnimDoubleProfile())
	{
	CurveADP->SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY);
	CurveADP->SetDefaults(this, (char)(Ct + 21));
	CurveADP->RemoveNode(10.0, 0.0);
	if (Node = CurveADP->AddNode(1.0, 1.0, 0.0))
		{
		Node->TCB[0] = 1.0;
		} // if
	CurveADP->ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY);
	} // if

TexType = WCS_TEXTURE_TYPE_CUSTOMCURVE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // CustomCurveTexture::SetDefaults

/*===========================================================================*/

char CustomCurveTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, 0,
	WCS_TEXTURE_PARAMTYPE_AMPLITUDE, WCS_TEXTURE_PARAMTYPE_SHIFT,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, 0,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // CustomCurveTexture::GetTextureParamType

/*===========================================================================*/

void CustomCurveTexture::DeleteSpecificResources(void)
{

if (CurveADP)
	delete CurveADP;
CurveADP = NULL;

} // CustomCurveTexture::DeleteSpecificResources

/*===========================================================================*/

double CustomCurveTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
long Ct;

if (CurveADP)
	{
	if (ApplyToColor)
		{
		if (! StrataBlend)
			ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
		for (Ct = 0; Ct < 3; Ct ++)
			{
			Output[Ct] = (CurveADP->GetValue(0, Input[WCS_TEXTURE_COLOR1][Ct]) - .5) * 2.0;
			Output[Ct] *= Input[WCS_TEXTURE_BELLCURVE_AMPLITUDE][0];
			Output[Ct] += Input[WCS_TEXTURE_BELLCURVE_SHIFT][0];
			if (Output[Ct] > Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0])
				Output[Ct] = Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0];
			else if (Output[Ct] < Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0])
				Output[Ct] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
			} // for
		} // if
	else
		{
		Output[0] = (CurveADP->GetValue(0, Input[WCS_TEXTURE_LOW][0]) - .5) * 2.0;
		Output[0] *= Input[WCS_TEXTURE_BELLCURVE_AMPLITUDE][0];
		Output[0] += Input[WCS_TEXTURE_BELLCURVE_SHIFT][0];
		if (Output[0] > Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0])
			Output[0] = Input[WCS_TEXTURE_BELLCURVE_UPPERCLAMP][0];
		else if (Output[0] < Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0])
			Output[0] = Input[WCS_TEXTURE_BELLCURVE_LOWERCLAMP][0];
		Output[1] = Output[2] = 0.0;
		} // else
	} // if
else
	Output[0] = Output[1] = Output[2] = 0.0;

return (1.0);

} // CustomCurveTexture::Analyze

/*===========================================================================*/

MaximumTexture::MaximumTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_MAXIMUM)
{

SetDefaults(ParamNumber);

} // MaximumTexture::MaximumTexture

/*===========================================================================*/

MaximumTexture::MaximumTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_MAXIMUM, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // MaximumTexture::MaximumTexture

/*===========================================================================*/

void MaximumTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	0, 0,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value A (%) ", "Value B (%) ",			// 1
	"", "",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 80.0, 40.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_MAXIMUM;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // MaximumTexture::SetDefaults

/*===========================================================================*/

char MaximumTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // MaximumTexture::GetTextureParamType

/*===========================================================================*/

double MaximumTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{

if (ApplyToColor)
	{
	ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
	ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR2], 1.0, Data, EvalChildren);
	if (Input[WCS_TEXTURE_COLOR1][0] + Input[WCS_TEXTURE_COLOR1][1] + Input[WCS_TEXTURE_COLOR1][2] >=
		Input[WCS_TEXTURE_COLOR2][0] + Input[WCS_TEXTURE_COLOR2][1] + Input[WCS_TEXTURE_COLOR2][2])
		{
		Output[0] = Input[WCS_TEXTURE_COLOR1][0];
		Output[1] = Input[WCS_TEXTURE_COLOR1][1];
		Output[2] = Input[WCS_TEXTURE_COLOR1][2];
		return (0.0);
		} // if
	else
		{
		Output[0] = Input[WCS_TEXTURE_COLOR2][0];
		Output[1] = Input[WCS_TEXTURE_COLOR2][1];
		Output[2] = Input[WCS_TEXTURE_COLOR2][2];
		return (1.0);
		} // else
	} // if
else
	{
	Output[1] = 0.0;
	Output[2] = 0.0;
	if (Input[WCS_TEXTURE_LOW][0] >= Input[WCS_TEXTURE_HIGH][0])
		{
		Output[0] = Input[WCS_TEXTURE_LOW][0];
		return (0.0);
		} // if
	else
		{
		Output[0] = Input[WCS_TEXTURE_HIGH][0];
		return (1.0);
		} // else
	} // else

} // MaximumTexture::Analyze

/*===========================================================================*/

MaximumSwitchTexture::MaximumSwitchTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_MAXIMUMSWITCH)
{

SetDefaults(ParamNumber);

} // MaximumSwitchTexture::MaximumSwitchTexture

/*===========================================================================*/

MaximumSwitchTexture::MaximumSwitchTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_MAXIMUMSWITCH, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // MaximumSwitchTexture::MaximumSwitchTexture

/*===========================================================================*/

void MaximumSwitchTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value A (%) ", "Value B (%) ",			// 1
	"Switch A (%) ", "Switch B (%) ",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 80.0, 40.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_MAXIMUMSWITCH;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // MaximumSwitchTexture::SetDefaults

/*===========================================================================*/

char MaximumSwitchTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_SWITCH, WCS_TEXTURE_PARAMTYPE_SWITCH,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // MaximumSwitchTexture::GetTextureParamType

/*===========================================================================*/

double MaximumSwitchTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{

if (ApplyToColor)
	{
	ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
	ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR2], 1.0, Data, EvalChildren);
	if (Input[WCS_TEXTURE_MINIMUMSWITCH_SWITCHA][0] >= Input[WCS_TEXTURE_MINIMUMSWITCH_SWITCHB][0])
		{
		Output[0] = Input[WCS_TEXTURE_COLOR1][0];
		Output[1] = Input[WCS_TEXTURE_COLOR1][1];
		Output[2] = Input[WCS_TEXTURE_COLOR1][2];
		return (0.0);
		} // if
	else
		{
		Output[0] = Input[WCS_TEXTURE_COLOR2][0];
		Output[1] = Input[WCS_TEXTURE_COLOR2][1];
		Output[2] = Input[WCS_TEXTURE_COLOR2][2];
		return (1.0);
		} // else
	} // if
else
	{
	Output[1] = 0.0;
	Output[2] = 0.0;
	if (Input[WCS_TEXTURE_MINIMUMSWITCH_SWITCHA][0] >= Input[WCS_TEXTURE_MINIMUMSWITCH_SWITCHB][0])
		{
		Output[0] = Input[WCS_TEXTURE_LOW][0];
		return (0.0);
		} // if
	else
		{
		Output[0] = Input[WCS_TEXTURE_HIGH][0];
		return (1.0);
		} // else
	} // else

} // MaximumSwitchTexture::Analyze

/*===========================================================================*/

MinimumTexture::MinimumTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_MINIMUM)
{

SetDefaults(ParamNumber);

} // MinimumTexture::MinimumTexture

/*===========================================================================*/

MinimumTexture::MinimumTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_MINIMUM, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // MinimumTexture::MinimumTexture

/*===========================================================================*/

void MinimumTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	0, 0,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value A (%) ", "Value B (%) ",			// 1
	"", "",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 80.0, 40.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_MINIMUM;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // MinimumTexture::SetDefaults

/*===========================================================================*/

char MinimumTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // MinimumTexture::GetTextureParamType

/*===========================================================================*/

double MinimumTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{

if (ApplyToColor)
	{
	ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
	ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR2], 1.0, Data, EvalChildren);
	if (Input[WCS_TEXTURE_COLOR1][0] + Input[WCS_TEXTURE_COLOR1][1] + Input[WCS_TEXTURE_COLOR1][2] <=
		Input[WCS_TEXTURE_COLOR2][0] + Input[WCS_TEXTURE_COLOR2][1] + Input[WCS_TEXTURE_COLOR2][2])
		{
		Output[0] = Input[WCS_TEXTURE_COLOR1][0];
		Output[1] = Input[WCS_TEXTURE_COLOR1][1];
		Output[2] = Input[WCS_TEXTURE_COLOR1][2];
		return (0.0);
		} // if
	else
		{
		ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR2], 1.0, Data, EvalChildren);
		Output[0] = Input[WCS_TEXTURE_COLOR2][0];
		Output[1] = Input[WCS_TEXTURE_COLOR2][1];
		Output[2] = Input[WCS_TEXTURE_COLOR2][2];
		return (1.0);
		} // else
	} // if
else
	{
	Output[1] = 0.0;
	Output[2] = 0.0;
	if (Input[WCS_TEXTURE_LOW][0] <= Input[WCS_TEXTURE_HIGH][0])
		{
		Output[0] = Input[WCS_TEXTURE_LOW][0];
		return (0.0);
		} // if
	else
		{
		Output[0] = Input[WCS_TEXTURE_HIGH][0];
		return (1.0);
		} // else
	} // else

} // MinimumTexture::Analyze

/*===========================================================================*/

MinimumSwitchTexture::MinimumSwitchTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_MINIMUMSWITCH)
{

SetDefaults(ParamNumber);

} // MinimumSwitchTexture::MinimumSwitchTexture

/*===========================================================================*/

MinimumSwitchTexture::MinimumSwitchTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_MINIMUMSWITCH, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // MinimumSwitchTexture::MinimumSwitchTexture

/*===========================================================================*/

void MinimumSwitchTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value A (%) ", "Value B (%) ",			// 1
	"Switch A (%) ", "Switch B (%) ",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 80.0, 40.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_MINIMUMSWITCH;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // MinimumSwitchTexture::SetDefaults

/*===========================================================================*/

char MinimumSwitchTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_SWITCH, WCS_TEXTURE_PARAMTYPE_SWITCH,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // MinimumSwitchTexture::GetTextureParamType

/*===========================================================================*/

double MinimumSwitchTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{

if (ApplyToColor)
	{
	if (Input[WCS_TEXTURE_MINIMUMSWITCH_SWITCHA][0] <= Input[WCS_TEXTURE_MINIMUMSWITCH_SWITCHB][0])
		{
		ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
		Output[0] = Input[WCS_TEXTURE_COLOR1][0];
		Output[1] = Input[WCS_TEXTURE_COLOR1][1];
		Output[2] = Input[WCS_TEXTURE_COLOR1][2];
		return (0.0);
		} // if
	else
		{
		ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR2], 1.0, Data, EvalChildren);
		Output[0] = Input[WCS_TEXTURE_COLOR2][0];
		Output[1] = Input[WCS_TEXTURE_COLOR2][1];
		Output[2] = Input[WCS_TEXTURE_COLOR2][2];
		return (1.0);
		} // else
	} // if
else
	{
	Output[1] = 0.0;
	Output[2] = 0.0;
	if (Input[WCS_TEXTURE_MINIMUMSWITCH_SWITCHA][0] <= Input[WCS_TEXTURE_MINIMUMSWITCH_SWITCHB][0])
		{
		Output[0] = Input[WCS_TEXTURE_LOW][0];
		return (0.0);
		} // if
	else
		{
		Output[0] = Input[WCS_TEXTURE_HIGH][0];
		return (1.0);
		} // else
	} // else

} // MinimumSwitchTexture::Analyze

/*===========================================================================*/

ThresholdTexture::ThresholdTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_THRESHOLD)
{

SetDefaults(ParamNumber);

} // ThresholdTexture::ThresholdTexture

/*===========================================================================*/

ThresholdTexture::ThresholdTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_THRESHOLD, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // ThresholdTexture::ThresholdTexture

/*===========================================================================*/

void ThresholdTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	0, 0	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Value A (%) ", "Value B (%) ",			// 1
	"Threshold (%) ", "Sample (%) ",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"", ""	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 15.0, 70.0, 50.0, 60.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_THRESHOLD;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // ThresholdTexture::SetDefaults

/*===========================================================================*/

char ThresholdTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_THRESHOLD, WCS_TEXTURE_PARAMTYPE_SAMPLE,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	0, 0
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // ThresholdTexture::GetTextureParamType

/*===========================================================================*/

double ThresholdTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{

if (ApplyToColor)
	{
	if (Input[WCS_TEXTURE_THRESHOLD_SAMPLE][0] >= Input[WCS_TEXTURE_THRESHOLD_THRESHOLD][0])
		{
		ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR2], 1.0, Data, EvalChildren);
		Output[0] = Input[WCS_TEXTURE_COLOR2][0];
		Output[1] = Input[WCS_TEXTURE_COLOR2][1];
		Output[2] = Input[WCS_TEXTURE_COLOR2][2];
		return (1.0);
		} // if
	else
		{
		ColorGrad.Analyze(Input[WCS_TEXTURE_COLOR1], 0.0, Data, EvalChildren);
		Output[0] = Input[WCS_TEXTURE_COLOR1][0];
		Output[1] = Input[WCS_TEXTURE_COLOR1][1];
		Output[2] = Input[WCS_TEXTURE_COLOR1][2];
		return (0.0);
		} // else
	} // if
else
	{
	Output[1] = 0.0;
	Output[2] = 0.0;
	if (Input[WCS_TEXTURE_THRESHOLD_SAMPLE][0] >= Input[WCS_TEXTURE_THRESHOLD_THRESHOLD][0])
		{
		Output[0] = Input[WCS_TEXTURE_HIGH][0];
		return (1.0);
		} // if
	else
		{
		Output[0] = Input[WCS_TEXTURE_LOW][0];
		return (0.0);
		} // else
	} // else

} // ThresholdTexture::Analyze

/*===========================================================================*/

TerrainParameterTexture::TerrainParameterTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_TERRAINPARAM)
{

SetDefaults(ParamNumber);

} // TerrainParameterTexture::TerrainParameterTexture

/*===========================================================================*/

TerrainParameterTexture::TerrainParameterTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_TERRAINPARAM, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // TerrainParameterTexture::TerrainParameterTexture

/*===========================================================================*/

void TerrainParameterTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	WCS_TEXTURE_PARAMAVAIL, WCS_TEXTURE_PARAMAVAIL,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Out Low (%) ", "Out High (%) ",			// 1
	"Input Low ", "Input High ",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 0.0, 100.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
double InputRangeDefaults[3] = {FLT_MAX, -FLT_MAX, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for
TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetRangeDefaults(InputRangeDefaults);
TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetRangeDefaults(InputRangeDefaults);

TexType = WCS_TEXTURE_TYPE_TERRAINPARAM;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetMiscDefaults();
SetupForInputParamTexture(ParamNumber);

} // TerrainParameterTexture::SetDefaults

/*===========================================================================*/

void TerrainParameterTexture::SetMiscDefaults(void)
{
double InputRangeDefaults[3] = {FLT_MAX, -FLT_MAX, 1.0};
unsigned char Dimension = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;

switch (Misc)
	{
	case WCS_TEXTURE_TERRAINPARAM_ELEV:
		{
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTLOW + 20,
			0.0);
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTHIGH + 20,
			1000.0);
		Dimension = WCS_ANIMDOUBLE_METRIC_HEIGHT;
		break;
		} // WCS_TEXTURE_TERRAINPARAM_ELEV
	case WCS_TEXTURE_TERRAINPARAM_RELELEV:
		{
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTLOW + 20,
			-100.0);
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTHIGH + 20,
			100.0);
		Dimension = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;
		break;
		} // WCS_TEXTURE_TERRAINPARAM_RELELEV
	case WCS_TEXTURE_TERRAINPARAM_SLOPE:
		{
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTLOW + 20,
			0.0);
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTHIGH + 20,
			90.0);
		InputRangeDefaults[1] = 0.0;
		InputRangeDefaults[0] = 90.0;
		Dimension = WCS_ANIMDOUBLE_METRIC_ANGLE;
		break;
		} // WCS_TEXTURE_TERRAINPARAM_SLOPE
	case WCS_TEXTURE_TERRAINPARAM_ASPECT:
		{
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTLOW + 20,
			0.0);
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTHIGH + 20,
			360.0);
		InputRangeDefaults[1] = -360.0;
		InputRangeDefaults[0] = 360.0;
		Dimension = WCS_ANIMDOUBLE_METRIC_ANGLE;
		break;
		} // WCS_TEXTURE_TERRAINPARAM_ASPECT
	case WCS_TEXTURE_TERRAINPARAM_NORTHDEV:
		{
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTLOW + 20,
			0.0);
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTHIGH + 20,
			180.0);
		InputRangeDefaults[1] = 0.0;
		InputRangeDefaults[0] = 180.0;
		Dimension = WCS_ANIMDOUBLE_METRIC_ANGLE;
		break;
		} // WCS_TEXTURE_TERRAINPARAM_NORTHDEV
	case WCS_TEXTURE_TERRAINPARAM_EASTDEV:
		{
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTLOW + 20,
			0.0);
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTHIGH + 20,
			180.0);
		InputRangeDefaults[1] = 0.0;
		InputRangeDefaults[0] = 180.0;
		Dimension = WCS_ANIMDOUBLE_METRIC_ANGLE;
		break;
		} // WCS_TEXTURE_TERRAINPARAM_EASTDEV
	case WCS_TEXTURE_TERRAINPARAM_LATITUDE:
		{
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTLOW + 20,
			-90.0);
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTHIGH + 20,
			90.0);
		InputRangeDefaults[1] = -90.0;
		InputRangeDefaults[0] = 90.0;
		Dimension = WCS_ANIMDOUBLE_METRIC_LATITUDE;
		break;
		} // WCS_TEXTURE_TERRAINPARAM_LATITUDE
	case WCS_TEXTURE_TERRAINPARAM_LONGITUDE:
		{
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTLOW + 20,
			0.0);
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTHIGH + 20,
			360.0);
		Dimension = WCS_ANIMDOUBLE_METRIC_LONGITUDE;
		break;
		} // WCS_TEXTURE_TERRAINPARAM_LONGITUDE
	case WCS_TEXTURE_TERRAINPARAM_ILLUMINATION:
		{
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTLOW + 20,
			0.0);
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTHIGH + 20,
			100.0);
		InputRangeDefaults[1] = 0.0;
		InputRangeDefaults[0] = 100000.0;
		Dimension = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;
		break;
		} // WCS_TEXTURE_TERRAINPARAM_ILLUMINATION
	case WCS_TEXTURE_TERRAINPARAM_ZDISTANCE:
		{
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTLOW + 20,
			0.0);
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTHIGH + 20,
			1000.0);
		InputRangeDefaults[1] = 0.0;
		Dimension = WCS_ANIMDOUBLE_METRIC_DISTANCE;
		break;
		} // WCS_TEXTURE_TERRAINPARAM_ZDISTANCE
	case WCS_TEXTURE_TERRAINPARAM_WATERDEPTH:
		{
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTLOW + 20,
			0.0);
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTHIGH + 20,
			100.0);
		Dimension = WCS_ANIMDOUBLE_METRIC_HEIGHT;
		break;
		} // WCS_TEXTURE_TERRAINPARAM_WATERDEPTH
	case WCS_TEXTURE_TERRAINPARAM_VECTORSLOPE:
		{
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTLOW + 20,
			0.0);
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTHIGH + 20,
			10.0);
		InputRangeDefaults[1] = 0.0;
		InputRangeDefaults[0] = 1000.0;
		Dimension = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;
		break;
		} // WCS_TEXTURE_TERRAINPARAM_VECTORSLOPE
	case WCS_TEXTURE_TERRAINPARAM_RED:
	case WCS_TEXTURE_TERRAINPARAM_GREEN:
	case WCS_TEXTURE_TERRAINPARAM_BLUE:
		{
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTLOW + 20,
			0.0);
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTHIGH + 20,
			255.0);
		InputRangeDefaults[1] = 0.0;
		InputRangeDefaults[0] = 100000.0;
		Dimension = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;
		break;
		} // WCS_TEXTURE_TERRAINPARAM_RED
	case WCS_TEXTURE_TERRAINPARAM_HUE:
		{
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTLOW + 20,
			0.0);
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTHIGH + 20,
			360.0);
		InputRangeDefaults[1] = 0.0;
		InputRangeDefaults[0] = 360.0;
		Dimension = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;
		break;
		} // WCS_TEXTURE_TERRAINPARAM_HUE
	case WCS_TEXTURE_TERRAINPARAM_SATURATION:
	case WCS_TEXTURE_TERRAINPARAM_VALUE:
	case WCS_TEXTURE_TERRAINPARAM_LUMINOSITY:
		{
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTLOW + 20,
			0.0);
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTHIGH + 20,
			100.0);
		InputRangeDefaults[1] = 0.0;
		InputRangeDefaults[0] = 100000.0;
		Dimension = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;
		break;
		} // WCS_TEXTURE_TERRAINPARAM_SATURATION
	case WCS_TEXTURE_TERRAINPARAM_REFLECTIVITY:
		{
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTLOW + 20,
			0.0);
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTHIGH + 20,
			100.0);
		InputRangeDefaults[1] = 0.0;
		InputRangeDefaults[0] = 100000.0;
		Dimension = WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS;
		break;
		} // WCS_TEXTURE_TERRAINPARAM_REFLECTIVITY
	case WCS_TEXTURE_TERRAINPARAM_NORMALFROMCAMERA:
		{
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTLOW + 20,
			0.0);
		TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetDefaults(this, (char)WCS_TEXTURE_TERRAINPARAM_INPUTHIGH + 20,
			90.0);
		InputRangeDefaults[1] = 0.0;
		InputRangeDefaults[0] = 90.0;
		Dimension = WCS_ANIMDOUBLE_METRIC_ANGLE;
		break;
		} // WCS_TEXTURE_TERRAINPARAM_NORMALFROMCAMERA
	} // switch
TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetRangeDefaults(InputRangeDefaults);
TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetRangeDefaults(InputRangeDefaults);
TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetMetricType(Dimension);
TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetMetricType(Dimension);

} // TerrainParameterTexture::SetMiscDefaults

/*===========================================================================*/

char TerrainParameterTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	WCS_TEXTURE_PARAMTYPE_TERRAINPAR, WCS_TEXTURE_PARAMTYPE_TERRAINPAR,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // TerrainParameterTexture::GetTextureParamType

/*===========================================================================*/

double TerrainParameterTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Value, TerrainParam, HSV[3];

if (Data->IsSampleThumbnail)
	{
	Value = 1.0 - Data->ThumbnailY;
	} // if
else if (! Data->SampleType)
	{
	switch (Misc)
		{
		case WCS_TEXTURE_TERRAINPARAM_ELEV:
			{
			TerrainParam = Data->Elev;
			if (Data->ExagerateElevLines)
				{
				if (Data->Exageration != 0.0)
					TerrainParam = Data->Datum + ((TerrainParam - Data->Datum) / Data->Exageration);
				} // if
			break;
			} // WCS_TEXTURE_TERRAINPARAM_ELEV
		case WCS_TEXTURE_TERRAINPARAM_RELELEV:
			{
			TerrainParam = Data->RelElev;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_RELELEV
		case WCS_TEXTURE_TERRAINPARAM_SLOPE:
			{
			TerrainParam = Data->Slope;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_SLOPE
		case WCS_TEXTURE_TERRAINPARAM_ASPECT:
			{
			double HighOffset, LowOffset;
			TerrainParam = Data->Aspect;
			if (Input[WCS_TEXTURE_TERRAINPARAM_INPUTLOW][0] <= Input[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH][0])
				{
				// try to fit the value inside the range
				while (TerrainParam > Input[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH][0])
					TerrainParam -= 360.0;
				while (TerrainParam < Input[WCS_TEXTURE_TERRAINPARAM_INPUTLOW][0])
					TerrainParam += 360.0;
				// if still outside the range it will be above the range so figure out if it is closer to the range above or below
				if (TerrainParam > Input[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH][0])
					{
					HighOffset = TerrainParam - Input[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH][0];
					LowOffset = Input[WCS_TEXTURE_TERRAINPARAM_INPUTLOW][0] - (TerrainParam - 360.0);
					if (LowOffset < HighOffset)
						TerrainParam -= 360.0;
					} // if still outside range
				} // if
			else
				{
				while (TerrainParam > Input[WCS_TEXTURE_TERRAINPARAM_INPUTLOW][0])
					TerrainParam -= 360.0;
				while (TerrainParam < Input[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH][0])
					TerrainParam += 360.0;
				// if still outside the range it will be above the range so figure out if it is closer to the range above or below
				if (TerrainParam > Input[WCS_TEXTURE_TERRAINPARAM_INPUTLOW][0])
					{
					HighOffset = TerrainParam - Input[WCS_TEXTURE_TERRAINPARAM_INPUTLOW][0];
					LowOffset = Input[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH][0] - (TerrainParam - 360.0);
					if (LowOffset < HighOffset)
						TerrainParam -= 360.0;
					} // if still outside range
				} // else
			break;
			} // WCS_TEXTURE_TERRAINPARAM_ASPECT
		case WCS_TEXTURE_TERRAINPARAM_NORTHDEV:
			{
			TerrainParam = Data->Aspect;
			while (TerrainParam > 180.0)
				TerrainParam -= 360.0;
			while (TerrainParam < -180.0)
				TerrainParam += 360.0;
			TerrainParam = fabs(TerrainParam);
			break;
			} // WCS_TEXTURE_TERRAINPARAM_NORTHDEV
		case WCS_TEXTURE_TERRAINPARAM_EASTDEV:
			{
			TerrainParam = Data->Aspect - 90.0;
			while (TerrainParam > 180.0)
				TerrainParam -= 360.0;
			while (TerrainParam < -180.0)
				TerrainParam += 360.0;
			TerrainParam = fabs(TerrainParam);
			break;
			} // WCS_TEXTURE_TERRAINPARAM_EASTDEV
		case WCS_TEXTURE_TERRAINPARAM_LATITUDE:
			{
			TerrainParam = Data->Latitude;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_LATITUDE
		case WCS_TEXTURE_TERRAINPARAM_LONGITUDE:
			{
			TerrainParam = Data->Longitude;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_LONGITUDE
		case WCS_TEXTURE_TERRAINPARAM_ILLUMINATION:
			{
			TerrainParam = Data->Illumination * 100;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_ILLUMINATION
		case WCS_TEXTURE_TERRAINPARAM_ZDISTANCE:
			{
			TerrainParam = Data->ZDist;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_ZDISTANCE
		case WCS_TEXTURE_TERRAINPARAM_WATERDEPTH:
			{
			TerrainParam = Data->WaterDepth;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_WATERDEPTH
		case WCS_TEXTURE_TERRAINPARAM_VECTORSLOPE:
			{
			TerrainParam = Data->VectorSlope;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_VECTORSLOPE
		case WCS_TEXTURE_TERRAINPARAM_RED:
			{
			TerrainParam = Data->RGB[0] * 255.0;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_RED
		case WCS_TEXTURE_TERRAINPARAM_GREEN:
			{
			TerrainParam = Data->RGB[1] * 255.0;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_GREEN
		case WCS_TEXTURE_TERRAINPARAM_BLUE:
			{
			TerrainParam = Data->RGB[2] * 255.0;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_BLUE
		case WCS_TEXTURE_TERRAINPARAM_HUE:
			{
			RGBtoHSV(HSV, Data->RGB);
			TerrainParam = HSV[0];
			break;
			} // WCS_TEXTURE_TERRAINPARAM_HUE
		case WCS_TEXTURE_TERRAINPARAM_SATURATION:
			{
			RGBtoHSV(HSV, Data->RGB);
			TerrainParam = HSV[1];
			break;
			} // WCS_TEXTURE_TERRAINPARAM_SATURATION
		case WCS_TEXTURE_TERRAINPARAM_VALUE:
			{
			RGBtoHSV(HSV, Data->RGB);
			TerrainParam = HSV[2];
			break;
			} // WCS_TEXTURE_TERRAINPARAM_VALUE
		case WCS_TEXTURE_TERRAINPARAM_LUMINOSITY:
			{
			TerrainParam = (Data->RGB[0] + Data->RGB[1] + Data->RGB[2]) * (100.0 / 3.0);
			break;
			} // WCS_TEXTURE_TERRAINPARAM_LUMINOSITY
		case WCS_TEXTURE_TERRAINPARAM_REFLECTIVITY:
			{
			TerrainParam = Data->Reflectivity * 100.0;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_REFLECTIVITY
		case WCS_TEXTURE_TERRAINPARAM_NORMALFROMCAMERA:
			{
			TerrainParam = acos(VectorAngle(Data->Normal, Data->CamAimVec)) * PiUnder180;
			break;
			} // WCS_TEXTURE_TERRAINPARAM_NORMALFROMCAMERA
		default:
			{
			TerrainParam = 0.0;
			break;
			} // default
		} // switch
	if (Input[WCS_TEXTURE_TERRAINPARAM_INPUTLOW][0] <= Input[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH][0])
		{
		if (TerrainParam <= Input[WCS_TEXTURE_TERRAINPARAM_INPUTLOW][0])
			Value = 0.0;
		else if (TerrainParam >= Input[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH][0])
			Value = 1.0;
		else
			Value = (TerrainParam - Input[WCS_TEXTURE_TERRAINPARAM_INPUTLOW][0]) / (Input[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH][0] - Input[WCS_TEXTURE_TERRAINPARAM_INPUTLOW][0]);
		} // if
	else
		{
		if (TerrainParam >= Input[WCS_TEXTURE_TERRAINPARAM_INPUTLOW][0])
			Value = 0.0;
		else if (TerrainParam <= Input[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH][0])
			Value = 1.0;
		else
			Value = (TerrainParam - Input[WCS_TEXTURE_TERRAINPARAM_INPUTLOW][0]) / (Input[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH][0] - Input[WCS_TEXTURE_TERRAINPARAM_INPUTLOW][0]);
		} // else
	} // if
else
	Value = (Data->TLowY + Data->THighY) * .5;  // Optimized out division. Was / 2.0

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // TerrainParameterTexture::Analyze

/*===========================================================================*/

GradientTexture::GradientTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_GRADIENT)
{

SetDefaults(ParamNumber);

} // GradientTexture::GradientTexture

/*===========================================================================*/

GradientTexture::GradientTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_GRADIENT, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // GradientTexture::GradientTexture

/*===========================================================================*/

void GradientTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	0, 0,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// size
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// center
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL,	// falloff
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Out Low (%) ", "Out High (%) ",			// 1
	"", "",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "X Size", "Y Size", "Z Size",	// size
	"X Center", "Y Center", "Z Center",	// center
	"X Falloff", "Y Falloff", "Z Falloff",	// falloff
	"X Rotation", "Y Rotation", "Z Rotation"	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_GRADIENT;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();
SetupForInputParamTexture(ParamNumber);

} // GradientTexture::SetDefaults

/*===========================================================================*/

char GradientTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE, WCS_TEXTURE_PARAMTYPE_SIZE,	// size
	WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER, WCS_TEXTURE_PARAMTYPE_CENTER,	// center
	WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF, WCS_TEXTURE_PARAMTYPE_FALLOFF,	// falloff
	WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION, WCS_TEXTURE_PARAMTYPE_ROTATION	// rotation
	};

return (ParamType[ParamNum]);

} // GradientTexture::GetTextureParamType

/*===========================================================================*/

double GradientTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Value, Temp;

ReCenterMoveRotate(LowX, HighX, LowY, HighY, LowZ, HighZ);

if (TexAxis == 0)
	{
	Temp = ((LowX + HighX) * .5) / TSize[0];
	} // if
else if (TexAxis == 1)
	{
	Temp = ((LowY + HighY) * .5) / TSize[1];
	} // else if
else
	{
	Temp = ((LowZ + HighZ) * .5) / TSize[2];
	} // else

if (Temp <= -.5)
	Value = 0.0;
else if (Temp < .5)
	Value = (Temp + .5);
else
	Value = 1.0;

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // GradientTexture::Analyze

/*===========================================================================*/

IncludeExcludeTexture::IncludeExcludeTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_INCLUDEEXCLUDE)
{

SetDefaults(ParamNumber);

} // IncludeExcludeTexture::IncludeExcludeTexture

/*===========================================================================*/

IncludeExcludeTexture::IncludeExcludeTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_INCLUDEEXCLUDE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // IncludeExcludeTexture::IncludeExcludeTexture

/*===========================================================================*/

void IncludeExcludeTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	0, 0,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Out Low (%) ", "Out High (%) ",			// 1
	"", "",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

if (InclExcl = new IncludeExcludeList(this))
	InclExcl->Enabled = 1;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_INCLUDEEXCLUDE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // IncludeExcludeTexture::SetDefaults

/*===========================================================================*/

char IncludeExcludeTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // IncludeExcludeTexture::GetTextureParamType

/*===========================================================================*/

double IncludeExcludeTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Value;

if (Data->IsSampleThumbnail)
	{
	Value = Data->ThumbnailX < .5 ? 0.0: 1.0;
	} // if
else if (! Data->SampleType)
	{
	if (InclExcl && InclExcl->PassTest(Data->Object))
		Value = 1.0;
	else
		Value = 0.0;
	} // else if
else
	Value = (Data->TLowY + Data->THighY) * .5;  // Optimized out division. Was / 2.0

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // IncludeExcludeTexture::Analyze

/*===========================================================================*/

IncludeExcludeTypeTexture::IncludeExcludeTypeTexture(RasterAnimHost *RAHost, long ParamNumber)
: Texture(RAHost, WCS_TEXTURE_TYPE_INCLUDEEXCLUDETYPE)
{

SetDefaults(ParamNumber);

} // IncludeExcludeTypeTexture::IncludeExcludeTypeTexture

/*===========================================================================*/

IncludeExcludeTypeTexture::IncludeExcludeTypeTexture(RasterAnimHost *RAHost, long ParamNumber, unsigned char EcosysSource, unsigned char ColorSource, Texture *CopyFrom, AnimColorTime *DefaultColor)
: Texture(RAHost, WCS_TEXTURE_TYPE_INCLUDEEXCLUDETYPE, EcosysSource, ColorSource, DefaultColor)
{

SetDefaults(ParamNumber);
CopyAndTransfer(CopyFrom);

} // IncludeExcludeTypeTexture::IncludeExcludeTypeTexture

/*===========================================================================*/

void IncludeExcludeTypeTexture::SetDefaults(long ParamNumber)
{
unsigned char PFlags[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 0
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL | WCS_TEXTURE_PARAMPERCENT,	// 1
	0, 0,	// 3
	0, 0,	// 5
	0, 0,	// 7
	0, 0,	// 9
	WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_PARAMAVAIL | WCS_TEXTURE_TEXTUREAVAIL,	// 11
	WCS_TEXTURE_TEXTUREAVAIL, WCS_TEXTURE_TEXTUREAVAIL	// 13
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};
char *PName[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	"Opacity (%) ",				// 0
	"Out Low (%) ", "Out High (%) ",			// 1
	"", "",	// 3
	"", "",	// 5
	"", "",	// 7
	"", "",		// 9
	"Color ", "Color ",		// 11
	"Remap Function 1 ", "Remap Function 2 "	// 13
	, "", "", "",	// size
	"", "", "",	// center
	"", "", "",	// falloff
	"", "", ""	// rotation
	};
double TexDefault[WCS_TEXTURE_MAXPARAMS] = {100.0, 0.0, 100.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
double RangeDefaults[3] = {100.0, 0.0, 1.0};
long Ct;

if (InclExclType = new IncludeExcludeTypeList(this))
	InclExclType->Enabled = 1;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	TexParam[Ct].SetDefaults(this, (char)(Ct + 20), TexDefault[Ct]);
	TexParam[Ct].SetRangeDefaults(RangeDefaults);
	} // for

TexType = WCS_TEXTURE_TYPE_INCLUDEEXCLUDETYPE;

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	ParamFlags[Ct] = PFlags[Ct];
	ParamName[Ct] = PName[Ct];
	} // for
SetDimensionUnits();

} // IncludeExcludeTypeTexture::SetDefaults

/*===========================================================================*/

char IncludeExcludeTypeTexture::GetTextureParamType(int ParamNum)
{
char ParamType[WCS_TEXTURE_MAXPARAMTEXTURES] = {
	WCS_TEXTURE_PARAMTYPE_OPACITY,
	WCS_TEXTURE_PARAMTYPE_EXTREMA, WCS_TEXTURE_PARAMTYPE_EXTREMA,
	0, 0,
	0, 0,
	0, 0,
	0, 0,
	WCS_TEXTURE_PARAMTYPE_ENDMEMBER, WCS_TEXTURE_PARAMTYPE_ENDMEMBER,
	WCS_TEXTURE_PARAMTYPE_STRATAFUNC, WCS_TEXTURE_PARAMTYPE_BLENDINGFUNC
	, 0, 0, 0,	// size
	0, 0, 0,	// center
	0, 0, 0,	// falloff
	0, 0, 0	// rotation
	};

return (ParamType[ParamNum]);

} // IncludeExcludeTypeTexture::GetTextureParamType

/*===========================================================================*/

double IncludeExcludeTypeTexture::Analyze(double Output[3], double LowX, double HighX, double LowY, double HighY, double LowZ, double HighZ, double Input[WCS_TEXTURE_MAXPARAMTEXTURES][3], TextureData *Data, int EvalChildren, int StrataBlend)
{
double Value;

if (Data->IsSampleThumbnail)
	{
	Value = Data->ThumbnailY < .5 ? 0.0: 1.0;
	} // if
else if (! Data->SampleType)
	{
	if (InclExclType && InclExclType->PassTest(Data->ObjectType))
		Value = 1.0;
	else
		Value = 0.0;
	} // else if
else
	Value = (Data->TLowY + Data->THighY) * .5;  // Optimized out division. Was / 2.0

return (AnalyzeWrapup(Output, Input, Value, Data, EvalChildren));

} // IncludeExcludeTypeTexture::Analyze

/*===========================================================================*/

// <<<>>> ADD_NEW_TEXTURE

unsigned long RootTexture::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char TextureTypeStr[64];
char TexType;
double TempVal;
Texture *LoadTex = NULL, *CurrentTex = NULL;

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
					case WCS_ROOTTEXTURE_BROWSEDATA:
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
					case WCS_ROOTTEXTURE_NAME:
						{
						BytesRead = ReadBlock(ffile, (char *)Name, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_ROOTTEXTURE_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_ROOTTEXTURE_APPLYTOECOSYS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ApplyToEcosys, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_ROOTTEXTURE_APPLYTOCOLOR:
						{
						BytesRead = ReadBlock(ffile, (char *)&ApplyToColor, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_ROOTTEXTURE_APPLYTODISPLACE:
						{
						BytesRead = ReadBlock(ffile, (char *)&ApplyToDisplace, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_ROOTTEXTURE_SHOWSIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&ShowSize, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_ROOTTEXTURE_SHOWROOTNAIL:
						{
						BytesRead = ReadBlock(ffile, (char *)&ShowRootNail, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_ROOTTEXTURE_SHOWCOMPCHILDNAIL:
						{
						BytesRead = ReadBlock(ffile, (char *)&ShowCompChildNail, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_ROOTTEXTURE_SHOWCOMPONENTNAIL:
						{
						BytesRead = ReadBlock(ffile, (char *)&ShowComponentNail, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_ROOTTEXTURE_PREVIEWSIZE_DBL:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempVal, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						PreviewSize.SetValue(TempVal);
						break;
						}
					case WCS_ROOTTEXTURE_PREVIEWSIZE:
						{
						BytesRead = PreviewSize.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_ROOTTEXTURE_PREVIEWDIRECTION:
						{
						BytesRead = ReadBlock(ffile, (char *)&PreviewDirection, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_ROOTTEXTURE_TEXTURETYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)TextureTypeStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if ((TexType = IdentifyTextureType(TextureTypeStr)) >= 0)
							{
							LoadTex = AddNewTexture(NULL, TexType);
							} // if
						break;
						}
					case WCS_ROOTTEXTURE_TEXTURE:
						{
						if (LoadTex)
							{
							BytesRead = LoadTex->Load(ffile, Size, ByteFlip);
							if (CurrentTex)
								CurrentTex->Next = LoadTex;
							CurrentTex = LoadTex;
							LoadTex->InitNoise();
							LoadTex->InitBasis();
							LoadTex->SetDimensionUnits();
							LoadTex = NULL;		// set it to NULL so don't read it again
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

return (TotalRead);

} // RootTexture::Load

/*===========================================================================*/

unsigned long RootTexture::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
Texture *Current;

if (BrowseInfo)
	{
	ItemTag = WCS_ROOTTEXTURE_BROWSEDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_ROOTTEXTURE_NAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Name) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Name)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ROOTTEXTURE_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ROOTTEXTURE_APPLYTOECOSYS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ApplyToEcosys)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ROOTTEXTURE_APPLYTOCOLOR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ApplyToColor)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ROOTTEXTURE_APPLYTODISPLACE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ApplyToDisplace)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ROOTTEXTURE_SHOWSIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ShowSize)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ROOTTEXTURE_SHOWROOTNAIL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ShowRootNail)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ROOTTEXTURE_SHOWCOMPCHILDNAIL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ShowCompChildNail)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ROOTTEXTURE_SHOWCOMPONENTNAIL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ShowComponentNail)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ROOTTEXTURE_PREVIEWDIRECTION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PreviewDirection)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_ROOTTEXTURE_PREVIEWSIZE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = PreviewSize.Save(ffile))
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
			} // if preview size saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

Current = Tex;
while (Current)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_ROOTTEXTURE_TEXTURETYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(GetTextureName(Current->GetTexType())) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)GetTextureName(Current->GetTexType()))) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	ItemTag = WCS_ROOTTEXTURE_TEXTURE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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
	Current = Current->Next;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // RootTexture::Save

/*===========================================================================*/

unsigned long Texture::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
unsigned long ImageID;
short GeoRegLoaded = 0, V5MiscParam = 0;
char TextureTypeStr[64], CoordSpaceName[64], TempSpace;
char NewTexType, MaxParams = 0, MaxParamTextures = 0, TextureNumber = -1, UseTexture = -1, SizeRead = 0, CenterRead = 0, VelocityRead = 0,
	FalloffRead = 0, ParamsRead = 0, RotationRead = 0, TempChar;
RasterAttribute *MyAttr;
GeoRegister GeoReg(NULL);

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
					case WCS_TEXTURE_BROWSEDATA:
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
					case WCS_TEXTURE_SAVEDMAXPARAMS:
						{
						BytesRead = ReadBlock(ffile, (char *)&MaxParams, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_SAVEDMAXPARAMTEXTURES:
						{
						BytesRead = ReadBlock(ffile, (char *)&MaxParamTextures, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_ANTIALIAS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Antialias, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_SELFOPACITY:
						{
						BytesRead = ReadBlock(ffile, (char *)&SelfOpacity, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_AASAMPLES:
						{
						BytesRead = ReadBlock(ffile, (char *)&AASamples, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_APPLYTOECOSYS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ApplyToEcosys, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_APPLYTOCOLOR:
						{
						BytesRead = ReadBlock(ffile, (char *)&ApplyToColor, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_ALIGNTOVECTOR:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempChar, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempChar)
							CoordSpace = WCS_TEXTURE_COORDSPACE_VECTOR_ALIGNED;
						break;
						}
					case WCS_TEXTURE_LATLONBOUNDS:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempChar, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempChar && ! WCS_TEXTURETYPE_LATLONBNDSILLEGAL(TexType))
							CoordSpace = WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED;
						break;
						}
					case WCS_TEXTURE_USEUVWCOORDS:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempChar, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempChar)
							SetCoordSpace(WCS_TEXTURE_COORDSPACE_VERTEX_UVW, FALSE);
						break;
						}
					case WCS_TEXTURE_TEXAXIS:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexAxis, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_IMAGENEG:
						{
						BytesRead = ReadBlock(ffile, (char *)&ImageNeg, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_COORDSPACE:
						{
						BytesRead = ReadBlock(ffile, (char *)CoordSpaceName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if ((TempSpace = RootTexture::GetCoordSpaceNumberFromName(CoordSpaceName)) >= 0)
							CoordSpace = TempSpace;
						break;
						}
					case WCS_TEXTURE_REVERSEWIDTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReverseWidth, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_REVERSEHEIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReverseHeight, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_ALPHAONLY:
						{
						BytesRead = ReadBlock(ffile, (char *)&AlphaOnly, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_PIXELBLEND:
						{
						BytesRead = ReadBlock(ffile, (char *)&PixelBlend, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_QUANTIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Quantize, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_WRAPWIDTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&WrapWidth, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_WRAPHEIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)&WrapHeight, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_TILEWIDTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&TileWidth, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_FLIPWIDTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&FlipWidth, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_TILEHEIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)&TileHeight, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_FLIPHEIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)&FlipHeight, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_THREED:
						{
						BytesRead = ReadBlock(ffile, (char *)&ThreeD, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_V5MISC:
						{
						BytesRead = ReadBlock(ffile, (char *)&Misc, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						V5MiscParam = 1;
						break;
						}
					case WCS_TEXTURE_MISC:
						{
						BytesRead = ReadBlock(ffile, (char *)&Misc, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_ABSOLUTEOUTPUT:
						{
						BytesRead = ReadBlock(ffile, (char *)&AbsoluteOutput, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_MAPNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)MapName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_GEOREG:
						{
						BytesRead = GeoReg.Load(ffile, Size, ByteFlip);
						GeoRegLoaded = 1;
						break;
						}
					case WCS_TEXTURE_NORTH:
						{
						BytesRead = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].Load(ffile, Size, ByteFlip);
						GeoRegLoaded = 1;
						break;
						}
					case WCS_TEXTURE_SOUTH:
						{
						BytesRead = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_WEST:
						{
						BytesRead = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_EAST:
						{
						BytesRead = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_TEXFEATHER:
						{
						BytesRead = TexFeather.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_COLORGRADIENT:
						{
						BytesRead = ColorGrad.Load(ffile, Size, ByteFlip);
						ColorGrad.SetTexturePrev(this);
						break;
						}
					case WCS_TEXTURE_IMAGEID:
						{
						BytesRead = ReadBlock(ffile, (char *)&ImageID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (ImageID > 0 && (Img = new RasterShell))
							{
							GlobalApp->LoadToImageLib->MatchRasterSetShell(ImageID, Img, this);
							if (GeoRegLoaded && Img->GetRaster())
								{
								// add GeoRef attribute to Raster if needed and set coords
								// since this is an old version from a time when we only supported geographic coord
								// it isn't necessary to attach a coordinate system.
								if ((MyAttr = Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) ||
									(MyAttr = Img->GetRaster()->AddAttribute(WCS_RASTERSHELL_TYPE_GEOREF, NULL, NULL)))
									{
									GeoReg.Copy(&((GeoRefShell *)MyAttr->GetShell())->GeoReg, &GeoReg);
									((GeoRefShell *)MyAttr->GetShell())->BoundsType = WCS_GEOREFSHELL_BOUNDSTYPE_EDGES;
									} // if
								} // if
							} // if
						break;
						}
					case WCS_TEXTURE_TEXCENTER:
						{
						BytesRead = TexCenter[CenterRead].Load(ffile, Size, ByteFlip);
						CenterRead ++;
						break;
						}
					case WCS_TEXTURE_TEXFALLOFF:
						{
						BytesRead = TexFalloff[FalloffRead].Load(ffile, Size, ByteFlip);
						FalloffRead ++;
						break;
						}
					case WCS_TEXTURE_TEXVELOCITY:
						{
						BytesRead = TexVelocity[VelocityRead].Load(ffile, Size, ByteFlip);
						VelocityRead ++;
						break;
						}
					case WCS_TEXTURE_TEXROTATION:
						{
						BytesRead = TexRotation[RotationRead].Load(ffile, Size, ByteFlip);
						RotationRead ++;
						break;
						}
					case WCS_TEXTURE_TEXSIZE:
						{
						BytesRead = TexSize[SizeRead].Load(ffile, Size, ByteFlip);
						SizeRead ++;
						break;
						}

					case WCS_TEXTURE_PARAMNAME:
						{
						// this isn't being used for now, so just read and discard
						BytesRead = ReadBlock(ffile, (char *)TextureTypeStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_PARAMFLAGS:
						{
						if (ParamsRead < WCS_TEXTURE_MAXPARAMS)
							BytesRead = ReadBlock(ffile, (char *)&ParamFlags[ParamsRead], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_TEXTURE_TEXPARAM:
						{
						if (ParamsRead < WCS_TEXTURE_MAXPARAMS)
							{
							BytesRead = TexParam[ParamsRead].Load(ffile, Size, ByteFlip);
							if (V5MiscParam && TexType == WCS_TEXTURE_TYPE_TERRAINPARAM && 
								(ParamsRead == WCS_TEXTURE_TERRAINPARAM_INPUTLOW || ParamsRead == WCS_TEXTURE_TERRAINPARAM_INPUTHIGH) && 
								Misc == WCS_TEXTURE_TERRAINPARAM_ZDISTANCE)
								{
								TexParam[ParamsRead].ScaleValues(1000.0);	// V5 and earlier stored this as kilometers, now it is meters
								} // if
							ParamsRead ++;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}

					case WCS_TEXTURE_TEXTURENUMBER:
						{
						BytesRead = ReadBlock(ffile, (char *)&TextureNumber, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_TEXTURE_TEXTURETYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)TextureTypeStr, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if ((NewTexType = RootTexture::IdentifyTextureType(TextureTypeStr)) >= 0)
							{
							if (TextureNumber >= 0 && TextureNumber < WCS_TEXTURE_MAXPARAMTEXTURES)
								Tex[TextureNumber] = RootTexture::NewTexture(this, NULL, NewTexType, TextureNumber);
							} // if
						break;
						}
					case WCS_TEXTURE_USETEXTURE:
						{
						if (TextureNumber >= 0 && TextureNumber < WCS_TEXTURE_MAXPARAMTEXTURES && Tex[TextureNumber])
							{
							BytesRead = ReadBlock(ffile, (char *)&UseTexture, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
							}
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_TEXTURE_TEXTURE:
						{
						if (TextureNumber >= 0 && TextureNumber < WCS_TEXTURE_MAXPARAMTEXTURES && Tex[TextureNumber])
							{
							BytesRead = Tex[TextureNumber]->Load(ffile, Size, ByteFlip);
							Tex[TextureNumber]->Prev = this;
							if (UseTexture >= 0)
								Tex[TextureNumber]->Enabled = (UseTexture && Tex[TextureNumber]->Enabled);
							if (TextureNumber == WCS_TEXTURE_STRATAFUNC || TextureNumber == WCS_TEXTURE_BLENDINGFUNC)
								Tex[TextureNumber]->ApplyToColor = 0;	// seems v4 set this to 1 which is bad
							Tex[TextureNumber]->InitNoise();	// do this in case any of the values loaded differ from defaults
							Tex[TextureNumber]->InitBasis();
							Tex[TextureNumber]->SetDimensionUnits();
							}
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						TextureNumber = -1;		// set it to -1 so don't read it again
						UseTexture = -1;
						break;
						}
					case WCS_TEXTURE_CURVEADP:
						{
						if (TexType == WCS_TEXTURE_TYPE_CUSTOMCURVE && ((CustomCurveTexture *)this)->CurveADP)
							BytesRead = ((CustomCurveTexture *)this)->CurveADP->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_TEXTURE_INCLUDEEXCLUDE:
						{
						if (InclExcl)
							BytesRead = InclExcl->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_TEXTURE_INCLUDEEXCLUDETYPE:
						{
						if (InclExclType)
							BytesRead = InclExclType->Load(ffile, Size, ByteFlip);
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

if (GeoRegLoaded && Img && Img->GetRaster() && CoordSpace == WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED)
	{
	// add GeoRef attribute to Raster if needed and set coords
	// since this is an old version from a time when we only supported geographic coord
	// it isn't necessary to attach a coordinate system.
	if ((MyAttr = Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) ||
		(MyAttr = Img->GetRaster()->AddAttribute(WCS_RASTERSHELL_TYPE_GEOREF, NULL, NULL)))
		{
		GeoReg.Copy(&((GeoRefShell *)MyAttr->GetShell())->GeoReg, &GeoReg);
		} // if
	} // if

return (TotalRead);

} // Texture::Load

/*===========================================================================*/

unsigned long Texture::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
unsigned long ImageID;
char Ct, MaxParams = WCS_TEXTURE_MAXPARAMS, MaxParamTextures = WCS_TEXTURE_MAXPARAMTEXTURES;

if (BrowseInfo)
	{
	ItemTag = WCS_TEXTURE_BROWSEDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_SAVEDMAXPARAMS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&MaxParams)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_SAVEDMAXPARAMTEXTURES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&MaxParamTextures)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_ANTIALIAS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Antialias)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_SELFOPACITY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&SelfOpacity)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_AASAMPLES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AASamples)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_APPLYTOECOSYS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ApplyToEcosys)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_APPLYTOCOLOR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ApplyToColor)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_PALCOLOR1, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&PalColor[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_PALCOLOR2, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&PalColor[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_TEXAXIS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TexAxis)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_IMAGENEG, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ImageNeg)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_COORDSPACE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(RootTexture::GetCoordSpaceName(CoordSpace)) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)RootTexture::GetCoordSpaceName(CoordSpace))) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_REVERSEWIDTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReverseWidth)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_REVERSEHEIGHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReverseHeight)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_ALPHAONLY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AlphaOnly)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_PIXELBLEND, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PixelBlend)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_QUANTIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Quantize)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_WRAPWIDTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&WrapWidth)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_WRAPHEIGHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&WrapHeight)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_TILEWIDTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TileWidth)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_FLIPWIDTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FlipWidth)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_TILEHEIGHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TileHeight)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_FLIPHEIGHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FlipHeight)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_THREED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ThreeD)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_MISC, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Misc)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_ABSOLUTEOUTPUT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AbsoluteOutput)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_MAPNAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(MapName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)MapName)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_TEXTURE_TEXFEATHER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = TexFeather.Save(ffile))
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

// raster shell
if (Img && (ImageID = Img->GetRasterID()) > 0)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_IMAGEID, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
		WCS_BLOCKTYPE_LONGINT, (char *)&ImageID)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if


// tex center, falloff, velocity, size
for (Ct = 0; Ct < 3; Ct ++)
	{
	ItemTag = WCS_TEXTURE_TEXCENTER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = TexCenter[Ct].Save(ffile))
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

	ItemTag = WCS_TEXTURE_TEXFALLOFF + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = TexFalloff[Ct].Save(ffile))
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

	ItemTag = WCS_TEXTURE_TEXVELOCITY + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = TexVelocity[Ct].Save(ffile))
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

	ItemTag = WCS_TEXTURE_TEXSIZE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = TexSize[Ct].Save(ffile))
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

	ItemTag = WCS_TEXTURE_TEXROTATION + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = TexRotation[Ct].Save(ffile))
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


// params
for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMS; Ct ++)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_PARAMNAME, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(ParamName[Ct]) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)ParamName[Ct])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_PARAMFLAGS, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (char *)&ParamFlags[Ct])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	ItemTag = WCS_TEXTURE_TEXPARAM + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = TexParam[Ct].Save(ffile))
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

for (Ct = 0; Ct < WCS_TEXTURE_MAXPARAMTEXTURES; Ct ++)
	{
	if (Tex[Ct])
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_TEXTURENUMBER, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (char *)&Ct)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_TEXTURE_TEXTURETYPE, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(RootTexture::GetTextureName(Tex[Ct]->GetTexType())) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)RootTexture::GetTextureName(Tex[Ct]->GetTexType()))) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		ItemTag = WCS_TEXTURE_TEXTURE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = Tex[Ct]->Save(ffile))
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
					} // if texture saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		}
	} // for

ItemTag = WCS_TEXTURE_COLORGRADIENT + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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
			} // if anim color saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

if (TexType == WCS_TEXTURE_TYPE_CUSTOMCURVE && ((CustomCurveTexture *)this)->CurveADP)
	{
	ItemTag = WCS_TEXTURE_CURVEADP + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = ((CustomCurveTexture *)this)->CurveADP->Save(ffile))
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
				} // if anim double profile saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if

if (InclExcl)
	{
	ItemTag = WCS_TEXTURE_INCLUDEEXCLUDE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = InclExcl->Save(ffile))
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
				} // if Include/Exclude List saved
			else
				goto WriteError;
			} // if size written
		else
			goto WriteError;
		} // if tag written
	else
		goto WriteError;
	} // if

if (InclExclType)
	{
	ItemTag = WCS_TEXTURE_INCLUDEEXCLUDETYPE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = InclExclType->Save(ffile))
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
				} // if Include/Exclude Type List saved
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

} // Texture::Save

/*===========================================================================*/
/*===========================================================================*/

// coordinates as doubles version implemented as inline in Raster.h
// double Raster::SampleManagedByteDouble3_DXDY(double *Output, double SampleXLow, double SampleYLow, int &Abort)

double Raster::SampleManagedByteDouble3(double *Output, unsigned long SampleXCoord, unsigned long SampleYCoord, int &Abort)
{
#ifdef WCS_IMAGE_MANAGEMENT
Raster *FoundTile = NULL, *TileSearch = NULL, *ClearMe = NULL, *CacheEnd = NULL, *LoadInto = NULL, *FreshTile = NULL;
long TileCell;
unsigned int CreateAtHead = 0;
char filename[256];
RasterAttribute *MyAttr;
ImageManagerShell *IMS;
unsigned long PrefTileSizeX, PrefTileSizeY;
// Abort must be preinitialized to 0

if(MyAttr = MatchAttribute(WCS_RASTERSHELL_TYPE_IMAGEMANAGER))
	{
	if(!(IMS = (ImageManagerShell *)MyAttr->Shell))
		{
		// display pink as noticable error code
		Output[0] = 1.0;
		Output[1] = .5;
		Output[2] = .5;
		return(1.0);
		} // if
	} // if
else
	{
	// display pink as noticable error code
	Output[0] = 1.0;
	Output[1] = .5;
	Output[2] = .5;
	return(1.0);
	} // else

PrefTileSizeX = IMS->GetPreferredTileSizeX();
PrefTileSizeY = IMS->GetPreferredTileSizeY();

if(TileCache)
	{ // search for proper tile
	for(TileSearch = TileCache; TileSearch; TileSearch = TileSearch->CacheNext)
		{
		if(!TileSearch->CacheNext)
			{
			CacheEnd = TileSearch;
			} // if
		// NativeTileWidth and Height in TileCache Rasters are actually X and Y offset
		// no safegaurding against reading coords outside of image and therefore off edge of last tile(s)
		if(TileSearch->Next == this) // does this tile even belong to our image?
			{
			if((SampleXCoord >= TileSearch->NativeTileWidth) && (SampleXCoord < TileSearch->NativeTileWidth + PrefTileSizeX) &&
			 (SampleYCoord >= TileSearch->NativeTileHeight) && (SampleYCoord < TileSearch->NativeTileHeight + PrefTileSizeY))
				{ // it's within this tile, go with it
				FoundTile = TileSearch;
				// move this tile to the head of the list for MRU ordering (if necessary)
				if(FoundTile != TileCache)
					{
					if(FoundTile->CachePrev)
						{  // remove us from our current position in list
						FoundTile->CachePrev->CacheNext = FoundTile->CacheNext;
						} // if
					if(FoundTile->CacheNext)
						{ // remove us from our current position in list
						FoundTile->CacheNext->CachePrev = FoundTile->CachePrev; 
						} // if
					// add us at beginning of list
					FoundTile->CachePrev = NULL;
					FoundTile->CacheNext = TileCache;
					TileCache->CachePrev = FoundTile;
					TileCache = FoundTile;
					} // if
				break;
				} // if
			} // if
		} // for
	} // if

if(!FoundTile)
	{
	if(!TileCache)
		{ // no tiles in cache yet, create one to load into
		CreateAtHead = 1;
		} // if
	if(TileCache)
		{
		unsigned long PredictedTileSize, PredictedClearSize;
		PredictedTileSize = PrefTileSizeX * PrefTileSizeY * ByteBands;
		if(CacheCurrentMemorySize + PredictedTileSize > QueryMaxCacheSize())
			{ // need to unload a tile before we can load a new one
			// might really need to unload more than one, but that would be
			// performance-inhibiting, so we'll unload another next time around
			// if we still need to then.
			if(TileCache)
				{
				// remove some entries from the list until the next one removed/cleared will satisfy our memory limits
				if(CacheEnd)
					{
					for (;;) // exit via break
						{
						PredictedClearSize = CacheEnd->Rows * CacheEnd->Cols * CacheEnd->ByteBands;
						if(CacheCurrentMemorySize + PredictedTileSize - PredictedClearSize <= QueryMaxCacheSize())
							{
							ClearMe = CacheEnd;
							break;
							} // if
						else
							{ // nuke oldest tile, try again
							CacheCurrentMemorySize -= PredictedClearSize; // lower current cache size in anticipation
							if(CacheEnd != TileCache) // are we not the last remaining tile?
								{
								Raster *ToDelete = NULL;
								// remove from current position
								if(CacheEnd->CacheNext) // unlikely, but here for completeness, CacheNext is probably NULL
									{
									CacheEnd->CacheNext->CachePrev = CacheEnd->CachePrev;
									} // if
								if(CacheEnd->CachePrev)
									{
									CacheEnd->CachePrev->CacheNext = CacheEnd->CacheNext; // CacheNext is most likely NULL
									} // if
								ToDelete = CacheEnd;
								CacheEnd = ToDelete->CachePrev; // work backward to see if we'll need to nix another
								delete ToDelete; // delete us
								ToDelete = NULL;
								} // if
							else
								{ // we're the only remaining tile, schedule us for clearing
								ClearMe = TileCache;
								break;
								} // else
							} // else
						} // for
					} // if
				else
					{
					// should NOT happen
					CacheEnd = NULL;
					} // else
				} // if
			else
				{
				// cache is empty, can't unload any more
				} // else
			} // if
		else
			{
			// still room in cache to add more tiles, create one to load into it
			CreateAtHead = 1;
			} // if

		// clear any tile scheduled for such treatment
		if(ClearMe)
			{ // already at max number of tiles permitted in cache
			// unload any data in this tile and load new desired tile
			LoadInto = ClearMe;
			// decrease cache size to reflect unloading this tile
			CacheCurrentMemorySize -= ((ClearMe->Rows * ClearMe->Cols) * ClearMe->ByteBands);
			ClearMe->FreeAllBands(WCS_RASTER_LONGEVITY_FORCEFREE);
			if(TileCache != ClearMe) // are we already first?
				{
				// move this tile to head of list for MRU ordering
				// remove from current position
				if(ClearMe->CacheNext)
					{
					ClearMe->CacheNext->CachePrev = ClearMe->CachePrev;
					} // if
				if(ClearMe->CachePrev)
					{
					ClearMe->CachePrev->CacheNext = ClearMe->CacheNext;
					} // if
				// put us at the front of the MRU cache list
				ClearMe->CachePrev = NULL;
				ClearMe->CacheNext = TileCache;
				TileCache->CachePrev = ClearMe;
				TileCache = ClearMe;
				} // if
			} // else
		} // if TileCache

	// do we need to create a new tile to load into?
	if(CreateAtHead)
		{
		// create empty tile at head, put into LoadInto for subsequent loading
		if(FreshTile = new Raster)
			{
			// mark it as belonging to our host Raster
			FreshTile->Next = this;
			if(TileCache)
				{
				TileCache->CachePrev = FreshTile;
				} // if
			FreshTile->CacheNext = TileCache; // maintain linked list chain
			FreshTile->CachePrev = NULL;
			TileCache = FreshTile; // put our new tile at the head of the MRU list
			LoadInto = FreshTile;
			} // if
		} // if

	if(LoadInto)
		{
		ImageCacheControl TileCacheControl;

		TileCacheControl.LoadingSubTile = 1; // so loader knows what's up
		if(PrefTileSizeX == 0) PrefTileSizeX = 256;
		if(PrefTileSizeY == 0) PrefTileSizeY = 256;
		TileCacheControl.LoadXOri = ((int)(SampleXCoord / PrefTileSizeX)) * PrefTileSizeX; // X origin of tile we're within
		TileCacheControl.LoadYOri = ((int)(SampleYCoord / PrefTileSizeY)) * PrefTileSizeY; // Y origin of tile we're within
		TileCacheControl.LoadWidth = PrefTileSizeX;
		TileCacheControl.LoadHeight = PrefTileSizeY;
		// prevent requesting a tile that extends beyond the image boundaries
		if(TileCacheControl.LoadXOri + TileCacheControl.LoadWidth > (unsigned)Cols)
			{
			TileCacheControl.LoadWidth = (unsigned)Cols - TileCacheControl.LoadXOri;
			} // if
		if(TileCacheControl.LoadYOri + TileCacheControl.LoadHeight > (unsigned)Rows)
			{
			TileCacheControl.LoadHeight = (unsigned)Rows - TileCacheControl.LoadYOri;
			} // if

		LoadInto->NativeTileWidth = LoadInto->NativeTileHeight = 0; // prevent future problems if load fails for some reason

		// load desired image data into LoadInto
	#ifdef WCS_BUILD_REMOTEFILE_SUPPORT
		/*if*/ (PAF.GetValidPathAndName(filename));
	#else // !WCS_BUILD_REMOTEFILE_SUPPORT
		if(PAF.GetValidPathAndName(filename))
	#endif // !WCS_BUILD_REMOTEFILE_SUPPORT
		if(LoadRasterImage(filename, LoadInto, 1, &TileCacheControl))
			{
			FoundTile = LoadInto;
			// mark it as belonging to our host Raster
			FoundTile->Next = this;
			// record offsets in LoadInto->NativeTileWidth and LoadInto->NativeTileHeight for future searching of loaded tiles
			LoadInto->NativeTileWidth = TileCacheControl.LoadXOri;
			LoadInto->NativeTileHeight = TileCacheControl.LoadYOri;
			CacheCurrentMemorySize += ((LoadInto->Rows * LoadInto->Cols) * LoadInto->ByteBands);
			} // if
		else
			{
			LoadInto->Rows = LoadInto->Cols = 0;
			FoundTile = NULL;
			} // else 
		} // if
	
	// discard any tile we added at the head that failed to load
	if(CreateAtHead && FreshTile && FoundTile == NULL)
		{
		TileCache = FreshTile->CacheNext;
		if(FreshTile->CacheNext)
			{
			FreshTile->CacheNext->CachePrev = NULL;
			} // if
		delete FreshTile;
		FreshTile = NULL;
		} // if
	} // if



if(FoundTile)
	{
	// FoundTile->NativeTileHeight and FoundTile->NativeTileWidth are actually X and Y offsets of this tile, not tile sizes
	TileCell = ((SampleYCoord - FoundTile->NativeTileHeight) * FoundTile->Cols) + (SampleXCoord - FoundTile->NativeTileWidth);
	if (FoundTile->ByteMap[WCS_RASTER_IMAGE_BAND_GREEN] && FoundTile->ByteMap[WCS_RASTER_IMAGE_BAND_BLUE])
		{
		Output[0] = FoundTile->ByteMap[WCS_RASTER_IMAGE_BAND_RED][TileCell] * (1.0 / 255.0);
		Output[1] = FoundTile->ByteMap[WCS_RASTER_IMAGE_BAND_GREEN][TileCell] * (1.0 / 255.0);
		Output[2] = FoundTile->ByteMap[WCS_RASTER_IMAGE_BAND_BLUE][TileCell] * (1.0 / 255.0);
		} // if
	else
		{
		if(FoundTile->ByteMap[WCS_RASTER_IMAGE_BAND_RED])
			{
			Output[0] = Output[1] = Output[2] = FoundTile->ByteMap[WCS_RASTER_IMAGE_BAND_RED][TileCell] * (1.0 / 255.0);
			} // if
		else
			{
			// display pink as noticable error code
			Output[0] = 1.0;
			Output[1] = .5;
			Output[2] = .5;
			} // else
		} // else
	if (FoundTile->AltFloatMap && FoundTile->GetAlphaStatus())
		return (FoundTile->AltFloatMap[TileCell]);
	} // if
else
#endif // WCS_IMAGE_MANAGEMENT
	{
	// display pink as noticable error code
	Output[0] = 1.0;
	Output[1] = .5;
	Output[2] = .5;
	} // else
return (1.0);

} // Raster::SampleManagedByteDouble3

/*===========================================================================*/


double Raster::SampleByteDouble3(double *Output, double SampleXLow, double SampleYLow, int &Abort)
{

// Abort must be preinitialized to 0

#ifdef WCS_IMAGE_MANAGEMENT
if(QueryImageManagementEnabled())
	{
	SampleManagedByteDouble3_DXDY(Output, SampleXLow, SampleYLow, Abort);
	return(1.0);
	} // if
#endif // WCS_IMAGE_MANAGEMENT

// if not loaded, load to render
if (! LoadToRender(GlobalApp->MainProj->Interactive->GetActiveTime(), GlobalApp->MainProj->Interactive->GetFrameRate(), WCS_RASTER_INITFLAGS_IMAGELOADED | WCS_RASTER_INITFLAGS_DOWNSAMPLED))
	{
	Output[0] = Output[1] = Output[2] = 0.0;
	Abort = 1;
	return (1.0);
	} // if

// WARNING! No test for valid sample range
RasterCell = quickftol(SampleYLow * (Rows - .0001)) * Cols + quickftol(SampleXLow * (Cols - .0001));

if (ByteMap[WCS_RASTER_IMAGE_BAND_GREEN] && ByteMap[WCS_RASTER_IMAGE_BAND_BLUE])
	{
	Output[0] = ByteMap[WCS_RASTER_IMAGE_BAND_RED][RasterCell] * (1.0 / 255.0);
	Output[1] = ByteMap[WCS_RASTER_IMAGE_BAND_GREEN][RasterCell] * (1.0 / 255.0);
	Output[2] = ByteMap[WCS_RASTER_IMAGE_BAND_BLUE][RasterCell] * (1.0 / 255.0);
	} // if
else
	{
	Output[2] = Output[1] = Output[0] = ByteMap[WCS_RASTER_IMAGE_BAND_RED][RasterCell] * (1.0 / 255.0);
	} // else

if (AltFloatMap && GetAlphaStatus())
	return (AltFloatMap[RasterCell]);
return (1.0);

} // Raster::SampleByteDouble3

/*===========================================================================*/

double Raster::SampleBlendDouble3(double *Output, double SampleXLow, double SampleYLow, int &Abort)
{
double Wt[4], D[4], SumWt, dRow, dCol, AlphaVal;

// Abort must be preinitialized to 0

// if not loaded, load to render
#ifdef WCS_IMAGE_MANAGEMENT
if(!QueryImageManagementEnabled())
#endif // WCS_IMAGE_MANAGEMENT
	{
	if (! LoadToRender(GlobalApp->MainProj->Interactive->GetActiveTime(), GlobalApp->MainProj->Interactive->GetFrameRate(), WCS_RASTER_INITFLAGS_IMAGELOADED | WCS_RASTER_INITFLAGS_DOWNSAMPLED))
		{
		Output[2] = Output[1] = Output[0] = 0.0;
		Abort = 1;
		return (1.0);
		} // if
	} // if

// WARNING! No test for valid sample range
dRow = (SampleYLow * (Rows - .0001));
dCol = (SampleXLow * (Cols - .0001));
Row = quickftol(dRow);
Col = quickftol(dCol);
RasterCell = Row * Cols + Col;

D[0] = dRow - Row;
D[1] = dCol - Col;
D[2] = 1.0 - D[0];
D[3] = 1.0 - D[1];

Wt[0] = D[2] * D[3];
Wt[1] = D[2] * D[1];
Wt[2] = D[0] * D[3];
Wt[3] = D[0] * D[1];

// for performance and clarity reasons, I've made a complete copy of this code below
// for image management purposes
#ifdef WCS_IMAGE_MANAGEMENT
if(!QueryImageManagementEnabled())
#endif // WCS_IMAGE_MANAGEMENT
	{ // this is the original un-management code, placed first to grant it the better execution time of not having the if branch
	if (ByteMap[WCS_RASTER_IMAGE_BAND_GREEN] && ByteMap[WCS_RASTER_IMAGE_BAND_BLUE])
		{
		Output[0] = Wt[0] * ByteMap[WCS_RASTER_IMAGE_BAND_RED][RasterCell] * (1.0 / 255.0);
		Output[1] = Wt[0] * ByteMap[WCS_RASTER_IMAGE_BAND_GREEN][RasterCell] * (1.0 / 255.0);
		Output[2] = Wt[0] * ByteMap[WCS_RASTER_IMAGE_BAND_BLUE][RasterCell] * (1.0 / 255.0);
		SumWt = Wt[0];
		if (Col < Cols - 1)
			{
			Output[0] += Wt[1] * ByteMap[WCS_RASTER_IMAGE_BAND_RED][RasterCell + 1] * (1.0 / 255.0);
			Output[1] += Wt[1] * ByteMap[WCS_RASTER_IMAGE_BAND_GREEN][RasterCell + 1] * (1.0 / 255.0);
			Output[2] += Wt[1] * ByteMap[WCS_RASTER_IMAGE_BAND_BLUE][RasterCell + 1] * (1.0 / 255.0);
			SumWt += Wt[1];
			} // if
		if (Row < Rows - 1)
			{
			Output[0] += Wt[2] * ByteMap[WCS_RASTER_IMAGE_BAND_RED][RasterCell + Cols] * (1.0 / 255.0);
			Output[1] += Wt[2] * ByteMap[WCS_RASTER_IMAGE_BAND_GREEN][RasterCell + Cols] * (1.0 / 255.0);
			Output[2] += Wt[2] * ByteMap[WCS_RASTER_IMAGE_BAND_BLUE][RasterCell + Cols] * (1.0 / 255.0);
			SumWt += Wt[2];
			} // if
		if (Col < Cols - 1 && Row < Rows - 1)
			{
			Output[0] += Wt[3] * ByteMap[WCS_RASTER_IMAGE_BAND_RED][RasterCell + Cols + 1] * (1.0 / 255.0);
			Output[1] += Wt[3] * ByteMap[WCS_RASTER_IMAGE_BAND_GREEN][RasterCell + Cols + 1] * (1.0 / 255.0);
			Output[2] += Wt[3] * ByteMap[WCS_RASTER_IMAGE_BAND_BLUE][RasterCell + Cols + 1] * (1.0 / 255.0);
			SumWt += Wt[3];
			} // if
		} // if
	else
		{
		Output[0] = Wt[0] * ByteMap[WCS_RASTER_IMAGE_BAND_RED][RasterCell] * (1.0 / 255.0);
		SumWt = Wt[0];
		if (Col < Cols - 1)
			{
			Output[0] += Wt[1] * ByteMap[WCS_RASTER_IMAGE_BAND_RED][RasterCell + 1] * (1.0 / 255.0);
			SumWt += Wt[1];
			} // if
		if (Row < Rows - 1)
			{
			Output[0] += Wt[2] * ByteMap[WCS_RASTER_IMAGE_BAND_RED][RasterCell + Cols] * (1.0 / 255.0);
			SumWt += Wt[2];
			} // if
		if (Col < Cols - 1 && Row < Rows - 1)
			{
			Output[0] += Wt[3] * ByteMap[WCS_RASTER_IMAGE_BAND_RED][RasterCell + Cols + 1] * (1.0 / 255.0);
			SumWt += Wt[3];
			} // if
		Output[1] = Output[2] = Output[0];
		} // else

	if (SumWt > 0.0 && SumWt != 1.0)
		{
		Output[0] /= SumWt;
		Output[1] /= SumWt;
		Output[2] /= SumWt;
		} // if

	if (AltFloatMap && GetAlphaStatus())
		{
		AlphaVal = Wt[0] * AltFloatMap[RasterCell];
		if (Col < Cols - 1)
			{
			AlphaVal += Wt[1] * AltFloatMap[RasterCell + 1];
			} // if
		if (Row < Rows - 1)
			{
			AlphaVal += Wt[2] * AltFloatMap[RasterCell + Cols];
			} // if
		if (Col < Cols - 1 && Row < Rows - 1)
			{
			AlphaVal += Wt[3] * AltFloatMap[RasterCell + Cols + 1];
			} // if
		if (SumWt > 0.0 && SumWt != 1.0)
			AlphaVal /= SumWt;
		return (AlphaVal);
		} // if
	} // if
#ifdef WCS_IMAGE_MANAGEMENT
else
	{
	double CellOutput[3];
	// must determine if green and blue maps are available
	if (ByteBands > 1) // if not monochrome, treat as color
		{
		//Output[0] = Wt[0] * ByteMap[WCS_RASTER_IMAGE_BAND_RED][Cell] * (1.0 / 255.0);
		//Output[1] = Wt[0] * ByteMap[WCS_RASTER_IMAGE_BAND_GREEN][Cell] * (1.0 / 255.0);
		//Output[2] = Wt[0] * ByteMap[WCS_RASTER_IMAGE_BAND_BLUE][Cell] * (1.0 / 255.0);
		AlphaVal = Wt[0] * SampleManagedByteDouble3(CellOutput, Col, Row, Abort);
		Output[0] = Wt[0] * CellOutput[0];
		Output[1] = Wt[0] * CellOutput[1];
		Output[2] = Wt[0] * CellOutput[2];
		SumWt = Wt[0];
		if (Col < Cols - 1)
			{
			//Output[0] += Wt[1] * ByteMap[WCS_RASTER_IMAGE_BAND_RED][Cell + 1] * (1.0 / 255.0);
			//Output[1] += Wt[1] * ByteMap[WCS_RASTER_IMAGE_BAND_GREEN][Cell + 1] * (1.0 / 255.0);
			//Output[2] += Wt[1] * ByteMap[WCS_RASTER_IMAGE_BAND_BLUE][Cell + 1] * (1.0 / 255.0);
			AlphaVal += Wt[1] * SampleManagedByteDouble3(CellOutput, Col + 1, Row, Abort);
			Output[0] += Wt[1] * CellOutput[0];
			Output[1] += Wt[1] * CellOutput[1];
			Output[2] += Wt[1] * CellOutput[2];
			SumWt += Wt[1];
			} // if
		if (Row < Rows - 1)
			{
			//Output[0] += Wt[2] * ByteMap[WCS_RASTER_IMAGE_BAND_RED][Cell + Cols] * (1.0 / 255.0);
			//Output[1] += Wt[2] * ByteMap[WCS_RASTER_IMAGE_BAND_GREEN][Cell + Cols] * (1.0 / 255.0);
			//Output[2] += Wt[2] * ByteMap[WCS_RASTER_IMAGE_BAND_BLUE][Cell + Cols] * (1.0 / 255.0);
			AlphaVal += Wt[2] * SampleManagedByteDouble3(CellOutput, Col, Row + 1, Abort);
			Output[0] += Wt[2] * CellOutput[0];
			Output[1] += Wt[2] * CellOutput[1];
			Output[2] += Wt[2] * CellOutput[2];
			SumWt += Wt[2];
			} // if
		if (Col < Cols - 1 && Row < Rows - 1)
			{
			//Output[0] += Wt[3] * ByteMap[WCS_RASTER_IMAGE_BAND_RED][Cell + Cols + 1] * (1.0 / 255.0);
			//Output[1] += Wt[3] * ByteMap[WCS_RASTER_IMAGE_BAND_GREEN][Cell + Cols + 1] * (1.0 / 255.0);
			//Output[2] += Wt[3] * ByteMap[WCS_RASTER_IMAGE_BAND_BLUE][Cell + Cols + 1] * (1.0 / 255.0);
			AlphaVal += Wt[3] * SampleManagedByteDouble3(CellOutput, Col + 1, Row + 1, Abort);
			Output[0] += Wt[3] * CellOutput[0];
			Output[1] += Wt[3] * CellOutput[1];
			Output[2] += Wt[3] * CellOutput[2];
			SumWt += Wt[3];
			} // if
		} // if
	else
		{
		//Output[0] = Wt[0] * ByteMap[WCS_RASTER_IMAGE_BAND_RED][Cell] * (1.0 / 255.0);
		AlphaVal = Wt[0] * SampleManagedByteDouble3(CellOutput, Col, Row, Abort);
		Output[0] = Wt[0] * CellOutput[0];
		SumWt = Wt[0];
		if (Col < Cols - 1)
			{
			//Output[0] += Wt[1] * ByteMap[WCS_RASTER_IMAGE_BAND_RED][Cell + 1] * (1.0 / 255.0);
			AlphaVal += Wt[1] * SampleManagedByteDouble3(CellOutput, Col + 1, Row, Abort);
			Output[0] += Wt[1] * CellOutput[0];
			SumWt += Wt[1];
			} // if
		if (Row < Rows - 1)
			{
			//Output[0] += Wt[2] * ByteMap[WCS_RASTER_IMAGE_BAND_RED][Cell + Cols] * (1.0 / 255.0);
			AlphaVal += Wt[2] * SampleManagedByteDouble3(CellOutput, Col, Row + 1, Abort);
			Output[0] += CellOutput[0];
			SumWt += Wt[2];
			} // if
		if (Col < Cols - 1 && Row < Rows - 1)
			{
			//Output[0] += Wt[3] * ByteMap[WCS_RASTER_IMAGE_BAND_RED][Cell + Cols + 1] * (1.0 / 255.0);
			AlphaVal += Wt[3] * SampleManagedByteDouble3(CellOutput, Col + 1, Row + 1, Abort);
			Output[0] += Wt[3] * CellOutput[0];
			SumWt += Wt[3];
			} // if
		Output[1] = Output[2] = Output[0];
		} // else

	if (SumWt > 0.0 && SumWt != 1.0)
		{
		SumWt = 1.0 / SumWt; // multiplicative inverse instead of four divides
		Output[0] *= SumWt;
		Output[1] *= SumWt;
		Output[2] *= SumWt;
		AlphaVal *= SumWt;
		} // if

// this is managed inline above
/*
	if (AltFloatMap && GetAlphaStatus())
		{
		AlphaVal = Wt[0] * AltFloatMap[Cell];
		if (Col < Cols - 1)
			{
			AlphaVal += Wt[1] * AltFloatMap[Cell + 1];
			} // if
		if (Row < Rows - 1)
			{
			AlphaVal += Wt[2] * AltFloatMap[Cell + Cols];
			} // if
		if (Col < Cols - 1 && Row < Rows - 1)
			{
			AlphaVal += Wt[3] * AltFloatMap[Cell + Cols + 1];
			} // if
		if (SumWt > 0.0 && SumWt != 1.0)
			AlphaVal /= SumWt;
		return (AlphaVal);
		} // if
*/
	} // else
#endif // WCS_IMAGE_MANAGEMENT
return (1.0);

} // Raster::SampleBlendDouble3

/*===========================================================================*/

/*** old version
Raster *Raster::FindBestDownsampledVersion(double SampleWidth, double SampleHeight)
{
double PWx2, PHx2;
Raster *Source = this;

// if not loaded, load to render
if (! LoadToRender(GlobalApp->MainProj->Interactive->GetActiveTime(), GlobalApp->MainProj->Interactive->GetFrameRate(), WCS_RASTER_INITFLAGS_IMAGELOADED | WCS_RASTER_INITFLAGS_DOWNSAMPLED))
	{
	return (NULL);
	} // if

PWx2 = 2.0 / Cols;
PHx2 = 2.0 / Rows;

while (Source->Smaller && SampleWidth >= PWx2 && SampleHeight >= PHx2)
	{
	Source = Source->Smaller;
	PWx2 = 2.0 / Source->Cols;
	PHx2 = 2.0 / Source->Rows;
	} // while

Source->InitFlags = InitFlags;
return (Source);

} // Raster::FindBestDownsampledVersion
***/

Raster *Raster::FindBestDownsampledVersion(double SampleWidth, double SampleHeight)
{
//double PWx2, PHx2;
//float iWidth, iHeight;
Raster *Source = this;
long curCols, curRows;
//char dstr[256];

//iWidth =  float(1.0 / SampleWidth);
//iHeight = float(1.0 / SampleHeight);

// if not loaded, load to render
if (! LoadToRender(GlobalApp->MainProj->Interactive->GetActiveTime(), GlobalApp->MainProj->Interactive->GetFrameRate(), WCS_RASTER_INITFLAGS_IMAGELOADED | WCS_RASTER_INITFLAGS_DOWNSAMPLED))
	{
	return (NULL);
	} // if

//PWx2 = 2.0 / Cols;
//PHx2 = 2.0 / Rows;
curCols = Cols / 2;	// right shifts are machine dependent (sign fill vs. zero fill)
curRows = Rows / 2;

//sprintf(dstr, "Cols = %d, Rows = %d\n", Cols, Rows);
//DEBUGOUT(dstr);
//sprintf(dstr, "Cols = %d, Rows = %d\n", curCols, curRows);
//DEBUGOUT(dstr);

//while (Source->Smaller && SampleWidth >= PWx2 && SampleHeight >= PHx2)
while ((Source->Smaller) && (SampleWidth <= (double)curCols) && (SampleHeight <= (double)curRows))
	{
	Source = Source->Smaller;
	//PWx2 = 2.0 / Source->Cols;
	//PHx2 = 2.0 / Source->Rows;
	curCols = Source->Cols / 2;	// right shifts are machine dependent (sign fill vs. zero fill)
	curRows = Source->Rows / 2;
	//sprintf(dstr, "--> Cols = %d, Rows = %d\n", Source->Cols, Source->Rows);
	//DEBUGOUT(dstr);
	//sprintf(dstr, "--> curCols = %d, curRows = %d\n", curCols, curRows);
	//DEBUGOUT(dstr);
	} // while

Source->InitFlags = InitFlags;
return (Source);

} // Raster::FindBestDownsampledVersion

/*===========================================================================*/

double Raster::SampleRangeByteDouble3(double *Output, double SampleXLow, double SampleXHigh, double SampleYLow, 
	double SampleYHigh, int &Abort, int SelfMasking)
{
double SampleWidth, SampleHeight, PWx2, PHx2, PixelXLow, PixelXHigh, PixelYLow, PixelYHigh, wt, wtx, wty, SumWt, MaxWt;
#ifdef WCS_IMAGE_MANAGEMENT
double CellOutput[3], ManagedAlpha;
#endif // WCS_IMAGE_MANAGEMENT
Raster *Source = this;
long ColorImage, Py, Px, Pyp1, Pxp1, ManagedImage = 0;

// Outputs must be pre-initialized to 0
// Abort must be preinitialized to 0

// if not loaded, load to render
#ifdef WCS_IMAGE_MANAGEMENT
if(!QueryImageManagementEnabled())
#endif // WCS_IMAGE_MANAGEMENT
	{
	if (! LoadToRender(GlobalApp->MainProj->Interactive->GetActiveTime(), GlobalApp->MainProj->Interactive->GetFrameRate(), WCS_RASTER_INITFLAGS_IMAGELOADED | WCS_RASTER_INITFLAGS_DOWNSAMPLED))
		{
		Output[2] = Output[1] = Output[0] = 0.0;
		Abort = 1;
		return (1.0);
		} // if
	// check band presence to see if color
	ColorImage = (ByteMap[WCS_RASTER_IMAGE_BAND_GREEN] && ByteMap[WCS_RASTER_IMAGE_BAND_BLUE]);
	} // if
#ifdef WCS_IMAGE_MANAGEMENT
else
	{ // image management
	// rely on band count to see if color
	ColorImage = (ByteBands > 1);
	ManagedImage = 1;
	} // else
#endif // WCS_IMAGE_MANAGEMENT

// WARNING! No test for valid sample range
SampleWidth = SampleXHigh - SampleXLow;
SampleHeight = SampleYHigh - SampleYLow;
PWx2 = 2.0 / Cols;
PHx2 = 2.0 / Rows;

// Source->Smaller should be NULL if image management is on, and non-NULL if LoadToRender
// has been done. If Image Management is in effect, and we've been fully loaded anyway
// because some idiot is using us for foliage or somesuch) we might as well not use
// Image Management as the fully-loaded version will be faster and we've already paid for it
while (Source->Smaller && SampleWidth >= PWx2 && SampleHeight >= PHx2)
	{
	ManagedImage = 0;
	Source = Source->Smaller;
	PWx2 = 2.0 / Source->Cols;
	PHx2 = 2.0 / Source->Rows;
	} // while


PixelXLow = SampleXLow * (Source->Cols - .001);
PixelXHigh = SampleXHigh * (Source->Cols - .001);
PixelYLow = SampleYLow * (Source->Rows - .001);
PixelYHigh = SampleYHigh * (Source->Rows - .001);
MaxWt = (PixelYHigh - PixelYLow) * (PixelXHigh - PixelXLow);
SumWt = 0.0;

if (MaxWt > 0.0)
	{
	for (Py = quickftol(PixelYLow), Pyp1 = quickftol(PixelYLow + 1); Py < PixelYHigh && Py < Source->Rows; Py ++, Pyp1 ++)
		{
		if (Py < 0)
			continue;
		if ((wty = min((double)Pyp1, PixelYHigh) - max((double)Py, PixelYLow)) > 0.0)
			{
			for (Px = quickftol(PixelXLow), Pxp1 = quickftol(PixelXLow + 1); Px < PixelXHigh && Px < Source->Cols; Px ++, Pxp1 ++)
				{
				if (Px < 0)
					continue;
				RasterCell = Py * Source->Cols + Px;
				if ((wtx = min((double)Pxp1, PixelXHigh) - max((double)Px, PixelXLow)) > 0.0)
					{
					wt = wty * wtx;
					#ifdef WCS_IMAGE_MANAGEMENT
					if(ManagedImage)
						{
						ManagedAlpha = SampleManagedByteDouble3(CellOutput, Px, Py, Abort);
						Output[0] += (wt * CellOutput[0]);
						} // if
					else
					#endif // WCS_IMAGE_MANAGEMENT
						{
						Output[0] += (wt * Source->ByteMap[WCS_RASTER_IMAGE_BAND_RED][RasterCell] * (1.0 / 255.0));
						} // else not managed
					if (ColorImage)
						{
						#ifdef WCS_IMAGE_MANAGEMENT
						if(ManagedImage)
							{
							// already fetched from other bands as part of SampleManagedByteDouble3 call above, just transfer into weighted Output
							Output[1] += (wt * CellOutput[1]);
							Output[2] += (wt * CellOutput[2]);
							} // if
						else
						#endif // WCS_IMAGE_MANAGEMENT
							{
							Output[1] += (wt * Source->ByteMap[WCS_RASTER_IMAGE_BAND_GREEN][RasterCell] * (1.0 / 255.0));
							Output[2] += (wt * Source->ByteMap[WCS_RASTER_IMAGE_BAND_BLUE][RasterCell] * (1.0 / 255.0));
							} // else not managed
						} // if
					if (GetAlphaStatus())
						{
						#ifdef WCS_IMAGE_MANAGEMENT
						if(ManagedImage)
							{
							wt *= ManagedAlpha; // already fetched ManagedAlpha as part of SampleManagedByteDouble3 call above
							} // if
						else
						#endif // WCS_IMAGE_MANAGEMENT
							{
							if(Source->AltFloatMap)
								{
								wt *= Source->AltFloatMap[RasterCell];
								} // if
							} // else not managed
						} // if
					if (SelfMasking)
						{
						#ifdef WCS_IMAGE_MANAGEMENT
						if(ManagedImage)
							{
							if (CellOutput[0] != 0.0 || 
							 (ColorImage && (CellOutput[1] != 0.0 || CellOutput[2] != 0.0)))
								{
								SumWt += wt;
								} // if
							} // if
						else
						#endif // WCS_IMAGE_MANAGEMENT
							{
							if (Source->ByteMap[WCS_RASTER_IMAGE_BAND_RED][RasterCell] || 
							 (ColorImage && (Source->ByteMap[WCS_RASTER_IMAGE_BAND_GREEN][RasterCell] || Source->ByteMap[WCS_RASTER_IMAGE_BAND_BLUE][RasterCell])))
								{
								SumWt += wt;
								} // if
							} // else
						} // if 
					else
						SumWt += wt;
					} // if
				} // for
			} // if
		} // for

	if (! ColorImage)
		{
		Output[2] = Output[1] = Output[0];
		} // if

	MaxWt = 1.0 / MaxWt; // turn into multiplicative inverse to avoid four divides, below
	SumWt *= MaxWt;
	Output[0] *= MaxWt;
	Output[1] *= MaxWt;
	Output[2] *= MaxWt;
	} // if
else
	{
	if ((Px = quickftol(PixelXLow)) >= 0)
		{
		if ((Py = quickftol(PixelYLow)) >= 0)
			{
			RasterCell = Py * Source->Cols + Px;

			wt = 1.0;

			#ifdef WCS_IMAGE_MANAGEMENT
			if(ManagedImage)
				{
				ManagedAlpha = SampleManagedByteDouble3(CellOutput, Px, Py, Abort);
				Output[0] = CellOutput[0];
				} // if
			else
			#endif // WCS_IMAGE_MANAGEMENT
				{
				Output[0] = (Source->ByteMap[WCS_RASTER_IMAGE_BAND_RED][RasterCell] * (1.0 / 255.0));
				} // else not managed
			if (ColorImage)
				{
				#ifdef WCS_IMAGE_MANAGEMENT
				if(ManagedImage)
					{
					// already fetched from other bands as part of SampleManagedByteDouble3 call above, just transfer into weighted Output
					Output[1] = (wt * CellOutput[1]);
					Output[2] = (wt * CellOutput[2]);
					} // if
				else
				#endif // WCS_IMAGE_MANAGEMENT
					{
					Output[1] = (Source->ByteMap[WCS_RASTER_IMAGE_BAND_GREEN][RasterCell] * (1.0 / 255.0));
					Output[2] = (Source->ByteMap[WCS_RASTER_IMAGE_BAND_BLUE][RasterCell] * (1.0 / 255.0));
					} // else
				} // if
			if (GetAlphaStatus())
				{
				#ifdef WCS_IMAGE_MANAGEMENT
				if(ManagedImage)
					{
					wt *= ManagedAlpha;
					} // if
				else
				#endif // WCS_IMAGE_MANAGEMENT
					{
					if(Source->AltFloatMap)
						{
						wt *= Source->AltFloatMap[RasterCell];
						} // if
					} // else not managed
				} // if
			if (SelfMasking)
				{
				#ifdef WCS_IMAGE_MANAGEMENT
				if(ManagedImage)
					{
					if (CellOutput[0] != 0.0 || 
					 (ColorImage && (CellOutput[1] != 0.0 || CellOutput[2] != 0.0)))
						{
						SumWt += wt;
						} // if
					} // if
				else
				#endif // WCS_IMAGE_MANAGEMENT
					{
					if (Source->ByteMap[WCS_RASTER_IMAGE_BAND_RED][RasterCell] || 
					 (ColorImage && (Source->ByteMap[WCS_RASTER_IMAGE_BAND_GREEN][RasterCell] || Source->ByteMap[WCS_RASTER_IMAGE_BAND_BLUE][RasterCell])))
						{
						SumWt += wt;
						} // if
					} // else not managed
				} // if 
			else
				SumWt += wt;
			} // if
		} // if
	if (! ColorImage)
		{
		Output[2] = Output[1] = Output[0];
		} // if
	} // else

return (SumWt);

} // Raster::SampleRangeByteDouble3

/*===========================================================================*/
