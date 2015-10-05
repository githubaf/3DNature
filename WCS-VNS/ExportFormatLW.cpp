// ExportFormatLW.cpp
// Code module for Lightwave export code
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
#include "Lists.h"

// also defined in LWSupport.cpp
#define WCS_LW7_DATA_NUMCHANNELS	14

ExportFormatLW::ExportFormatLW(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource)
: ExportFormat(MasterSource, ProjectSource, EffectsSource, DBSource, ImageSource)
{

SceneFile = NULL;

} // ExportFormatLW::ExportFormatLW

/*===========================================================================*/

ExportFormatLW::~ExportFormatLW()
{

} // ExportFormatLW::~ExportFormatLW

/*===========================================================================*/

int ExportFormatLW::PackageExport(NameList **FileNamesCreated)
{
long ObjectsWritten = 0, FoliageTargetNum;
Atmosphere *CurHaze;
FormatSpecificFile CurFile;
const char *IndexFileName;
int FileType, HVSpritesExist, HDInstancesExist, Success = 1;
char FullFileName[512];

// make directories
// Scenes
strmfp(FullFileName, Master->OutPath.GetPath(), "Scenes");
// create directory if necessary
if (PROJ_chdir(FullFileName))
	PROJ_mkdir(FullFileName);

// Objects
strmfp(FullFileName, Master->OutPath.GetPath(), "Objects");
// create directory if necessary
if (PROJ_chdir(FullFileName))
	PROJ_mkdir(FullFileName);

strcat(FullFileName, "/");
strcat(FullFileName, Master->OutPath.GetName());
// create directory if necessary
if (PROJ_chdir(FullFileName))
	PROJ_mkdir(FullFileName);

// Images
strmfp(FullFileName, Master->OutPath.GetPath(), "Images");
// create directory if necessary
if (PROJ_chdir(FullFileName))
	PROJ_mkdir(FullFileName);

strcat(FullFileName, "/");
strcat(FullFileName, Master->OutPath.GetName());
// create directory if necessary
if (PROJ_chdir(FullFileName))
	PROJ_mkdir(FullFileName);

// open project file
if (CurFile = OpenSceneFile(Master->OutPath.GetPath(), Master->OutPath.GetName()))
	{
	// needed later
	if (Master->ExportHaze && Master->Haze)
		CurHaze = (Atmosphere *)Master->Haze->Me;
	else
		CurHaze = NULL;

	// scene - frame ranges
	HVSpritesExist = CheckHVSpritesExist();
	HDInstancesExist = CheckHDInstancesExist();
	Success = ExportSceneDetail(CurFile, Master->ExportCameras ? Master->Cameras: NULL, HVSpritesExist);

	// targeted cameras need NULL objects
	if (Success)
		Success = ExportCameraTargets(CurFile, Master->Cameras, ObjectsWritten);

	// flipboard foliage needs a target null parented to the first camera, same with HDI foliage
	if (Success)
		Success = ExportFoliageTarget(CurFile, ObjectsWritten, HDInstancesExist);
	FoliageTargetNum = ObjectsWritten - 1;

	// each object with 9 channels of object motion envelopes
	// export objects (terrain, sky, walls, 3d objects, foliage, vectors)
	if (Success)
		Success = SaveAllObjects(TRUE, CurFile, FoliageTargetNum, ObjectsWritten, FileNamesCreated);	// TRUE=OneFile

	// export lights
	if (Success)
		Success = ExportLights(CurFile, Master->Lights, CurHaze);

	// export cameras
	if (Success)
		Success = ExportCameras(CurFile, Master->Cameras);

	// other effects incl haze, sky, fog, backdrop
	// export haze
	if (Success)
		{
		Success = ExportSceneEffects(CurFile, CurHaze, HVSpritesExist, HDInstancesExist);
		} // if

	// rendering and interface
	if (Success)
		Success = ExportRenderInterface(CurFile, Master->ExportCameras ? Master->Cameras: NULL);

	if (! CloseFile(CurFile))
		Success = 0;
	} // if
else
	Success = 0;

// remove any unused files
FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILEIDX;
if (*FileNamesCreated && (IndexFileName = (*FileNamesCreated)->FindNameOfType(FileType)))
	{
	strmfp(FullFileName, Master->OutPath.GetPath(), IndexFileName);
	PROJ_remove(FullFileName);
	} // if
FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILE;
if (*FileNamesCreated && (IndexFileName = (*FileNamesCreated)->FindNameOfType(FileType)))
	{
	strmfp(FullFileName, Master->OutPath.GetPath(), IndexFileName);
	PROJ_remove(FullFileName);
	} // if

return (Success);

} // ExportFormatLW::PackageExport

/*===========================================================================*/

FormatSpecificFile ExportFormatLW::OpenSceneFile(const char *PathName, const char *FileName)
{
char FullFileName[512];

// copy file name
strmfp(FullFileName, PathName, "Scenes");
// create directory if necessary
if (PROJ_chdir(FullFileName))
	PROJ_mkdir(FullFileName);

// copy file name
strcat(FullFileName, "/");
strcat(FullFileName, FileName);
// append extension
strcat(FullFileName, ".lws");

if (SceneFile = PROJ_fopen(FullFileName, "w"))
	{
	fprintf(SceneFile, "LWSC\n3\n\n");
	} // if

return (SceneFile);

} // ExportFormatLW::OpenSceneFile

/*===========================================================================*/

int ExportFormatLW::CloseFile(FormatSpecificFile fFile)
{

fclose((FILE *)fFile);

return (1);

} // ExportFormatLW::CloseFile

/*===========================================================================*/

int ExportFormatLW::CheckHVSpritesExist(void)
{
Object3DInstance *CurInstance = Master->ObjectInstanceList;

while (CurInstance)
	{
	if (CurInstance->Flags & WCS_OBJECT3DINSTANCE_FLAGBIT_HVSPRITE)
		{
		return (1);
		}
	CurInstance = CurInstance->Next;
	} // while

return (0);

} // ExportFormatLW::CheckHVSpritesExist

/*===========================================================================*/

int ExportFormatLW::CheckHDInstancesExist(void)
{
Object3DInstance *CurInstance = Master->ObjectInstanceList;

while (CurInstance)
	{
	if (CurInstance->Flags & WCS_OBJECT3DINSTANCE_FLAGBIT_HDINSTANCE)
		{
		return (1);
		}
	CurInstance = CurInstance->Next;
	} // while

return (0);

} // ExportFormatLW::CheckHDInstancesExist

/*===========================================================================*/

int ExportFormatLW::ExportSceneDetail(FormatSpecificFile CurFile, EffectList *CameraList, int HVSpritesExist)
{
double DefaultFrameRate, FrameRate, FrameStep, HighFrame, LowFrame;
long PreviewHighFrame;
FILE *LWFile = (FILE *)CurFile;
RenderJob *CurRJ;
RenderOpt *CurOpt;
EffectList *CurCam;
int MatchFound;
RasterAnimHostProperties Prop;

// search the camera list for render jobs associated with the cameras and the associated render options
// find the maximum frame range and the minimum frame step

MatchFound = 0;
PreviewHighFrame = 0;
DefaultFrameRate = GlobalApp->MainProj->Interactive->GetFrameRate();

Prop.PropMask = WCS_RAHOST_MASKBIT_KEYRANGE;
Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;

// if cameras
if (CameraList)
	{
	HighFrame = -FLT_MAX;
	LowFrame = FLT_MAX;
	FrameRate = -FLT_MAX;
	FrameStep = FLT_MAX;
	// for each camera
	for (CurCam = CameraList; CurCam; CurCam = CurCam->Next)
		{
		if (CurCam->Me)
			{
			// for each render job, does the camera match
			for (CurRJ = (RenderJob *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_RENDERJOB); CurRJ; CurRJ = (RenderJob *)CurRJ->Next)
				{
				if (CurRJ->Cam == (Camera *)CurCam->Me)
					{
					// what is the matching render options
					if (CurOpt = CurRJ->Options)
						{
						// is the frame range larger than the largest found so far
						if (CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].CurValue * CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue > HighFrame)
							HighFrame = CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].CurValue * CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue;
						if (CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].CurValue * CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue < LowFrame)
							LowFrame = CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].CurValue * CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue;
						// is the frame rate higher
						if (CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue > FrameRate)
							FrameRate = CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue;
						// is the frame step lower
						if (CurOpt->FrameStep < FrameStep)
							FrameStep = CurOpt->FrameStep;
						MatchFound = 1;
						} // if
					} // if
				} // for
			// for preview anim length look to the camera
			((Camera *)CurCam->Me)->GetRAHostProperties(&Prop);
			if (quickftol(Prop.KeyNodeRange[1] * DefaultFrameRate + .5) > PreviewHighFrame)
				PreviewHighFrame = quickftol(Prop.KeyNodeRange[1] * DefaultFrameRate + .5);
			} // if
		} // for CurCam
	} // if cameras

// if no cameras, choose defaults
if (! MatchFound)
	{
	HighFrame = 30.0;
	LowFrame = 0.0;
	FrameRate = GlobalApp->MainProj->Interactive->GetFrameRate();
	FrameStep = 1.0;
	} // else

// write data
fprintf(LWFile, "FirstFrame %d\n", quickftol(WCS_round(LowFrame)));
fprintf(LWFile, "LastFrame %d\n", quickftol(WCS_round(HighFrame)));
fprintf(LWFile, "FrameStep %d\n", quickftol(WCS_round(FrameStep)));
fprintf(LWFile, "PreviewFirstFrame %d\n", 0);
fprintf(LWFile, "PreviewLastFrame %d\n", PreviewHighFrame);
fprintf(LWFile, "PreviewFrameStep %d\n", quickftol(WCS_round(FrameStep)));
fprintf(LWFile, "CurrentFrame %d\n", quickftol(WCS_round(Master->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue * FrameRate)));
fprintf(LWFile, "FramesPerSecond %d\n", quickftol(WCS_round(FrameRate)));

if (HVSpritesExist)
	{
	fprintf(LWFile, "\
Plugin MasterHandler 1 .HyperVoxels\n\
EndPlugin\n");
	} // if

fprintf(LWFile, "\n");

return (1);

} // ExportFormatLW::ExportSceneDetail

/*===========================================================================*/

int ExportFormatLW::SaveAllObjects(int OneFile, FormatSpecificFile CurFile, long TargetObjNum, long &ObjectsWritten, NameList **FileNamesCreated)
{
long TotalInstances, ObjsDone, SaveIt, ObjCt, ObjectNum, NumLights, HDInstanceCt = 0, HDSources = 0;
long *HDInstanceList = NULL;
Object3DInstance *CurInstance;
Object3DEffect *CurObj, **ObjectsDoneList = NULL;
EffectList *CurLight;
char *VertMapName = NULL, *FullImageName = NULL;
FormatSpecificFile ObjectFile;
int Success = 1, ExporterMadeObject;
char TempFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN], OrigObjPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];
PathAndFile SceneAux;

SceneAux.SetPath((char *)Master->OutPath.GetPath());

// count lights - needed for turning off light effects on luminous objects
NumLights = 0;
if (Master->Lights && Master->ExportLights)
	{
	for (CurLight = Master->Lights; CurLight; CurLight = CurLight->Next)
		{
		if (CurLight->Me)
			NumLights ++;
		} // for
	} // if

if (CurInstance = Master->ObjectInstanceList)
	{
	if ((TotalInstances = CurInstance->CountBoundedInstances(Master->FetchRBounds())) > 0)
		{
		if ((ObjectsDoneList = (Object3DEffect **)AppMem_Alloc(min(TotalInstances, Master->Unique3DObjectInstances) * sizeof (Object3DEffect *), APPMEM_CLEAR))
			&& (HDInstanceList = (long *)AppMem_Alloc(min(TotalInstances, Master->Unique3DObjectInstances) * sizeof (long), APPMEM_CLEAR))
			&& (VertMapName = (char *)AppMem_Alloc(min(TotalInstances, Master->Unique3DObjectInstances) * WCS_EFFECT_MAXNAMELENGTH, APPMEM_CLEAR))
			&& (FullImageName = (char *)AppMem_Alloc(min(TotalInstances, Master->Unique3DObjectInstances) * WCS_PATHANDFILE_PATH_PLUS_NAME_LEN, APPMEM_CLEAR)))
			{
			ObjsDone = 0;
			// need to save each object as a separate file
			while (CurInstance && Success && ObjsDone < Master->Unique3DObjectInstances)
				{
				if (CurObj = CurInstance->MyObj)
					{
					if (CurInstance->IsBounded(Master->FetchRBounds()))
						{
						// figure out a file name
						strcpy(TempFullPath, CurObj->GetName());
						ReplaceChar(TempFullPath, '.', '_');
						SceneAux.SetName(TempFullPath);
						SaveIt = 1;

						// have we exported this object already?
						for (ObjCt = 0; ObjCt < ObjsDone; ObjCt ++)
							{
							if (CurObj == ObjectsDoneList[ObjCt])
								{
								SaveIt = 0;
								ObjectNum = ObjCt;
								}
							} // for
						if (SaveIt)
							{
							ObjectNum = ObjsDone;
							// add to list of ones we've done
							ObjectsDoneList[ObjsDone] = CurObj;
							// if it is a foliage object enter it in the list ofpossible HD Instance source objects
							if (CurInstance->Flags & WCS_OBJECT3DINSTANCE_FLAGBIT_FOLIAGE)
								HDInstanceList[HDSources ++] = ObjectsWritten;
							ObjsDone ++;

							if (Master->ObjectTreatment == WCS_EFFECTS_SCENEEXPORTER_OBJECTTREATMENT_COPYANY && FindFile(CurObj, OrigObjPath))
								{
								strmfp(TempFullPath, Master->OutPath.GetPath(), "Objects");
								// create directory if necessary
								if (PROJ_chdir(TempFullPath))
									PROJ_mkdir(TempFullPath);

								strcat(TempFullPath, "/");
								strcat(TempFullPath, Master->OutPath.GetName());
								// create directory if necessary
								if (PROJ_chdir(TempFullPath))
									PROJ_mkdir(TempFullPath);
								Success = CopyExistingFile(OrigObjPath, TempFullPath, CurObj->FileName);
								} // if
							else
								{
								// open an object file
								if (ObjectFile = OpenObjectFile(SceneAux.GetPath(), "Objects", Master->OutPath.GetName(), SceneAux.GetName()))
									{
									// find out if it is an object made by the exporter which needs to have its texture images deleted
									ExporterMadeObject = Master->FindInEphemeralList(CurObj);

									// save object
									if (Success && ! SaveOneObject(ObjectFile, CurObj, CurInstance, &VertMapName[ObjectNum * WCS_EFFECT_MAXNAMELENGTH], 
										&FullImageName[ObjectNum * WCS_PATHANDFILE_PATH_PLUS_NAME_LEN], ExporterMadeObject, FileNamesCreated))
										Success = 0;
									// close file
									CloseFile(ObjectFile);
									ObjectFile = NULL;
									} // if
								else
									Success = 0;
								} // else
							} // if
						// save object position and rotation
						if (Success)
							{
							Success = SaveObjectMotion(CurFile, CurObj, CurInstance, &Master->ExportRefData, 
								Master->OutPath.GetName(), SceneAux.GetName(), NumLights, TargetObjNum, HDInstanceList[HDInstanceCt],
								&VertMapName[ObjectNum * WCS_EFFECT_MAXNAMELENGTH], &FullImageName[ObjectNum * WCS_PATHANDFILE_PATH_PLUS_NAME_LEN]);
							if (CurInstance->Flags & WCS_OBJECT3DINSTANCE_FLAGBIT_HDINSTANCE)
								HDInstanceCt ++;
							ObjectsWritten ++;
							} // if
						} // if
					} // if object
				CurInstance = CurInstance->Next;
				} // while
			} // if
		else
			Success = 0;
		if (HDInstanceList)
			AppMem_Free(HDInstanceList, min(TotalInstances, Master->Unique3DObjectInstances) * sizeof (long));
		if (ObjectsDoneList)
			AppMem_Free(ObjectsDoneList, min(TotalInstances, Master->Unique3DObjectInstances) * sizeof (Object3DEffect *));
		if (VertMapName)
			AppMem_Free(VertMapName, min(TotalInstances, Master->Unique3DObjectInstances) * WCS_EFFECT_MAXNAMELENGTH);
		if (FullImageName)
			AppMem_Free(FullImageName, min(TotalInstances, Master->Unique3DObjectInstances) * WCS_PATHANDFILE_PATH_PLUS_NAME_LEN);
		} // if
	} // if

return (Success);

} // ExportFormatLW::SaveAllObjects

/*===========================================================================*/

FormatSpecificFile ExportFormatLW::OpenObjectFile(const char *PathName, const char *ObjectDirName, const char *SubDirName, const char *FileName)
{
char FullFileName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN], TempPathName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

if (strlen(PathName) + (SubDirName ? strlen(SubDirName): 0) + strlen(FileName) + (ObjectDirName ? strlen(ObjectDirName): 0) + 7 >= WCS_PATHANDFILE_PATH_PLUS_NAME_LEN)
	{
	UserMessageOK("Lightwave Export", "Compound file path too long. Try a shorter path name.");
	return (0);
	} // if

strcpy(TempPathName, PathName);

// copy file name
if (ObjectDirName && strlen(ObjectDirName) > 0)
	{
	strmfp(FullFileName, TempPathName, ObjectDirName);
	// create directory if necessary
	if (PROJ_chdir(FullFileName))
		PROJ_mkdir(FullFileName);
	strcpy(TempPathName, FullFileName);
	} // if

if (SubDirName && strlen(SubDirName) > 0)
	{
	strmfp(FullFileName, TempPathName, SubDirName);
	// create directory if necessary
	if (PROJ_chdir(FullFileName))
		PROJ_mkdir(FullFileName);
	strcpy(TempPathName, FullFileName);
	} // if

