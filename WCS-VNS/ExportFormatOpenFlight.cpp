// ExportFormatOpenFlight.cpp
// Code module for OpenFlight export code
// Created from ExportFormat.cpp on 5/18/04 by CXH
// ExportFormat.cpp Created from scratch 07/01/03 by Gary R. Huber
// Copyright 2003 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "ExportFormat.h"
#include "EffectsLib.h"
#include "ExportControlGUI.h"
#include "IncludeExcludeList.h"
#include "Raster.h"
#include "ImageOutputEvent.h"
#include "Project.h"
#include "Requester.h"
#include "TerrainWriter.h"
#include "CubeSphere.h"
#include "AppMem.h"
#include "RasterResampler.h"
#include "Log.h"
#include "SceneExportGUI.h"
#include "SXExtension.h"
#include "zlib.h"
#include "ImageInputFormat.h"
#include "Lists.h"

//lint -save -e569 -e603 -e790
ExportFormatOpenFlight::ExportFormatOpenFlight(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource)
: ExportFormat(MasterSource, ProjectSource, EffectsSource, DBSource, ImageSource)
{

fOF_Master = NULL;
OF_SetRev(1560);	// for better compatibility at the moment
MinTable = NULL;
Ambient = 0.0f;
//SuppressAlpha = 0;

} // ExportFormatOpenFlight::ExportFormatOpenFlight

/*===========================================================================*/

ExportFormatOpenFlight::~ExportFormatOpenFlight()
{

if (fOF_Master)
	fclose(fOF_Master);

} // ExportFormatOpenFlight::~ExportFormatOpenFlight

/*===========================================================================*/

// remove all temp files
void ExportFormatOpenFlight::Cleanup(long Tiled, const char *OutputFilePath, NameList **FileNamesCreated)
{
long FileType;
char FullFileName[512];
const char *FileName;

// remove foliage temp files
if (Master->ExportFoliage)
	{
	FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILEIDX;
	if (FileName = (*FileNamesCreated)->FindNameOfType(FileType))
		{
		strmfp(FullFileName, OutputFilePath, FileName);
		PROJ_remove(FullFileName);

		FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILE;
		if (FileName = (*FileNamesCreated)->FindNameOfType(FileType))
			{
			strmfp(FullFileName, OutputFilePath, FileName);
			PROJ_remove(FullFileName);
			} // if
		} // if
	} // if

// remove terrain temp file(s)
if (Master->ExportTerrain)
	{
	FileType = WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN;
	if (Tiled)
		{
		FileName = NULL;
		FileName = (*FileNamesCreated)->FindNextNameOfType(FileType, FileName);
		while (FileName)
			{
			strmfp(FullFileName, OutputFilePath, FileName);
			PROJ_remove(FullFileName);
			FileName = (*FileNamesCreated)->FindNextNameOfType(FileType, FileName);
			} // while
		} // if Tiled
	else
		{
		FileName = (*FileNamesCreated)->FindNameOfType(FileType);
		strmfp(FullFileName, OutputFilePath, FileName);
		PROJ_remove(FullFileName);
		} // else Tiled
	} // if


} // ExportFormatOpenFlight::Cleanup

/*===========================================================================*/

int ExportFormatOpenFlight::ExportCameras(FILE *fOF, EffectList *CameraList)
{
double X, Y;
//double KeyValue, KeyTime, Tension, Continuity, Bias, Mult;
int Err = 0;
//long Channel, ADTChannel, NumKeys, CurKey, SpanType, AALevel, TargetNum, Width, Height, CamNum = 0;
long EyePoint = 1;
RenderJob *CurRJ;
RenderOpt *CurOpt;
EffectList *CurCam;
AnimDoubleTime *ADT;
//GraphNode *CurNode;
OF_EyepointAndTrackplanePalette EaTP;

// if cameras
if (CameraList && Master->ExportCameras)
	{
	// for each camera
	for (CurCam = CameraList; CurCam; CurCam = CurCam->Next)
		{
		if (CurCam->Me && EyePoint < 10)
			{
			float LookFrom[3];
			Point3d Orient;
			Camera *Cam = (Camera *)CurCam->Me;

			// search for a matching RenderOpt to get additional settings from
			CurOpt = NULL;
			// for each render job, does the camera match
			for (CurRJ = (RenderJob *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_RENDERJOB); CurRJ;
				CurRJ = (RenderJob *)CurRJ->Next)
				{
				if (CurRJ->Cam == Cam)
					{
					CurOpt = CurRJ->Options;
					break;
					} // if
				} // for
			// write camera values
			Master->RBounds.DefDegToRBounds(Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue, Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue, X, Y);
			LookFrom[0] = (float)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale + bias_x);
			LookFrom[1] = (float)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale + bias_y);
			LookFrom[2] = (float)(Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue * Master->ExportRefData.ElevScale);
			ADT = Cam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV);
			EaTP.Eyepoint[EyePoint].RotCtrX = LookFrom[0];
			EaTP.Eyepoint[EyePoint].RotCtrY = LookFrom[1];
			EaTP.Eyepoint[EyePoint].RotCtrZ = LookFrom[2];
			EaTP.Eyepoint[EyePoint].RotationMatrix[0] = 1.0f;
			EaTP.Eyepoint[EyePoint].RotationMatrix[5] = 1.0f;
			EaTP.Eyepoint[EyePoint].RotationMatrix[10] = 1.0f;
			EaTP.Eyepoint[EyePoint].RotationMatrix[15] = 1.0f;
			EaTP.Eyepoint[EyePoint].FieldOfView = (float)ADT->CurValue;
			EaTP.Eyepoint[EyePoint].NearClipPlane = 1.0f;		// use 1/2 cellsize
			EaTP.Eyepoint[EyePoint].FarClipPlane = 1000000.0f;	// use 2x output terrain size
			EaTP.Eyepoint[EyePoint].FlythroughYaw = 0.0f;
			EaTP.Eyepoint[EyePoint].FlythroughPitch = 0.0f;
			EaTP.Eyepoint[EyePoint].FlythroughMatrix[0] = 1.0f;
			EaTP.Eyepoint[EyePoint].FlythroughMatrix[5] = 1.0f;
			EaTP.Eyepoint[EyePoint].FlythroughMatrix[10] = 1.0f;
			EaTP.Eyepoint[EyePoint].FlythroughMatrix[15] = 1.0f;
			EaTP.Eyepoint[EyePoint].EyepointX = LookFrom[0];
			EaTP.Eyepoint[EyePoint].EyepointY = LookFrom[1];
			EaTP.Eyepoint[EyePoint].EyepointZ = LookFrom[2];
			EaTP.Eyepoint[EyePoint].NoFlythrough = 1;
			if (Cam->Orthographic)
				EaTP.Eyepoint[EyePoint].OrthoView = 1;
			EaTP.Eyepoint[EyePoint].ValidEyepoint = 1;
			EaTP.Eyepoint[EyePoint].ImageOffsetX = 0;
			EaTP.Eyepoint[EyePoint].ImageOffsetY = 0;
			EaTP.Eyepoint[EyePoint].ImageZoom = 1;
			// NOTE: Ask Gary about why this code needs to be like this again
			if (Cam->CameraType == WCS_EFFECTS_CAMERATYPE_UNTARGETED)
				{
				Orient[0] = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].CurValue;
				Orient[1] = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].CurValue;
				Orient[2] = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].CurValue;
				} // if untargeted
			else if (Cam->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED)
				{
				Orient[0] = Cam->CamPitch;
				Orient[1] = Cam->CamHeading;
				Orient[2] = Cam->CamBank;
				} // else if targeted
			else if (Cam->CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD)
				{
				Orient[0] = Cam->CamPitch;
				Orient[1] = Cam->CamHeading;
				Orient[2] = Cam->CamBank;
				} // else if overhead
			if (Cam->CameraType == WCS_EFFECTS_CAMERATYPE_ALIGNED)
				{
				Orient[0] = Cam->CamPitch;
				Orient[1] = Cam->CamHeading;
				Orient[2] = Cam->CamBank;
				} // if untargeted
			EaTP.Eyepoint[EyePoint].Roll = (float)Orient[2];
			EaTP.Eyepoint[EyePoint].Pitch = (float)Orient[0];
			EaTP.Eyepoint[EyePoint].Yaw = (float)Orient[1];
			EaTP.Eyepoint[EyePoint].EyepointDirectVecI = 0.0f;
			EaTP.Eyepoint[EyePoint].EyepointDirectVecJ = 0.0f;
			EaTP.Eyepoint[EyePoint].EyepointDirectVecK = -1.0f;
			EaTP.Eyepoint[EyePoint].ImageZoom = 1;
			if (EyePoint == 1)
				{
				// PolyTrans ignores EyePoint 0, Creater sets initial view to EyePoint 0.
				// The solution we use is to store EyePoints starting at slot 1, but copying
				// the first one back to slot 0
				memcpy((void *)(&EaTP.Eyepoint[0]), (const void *)(&EaTP.Eyepoint[1]), 272);
				} // if
			EyePoint++;

			/***
			fprintf(LWFile, "AddCamera\n");
			fprintf(LWFile, "CameraName %s\n", ((Camera *)CurCam->Me)->GetName());
			fprintf(LWFile, "ShowCamera 1 2\n");
			fprintf(LWFile, "CameraMotion\n");
			fprintf(LWFile, "NumChannels 6\n");
			SpanType = 0;	// TCB
			for (Channel = 0; Channel < 6; Channel ++)
				{
				ADTChannel = Channel < 3 ? Channel: Channel + 3;
				Mult = Channel < 3 ? 1.0: PiOver180;
				fprintf(LWFile, "Channel %d\n", Channel);
				fprintf(LWFile, "{ Envelope\n");
				ADT = ((Camera *)CurCam->Me)->GetAnimPtr(ADTChannel);
				NumKeys = ADT->GetNumNodes(0);
				NumKeys = NumKeys > 1 ? NumKeys: 1;
				fprintf(LWFile, "  %d\n", NumKeys);
				for (CurKey = 0, CurNode = ADT->GetFirstNode(0); CurKey < NumKeys; CurKey ++, CurNode = ADT->GetNextNode(0, CurNode))
					{
					// <<<>>> need translation to XYZ export coords
					if (CurNode)
						{
						KeyValue = CurNode->Value * Mult;
						KeyTime = CurNode->Distance;
						Tension = CurNode->TCB[0];
						Continuity = CurNode->TCB[1];
						Bias = CurNode->TCB[2];
						} // if
					else
						{
						KeyValue = ADT->CurValue * Mult;
						KeyTime = 0.0;
						Tension = Continuity = Bias = 0.0;
						} // else
					fprintf(LWFile, "  Key %g %g %d %g %g %g 0 0 0\n", KeyValue, KeyTime, SpanType,
						Tension, Continuity, Bias);
					} // for
				fprintf(LWFile, "  Behaviors 1 1\n");
				fprintf(LWFile, "}\n");
				} // for
			fprintf(LWFile, "\n");
			// camera target
			if (((Camera *)CurCam->Me)->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED)
				{
				fprintf(LWFile, "HController 1\n");
				fprintf(LWFile, "PController 1\n");
				TargetNum = (0x10000000 | CamNum);
				fprintf(LWFile, "TargetItem %8X\n", TargetNum);
				CamNum ++;
				} // if
			// additional camera data is in render options
			if (CurOpt)
				{
				Width = CurOpt->OutputImageWidth;
				Height = CurOpt->OutputImageHeight;
				fprintf(LWFile, "ZoomFactor (envelope)\n");
				fprintf(LWFile, "{ Envelope\n");
				ADT = ((Camera *)CurCam->Me)->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV);
				NumKeys = ADT->GetNumNodes(0);
				NumKeys = NumKeys > 1 ? NumKeys: 1;
				fprintf(LWFile, "  %d\n", NumKeys);
				for (CurKey = 0, CurNode = ADT->GetFirstNode(0); CurKey < NumKeys; CurKey ++, CurNode = ADT->GetNextNode(0, CurNode))
					{
					if (CurNode)
						{
						KeyValue = (CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue * Width / Height) / 
							tan((CurNode->Value / 2.0) * PiOver180);
						KeyTime = CurNode->Distance;
						Tension = CurNode->TCB[0];
						Continuity = CurNode->TCB[1];
						Bias = CurNode->TCB[2];
						} // if
					else
						{
						KeyValue = (CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue * Width / Height) / 
							tan((ADT->CurValue / 2.0) * PiOver180);
						KeyTime = 0.0;
						Tension = Continuity = Bias = 0.0;
						} // else
					fprintf(LWFile, "  Key %g %g %d %g %g %g 0 0 0\n", KeyValue, KeyTime, SpanType,
						Tension, Continuity, Bias);
					} // for
				fprintf(LWFile, "  Behaviors 1 1\n");
				fprintf(LWFile, "}\n");

				fprintf(LWFile, "ResolutionMultiplier 1.0\n");
				fprintf(LWFile, "FrameSize %d %d\n", Width, Height);
				fprintf(LWFile, "PixelAspect %g\n", CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue);
				} // if
			else
				{
				Width = 640;
				Height = 480;
				fprintf(LWFile, "ZoomFactor (envelope)\n");
				fprintf(LWFile, "{ Envelope\n");
				ADT = ((Camera *)CurCam->Me)->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV);
				NumKeys = ADT->GetNumNodes(0);
				NumKeys = NumKeys > 1 ? NumKeys: 1;
				fprintf(LWFile, "  %d\n", NumKeys);
				for (CurKey = 0, CurNode = ADT->GetFirstNode(0); CurKey < NumKeys; CurKey ++, CurNode = ADT->GetNextNode(0, CurNode))
					{
					if (CurNode)
						{
						KeyValue = (1.0 * (double)Width / Height) / 
							tan((CurNode->Value / 2.0) * PiOver180);
						KeyTime = CurNode->Distance;
						Tension = CurNode->TCB[0];
						Continuity = CurNode->TCB[1];
						Bias = CurNode->TCB[2];
						} // if
					else
						{
						KeyValue = (1.0 * (double)Width / Height) / 
							tan((ADT->CurValue / 2.0) * PiOver180);
						KeyTime = 0.0;
						Tension = Continuity = Bias = 0.0;
						} // else
					fprintf(LWFile, "  Key %g %g %d %g %g %g 0 0 0\n", KeyValue, KeyTime, SpanType,
						Tension, Continuity, Bias);
					} // for
				fprintf(LWFile, "  Behaviors 1 1\n");
				fprintf(LWFile, "}\n");
				fprintf(LWFile, "ResolutionMultiplier 1.0\n");
				fprintf(LWFile, "FrameSize %d %d\n", Width, Height);
				fprintf(LWFile, "PixelAspect %g\n", 1.0);
				} // else
			fprintf(LWFile, "MaskPosition 0 0 %d %d\n", Width, Height);
			fprintf(LWFile, "MotionBlur %d\n", ((Camera *)CurCam->Me)->MotionBlur ? 1: 0);
			fprintf(LWFile, "FieldRendering 1\n", ((Camera *)CurCam->Me)->FieldRender ? 1: 0);
			fprintf(LWFile, "ReverseFields 1\n", ((Camera *)CurCam->Me)->FieldRenderPriority ? 1: 0);
			fprintf(LWFile, "\n");
			fprintf(LWFile, "ApertureHeight 0.015\n");
			AALevel = (((Camera *)CurCam->Me)->AAPasses <= 1) ? 0: 
				(((Camera *)CurCam->Me)->AAPasses <= 5) ? 1:
				(((Camera *)CurCam->Me)->AAPasses <= 9) ? 2:
				(((Camera *)CurCam->Me)->AAPasses <= 17) ? 3: 4;
			fprintf(LWFile, "Antialiasing %d\n", AALevel);
			fprintf(LWFile, "AdaptiveSampling 1\n");
			fprintf(LWFile, "AdaptiveThreshold 0.1254902\n");
			fprintf(LWFile, "\n");
			***/
			} // if CurCam->Me
		} // for CurCam
		Err = EaTP.WriteToFile(fOF);
	} // if cameras

return Err;

} // ExportFormatOpenFlight::ExportCameras

/*===========================================================================*/

int ExportFormatOpenFlight::ExportLights(long which)
{
double X, Y;
int Err = 0;
long index;
EffectList *LiteList = Master->Lights;
Light *Lite;
OF_LightSource LS;
OF_LightSourcePalette LSP;
VertexDEM VDEM;

if (which == 0)
	{
	// write the LightSourcePalette
	index = 0;
	LiteList = Master->Lights;

	if (LiteList)
		{
		// export ambient lighting (from atmospheres)
		EffectList *CurHaze;
		if (Master->ExportHaze && (CurHaze = Master->Haze))
			{
			if (CurHaze->Me)
				{
				Atmosphere *Haze = (Atmosphere *)CurHaze->Me;
				float Bottom, Top;

				Bottom = (float)Haze->BottomAmbientColor.GetIntensity();
				Top = (float)Haze->TopAmbientColor.GetIntensity();
				Ambient = __max(Bottom, Top);
				}
			} // if
		} // if LiteList

	while (LiteList)
		{
		Lite = (class Light *)LiteList->Me;
		//if (Lite->Enabled)
		//	Illuminate3D = 1;	// if any enabled lights are exported, illuminate 3d objects
		LSP.PaletteIndex = index++;
		sprintf(LSP.LightSourceName, "LS%d", index);
		switch (Lite->LightType)
			{
			default:
			case WCS_EFFECTS_LIGHTTYPE_PARALLEL:
				{
				//LSP.AmbientRGBA[0] = (float)Lite->Color.CurValue[0];
				//LSP.AmbientRGBA[1] = (float)Lite->Color.CurValue[1];
				//LSP.AmbientRGBA[2] = (float)Lite->Color.CurValue[2];
				LSP.DiffuseRGBA[0] = (float)Lite->Color.CurValue[0];
				LSP.DiffuseRGBA[1] = (float)Lite->Color.CurValue[1];
				LSP.DiffuseRGBA[2] = (float)Lite->Color.CurValue[2];
				LSP.SpecularRGBA[0] = (float)Lite->Color.CurValue[0] * 0.01f;
				LSP.SpecularRGBA[1] = (float)Lite->Color.CurValue[1] * 0.01f;
				LSP.SpecularRGBA[2] = (float)Lite->Color.CurValue[2] * 0.01f;
				//LSP.LightType = 0;
				}
				break;
			case WCS_EFFECTS_LIGHTTYPE_OMNI:
				{
				//LSP.AmbientRGBA[0] = (float)Lite->Color.CurValue[0];
				//LSP.AmbientRGBA[1] = (float)Lite->Color.CurValue[1];
				//LSP.AmbientRGBA[2] = (float)Lite->Color.CurValue[2];
				LSP.DiffuseRGBA[0] = (float)Lite->Color.CurValue[0];
				LSP.DiffuseRGBA[1] = (float)Lite->Color.CurValue[1];
				LSP.DiffuseRGBA[2] = (float)Lite->Color.CurValue[2];
				LSP.SpecularRGBA[0] = (float)Lite->Color.CurValue[0] * 0.01f;
				LSP.SpecularRGBA[1] = (float)Lite->Color.CurValue[1] * 0.01f;
				LSP.SpecularRGBA[2] = (float)Lite->Color.CurValue[2] * 0.01f;
				LSP.LightType = 1;
				}
				break;
			case WCS_EFFECTS_LIGHTTYPE_SPOT:
				{
				//LSP.AmbientRGBA[0] = (float)Lite->Color.CurValue[0];
				//LSP.AmbientRGBA[1] = (float)Lite->Color.CurValue[1];
				//LSP.AmbientRGBA[2] = (float)Lite->Color.CurValue[2];
				LSP.DiffuseRGBA[0] = (float)Lite->Color.CurValue[0];
				LSP.DiffuseRGBA[1] = (float)Lite->Color.CurValue[1];
				LSP.DiffuseRGBA[2] = (float)Lite->Color.CurValue[2];
				LSP.SpecularRGBA[0] = (float)Lite->Color.CurValue[0] * 0.01f;
				LSP.SpecularRGBA[1] = (float)Lite->Color.CurValue[1] * 0.01f;
				LSP.SpecularRGBA[2] = (float)Lite->Color.CurValue[2] * 0.01f;
				LSP.LightType = 2;
				LSP.SpotExpDropoff = (float)Lite->FallOffExp;
				LSP.SpotCutoffAngle = (float)(Lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].CurValue);
				LSP.ConstantAttenuationCoeff = 1.0f;
				}
				break;
			} // switch LightType
		LSP.WriteToFile(fOF_Master);
		LiteList = LiteList->Next;
		} // while LiteList
	} // if which == 0
