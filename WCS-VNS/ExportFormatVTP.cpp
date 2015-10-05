// ExportFormatVTP.cpp
// Code module for VTP export code
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

ExportFormatVTP::ExportFormatVTP(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource)
: ExportFormat(MasterSource, ProjectSource, EffectsSource, DBSource, ImageSource)
{

RelativeFoliageFileName[0] = RelativeSpeciesFileName[0] = 0;
RelativeSkyFileName[0] = RelativeStructFileName[0] = RelativeLocFileName[0] = FirstLocName[0] = 0;
WGS84Sys = NULL;
IsProjected = false;

} // ExportFormatVTP::ExportFormatVTP

/*===========================================================================*/

ExportFormatVTP::~ExportFormatVTP()
{
if(WGS84Sys)
	{
	delete WGS84Sys;
	WGS84Sys = NULL;
	} // if

} // ExportFormatVTP::~ExportFormatVTP

/*===========================================================================*/

int ExportFormatVTP::SanityCheck(void)
{
int Success = 1;

#ifdef WCS_BUILD_VNS
/*
if(Master->Coords && (Master->Coords->Method.GCTPMethod != 0))
	{
	UserMessageOK("VTP Export", "VTP Export currently does not support non-Geographic coordinates.\nAborting export job.");
	return(0);
	} // if
*/

#endif // WCS_BUILD_VNS
return (Success);

} // ExportFormatVTP::SanityCheck

/*===========================================================================*/

int ExportFormatVTP::PackageExport(NameList **FileNamesCreated)
{
long FileType;
int Success = 1, VFExists = 0, SpeciesExists = 0;
const char *FileNameOfKnownType, *OutputFilePath;
const char *IndexFileName, *DataFileName;
FILE *FScene = NULL;
PathAndFile SceneOutput, SceneAux;
char TempFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN], TempFullPathB[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN], ImageName[256];
Raster *CurRast;

#ifdef WCS_BUILD_VNS
CoordSys *ExportCS = NULL;

if(Master->Coords)
	{
	if(Master->Coords->Method.GCTPMethod == 0)
		{
		IsProjected = false;
		} // if
	else
		{
		IsProjected = true;
		WGS84Sys = new CoordSys;
		WGS84Sys->SetSystemByCode(12); // 12=WGS84
		} // else
	} // if
#endif // WCS_BUILD_VNS

// The directory where all the files should be created is:
OutputFilePath = Master->OutPath.GetPath();

SceneAux.SetPath((char *)Master->OutPath.GetPath());
// Ensure we have a Sky destination dir
SceneAux.SetName("Sky");
SceneAux.GetPathAndName(TempFullPath);
PROJ_mkdir(TempFullPath);

// Ensure we have a Locations destination dir
SceneAux.SetPath((char *)Master->OutPath.GetPath());
SceneAux.SetName("Locations");
SceneAux.GetPathAndName(TempFullPath);
PROJ_mkdir(TempFullPath);

