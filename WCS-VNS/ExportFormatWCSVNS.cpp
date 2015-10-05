// ExportFormatWCSVNS.cpp
// Code module for WCS/VNS export code
// Created on 02/03/05 by FPW2
// Copyright 2005 3D Nature, LLC. All rights reserved.

#include "stdafx.h"
#include "ExportControlGUI.h"
#include "ExportFormat.h"
#include "Database.h"
#include "Project.h"
#include "EffectsLib.h"
#include "Raster.h"
#include "Security.h"
#include "DEMCore.h"
#include "RequesterBasic.h"
#include "Conservatory.h"
#include "AppMem.h"
#include "Lists.h"
#include "WCSVersion.h"

extern WCSApp *GlobalApp;

ExportFormatWCSVNS::ExportFormatWCSVNS(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource)
: ExportFormat(MasterSource, ProjectSource, EffectsSource, DBSource, ImageSource)
{

NewExportProj = NULL;
ExportDB = NULL;
ExportProj = NULL;
ExportImages = NULL;
ExportEffects = NULL;

} // ExportFormatWCSVNS::ExportFormatWCSVNS

/*===========================================================================*/

ExportFormatWCSVNS::~ExportFormatWCSVNS()
{

if (NewExportProj)
	{
	delete NewExportProj;
	NewExportProj = NULL;
	} // if

if (ExportEffects)
	{
	delete ExportEffects;
	ExportEffects = NULL;
	} // if

if (ExportImages)
	{
	delete ExportImages;
	ExportImages = NULL;
	} // if

if (ExportProj)
	{
	delete ExportProj;
	ExportProj = NULL;
	} // if

if (ExportDB)
	{
	delete ExportDB;
	ExportDB = NULL;
	} // if

} // ExportFormatWCSVNS::~ExportFormatWCSVNS

/*===========================================================================*/

// remove all temp files
void ExportFormatWCSVNS::Cleanup(const char *OutputFilePath, NameList **FileNamesCreated)
{
/* do not remove these files from disk - they are needed
const char *FileName;
long FileType;
char FullFileName[512];

// remove foliage temp files
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
*/

} // ExportFormatWCSVNS::Cleanup

/*===========================================================================*/

