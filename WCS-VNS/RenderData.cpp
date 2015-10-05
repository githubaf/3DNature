// RenderData.cpp
// Created from elements of RenderMaterials.cpp on 3/19/01 by Gary R. Huber
// Copyright 2001 by 3D Nature LLC. All rights reserved.

#include "stdafx.h"
#include "Render.h"
#include "Texture.h"
#include "Database.h"
#include "EffectsLib.h"
#include "Project.h"
#include "Interactive.h"
#include "VectorNode.h"

// RenderData will be used to store certain data members for use in Effects initialization
// so that ViewGUI doesn't need to create a whole Renderer just to use camera projections, etc.

RenderData::RenderData(Renderer *MyRenderer)
{

Rend = MyRenderer;
Cam = NULL;
Opt = NULL;
Interactive = NULL;
DBase = NULL;
EffectsBase = NULL;
ProjectBase = NULL;
DefCoords = NULL;
DefaultEnvironment = NULL;
Substances = NULL;
IsCamView = IsProcessing = ExagerateElevLines = IsTextureMap = ShadowMapInProgress = 0;
PanoPanel = Segment = StereoSide = SetupWidth = SetupHeight = Width = Height = 0;
NumPanels = NumSegments = 1;
PlanetRad = 6362683.195;
EarthLatScaleMeters = PlanetRad * TwoPi / 360.0;
RenderTime = ElevDatum = Exageration = EarthRotation = 0.0;
PixelAspect = 1.0;
FrameRate = 30.0;
TexRefLon = TexRefLat = TexRefElev = 0.0;
RefLonScaleMeters = 1.0;
ShadowMapDistanceOffset = 0.0;

} // RenderData::RenderData

/*===========================================================================*/

int RenderData::InitToView(EffectsLib *EffectsSource, Project *ProjectSource, Database *NewDB, InterCommon *NewInter, RenderOpt *NewOpt, Camera *NewCam, long ImageWidth, long CamSetupWidth)
{
PlanetOpt *DefPlanetOpt;

IsCamView = 1;
PanoPanel = Segment = 0;
NumPanels = NumSegments = 1;
StereoSide = 0;
if (ImageWidth > 0)
	{
	SetupWidth = CamSetupWidth;
	Width = ImageWidth;
	if (Interactive = NewInter)
		{
		RenderTime = Interactive->GetActiveTime();
		if ((EffectsBase = EffectsSource) && (ProjectBase = ProjectSource))
			{
			if (DBase = NewDB)
				{
				if (NewCam || (NewCam = (Camera *)EffectsSource->GetDefaultEffect(WCS_EFFECTSSUBCLASS_CAMERA, 1, NULL)))
					{
					Cam = NewCam;
					Opt = NewOpt;
					DefCoords = EffectsSource->FetchDefaultCoordSys();
					DefaultEnvironment = (EnvironmentEffect *)EffectsSource->GetDefaultEffect(WCS_EFFECTSSUBCLASS_ENVIRONMENT, 1, NULL);
					if (DefPlanetOpt = (PlanetOpt *)EffectsSource->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, NULL))
						{
						ExagerateElevLines = DefPlanetOpt->EcoExageration;
						PlanetRad = EffectsSource->GetPlanetRadius();
						EarthRotation = DefPlanetOpt->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_ROTATION].CurValue;
						EarthLatScaleMeters = PlanetRad * TwoPi / 360.0;
						ElevDatum = DefPlanetOpt->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue;
						Exageration = DefPlanetOpt->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].CurValue;
						if (Opt)
							{
							PixelAspect = Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue;
							FrameRate = Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue;
							} // if
						return (1);
						} // if
					} // if
				} // if
			} // if
		} // if
	} // if

return (0);

} // RenderData::InitToView

/*===========================================================================*/