// process foliage. WCS-style list needs to be recreated as a VTP kind of list
if (Success = ProcessFoliageList(FileNamesCreated, OutputFilePath))
	{
	// process sky cubic images into the preferred format
	if (Success = ProcessSky(FileNamesCreated, OutputFilePath))
		{
		// Process 3D Objects
		Process3DObjects(FileNamesCreated, OutputFilePath);

		SceneAux.SetPath((char *)Master->OutPath.GetPath());

		// Ensure we have a Terrains destination dir
		SceneAux.SetName("Terrains");
		SceneAux.GetPathAndName(TempFullPath);
		PROJ_mkdir(TempFullPath);

		// Ensure we have a PlantModels destination dir
		SceneAux.SetName("PlantModels");
		SceneAux.GetPathAndName(TempFullPath);
		PROJ_mkdir(TempFullPath);

		// Ensure we have a PlantData destination dir
		SceneAux.SetName("PlantData");
		SceneAux.GetPathAndName(TempFullPath);
		PROJ_mkdir(TempFullPath);

		SceneOutput.SetPath((char *)Master->OutPath.GetPath());
		SceneOutput.AppendDirToPath("Terrains"); // put INI file in Terrains
		SceneOutput.SetName((char *)Master->OutPath.GetName());

		SceneOutput.GetFramePathAndName(TempFullPath, ".xml", 0, WCS_PATHANDFILE_PATH_PLUS_NAME_LEN, 0);
		if(FScene = PROJ_fopen(TempFullPath, "w"))
			{
			// write minty XML header
			fprintf(FScene, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<Terrain_Parameters>\n");

			// write out VTP scene file, fetching names of the kinds of files VTP is interested in from the File name list
			// if file types do not exist then the user did not wish to export them.
			fprintf(FScene, "\t<Name>%s</Name>\n", Master->OutPath.GetName());

			// DEM (BT) name
			const char *BTName = "";
			FileType = WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN;
			if (FileNameOfKnownType = (*FileNamesCreated)->FindNameOfType(FileType))
				{
				// move BT DEM to Elevation/ subdirectory
				BTName = FileNameOfKnownType;

				// Ensure we have an Elevation destination dir
				SceneAux.SetName("Elevation");
				SceneAux.GetPathAndName(TempFullPath);
				PROJ_mkdir(TempFullPath);

				// construct source path
				SceneOutput.SetPath((char *)Master->OutPath.GetPath());
				SceneOutput.SetName((char *)BTName);
				SceneOutput.GetPathAndName(TempFullPath);

				// construct new dest path
				SceneAux.AppendDirToPath("Elevation");
				SceneAux.SetName((char *)BTName);
				SceneAux.GetPathAndName(TempFullPathB);

				// remove any existing destination file
				PROJ_remove(TempFullPathB);
				// move the file to where it needs to be
				PROJ_rename(TempFullPath, TempFullPathB);

				// try for a .PRJ file
				if(IsProjected)
					{
					int TempLen;

					// chop off "bt" from end of source and dest
					TempLen = (int)strlen(TempFullPath);
					if(TempLen > 3)
						{
						TempFullPath[TempLen - 2] = NULL; // blitz the b in .bt, leave the dot
						strcat(TempFullPath, "prj"); // append the prj, no dot needed
						} // if

					TempLen = (int)strlen(TempFullPathB);
					if(TempLen > 3)
						{
						TempFullPathB[TempLen - 2] = NULL; // blitz the b in .bt, leave the dot
						strcat(TempFullPathB, "prj"); // append the prj, no dot needed
						} // if

					// remove any existing destination PRJ file
					PROJ_remove(TempFullPathB);
					// move the PRJ file to where it needs to be
					PROJ_rename(TempFullPath, TempFullPathB);

					} // if
				} // if

			fprintf(FScene, "\t<Elevation_Filename>%s</Elevation_Filename>\n", BTName);

			fprintf(FScene, "\t<Locations_File>%s</Locations_File>\n", RelativeLocFileName);
			if(FirstLocName[0])
				{
				fprintf(FScene, "\t<Init_Location>%s</Init_Location>\n", FirstLocName);
				} // if
			fprintf(FScene, "\t<Vertical_Exag>1</Vertical_Exag>\n");
			fprintf(FScene, "\t<Min_Height>1</Min_Height>\n");
			float xspace, zspace;
			if (Master->RBounds.IsGeographic)
				{
				xspace = (float)(Master->RBounds.CellSizeX * EARTHLATSCALE_METERS);	// convert geographic to meters
				zspace = (float)(Master->RBounds.CellSizeY * EARTHLATSCALE_METERS);
				} // if
			else
				{
				xspace = (float)Master->RBounds.CellSizeX;
				zspace = (float)Master->RBounds.CellSizeY;
				} // else
			fprintf(FScene, "\t<Nav_Speed>%d</Nav_Speed>\n", (int)(min(xspace, zspace) * 20.0));
			fprintf(FScene, "\t<Nav_Style>0</Nav_Style>\n");
			fprintf(FScene, "\t<Accel>false</Accel>\n");
			fprintf(FScene, "\t<Allow_Roll>false</Allow_Roll>\n");
			fprintf(FScene, "\t<Surface_Type>0</Surface_Type>\n");
			fprintf(FScene, "\t<LOD_Method>0</LOD_Method>\n");
			fprintf(FScene, "\t<Tri_Count>10000</Tri_Count>\n");
			fprintf(FScene, "\t<Tristrips>true</Tristrips>\n");
			fprintf(FScene, "\t<Vert_Count>20000</Vert_Count>\n");
			fprintf(FScene, "\t<Tile_Cache_Size>80</Tile_Cache_Size>\n");
			fprintf(FScene, "\t<Tile_Threading>false</Tile_Threading>\n");
			fprintf(FScene, "\t<Time_On>false</Time_On>\n");
			fprintf(FScene, "\t<Init_Time>104 2 21 8 0 0</Init_Time>\n");
			fprintf(FScene, "\t<Time_Speed>1.000000</Time_Speed>\n");
			fprintf(FScene, "\t<Texture>1</Texture>\n");
			fprintf(FScene, "\t<Tile_Size>1</Tile_Size>\n");
			// Texture name
			const char *TexName = "";
			FileType = WCS_EXPORTCONTROL_FILETYPE_TEX1;
			if (FileNameOfKnownType = (*FileNamesCreated)->FindNameOfType(FileType))
				{
				TexName = FileNameOfKnownType;

				// Reset Path that was tinkered with, above
				SceneAux.SetPath((char *)Master->OutPath.GetPath());

				// Ensure we have an GeoSpecific destination dir
				SceneAux.SetName("GeoSpecific");
				SceneAux.GetFramePathAndName(TempFullPath, "", 0, WCS_PATHANDFILE_PATH_PLUS_NAME_LEN, 0);
				PROJ_mkdir(TempFullPath);

				// construct source path
				SceneOutput.SetPath((char *)Master->OutPath.GetPath());
				SceneOutput.SetName((char *)TexName);
				SceneOutput.GetPathAndName(TempFullPath);

				// construct new dest path
				SceneAux.AppendDirToPath("GeoSpecific");
				SceneAux.SetName((char *)TexName);
				SceneAux.GetPathAndName(TempFullPathB);

				// remove any existing destination file
				PROJ_remove(TempFullPathB);
				// move the file to where it needs to be
				PROJ_rename(TempFullPath, TempFullPathB);

				// dispose of World and Proj files.
				// BlueMesa_RGB.pgw
				// BlueMesa_RGB.jgw
				// BlueMesa_RGB.bpw
				// BlueMesa_RGB.prj
				strmfp(TempFullPathB, OutputFilePath, TexName);
				StripExtension(TempFullPathB);
				strcat(TempFullPathB, ".prj");
				PROJ_remove(TempFullPathB);
				StripExtension(TempFullPathB);
				strcat(TempFullPathB, ".pgw");
				PROJ_remove(TempFullPathB);
				StripExtension(TempFullPathB);
				strcat(TempFullPathB, ".jgw");
				PROJ_remove(TempFullPathB);
				StripExtension(TempFullPathB);
				strcat(TempFullPathB, ".bpw");
				PROJ_remove(TempFullPathB);

				} // if
			fprintf(FScene, "\t<Texture_Filename>%s</Texture_Filename>\n", TexName);
			fprintf(FScene, "\t<Base_Texture>&lt;none&gt;</Base_Texture>\n");
			fprintf(FScene, "\t<Texture_4by4>&lt;none&gt;</Texture_4by4>\n");
			fprintf(FScene, "\t<Texture_Gradual>false</Texture_Gradual>\n");
			fprintf(FScene, "\t<MIP_Map>false</MIP_Map>\n");
			fprintf(FScene, "\t<Color_Map></Color_Map>\n");
			fprintf(FScene, "\t<Texture_Retain>false</Texture_Retain>\n");
			fprintf(FScene, "\t<Detail_Texture>false</Detail_Texture>\n");
			fprintf(FScene, "\t<Request_16_Bit>true</Request_16_Bit>\n");
			fprintf(FScene, "\t<Pre-Light>true</Pre-Light>\n");
			fprintf(FScene, "\t<PreLight_Factor>0.000000</PreLight_Factor>\n");
			fprintf(FScene, "\t<Cast_Shadows>false</Cast_Shadows>\n");

			fprintf(FScene, "\t<Roads>false</Roads>\n");
			fprintf(FScene, "\t<Road_File></Road_File>\n");
			fprintf(FScene, "\t<Highway>true</Highway>\n");
			fprintf(FScene, "\t<Paved>true</Paved>\n");
			fprintf(FScene, "\t<Dirt>true</Dirt>\n");
			fprintf(FScene, "\t<Road_Height>2.000000</Road_Height>\n");
			fprintf(FScene, "\t<Road_Distance>2.000000</Road_Distance>\n");
			fprintf(FScene, "\t<Road_Texture>true</Road_Texture>\n");
			fprintf(FScene, "\t<Road_Culture>false</Road_Culture>\n");

			fprintf(FScene, "\t<Trees>%s</Trees>\n", (RelativeFoliageFileName[0] ? "true" : "false"));
			fprintf(FScene, "\t<Tree_File>%s</Tree_File>\n", RelativeFoliageFileName);
			fprintf(FScene, "\t<Tree_Distance>%d</Tree_Distance>\n", (int)(Master->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_FOLDISTVANISH].CurValue));

			fprintf(FScene, "\t<Fog>%s</Fog>\n", (Master->ExportHaze ? "true" : "false"));

			// export haze
			EffectList *CurHaze;
			Atmosphere *CurHazeAtmo;
			int WroteFog = 0;
			if (Master->ExportHaze && (CurHaze = Master->Haze))
				{
				if (CurHaze->Me)
					{
					float range, fogcolorr, fogcolorg, fogcolorb;
					CurHazeAtmo = (Atmosphere *)CurHaze->Me;
					range = (float)(CurHazeAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].CurValue + CurHazeAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE].CurValue);
					fogcolorr = (float)(CurHazeAtmo->HazeColor.GetClampedCompleteValue(0));
					fogcolorg = (float)(CurHazeAtmo->HazeColor.GetClampedCompleteValue(1));
					fogcolorb = (float)(CurHazeAtmo->HazeColor.GetClampedCompleteValue(2));
					WroteFog = 1;
					fprintf(FScene, "\t<Fog_Distance>%f</Fog_Distance>\n", range / 1000.0f); // VTP fog is in Km, not m
					fprintf(FScene, "\t<Fog_Color>%d %d %d</Fog_Color>\n", (int)(fogcolorr * 255), (int)(fogcolorg * 255), (int)(fogcolorb * 255));
					} // if
				} // if

			if(!WroteFog)
				{
				fprintf(FScene, "\t<Fog_Distance>50</Fog_Distance>\n");
				} // if

			fprintf(FScene, "\t<Structure_Distance>%d</Structure_Distance>\n", (int)Master->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OBJECTDISTVANISH].CurValue);
			fprintf(FScene, "\t<Structure_Shadows>false</Structure_Shadows>");
			fprintf(FScene, "\t<Trans_Towers>false</Trans_Towers>\n");
			fprintf(FScene, "\t<Tower_File></Tower_File>");
			fprintf(FScene, "\t<Route_File>%s</Route_File>\n", ""); // route file?
			fprintf(FScene, "\t<Route_Enable>false</Route_Enable>\n");

			fprintf(FScene, "\t<Vehicles>false</Vehicles>\n");
			fprintf(FScene, "\t<Vehicle_Size>1</Vehicle_Size>\n");
			fprintf(FScene, "\t<Vehicle_Speed>1</Vehicle_Speed>\n");
			fprintf(FScene, "\t<Number_of_Cars>0</Number_of_Cars>\n");

			fprintf(FScene, "\t<Sky>%s</Sky>\n", RelativeSkyFileName[0] ? "true" : "false"); // turn sky on here
			fprintf(FScene, "\t<Sky_Texture>%s</Sky_Texture>\n", RelativeSkyFileName);
			fprintf(FScene, "\t<Ocean_Plane>false</Ocean_Plane>\n");
			fprintf(FScene, "\t<Ocean_Plane_Level>-20.000000</Ocean_Plane_Level>\n");
			fprintf(FScene, "\t<Depress_Ocean>false</Depress_Ocean>\n");
			fprintf(FScene, "\t<Depress_Ocean_Level>-40.000000</Depress_Ocean_Level>\n");
			fprintf(FScene, "\t<Horizon>false</Horizon>\n");
			fprintf(FScene, "\t<HUD_Overlay>,0,0</HUD_Overlay>\n");

			if(RelativeStructFileName)
				{
				fprintf(FScene, "\t<Layer>\n");
				fprintf(FScene, "\t\t<Type>Structure</Type>\n");
				fprintf(FScene, "\t\t<Filename>%s</Filename>\n", RelativeStructFileName);
				fprintf(FScene, "\t</Layer>\n");
				} // if

			// Process Locations/Animation Paths
			ProcessCameras(OutputFilePath, FScene);

			fprintf(FScene, "</Terrain_Parameters>\n");


			fclose(FScene);
			FScene = NULL;
			} // if

		// move foliage images and files into proper directories

		if(RelativeFoliageFileName[0])
			{ // move VF file to PlantData subdirectory
			// construct source path
			SceneOutput.SetPath((char *)Master->OutPath.GetPath());
			SceneOutput.SetName((char *)RelativeFoliageFileName);
			SceneOutput.GetPathAndName(TempFullPath);

			// construct new dest path
			SceneOutput.AppendDirToPath("PlantData");
			//SceneOutput.SetName((char *)TexName);
			SceneOutput.GetPathAndName(TempFullPathB);

			// remove any existing destination file
			PROJ_remove(TempFullPathB);
			// move the file to where it needs to be
			PROJ_rename(TempFullPath, TempFullPathB);
			} // if

		if(RelativeSpeciesFileName[0])
			{ // move species.xml file to PlantData subdirectory
			// construct source path
			SceneOutput.SetPath((char *)Master->OutPath.GetPath());
			SceneOutput.SetName((char *)RelativeSpeciesFileName);
			SceneOutput.GetPathAndName(TempFullPath);

			// construct new dest path
			SceneOutput.AppendDirToPath("PlantData");
			//SceneOutput.SetName((char *)TexName);
			SceneOutput.GetPathAndName(TempFullPathB);

			// remove any existing destination file
			PROJ_remove(TempFullPathB);
			// move the file to where it needs to be
			PROJ_rename(TempFullPath, TempFullPathB);
			} // if

		// move any created foliage images into PlantModels
		if(Master->ExportFoliage)
			{
			for(CurRast = Images->GetFirstRast(); CurRast; CurRast = Images->GetNextRast(CurRast))
				{
				strcpy(ImageName, CurRast->GetUserName());
				ImageSaverLibrary::StripImageExtension(ImageName);
				strcat(ImageName, "_Fol");
				ReplaceChar(ImageName, '.', '_');
				strcat(ImageName, ImageSaverLibrary::GetDefaultExtension(Master->FoliageImageFormat));
				FileType = WCS_EXPORTCONTROL_FILETYPE_FOLIAGETEX;
				if ((*FileNamesCreated)->FindNameExists(FileType, ImageName))
					{ // indeed a resampled image has been created for this entry
					// construct source path
					SceneOutput.SetPath((char *)Master->OutPath.GetPath());
					SceneOutput.SetName((char *)ImageName);
					SceneOutput.GetPathAndName(TempFullPath);

					// construct new dest path
					SceneOutput.AppendDirToPath("PlantModels");
					SceneOutput.GetPathAndName(TempFullPathB);

					// remove any existing destination file
					PROJ_remove(TempFullPathB);
					// move the file to where it needs to be
					PROJ_rename(TempFullPath, TempFullPathB);
					} // if
				} // for
			} // if

		// dispose of now-unneeded foliage files.
		// BlueMesa_Fol.dat
		// BlueMesa_FolFoliageList0.dat
		FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILEIDX;
		if (IndexFileName = (*FileNamesCreated)->FindNameOfType(FileType))
			{
			strmfp(TempFullPathB, OutputFilePath, IndexFileName);
			PROJ_remove(TempFullPathB);
			} // if

		FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILE;
		if (DataFileName = (*FileNamesCreated)->FindNameOfType(FileType))
			{
			strmfp(TempFullPathB, OutputFilePath, DataFileName);
			PROJ_remove(TempFullPathB);
			} // if


		} // if 
	} // if