int ExportFormatWCSVNS::PackageExport(NameList **FileNamesCreated)
{
GeneralEffect *CurEffect, *Result; //, *SpareEffect;
int Success = 1;
CoordSys* NewCoordSys = NULL;
DEM* DP = NULL;
FILE* phyle = NULL;
Joe* Clip, JP;
Project* DaProject;
RenderJob *CurJob;
const char* FileNameOfKnownType;
const char* OutputFilePath;
//const char* TextureFile;
long FileType;
char FullFileName[512];
RealtimeFoliageIndex Index;

if (GlobalApp->Sentinal->CheckAuthFieldSX2() == 0)
	return 0;

// The directory where all the files should be created is:
OutputFilePath = Master->OutPath.GetPath();

if (ExportDB = new Database)
	ExportDB->SuppressNotifiesDuringLoad = 1;	// don't want anybody listening to all the changes going on
else
	Success = 0;

if (!(ExportProj = new Project))
	{
	Success = 0;
	} // if

if (!(ExportProj->Interactive = new InterCommon(ExportDB, ExportProj)))
	{
	Success = 0;
	} // if

if (!(ExportImages = new ImageLib()))
	{
	Success = 0;
	} // if

if (!(ExportEffects = new EffectsLib()))
	{
	Success = 0;
	} // if

if (Success && (NewExportProj = new NewProject(ExportProj, ExportDB, ExportEffects, ExportImages)))
	{
	NewExportProj->ProjectPathIncludesProjDir = true;
	NewExportProj->ProjName.SetPath((char *)OutputFilePath);
	NewExportProj->ProjName.SetName((char *)Master->OutPath.GetName());
	if (! NewExportProj->Create())
		{
		UserMessageOK("WCS-VNS Exporter", "Error creating export project.");
		Success = 0;
		} // else
	} // if
else
	{
	UserMessageOK("WCS-VNS Exporter", "Error creating export project.");
	Success = 0;
	} // else

// now the REAL fun begins
if (Success)
	{
	// remove detrimental effects
	ExportEffects->DeleteGroup(WCS_EFFECTSSUBCLASS_ATMOSPHERE);	// kill defaults, if user doesn't want to export any then give them none
	ExportEffects->DeleteGroup(WCS_EFFECTSSUBCLASS_SKY);	// kill defaults, if user doesn't want to export any then give them none

	// 3D Materials

	// 3D Objects
	Success = Process3DObjects(FileNamesCreated, OutputFilePath);

	// Area Terrafectors - N/A

	// Atmospheres
	if (Success && (Master->ExportHaze))
		{
		EffectList *AtmosList = Master->Haze;
		Atmosphere *Atmos;

		while (Success && AtmosList)
			{
			Atmos = (class Atmosphere *)AtmosList->Me;
			GlobalApp->SetCopyToLibs(ExportEffects, ExportImages);
			if (! (Result = ExportEffects->CloneNamedEffect(EffectsHost, WCS_EFFECTSSUBCLASS_ATMOSPHERE, Atmos->Name)))	//lint -e550
				Success = 0;
			GlobalApp->SetCopyToLibs(GlobalApp->AppEffects, GlobalApp->AppImages);
			AtmosList = AtmosList->Next;
			} // while
		} // if Atmospheres

	// Camera's
	if (Success && (Master->ExportCameras))
		{
		EffectList *CamList = Master->Cameras;
		Camera *Cam;

		if (CamList)
			ExportEffects->DeleteGroup(WCS_EFFECTSSUBCLASS_CAMERA);	// kill defaults if there's something being exported
		while (Success && CamList)
			{
			Cam = (class Camera *)CamList->Me;
			GlobalApp->SetCopyToLibs(ExportEffects, ExportImages);
			if (! (Result = ExportEffects->CloneNamedEffect(EffectsHost, WCS_EFFECTSSUBCLASS_CAMERA, Cam->Name)))
				Success = 0;
			GlobalApp->SetCopyToLibs(GlobalApp->AppEffects, GlobalApp->AppImages);
			CamList = CamList->Next;
			} // while
		} // if Camera's
	//   Need to clone Camera's with open views too
	if (Success)
		{
		for (int ViewNum = 0; ViewNum < WCS_VIEWGUI_VIEWS_OPEN_MAX; ViewNum++)
			{
			ViewGUI *VG = GlobalApp->GUIWins->CVG;

			if (VG->ViewSpaces[ViewNum] && VG->ViewSpaces[ViewNum]->VCamera)
				{
				// clone the Camera if it's not already in the export
				if (! ExportEffects->FindByName(WCS_EFFECTSSUBCLASS_CAMERA, VG->ViewSpaces[ViewNum]->VCamera->Name))
					{
					GlobalApp->SetCopyToLibs(ExportEffects, ExportImages);
					if (! (Result = ExportEffects->CloneNamedEffect(EffectsHost, WCS_EFFECTSSUBCLASS_CAMERA, VG->ViewSpaces[ViewNum]->VCamera->Name)))
						Success = 0;
					GlobalApp->SetCopyToLibs(GlobalApp->AppEffects, GlobalApp->AppImages);
					} // if
				} // if
			} // for
		} // if

	// Celestial Objects - N/A, and delete the defaults so that we don't need the Sun & Moon in the IOL
	ExportEffects->DeleteGroup(WCS_EFFECTSSUBCLASS_CELESTIAL);
	
	// Cloud Models - N/A
	// Color Maps - N/A

	// Control Points

	// CoordSys
	CoordSys *MCS = Master->RBounds.MyCoords;

	MCS = (CoordSys *)ExportEffects->AddEffect(WCS_EFFECTSSUBCLASS_COORDSYS, NULL, MCS);
	ExportEffects->UpdateDefaultCoords(MCS, true);
	ExportEffects->PlanetOpts->SetCoords(MCS);

	// DEM Mergers - N/A

	// DEMs
	if (Success && (Master->ExportTerrain))
		{
		char ElevName[512];

		// Look for the DEM, and add it to the export database if found
		FileType = WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN;
		if ((Master->DEMTilesX > 1) || (Master->DEMTilesY > 1))
			{
			/*** Question for Gary - why don't tiles contain overlap? ***/
			// add all tiles
			FileNameOfKnownType = NULL;
			while (FileNameOfKnownType = (*FileNamesCreated)->FindNextNameOfType(FileType, FileNameOfKnownType))
				{
				strmfp(FullFileName, Master->OutPath.GetPath(), FileNameOfKnownType);
				if (phyle = PROJ_fopen(FullFileName, "rb"))
					{
					if (DP = new DEM)
						{
						DP->LoadDEM(phyle, 0, &NewCoordSys);
						DP->FindElMaxMin();

						strncpy(ElevName, FileNameOfKnownType, sizeof(ElevName));
						StripExtension(ElevName);
						//#ifdef WCS_BUILD_VNS
						Clip = ExportDB->AddDEMToDatabase(APP_TLA" Exporter", ElevName, DP, NewCoordSys, ExportProj, ExportEffects);
						//#else // WCS_BUILD_VNS
						//Clip = ExportDB->AddDEMToDatabase("ExportDEM", ElevName, DP, NULL, ExportProj, ExportEffects);
						//#endif // WCS_BUILD_VNS

						delete DP;
						} // if DP
					fclose(phyle);
					} // if phyle
				} // if FileNameOfKnownType
			} // if tiled
		else
			{
			if (FileNameOfKnownType = (*FileNamesCreated)->FindNameOfType(FileType))
				{
				strmfp(FullFileName, Master->OutPath.GetPath(), FileNameOfKnownType);
				if (phyle = PROJ_fopen(FullFileName, "rb"))
					{
					if (DP = new DEM)
						{
						DP->LoadDEM(phyle, 0, &NewCoordSys);
						DP->FindElMaxMin();

						strncpy(ElevName, FileNameOfKnownType, sizeof(ElevName));
						StripExtension(ElevName);
						//#ifdef WCS_BUILD_VNS
						Clip = ExportDB->AddDEMToDatabase(APP_TLA" Exporter", ElevName, DP, NewCoordSys, ExportProj, ExportEffects);
						//#else // WCS_BUILD_VNS
						//Clip = ExportDB->AddDEMToDatabase("ExportDEM", ElevName, DP, NULL, ExportProj, ExportEffects);
						//#endif // WCS_BUILD_VNS

						delete DP;
						} // if DP
					fclose(phyle);
					} // if phyle
				} // if FileNameOfKnownType
			} // else not tiled
		// add texture to ground
		FileType = WCS_EXPORTCONTROL_FILETYPE_TEX1;
		FileNameOfKnownType = NULL;
		if (FileNameOfKnownType = (*FileNamesCreated)->FindNextNameOfType(FileType, FileNameOfKnownType))
			{
			Raster *TexRast;
			GroundEffect *Grounded;
			GradientCritter *MatNode;
			RasterAttribute *MyAttr;
			CoordSys *CS;

			GlobalApp->LoadToEffectsLib = ExportEffects;	// adding a raster with georeferencing will create a CS in this library
			GlobalApp->LoadToImageLib = ExportImages;	// referenced by image loader
			if (TexRast = ExportImages->AddRaster((char *)OutputFilePath, (char *)FileNameOfKnownType, 1, 1, 1))
				{
				if (NewCoordSys && (MyAttr = TexRast->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)))
					{
					if (CS = (CoordSys *)ExportEffects->FindByName(WCS_EFFECTSSUBCLASS_COORDSYS, NewCoordSys->Name))
						{
						// these two are deemed unnecessary since they are done in AddRaster anyway
						//((CoordSys *)MyAttr->GetShell()->Host)->RemoveRaster(MyAttr->GetShell());
						//MyAttr->GetShell()->SetHost(CS);
						CS->AddRaster(TexRast);
						} // if
					} // if
				if (Grounded = (GroundEffect *)ExportEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_GROUND, 1, ExportDB))
					{
					// make a ground texture using the raster just added
					if (MatNode = Grounded->EcoMat.FindNode(0.0, 0.0))
						{
						MaterialEffect *Mat;
						RootTexture *Root;
						Texture *Tex;
						if (Mat = (MaterialEffect *)MatNode->GetThing())
							{
							if (Master->BurnShading && Master->BurnShadows)
								Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].SetValue (1.0);
							// create root texture
							if (Root = Mat->NewRootTexture(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
								{
								// set texture type to planar image
								if (Tex = Root->AddNewTexture(NULL, WCS_TEXTURE_TYPE_PLANARIMAGE))
									{
									// install image
									Tex->SetRaster(TexRast);
									Tex->CoordSpace = WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED;
									} // if
								else
									Success = 0;
								} // if
							else
								Success = 0;
							} // if
						else
							Success = 0;
						} // if
					else
						Success = 0;
					} // if
				else
					Success = 0;
				} // if
			else
				Success = 0;
			GlobalApp->LoadToEffectsLib = GlobalApp->AppEffects;
			GlobalApp->LoadToImageLib = GlobalApp->AppImages;
			} // if
		} // if Terrain

	// Ecosystems - N/A
	// Foliage Effects - N/A
	// create a foliage effect for rendering the foliage file - gh
	if (Success && Master->ExportFoliage)
		{
		const char *IndexFileName, *DataFileName;
		FILE *ffile;

		// look for foliage files
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
				strmfp(FullFileName, OutputFilePath, IndexFileName);
				if (ffile = PROJ_fopen(FullFileName, "rb"))
					{
					// read file descriptor, no need to keep it around unless you want to
					fgets(FullFileName, 256, ffile);
					// version
					fread((char *)&Index.FileVersion, sizeof (char), 1, ffile);
					// number of files
					fread((char *)&Index.NumCells, sizeof (long), 1, ffile);
					// reference XYZ
					fread((char *)&Index.RefXYZ[0], sizeof (double), 1, ffile);
					fread((char *)&Index.RefXYZ[1], sizeof (double), 1, ffile);
					fread((char *)&Index.RefXYZ[2], sizeof (double), 1, ffile);
					fclose(ffile);

					if (Index.NumCells > 0)
						{
						FoliageEffect *Forest;
						// we have a valid foliage file so create a special kind of Foliage Effect
						if (Forest = (FoliageEffect *)ExportEffects->AddEffect(WCS_EFFECTSSUBCLASS_FOLIAGE, "Foliage Data", NULL))
							{
							Forest->UseFoliageFile = true;
							Forest->FoliageFile.SetPathAndName((char *)OutputFilePath, (char *)IndexFileName);
							// create a file with the image objects by name in the current Image Object Library
							// take the index file name, strip off the extension
							strmfp(FullFileName, OutputFilePath, IndexFileName);
							StripExtension(FullFileName);
							strcat(FullFileName, "ImageList.dat");
							if (ffile = PROJ_fopen(FullFileName, "w"))
								{
								fprintf(ffile, "%d\n", GlobalApp->AppImages->GetImageCount());
								for (Raster *CurRast = GlobalApp->AppImages->GetFirstRast(); CurRast; CurRast = GlobalApp->AppImages->GetNextRast(CurRast))
									{
									fprintf(ffile, "%s\n", CurRast->GetUserName());
									} // for
								fclose(ffile);
								} // if
							} // if
						} // if
					} // if
				} // if
			} // if
		} // if

	// Ground Objects
	// set up ground material texture in default ground -gh

	// Image Objects
	if (Success)
		Success = ProcessImages(OutputFilePath, FileNamesCreated);

	// Labels - N/A

	// Lakes

	// Lights
	if (Success && (Master->ExportLights))
		{
		EffectList *LiteList = Master->Lights;
		Light *Lite;

		if (LiteList)
			ExportEffects->DeleteGroup(WCS_EFFECTSSUBCLASS_LIGHT);	// kill defaults if there's something being exported
		while (Success && LiteList)
			{
			Lite = (class Light *)LiteList->Me;
			GlobalApp->SetCopyToLibs(ExportEffects, ExportImages);
			if (! (Result = ExportEffects->CloneNamedEffect(EffectsHost, WCS_EFFECTSSUBCLASS_LIGHT, Lite->Name)))
				Success = 0;
			GlobalApp->SetCopyToLibs(GlobalApp->AppEffects, GlobalApp->AppImages);
			LiteList = LiteList->Next;
			} // while
		} // if Lights

	// Planet Options
	ExportEffects->PlanetOpts->SetCoords(Master->Coords);

	// Post Processes
	GlobalApp->SetCopyToLibs(ExportEffects, ExportImages);
	if (Success)
		Success = ExportEffects->CloneEffects(EffectsHost, WCS_EFFECTSSUBCLASS_POSTPROC, true);	// grab all that are there
	GlobalApp->SetCopyToLibs(GlobalApp->AppEffects, GlobalApp->AppImages);

	// Render Jobs & Render Options
	CurJob = (RenderJob *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_RENDERJOB);
	while (Success && CurJob)
		{
		// clone RenderJob if an exported camera is attached
		if(CurJob->Cam) // protect against Jobs which have no Camera
			{
			if (CurEffect = ExportEffects->FindByName(WCS_EFFECTSSUBCLASS_CAMERA, CurJob->Cam->Name))	//lint -e550
				{
				GlobalApp->SetCopyToLibs(ExportEffects, ExportImages);
				if (Result = ExportEffects->CloneNamedEffect(EffectsHost, WCS_EFFECTSSUBCLASS_RENDERJOB, CurJob->Name))
					{
					if(CurJob->Options) // do we have an options to clone?
						{
						// if cloning succeeded, clone the linked RenderOpt
						if (! (Result = ExportEffects->CloneNamedEffect(EffectsHost, WCS_EFFECTSSUBCLASS_RENDEROPT, CurJob->Options->Name)))
							Success = 0;
						} // if
					} // if camera clone succeeded
				else
					Success = 0;
				GlobalApp->SetCopyToLibs(GlobalApp->AppEffects, GlobalApp->AppImages);
				} // if
			} // if
		CurJob = (RenderJob *)CurJob->Next;
		} // while
	// remove Scenarios added during RenderJob copying
	ExportEffects->DeleteGroup(WCS_EFFECTSSUBCLASS_SCENARIO);	// kill defaults if there's something being exported
	//   Need to clone Render Options used by open views too
	if (Success)
		{
		for (int ViewNum = 0; ViewNum < WCS_VIEWGUI_VIEWS_OPEN_MAX; ViewNum++)
			{
			ViewGUI *VG = GlobalApp->GUIWins->CVG;

			if (VG->ViewSpaces[ViewNum] && VG->ViewSpaces[ViewNum]->RO)
				{
				// clone the RenderOpt if it's not already in the export
				if (! ExportEffects->FindByName(WCS_EFFECTSSUBCLASS_RENDEROPT, VG->ViewSpaces[ViewNum]->RO->Name))
					{
					GlobalApp->SetCopyToLibs(ExportEffects, ExportImages);
					if (! (Result = ExportEffects->CloneNamedEffect(EffectsHost, WCS_EFFECTSSUBCLASS_RENDEROPT, VG->ViewSpaces[ViewNum]->RO->Name)))
						Success = 0;
					GlobalApp->SetCopyToLibs(GlobalApp->AppEffects, GlobalApp->AppImages);
					} // if
				} // if
			} // for
		} // if

	// Scene Exporters - N/A
	// Search Queries - N/A
	// some search queries will be created as a result of copying other components - gh

	// Shadows - to catch shadows from 3d objects only, I think - GH

	// Skies
	if (Success && (! Master->ExportSky))
		{
		ProcessSky(FileNamesCreated, OutputFilePath);
		} // if Lights

	// Snow Effects - N/A
	// Starfields - N/A

	// Streams - N/A

	// Terrafectors - N/A
	// Terrain Generators - N/A
	// Terrain Griddrs - N/A

	// Terrain Parameters
	// just use the default from project creation

	// Thematic Maps - if needed by 3d objects - GH

	// Vectors
	if (Success)
		Success = ProcessVectors();

	// Walls
	// exported as 3d objects? probably faster to render as walls and more likely to be textured and sized correctly when using thematic maps
	/***
	if (Success && Master->ExportWalls && Master->ObjectInstanceList)
		{
		CurEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_FENCE);
		while (Success && CurEffect)
			{
			if (CurEffect->Enabled)
				{
				if (Result = ExportEffects->AddEffect(WCS_EFFECTSSUBCLASS_FENCE, CurEffect->Name, CurEffect))
					{
					printf("do vector stuff");
					} // if
				else
					Success = 0;
				} // if enabled
			CurEffect = CurEffect->Next;
			} // while
		} // if Walls
	***/

	// Wave Models

	if (Success)
		{
		DaProject = NewExportProj->ProjHost;
		// Various Project settings
		// copy prefs
		ProcessProjPrefsInfo(DaProject, ProjectHost);
		// < copy views of exported cameras >
		// < copy view prefs >
		// < do intercommon stuff >
		DoInterCommon();

		// < create dir list >
		//DaProject->DirList_Add(DaProject->DL, (char *)OutputFilePath, 0);
		// < copy window settings >

		// At last, we save the new project
		DaProject->Save(NULL, ExportProj->projectname, ExportDB, ExportEffects, ExportImages);
		} // if
	} // if Success