void RenderData::InitFrameToRender(Renderer *Rend)
{

Cam = Rend->Cam;
Opt = Rend->Opt;
Interactive = Rend->ProjectBase->Interactive;
DBase = Rend->DBase;
EffectsBase = Rend->EffectsBase;
ProjectBase = Rend->ProjectBase;
DefCoords = Rend->DefCoords;
DefaultEnvironment = Rend->DefaultEnvironment;
IsCamView = Rend->IsCamView;
IsProcessing = Rend->IsProcessing;
PanoPanel = Rend->PanoPanel;
Segment = Rend->Segment;
NumPanels = Rend->NumPanels;
NumSegments = Rend->NumSegments;
StereoSide = Rend->StereoSide;
RenderTime = Rend->RenderTime;
ExagerateElevLines = Rend->ExagerateElevLines;
ShadowMapInProgress = Rend->ShadowMapInProgress;
PlanetRad = Rend->PlanetRad;
EarthRotation = Rend->EarthRotation;
EarthLatScaleMeters = Rend->EarthLatScaleMeters;
ElevDatum = Rend->ElevDatum;
Exageration = Rend->Exageration;
TexRefLon = Rend->TexRefLon;
TexRefLat = Rend->TexRefLat;
TexRefElev = Rend->TexRefElev;
RefLonScaleMeters = Rend->RefLonScaleMeters;
TexData.MetersPerDegLat = TexData1.MetersPerDegLat = Rend->EarthLatScaleMeters;
TexData.Datum = TexData1.Datum = Rend->ElevDatum;
TexData.Exageration = TexData1.Exageration = Rend->Exageration;
TexData.ExagerateElevLines = TexData1.ExagerateElevLines = Rend->ExagerateElevLines;
ShadowMapDistanceOffset = Rend->ShadowMapDistanceOffset;
Width = Rend->Width;
SetupWidth = Rend->SetupWidth;
SetupHeight = Rend->SetupHeight;
Height = Rend->Height;
if (Opt)	// should always be valid for rendering
	{
	PixelAspect = Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue;
	FrameRate = Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue;
	} // if

} // RenderData::InitFrameToRender

/*===========================================================================*/

double RenderData::GetElevationPoint(double Lat, double Lon)
{

return (Interactive->ElevationPoint(Lat, Lon));

} // RenderData::GetElevationPoint

/*===========================================================================*/
/*===========================================================================*/
// These functions just transfer data from an object to one of its members that is used for evaluating textures.
// The reason for doing it this way is so that only one kind of structure is needed for evaluating textures
// and so that the data need only be transferred if actually needed to evaluate a texture. The function returns
// a pointer to the member with the transferred data so that it can be passed as an argument to the texture evaluator.
// The transfer will only happen once no matter how many textures are evaluated because of the flag that signals 
// that the operation has been done.

TextureData *RenderData::TransferTextureData(PolygonData *Poly)
{

TexData.Vec = Poly->Vector;
TexData.VectorEffectType = Poly->VectorType;
TexData.Elev = Poly->Elev;
TexData.RelElev = Poly->RelEl;
TexData.Slope = Poly->Slope;
TexData.Aspect = Poly->Aspect;
TexData.Latitude = Poly->Lat;
TexData.Longitude = Poly->Lon;
TexData.ZDist = Poly->Z;
TexData.QDist = Poly->Q;
TexData.WaterDepth = Poly->WaterElev - Poly->Elev;
TexData.VectorSlope = Poly->VectorSlope;
TexData.Object = Poly->Object;
TexData.PData = Poly;
TexData.VData[0] = TexData.VData[1] = TexData.VData[2] = NULL;
TexData.VDEM[0] = TexData.VDEM[1] = TexData.VDEM[2] = NULL;
TexData.TexRefLon = TexRefLon;
TexData.TexRefLat = TexRefLat;
TexData.TexRefElev = TexRefElev;
TexData.MetersPerDegLon = RefLonScaleMeters;
TexData.MetersPerDegLat = EarthLatScaleMeters;
TexData.Datum = ElevDatum;
TexData.Exageration = Exageration;
TexData.ExagerateElevLines = ExagerateElevLines;
if (Cam && Cam->TargPos)
	{
	// for texture dynamic param need vector pointing towards camera
	TexData.CamAimVec[0] = -Cam->TargPos->XYZ[0];
	TexData.CamAimVec[1] = -Cam->TargPos->XYZ[1];
	TexData.CamAimVec[2] = -Cam->TargPos->XYZ[2];
	} // if
TexData.Normal[0] = Poly->Normal[0];
TexData.Normal[1] = Poly->Normal[1];
TexData.Normal[2] = Poly->Normal[2];

TexData.VectorOffsetsComputed = 0;

return (&TexData);

} // RenderData::TransferTextureData