strmfp(FullFileName, TempPathName, FileName);
// append extension
strcat(FullFileName, ".lwo");

return (PROJ_fopen(FullFileName, "wb"));

} // ExportFormatLW::OpenObjectFile

/*===========================================================================*/

// VertMapName can be NULL if the calling app does not need to know the name
// FullImageName can be NULL if the calling app does not need to know the name

int ExportFormatLW::SaveOneObject(FormatSpecificFile CurFile, Object3DEffect *Object3D, Object3DInstance *CurInstance,
	char *VertMapName, char *FullImageName, int ExporterMadeObject, NameList **FileNamesCreated)
{
float FPad = 0.0f, SubChunkFloat;
long PNTSSize, Size, NumVertData, i, DataCt, CurMap, CurClip, BaseImageClip;
unsigned long  PolyWrite32, FORMSeek, POLSSeek, SURFSeek, CLIPSeek, BLOKSeek, IMAPSeek, VMAPSeek, VMADSeek;
FILE *LWFile = (FILE *)CurFile;
char *ChunkTag, *Ext;
float *VertData;
MaterialEffect *Mat;
RootTexture *RootTex;
Texture *UVTex;
Raster *Rast;
VertexReferenceData *VertexRef, *MapRef;
int Success = 1, TAGSSize, NeedVMAD, DiffuseAlphaAvailable = 0;
unsigned short PolyWrite16, SizeS, ShortPad = 0;
short CurLen;
char ImageShortName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN], Extension[24], SaveToPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN], FullFileName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN], OriginalFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

if ((Object3D->NumVertices && ! Object3D->Vertices) || (Object3D->NumPolys && ! Object3D->Polygons))
	{
	if (! Object3D->OpenInputFile(NULL, FALSE, FALSE, FALSE))
		{
		UserMessageOK(Object3D->GetName(), "Could not load 3D Object. Export terminated.");
		Success = 0;
		return (0);
		} // if
	} // if

strmfp(SaveToPath, Master->OutPath.GetPath(), "Images");
if (PROJ_chdir(SaveToPath))
	{
	if (PROJ_mkdir(SaveToPath))
		{
		UserMessageOK("LightWave Export", "Could not create Images subdirectory. Export terminated.");
		return (0);
		} // if
	} // if
strcat(SaveToPath, "/");
strcat(SaveToPath, Master->OutPath.GetName());
if (PROJ_chdir(SaveToPath))
	{
	if (PROJ_mkdir(SaveToPath))
		{
		UserMessageOK("LightWave Export", "Could not create Images subdirectory. Export terminated.");
		return (0);
		} // if
	} // if

if (! (FORMSeek = WriteChunkTag(LWFile, "FORM", 4, sizeof (long))))
	{
	return (0);
	} // if

ChunkTag = "LWO2";
fwrite((char *)ChunkTag, 4, 1, LWFile);

// TAGS
if (Object3D->NumMaterials > 0)
	{
	TAGSSize = 0;
	// add sizes of all material names
	for (i = 0; i < Object3D->NumMaterials; i ++)
		{
		CurLen = (short)strlen(Object3D->NameTable[i].Name) + 1;
		if (CurLen % 2)
			CurLen += 1;
		TAGSSize += CurLen;
		} // for
	if (! (WriteChunkTag(LWFile, "TAGS", TAGSSize, sizeof (long))))
		{
		return (0);
		} // if

	for (i = 0; i < Object3D->NumMaterials; i ++)
		{
		CurLen = (short)strlen(Object3D->NameTable[i].Name) + 1;
		fwrite((char *)Object3D->NameTable[i].Name, CurLen, 1, LWFile);
		if (CurLen % 2)
			fwrite((char *)&ShortPad, 1, 1, LWFile);
		} // for
	} // TAGS

// write out the CLIPs for each material
CurClip = 1;
if (Object3D->VertexUVWAvailable && Object3D->UVWTable && Object3D->UVWTable[0].MapsValid())
	{
	for (i = 0; Success && i < Object3D->NumMaterials; i ++)
		{
		if (Mat = Object3D->NameTable[i].Mat)
			{
			if (RootTex = Mat->GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
				{
				if (UVTex = RootTex->Tex)
					{
					if (UVTex->TexType == WCS_TEXTURE_TYPE_UVW)
						{
						//tmap = &Material3ds->texture.map;
						if (UVTex->Img && (Rast = UVTex->Img->GetRaster()))
							{
							// put file name in ImageShortName
							if (CheckAndShortenFileName(ImageShortName, Rast->GetPath(), Rast->GetName(), 31))
								Rast->PAF.SetName(ImageShortName);
							strmfp(OriginalFullPath, Rast->GetPath(), Rast->GetName());
							CopyExistingFile(OriginalFullPath, SaveToPath, ImageShortName);
							if (ExporterMadeObject)
								PROJ_remove(OriginalFullPath);
							DiffuseAlphaAvailable = (Rast->AlphaAvailable && Rast->AlphaEnabled);
							// make file name for saving to object file
							sprintf(FullFileName, "Images/%s/%s", Master->OutPath.GetName(), ImageShortName);
							// need path/name of file just created in order to make a copy of it for clip map
							// Lightwave gets VERY confused if clip maps and textures use the same image
							strmfp(OriginalFullPath, SaveToPath, ImageShortName);
							// make a file name for use as a clip map
							// get extension
							if (Ext = FindFileExtension(ImageShortName))
								{
								strcpy(Extension, Ext);
								// strip extension
								StripExtension(ImageShortName);
								} // if
							// concatenate "a"
							strcat(ImageShortName, "a");
							// add extension
							AddExtension(ImageShortName, Extension);
							if (FullImageName)
								sprintf(FullImageName, "Images/%s/%s", Master->OutPath.GetName(), ImageShortName);
							// add image to name list
							AddNewNameList(FileNamesCreated, ImageShortName, WCS_EXPORTCONTROL_FILETYPE_3DOBJTEX);

							// CLIP chunk
								{
								if (! (CLIPSeek = WriteChunkTag(LWFile, "CLIP", 4, sizeof (long))))
									{
									return (0);
									} // if

								// 4-byte clip index (1 or larger)
								PolyWrite32 = CurClip ++;
								#ifdef BYTEORDER_LITTLEENDIAN
								SimpleEndianFlip32U(PolyWrite32, &PolyWrite32);
								#endif // BYTEORDER_LITTLEENDIAN
								fwrite((char *)&PolyWrite32, sizeof (long), 1, LWFile);

								// STIL subchunk
									{
									SizeS = (unsigned short)strlen(FullFileName) + 1;
									if (SizeS % 2)
										SizeS += 1;
									if (! (WriteChunkTag(LWFile, "STIL", SizeS, sizeof (short))))
										{
										return (0);
										} // if
									
									// file name
									SizeS = (unsigned short)strlen(FullFileName) + 1;
									fwrite((char *)FullFileName, SizeS, 1, LWFile);
									if (SizeS % 2)
										fwrite((char *)&ShortPad, 1, 1, LWFile);
									} // STIL

								if (! CloseChunk(LWFile, CLIPSeek, sizeof (long)))
									{
									return (0);
									} // if
								} // CLIP

							// CLIP chunk for transparency channel image
							if (DiffuseAlphaAvailable)
								{
								if (! (CurInstance->Flags & WCS_OBJECT3DINSTANCE_FLAGBIT_CLIPMAP))
									{
									if (! (CLIPSeek = WriteChunkTag(LWFile, "CLIP", 4, sizeof (long))))
										{
										return (0);
										} // if

									// 4-byte clip index (1 or larger)
									PolyWrite32 = CurClip ++;
									#ifdef BYTEORDER_LITTLEENDIAN
									SimpleEndianFlip32U(PolyWrite32, &PolyWrite32);
									#endif // BYTEORDER_LITTLEENDIAN
									fwrite((char *)&PolyWrite32, sizeof (long), 1, LWFile);

									// XREF subchunk
										{
										//strcat(FullFileName, " (1)");
										FullFileName[strlen(FullFileName) - 1] = 0;
										SizeS = (unsigned short)strlen(FullFileName) + 1;
										if (SizeS % 2)
											SizeS += 1;
										SizeS += 4;	// reference clip index
										if (! (WriteChunkTag(LWFile, "XREF", SizeS, sizeof (short))))
											{
											return (0);
											} // if

										// 4-byte reference clip index (1 or larger)
										PolyWrite32 = CurClip - 2;
										#ifdef BYTEORDER_LITTLEENDIAN
										SimpleEndianFlip32U(PolyWrite32, &PolyWrite32);
										#endif // BYTEORDER_LITTLEENDIAN
										fwrite((char *)&PolyWrite32, sizeof (long), 1, LWFile);
										
										// file name
										SizeS = (unsigned short)strlen(FullFileName) + 1;
										fwrite((char *)FullFileName, SizeS, 1, LWFile);
										if (SizeS % 2)
											fwrite((char *)&ShortPad, 1, 1, LWFile);
										} // XREF

									// AMOD subchunk
										{
										if (! (WriteChunkTag(LWFile, "AMOD", 2, sizeof (short))))
											{
											return (0);
											} // if

										// not sure what this does but maybe it represents the Alpha Only setting
										PolyWrite16 = 2;
										#ifdef BYTEORDER_LITTLEENDIAN
										SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
										#endif // BYTEORDER_LITTLEENDIAN
										fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
										} // AMOD

									if (! CloseChunk(LWFile, CLIPSeek, sizeof (long)))
										{
										return (0);
										} // if
									} // if ! clip map
								else
									{
									CopyExistingFile(OriginalFullPath, SaveToPath, ImageShortName);
									} // else clip map
								} // CLIP
							} // if
						} // if
					} // if
				} // if
			} // if
		} // for
	} // if VertexUVWAvailable

// LAYR
	{
	if (! (WriteChunkTag(LWFile, "LAYR", 18, sizeof (long))))
		{
		return (0);
		} // if

	// LayerNum = 0
	fwrite((char *)&ShortPad, 2, 1, LWFile);
	// Flags = 0
	fwrite((char *)&ShortPad, 2, 1, LWFile);
	// Pivot (4 floats, not endian-flipped because they're all 0)
	fwrite((char *)&FPad, 4, 1, LWFile);
	fwrite((char *)&FPad, 4, 1, LWFile);
	fwrite((char *)&FPad, 4, 1, LWFile);

	// Name ([S0]NULL, plus NULL pad byte
	fwrite((char *)&ShortPad, 2, 1, LWFile);
	// No parent field
	} // LAYR

// PNTS
if (Object3D->NumVertices > 0)
	{
	NumVertData = Object3D->NumVertices * 3;
	PNTSSize = NumVertData * sizeof (float);	// 3 float values per vertex
	if (! (WriteChunkTag(LWFile, "PNTS", PNTSSize, sizeof (long))))
		{
		return (0);
		} // if

	// prep vertex data
	if (VertData = (float *)AppMem_Alloc(PNTSSize, 0))
		{
		// copy vertex data
		for (i = DataCt = 0; i < Object3D->NumVertices; i ++)
			{
			VertData[DataCt ++] = (float)Object3D->Vertices[i].xyz[0];
			VertData[DataCt ++] = (float)Object3D->Vertices[i].xyz[1];
			VertData[DataCt ++] = (float)Object3D->Vertices[i].xyz[2];
			} // for
		// flip VertexData
		#ifdef BYTEORDER_LITTLEENDIAN
		for (i = 0; i < NumVertData; i ++)
			{
			SimpleEndianFlip32F(&VertData[i], &VertData[i]);
			} // for
		#endif // BYTEORDER_LITTLEENDIAN
		// write the points array to the file 
		fwrite((char *)VertData, PNTSSize, 1, LWFile);
		AppMem_Free(VertData, PNTSSize);
		} // if
	else
		return (0);
	} // PNTS

// write out the VMAPs
if (Object3D->VertexUVWAvailable && Object3D->UVWTable && Object3D->UVWTable[0].MapsValid())
	{
	// VMAP chunks
	for (i = 0; i < Object3D->NumUVWMaps; i ++)
		{
		if (Object3D->UVWTable[i].MapsValid())
			{
			if (! (VMAPSeek = WriteChunkTag(LWFile, "VMAP", 4, sizeof (long))))
				{
				Success = 0;
				break;
				} // if

			if (Object3D->UVWTable[0].UVMapType == WCS_OBJPERVERTMAP_MAPTYPE_UV)
				{
				// type
				ChunkTag = "TXUV";
				fwrite((char *)ChunkTag, 4, 1, LWFile);
				// dimension
				PolyWrite16 = 2;
				#ifdef BYTEORDER_LITTLEENDIAN
				SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
				#endif // BYTEORDER_LITTLEENDIAN
				fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
				} // if
			else if (Object3D->UVWTable[0].UVMapType == WCS_OBJPERVERTMAP_MAPTYPE_WEIGHT)
				{
				// type
				ChunkTag = "WGHT";
				fwrite((char *)ChunkTag, 4, 1, LWFile);
				// dimension
				PolyWrite16 = 1;
				#ifdef BYTEORDER_LITTLEENDIAN
				SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
				#endif // BYTEORDER_LITTLEENDIAN
				fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
				} // else

			// UV map name
			// some apps might not provide a VertMapName because they are not writing out a LW scene file
			if (VertMapName)
				strcpy(VertMapName, Object3D->UVWTable[i].Name);
			SizeS = (unsigned short)strlen(Object3D->UVWTable[i].Name) + 1;
			fwrite((char *)Object3D->UVWTable[i].Name, SizeS, 1, LWFile);
			if (SizeS % 2)
				fwrite((char *)&ShortPad, 1, 1, LWFile);

			// UV pairs
			for (DataCt = 0; DataCt < Object3D->NumVertices; DataCt ++)
				{
				if (Object3D->UVWTable[i].CoordsValid[DataCt])
					{
					// write the vertex number in VX format
					if (DataCt < 0xff00)
						{
						PolyWrite16 = (unsigned short)DataCt;
						#ifdef BYTEORDER_LITTLEENDIAN
						SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
						#endif // BYTEORDER_LITTLEENDIAN
						fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
						} // if
					else
						{
						PolyWrite32 = (DataCt | 0xff000000);
						#ifdef BYTEORDER_LITTLEENDIAN
						SimpleEndianFlip32U(PolyWrite32, &PolyWrite32);
						#endif // BYTEORDER_LITTLEENDIAN
						fwrite((char *)&PolyWrite32, sizeof (long), 1, LWFile);
						} // else
					// write u and v as floats
					SubChunkFloat = (float)Object3D->UVWTable[i].CoordsArray[0][DataCt];
					#ifdef BYTEORDER_LITTLEENDIAN
					SimpleEndianFlip32F(&SubChunkFloat, &SubChunkFloat);
					#endif // BYTEORDER_LITTLEENDIAN
					fwrite((char *)&SubChunkFloat, sizeof (float), 1, LWFile);
					if (Object3D->UVWTable[0].UVMapType == WCS_OBJPERVERTMAP_MAPTYPE_UV)
						{
						SubChunkFloat = (float)Object3D->UVWTable[i].CoordsArray[1][DataCt];
						#ifdef BYTEORDER_LITTLEENDIAN
						SimpleEndianFlip32F(&SubChunkFloat, &SubChunkFloat);
						#endif // BYTEORDER_LITTLEENDIAN
						fwrite((char *)&SubChunkFloat, sizeof (float), 1, LWFile);
						} // if UV map
					} // if
				} // for

			if (! CloseChunk(LWFile, VMAPSeek, sizeof (long)))
				{
				Success = 0;
				break;
				} // if
			} // if
		} // for VMAP
	} // if

// POLS
if (Object3D->NumPolys > 0)
	{
	if (! (POLSSeek = WriteChunkTag(LWFile, "POLS", 4, sizeof (long))))
		{
		return (0);
		} // if

	// write "FACE" 
	ChunkTag = "FACE";
	fwrite((char *)ChunkTag, 4, 1, LWFile);

	for (i = 0; i < Object3D->NumPolys; i ++)
		{
		// write number of vertices as a short
		// if num vertices > 1023 report error and quit
		if (Object3D->Polygons[i].NumVerts > 1023)
			{
			UserMessageOK(Object3D->Name, "More than 1023 vertices in one 3D Object polygon. Export terminated");
			return (0);
			} // if too many vertices
		PolyWrite16 = (unsigned short)Object3D->Polygons[i].NumVerts;
		#ifdef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
		#endif // BYTEORDER_LITTLEENDIAN
		if (fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile) != 1)
			{
			UserMessageOK(Object3D->Name, "Error writing 3D Object LightWave file. Export terminated");
			return (0);
			} // if write error
		// write vertex indices as either 2 or four byte values
		for (DataCt = 0; DataCt < Object3D->Polygons[i].NumVerts; DataCt ++)
			{
			if (Object3D->Polygons[i].VertRef[DataCt] < 0xff00)
				{
				// if index < 0xff00 write as two bytes
				PolyWrite16 = (short)Object3D->Polygons[i].VertRef[DataCt];
				#ifdef BYTEORDER_LITTLEENDIAN
				SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
				#endif // BYTEORDER_LITTLEENDIAN
				fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
				} // if
			else
				{
				// else write as four bytes or'd with 0xff000000
				PolyWrite32 = (Object3D->Polygons[i].VertRef[DataCt] | 0xff000000);
				#ifdef BYTEORDER_LITTLEENDIAN
				SimpleEndianFlip32U(PolyWrite32, &PolyWrite32);
				#endif // BYTEORDER_LITTLEENDIAN
				fwrite((char *)&PolyWrite32, sizeof (long), 1, LWFile);
				} // else
			} // for
		} // for

	if (! CloseChunk(LWFile, POLSSeek, sizeof (long)))
		{
		return (0);
		} // if
	} // POLS