else
	{
	// write the LightSource
	index = 0;
	LiteList = Master->Lights;
	while (LiteList)
		{
		Lite = (class Light *)LiteList->Me;
		sprintf(LS.ASCII_ID, "L%d", index);
		LS.LightPaletteIndex = index++;
		if (Lite->Enabled)
			LS.Flags = 0xC0000000;	// Enabled + Global
		else
			LS.Flags = 0x40000000;	// Global

		// set the position
		switch (Lite->LightType)
			{
			default:
			case WCS_EFFECTS_LIGHTTYPE_PARALLEL:
				break;
			case WCS_EFFECTS_LIGHTTYPE_OMNI:
			case WCS_EFFECTS_LIGHTTYPE_SPOT:
				Master->RBounds.DefDegToRBounds(Lite->LightPos->Lat, Lite->LightPos->Lon, X, Y);
				LS.PositionXYZ[0] = (X - Master->ExportRefData.ExportRefLon + bias_x) * Master->ExportRefData.ExportLonScale;
				LS.PositionXYZ[1] = (Y - Master->ExportRefData.ExportRefLat + bias_y) * Master->ExportRefData.ExportLatScale;
				LS.PositionXYZ[2] = (Lite->LightPos->Elev) * Master->ExportRefData.ElevScale;
				break;
			} // switch LightType

		// set the direction
		switch (Lite->LightType)
			{
			default:
			case WCS_EFFECTS_LIGHTTYPE_OMNI:
				break;
			case WCS_EFFECTS_LIGHTTYPE_PARALLEL:
				VDEM.CopyXYZ(Lite->LightAim);
				VDEM.Lat = Master->ExportRefData.WCSRefLat;
				VDEM.Lon = Master->ExportRefData.WCSRefLon;
				VDEM.Elev = Master->ExportRefData.RefElev;
				VDEM.RotateToHome();
				LS.Yaw = (float)VDEM.FindAngleYfromZ();	// the heading
				VDEM.RotateY((double)-LS.Yaw);
				LS.Pitch = (float)VDEM.FindAngleXfromZ();
				break;
			case WCS_EFFECTS_LIGHTTYPE_SPOT:
				LS.Yaw = -(float)Lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].CurValue;
				LS.Pitch = (float)Lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_PITCH].CurValue;
				break;
			} // switch LightType
		LS.WriteToFile(fOF_Master);
		LiteList = LiteList->Next;
		} // while LiteList
	} // else

return Err;

} // ExportFormatOpenFlight::ExportLights

/*===========================================================================*/

long ExportFormatOpenFlight::ExportTerrain(const char *inFileName, FILE *foutBaseTile, long HasTexture, long XTile, long YTile,
										   double sw_x, double sw_y, long numLevels, class OF_TexturePalette *TP)
{
double DivFactor, sizex, sizey;
double dx, dy;
RasterResampler *Resamp = NULL;
long i, Success = 1, thisLevel = 1, xsize, ysize;
OF_LOD LOD;
FILE *fOF, *fRaw = NULL, *xfile = NULL;
char resampledFileName[512], meshFileName[512];

if (Master->RBounds.IsGeographic)
	{
	sizex = (Master->OneDEMResX - 1) * Master->RBounds.CellSizeX * Master->ExportRefData.ExportLonScale;
	sizey = (Master->OneDEMResY - 1) * Master->RBounds.CellSizeY * Master->ExportRefData.ExportLatScale;
	//sw_x *= Master->ExportRefData.ExportLonScale;
	//sw_y *= Master->ExportRefData.ExportLatScale;
	} // if
else
	{
	sizex = (Master->OneDEMResX - 1) * Master->RBounds.CellSizeX;
	sizey = (Master->OneDEMResY - 1) * Master->RBounds.CellSizeY;
	} // else
LOD.CenterCoordX = sizex * 0.5 + sw_x * Master->ExportRefData.ExportLonScale;
LOD.CenterCoordY = sizey * 0.5 + sw_y * Master->ExportRefData.ExportLatScale;
LOD.CenterCoordZ = (Master->ExportRefData.RefElev + Master->ExportRefData.MaxElev ) * 0.5;

fOF = foutBaseTile;
Push.WriteToFile(fOF);
Group.WriteToFile(fOF);
Push.WriteToFile(fOF);
sprintf(Group.ASCII_ID, "gT%02d%02d", XTile, YTile);
Group.WriteToFile(fOF);
Push.WriteToFile(fOF);

XRef.Flags = 0xE0000000;

Master->RBounds.DeriveTileCoords(Master->DEMResY, Master->DEMResX, Master->DEMTilesY, Master->DEMTilesX, YTile, XTile, Master->DEMTileOverlap);

EmitFlags = EmitFlaps = 0;
flaps_x = flaps_y = 0;
if (Master->LODFillGaps && (Master->LODLevels != 1))	// add flaps if told to & if we have multiple LOD's
	{
	if (XTile != 0)
		{
		EmitFlags |= FLAPFLAG_EMIT_W;
		flaps_x++;
		} // if
	if (XTile != (Master->DEMTilesX - 1))
		{
		EmitFlags |= FLAPFLAG_EMIT_E;
		flaps_x++;
		} // if
	if (YTile != 0)
		{
		EmitFlags |= FLAPFLAG_EMIT_N;
		flaps_y++;
		} // if
	if (YTile != (Master->DEMTilesY - 1))
		{
		EmitFlags |= FLAPFLAG_EMIT_S;
		flaps_y++;
		} // if
	if (EmitFlags)
		EmitFlaps = 1;
	// translate this tile into correct alignment since we're adding flaps
	dx = dy = 0.0f;
	if (EmitFlags & FLAPFLAG_EMIT_W)
		dx = Master->ExportRefData.ExportLonScale;
	if (EmitFlags & FLAPFLAG_EMIT_S)
		dy = -Master->ExportRefData.ExportLatScale;
	} // if LODCtr != 1

if (Tiled && EmitFlaps)
	{
	long last_ndx = Master->DEMTilesX * Master->DEMTilesY;
	long qwik;

	FlapElev = FLT_MAX;
	if (EmitFlags & FLAPFLAG_EMIT_N)
		{
		qwik = (YTile - 1) * Master->DEMTilesX + XTile - 1;	// NW cell
		if ((qwik >= 0) && (qwik < last_ndx) && (MinTable[qwik] < FlapElev))
			FlapElev = MinTable[qwik];
		qwik++;	// N cell
		if ((qwik >= 0) && (qwik < last_ndx) && (MinTable[qwik] < FlapElev))
			FlapElev = MinTable[qwik];
		qwik++; // NE cell
		if ((qwik >= 0) && (qwik < last_ndx) && (MinTable[qwik] < FlapElev))
			FlapElev = MinTable[qwik];
		} // if EmitFlags & FLAPFLAG_EMIT_N
	if (EmitFlags & FLAPFLAG_EMIT_S)
		{
		qwik = (YTile + 1) * Master->DEMTilesX + XTile - 1;	// SW cell
		if ((qwik >= 0) && (qwik < last_ndx) && (MinTable[qwik] < FlapElev))
			FlapElev = MinTable[qwik];
		qwik++;	// S cell
		if ((qwik >= 0) && (qwik < last_ndx) && (MinTable[qwik] < FlapElev))
			FlapElev = MinTable[qwik];
		qwik++; // SE cell
		if ((qwik >= 0) && (qwik < last_ndx) && (MinTable[qwik] < FlapElev))
			FlapElev = MinTable[qwik];
		} // if EmitFlags & FLAPFLAG_EMIT_S
	if (EmitFlags & FLAPFLAG_EMIT_E)
		{
		qwik = (YTile - 1) * Master->DEMTilesX + XTile + 1;	// NE cell
		if ((qwik >= 0) && (qwik < last_ndx) && (MinTable[qwik] < FlapElev))
			FlapElev = MinTable[qwik];
		qwik += Master->DEMTilesX;	// E cell
		if ((qwik >= 0) && (qwik < last_ndx) && (MinTable[qwik] < FlapElev))
			FlapElev = MinTable[qwik];
		qwik += Master->DEMTilesX;	// SE cell
		if ((qwik >= 0) && (qwik < last_ndx) && (MinTable[qwik] < FlapElev))
			FlapElev = MinTable[qwik];
		} // if EmitFlags & FLAPFLAG_EMIT_E
	if (EmitFlags & FLAPFLAG_EMIT_W)
		{
		qwik = (YTile - 1) * Master->DEMTilesX + XTile - 1;	// NW cell
		if ((qwik >= 0) && (qwik < last_ndx) && (MinTable[qwik] < FlapElev))
			FlapElev = MinTable[qwik];
		qwik += Master->DEMTilesX;	// W cell
		if ((qwik >= 0) && (qwik < last_ndx) && (MinTable[qwik] < FlapElev))
			FlapElev = MinTable[qwik];
		qwik += Master->DEMTilesX;	// SW cell
		if ((qwik >= 0) && (qwik < last_ndx) && (MinTable[qwik] < FlapElev))
			FlapElev = MinTable[qwik];
		} // if EmitFlags & FLAPFLAG_EMIT_W
	FlapElev += ElevAdjust;
	} // if Tiles

do
	{
	if (thisLevel != numLevels)
		{
		sprintf(LOD.ASCII_ID, "l%02d%02d%cL", XTile, YTile, numLevels - thisLevel + 97);	// left sibling
		LOD.SwitchIn = 1000000.0;
		LOD.SwitchOut = (numLevels - thisLevel) * Master->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LODDISTBETWEEN].CurValue;
		LOD.WriteToFile(fOF);
		LOD.Flags = 0x80000000;	// Use previous slant range on any additional LOD's
		if (Resamp = new FloatRasterResampler())
			{
			Resamp->SetNull(-9999.0f);
			strmfp(resampledFileName, Master->OutPath.Path, "lod_tmp.raw");
			DivFactor = pow(2.0, (double)(numLevels - thisLevel));
			xsize = (long)(Master->OneDEMResX / DivFactor);
			if (xsize < 2)
				xsize = 2;
			ysize = (long)(Master->OneDEMResY / DivFactor);
			if (ysize < 2)
				ysize = 2;
			if (Resamp->Resample(inFileName, resampledFileName, Master->OneDEMResY, Master->OneDEMResX, ysize, xsize))
				{
				sprintf(meshFileName, "flight%d_%d%c.flt", XTile, YTile, numLevels - thisLevel + 97);
				if ((fRaw = PROJ_fopen(resampledFileName, "rb")) && (xfile = PROJ_fopen(meshFileName, "wb")))
					{
					if (ExportTerrainFace(fRaw, xfile, HasTexture, XTile, YTile,
							xsize, ysize, sw_x, sw_y, numLevels - thisLevel, TP))
						{
						Push.WriteToFile(fOF);
						strcpy(XRef.ASCII_Path, meshFileName);
						XRef.WriteToFile(fOF);
						Pop.WriteToFile(fOF);
						} // if
					fclose(xfile);
					fclose(fRaw);
					} // if
				} // if
			PROJ_remove(resampledFileName);
			/*** VRML code

			if (Tiles)
				{
				if (Resamp->Resample(RawTerrainFile, ResampledFile,
					Master->OneDEMResY, Master->OneDEMResX, lod_tilesizey, lod_tilesizex))
				//if (Resamp->ResampleTile(RawTerrainFile, ResampledFile, Master->OneDEMResY, Master->OneDEMResX,
				//	lod_tilesizey, lod_tilesizex, Master->DEMTilesY, Master->DEMTilesX, YTile, XTile, Master->DEMTileOverlap))
					{
					// reset RasterBounds
					Master->RBounds.DeriveTileCoords(Master->DEMResY, Master->DEMResX, 
						Master->DEMTilesY, Master->DEMTilesX, YTile, XTile, Master->DEMTileOverlap);
					fRaw = PROJ_fopen(ResampledFile, "rb");
					} // if
				//else
					//UserMessageOK(RawTerrainFile, "Error tiling this file. Export terminated.");
				} // if Tiles
			else
				{
				if (Resamp->Resample(RawTerrainFile, ResampledFile,
					Master->DEMResY, Master->DEMResX, lod_tilesizey, lod_tilesizex))
					{
					// reset RasterBounds
					Master->RBounds.DeriveCoords(Master->DEMResY, Master->DEMResX);
					fRaw = PROJ_fopen(ResampledFile, "rb");
					} // if
				//else
					//UserMessageOK(RawTerrainFile, "Error resampling this file. Export terminated.");
				} // else Tiles

			***/
			delete Resamp;
			Resamp = NULL;
			} // if
		sprintf(LOD.ASCII_ID, "l%02d%02d%cR", XTile, YTile, numLevels - thisLevel + 97);	// right sibling
		LOD.SwitchIn = (numLevels - thisLevel) * Master->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LODDISTBETWEEN].CurValue;
		LOD.SwitchOut = 0;
		LOD.WriteToFile(fOF);
		Push.WriteToFile(fOF);
		thisLevel++;
		} // if
	else
		{
		// last copy = full size
		sprintf(meshFileName, "flight%d_%da.flt", XTile, YTile);
		if ((fRaw = PROJ_fopen(inFileName, "rb")) && (xfile = PROJ_fopen(meshFileName, "wb")))
			{
			xsize = Master->OneDEMResX;
			ysize = Master->OneDEMResY;
			if (ExportTerrainFace(fRaw, xfile, HasTexture, XTile, YTile,
					xsize, ysize, sw_x, sw_y, numLevels - thisLevel, TP))
				{
				//Push.WriteToFile(fOF);
				strcpy(XRef.ASCII_Path, meshFileName);
				XRef.WriteToFile(fOF);
				//Pop.WriteToFile(fOF);
				} // if
			fclose(xfile);
			fclose(fRaw);
			} // if
		thisLevel++;
		} // else
	} while (thisLevel <= numLevels);

for (i = 0; i < numLevels; i++)
		Pop.WriteToFile(fOF);
//Pop.WriteToFile(fOF);

PROJ_remove(inFileName);

return Success;

} // ExportFormatOpenFlight::ExportTerrain

/*===========================================================================*/