/*===========================================================================*/

TextureData *RenderData::TransferTextureData(VertexData *Vert)
{

TexData.Vec = Vert->Vector;
TexData.VectorEffectType = Vert->VectorType;
TexData.Elev = Vert->Elev;
TexData.RelElev = Vert->RelEl;
TexData.Latitude = Vert->Lat;
TexData.Longitude = Vert->Lon;
TexData.ZDist = Vert->ScrnXYZ[2];
TexData.QDist = Vert->Q;
TexData.WaterDepth = Vert->WaterElev - Vert->Elev;
TexData.VectorSlope = Vert->VectorSlope;
TexData.PData = NULL;
TexData.VData[0] = Vert;
TexData.VDEM[0] = Vert;
TexData.VData[1] = TexData.VData[2] = NULL;
TexData.VDEM[1] = TexData.VDEM[2] = NULL;
TexData.TexRefLon = TexRefLon;
TexData.TexRefLat = TexRefLat;
TexData.TexRefElev = TexRefElev;
TexData.MetersPerDegLon = RefLonScaleMeters;
TexData.MetersPerDegLat = EarthLatScaleMeters;
TexData.Datum = ElevDatum;
TexData.Exageration = Exageration;
TexData.ExagerateElevLines = ExagerateElevLines;

TexData.VectorOffsetsComputed = 0;

return (&TexData);

} // RenderData::TransferTextureData

/*===========================================================================*/

TextureData *RenderData::TransferTextureData(VectorNode *Node, Joe *RefVec, MaterialList *RefMat)
{

TexData1.Vec = RefVec;
TexData1.VectorEffectType = WCS_TEXTURE_VECTOREFFECTTYPE_AREA;
TexData1.VectorOffsetsComputed = 0;
TexData1.Elev = Node->Elev;
TexData1.RelElev = Node->NodeData->RelEl;
TexData1.Slope = Node->NodeData->Slope;
TexData1.Aspect = Node->NodeData->Aspect;
TexData1.Latitude = Node->Lat;
TexData1.Longitude = Node->Lon;
TexData1.ZDist = 0.0;
TexData1.QDist = 0.0;
TexData1.WaterDepth = 0.0;
TexData1.WaveHeight = 0.0;
TexData1.VectorSlope = 0.0;
TexData1.VecOffset[0] = TexData1.VecOffset[1] = TexData1.VecOffset[2] = 0.0;
TexData1.PData = NULL;
TexData1.VData[0] = TexData1.VData[1] = TexData1.VData[2] = NULL;
TexData1.VDEM[0] = TexData1.VDEM[1] = TexData1.VDEM[2] = NULL;
TexData1.TexRefLon = TexRefLon;
TexData1.TexRefLat = TexRefLat;
TexData1.TexRefElev = TexRefElev;
TexData1.MetersPerDegLon = RefLonScaleMeters;
TexData1.MetersPerDegLat = EarthLatScaleMeters;
TexData1.Datum = ElevDatum;
TexData1.Exageration = Exageration;
TexData1.ExagerateElevLines = ExagerateElevLines;
if (RefMat)
	{
	if (RefMat->WaterProp)
		{
		TexData1.WaterDepth = RefMat->WaterProp->WaterDepth; 
		TexData1.WaveHeight = RefMat->WaterProp->WaveHeight; 
		} // if
	if (RefMat->VectorProp)
		{
		TexData1.VectorEffectType = RefMat->VectorProp->VectorType;
		TexData1.VectorSlope = RefMat->VectorProp->VectorSlope;
		TexData1.VecOffset[0] = TexData1.VecOffsetPixel[1] = TexData1.VecOffsetPixel[0] = RefMat->VectorProp->VecOffsets[0];
		TexData1.VecOffset[1] = TexData1.VecOffsetPixel[3] = TexData1.VecOffsetPixel[2] = RefMat->VectorProp->VecOffsets[1];
		TexData1.VecOffset[2] = TexData1.VecOffsetPixel[5] = TexData1.VecOffsetPixel[4] = RefMat->VectorProp->VecOffsets[2];
		TexData1.VectorOffsetsComputed = true;	//(RefMat->VectorProp->VecOffsets[0] != 0.0f || RefMat->VectorProp->VecOffsets[1] != 0.0f || RefMat->VectorProp->VecOffsets[2] != 0.0f);
		} // if
	} // if

return (&TexData1);

} // RenderData::TransferTextureData