// write out the VMADs
if (Object3D->VertexUVWAvailable && Object3D->UVWTable && Object3D->UVWTable[0].MapsValid())
	{
	// VMAD chunks
	NeedVMAD = 0;
	// see if we need a VMAD chunk
	for (i = 0; i < Object3D->NumPolys && ! NeedVMAD; i ++)
		{
		if (VertexRef = Object3D->Polygons[i].RefData)
			{
			while (VertexRef && ! NeedVMAD)
				{
				MapRef = VertexRef;
				while (MapRef)
					{
					if (MapRef->MapType == WCS_VERTREFDATA_MAPTYPE_UVW)
						{
						NeedVMAD = 1;
						break;
						} // if
					MapRef = MapRef->NextMap;
					} // while
				VertexRef = VertexRef->NextVertex;
				} // while
			} // if
		} // for
	if (NeedVMAD)
		{
		for (CurMap = 0; CurMap < Object3D->NumUVWMaps; CurMap ++)
			{
			VMADSeek = 0;
			for (i = 0; i < Object3D->NumPolys; i ++)
				{
				if (VertexRef = Object3D->Polygons[i].RefData)
					{
					while (VertexRef)
						{
						MapRef = VertexRef;
						while (MapRef)
							{
							if (MapRef->MapType == WCS_VERTREFDATA_MAPTYPE_UVW && MapRef->MapNumber == CurMap)
								{
								// if not written write out chunk tag, type, dimension and map name
								if (! VMADSeek)
									{
									if (! (VMADSeek = WriteChunkTag(LWFile, "VMAD", 4, sizeof (long))))
										{
										return (0);
										} // if
									// type
									ChunkTag = "TXUV";
									fwrite((char *)ChunkTag, 4, 1, LWFile);
									// dimension
									PolyWrite16 = 2;
									#ifdef BYTEORDER_LITTLEENDIAN
									SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
									#endif // BYTEORDER_LITTLEENDIAN
									fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);

									// UV map name
									SizeS = (unsigned short)strlen(Object3D->UVWTable[CurMap].Name) + 1;
									fwrite((char *)Object3D->UVWTable[CurMap].Name, SizeS, 1, LWFile);
									if (SizeS % 2)
										fwrite((char *)&ShortPad, 1, 1, LWFile);
									} // if
								// write out this vertex's info
								// vertex number VX
								if (MapRef->ObjVertNumber < 0xff00)
									{
									// if index < 0xff00 write as two bytes
									PolyWrite16 = (short)MapRef->ObjVertNumber;
									#ifdef BYTEORDER_LITTLEENDIAN
									SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
									#endif // BYTEORDER_LITTLEENDIAN
									fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
									} // if
								else
									{
									// else write as four bytes or'd with 0xff000000
									PolyWrite32 = (MapRef->ObjVertNumber | 0xff000000);
									#ifdef BYTEORDER_LITTLEENDIAN
									SimpleEndianFlip32U(PolyWrite32, &PolyWrite32);
									#endif // BYTEORDER_LITTLEENDIAN
									fwrite((char *)&PolyWrite32, sizeof (long), 1, LWFile);
									} // else
								// polygon number VX
								if (i < 0xff00)
									{
									// if index < 0xff00 write as two bytes
									PolyWrite16 = (short)i;
									#ifdef BYTEORDER_LITTLEENDIAN
									SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
									#endif // BYTEORDER_LITTLEENDIAN
									fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
									} // if
								else
									{
									// else write as four bytes or'd with 0xff000000
									PolyWrite32 = (i | 0xff000000);
									#ifdef BYTEORDER_LITTLEENDIAN
									SimpleEndianFlip32U(PolyWrite32, &PolyWrite32);
									#endif // BYTEORDER_LITTLEENDIAN
									fwrite((char *)&PolyWrite32, sizeof (long), 1, LWFile);
									} // else
								// write u and v as floats
								SubChunkFloat = (float)Object3D->UVWTable[CurMap].CoordsArray[0][MapRef->VertRefNumber];
								#ifdef BYTEORDER_LITTLEENDIAN
								SimpleEndianFlip32F(&SubChunkFloat, &SubChunkFloat);
								#endif // BYTEORDER_LITTLEENDIAN
								fwrite((char *)&SubChunkFloat, sizeof (float), 1, LWFile);
								SubChunkFloat = (float)Object3D->UVWTable[CurMap].CoordsArray[1][MapRef->VertRefNumber];
								#ifdef BYTEORDER_LITTLEENDIAN
								SimpleEndianFlip32F(&SubChunkFloat, &SubChunkFloat);
								#endif // BYTEORDER_LITTLEENDIAN
								fwrite((char *)&SubChunkFloat, sizeof (float), 1, LWFile);
								// break to next vertex
								break;
								} // if
							MapRef = MapRef->NextMap;
							} // while
						VertexRef = VertexRef->NextVertex;
						} // while
					} // if VertexRef
				} // for

			if (VMADSeek && ! CloseChunk(LWFile, VMADSeek, sizeof (long)))
				{
				return (0);
				} // if
			} // for each vertex map
		} // if NeedVMAD
	} // if VertexUVWAvailable

// PTAG/SURF - material numbers
if (Object3D->NumPolys > 0)
	{
	Size = 4; // SURF + ...
	// add up ptag VXes
	for (i = 0; i < Object3D->NumPolys; i++)
		{
		if (i < 0xff00)
			{
			Size += 4; // VX[2] + U2
			} // if
		else
			{
			Size += 6; // VX[4] + U2
			} // else
		} // if
	if (! (WriteChunkTag(LWFile, "PTAG", Size, sizeof (long))))
		{
		return (0);
		} // if

	// write SURF
	ChunkTag = "SURF";
	fwrite((char *)ChunkTag, 4, 1, LWFile);

	// write ptags
	for (i = 0; i < Object3D->NumPolys; i ++)
		{
		if (i < 0xff00)
			{
			PolyWrite16 = (unsigned short)i;
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
			#endif // BYTEORDER_LITTLEENDIAN
			fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
			} // if
		else
			{
			PolyWrite32 = (i | 0xff000000);
			#ifdef BYTEORDER_LITTLEENDIAN
			SimpleEndianFlip32U(PolyWrite32, &PolyWrite32);
			#endif // BYTEORDER_LITTLEENDIAN
			fwrite((char *)&PolyWrite32, sizeof (long), 1, LWFile);
			} // else
		// write tag value - material number
		PolyWrite16 = (short)Object3D->Polygons[i].Material;
		#ifdef BYTEORDER_LITTLEENDIAN
		SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
		#endif // BYTEORDER_LITTLEENDIAN
		fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile); // 16-bit zero
		} // if
	} // PTAG - SURF

