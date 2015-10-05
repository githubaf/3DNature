// ExportFormatNV.cpp
// Code module for NatureView export code
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
//#include "TerrainWriter.h"
#include "CubeSphere.h"
#include "AppMem.h"
#include "RasterResampler.h"
#include "Log.h"
#include "SceneExportGUI.h"
#include "SXExtension.h"
#include "UsefulZip.h"
#include "NatureViewCrypto.h"
#include "Security.h"
#include "zlib.h"
#include "Lists.h"

ExportFormatNV::ExportFormatNV(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource)
: ExportFormat(MasterSource, ProjectSource, EffectsSource, DBSource, ImageSource)
{
RelativeFoliageFileName[0] = RelativeSpeciesFileName[0] = 0;
RelativeSkyFileName[0] = RelativeStructFileName[0] = 0;
ZipBuilder = NULL;

} // ExportFormatNV::ExportFormatNV

/*===========================================================================*/

ExportFormatNV::~ExportFormatNV()
{

} // ExportFormatNV::~ExportFormatNV

/*===========================================================================*/

int ExportFormatNV::SanityCheck(void)
{
int Success = 1;

#ifdef WCS_BUILD_VNS
if (Master->Coords && (Master->Coords->Method.GCTPMethod != 0))
	{
	UserMessageOK("NatureView Export", "NatureView currently does not support non-Geographic export.\nAborting export job.");
	return(0);
	} // if

if (!stricmp(Master->ImageFormat, "JPEG 2000") || !stricmp(Master->ImageFormat, "Lossless JPEG 2000"))
	{
	if (Master->TexResX * Master->TexResY > 166666666) // ~12909^2
		{
		UserMessageOK("NatureView Export", "Your texture export size exceeds the 500Mb limit\n(approx 166 million pixels) of the JPEG2000 compressor.");
		return(0);
		} // if
	if (Master->DEMResX * Master->DEMResY > 125000000) // ~11180^2
		{
		UserMessageOK("NatureView Export", "Your DEM export size exceeds the 500Mb limit\n(approx 125 million cells) of the JPEG2000 DEM compressor.");
		return(0);
		} // if
	} // if

// ECW only allows 8-bit DEM storage, so DEM capacity goes further, larger limits
if (!stricmp(Master->ImageFormat, "ERMapper ECW"))
	{
	if (Master->TexResX * Master->TexResY > 166666666) // ~12909^2
		{
		UserMessageOK("NatureView Export", "Your texture export size exceeds the 500Mb limit\n(approx 166 million pixels) of the ECW compressor.");
		return(0);
		} // if
	if (Master->DEMResX * Master->DEMResY > 500000000) // ~22360^2
		{
		UserMessageOK("NatureView Export", "Your DEM export size exceeds the 500Mb limit\n(approx 500 million cells) of the ECW DEM compressor.");
		return(0);
		} // if
	} // if

#endif // WCS_BUILD_VNS

if (ZipBuilder)
	{
	delete ZipBuilder;
	ZipBuilder = NULL;
	} // if

return (Success);

} // ExportFormatNV::SanityCheck

/*===========================================================================*/

int ExportFormatNV::PackageExport(NameList **FileNamesCreated)
{
long FileType;
int Success = 1, VFExists = 0, SpeciesExists = 0;
const char *FileNameOfKnownType, *OutputFilePath;
const char *IndexFileName;
FILE *FScene = NULL;
PathAndFile SceneOutput, SceneAux;
char TempFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN], TempFullPathB[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN], NVWFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN],
 TempFullPathC[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];
RasterAnimHostProperties Prop;
int NVKeyID;
char DependentSigA[100], DependentSigB[100], DependentSigC[100], DependentSigAStr[100], DependentSigBStr[100], DependentSigCStr[100];

#ifdef WCS_BUILD_VNS
NVKeyID = NVW_KEY_SX1_VNS_2;
#else // WCS (!VNS)
NVKeyID = NVW_KEY_SX1_WCS_6;
#endif // WCS (!VNS)


// The directory where all the files should be created is:
OutputFilePath = Master->OutPath.GetPath();

SceneAux.SetPath((char *)Master->OutPath.GetPath());

if (Master->ZipItUp)
	{
	if (ZipBuilder = new Zipper)
		{
		strcpy(TempFullPath, (char *)Master->OutPath.GetPath());
		ZipBuilder->SetBaseDirectory(GlobalApp->MainProj->MungPath(TempFullPath));
		} // if
	} // if