/*===========================================================================*/

TextureData *RenderData::TransferTextureData(VertexDEM *Vert)
{

TexData.Vec = NULL;
TexData.VectorEffectType = 0;
TexData.Elev = Vert->Elev;
TexData.RelElev = 0.0f;
TexData.Latitude = Vert->Lat;
TexData.Longitude = Vert->Lon;
TexData.ZDist = Vert->ScrnXYZ[2];
TexData.QDist = Vert->Q;
TexData.WaterDepth = 0.0;
TexData.VectorSlope = 0.0f;
TexData.PData = NULL;
TexData.VData[0] = TexData.VData[1] = TexData.VData[2] = NULL;
TexData.VDEM[0] = Vert;
TexData.VDEM[1] = TexData.VDEM[2] = NULL;
TexData.TexRefLon = TexRefLon;
TexData.TexRefLat = TexRefLat;
TexData.TexRefElev = TexRefElev;
TexData.MetersPerDegLon = RefLonScaleMeters;
TexData.MetersPerDegLat = EarthLatScaleMeters;
TexData.Datum = ElevDatum;
TexData.Exageration = Exageration;
TexData.ExagerateElevLines = ExagerateElevLines;

TexData.VectorOffsetsComputed = 0;
return (&TexData);

} // RenderData::TransferTextureData

/*===========================================================================*/

TextureData *RenderData::TransferTextureData(PixelData *Pix)
{

TexData.Object = Pix->Object;
TexData.PData = NULL;
TexData.VData[0] = TexData.VData[1] = TexData.VData[2] = NULL;
TexData.VDEM[0] = TexData.VDEM[1] = TexData.VDEM[2] = NULL;

TexData.Vec = NULL;
TexData.VectorEffectType = 0;
TexData.Latitude = Pix->Lat;
TexData.Longitude = Pix->Lon;
TexData.Elev = Pix->Elev;
TexData.ZDist = Pix->Zflt;
TexData.QDist = Pix->Q;
TexData.RelElev = 0.0f;
TexData.WaterDepth = 0.0;
TexData.VectorSlope = 0.0f;
TexData.Aspect = 0.0f;
TexData.Slope = 0.0f;

TexData.TexRefLon = TexRefLon;
TexData.TexRefLat = TexRefLat;
TexData.TexRefElev = TexRefElev;
TexData.MetersPerDegLon = RefLonScaleMeters;
TexData.MetersPerDegLat = EarthLatScaleMeters;
TexData.Datum = ElevDatum;
TexData.Exageration = Exageration;
TexData.ExagerateElevLines = ExagerateElevLines;

// for texture dynamic param need vector pointing towards camera
TexData.CamAimVec[0] = Pix->ViewVec[0];
TexData.CamAimVec[1] = Pix->ViewVec[1];
TexData.CamAimVec[2] = Pix->ViewVec[2];
TexData.Normal[0] = Pix->Normal[0];
TexData.Normal[1] = Pix->Normal[1];
TexData.Normal[2] = Pix->Normal[2];

TexData.VectorOffsetsComputed = 0;

return (&TexData);

} // RenderData::TransferTextureData

/*===========================================================================*/

