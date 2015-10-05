// ExportFormatFBX.cpp
// Code module for FBX export code
// Created from ExportFormatCOLLADA.cpp on 03/15/05 by FPW2
// Copyright 2005 3D Nature, LLC. All rights reserved.

// SDK used for this code is 2009.1 from ~05/24/2008
// SDK also identifies itself as build 20080516, and generates FBX format 6.1.0

/***

   3DN Axis:

   Z Y
   |/
   *- X

***/

/***

   3D Object Axis:

   Y Z
   |/
   *- X

***/

/***

   FBX Axis: (this is the current default, but we're also initializing it to this {MayaYUp} just in case it changes)

   Y
   |
   *- X
  /
 Z

***/

#include "stdafx.h"
#include "FeatureConfig.h" // to pick up FBX define

#ifdef WCS_BUILD_FBX

#include "stdafx.h"
#include "Lists.h"
#include "ExportFormat.h"
#include "EffectsLib.h"
#include "ExportControlGUI.h"
#include "IncludeExcludeList.h"
#include "Raster.h"
#include "ImageOutputEvent.h"
#include "Project.h"
#include "Requester.h"
#include "RasterResampler.h"
#include "Log.h"
#include "SceneExportGUI.h"
#include "SXExtension.h"
#include "Security.h"
#include "WCSVersion.h"

#define K_PLUGIN
#define K_FBXSDK
#define K_NODLL

#include <fbxsdk.h>

#include <fbxfilesdk/kfbxio/kfbximporter.h>
#include <fbxfilesdk/kfbxplugins/kfbxsdkmanager.h>
#include <fbxfilesdk/kfbxplugins/kfbxscene.h>

#include <fbxfilesdk/components/kbaselib/klib/kstring.h>
#include <fbxfilesdk/components/kbaselib/klib/ktime.h>
#include <fbxfilesdk/kfbxmath/kfbxtransformation.h>
#include <fbxfilesdk/kfbxmath/kfbxvector4.h>
#include <fbxfilesdk/kfbxplugins/kfbxcamera.h>
#include <fbxfilesdk/kfbxplugins/kfbxgloballightsettings.h>
#include <fbxfilesdk/kfbxplugins/kfbxnode.h>
#include <fbxfilesdk/kfbxplugins/kfbxsurfacematerial.h>
#include <fbxfilesdk/kfbxplugins/kfbxsurfacelambert.h>
#include <fbxfilesdk/kfbxplugins/kfbxsurfacephong.h>

#include "FBX_Thumbnail.h"

ExportFormatFBX::ExportFormatFBX(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource)
: ExportFormat(MasterSource, ProjectSource, EffectsSource, DBSource, ImageSource)
{
pScene = NULL;
fbxExporter = NULL;
//lWhiteMat = NULL;
phongNum = 0;

pSdkManager = KFbxSdkManager::Create();
if (pSdkManager)
	{
	// Create the entity that will hold the scene.
	pScene = KFbxScene::Create(pSdkManager, "");

	strcpy(takeName, "Take001");

// Create a white material to be used for textured polygons.
//	lWhiteMat = pSdkManager->CreateKFbxMaterial("White");
//	lWhiteMat->SetEmissive(KFbxColor(0.0, 0.0, 0.0));
//	lWhiteMat->SetAmbient(KFbxColor(0.0, 0.0, 0.0));
//	lWhiteMat->SetDiffuse(KFbxColor(1.0, 1.0, 1.0));
//	lWhiteMat->SetSpecular(KFbxColor(0.0, 0.0, 0.0));
//	lWhiteMat->SetOpacity(1.0);
//	lWhiteMat->SetShininess(0.5);
//	lWhiteMat->SetShadingModel("phong");

	} // if
else
	{
	UserMessageOK("FBX Export", "Unable to create the FBX SDK manager, the license may be expired\n");
	takeName[0] = 0;
	} // else

//matIndex = 1;	// 0 is reserved for "White"
texIndex = 0;

} // ExportFormatFBX::ExportFormatFBX

/*===========================================================================*/

ExportFormatFBX::~ExportFormatFBX()
{

pSdkManager->Destroy();

} // ExportFormatFBX::~ExportFormatFBX

/*===========================================================================*/

int ExportFormatFBX::SanityCheck(void)
{

return (1);

} // ExportFormatFBX::SanityCheck

/*===========================================================================*/

int ExportFormatFBX::PackageExport(NameList **FileNamesCreated)
{
KFbxNode *l3DObjects = NULL, *lCameras = NULL, *lFoliage = NULL, *lLights = NULL, *lRootNode = NULL, *lTerrain = NULL;	//, *lVectors = NULL;
KTime lTime;
EffectList *CurHaze;
const char *CleanFileName, *OutputFilePath;
int lMajor, lMinor, lRevision;
int success = 1;
long FileType;
PathAndFile PaF;
char TempFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

if (GlobalApp->Sentinal->CheckAuthFieldSX2() == 0)
	return 0;

if ((! pSdkManager) || (! pScene))
	{
	UserMessageOK("FBX Export", "FBX Scene creation failed.");
	success = 0;
	} // if

// The directory where all the files should be created is:
OutputFilePath = Master->OutPath.GetPath();

fbxExporter = KFbxExporter::Create(pSdkManager, "");
if (fbxExporter)
	{
	// Initialize the exporter by providing a filename.
	PaF.SetPath((char *)Master->OutPath.GetPath());
	PaF.SetName((char *)Master->OutPath.GetName());
	PaF.GetFramePathAndName(TempFullPath, ".fbx", 0, WCS_PATHANDFILE_PATH_PLUS_NAME_LEN, 0);
	if (fbxExporter->Initialize(GlobalApp->MainProj->MungPath(TempFullPath)))
		{
		char versionMsg[64];

		KFbxIO::GetCurrentVersion(lMajor, lMinor, lRevision);
		sprintf(versionMsg, "Using FBX SDK Version %d.%d.%d\n\n", lMajor, lMinor, lRevision);
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, versionMsg);
		} // if
	else
		{
		UserMessageOK("FBX Export", "FBX Exporter initialization failed.");
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, fbxExporter->GetLastErrorString());
		success = 0;
		} // else
	} // if
else
	{
	UserMessageOK("FBX Export", "FBX Exporter creation failed.");
	success = 0;
	} // else

if (! success)
	return 0;	// we've already failed

SetGlobalSettings();

DoSceneInfo();

if (lRootNode = pScene->GetRootNode())
	{
	centerCoordZ = (Master->ExportRefData.RefElev + Master->ExportRefData.MaxElev ) * 0.5;
	//pScene->GetGlobalCameraSettings().GetCameraProducerPerspective().SetNearPlane(0.1);
	//pScene->GetGlobalCameraSettings().GetCameraProducerPerspective().SetFarPlane(50000.0);
	} // if
else
	{
	success = 0;
	} // else

pScene->GetGlobalLightSettings().SetAmbientColor(KFbxColor(0.5, 0.5, 0.5));

lTime.SetGlobalTimeMode(KTime::eCUSTOM, GlobalApp->MainProj->Interactive->GetFrameRate());
//pScene->GetGlobalTimeSettings().SetTimeMode(KTime::eCUSTOM);

if (success && Master->ObjectInstanceList)
	{
	if (success = ExportObjects(Master->ObjectInstanceList, l3DObjects))
		success = lRootNode->AddChild(l3DObjects);
	} // if

if (success && Master->ExportCameras)
	{
	if (success = ExportCameras(Master->Cameras, lCameras))
		{
		success = lRootNode->AddChild(lCameras);
		//pScene->GetGlobalCameraSettings().SetDefaultCamera(namedThing);	// working???
		} // if
	} // if

if (success && Master->ExportTerrain)
	{
	if (success = CreateTerrainMesh(FileNamesCreated, lTerrain))
		success = lRootNode->AddChild(lTerrain);
	} // if

if (success && Master->ExportLights)
	{
	if (success = ExportLights(Master->Lights, lLights))
		success = lRootNode->AddChild(lLights);
	} // if

if (success && Master->ExportFoliage)
	{
	if (success = ExportFoliage(FileNamesCreated, OutputFilePath, lFoliage))
		success = lRootNode->AddChild(lFoliage);
	} // if

if (success && Master->ExportHaze && (CurHaze = Master->Haze))
	{
	if (CurHaze->Me)
		success = ExportHaze((Atmosphere *)CurHaze->Me, 0);
	} // if

/*
if (success && Master->ExportVectors && Master->VecInstanceList)
	{
	if (success = ExportVectors(Master->VecInstanceList, lVectors))
		lRootNode->AddChild(lVectors);
	} // if
*/

pScene->CreateTake(takeName);
pScene->SetCurrentTake(takeName);

if (success)
	success = SaveScene();

// remove foliage temp files
FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILEIDX;
if (CleanFileName = (*FileNamesCreated)->FindNameOfType(FileType))
	{
	strmfp(TempFullPath, OutputFilePath, CleanFileName);
	PROJ_remove(TempFullPath);

	FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILE;
	if (CleanFileName = (*FileNamesCreated)->FindNameOfType(FileType))
		{
		strmfp(TempFullPath, OutputFilePath, CleanFileName);
		PROJ_remove(TempFullPath);
		} // if
	} // if

return (success);

} // ExportFormatFBX::PackageExport

/*===========================================================================*/

int ExportFormatFBX::Process3DObjects(NameList **FileNamesCreated, const char *OutputFilePath)
{
int Success = 1;

return (Success);

} // ExportFormatFBX::Process3DObjects

/*===========================================================================*/

bool ExportFormatFBX::ExportFoliage(NameList **FileNamesCreated, const char *OutputFilePath, KFbxNode* &lFoliage)
{
FILE *ffile = NULL, *mel = NULL;
const char *DataFileName, *IndexFileName;
bool success = false;
long DatPt, FileType;
FoliagePreviewData PointData;
RealtimeFoliageCellData RFCD;
RealtimeFoliageData FolData;
RealtimeFoliageIndex Index;
PathAndFile PaF;
char TempFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];
char FileName[512], TestFileVersion;

if (!(lFoliage = KFbxNode::Create(pSdkManager, "Foliage")))
	return success;

// Initialize the exporter by providing a filename.
PaF.SetPath((char *)Master->OutPath.GetPath());
PaF.SetName((char *)Master->OutPath.GetName());
PaF.GetFramePathAndName(TempFullPath, ".mel", 0, WCS_PATHANDFILE_PATH_PLUS_NAME_LEN, 0);

FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILEIDX;
if (IndexFileName = (*FileNamesCreated)->FindNameOfType(FileType))
	{
	FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILE;
	if (DataFileName = (*FileNamesCreated)->FindNameOfType(FileType))
		{
		//double TexRefLat, TexRefLon, TexRefElev, TexDeltaLat, TexDeltaLon;
		// you've got the two file names that the Renderer wrote out.
		// combine them with the output file path to make a file that can be opened with PROJ_fopen()

		mel = PROJ_fopen(GlobalApp->MainProj->MungPath(TempFullPath), "w");

		// find and open the index file
		strmfp(FileName, OutputFilePath, IndexFileName);
		if (ffile = PROJ_fopen(FileName, "rb"))
			{
			// read file descriptor, no need to keep it around unless you want to
			fgets(FileName, 256, ffile);
			// version
			fread((char *)&Index.FileVersion, sizeof(char), 1, ffile);
			// number of files
			fread((char *)&Index.NumCells, sizeof(long), 1, ffile);
			// reference XYZ
			fread((char *)&Index.RefXYZ[0], sizeof(double), 1, ffile);
			fread((char *)&Index.RefXYZ[1], sizeof(double), 1, ffile);
			fread((char *)&Index.RefXYZ[2], sizeof(double), 1, ffile);

			if (Index.NumCells > 0)
				{
				// only one cell data entry is provided
				if (Index.CellDat = &RFCD)
					{
					// file name
					fgets(Index.CellDat->FileName, 64, ffile);
					// center XYZ
					fread((char *)&Index.CellDat->CellXYZ[0], sizeof(double), 1, ffile);
					fread((char *)&Index.CellDat->CellXYZ[1], sizeof(double), 1, ffile);
					fread((char *)&Index.CellDat->CellXYZ[2], sizeof(double), 1, ffile);
					// half cube cell dimension
					fread((char *)&Index.CellDat->CellRad, sizeof(double), 1, ffile);
					// number of trees in file
					fread((char *)&Index.CellDat->DatCt, sizeof(long), 1, ffile);
					} // if
				} // if some cells to read
			fclose(ffile);

			if ((Index.NumCells > 0) && (Index.CellDat->DatCt > 0))
				{
				strmfp(FileName, OutputFilePath, DataFileName);
				if (ffile = PROJ_fopen(FileName, "rb"))
					{
					Raster *CurRast, *RastLoop;
					long n;

					for (n = 1, RastLoop = Images->GetFirstRast(); RastLoop; RastLoop = Images->GetNextRast(RastLoop), n++)
						{
						char ImageName[256];

						if (CurRast = Images->FindByID(n))
							{
							strcpy(ImageName, CurRast->GetUserName());
							ImageSaverLibrary::StripImageExtension(ImageName);
							strcat(ImageName, "_Fol");
							ReplaceChar(ImageName, '.', '_');
							strcat(ImageName, ImageSaverLibrary::GetDefaultExtension(Master->FoliageImageFormat));
							FileType = WCS_EXPORTCONTROL_FILETYPE_FOLIAGETEX;
							if ((*FileNamesCreated)->FindNameExists(FileType, ImageName))
								{
								// indeed a resampled image has been created for this entry
								// so it is safe to add this entry into the foliage definitions
								strcpy(ImageName, RastLoop->GetUserName());
								ImageSaverLibrary::StripImageExtension(ImageName);
								strcat(ImageName, "_Fol");
								ReplaceChar(ImageName, '.', '_');
								strcat(ImageName, ImageSaverLibrary::GetDefaultExtension(Master->FoliageImageFormat));
								} // if
							} // if
						} // for

					fgets(FileName, 64, ffile);
					// version
					fread((char *)&TestFileVersion, sizeof(char), 1, ffile);
					// Pointless version check -- we know we wrote it
					if (TestFileVersion == Index.FileVersion)
						{
						unsigned long folNum = 0;

						phongNum += Index.CellDat->DatCt;	// we're going to be counting down for whatever reason...
						for (DatPt = 0; DatPt < Index.CellDat->DatCt; DatPt ++)
							{
							if (FolData.ReadFoliageRecord(ffile, Index.FileVersion))
								{
								if (FolData.InterpretFoliageRecord(NULL, Images, &PointData)) // don't need full decoding of 3dobjects, just height, etc
									{
									double dx, dy, x, y, z;
									KFbxLayer* lLayer;
									KFbxLayerElementNormal* lLayerElementNormal;
									KFbxLayerElementUV* lUVDiffuseLayer;
									KFbxMesh* treeMesh;
									KFbxNode* treeNode;
									KFbxVector4* controlPoints;
									char nodeName[16];

									x = FolData.XYZ[0] * Master->ExportRefData.ExportLonScale;
									//y = (float)(FolData.XYZ[2] * Master->ExportRefData.ElevScale + (PointData.Height * 0.5));
									y = FolData.XYZ[2] + Master->ExportRefData.RefElev - centerCoordZ;
									z = -FolData.XYZ[1] * Master->ExportRefData.ExportLatScale;
									dx = PointData.Width * 0.5;
									dy = PointData.Height;
									sprintf(nodeName, "Fol%d", folNum++);
									treeNode = KFbxNode::Create(pSdkManager, nodeName);
									treeMesh = KFbxMesh::Create(pSdkManager, nodeName);

									if (treeNode && treeMesh)
										{
										KFbxTexture* lTexture;
										//KFbxLayerElementMaterial* lMaterialLayer;
										KFbxLayerElementTexture* lTextureDiffuseLayer;
										bool status;
										char ImageName[256], ImageNameM[256];
										//char FullFileName[1024], FullFileName2[1024];

										treeMesh->Color.Set(fbxDouble3(1.0, 1.0, 1.0));

										//   0 --- 3
										//   |   / |
										//   |  /  |
										//   | /   |
										//   1 --- 2
										treeMesh->InitControlPoints(4);
										controlPoints = treeMesh->GetControlPoints();
										controlPoints[0].Set(x - dx, y + dy, z);
										controlPoints[1].Set(x - dx, y, z);
										controlPoints[2].Set(x + dx, y, z);
										controlPoints[3].Set(x + dx, y + dy, z);

										lLayer = treeMesh->GetLayer(0);
										if (NULL == lLayer)
											{
											treeMesh->CreateLayer();
											lLayer = treeMesh->GetLayer(0);
											} // if

										lLayerElementNormal = KFbxLayerElementNormal::Create(treeMesh, "");
										lLayerElementNormal->SetMappingMode(KFbxLayerElement::eALL_SAME);
										lLayerElementNormal->SetReferenceMode(KFbxLayerElement::eINDEX_TO_DIRECT);
										lLayerElementNormal->GetDirectArray().Add(KFbxVector4(0.0, 0.0, 1.0));
										lLayerElementNormal->GetIndexArray().Add(0);;
										lLayer->SetNormals(lLayerElementNormal);

										// Set texture mapping for diffuse channel.
										lTextureDiffuseLayer = KFbxLayerElementTexture::Create(treeMesh, "Diffuse Texture");
										lTextureDiffuseLayer->SetMappingMode(KFbxLayerElement::eALL_SAME);
										lTextureDiffuseLayer->SetReferenceMode(KFbxLayerElement::eINDEX_TO_DIRECT);
										lTextureDiffuseLayer->GetIndexArray().Add(0);
										lLayer->SetDiffuseTextures(lTextureDiffuseLayer);

										// Create UV for Diffuse channel.
										lUVDiffuseLayer = KFbxLayerElementUV::Create(treeMesh, "DiffuseUV");
										lUVDiffuseLayer->SetMappingMode(KFbxLayerElement::eBY_CONTROL_POINT);
										lUVDiffuseLayer->SetReferenceMode(KFbxLayerElement::eINDEX_TO_DIRECT);
										lLayer->SetUVs(lUVDiffuseLayer, KFbxLayerElement::eDIFFUSE_TEXTURES);

										// 0-1-3 poly
										treeMesh->BeginPolygon(-1, -1, -1);
										treeMesh->AddPolygon(0, -1);
										treeMesh->AddPolygon(1, -1);
										treeMesh->AddPolygon(3, -1);
										treeMesh->EndPolygon();
										// 1-2-3 poly
										treeMesh->BeginPolygon(-1, -1, -1);
										treeMesh->AddPolygon(1, -1);
										treeMesh->AddPolygon(2, -1);
										treeMesh->AddPolygon(3, -1);
										treeMesh->EndPolygon();

										if (PointData.FlipX)
											{
											lUVDiffuseLayer->GetDirectArray().Add(KFbxVector2(1.0, 1.0));
											lUVDiffuseLayer->GetDirectArray().Add(KFbxVector2(1.0, 0.0));
											lUVDiffuseLayer->GetDirectArray().Add(KFbxVector2(0.0, 0.0));
											lUVDiffuseLayer->GetDirectArray().Add(KFbxVector2(0.0, 1.0));	
											} // if
										else
											{
											lUVDiffuseLayer->GetDirectArray().Add(KFbxVector2(0.0, 1.0));
											lUVDiffuseLayer->GetDirectArray().Add(KFbxVector2(0.0, 0.0));
											lUVDiffuseLayer->GetDirectArray().Add(KFbxVector2(1.0, 0.0));
											lUVDiffuseLayer->GetDirectArray().Add(KFbxVector2(1.0, 1.0));	
											} // else
										lUVDiffuseLayer->GetIndexArray().Add(0);
										lUVDiffuseLayer->GetIndexArray().Add(1);
										lUVDiffuseLayer->GetIndexArray().Add(2);
										lUVDiffuseLayer->GetIndexArray().Add(3);

										treeNode->SetShadingMode(KFbxNode::eTEXTURE_SHADING);
										treeNode->SetMultiLayer(false);

										CurRast = Images->FindByID(FolData.ElementID);
										strcpy(ImageName, CurRast->GetUserName());
										ImageSaverLibrary::StripImageExtension(ImageName);
										strcat(ImageName, "_Fol");
										ReplaceChar(ImageName, '.', '_');
										strcat(ImageName, ImageSaverLibrary::GetDefaultExtension(Master->FoliageImageFormat));
										strcpy(ImageNameM, ImageName);
										ReplaceChar(ImageNameM, '.', '_');
										if (mel)
											{
											fprintf(mel, "select -r %s;\n", nodeName);
											fprintf(mel, "connectAttr %s.outTransparency phong%d.transparency;\n", ImageNameM, phongNum--); // why is phongNum backwards???
											} // if
										// Set texture properties.
										//strmfp(FullFileName2, Master->OutPath.GetPath(), ImageName);
										//strcpy(FullFileName, GlobalApp->MainProj->MungPath(FullFileName2));
										lTexture = KFbxTexture::Create(pSdkManager, ImageName);
										status = lTexture->SetFileName(ImageName);
										//status = lTexture->SetRelativeFileName((char *)ImageName);
										lTexture->SetTextureUse(KFbxTexture::eSTANDARD);
										lTexture->SetMappingType(KFbxTexture::eUV);
										lTexture->SetMaterialUse(KFbxTexture::eMODEL_MATERIAL);
										lTexture->SetSwapUV(false);
										lTexture->UseMaterial.Set(fbxBool1(0));
										//lTexture->SetCurrentTakeNode(takeName);
										//lTexture->SetAlphaSource(KFbxTexture::eBLACK);

										treeMesh->GetLayer(0)->GetDiffuseTextures()->GetDirectArray().Add(lTexture);

										treeNode->SetNodeAttribute(treeMesh);
										treeNode->SetShadingMode(KFbxNode::eTEXTURE_SHADING);
										//treeNode->AddMaterial(gMaterial);

										success = lFoliage->AddChild(treeNode);
										} // if
									} // if
								} // if
							} // for
						// must do this as it's not dynamically allocated and RealtimeFoliageIndex
						// destructor will blow chunks if we don't
						Index.CellDat = NULL;
						} // if
					fclose(ffile);
					} // if
				} // if
			} // if ffile
		} // if DataFileName
	} // if IndexFileName

if (mel)
	fclose(mel);

return (success);

} // ExportFormatFBX::ExportFoliage

/*===========================================================================*/

int ExportFormatFBX::ProcessSky(NameList **FileNamesCreated, const char *OutputFilePath)
{
int Success = 1;

return (Success);

} // ExportFormatFBX::Sky

/*===========================================================================*/