// process foliage. WCS-style list needs to be recreated
if (Success = ProcessFoliageList(FileNamesCreated, OutputFilePath))
	{
	// process sky cubic images into the preferred spherical form
	if (Success = ProcessSky(FileNamesCreated, OutputFilePath))
		{
		SceneAux.SetPath((char *)Master->OutPath.GetPath());

		SceneOutput.SetPath((char *)Master->OutPath.GetPath());
		SceneOutput.SetName((char *)Master->OutPath.GetName());

		SceneOutput.GetFramePathAndName(TempFullPath, ".nvw", 0, WCS_PATHANDFILE_PATH_PLUS_NAME_LEN, 0);
		if (FScene = PROJ_fopen(TempFullPath, "w"))
			{
			strcpy(NVWFullPath, TempFullPath);
			if (ZipBuilder)
				{
				strcpy(TempFullPath, SceneOutput.GetName());
				strcat(TempFullPath, ".nvw");
				ZipBuilder->AddToZipList(TempFullPath);
				} // if
			// write out NV scene file, fetching names of the files NV is interested in from the File name list
			// if file types do not exist then the user did not wish to export them.
			fprintf(FScene, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"); // XML header
			fprintf(FScene, "<NATUREVIEW>\n"); // top-level document element

			fprintf(FScene, " <VERSION fileversion=\".01\" preferredviewer=\"NatureView Express\"></VERSION>\n");
			fprintf(FScene, " <META");
			fprintf(FScene, " name=\"%s\"", 
				Master->FormatExtension && ((SXExtensionNVE *)Master->FormatExtension)->MetaName ? 
				((SXExtensionNVE *)Master->FormatExtension)->MetaName: GlobalApp->MainProj->projectname);  // metadata element
			if (Master->FormatExtension && ((SXExtensionNVE *)Master->FormatExtension)->MetaCopyright)
				fprintf(FScene, " copyright=\"%s\"", ((SXExtensionNVE *)Master->FormatExtension)->MetaCopyright); 
			if (Master->FormatExtension && ((SXExtensionNVE *)Master->FormatExtension)->MetaAuthor)
				fprintf(FScene, " author=\"%s\"", ((SXExtensionNVE *)Master->FormatExtension)->MetaAuthor); 
			if (Master->FormatExtension && ((SXExtensionNVE *)Master->FormatExtension)->MetaEmail)
				fprintf(FScene, " email=\"%s\"", ((SXExtensionNVE *)Master->FormatExtension)->MetaEmail);
			fprintf(FScene, " userone=\"%s\"", 
				Master->FormatExtension && ((SXExtensionNVE *)Master->FormatExtension)->MetaUser1 ? ((SXExtensionNVE *)Master->FormatExtension)->MetaUser1: Master->Name);
			if (Master->FormatExtension && ((SXExtensionNVE *)Master->FormatExtension)->MetaUser2)
				fprintf(FScene, " usertwo=\"%s\"", ((SXExtensionNVE *)Master->FormatExtension)->MetaUser2);
			fprintf(FScene, "></META>\n");

			double TN, TS, TE, TW, DEMCellSizeX, DEMCellSizeY, ImgCellSizeX, ImgCellSizeY;
			
			Master->RBounds.DeriveCoords(Master->DEMResY, Master->DEMResX);
			Master->RBounds.FetchBoundsCentersGIS(TN, TS, TW, TE);
			Master->RBounds.FetchCellSizeGIS(DEMCellSizeY, DEMCellSizeX);

			Master->RBounds.DeriveCoords(Master->TexResY, Master->TexResX);
			Master->RBounds.FetchCellSizeGIS(ImgCellSizeY, ImgCellSizeX);

			fprintf(FScene, " <BOUNDS minx=%f maxx=%f miny=%f maxy=%f elevcellsx=%d elevcellsy=%d elevdimx=%f elevdimy=%f imgcellsx=%d imgcellsy=%d imgdimx=%f imgdimy=%f>\n",
			 TE, TW, TS, TN, //min/max X, min/max Y
			 Master->DEMResX, Master->DEMResY, // elev cells X, Y
			 DEMCellSizeX, DEMCellSizeY, // elev cell dimension X, Y
			 Master->TexResX, Master->TexResY, // image cells X, Y
			 ImgCellSizeX, ImgCellSizeY // image cell dimension X, Y
			 ); // bounds metadata, not used by NV, but helpful to humans
			fprintf(FScene, " </BOUNDS>\n"); // closing scene-level element

			fprintf(FScene, " <SCENE name=\"%s\" thumb=\"%s\">\n", Master->Name, ""); // opening scene-level element
			fprintf(FScene, "  <LOD"); // LOD tuning begin tag
			if (Master->FolTransparencyStyle == WCS_EFFECTS_SCENEEXPORTER_FOLTRANSPARSTYLE_ALPHA)
				{
				fprintf(FScene, " foliagequality=\"HIGH\""); // Alpha blend high foliage quality
				} // if
			else
				{
				fprintf(FScene, " foliagequality=\"LOW\""); // Alpha clip low foliage quality
				} // else

			// TERRAINCLIP FOLIAGECLIP OBJECTCLIP OBJECTBOX LABELCLIP
			fprintf(FScene, " terrainclip=\"%f\"", Master->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LODDISTVANISH].CurValue);
			fprintf(FScene, " foliageclip=\"%f\"", Master->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_FOLDISTVANISH].CurValue);
			fprintf(FScene, " objectclip=\"%f\"", Master->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OBJECTDISTVANISH].CurValue);
			fprintf(FScene, " objectbox=\"%f\"", Master->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_OBJECTDISTBOX].CurValue);
			fprintf(FScene, " labelclip=\"%f\"", Master->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_LABELDISTVANISH].CurValue);

			if (Master->FormatExtension)
				{
				fprintf(FScene, " minfeaturesizepixels=\"%d\"", ((SXExtensionNVE *)Master->FormatExtension)->LODMinFeatureSizePixels);
				fprintf(FScene, " maxfoliagestems=\"%d\"", ((SXExtensionNVE *)Master->FormatExtension)->LODMaxFoliageStems); // 
				if (((SXExtensionNVE *)Master->FormatExtension)->LODCompressTerrainTex)
					fprintf(FScene, " compressterraintex=\"%d\"", ((SXExtensionNVE *)Master->FormatExtension)->LODCompressTerrainTex); // 
				if (((SXExtensionNVE *)Master->FormatExtension)->LODCompressFoliageTex)
					fprintf(FScene, " compressfoliagetex=\"%d\"", ((SXExtensionNVE *)Master->FormatExtension)->LODCompressFoliageTex); // 
				if (((SXExtensionNVE *)Master->FormatExtension)->LODOptimizeMove)
					fprintf(FScene, " optimizemove=\"%d\"", ((SXExtensionNVE *)Master->FormatExtension)->LODOptimizeMove); // 
				} // if
			fprintf(FScene, "></LOD>\n"); // LOD tuning end tag

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
			if (Master->FormatExtension)
				{
				fprintf(FScene, "  <NAVIGATION");
				fprintf(FScene, " speed=\"%g\"", ((SXExtensionNVE *)Master->FormatExtension)->NavUseDefaultSpeed ? (int)(min(xspace, zspace) * 100.0): ((SXExtensionNVE *)Master->FormatExtension)->NavSpeed);
				fprintf(FScene, " type=\"SIMPLE\""); // 
				if (((SXExtensionNVE *)Master->FormatExtension)->NavFollowTerrainHeight != 0)
					fprintf(FScene, " followterrainheight=\"%g\"", ((SXExtensionNVE *)Master->FormatExtension)->NavFollowTerrainHeight); // 
				fprintf(FScene, " friction=\"%g\"", ((SXExtensionNVE *)Master->FormatExtension)->NavFriction); // 
				fprintf(FScene, " acceleration=\"%g\"", ((SXExtensionNVE *)Master->FormatExtension)->NavAcceleration); // 
				fprintf(FScene, " inertia=\"%g\"", ((SXExtensionNVE *)Master->FormatExtension)->NavInertia); // 
				if (((SXExtensionNVE *)Master->FormatExtension)->NavConstrain)
					fprintf(FScene, " constrain=\"%d\"", ((SXExtensionNVE *)Master->FormatExtension)->NavConstrain); // 
				if (((SXExtensionNVE *)Master->FormatExtension)->NavMaxHtConstrain)
					fprintf(FScene, " followterrainmaxheight=\"%g\"", ((SXExtensionNVE *)Master->FormatExtension)->NavFollowTerrainMaxHeight);
				fprintf(FScene, "></NAVIGATION>\n");
				} // if

			// Texture name
			TempFullPath[0] = NULL;
			const char *TexName = "";
			DependentSigBStr[0] = NULL;
			FileType = WCS_EXPORTCONTROL_FILETYPE_TEX1;
			if (FileNameOfKnownType = (*FileNamesCreated)->FindNameOfType(FileType))
				{
				TexName = FileNameOfKnownType;

				// Reset Path that was tinkered with, above
				SceneAux.SetPath((char *)Master->OutPath.GetPath());

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

				// assemble drape image signature
				if (GenerateNVWDependentFileSignature(TexName, NVKeyID, DependentSigB))
					{
					sprintf(DependentSigBStr, " drapesig=\"%s\"", DependentSigB);
					} // if

				sprintf(TempFullPathB, " drapeimagefilename=\"%s\"", TexName);
				if (ZipBuilder) ZipBuilder->AddToZipList(TexName);

				} // if

			// second (usually foliage) texture image
			const char *TexNameC = "";
			DependentSigCStr[0] = NULL;
			TempFullPathC[0] = NULL;
			FileType = WCS_EXPORTCONTROL_FILETYPE_TEX2;
			if (FileNameOfKnownType = (*FileNamesCreated)->FindNameOfType(FileType))
				{
				TexNameC = FileNameOfKnownType;

				// Reset Path that was tinkered with, above
				SceneAux.SetPath((char *)Master->OutPath.GetPath());

				// assemble drape image signature
				if (GenerateNVWDependentFileSignature(TexNameC, NVKeyID, DependentSigC))
					{
					sprintf(DependentSigCStr, " foldrapesig=\"%s\"", DependentSigC);
					} // if

				sprintf(TempFullPathC, " foldrapeimagefilename=\"%s\"", TexNameC);
				if (ZipBuilder) ZipBuilder->AddToZipList(TexNameC);

				} // if


			// DEM (ELEV) name (and drapeimagename if available)
			const char *ELEVName = "";
			FileType = WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN;
			if (FileNameOfKnownType = (*FileNamesCreated)->FindNameOfType(FileType))
				{
				double MaxElev = 1.0, MinElev = 1.0;
				ELEVName = FileNameOfKnownType;

				MaxElev = Master->MaxRenderedElevation;
				MinElev = Master->MinRenderedElevation;

				// assemble ELEV signature
				DependentSigAStr[0] = NULL;
				if (GenerateNVWDependentFileSignature(ELEVName, NVKeyID, DependentSigA))
					{
					sprintf(DependentSigAStr, " demsig=\"%s\"", DependentSigA);
					} // if

				fprintf(FScene, "  <DEM filename=\"%s\" maxelev=\"%f\" minelev=\"%f\"%s%s%s%s%s></DEM>\n", ELEVName, MaxElev, MinElev, TempFullPathB, TempFullPathC, DependentSigAStr, DependentSigBStr, DependentSigCStr);
				if (ZipBuilder) ZipBuilder->AddToZipList(ELEVName);
				} // if

			if (RelativeFoliageFileName[0])
				{
				// assemble Instance signature
				DependentSigAStr[0] = NULL;
				if (GenerateNVWDependentFileSignature(RelativeFoliageFileName, NVKeyID, DependentSigA))
					{
					sprintf(DependentSigAStr, " sig=\"%s\"", DependentSigA);
					} // if

				fprintf(FScene, "  <INSTANCEFILE ");
				if (Master->FoliageStyle == WCS_EFFECTS_SCENEEXPORTER_FOLSTYLE_CROSSBOARDS)
					{
					fprintf(FScene, "foliagetype=\"CROSSBOARDS\" ");
					} // if
				fprintf(FScene, "filename=\"%s\"%s></INSTANCEFILE>\n", RelativeFoliageFileName, DependentSigAStr);

				if (ZipBuilder) ZipBuilder->AddToZipList(RelativeFoliageFileName);
				} // if

			if (RelativeStructFileName[0])
				{
				// assemble Instance signature
				DependentSigAStr[0] = NULL;
				if (GenerateNVWDependentFileSignature(RelativeStructFileName, NVKeyID, DependentSigA))
					{
					sprintf(DependentSigAStr, " sig=\"%s\"", DependentSigA);
					} // if
				fprintf(FScene, "  <3DOBJFILE filename=\"%s\"%s></3DOBJFILE>\n", RelativeStructFileName, DependentSigAStr);
				if (ZipBuilder) ZipBuilder->AddToZipList(RelativeStructFileName);
				} // if

			if (RelativeSkyFileName[0])
				{
				// assemble sky signature
				DependentSigAStr[0] = NULL;
				if (GenerateNVWDependentFileSignature(RelativeSkyFileName, NVKeyID, DependentSigA))
					{
					sprintf(DependentSigAStr, " sig=\"%s\"", DependentSigA);
					} // if
				fprintf(FScene, "  <SKY type=\"SPHERE\">\n   <KEY time=\"0.0\" filename=\"%s\"%s></KEY>\n  </SKY>\n", RelativeSkyFileName, DependentSigAStr);
				if (ZipBuilder) ZipBuilder->AddToZipList(RelativeSkyFileName);
				} // if

			ProcessCameras(FScene);
			ProcessLights(FScene);

			// export haze
			EffectList *CurHaze;
			if (Master->ExportHaze && (CurHaze = Master->Haze))
				{
				if (CurHaze->Me) ExportHaze((Atmosphere *)CurHaze->Me, FScene);
				} // if


			// Export 3D Objects
			Process3DObjects(FileNamesCreated, FScene, OutputFilePath, NVKeyID);

			if (Master->FormatExtension)
				{
				// Logo
				const char *LogoName;
				fprintf(FScene, "  <OVERLAY");
				LogoName = ((SXExtensionNVE *)Master->FormatExtension)->OverlayLogoFileName.GetName();

				if (LogoName[0])
					{
					char DependentSigA[100];

					// get real path and name (LogoName doesn't have full path)
					((SXExtensionNVE *)Master->FormatExtension)->OverlayLogoFileName.GetPathAndName(TempFullPath);

					// assemble Logo image signature
					DependentSigA[0] = NULL;
					// use TempFullPath and not LogoName, as we need the full path
					if (GenerateNVWDependentFileSignature(GlobalApp->MainProj->MungPath(TempFullPath), NVKeyID, DependentSigA))
						{
						fprintf(FScene, " logofilename=\"%s\"", LogoName);
						fprintf(FScene, " logosig=\"%s\"", DependentSigA);
						} // if
					// copy overlay image to output directory
					CopyExistingFile(TempFullPath, OutputFilePath, ((SXExtensionNVE *)Master->FormatExtension)->OverlayLogoFileName.GetName());
					if (ZipBuilder)
						{
						ZipBuilder->AddToZipList(((SXExtensionNVE *)Master->FormatExtension)->OverlayLogoFileName.GetName());
						} // if
					} // if
				if (((SXExtensionNVE *)Master->FormatExtension)->OverlayLogoText)
					fprintf(FScene, " text=\"%s\"", ((SXExtensionNVE *)Master->FormatExtension)->OverlayLogoText);
				fprintf(FScene, " numlines=\"%d\"", ((SXExtensionNVE *)Master->FormatExtension)->OverlayNumLines);
				fprintf(FScene, " showmap=\"%d\"", ((SXExtensionNVE *)Master->FormatExtension)->OverlayShowMap);
				fprintf(FScene, "></OVERLAY>\n");

				// Watermark
				if (((SXExtensionNVE *)Master->FormatExtension)->WatermarkText)
					fprintf(FScene, "  <WATERMARK text=\"%s\"></WATERMARK>\n", ((SXExtensionNVE *)Master->FormatExtension)->WatermarkText);
				} // if

			// this needs to be written after all action-generating items are done.
			Master->CloseQueryFiles(); // close and flush objects/actions files so we can hash them
			#ifdef WCS_BUILD_SX2
			// only write these tags if these files were successfully generated
			const char *SXObjectRecords = "", *SXObjectIndex = "", *SXActionRecords = "", *SXActionIndex = "";
			SXObjectRecords = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_QUERYOBJECT);
			SXObjectIndex = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_QUERYOBJECTIDX);
			SXActionRecords = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_QUERYACTION);
			SXActionIndex = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_QUERYACTIONIDX);
			if (SXObjectRecords && SXObjectIndex && SXActionRecords && SXActionIndex)
				{
				// assemble Object record signature
				DependentSigA[0] = NULL;
				GenerateNVWDependentFileSignature(SXObjectRecords, NVKeyID, DependentSigA);
				// assemble Object record signature
				DependentSigB[0] = NULL;
				GenerateNVWDependentFileSignature(SXActionRecords, NVKeyID, DependentSigB);
				fprintf(FScene, "  <ACTION objectrecordsfile=\"SXObjectRecords.nqa\" objectindexfile=\"SXObjectIndex.nqx\" actionrecordsfile=\"SXActionRecords.nqa\" actionindexfile=\"SXActionIndex.nqx\" objectrecordssig=\"%s\" actionrecordssig=\"%s\" ></ACTION>\n", DependentSigA, DependentSigB); // ACTION

				if (ZipBuilder) ZipBuilder->AddToZipList(SXObjectRecords);
				if (ZipBuilder) ZipBuilder->AddToZipList(SXObjectIndex);
				if (ZipBuilder) ZipBuilder->AddToZipList(SXActionRecords);
				if (ZipBuilder) ZipBuilder->AddToZipList(SXActionIndex);
				} // if
			#endif // WCS_BUILD_SX2


			fprintf(FScene, " </SCENE>\n"); // closing scene-level element
			fprintf(FScene, "</NATUREVIEW>\n"); // closing top-level element
			//fprintf(FScene, "<!-- SIGNATURE type=PRERELEASE value=\"%s\" -->", "Blank");
			fclose(FScene);
			FScene = NULL;

			// cryptographically sign the file
			AppendSignNVWSceneFile(GlobalApp->MainProj->MungPath(NVWFullPath), NVKeyID);
			} // if


		// dispose of now-unneeded foliage files.
		// BlueMesa_Fol.dat
		// BlueMesa_FolFoliageList0.dat
		// BlueMesa_FolFoliageList0_stripped.dat
		FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILEIDX;
		for(IndexFileName = (*FileNamesCreated)->FindNameOfType(FileType); IndexFileName; IndexFileName = (*FileNamesCreated)->FindNextNameOfType(FileType, IndexFileName))
			{
			strmfp(TempFullPathB, OutputFilePath, IndexFileName);
			PROJ_remove(TempFullPathB);
			} // for
		FileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILE;
		for(IndexFileName = (*FileNamesCreated)->FindNameOfType(FileType); IndexFileName; IndexFileName = (*FileNamesCreated)->FindNextNameOfType(FileType, IndexFileName))
			{
			strmfp(TempFullPathB, OutputFilePath, IndexFileName);
			PROJ_remove(TempFullPathB);
			} // for

		} // if 
	} // if