long ExportFormatOpenFlight::ExportTerrainFace(FILE *rawElevs, FILE *fOF, long HasTexture, long XTile, long YTile,
											   long xsize, long ysize, double sw_x, double sw_y, long levelCode, class OF_TexturePalette *TP)
{
double sizex, sizey, vx, vy, vz, xstep;	// sizes, vertex positions
float XTexConst, XTexFactor, YTexConst, YTexFactor;
unsigned long f = 0, n, m;
long Success = 1, x, xstart, xmax, y, ystart, ymax;
OF_ColorPalette CP;
OF_Face Face;
OF_Group Group;
OF_Header Hdr;
OF_Vertex_withColorUV V_wCUV;
OF_VertexList VL;
OF_VertexPalette VP;
OF_Object Object;

sprintf(Hdr.ASCII_ID, "db%02d%02d%c", XTile, YTile, levelCode + 97);
Hdr.WriteToFile(fOF);
CP.WriteToFile(fOF);

// since damn openflight dlls apparently don't deal with inherited textures correctly...
TP->WriteToFile(fOF);

if (Master->RBounds.IsGeographic)
	{
	sizex = (Master->OneDEMResX - 1) * Master->RBounds.CellSizeX * Master->ExportRefData.ExportLonScale;
	sizey = (Master->OneDEMResY - 1) * Master->RBounds.CellSizeY * Master->ExportRefData.ExportLatScale;
	sw_x *= Master->ExportRefData.ExportLonScale;
	sw_y *= Master->ExportRefData.ExportLatScale;
	} // if
else
	{
	sizex = (Master->OneDEMResX - 1) * Master->RBounds.CellSizeX;
	sizey = (Master->OneDEMResY - 1) * Master->RBounds.CellSizeY;
	} // else

// create the vertex list for this DEM
// coord 0,0 is located at SW corner of SW tile (not an OF restriction, but normal convention apparently)
VP.TotalLength = (xsize + flaps_x) * (ysize + flaps_y) * 48 + 8;
VP.WriteToFile(fOF);

if (EmitFlags & FLAPFLAG_EMIT_W)
	xstart = -1;
else
	xstart = 0;
xmax = xstart + xsize + flaps_x;
if (EmitFlags & FLAPFLAG_EMIT_S)
	ystart = -1;
else
	ystart = 0;
ymax = ystart + ysize + flaps_y;

xstep = sizex / (xsize - 1);

XTexConst = 0.5f / Master->OneTexResX;	// half texture pixel
XTexFactor = (Master->OneTexResX - 2.0f) / Master->OneTexResX;
YTexConst = 0.5f / Master->OneTexResY;
YTexFactor = (Master->OneTexResY - 2.0f) / Master->OneTexResY;
for (y = ystart; y < ymax; y++)
	{
	float v, ym1;

	ym1 = (float)(ysize - 1);
	v = (ym1 - y) / ym1 * YTexFactor + YTexConst;	// cheat texture end coords by 1/2 texture pixel size
	vx = xstart * xstep;
	vy = (ym1 - y) * sizey / ym1;
	for (x = xstart; x < xmax; x++)
		{
		float elev, u;

		u = x / (float)(xsize - 1) * XTexFactor + XTexConst;
		// see if we're on the elev model or a flap
		if ((x >=0) && (x < xsize) && (y >= 0) && (y < ysize))
			fread((void *)&elev, 1, 4, rawElevs);
		else
			elev = (float)FlapElev;
		vz = elev;
		V_wCUV.VertexCoordX = vx + sw_x;
		V_wCUV.VertexCoordY = vy + sw_y;
		V_wCUV.VertexCoordZ = vz;
		if (HasTexture)
			{
			V_wCUV.TextureCoordU = u;
			V_wCUV.TextureCoordV = v;
			} // if
		V_wCUV.WriteToFile(fOF);

		vx += xstep;

		} // for x
	} // for y

Push.WriteToFile(fOF);
Group.WriteToFile(fOF);
Push.WriteToFile(fOF);
sprintf(Group.ASCII_ID, "gT%02d%02d%c", XTile, YTile, levelCode + 97);
Group.WriteToFile(fOF);
Push.WriteToFile(fOF);
sprintf(Object.ASCII_ID, "oT%02d%02d%c", XTile, YTile, levelCode + 97);
Object.RelPri = 0;
Object.WriteToFile(fOF);
Push.WriteToFile(fOF);

VL.Length = 16;
Face.TexPatIndex = 0;
Face.Flags = 0xC0000000;
for (y = 1; y < (ysize + flaps_y); y++)
	{
	n = (xsize + flaps_x) * (y - 1);
	m = (xsize + flaps_x) * y;
	for (x = 1; x < (xsize + flaps_x); x++)
		{
		// Top Left Face - CCW order from n
		//
		// n -- n+1
		// |  /
		// | /
		// |/
		// m

		sprintf(Face.ASCII_ID, "f%d", f++);
		Face.WriteToFile(fOF);
		Push.WriteToFile(fOF);
		VL.WriteToFile(fOF);
		// below --> 48 is size in bytes of Vertex with Color and UV Record (V_wCUV), 8 is size of Vertex Palette Header Record
		(void)PutB32U(n * 48 + 8, fOF);
		(void)PutB32U(m * 48 + 8, fOF);
		(void)PutB32U(n * 48 + 48 + 8, fOF);
		Pop.WriteToFile(fOF);

		// Bottom Right Face - CCW order from n+1
		//
		//      n+1
		//    / |
		//   /  |
		//  /   |
		// m -- m+1

		sprintf(Face.ASCII_ID, "f%d", f++);
		Face.WriteToFile(fOF);
		Push.WriteToFile(fOF);
		VL.WriteToFile(fOF);
		(void)PutB32U(n * 48 + 48 + 8, fOF);
		(void)PutB32U(m * 48 + 8, fOF);
		(void)PutB32U(m * 48 + 48 + 8, fOF);
		Pop.WriteToFile(fOF);

		n++; m++;

		} // for x
	} // for y

Pop.WriteToFile(fOF);
Pop.WriteToFile(fOF);
Pop.WriteToFile(fOF);
Pop.WriteToFile(fOF);

return Success;

} // ExportFormatOpenFlight::ExportTerrainFace

/*===========================================================================*/

long ExportFormatOpenFlight::ExportTerrainMesh(FILE *RawElev, FILE *fOF, long HasTexture)
{
double vx, vy, vz;
float elev;
unsigned long LastVert, temp, ThisVert = 0, v1, v2, VertsLeft, vsize;
static long MeshNum = 1;
long Success = 1, x, y;
OF_Continuation Cont;
OF_Group Group;
OF_LocalVertexPool LVP;
OF_Mesh Mesh;
OF_MeshPrimitive MP;

Push.WriteToFile(fOF);
strcpy(Group.ASCII_ID, "gT");
Group.WriteToFile(fOF);
Push.WriteToFile(fOF);

if (HasTexture)
	{
	Mesh.TexPatIndex = 0;
	LVP.AttributeMask = 0x88000000;	// HasPosition + HasBaseUV
	vsize = 8 * 3 + 4 * 2;			// 3 doubles, 2 floats
	} // if
else
	{
	LVP.AttributeMask = 0x80000000;	// HasPosition
	vsize = 8 * 3;					// 3 doubles
	} // else
sprintf(Mesh.ASCII_ID, "m%d", MeshNum++);
Mesh.Flags = 0xA0000000;	// Terrain, No alternate color
Mesh.LightMode = 2;			// Use mesh color & vertex normals
Mesh.TexPatIndex = 0;
Mesh.WriteToFile(fOF);

LVP.NumVerts = VertsLeft = Master->OneDEMResX * Master->OneDEMResY;
temp = VertsLeft * vsize + 12;
if (temp >= 65536)
	{
	LastVert = (65536 - 12) / vsize;
	VertsLeft -= LastVert;
	LVP.Length = (unsigned short)(LastVert * vsize + 12);
	} // if
else
	{
	LastVert = 0;
	LVP.Length = (unsigned short)temp;
	} // else

LVP.WriteToFile(fOF);
for (y = 0; y < Master->OneDEMResY; y++)
	{
	float v;

	v = (Master->OneDEMResY - 1 - y) / (float)(Master->OneDEMResY - 1);
	vx = Master->RBounds.ULcorner.x;
	vy = Master->RBounds.ULcorner.y - y * Master->RBounds.CellSizeY;
	for (x = 0; x < Master->OneDEMResX; x++)
		{
		float u;

		u = x / (float)(Master->OneDEMResX - 1);
		fread((void *)&elev, 1, 4, RawElev);
		//elev *= scaling
		vz = elev;
		PutB64(&vx, fOF);
		PutB64(&vy, fOF);
		PutB64(&vz, fOF);
		if (HasTexture)
			{
			PutB32F(&u, fOF);
			PutB32F(&v, fOF);
			} // if

		vx += Master->RBounds.CellSizeX;

		// do we need a continuation record?
		if (LastVert)
			{
			ThisVert++;
			if (ThisVert == LastVert)
				{
				ThisVert = 0;
				temp = VertsLeft * vsize + 4;
				if (temp >= 65536)
					{
					LastVert = 65536 / vsize;
					VertsLeft -= LastVert;
					Cont.ContinueLength = LastVert * vsize + 4;
					} // if
				else
					{
					LastVert = 0;
					Cont.ContinueLength = temp + 4;
					} // else
				Cont.Length = (unsigned short)Cont.ContinueLength + 4;
				Cont.WriteToFile(fOF);
				} // if
			} // if LastVert
		} // for x
	} // for y

long fpos = ftell(fOF);	// testing only
Push.WriteToFile(fOF);

MP.Length = (unsigned short)(Master->OneDEMResX * 2 * 4 + 12);
MP.PrimitiveType = 3;	// Quad Strip
MP.IndexSize = 4;
MP.VertexCount = Master->OneDEMResX * 2;

// fOF each row as a quad strip
for (y = 1; y < Master->OneDEMResY; y++)
	{
	MP.WriteToFile(fOF);	// begin the Mesh Primitive Record
	v1 = y * Master->OneDEMResX;	// OF docs are wrong - start at vertex #0, not 1
	v2 = v1 - Master->OneDEMResX;
	for (x = 0; x < Master->OneDEMResX; x++, v1++, v2++)
		{
		PutB32S(v1, fOF);
		PutB32S(v2, fOF);
		} // for x
	} // for y

Pop.WriteToFile(fOF);
Pop.WriteToFile(fOF);
Pop.WriteToFile(fOF);
Pop.WriteToFile(fOF);

return Success;

} // ExportFormatOpenFlight::ExportTerrainMesh

/*===========================================================================*/

float ExportFormatOpenFlight::MinElev(FILE *RawElevs, long Cols, long Rows)
{
float min = FLT_MAX, val;
long i, j = Cols * Rows;

if (RawElevs)
	{
	rewind(RawElevs);
	for (i = 0; i < j; i++)
		{
		fread(&val, 1, 4, RawElevs);
		if ((val != -9999.0f) && (val < min))
			min = val;
		} // for

	//if (min == -9999.0f) commented out 04/12/05
	//	val = 0.0f;
	} // if
else
	min = 0.0f;

return min;

} // ExportFormatOpenFlight::MinElev

/*===========================================================================*/

int ExportFormatOpenFlight::PackageExport(NameList **FileNamesCreated)
{
FILE *fRaw = NULL;
int Err, Success = 1;
long FileType, PatternIndex = 0, XTile, YTile;
PathAndFile SceneOutput;
char TempFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];
const char *OutputFilePath, *RawTerrainFile, *TextureFile;
OF_ColorPalette CP;
OF_VertexPalette VP;
size_t MinTableSize;

// The directory where all the files should be created is:
OutputFilePath = Master->OutPath.GetPath();

SceneOutput.SetPath((char *)Master->OutPath.GetPath());
SceneOutput.SetName((char *)Master->OutPath.GetName());
SceneOutput.GetFramePathAndName(TempFullPath, ".flt", 0, WCS_PATHANDFILE_PATH_PLUS_NAME_LEN, 0);
fOF_Master = PROJ_fopen(TempFullPath, "wb");

if (fOF_Master)
	{
	// change from 0,0 at center of export area to 0,0 at SW corner
	bias_x = Master->DEMTilesX * (Master->OneDEMResX - 1) * Master->RBounds.CellSizeX * 0.5;
	bias_y = Master->DEMTilesY * (Master->OneDEMResY - 1) * Master->RBounds.CellSizeY * 0.5;

	//MasterHeader.FormatRev = OF_GetRev();
	//MasterHeader.ProjectionType = 6;	// Geodetic - for testing
	MasterHeader.ProjectionType = 0;	// Flat earth
	MasterHeader.EllipsoidModel = 0;	// WGS84 - for testing
	//MasterHeader.MajorAxis = 
	MasterHeader.SW_Corn_Lat = Master->RBounds.LRcorner.y;
	MasterHeader.SW_Corn_Lon = Master->RBounds.ULcorner.x;
	MasterHeader.NE_Corn_Lat = Master->RBounds.ULcorner.y;
	MasterHeader.NE_Corn_Lon = Master->RBounds.LRcorner.x;
	MasterHeader.OriginLat = (MasterHeader.SW_Corn_Lat + MasterHeader.NE_Corn_Lat) * 0.5;
	MasterHeader.OriginLon = (MasterHeader.SW_Corn_Lon + MasterHeader.NE_Corn_Lon) * 0.5;
	strcpy(MasterHeader.DateAndTime, TimeStamp());
	MasterHeader.WriteToFile(fOF_Master);
	Err = CP.WriteToFile(fOF_Master);

	if ((!Err) && Master->ExportLights)
		Err = ExportLights(0);

	if (!Err)
		Err = ExportCameras(fOF_Master, Master->Cameras);

	// write an empty VP, since many readers seem to expect to find one
	VP.TotalLength = 8;
	Err = VP.WriteToFile(fOF_Master);

	Push.WriteToFile(fOF_Master);
	strcpy(Group.ASCII_ID, "gRoot");
	Group.WriteToFile(fOF_Master);
	Push.WriteToFile(fOF_Master);
	strcpy(Group.ASCII_ID, "gMaster");
	Group.WriteToFile(fOF_Master);

	if (!Err)
		{
		double base_x, base_y;
		FILE *fTerrain = NULL;
		OF_Header TerrainHdr;
		OF_TexturePalette TextureRec;

		TerrainHdr.FormatRev = OF_GetRev();
		if ((Master->DEMTilesX > 1) || (Master->DEMTilesY > 1))
			{
			Tiled = 1;
			FileType = WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN;
			RawTerrainFile = NULL;
			RawTerrainFile = (*FileNamesCreated)->FindNextNameOfType(FileType, RawTerrainFile);
			fRaw = PROJ_fopen(RawTerrainFile, "rb");

			FileType = WCS_EXPORTCONTROL_FILETYPE_TEX1;
			TextureFile = NULL;
			TextureFile = (*FileNamesCreated)->FindNextNameOfType(FileType, TextureFile);

			//tilesizex = (long)((Master->DEMResX + Master->DEMTilesX - 1) / Master->DEMTilesX);
			//tilesizey = (long)((Master->DEMResY + Master->DEMTilesY - 1) / Master->DEMTilesY);
			} // if tiles
		else
			{
			FileType = WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN;
			RawTerrainFile = (*FileNamesCreated)->FindNameOfType(FileType);
			fRaw = PROJ_fopen(RawTerrainFile, "rb");

			FileType = WCS_EXPORTCONTROL_FILETYPE_TEX1;
			TextureFile = (*FileNamesCreated)->FindNameOfType(FileType);
			//tilesizex = Master->DEMResX;
			//tilesizey = Master->DEMResY;
			} // else tiles

		//Push.WriteToFile(fOF_Master);
		//sprintf(Group.ASCII_ID, "g%d", GroupCounter++);
		//Group.WriteToFile(fOF_Master);
		Push.WriteToFile(fOF_Master);
		Err = ExportLights(1);
		strcpy(Group.ASCII_ID, "gLiteMe");
		Group.WriteToFile(fOF_Master);
		Push.WriteToFile(fOF_Master);

		MinTableSize = sizeof(float) * Master->DEMTilesX * Master->DEMTilesY;
		MinTable = (float *)AppMem_Alloc(MinTableSize, APPMEM_CLEAR);
		if (MinTable == NULL)
			{
			Success = 0;
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "OpenFlight Export aborted - can't create MinTable!");
			goto Abort;
			} // if
		if (Tiled)
			{
			FileType = WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN;
			// scan tiles for min elev
			for (long ndx = 0, YTile = 0; YTile < Master->DEMTilesY; YTile++)
				{
				for (XTile = 0; XTile < Master->DEMTilesX; XTile++, ndx++)
					{
					MinTable[ndx] = MinElev(fRaw, Master->OneDEMResX, Master->OneDEMResY);
					if (fRaw)
						fclose(fRaw);
					RawTerrainFile = (*FileNamesCreated)->FindNextNameOfType(FileType, RawTerrainFile);
					fRaw = PROJ_fopen(RawTerrainFile, "rb");
					} // for XTile
				} // for YTile
			// reset back to the first tile
			RawTerrainFile = NULL;
			RawTerrainFile = (*FileNamesCreated)->FindNextNameOfType(FileType, RawTerrainFile);
			fRaw = PROJ_fopen(RawTerrainFile, "rb");
			} // if Tiles
		else
			{
			MinTable[0] = MinElev(fRaw, Master->OneDEMResX, Master->OneDEMResY);
			} // else Tiles

		for (YTile = 0; YTile < Master->DEMTilesY; YTile++)
			{
			base_y = (Master->DEMTilesY - (YTile + 1)) * (Master->OneDEMResY - 1) * Master->RBounds.CellSizeY;
			for (XTile = 0; XTile < Master->DEMTilesX; XTile++)
				{
				long YNum = Master->DEMTilesY - (YTile + 1);

				base_x = XTile * (Master->OneDEMResX - 1) * Master->RBounds.CellSizeX;
				sprintf(XRef.ASCII_Path, "flight%d_%d.flt", XTile, YNum);
				SceneOutput.SetPath((char *)Master->OutPath.GetPath());
				SceneOutput.SetName(XRef.ASCII_Path);
				SceneOutput.GetFramePathAndName(TempFullPath, NULL, 0, WCS_PATHANDFILE_PATH_PLUS_NAME_LEN, 0);
				if (fTerrain = PROJ_fopen(TempFullPath, "wb"))
					{
					long HasTexture;

					XRef.Flags = 0xE0000000;
					XRef.WriteToFile(fOF_Master);
					sprintf(TerrainHdr.ASCII_ID, "db%d_%d", XTile, YNum);
					TerrainHdr.WriteToFile(fTerrain);
					CP.WriteToFile(fTerrain);
					if (TextureFile)
						{
						FILE *fATTR;

						HasTexture = 1;
						strcpy(TextureRec.Filename, TextureFile);
						TextureRec.PatternIndex = PatternIndex;
						TextureRec.WriteToFile(fTerrain);
						strcat(TextureRec.Filename, ".attr");
						if (fATTR = PROJ_fopen(TextureRec.Filename, "wb"))
							{
							OF_TextureAttributeFile TAF;

							TAF.TexelsU = Master->OneTexResX;
							TAF.TexelsV = Master->OneTexResY;
							TAF.WriteToFile(fATTR);
							fclose(fATTR);
							} // if
						strcpy(TextureRec.Filename, TextureFile);
						} // if
					else
						HasTexture = 0;

					VP.WriteToFile(fTerrain);
					if (Master->ExportTerrain)
						{
						ExportTerrain(RawTerrainFile, fTerrain, HasTexture, XTile, YNum, base_x, base_y, Master->LODLevels, &TextureRec);
						PROJ_remove(RawTerrainFile);
						} // if
					//else
					//	{
					//	VP.WriteToFile(fTerrain);
					//	} // else

					if (Master->Export3DObjects || Master->ExportWalls || Master->Export3DFoliage)
						Process3DObjects(FileNamesCreated, OutputFilePath, fTerrain, XTile, YNum, base_x, base_y);

					if (Master->ExportFoliage)
						ProcessFoliageList(FileNamesCreated, OutputFilePath, fTerrain, XTile, YNum, base_x, base_y);

					Pop.WriteToFile(fTerrain);
					Pop.WriteToFile(fTerrain);

					if (fRaw)
						fclose(fRaw);
					fRaw = NULL;
					if (Tiled)
						{
						FileType = WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN;
						RawTerrainFile = (*FileNamesCreated)->FindNextNameOfType(FileType, RawTerrainFile);
						fRaw = PROJ_fopen(RawTerrainFile, "rb");

						FileType = WCS_EXPORTCONTROL_FILETYPE_TEX1;
						TextureFile = (*FileNamesCreated)->FindNextNameOfType(FileType, TextureFile);
						} // if tiles

					fclose(fTerrain);
					} // if fTerrain
				} // for XTile
			} // for YTile

		if (fRaw)
			{
			fclose(fRaw);
			fRaw = NULL;
			} // if

		Cleanup(Tiled, OutputFilePath, FileNamesCreated);

		if (MinTable)
			AppMem_Free(MinTable, MinTableSize);

		} // if ExportTerrain

	if (Master->RBounds.IsGeographic)
		{
		bias_x = Master->DEMTilesX * (Master->OneDEMResX - 1) * Master->RBounds.CellSizeX * Master->ExportRefData.ExportLonScale * 0.5;
		bias_y = Master->DEMTilesY * (Master->OneDEMResY - 1) * Master->RBounds.CellSizeY * Master->ExportRefData.ExportLatScale * 0.5;
		} // if

	if (Master->ExportSky)
		{
		Success = ProcessSky(FileNamesCreated, OutputFilePath);
		} // if ExportSky