CurClip = 1;
// surface array
for (i = 0; Success && i < Object3D->NumMaterials; i ++)
	{
	if (Mat = Object3D->NameTable[i].Mat)
		{
		// SURF
			{
			if (! (SURFSeek = WriteChunkTag(LWFile, "SURF", 4, sizeof (long))))
				{
				Success = 0;
				break;
				} // if

			// write SURFace name
			CurLen = (short)strlen(Object3D->NameTable[i].Name) + 1;
			fwrite((char *)Object3D->NameTable[i].Name, CurLen, 1, LWFile);
			if (CurLen % 2)
				fwrite((char *)&ShortPad, 1, 1, LWFile);

			// write surface source (null) name
			fwrite((char *)&ShortPad, 2, 1, LWFile); // ending NULL, plus pad NULL

			// Subchunk COLR
				{
				if (! (WriteChunkTag(LWFile, "COLR", 14, sizeof (short))))
					{
					return (0);
					} // if

				// Color Data
				// write three color channels
				SubChunkFloat = (float)Mat->DiffuseColor.GetCurValue(0);
				#ifdef BYTEORDER_LITTLEENDIAN
				SimpleEndianFlip32F(&SubChunkFloat, &SubChunkFloat);
				#endif // BYTEORDER_LITTLEENDIAN
				fwrite((char *)&SubChunkFloat, sizeof (float), 1, LWFile);
				SubChunkFloat = (float)Mat->DiffuseColor.GetCurValue(1);
				#ifdef BYTEORDER_LITTLEENDIAN
				SimpleEndianFlip32F(&SubChunkFloat, &SubChunkFloat);
				#endif // BYTEORDER_LITTLEENDIAN
				fwrite((char *)&SubChunkFloat, sizeof (float), 1, LWFile);
				SubChunkFloat = (float)Mat->DiffuseColor.GetCurValue(2);
				#ifdef BYTEORDER_LITTLEENDIAN
				SimpleEndianFlip32F(&SubChunkFloat, &SubChunkFloat);
				#endif // BYTEORDER_LITTLEENDIAN
				fwrite((char *)&SubChunkFloat, sizeof (float), 1, LWFile);
				// write a VX envelope ID of 0 (with a pad byte)
				fwrite((char *)&ShortPad, sizeof (short), 1, LWFile);
				} // COLR

			// Subchunk DIFF
				{
				if (! (WriteChunkTag(LWFile, "DIFF", 6, sizeof (short))))
					{
					return (0);
					} // if

				// DIFFuse Data
				SubChunkFloat = (float)(Mat->DiffuseColor.Intensity.GetCurValue(0) *
					Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DIFFUSEINTENSITY].GetCurValue(0));
				#ifdef BYTEORDER_LITTLEENDIAN
				SimpleEndianFlip32F(&SubChunkFloat, &SubChunkFloat);
				#endif // BYTEORDER_LITTLEENDIAN
				fwrite((char *)&SubChunkFloat, sizeof (float), 1, LWFile);
				// write a VX envelope ID of 0 (with a pad byte)
				fwrite((char *)&ShortPad, sizeof (short), 1, LWFile);
				} // DIFF

			// Subchunk LUMI
				{
				if (! (WriteChunkTag(LWFile, "LUMI", 6, sizeof (short))))
					{
					return (0);
					} // if

				// LUMI Data
				SubChunkFloat = (float)Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].GetCurValue(0);
				#ifdef BYTEORDER_LITTLEENDIAN
				SimpleEndianFlip32F(&SubChunkFloat, &SubChunkFloat);
				#endif // BYTEORDER_LITTLEENDIAN
				fwrite((char *)&SubChunkFloat, sizeof (float), 1, LWFile);
				// write a VX envelope ID of 0 (with a pad byte)
				fwrite((char *)&ShortPad, sizeof (short), 1, LWFile);
				} // LUMI

			// Subchunk SPEC
				{
				if (! (WriteChunkTag(LWFile, "SPEC", 6, sizeof (short))))
					{
					return (0);
					} // if

				// SPEC Data
				SubChunkFloat = (float)Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY].GetCurValue(0);
				#ifdef BYTEORDER_LITTLEENDIAN
				SimpleEndianFlip32F(&SubChunkFloat, &SubChunkFloat);
				#endif // BYTEORDER_LITTLEENDIAN
				fwrite((char *)&SubChunkFloat, sizeof (float), 1, LWFile);
				// write a VX envelope ID of 0 (with a pad byte)
				fwrite((char *)&ShortPad, sizeof (short), 1, LWFile);
				} // SPEC

			// Subchunk REFL
				{
				if (! (WriteChunkTag(LWFile, "REFL", 6, sizeof (short))))
					{
					return (0);
					} // if

				// REFL Data
				SubChunkFloat = (float)Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY].GetCurValue(0);
				#ifdef BYTEORDER_LITTLEENDIAN
				SimpleEndianFlip32F(&SubChunkFloat, &SubChunkFloat);
				#endif // BYTEORDER_LITTLEENDIAN
				fwrite((char *)&SubChunkFloat, sizeof (float), 1, LWFile);
				// write a VX envelope ID of 0 (with a pad byte)
				fwrite((char *)&ShortPad, sizeof (short), 1, LWFile);
				} // REFL

			// Subchunk TRAN
				{
				if (! (WriteChunkTag(LWFile, "TRAN", 6, sizeof (short))))
					{
					return (0);
					} // if

				// TRAN Data
				SubChunkFloat = (float)Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].GetCurValue(0);
				#ifdef BYTEORDER_LITTLEENDIAN
				SimpleEndianFlip32F(&SubChunkFloat, &SubChunkFloat);
				#endif // BYTEORDER_LITTLEENDIAN
				fwrite((char *)&SubChunkFloat, sizeof (float), 1, LWFile);
				// write a VX envelope ID of 0 (with a pad byte)
				fwrite((char *)&ShortPad, sizeof (short), 1, LWFile);
				} // TRAN

			// Subchunk GLOS
				{
				if (! (WriteChunkTag(LWFile, "GLOS", 6, sizeof (short))))
					{
					return (0);
					} // if

				// GLOS Data
				// for specular exponent of the type: cos^n(a), where a is the angle between the reflection and view vectors
				// a glossiness factor is defined from n as:
				// n = 2^(10g + 2)
				// log(n) = log(2) * (10g + 2)
				// 10g + 2 = log(n) / log(2)
				// g = .1 * (-2 + log(n) / log(2))
				// in order for g to remain non-negative, n must be greater or equal to 4
				if (Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].GetCurValue(0) > 4.0)
					SubChunkFloat = (float)(.1 * (-2.0 + log(Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].GetCurValue(0)) / log(2.0)));
				else
					SubChunkFloat = 0.0f;
				#ifdef BYTEORDER_LITTLEENDIAN
				SimpleEndianFlip32F(&SubChunkFloat, &SubChunkFloat);
				#endif // BYTEORDER_LITTLEENDIAN
				fwrite((char *)&SubChunkFloat, sizeof (float), 1, LWFile);
				// write a VX envelope ID of 0 (with a pad byte)
				fwrite((char *)&ShortPad, sizeof (short), 1, LWFile);
				} // GLOS

			// Subchunk BUMP
				{
				if (! (WriteChunkTag(LWFile, "BUMP", 6, sizeof (short))))
					{
					return (0);
					} // if

				// BUMP Data
				SubChunkFloat = (float)Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_BUMPINTENSITY].GetCurValue(0);
				#ifdef BYTEORDER_LITTLEENDIAN
				SimpleEndianFlip32F(&SubChunkFloat, &SubChunkFloat);
				#endif // BYTEORDER_LITTLEENDIAN
				fwrite((char *)&SubChunkFloat, sizeof (float), 1, LWFile);
				// write a VX envelope ID of 0 (with a pad byte)
				fwrite((char *)&ShortPad, sizeof (short), 1, LWFile);
				} // BUMP

			// Subchunk SIDE
				{
				if (! (WriteChunkTag(LWFile, "SIDE", 2, sizeof (short))))
					{
					return (0);
					} // if

				// SIDEedness Data
				PolyWrite16 = (short)Mat->DoubleSided ? 3: 1;
				#ifdef BYTEORDER_LITTLEENDIAN
				SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
				#endif // BYTEORDER_LITTLEENDIAN
				fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
				} // SIDE

			// Subchunk SMAN
			if (Mat->Shading == WCS_EFFECT_MATERIAL_SHADING_PHONG)
				{
				if (! (WriteChunkTag(LWFile, "SMAN", 4, sizeof (short))))
					{
					return (0);
					} // if

				// SMAN smoothing angle Data
				SubChunkFloat = (float)(Mat->SmoothAngle * PiOver180);
				#ifdef BYTEORDER_LITTLEENDIAN
				SimpleEndianFlip32F(&SubChunkFloat, &SubChunkFloat);
				#endif // BYTEORDER_LITTLEENDIAN
				fwrite((char *)&SubChunkFloat, sizeof (float), 1, LWFile);
				} // if SMAN
			
			// Subchunk texture map
			if (Object3D->VertexUVWAvailable && Object3D->UVWTable && Object3D->UVWTable[0].MapsValid())
				{
				if (RootTex = Mat->GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
					{
					if (UVTex = RootTex->Tex)
						{
						if (UVTex->TexType == WCS_TEXTURE_TYPE_UVW)
							{
							if (UVTex->Img && (Rast = UVTex->Img->GetRaster()))
								{
								DiffuseAlphaAvailable = (Rast->AlphaAvailable && Rast->AlphaEnabled);

								// first diffuse color
								// BLOK chunk
									{
									if (! (BLOKSeek = WriteChunkTag(LWFile, "BLOK", 4, sizeof (short))))
										{
										return (0);
										} // if

									// header subchunk
										{
										if (! (IMAPSeek = WriteChunkTag(LWFile, "IMAP", 4, sizeof (short))))
											{
											return (0);
											} // if
										
										// ordinal for sorting
										ChunkTag = "\x80";
										fwrite((char *)ChunkTag, strlen(ChunkTag) + 1, 1, LWFile);
										if ((strlen(ChunkTag) + 1) % 2)
											fwrite((char *)&ShortPad, 1, 1, LWFile);

										// CHAN - channel subchunk
											{
											if (! (WriteChunkTag(LWFile, "CHAN", 4, sizeof (short))))
												{
												return (0);
												} // if

											// block is for diffuse color channel
											ChunkTag = "COLR";
											fwrite((char *)ChunkTag, 4, 1, LWFile);
											} // CHAN

										// ENAB - enabled state defaults to TRUE
											{
											if (! (WriteChunkTag(LWFile, "ENAB", 2, sizeof (short))))
												{
												return (0);
												} // if

											// enabled state
											PolyWrite16 = 1;
											#ifdef BYTEORDER_LITTLEENDIAN
											SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
											#endif // BYTEORDER_LITTLEENDIAN
											fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
											} // ENAB

										// NEGA
											{
											if (! (WriteChunkTag(LWFile, "NEGA", 2, sizeof (short))))
												{
												return (0);
												} // if

											// negation
											PolyWrite16 = UVTex->ImageNeg ? 1: 0;
											#ifdef BYTEORDER_LITTLEENDIAN
											SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
											#endif // BYTEORDER_LITTLEENDIAN
											fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
											} // NEGA

										// OPAC - opacity subchunk
											{
											if (! (WriteChunkTag(LWFile, "OPAC", 8, sizeof (short))))
												{
												return (0);
												} // if

											// texture opacity
											fwrite((char *)&ShortPad, sizeof (short), 1, LWFile);
											// opacity
											SubChunkFloat = (float)(UVTex->TexParam[WCS_TEXTURE_OPACITY].CurValue * .01);
											#ifdef BYTEORDER_LITTLEENDIAN
											SimpleEndianFlip32F(&SubChunkFloat, &SubChunkFloat);
											#endif // BYTEORDER_LITTLEENDIAN
											fwrite((char *)&SubChunkFloat, sizeof (float), 1, LWFile);
											// envelope
											fwrite((char *)&ShortPad, sizeof (short), 1, LWFile);
											} // OPAC

										if (! CloseChunk(LWFile, IMAPSeek, sizeof (short)))
											{
											return (0);
											} // if
										} // header subchunk

									// IMAG subchunk
										{
										if (! (WriteChunkTag(LWFile, "IMAG", 2, sizeof (short))))
											{
											return (0);
											} // if

										// specify CLIP index
										BaseImageClip = CurClip;
										PolyWrite16 = (unsigned short)CurClip ++;
										#ifdef BYTEORDER_LITTLEENDIAN
										SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
										#endif // BYTEORDER_LITTLEENDIAN
										fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
										} // IMAG

									// VMAP subchunk
										{
										SizeS = (unsigned short)strlen(UVTex->MapName) + 1;
										if (SizeS % 2)
											SizeS ++;
										if (! (WriteChunkTag(LWFile, "VMAP", SizeS, sizeof (short))))
											{
											return (0);
											} // if

										// specify vertex map name
										SizeS = (unsigned short)strlen(UVTex->MapName) + 1;
										fwrite((char *)UVTex->MapName, SizeS, 1, LWFile);
										if (SizeS % 2)
											fwrite((char *)&ShortPad, 1, 1, LWFile);
										} // VMAP

									// PROJ subchunk
										{
										if (! (WriteChunkTag(LWFile, "PROJ", 2, sizeof (short))))
											{
											return (0);
											} // if

										// specify projection type as UV
										PolyWrite16 = 5;
										#ifdef BYTEORDER_LITTLEENDIAN
										SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
										#endif // BYTEORDER_LITTLEENDIAN
										fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
										} // PROJ

									// WRAP subchunk
										{
										if (! (WriteChunkTag(LWFile, "WRAP", 4, sizeof (short))))
											{
											return (0);
											} // if

										// specify image repeat for width and height
										PolyWrite16 = UVTex->TileWidth ? 1: 0;
										#ifdef BYTEORDER_LITTLEENDIAN
										SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
										#endif // BYTEORDER_LITTLEENDIAN
										fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
										PolyWrite16 = UVTex->TileHeight ? 1: 0;
										#ifdef BYTEORDER_LITTLEENDIAN
										SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
										#endif // BYTEORDER_LITTLEENDIAN
										fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
										} // WRAP

									if (! CloseChunk(LWFile, BLOKSeek, sizeof (short)))
										{
										return (0);
										} // if
									} // BLOK

								// BLOK chunk for transparency channel
								if (DiffuseAlphaAvailable && ! (CurInstance->Flags & WCS_OBJECT3DINSTANCE_FLAGBIT_CLIPMAP))
									{
									if (! (BLOKSeek = WriteChunkTag(LWFile, "BLOK", 4, sizeof (short))))
										{
										return (0);
										} // if

									// header subchunk
										{
										if (! (IMAPSeek = WriteChunkTag(LWFile, "IMAP", 4, sizeof (short))))
											{
											return (0);
											} // if
										
										// ordinal for sorting
										ChunkTag = "\x80";
										fwrite((char *)ChunkTag, strlen(ChunkTag) + 1, 1, LWFile);
										if ((strlen(ChunkTag) + 1) % 2)
											fwrite((char *)&ShortPad, 1, 1, LWFile);

										// CHAN - channel subchunk
											{
											if (! (WriteChunkTag(LWFile, "CHAN", 4, sizeof (short))))
												{
												return (0);
												} // if

											// block is for diffuse color channel
											ChunkTag = "TRAN";
											fwrite((char *)ChunkTag, 4, 1, LWFile);
											} // CHAN

										// ENAB - enabled state defaults to TRUE
											{
											if (! (WriteChunkTag(LWFile, "ENAB", 2, sizeof (short))))
												{
												return (0);
												} // if

											// enabled state
											PolyWrite16 = 1;
											#ifdef BYTEORDER_LITTLEENDIAN
											SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
											#endif // BYTEORDER_LITTLEENDIAN
											fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
											} // ENAB

										// NEGA
											{
											if (! (WriteChunkTag(LWFile, "NEGA", 2, sizeof (short))))
												{
												return (0);
												} // if

											// negation
											PolyWrite16 = 1;
											#ifdef BYTEORDER_LITTLEENDIAN
											SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
											#endif // BYTEORDER_LITTLEENDIAN
											fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
											} // NEGA

										// OPAC - opacity subchunk
											{
											if (! (WriteChunkTag(LWFile, "OPAC", 8, sizeof (short))))
												{
												return (0);
												} // if

											// texture opacity
											fwrite((char *)&ShortPad, sizeof (short), 1, LWFile);
											// opacity
											SubChunkFloat = (float)(UVTex->TexParam[WCS_TEXTURE_OPACITY].CurValue * .01);
											#ifdef BYTEORDER_LITTLEENDIAN
											SimpleEndianFlip32F(&SubChunkFloat, &SubChunkFloat);
											#endif // BYTEORDER_LITTLEENDIAN
											fwrite((char *)&SubChunkFloat, sizeof (float), 1, LWFile);
											// envelope
											fwrite((char *)&ShortPad, sizeof (short), 1, LWFile);
											} // OPAC

										if (! CloseChunk(LWFile, IMAPSeek, sizeof (short)))
											{
											return (0);
											} // if
										} // header subchunk

									// IMAG subchunk
										{
										if (! (WriteChunkTag(LWFile, "IMAG", 2, sizeof (short))))
											{
											return (0);
											} // if

										// specify CLIP index
										PolyWrite16 = (unsigned short)CurClip ++;
										#ifdef BYTEORDER_LITTLEENDIAN
										SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
										#endif // BYTEORDER_LITTLEENDIAN
										fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
										} // IMAG

									// VMAP subchunk
										{
										SizeS = (unsigned short)strlen(UVTex->MapName) + 1;
										if (SizeS % 2)
											SizeS ++;
										if (! (WriteChunkTag(LWFile, "VMAP", SizeS, sizeof (short))))
											{
											return (0);
											} // if

										// specify vertex map name
										SizeS = (unsigned short)strlen(UVTex->MapName) + 1;
										fwrite((char *)UVTex->MapName, SizeS, 1, LWFile);
										if (SizeS % 2)
											fwrite((char *)&ShortPad, 1, 1, LWFile);
										} // VMAP

									// PROJ subchunk
										{
										if (! (WriteChunkTag(LWFile, "PROJ", 2, sizeof (short))))
											{
											return (0);
											} // if

										// specify projection type as UV
										PolyWrite16 = 5;
										#ifdef BYTEORDER_LITTLEENDIAN
										SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
										#endif // BYTEORDER_LITTLEENDIAN
										fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
										} // PROJ

									// WRAP subchunk
										{
										if (! (WriteChunkTag(LWFile, "WRAP", 4, sizeof (short))))
											{
											return (0);
											} // if

										// specify image repeat for width and height
										PolyWrite16 = UVTex->TileWidth ? 1: 0;
										#ifdef BYTEORDER_LITTLEENDIAN
										SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
										#endif // BYTEORDER_LITTLEENDIAN
										fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
										PolyWrite16 = UVTex->TileHeight ? 1: 0;
										#ifdef BYTEORDER_LITTLEENDIAN
										SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
										#endif // BYTEORDER_LITTLEENDIAN
										fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
										} // WRAP

									if (! CloseChunk(LWFile, BLOKSeek, sizeof (short)))
										{
										return (0);
										} // if
									} // BLOK
								} // if Img
							} // UVTex->TexType
						} // if UVTex
					} // if RootTex

				// bump channel
				if (RootTex = Mat->GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP))
					{
					if (UVTex = RootTex->Tex)
						{
						if (UVTex->TexType == WCS_TEXTURE_TYPE_UVW)
							{
							if (UVTex->Img && (Rast = UVTex->Img->GetRaster()))
								{
								// BLOK chunk
									{
									if (! (BLOKSeek = WriteChunkTag(LWFile, "BLOK", 4, sizeof (short))))
										{
										return (0);
										} // if

									// header subchunk
										{
										if (! (IMAPSeek = WriteChunkTag(LWFile, "IMAP", 4, sizeof (short))))
											{
											return (0);
											} // if
										
										// ordinal for sorting
										ChunkTag = "\x80";
										fwrite((char *)ChunkTag, strlen(ChunkTag) + 1, 1, LWFile);
										if ((strlen(ChunkTag) + 1) % 2)
											fwrite((char *)&ShortPad, 1, 1, LWFile);

										// CHAN - channel subchunk
											{
											if (! (WriteChunkTag(LWFile, "CHAN", 4, sizeof (short))))
												{
												return (0);
												} // if

											// block is for diffuse color channel
											ChunkTag = "BUMP";
											fwrite((char *)ChunkTag, 4, 1, LWFile);
											} // CHAN

										// ENAB - enabled state defaults to TRUE
											{
											if (! (WriteChunkTag(LWFile, "ENAB", 2, sizeof (short))))
												{
												return (0);
												} // if

											// enabled state
											PolyWrite16 = 1;
											#ifdef BYTEORDER_LITTLEENDIAN
											SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
											#endif // BYTEORDER_LITTLEENDIAN
											fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
											} // ENAB

										// NEGA
											{
											if (! (WriteChunkTag(LWFile, "NEGA", 2, sizeof (short))))
												{
												return (0);
												} // if

											// negation
											PolyWrite16 = UVTex->ImageNeg ? 1: 0;
											#ifdef BYTEORDER_LITTLEENDIAN
											SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
											#endif // BYTEORDER_LITTLEENDIAN
											fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
											} // NEGA

										// OPAC - opacity subchunk
											{
											if (! (WriteChunkTag(LWFile, "OPAC", 8, sizeof (short))))
												{
												return (0);
												} // if

											// texture opacity
											fwrite((char *)&ShortPad, sizeof (short), 1, LWFile);
											// opacity
											SubChunkFloat = (float)(UVTex->TexParam[WCS_TEXTURE_OPACITY].CurValue * .01);
											#ifdef BYTEORDER_LITTLEENDIAN
											SimpleEndianFlip32F(&SubChunkFloat, &SubChunkFloat);
											#endif // BYTEORDER_LITTLEENDIAN
											fwrite((char *)&SubChunkFloat, sizeof (float), 1, LWFile);
											// envelope
											fwrite((char *)&ShortPad, sizeof (short), 1, LWFile);
											} // OPAC

										if (! CloseChunk(LWFile, IMAPSeek, sizeof (short)))
											{
											return (0);
											} // if
										} // header subchunk

									// IMAG subchunk
										{
										if (! (WriteChunkTag(LWFile, "IMAG", 2, sizeof (short))))
											{
											return (0);
											} // if

										// specify CLIP index
										PolyWrite16 = (unsigned short)BaseImageClip;
										#ifdef BYTEORDER_LITTLEENDIAN
										SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
										#endif // BYTEORDER_LITTLEENDIAN
										fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
										} // IMAG

									// VMAP subchunk
										{
										SizeS = (unsigned short)strlen(UVTex->MapName) + 1;
										if (SizeS % 2)
											SizeS ++;
										if (! (WriteChunkTag(LWFile, "VMAP", SizeS, sizeof (short))))
											{
											return (0);
											} // if

										// specify vertex map name
										SizeS = (unsigned short)strlen(UVTex->MapName) + 1;
										fwrite((char *)UVTex->MapName, SizeS, 1, LWFile);
										if (SizeS % 2)
											fwrite((char *)&ShortPad, 1, 1, LWFile);
										} // VMAP

									// PROJ subchunk
										{
										if (! (WriteChunkTag(LWFile, "PROJ", 2, sizeof (short))))
											{
											return (0);
											} // if

										// specify projection type as UV
										PolyWrite16 = 5;
										#ifdef BYTEORDER_LITTLEENDIAN
										SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
										#endif // BYTEORDER_LITTLEENDIAN
										fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
										} // PROJ

									// WRAP subchunk
										{
										if (! (WriteChunkTag(LWFile, "WRAP", 4, sizeof (short))))
											{
											return (0);
											} // if

										// specify image repeat for width and height
										PolyWrite16 = UVTex->TileWidth ? 1: 0;
										#ifdef BYTEORDER_LITTLEENDIAN
										SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
										#endif // BYTEORDER_LITTLEENDIAN
										fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
										PolyWrite16 = UVTex->TileHeight ? 1: 0;
										#ifdef BYTEORDER_LITTLEENDIAN
										SimpleEndianFlip16U(PolyWrite16, &PolyWrite16);
										#endif // BYTEORDER_LITTLEENDIAN
										fwrite((char *)&PolyWrite16, sizeof (short), 1, LWFile);
										} // WRAP

									if (! CloseChunk(LWFile, BLOKSeek, sizeof (short)))
										{
										return (0);
										} // if
									} // BLOK
								} // if Img
							} // UVTex->TexType
						} // if UVTex
					} // if RootTex
				} // Object3D->VertexUVWAvailable
				
			if (! CloseChunk(LWFile, SURFSeek, sizeof (long)))
				{
				return (0);
				} // if

			} // SURF
		} // if Mat
	} // for

if (! CloseChunk(LWFile, FORMSeek, sizeof (long)))
	{
	return (0);
	} // if

return (Success);

} // ExportFormatLW::SaveOneObject

/*===========================================================================*/

long ExportFormatLW::WriteChunkTag(FILE *LWFile, char *ChunkTag, long ChunkSize, int SizeBytes)
{
long SizePos;
unsigned long Size;
unsigned short SizeS;

if (fwrite((char *)ChunkTag, 4, 1, LWFile) != 1)
	return (0);

SizePos = ftell(LWFile);

if (SizeBytes == sizeof (short))
	{
	SizeS = (unsigned short)ChunkSize;
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip16U(SizeS, &SizeS);
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((char *)&SizeS, sizeof (short), 1, LWFile);
	} // if
else
	{
	Size = ChunkSize;
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32U(Size, &Size);
	#endif // BYTEORDER_LITTLEENDIAN
	fwrite((char *)&Size, sizeof (long), 1, LWFile);
	} // else

return (SizePos);

} // ExportFormatLW::WriteChunkTag

/*===========================================================================*/

int ExportFormatLW::CloseChunk(FILE *LWFile, long SeekPos, int SizeBytes)
{
unsigned long Size;
unsigned short SizeS;

if (SizeBytes == sizeof (short))
	{
	SizeS = (unsigned short)(ftell(LWFile) - SeekPos - sizeof (short));
	if (fseek(LWFile, SeekPos, SEEK_SET))
		return (0);
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip16U(SizeS, &SizeS);
	#endif // BYTEORDER_LITTLEENDIAN
	if (fwrite((char *)&SizeS, sizeof (short), 1, LWFile) != 1)
		return (0);
	}
else
	{
	Size = (unsigned long)(ftell(LWFile) - SeekPos - sizeof (long));
	if (fseek(LWFile, SeekPos, SEEK_SET))
		return (0);
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32U(Size, &Size);
	#endif // BYTEORDER_LITTLEENDIAN
	if (! fwrite((char *)&Size, sizeof (long), 1, LWFile))
		return (0);
	} // else

if (fseek(LWFile, 0, SEEK_END))
	return (0);

return (1);

} // ExportFormatLW::CloseChunk

/*===========================================================================*/

int ExportFormatLW::SaveObjectMotion(FormatSpecificFile CurFile, Object3DEffect *Object3D, Object3DInstance *CurInstance,
	ExportReferenceData *RefData, const char *DirName, const char *FileName, long NumLights, long TargetObjNum, long HDInstanceCt,
	const char *VertMapName, const char *FullImageName)
{
unsigned long ObjectToClone;
long LightCt, TargetNum;
FILE *LWFile = (FILE *)CurFile;
int Success = 1;
char FullFileName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN], UniqueImageName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN], ImageName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

UniqueImageName[0] = 0;
ImageName[0] = 0;
if (FullImageName && FullImageName[0])
	{
	BreakFileName((char *)FullImageName, FullFileName, WCS_PATHANDFILE_PATH_PLUS_NAME_LEN, ImageName, WCS_PATHANDFILE_PATH_PLUS_NAME_LEN);
	strcpy(UniqueImageName, FullImageName);
	UniqueImageName[strlen(UniqueImageName) - 1] = 0;
	//strcat(UniqueImageName, " (1)");
	} // if

sprintf(FullFileName, "Objects/%s/%s", DirName, FileName);
strcat(FullFileName, ".lwo");

fprintf(LWFile, "LoadObjectLayer 1 %s\n", FullFileName);
fprintf(LWFile, "ShowObject 6 7\n");
fprintf(LWFile, "ObjectMotion\n");
fprintf(LWFile, "NumChannels 9\n");

// location
fprintf(LWFile, "Channel 0\n");
fprintf(LWFile, "{ Envelope\n");
fprintf(LWFile, "  1\n");
fprintf(LWFile, "  Key %g 0 0 0 0 0 0 0 0\n", (CurInstance->ExportXYZ[0]) * RefData->ExportLonScale);
fprintf(LWFile, "  Behaviors 1 1\n");
fprintf(LWFile, "}\n");

fprintf(LWFile, "Channel 1\n");
fprintf(LWFile, "{ Envelope\n");
fprintf(LWFile, "  1\n");
fprintf(LWFile, "  Key %g 0 0 0 0 0 0 0 0\n", (CurInstance->ExportXYZ[2]) * RefData->ElevScale);
fprintf(LWFile, "  Behaviors 1 1\n");
fprintf(LWFile, "}\n");