if (ZipBuilder)
	{
	sprintf(TempFullPath, "%s.nvz", (char *)Master->OutPath.GetName());
	SceneAux.SetPath((char *)Master->OutPath.GetPath());
	SceneAux.SetName(TempFullPath);
	SceneAux.GetPathAndName(TempFullPath);
	// remove files after archiving
	ZipBuilder->SetRemoveStateOnAll(true);
	ZipBuilder->ZipToFile(GlobalApp->MainProj->MungPath(TempFullPath)); 
	delete ZipBuilder;
	ZipBuilder = NULL;

	// clean up 3DO subdir now that it's empty
	strcpy(TempFullPath, Master->OutPath.GetName());
	strcat(TempFullPath, "_3DOs");
	SceneAux.SetName(TempFullPath);
	SceneAux.GetPathAndName(TempFullPathB);
	PROJ_rmdir(TempFullPathB);

	// clean up Images/exportname subdir now that it's empty
	strcpy(TempFullPath, "Images/");
	strcat(TempFullPath, Master->OutPath.GetName());
	SceneAux.SetName(TempFullPath);
	SceneAux.GetPathAndName(TempFullPathB);
	PROJ_rmdir(TempFullPathB);

	// clean up Images subdir now that it's empty
	strcpy(TempFullPath, "Images");
	SceneAux.SetName(TempFullPath);
	SceneAux.GetPathAndName(TempFullPathB);
	PROJ_rmdir(TempFullPathB);

	} // if

return (Success);

} // ExportFormatNV::PackageExport

/*===========================================================================*/