Cleanup(OutputFilePath, FileNamesCreated);

if (NewCoordSys)
	{
	delete NewCoordSys;
	NewCoordSys = NULL;
	} // if

return Success;

} // ExportFormatWCSVNS::PackageExport

/*===========================================================================*/

int ExportFormatWCSVNS::Process3DObjects(NameList **FileNamesCreated, const char *OutputFilePath)
{
Object3DEffect *CurObj;
Object3DInstance *CurInstance;
RasterBounds *RBounds = Master->FetchRBounds();
int Success = 1;
long MatCt, TotalInstances;

// null out short names so they can be created again as needed and keep numbers of materials unique
if (CurInstance = Master->ObjectInstanceList)
	{
	while (CurInstance)
		{
		if (CurObj = CurInstance->MyObj)
			{
			for (MatCt = 0; MatCt < CurObj->NumMaterials; MatCt ++)
				{
				if (CurObj->NameTable[MatCt].Mat)
					CurObj->NameTable[MatCt].Mat->ShortName[0] = 0;
				} // for
			} // if
		CurInstance = CurInstance->Next;
		} // while
	} // if

// there shouldn't be many objects in this list, sky perhaps
if (CurInstance = Master->ObjectInstanceList)
	{
	if ((TotalInstances = CurInstance->CountBoundedInstances(RBounds)) > 0)	//lint -e550
		{
		while (CurInstance && Success)
			{
			if ((CurObj = CurInstance->MyObj) && (CurObj->Enabled))
				{
				// see if we're in the export
				if (CurInstance->IsBounded(RBounds))
					{
					// see if the object is aleady in the new Effects library
					if (! ExportEffects->FindByName(WCS_EFFECTSSUBCLASS_OBJECT3D, CurObj->Name))
						{
						GlobalApp->SetCopyToLibs(ExportEffects, ExportImages);
						if (! (ExportEffects->CloneNamedEffect(EffectsHost, WCS_EFFECTSSUBCLASS_OBJECT3D, CurObj->Name)))
							{
							if (! CurObj->FileName[0])
								{
								// save out the object's w3o file
								strcpy(CurObj->FilePath, OutputFilePath);
								strcpy(CurObj->FileName, CurObj->Name);
								strcat(CurObj->FileName, ".w3o");
								CurObj->SaveObjectW3O();
								} // if
							if (! ExportEffects->AddEffect(WCS_EFFECTSSUBCLASS_OBJECT3D, CurObj->Name, CurObj))
								Success = 0;
							} // if
						GlobalApp->SetCopyToLibs(GlobalApp->AppEffects, GlobalApp->AppImages);
						} // if
					} // if
				} // if
			CurInstance = CurInstance->Next;
			} // while
		} // if
	} // if

// geographic instances
for (CurObj = (Object3DEffect *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D); CurObj; CurObj = (Object3DEffect *)CurObj->Next)
	{
	if (CurObj->Enabled && CurObj->GeographicInstance)
		{
		// see if the object is aleady in the new Effects library
		if (! ExportEffects->FindByName(WCS_EFFECTSSUBCLASS_OBJECT3D, CurObj->Name))
			{
			GlobalApp->SetCopyToLibs(ExportEffects, ExportImages);
			if (! (ExportEffects->CloneNamedEffect(EffectsHost, WCS_EFFECTSSUBCLASS_OBJECT3D, CurObj->Name)))
				{
				Success = 0;
				} // if
			GlobalApp->SetCopyToLibs(GlobalApp->AppEffects, GlobalApp->AppImages);
			} // if
		} // for
	} // for

// clean up image textures that are still in the scene export root dir
/*
if (Success)
	{
	const char *MaterialImageFile;

	for (MaterialImageFile = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_3DOBJTEX); MaterialImageFile; MaterialImageFile = (*FileNamesCreated)->FindNextNameOfType(WCS_EXPORTCONTROL_FILETYPE_3DOBJTEX, MaterialImageFile))
		{
		PROJ_remove(MaterialImageFile);
		} // for

	for (MaterialImageFile = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_WALLTEX); MaterialImageFile; MaterialImageFile = (*FileNamesCreated)->FindNextNameOfType(WCS_EXPORTCONTROL_FILETYPE_WALLTEX, MaterialImageFile))
		{
		PROJ_remove(MaterialImageFile);
		} // for

	for (MaterialImageFile = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_ROOFTEX); MaterialImageFile; MaterialImageFile = (*FileNamesCreated)->FindNextNameOfType(WCS_EXPORTCONTROL_FILETYPE_ROOFTEX, MaterialImageFile))
		{
		PROJ_remove(MaterialImageFile);
		} // for
	} // if
*/

return (Success);

} // ExportFormatWCSVNS::Process3DObjects