return (Success);

} // ExportFormatVTP::PackageExport

/*===========================================================================*/

int ExportFormatVTP::ProcessFoliageList(NameList **FileNamesCreated, const char *OutputFilePath)
{
long FileType, DatPt;
int Success = 1;
const char *IndexFileName, *DataFileName;
Raster *CurRast;
char BaseName[64], ImageName[256], FileName[512], TestFileVersion;
RealtimeFoliageIndex Index;
RealtimeFoliageCellData RFCD;
RealtimeFoliageData FolData;
FILE *ffile, *vffile;
FoliagePreviewData PointData;

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

			if((Index.NumCells > 0) && (Index.CellDat->DatCt > 0))
				{
				strmfp(FileName, OutputFilePath, DataFileName);
				if (ffile = PROJ_fopen(FileName, "rb"))
					{
					fgets(FileName, 64, ffile);
					// version
					fread((char *)&TestFileVersion, sizeof(char), 1, ffile);
					// Pointless version check -- we know we wrote it
					if (TestFileVersion == Index.FileVersion)
						{
						strmfp(FileName, OutputFilePath, IndexFileName);
						// lop off extension if provided -- we will add back as necessary
						StripExtension(FileName);
						strcat(FileName, ".vf");
						if(vffile = PROJ_fopen(FileName, "wb"))
							{
							long Count;
							unsigned short WKTCRSLen;

							strcpy(RelativeFoliageFileName, IndexFileName);
							StripExtension(RelativeFoliageFileName);
							strcat(RelativeFoliageFileName, ".vf");

/*
							// file descriptor
							fprintf(vffile, "vf1.1");
							fputc(0, vffile); // NULL
							// UTM flag and zone (0 for geographic)
							UTMFlag = 0;
							UTMZone = 0;
							fwrite((char *)&UTMFlag, sizeof (char), 1, vffile);
							#ifdef BYTEORDER_BIGENDIAN
							SimpleEndianFlip32S(UTMZone, &UTMZone);
							#endif // BYTEORDER_BIGENDIAN
							fwrite((char *)&UTMZone, sizeof (long), 1, vffile);


							
							// datum
							Datum = 6326; // -1 = unknown for now, 23 or 6326 are WGS84
							#ifdef BYTEORDER_BIGENDIAN
							SimpleEndianFlip32S(Datum, &Datum);
							#endif // BYTEORDER_BIGENDIAN
							fwrite((char *)&Datum, sizeof (long), 1, vffile);

							// Count
							Count = Index.CellDat->DatCt;
							#ifdef BYTEORDER_BIGENDIAN
							SimpleEndianFlip32S(Count, &Count);
							#endif // BYTEORDER_BIGENDIAN
							fwrite((char *)&Count, sizeof (long), 1, vffile);

*/

							// file descriptor
							fprintf(vffile, "vf2.0");
							fputc(0, vffile); // NULL

							long CRSLenField, CRSBegin, CRSEnd, CRSLen;
							int ImageLoop;
							
							CRSLenField = ftell(vffile);
							// WKT CRS Length
							WKTCRSLen = 0; // will come back to rewrite later
							#ifdef BYTEORDER_BIGENDIAN
							SimpleEndianFlip16U(WKTCRSLen, &WKTCRSLen);
							#endif // BYTEORDER_BIGENDIAN
							fwrite((char *)&WKTCRSLen, sizeof(unsigned short), 1, vffile);

							CRSBegin = ftell(vffile);
							
							// write WKT CRS here
							ftell(vffile);

							if(Master->Coords)
								{
								Master->Coords->SaveToArcPrj(vffile);
								} // if
							else
								{
								// write the WGS84 dummy
								fprintf(vffile, "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]");
								} // else

							CRSEnd = ftell(vffile);
							CRSLen = CRSEnd - CRSBegin;
							fseek(vffile, CRSLenField, SEEK_SET);

							WKTCRSLen = (unsigned short)CRSLen; // Should not be bigger than 2^16
							#ifdef BYTEORDER_BIGENDIAN
							SimpleEndianFlip16U(WKTCRSLen, &WKTCRSLen);
							#endif // BYTEORDER_BIGENDIAN
							fwrite((char *)&WKTCRSLen, sizeof(unsigned short), 1, vffile);
							fseek(vffile, CRSEnd, SEEK_SET);

							// count number of species using same method as below
							unsigned long int NumSpeciesCount = 0;
							Raster *RastLoop;
							for(ImageLoop = 1, RastLoop = GlobalApp->AppImages->GetFirstRast(); RastLoop; ImageLoop++, RastLoop = GlobalApp->AppImages->GetNextRast(RastLoop))
								{
								NumSpeciesCount++;
								} // for

							// write count of species
							#ifdef BYTEORDER_BIGENDIAN
							SimpleEndianFlip32U(NumSpeciesCount, &NumSpeciesCount);
							#endif // BYTEORDER_BIGENDIAN
							fwrite((char *)&NumSpeciesCount, sizeof(long), 1, vffile);


							// write list of species, using same notation and encoding as in Species.xml below
							char SpeciesNameBuf[200];
							unsigned short SpeciesNameLen;
							for(ImageLoop = 1, RastLoop = GlobalApp->AppImages->GetFirstRast(); RastLoop; ImageLoop++, RastLoop = GlobalApp->AppImages->GetNextRast(RastLoop))
								{
								SpeciesNameBuf[0] = NULL;
								sprintf(SpeciesNameBuf, "%d_%s", ImageLoop - 1, RastLoop->GetUserName());
								SpeciesNameLen = (unsigned short)strlen(SpeciesNameBuf);

								// write species name length
								#ifdef BYTEORDER_BIGENDIAN
								SimpleEndianFlip16U(SpeciesNameLen, &SpeciesNameLen);
								#endif // BYTEORDER_BIGENDIAN
								fwrite((char *)&SpeciesNameLen, sizeof(unsigned short), 1, vffile);

								// write species name
								fwrite(SpeciesNameBuf, SpeciesNameLen, 1, vffile);
								} // for

							// Count (number of plant instances)
							Count = Index.CellDat->DatCt;
							#ifdef BYTEORDER_BIGENDIAN
							SimpleEndianFlip32S(Count, &Count);
							#endif // BYTEORDER_BIGENDIAN
							fwrite((char *)&Count, sizeof(long), 1, vffile);

							// local origin for file
							double TexRefLat, TexRefLon;
							TexRefLat = Index.RefXYZ[1];	//GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y);
							TexRefLon = Index.RefXYZ[0];	//GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X);

							#ifdef BYTEORDER_BIGENDIAN
							SimpleEndianFlip64(&TexRefLat, &TexRefLat);
							SimpleEndianFlip64(&TexRefLon, &TexRefLon);
							#endif // BYTEORDER_BIGENDIAN
							// X,Y (Lon, Lat order)
							fwrite((char *)&TexRefLon, sizeof(double), 1, vffile);
							fwrite((char *)&TexRefLat, sizeof(double), 1, vffile);

							
							// now write actual stem locations
							for (DatPt = 0; DatPt < Index.CellDat->DatCt; DatPt ++)
								{
								if (FolData.ReadFoliageRecord(ffile, Index.FileVersion))
									{
									if (FolData.InterpretFoliageRecord(NULL, GlobalApp->AppImages, &PointData)) // don't need full decoding of 3dobjects, just height, etc
										{
										// WriteFoliageRecordVF will ignore 3D Object entities
										FolData.WriteFoliageRecordVF(vffile, &Index);
										} // if
									} // if
								} // for
							// must do this as it's not dynamically allocated and RealtimeFoliageIndex
							// destructor will blow chunks if we don't
							Index.CellDat = NULL;
							fclose(vffile);
							vffile = NULL;
							} // if
						} // if
					fclose(ffile);

					// write VTP species.xml

					const char *BTName = "", *FileNameOfKnownType;
					FileType = WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN;
					if (FileNameOfKnownType = (*FileNamesCreated)->FindNameOfType(FileType))
						{
						BTName = FileNameOfKnownType;
						strcpy(FileName, BTName);
						} // if
					if(!BTName || !BTName[0])
						{
						// synthesize it ourselves the hard way
						strcpy(FileName, Master->OutPath.GetName());
						} // if
					// lop off extension if provided -- we will add back as necessary
					StripExtension(FileName);
					sprintf(BaseName, "%s-species.xml", FileName);
					strcpy(RelativeSpeciesFileName, BaseName);
					strmfp(FileName, OutputFilePath, BaseName);
					if (ffile = PROJ_fopen(FileName, "wb"))
						{
						// no docs for this, just reverse-engineered it
						Raster *RastLoop;
						int ImageLoop;

						// header
						fprintf(ffile, "<?xml version=\"1.0\"?>\n\n");
						fprintf(ffile, "<species-file file-format-version=\"1.0\">\n");

						for(ImageLoop = 1, RastLoop = GlobalApp->AppImages->GetFirstRast(); RastLoop; ImageLoop++, RastLoop = GlobalApp->AppImages->GetNextRast(RastLoop))
							{
							//if(FolListFlag[ImageLoop])
								{
								float MaxSpeciesHeight = 100.0f;
// unimportant step, not worth the effort, leave at default 100m
/*								if(FolListFlag[ImageLoop])
									{
									MaxSpeciesHeight = FolListMaxHt[ImageLoop];
									} // if
*/
								fprintf(ffile, "	<species id=\"%d\" name=\"%d_%s\" max_height=\"100.00\">\n", ImageLoop - 1, ImageLoop - 1, RastLoop->GetUserName());
								fprintf(ffile, "		<common name=\"%s\" />\n", RastLoop->GetUserName());
								FileName[0] = NULL;
								if (CurRast = Images->FindByID(ImageLoop))
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
										// so it is safe to build this entry into the final export foliage library
										strcpy(FileName, ImageName);
										} // if
									} // if
								float WriteHeight, WriteWidth, WriteWidthFac;
								WriteHeight = WriteWidth = 1.0f;
								//if(FolListFlag[ImageLoop])
								if(1)
									{
									WriteWidthFac = (float)(RastLoop->Rows != 0 ? ((float)RastLoop->Cols / (float)RastLoop->Rows) : 1.0f);
									WriteHeight = MaxSpeciesHeight;
									WriteWidth = WriteWidthFac * WriteHeight;
									} // if
								//if(FolListFlag[ImageLoop])
								if(1)
									{
									fprintf(ffile, "		<appearance type=\"1\" filename=\"%s\" width=\"%f\" height=\"%f\" />\n", FileName, WriteWidth, WriteHeight);
									fprintf(ffile, "	</species>\n");
									} // if
/*
								else
									{
									fprintf(ffile, "		<appearance type=\"1\" filename=\"%d.png\" width=\"%f\" height=\"%f\" />\n", ImageLoop, WriteWidth, WriteHeight);
									fprintf(ffile, "	</species>\n");
									} // else
*/
								} // if
							} // for
						fprintf(ffile, "</species-file>\n");

						fclose(ffile);
						} // if


					} // if
				} // if

			} // if index opened

		// set Success = 0 if an error occurs that should abort exporting

		} // if
	} // if