bool ExportFormatFBX::CreateTerrainMesh(NameList **FileNamesCreated, KFbxNode* &lTerrain)
{
double adjustX, adjustY, centerCoordX, centerCoordY, csx, csy, sizeX, sizeY;
KFbxMesh* terrainMesh;
KFbxNode* tileNode;
KFbxVector4* controlPoints;
const char *RawTerrainFile = NULL;	// needs to be initialized to NULL for FindNextNameOfType routine
const char *TextureFile = NULL;		// needs to be initialized to NULL for FindNextNameOfType routine
int CPSize = Master->OneDEMResX * Master->OneDEMResY;
bool success = false, tiled = false;
long xTile, yTile;

if (!(lTerrain = KFbxNode::Create(pSdkManager, "Terrain")))
	return success;

if (Master->RBounds.IsGeographic)
	{
	csx = Master->RBounds.CellSizeX * Master->ExportRefData.ExportLonScale;
	csy = Master->RBounds.CellSizeY * Master->ExportRefData.ExportLatScale;
	sizeX = (Master->OneDEMResX - 1) * csx;
	sizeY = (Master->OneDEMResY - 1) * csy;
	} // if
else
	{
	csx = Master->RBounds.CellSizeX;
	csy = Master->RBounds.CellSizeY;
	sizeX = (Master->OneDEMResX - 1) * csx;
	sizeY = (Master->OneDEMResY - 1) * csy;
	} // else

adjustX = sizeX * Master->DEMTilesX * 0.5 - sizeX * 0.5;	// half distance of export area - half tile size
adjustY = sizeY * Master->DEMTilesY * 0.5 - sizeY * 0.5;

if ((Master->DEMTilesX > 1) || (Master->DEMTilesY > 1))
	tiled = true;

for (yTile = 0; yTile < Master->DEMTilesY; ++yTile)
	{
	char tileName[16];

	centerCoordY = (yTile * sizeY)- adjustY;	// this tiles center point - exported area centered on origin
	for (xTile = 0; xTile < Master->DEMTilesX; ++xTile)
		{
		sprintf(tileName, "Tile_%dy_%dx", yTile, xTile);

		centerCoordX = (xTile * sizeX) - adjustX;

		if ((tileNode = KFbxNode::Create(pSdkManager, tileName)) && (terrainMesh = KFbxMesh::Create(pSdkManager, tileName)))
			{
			double xpos, ypos;
			FILE *fRaw;
			KFbxLayer* lLayer;
			float elev;
			KFbxVector4 vert;
			unsigned long m, n, index = 0;
			long FileType;

			phongNum++;

			terrainMesh->Color.Set(fbxDouble3(1.0, 1.0, 1.0));
			// get the number of control points/vertices in the terrain mesh
			terrainMesh->InitControlPoints(CPSize);
			terrainMesh->InitNormals();
			controlPoints = terrainMesh->GetControlPoints();

			lLayer = terrainMesh->GetLayer(0);
			if (lLayer == NULL)
				{
				terrainMesh->CreateLayer();
				lLayer = terrainMesh->GetLayer(0);
				} // if

			// dump the geometry here
			FileType = WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN;
			if (tiled)
				RawTerrainFile = (*FileNamesCreated)->FindNextNameOfType(FileType, RawTerrainFile);
			else
				RawTerrainFile = (*FileNamesCreated)->FindNameOfType(FileType);
			if (fRaw = PROJ_fopen(RawTerrainFile, "rb"))
				{
				long Texturing = Master->ExportTexture;

				ypos = -sizeY * 0.5;	// starting on North end
				for (long row = 0; row < Master->OneDEMResY; row++, ypos += csy)
					{
					xpos = -sizeX * 0.5;	// starting on West end
					for (long col = 0; col < Master->OneDEMResX; col++, index++, xpos += csx)
						{
						fread((void *)&elev, 1, 4, fRaw);
						controlPoints[index].Set(xpos + centerCoordX, elev - centerCoordZ, ypos + centerCoordY);
						} // for xpos
					} // for ypos
				fclose(fRaw);
				PROJ_remove(RawTerrainFile);

				if (Texturing)
					{
					KFbxLayerElementTexture* lTextureDiffuseLayer;
					KFbxLayerElementUV* lUVDiffuseLayer;

					// Set texture mapping for diffuse channel.
					lTextureDiffuseLayer = KFbxLayerElementTexture::Create(terrainMesh, "Diffuse Texture");
					lTextureDiffuseLayer->SetMappingMode(KFbxLayerElement::eALL_SAME);
					lTextureDiffuseLayer->SetReferenceMode(KFbxLayerElement::eINDEX_TO_DIRECT);
					lTextureDiffuseLayer->GetIndexArray().Add(0);
					lLayer->SetDiffuseTextures(lTextureDiffuseLayer);

					// Create UV for Diffuse channel.
					lUVDiffuseLayer = KFbxLayerElementUV::Create(terrainMesh, "DiffuseUV");
					lUVDiffuseLayer->SetMappingMode(KFbxLayerElement::eBY_CONTROL_POINT);
					lUVDiffuseLayer->SetReferenceMode(KFbxLayerElement::eDIRECT);
					lLayer->SetUVs(lUVDiffuseLayer, KFbxLayerElement::eDIFFUSE_TEXTURES);

					ypos = 1.0;	// Y coord
					for (long row = 0; row < Master->OneDEMResY; row++, ypos -= (1.0 / (Master->OneDEMResY - 1)))
						{
						xpos = 0.0;	// U coord
						for (long col = 0; col < Master->OneDEMResX; col++, xpos += (1.0 / (Master->OneDEMResX - 1)))
							{
							lUVDiffuseLayer->GetDirectArray().Add(KFbxVector2(xpos, ypos));
							} // for
						} // for

					tileNode->SetShadingMode(KFbxNode::eTEXTURE_SHADING);
					tileNode->SetMultiLayer(false);
					} // if
				else
					{
					} // else

				// we want to do a loop here to create polygons for the whole terrain
				// Top Left Face - CCW order from n
				//
				// n -- n+1   n+1
				// |  /       /|
				// | /       / |
				// |/       /  |
				// m      m -- m+1
				// m is actually n + OneDemRESX, because we have straight index of control points
				for (long ypos = 0; ypos < (Master->OneDEMResY - 1); ypos++)
					{
					n = ypos * Master->OneDEMResX;
					m = n + Master->OneDEMResX;
					for (long xpos = 0; xpos < (Master->OneDEMResX - 1); xpos++, n++, m++)
						{
						if (Texturing)
							{
							terrainMesh->BeginPolygon(-1, -1, -1);
							terrainMesh->AddPolygon(n, n);
							terrainMesh->AddPolygon(m, m);
							terrainMesh->AddPolygon(n+1, n+1);
							terrainMesh->EndPolygon();
							terrainMesh->BeginPolygon(-1, -1, -1);
							terrainMesh->AddPolygon(n+1, n+1);
							terrainMesh->AddPolygon(m, m);
							terrainMesh->AddPolygon(m+1, m+1);
							terrainMesh->EndPolygon();
							} // else
						else
							{
							terrainMesh->BeginPolygon(-1, -1, -1);
							terrainMesh->AddPolygon(n, -1);
							terrainMesh->AddPolygon(m, -1);
							terrainMesh->AddPolygon(n+1, -1);
							terrainMesh->EndPolygon();
							terrainMesh->BeginPolygon(-1, -1, -1);
							terrainMesh->AddPolygon(n+1, -1);
							terrainMesh->AddPolygon(m, -1);
							terrainMesh->AddPolygon(m+1, -1);
							terrainMesh->EndPolygon();
							} // if
						} // for xpos
					} // for ypos

				terrainMesh->ComputeVertexNormals(false);

				tileNode->SetNodeAttribute(terrainMesh);
				
				if (Master->ExportTexture)
					{
					KFbxTexture* lTexture;
					bool status;
					//char FullFileName[1024], FullFileName2[1024];

					tileNode->SetShadingMode(KFbxNode::eTEXTURE_SHADING);
					tileNode->SetMultiLayer(false);

					FileType = WCS_EXPORTCONTROL_FILETYPE_TEX1;
					if (tiled)
						TextureFile = (*FileNamesCreated)->FindNextNameOfType(FileType, TextureFile);
					else
						TextureFile = (*FileNamesCreated)->FindNameOfType(FileType);
					lTexture = KFbxTexture::Create(pSdkManager, TextureFile);
					texIndex++;
					// Set texture properties.
					//strmfp(FullFileName2, Master->OutPath.GetPath(), TextureFile);
					//strcpy(FullFileName, GlobalApp->MainProj->MungPath(FullFileName2));
					//status = lTexture->SetFileName(FullFileName);
					status = lTexture->SetFileName(TextureFile);
					//status = lTexture->SetRelativeFileName((char *)TextureFile);
					lTexture->SetTextureUse(KFbxTexture::eSTANDARD);
					lTexture->SetMappingType(KFbxTexture::eUV);
					lTexture->SetMaterialUse(KFbxTexture::eMODEL_MATERIAL);
					lTexture->SetSwapUV(false);
					lTexture->UseMaterial.Set(fbxBool1(false));

					terrainMesh->GetLayer(0)->GetDiffuseTextures()->GetDirectArray().Add(lTexture);
					} // if

				if (lTerrain->AddChild(tileNode))
					success = true;
				else
					success = false;
				} // if fRaw
			else
				{
				// kill the terrainMesh & whatever here
				lTerrain = NULL;
				} // else
			} // if
		} // for xTile
	} // for yTile

return success;

} // ExportFormatFBX::CreateTerrainMesh

/*===========================================================================*/