/*===========================================================================*/

int ExportFormatWCSVNS::ProcessFoliageList(NameList **FileNamesCreated, const char *OutputFilePath)
{
int Success = 1;

return Success;

} // ExportFormatWCSVNS::ProcessFoliageList

/*===========================================================================*/

long ExportFormatWCSVNS::ProcessImages(const char *OutputFilePath, NameList **FileNamesCreated)
{
//const char *DataFileName, *IndexFileName;
const char *TextureFile;
Raster *CurRast, *GnuRast;
RasterAnimHostProperties CurRAHProp;
//FILE *ffile;
long FileType;
/***
RealtimeFoliageIndex Index;
RealtimeFoliageCellData RFCD;
RealtimeFoliageData FolData;
char FileName[512];
***/

GlobalApp->CopyToEffectsLib = ExportEffects;
GlobalApp->CopyToImageLib = ExportImages;

// copy Image Objects that are Foliage
CurRast = GlobalApp->AppImages->List;
while (CurRast)
	{
	if (CurRast->MatchAttribute(WCS_RASTERSHELL_TYPE_FOLIAGE))
		{
		// ok, while this creates the image in the exported IOL, it no longer has the "current apps", and the
		// numbering may screw things up (ie: first image is identified as Sun, second Moon)
		if (GnuRast = ExportImages->AddRaster())
			{
			GnuRast->Copy(GnuRast, CurRast, 1);
			} // if
		} // if
	CurRast = CurRast->Next;
	} // while

GlobalApp->CopyToEffectsLib = GlobalApp->AppEffects;
GlobalApp->CopyToImageLib = GlobalApp->AppImages;

/***
// add foliage textures
FileType = WCS_EXPORTCONTROL_FILETYPE_FOLIAGETEX;
TextureFile = NULL;
TextureFile = (*FileNamesCreated)->FindNextNameOfType(FileType, TextureFile);
while (TextureFile)
	{
	ExportImages->AddRaster((char *)OutputFilePath, (char *)TextureFile, 1, 1, 1);
	TextureFile = (*FileNamesCreated)->FindNextNameOfType(FileType, TextureFile);
	} // while TextureFile
***/

/***
// add foliage images - files may not exist. If they don't then user must not have chosen foliage export features
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
					} // if
				} // if
			} // if
		} // if
	} // if
***/

// add terrain texture images
FileType = WCS_EXPORTCONTROL_FILETYPE_TEX1;
TextureFile = NULL;
TextureFile = (*FileNamesCreated)->FindNextNameOfType(FileType, TextureFile);
while (TextureFile)
	{
	ExportImages->AddRaster((char *)OutputFilePath, (char *)TextureFile, 1, 1, 1);
	TextureFile = (*FileNamesCreated)->FindNextNameOfType(FileType, TextureFile);
	} // while TextureFile

// add sky textures

// add label textures
FileType = WCS_EXPORTCONTROL_FILETYPE_FOLIAGETEX;
TextureFile = NULL;
TextureFile = (*FileNamesCreated)->FindNextNameOfType(FileType, TextureFile);
while (TextureFile)
	{
	// see if it's a Label Image
	if ((strncmp(TextureFile, "LI", 2) == 0) && (strstr(TextureFile, "_Fol.")))
		ExportImages->AddRaster((char *)OutputFilePath, (char *)TextureFile, 1, 1, 1);
	TextureFile = (*FileNamesCreated)->FindNextNameOfType(FileType, TextureFile);
	} // while TextureFile

// must do this as it's not dynamically allocated and RealtimeFoliageIndex
// destructor will blow chunks if we don't
//Index.CellDat = NULL;

return 1;	// should really do error checking on AddRasters

} // ExportFormatWCSVNS::ProcessImages