int ExportFormatNV::ProcessFoliageList(NameList **FileNamesCreated, const char *OutputFilePath)
{
long FileType, DatPt;
int Success = 1;
const char *IndexFileName, *DataFileName;
Raster *CurRast;
char ImageName[256], FileName[512], TestFileVersion;
RealtimeFoliageIndex Index;
RealtimeFoliageCellData RFCD;
RealtimeFoliageData FolData;
FILE *ffile, *nvifile;
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

			if ((Index.NumCells > 0) && (Index.CellDat->DatCt > 0))
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
						strcat(FileName, ".nvi");
						if (nvifile = PROJ_fopen(FileName, "wb"))
							{
							unsigned long int ByteOrderSignature = 0xaabbccdd, NumEntries, Version, SeekBack;
							unsigned char TypeMask = 0;

							strcpy(RelativeFoliageFileName, IndexFileName);
							StripExtension(RelativeFoliageFileName);
							strcat(RelativeFoliageFileName, ".nvi");

							// file descriptor
							fprintf(nvifile, "NVIN");
							fwrite((char *)&ByteOrderSignature, sizeof(long), 1, nvifile);
							#ifdef WCS_BUILD_SX2
							if (GlobalApp->Sentinal->CheckAuthFieldSX2())
								Version = 0x00000101; // Super, Major, Minor, Rev
							else
								Version = 0x00000001; // Super, Major, Minor, Rev
							#else // WCS_BUILD_SX2
							Version = 0x00000001; // Super, Major, Minor, Rev
							#endif // WCS_BUILD_SX2
							fwrite((char *)&Version, sizeof(long), 1, nvifile);

							// extents (blank for now)
							double MinX, MaxX, MinY, MaxY, MinZ, MaxZ, MaxHt, MinHt;
							MinX = MaxX = MinY = MaxY = MinZ = MaxZ = MaxHt = MinHt = FLT_MAX; // FLT_MAX indicates unset
							fwrite((char *)&MinX, sizeof(double), 1, nvifile);
							fwrite((char *)&MaxX, sizeof(double), 1, nvifile);
							fwrite((char *)&MinY, sizeof(double), 1, nvifile);
							fwrite((char *)&MaxY, sizeof(double), 1, nvifile);
							fwrite((char *)&MinZ, sizeof(double), 1, nvifile);
							fwrite((char *)&MaxZ, sizeof(double), 1, nvifile);
							fwrite((char *)&MinHt, sizeof(double), 1, nvifile);
							fwrite((char *)&MaxHt, sizeof(double), 1, nvifile);

							// Typemask
							TypeMask = 0x01; // foliage only. 0x02 indicated 3DOs
							fwrite((char *)&TypeMask, sizeof(char), 1, nvifile);

							// NumEntries
							NumEntries = Index.CellDat->DatCt;
							fwrite((char *)&NumEntries, sizeof(long), 1, nvifile);

							// write IMGL
							fprintf(nvifile, "IMGL");
							unsigned long int IMGLSize = 0, IMGLPos = 0;
							IMGLPos = ftell(nvifile);
							fwrite((char *)&IMGLSize, sizeof(long), 1, nvifile);

							Raster *RastLoop;
							int ImageLoop;
							unsigned long int IMGLNumImages = 0;
							unsigned char ImageFlag = 0;

							// count Images
							for(ImageLoop = 1, RastLoop = GlobalApp->AppImages->GetFirstRast(); RastLoop; ImageLoop++, RastLoop = GlobalApp->AppImages->GetNextRast(RastLoop))
								{
								// nothing but counting
								} // for

							IMGLNumImages = ImageLoop - 1;
							fwrite((char *)&IMGLNumImages, sizeof(long), 1, nvifile);
							IMGLSize += 4;

							for(ImageLoop = 1, RastLoop = GlobalApp->AppImages->GetFirstRast(); RastLoop; ImageLoop++, RastLoop = GlobalApp->AppImages->GetNextRast(RastLoop))
								{
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

										if (ZipBuilder) ZipBuilder->AddToZipList(ImageName);
										} // if
									} // if
								float WriteWidthFac;
								WriteWidthFac = (float)(RastLoop->Rows != 0 ? ((float)RastLoop->Cols / (float)RastLoop->Rows) : 1.0f);
								fwrite((char *)&WriteWidthFac, sizeof(float), 1, nvifile);
								fwrite((char *)&ImageFlag, sizeof(char), 1, nvifile);
								fprintf(nvifile, "%s", FileName);
								fputc(0, nvifile); // Explicitly write NULL
								IMGLSize += (4 + 1 + (unsigned long)strlen(FileName) + 1); // WidthFac + Flag + string + NULL
								} // for
						
							// rewrite IMGL Size at IMGLPos
							SeekBack = ftell(nvifile);
							fseek(nvifile, IMGLPos, SEEK_SET);
							fwrite((char *)&IMGLSize, sizeof(long), 1, nvifile);
							fseek(nvifile, SeekBack, SEEK_SET);

							// INLS
							fprintf(nvifile, "INLS");
							unsigned long int INLSSize = 0, INLSPos = 0;
							INLSPos = ftell(nvifile);
							fwrite((char *)&INLSSize, sizeof(long), 1, nvifile);

							// INLS extents (blank for now)
							double INLSMinX, INLSMaxX, INLSMinY, INLSMaxY, INLSMinZ, INLSMaxZ, INLSMaxHt, INLSMinHt;
							double INLSOrigX, INLSOrigY, INLSOrigZ;
							INLSMinX = INLSMaxX = INLSMinY = INLSMaxY = INLSMinZ = INLSMaxZ = INLSMaxHt = INLSMinHt = FLT_MAX; // FLT_MAX = undefined

							INLSOrigX = Index.RefXYZ[0];
							INLSOrigY = Index.RefXYZ[1];
							INLSOrigZ = Index.RefXYZ[2];

							fwrite((char *)&INLSMinX, sizeof(double), 1, nvifile);
							fwrite((char *)&INLSMaxX, sizeof(double), 1, nvifile);
							fwrite((char *)&INLSMinY, sizeof(double), 1, nvifile);
							fwrite((char *)&INLSMaxY, sizeof(double), 1, nvifile);
							fwrite((char *)&INLSMinZ, sizeof(double), 1, nvifile);
							fwrite((char *)&INLSMaxZ, sizeof(double), 1, nvifile);

							fwrite((char *)&INLSOrigX, sizeof(double), 1, nvifile);
							fwrite((char *)&INLSOrigY, sizeof(double), 1, nvifile);
							fwrite((char *)&INLSOrigZ, sizeof(double), 1, nvifile);

							fwrite((char *)&INLSMinHt, sizeof(double), 1, nvifile);
							fwrite((char *)&INLSMaxHt, sizeof(double), 1, nvifile);

							// INLS Typemask
							TypeMask = 0x01; // foliage only. 0x02 indicated 3DOs
							fwrite((char *)&TypeMask, sizeof(char), 1, nvifile);

							long FilePos;
							FilePos = ftell(nvifile);
							FilePos += 8; // sizeof(UL64)
							#ifdef SIXTYFOUR_BIT_SUPPORTED
							UL64 Seek64;
							Seek64 = CurPos;
							fwrite((char *)&Seek64, sizeof(UL64), 1, nvifile);
							#else // !SIXTYFOUR_BIT_SUPPORTED
							// <<<>>> implement 32-bit writing...
							#endif // !SIXTYFOUR_BIT_SUPPORTED

							// INLS entries point to the 4-byte chunkID of an INFL, for safety/validation after a seek
							
							// rewrite INLS Size at INLSPos
							SeekBack = ftell(nvifile);
							fseek(nvifile, INLSPos, SEEK_SET);
							fwrite((char *)&INLSSize, sizeof(long), 1, nvifile);
							fseek(nvifile, SeekBack, SEEK_SET);



							// INFL
							fprintf(nvifile, "INFL");
							unsigned long int INFLSize = 0, INFLPos = 0;
							INFLPos = ftell(nvifile);
							fwrite((char *)&INFLSize, sizeof(long), 1, nvifile);

							for (DatPt = 0; DatPt < Index.CellDat->DatCt; DatPt ++)
								{
								if (FolData.ReadFoliageRecord(ffile, Index.FileVersion))
									{
									if (FolData.InterpretFoliageRecord(NULL, GlobalApp->AppImages, &PointData)) // don't need full decoding of 3dobjects, just height, etc
										{
										// need to properly encode species if > 127
										if (FolData.ElementID > -1)
											{
											// it turns out, the way the NVE reader is written, these 16-bit and 32-bit
											// EID encodings are effectively Motorola/BigEndian byte order, so we
											// need to write them that way.
											if (FolData.ElementID >= 16384) // we can only describe up to 14 bits if top two bits are flags
												{ // write as long
												unsigned long EIDLong;
												//EIDLong = (unsigned long)FolData.ElementID; --> Lint points out that this doesn't work the way you think it does
												EIDLong = (unsigned short)FolData.ElementID;
												EIDLong |= 0xC0000000; // top two bits on = 32-bit (well, 30-bit anyway)
												//fwrite((char *)&EIDLong, sizeof (unsigned long), 1, nvifile);
												PutB32U(EIDLong, nvifile);
												INFLSize += 4;
												} // if
											else if (FolData.ElementID > 127)
												{
												unsigned short EIDShort;
												EIDShort = (unsigned short)FolData.ElementID;
												EIDShort |= 0x8000; // top bit on = 16-bit
												//fwrite((char *)&EIDShort, sizeof (unsigned short), 1, nvifile);
												PutB16U(EIDShort, nvifile);
												INFLSize += 2;
												} // if
											else // (FolData.ElementID !> 127) // we can use bit 7 as data if bit 8 is 0
												{
												unsigned char EIDChar;
												EIDChar = (unsigned char)FolData.ElementID;
												// endian doesn't matter with single bytes, so we use fwrite
												fwrite((char *)&EIDChar, sizeof (char), 1, nvifile);
												INFLSize += 1;
												} // if

											long ClickTableEntry = -1;
											unsigned char InstanceFlags;
											InstanceFlags = 0;
											if (FolData.BitInfo & WCS_REALTIME_FOLDAT_BITINFO_FLIPX) // flipX
												{
												InstanceFlags |= 0x01;
												} // if
											if (FolData.BitInfo & WCS_REALTIME_FOLDAT_BITINFO_SHADE3D) // shade3d
												{
												InstanceFlags |= 0x02;
												} // if
											if (FolData.ImageColorOpacity < 255) // not 100% opacity, need color and opacity values
												{
												if (Version == 0x01)
													InstanceFlags |= 0x03; // <<<>>> this 0x03 value is wrong, but has to be changed carefully
												// compensate for error in version 1
												else
													InstanceFlags |= 0x04;
												} // if
											if (FolData.BitInfo & WCS_REALTIME_FOLDAT_BITINFO_LABEL) // label
												{
												InstanceFlags |= 0x08;
												// these are only interpreted this way for labels
												if (FolData.BitInfo & WCS_REALTIME_FOLDAT_BITINFO_LEFTPOLE) // label pole left
													{
													InstanceFlags |= 0x10;
													} // if
												else if (FolData.BitInfo & WCS_REALTIME_FOLDAT_BITINFO_RIGHTPOLE) // label pole right
													{
													InstanceFlags |= 0x20;
													} // if
												} // if
											#ifdef WCS_BUILD_SX2
											if (Version != 0x00000001 && (FolData.BitInfo & WCS_REALTIME_FOLDAT_BITINFO_CLICKQUERY) && FolData.MyEffect) // click-to-query
												{
												if (FolData.MyEffect->EffectType == WCS_EFFECTSSUBCLASS_FOLIAGE)
													ClickTableEntry = ((FoliageEffect *)FolData.MyEffect)->GetRecordNumber(FolData.MyEffect, FolData.MyVec, FileNamesCreated);
												else if (FolData.MyEffect->EffectType == WCS_EFFECTSSUBCLASS_LABEL)
													ClickTableEntry = ((Label *)FolData.MyEffect)->GetRecordNumber(FolData.MyEffect, FolData.MyVec, FileNamesCreated);
												else if (FolData.MyEffect->EffectType == WCS_EFFECTSSUBCLASS_OBJECT3D)
													ClickTableEntry = ((Object3DEffect *)FolData.MyEffect)->GetRecordNumber(FolData.MyEffect, FolData.MyVec, FileNamesCreated);
												if (ClickTableEntry >= 0)
													InstanceFlags |= 0x40;
												} // if
											#endif // WCS_BUILD_SX2
											fwrite((char *)&InstanceFlags, sizeof(char), 1, nvifile);
											INFLSize += 1;

											// height (WCS encoded)
											fwrite((char *)&FolData.Height, sizeof(unsigned short), 1, nvifile);
											INFLSize += 2;

											fwrite((char *)&FolData.XYZ[0], sizeof(float), 1, nvifile);
											fwrite((char *)&FolData.XYZ[1], sizeof(float), 1, nvifile);
											fwrite((char *)&FolData.XYZ[2], sizeof(float), 1, nvifile);
											INFLSize += 12;

											// optional color and opacity values
											// compensate for error in version 1
											if ((Version == 0x01 && (InstanceFlags & 0x03)) ||
												(Version != 0x01 && (InstanceFlags & 0x04)))
												{
												fwrite((char *)&FolData.ImageColorOpacity, sizeof(char), 1, nvifile);
												fwrite((char *)&FolData.TripleValue[0], sizeof(char), 1, nvifile);
												fwrite((char *)&FolData.TripleValue[1], sizeof(char), 1, nvifile);
												fwrite((char *)&FolData.TripleValue[2], sizeof(char), 1, nvifile);
												INFLSize += 4;
												} // if
											#ifdef WCS_BUILD_SX2
											// optional click-to-query object table entry
											if (InstanceFlags & 0x40)
												{
												// write the click-to-query object table entry number
												fwrite((char *)&ClickTableEntry, sizeof(long), 1, nvifile);
												} // if
											#endif // WCS_BUILD_SX2
											
											{
											char DebugStr[600];

											// Decode Height
											long HtInterp, HeightBits;
											double HeightNum;
											float FloatHeight;
											HtInterp = ((FolData.Height & ~0x3fff) >> 14);
											HeightBits = (FolData.Height & 0x3fff);
											HeightNum = (double)HeightBits;
											FloatHeight = (float)(HtInterp == 0x00 ? FolData.Height :
											HtInterp == 0x01 ? HeightNum * .1 :
											HtInterp == 0x02 ? HeightNum * .01 :
											HeightNum * .001);

											if(FloatHeight > 30)
												{
												sprintf(DebugStr, "%d: %x, %f\n", FolData.ElementID, FolData.Height, FloatHeight);
												OutputDebugStr(DebugStr);
												} // if
											} // temp

											} // if
										else
											{
											FolData.ElementID = FolData.ElementID; // ERROR
											} // else
										} // if
									} // if
								} // for

							// rewrite INFL Size at INFLPos
							SeekBack = ftell(nvifile);
							fseek(nvifile, INFLPos, SEEK_SET);
							fwrite((char *)&INFLSize, sizeof(long), 1, nvifile);
							// What is the purpose of this fseek? The file gets closed next? -GRH
							// CXH: In case we decide to add additional functionality and data after this block,
							// I wanted to leave the file descriptor in a known and logical state.
							fseek(nvifile, SeekBack, SEEK_SET);
							// must do this as it's not dynamically allocated and RealtimeFoliageIndex
							// destructor will blow chunks if we don't
							Index.CellDat = NULL;
							fclose(nvifile);
							nvifile = NULL;
							} // if
						} // if
					fclose(ffile);

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

} // ExportFormatNV::ProcessFoliageList