bool ExportFormatFBX::ExportCameras(EffectList *CameraList, KFbxNode* &lCameras)
{
double X, Y;
KFbxNode *camNode, *targetNode;
//RenderJob *curRJ;
//RenderOpt *curOpt;
EffectList *curCam;
bool animated, success = false;
RenderData rendData(NULL);
RasterAnimHostProperties prop;
KTime lTime;
char camName[64], targetName[80];

//lTime.SetGlobalTimeMode(KTime::eFRAMES30);

// if cameras
if (CameraList && (lCameras = KFbxNode::Create(pSdkManager, "Cameras")))
	{
	// for each camera
	for (curCam = CameraList; curCam; curCam = curCam->Next)
		{
		if (curCam->Me)
			{
			double lookFrom[3], lookTo[3], dist;//, pitch = 0.0;
			//Point3d Orient;
			Camera *cam = (Camera *)curCam->Me;
			bool unaimed = true;

			// search for a matching RenderOpt to get additional settings from
			//curOpt = NULL;
			// for each render job, does the camera match
			//for (curRJ = (RenderJob *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_RENDERJOB); curRJ; curRJ = (RenderJob *)curRJ->Next)
			//	{
			//	if (curRJ->Cam == cam)
			//		{
			//		curOpt = curRJ->Options;
			//		break;
			//		} // if
			//	} // for
			
			rendData.InitToView(EffectsHost, ProjectHost, DBHost, ProjectHost->Interactive, NULL, cam, 320, 320);
			cam->InitFrameToRender(GlobalApp->AppEffects, &rendData);

			// make a false target some distance from camera
			dist = 1000.0;	// default to 1000m
			if ((WCS_EFFECTS_CAMERATYPE_OVERHEAD == cam->CameraType) || (WCS_EFFECTS_CAMERATYPE_PLANIMETRIC == cam->CameraType))
				dist = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue - cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].CurValue;
			cam->TargPos->XYZ[0] = cam->CamPos->XYZ[0] + cam->TargPos->XYZ[0] * dist;
			cam->TargPos->XYZ[1] = cam->CamPos->XYZ[1] + cam->TargPos->XYZ[1] * dist;
			cam->TargPos->XYZ[2] = cam->CamPos->XYZ[2] + cam->TargPos->XYZ[2] * dist;

			#ifdef WCS_BUILD_VNS
			rendData.DefCoords->CartToDeg(cam->CamPos);
			rendData.DefCoords->CartToDeg(cam->TargPos);
			#else // WCS_BUILD_VNS
			cam->CamPos->CartToDeg(RendData.PlanetRad);
			cam->TargPos->CartToDeg(RendData.PlanetRad);
			#endif // WCS_BUILD_VNS

			KFbxCamera* lCamera = KFbxCamera::Create(pSdkManager, "");
    
			Master->RBounds.DefDegToRBounds(cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue, cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue, X, Y);
			lookFrom[0] = (X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale;
			lookFrom[1] = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue * Master->ExportRefData.ElevScale - centerCoordZ;
			lookFrom[2] = (Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale;
			Master->RBounds.DefDegToRBounds(cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].CurValue, cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].CurValue, X, Y);
			lookTo[0] = (X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale;
			lookTo[1] = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].CurValue * Master->ExportRefData.ElevScale - centerCoordZ;
			lookTo[2] = (Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale;
			//ADT = Cam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV);

			//KFbxVector4 lCameraLocation(LookFrom[0], LookFrom[1], -LookFrom[2]);	
			//KFbxVector4 lDefaultPointOfInterest(LookTo[0], LookTo[1], -LookTo[2]);	// old way
			// new way - look straight down negative Z - nope, the importers can't standardize on _THIS_ either
			//KFbxVector4 lDefaultPointOfInterest(LookFrom[0], LookFrom[1], -(LookFrom[2] + 1000.0));
			//KFbxVector4 lNewPointOfInterest(0.0, 0.0, 0.0);
			//KFbxVector4 lRotation(-Cam->CamPitch, 90.0 + Cam->CamHeading, Cam->CamBank);
			//KFbxVector4 lRotation;
			//KFbxVector4 lRotation(0.0, 0.0, 0.0);	// zero'd version for debugging
			//KFbxVector4 lScaling(1.0, 1.0, 1.0);
			fbxDouble3 lCameraLocation(lookFrom[0], lookFrom[1], -lookFrom[2]);	
			fbxDouble3 lDefaultPointOfInterest(lookTo[0], lookTo[1], -lookTo[2]);
			//fbxDouble3 lRotation(-cam->CamPitch, 90.0 - cam->CamHeading, cam->CamBank);
			//if ((WCS_EFFECTS_CAMERATYPE_OVERHEAD == cam->CameraType) || (WCS_EFFECTS_CAMERATYPE_PLANIMETRIC == cam->CameraType))
			//	{
			//	pitch = 270.0;
			//	} // if
			//fbxDouble3 lRotation(cam->CamBank, 90.0 - cam->CamHeading, pitch - cam->CamPitch);
			fbxDouble3 lRotation(cam->CamBank, 90.0 - cam->CamHeading, -cam->CamPitch);
			//fbxDouble3 lRotation2(37.0, 47.0, 77.0);
			//fbxDouble3 lScaling(1.0, 1.0, 1.0);
			fbxDouble3 lUp;
			//KFbxXMatrix xm;

			if ((WCS_EFFECTS_CAMERATYPE_OVERHEAD == cam->CameraType) || (WCS_EFFECTS_CAMERATYPE_PLANIMETRIC == cam->CameraType))
				lUp = fbxDouble3(0.0, 0.0, -1.0);
			else
				lUp = fbxDouble3(0.0, 1.0, 0.0);
			lCamera->UpVector.Set(lUp);
			//lCamera->Position.Set(lCameraLocation);
			lCamera->SetNearPlane(0.001);
			lCamera->SetFarPlane(Master->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LODDISTVANISH].CurValue);
			// camera defaults to ePERSPECTIVE
			if ((cam->Orthographic) || (WCS_EFFECTS_CAMERATYPE_PLANIMETRIC == cam->CameraType))
				{
				lCamera->ProjectionType.Set(KFbxCamera::eORTHOGONAL);
				lCamera->OrthoZoom.Set(fbxDouble1(1000.0 / cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH].CurValue));
				} // if
			//KFbxVector4::AxisAlignmentInEulerAngle(lCameraLocation, lDefaultPointOfInterest, lNewPointOfInterest, lRotation);
			lCamera->FocusSource.Set(KFbxCamera::eSPECIFIC_DISTANCE);

			strcpy(camName, cam->Name);
			strcat(camName, "_cam");	// try to generate a unique name
			camNode = KFbxNode::Create(pSdkManager, camName);
			camNode->SetNodeAttribute(lCamera);
			camNode->LclTranslation.Set(lCameraLocation);
			//KFbxVector4::AxisAlignmentInEulerAngle(lCameraLocation, lDefaultPointOfInterest, lNewPointOfInterest, lRotation);	
			camNode->LclRotation.Set(lRotation);
			//camNode->SetDefaultR(lRotation2);
			//camNode->SetGeometricRotation(KFbxNode::eSOURCE_SET, KFbxVector4(45.0, 0.0, 0.0, 0.0));
			//KFbxVector4 kv = KFbxVector4(45.0, 0.0, 0.0, 0.0);
			//xm.SetR(kv);
			//camNode->GlobalTransform.Set(xm);
			//camNode->GeometricRotation.Set(lRotation);
			//camNode->LclScaling.Set(lScaling);
			//camNode->SetDefaultT(lCameraLocation);
			//camNode->SetDefaultR(lRotation);
			//camNode->SetDefaultS(lScaling);
			// assume equivalent to landscape on consumer 35mm film camera for now (ie: 36mm x 24mm)
			// Maya really wants to know _EVERYTHING_ about the film & lens (ie: anamorphics, centerx, centery)
			lCamera->SetFormat(KFbxCamera::eFULL_SCREEN);
			//lCamera->SetFormat(KFbxCamera::ECameraFormat::e640x480);
			lCamera->SetApertureFormat(KFbxCamera::eCUSTOM_APERTURE_FORMAT);
			lCamera->SetApertureMode(KFbxCamera::eHORIZONTAL);
			// film gate size in inches
			double fgs = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FILMSIZE].CurValue / 25.4;	// mm to inches
			lCamera->SetApertureWidth(fgs);
			lCamera->SetApertureHeight(fgs / 1.5);
			lCamera->SetDefaultFocalLength(cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FOCALLENGTH].CurValue);
			//lCamera->SetSqueezeRatio(1.0);
			lCamera->FieldOfView.Set(cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue);
			lCamera->SetShowGrid(false);
			lCamera->DisplaySafeArea.Set(true);
			lCamera->DisplaySafeAreaOnRender.Set(true);

			// check to see if the target is animated
			if (WCS_EFFECTS_CAMERATYPE_TARGETED == cam->CameraType)
				{
				prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
				prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATED;
				prop.ItemOperator = WCS_KEYOPERATION_ALLKEYS;
				prop.TypeNumber = WCS_EFFECTSSUBCLASS_OBJECT3D;
				if (cam->TargetObj)
					{
					cam->TargetObj->GetRAHostProperties(&prop);
					if (prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
						{
						if (targetNode = pScene->GetRootNode()->FindChild(cam->TargetObj->Name))
							{
							// Set the camera to always point at this node.
							camNode->SetTarget(targetNode);
							unaimed = false;
							} // if
						} // if
					} // if
				} // if

			// check to see if this camera is animated
			prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
			prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATED;
			prop.ItemOperator = WCS_KEYOPERATION_ALLKEYS;
			prop.TypeNumber = WCS_EFFECTSSUBCLASS_CAMERA;
			cam->GetRAHostProperties(&prop);
			animated = false;
			if (prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
				animated = true;

			if (unaimed)
				{
				strcpy(targetName, camName);
				strcat(targetName, "_target");
				targetNode = KFbxNode::Create(pSdkManager, targetName);
				switch (cam->CameraType)
					{
					case WCS_EFFECTS_CAMERATYPE_TARGETED:
						if (animated)
							{
							Master->RBounds.DefDegToRBounds(cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].CurValue, cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].CurValue, X, Y);
							lookTo[0] = (X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale;
							lookTo[1] = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].CurValue * Master->ExportRefData.ElevScale - centerCoordZ;
							lookTo[2] = (Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale;
							} // if
						else
							{
							Master->RBounds.DefDegToRBounds(cam->TargPos->Lat, cam->TargPos->Lon, X, Y);
							lookTo[0] = (X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale;
							lookTo[1] = cam->TargPos->Elev * Master->ExportRefData.ElevScale - centerCoordZ;
							lookTo[2] = (Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale;
							} // else
						break;
					case WCS_EFFECTS_CAMERATYPE_UNTARGETED:
						if (animated)
							{
							Master->RBounds.DefDegToRBounds(cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].CurValue, cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].CurValue, X, Y);
							lookTo[0] = (X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale;
							lookTo[1] = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].CurValue * Master->ExportRefData.ElevScale - centerCoordZ;
							lookTo[2] = (Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale;
							} // if
						else
							{
							Master->RBounds.DefDegToRBounds(cam->TargPos->Lat, cam->TargPos->Lon, X, Y);
							lookTo[0] = (X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale;
							lookTo[1] = cam->TargPos->Elev * Master->ExportRefData.ElevScale - centerCoordZ;
							lookTo[2] = (Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale;
							} // else
						break;
					case WCS_EFFECTS_CAMERATYPE_ALIGNED:
						if (animated)
							{
							Master->RBounds.DefDegToRBounds(cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].CurValue, cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].CurValue, X, Y);
							lookTo[0] = (X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale;
							lookTo[1] = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].CurValue * Master->ExportRefData.ElevScale - centerCoordZ;
							lookTo[2] = (Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale;
							} // if
						else
							{
							Master->RBounds.DefDegToRBounds(cam->TargPos->Lat, cam->TargPos->Lon, X, Y);
							lookTo[0] = (X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale;
							lookTo[1] = cam->TargPos->Elev * Master->ExportRefData.ElevScale - centerCoordZ;
							lookTo[2] = (Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale;
							} // else
						break;
					case WCS_EFFECTS_CAMERATYPE_OVERHEAD:
						if (animated)
							{
							Master->RBounds.DefDegToRBounds(cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].CurValue, cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].CurValue, X, Y);
							lookTo[0] = (X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale;
							lookTo[1] = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].CurValue * Master->ExportRefData.ElevScale - centerCoordZ;
							lookTo[2] = (Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale;
							} // if
						else
							{
							Master->RBounds.DefDegToRBounds(cam->TargPos->Lat, cam->TargPos->Lon, X, Y);
							lookTo[0] = (X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale;
							lookTo[1] = cam->TargPos->Elev * Master->ExportRefData.ElevScale - centerCoordZ;
							lookTo[2] = (Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale;
							} // else
						break;
					case WCS_EFFECTS_CAMERATYPE_PLANIMETRIC:
						if (animated)
							{
							Master->RBounds.DefDegToRBounds(cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].CurValue, cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].CurValue, X, Y);
							lookTo[0] = (X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale;
							lookTo[1] = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].CurValue * Master->ExportRefData.ElevScale - centerCoordZ;
							lookTo[2] = (Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale;
							} // if
						else
							{
							Master->RBounds.DefDegToRBounds(cam->TargPos->Lat, cam->TargPos->Lon, X, Y);
							lookTo[0] = (X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale;
							lookTo[1] = cam->TargPos->Elev * Master->ExportRefData.ElevScale - centerCoordZ;
							lookTo[2] = (Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale;
							} // else
						break;
					default:
						break;	// need error routine here
					} // switch
				camNode->LclRotation.Set(fbxDouble3(0.0, 0.0, 0.0));	// undo our values
				lDefaultPointOfInterest = fbxDouble3(lookTo[0], lookTo[1], -lookTo[2]);
				targetNode->LclTranslation.Set(lDefaultPointOfInterest);
				camNode->SetTarget(targetNode);
				success = lCameras->AddChild(targetNode);
				} // if

			// extra stuff for animated cameras
			if (animated)// && curOpt)
				{
				//Point4d AxisAngle;
				//Point3d Orient;
				//double LookAt[3];
				//double LookFrom[3];
				double GeoCtrLat, GeoCtrLon;
				//double keySecs;
				//double keyTime;
				double keyValue;
				//AnimDoubleTime *adt;
				GraphNode *grNode;//, *grNode2;
				KFbxTakeNode* lCurrentTakeNode = NULL;
				KFCurve *lCurve;
				//KFCurve *lCurveCamX, *lCurveCamY, *lCurveCamZ;
				//KFCurve *lCurveCamRotX = NULL, *lCurveCamRotY = NULL, *lCurveCamRotZ = NULL;
				//KFCurve *lCurveCamFOV = NULL;
				kFCurveDouble lcd;
				kFCurveInterpolation iMode = KFCURVE_INTERPOLATION_LINEAR;	// keep Lint happy
				KFCurveKey* lKey = NULL;
				RenderData RendData(NULL);
				int lKeyIndex;
				//long lastKey, maxKeys, n;
				long totalKeys;
				//long curKey;

				GeoCtrLon = Master->ExportRefData.ExportRefLon;
				GeoCtrLat = Master->ExportRefData.ExportRefLat;

#ifdef GORILLA2k9
				RendData.FrameRate = curOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue;
				RendData.Width = curOpt->OutputImageWidth;
				RendData.Height = curOpt->OutputImageHeight;
				RendData.InitToView(EffectsHost, ProjectHost, DBHost, ProjectHost->Interactive, curOpt, cam, RendData.Width, RendData.Width);

				// are there motion (translation, HPB) key frames?
				totalKeys = 0;
				lastKey = -2;
				while ((lastKey = cam->GetNextMotionKeyFrame(lastKey, RendData.FrameRate)) >= 0)
					totalKeys ++;

				//maxKeys = (totalKeys > 0) ? totalKeys: 1;
				// FOV keyframes only cause crash with above line
				maxKeys = 0;
				if (totalKeys > 1)
					maxKeys = totalKeys;
#endif // GORILLA2k9

				camNode->CreateTakeNode(takeName);
				camNode->SetCurrentTakeNode(takeName);
				if (cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetNumNodes(0) ||
					cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetNumNodes(0) ||
					cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetNumNodes(0))
					{
					camNode->LclTranslation.GetKFCurveNode(true, takeName);

					// X translation
					totalKeys = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetNumNodes(0);
					if (totalKeys)
						{
						lCurve = camNode->LclTranslation.GetKFCurve(KFCURVENODE_T_X, takeName);
						if (lCurve)
							{
							lCurve->KeyModifyBegin();

							lKeyIndex = 0;
							grNode = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetFirstNode(0);
							while (grNode)
								{
								keyValue = (-grNode->Value - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale;
								lcd = (kFCurveDouble)keyValue;
								lTime.SetSecondDouble(grNode->Distance);
								lKeyIndex = lCurve->KeyAdd(lTime);
								lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
								grNode = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetNextNode(0, grNode);
								} // while

							lCurve->KeyModifyEnd();
							} // if
						} // if

					// Y translation
					totalKeys = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetNumNodes(0);
					if (totalKeys)
						{
						lCurve = camNode->LclTranslation.GetKFCurve(KFCURVENODE_T_Y, takeName);
						if (lCurve)
							{
							lCurve->KeyModifyBegin();

							lKeyIndex = 0;
							grNode = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetFirstNode(0);
							while (grNode)
								{
								keyValue = grNode->Value * Master->ExportRefData.ElevScale - centerCoordZ;
								lcd = (kFCurveDouble)keyValue;
								lTime.SetSecondDouble(grNode->Distance);
								lKeyIndex = lCurve->KeyAdd(lTime);
								lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
								grNode = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetNextNode(0, grNode);
								} // while

							lCurve->KeyModifyEnd();
							} // if
						} // if

					// Z translation
					totalKeys = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetNumNodes(0);
					if (totalKeys)
						{
						lCurve = camNode->LclTranslation.GetKFCurve(KFCURVENODE_T_Z, takeName);
						if (lCurve)
							{
							lCurve->KeyModifyBegin();

							lKeyIndex = 0;
							grNode = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetFirstNode(0);
							while (grNode)
								{
								keyValue = (grNode->Value - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale;
								lcd = (kFCurveDouble)keyValue;
								lTime.SetSecondDouble(grNode->Distance);
								lKeyIndex = lCurve->KeyAdd(lTime);
								lCurve->KeySet(lKeyIndex, lTime, -lcd, iMode);
								grNode = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetNextNode(0, grNode);
								} // while

							lCurve->KeyModifyEnd();
							} // if
						} // if
					} // if

				if (cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].GetNumNodes(0) ||
					cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].GetNumNodes(0) ||
					cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].GetNumNodes(0))
					{
					camNode->LclRotation.GetKFCurveNode(true, takeName);

					// Camera heading
					totalKeys = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].GetNumNodes(0);
					if (totalKeys)
						{
						lCurve = camNode->LclRotation.GetKFCurve(KFCURVENODE_R_X, takeName);
						if (lCurve)
							{
							lCurve->KeyModifyBegin();

							lKeyIndex = 0;
							grNode = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].GetFirstNode(0);
							while (grNode)
								{
								keyValue = grNode->Value;
								lcd = (kFCurveDouble)keyValue;
								lTime.SetSecondDouble(grNode->Distance);
								lKeyIndex = lCurve->KeyAdd(lTime);
								lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
								grNode = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].GetNextNode(0, grNode);
								} // while

							lCurve->KeyModifyEnd();
							} // if
						} // if

					// Camera pitch
					totalKeys = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].GetNumNodes(0);
					if (totalKeys)
						{
						lCurve = camNode->LclRotation.GetKFCurve(KFCURVENODE_R_Y, takeName);
						if (lCurve)
							{
							lCurve->KeyModifyBegin();

							lKeyIndex = 0;
							grNode = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].GetFirstNode(0);
							while (grNode)
								{
								keyValue = grNode->Value;
								lcd = (kFCurveDouble)keyValue;
								lTime.SetSecondDouble(grNode->Distance);
								lKeyIndex = lCurve->KeyAdd(lTime);
								lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
								grNode = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].GetNextNode(0, grNode);
								} // while

							lCurve->KeyModifyEnd();
							} // if
						} // if

					// Camera bank
					totalKeys = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].GetNumNodes(0);
					if (totalKeys)
						{
						lCurve = camNode->LclRotation.GetKFCurve(KFCURVENODE_R_Z, takeName);
						if (lCurve)
							{
							lCurve->KeyModifyBegin();

							lKeyIndex = 0;
							grNode = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].GetFirstNode(0);
							while (grNode)
								{
								keyValue = grNode->Value;
								lcd = (kFCurveDouble)keyValue;
								lTime.SetSecondDouble(grNode->Distance);
								lKeyIndex = lCurve->KeyAdd(lTime);
								lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
								grNode = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].GetNextNode(0, grNode);
								} // while

							lCurve->KeyModifyEnd();
							} // if
						} // if

					//cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetNumNodes(0);
					//cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].GetNumNodes(0);
					//cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].GetNumNodes(0);