/***
	if (Success && Master->ExportFoliage)
		{
		sprintf(XRef.ASCII_Path, "foliage.flt");
		ProcessFoliageList(FileNamesCreated, OutputFilePath);
		} // if ExportFoliage

	if (Master->Export3DObjects || Master->ExportWalls || Master->Export3DFoliage)
		Process3DObjects(FileNamesCreated, OutputFilePath, fOF_Master);
***/

	if (Master->ExportVectors && Master->VecInstanceList)
		ProcessVectors(OutputFilePath);

	Pop.WriteToFile(fOF_Master);
	Pop.WriteToFile(fOF_Master);
	Pop.WriteToFile(fOF_Master);
	Pop.WriteToFile(fOF_Master);
	fclose(fOF_Master);
	fOF_Master = NULL;
	} // if


Abort:

return Success;

} // ExportFormatOpenFlight::PackageExport

/*===========================================================================*/

int ExportFormatOpenFlight::Process3DObjects(NameList **FileNamesCreated, const char *OutputFilePath, FILE *fOF_Parent,
											 long XTile, long YTile, double ref_x, double ref_y)
{
double BBot, BLeft, BRight, BTop;
double ExportX, ExportY;
double /* geo[3], rot[3], scale[3], */ x, y, z; /*, trans[3];*/
FILE *fVertSeeks = NULL;	// a sideways solution to the chicken & eggs problem
Object3DInstance *ObjectList = Master->ObjectInstanceList;
Object3DEffect *Object3D;
ObjectPerVertexMap *UVMap = NULL;
int Success = 1;
//float boxx, boxy, boxz,boxctr[3], dx, dy, dz;
static long lodnum = 0, n = 0;
long Illuminate3D = 0, /* MatCt, */ PolyCt, VertCt;
OF_ColorPalette CP;
OF_Face Face;
OF_Header Hdr;
OF_VertexPalette VP;
OF_Vertex_withColorNormal V_wCN;
OF_Vertex_withColorNormalUV V_wCNUV;
OF_Vertex_withColor V_wC;
OF_Vertex_withColorUV V_wCUV;
PathAndFile objPAF;
char objFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

fVertSeeks = PROJ_fopen("VertSeeks.tmp", "wb+");
if (fVertSeeks == NULL)
	return 0;	// Ack!!! Failure!

// light objects if exporting lights
if (Master->ExportLights)
	Illuminate3D = 1;

// bounds of this tile
if (Master->RBounds.IsGeographic)
	{
	BLeft = ref_x * Master->ExportRefData.ExportLonScale;
	BRight = (ref_x + Master->OneDEMResX * Master->RBounds.CellSizeX) * Master->ExportRefData.ExportLonScale;
	BBot = ref_y * Master->ExportRefData.ExportLatScale;
	BTop = (ref_y + Master->OneDEMResY * Master->RBounds.CellSizeY) * Master->ExportRefData.ExportLatScale ;
	} //
else
	{
	BLeft = ref_x;
	BRight = ref_x + Master->OneDEMResX * Master->RBounds.CellSizeX;
	BBot = ref_y;
	BTop = ref_y + Master->OneDEMResY * Master->RBounds.CellSizeY;
	} // else

while (ObjectList)
	{
	//Point4d AxisAngle;
	FILE *fOF_Obj;
	long DoIt = 0;
	short VertTotal;
	//long i;
	char ObjName[32];
	OF_Group Group;
	OF_Object Obj;
	OF_TexturePalette TP;

	rewind(fVertSeeks);	// recycle the file for all objects
	VertTotal = 0;

	// see if this object falls on the tile we're exporting
	ExportX = (ObjectList->ExportXYZ[0] + bias_x) * Master->ExportRefData.ExportLonScale;
	ExportY = (ObjectList->ExportXYZ[1] + bias_y) * Master->ExportRefData.ExportLatScale;
	if ((ExportX >= BLeft) && (ExportX < BRight) && (ExportY >= BBot) && (ExportY < BTop))
		DoIt = 1;
	if (!DoIt)
		goto SkipObject;
	strcpy(Hdr.DateAndTime, TimeStamp());
	Face.DrawType = 0;	// Draw solid, use backface culling
	//Face.DrawType = 1;	// Draw solid, no backface culling
	Face.ColorNameIndex = 0x007f;		// no clue...
	Face.AltColorNameIndex = 0x7f00;	// no clue...
	Face.TexPatIndex = -1;
	Face.LightMode = 2;
	//Face.Flags = 0x20000000;
	Face.Flags = 0x00000000;
	Face.TexMapIndex = -1;
	Face.PrimaryColorIndex = 127;	// color palette entry 0, intensity 127 (max)
	Face.AlternateColorIndex = 0xffffffff;
	Face.PackedColorPrimary = 0xffffffff;
	Face.PackedColorAlternate = 0;
	Object3D = ObjectList->MyObj;
	if (!((Object3D->Vertices && Object3D->Polygons && Object3D->NameTable) || Object3D->OpenInputFile(NULL, FALSE, FALSE, FALSE)))
		goto SkipObject;
	// F2 - 07/29/2008: Uncommented these next two lines.  No record of why they were commented out at this point.
	strcpy(ObjName, Object3D->ShortName);
	strcat(ObjName, ".flt");
	objPAF.SetPath(OutputFilePath);
	objPAF.SetName(Object3D->ShortName);
	objPAF.GetFramePathAndName(objFullPath, ".flt", 0, WCS_PATHANDFILE_PATH_PLUS_NAME_LEN, 0);
	if (fOF_Obj = PROJ_fopen(objFullPath, "wb"))
		{
		double RotX, RotY, RotZ;
		double mA, mB, mC, mD, mE, mF, mAD, mBD;
		ObjectMaterialEntry *NameTable;
		MaterialEffect *Mat;
		RootTexture *RootTex;
		Texture *UVTex;
		Raster *Rast;
		//float u, v;
		long HasUV, MatCt, /*** VertSize, ***/ VOffset = 8, VPTotalLength;
		OF_Matrix Matrix;
		OF_LevelOfDetail LOD;
		OF_LightSourcePalette LSP;

		if (! Illuminate3D)
			Obj.Flags |= 0x1000000;	// Don't illuminate (self-illuminating)

		strcpy(Hdr.ASCII_ID, "db");
		Hdr.Flags = 0xC0000000;	// Save Vertex Normals & Packed Color Mode
		Hdr.WriteToFile(fOF_Obj);
		CP.WriteToFile(fOF_Obj);

		// make material palettes
		for (MatCt = 0; MatCt < Object3D->NumMaterials; MatCt++)
			ProcessMatTexture(MatCt, fOF_Obj, Object3D);

		// make texture palettes
		for (MatCt = 0; MatCt < Object3D->NumMaterials; MatCt++)
			{
			NameTable = (ObjectMaterialEntry *)&Object3D->NameTable[MatCt];
			HasUV = FALSE;
			if (! NameTable->Mat)
				NameTable->Mat = (MaterialEffect *)GlobalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, NameTable->Name);
			if (Mat = NameTable->Mat)
				{
				if (Object3D->VertexUVWAvailable)
					{
					HasUV = TRUE;
					if (RootTex = Mat->GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
						{
						if (UVTex = RootTex->Tex)
							{
							if (UVTex->TexType == WCS_TEXTURE_TYPE_UVW)
								{
								if (UVTex->Img && (Rast = UVTex->Img->GetRaster()))
									{
									strcpy(TP.Filename, Rast->GetName());
									TP.PatternIndex = MatCt;
									TP.WriteToFile(fOF_Obj);
									} // if
								} // if if UVTex->TexType
							} // if UVTex
						} // if RootTex
					} // if Object3D->VertexUVWAvailable
				} // if Mat
			} // for

		LSP.WriteToFile(fOF_Obj);

		VPTotalLength = ftell(fOF_Obj) + 4;
		VP.WriteToFile(fOF_Obj);
		sprintf(Group.ASCII_ID, "3g%d", n);
		sprintf(Obj.ASCII_ID, "3d%d", n);

		UVMap = &Object3D->UVWTable[0];
		/*** debug stuff
		if (UVMap)
			{
			for (long node = 0; node < UVMap->NumNodes; node++)
				{
				char string[512];

				sprintf(string, "%3d %d %lf %lf %lf\n", node, UVMap->CoordsValid[node],
					UVMap->CoordsArray[0][node], UVMap->CoordsArray[1][node], UVMap->CoordsArray[2][node]);
				DEBUGOUT(string);
				}
			} // if UVMap
		***/
		if (Illuminate3D)
			{
			for (PolyCt = 0; PolyCt < Object3D->NumPolys; PolyCt++)
				{
				long Mat, ShadeType;
				
				Mat = Object3D->Polygons[PolyCt].Material;
				ShadeType = Object3D->NameTable[Mat].Mat->Shading;

				// we need to generate normals
				if (ShadeType == WCS_EFFECT_MATERIAL_SHADING_FLAT)
					{
					// we need normals for each vertex
					for (VertCt = Object3D->Polygons[PolyCt].NumVerts - 1; VertCt >= 0; VertCt--)
						{
						if (UVMap && UVMap->CoordsValid[Object3D->Polygons[PolyCt].VertRef[VertCt]])
							{
							V_wCNUV.VertexCoordX = Object3D->Vertices[Object3D->Polygons[PolyCt].VertRef[VertCt]].xyz[0];
							V_wCNUV.VertexCoordY = Object3D->Vertices[Object3D->Polygons[PolyCt].VertRef[VertCt]].xyz[2];
							V_wCNUV.VertexCoordZ = Object3D->Vertices[Object3D->Polygons[PolyCt].VertRef[VertCt]].xyz[1];
							V_wCNUV.VertexNormalI = -(float)Object3D->Polygons[PolyCt].Normal[0];
							V_wCNUV.VertexNormalJ = (float)Object3D->Polygons[PolyCt].Normal[2];
							V_wCNUV.VertexNormalK = -(float)Object3D->Polygons[PolyCt].Normal[1];
							V_wCNUV.TextureCoordU = UVMap->CoordsArray[0][Object3D->Polygons[PolyCt].VertRef[VertCt]];
							V_wCNUV.TextureCoordV = UVMap->CoordsArray[1][Object3D->Polygons[PolyCt].VertRef[VertCt]];
							V_wCNUV.WriteToFile(fOF_Obj);
							fwrite(&VOffset, 4, 1, fVertSeeks);
							VOffset += V_wCNUV.Length;
							VertTotal++;
							} // if UV coords
						else
							{
							V_wCN.VertexCoordX = Object3D->Vertices[Object3D->Polygons[PolyCt].VertRef[VertCt]].xyz[0];
							V_wCN.VertexCoordY = Object3D->Vertices[Object3D->Polygons[PolyCt].VertRef[VertCt]].xyz[2];
							V_wCN.VertexCoordZ = Object3D->Vertices[Object3D->Polygons[PolyCt].VertRef[VertCt]].xyz[1];
							V_wCN.VertexNormalI = -(float)Object3D->Polygons[PolyCt].Normal[0];
							V_wCN.VertexNormalJ = (float)Object3D->Polygons[PolyCt].Normal[2];
							V_wCN.VertexNormalK = -(float)Object3D->Polygons[PolyCt].Normal[1];
							V_wCN.WriteToFile(fOF_Obj);
							fwrite(&VOffset, 4, 1, fVertSeeks);
							VOffset += V_wCN.Length;
							VertTotal++;
							} // else HasUV
						} // for
					} // flat shaded
				else if ((ShadeType == WCS_EFFECT_MATERIAL_SHADING_PHONG) && Object3D->CalcNormals)
					{
					for (VertCt = Object3D->Polygons[PolyCt].NumVerts - 1; VertCt >= 0; VertCt--)
						{
						Object3D->Vertices[Object3D->Polygons[PolyCt].VertRef[VertCt]].Normalized = 0;
						Object3D->Vertices[Object3D->Polygons[PolyCt].VertRef[VertCt]].Normalize(Object3D->Polygons, Object3D->Vertices, Object3D->Polygons, Object3D->NameTable);
						} // for
					for (VertCt = Object3D->Polygons[PolyCt].NumVerts - 1; VertCt >= 0; VertCt--)
						{
						if (UVMap && UVMap->CoordsValid[Object3D->Polygons[PolyCt].VertRef[VertCt]])
							{
							V_wCNUV.VertexCoordX = Object3D->Vertices[Object3D->Polygons[PolyCt].VertRef[VertCt]].xyz[0];
							V_wCNUV.VertexCoordY = Object3D->Vertices[Object3D->Polygons[PolyCt].VertRef[VertCt]].xyz[2];
							V_wCNUV.VertexCoordZ = Object3D->Vertices[Object3D->Polygons[PolyCt].VertRef[VertCt]].xyz[1];
							V_wCNUV.VertexNormalI = -(float)Object3D->Polygons[PolyCt].Normal[0];
							V_wCNUV.VertexNormalJ = (float)Object3D->Polygons[PolyCt].Normal[2];
							V_wCNUV.VertexNormalK = -(float)Object3D->Polygons[PolyCt].Normal[1];
							V_wCNUV.TextureCoordU = UVMap->CoordsArray[0][Object3D->Polygons[PolyCt].VertRef[VertCt]];
							V_wCNUV.TextureCoordV = UVMap->CoordsArray[1][Object3D->Polygons[PolyCt].VertRef[VertCt]];
							V_wCNUV.WriteToFile(fOF_Obj);
							fwrite(&VOffset, 4, 1, fVertSeeks);
							VOffset += V_wCNUV.Length;
							VertTotal++;
							} // if UV coords
						else
							{
							V_wCN.VertexCoordX = Object3D->Vertices[Object3D->Polygons[PolyCt].VertRef[VertCt]].xyz[0];
							V_wCN.VertexCoordY = Object3D->Vertices[Object3D->Polygons[PolyCt].VertRef[VertCt]].xyz[2];
							V_wCN.VertexCoordZ = Object3D->Vertices[Object3D->Polygons[PolyCt].VertRef[VertCt]].xyz[1];
							V_wCN.VertexNormalI = -(float)Object3D->Polygons[PolyCt].Normal[0];
							V_wCN.VertexNormalJ = (float)Object3D->Polygons[PolyCt].Normal[2];
							V_wCN.VertexNormalK = -(float)Object3D->Polygons[PolyCt].Normal[1];
							V_wCN.WriteToFile(fOF_Obj);
							fwrite(&VOffset, 4, 1, fVertSeeks);
							VOffset += V_wCN.Length;
							VertTotal++;
							} // else HasUV
						} // for
					} // phong shaded

				} // for PolyCt
			} // if illuminated
		else
			{
			// just write out verts in order
			for (VertCt = 0; VertCt < Object3D->NumVertices; VertCt++)
				{
				x = Object3D->Vertices[VertCt].xyz[0];
				y = Object3D->Vertices[VertCt].xyz[1];
				z = Object3D->Vertices[VertCt].xyz[2];
				if (UVMap && UVMap->CoordsValid[VertCt])
					{
					V_wCUV.VertexCoordX = x;
					V_wCUV.VertexCoordY = z;
					V_wCUV.VertexCoordZ = y;
					V_wCUV.TextureCoordU = UVMap->CoordsArray[0][VertCt];
					V_wCUV.TextureCoordV = UVMap->CoordsArray[1][VertCt];
					V_wCUV.WriteToFile(fOF_Obj);
					fwrite(&VOffset, 4, 1, fVertSeeks);
					VOffset += V_wCUV.Length;
					} // if UV coords
				else
					{
					V_wC.VertexCoordX = x;
					V_wC.VertexCoordY = z;
					V_wC.VertexCoordZ = y;
					V_wC.WriteToFile(fOF_Obj);
					fwrite(&VOffset, 4, 1, fVertSeeks);
					VOffset += V_wC.Length;
					} // else HasUV
				} // for VertCt
			} // self illuminating

		// now touch up Total Length in Vertex Palette Record
		fseek(fOF_Obj, VPTotalLength, SEEK_SET);
		PutB32S(VOffset, fOF_Obj);
		fseek(fOF_Obj, 0, SEEK_END);	// reposition back to end of file

		Push.WriteToFile(fOF_Obj);
		Group.WriteToFile(fOF_Obj);
		Push.WriteToFile(fOF_Obj);
		Group.WriteToFile(fOF_Obj);
		Push.WriteToFile(fOF_Obj);
		sprintf(LOD.ASCII_ID, "l%d", lodnum++);
		LOD.CenterCoordX = (Object3D->ObjectBounds[0] + Object3D->ObjectBounds[1]) * 0.5;
		LOD.CenterCoordY = (Object3D->ObjectBounds[4] + Object3D->ObjectBounds[5]) * 0.5;
		LOD.CenterCoordZ = (Object3D->ObjectBounds[2] + Object3D->ObjectBounds[3]) * 0.5;
		LOD.SwitchInDist = Master->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OBJECTDISTVANISH].CurValue;
		LOD.WriteToFile(fOF_Obj);
		Push.WriteToFile(fOF_Obj);
		Obj.WriteToFile(fOF_Obj);

		// calculate the rotation matrix
		// xref http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q36
		// equations fairly similar, but not exactly same, probably due to different positive axis direction or something
		// A       = cos(angle_x);
		// B       = sin(angle_x);
		// C       = cos(angle_y);
		// D       = sin(angle_y);
		// E       = cos(angle_z);
		// F       = sin(angle_z);
		// AD      =   A * D;
		// BD      =   B * D;
		// mat[0]  =   C * E;
		// mat[1]  =   C * F;
		// mat[2]  =  -D;
		// mat[4]  = -(-BD * E + A * F);
		// mat[5]  =  BD * F + A * E;
		// mat[6]  =   B * C;
		// mat[8]  =  AD * E + B * F;
		// mat[9]  =  AD * F - B * E;
		// mat[10] =   A * C;
		// mat[3]  =  mat[7] = mat[11] = mat[12] = mat[13] = mat[14] = 0;
		// mat[15] =  1;
		RotX = Object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX].CurValue * PiOver180;
		RotY = Object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY].CurValue * PiOver180;
		RotZ = Object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ].CurValue * PiOver180;
		sincos(-RotX, &mB, &mA);
		sincos(-RotZ, &mD, &mC);
		sincos(-RotY, &mF, &mE);
		mAD = mA * mD;
		mBD = mB * mD;
		Matrix.Clear();
		Matrix.Matrix[0] = (float)(mC * mE);
		Matrix.Matrix[1] = (float)(mC * mF);
		Matrix.Matrix[2] = (float)-mD;
		Matrix.Matrix[4] = (float)-(-mBD * mE + mA * mF);
		Matrix.Matrix[5] = (float)(mBD * mF + mA * mE);
		Matrix.Matrix[6] = (float)(mB * mC);
		Matrix.Matrix[8] = (float)(mAD * mE + mB * mF);
		Matrix.Matrix[9] = (float)((mAD * mF) - (mB * mE));
		Matrix.Matrix[10] = (float)(mA * mC);
		Matrix.Matrix[12] = (float)(bias_x + ObjectList->ExportXYZ[0]);
		Matrix.Matrix[13] = (float)(bias_y + ObjectList->ExportXYZ[1]);
		Matrix.Matrix[14] = (float)(Master->ExportRefData.RefElev + ObjectList->ExportXYZ[2]);
		Matrix.Matrix[15] = 1.0f;
		// scale the object
		Matrix.Matrix[0] *= (float)ObjectList->Scale[0];
		Matrix.Matrix[5] *= (float)ObjectList->Scale[2];
		Matrix.Matrix[10] *= (float)ObjectList->Scale[1];
		if (Master->RBounds.IsGeographic)
			{
			Matrix.Matrix[12] = (float)(Matrix.Matrix[12] * Master->ExportRefData.ExportLonScale);
			Matrix.Matrix[13] = (float)(Matrix.Matrix[13] * Master->ExportRefData.ExportLatScale);
			} // if

		Push.WriteToFile(fOF_Obj);
		//Group.WriteToFile(fOF_Obj);
		//Push.WriteToFile(fOF_Obj);
		memset(Face.ASCII_ID, 0, 8);

		//fflush(fVertSeeks);

		if (Illuminate3D)
			{
			rewind(fVertSeeks);
			for (PolyCt = 0; PolyCt < Object3D->NumPolys; PolyCt++)
				{
				long Mat, ShadeType;
				OF_VertexList VList;
				
				sprintf(Face.ASCII_ID, "f%d", PolyCt);
				Mat = Object3D->Polygons[PolyCt].Material;
				if (HasUV)
					{
					Face.TexWhite = 0;
					Face.ColorNameIndex = -1;
					Face.AltColorNameIndex = -1;
					Face.Flags = 0x60000000;
					Face.MatIndex = (short)Mat;
					Face.TexPatIndex = (short)Mat;
					Face.PrimaryColorIndex = 0xffffffff;
					} // if HasUV
				else
					{
					Face.Flags = 0x10000000;
					Face.LightMode = 2;
					Face.MatIndex = (short)Mat;
					Face.TexPatIndex = -1;
					Face.PackedColorPrimary = 0xffffffff;
					} // else HasUV
				Face.WriteToFile(fOF_Obj);
				Push.WriteToFile(fOF_Obj);

				ShadeType = Object3D->NameTable[Mat].Mat->Shading;

				if (ShadeType == WCS_EFFECT_MATERIAL_SHADING_FLAT)
					{
					VList.Length = Object3D->Polygons[PolyCt].NumVerts * 4 + 4;
					VList.WriteToFile(fOF_Obj);
					for (long i = (Object3D->Polygons[PolyCt].NumVerts - 1); i >= 0; i--)	// fix for Deep Exploration
						{
						long VOffset;

						fread(&VOffset, 4, 1, fVertSeeks);
						PutB32S(VOffset, fOF_Obj);
						} // for
					} // flat shaded
				else if (ShadeType == WCS_EFFECT_MATERIAL_SHADING_PHONG)
					{
					VList.Length = Object3D->Polygons[PolyCt].NumVerts * 4 + 4;
					VList.WriteToFile(fOF_Obj);
					for (long i = (Object3D->Polygons[PolyCt].NumVerts - 1); i >= 0; i--)	// fix for Deep Exploration
						{
						long VOffset;

						fread(&VOffset, 4, 1, fVertSeeks);
						PutB32S(VOffset, fOF_Obj);
						} // for
					} // phong shaded
				Pop.WriteToFile(fOF_Obj);
				} // for PolyCt
			} // if illuminated
		else
			{
			for (PolyCt = 0; PolyCt < Object3D->NumPolys; PolyCt++)
				{
				OF_VertexList VList;

				sprintf(Face.ASCII_ID, "f%d", PolyCt);
				if (HasUV)
					{
					Face.Flags = 0x00000000;
					Face.MatIndex = -1;
					Face.TexPatIndex = 0;
					} // if HasUV
				else
					{
					Face.Flags = 0x10000000;
					Face.LightMode = 2;
					Face.MatIndex = (short)Object3D->Polygons[PolyCt].Material;
					Face.TexPatIndex = -1;
					Face.PackedColorPrimary = 0xffffffff;
					} // else HasUV
				Face.WriteToFile(fOF_Obj);
				Push.WriteToFile(fOF_Obj);
				VList.Length = Object3D->Polygons[PolyCt].NumVerts * 4 + 4;
				VList.WriteToFile(fOF_Obj);
				//for (long i = 0; i < Object3D->Polygons[PolyCt].NumVerts; i++)
				for (long i = (Object3D->Polygons[PolyCt].NumVerts - 1); i >= 0; i--)	// fix for Deep Exploration
					{
					long VOffset;

					fseek(fVertSeeks, Object3D->Polygons[PolyCt].VertRef[i] * 4, SEEK_SET);
					fread(&VOffset, 4, 1, fVertSeeks);
					PutB32S(VOffset, fOF_Obj);
					} // for
				Pop.WriteToFile(fOF_Obj);
				} // for PolyCt
			} // else self illuminated

		Pop.WriteToFile(fOF_Obj);
		Pop.WriteToFile(fOF_Obj);
		Pop.WriteToFile(fOF_Obj);	// back to Object level
		Pop.WriteToFile(fOF_Obj);	// back to Group level
		Pop.WriteToFile(fOF_Obj);	// back to Root level

		fclose(fOF_Obj);

		// make the connection from the parent file
		Group.WriteToFile(fOF_Parent);
		Matrix.WriteToFile(fOF_Parent);
		Push.WriteToFile(fOF_Parent);
		memset(XRef.ASCII_Path, 0, 200);
		strcpy(XRef.ASCII_Path, ObjName);
		//XRef.Flags = 0xE0000000;
		XRef.WriteToFile(fOF_Parent);
		Pop.WriteToFile(fOF_Parent);

		n++;
		} // if fOF_Obj
	else
		Success = 0;