/*===========================================================================*/

void ExportFormatWCSVNS::ProcessProjPrefsInfo(Project *DestProj, Project *SourceProj)
{
ProjPrefsInfo *To, *From;
long i;

To = &DestProj->Prefs;
From = &SourceProj->Prefs;
To->RenderTaskPri = From->RenderTaskPri;
To->RenderSize = From ->RenderSize;
To->PosLonHemisphere = From->PosLonHemisphere;
To->VertDisplayUnits = From->VertDisplayUnits;
To->HorDisplayUnits = From->HorDisplayUnits;
To->AngleDisplayUnits = From->AngleDisplayUnits;
To->TimeDisplayUnits = From->TimeDisplayUnits;
To->LatLonSignDisplay = From->LatLonSignDisplay;
To->TaskMode = From->TaskMode;
To->EnabledFilter = From->EnabledFilter;
To->AnimatedFilter = From->AnimatedFilter;
To->ShowDBbyLayer = From->ShowDBbyLayer;
To->GUIConfiguration = From->GUIConfiguration;
To->SAGExpanded = From->SAGExpanded;
// don't copy these as a safety check: RecordMode, InteractiveMode, KeyGroupMode
To->SignificantDigits = From->SignificantDigits;
To->MatrixTaskModeEnabled = From->MatrixTaskModeEnabled;
To->SAGBottomHtPct = From->SAGBottomHtPct;
To->InteractiveStyle = From->InteractiveStyle;
To->LastUTMZone = From->LastUTMZone;
To->MultiUserMode = From->MultiUserMode;
To->DisplayGeoUnitsProjected = From->DisplayGeoUnitsProjected;
To->NewProjUseTemplates = From->NewProjUseTemplates;
To->PaintDefaultsValid = 0;	//From->PaintDefaultsValid;
//To->ReportMesg[0] = From->ReportMesg[0];
//To->ReportMesg[1] = From->ReportMesg[1];
//To->ReportMesg[2] = From->ReportMesg[2];
//To->ReportMesg[3] = From->ReportMesg[3];
To->LoadOnOpen = From->LoadOnOpen;
To->MemoryLimitsEnabled = From->MemoryLimitsEnabled;
To->VecPolyMemoryLimit = From->VecPolyMemoryLimit;
To->DEMMemoryLimit = From->DEMMemoryLimit;
To->OpenWindows = From->OpenWindows;
To->ProjShowIconTools = From->ProjShowIconTools;
To->ProjShowAnimTools = From->ProjShowAnimTools;
To->MaxSAGDBEntries = From->MaxSAGDBEntries;
To->MaxSortedSAGDBEntries = From->MaxSortedSAGDBEntries;
To->LastUpdateDate = From->LastUpdateDate;
To->GlobalStartupPage = From->GlobalStartupPage;
To->PaintMode = From->PaintMode;
To->GradMode = From->GradMode;
To->ActiveBrush = From->ActiveBrush;
To->Effect = From->Effect;

for (i = 0; i < WCS_VIEWGUI_VIEWTYPE_MAX; i++)
	memcpy(&To->ViewEnabled[i][0], &From->ViewEnabled[i][0], WCS_VIEWGUI_ENABLE_MAX);

strcpy(To->CurrentUserName, From->CurrentUserName);
strcpy(To->CurrentPassword, From->CurrentPassword);
strcpy(To->LastColorSwatch, From->LastColorSwatch);

for (i = 0; i < WCS_VIEWGUI_VIEWTYPE_MAX; i++)
	To->ViewContOverlayInt[i] = From->ViewContOverlayInt[i];

To->Tolerance = From->Tolerance;
To->ForeElev = From->ForeElev;
To->BackElev = From->BackElev;
To->BrushScale = From->BrushScale;
To->Opacity = From->Opacity;

// VectorExportData???

for (i = 0; i < WCS_MAX_TEMPLATES; i++)
	To->DefaultTemplates[i] = From->DefaultTemplates[i];

for (i = 0; i < WCS_PROJECT_PREFS_CONFIG_OPTIONS_MAX; i++)
	strcpy(&To->ConfigOptions[i][0], &From->ConfigOptions[i][0]);

} // ExportFormatWCSVNS::ProcessProjPrefsInfo