// must do this as it's not dynamically allocated and RealtimeFoliageIndex
// destructor will blow chunks if we don't
Index.CellDat = NULL;

return (Success);

} // ExportFormatVTP::ProcessFoliageList

/*===========================================================================*/

int ExportFormatVTP::ProcessSky(NameList **FileNamesCreated, const char *OutputFilePath)
{
long FileType;
int Success = 1;
const char *NorthFileName, *SouthFileName, *EastFileName, *WestFileName, *TopFileName, *BotFileName;
Raster *FaceRast[6]; // indexed by CubeSphere.h's STC_CUBEFACENAME_ enums
Raster *OutputRast = NULL;
char TempFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

FaceRast[0] = FaceRast[1] = FaceRast[2] = FaceRast[3] = FaceRast[4] = FaceRast[5] = NULL;

// files may not exist. If they don't then user must not have chosen any of the sky export features
FileType = WCS_EXPORTCONTROL_FILETYPE_SKYNORTH;
if (NorthFileName = (*FileNamesCreated)->FindNameOfType(FileType))
	{
	FileType = WCS_EXPORTCONTROL_FILETYPE_SKYSOUTH;
	if (SouthFileName = (*FileNamesCreated)->FindNameOfType(FileType))
		{
		FileType = WCS_EXPORTCONTROL_FILETYPE_SKYEAST;
		if (EastFileName = (*FileNamesCreated)->FindNameOfType(FileType))
			{
			FileType = WCS_EXPORTCONTROL_FILETYPE_SKYWEST;
			if (WestFileName = (*FileNamesCreated)->FindNameOfType(FileType))
				{
				FileType = WCS_EXPORTCONTROL_FILETYPE_SKYTOP;
				if (TopFileName = (*FileNamesCreated)->FindNameOfType(FileType))
					{
					FileType = WCS_EXPORTCONTROL_FILETYPE_SKYBOTTOM;
					if (BotFileName = (*FileNamesCreated)->FindNameOfType(FileType))
						{
						// you've got the six file names that the Renderer wrote out.
						// combine them with the output file path to make files that can be opened with PROJ_fopen()
						// or create a Raster object and set the file name and path like

						// NORTH
						if (FaceRast[STC_CUBEFACENAME_FRONT_NORTH] = new Raster())
							{
							FaceRast[STC_CUBEFACENAME_FRONT_NORTH]->PAF.SetPathAndName((char *)OutputFilePath, (char *)NorthFileName);
							if (FaceRast[STC_CUBEFACENAME_FRONT_NORTH]->LoadnPrepImage(FALSE, FALSE))
								{
								if (FaceRast[STC_CUBEFACENAME_FRONT_NORTH]->LoadnProcessImage(FALSE))
									{
									// north image is loaded - repeat for other images
									// set Success = 0 if an error occurs that should abort exporting
									} // if
								else
									Success = 0;
								} // if
							else
								Success = 0;
							} // if
						else
							Success = 0;

						if(Success == 0)
							{ // mark as unavailable but proceed
							delete FaceRast[STC_CUBEFACENAME_FRONT_NORTH]; FaceRast[STC_CUBEFACENAME_FRONT_NORTH] = NULL;
							} // if
						Success = 1; // go on and try next

						// SOUTH
						if (FaceRast[STC_CUBEFACENAME_REAR_SOUTH] = new Raster())
							{
							FaceRast[STC_CUBEFACENAME_REAR_SOUTH]->PAF.SetPathAndName((char *)OutputFilePath, (char *)SouthFileName);
							if (FaceRast[STC_CUBEFACENAME_REAR_SOUTH]->LoadnPrepImage(FALSE, FALSE))
								{
								if (FaceRast[STC_CUBEFACENAME_REAR_SOUTH]->LoadnProcessImage(FALSE))
									{
									// north image is loaded - repeat for other images
									// set Success = 0 if an error occurs that should abort exporting
									} // if
								else
									Success = 0;
								} // if
							else
								Success = 0;
							} // if
						else
							Success = 0;

						if(Success == 0)
							{ // mark as unavailable but proceed
							delete FaceRast[STC_CUBEFACENAME_REAR_SOUTH]; FaceRast[STC_CUBEFACENAME_REAR_SOUTH] = NULL;
							} // if
						Success = 1; // go on and try next

						// EAST
						if (FaceRast[STC_CUBEFACENAME_RIGHT_EAST] = new Raster())
							{
							FaceRast[STC_CUBEFACENAME_RIGHT_EAST]->PAF.SetPathAndName((char *)OutputFilePath, (char *)EastFileName);
							if (FaceRast[STC_CUBEFACENAME_RIGHT_EAST]->LoadnPrepImage(FALSE, FALSE))
								{
								if (FaceRast[STC_CUBEFACENAME_RIGHT_EAST]->LoadnProcessImage(FALSE))
									{
									// north image is loaded - repeat for other images
									// set Success = 0 if an error occurs that should abort exporting
									} // if
								else
									Success = 0;
								} // if
							else
								Success = 0;
							} // if
						else
							Success = 0;

						if(Success == 0)
							{ // mark as unavailable but proceed
							delete FaceRast[STC_CUBEFACENAME_RIGHT_EAST]; FaceRast[STC_CUBEFACENAME_RIGHT_EAST] = NULL;
							} // if
						Success = 1; // go on and try next

						// WEST
						if (FaceRast[STC_CUBEFACENAME_LEFT_WEST] = new Raster())
							{
							FaceRast[STC_CUBEFACENAME_LEFT_WEST]->PAF.SetPathAndName((char *)OutputFilePath, (char *)WestFileName);
							if (FaceRast[STC_CUBEFACENAME_LEFT_WEST]->LoadnPrepImage(FALSE, FALSE))
								{
								if (FaceRast[STC_CUBEFACENAME_LEFT_WEST]->LoadnProcessImage(FALSE))
									{
									// north image is loaded - repeat for other images
									// set Success = 0 if an error occurs that should abort exporting
									} // if
								else
									Success = 0;
								} // if
							else
								Success = 0;
							} // if
						else
							Success = 0;

						if(Success == 0)
							{ // mark as unavailable but proceed
							delete FaceRast[STC_CUBEFACENAME_LEFT_WEST]; FaceRast[STC_CUBEFACENAME_LEFT_WEST] = NULL;
							} // if
						Success = 1; // go on and try next

						// TOP
						if (FaceRast[STC_CUBEFACENAME_TOP] = new Raster())
							{
							FaceRast[STC_CUBEFACENAME_TOP]->PAF.SetPathAndName((char *)OutputFilePath, (char *)TopFileName);
							if (FaceRast[STC_CUBEFACENAME_TOP]->LoadnPrepImage(FALSE, FALSE))
								{
								if (FaceRast[STC_CUBEFACENAME_TOP]->LoadnProcessImage(FALSE))
									{
									// north image is loaded - repeat for other images
									// set Success = 0 if an error occurs that should abort exporting
									} // if
								else
									Success = 0;
								} // if
							else
								Success = 0;
							} // if
						else
							Success = 0;

						if(Success == 0)
							{ // mark as unavailable but proceed
							delete FaceRast[STC_CUBEFACENAME_TOP]; FaceRast[STC_CUBEFACENAME_TOP] = NULL;
							} // if
						Success = 1; // go on and try next

						// BOTTOM
						if (FaceRast[STC_CUBEFACENAME_BOT] = new Raster())
							{
							FaceRast[STC_CUBEFACENAME_BOT]->PAF.SetPathAndName((char *)OutputFilePath, (char *)BotFileName);
							if (FaceRast[STC_CUBEFACENAME_BOT]->LoadnPrepImage(FALSE, FALSE))
								{
								if (FaceRast[STC_CUBEFACENAME_BOT]->LoadnProcessImage(FALSE))
									{
									// north image is loaded - repeat for other images
									// set Success = 0 if an error occurs that should abort exporting
									} // if
								else
									Success = 0;
								} // if
							else
								Success = 0;
							} // if
						else
							Success = 0;

						if(Success == 0)
							{ // mark as unavailable but proceed
							delete FaceRast[STC_CUBEFACENAME_BOT]; FaceRast[STC_CUBEFACENAME_BOT] = NULL;
							} // if
						Success = 1; // go on and try next

						if(OutputRast = new Raster)
							{
							char PanoName[WCS_PATHANDFILE_NAME_LEN];
							OutputRast->Rows = max(FaceRast[STC_CUBEFACENAME_BOT]->Rows, 256); // this seems like a reasonable size to steal
							OutputRast->Cols = OutputRast->Rows * 4; // four times as wide as high

							// Setup a path and name with a suffix
							strcpy(PanoName, Master->OutPath.GetName());
							strcat(PanoName, "_HemiRGB");
							strcat(PanoName, ImageSaverLibrary::GetDefaultExtension(Master->ImageFormat));

							// allocate and clear raster
							OutputRast->AllocRGBBands();

							// Resample
							for(int OutY = 0; OutY < OutputRast->Rows; OutY++)
								{
								for(int OutX = 0; OutX < OutputRast->Cols; OutX++)
									{
									double HemiLat, HemiLon, FaceX, FaceY;
									unsigned long int ImageX, ImageY, OutZip, InZip;
									unsigned char CubeFace;

									HemiLat = 90.0 - (90.0 * ((double)OutY / (double)(OutputRast->Rows - 1))); // Y=0:Lat=90 ... Y=Rows:Lat=0
									HemiLon = -180.0 + (360.0 * ((double)OutX / (double)(OutputRast->Cols - 1))); // X=0:Lon=-180 ... X=Cols:Lon=+180

									if(SphereToCube(HemiLon, HemiLat, CubeFace, FaceX, FaceY))
										{
										if(FaceRast[CubeFace])
											{
											// calculate image coords
											ImageX = (unsigned long)min(FaceX * (FaceRast[CubeFace]->Cols - 1), FaceRast[CubeFace]->Cols - 1);
											ImageY = (unsigned long)min(FaceY * (FaceRast[CubeFace]->Rows - 1), FaceRast[CubeFace]->Rows - 1);

											OutZip = OutX + OutY * OutputRast->Cols;
											InZip = ImageX + ImageY * FaceRast[CubeFace]->Cols;

											OutputRast->ByteMap[0][OutZip] = FaceRast[CubeFace]->ByteMap[0][InZip];
											OutputRast->ByteMap[1][OutZip] = FaceRast[CubeFace]->ByteMap[1][InZip];
											OutputRast->ByteMap[2][OutZip] = FaceRast[CubeFace]->ByteMap[2][InZip];
											} // if
										} // if
									} // for
								} // for

							OutputRast->PAF.SetPathAndName((char *)OutputFilePath, (char *)PanoName);
							OutputRast->PAF.AppendDirToPath("Sky");
							OutputRast->SaveImage(1);
							strcpy(RelativeSkyFileName, PanoName);
							} // if

						// delete source cube images

						FaceRast[STC_CUBEFACENAME_FRONT_NORTH]->PAF.GetPathAndName(TempFullPath);
						PROJ_remove(TempFullPath);
						FaceRast[STC_CUBEFACENAME_REAR_SOUTH]->PAF.GetPathAndName(TempFullPath);
						PROJ_remove(TempFullPath);
						FaceRast[STC_CUBEFACENAME_RIGHT_EAST]->PAF.GetPathAndName(TempFullPath);
						PROJ_remove(TempFullPath);
						FaceRast[STC_CUBEFACENAME_LEFT_WEST]->PAF.GetPathAndName(TempFullPath);
						PROJ_remove(TempFullPath);
						FaceRast[STC_CUBEFACENAME_TOP]->PAF.GetPathAndName(TempFullPath);
						PROJ_remove(TempFullPath);
						FaceRast[STC_CUBEFACENAME_BOT]->PAF.GetPathAndName(TempFullPath);
						PROJ_remove(TempFullPath);

						} // if
					} // if
				} // if
			} // if
		} // if
	} // if

delete FaceRast[0]; FaceRast[0] = NULL;
delete FaceRast[1]; FaceRast[1] = NULL;
delete FaceRast[2]; FaceRast[2] = NULL;
delete FaceRast[3]; FaceRast[3] = NULL;
delete FaceRast[4]; FaceRast[4] = NULL;
delete FaceRast[5]; FaceRast[5] = NULL;
delete OutputRast; OutputRast = NULL;

return (Success);

} // ExportFormatVTP::ProcessSky