fprintf(LWFile, "Channel 2\n");
fprintf(LWFile, "{ Envelope\n");
fprintf(LWFile, "  1\n");
fprintf(LWFile, "  Key %g 0 0 0 0 0 0 0 0\n", (CurInstance->ExportXYZ[1]) * RefData->ExportLatScale);
fprintf(LWFile, "  Behaviors 1 1\n");
fprintf(LWFile, "}\n");


// rotation
fprintf(LWFile, "Channel 3\n");
fprintf(LWFile, "{ Envelope\n");
fprintf(LWFile, "  1\n");
fprintf(LWFile, "  Key %g 0 0 0 0 0 0 0 0\n", (CurInstance->Euler[1]) * PiOver180);	// heading
fprintf(LWFile, "  Behaviors 1 1\n");
fprintf(LWFile, "}\n");

fprintf(LWFile, "Channel 4\n");
fprintf(LWFile, "{ Envelope\n");
fprintf(LWFile, "  1\n");
fprintf(LWFile, "  Key %g 0 0 0 0 0 0 0 0\n", (CurInstance->Euler[0]) * PiOver180);	// pitch
fprintf(LWFile, "  Behaviors 1 1\n");
fprintf(LWFile, "}\n");

fprintf(LWFile, "Channel 5\n");
fprintf(LWFile, "{ Envelope\n");
fprintf(LWFile, "  1\n");
fprintf(LWFile, "  Key %g 0 0 0 0 0 0 0 0\n", (CurInstance->Euler[2]) * PiOver180);	// bank
fprintf(LWFile, "  Behaviors 1 1\n");
fprintf(LWFile, "}\n");

// scale
if ((CurInstance->Flags & WCS_OBJECT3DINSTANCE_FLAGBIT_HVSPRITE) || (CurInstance->Flags & WCS_OBJECT3DINSTANCE_FLAGBIT_HDINSTANCE))
	{
	// for sprites, the scale values are the width and max height of the sprite. Height is in Scale[1].
	fprintf(LWFile, "Channel 6\n");
	fprintf(LWFile, "{ Envelope\n");
	fprintf(LWFile, "  1\n");
	fprintf(LWFile, "  Key 1 0 0 0 0 0 0 0 0\n");
	fprintf(LWFile, "  Behaviors 1 1\n");
	fprintf(LWFile, "}\n");

	fprintf(LWFile, "Channel 7\n");
	fprintf(LWFile, "{ Envelope\n");
	fprintf(LWFile, "  1\n");
	fprintf(LWFile, "  Key 1 0 0 0 0 0 0 0 0\n");
	fprintf(LWFile, "  Behaviors 1 1\n");
	fprintf(LWFile, "}\n");

	fprintf(LWFile, "Channel 8\n");
	fprintf(LWFile, "{ Envelope\n");
	fprintf(LWFile, "  1\n");
	fprintf(LWFile, "  Key 1 0 0 0 0 0 0 0 0\n");
	fprintf(LWFile, "  Behaviors 1 1\n");
	fprintf(LWFile, "}\n");
	} // if
else
	{
	fprintf(LWFile, "Channel 6\n");
	fprintf(LWFile, "{ Envelope\n");
	fprintf(LWFile, "  1\n");
	fprintf(LWFile, "  Key %g 0 0 0 0 0 0 0 0\n", CurInstance->Scale[0]);
	fprintf(LWFile, "  Behaviors 1 1\n");
	fprintf(LWFile, "}\n");

	fprintf(LWFile, "Channel 7\n");
	fprintf(LWFile, "{ Envelope\n");
	fprintf(LWFile, "  1\n");
	fprintf(LWFile, "  Key %g 0 0 0 0 0 0 0 0\n", CurInstance->Scale[1]);
	fprintf(LWFile, "  Behaviors 1 1\n");
	fprintf(LWFile, "}\n");

	fprintf(LWFile, "Channel 8\n");
	fprintf(LWFile, "{ Envelope\n");
	fprintf(LWFile, "  1\n");
	fprintf(LWFile, "  Key %g 0 0 0 0 0 0 0 0\n", CurInstance->Scale[2]);
	fprintf(LWFile, "  Behaviors 1 1\n");
	fprintf(LWFile, "}\n");
	} // else

fprintf(LWFile, "\n");
if (CurInstance->Flags & WCS_OBJECT3DINSTANCE_FLAGBIT_ALIGNTOCAMERA)
	{
	fprintf(LWFile, "HController 1\n");
	fprintf(LWFile, "PController 1\n");
	TargetNum = (0x10000000 | TargetObjNum);	// align to foliage target NULL object
	fprintf(LWFile, "TargetItem %8X\n", TargetNum);
	} // if
if (CurInstance->Flags & WCS_OBJECT3DINSTANCE_FLAGBIT_CLIPMAP)
	{
	fprintf(LWFile, "\
ClipMaps\n\
{ TextureBlock\n\
  { Texture\n\
    { ImageMap\n\
      \"(-128)\"\n\
      Channel 1954051186\n\
      { Opacity\n\
        0\n\
        1\n\
        0\n\
      }\n\
      Enable 1\n\
      Negative 1\n\
    }\n");

	fprintf(LWFile, "\
    { TextureMap\n\
      { Center\n\
        0 0 0\n\
        0\n\
      }\n\
      { Size\n\
        1 1 1\n\
        0\n\
      }\n\
      { Rotation\n\
        0 0 0\n\
        0\n\
      }\n\
      { Falloff\n\
        0\n\
        0 0 0\n\
        0\n\
      }\n\
      RefObject \"(none)\"\n\
      Coordinates 0\n\
    }\n");

	fprintf(LWFile, "\
    Projection 5\n\
    Axis 2\n\
    { Image\n\
      { Clip\n\
        { Still\n");

	fprintf(LWFile, "\
          \"%s\"\n\
        }\n\
        Times 0 0.029999999 30 0 0\n\
        AlphaMode 2\n\
      }\n\
    }\n", FullImageName);

	fprintf(LWFile, "\
    WrapOptions 1 1\n\
    { WidthWrap\n\
      1\n\
      0\n\
    }\n\
    { HeightWrap\n\
      1\n\
      0\n\
    }\n\
    VertexMap \"%s\"\n\
    AntiAliasing 1 1\n\
    PixelBlending 1\n\
  }\n\
}\n", VertMapName);
	} // if
if (CurInstance->Flags & WCS_OBJECT3DINSTANCE_FLAGBIT_HVSPRITE)
	{
	fprintf(LWFile, "\
Plugin CustomObjHandler 1 HyperVoxelsDrawing\n\
EndPlugin\n\
Plugin DisplacementHandler 1 HyperVoxelsParticles\n\
EndPlugin\n");
	} // if 

if (CurInstance->Flags & WCS_OBJECT3DINSTANCE_FLAGBIT_HDINSTANCE)
	{
	ObjectToClone = 0x10000000 | HDInstanceCt;
	strcpy(FullFileName, VertMapName);
	if (FullFileName[strlen(FullFileName) - 1] == 'i')
		FullFileName[strlen(FullFileName) - 1] = 0;
	fprintf(LWFile, "\
Plugin CustomObjHandler 1 HD_Instance\n\
{ PLUG\n\
  { EMIT\n\
    1229870166\n\
    0\n\
  }\n\
  { EMOB\n\
    1162694466\n\
    1600\n\
    1\n\
    1\n\
    \"\"\n\
    %d\n\
    0\n\
    2\n\
    0\n\
    0\n\
    0\n\
    0\n\
    0\n\
    0\n\
    %d\n\
    1\n\
    1\n\
    1\n\
    1\n\
    1\n\
    1\n\
    1\n\
    \"%s\"\n\
  }\n\
  { PREV\n\
    1229870166\n\
    2\n\
  }\n\
  { SEED\n\
    1229870166\n\
    %d\n\
  }\n\
}\n\
EndPlugin\n", ObjectToClone, 0x10000000 | TargetObjNum, VertMapName, 4101 + HDInstanceCt);
	} // if 

if (CurInstance->Flags & WCS_OBJECT3DINSTANCE_FLAGBIT_UNSEENBYCAMERA)
	{
	fprintf(LWFile, "UnseenByCamera 1\n");
	} // if

fprintf(LWFile, "ShadowOptions 7\n");
if (NumLights && Object3D->AllMaterialsLuminous())
	{
	for (LightCt = 0; LightCt < NumLights; LightCt ++)
		{
		fprintf(LWFile, "ExcludeLight 2000%04d\n", LightCt);
		} // for
	} // if
fprintf(LWFile, "\n");

return (Success);

} // ExportFormatLW::SaveObjectMotion

/*===========================================================================*/

int ExportFormatLW::ExportLights(FormatSpecificFile CurFile, EffectList *LightList, Atmosphere *CurHaze)
{
double KeyValue, KeyTime, Tension, Continuity, Bias, FrameRate, Elev, X, Y;
long Channel, KeyFrames, LastKeyFrame, NextKeyFrame, CurKey, LW7NumKeys, LW7Ct, SpanType, LightType, LightFalloffType, ShadowType;
FILE *LWFile = (FILE *)CurFile;
EffectList *CurLightList;
Light *CurLight;
AnimDoubleTime *ADT;
GraphNode *GrNode;
float *LW7Data[WCS_LW7_DATA_NUMCHANNELS];
int Success = 1;
RasterAnimHostProperties Prop;
RenderData RendData(NULL);

// ambient light
if (CurHaze)
	{
	fprintf(LWFile, "AmbientColor %g %g %g\n", CurHaze->TopAmbientColor.GetCurValue(0), CurHaze->TopAmbientColor.GetCurValue(1), CurHaze->TopAmbientColor.GetCurValue(2));
	fprintf(LWFile, "AmbientIntensity %g\n", CurHaze->TopAmbientColor.GetIntensity());
	fprintf(LWFile, "\n");
	} // if

// if lights
if (LightList && Master->ExportLights)
	{
	if (RendData.InitToView(EffectsHost, ProjectHost, DBHost, ProjectHost->Interactive, NULL, NULL, 320, 320))
		{
		FrameRate = GlobalApp->MainProj->Interactive->GetFrameRate();

		// for each light
		for (CurLightList = LightList; CurLightList; CurLightList = CurLightList->Next)
			{
			if (CurLight = (Light *)CurLightList->Me)
				{
				fprintf(LWFile, "AddLight\n");
				fprintf(LWFile, "LightName %s\n", CurLight->GetName());
				fprintf(LWFile, "ShowLight 1 5\n");
				fprintf(LWFile, "LightMotion\n");
				fprintf(LWFile, "NumChannels 9\n");

				// if position keys
				Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
				Prop.ItemOperator = WCS_KEYOPERATION_CUROBJGROUP;
				LastKeyFrame = -2;
				KeyFrames = 0;

				while (1)	//lint !e716
					{
					NextKeyFrame = -1;
					Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / FrameRate;
					Prop.NewKeyNodeRange[0] = -DBL_MAX;
					Prop.NewKeyNodeRange[1] = DBL_MAX;
					CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetRAHostProperties(&Prop);
					if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
						NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * FrameRate + .5);
					if (NextKeyFrame > LastKeyFrame && NextKeyFrame >= 0)
						{
						KeyFrames ++;
						LastKeyFrame = NextKeyFrame;
						} // if
					else
						break;
					} // while

				LW7NumKeys = (KeyFrames > 0) ? KeyFrames: 1;
				if (LW7Data[0] = (float *)AppMem_Alloc(WCS_LW7_DATA_NUMCHANNELS * LW7NumKeys * sizeof (float), APPMEM_CLEAR))
					{
					LW7Data[1] = &LW7Data[0][LW7NumKeys];
					LW7Data[2] = &LW7Data[0][LW7NumKeys * 2];
					LW7Data[9] = &LW7Data[0][LW7NumKeys * 9];
					LW7Data[10] = &LW7Data[0][LW7NumKeys * 10];
					LW7Data[11] = &LW7Data[0][LW7NumKeys * 11];
					LW7Data[12] = &LW7Data[0][LW7NumKeys * 12];
					LW7Data[13] = &LW7Data[0][LW7NumKeys * 13];
					} // if
				else
					{
					Success = 0;
					break;
					} // else

				if (KeyFrames)
					{
					LastKeyFrame = -2;
					LW7Ct = 0;
					while (1)	//lint !e716
						{
						NextKeyFrame = -1;
						Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / FrameRate;
						Prop.NewKeyNodeRange[0] = -DBL_MAX;
						Prop.NewKeyNodeRange[1] = DBL_MAX;
						CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetRAHostProperties(&Prop);
						if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
							NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * FrameRate + .5);
						if (NextKeyFrame > LastKeyFrame && NextKeyFrame >= 0)
							{
							if (GrNode = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].FindNearestSiblingNode(NextKeyFrame / FrameRate, NULL))
								{
								RendData.RenderTime = NextKeyFrame / FrameRate;
								CurLight->SetToTime(RendData.RenderTime);
								CurLight->InitFrameToRender(GlobalApp->AppEffects, &RendData);

								Elev = min(5000000.0, CurLight->LightPos->Elev);

								// convert WCS coords into export coords
								Master->RBounds.DefDegToRBounds(CurLight->LightPos->Lat, CurLight->LightPos->Lon, X, Y);
								LW7Data[0][LW7Ct] = (float)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale);
								LW7Data[2][LW7Ct] = (float)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
								LW7Data[1][LW7Ct] = (float)((Elev - Master->ExportRefData.RefElev) * Master->ExportRefData.ElevScale);
								LW7Data[9][LW7Ct] = (float)GrNode->TCB[0];
								LW7Data[10][LW7Ct] = (float)GrNode->TCB[1];
								LW7Data[11][LW7Ct] = (float)GrNode->TCB[2];
								LW7Data[12][LW7Ct] = (float)NextKeyFrame;
								LW7Data[13][LW7Ct] = (float)GrNode->Linear;
								LW7Ct ++;
								LastKeyFrame = NextKeyFrame;
								} // if
							else
								break;
							} // if
						else
							break;
						} // while

					// write position channels
					for (Channel = 0; Channel < 3; Channel ++)
						{
						fprintf(LWFile, "Channel %d\n", Channel);
						fprintf(LWFile, "{ Envelope\n");
						fprintf(LWFile, "  %d\n", LW7NumKeys);
						for (int LW7CtWrite = 0; LW7CtWrite < LW7Ct; LW7CtWrite ++)
							{
							if(LW7Data[13][LW7CtWrite] != 0.0)
								{ // linear
								fprintf(LWFile, "  Key %g %g 3 0 0 0 0 0 0\n", LW7Data[Channel][LW7CtWrite], LW7Data[12][LW7CtWrite] / FrameRate);
								} // if
							else
								{ // TCB spline
								fprintf(LWFile, "  Key %g %g 0 %g %g %g 0 0 0\n", LW7Data[Channel][LW7CtWrite], LW7Data[12][LW7CtWrite] / FrameRate, LW7Data[9][LW7CtWrite], LW7Data[10][LW7CtWrite], LW7Data[11][LW7CtWrite]);
								} // else
							} // for
						fprintf(LWFile, "  Behaviors 1 1\n");
						fprintf(LWFile, "}\n");
						} // for
					} // if position keys
				else
					{
					RendData.RenderTime = 0.0;
					CurLight->SetToTime(RendData.RenderTime);
					CurLight->InitFrameToRender(GlobalApp->AppEffects, &RendData);

					Elev = min(5000000.0, CurLight->LightPos->Elev);

					// convert WCS coords into export coords
					Master->RBounds.DefDegToRBounds(CurLight->LightPos->Lat, CurLight->LightPos->Lon, X, Y);
					LW7Data[0][0] = (float)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale);
					LW7Data[2][0] = (float)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
					LW7Data[1][0] = (float)((Elev - Master->ExportRefData.RefElev) * Master->ExportRefData.ElevScale);

					// write position channels
					for (Channel = 0; Channel < 3; Channel ++)
						{
						fprintf(LWFile, "Channel %d\n", Channel);
						fprintf(LWFile, "{ Envelope\n");
						fprintf(LWFile, "  1\n");
						fprintf(LWFile, "  Key %g 0 0 0 0 0 0 0 0\n", LW7Data[Channel][0]);
						fprintf(LWFile, "  Behaviors 1 1\n");
						fprintf(LWFile, "}\n");
						} // for
					} // else

				// now free our array
				AppMem_Free(LW7Data[0], WCS_LW7_DATA_NUMCHANNELS * LW7NumKeys * sizeof (float));
				LW7Data[0] = NULL;
				LW7Ct = LW7NumKeys = 0;

				LastKeyFrame = -2;
				KeyFrames = 0;

				while (1)	//lint !e716
					{
					NextKeyFrame = -1;
					Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / FrameRate;
					Prop.NewKeyNodeRange[0] = -DBL_MAX;
					Prop.NewKeyNodeRange[1] = DBL_MAX;
					CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].GetRAHostProperties(&Prop);
					if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
						NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * FrameRate + .5);
					if (NextKeyFrame > LastKeyFrame && NextKeyFrame >= 0)
						{
						KeyFrames ++;
						LastKeyFrame = NextKeyFrame;
						} // if
					else
						break;
					} // while

				LW7NumKeys = (KeyFrames > 0) ? KeyFrames: 1;
				if (LW7Data[0] = (float *)AppMem_Alloc(WCS_LW7_DATA_NUMCHANNELS * LW7NumKeys * sizeof (float), APPMEM_CLEAR))
					{
					LW7Data[3] = &LW7Data[0][LW7NumKeys * 3];
					LW7Data[4] = &LW7Data[0][LW7NumKeys * 4];
					LW7Data[9] = &LW7Data[0][LW7NumKeys * 9];
					LW7Data[10] = &LW7Data[0][LW7NumKeys * 10];
					LW7Data[11] = &LW7Data[0][LW7NumKeys * 11];
					LW7Data[12] = &LW7Data[0][LW7NumKeys * 12];
					LW7Data[13] = &LW7Data[0][LW7NumKeys * 13];
					} // if
				else
					{
					Success = 0;
					break;
					} // else

				if (KeyFrames)
					{
					LastKeyFrame = -2;
					LW7Ct = 0;
					while (1)	//lint !e716
						{
						NextKeyFrame = -1;
						Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / FrameRate;
						Prop.NewKeyNodeRange[0] = -DBL_MAX;
						Prop.NewKeyNodeRange[1] = DBL_MAX;
						CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].GetRAHostProperties(&Prop);
						if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
							NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * FrameRate + .5);
						if (NextKeyFrame > LastKeyFrame && NextKeyFrame >= 0)
							{
							if (GrNode = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].FindNearestSiblingNode(NextKeyFrame / FrameRate, NULL))
								{
								RendData.RenderTime = NextKeyFrame / FrameRate;
								CurLight->SetToTime(RendData.RenderTime);
								CurLight->InitFrameToRender(GlobalApp->AppEffects, &RendData);

								LW7Data[3][LW7Ct] = (float)(CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].CurValue * PiOver180);
								LW7Data[4][LW7Ct] = (float)(CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_PITCH].CurValue * PiOver180);
								LW7Data[9][LW7Ct] = (float)GrNode->TCB[0];
								LW7Data[10][LW7Ct] = (float)GrNode->TCB[1];
								LW7Data[11][LW7Ct] = (float)GrNode->TCB[2];
								LW7Data[12][LW7Ct] = (float)NextKeyFrame;
								LW7Data[13][LW7Ct] = (float)GrNode->Linear;
								LW7Ct ++;
								LastKeyFrame = NextKeyFrame;
								} // if
							else
								break;
							} // if
						else
							break;
						} // while
					// write orientation channels
					for (Channel = 3; Channel < 5; Channel ++)
						{
						fprintf(LWFile, "Channel %d\n", Channel);
						fprintf(LWFile, "{ Envelope\n");
						fprintf(LWFile, "  %d\n", LW7NumKeys);
						for (int LW7CtWrite = 0; LW7CtWrite < LW7Ct; LW7CtWrite ++)
							{
							if(LW7Data[13][LW7CtWrite] != 0.0)
								{ // linear
								fprintf(LWFile, "  Key %g %g 3 0 0 0 0 0 0\n", LW7Data[Channel][LW7CtWrite], LW7Data[12][LW7CtWrite] / FrameRate);
								} // if
							else
								{ // TCB spline
								fprintf(LWFile, "  Key %g %g 0 %g %g %g 0 0 0\n", LW7Data[Channel][LW7CtWrite], LW7Data[12][LW7CtWrite] / FrameRate, LW7Data[9][LW7CtWrite], LW7Data[10][LW7CtWrite], LW7Data[11][LW7CtWrite]);
								} // else
							} // for
						fprintf(LWFile, "  Behaviors 1 1\n");
						fprintf(LWFile, "}\n");
						} // for
					} // if keyframes
				else
					{
					RendData.RenderTime = 0.0;
					CurLight->SetToTime(RendData.RenderTime);
					CurLight->InitFrameToRender(GlobalApp->AppEffects, &RendData);
					LW7Data[3][0] = (float)(CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].CurValue * PiOver180);
					LW7Data[4][0] = (float)(CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_PITCH].CurValue * PiOver180);

					// write position channels
					for (Channel = 3; Channel < 5; Channel ++)
						{
						fprintf(LWFile, "Channel %d\n", Channel);
						fprintf(LWFile, "{ Envelope\n");
						fprintf(LWFile, "  1\n");
						fprintf(LWFile, "  Key %g 0 0 0 0 0 0 0 0\n", LW7Data[Channel][0]);
						fprintf(LWFile, "  Behaviors 1 1\n");
						fprintf(LWFile, "}\n");
						} // for
					} // else

				// now free our array
				AppMem_Free(LW7Data[0], WCS_LW7_DATA_NUMCHANNELS * LW7NumKeys * sizeof (float));
				LW7Data[0] = NULL;
				LW7Ct = LW7NumKeys = 0;

				// write bank, scale channels
				for (Channel = 5; Channel < 9; Channel ++)
					{
					fprintf(LWFile, "Channel %d\n", Channel);
					fprintf(LWFile, "{ Envelope\n");
					fprintf(LWFile, "  1\n");
					fprintf(LWFile, "  Key 1 0 0 0 0 0 0 0 0\n");
					fprintf(LWFile, "  Behaviors 1 1\n");
					fprintf(LWFile, "}\n");
					} // for

				fprintf(LWFile, "\n");
				fprintf(LWFile, "LightColor %g %g %g\n", CurLight->Color.GetCurValue(0),
					CurLight->Color.GetCurValue(1), CurLight->Color.GetCurValue(2));
				fprintf(LWFile, "LightIntensity %g\n", CurLight->Color.Intensity.CurValue);
				fprintf(LWFile, "AffectCaustics 1\n");
				// Distant lights (type = 0) seem to be in the wrong place in LW. Point lights work better.
				//LightType = (CurLight->LightType == WCS_EFFECTS_LIGHTTYPE_PARALLEL && CurLight->Distant) ? 0:
				//	(CurLight->LightType  == WCS_EFFECTS_LIGHTTYPE_SPOT) ? 2: 1;
				LightType = (CurLight->LightType  == WCS_EFFECTS_LIGHTTYPE_SPOT) ? 2: 1;
				fprintf(LWFile, "LightType %d\n", LightType);
				// falloff
				LightFalloffType = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_FALLOFFEXP].CurValue > 0.0 ? 3: 0;	//  Inverse Squared: (r / d)2 or None
				fprintf(LWFile, "LightFalloffType %d\n", LightFalloffType);
				// spotlight angles
				if (CurLight->LightType  == WCS_EFFECTS_LIGHTTYPE_SPOT)
					{
					fprintf(LWFile, "LightConeAngle (envelope)\n");
					fprintf(LWFile, "{ Envelope\n");
					ADT = CurLight->GetAnimPtr(WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE);
					LW7NumKeys = ADT->GetNumNodes(0);
					LW7NumKeys = LW7NumKeys > 1 ? LW7NumKeys: 1;
					fprintf(LWFile, "  %d\n", LW7NumKeys);
					for (CurKey = 0, GrNode = ADT->GetFirstNode(0); CurKey < LW7NumKeys; CurKey ++, GrNode = ADT->GetNextNode(0, GrNode))
						{
						if (GrNode)
							{
							KeyValue = GrNode->Value * PiOver180;
							KeyTime = GrNode->Distance;
							Tension = GrNode->Linear ? 0.0: GrNode->TCB[0];
							Continuity = GrNode->Linear ? 0.0: GrNode->TCB[1];
							Bias = GrNode->Linear ? 0.0: GrNode->TCB[2];
							SpanType = GrNode->Linear ? 3: 0;
							} // if
						else
							{
							KeyValue = ADT->CurValue * PiOver180;
							KeyTime = 0.0;
							Tension = Continuity = Bias = 0.0;
							SpanType = 0;
							} // else
						fprintf(LWFile, "  Key %g %g %d %g %g %g 0 0 0\n", KeyValue, KeyTime, SpanType,
							Tension, Continuity, Bias);
						} // for
					fprintf(LWFile, "  Behaviors 1 1\n");
					fprintf(LWFile, "}\n");
					fprintf(LWFile, "LightEdgeAngle %g\n", CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONEEDGE].CurValue);
					fprintf(LWFile, "UseConeAngle 1\n");
					} // if
				ShadowType = CurLight->CastShadows ? 1: 0;	// 0=no shadows, 1=ray trace, 2=shadow map don't know what this means
				fprintf(LWFile, "ShadowType %d\n", ShadowType);
				fprintf(LWFile, "ShadowColor 0 0 0\n");
				if (ShadowType == 2)
					{
					fprintf(LWFile, "ShadowMapSize %d\n", 512);		// kind of arbitrary since our shadow map sizes are not determined in the light itself
					fprintf(LWFile, "ShadowFuzziness 1\n");
					} // if
				fprintf(LWFile, "\n");
				} // if
			} // for CurLight
		} // if RendData Init
	else
		Success = 0;
	} // if lights