/*===========================================================================*/

int ExportFormatWCSVNS::ProcessVectors(void)
{
#ifdef DEBUG
char *DaName;
#endif // DEBUG
Joe *CurJoe;
int Success = 1, NumLists = 4, ListNum, ListCompareNum, DontAdvance;
RenderJoeList *RJL[4], **RJLPtr, *ListCur, *ListCompare, *ListLast, *ListDelete;

// create a JoeList that can be used to ensure that each vector is copied only once
// We are using RenderJoeLists because a function in the database is already set up to generate such lists and 
// performs useful tasks like checking to see if there are any points in the vector and elliminating disabled
// or parent database items
if (Master->Export3DObjects)
	RJL[0] = DBHost->CreateRenderJoeList(EffectsHost, WCS_EFFECTSSUBCLASS_OBJECT3D);
else
	RJL[0] = NULL;
if (Master->ExportWalls)
	RJL[1] = DBHost->CreateRenderJoeList(EffectsHost, WCS_EFFECTSSUBCLASS_FENCE);
else
	RJL[1] = NULL;
if (! Master->BurnShadows)
	RJL[2] = DBHost->CreateRenderJoeList(EffectsHost, WCS_EFFECTSSUBCLASS_SHADOW);
else
	RJL[2] = NULL;
RJL[3] = NULL;

// elliminate out of bounds vectors from the first two lists
for (ListNum = 0; ListNum < NumLists - 1; ListNum ++)
	{
	if (RJL[ListNum])
		{
		ListLast = NULL;
		for (ListCur = RJL[ListNum]; ListCur; )
			{
			DontAdvance = 0;
			if (ListCur->Me)
				{
				double NNorthing, WEasting, SNorthing, EEasting;
				// test to see if vector at least partially overlaps with the exported region
				// GIS convention is required for testing against RasterBounds
				if (! (ListCur->Me->GetBoundsProjected(Master->Coords, NNorthing, WEasting, SNorthing, EEasting, true)
					&& Master->RBounds.TestBoundsOverlap(NNorthing, WEasting, SNorthing, EEasting)))
					{
					// remove the item
					ListDelete = ListCur;
					if (ListLast)
						ListLast->Next = (RenderJoeList *)ListCur->Next;
					else
						RJL[ListNum] = (RenderJoeList *)ListCur->Next;
					ListCur = (RenderJoeList *)ListCur->Next;
					DontAdvance = 1;
					delete ListDelete;
					} // if
				} // if
			if (! DontAdvance)
				{
				ListLast = ListCur;
				ListCur = (RenderJoeList *)ListCur->Next;
				} // if
			} // for
		} // if
	} // for

// make entries for all Joes that have rendering enabled.
if (Master->ExportVectors)
	{
	RJLPtr = &RJL[3];
	for (CurJoe = DBHost->GetFirst(WCS_DATABASE_STATIC); CurJoe; CurJoe = DBHost->GetNext(CurJoe))
		{
		if (! CurJoe->TestFlags(WCS_JOEFLAG_ISDEM) && ! CurJoe->TestFlags(WCS_JOEFLAG_HASKIDS))
			{
			if (CurJoe->TestFlags(WCS_JOEFLAG_ACTIVATED))
				{
				#ifdef DEBUG
				DaName = (char *)CurJoe->GetBestName();
				#endif // DEBUG
				if (CurJoe->TestFlags(WCS_JOEFLAG_RENDERENABLED))
					{
					double NNorthing, WEasting, SNorthing, EEasting;
					// test to see if vector at least partially overlaps with the exported region
					if (CurJoe->GetBoundsProjected(Master->Coords, NNorthing, WEasting, SNorthing, EEasting, true))
						{
						if (Master->RBounds.TestBoundsOverlap(NNorthing, WEasting, SNorthing, EEasting))
							{
							// add item to list
							if (*RJLPtr = new RenderJoeList())
								{
								(*RJLPtr)->Me = CurJoe;
								} // if
							else
								{
								Success = 0;
								break;
								} // else
							} // if
						} // if
					RJLPtr = (RenderJoeList **)&(*RJLPtr)->Next;
					} // if
				} // if
			} // if
		} // if
	} // for

// elliminate duplicates
for (ListNum = 0; ListNum < NumLists; ListNum ++)
	{
	if (RJL[ListNum])
		{
		for (ListCur = RJL[ListNum]; ListCur; ListCur = (RenderJoeList *)ListCur->Next)
			{
			for (ListCompareNum = 0; ListCompareNum < NumLists; ListCompareNum ++)
				{
				if (ListCompareNum == ListNum)
					continue;
				ListCompare = RJL[ListCompareNum];
				ListLast = NULL;
				while (ListCompare)
					{
					DontAdvance = 0;
					if (ListCur == ListCompare)
						{
						ListDelete = ListCompare;
						if (ListLast)
							ListLast->Next = (RenderJoeList *)ListCompare->Next;
						else
							RJL[ListCompareNum] = (RenderJoeList *)ListCompare->Next;
						ListCompare = (RenderJoeList *)ListCompare->Next;
						DontAdvance = 1;
						delete ListDelete;
						} // if
					if (! DontAdvance)
						{
						ListLast = ListCompare;
						ListCompare = (RenderJoeList *)ListCompare->Next;
						} // if
					} // for
				} // for
			} // for
		} // if
	} // for

// copy any effects
// for each effect, see if any vectors use it
// 3d objects = 0, walls = 1, shadows = 2
for (ListNum = 0; ListNum < 3; ListNum ++)
	{
	for (ListCur = RJL[ListNum]; Success && ListCur; ListCur = (RenderJoeList *)ListCur->Next)
		{
		if (ListCur->Effect)
			{
			if (! ExportEffects->FindByName(ListCur->Effect->EffectType, ListCur->Effect->Name))
				{
				GlobalApp->SetCopyToLibs(ExportEffects, ExportImages);
				if (! ExportEffects->CloneNamedEffect(EffectsHost, ListCur->Effect->EffectType, ListCur->Effect->Name))
					Success = 0;
				GlobalApp->SetCopyToLibs(GlobalApp->AppEffects, GlobalApp->AppImages);
				} // if
			} // if
		} // for
	} // for

// copy vectors
for (ListNum = 0; Success && ListNum < NumLists; ListNum ++)
	{
	for (ListCur = RJL[ListNum]; ListCur; ListCur = (RenderJoeList *)ListCur->Next)
		{
		if (! JoeDBCopy(ListCur->Me, ExportDB, ExportProj))
			{
			Success = 0;
			break;
			} // if
		} // for
	} // for

// delete lists
for (ListNum = 0; ListNum < NumLists; ListNum ++)
	{
	while (RJL[ListNum])
		{
		ListCur = (RenderJoeList *)RJL[ListNum]->Next;
		delete RJL[ListNum];
		RJL[ListNum] = ListCur;
		} // while
	} // for

return Success;

} // ExportFormatWCSVNS::ProcessVectors