/***
**** This seems like the way to get around the camera aim issues, but Cinema seems to ignore these.  Actually, the roll seems to show
**** up in Cinema, although the UI doesn't show the roll parameter as being animated!  Check that - The camera node doesn't have the
**** animation, but the camera child DOES have the roll animated!
**** Quicktime only seems to use the Roll
					// Camera heading
					totalKeys = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].GetNumNodes(0);
					if (totalKeys)
						{
						KFbxCamera* kcam = camNode->GetCamera();

						kcam->TurnTable.GetKFCurveNode(true, takeName);
						lCurve = kcam->TurnTable.GetKFCurve(NULL, takeName);
						if (lCurve)
							{
							lCurve->KeyModifyBegin();

							lKeyIndex = 0;
							grNode = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].GetFirstNode(0);
							while (grNode)
								{
								keyValue = grNode->Value;
								lTime.SetSecondDouble(grNode->Distance);
								lKeyIndex = lCurve->KeyAdd(lTime);
								lCurve->KeySet(lKeyIndex, lTime, keyValue, iMode);
								grNode = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].GetNextNode(0, grNode);
								} // while

							lCurve->KeyModifyEnd();
							} // if
						} // if

#ifdef FBX_PITCH
					modify camera up vector for this???
					// Camera pitch
					totalKeys = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].GetNumNodes(0);
					if (totalKeys)
						{
						lCamera->???.GetKFCurveNode(true, takeName);
						lCurve = lCamera->???.GetKFCurve(NULL, takeName);
						if (lCurve)
							{
							lCurve->KeyModifyBegin();

							lKeyIndex = 0;
							grNode = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].GetFirstNode(0);
							while (grNode)
								{
								keyValue = grNode->Value;
								lTime.SetSecondDouble(grNode->Distance);
								lKeyIndex = lCurve->KeyAdd(lTime);
								lCurve->KeySet(lKeyIndex, lTime, keyValue, iMode);
								grNode = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].GetNextNode(0, grNode);
								} // while

							lCurve->KeyModifyEnd();
							} // if
						} // if
#endif FBX_PITCH

					// Camera bank
					totalKeys = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].GetNumNodes(0);
					if (totalKeys)
						{
						KFbxCamera* kcam = camNode->GetCamera();

						kcam->Roll.GetKFCurveNode(true, takeName);
						lCurve = kcam->Roll.GetKFCurve(NULL, takeName);
						if (lCurve)
							{
							lCurve->KeyModifyBegin();

							lKeyIndex = 0;
							grNode = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].GetFirstNode(0);
							while (grNode)
								{
								keyValue = grNode->Value;
								lTime.SetSecondDouble(grNode->Distance);
								lKeyIndex = lCurve->KeyAdd(lTime);
								lCurve->KeySet(lKeyIndex, lTime, keyValue, iMode);
								grNode = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].GetNextNode(0, grNode);
								} // while

							lCurve->KeyModifyEnd();
							} // if
						} // if
***/
					} // if

				} // if animated
			success = lCameras->AddChild(camNode);
			} // if CurCam->Me
		} // for CurCam
		success = true;
	} // if cameras

return success;

} // ExportFormatFBX::ExportCameras

/*===========================================================================*/

int ExportFormatFBX::ExportHaze(Atmosphere *CurHaze, long HazeNum)
{

// Maya does fog strange - disabled until further notice
#ifdef GORILLA
if (CurHaze->HazeEnabled)
	{
	pScene->GetGlobalLightSettings().SetFogEnable(true);
	pScene->GetGlobalLightSettings().SetFogStart(CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].CurValue);
	pScene->GetGlobalLightSettings().SetFogDensity(CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY].CurValue);
	pScene->GetGlobalLightSettings().SetFogEnd(CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].CurValue + CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE].CurValue);
//		Atmosphere3ds->fog.fardensity = (float3ds)(CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY].CurValue);
	pScene->GetGlobalLightSettings().SetFogColor(KFbxColor(
		CurHaze->HazeColor.GetClampedCompleteValue(0),
		CurHaze->HazeColor.GetClampedCompleteValue(1),
		CurHaze->HazeColor.GetClampedCompleteValue(2)));
	switch (CurHaze->AtmosphereType)
		{
		default:
		case WCS_EFFECTS_ATMOSPHERETYPE_SIMPLE:
			pScene->GetGlobalLightSettings().SetFogMode(KFbxGlobalLightSettings::eLINEAR);
			break;
		case WCS_EFFECTS_ATMOSPHERETYPE_EXPONENTIAL:
			pScene->GetGlobalLightSettings().SetFogMode(KFbxGlobalLightSettings::eEXPONENTIAL);
			break;
		} // switch AtmosphereType
	} // if
/*
// Ignore WCS/VNS fog for FBX, might have to custom-make a fog or something
if (CurHaze->FogEnabled)
{
	Atmosphere3ds->layerfog.zmin = (float3ds)CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEV].CurValue;
	Atmosphere3ds->layerfog.zmax = (float3ds)CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEV].CurValue;
	Atmosphere3ds->layerfog.density = (float3ds)(CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEVINTENSITY].CurValue);
	Atmosphere3ds->layerfog.fogcolor.r = (float3ds)(CurHaze->FogColor.GetClampedCompleteValue(0));
	Atmosphere3ds->layerfog.fogcolor.r = (float3ds)(CurHaze->FogColor.GetClampedCompleteValue(1));
	Atmosphere3ds->layerfog.fogcolor.r = (float3ds)(CurHaze->FogColor.GetClampedCompleteValue(2));
	Atmosphere3ds->layerfog.falloff = NoFall;
	Atmosphere3ds->layerfog.fogbgnd = False3ds;
} // if
Atmosphere3ds->dcue.nearplane = (float3ds)0.0;
Atmosphere3ds->dcue.farplane = (float3ds)100.0;
Atmosphere3ds->dcue.neardim = (float3ds)0.0;
Atmosphere3ds->dcue.fardim = (float3ds)0.0;
Atmosphere3ds->dcue.dcuebgnd = False3ds;
*/
#endif // GORILLA
return 1;

} // ExportFormatFBX::ExportHaze

/*===========================================================================*/