/*===========================================================================*/
/*===========================================================================*/

// all this and the VTP/BT code from Exports.cpp should end up in VTPSupport.h/.cpp someday
void WriteVTSTHeader(FILE *VTSTFile)
{

fprintf(VTSTFile, "<?xml version=\"1.0\"?>\n");
fprintf(VTSTFile, "\n");
fprintf(VTSTFile, "<StructureCollection xmlns=\"http://www.openplans.net\"\n");
fprintf(VTSTFile, "					 xmlns:gml=\"http://www.opengis.net/gml\"\n");
fprintf(VTSTFile, "					 xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n");
fprintf(VTSTFile, "					 xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
fprintf(VTSTFile, "					 xsi:schemaLocation=\"http://www.openplans.net/buildings.xsd\">\n");
fprintf(VTSTFile, "\n");

} // WriteVTSTHeader

/*===========================================================================*/

void WriteVTSTBoundBox(FILE *VTSTFile, double MaxX, double MinX, double MaxY, double MinY)
{
fprintf(VTSTFile, "	<gml:boundedBy>\n");
fprintf(VTSTFile, "		<gml:Box>\n");
// modified coord convention
//fprintf(VTSTFile, "			<gml:coordinates>%f,%f %f,%f</gml:coordinates>\n", -MinX, MinY, -MaxX, MaxY);
fprintf(VTSTFile, "			<gml:coordinates>%f,%f %f,%f</gml:coordinates>\n", MinX, MinY, MaxX, MaxY);
fprintf(VTSTFile, "		</gml:Box>\n");
fprintf(VTSTFile, "	</gml:boundedBy>\n");
fprintf(VTSTFile, "\n");
} // WriteVTSTBoundBox

/*===========================================================================*/

void WriteVTSTCoordSys(FILE *VTSTFile, CoordSys *OutCS)
{
if(OutCS)
	{
	fprintf(VTSTFile, "	<SRS>");
	OutCS->SaveToArcPrj(VTSTFile);
	fprintf(VTSTFile, "</SRS>\n");
	fprintf(VTSTFile, "\n");
	} // if
else
	{
	fprintf(VTSTFile, "	<SRS>GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]</SRS>\n");
	} // else
} // WriteVTSTCoordSys

/*===========================================================================*/

void WriteVTSTEntry(FILE *VTSTFile, double XCoord, double YCoord, double ZCoord, double RotRadians, double Scale, char *FileName, char*FullPath)
{

// note from Ben Discoe
// It turns out that VTST does, indeed, have a vertical offset field.  You can
// write it.  Instead of:
// 	<Imported>
// 
// you can put the attribute:
// 	<Imported ElevationOffset="1.0">
// 
// for a 1-meter vertical offset relative to the terrain.  If you could add
// that to the SceneExpress export, it would be great for the future.  (If the
// offset is effectively 0, it is best to omit it for efficiency.)

if(ZCoord < 0.0000001)
	{
	fprintf(VTSTFile, "	<Imported>\n");
	} // if
else
	{
	fprintf(VTSTFile, "	<Imported ElevationOffset=\"%f\">\n", ZCoord);
	} // else
fprintf(VTSTFile, "		<Location>\n");
// grh modified coord convention
//fprintf(VTSTFile, "			<gml:coordinates>%f,%f</gml:coordinates>\n", -XCoord, YCoord);
fprintf(VTSTFile, "			<gml:coordinates>%f,%f</gml:coordinates>\n", XCoord, YCoord);
fprintf(VTSTFile, "		</Location>\n");
if(RotRadians != 0)
	{
	fprintf(VTSTFile, "		<Rotation>%f</Rotation>\n", RotRadians);
	} // if
if(FullPath)
	{
	fprintf(VTSTFile, "		<filename>%s</filename>\n", FullPath);
	} // if
else
	{
	fprintf(VTSTFile, "		<filename>BuildingModels/%s</filename>\n", FileName);
	} // else
if(Scale != 1.0)
	{
	fprintf(VTSTFile, "		<scale>%f</scale>\n", Scale);
	} // if
fprintf(VTSTFile, "	</Imported>\n");
fprintf(VTSTFile, "\n");

} // WriteVTSTEntry

/*===========================================================================*/

void WriteVTSTFooter(FILE *VTSTFile)
{
fprintf(VTSTFile, "</StructureCollection>\n");

} // WriteVTSTFooter

/*===========================================================================*/

int ExportFormatVTP::FindCopyable3DObjFile(Object3DEffect *Object3D)
{
char DummyString[512];

return (ExportFormat3DS::FindFile(Object3D, DummyString));

} // ExportFormatVTP::FindCopyable3DObjFile

/*===========================================================================*/

int ExportFormatVTP::Process3DObjects(NameList **FileNamesCreated, const char *OutputFilePath)
{
long TotalInstances, ObjsDone, ObjCt, DoIt, MatCt, BaseMatCt = 0;
ExportFormat3DS *ObjExporter3DS;
Object3DInstance *CurInstance;
Object3DEffect *CurObj, **ObjectsDoneList;
FormatSpecificFile CurFile;
int Success = 1;
char TempFullPath[512];
PathAndFile SceneAux;
FILE *fVTST = NULL;

SceneAux.SetPath((char *)OutputFilePath);
// Ensure we have a BuildingModels destination dir
SceneAux.SetName("BuildingModels");
SceneAux.GetPathAndName(TempFullPath);
PROJ_mkdir(TempFullPath);
SceneAux.AppendDirToPath("BuildingModels");

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

if (CurInstance = Master->ObjectInstanceList)
	{
	if ((TotalInstances = CurInstance->CountBoundedInstances(Master->FetchRBounds())) > 0)
		{
		if (ObjectsDoneList = (Object3DEffect **)AppMem_Alloc(min(TotalInstances, Master->Unique3DObjectInstances) * sizeof(Object3DEffect *), APPMEM_CLEAR))
			{
			ObjsDone = 0;
			if (ObjExporter3DS = new ExportFormat3DS(Master, ProjectHost, EffectsHost, DBHost, Images))
				{
				// need to save each object as a separate file
				while (CurInstance && Success && ObjsDone < Master->Unique3DObjectInstances)
					{
					if (CurObj = CurInstance->MyObj)
						{
						if (CurInstance->IsBounded(Master->FetchRBounds()))
							{
							DoIt = 1;
							// have we exported this object already?
							for (ObjCt = 0; ObjCt < ObjsDone; ObjCt ++)
								{
								if (CurObj == ObjectsDoneList[ObjCt])
									DoIt = 0;
								} // for
							if (DoIt)
								{
								// add to list of ones we've done
								ObjectsDoneList[ObjsDone] = CurObj;
								ObjsDone ++;
								// figure out a file name
								strcpy(TempFullPath, CurObj->GetName());
								ReplaceChar(TempFullPath, '.', '_');
								SceneAux.SetName(TempFullPath);

								// if we are supposed to copy an existing file
								if (Master->ObjectTreatment == WCS_EFFECTS_SCENEEXPORTER_OBJECTTREATMENT_COPYANY && ObjExporter3DS->FindFile(CurObj, TempFullPath))
									{
									Success = CopyExistingFile(TempFullPath, SceneAux.GetPath(), CurObj->FileName);
									} // if
								else
									{
									// open an object file
									if (CurFile = ObjExporter3DS->OpenObjectFile(SceneAux.GetPath(), SceneAux.GetName()))
										{
										// save object
										if (! ObjExporter3DS->SaveOneObject(CurFile, CurObj, SceneAux.GetPath(), TRUE, BaseMatCt, NULL, NULL))	// TRUE=SaveMaterials
											Success = 0;
										// close file, also writes the file so can fail
										if (! ObjExporter3DS->CloseFile(CurFile))
											Success = 0;
										BaseMatCt += CurObj->NumMaterials;
										} // if
									else
										Success = 0;
									} // else
								} // if
							} // if
						} // if object
					CurInstance = CurInstance->Next;
					} // while
				delete ObjExporter3DS;
				} // if
			else
				Success = 0;
			
			if(Success)
				{
				// write VTP VTST file

				SceneAux.SetPath((char *)OutputFilePath);
				// Ensure we have a BuildingData destination dir
				SceneAux.SetName("BuildingData");
				SceneAux.GetPathAndName(TempFullPath);
				PROJ_mkdir(TempFullPath);
				SceneAux.AppendDirToPath("BuildingData");

				// Setup a path and name with a suffix
				strcpy(TempFullPath, Master->OutPath.GetName());
				strcat(TempFullPath, "_3DO");
				strcat(TempFullPath, ".vtst");
				SceneAux.SetName(TempFullPath);
				strcpy(RelativeStructFileName, TempFullPath);
				SceneAux.GetPathAndName(TempFullPath);

				if (fVTST = PROJ_fopen(TempFullPath, "w"))
					{
					double MinX, MaxX, MinY, MaxY;

					// write header elements
					WriteVTSTHeader(fVTST);

					// Need to calculate bounding box here...
					MinX = MinY = FLT_MAX;
					MaxX = MaxY = -FLT_MAX;
					// bound 3D Object records
					for(CurInstance = Master->ObjectInstanceList; CurInstance; CurInstance = CurInstance->Next)
						{
						// grh modified coord convention
						//if(CurInstance->Geographic[0] > MaxX) MaxX = CurInstance->Geographic[0];
						//if(CurInstance->Geographic[0] < MinX) MinX = CurInstance->Geographic[0];
						//if(CurInstance->Geographic[1] > MaxY) MaxY = CurInstance->Geographic[1];
						//if(CurInstance->Geographic[1] < MinY) MinY = CurInstance->Geographic[1];
						if(CurInstance->ExportXYZ[0] + Master->ExportRefData.ExportRefLon > MaxX) MaxX = CurInstance->ExportXYZ[0] + Master->ExportRefData.ExportRefLon;
						if(CurInstance->ExportXYZ[0] + Master->ExportRefData.ExportRefLon < MinX) MinX = CurInstance->ExportXYZ[0] + Master->ExportRefData.ExportRefLon;
						if(CurInstance->ExportXYZ[1] + Master->ExportRefData.ExportRefLat > MaxY) MaxY = CurInstance->ExportXYZ[1] + Master->ExportRefData.ExportRefLat;
						if(CurInstance->ExportXYZ[1] + Master->ExportRefData.ExportRefLat < MinY) MinY = CurInstance->ExportXYZ[1] + Master->ExportRefData.ExportRefLat;
						} // for

					WriteVTSTBoundBox(fVTST, MaxX, MinX, MaxY, MinY);

					// Write Coordinate System in WKT form
					WriteVTSTCoordSys(fVTST, Master->Coords);

					// write 3D Object records
					for(CurInstance = Master->ObjectInstanceList; CurInstance; CurInstance = CurInstance->Next)
						{
						if(CurObj = CurInstance->MyObj)
							{
							double MaxScale;
							MaxScale = max(CurInstance->Scale[0], max(CurInstance->Scale[1], CurInstance->Scale[2]));

							// figure out a file name
							strcpy(TempFullPath, CurObj->GetName());
							ReplaceChar(TempFullPath, '.', '_');
							strcat(TempFullPath, ".3ds");

							// Come up with some sort of heading rotation
/*
							double Heading, PreAtan;
							PreAtan = ((CurInstance->Quaternion[0] * CurInstance->Quaternion[1]) + (CurInstance->Quaternion[2] * CurInstance->Quaternion[3])) /
							 ((CurInstance->Quaternion[0] * CurInstance->Quaternion[0]) - (CurInstance->Quaternion[1] * CurInstance->Quaternion[1]) -
							 (CurInstance->Quaternion[2] * CurInstance->Quaternion[2]) + (CurInstance->Quaternion[3] * CurInstance->Quaternion[3]));
							Heading = atan(2.0 * PreAtan);
							*/
							Point4d AxisAngle;
							double Heading, ZOffsetRelativeToTerrain = 0.0;
						
							// this will only be right if we have only Heading rotation -- P/B rotation will hose this value
							QuaternionToAxisAngle(AxisAngle, CurInstance->Quaternion);
							Heading = AxisAngle[3];
							if(AxisAngle[1] > 0.0)
								Heading = -Heading;

							Heading = -DEG2RAD(Heading);

							// Need to work out ZOffsetRelativeToTerrain as a value relative to terrain elevation at this point
							// If ZOffsetRelativeToTerrain is 0, VTST writer will write an entry without Z offset info, as it always did previously.
							double ObjectAbsElev, TerrainAbsElev, ObjectRelOffset;
							unsigned long Flags;
							PolygonData Poly;
							VertexData VertData;
							RenderData RendData(NULL);

							if (RendData.InitToView(EffectsHost, ProjectHost, DBHost, ProjectHost->Interactive, NULL, NULL, 320, 320))
								{
								VertData.Lon = CurInstance->WCSGeographic[0];
								VertData.Lat = CurInstance->WCSGeographic[1];
								VertData.Elev = 0; // doesn't matter, AFAIK, as we're calculating this very thing...

								Flags = (WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_RELEL |
									WCS_VERTEXDATA_FLAG_WATERELEV | WCS_VERTEXDATA_FLAG_WATERDEPTH |    
									WCS_VERTEXDATA_FLAG_LAKEAPPLIED | WCS_VERTEXDATA_FLAG_STREAMAPPLIED |
									WCS_VERTEXDATA_FLAG_WAVEHEIGHT);
								RendData.Interactive->VertexDataPoint(&RendData, &VertData, &Poly, Flags);

								// for clarity, we use a lot of extraneous variables here...
								ObjectAbsElev = CurInstance->ExportXYZ[2] + Master->ExportRefData.RefElev;
								TerrainAbsElev = VertData.Elev; // including effect of all Effects
								ObjectRelOffset = (ObjectAbsElev - TerrainAbsElev);

								ZOffsetRelativeToTerrain = ObjectRelOffset;
								} // if

							WriteVTSTEntry(fVTST, CurInstance->ExportXYZ[0] + Master->ExportRefData.ExportRefLon, CurInstance->ExportXYZ[1] + Master->ExportRefData.ExportRefLat, ZOffsetRelativeToTerrain, Heading, MaxScale, TempFullPath, NULL);
							} // if
						} // for

					// finish file
					WriteVTSTFooter(fVTST);
					fclose(fVTST);
					fVTST = NULL;
					} // if

				} // if
			
			AppMem_Free(ObjectsDoneList, min(TotalInstances, Master->Unique3DObjectInstances) * sizeof(Object3DEffect *));
			} // if
		else
			Success = 0;

		// clean up image textures that are still in the scene export root dir
		if(Success)
			{
			const char *MaterialImageFile;
			for(MaterialImageFile = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_3DOBJTEX); MaterialImageFile; MaterialImageFile = (*FileNamesCreated)->FindNextNameOfType(WCS_EXPORTCONTROL_FILETYPE_3DOBJTEX, MaterialImageFile))
				{
				PROJ_remove(MaterialImageFile);
				} // for

			for(MaterialImageFile = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_WALLTEX); MaterialImageFile; MaterialImageFile = (*FileNamesCreated)->FindNextNameOfType(WCS_EXPORTCONTROL_FILETYPE_WALLTEX, MaterialImageFile))
				{
				PROJ_remove(MaterialImageFile);
				} // for

			for(MaterialImageFile = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_ROOFTEX); MaterialImageFile; MaterialImageFile = (*FileNamesCreated)->FindNextNameOfType(WCS_EXPORTCONTROL_FILETYPE_ROOFTEX, MaterialImageFile))
				{
				PROJ_remove(MaterialImageFile);
				} // for
			} // if

		} // if
	} // if