/*===========================================================================*/

Joe *ExportFormatWCSVNS::JoeDBCopy(Joe *DupeMe, Database *Add2DB, Project *ProjHost)
{
char *DaName;
Joe *NewJoe;
RasterAnimHost *CurHost;
RasterAnimHostProperties *TempProp;
PlanetOpt *DefPlanetOpt;
long EffectType, Success = 1, ExaggerateElev = 0;

DefPlanetOpt = (PlanetOpt *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, DBHost);
if (DefPlanetOpt && DefPlanetOpt->EcoExageration && DefPlanetOpt->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].CurValue != 1.0)
	ExaggerateElev = 1;

DaName = (char *)DupeMe->GetBestName();
if (NewJoe = Add2DB->NewObject(ExportProj, DaName))
	{
	NewJoe->Copy(NewJoe, DupeMe);
	NewJoe->CopyPoints(NewJoe, DupeMe, 1, 1);
	if (ExaggerateElev)
		NewJoe->ExaggeratePointElevations(DefPlanetOpt);
	NewJoe->ClearFlags(NewJoe->GetFlags());
	NewJoe->SetFlags(DupeMe->GetFlags() & (WCS_JOEFLAG_ISCONTROL | WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED));
	// add effects
	if (TempProp = new RasterAnimHostProperties())
		{
		CurHost = NULL; 
		while (CurHost = DupeMe->GetRAHostChild(CurHost, 0))
			{
			TempProp->PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
			CurHost->GetRAHostProperties(TempProp);
			EffectType = TempProp->TypeNumber;
			if (EffectsHost->EffectTypeImplemented(EffectType))
				{
				GeneralEffect *GenFX = NULL;

				if ((EffectType == WCS_EFFECTSSUBCLASS_OBJECT3D && Master->Export3DObjects)
					|| (EffectType == WCS_EFFECTSSUBCLASS_FENCE && Master->ExportWalls)
					|| (EffectType == WCS_EFFECTSSUBCLASS_SHADOW && ! Master->BurnShadows)
					|| (EffectType == WCS_EFFECTSSUBCLASS_COORDSYS))
					{
					char *FXName;

					// need to see if already in export library, otherwise we end up making dupes
					FXName = CurHost->GetRAHostName();
					if (! (GenFX = ExportEffects->FindByName(EffectType, FXName)))
						{
						GlobalApp->SetCopyToLibs(ExportEffects, ExportImages);
						if (! (GenFX = ExportEffects->CloneNamedEffect(EffectsHost, EffectType, FXName)))
							Success = 0;
						GlobalApp->SetCopyToLibs(GlobalApp->AppEffects, GlobalApp->AppImages);
						} // if
					} // if
				if (Success && GenFX)
					NewJoe->AddEffect(GenFX, -1);
				} // if
			} // while 
		delete TempProp;
		} // if

	// convert points to new CS if necessary
	// if there is a CS then the CS was already attached
	if (! DupeMe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS))
		{
		PlanetOpt *DefPlanetOpt;
		if (DefPlanetOpt = (PlanetOpt *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, DBHost))
			{
			if (Master->Coords && Master->Coords != DefPlanetOpt->Coords)
				{
				// coords must be geographic in the current default coordinates
				// need to convert to a different geographic coords (datum)
				NewJoe->ConvertPointDatums(DefPlanetOpt->Coords, Master->Coords);
				} // if
			} // if
		} // if

	// copy attributes & layers
	if (Success)
		{
		LayerEntry *CurLayer, *LE;
		LayerStub *CurStub;
		unsigned long IsAttribute;

		for (CurStub = DupeMe->FirstLayer(); CurStub; CurStub = DupeMe->NextLayer(CurStub))
			{
			if (CurLayer = CurStub->MyLayer())
				{
				LE = Add2DB->DBLayers.MatchMakeLayer((char *)CurLayer->GetName(), 0);
				IsAttribute = CurLayer->TestFlags(WCS_LAYER_ISATTRIBUTE);
				if (IsAttribute)
					{
					if (! CurLayer->TestFlags(WCS_LAYER_LINKATTRIBUTE))
						{
						if (CurLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE))
							{
							NewJoe->AddTextAttribute(LE, CurStub->GetTextAttribVal());
							} // if
						else
							{
							NewJoe->AddIEEEAttribute(LE, CurStub->GetIEEEAttribVal());
							} // else
						} // if not link
					else
						{
						if (! NewJoe->MatchLayer(LE))
							NewJoe->AddObjectToLayer(LE);
						} // else
					} // if
				else
					{
					if (! NewJoe->MatchLayer(LE))
						NewJoe->AddObjectToLayer(LE);
					} // if
				} // if
			} // for
		} // if
	} // if