/*===========================================================================*/

int ExportFormatNV::ProcessSky(NameList **FileNamesCreated, const char *OutputFilePath)
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

						if (Success == 0)
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

						if (Success == 0)
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

						if (Success == 0)
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

						if (Success == 0)
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

						if (Success == 0)
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

						if (Success == 0)
							{ // mark as unavailable but proceed
							delete FaceRast[STC_CUBEFACENAME_BOT]; FaceRast[STC_CUBEFACENAME_BOT] = NULL;
							} // if
						Success = 1; // go on and try next

						if (OutputRast = new Raster)
							{
							char PanoName[WCS_PATHANDFILE_NAME_LEN];
							OutputRast->Rows = max(FaceRast[STC_CUBEFACENAME_BOT]->Rows, 256); // this seems like a reasonable size to steal
							OutputRast->Cols = OutputRast->Rows * 2; // twice as wide as high (360 degrees wide, 180 high)

							// Setup a path and name with a suffix
							strcpy(PanoName, Master->OutPath.GetName());
							strcat(PanoName, "_Sphere");
							strcat(PanoName, ImageSaverLibrary::GetDefaultExtension(Master->FoliageImageFormat));

							// allocate and clear raster
							OutputRast->AllocRGBBands();

							// Resample
							for(int OutY = 0; OutY < OutputRast->Rows; OutY++)
								{
								double HemiLat;
								HemiLat = 90.0 - (180.0 * ((double)OutY / (double)(OutputRast->Rows - 1))); // Y=0:Lat=90 ... Y=Rows:Lat=-90
								for(int OutX = 0; OutX < OutputRast->Cols; OutX++)
									{
									double HemiLon, FaceX, FaceY;
									unsigned long int ImageX, ImageY, OutZip, InZip;
									unsigned char CubeFace;

									// adapted Lon start and end for NatureView
									// -180.0 + 
									HemiLon = -180 + (360.0 * ((double)OutX / (double)(OutputRast->Cols - 1))); // X=0:Lon=-180 ... X=Cols:Lon=+180

									if (SphereToCube(HemiLon, HemiLat, CubeFace, FaceX, FaceY))
										{
										if (FaceRast[CubeFace])
											{
											int WrappedX;
											// calculate image coords
											ImageX = (unsigned long)min(FaceX * (FaceRast[CubeFace]->Cols - 1), FaceRast[CubeFace]->Cols - 1);
											ImageY = (unsigned long)min(FaceY * (FaceRast[CubeFace]->Rows - 1), FaceRast[CubeFace]->Rows - 1);

											WrappedX = (OutX + (OutputRast->Cols / 2)) % OutputRast->Cols;

											OutZip = WrappedX + OutY * OutputRast->Cols;
											InZip = ImageX + ImageY * FaceRast[CubeFace]->Cols;

											OutputRast->ByteMap[0][OutZip] = FaceRast[CubeFace]->ByteMap[0][InZip];
											OutputRast->ByteMap[1][OutZip] = FaceRast[CubeFace]->ByteMap[1][InZip];
											OutputRast->ByteMap[2][OutZip] = FaceRast[CubeFace]->ByteMap[2][InZip];
											} // if
										} // if
									} // for
								} // for

							OutputRast->PAF.SetPathAndName((char *)OutputFilePath, (char *)PanoName);
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

} // ExportFormatNV::ProcessSky