return (Success);

} // ExportFormatVTP::Process3DObjects

/*===========================================================================*/

int ExportFormatVTP::ProcessCameras(const char *OutputFilePath, FILE *FScene)
{
FILE *floc, *fvtap;
EffectList *CurCameraEL;
Camera *CurCamera;
double X, Y;
int Success = 1, AnimCam;
RenderData RendData(NULL);
RasterAnimHostProperties Prop;
char FileName[512], BaseName[100];

// need to reproject locations points to WGS84 regardless of actual coordinate system

// export cameras
if (Master->ExportCameras && (CurCameraEL = Master->Cameras))
	{
	sprintf(RelativeLocFileName, "%s.loc", Master->OutPath.GetName());
	sprintf(BaseName, "Locations/%s.loc", Master->OutPath.GetName());
	fprintf(FScene, "\t<Locations_File>%s.loc</Locations_File>\n", Master->OutPath.GetName());
	strmfp(FileName, OutputFilePath, BaseName);
	if (floc = PROJ_fopen(FileName, "w"))
		{
		fprintf(floc, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"); // XML header
		fprintf(floc, "<locations-file file-format-version=\"1.0\">\n"); // XML Document element
		while (CurCameraEL)
			{
			if (CurCameraEL->Me)
				{
				fprintf(floc, " <location>\n");
				CurCamera = (Camera *)CurCameraEL->Me;
				if (RendData.InitToView(EffectsHost, ProjectHost, DBHost, ProjectHost->Interactive, NULL, CurCamera, 320, 320))
					{
					// is camera animated?
					Prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
					Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATED;
					Prop.ItemOperator = WCS_KEYOPERATION_ALLKEYS;
					Prop.TypeNumber = WCS_EFFECTSSUBCLASS_CAMERA;
					CurCamera->GetRAHostProperties(&Prop);
					AnimCam = (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED) ? 1: 0;

					if(!FirstLocName[0])
						{
						strcpy(FirstLocName, CurCamera->Name);
						} // if
					fprintf(floc, "  <name>%s</name>\n", CurCamera->Name);

					// make a false target 10m from camera
					CurCamera->TargPos->XYZ[0] = CurCamera->CamPos->XYZ[0] + CurCamera->TargPos->XYZ[0] * 10.0;
					CurCamera->TargPos->XYZ[1] = CurCamera->CamPos->XYZ[1] + CurCamera->TargPos->XYZ[1] * 10.0;
					CurCamera->TargPos->XYZ[2] = CurCamera->CamPos->XYZ[2] + CurCamera->TargPos->XYZ[2] * 10.0;
					
					#ifdef WCS_BUILD_VNS
					RendData.DefCoords->CartToDeg(CurCamera->CamPos);
					RendData.DefCoords->CartToDeg(CurCamera->TargPos);
					#else // WCS_BUILD_VNS
					CurCamera->CamPos->CartToDeg(RendData.PlanetRad);
					CurCamera->TargPos->CartToDeg(RendData.PlanetRad);
					#endif // WCS_BUILD_VNS

					if (IsProjected)
						{ // we have to do what Master->RBounds.DefDegToRBounds() does, but use WGS84 geographic instead
						VertexDEM Vert;
						Vert.xyz[0] = Vert.Lon = CurCamera->CamPos->Lon;
						Vert.xyz[1] = Vert.Lat = CurCamera->CamPos->Lat;
						WGS84Sys->DefDegToProj(&Vert);
						// .xyz[0] is longitude in WCS convention of positive west -- need to negate for GIS convention
						X = -Vert.xyz[0]; 
						Y = Vert.xyz[1];
						} // if
					else
						{
						Master->RBounds.DefDegToRBounds(CurCamera->CamPos->Lat, CurCamera->CamPos->Lon, X, Y);
						} // else
					fprintf(floc, "  <point1>%.12f,%.12f,%f</point1>\n", X, Y, CurCamera->CamPos->Elev);

					if (IsProjected)
						{ // we have to do what Master->RBounds.DefDegToRBounds() does, but use WGS84 geographic instead
						VertexDEM Vert;
						Vert.xyz[0] = Vert.Lon = CurCamera->TargPos->Lon;
						Vert.xyz[1] = Vert.Lat = CurCamera->TargPos->Lat;
						WGS84Sys->DefDegToProj(&Vert);
						// .xyz[0] is longitude in WCS convention of positive west -- need to negate for GIS convention
						X = -Vert.xyz[0];
						Y = Vert.xyz[1];
						} // if
					else
						{
						Master->RBounds.DefDegToRBounds(CurCamera->TargPos->Lat, CurCamera->TargPos->Lon, X, Y);
						} // else
					fprintf(floc, "  <point2>%.12f,%.12f,%f</point2>\n", X, Y, CurCamera->TargPos->Elev);

					if (AnimCam)
						{
						Point3d lookFrom, lookTo;
						long key = 0;

						//sprintf(RelativeLocFileName, "%s.vtap", Master->OutPath.GetName());
						sprintf(BaseName, "Locations/%s_%s.vtap", Master->OutPath.GetName(), CurCamera->Name);
						fprintf(FScene, "\t<AnimPath>%s_%s.vtap</AnimPath>\n", Master->OutPath.GetName(), CurCamera->Name);
						strmfp(FileName, OutputFilePath, BaseName);
						if (fvtap = PROJ_fopen(FileName, "w"))
							{
							fprintf(fvtap, "<?xml version=\"1.0\"?>\n");						// XML header
							fprintf(fvtap, "<animation-path file-format-version=\"1.0\">\n");	// XML Document element
							do
								{
								if (RendData.Cam->TargetObj)
									RendData.Cam->TargetObj->SetToTime((double)key / RendData.FrameRate);
								RendData.Cam->SetToTime((double)key / RendData.FrameRate);
								RendData.Cam->InitToRender(NULL, NULL);
								RendData.Cam->InitFrameToRender(GlobalApp->AppEffects, &RendData);

								/*** F2_NOTE: Values need to be in WGS84 only ***/
								lookFrom[0] = RendData.Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue;
								lookFrom[1] = -RendData.Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue;
								lookFrom[2] = RendData.Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue;
								lookTo[0] = RendData.Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].CurValue;
								lookTo[1] = -RendData.Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].CurValue;
								lookTo[2] = RendData.Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].CurValue;

								/***
								if (CurCamera->CameraType == WCS_EFFECTS_CAMERATYPE_UNTARGETED)
									{
									lookTo[0] = -CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].CurValue;
									lookTo[1] = -CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].CurValue;
									lookTo[2] = CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].CurValue;
									} // if untargeted
								else if (CurCamera->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED)
									{
									lookTo[0] = -CurCamera->CamPitch;
									lookTo[1] = -CurCamera->CamHeading;
									lookTo[2] = CurCamera->CamBank;
									} // else if targeted
								else if (CurCamera->CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD)
									{
									lookTo[0] = -CurCamera->CamPitch;
									lookTo[1] = -CurCamera->CamHeading;
									lookTo[2] = CurCamera->CamBank;
									} // else if overhead
								else if (CurCamera->CameraType == WCS_EFFECTS_CAMERATYPE_ALIGNED)
									{
									lookTo[0] = -CurCamera->CamPitch;
									lookTo[1] = -CurCamera->CamHeading;
									lookTo[2] = CurCamera->CamBank;
									} // else if aligned
								***/

								fprintf(fvtap, "	<location p1=\"%f,%f,%f\" p2=\"%f,%f,%f\" time=\"%f\" hfov=\"%f\"/>\n",
									(float)lookFrom[1], (float)lookFrom[0], (float)lookFrom[2],
									(float)lookTo[1], (float)lookTo[0], (float)lookTo[2],
									((double)key / RendData.FrameRate), RendData.Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue);

								key = RendData.Cam->GetNextMotionKeyFrame(key, RendData.FrameRate);
								} while (key >= 0);

							fprintf(fvtap, "</animation-path>\n");
							fclose(fvtap);
							fvtap = NULL;
							} // if fvtap
						} // if
					} // if RendData
				else
					Success = 0;

				fprintf(floc, " </location>\n");
				} // if
			CurCameraEL = CurCameraEL->Next;
			} // while
		fprintf(floc, "</locations-file>\n");
		fclose(floc); floc = NULL;
		} // if
	} // if

return (Success);

} // ExportFormatVTP::ProcessCameras