int ExportFormatFBX::ExportLights(EffectList *CameraList, KFbxNode* &lLights)
{
double pitch, x, y, yaw;
class EffectList *liteList = Master->Lights;
class Light *lite;
KFbxNode* lNode = NULL;
KFbxLight *lLight = NULL;
KFbxTakeNode* lDefaultTakeNode = NULL;
KString lLightName;
KFbxVector4 lLightLocation, lLightTo;
VertexDEM vdem; // for light mucking
RasterAnimHostProperties prop;

if (liteList)
	{
	lLights = KFbxNode::Create(pSdkManager, "Lights");
	} // if

while (liteList)
	{
	fbxDouble3 dbl3;
	bool animated;

	lite = (class Light *)liteList->Me;
	lLight = KFbxLight::Create(pSdkManager, lite->Name);
	lNode = KFbxNode::Create(pSdkManager, lite->Name);
	lNode->SetNodeAttribute(lLight);
	if (lite->Enabled)
		lLight->CastLight.Set(true);
	else
		lLight->CastLight.Set(false);
	lLights->AddChild(lNode);

	dbl3 = fbxDouble3(lite->Color.CurValue[0], lite->Color.CurValue[1], lite->Color.CurValue[2]);
	lLight->Color.Set(dbl3);
	lLight->Intensity.Set(lite->Color.Intensity.CurValue * 100.0);
	// if it's not a distant light, put it somewhere
	//if (!Lite->Distant)
	//	{
		Master->RBounds.DefDegToRBounds(lite->LightPos->Lat, lite->LightPos->Lon, x, y);
		lLightLocation = KFbxVector4(
			(x - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale,
			lite->LightPos->Elev * Master->ExportRefData.ElevScale - centerCoordZ,
			(y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
	//	} // if
	// openflight export, ProcessLights function
	// takes vector and cooks to heading & pitch
	vdem.CopyXYZ(lite->LightAim);
	vdem.Lat = Master->ExportRefData.WCSRefLat;
	vdem.Lon = Master->ExportRefData.WCSRefLon;
	vdem.Elev = Master->ExportRefData.RefElev;
	vdem.RotateToHome();
	lNode->SetDefaultT(KFbxVector4(lLightLocation[0], lLightLocation[1], -lLightLocation[2]));
	//lNode->SetDefaultR(KFbxVector4(-VDEM.FindAngleXfromZ(), -VDEM.FindAngleYfromZ(), 0.0));
	//lNode->SetDefaultR(KFbxVector4(VDEM.FindAngleXfromZ(), VDEM.FindAngleYfromZ(), -VDEM.FindAngleXfromY()));
	yaw = vdem.FindAngleYfromZ();
	vdem.RotateY(-yaw);
	pitch = vdem.FindAngleXfromZ();
	lNode->SetDefaultR(KFbxVector4(90.0 + pitch, 180.0 - yaw, 0.0));
	lNode->SetRotationOrder(KFbxNode::eSOURCE_SET, eEULER_ZXY);
	//lNode->SetGeometricRotation(KFbxNode::eSOURCE_SET, KFbxVector4(-90.0, 0.0, 0.0));
	lNode->SetRotationActive(true);
	//lNode->SetDefaultS(KFbxVector4(1.0, 1.0, 1.0));

	switch (lite->LightType)
		{
		default:
		case WCS_EFFECTS_LIGHTTYPE_PARALLEL:
			lLight->LightType.Set(KFbxLight::eDIRECTIONAL);
			lLight->DecayType.Set(KFbxLight::eNONE);
			break;
		case WCS_EFFECTS_LIGHTTYPE_OMNI:
			lLight->LightType.Set(KFbxLight::ePOINT);
			lLight->DecayType.Set(KFbxLight::eQUADRATIC);
			break;
		case WCS_EFFECTS_LIGHTTYPE_SPOT:
			lLight->LightType.Set(KFbxLight::eSPOT);
			lLight->DecayType.Set(KFbxLight::eQUADRATIC);
			// Spotlight needs cone angle and such
			lLight->ConeAngle.Set(lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].CurValue);
			break;
		} // switch

		// check to see if this object is animated
		prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
		prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATED;
		prop.ItemOperator = WCS_KEYOPERATION_ALLKEYS;
		prop.TypeNumber = WCS_EFFECTSSUBCLASS_LIGHT;
		lite->GetRAHostProperties(&prop);
		animated = false;
		if (prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
			animated = true;

		if (animated)
			{
			double keyValue;
			KFCurve *lCurve;
			kFCurveDouble lcd;
			kFCurveInterpolation iMode = KFCURVE_INTERPOLATION_LINEAR;
			KTime lTime;
			GraphNode *grNode;
			int lKeyIndex;
			long totalKeys;

			lNode->CreateTakeNode(takeName);
			lNode->SetCurrentTakeNode(takeName);
			// translations
			if (lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetNumNodes(0) ||
				lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].GetNumNodes(0) ||
				lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].GetNumNodes(0))
				{
				lNode->LclTranslation.GetKFCurveNode(true, takeName);

				// X translation
				totalKeys = lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].GetNumNodes(0);
				if (totalKeys)
					{
					lCurve = lNode->LclTranslation.GetKFCurve(KFCURVENODE_T_X, takeName);
					if (lCurve)
						{
						lCurve->KeyModifyBegin();

						lKeyIndex = 0;
						grNode = lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].GetFirstNode(0);
						while (grNode)
							{
							lTime.SetSecondDouble(grNode->Distance);
							lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].SetToTime(grNode->Distance);
							lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].SetToTime(grNode->Distance);
							Master->RBounds.DefDegToRBounds(lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue, lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].CurValue, x, y);
							keyValue = (x - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale;
							lcd = (kFCurveDouble)keyValue;
							lKeyIndex = lCurve->KeyAdd(lTime);
							lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
							grNode = lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].GetNextNode(0, grNode);
							} // while

						lCurve->KeyModifyEnd();
						} // if
					} // if

				// Y translation
				totalKeys = lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].GetNumNodes(0);
				if (totalKeys)
					{
					lCurve = lNode->LclTranslation.GetKFCurve(KFCURVENODE_T_Y, takeName);
					if (lCurve)
						{
						lCurve->KeyModifyBegin();

						lKeyIndex = 0;
						grNode = lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].GetFirstNode(0);
						while (grNode)
							{
							lTime.SetSecondDouble(grNode->Distance);
							keyValue = grNode->Value * Master->ExportRefData.ElevScale - centerCoordZ;
							lcd = (kFCurveDouble)keyValue;
							lKeyIndex = lCurve->KeyAdd(lTime);
							lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
							grNode = lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].GetNextNode(0, grNode);
							} // while

						lCurve->KeyModifyEnd();
						} // if
					} // if

				// Z translation
				totalKeys = lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetNumNodes(0);
				if (totalKeys)
					{
					lCurve = lNode->LclTranslation.GetKFCurve(KFCURVENODE_T_Z, takeName);
					if (lCurve)
						{
						lCurve->KeyModifyBegin();

						lKeyIndex = 0;
						grNode = lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetFirstNode(0);
						while (grNode)
							{
							lTime.SetSecondDouble(grNode->Distance);
							lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].SetToTime(grNode->Distance);
							lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].SetToTime(grNode->Distance);
							Master->RBounds.DefDegToRBounds(lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue, lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].CurValue, x, y);
							keyValue = (y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale;
							lcd = (kFCurveDouble)-keyValue;
							lKeyIndex = lCurve->KeyAdd(lTime);
							lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
							grNode = lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetNextNode(0, grNode);
							} // while

						lCurve->KeyModifyEnd();
						} // if
					} // if
				} // if translations

			// rotations
			if (lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].GetNumNodes(0) ||
				lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_PITCH].GetNumNodes(0))
				{
				lNode->LclRotation.GetKFCurveNode(true, takeName);

				// heading
				totalKeys = lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].GetNumNodes(0);
				if (totalKeys)
					{
					lCurve = lNode->LclTranslation.GetKFCurve(KFCURVENODE_R_X, takeName);
					if (lCurve)
						{
						lCurve->KeyModifyBegin();

						lKeyIndex = 0;
						grNode = lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].GetFirstNode(0);
						while (grNode)
							{
							keyValue = grNode->Value;
							lcd = (kFCurveDouble)keyValue;
							lTime.SetSecondDouble(grNode->Distance);
							lKeyIndex = lCurve->KeyAdd(lTime);
							lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
							grNode = lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].GetNextNode(0, grNode);
							} // while

						lCurve->KeyModifyEnd();
						} // if
					} // if

				// pitch
				totalKeys = lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_PITCH].GetNumNodes(0);
				if (totalKeys)
					{
					lCurve = lNode->LclRotation.GetKFCurve(KFCURVENODE_R_Y, takeName);
					if (lCurve)
						{
						lCurve->KeyModifyBegin();

						lKeyIndex = 0;
						grNode = lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_PITCH].GetFirstNode(0);
						while (grNode)
							{
							keyValue = grNode->Value;
							lcd = (kFCurveDouble)keyValue;
							lTime.SetSecondDouble(grNode->Distance);
							lKeyIndex = lCurve->KeyAdd(lTime);
							lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
							grNode = lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_PITCH].GetNextNode(0, grNode);
							} // while

						lCurve->KeyModifyEnd();
						} // if
					} // if
				} // if rotations

			// color
			if (totalKeys = lite->Color.GetNumNodes(0))
				{
				KFCurve* lCurve3[3];

				lLight->Color.GetKFCurveNode(true, takeName);
				lCurve3[0] = lLight->Color.GetKFCurve(KFCURVENODE_COLOR_RED, takeName);
				lCurve3[1] = lLight->Color.GetKFCurve(KFCURVENODE_COLOR_GREEN, takeName);
				lCurve3[2] = lLight->Color.GetKFCurve(KFCURVENODE_COLOR_BLUE, takeName);
				if (lCurve3[0] && lCurve3[1] && lCurve3[2])
					{
					lCurve3[0]->KeyModifyBegin();
					lCurve3[1]->KeyModifyBegin();
					lCurve3[2]->KeyModifyBegin();

					lKeyIndex = 0;
					grNode = lite->Color.GetFirstNode(0);
					while (grNode)
						{
						lTime.SetSecondDouble(grNode->Distance);
						lite->Color.SetToTime(grNode->Distance);

						keyValue = lite->Color.CurValue[0];
						lcd = (kFCurveDouble)keyValue;
						lKeyIndex = lCurve3[0]->KeyAdd(lTime);
						lCurve3[0]->KeySet(lKeyIndex, lTime, lcd, iMode);

						keyValue = lite->Color.CurValue[1];
						lcd = (kFCurveDouble)keyValue;
						lKeyIndex = lCurve3[1]->KeyAdd(lTime);
						lCurve3[1]->KeySet(lKeyIndex, lTime, lcd, iMode);

						keyValue = lite->Color.CurValue[2];
						lcd = (kFCurveDouble)keyValue;
						lKeyIndex = lCurve3[2]->KeyAdd(lTime);
						lCurve3[2]->KeySet(lKeyIndex, lTime, lcd, iMode);

						grNode = lite->Color.GetNextNode(0, grNode);
						} // while

					lCurve3[0]->KeyModifyEnd();
					lCurve3[1]->KeyModifyEnd();
					lCurve3[2]->KeyModifyEnd();
					} // if
				} // if color

			// intensity
			if (totalKeys = lite->Color.Intensity.GetNumNodes(0))
				{
				lLight->Intensity.GetKFCurveNode(true, takeName);
				//lCurve = lLight->Intensity.GetKFCurve(KFCURVENODE_LIGHT_INTENSITY, takeName);
				lCurve = lLight->Intensity.GetKFCurve(NULL, takeName);
				if (lCurve)
					{
					lCurve->KeyModifyBegin();

					lKeyIndex = 0;
					grNode = lite->Color.Intensity.GetFirstNode(0);
					while (grNode)
						{
						keyValue = grNode->Value * 100.0;
						lcd = (kFCurveDouble)keyValue;
						lTime.SetSecondDouble(grNode->Distance);
						lKeyIndex = lCurve->KeyAdd(lTime);
						lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
						grNode = lite->Color.Intensity.GetNextNode(0, grNode);
						} // while

					lCurve->KeyModifyEnd();
					} // if
				} // if intensity

			//WCS_EFFECTS_LIGHT_ANIMPAR_FALLOFFEXP,
			//WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE,		KFCURVENODE_LIGHT_CONEANGLE
			//WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONEEDGE,
			//WCS_EFFECTS_LIGHT_ANIMPAR_LIGHTRADIUS,
			} // if animated
	liteList = liteList->Next;
	} // while

return 1;

} // ExportFormatFBX::ExportLights

/*===========================================================================*/