TextureData *RenderData::TransferTextureDataRange(VertexData *Vert, TextureData *TxData, double PixWidth)
{

TexData.Object = TxData->Object;
TexData.Object3D = TxData->Object3D;
TexData.PData = TxData->PData;
TexData.VData[0] = Vert;
TexData.VData[1] = TexData.VData[2] = NULL;
TexData.VDEM[0] = Vert;
TexData.VDEM[1] = TexData.VDEM[2] = NULL;
TexData.TexRefLon = TexRefLon;
TexData.TexRefLat = TexRefLat;
TexData.TexRefElev = TexRefElev;
TexData.MetersPerDegLon = RefLonScaleMeters;
TexData.MetersPerDegLat = EarthLatScaleMeters;
TexData.Datum = ElevDatum;
TexData.Exageration = Exageration;
TexData.ExagerateElevLines = ExagerateElevLines;
TexData.PixelWidth = PixWidth * .5;

TexData.Vec = Vert->Vector;
TexData.VectorEffectType = Vert->VectorType;
TexData.Elev = Vert->Elev;
TexData.RelElev = Vert->RelEl;
TexData.Latitude = Vert->Lat;
TexData.Longitude = Vert->Lon;
TexData.ZDist = Vert->ScrnXYZ[2];
TexData.QDist = Vert->Q;
TexData.WaterDepth = Vert->WaterElev - Vert->Elev;
TexData.VectorSlope = Vert->VectorSlope;

TexData.VectorOffsetsComputed = 0;
TexData.VDataVecOffsetsValid = 0;

return (&TexData);

} // RenderData::TransferTextureDataRange

/*===========================================================================*/

TextureData *RenderData::Transfer3DTextureDataRange(VertexData *Vert, double PixWidth, double ObjPixWidth)
{
//double HalfWidth;

TexData.VDEM[0] = Vert;
TexData.VDEM[1] = TexData.VDEM[2] = NULL;
TexData.VData[0] = Vert;
TexData.VData[1] = TexData.VData[2] = NULL;
TexData.PData = NULL;
TexData.PixelWidth = PixWidth * .5;
TexData.ObjectPixelWidth = ObjPixWidth * .5;

return (&TexData);

} // RenderData::Transfer3DTextureDataRange

/*===========================================================================*/

VertexData *RenderData::ReverseTransferTextureData(VertexData *Vert, TextureData *TxData)
{

Vert->Vector = TxData->Vec;
Vert->VectorType = TxData->VectorEffectType;
Vert->Elev = TxData->Elev;
Vert->RelEl = TxData->RelElev;
Vert->Lat = TxData->Latitude;
Vert->Lon = TxData->Longitude;
Vert->ScrnXYZ[2] = TxData->ZDist;
Vert->Q = TxData->QDist;
Vert->WaterElev = TxData->WaterDepth - Vert->Elev;
Vert->VectorSlope = TxData->VectorSlope;

return (Vert);

} // RenderData::ReverseTransferTextureData

/*===========================================================================*/
/*===========================================================================*/