/*===========================================================================*/

int ExportFormatNV::FindCopyable3DObjFile(Object3DEffect *Object3D)
{
char DummyString[512];

return (ExportFormat3DS::FindFile(Object3D, DummyString) || ExportFormatLW::FindFile(Object3D, DummyString));

} // ExportFormatNV::FindCopyable3DObjFile

/*===========================================================================*/

int ExportFormatNV::Process3DObjects(NameList **FileNamesCreated, FILE *fNVW, const char *OutputFilePath, int NVKeyID)
{
long TotalInstances, ObjsDone, ObjCt, DoIt, MatCt, BaseMatCt = 0;
ExportFormat3DS *ObjExporter3DS = NULL;
ExportFormatLW *ObjExporterLW = NULL;
Object3DInstance *CurInstance;
Object3DEffect *CurObj, **ObjectsDoneList;
FormatSpecificFile CurFile;
int Success = 1, ExporterMadeObject;
char TempFullPath[512], TempFullPathB[512], ObjectPath[512], NVExCurrentDirStash[1024];
PathAndFile SceneAux;

SceneAux.SetPath((char *)OutputFilePath);
// Ensure we have a subdirectory to place all the files in for organization

strcpy(TempFullPath, Master->OutPath.GetName());
strcat(TempFullPath, "_3DOs");
strcpy(ObjectPath, TempFullPath);
SceneAux.SetName(TempFullPath);
SceneAux.GetPathAndName(TempFullPathB);
PROJ_mkdir(TempFullPathB);
SceneAux.AppendDirToPath(TempFullPath);



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
			_getcwd(NVExCurrentDirStash, 1023);
			if ((ObjExporter3DS = new ExportFormat3DS(Master, ProjectHost, EffectsHost, DBHost, Images)) && (ObjExporterLW = new ExportFormatLW(Master, ProjectHost, EffectsHost, DBHost, Images)))
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
								// checks to see if the object file exists and if it is a 3DS file as shown by the file suffix ".3ds"
								if (Master->ObjectTreatment == WCS_EFFECTS_SCENEEXPORTER_OBJECTTREATMENT_COPYANY && (ObjExporter3DS->FindFile(CurObj, TempFullPath) || ObjExporterLW->FindFile(CurObj, TempFullPath)))
									{
									Success = CopyExistingFile(TempFullPath, SceneAux.GetPath(), CurObj->FileName);
									} // if
								else
									{
									// open an object file
									if (CurFile = ObjExporterLW->OpenObjectFile(SceneAux.GetPath(), NULL, NULL, SceneAux.GetName()))
									//if (CurFile = ObjExporter3DS->OpenObjectFile(SceneAux.GetPath(), SceneAux.GetName()))
										{
										// save object
										ExporterMadeObject = Master->FindInEphemeralList(CurObj);
										// by passing NULL arguments for the two strings, there is no way to recover the names of the VertexMap and the FullImageName
										// the LW object exporter can make copies of texture images if it needs to, and sometimes it needs to to use them for clip maps
										// Make sure the images get written to the Name List.
										if (! ObjExporterLW->SaveOneObject(CurFile, CurObj, CurInstance, NULL, NULL, ExporterMadeObject, FileNamesCreated))
										//if (! ObjExporter3DS->SaveOneObject(CurFile, CurObj, SceneAux.GetPath(), TRUE, BaseMatCt, NULL, NULL))	// TRUE=SaveMaterials
											Success = 0;
										// close file, also writes the file so can fail
										if (! ObjExporterLW->CloseFile(CurFile))
										//if (! ObjExporter3DS->CloseFile(CurFile))
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
				} // if
			else
				{
				Success = 0;
				} // else

			// restore working dir if exporter messed with it
			_chdir(NVExCurrentDirStash); // PROJ_chdir not needed, this is a path returned directly from _getcwd()

			if (ObjExporter3DS)
				delete ObjExporter3DS;
			if (ObjExporterLW)
				delete ObjExporterLW;

			if (Success)
				{
				char DependentSigA[100];
				// write 3D objects entities file

				// write 3D Object records
				for(CurInstance = Master->ObjectInstanceList; CurInstance; CurInstance = CurInstance->Next)
					{
					if (CurObj = CurInstance->MyObj)
						{
						Point4d AxisAngle;
						char *FullPath = NULL;
						char OrigObjPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];
						// figure out a file name
						if (Master->ObjectTreatment == WCS_EFFECTS_SCENEEXPORTER_OBJECTTREATMENT_COPYANY && FindFile(CurObj, OrigObjPath))
							{
							// this directory should already exist by this point...
							strcpy(TempFullPath, Master->OutPath.GetName());
							strcat(TempFullPath, "_3DOs/");

							Success = CopyExistingFile(OrigObjPath, TempFullPath, CurObj->FileName);
							strcpy(TempFullPath, CurObj->FileName);
							} // if
						else
							{
							strcpy(TempFullPath, CurObj->GetName());
							ReplaceChar(TempFullPath, '.', '_');
							//strcat(TempFullPath, ".3ds");
							strcat(TempFullPath, ".lwo");
							} // else

						fprintf(fNVW, "  <OBJINSTANCE ");
						sprintf(TempFullPathB, "%s/%s", ObjectPath, TempFullPath);

						// assemble 3DO Instance signature
						if (GenerateNVWDependentFileSignature(GlobalApp->MainProj->MungPath(TempFullPathB), NVKeyID, DependentSigA))
							{
							fprintf(fNVW, "sig=\"%s\" ", DependentSigA);
							} // if

						#ifdef WCS_BUILD_SX2
						if (CurInstance->ClickQueryObjectID >= 0)
							{
							fprintf(fNVW, "filename=\"%s\" actionid=\"%d\">\n", TempFullPathB, CurInstance->ClickQueryObjectID);
							} // if
						else
							fprintf(fNVW, "filename=\"%s\">\n", TempFullPathB);
						#else
						fprintf(fNVW, "filename=\"%s\">\n", TempFullPathB);
						#endif // WCS_BUILD_SX2
						fprintf(fNVW, "   <KEY time=\"0.0\" ");
						fprintf(fNVW, "x=\"%f\" y=\"%f\" elevation=\"%f\" ", -(CurInstance->ExportXYZ[0] + Master->ExportRefData.ExportRefLon), CurInstance->ExportXYZ[1] + Master->ExportRefData.ExportRefLat, CurInstance->ExportXYZ[2] + Master->ExportRefData.RefElev);
						// Euler[0] is Pitch, Euler[1] is Heading, Euler[2] is Bank
						//fprintf(fNVW, "pitch=\"%f\" heading=\"%f\" bank=\"%f\" ", CurInstance->Euler[0], CurInstance->Euler[1], CurInstance->Euler[2]);
						fprintf(fNVW, "qi=\"%f\" qj=\"%f\" qk=\"%f\" qw=\"%f\" ", CurInstance->Quaternion[0], CurInstance->Quaternion[1], CurInstance->Quaternion[2], CurInstance->Quaternion[3]);
						QuaternionToAxisAngle(AxisAngle, CurInstance->Quaternion);
						AxisAngle[3] *= PiOver180;
						// AXISX AXISY AXISZ AXISANGLE
						// AxisAngle[0], -AxisAngle[1], AxisAngle[2], -AxisAngle[3]);
						fprintf(fNVW, "axisx=\"%f\" axisy=\"%f\" axisz=\"%f\" axisangle=\"%f\" ", AxisAngle[0], AxisAngle[2], AxisAngle[1], AxisAngle[3]); // Y and Z are flipped in OSG/NV convention

						if (ZipBuilder)
							{
							// Add to Zip list
							ZipBuilder->AddToZipList(TempFullPathB);
							} // if


						fprintf(fNVW, "scalex=\"%f\" scaley=\"%f\" scalez=\"%f\" ", CurInstance->Scale[0], CurInstance->Scale[2], CurInstance->Scale[1]);
						fprintf(fNVW, "></KEY>\n");
						fprintf(fNVW, "  </OBJINSTANCE>\n");

						} // if
					} // for

				} // if
			AppMem_Free(ObjectsDoneList, min(TotalInstances, Master->Unique3DObjectInstances) * sizeof(Object3DEffect *));
			} // if
		else
			{
			Success = 0;
			} // else

		if (Success && ZipBuilder)
			{
			const char *MaterialImageFile;
			char FullQualifiedMaterialImageName[512];

			// create the subdir path
			//strcpy(FullQualifiedMaterialImageName, Master->OutPath.GetName());
			//strcat(FullQualifiedMaterialImageName, "_3DOs");
			//strcpy(ObjectPath, FullQualifiedMaterialImageName);
			//strcpy(ObjectPath, "Images");
			sprintf(ObjectPath, "Images/%s", Master->OutPath.GetName());

			for(MaterialImageFile = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_3DOBJTEX); MaterialImageFile; MaterialImageFile = (*FileNamesCreated)->FindNextNameOfType(WCS_EXPORTCONTROL_FILETYPE_3DOBJTEX, MaterialImageFile))
				{
				// Add to Zip list
				sprintf(FullQualifiedMaterialImageName, "%s/%s", ObjectPath, MaterialImageFile);
				ZipBuilder->AddToZipList(FullQualifiedMaterialImageName);
				} // for

			for(MaterialImageFile = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_WALLTEX); MaterialImageFile; MaterialImageFile = (*FileNamesCreated)->FindNextNameOfType(WCS_EXPORTCONTROL_FILETYPE_WALLTEX, MaterialImageFile))
				{
				// Add to Zip list
				sprintf(FullQualifiedMaterialImageName, "%s/%s", ObjectPath, MaterialImageFile);
				ZipBuilder->AddToZipList(FullQualifiedMaterialImageName);
				} // for

			for(MaterialImageFile = (*FileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_ROOFTEX); MaterialImageFile; MaterialImageFile = (*FileNamesCreated)->FindNextNameOfType(WCS_EXPORTCONTROL_FILETYPE_ROOFTEX, MaterialImageFile))
				{
				// Add to Zip list
				sprintf(FullQualifiedMaterialImageName, "%s/%s", ObjectPath, MaterialImageFile);
				ZipBuilder->AddToZipList(FullQualifiedMaterialImageName);
				} // for
			} // if

		// clean up image textures that are still in the scene export root dir
		if (Success)
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

} // ExportFormatNV::Process3DObjects