int ExportFormatFBX::ExportObjects(Object3DInstance *ObjectInstanceList, KFbxNode* &l3DObjects)
{
KFbxLayer* lLayer;
KFbxMesh* objMesh;
KFbxNode* objNode;
KFbxVector4* controlPoints;
Object3DEffect *object3D;
Object3DInstance *curObject = ObjectInstanceList;
unsigned long uniqueness = 0;
int success = 0;
//long wmat3d = 0;	// to generate unique white materials for textures - textures end up being shared when using same material on all objects
long unnamedMat = 0;	// to generate unique material names for each object
bool animated, matIsTexture;
RasterAnimHostProperties prop;

if (l3DObjects = KFbxNode::Create(pSdkManager, "Objects"))
	success = 1;

while (success && curObject)
	{
	double dx, dy, dz, geo[3];
	//ObjectPerVertexMap *UVMap;
	unsigned long index;
	long vertCt;
	char matName[256];

	object3D = curObject->MyObj;
	if ((object3D->Vertices && object3D->Polygons && object3D->NameTable) || object3D->OpenInputFile(NULL, FALSE, FALSE, FALSE))
		{
		KFbxLayerElementMaterial* lMaterialLayer = NULL;
	    KFbxNode* lNode = NULL;
		char *matImageFormat;
		int matnum = 0, texnum = 0;
		long matLimit;
		bool uniqueify;
		char fileExt[8], objName[64];

		// see if it's a 3D foliage object
		if (strncmp(object3D->Name, "FO0", 3) == 0)
			{
			matImageFormat = Master->FoliageImageFormat;
			uniqueify = 1;
			sprintf(objName, "%s%d", object3D->Name, uniqueness++);
			} // if
		else
			{
			matImageFormat = Master->TextureImageFormat;
			uniqueify = 0;
			strcpy(objName, object3D->Name);
			} // else

		// set material file extension
		if (stricmp("JPEG", matImageFormat) == 0)
			strcpy(fileExt, ".jpg");
		else if (stricmp("PNG", matImageFormat) == 0)
			strcpy(fileExt, ".png");
		else if (stricmp("TARGA", matImageFormat) == 0)
			strcpy(fileExt, ".tga");
		else if (stricmp("TIFF", matImageFormat) == 0)
			strcpy(fileExt, ".tif");

		objMesh = KFbxMesh::Create(pSdkManager, objName);
		objMesh->Color.Set(fbxDouble3(1.0, 1.0, 1.0));
		objMesh->InitControlPoints(object3D->NumVertices);
		objMesh->InitNormals();
		controlPoints = objMesh->GetControlPoints();
		lLayer = objMesh->GetLayer(0);
		if (lLayer == NULL)
			{
			objMesh->CreateLayer();
			lLayer = objMesh->GetLayer(0);
			} // if

		// set up control points (vertices)
		index = 0;
		for (vertCt = 0; vertCt < object3D->NumVertices; ++vertCt)
			{
			double xcoord, ycoord, zcoord;

			xcoord = object3D->Vertices[vertCt].xyz[0];
			ycoord = object3D->Vertices[vertCt].xyz[1];
			zcoord = object3D->Vertices[vertCt].xyz[2];
			//controlPoints[index].Set(xcoord + centerCoordX, zcoord - centerCoordZ, ycoord + centerCoordY);
			controlPoints[index].Set(xcoord + 0.0, zcoord - 0.0, ycoord + 0.0);
			++index;
			} // for

		// Set material mapping.
		lMaterialLayer = KFbxLayerElementMaterial::Create(objMesh, "");
		lMaterialLayer->SetMappingMode(KFbxLayerElement::eBY_POLYGON);
		lMaterialLayer->SetReferenceMode(KFbxLayerElement::eINDEX_TO_DIRECT);
		lLayer->SetMaterials(lMaterialLayer);

		matIsTexture = (object3D->VertexUVWAvailable && &object3D->UVWTable[0]);

		// create polygons
		for (long polyCt = 0; polyCt < object3D->NumPolys; ++polyCt)
			{
			int matIndex = object3D->Polygons[polyCt].Material;
			int texIndex;
			int uvIndex;

			if (matIsTexture)// && (matIndex <= matnum))
				{
				texIndex = 0;
				matIndex = 0;
				uvIndex = 0;
				} // if
			else
				{
				texIndex = -1;
				uvIndex = -1;
				} // else
			objMesh->BeginPolygon(matIndex, texIndex, -1);
			for (long i = 0; i < object3D->Polygons[polyCt].NumVerts; i++)
				objMesh->AddPolygon(object3D->Polygons[polyCt].VertRef[i], uvIndex);	// polygon vertex index, index to texture UV coords
			objMesh->EndPolygon();
			lMaterialLayer->GetIndexArray().Add(matIndex);
			} // for

		objMesh->ComputeVertexNormals(false);

		// compute translation
		for (unsigned long j = 0; j < 3; j++)
			{
			geo[j] = curObject->ExportXYZ[j];
			} // for

		// convert geographic position to meters & our repositioning
		dx = geo[0] * Master->ExportRefData.ExportLonScale;
		dy = geo[2] - ((Master->ExportRefData.MaxElev - Master->ExportRefData.RefElev) * 0.5);
		dz = -geo[1] * Master->ExportRefData.ExportLatScale;

		objNode = KFbxNode::Create(pSdkManager, objName);
		objNode->SetDefaultT(KFbxVector4(dx, dy, dz));
		//objNode->SetDefaultR(KFbxVector4(-(curObject->Euler[0] + 90.0), -curObject->Euler[1], curObject->Euler[2]));
		objNode->SetGeometricRotation(KFbxNode::eSOURCE_SET, KFbxVector4(-90.0, 0.0, 0.0));
		objNode->SetDefaultR(KFbxVector4(-curObject->Euler[0], -curObject->Euler[1], curObject->Euler[2]));
		objNode->SetDefaultS(KFbxVector4(curObject->Scale[0], curObject->Scale[1], curObject->Scale[2]));
		objNode->SetRotationOrder(KFbxNode::eSOURCE_SET, eEULER_ZXY);
		objNode->SetRotationActive(true);
		objNode->SetNodeAttribute(objMesh);

		lNode = objMesh->GetNode();

		// set up materials and textures
		// if textures exist, we're just going to use the rendered materials in the image
		if (matIsTexture)
			matLimit = 1;
		else
			matLimit = object3D->NumMaterials;
		for (long i = 0; i < matLimit; i++)
			{
			KString lMaterialName;
			MaterialEffect* mat;

			if (! object3D->NameTable[i].Mat)
				object3D->NameTable[i].Mat = (MaterialEffect *)GlobalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, object3D->NameTable[i].Name);

			if (mat = object3D->NameTable[i].Mat)
				{
				lMaterialName = mat->Name;
				} // if
			else
				{
				sprintf(matName, "Material%05d", unnamedMat++);	// safety - don't think we'll ever hit this
				lMaterialName = mat->Name;
				} // else

			if (matIsTexture)
				{
				KFbxLayerElementTexture* lTextureDiffuseLayer;
				KFbxLayerElementUV* lUVDiffuseLayer;
				KFbxTexture* lTexture;
				const char *textureFile = NULL;
				bool status;
				//char fullFileName[1024], fullFileName2[1024];

				// Set texture mapping for diffuse channel.
				lTextureDiffuseLayer = KFbxLayerElementTexture::Create(objMesh, "Diffuse Texture");
				lTextureDiffuseLayer->SetMappingMode(KFbxLayerElement::eALL_SAME);
				lTextureDiffuseLayer->SetReferenceMode(KFbxLayerElement::eINDEX_TO_DIRECT);
				lTextureDiffuseLayer->GetIndexArray().Add(0);
				lLayer->SetDiffuseTextures(lTextureDiffuseLayer);

				// Create UV for Diffuse channel.
				lUVDiffuseLayer = KFbxLayerElementUV::Create(objMesh, "DiffuseUV");
				lUVDiffuseLayer->SetMappingMode(KFbxLayerElement::eBY_CONTROL_POINT);
				lUVDiffuseLayer->SetReferenceMode(KFbxLayerElement::eDIRECT);
				lLayer->SetUVs(lUVDiffuseLayer, KFbxLayerElement::eDIFFUSE_TEXTURES);

				objNode->SetShadingMode(KFbxNode::eTEXTURE_SHADING);
				objNode->SetMultiLayer(false);

				texIndex++;
				// Set texture properties.
				lTexture = KFbxTexture::Create(pSdkManager, lMaterialName);
				lMaterialName += fileExt;
				//strmfp(fullFileName2, Master->OutPath.GetPath(), lMaterialName);
				//strcpy(fullFileName, GlobalApp->MainProj->MungPath(fullFileName2));
				//status = lTexture->SetFileName(fullFileName);
				status = lTexture->SetFileName(lMaterialName);
				//status = lTexture->SetRelativeFileName(lMaterialName.Buffer());
				lTexture->SetTextureUse(KFbxTexture::eSTANDARD);
				lTexture->SetMappingType(KFbxTexture::eUV);
				lTexture->SetMaterialUse(KFbxTexture::eMODEL_MATERIAL);
				lTexture->SetSwapUV(false);
				lTexture->UseMaterial.Set(fbxBool1(false));

				objMesh->GetLayer(0)->GetDiffuseTextures()->GetDirectArray().Add(lTexture);
				} // if
			else
				{
				fbxDouble3 tempd3;
				KString lShadingName;

				if (mat->Shading == 1) // flat
					{
					KFbxSurfaceLambert* lLambert;

					if (lNode)
						{
						lShadingName = "Lambert";
						lLambert = KFbxSurfaceLambert::Create(pSdkManager, lMaterialName.Buffer());
						tempd3 = fbxDouble3(
							mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].CurValue * mat->DiffuseColor.CurValue[0],
							mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].CurValue * mat->DiffuseColor.CurValue[1],
							mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].CurValue * mat->DiffuseColor.CurValue[2]);
						lLambert->GetEmissiveColor().Set(tempd3);
						tempd3 = fbxDouble3(0.0, 0.0, 0.0);
						lLambert->GetAmbientColor().Set(tempd3);
						if (mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].CurValue != 0.0)
							tempd3 = fbxDouble3(0.0, 0.0, 0.0);
						else
							tempd3 = fbxDouble3(mat->DiffuseColor.CurValue[0], mat->DiffuseColor.CurValue[1], mat->DiffuseColor.CurValue[2]);
						lLambert->GetDiffuseColor().Set(tempd3);
						lLambert->GetTransparencyFactor().Set(mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
						lLambert->GetShadingModel().Set(lShadingName);
						lNode->AddMaterial(lLambert);
						} // if
					} // if
				else if (mat->Shading == 2) // phong
					{
					KFbxSurfacePhong* lPhong;

					if (lNode)
						{
						lShadingName = "Phong";
						lPhong = KFbxSurfacePhong::Create(pSdkManager, lMaterialName.Buffer());
						tempd3 = fbxDouble3(mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].CurValue,
							mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].CurValue,
							mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].CurValue);
						lPhong->GetEmissiveColor().Set(tempd3);
						tempd3 = fbxDouble3(0.0, 0.0, 0.0);
						lPhong->GetAmbientColor().Set(tempd3);
						tempd3 = fbxDouble3(mat->DiffuseColor.CurValue[0], mat->DiffuseColor.CurValue[1], mat->DiffuseColor.CurValue[2]);
						lPhong->GetDiffuseColor().Set(tempd3);
						lPhong->GetTransparencyFactor().Set(mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
						lPhong->GetShininess().Set(mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY].CurValue);
						tempd3 = fbxDouble3(mat->SpecularColor.CurValue[0], mat->SpecularColor.CurValue[1], mat->SpecularColor.CurValue[2]);
						lPhong->GetSpecularColor().Set(tempd3);
						lPhong->GetSpecularFactor().Set(mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].CurValue);
						lPhong->GetShadingModel().Set(lShadingName);
						lNode->AddMaterial(lPhong);
						} // if
					} // else
				} // else

			// set up textures
			if (matIsTexture)
				{
				double u, v;
				KFbxLayerElementUV* lUVDiffuseLayer;
				ObjectPerVertexMap *UVMap = NULL;

				// Create UV for Diffuse channel.
				lUVDiffuseLayer = KFbxLayerElementUV::Create(objMesh, "DiffuseUV");
				lUVDiffuseLayer->SetMappingMode(KFbxLayerElement::eBY_CONTROL_POINT);
				lUVDiffuseLayer->SetReferenceMode(KFbxLayerElement::eDIRECT);
				lLayer->SetUVs(lUVDiffuseLayer, KFbxLayerElement::eDIFFUSE_TEXTURES);

				UVMap = &object3D->UVWTable[0];
				for (vertCt = 0; vertCt < object3D->NumVertices; ++vertCt)
					{
					if (UVMap->CoordsValid[vertCt])
						{
						u = UVMap->CoordsArray[0][vertCt];
						v = UVMap->CoordsArray[1][vertCt];
						} // if CoordsValid
					else
						v = u = 0.0;
					lUVDiffuseLayer->GetDirectArray().Add(KFbxVector2(u, v));
					} // for VertCt
				} // if material is texture
			} // for

		// check to see if this object is animated
		prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
		prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATED;
		prop.ItemOperator = WCS_KEYOPERATION_ALLKEYS;
		prop.TypeNumber = WCS_EFFECTSSUBCLASS_OBJECT3D;
		object3D->GetRAHostProperties(&prop);
		animated = false;
		if (prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
			animated = true;

		if (animated)
			{
			double keyValue;
			KFCurve *lCurve;
			kFCurveDouble lcd;
			kFCurveInterpolation iMode = KFCURVE_INTERPOLATION_LINEAR;
			KTime lTime;
			GraphNode *grNode;
			int lKeyIndex;
			long totalKeys;

			objNode->CreateTakeNode(takeName);
			objNode->SetCurrentTakeNode(takeName);
			// translations
			if (object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONX].GetNumNodes(0) ||
				object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONY].GetNumNodes(0) ||
				object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONZ].GetNumNodes(0))
				{
				objNode->LclTranslation.GetKFCurveNode(true, takeName);

				// X translation
				totalKeys = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONX].GetNumNodes(0);
				if (totalKeys)
					{
					lCurve = objNode->LclTranslation.GetKFCurve(KFCURVENODE_T_X, takeName);
					if (lCurve)
						{
						lCurve->KeyModifyBegin();

						lKeyIndex = 0;
						grNode = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONX].GetFirstNode(0);
						while (grNode)
							{
							keyValue = grNode->Value + dx;
							lcd = (kFCurveDouble)keyValue;
							lTime.SetSecondDouble(grNode->Distance);
							lKeyIndex = lCurve->KeyAdd(lTime);
							lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
							grNode = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONX].GetNextNode(0, grNode);
							} // while

						lCurve->KeyModifyEnd();
						} // if
					} // if

				// Y translation
				totalKeys = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONY].GetNumNodes(0);
				if (totalKeys)
					{
					lCurve = objNode->LclTranslation.GetKFCurve(KFCURVENODE_T_Y, takeName);
					if (lCurve)
						{
						lCurve->KeyModifyBegin();

						lKeyIndex = 0;
						grNode = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONY].GetFirstNode(0);
						while (grNode)
							{
							keyValue = grNode->Value + dy;
							lcd = (kFCurveDouble)keyValue;
							lTime.SetSecondDouble(grNode->Distance);
							lKeyIndex = lCurve->KeyAdd(lTime);
							lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
							grNode = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONY].GetNextNode(0, grNode);
							} // while

						lCurve->KeyModifyEnd();
						} // if
					} // if

				// Z translation
				totalKeys = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONZ].GetNumNodes(0);
				if (totalKeys)
					{
					lCurve = objNode->LclTranslation.GetKFCurve(KFCURVENODE_T_Z, takeName);
					if (lCurve)
						{
						lCurve->KeyModifyBegin();

						lKeyIndex = 0;
						grNode = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONZ].GetFirstNode(0);
						while (grNode)
							{
							keyValue = -grNode->Value + dz;
							lcd = (kFCurveDouble)keyValue;
							lTime.SetSecondDouble(grNode->Distance);
							lKeyIndex = lCurve->KeyAdd(lTime);
							lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
							grNode = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONZ].GetNextNode(0, grNode);
							} // while

						lCurve->KeyModifyEnd();
						} // if
					} // if
				} // if

			// rotations
			if (object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX].GetNumNodes(0) ||
				object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY].GetNumNodes(0) ||
				object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ].GetNumNodes(0))
				{
				objNode->LclRotation.GetKFCurveNode(true, takeName);

				// X rotation
				totalKeys = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX].GetNumNodes(0);
				if (totalKeys)
					{
					lCurve = objNode->LclRotation.GetKFCurve(KFCURVENODE_R_X, takeName);
					if (lCurve)
						{
						lCurve->KeyModifyBegin();

						lKeyIndex = 0;
						grNode = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX].GetFirstNode(0);
						while (grNode)
							{
							keyValue = -grNode->Value;
							lcd = (kFCurveDouble)keyValue;
							lTime.SetSecondDouble(grNode->Distance);
							lKeyIndex = lCurve->KeyAdd(lTime);
							lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
							grNode = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX].GetNextNode(0, grNode);
							} // while

						lCurve->KeyModifyEnd();
						} // if
					} // if

				// Y rotation
				totalKeys = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY].GetNumNodes(0);
				if (totalKeys)
					{
					lCurve = objNode->LclRotation.GetKFCurve(KFCURVENODE_R_Y, takeName);
					if (lCurve)
						{
						lCurve->KeyModifyBegin();

						lKeyIndex = 0;
						grNode = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY].GetFirstNode(0);
						while (grNode)
							{
							keyValue = -grNode->Value;
							lcd = (kFCurveDouble)keyValue;
							lTime.SetSecondDouble(grNode->Distance);
							lKeyIndex = lCurve->KeyAdd(lTime);
							lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
							grNode = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY].GetNextNode(0, grNode);
							} // while

						lCurve->KeyModifyEnd();
						} // if
					} // if

				// Z rotation
				totalKeys = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ].GetNumNodes(0);
				if (totalKeys)
					{
					lCurve = objNode->LclRotation.GetKFCurve(KFCURVENODE_R_Z, takeName);
					if (lCurve)
						{
						lCurve->KeyModifyBegin();

						lKeyIndex = 0;
						grNode = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ].GetFirstNode(0);
						while (grNode)
							{
							keyValue = grNode->Value;
							lcd = (kFCurveDouble)keyValue;
							lTime.SetSecondDouble(grNode->Distance);
							lKeyIndex = lCurve->KeyAdd(lTime);
							lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
							grNode = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ].GetNextNode(0, grNode);
							} // while

						lCurve->KeyModifyEnd();
						} // if
					} // if
				} // if

			// scaling
			if (object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].GetNumNodes(0) ||
				object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].GetNumNodes(0) ||
				object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].GetNumNodes(0))
				{
				objNode->LclScaling.GetKFCurveNode(true, takeName);

				// X scaling
				totalKeys = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].GetNumNodes(0);
				if (totalKeys)
					{
					lCurve = objNode->LclScaling.GetKFCurve(KFCURVENODE_S_X, takeName);
					if (lCurve)
						{
						lCurve->KeyModifyBegin();

						lKeyIndex = 0;
						grNode = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].GetFirstNode(0);
						while (grNode)
							{
							keyValue = grNode->Value;
							lcd = (kFCurveDouble)keyValue;
							lTime.SetSecondDouble(grNode->Distance);
							lKeyIndex = lCurve->KeyAdd(lTime);
							lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
							grNode = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].GetNextNode(0, grNode);
							} // while

						lCurve->KeyModifyEnd();
						} // if
					} // if

				// Y scaling
				totalKeys = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].GetNumNodes(0);
				if (totalKeys)
					{
					lCurve = objNode->LclScaling.GetKFCurve(KFCURVENODE_S_Y, takeName);
					if (lCurve)
						{
						lCurve->KeyModifyBegin();

						lKeyIndex = 0;
						grNode = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].GetFirstNode(0);
						while (grNode)
							{
							keyValue = grNode->Value;
							lcd = (kFCurveDouble)keyValue;
							lTime.SetSecondDouble(grNode->Distance);
							lKeyIndex = lCurve->KeyAdd(lTime);
							lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
							grNode = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].GetNextNode(0, grNode);
							} // while

						lCurve->KeyModifyEnd();
						} // if
					} // if

				// Z scaling
				totalKeys = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].GetNumNodes(0);
				if (totalKeys)
					{
					lCurve = objNode->LclScaling.GetKFCurve(KFCURVENODE_S_Z, takeName);
					if (lCurve)
						{
						lCurve->KeyModifyBegin();

						lKeyIndex = 0;
						grNode = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].GetFirstNode(0);
						while (grNode)
							{
							keyValue = grNode->Value;
							lcd = (kFCurveDouble)keyValue;
							lTime.SetSecondDouble(grNode->Distance);
							lKeyIndex = lCurve->KeyAdd(lTime);
							lCurve->KeySet(lKeyIndex, lTime, lcd, iMode);
							grNode = object3D->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].GetNextNode(0, grNode);
							} // while

						lCurve->KeyModifyEnd();
						} // if
					} // if
				} // if
			} // if
		l3DObjects->AddChild(objNode);
		} // if

	curObject = curObject->Next;
	} // while ObjectList