void TextureData::CopyData(TextureData *CopyFrom)
{
int Ct1, Ct2;

Vec = CopyFrom->Vec;
VectorEffectType = CopyFrom->VectorEffectType;
VectorSlope = CopyFrom->VectorSlope;
ZDist = CopyFrom->ZDist;
QDist = CopyFrom->QDist;
PixelX[0] = CopyFrom->PixelX[0];
PixelX[1] = CopyFrom->PixelX[1];
PixelY[0] = CopyFrom->PixelY[0];
PixelY[1] = CopyFrom->PixelY[1];
PixelUnityX[0] = CopyFrom->PixelUnityX[0];
PixelUnityX[1] = CopyFrom->PixelUnityX[1];
PixelUnityY[0] = CopyFrom->PixelUnityY[0];
PixelUnityY[1] = CopyFrom->PixelUnityY[1];
Elev = CopyFrom->Elev;
Slope = CopyFrom->Slope;
Aspect = CopyFrom->Aspect;
Latitude = CopyFrom->Latitude;
Longitude = CopyFrom->Longitude;
RelElev = CopyFrom->RelElev;
Illumination = CopyFrom->Illumination;
Reflectivity = CopyFrom->Reflectivity;
WaterDepth = CopyFrom->WaterDepth;
WaveHeight = CopyFrom->WaveHeight;
TLowX = CopyFrom->TLowX;
TLowY = CopyFrom->TLowY;
TLowZ = CopyFrom->TLowZ;
THighX = CopyFrom->THighX;
THighY = CopyFrom->THighY;
THighZ = CopyFrom->THighZ;
MetersPerDegLat = CopyFrom->MetersPerDegLat;
MetersPerDegLon = CopyFrom->MetersPerDegLon;
TexRefLat = CopyFrom->TexRefLat;
TexRefLon = CopyFrom->TexRefLon;
TexRefElev = CopyFrom->TexRefElev;
Datum = CopyFrom->Datum;
Exageration = CopyFrom->Exageration;
ExagerateElevLines = CopyFrom->ExagerateElevLines;
PixelWidth = CopyFrom->PixelWidth;
ObjectPixelWidth = CopyFrom->ObjectPixelWidth;
Object = CopyFrom->Object;
Object3D = CopyFrom->Object3D;
PData = CopyFrom->PData;
VectorOffsetsComputed = 0;
VDataVecOffsetsValid = CopyFrom->VDataVecOffsetsValid;
for (Ct2 = 0; Ct2 < 3; Ct2 ++)
	{
	CamAimVec[Ct2] = CopyFrom->CamAimVec[Ct2];
	Normal[Ct2] = CopyFrom->Normal[Ct2];
	RGB[Ct2] = CopyFrom->RGB[Ct2];
	VtxWt[Ct2] = CopyFrom->VtxWt[Ct2];
	VtxNum[Ct2] = CopyFrom->VtxNum[Ct2];
	VDEM[Ct2] = CopyFrom->VDEM[Ct2];
	VData[Ct2] = CopyFrom->VData[Ct2];
	for (Ct1 = 0; Ct1 < 4; Ct1 ++)
		{
		VtxPixCornerWt[Ct1][Ct2] = CopyFrom->VtxPixCornerWt[Ct1][Ct2];
		} // for
	} // for

} // TextureData::CopyData

/*===========================================================================*/