SkipObject:
	ObjectList = ObjectList->Next;
	} // while ObjectList

if (fVertSeeks)
	{
	fclose(fVertSeeks);
	PROJ_remove("VertSeeks.tmp");
	} // if

return Success;

} // ExportFormatOpenFlight::Process3DObjects

/*===========================================================================*/

int ExportFormatOpenFlight::ProcessSky(NameList **FileNamesCreated, const char *OutputFilePath)
{
double NWX, NWY, NEX, NEY, SEX, SEY, SWX, SWY, tmp, ZLO, ZHI;
FILE *fATTR, *fOF_Sky;
int Success = 0;
long FileType, offset = 8;
const char *SkyImage;
OF_ColorPalette CP;
OF_Face Face;
OF_Header Hdr;
OF_Object Obj;
OF_TexturePalette TP;
OF_Vertex_withColorUV V_wCUV;
OF_VertexList VL;
OF_VertexPalette VP;

FileType = WCS_EXPORTCONTROL_FILETYPE_SKYNORTH;
if (SkyImage = (*FileNamesCreated)->FindNameOfType(FileType))
	{
	if (fOF_Sky = PROJ_fopen("SkyCube.flt", "wb"))
		{
		float TexMax, TexMin;
		OF_TextureAttributeFile TAF;

		TAF.TexelsU = TAF.TexelsV = Master->SkyRes;

		strcpy(XRef.ASCII_Path, "SkyCube.flt");
		XRef.WriteToFile(fOF_Master);

		strcpy(Hdr.ASCII_ID, "Sky");
		Hdr.WriteToFile(fOF_Sky);
		CP.WriteToFile(fOF_Sky);

		strcpy(TP.Filename, SkyImage);
		TP.PatternIndex = 0;
		TP.WriteToFile(fOF_Sky);
		strcat(TP.Filename, ".attr");
		if (fATTR = PROJ_fopen(TP.Filename, "wb"))
			{
			TAF.WriteToFile(fATTR);
			fclose(fATTR);
			} // if
		FileType = WCS_EXPORTCONTROL_FILETYPE_SKYSOUTH;
		SkyImage = (*FileNamesCreated)->FindNameOfType(FileType);
		strcpy(TP.Filename, SkyImage);
		TP.PatternIndex = 1;
		TP.WriteToFile(fOF_Sky);
		strcat(TP.Filename, ".attr");
		if (fATTR = PROJ_fopen(TP.Filename, "wb"))
			{
			TAF.WriteToFile(fATTR);
			fclose(fATTR);
			} // if
		FileType = WCS_EXPORTCONTROL_FILETYPE_SKYWEST;
		SkyImage = (*FileNamesCreated)->FindNameOfType(FileType);
		strcpy(TP.Filename, SkyImage);
		TP.PatternIndex = 2;
		TP.WriteToFile(fOF_Sky);
		strcat(TP.Filename, ".attr");
		if (fATTR = PROJ_fopen(TP.Filename, "wb"))
			{
			TAF.WriteToFile(fATTR);
			fclose(fATTR);
			} // if
		FileType = WCS_EXPORTCONTROL_FILETYPE_SKYEAST;
		SkyImage = (*FileNamesCreated)->FindNameOfType(FileType);
		strcpy(TP.Filename, SkyImage);
		TP.PatternIndex = 3;
		TP.WriteToFile(fOF_Sky);
		strcat(TP.Filename, ".attr");
		if (fATTR = PROJ_fopen(TP.Filename, "wb"))
			{
			TAF.WriteToFile(fATTR);
			fclose(fATTR);
			} // if
		FileType = WCS_EXPORTCONTROL_FILETYPE_SKYTOP;
		SkyImage = (*FileNamesCreated)->FindNameOfType(FileType);
		strcpy(TP.Filename, SkyImage);
		TP.PatternIndex = 4;
		TP.WriteToFile(fOF_Sky);
		strcat(TP.Filename, ".attr");
		if (fATTR = PROJ_fopen(TP.Filename, "wb"))
			{
			TAF.WriteToFile(fATTR);
			fclose(fATTR);
			} // if
		FileType = WCS_EXPORTCONTROL_FILETYPE_SKYBOTTOM;
		SkyImage = (*FileNamesCreated)->FindNameOfType(FileType);
		strcpy(TP.Filename, SkyImage);
		TP.PatternIndex = 5;
		TP.WriteToFile(fOF_Sky);
		strcat(TP.Filename, ".attr");
		if (fATTR = PROJ_fopen(TP.Filename, "wb"))
			{
			TAF.WriteToFile(fATTR);
			fclose(fATTR);
			} // if

		// compute our "cube" corners
		/***
		NWX = Master->RBounds.ULcorner.x * 2.0 + bias_x;
		NWY = Master->RBounds.ULcorner.y * 2.0 + bias_y;
		NEX = Master->RBounds.LRcorner.x * 2.0 + bias_x;
		NEY = NWY;
		SEX = NEX;
		SEY = Master->RBounds.LRcorner.y * 2.0 + bias_y;
		SWX = NWX;
		SWY = SEY;
		tmp = ((NEX - NWX) + (NWY - SWY)) * 0.25; // 1/2 avg of width & height
		***/
		tmp = bias_x * 2.0;
		NWX = SWX = bias_x - tmp;
		NEX = SEX = bias_x + tmp;
		tmp = bias_y * 2.0;
		SWY = SEY = bias_y - tmp;
		NWY = NEY = bias_y + tmp;
		ZLO = Master->ExportRefData.RefElev - tmp;
		ZHI = Master->ExportRefData.RefElev + tmp;

		VP.TotalLength = 8 + 24 * 48;
		VP.WriteToFile(fOF_Sky);

		TexMax = (float)((Master->SkyRes - 1.0) / Master->SkyRes);
		TexMin = (float)(1.0 / Master->SkyRes);
		// N face from inside box
		//
		// v0-v3
		// |   |
		// v1-v2
		//
		// v0
		V_wCUV.VertexCoordX = NWX;
		V_wCUV.VertexCoordY = NWY;
		V_wCUV.VertexCoordZ = ZHI;
		V_wCUV.TextureCoordU = TexMin;
		V_wCUV.TextureCoordV = TexMax;
		V_wCUV.WriteToFile(fOF_Sky);
		// v1
		//V_wCUV.VertexCoordX = NWX;
		//V_wCUV.VertexCoordY = NWY;
		V_wCUV.VertexCoordZ = ZLO;
		//V_wCUV.TextureCoordU = 0.0f;
		V_wCUV.TextureCoordV = TexMin;;
		V_wCUV.WriteToFile(fOF_Sky);
		// v2
		V_wCUV.VertexCoordX = NEX;
		V_wCUV.VertexCoordY = NEY;
		//V_wCUV.VertexCoordZ = ZLO;
		V_wCUV.TextureCoordU = TexMax;
		//V_wCUV.TextureCoordV = 0.0f;
		V_wCUV.WriteToFile(fOF_Sky);
		// v3
		//V_wCUV.VertexCoordX = NEX;
		//V_wCUV.VertexCoordY = NEY;
		V_wCUV.VertexCoordZ = ZHI;
		//V_wCUV.TextureCoordU = 1.0f;
		V_wCUV.TextureCoordV = TexMax;
		V_wCUV.WriteToFile(fOF_Sky);

		// S face from inside box
		//
		// v0-v3
		// |   |
		// v1-v2
		//
		// v0
		V_wCUV.VertexCoordX = SEX;
		V_wCUV.VertexCoordY = SEY;
		V_wCUV.VertexCoordZ = ZHI;
		V_wCUV.TextureCoordU = TexMin;
		V_wCUV.TextureCoordV = TexMax;
		V_wCUV.WriteToFile(fOF_Sky);
		// v1
		//V_wCUV.VertexCoordX = SEX;
		//V_wCUV.VertexCoordY = SEY;
		V_wCUV.VertexCoordZ = ZLO;
		//V_wCUV.TextureCoordU = 0.0f;
		V_wCUV.TextureCoordV = TexMin;
		V_wCUV.WriteToFile(fOF_Sky);
		// v2
		V_wCUV.VertexCoordX = SWX;
		V_wCUV.VertexCoordY = SWY;
		//V_wCUV.VertexCoordZ = ZLO;
		V_wCUV.TextureCoordU = TexMax;
		//V_wCUV.TextureCoordV = 0.0f;
		V_wCUV.WriteToFile(fOF_Sky);
		// v3
		//V_wCUV.VertexCoordX = SWX;
		//V_wCUV.VertexCoordY = SWY;
		V_wCUV.VertexCoordZ = ZHI;
		//V_wCUV.TextureCoordU = 1.0f;
		V_wCUV.TextureCoordV = TexMax;
		V_wCUV.WriteToFile(fOF_Sky);

		// W face from inside box
		//
		// v0-v3
		// |   |
		// v1-v2
		//
		// v0
		V_wCUV.VertexCoordX = SWX;
		V_wCUV.VertexCoordY = SWY;
		V_wCUV.VertexCoordZ = ZHI;
		V_wCUV.TextureCoordU = TexMin;
		V_wCUV.TextureCoordV = TexMax;
		V_wCUV.WriteToFile(fOF_Sky);
		// v1
		//V_wCUV.VertexCoordX = SWX;
		//V_wCUV.VertexCoordY = SWY;
		V_wCUV.VertexCoordZ = ZLO;
		//V_wCUV.TextureCoordU = 0.0f;
		V_wCUV.TextureCoordV = TexMin;
		V_wCUV.WriteToFile(fOF_Sky);
		// v2
		V_wCUV.VertexCoordX = NWX;
		V_wCUV.VertexCoordY = NWY;
		//V_wCUV.VertexCoordZ = ZLO;
		V_wCUV.TextureCoordU = TexMax;
		//V_wCUV.TextureCoordV = 0.0f;
		V_wCUV.WriteToFile(fOF_Sky);
		// v3
		//V_wCUV.VertexCoordX = NWX;
		//V_wCUV.VertexCoordY = NWY;
		V_wCUV.VertexCoordZ = ZHI;
		//V_wCUV.TextureCoordU = 1.0f;
		V_wCUV.TextureCoordV = TexMax;
		V_wCUV.WriteToFile(fOF_Sky);

		// E face from inside box
		//
		// v0-v3
		// |   |
		// v1-v2
		//
		// v0
		V_wCUV.VertexCoordX = NEX;
		V_wCUV.VertexCoordY = NEY;
		V_wCUV.VertexCoordZ = ZHI;
		V_wCUV.TextureCoordU = TexMin;
		V_wCUV.TextureCoordV = TexMax;
		V_wCUV.WriteToFile(fOF_Sky);
		// v1
		//V_wCUV.VertexCoordX = NEX;
		//V_wCUV.VertexCoordY = NEY;
		V_wCUV.VertexCoordZ = ZLO;
		//V_wCUV.TextureCoordU = 0.0f;
		V_wCUV.TextureCoordV = TexMin;
		V_wCUV.WriteToFile(fOF_Sky);
		// v2
		V_wCUV.VertexCoordX = SEX;
		V_wCUV.VertexCoordY = SEY;
		//V_wCUV.VertexCoordZ = ZLO;
		V_wCUV.TextureCoordU = TexMax;
		//V_wCUV.TextureCoordV = 0.0f;
		V_wCUV.WriteToFile(fOF_Sky);
		// v3
		//V_wCUV.VertexCoordX = SEX;
		//V_wCUV.VertexCoordY = SEY;
		V_wCUV.VertexCoordZ = ZHI;
		//V_wCUV.TextureCoordU = 1.0f;
		V_wCUV.TextureCoordV = TexMax;
		V_wCUV.WriteToFile(fOF_Sky);

		// Top face from inside box
		//
		// v0-v3
		// |   |
		// v1-v2
		//
		// v0
		V_wCUV.VertexCoordX = SWX;
		V_wCUV.VertexCoordY = SWY;
		V_wCUV.VertexCoordZ = ZHI;
		V_wCUV.TextureCoordU = TexMin;
		V_wCUV.TextureCoordV = TexMax;
		V_wCUV.WriteToFile(fOF_Sky);
		// v1
		V_wCUV.VertexCoordX = NWX;
		V_wCUV.VertexCoordY = NWY;
		//V_wCUV.VertexCoordZ = ZHI;
		//V_wCUV.TextureCoordU = 0.0f;
		V_wCUV.TextureCoordV = TexMin;
		V_wCUV.WriteToFile(fOF_Sky);
		// v2
		V_wCUV.VertexCoordX = NEX;
		V_wCUV.VertexCoordY = NEY;
		//V_wCUV.VertexCoordZ = ZHI;
		V_wCUV.TextureCoordU = TexMax;
		//V_wCUV.TextureCoordV = 0.0f;
		V_wCUV.WriteToFile(fOF_Sky);
		// v3
		V_wCUV.VertexCoordX = SEX;
		V_wCUV.VertexCoordY = SEY;
		//V_wCUV.VertexCoordZ = ZHI;
		//V_wCUV.TextureCoordU = 1.0f;
		V_wCUV.TextureCoordV = TexMax;
		V_wCUV.WriteToFile(fOF_Sky);

		// Bottom face from inside box
		//
		// v0-v3
		// |   |
		// v1-v2
		//
		// v0
		V_wCUV.VertexCoordX = NWX;
		V_wCUV.VertexCoordY = NWY;
		V_wCUV.VertexCoordZ = ZLO;
		V_wCUV.TextureCoordU = TexMin;
		V_wCUV.TextureCoordV = TexMax;
		V_wCUV.WriteToFile(fOF_Sky);
		// v1
		V_wCUV.VertexCoordX = SWX;
		V_wCUV.VertexCoordY = SWY;
		//V_wCUV.VertexCoordZ = ZLO;
		//V_wCUV.TextureCoordU = 0.0f;
		V_wCUV.TextureCoordV = TexMin;
		V_wCUV.WriteToFile(fOF_Sky);
		// v2
		V_wCUV.VertexCoordX = SEX;
		V_wCUV.VertexCoordY = SEY;
		//V_wCUV.VertexCoordZ = ZLO;
		V_wCUV.TextureCoordU = TexMax;
		//V_wCUV.TextureCoordV = 0.0f;
		V_wCUV.WriteToFile(fOF_Sky);
		// v3
		V_wCUV.VertexCoordX = NEX;
		V_wCUV.VertexCoordY = NEY;
		//V_wCUV.VertexCoordZ = ZLO;
		//V_wCUV.TextureCoordU = 1.0f;
		V_wCUV.TextureCoordV = TexMax;
		V_wCUV.WriteToFile(fOF_Sky);

		Push.WriteToFile(fOF_Sky);
		strcpy(Group.ASCII_ID, "gSky");
		Group.WriteToFile(fOF_Sky);
		Push.WriteToFile(fOF_Sky);
		strcpy(Obj.ASCII_ID, "SkyBox");
		Obj.WriteToFile(fOF_Sky);
		Push.WriteToFile(fOF_Sky);
		VL.Length = 4 + 4 * 4;
		Face.Flags = 0x60000000;

		strcpy(Face.ASCII_ID, "SkyN");
		Face.TexPatIndex = 0;
		Face.WriteToFile(fOF_Sky);
		Push.WriteToFile(fOF_Sky);
		VL.WriteToFile(fOF_Sky);
		PutB32S(offset, fOF_Sky);
		offset += 48;
		PutB32S(offset, fOF_Sky);
		offset += 48;
		PutB32S(offset, fOF_Sky);
		offset += 48;
		PutB32S(offset, fOF_Sky);
		offset += 48;
		Pop.WriteToFile(fOF_Sky);

		strcpy(Face.ASCII_ID, "SkyS");
		Face.TexPatIndex = 1;
		Face.WriteToFile(fOF_Sky);
		Push.WriteToFile(fOF_Sky);
		VL.WriteToFile(fOF_Sky);
		PutB32S(offset, fOF_Sky);
		offset += 48;
		PutB32S(offset, fOF_Sky);
		offset += 48;
		PutB32S(offset, fOF_Sky);
		offset += 48;
		PutB32S(offset, fOF_Sky);
		offset += 48;
		Pop.WriteToFile(fOF_Sky);

		strcpy(Face.ASCII_ID, "SkyW");
		Face.TexPatIndex = 2;
		Face.WriteToFile(fOF_Sky);
		Push.WriteToFile(fOF_Sky);
		VL.WriteToFile(fOF_Sky);
		PutB32S(offset, fOF_Sky);
		offset += 48;
		PutB32S(offset, fOF_Sky);
		offset += 48;
		PutB32S(offset, fOF_Sky);
		offset += 48;
		PutB32S(offset, fOF_Sky);
		offset += 48;
		Pop.WriteToFile(fOF_Sky);

		strcpy(Face.ASCII_ID, "SkyE");
		Face.TexPatIndex = 3;
		Face.WriteToFile(fOF_Sky);
		Push.WriteToFile(fOF_Sky);
		VL.WriteToFile(fOF_Sky);
		PutB32S(offset, fOF_Sky);
		offset += 48;
		PutB32S(offset, fOF_Sky);
		offset += 48;
		PutB32S(offset, fOF_Sky);
		offset += 48;
		PutB32S(offset, fOF_Sky);
		offset += 48;
		Pop.WriteToFile(fOF_Sky);

		strcpy(Face.ASCII_ID, "SkyT");
		Face.TexPatIndex = 4;
		Face.WriteToFile(fOF_Sky);
		Push.WriteToFile(fOF_Sky);
		VL.WriteToFile(fOF_Sky);
		PutB32S(offset, fOF_Sky);
		offset += 48;
		PutB32S(offset, fOF_Sky);
		offset += 48;
		PutB32S(offset, fOF_Sky);
		offset += 48;
		PutB32S(offset, fOF_Sky);
		offset += 48;
		Pop.WriteToFile(fOF_Sky);

		strcpy(Face.ASCII_ID, "SkyB");
		Face.TexPatIndex = 5;
		Face.WriteToFile(fOF_Sky);
		Push.WriteToFile(fOF_Sky);
		VL.WriteToFile(fOF_Sky);
		PutB32S(offset, fOF_Sky);
		offset += 48;
		PutB32S(offset, fOF_Sky);
		offset += 48;
		PutB32S(offset, fOF_Sky);
		offset += 48;
		PutB32S(offset, fOF_Sky);
		offset += 48;
		Pop.WriteToFile(fOF_Sky);

		Pop.WriteToFile(fOF_Sky);
		Pop.WriteToFile(fOF_Sky);
		if (Pop.WriteToFile(fOF_Sky) == 0)
			Success = 1;
		fclose(fOF_Sky);
		} // if fOF_Sky
	} // if SkyImage

return Success;

} // ExportFormatOpenFlight::ProcessSky