return success;

} // ExportFormatFBX::ExportObjects

/*===========================================================================*/

bool ExportFormatFBX::SaveScene(void)
{
class SXExtensionFBX *SXExtFBX = (class SXExtensionFBX *)Master->FormatExtension;
char *lFormatName;
int lFormatIndex, lFormatCount = pSdkManager->GetIOPluginRegistry()->GetWriterFormatCount();
int pFileFormat;
bool lStatus = true;
char logMsg[64];

pFileFormat = pSdkManager->GetIOPluginRegistry()->GetNativeWriterFormat();

if (SXExtFBX->UsePassword)
	{
	lFormatName = "FBX encrypted";
	} // if
else if (SXExtFBX->SaveV5)
	{
	if (SXExtFBX->SaveBinary)
		{
		lFormatName = "FBX 5.0 binary";
		} // if
	else
		{
		lFormatName = "FBX 5.0 ascii";
		} // else
	} // else if
else if (SXExtFBX->SaveBinary)
	{
	lFormatName = "FBX binary";
	} // else if
else
	{
	lFormatName = "FBX ascii";
	} // else

for (lFormatIndex = 0; lFormatIndex < lFormatCount; lFormatIndex++)
	{
	if (pSdkManager->GetIOPluginRegistry()->WriterIsFBX(lFormatIndex))
		{
		KString lDesc = pSdkManager->GetIOPluginRegistry()->GetWriterFormatDescription(lFormatIndex);

		if (lDesc.Find(lFormatName) >= 0)
			{
			pFileFormat = lFormatIndex;
			break;
			} // if
		} // if
	} // for

// Set the file format
fbxExporter->SetFileFormat(pFileFormat);

KFbxStreamOptionsFbxWriter* lExportOptions = KFbxStreamOptionsFbxWriter::Create(pSdkManager, "ExportOptions");
if (pSdkManager->GetIOPluginRegistry()->WriterIsFBX(pFileFormat))
	{
	lExportOptions->SetOption(KFBXSTREAMOPT_FBX_MATERIAL, true);
	lExportOptions->SetOption(KFBXSTREAMOPT_FBX_TEXTURE, true);
	lExportOptions->SetOption(KFBXSTREAMOPT_FBX_EMBEDDED, 1 == SXExtFBX->EmbedMedia);
	lExportOptions->SetOption(KFBXSTREAMOPT_FBX_LINK, false);
	lExportOptions->SetOption(KFBXSTREAMOPT_FBX_SHAPE, false);
	lExportOptions->SetOption(KFBXSTREAMOPT_FBX_GOBO, false);
	lExportOptions->SetOption(KFBXSTREAMOPT_FBX_ANIMATION, true);
	lExportOptions->SetOption(KFBXSTREAMOPT_FBX_GLOBAL_SETTINGS, true);
	if (SXExtFBX->UsePassword)
		{
		lExportOptions->SetOption(KFBXSTREAMOPT_FBX_PASSWORD_ENABLE, true);
		lExportOptions->SetOption(KFBXSTREAMOPT_FBX_PASSWORD, SXExtFBX->Password);
		} // if
	} // if

// Export the scene.
lStatus = fbxExporter->Export(pScene, lExportOptions);
if (lStatus)
	{
	sprintf(logMsg, "FBX Export Succeeded.\n");
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, logMsg);
	} // if
else
	{
	sprintf(logMsg, "FBX SDK Error:\n");
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, logMsg);
	strncpy(logMsg, fbxExporter->GetLastErrorString(), sizeof(logMsg - 1));
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, logMsg);
	} // else

if (lExportOptions)
	lExportOptions->Destroy();
lExportOptions = NULL;
fbxExporter->Destroy();

return lStatus;

} // ExportFormatFBX::SaveScene

/*===========================================================================*/

bool ExportFormatFBX::DoSceneInfo(void)
{
bool success = false;
KString kStr;

// create scene info
KFbxDocumentInfo* docInfo = KFbxDocumentInfo::Create(pSdkManager, "Scene Info");
docInfo->Original_ApplicationVendor = KString("3D Nature");
docInfo->Original_ApplicationName = KString("Scene Express in ") + KString(APP_TLA);
docInfo->Original_ApplicationVersion = KString(APP_VERS);
kStr = GlobalApp->MainProj->projectname;
kStr = kStr.Left(kStr.GetLen() - 5);
docInfo->mTitle = kStr;
docInfo->mAuthor = GlobalApp->MainProj->UserName;
kStr = KString("Generated by 3D Nature's Scene Express in ") + KString(APP_TLA) + KString(" ") + KString(APP_VERS);
docInfo->mComment =	kStr;

// we need to add the sceneInfo before calling AddThumbNailToScene because
// that function is asking the scene for the sceneInfo.
pScene->SetSceneInfo(docInfo);

KFbxThumbnail* lThumbnail = KFbxThumbnail::Create(pSdkManager, "");

lThumbnail->SetDataFormat(KFbxThumbnail::eRGB_24);
lThumbnail->SetSize(KFbxThumbnail::e64x64);
lThumbnail->SetThumbnailImage(cSceneThumbnail);

if (pScene->GetSceneInfo())
	{
	pScene->GetSceneInfo()->SetSceneThumbnail(lThumbnail);
	success = true;
	} // if

return success;

} // ExportFormatFBX::DoSceneInfo

/*===========================================================================*/

bool ExportFormatFBX::SetGlobalSettings(void)
{
//KFbxAxisSystem* lAxSys;	// useful to actually see what's happening
KFbxGlobalSettings* lGSet;

lGSet = &pScene->GetGlobalSettings();
lGSet->SetAxisSystem(KFbxAxisSystem::eMayaYUp);	// this is the current default system, but set it just in case it changes
lGSet->SetSystemUnit(KFbxSystemUnit(100.0));	// set system unit as 1 meter in size (100 cm)
//lAxSys = &lGSet->GetAxisSystem();

return true;

} // ExportFormatFBX::SetGlobalSettings

/*===========================================================================*/

//#undef K_PLUGIN
//#undef K_FBXSDK
//#undef K_NODLL

#endif // WCS_BUILD_FBX