void TextureData::ExtractMatTableData(MaterialTable *MatTable, long *VertIndex)
{
MaterialList *MatListToUse;

if (! (MatListToUse = MatTable->MatListPtr[VertIndex[0]]))
	{
	if (! (MatListToUse = MatTable->MatListPtr[VertIndex[1]]))
		{
		MatListToUse = MatTable->MatListPtr[VertIndex[2]];
		} // if
	} // if
if (MatListToUse)
	{
	Vec = MatListToUse->MyVec;
	if (MatListToUse->VectorProp)
		{
		VectorEffectType = MatListToUse->VectorProp->VectorType;
		VectorSlope = MatListToUse->VectorProp->VectorSlope;
		VecOffset[0] = VecOffsetPixel[1] = VecOffsetPixel[0] = MatListToUse->VectorProp->VecOffsets[0];
		VecOffset[1] = VecOffsetPixel[3] = VecOffsetPixel[2] = MatListToUse->VectorProp->VecOffsets[1];
		VecOffset[2] = VecOffsetPixel[5] = VecOffsetPixel[4] = MatListToUse->VectorProp->VecOffsets[2];
		VectorOffsetsComputed = 1;
		} // if
	else
		{
		VectorEffectType = WCS_TEXTURE_VECTOREFFECTTYPE_AREA;
		VectorSlope = 0.0f;
		VecOffset[2] = VecOffset[1] = VecOffset[0] = 0.0;
		VectorOffsetsComputed = 0;
		} // else
	Object = MatListToUse->MyEffect;
	if (PData)
		{
		PData->Vector = Vec;
		PData->VectorType = VectorEffectType;
		PData->VectorSlope = VectorSlope;
		PData->Object = Object;
		} // if
	if (VData[0] && VData[1] && VData[2])
		{
		VData[0]->Vector = VData[1]->Vector = VData[2]->Vector = Vec;
		VData[0]->VectorType = VData[1]->VectorType = VData[2]->VectorType = VectorEffectType;
		if (MatTable->MatListPtr[VertIndex[0]] && MatTable->MatListPtr[VertIndex[0]]->VectorProp)
			{
			VData[0]->VectorSlope = MatTable->MatListPtr[VertIndex[0]]->VectorProp->VectorSlope;
			VData[0]->VecOffsets[0] = MatTable->MatListPtr[VertIndex[0]]->VectorProp->VecOffsets[0];
			VData[0]->VecOffsets[1] = MatTable->MatListPtr[VertIndex[0]]->VectorProp->VecOffsets[1];
			VData[0]->VecOffsets[2] = MatTable->MatListPtr[VertIndex[0]]->VectorProp->VecOffsets[2];
			} // if
		else
			{
			VData[0]->VectorSlope = VectorSlope;
			VData[0]->VecOffsets[0] = (float)VecOffset[0];
			VData[0]->VecOffsets[1] = (float)VecOffset[1];
			VData[0]->VecOffsets[2] = (float)VecOffset[2];
			} // else
		if (MatTable->MatListPtr[VertIndex[1]] && MatTable->MatListPtr[VertIndex[1]]->VectorProp)
			{
			VData[1]->VectorSlope = MatTable->MatListPtr[VertIndex[1]]->VectorProp->VectorSlope;
			VData[1]->VecOffsets[0] = MatTable->MatListPtr[VertIndex[1]]->VectorProp->VecOffsets[0];
			VData[1]->VecOffsets[1] = MatTable->MatListPtr[VertIndex[1]]->VectorProp->VecOffsets[1];
			VData[1]->VecOffsets[2] = MatTable->MatListPtr[VertIndex[1]]->VectorProp->VecOffsets[2];
			} // if
		else
			{
			VData[1]->VectorSlope = VectorSlope;
			VData[1]->VecOffsets[0] = (float)VecOffset[0];
			VData[1]->VecOffsets[1] = (float)VecOffset[1];
			VData[1]->VecOffsets[2] = (float)VecOffset[2];
			} // else
		if (MatTable->MatListPtr[VertIndex[2]] && MatTable->MatListPtr[VertIndex[2]]->VectorProp)
			{
			VData[2]->VectorSlope = MatTable->MatListPtr[VertIndex[2]]->VectorProp->VectorSlope;
			VData[2]->VecOffsets[0] = MatTable->MatListPtr[VertIndex[2]]->VectorProp->VecOffsets[0];
			VData[2]->VecOffsets[1] = MatTable->MatListPtr[VertIndex[2]]->VectorProp->VecOffsets[1];
			VData[2]->VecOffsets[2] = MatTable->MatListPtr[VertIndex[2]]->VectorProp->VecOffsets[2];
			} // if
		else
			{
			VData[2]->VectorSlope = VectorSlope;
			VData[2]->VecOffsets[0] = (float)VecOffset[0];
			VData[2]->VecOffsets[1] = (float)VecOffset[1];
			VData[2]->VecOffsets[2] = (float)VecOffset[2];
			} // else
		VDataVecOffsetsValid = VectorOffsetsComputed;
		} // if
	} // if
	
} // TextureData::ExtractMatTableData

/*===========================================================================*/

void PolygonData::ExtractMatTableData(MaterialTable *MatTable)
{
MaterialList *MatListToUse;

if (! (MatListToUse = MatTable->MatListPtr[0]))
	{
	if (! (MatListToUse = MatTable->MatListPtr[1]))
		{
		MatListToUse = MatTable->MatListPtr[2];
		} // if
	} // if
if (MatListToUse)
	{
	Vector = MatListToUse->MyVec;
	if (MatListToUse->VectorProp)
		{
		VectorType = MatListToUse->VectorProp->VectorType;
		VectorSlope = MatListToUse->VectorProp->VectorSlope;
		} // if
	if (MatListToUse->WaterProp)
		{
		WaterElev = Elev + MatListToUse->WaterProp->WaterDepth;
		} // if
	Object = MatListToUse->MyEffect;
	} // if
	
} // PolygonData::ExtractMatTableData