/*===========================================================================*/

int ExportFormatOpenFlight::ProcessVectors(const char *OutputFilePath)
{
FILE *fOF_V;
Joe *CurJoe;
VectorExportItem *VEI;
VectorPoint *vert;
float vElev;
int Success = 1;
long n, voffset = 8, total_verts = 0, vertnum = 0;
OF_ColorPalette CP;
OF_Face Face;
OF_Header Hdr;
OF_Vertex_withColor V_wC;
OF_VertexList VL;
OF_VertexPalette VP;
PathAndFile SceneOutput;
char TempFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

sprintf(XRef.ASCII_Path, "vectors.flt");
XRef.WriteToFile(fOF_Master);

Face.DrawType = 3;	// Open Wireframe - *** NOTE *** all OF docs apparently have Open/Closed wireframe values reversed!!!

SceneOutput.SetPath((char *)Master->OutPath.GetPath());
SceneOutput.SetName("vectors.flt");
SceneOutput.GetFramePathAndName(TempFullPath, NULL, 0, WCS_PATHANDFILE_PATH_PLUS_NAME_LEN, 0);
if (fOF_V = PROJ_fopen(TempFullPath, "wb"))
	{
	strcpy(Hdr.ASCII_ID, "db_vect");
	Hdr.WriteToFile(fOF_V);
	CP.WriteToFile(fOF_V);

	// figure out how large our vertex palette is
	for (n = 0; n < Master->NumVecInstances; n++)
		{
		VEI = &Master->VecInstanceList[n];
		vert = VEI->Points;
		while (vert)
			{
			total_verts++;
			vert = vert->Next;
			} // while
		} // for
	VP.TotalLength = 8 + 40 * total_verts;
	VP.WriteToFile(fOF_V);

	// write the vertex palette
	V_wC.Flags = 0x1000;		// packed color
	for (n = 0; n < Master->NumVecInstances; n++)
		{
		double vLat, vLon;
		long ABGR;

		VEI = &Master->VecInstanceList[n];
		CurJoe = VEI->MyJoe;
		ABGR = (CurJoe->AttribInfo.BlueGun << 16) + (CurJoe->AttribInfo.GreenGun << 8) + (CurJoe->AttribInfo.RedGun);
		vert = VEI->Points;
		while (vert)
			{
			vLat = vert->Latitude * Master->ExportRefData.ExportLatScale;
			vLon = vert->Longitude * Master->ExportRefData.ExportLonScale;
			vElev = vert->Elevation;
			V_wC.VertexCoordX = vLon + bias_x;
			V_wC.VertexCoordY = vLat + bias_y;
			V_wC.VertexCoordZ = vElev + Master->ExportRefData.RefElev;
			V_wC.PackedColorABGR = ABGR;
			V_wC.WriteToFile (fOF_V);
			vert = vert->Next;
			} // while vert
		} // for

	Push.WriteToFile(fOF_V);
	strcpy(Group.ASCII_ID, "gVector");
	Group.WriteToFile(fOF_V);
	Push.WriteToFile(fOF_V);

	// write the objects & indices
	Face.Flags = 0x30000000;
	for (n = 0; n < Master->NumVecInstances; n++)
		{
		long ABGR;

		VEI = &Master->VecInstanceList[n];
		CurJoe = VEI->MyJoe;
		ABGR = (CurJoe->AttribInfo.BlueGun << 16) + (CurJoe->AttribInfo.GreenGun << 8) + (CurJoe->AttribInfo.RedGun);
		Face.PackedColorPrimary = ABGR;
		Face.WriteToFile(fOF_V);
		Push.WriteToFile(fOF_V);
		VL.Length = (unsigned short)(4 + VEI->NumPoints * 4);
		VL.WriteToFile(fOF_V);

		// save the vert list
		vert = VEI->Points;
		while (vert)
			{
			PutB32S(voffset, fOF_V);
			voffset += 40;
			vert = vert->Next;
			} // while vert

		Pop.WriteToFile(fOF_V);

		} // for

	Pop.WriteToFile(fOF_V);
	Pop.WriteToFile(fOF_V);
	fclose(fOF_V);

	} // if

return Success;

} // ExportFormatOpenFlight::ProcessVectors

/*===========================================================================*/

// Ok, this sucks, this TRULY sucks.  Foliage has held up development for something like 6 months, so now there are SEVERAL
// different ways to generate foliage in the hope that a given OF app will find one of them that it likes.
// 1) Flipboard with/without instancing, with/without individual LOD's
// 2) Crossboard
int ExportFormatOpenFlight::ProcessFoliageList(NameList **FileNamesCreated, const char *OutputFilePath, FILE *fOF_Parent,
											   long XTile, long YTile, double ref_x, double ref_y)
{
double BBot, BLeft, BRight, BTop;
double ExportX, ExportY;
FILE *ffile, *fOF;
static long TreeNum = 0;	// for labeling tree LOD's
long CreateLOD = 1, DatPt, FileType, Instancing = 1, VertStart, WriteFol = 0, XlateNum;
short *XlateTable = NULL;
int Success = 1;
short InstDefNum = 0;	// it looks like MGI's DLL's crash if our instance definitions don't start at 0!!!
const char *IndexFileName, *DataFileName;
PathAndFile PAF;
RealtimeFoliageIndex Index;
RealtimeFoliageCellData RFCD;
RealtimeFoliageData FolData;
FoliagePreviewData PointData;
OF_ColorPalette CP;
OF_Face Face;
OF_Header FolHdr;
OF_LOD TreeLOD;
OF_TexturePalette FolTex;
OF_Vertex_withColorUV VwCUV;
//OF_Vertex_withColorNomralUV VwCNUV;
OF_VertexList VList;
OF_VertexPalette VP;
//char *ConfigOpt;
char TempFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];
char FileName[512], TestFileVersion;

Instancing = ((SXExtensionOF *)Master->FormatExtension)->CreateFoliage;
CreateLOD = ((SXExtensionOF *)Master->FormatExtension)->IndividualFolLOD;

/* no longer necssary, replaced by SX extension variables
if (ConfigOpt = GlobalApp->MainProj->Prefs.QueryConfigOpt("SX_OF_NoFoliageInstancing"))
	{
	if (stricmp(ConfigOpt, "True") == 0)
		{
		Instancing = 0;
		} // if
	} // if

if (ConfigOpt = GlobalApp->MainProj->Prefs.QueryConfigOpt("SX_OF_NoFoliageLOD"))
	{
	if (stricmp(ConfigOpt, "True") == 0)
		{
		CreateLOD = 0;
		} // if
	} // if
*/

TreeLOD.Flags = 0x20000000;	// Freeze Center
TreeLOD.SwitchIn = Master->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_FOLDISTVANISH].CurValue;

PAF.SetPath((char *)Master->OutPath.GetPath());

// bounds of this tile
if (Master->RBounds.IsGeographic)
	{
	BLeft = ref_x * Master->ExportRefData.ExportLonScale;
	BRight = (ref_x + Master->OneDEMResX * Master->RBounds.CellSizeX) * Master->ExportRefData.ExportLonScale;
	BBot = ref_y * Master->ExportRefData.ExportLatScale;
	BTop = (ref_y + Master->OneDEMResY * Master->RBounds.CellSizeY) * Master->ExportRefData.ExportLatScale;
	} //
else
	{
	BLeft = ref_x;
	BRight = ref_x + Master->OneDEMResX * Master->RBounds.CellSizeX;
	BBot = ref_y;
	BTop = ref_y + Master->OneDEMResY * Master->RBounds.CellSizeY;
	} // else

Face.DrawType = 1;			// no backface culling
Face.Flags = 0x20000000;	// no alt color
Face.Template = 2;			// axial rotate with alpha blending
Face.RelativePri = 2;
Face.LightMode = 0;
Face.SurfaceMatCode = 12;	// DFAD SMC for trees
Face.FeatureID = 950;		// DFAD FID for general vegetation

//VwCNUV.VertexNormalI = 0.0f;
//VwCNUV.VertexNormalJ = 1.0f;
//VwCNUV.VertexNormalK = 0.0f;

// make a translation table to convert our IOL index numbers to OF texture numbers
XlateNum = Images->GetImageCount() + 1;	// IOL starts at 1
XlateTable = (short *)AppMem_Alloc(XlateNum * 2, APPMEM_CLEAR);
if (XlateTable == NULL)
	return 0;