return (Success);

} // ExportFormatLW::ExportLight

/*===========================================================================*/

int ExportFormatLW::ExportCameras(FormatSpecificFile CurFile, EffectList *CameraList)
{
double KeyValue, KeyTime, Tension, Continuity, Bias, FrameRate, X, Y;
long Channel, KeyFrames, LastKeyFrame, NextKeyFrame, CurKey, LW7NumKeys, LW7Ct, SpanType, AALevel, TargetNum, Width, Height, CamNum = 0;
FILE *LWFile = (FILE *)CurFile;
RenderJob *CurRJ;
RenderOpt *CurOpt;
EffectList *CurCamList;
Camera *CurCam;
AnimDoubleTime *ADT, *TargetADT;
GraphNode *GrNode;
float *LW7Data[WCS_LW7_DATA_NUMCHANNELS];
int Success = 1;
RasterAnimHostProperties Prop;
RenderData RendData(NULL);

// if cameras
if (CameraList && Master->ExportCameras)
	{
	if (RendData.InitToView(EffectsHost, ProjectHost, DBHost, ProjectHost->Interactive, NULL, NULL, 320, 320))
		{
		FrameRate = GlobalApp->MainProj->Interactive->GetFrameRate();

		// for each camera
		for (CurCamList = CameraList; CurCamList; CurCamList = CurCamList->Next)
			{
			if (CurCam = (Camera *)CurCamList->Me)
				{
				// search for a matching RenderOpt to get additional settings from
				CurOpt = NULL;
				// for each render job, does the camera match
				for (CurRJ = (RenderJob *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_RENDERJOB); CurRJ;
					CurRJ = (RenderJob *)CurRJ->Next)
					{
					if (CurRJ->Cam == CurCam)
						{
						CurOpt = CurRJ->Options;
						break;
						} // if
					} // for
				// write camera values
				fprintf(LWFile, "AddCamera\n");
				fprintf(LWFile, "CameraName %s\n", CurCam->GetName());
				fprintf(LWFile, "ShowCamera 1 2\n");
				fprintf(LWFile, "CameraMotion\n");
				fprintf(LWFile, "NumChannels 6\n");

				// if position keys
				Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
				Prop.ItemOperator = WCS_KEYOPERATION_CUROBJGROUP;
				LastKeyFrame = -2;
				KeyFrames = 0;

				while (1)	//lint !e716
					{
					NextKeyFrame = -1;
					Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / FrameRate;
					Prop.NewKeyNodeRange[0] = -DBL_MAX;
					Prop.NewKeyNodeRange[1] = DBL_MAX;
					CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetRAHostProperties(&Prop);
					if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
						NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * FrameRate + .5);
					if (NextKeyFrame > LastKeyFrame && NextKeyFrame >= 0)
						{
						KeyFrames ++;
						LastKeyFrame = NextKeyFrame;
						} // if
					else
						break;
					} // while

				LW7NumKeys = (KeyFrames > 0) ? KeyFrames: 1;
				if (LW7Data[0] = (float *)AppMem_Alloc(WCS_LW7_DATA_NUMCHANNELS * LW7NumKeys * sizeof (float), APPMEM_CLEAR))
					{
					LW7Data[1] = &LW7Data[0][LW7NumKeys];
					LW7Data[2] = &LW7Data[0][LW7NumKeys * 2];
					LW7Data[9] = &LW7Data[0][LW7NumKeys * 9];
					LW7Data[10] = &LW7Data[0][LW7NumKeys * 10];
					LW7Data[11] = &LW7Data[0][LW7NumKeys * 11];
					LW7Data[12] = &LW7Data[0][LW7NumKeys * 12];
					LW7Data[13] = &LW7Data[0][LW7NumKeys * 13];
					} // if
				else
					{
					Success = 0;
					break;
					} // else

				if (KeyFrames)
					{
					LastKeyFrame = -2;
					LW7Ct = 0;
					while (1)	//lint !e716
						{
						NextKeyFrame = -1;
						Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / FrameRate;
						Prop.NewKeyNodeRange[0] = -DBL_MAX;
						Prop.NewKeyNodeRange[1] = DBL_MAX;
						CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetRAHostProperties(&Prop);
						if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
							NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * FrameRate + .5);
						if (NextKeyFrame > LastKeyFrame && NextKeyFrame >= 0)
							{
							if (GrNode = CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].FindNearestSiblingNode(NextKeyFrame / FrameRate, NULL))
								{
								RendData.RenderTime = NextKeyFrame / FrameRate;
								CurCam->SetToTime(RendData.RenderTime);
								if (CurCam->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED && CurCam->TargetObj)
									CurCam->TargetObj->SetToTime(RendData.RenderTime);
								CurCam->InitFrameToRender(GlobalApp->AppEffects, &RendData);

								// convert WCS coords into export coords
								Master->RBounds.DefDegToRBounds(CurCam->CamPos->Lat, CurCam->CamPos->Lon, X, Y);
								LW7Data[0][LW7Ct] = (float)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale);
								LW7Data[2][LW7Ct] = (float)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
								LW7Data[1][LW7Ct] = (float)((CurCam->CamPos->Elev - Master->ExportRefData.RefElev) * Master->ExportRefData.ElevScale);
								LW7Data[9][LW7Ct] = (float)GrNode->TCB[0];
								LW7Data[10][LW7Ct] = (float)GrNode->TCB[1];
								LW7Data[11][LW7Ct] = (float)GrNode->TCB[2];
								LW7Data[12][LW7Ct] = (float)NextKeyFrame;
								LW7Data[13][LW7Ct] = (float)GrNode->Linear;
								LW7Ct ++;
								LastKeyFrame = NextKeyFrame;
								} // if
							else
								break;
							} // if
						else
							break;
						} // while

					// write position channels
					for (Channel = 0; Channel < 3; Channel ++)
						{
						fprintf(LWFile, "Channel %d\n", Channel);
						fprintf(LWFile, "{ Envelope\n");
						fprintf(LWFile, "  %d\n", LW7NumKeys);
						for (int LW7CtWrite = 0; LW7CtWrite < LW7Ct; LW7CtWrite ++)
							{
							if(LW7Data[13][LW7CtWrite] != 0.0)
								{ // linear
								fprintf(LWFile, "  Key %g %g 3 0 0 0 0 0 0\n", LW7Data[Channel][LW7CtWrite], LW7Data[12][LW7CtWrite] / FrameRate);
								} // if
							else
								{ // TCB spline
								fprintf(LWFile, "  Key %g %g 0 %g %g %g 0 0 0\n", LW7Data[Channel][LW7CtWrite], LW7Data[12][LW7CtWrite] / FrameRate, LW7Data[9][LW7CtWrite], LW7Data[10][LW7CtWrite], LW7Data[11][LW7CtWrite]);
								} // else
							} // for
						fprintf(LWFile, "  Behaviors 1 1\n");
						fprintf(LWFile, "}\n");
						} // for
					} // if position keys
				else
					{
					RendData.RenderTime = 0.0;
					CurCam->SetToTime(RendData.RenderTime);
					if (CurCam->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED && CurCam->TargetObj)
						CurCam->TargetObj->SetToTime(RendData.RenderTime);
					CurCam->InitFrameToRender(GlobalApp->AppEffects, &RendData);

					// convert WCS coords into export coords
					Master->RBounds.DefDegToRBounds(CurCam->CamPos->Lat, CurCam->CamPos->Lon, X, Y);
					LW7Data[0][0] = (float)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale);
					LW7Data[2][0] = (float)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
					LW7Data[1][0] = (float)((CurCam->CamPos->Elev - Master->ExportRefData.RefElev) * Master->ExportRefData.ElevScale);

					// write position channels
					for (Channel = 0; Channel < 3; Channel ++)
						{
						fprintf(LWFile, "Channel %d\n", Channel);
						fprintf(LWFile, "{ Envelope\n");
						fprintf(LWFile, "  1\n");
						fprintf(LWFile, "  Key %g 0 0 0 0 0 0 0 0\n", LW7Data[Channel][0]);
						fprintf(LWFile, "  Behaviors 1 1\n");
						fprintf(LWFile, "}\n");
						} // for
					} // else

				// now free our array
				AppMem_Free(LW7Data[0], WCS_LW7_DATA_NUMCHANNELS * LW7NumKeys * sizeof (float));
				LW7Data[0] = NULL;
				LW7Ct = LW7NumKeys = 0;

				// HPB
				// if a non-targeted camera, just read HPB directly
				// if targeted and not 3d object targeted, just read HPB directly
				// if targeted and 3d object targeted, just read HPB directly but be sure 3d object NULL is correct
				// if aligned, use camera's HPB values and see how it works
				LastKeyFrame = -2;
				KeyFrames = 0;

				if (CurCam->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED || CurCam->CameraType == WCS_EFFECTS_CAMERATYPE_UNTARGETED)
					TargetADT = &CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING];
				else
					TargetADT = &CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT];

				while (1)	//lint !e716
					{
					NextKeyFrame = -1;
					Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / FrameRate;
					Prop.NewKeyNodeRange[0] = -DBL_MAX;
					Prop.NewKeyNodeRange[1] = DBL_MAX;
					TargetADT->GetRAHostProperties(&Prop);

					if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
						NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * FrameRate + .5);
					if (NextKeyFrame > LastKeyFrame && NextKeyFrame >= 0)
						{
						KeyFrames ++;
						LastKeyFrame = NextKeyFrame;
						} // if
					else
						break;
					} // while

				LW7NumKeys = (KeyFrames > 0) ? KeyFrames: 1;
				if (LW7Data[0] = (float *)AppMem_Alloc(WCS_LW7_DATA_NUMCHANNELS * LW7NumKeys * sizeof (float), APPMEM_CLEAR))
					{
					LW7Data[3] = &LW7Data[0][LW7NumKeys * 3];
					LW7Data[4] = &LW7Data[0][LW7NumKeys * 4];
					LW7Data[5] = &LW7Data[0][LW7NumKeys * 5];
					LW7Data[9] = &LW7Data[0][LW7NumKeys * 9];
					LW7Data[10] = &LW7Data[0][LW7NumKeys * 10];
					LW7Data[11] = &LW7Data[0][LW7NumKeys * 11];
					LW7Data[12] = &LW7Data[0][LW7NumKeys * 12];
					LW7Data[13] = &LW7Data[0][LW7NumKeys * 13];
					} // if
				else
					{
					Success = 0;
					break;
					} // else

				if (KeyFrames)
					{
					LastKeyFrame = -2;
					LW7Ct = 0;
					while (1)	//lint !e716
						{
						NextKeyFrame = -1;
						Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / FrameRate;
						Prop.NewKeyNodeRange[0] = -DBL_MAX;
						Prop.NewKeyNodeRange[1] = DBL_MAX;
						TargetADT->GetRAHostProperties(&Prop);
						if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
							NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * FrameRate + .5);
						if (NextKeyFrame > LastKeyFrame && NextKeyFrame >= 0)
							{
							if (GrNode = TargetADT->FindNearestSiblingNode(NextKeyFrame / FrameRate, NULL))
								{
								RendData.RenderTime = NextKeyFrame / FrameRate;
								CurCam->SetToTime(RendData.RenderTime);
								if (CurCam->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED && CurCam->TargetObj)
									CurCam->TargetObj->SetToTime(RendData.RenderTime);
								CurCam->InitFrameToRender(GlobalApp->AppEffects, &RendData);

								if (CurCam->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED || CurCam->CameraType == WCS_EFFECTS_CAMERATYPE_UNTARGETED)
									{
									LW7Data[3][LW7Ct] = (float)(CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].CurValue * PiOver180);
									LW7Data[4][LW7Ct] = (float)(CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].CurValue * PiOver180);
									LW7Data[5][LW7Ct] = (float)(CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].CurValue * PiOver180);
									} // if
								else
									{
									LW7Data[3][LW7Ct] = (float)(CurCam->CamHeading * PiOver180);
									LW7Data[4][LW7Ct] = (float)(CurCam->CamPitch * PiOver180);
									LW7Data[5][LW7Ct] = (float)(CurCam->CamBank * PiOver180);
									} // else
								LW7Data[9][LW7Ct] = (float)GrNode->TCB[0];
								LW7Data[10][LW7Ct] = (float)GrNode->TCB[1];
								LW7Data[11][LW7Ct] = (float)GrNode->TCB[2];
								LW7Data[12][LW7Ct] = (float)NextKeyFrame;
								LW7Data[13][LW7Ct] = (float)GrNode->Linear;
								LW7Ct ++;
								LastKeyFrame = NextKeyFrame;
								} // if
							else
								break;
							} // if
						else
							break;
						} // while
					// write orientation channels
					for (Channel = 3; Channel < 6; Channel ++)
						{
						fprintf(LWFile, "Channel %d\n", Channel);
						fprintf(LWFile, "{ Envelope\n");
						fprintf(LWFile, "  %d\n", LW7NumKeys);
						for (int LW7CtWrite = 0; LW7CtWrite < LW7Ct; LW7CtWrite ++)
							{
							if(LW7Data[13][LW7CtWrite] != 0.0)
								{ // linear
								fprintf(LWFile, "  Key %g %g 3 0 0 0 0 0 0\n", LW7Data[Channel][LW7CtWrite], LW7Data[12][LW7CtWrite] / FrameRate);
								} // if
							else
								{ // TCB spline
								fprintf(LWFile, "  Key %g %g 0 %g %g %g 0 0 0\n", LW7Data[Channel][LW7CtWrite], LW7Data[12][LW7CtWrite] / FrameRate, LW7Data[9][LW7CtWrite], LW7Data[10][LW7CtWrite], LW7Data[11][LW7CtWrite]);
								} // else
							} // for
						fprintf(LWFile, "  Behaviors 1 1\n");
						fprintf(LWFile, "}\n");
						} // for
					} // if keyframes
				else
					{
					RendData.RenderTime = 0.0;
					CurCam->SetToTime(RendData.RenderTime);
					if (CurCam->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED && CurCam->TargetObj)
						CurCam->TargetObj->SetToTime(RendData.RenderTime);
					CurCam->InitFrameToRender(GlobalApp->AppEffects, &RendData);
					LW7Data[3][0] = (float)(CurCam->CamHeading * PiOver180);
					LW7Data[4][0] = (float)(CurCam->CamPitch * PiOver180);
					LW7Data[5][0] = (float)(CurCam->CamBank * PiOver180);

					// write position channels
					for (Channel = 3; Channel < 6; Channel ++)
						{
						fprintf(LWFile, "Channel %d\n", Channel);
						fprintf(LWFile, "{ Envelope\n");
						fprintf(LWFile, "  1\n");
						fprintf(LWFile, "  Key %g 0 0 0 0 0 0 0 0\n", LW7Data[Channel][0]);
						fprintf(LWFile, "  Behaviors 1 1\n");
						fprintf(LWFile, "}\n");
						} // for
					} // else

				// now free our array
				AppMem_Free(LW7Data[0], WCS_LW7_DATA_NUMCHANNELS * LW7NumKeys * sizeof (float));
				LW7Data[0] = NULL;
				LW7Ct = LW7NumKeys = 0;		
				
				fprintf(LWFile, "\n");

				// camera target
				if (CurCam->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED)
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
					ADT = CurCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV);
					LW7NumKeys = ADT->GetNumNodes(0);
					LW7NumKeys = LW7NumKeys > 1 ? LW7NumKeys: 1;
					fprintf(LWFile, "  %d\n", LW7NumKeys);
					for (CurKey = 0, GrNode = ADT->GetFirstNode(0); CurKey < LW7NumKeys; CurKey ++, GrNode = ADT->GetNextNode(0, GrNode))
						{
						if (GrNode)
							{
							KeyValue = (CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue * Width / Height) / 
								tan((GrNode->Value / 2.0) * PiOver180);
							KeyTime = GrNode->Distance;
							Tension = GrNode->Linear ? 0.0: GrNode->TCB[0];
							Continuity = GrNode->Linear ? 0.0: GrNode->TCB[1];
							Bias = GrNode->Linear ? 0.0: GrNode->TCB[2];
							SpanType = GrNode->Linear ? 3: 0;
							} // if
						else
							{
							KeyValue = (CurOpt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue * Width / Height) / 
								tan((ADT->CurValue / 2.0) * PiOver180);
							KeyTime = 0.0;
							Tension = Continuity = Bias = 0.0;
							SpanType = 0;
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
					ADT = CurCam->GetAnimPtr(WCS_EFFECTS_CAMERA_ANIMPAR_HFOV);
					LW7NumKeys = ADT->GetNumNodes(0);
					LW7NumKeys = LW7NumKeys > 1 ? LW7NumKeys: 1;
					fprintf(LWFile, "  %d\n", LW7NumKeys);
					for (CurKey = 0, GrNode = ADT->GetFirstNode(0); CurKey < LW7NumKeys; CurKey ++, GrNode = ADT->GetNextNode(0, GrNode))
						{
						if (GrNode)
							{
							KeyValue = (1.0 * (double)Width / Height) / 
								tan((GrNode->Value / 2.0) * PiOver180);
							KeyTime = GrNode->Distance;
							Tension = GrNode->Linear ? 0.0: GrNode->TCB[0];
							Continuity = GrNode->Linear ? 0.0: GrNode->TCB[1];
							Bias = GrNode->Linear ? 0.0: GrNode->TCB[2];
							SpanType = GrNode->Linear ? 3: 0;
							} // if
						else
							{
							KeyValue = (1.0 * (double)Width / Height) / 
								tan((ADT->CurValue / 2.0) * PiOver180);
							KeyTime = 0.0;
							Tension = Continuity = Bias = 0.0;
							SpanType = 0;
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
				fprintf(LWFile, "MotionBlur %d\n", CurCam->MotionBlur ? 2: 0);
				fprintf(LWFile, "FieldRendering %d\n", CurCam->FieldRender ? 1: 0);
				if (CurCam->FieldRender)
					fprintf(LWFile, "ReverseFields %d\n", CurCam->FieldRenderPriority ? 1: 0);
				if (CurCam->StereoCam)
					{
					fprintf(LWFile, "Stereoscopic 1\n");
					fprintf(LWFile, "EyeSeparation %g\n", CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_STEREOSEPARATION].CurValue);
					} // if
				if (CurCam->MotionBlur)
					fprintf(LWFile, "BlurLength %g\n", CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_MOBLURPERCENT].CurValue);
				if (CurCam->DepthOfField)
					{
					fprintf(LWFile, "DepthOfField 1\n");
					fprintf(LWFile, "FocalDistance %g\n", CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FOCALDIST].CurValue);
					fprintf(LWFile, "LensFStop %g\n", CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FSTOP].CurValue);
					} // if
				fprintf(LWFile, "\n");
				fprintf(LWFile, "ApertureHeight 0.015\n");
				AALevel = (CurCam->AAPasses <= 5) ? 1:
					(CurCam->AAPasses <= 9) ? 2:
					(CurCam->AAPasses <= 17) ? 3: 4;
				fprintf(LWFile, "Antialiasing %d\n", AALevel);
				fprintf(LWFile, "EnhancedAA 1\n");
				fprintf(LWFile, "AdaptiveSampling 1\n");
				fprintf(LWFile, "AdaptiveThreshold 0.1254902\n");
				fprintf(LWFile, "\n");
				} // if
			} // for CurCam
		} // if RendData Init
	else
		Success = 0;
	} // if cameras