/*===========================================================================*/

int ExportFormatNV::ExportHaze(Atmosphere *CurHaze, FILE *FScene)
{
float nearplane, neardensity, range, fardensity, fogcolorr, fogcolorg, fogcolorb;
unsigned char FogR, FogG, FogB;

if (FScene)
	{
	if (CurHaze->HazeEnabled)
		{
		nearplane = (float)(CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].CurValue);
		neardensity = (float)(CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY].CurValue);
		range = (float)(CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE].CurValue);
		fardensity = (float)(CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY].CurValue);
		fogcolorr = (float)(CurHaze->HazeColor.GetClampedCompleteValue(0));
		fogcolorg = (float)(CurHaze->HazeColor.GetClampedCompleteValue(1));
		fogcolorb = (float)(CurHaze->HazeColor.GetClampedCompleteValue(2));
		FogR = (unsigned char)(fogcolorr * 255.0f);
		FogG = (unsigned char)(fogcolorg * 255.0f);
		FogB = (unsigned char)(fogcolorb * 255.0f);
		fprintf(FScene, "  <HAZE type=\"LINEAR\">\n");
		fprintf(FScene, "   <KEY time=\"0.0\" start=\"%f\" range=\"%f\" startintensity=\"%f\" endintensity=\"%f\" color=\"#%02X%02X%02X\"></KEY>\n",
		 nearplane, range, neardensity, fardensity, FogR, FogG, FogB);
		fprintf(FScene, "  </HAZE>\n");
		} // if
	return(1);
	} // if

return(0);

} // ExportFormatNV::ExportHaze

/*===========================================================================*/

int ExportFormatNV::ProcessCameras(FILE *FScene)
{
double CX, CY, TX, TY, Bank, LoopTime, EndTime, LoopInt = .1;
EffectList *CurCameraEL;
Camera *CurCamera;
RenderData RendData(NULL);
RasterAnimHostProperties Prop;
int Success = 1, AnimCam;

// export cameras
if (Master->ExportCameras && (CurCameraEL = Master->Cameras))
	{
	while (CurCameraEL)
		{
		if (CurCameraEL->Me)
			{
			CurCamera = (Camera *)CurCameraEL->Me;

			fprintf(FScene, "  <CAMERA name=\"%s\">\n", CurCamera->Name);
			if (RendData.InitToView(EffectsHost, ProjectHost, DBHost, ProjectHost->Interactive, NULL, CurCamera, 320, 320))
				{
				// is camera animated?
				Prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_KEYRANGE);
				Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATED;
				Prop.ItemOperator = WCS_KEYOPERATION_ALLKEYS;
				Prop.TypeNumber = WCS_EFFECTSSUBCLASS_CAMERA;
				CurCamera->GetRAHostProperties(&Prop);
				AnimCam = (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED) ? 1: 0;
				EndTime = Prop.KeyNodeRange[1];

				for (LoopTime = 0.0; LoopTime <= EndTime; LoopTime += LoopInt)
					{
					RendData.RenderTime = LoopTime;
					CurCamera->SetToTime(LoopTime);
					if (CurCamera->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED && CurCamera->TargetObj)
						CurCamera->TargetObj->SetToTime(LoopTime);
					CurCamera->InitFrameToRender(GlobalApp->AppEffects, &RendData);

					// make a false target 1000m from camera
					CurCamera->TargPos->XYZ[0] = CurCamera->CamPos->XYZ[0] + CurCamera->TargPos->XYZ[0] * 1000.0;
					CurCamera->TargPos->XYZ[1] = CurCamera->CamPos->XYZ[1] + CurCamera->TargPos->XYZ[1] * 1000.0;
					CurCamera->TargPos->XYZ[2] = CurCamera->CamPos->XYZ[2] + CurCamera->TargPos->XYZ[2] * 1000.0;
					
					#ifdef WCS_BUILD_VNS
					RendData.DefCoords->CartToDeg(CurCamera->CamPos);
					RendData.DefCoords->CartToDeg(CurCamera->TargPos);
					#else // WCS_BUILD_VNS
					CurCamera->CamPos->CartToDeg(RendData.PlanetRad);
					CurCamera->TargPos->CartToDeg(RendData.PlanetRad);
					#endif // WCS_BUILD_VNS

					Master->RBounds.DefDegToRBounds(CurCamera->CamPos->Lat, CurCamera->CamPos->Lon, CX, CY);
					Master->RBounds.DefDegToRBounds(CurCamera->TargPos->Lat, CurCamera->TargPos->Lon, TX, TY);

					// plan cameras need target shifted slightly north in order to alleviate
					// up-vector ambiguity and preserve north-up stability
					if (CurCamera->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
						TY += ConvertMetersToDeg(.1, 0, RendData.PlanetRad); // actual exact planet radius is not too critical

					Bank = CurCamera->CamBank;
					if (fabs(Bank) >= 360.0)
						Bank = fmod(Bank, 360.0);	// retains the original sign
					if (Bank < 0.0)
						Bank += 360.0;

					// Slight offset so camera ends up with North up.  Copying X to try to prevent any rotation.
					if ((WCS_EFFECTS_CAMERATYPE_OVERHEAD == CurCamera->CameraType) || (WCS_EFFECTS_CAMERATYPE_PLANIMETRIC == CurCamera->CameraType))
						{
						TX = CX;
						TY = CY + 0.00001;
						} // if

					fprintf(FScene, "   <KEY time=\"%.12f\" latitude=\"%.12f\" longitude=\"%f\" elevation=\"%f\" targetlatitude=\"%.12f\" targetlongitude=\"%.12f\" targetelevation=\"%f\" hfov=\"%f\" bank=\"%f\"></KEY>\n",
					 LoopTime, CY, -CX, CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue,
					 TY, -TX, CurCamera->TargPos->Elev,
					 CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue, Bank);
					
					if (! AnimCam)
						break;
					} // for

				} // if RendData
			else
				Success = 0;

			fprintf(FScene, "  </CAMERA>\n");
			} // if
		CurCameraEL = CurCameraEL->Next;
		} // while
	} // if

return (Success);

} // ExportFormatNV::ProcessCameras

/*===========================================================================*/