// files may not exist. If they don't then user must not have chosen foliage export features
FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILEIDX;
if (IndexFileName = (*FileNamesCreated)->FindNameOfType(FileType))
	{
	FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILE;
	if (DataFileName = (*FileNamesCreated)->FindNameOfType(FileType))
		{
		// you've got the two file names that the Renderer wrote out.
		// combine them with the output file path to make a file that can be opened with PROJ_fopen()

		// find and open the index file
		strmfp(FileName, OutputFilePath, IndexFileName);
		if (ffile = PROJ_fopen(FileName, "rb"))
			{
			// read file descriptor, no need to keep it around unless you want to
			fgets(FileName, 256, ffile);
			// version
			fread((char *)&Index.FileVersion, sizeof (char), 1, ffile);
			// number of files
			fread((char *)&Index.NumCells, sizeof (long), 1, ffile);
			// reference XYZ
			fread((char *)&Index.RefXYZ[0], sizeof (double), 1, ffile);
			fread((char *)&Index.RefXYZ[1], sizeof (double), 1, ffile);
			fread((char *)&Index.RefXYZ[2], sizeof (double), 1, ffile);

			if (Index.NumCells > 0)
				{
				// only one cell data entry is provided
				if (Index.CellDat = &RFCD)
					{
					// file name
					fgets(Index.CellDat->FileName, 64, ffile);
					// center XYZ
					fread((char *)&Index.CellDat->CellXYZ[0], sizeof (double), 1, ffile);
					fread((char *)&Index.CellDat->CellXYZ[1], sizeof (double), 1, ffile);
					fread((char *)&Index.CellDat->CellXYZ[2], sizeof (double), 1, ffile);
					// half cube cell dimension
					fread((char *)&Index.CellDat->CellRad, sizeof (double), 1, ffile);
					// number of trees in file
					fread((char *)&Index.CellDat->DatCt, sizeof (long), 1, ffile);
					} // if
				} // if some cells to read
			fclose(ffile);

			if ((Index.NumCells > 0) && (Index.CellDat->DatCt > 0))
				{
				strmfp(FileName, OutputFilePath, DataFileName);
				if (ffile = PROJ_fopen(FileName, "rb"))
					{
					fgets(FileName, 64, ffile);
					// version
					fread((char *)&TestFileVersion, sizeof (char), 1, ffile);
					// Pointless version check -- we know we wrote it
					if (TestFileVersion == Index.FileVersion)
						{
						// see if any foliage exists on this tile
						for (DatPt = 0; DatPt < Index.CellDat->DatCt; DatPt++)
							{
							if (FolData.ReadFoliageRecord(ffile, Index.FileVersion))
								{
								//if (FolData.InterpretFoliageRecord(NULL, Images, &PointData))
								//	{
									if (Master->RBounds.IsGeographic)
										{
										ExportX = (float)((FolData.XYZ[0] + bias_x) * Master->ExportRefData.ExportLonScale);
										ExportY = (float)((FolData.XYZ[1] + bias_y) * Master->ExportRefData.ExportLatScale);
										} // if
									else
										{
										ExportX = (float)(FolData.XYZ[0] * Master->ExportRefData.ExportLonScale + bias_x);
										ExportY = (float)(FolData.XYZ[1] * Master->ExportRefData.ExportLatScale + bias_y);
										} // else

									// see if this object falls on the tile we're exporting
									if ((ExportX >= BLeft) && (ExportX < BRight) && (ExportY >= BBot) && (ExportY < BTop))
										{
										WriteFol = 1;
										break;
										} // if
								//	} // if
								} // if
							} // for

						if (WriteFol)
							{
							Raster *CurRast, *RastLoop;
							long n;
							short tref = 0;
							char FoliageFileName[16];
							struct InstanceInfo *Instances = NULL;
							OF_MaterialPalette MP;
							size_t InstanceSize;

							sprintf(FolHdr.ASCII_ID, "fo%d_%d", XTile, YTile);
							sprintf(FoliageFileName, "foliage%d_%d.flt", XTile, YTile);

							strmfp(FileName, OutputFilePath, FoliageFileName);
							fOF = PROJ_fopen(FileName, "wb");
							if (fOF == NULL)
								return 0;

							if (FolHdr.WriteToFile(fOF))
								return 0;

							CP.WriteToFile(fOF);
							strcpy(MP.MatName, "Void");
							MP.WriteToFile(fOF);	// dummy in case somebody expects to parse one

							for (n = 1, RastLoop = Images->GetFirstRast(); RastLoop; RastLoop = Images->GetNextRast(RastLoop), n++)
								{
								char ImageName[256], NewImageName[256], NewPAFName[512], OldPAFName[512];

								if (CurRast = Images->FindByID(n))
									{
									strcpy(ImageName, CurRast->GetUserName());
									ImageSaverLibrary::StripImageExtension(ImageName);
									strcat(ImageName, "_Fol");
									ReplaceChar(ImageName, '.', '_');
									strcpy(NewImageName, ImageName);
									strcat(ImageName, ImageSaverLibrary::GetDefaultExtension(Master->FoliageImageFormat));
									FileType = WCS_EXPORTCONTROL_FILETYPE_FOLIAGETEX;
									if ((*FileNamesCreated)->FindNameExists(FileType, ImageName))
										{
										FILE *fTAF, *image;
										long TCols = 0, TRows = 0, ZSize;
										OF_TextureAttributeFile TAF;
										char *ext;

										// indeed a resampled image has been created for this entry
										// so it is safe to add this entry into the foliage definitions
										memset(FolTex.Filename, 0, sizeof(FolTex.Filename));
										strcpy(FolTex.Filename, ImageName);
										FolTex.PatternIndex = tref;
										XlateTable[n] = tref++;

										// Everything expects to find an attribute file for each image used
										PAF.SetName(ImageName);
										strcpy(OldPAFName, PAF.GetPathAndName(TempFullPath));
										// we need the actual exported texture size
										ext = ImageSaverLibrary::GetDefaultExtension(Master->FoliageImageFormat);
										if ((stricmp(ext, ".rgb") == 0) || (stricmp(ext, ".rgba") == 0))
											{
											if (image = PROJ_fopen(ImageName, "rb"))
												{
												// grab the width & height from the file
												fseek(image, 6, SEEK_SET);
												TCols = GetB16U(image);
												TRows = GetB16U(image);
												ZSize = GetB16U(image);	// number of channels
												fclose(image);
												if (ZSize == 3)
													{
													TAF.FileFormatType = 4;	// SGI RGB
													if (stricmp(ext, ".rgb") != 0)
														{
														strcat(NewImageName, ".rgb");
														PAF.SetName(NewImageName);
														strcpy(NewPAFName, PAF.GetPathAndName(TempFullPath));
														PROJ_rename(OldPAFName, NewPAFName);
														strcpy(FolTex.Filename, NewImageName);
														strcpy(ImageName, NewImageName);
														} // if
													} // if
												else if (ZSize == 4)
													{
													TAF.FileFormatType = 5;	// SGI RGBA
													if (stricmp(ext, ".rgba") != 0)
														{
														strcat(NewImageName, ".rgba");
														PAF.SetName(NewImageName);
														strcpy(NewPAFName, PAF.GetPathAndName(TempFullPath));
														PROJ_rename(OldPAFName, NewPAFName);
														strcpy(FolTex.Filename, NewImageName);
														strcpy(ImageName, NewImageName);
														} // if
													} // else
												} // if image
											} // if ext == rgb or rgba
										else
											{
											Raster LoadRas;

											if (LoadRasterImage(ImageName, &LoadRas, 0))
												{
												TCols = LoadRas.Cols;
												TRows = LoadRas.Rows;
												} // if
											} // else
										TAF.TexelsU = TCols;
										TAF.TexelsV = TRows;
										TAF.EnviroType = 3;	// Replace
										// Now attempt to write the attribute file
										//PAF.SetName(ImageName);
										//PAF.GetFramePathAndName(TempFullPath, ".attr", 0, WCS_PATHANDFILE_PATH_PLUS_NAME_LEN, 0);
										PAF.SetPath(OutputFilePath);
										PAF.SetName(ImageName);
										PAF.GetFramePathAndName(TempFullPath, ".attr", 0, WCS_PATHANDFILE_PATH_PLUS_NAME_LEN, 0);
										if (fTAF = PROJ_fopen(TempFullPath, "wb"))
											{
											TAF.WriteToFile(fTAF);
											fclose(fTAF);
											} // if fTAF
										FolTex.WriteToFile(fOF);
										} // if
									} // if
								} // for

							if (Master->FoliageStyle == WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_FLIPBOARDS)
								{
								if (Instancing)
									{
									// Write two polygons that thru instancing & transformations will become _ALL_ trees in tile

									// now n has the total number of images in the Image Object Library, so use that to set up an instancing array
									InstanceSize = sizeof(struct InstanceInfo) * n;
									Instances = (struct InstanceInfo *)AppMem_Alloc(InstanceSize, APPMEM_CLEAR);
									// NOTE: Add error handling for the above!

									// polygon0 - UV 1,1 @ UR
									VertStart = ftell(fOF);
									VP.TotalLength = 8 + 8 * 48;	// VPR + 8 vertices * Vertex_wCUV size
									VP.WriteToFile(fOF);
									VwCUV.Flags = 0x2000;
									// write 4 vertices:
									//
									// 1 --- 0   Z
									// |     |   |
									// |     |   |
									// 2 --- 3   *----X
									//
									// 0 vertex
									VwCUV.VertexCoordX = 0.5f;
									VwCUV.VertexCoordY = 0.0f;
									VwCUV.VertexCoordZ = 1.0f;
									VwCUV.TextureCoordU = 1.0f;
									VwCUV.TextureCoordV = 1.0f;
									VwCUV.WriteToFile(fOF);
									// 1 vertex
									VwCUV.VertexCoordX = -0.5f;
									//VwCUV.VertexCoordY = 0.0f;
									//VwCUV.VertexCoordZ = 1.0f;
									VwCUV.TextureCoordU = 0.0f;
									//VwCUV.TextureCoordV = 1.0f;
									VwCUV.WriteToFile(fOF);
									// 2 vertex
									//VwCUV.VertexCoordX = -0.5f;
									//VwCUV.VertexCoordY = 0.0f;
									VwCUV.VertexCoordZ = 0.0f;
									//VwCUV.TextureCoordU = 0.0f;
									VwCUV.TextureCoordV = 0.0f;
									VwCUV.WriteToFile(fOF);
									// 3 vertex
									VwCUV.VertexCoordX = 0.5f;
									//VwCUV.VertexCoordY = 0.0f;
									//VwCUV.VertexCoordZ = 0.0f;
									VwCUV.TextureCoordU = 1.0f;
									//VwCUV.TextureCoordV = 0.0f;
									VwCUV.WriteToFile(fOF);

									// polygon1 - UV 1,1 @ UL (for "flipped" images)
									// write 4 vertices:
									//
									// 1 --- 0   Z
									// |     |   |
									// |     |   |
									// 2 --- 3   *----X
									//
									// 0 vertex
									VwCUV.VertexCoordX = 0.5f;
									VwCUV.VertexCoordY = 0.0f;
									VwCUV.VertexCoordZ = 1.0f;
									VwCUV.TextureCoordU = 0.0f;
									VwCUV.TextureCoordV = 1.0f;
									VwCUV.WriteToFile(fOF);
									// 1 vertex
									VwCUV.VertexCoordX = -0.5f;
									//VwCUV.VertexCoordY = 0.0f;
									//VwCUV.VertexCoordZ = 1.0f;
									VwCUV.TextureCoordU = 1.0f;
									//VwCUV.TextureCoordV = 1.0f;
									VwCUV.WriteToFile(fOF);
									// 2 vertex
									//VwCUV.VertexCoordX = -0.5f;
									//VwCUV.VertexCoordY = 0.0f;
									VwCUV.VertexCoordZ = 0.0f;
									//VwCUV.TextureCoordU = 1.0f;
									VwCUV.TextureCoordV = 0.0f;
									VwCUV.WriteToFile(fOF);
									// 3 vertex
									VwCUV.VertexCoordX = 0.5f;
									//VwCUV.VertexCoordY = 0.0f;
									//VwCUV.VertexCoordZ = 0.0f;
									VwCUV.TextureCoordU = 0.0f;
									//VwCUV.TextureCoordV = 0.0f;
									VwCUV.WriteToFile(fOF);

									Push.WriteToFile(fOF);
									Group.WriteToFile(fOF);
									Push.WriteToFile(fOF);
									sprintf(Group.ASCII_ID, "tg%d_%d", XTile, YTile);
									Group.WriteToFile(fOF);
									Push.WriteToFile(fOF);
									rewind(ffile);
									fgets(FileName, 64, ffile);
									fread((char *)&TestFileVersion, sizeof (char), 1, ffile);

									// now write the foliage objects
									for (DatPt = 0; DatPt < Index.CellDat->DatCt; DatPt++)
										{
										if (FolData.ReadFoliageRecord(ffile, Index.FileVersion))
											{
											if (FolData.InterpretFoliageRecord(NULL, Images, &PointData)) // don't need full decoding of 3dobjects, just height, etc
												{
												static long fnum = 0;
												OF_Matrix Matrix;
												OF_Object Obj;
												OF_InstanceDefinition InstDef;
												OF_InstanceReference InstRef;

												Matrix.Clear();
												Matrix.Matrix[0] = (float)PointData.Width;		// x dim
												Matrix.Matrix[10] = (float)PointData.Height;	// z dim
												if (Master->RBounds.IsGeographic)
													{
													ExportX = Matrix.Matrix[12] = (float)((FolData.XYZ[0] + bias_x) * Master->ExportRefData.ExportLonScale);
													ExportY = Matrix.Matrix[13] = (float)((FolData.XYZ[1] + bias_y) * Master->ExportRefData.ExportLatScale);
													} // if
												else
													{
													ExportX = Matrix.Matrix[12] = (float)(FolData.XYZ[0] * Master->ExportRefData.ExportLonScale + bias_x);
													ExportY = Matrix.Matrix[13] = (float)(FolData.XYZ[1] * Master->ExportRefData.ExportLatScale + bias_y);
													} // else
												Matrix.Matrix[14] = (float)(FolData.XYZ[2] * Master->ExportRefData.ElevScale + Master->ExportRefData.RefElev);
												Matrix.Matrix[5] = Matrix.Matrix[15] = 1.0f;

												// see if this object falls on the tile we're exporting
												if ((ExportX >= BLeft) && (ExportX < BRight) && (ExportY >= BBot) && (ExportY < BTop))
													{
													long BaseOffset = 8, FlipX, MakeRef;
													OF_Scale Scale;
													OF_Translate Translate;

													FlipX = PointData.FlipX;
													if (FlipX)
														MakeRef = Instances[FolData.ElementID].FlipInUse;
													else
														MakeRef = Instances[FolData.ElementID].InUse;
													if (MakeRef)
														{
														// use an Instance Reference
														Group.WriteToFile(fOF);
														Push.WriteToFile(fOF);
														Translate.FromXYZ[0] = 0.0;
														Translate.FromXYZ[1] = 0.0;
														Translate.FromXYZ[2] = 0.5;
														Translate.DeltaXYZ[0] = ExportX;
														Translate.DeltaXYZ[1] = ExportY;
														Translate.DeltaXYZ[2] = FolData.XYZ[2] * Master->ExportRefData.ElevScale + Master->ExportRefData.RefElev;
														Matrix.Matrix[12] = (float)Translate.DeltaXYZ[0];
														Matrix.Matrix[13] = (float)Translate.DeltaXYZ[1];
														Matrix.Matrix[14] = (float)Translate.DeltaXYZ[2];
														Scale.CenterX = (float)Translate.DeltaXYZ[0];
														Scale.CenterY = (float)Translate.DeltaXYZ[1];
														Scale.CenterZ = (float)Translate.DeltaXYZ[2];
														Scale.XScale = (float)PointData.Width;
														Scale.ZScale = (float)PointData.Height;
														if (CreateLOD)
															{
															sprintf(TreeLOD.ASCII_ID, "tl%d", TreeNum++);
															TreeLOD.CenterCoordX = Translate.DeltaXYZ[0];
															TreeLOD.CenterCoordY = Translate.DeltaXYZ[1];
															TreeLOD.CenterCoordZ = Translate.DeltaXYZ[2];
															TreeLOD.WriteToFile(fOF);
															Push.WriteToFile(fOF);
															} // if
														Matrix.WriteToFile(fOF);
														Scale.WriteToFile(fOF);
														Translate.WriteToFile(fOF);
														Push.WriteToFile(fOF);
														if (FlipX)
															InstRef.InstanceDefNum = Instances[FolData.ElementID].FlipDefNum;
														else
															InstRef.InstanceDefNum = Instances[FolData.ElementID].DefNum;
														InstRef.WriteToFile(fOF);
														Pop.WriteToFile(fOF);
														if (CreateLOD)
															Pop.WriteToFile(fOF);
														Pop.WriteToFile(fOF);
														} // if InstanceInfo
													else
														{
														// make an Instance Definition + it's own Instance Reference
														if (FlipX)
															{
															Instances[FolData.ElementID].FlipInUse = 1;
															Instances[FolData.ElementID].FlipDefNum = InstDefNum++;
															InstRef.InstanceDefNum = InstDef.InstanceDefNum = Instances[FolData.ElementID].FlipDefNum;
															} // if
														else
															{
															Instances[FolData.ElementID].InUse = 1;
															Instances[FolData.ElementID].DefNum = InstDefNum++;
															InstRef.InstanceDefNum = InstDef.InstanceDefNum = Instances[FolData.ElementID].DefNum;
															} // else
														Group.WriteToFile(fOF);
														Push.WriteToFile(fOF);
														Instances[FolData.ElementID].X = Translate.DeltaXYZ[0] = ExportX;
														Instances[FolData.ElementID].Y = Translate.DeltaXYZ[1] = ExportY;
														Instances[FolData.ElementID].Z = Translate.DeltaXYZ[2] = FolData.XYZ[2] * Master->ExportRefData.ElevScale + Master->ExportRefData.RefElev;
														if (CreateLOD)
															{
															sprintf(TreeLOD.ASCII_ID, "tl%d", TreeNum++);
															TreeLOD.CenterCoordX = Translate.DeltaXYZ[0];
															TreeLOD.CenterCoordY = Translate.DeltaXYZ[1];
															TreeLOD.CenterCoordZ = Translate.DeltaXYZ[2];
															TreeLOD.WriteToFile(fOF);
															Push.WriteToFile(fOF);
															} // if
														InstDef.WriteToFile(fOF);
														Push.WriteToFile(fOF);
														Group.WriteToFile(fOF);
														Push.WriteToFile(fOF);
														sprintf(Obj.ASCII_ID, "o%d", fnum);
														Obj.WriteToFile(fOF);
														Push.WriteToFile(fOF);
														sprintf(Face.ASCII_ID, "f%d", fnum);
														Face.TexPatIndex = XlateTable[FolData.ElementID];
														Face.DrawType = 0;	// Audition fix???
														Face.WriteToFile(fOF);
														Push.WriteToFile(fOF);
														VList.Length = 20;
														VList.WriteToFile(fOF);
														if (FlipX)
															{
															PutB32S(BaseOffset + 192, fOF);
															PutB32S(BaseOffset + 240, fOF);
															PutB32S(BaseOffset + 288, fOF);
															PutB32S(BaseOffset + 336, fOF);
															} // if
														else
															{
															PutB32S(BaseOffset + 0, fOF);
															PutB32S(BaseOffset + 48, fOF);	// size of OF_Vertex_withColorUV
															PutB32S(BaseOffset + 96, fOF);
															PutB32S(BaseOffset + 144, fOF);
															} // else
														/***
														PutB32S(BaseOffset + 60, fOF);	// size of OF_Vertex_withColorNormalUV
														PutB32S(BaseOffset + 120, fOF);
														PutB32S(BaseOffset + 180, fOF);
														***/
														fnum++;
														Pop.WriteToFile(fOF);
														Pop.WriteToFile(fOF);
														Pop.WriteToFile(fOF);
														Pop.WriteToFile(fOF);
														Group.WriteToFile(fOF);
														Matrix.WriteToFile(fOF);
														Translate.FromXYZ[0] = 0.0;
														Translate.FromXYZ[1] = 0.0;
														Translate.FromXYZ[2] = 0.5;
														Scale.CenterX = (float)Translate.DeltaXYZ[0];
														Scale.CenterY = (float)Translate.DeltaXYZ[1];
														Scale.CenterZ = (float)Translate.DeltaXYZ[2];
														Scale.XScale = (float)PointData.Width;
														Scale.ZScale = (float)PointData.Height;
														Scale.WriteToFile(fOF);
														Translate.WriteToFile(fOF);
														Push.WriteToFile(fOF);
														InstRef.WriteToFile(fOF);
														Pop.WriteToFile(fOF);
														if (CreateLOD)
															Pop.WriteToFile(fOF);
														Pop.WriteToFile(fOF);
														} // else InstanceInfo

													} // if in tile bounds

												} // if
											} // if
										} // for

									} // if Instancing
								else
									{
									// %*!(^@$ alternate method - _EACH_ tree has it's own polygon def with transforms & scale baked in
									long BaseOffset = 8, fnum = 0, VertLen;

									rewind(ffile);
									fgets(FileName, 64, ffile);
									// version
									fread((char *)&TestFileVersion, sizeof (char), 1, ffile);

									// must compute & save _ALL the verts first
									VertStart = ftell(fOF);
									//VP.TotalLength will be set later
									VP.WriteToFile(fOF);
									VwCUV.Flags = 0x2000;

									for (DatPt = 0; DatPt < Index.CellDat->DatCt; DatPt++)
										{
										if (FolData.ReadFoliageRecord(ffile, Index.FileVersion))
											{
											if (FolData.InterpretFoliageRecord(NULL, Images, &PointData)) // don't need full decoding of 3dobjects, just height, etc
												{
												double dx, dy, ExportZ;

												if (Master->RBounds.IsGeographic)
													{
													ExportX = (FolData.XYZ[0] + bias_x) * Master->ExportRefData.ExportLonScale;
													ExportY = (FolData.XYZ[1] + bias_y) * Master->ExportRefData.ExportLatScale;
													} // if
												else
													{
													ExportX = FolData.XYZ[0] * Master->ExportRefData.ExportLonScale + bias_x;
													ExportY = FolData.XYZ[1] * Master->ExportRefData.ExportLatScale + bias_y;
													} // else
												ExportZ = FolData.XYZ[2] * Master->ExportRefData.ElevScale + Master->ExportRefData.RefElev;

												// see if this object falls on the tile we're exporting
												if ((ExportX >= BLeft) && (ExportX < BRight) && (ExportY >= BBot) && (ExportY < BTop))
													{
													dx = PointData.Width * 0.5;
													dy = PointData.Height;

													// write 4 vertices:
													//
													// 0 --- 1   Z
													// |     |   |
													// |     |   |
													// 3 --- 2   *----X
													//
													// 0 vertex
													VwCUV.VertexCoordX = ExportX - dx;
													VwCUV.VertexCoordY = ExportY;
													VwCUV.VertexCoordZ = ExportZ + dy;
													VwCUV.TextureCoordU = 0.0f;
													VwCUV.TextureCoordV = 1.0f;
													VwCUV.WriteToFile(fOF);
													// 1 vertex
													VwCUV.VertexCoordX = ExportX + dx;
													//VwCUV.VertexCoordY = ExportY;
													//VwCUV.VertexCoordZ = ExportZ + dy;
													VwCUV.TextureCoordU = 1.0f;
													//VwCUV.TextureCoordV = 1.0f;
													VwCUV.WriteToFile(fOF);
													// 2 vertex
													//VwCUV.VertexCoordX = ExportX + dx;
													//VwCUV.VertexCoordY = ExportY;
													VwCUV.VertexCoordZ = ExportZ;
													//VwCUV.TextureCoordU = 1.0f;
													VwCUV.TextureCoordV = 0.0f;
													VwCUV.WriteToFile(fOF);
													// 3 vertex
													VwCUV.VertexCoordX = ExportX - dx;
													//VwCUV.VertexCoordY = ExportY;
													//VwCUV.VertexCoordZ = ExportZ;
													VwCUV.TextureCoordU = 0.0f;
													//VwCUV.TextureCoordV = 0.0f;
													VwCUV.WriteToFile(fOF);
													} // if on tile
												} // if
											} // if
										} // for
									VertLen = ftell(fOF) - VertStart;		// compute vertex list length
									fseek(fOF, VertStart + 4, SEEK_SET);	// seek to vertex palette length
									PutB32S(VertLen, fOF);					// write the length
									fseek(fOF, 0, SEEK_END);				// reposition to end of file

									Push.WriteToFile(fOF);
									Group.WriteToFile(fOF);
									Push.WriteToFile(fOF);
									sprintf(Group.ASCII_ID, "tg%d_%d", XTile, YTile);
									Group.WriteToFile(fOF);
									Push.WriteToFile(fOF);
									rewind(ffile);
									fgets(FileName, 64, ffile);
									fread((char *)&TestFileVersion, sizeof (char), 1, ffile);

									// now write the foliage objects
									for (DatPt = 0; DatPt < Index.CellDat->DatCt; DatPt++)
										{
										if (FolData.ReadFoliageRecord(ffile, Index.FileVersion))
											{
											if (FolData.InterpretFoliageRecord(NULL, Images, &PointData)) // don't need full decoding of 3dobjects, just height, etc
												{
												long CurVert = 0;
												OF_Object Obj;

												// see if this object falls on the tile we're exporting
												if ((ExportX >= BLeft) && (ExportX < BRight) && (ExportY >= BBot) && (ExportY < BTop))
													{
													Group.WriteToFile(fOF);
													Push.WriteToFile(fOF);
													//BSphere.WriteToFile(fOF);
													//Push.WriteToFile(fOF);
													sprintf(Obj.ASCII_ID, "o%d", fnum);
													Obj.WriteToFile(fOF);
													Push.WriteToFile(fOF);
													sprintf(Face.ASCII_ID, "f%d", fnum);
													Face.TexPatIndex = FolData.ElementID;
													Face.DrawType = 0;	// Audition fix???
													Face.WriteToFile(fOF);
													Push.WriteToFile(fOF);
													VList.Length = 20;
													VList.WriteToFile(fOF);
													PutB32S(BaseOffset, fOF);
													PutB32S(BaseOffset + 48, fOF);	// size of OF_Vertex_withColorUV
													PutB32S(BaseOffset + 96, fOF);
													PutB32S(BaseOffset + 144, fOF);
													BaseOffset += 192;
													fnum++;
													Pop.WriteToFile(fOF);
													Pop.WriteToFile(fOF);
													//Pop.WriteToFile(fOF);
													Pop.WriteToFile(fOF);
													} // if
												} // if
											} // if
										} // for

									} // else Instancing
								} // if flipboards
							else if (Master->FoliageStyle == WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_CROSSBOARDS)
								{
								long BaseOffset = 8, fnum = 0, VertLen;

								rewind(ffile);
								fgets(FileName, 64, ffile);
								// version
								fread((char *)&TestFileVersion, sizeof (char), 1, ffile);

								// must compute & save _ALL_ the verts first
								VertStart = ftell(fOF);
								//VP.TotalLength will be set later
								VP.WriteToFile(fOF);
								VwCUV.Flags = 0x2000;

								for (DatPt = 0; DatPt < Index.CellDat->DatCt; DatPt++)
									{
									if (FolData.ReadFoliageRecord(ffile, Index.FileVersion))
										{
										if (FolData.InterpretFoliageRecord(NULL, Images, &PointData)) // don't need full decoding of 3dobjects, just height, etc
											{
											double dx, dy, ExportZ;

											if (Master->RBounds.IsGeographic)
												{
												ExportX = (FolData.XYZ[0] + bias_x) * Master->ExportRefData.ExportLonScale;
												ExportY = (FolData.XYZ[1] + bias_y) * Master->ExportRefData.ExportLatScale;
												} // if
											else
												{
												ExportX = FolData.XYZ[0] * Master->ExportRefData.ExportLonScale + bias_x;
												ExportY = FolData.XYZ[1] * Master->ExportRefData.ExportLatScale + bias_y;
												} // else
											ExportZ = FolData.XYZ[2] * Master->ExportRefData.ElevScale + Master->ExportRefData.RefElev;

											// see if this object falls on the tile we're exporting
											if ((ExportX >= BLeft) && (ExportX < BRight) && (ExportY >= BBot) && (ExportY < BTop))
												{
												dx = PointData.Width * 0.5;
												dy = PointData.Height;

												// polygon in xz plane
												// write 4 vertices:
												//
												// 1 --- 0   Z
												// |     |   |
												// |     |   |
												// 2 --- 3   *----X
												//
												// 0 vertex
												VwCUV.VertexCoordX = ExportX + dx;
												VwCUV.VertexCoordY = ExportY;
												VwCUV.VertexCoordZ = ExportZ + dy;
												VwCUV.TextureCoordU = 1.0f;
												VwCUV.TextureCoordV = 1.0f;
												VwCUV.WriteToFile(fOF);
												// 1 vertex
												VwCUV.VertexCoordX = ExportX - dx;
												//VwCUV.VertexCoordY = ExportY;
												//VwCUV.VertexCoordZ = ExportZ + dy;
												VwCUV.TextureCoordU = 0.0f;
												//VwCUV.TextureCoordV = 1.0f;
												VwCUV.WriteToFile(fOF);
												// 2 vertex
												//VwCUV.VertexCoordX = ExportX - dx;
												//VwCUV.VertexCoordY = ExportY;
												VwCUV.VertexCoordZ = ExportZ;
												//VwCUV.TextureCoordU = 0.0f;
												VwCUV.TextureCoordV = 0.0f;
												VwCUV.WriteToFile(fOF);
												// 3 vertex
												VwCUV.VertexCoordX = ExportX + dx;
												//VwCUV.VertexCoordY = ExportY;
												//VwCUV.VertexCoordZ = ExportZ;
												VwCUV.TextureCoordU = 1.0f;
												//VwCUV.TextureCoordV = 0.0f;
												VwCUV.WriteToFile(fOF);

												// polygon in yz plane
												// write 4 vertices:
												//
												// 1 --- 0   Z
												// |     |   |
												// |     |   |
												// 2 --- 3   *----Y
												//
												// 0 vertex
												VwCUV.VertexCoordX = ExportX;
												VwCUV.VertexCoordY = ExportY + dx;	// confusing, eh? ;)
												VwCUV.VertexCoordZ = ExportZ + dy;
												VwCUV.TextureCoordU = 1.0f;
												VwCUV.TextureCoordV = 1.0f;
												VwCUV.WriteToFile(fOF);
												// 1 vertex
												//VwCUV.VertexCoordX = ExportX;
												VwCUV.VertexCoordY = ExportY - dx;
												//VwCUV.VertexCoordZ = ExportZ + dy;
												VwCUV.TextureCoordU = 0.0f;
												//VwCUV.TextureCoordV = 1.0f;
												VwCUV.WriteToFile(fOF);
												// 2 vertex
												//VwCUV.VertexCoordX = ExportX;
												//VwCUV.VertexCoordY = ExportY - dx;
												VwCUV.VertexCoordZ = ExportZ;
												//VwCUV.TextureCoordU = 0.0f;
												VwCUV.TextureCoordV = 0.0f;
												VwCUV.WriteToFile(fOF);
												// 3 vertex
												//VwCUV.VertexCoordX = ExportX;
												VwCUV.VertexCoordY = ExportY + dx;
												//VwCUV.VertexCoordZ = ExportZ;
												VwCUV.TextureCoordU = 1.0f;
												//VwCUV.TextureCoordV = 0.0f;
												VwCUV.WriteToFile(fOF);

												} // if on tile
											} // if
										} // if
									} // for
								VertLen = ftell(fOF) - VertStart;		// compute vertex list length
								fseek(fOF, VertStart + 4, SEEK_SET);	// seek to vertex palette length
								PutB32S(VertLen, fOF);					// write the length
								fseek(fOF, 0, SEEK_END);				// reposition to end of file

								Push.WriteToFile(fOF);
								Group.WriteToFile(fOF);
								Push.WriteToFile(fOF);
								sprintf(Group.ASCII_ID, "tg%d_%d", XTile, YTile);
								Group.WriteToFile(fOF);
								Push.WriteToFile(fOF);
								rewind(ffile);
								fgets(FileName, 64, ffile);
								fread((char *)&TestFileVersion, sizeof (char), 1, ffile);

								// now write the foliage objects
								Face.DrawType = 1;	// double-sided
								Face.Template = 1;	// fixed, alpha blending
								for (DatPt = 0; DatPt < Index.CellDat->DatCt; DatPt++)
									{
									if (FolData.ReadFoliageRecord(ffile, Index.FileVersion))
										{
										if (FolData.InterpretFoliageRecord(NULL, Images, &PointData)) // don't need full decoding of 3dobjects, just height, etc
											{
											OF_Object Obj;

											if (Master->RBounds.IsGeographic)
												{
												ExportX = (FolData.XYZ[0] + bias_x) * Master->ExportRefData.ExportLonScale;
												ExportY = (FolData.XYZ[1] + bias_y) * Master->ExportRefData.ExportLatScale;
												} // if
											else
												{
												ExportX = FolData.XYZ[0] * Master->ExportRefData.ExportLonScale + bias_x;
												ExportY = FolData.XYZ[1] * Master->ExportRefData.ExportLatScale + bias_y;
												} // else

											// see if this object falls on the tile we're exporting
											if ((ExportX >= BLeft) && (ExportX < BRight) && (ExportY >= BBot) && (ExportY < BTop))
												{
												Group.WriteToFile(fOF);
												Push.WriteToFile(fOF);
												if (CreateLOD)
													{
													sprintf(TreeLOD.ASCII_ID, "tl%d", TreeNum);
													TreeLOD.CenterCoordX = ExportX;
													TreeLOD.CenterCoordY = ExportY;
													TreeLOD.CenterCoordZ = FolData.XYZ[2] * Master->ExportRefData.ElevScale + Master->ExportRefData.RefElev;
													TreeLOD.WriteToFile(fOF);
													Push.WriteToFile(fOF);
													} // if
												sprintf(Obj.ASCII_ID, "o%d", TreeNum++);
												Obj.WriteToFile(fOF);
												Push.WriteToFile(fOF);
												sprintf(Face.ASCII_ID, "f%d", fnum);
												Face.TexPatIndex = XlateTable[FolData.ElementID];
												Face.WriteToFile(fOF);
												fnum++;
												Push.WriteToFile(fOF);
												VList.Length = 20;
												VList.WriteToFile(fOF);
												PutB32S(BaseOffset, fOF);
												PutB32S(BaseOffset + 48, fOF);	// size of OF_Vertex_withColorUV
												PutB32S(BaseOffset + 96, fOF);
												PutB32S(BaseOffset + 144, fOF);
												BaseOffset += 192;
												Pop.WriteToFile(fOF);
												Push.WriteToFile(fOF);
												sprintf(Face.ASCII_ID, "f%d", fnum);
												Face.TexPatIndex = XlateTable[FolData.ElementID];
												Face.WriteToFile(fOF);
												fnum++;
												Push.WriteToFile(fOF);
												VList.Length = 20;
												VList.WriteToFile(fOF);
												PutB32S(BaseOffset, fOF);
												PutB32S(BaseOffset + 48, fOF);	// size of OF_Vertex_withColorUV
												PutB32S(BaseOffset + 96, fOF);
												PutB32S(BaseOffset + 144, fOF);
												BaseOffset += 192;
												Pop.WriteToFile(fOF);
												Pop.WriteToFile(fOF);
												Pop.WriteToFile(fOF);
												if (CreateLOD)
													{
													Pop.WriteToFile(fOF);
													} // if
												Pop.WriteToFile(fOF);
												} // if
											} // if
										} // if
									} // for
								} // else if crossboards

							Pop.WriteToFile(fOF);
							Pop.WriteToFile(fOF);
							Pop.WriteToFile(fOF);

							fclose(fOF);

							// make the connection from the parent file
							Group.WriteToFile(fOF_Parent);
							Push.WriteToFile(fOF_Parent);
							strcpy(XRef.ASCII_Path, FoliageFileName);
							XRef.WriteToFile(fOF_Parent);
							Pop.WriteToFile(fOF_Parent);

							if (Instances)
								AppMem_Free(Instances, InstanceSize);

							} // if WriteFol

						} // if TestFileVersion

					fclose(ffile);

					} // if ffile
				} // if Index.NumCells && Index.CellDat->DatCt

			// must do this as it's not dynamically allocated and RealtimeFoliageIndex
			// destructor will blow chunks if we don't
			Index.CellDat = NULL;

			} // if ffile
		} // if DataFileName
	} // if IndexFileName