return (Success);

} // ExportFormatLW::ExportCamera

/*===========================================================================*/

int ExportFormatLW::ExportCameraTargets(FormatSpecificFile CurFile, EffectList *CameraList, long &ObjectsWritten)
{
double X, Y, FrameRate, KeyValue;
long Channel, KeyFrames, LastKeyFrame, NextKeyFrame, LW7NumKeys, LW7Ct;
FILE *LWFile = (FILE *)CurFile;
EffectList *CurCamList;
Camera *CurCam;
GraphNode *GrNode;
float *LW7Data[WCS_LW7_DATA_NUMCHANNELS];
int Success = 1;
RasterAnimHostProperties Prop;
RenderData RendData(NULL);

// if cameras
if (CameraList && Master->ExportCameras)
	{
	if (RendData.InitToView(EffectsHost, ProjectHost, DBHost, ProjectHost->Interactive, NULL, NULL, 320, 320))
		{
		FrameRate = GlobalApp->MainProj->Interactive->GetFrameRate();

		// for each camera
		for (CurCamList = CameraList; CurCamList; CurCamList = CurCamList->Next)
			{
			if (CurCam = (Camera *)CurCamList->Me)
				{
				if (CurCam->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED)
					{
					// write camera values
					fprintf(LWFile, "AddNullObject %s Target\n", CurCam->GetName());
					fprintf(LWFile, "ShowObject 6 3\n");
					fprintf(LWFile, "ObjectMotion\n");
					fprintf(LWFile, "NumChannels 9\n");

					// if position keys
					Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
					Prop.ItemOperator = WCS_KEYOPERATION_CUROBJGROUP;
					LastKeyFrame = -2;
					KeyFrames = 0;

					while (1)	//lint !e716
						{
						NextKeyFrame = -1;
						Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / FrameRate;
						Prop.NewKeyNodeRange[0] = -DBL_MAX;
						Prop.NewKeyNodeRange[1] = DBL_MAX;
						CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetRAHostProperties(&Prop);
						if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
							NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * FrameRate + .5);
						if (NextKeyFrame > LastKeyFrame && NextKeyFrame >= 0)
							{
							KeyFrames ++;
							LastKeyFrame = NextKeyFrame;
							} // if
						else
							break;
						} // while

					LW7NumKeys = (KeyFrames > 0) ? KeyFrames: 1;
					if (LW7Data[0] = (float *)AppMem_Alloc(WCS_LW7_DATA_NUMCHANNELS * LW7NumKeys * sizeof (float), APPMEM_CLEAR))
						{
						LW7Data[1] = &LW7Data[0][LW7NumKeys];
						LW7Data[2] = &LW7Data[0][LW7NumKeys * 2];
						LW7Data[9] = &LW7Data[0][LW7NumKeys * 9];
						LW7Data[10] = &LW7Data[0][LW7NumKeys * 10];
						LW7Data[11] = &LW7Data[0][LW7NumKeys * 11];
						LW7Data[12] = &LW7Data[0][LW7NumKeys * 12];
						LW7Data[13] = &LW7Data[0][LW7NumKeys * 13];
						} // if
					else
						{
						Success = 0;
						break;
						} // else

					if (KeyFrames)
						{
						LastKeyFrame = -2;
						LW7Ct = 0;
						while (1)	//lint !e716
							{
							NextKeyFrame = -1;
							Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / FrameRate;
							Prop.NewKeyNodeRange[0] = -DBL_MAX;
							Prop.NewKeyNodeRange[1] = DBL_MAX;
							CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetRAHostProperties(&Prop);
							if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
								NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * FrameRate + .5);
							if (NextKeyFrame > LastKeyFrame && NextKeyFrame >= 0)
								{
								if (GrNode = CurCam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].FindNearestSiblingNode(NextKeyFrame / FrameRate, NULL))
									{
									RendData.RenderTime = NextKeyFrame / FrameRate;
									if (CurCam->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED && CurCam->TargetObj)
										CurCam->TargetObj->SetToTime(RendData.RenderTime);
									CurCam->SetToTime(RendData.RenderTime);
									// this gets the target position as a unit vector from the camera, not what we need
									//CurCam->InitFrameToRender(GlobalApp->AppEffects, &RendData);
									// a function that gets the actual target position in XYZ
									CurCam->GetTargetPosition(&RendData);

									// target position needs to be translated back into lat/lon
									#ifdef WCS_BUILD_VNS
									RendData.DefCoords->CartToDeg(CurCam->TargPos);
									#else // WCS_BUILD_VNS
									CurCam->TargPos->CartToDeg(RendData.PlanetRad);
									#endif // WCS_BUILD_VNS
									// convert WCS coords into export coords
									Master->RBounds.DefDegToRBounds(CurCam->TargPos->Lat, CurCam->TargPos->Lon, X, Y);
									LW7Data[0][LW7Ct] = (float)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale);
									LW7Data[2][LW7Ct] = (float)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
									LW7Data[1][LW7Ct] = (float)((CurCam->TargPos->Elev - Master->ExportRefData.RefElev) * Master->ExportRefData.ElevScale);
									LW7Data[9][LW7Ct] = (float)GrNode->TCB[0];
									LW7Data[10][LW7Ct] = (float)GrNode->TCB[1];
									LW7Data[11][LW7Ct] = (float)GrNode->TCB[2];
									LW7Data[12][LW7Ct] = (float)NextKeyFrame;
									LW7Data[13][LW7Ct] = (float)GrNode->Linear;
									LW7Ct ++;
									LastKeyFrame = NextKeyFrame;
									} // if
								else
									break;
								} // if
							else
								break;
							} // while

						// write position channels
						for (Channel = 0; Channel < 3; Channel ++)
							{
							fprintf(LWFile, "Channel %d\n", Channel);
							fprintf(LWFile, "{ Envelope\n");
							fprintf(LWFile, "  %d\n", LW7NumKeys);
							for (int LW7CtWrite = 0; LW7CtWrite < LW7Ct; LW7CtWrite ++)
								{
								if(LW7Data[13][LW7CtWrite] != 0.0)
									{ // linear
									fprintf(LWFile, "  Key %g %g 3 0 0 0 0 0 0\n", LW7Data[Channel][LW7CtWrite], LW7Data[12][LW7CtWrite] / FrameRate);
									} // if
								else
									{ // TCB spline
									fprintf(LWFile, "  Key %g %g 0 %g %g %g 0 0 0\n", LW7Data[Channel][LW7CtWrite], LW7Data[12][LW7CtWrite] / FrameRate, LW7Data[9][LW7CtWrite], LW7Data[10][LW7CtWrite], LW7Data[11][LW7CtWrite]);
									} // else
								} // for
							fprintf(LWFile, "  Behaviors 1 1\n");
							fprintf(LWFile, "}\n");
							} // for
						} // if position keys
					else
						{
						RendData.RenderTime = 0.0;
						CurCam->SetToTime(RendData.RenderTime);
						if (CurCam->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED && CurCam->TargetObj)
							CurCam->TargetObj->SetToTime(RendData.RenderTime);
						// this gets the target position as a unit vector from the camera, not what we need
						//CurCam->InitFrameToRender(GlobalApp->AppEffects, &RendData);
						// a function that gets the actual target position in XYZ
						CurCam->GetTargetPosition(&RendData);

						// target position needs to be translated back into lat/lon
						#ifdef WCS_BUILD_VNS
						RendData.DefCoords->CartToDeg(CurCam->TargPos);
						#else // WCS_BUILD_VNS
						CurCam->TargPos->CartToDeg(RendData.PlanetRad);
						#endif // WCS_BUILD_VNS
						// convert WCS coords into export coords
						Master->RBounds.DefDegToRBounds(CurCam->TargPos->Lat, CurCam->TargPos->Lon, X, Y);
						LW7Data[0][0] = (float)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale);
						LW7Data[2][0] = (float)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
						LW7Data[1][0] = (float)((CurCam->TargPos->Elev - Master->ExportRefData.RefElev) * Master->ExportRefData.ElevScale);

						// write position channels
						for (Channel = 0; Channel < 3; Channel ++)
							{
							fprintf(LWFile, "Channel %d\n", Channel);
							fprintf(LWFile, "{ Envelope\n");
							fprintf(LWFile, "  1\n");
							fprintf(LWFile, "  Key %g 0 0 0 0 0 0 0 0\n", LW7Data[Channel][0]);
							fprintf(LWFile, "  Behaviors 1 1\n");
							fprintf(LWFile, "}\n");
							} // for
						} // else

					// now free our array
					AppMem_Free(LW7Data[0], WCS_LW7_DATA_NUMCHANNELS * LW7NumKeys * sizeof (float));
					LW7Data[0] = NULL;
					LW7Ct = LW7NumKeys = 0;

					for (Channel = 3; Channel < 9; Channel ++)
						{
						KeyValue = Channel < 6 ? 0.0: 1.0;	// rotation=0, scale=1
						fprintf(LWFile, "Channel %d\n", Channel);
						fprintf(LWFile, "{ Envelope\n");
						fprintf(LWFile, "  1\n");
						fprintf(LWFile, "  Key %g 0 0 0 0 0 0 0 0\n", KeyValue);
						fprintf(LWFile, "  Behaviors 1 1\n");
						fprintf(LWFile, "}\n");
						} // for
					fprintf(LWFile, "\n");
					fprintf(LWFile, "ShadowOptions 0\n");
					fprintf(LWFile, "\n");
					ObjectsWritten ++;
					} // if
				} // if
			} // for CurCam
		} // if RendData Init
	else
		Success = 0;
	} // if cameras