int ExportFormatNV::ProcessLights(FILE *FScene)
{
VertexDEM VDEM; // for light aim calculations

// lights
if (Master->ExportLights)
	{
	double X, Y;
	float tmp[3];
	class EffectList *LiteList = Master->Lights;
	class Light *Lite;

	while (LiteList)
		{
		Lite = (class Light *)LiteList->Me;
		if (Lite->Enabled) // only export enabled lights
			{
			double EffectiveFalloff = 0.0;
			if (!Lite->Distant && Lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_FALLOFFEXP].CurValue > 0.0)
				{ // Distant automatically makes Falloff=0 for all light classes
				EffectiveFalloff = Lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_FALLOFFEXP].CurValue;
				} // if
			char ColorEncoder[20]; // we only need about 8
			fprintf(FScene, "  <LIGHT name=\"%s\" type=\"", Lite->Name);
			// we treat DISTANT SPOTS as DISTANT PARALLEL
			if (Lite->LightType == WCS_EFFECTS_LIGHTTYPE_PARALLEL || (Lite->LightType == WCS_EFFECTS_LIGHTTYPE_SPOT && Lite->Distant))
				{
				fprintf(FScene, "PARALLEL\">\n");
				fprintf(FScene, "   <KEY time=\"0.0\"");
				EncodeColorAsHTML(Lite->Color.CurValue[0], Lite->Color.CurValue[1], Lite->Color.CurValue[2], ColorEncoder);
				fprintf(FScene, " color=\"%s\"", ColorEncoder);

				tmp[0] = (float)min(1.0, Lite->Color.Intensity.CurValue);
				fprintf(FScene, " startintensity=\"%f\"", tmp[0]);

				if (EffectiveFalloff > 0.0)
					{
					fprintf(FScene, " falloffexp=\"%f\"", EffectiveFalloff);
					} // if

				VDEM.CopyXYZ(Lite->LightAim);
				VDEM.MultiplyXYZ(-1.0);
				VDEM.Lat = Master->ExportRefData.WCSRefLat;
				VDEM.Lon = Master->ExportRefData.WCSRefLon;
				VDEM.Elev = Master->ExportRefData.RefElev;
				VDEM.RotateToHome();

				tmp[0] = (float)VDEM.XYZ[0];
				tmp[1] = (float)VDEM.XYZ[2]; // NVE and WCS/VNS disagree on Y/Z axis order
				tmp[2] = (float)VDEM.XYZ[1]; // NVE and WCS/VNS disagree on Y/Z axis order

				fprintf(FScene, " axisx=\"%f\" axisy=\"%f\" axisz=\"%f\"", -tmp[0], -tmp[1], tmp[2]);

				fprintf(FScene, "></KEY>\n");
				} // WCS_EFFECTS_LIGHTTYPE_PARALLEL || (WCS_EFFECTS_LIGHTTYPE_SPOT+Distant)
			else if (Lite->LightType == WCS_EFFECTS_LIGHTTYPE_OMNI)
				{
				fprintf(FScene, "OMNI\">\n");
				fprintf(FScene, "   <KEY time=\"0.0\"");
				EncodeColorAsHTML(Lite->Color.CurValue[0], Lite->Color.CurValue[1], Lite->Color.CurValue[2], ColorEncoder);
				fprintf(FScene, " color=\"%s\"", ColorEncoder);

				tmp[0] = (float)min(1.0, Lite->Color.Intensity.CurValue);
				fprintf(FScene, " startintensity=\"%f\"", tmp[0]);

				tmp[0] = (float)min(500000, Lite->MaxIllumDist);
				fprintf(FScene, " size=\"%f\"", tmp[0]);

				// DISTANT doesn't affect aim of OMNI lights, since they have no aim, and
				// falloff inhibition is already handled above
				if (EffectiveFalloff > 0.0)
					{
					fprintf(FScene, " falloffexp=\"%f\"", EffectiveFalloff);
					} // if

				Master->RBounds.DefDegToRBounds(Lite->LightPos->Lat, Lite->LightPos->Lon, X, Y);
				fprintf(FScene, " x=\"%.12f\" y=\"%.12f\" z=\"%f\"", -X, Y, Lite->LightPos->Elev);

				fprintf(FScene, "></KEY>\n");
				} // WCS_EFFECTS_LIGHTTYPE_OMNI
			else if (Lite->LightType == WCS_EFFECTS_LIGHTTYPE_SPOT)
				{
				fprintf(FScene, "SPOT\">\n");
				fprintf(FScene, "   <KEY time=\"0.0\"");
				EncodeColorAsHTML(Lite->Color.CurValue[0], Lite->Color.CurValue[1], Lite->Color.CurValue[2], ColorEncoder);
				fprintf(FScene, " color=\"%s\"", ColorEncoder);

				tmp[0] = (float)min(1.0, Lite->Color.Intensity.CurValue);
				fprintf(FScene, " startintensity=\"%f\"", tmp[0]);

				tmp[0] = (float)min(500000, Lite->MaxIllumDist);
				fprintf(FScene, " size=\"%f\"", tmp[0]);

				fprintf(FScene, " hfov=\"%f\"", Lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].CurValue); // angle from side to side of beam, not from center to side

				fprintf(FScene, " edge=\"%f\"", Lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONEEDGE].CurValue);
				
				if (EffectiveFalloff > 0.0)
					{
					fprintf(FScene, " falloffexp=\"%f\"", EffectiveFalloff);
					} // if

				// we don't need to handle DISTANT condition here, as it is interpreted in PARALLEL DISTANT, above
				VDEM.CopyXYZ(Lite->LightAim);
				VDEM.MultiplyXYZ(-1.0);
				VDEM.Lat = Master->ExportRefData.WCSRefLat;
				VDEM.Lon = Master->ExportRefData.WCSRefLon;
				VDEM.Elev = Master->ExportRefData.RefElev;
				VDEM.RotateToHome();

				tmp[0] = (float)VDEM.XYZ[0];
				tmp[1] = (float)VDEM.XYZ[2]; // NVE and WCS/VNS disagree on Y/Z axis order
				tmp[2] = (float)VDEM.XYZ[1]; // NVE and WCS/VNS disagree on Y/Z axis order

				fprintf(FScene, " axisx=\"%f\" axisy=\"%f\" axisz=\"%f\"", -tmp[0], -tmp[1], tmp[2]);

				Master->RBounds.DefDegToRBounds(Lite->LightPos->Lat, Lite->LightPos->Lon, X, Y);
				fprintf(FScene, " x=\"%.12f\" y=\"%.12f\" z=\"%f\"", -X, Y, Lite->LightPos->Elev);

				fprintf(FScene, "></KEY>\n");
				} // WCS_EFFECTS_LIGHTTYPE_SPOT
			fprintf(FScene, "  </LIGHT>\n");
			} // if
		LiteList = LiteList->Next;
		} // while LiteList

	// export ambient lighting (from atmospheres)
	EffectList *CurHaze;
	if (Master->ExportHaze && (CurHaze = Master->Haze))
		{
		if (CurHaze->Me) ExportAmbient((Atmosphere *)CurHaze->Me, FScene);
		} // if

	} // if Lights

return(0); // doesn't mean anything

} // ExportFormatNV::ProcessLights

/*===========================================================================*/

int ExportFormatNV::ExportAmbient(Atmosphere *CurHaze, FILE *FScene)
{

if (FScene)
	{
	if (1) // ambient light portion of Atmospheres cannot be disabled
		{
		char ColorEncoder[20]; // we only need about 8

		// do Ambient reflected from Sky first
		fprintf(FScene, "  <LIGHT name=\"%s.SKY\" type=\"AMBIENT SKY\">\n", CurHaze->Name);
		fprintf(FScene, "   <KEY time=\"0.0\"");
		EncodeColorAsHTML(CurHaze->TopAmbientColor.GetCurValue(0), CurHaze->TopAmbientColor.GetCurValue(1), CurHaze->TopAmbientColor.GetCurValue(2), ColorEncoder);
		fprintf(FScene, " color=\"%s\"", ColorEncoder);
		fprintf(FScene, " startintensity=\"%f\"", CurHaze->TopAmbientColor.GetIntensity());
		fprintf(FScene, "></KEY>\n");
		fprintf(FScene, "  </LIGHT>\n");

		// do Ambient reflected from Ground first
		fprintf(FScene, "  <LIGHT name=\"%s.GND\" type=\"AMBIENT GROUND\">\n", CurHaze->Name);
		fprintf(FScene, "   <KEY time=\"0.0\"");
		EncodeColorAsHTML(CurHaze->BottomAmbientColor.GetCurValue(0), CurHaze->BottomAmbientColor.GetCurValue(1), CurHaze->BottomAmbientColor.GetCurValue(2), ColorEncoder);
		fprintf(FScene, " color=\"%s\"", ColorEncoder);
		fprintf(FScene, " startintensity=\"%f\"", CurHaze->BottomAmbientColor.GetIntensity());
		fprintf(FScene, "></KEY>\n");
		fprintf(FScene, "  </LIGHT>\n");

		} // if
	return(1);
	} // if

return(0);

} // ExportFormatNV::ExportAmbient

/*===========================================================================*/

int ExportFormatNV::FindFile(Object3DEffect *Object3D, char *FoundPath)
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
	if (! stricmp(Extension, "3ds"))
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

} // ExportFormatNV::FindFile