// must do this as it's not dynamically allocated and RealtimeFoliageIndex
// destructor will blow chunks if we don't
Index.CellDat = NULL;

if (XlateTable)
	AppMem_Free(XlateTable, XlateNum * 2);

return (Success);

} // ExportFormatOpenFlight::ProcessFoliageList

/*===========================================================================*/

void ExportFormatOpenFlight::ProcessMatTexture(long MatCt, FILE *fOF, Object3DEffect *CurObj)
{
MaterialEffect *Mat;
RootTexture *RootTex;
Texture *UVTex;
Raster *Rast;
long UseImage = 0;
long DiffuseAlphaAvailable = 0;
char TextureName[256];
OF_MaterialPalette MP;

if (! CurObj->NameTable[MatCt].Mat)
	CurObj->NameTable[MatCt].Mat = (MaterialEffect *)GlobalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, CurObj->NameTable[MatCt].Name);

if (Mat = CurObj->NameTable[MatCt].Mat)
	{
	/***
	if (Mat->ShortName[0])
		{
		strncpy(MP.MatName, Mat->ShortName, 11);
		TextureName[11] = 0;
		} // if
	else
		{
		sprintf(MP.MatName, "OM%06d", MatCt);
		strcpy(Mat->ShortName, TextureName);
		} // else
	***/
	sprintf(MP.MatName, "Mat%04d", MatCt);

	MP.MatIndex = MatCt;
	MP.Shininess = 20.48f;	// ???
	MP.Alpha = 1.0f;
	for (long i = 0; i < 3; i++)
		{
		//MP.Ambient[i] = 0.3f;	// ???
		MP.Ambient[i] = (float)CurObj->NameTable[MatCt].Mat->DiffuseColor.CurValue[i];
		MP.Diffuse[i] = (float)CurObj->NameTable[MatCt].Mat->DiffuseColor.CurValue[i];
		MP.Specular[i] = (float)CurObj->NameTable[MatCt].Mat->SpecularColor.CurValue[i];
		MP.Emissive[i] = (float)CurObj->NameTable[MatCt].Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].CurValue;
		} // for
	MP.WriteToFile(fOF);

	if (CurObj->VertexUVWAvailable)
		{
		if (RootTex = Mat->GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
			{
			if (UVTex = RootTex->Tex)
				{
				if (UVTex->TexType == WCS_TEXTURE_TYPE_UVW)
					{
					if (UVTex->Img && (Rast = UVTex->Img->GetRaster()))
						{
						if (CheckAndShortenFileName(TextureName, Rast->GetPath(), Rast->GetName(), 200))
							Rast->PAF.SetName(TextureName);
						if (PROJ_fopen(TextureName, "rb") == NULL)
							{
							char FullPathAndName[512];

							Rast->GetPathAndName(FullPathAndName);
							CopyExistingFile(FullPathAndName, Master->OutPath.GetPath(), TextureName);
							} // if
						UseImage = 1;
						} // if
					} // if
				} // if
			} // if
		if (RootTex = Mat->GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY))
			{
			if (UVTex = RootTex->Tex)
				{
				if (UVTex->TexType == WCS_TEXTURE_TYPE_UVW)
					{
					if (UVTex->Img && (Rast = UVTex->Img->GetRaster()))
						{
						if (CheckAndShortenFileName(TextureName, Rast->GetPath(), Rast->GetName(), 200))
							Rast->PAF.SetName(TextureName);
						if (PROJ_fopen(TextureName, "rb") == NULL)
							{
							char FullPathAndName[512];

							Rast->GetPathAndName(FullPathAndName);
							CopyExistingFile(FullPathAndName, Master->OutPath.GetPath(), TextureName);
							} // if
						UseImage = 1;
						} // if
					} // if
				} // if
			} // if
		else if (DiffuseAlphaAvailable)
			{
			if (RootTex = Mat->GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
				{
				if (UVTex = RootTex->Tex)
					{
					if (UVTex->TexType == WCS_TEXTURE_TYPE_UVW)
						{
						if (UVTex->Img && (Rast = UVTex->Img->GetRaster()))
							{
							if (CheckAndShortenFileName(TextureName, Rast->GetPath(), Rast->GetName(), 200))
								Rast->PAF.SetName(TextureName);
							if (PROJ_fopen(TextureName, "rb") == NULL)
								{
								char FullPathAndName[512];

								Rast->GetPathAndName(FullPathAndName);
								CopyExistingFile(FullPathAndName, Master->OutPath.GetPath(), TextureName);
								} // if
							UseImage = 1;
							} // if
						} // if
					} // if
				} // if
			} // else
		} // if
	} // if
else
	{
	sprintf(TextureName, "OM%06d", MatCt);
	} // else

if (UseImage)
	{
	FILE *fTAF;
	OF_TextureAttributeFile TAF;
	PathAndFile attrPAF;
	char attrFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

	//strcat(TextureName, ".attr");
	attrPAF.SetPath((char *)Master->OutPath.GetPath());
	attrPAF.SetName(TextureName);
	attrPAF.GetFramePathAndName(attrFullPath, ".attr", 0, WCS_PATHANDFILE_PATH_PLUS_NAME_LEN, 0);
	if (fTAF = PROJ_fopen(attrFullPath, "wb"))
		{
		TAF.TexelsU = Rast->Cols;
		TAF.TexelsV = Rast->Rows;
		TAF.WriteToFile(fTAF);
		fclose(fTAF);
		} // if
	} // if UseImage

} // ExportFormatOpenFlight::ProcessMatTexture

/*===========================================================================*/

char *ExportFormatOpenFlight::TimeStamp(void)
{
struct tm *newtime;
time_t userclock;

time(&userclock);
newtime = localtime(&userclock);

return asctime(newtime);

} // ExportFormatOpenFlight::TimeStamp
//lint -restore