return (Success);

} // ExportFormatLW::ExportCameraTargets

/*===========================================================================*/

int ExportFormatLW::ExportFoliageTarget(FormatSpecificFile CurFile, long &ObjectsWritten, int HDInstancesExist)
{
double KeyValue;
long Channel;
FILE *LWFile = (FILE *)CurFile;

if (Master->ExportFoliage && Master->FoliageAsObj && 
	((Master->FoliageStyle == WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_FLIPBOARDS && Master->AlignFlipBoardsToCamera) ||
	(HDInstancesExist)))
	{
	fprintf(LWFile, "AddNullObject Foliage Target\n");
	fprintf(LWFile, "ShowObject 6 3\n");
	fprintf(LWFile, "ObjectMotion\n");
	fprintf(LWFile, "NumChannels 9\n");
	for (Channel = 0; Channel < 9; Channel ++)
		{
		KeyValue = Channel < 6 ? 0.0: 1.0;	// position & rotation=0, scale=1
		fprintf(LWFile, "Channel %d\n", Channel);
		fprintf(LWFile, "{ Envelope\n");
		fprintf(LWFile, "  1\n");
		fprintf(LWFile, "  Key %g 0 0 0 0 0 0 0 0\n", KeyValue);
		fprintf(LWFile, "  Behaviors 1 1\n");
		fprintf(LWFile, "}\n");
		} // for
	fprintf(LWFile, "\n");
	fprintf(LWFile, "ParentItem 30000000\n");	// parent to first camera
	fprintf(LWFile, "ShadowOptions 0\n");
	fprintf(LWFile, "\n");

	ObjectsWritten ++;
	} // if

return (1);

} // ExportFormatLW::ExportFoliageTarget

/*===========================================================================*/

int ExportFormatLW::ExportSceneEffects(FormatSpecificFile CurFile, Atmosphere *CurHaze, int HVSpritesExist, int HDInstancesExist)
{
FILE *LWFile = (FILE *)CurFile;
Sky *CurSky;
Object3DInstance *CurInstance;
char *FolExtension;
unsigned char Red, Green, Blue;
char FullFileName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN], ImageShortName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN], SaveToPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN],
	OriginalFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

FolExtension = ImageSaverLibrary::GetDefaultExtension(Master->FoliageImageFormat);

fprintf(LWFile, "SolidBackdrop 0\n");
fprintf(LWFile, "BackdropColor 0.5 0.6 0.9\n");

// find an enabled sky
if (CurSky = (Sky *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_SKY, 0, NULL))
	{
	CurSky->SkyGrad.GetBasicColor(Red, Green, Blue, 0.0);
	fprintf(LWFile, "ZenithColor %g %g %g\n", Red / 255.0, Green / 255.0, Blue / 255.0);
	CurSky->SkyGrad.GetBasicColor(Red, Green, Blue, 0.501);
	fprintf(LWFile, "SkyColor %g %g %g\n", Red / 255.0, Green / 255.0, Blue / 255.0);
	CurSky->SkyGrad.GetBasicColor(Red, Green, Blue, 0.499);
	fprintf(LWFile, "GroundColor %g %g %g\n", Red / 255.0, Green / 255.0, Blue / 255.0);
	CurSky->SkyGrad.GetBasicColor(Red, Green, Blue, 1.0);
	fprintf(LWFile, "NadirColor %g %g %g\n", Red / 255.0, Green / 255.0, Blue / 255.0);
	} // if
else
	{
	fprintf(LWFile, "ZenithColor 0 0.1568628 0.3137255\n");
	fprintf(LWFile, "SkyColor 0.4705882 0.7058824 0.9411765\n");
	fprintf(LWFile, "GroundColor 0.4705882 0.7058824 0.9411765\n");
	fprintf(LWFile, "NadirColor 0.4235294 0.4235294 0.6078432\n");
	} // else
if (CurHaze && CurHaze->AtmosphereType != WCS_EFFECTS_ATMOSPHERETYPE_VOLUMETRIC)
	{
	if (CurHaze->HazeEnabled)
		{
		if (CurHaze->AtmosphereType == WCS_EFFECTS_ATMOSPHERETYPE_SIMPLE)
			fprintf(LWFile, "FogType 1\n");
		else if (CurHaze->AtmosphereType == WCS_EFFECTS_ATMOSPHERETYPE_SLOWINCREASE)
			fprintf(LWFile, "FogType 2\n");
		else
			fprintf(LWFile, "FogType 3\n");
		fprintf(LWFile, "FogMinDistance %g\n", CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].CurValue);
		fprintf(LWFile, "FogMaxDistance %g\n", CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE].CurValue);
		fprintf(LWFile, "FogMinAmount %g\n", CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY].CurValue);
		fprintf(LWFile, "FogMaxAmount %g\n", CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY].CurValue);
		fprintf(LWFile, "FogColor %g %g %g\n", CurHaze->HazeColor.GetCompleteValue(0), CurHaze->HazeColor.GetCompleteValue(1), CurHaze->HazeColor.GetCompleteValue(2));
		fprintf(LWFile, "BackdropFog 0\n");
		} // if
	if (CurHaze->FogEnabled)
		{
		fprintf(LWFile, "Plugin VolumetricHandler 1 LW_GroundFog\n");
		fprintf(LWFile, "3\n");
		fprintf(LWFile, "%g\n", CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEV].CurValue);
		fprintf(LWFile, "%g\n", CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEV].CurValue);
		if (CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEV].CurValue > CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEV].CurValue)
			fprintf(LWFile, "%g\n", 1.0 / (CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEV].CurValue - CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEV].CurValue));	// falloff
		else
			fprintf(LWFile, "%g\n", 1.0);	// falloff
		fprintf(LWFile, "%g\n", 1.0);	// luminosity
		fprintf(LWFile, "%g\n", 1.0);	// opacity
		fprintf(LWFile, "%g\n", 1.0);	// nominal distance
		fprintf(LWFile, "%g %g %g\n", CurHaze->FogColor.GetCompleteValue(0), CurHaze->FogColor.GetCompleteValue(1), CurHaze->FogColor.GetCompleteValue(2));
		fprintf(LWFile, "2\n0\n0\n0\n0\n");
		fprintf(LWFile, "EndPlugin\n");
		} // if
	} // if
else
	fprintf(LWFile, "FogType 0\n");

// HyperVoxel sprites
// one HVObject for each foliage image
if (HVSpritesExist)
	{
	fprintf(LWFile, "\
Plugin VolumetricHandler 1 HyperVoxelsFilter\n\
{ HyperVoxelData\n\
  HVFlags 2 0 4 1\n\
  HVBlendingGroups 0\n");
	
	// for each HV object
	for (CurInstance = Master->ObjectInstanceList; CurInstance; CurInstance = CurInstance->Next)
		{
		if (CurInstance->Flags & WCS_OBJECT3DINSTANCE_FLAGBIT_HVSPRITE)
			{
			// image name with extension
			strcpy(ImageShortName, CurInstance->MyObj->NameTable[0].Name);
			if (FolExtension)
				AddExtension(ImageShortName, FolExtension);

			// make directories if necessary
			strmfp(SaveToPath, Master->OutPath.GetPath(), "Images");
			if (PROJ_chdir(SaveToPath))
				{
				if (PROJ_mkdir(SaveToPath))
					{
					UserMessageOK("LightWave Export", "Could not create Images subdirectory. Export terminated.");
					return (0);
					} // if
				} // if
			strcat(SaveToPath, "/");
			strcat(SaveToPath, Master->OutPath.GetName());
			if (PROJ_chdir(SaveToPath))
				{
				if (PROJ_mkdir(SaveToPath))
					{
					UserMessageOK("LightWave Export", "Could not create Images subdirectory. Export terminated.");
					return (0);
					} // if
				} // if

			// for writing into the HyperVoxel block
			sprintf(FullFileName, "Images/%s/%s", Master->OutPath.GetName(), ImageShortName);

			// copy image to images directory
			strmfp(OriginalFullPath, Master->OutPath.GetPath(), ImageShortName);
			CopyExistingFile(OriginalFullPath, SaveToPath, ImageShortName);
			PROJ_remove(OriginalFullPath);

			// write HyperVoxel block
			fprintf(LWFile, "\
  { HVObject\n\
    { HVFlags\n\
      \"%s\"\n\
      1 1 0 2 0 0 0\n\
    }\n\
    { HVChannels\n\
      ParticleSize %g 0\n\
      SizeVariation 0 0\n\
      NearClip 1 0\n\
      VLight1 536870912\n\
      TextureAmplitude 0 0\n\
      AlignToPath 0\n\
    }\n", CurInstance->MyObj->GetName(), CurInstance->Scale[1] * 2.0);

			fprintf(LWFile, "\
    { HVTextures\n\
      { TextureBlock\n\
        1\n\
        { Texture\n\
          { Gradient\n\
            \"(-128)\"\n\
            Channel 1\n\
            { Opacity\n\
              0\n\
              1\n\
              0\n\
            }\n\
            Enable 1\n\
            Negative 0\n\
          }\n\
          ParamName \"Particle Weight\"\n\
          ItemName \"(none)\"\n\
          GradStart -1\n\
          GradEnd 1\n\
          GradRepeat 0\n\
          KeyFloats -1 -1 -1 -1 1 0 0 1 0 1 1 1 1 1 1\n\
          KeyInts 1 1 1\n\
        }\n\
      }\n");

			fprintf(LWFile, "\
      { TextureBlock\n\
        -1\n\
        { Texture\n\
          { Procedural\n\
            \"(-128)\"\n\
            Channel 65535\n\
            { Opacity\n\
              0\n\
              1\n\
              0\n\
            }\n\
            Enable 1\n\
            Negative 0\n\
          }\n\
          { TextureMap\n\
            { Center\n\
              0 0 0\n\
              0\n\
            }\n\
            { Size\n\
              1 1 1\n\
              0\n\
            }\n\
            { Rotation\n\
              0 0 0\n\
              0\n\
            }\n\
            { Falloff\n\
              0\n\
              0 0 0\n\
              0\n\
            }\n\
            RefObject \"(none)\"\n\
            Coordinates 0\n\
          }\n\
          Axis 2\n\
          TextureValue 1\n\
        }\n\
      }\n\
    }\n\
    { Shaders\n\
    }\n");

			fprintf(LWFile, "\
    { HVClip\n\
      15 3 0 3\n\
      0.003\n\
      0\n\
      { Clip\n\
        { Still\n\
          \"%s\"\n\
        }\n\
      }\n\
    }\n\
    { HVParticles\n\
      Color 0 \"(none)\"\n\
      Weight 1 \"%s\"\n\
    }\n\
    { HVoxelCache\n\
      FileName \"\"\n\
      Min 0 0 0\n\
      Max 0 0 0\n\
    }\n\
  }\n", FullFileName, CurInstance->MyObj->UVWTable[0].Name);

			} // if
		} // for
	fprintf(LWFile, "\
}\n\
EndPlugin\n");
	} // if

// HDInstance plugin for tree instances
if (HDInstancesExist)
	{
	fprintf(LWFile, "\
Plugin VolumetricHandler 1 HD_Instance\n\
{ PLUG\n\
}\n\
EndPlugin\n");
	} // if

fprintf(LWFile, "DitherIntensity 1\n");
fprintf(LWFile, "AnimatedDither 0\n");

fprintf(LWFile, "\n");

return (1);

} // ExportFormatLW::ExportSceneEffects

/*===========================================================================*/

int ExportFormatLW::ExportRenderInterface(FormatSpecificFile CurFile, EffectList *CameraList)
{
FILE *LWFile = (FILE *)CurFile;
ImageOutputEvent *OutEvent, *AlphaEvent;
RenderOpt *CurOpt;
EffectList *CurCam;
RenderJob *CurRJ;
int MatchFound = 0, SaveAlpha;
char FrameName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

// render
// if cameras
if (CameraList)
	{
	// for each camera
	for (CurCam = CameraList; CurCam && ! MatchFound; CurCam = CurCam->Next)
		{
		if (CurCam->Me)
			{
			// for each render job, does the camera match
			for (CurRJ = (RenderJob *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_RENDERJOB); CurRJ && ! MatchFound; CurRJ = (RenderJob *)CurRJ->Next)
				{
				if (CurRJ->Cam == (Camera *)CurCam->Me)
					{
					// what is the matching render options
					if ((CurOpt = CurRJ->Options) && CurOpt->OutputEvents)
						{
						if (OutEvent = CurOpt->OutputEvents->SaveBufferQuery("RED"))
							{
							// save alpha?
							SaveAlpha = ((AlphaEvent = OutEvent->SaveBufferQuery("ANTIALIAS")) && AlphaEvent == OutEvent);
							MatchFound = 1;
							// file prefix
							strmfp(FrameName, OutEvent->PAF.GetPath(), OutEvent->PAF.GetName());
							fprintf(LWFile, "RenderMode 2\n");
							fprintf(LWFile, "RayTraceEffects 1\n");
							fprintf(LWFile, "RayRecursionLimit 16\n");
							fprintf(LWFile, "RayTraceOptimization 1\n");
							fprintf(LWFile, "DataOverlayLabel\n");
							if (OutEvent->AutoDigits <= 3)
								{
								if (OutEvent->AutoExtension)
									{
									fprintf(LWFile, "OutputFilenameFormat 1\n");
									} // if
								else
									{
									fprintf(LWFile, "OutputFilenameFormat 0\n");
									} // else
								} // if
							else
								{
								if (OutEvent->AutoExtension)
									{
									fprintf(LWFile, "OutputFilenameFormat 3\n");
									} // if
								else
									{
									fprintf(LWFile, "OutputFilenameFormat 2\n");
									} // else
								} // else
							fprintf(LWFile, "SaveRGB 1\n");
							fprintf(LWFile, "SaveRGBImagesPrefix %s\n", ForceAmigaPathGlyphs(GlobalApp->MainProj->MungPath(FrameName)));
							fprintf(LWFile, "RGBImageSaver _Targa\n");	// safe file type
							fprintf(LWFile, "SaveAlpha %d\n", SaveAlpha);
							} // for
						} // if
					} // if
				} // for
			} // if
		} // for CurCam
	} // if cameras

// view stuff
fprintf(LWFile, "ViewConfiguration 0\n");
fprintf(LWFile, "DefineView 0\n");
fprintf(LWFile, "ViewType 9\n");
fprintf(LWFile, "ViewAimpoint 0 0 0\n");
fprintf(LWFile, "ViewZoomFactor 4\n");
fprintf(LWFile, "\n");

fprintf(LWFile, "GridNumber 80\n");
fprintf(LWFile, "GridSize 10\n");
fprintf(LWFile, "CameraViewBG 0\n");
fprintf(LWFile, "ShowMotionPath 1\n");
fprintf(LWFile, "ShowFogRadius 0\n");
fprintf(LWFile, "ShowFogEffect 1\n");
fprintf(LWFile, "ShowSafeAreas 0\n");
fprintf(LWFile, "ShowFieldChart 0\n");
fprintf(LWFile, "OverlayColor 7\n");
fprintf(LWFile, "\n");

fprintf(LWFile, "CurrentObject 0\n");
fprintf(LWFile, "CurrentLight 0\n");
fprintf(LWFile, "CurrentCamera 0\n");

return (1);

} // ExportFormatLW::ExportRenderInterface

/*===========================================================================*/

int ExportFormatLW::FindFile(Object3DEffect *Object3D, char *FoundPath)
{
char Extension[256];
FILE *ffile;

if (stcgfe(Extension, Object3D->FileName))
	{
	if (! stricmp(Extension, "lwo"))
		{
		strmfp(FoundPath, Object3D->FilePath, Object3D->FileName);
		if (ffile = PROJ_fopen(FoundPath, "rb"))
			{
			fclose(ffile);
			return (1);
			} // if
		} // if
	} // if

return (0);

} // ExportFormatLW::FindFile

/*===========================================================================*/

NameList *ExportFormatLW::AddNewNameList(NameList **Names, char *NewName, long FileType)
{
NameList **ListPtr;

if (Names)
	{
	ListPtr = Names;
	while (*ListPtr)
		{
		if (! stricmp((*ListPtr)->Name, NewName) && FileType == (*ListPtr)->ItemClass)
			return (*ListPtr);
		ListPtr = &(*ListPtr)->Next;
		} // if
	return (*ListPtr = new NameList(NewName, FileType));
	} // if

return (NULL);

} // ExportFormatLW::AddNewNameList