return NewJoe;

} // ExportFormatWCSVNS::JoeDBCopy

/*===========================================================================*/

void ExportFormatWCSVNS::DoInterCommon(void)
{
InterCommon *ExportInter = ExportProj->Interactive;
InterCommon *HostInter = ProjectHost->Interactive;

ExportInter->ProjFrameRate = HostInter->GetFrameRate();
ExportInter->ActiveTime = Master->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_EXPORTTIME].CurValue;
ExportInter->FollowTerrain = HostInter->FollowTerrain;
ExportInter->TfxPreview = HostInter->TfxPreview;
ExportInter->TfxRealtime = HostInter->TfxRealtime;
ExportInter->EditPointsMode = HostInter->EditPointsMode;
ExportInter->DigitizeDrawMode = HostInter->DigitizeDrawMode;
ExportInter->GridSample = HostInter->GridSample;

} // ExportFormatWCSVNS::DoInterCommon

/*===========================================================================*/
/*
int CubeVertSign[8][3] = {
	1,1,1,
	1,1,-1,
	-1,1,-1,
	-1,1,1,
	1,-1,1,
	1,-1,-1,
	-1,-1,-1,
	-1,-1,1
	};

int CubePolyRef[8][3] = {
	0,1,4,
	1,2,4,
	2,3,4,
	3,0,4,
	0,1,5,
	1,2,5,
	2,3,5,
	3,0,5
	};

int CubeVertRef[6][4] = {
	0,4,7,3,
	1,5,4,0,
	2,6,5,1,
	3,7,6,2,
	1,0,3,2,
	4,5,6,7
	};

char *SkyCubeMatName[6] = {"North", "East", "South", "West", "Top", "Bottom"};

int SkyCubeFileTypes[6] = {
	WCS_EXPORTCONTROL_FILETYPE_SKYNORTH,
	WCS_EXPORTCONTROL_FILETYPE_SKYEAST,
	WCS_EXPORTCONTROL_FILETYPE_SKYSOUTH,
	WCS_EXPORTCONTROL_FILETYPE_SKYWEST,
	WCS_EXPORTCONTROL_FILETYPE_SKYTOP,
	WCS_EXPORTCONTROL_FILETYPE_SKYBOTTOM
	};
*/
int ExportFormatWCSVNS::ProcessSky(NameList **FileNamesCreated, const char *OutputFilePath)
{
int Success = 1;

if (! Master->ExportSky)
	{
	GlobalApp->SetCopyToLibs(ExportEffects, ExportImages);
	ExportEffects->CloneEffects(EffectsHost, WCS_EFFECTSSUBCLASS_SKY, true);
	ExportEffects->CloneEffects(EffectsHost, WCS_EFFECTSSUBCLASS_CLOUD, true);
	ExportEffects->CloneEffects(EffectsHost, WCS_EFFECTSSUBCLASS_CELESTIAL, true);
	GlobalApp->SetCopyToLibs(GlobalApp->AppEffects, GlobalApp->AppImages);
	} // if

return Success;

} // ExportFormatWCSVNS::ProcessSky

/*===========================================================================*/
