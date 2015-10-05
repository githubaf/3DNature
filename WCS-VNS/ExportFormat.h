// ExportFormat.h
// header file for base and derived export classes
// Created from scratch 07/01/03 by Gary R. Huber
// Copyright 2003 Questar Productions. All rights reserved.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_EXPORTFORMAT_H
#define WCS_EXPORTFORMAT_H

/*** Steps to create a new scene exporter:

Security.h:
   Add enum for new format to RTX_FORMATS

Security.cpp:
   Add case for new format in CheckFormatRTX()

AuthorizeGUI.cpp:
   Add a CheckFormatRTX in ConfigureWidgets()
   Increment define WCS_AUTHGUI_MAX_STEPS

ExportFormat.h:
   Create new class definitions

ExportFormat????.cpp:
   Create your class code

ExportControlGUI.cpp:
   Add constructor in AllocExportFormat()
   Add test in FormatSpecificAuth()

ExportFormats.dbf:
   Add new row with allowed parameters (beware of bugs in Excel)

***/

class GeneralEffect;
class Atmosphere;
class Camera;
class CoordSys;
class Database;
class EffectList;
class EffectsLib;
class ExportReferenceData;
class ImageLib;
class Joe;
class Light;
class NameList;
class NewProject;
class Object3DEffect;
class ObjectMaterialEntry;
class Object3DInstance;
class PathAndFile;
class Project;
class RenderData;
class SceneExporter;
class SXQueryActionList;
class ThreeDSMotion;
class VectorExportItem;
class Zipper;

#include "3dsftk.h"
#include "Types.h"
#include "OpenFlight.h"
#include "PathAndFile.h"
#include "UsefulZip.h"

typedef void* FormatSpecificFile;

#ifdef WCS_BUILD_FBX
#include <fbxfilesdk/fbxfilesdk_def.h>
namespace FBXFILESDK_NAMESPACE
	{
	class KFbxNode;
	class KFbxSdkManager;
	class KFbxScene;
	class KFbxExporter;
	} // FBXFILESDK_NAMESPACE
#include <fbxfilesdk/fbxfilesdk_nsuse.h>
#endif // WCS_BUILD_FBX

class ExportFormat
	{
	public:
		SceneExporter *Master;
		ImageLib *Images;
		EffectsLib *EffectsHost;
		Database *DBHost;
		Project *ProjectHost;
		ExportFormat(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource)	
			{Master = MasterSource; EffectsHost = EffectsSource; ProjectHost = ProjectSource; DBHost = DBSource; Images = ImageSource;};
		virtual ~ExportFormat()	{};
		virtual int SanityCheck(void)	{return (1);};
		virtual int PackageExport(NameList **FileNamesCreated)	{return (1);};
		virtual int ProcessFoliageList(NameList **FileNamesCreated, const char *OutputFilePath)	{return (1);};
		virtual int ProcessSky(NameList **FileNamesCreated, const char *OutputFilePath)	{return (1);};
		virtual int Process3DObjects(NameList **FileNamesCreated, const char *OutputFilePath)	{return (1);};
		virtual FormatSpecificFile OpenSceneFile(const char *PathName, const char *FileName)	{return (0);};
		virtual FormatSpecificFile OpenObjectFile(const char *PathName, const char *FileName)	{return (0);};
		virtual int CloseFile(FormatSpecificFile fFile)	{return (1);};
		virtual int SaveAllObjects(int OneFile, FormatSpecificFile CurFile)	{return (1);};
		virtual int SaveOneObject(FormatSpecificFile fFile, Object3DEffect *Object3D, const char *SaveToPath, 
			int SaveMaterials, float *BoundMin, float *BoundMax)	{return (1);};
		virtual int CautionAlphaSave(void)	{return (0);};
		virtual int ProhibitAlphaSave(void)	{return (0);};
		virtual int FindCopyable3DObjFile(Object3DEffect *Object3D)	{return (0);};
		int CheckAndShortenFileName(char *OutputName, const char *FilePath, const char *FileName, long MaxLen);
		int CopyExistingFile(const char *OriginalFullPath, const char *PathName, const char *FileName);

	}; // class ExportFormat

class ExportFormatNV : public ExportFormat
	{
	public:
		char RelativeFoliageFileName[255], RelativeSpeciesFileName[255], RelativeSkyFileName[255], RelativeStructFileName[255];
		Zipper *ZipBuilder;

		ExportFormatNV(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource);
		~ExportFormatNV();
		virtual int SanityCheck(void);
		virtual int PackageExport(NameList **FileNamesCreated);
		virtual int ProcessFoliageList(NameList **FileNamesCreated, const char *OutputFilePath);
		virtual int ProcessSky(NameList **FileNamesCreated, const char *OutputFilePath);
		virtual int ProcessLights(FILE *FScene);
		int ExportAmbient(Atmosphere *CurHaze, FILE *FScene);
		virtual int Process3DObjects(NameList **FileNamesCreated, FILE *FScene, const char *OutputFilePath, int NVKeyID);
		virtual int FindCopyable3DObjFile(Object3DEffect *Object3D);
		int ExportHaze(Atmosphere *CurHaze, FILE *FScene);
		virtual int ProcessCameras(FILE *FScene);
		static int FindFile(Object3DEffect *Object3D, char *FoundPath);

	}; // class ExportFormatNV

class ExportFormatVTP : public ExportFormat
	{
	public:
		char RelativeFoliageFileName[255], RelativeSpeciesFileName[255], RelativeSkyFileName[255], RelativeStructFileName[255], RelativeLocFileName[255], FirstLocName[200];
		CoordSys *WGS84Sys;
		bool IsProjected;

		ExportFormatVTP(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource);
		~ExportFormatVTP();
		virtual int SanityCheck(void);
		virtual int PackageExport(NameList **FileNamesCreated);
		virtual int ProcessFoliageList(NameList **FileNamesCreated, const char *OutputFilePath);
		virtual int ProcessSky(NameList **FileNamesCreated, const char *OutputFilePath);
		virtual int Process3DObjects(NameList **FileNamesCreated, const char *OutputFilePath);
		virtual int FindCopyable3DObjFile(Object3DEffect *Object3D);
		virtual int ProcessCameras(const char *OutputFilePath, FILE *FScene);

	}; // class ExportFormatVTP

class ExportFormat3DS : public ExportFormat
	{
	public:
		database3ds *Database3ds;
		file3ds *File3ds;

		ExportFormat3DS(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource);
		~ExportFormat3DS();
		static int FindFile(Object3DEffect *Object3D, char *FoundPath);
		virtual int PackageExport(NameList **FileNamesCreated);
		virtual FormatSpecificFile OpenSceneFile(const char *PathName, const char *FileName);
		virtual FormatSpecificFile OpenObjectFile(const char *PathName, const char *FileName);
		virtual int CloseFile(FormatSpecificFile fFile);
		virtual int SaveAllObjects(int OneFile, FormatSpecificFile CurFile);
		virtual int SaveOneObject(FormatSpecificFile fFile, Object3DEffect *Object3D, const char *SaveToPath, 
			int SaveMaterials, long BaseMatCt, float *BoundMin, float *BoundMax);
		virtual int CautionAlphaSave(void)	{return (1);};	// Avoid transparency unless necessary
		int SaveOneMaterial(Object3DEffect *Object3D, 
			ObjectMaterialEntry *NameTable, long MatCt, const char *SaveToPath);
		FormatSpecificFile InitFile3DS(const char *PathName, const char *FileName);
		int CreateObjectMesh(Object3DEffect *Object3D, point3ds *VtxData, face3ds *PolyData, textvert3ds *TexData);
		void SetMaterialProperties(material3ds *Material3ds, Object3DEffect *Object3D, 
			ObjectMaterialEntry *NameTable, long MatCt, const char *SaveToPath);
		void FindObjectMeshBounds(ushort3ds nvertices, point3ds *vertexarray, float *boundmin, float *boundmax);
		int SaveObjectMotion(Object3DEffect *Object3D, Object3DInstance *CurInstance, ExportReferenceData *RefData,
			float *BoundsMin, float *BoundsMax, long InstanceNum);
		int ExportHaze(Atmosphere *CurHaze, long HazeNum);
		int ExportLight(Light *CurLight, long LightNum);
		int ExportSpotlightKeys(Light *CurLight, kfspot3ds **KFSpot3dsPtr, RenderData *RendData);
		int ExportOmnilightKeys(Light *CurLight, kfomni3ds **KFOmni3dsPtr, RenderData *RendData);
		int ExportCamera(Camera *CurCamera, long CameraNum);
		int ExportCameraKeys(Camera *CurCamera, kfcamera3ds **KFCamera3ds, RenderData *RendData);
		int Set_3DSCamKey(Camera *CurCamera, ThreeDSMotion *LWM, RenderData *RendData);

	}; // class ExportFormat3DS

class ExportFormatLW : public ExportFormat
	{
	public:
		FILE *SceneFile;

		ExportFormatLW(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource);
		~ExportFormatLW();
		static int FindFile(Object3DEffect *Object3D, char *FoundPath);
		virtual int PackageExport(NameList **FileNamesCreated);
		virtual FormatSpecificFile OpenSceneFile(const char *PathName, const char *FileName);
		virtual int CloseFile(FormatSpecificFile fFile);
		virtual int CautionAlphaSave(void)	{return (1);};	// LW renders much slower with transparency
		int CheckHVSpritesExist(void);
		int CheckHDInstancesExist(void);
		int SaveAllObjects(int OneFile, FormatSpecificFile CurFile, long TargetObjNum, long &ObjectsWritten, NameList **FileNamesCreated);
		FormatSpecificFile OpenObjectFile(const char *PathName, const char *ObjectDirName, const char *SubDirName, const char *FileName);
		int SaveOneObject(FormatSpecificFile CurFile, Object3DEffect *Object3D, Object3DInstance *CurInstance,
			char *VertMapName, char *FullImageName, int ExporterMadeObject, NameList **FileNamesCreated);
		long WriteChunkTag(FILE *LWFile, char *ChunkTag, long ChunkSize, int SizeBytes);
		int CloseChunk(FILE *LWFile, long SeekPos, int SizeBytes);
		int SaveObjectMotion(FormatSpecificFile CurFile, Object3DEffect *Object3D, Object3DInstance *CurInstance, ExportReferenceData *RefData, 
			const char *DirName, const char *FileName, long NumLights, long TargetObjNum, long HDInstanceCt,
			const char *VertMapName, const char *FullImageName);
		int ExportSceneDetail(FormatSpecificFile CurFile, EffectList *CameraList, int HVSpritesExist);
		int ExportLights(FormatSpecificFile CurFile, EffectList *LightList, Atmosphere *CurHaze);
		int ExportCameras(FormatSpecificFile CurFile, EffectList *CameraList);
		int ExportCameraTargets(FormatSpecificFile CurFile, EffectList *CameraList, long &ObjectsWritten);
		int ExportFoliageTarget(FormatSpecificFile CurFile, long &ObjectsWritten, int HDInstancesExist);
		int ExportSceneEffects(FormatSpecificFile CurFile, Atmosphere *CurHaze, int HVSpritesExist, int HDInstancesExist);
		int ExportRenderInterface(FormatSpecificFile CurFile, EffectList *CameraList);
		NameList *AddNewNameList(NameList **Names, char *NewName, long FileType);

	}; // class ExportFormatLW

class ExportFormatVRML : public ExportFormat
	{
	public:
		char RelativeFoliageFileName[255], RelativeSpeciesFileName[255];
		float AvgElev(FILE *RawElevs, long Cols, long Rows);
		float MinElev(FILE *RawElevs, long Cols, long Rows);

		ExportFormatVRML(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource);
		virtual ~ExportFormatVRML();
		virtual int SanityCheck(void);
		virtual int PackageExport(NameList **FileNamesCreated);
		virtual int ProcessFoliageList(NameList **FileNamesCreated, const char *OutputFilePath);
		virtual int CautionAlphaSave(void)	{return (1);};	// VRML has problems with alpha channel textures
		virtual int ProhibitAlphaSave(void)	{return (1);};	// VRML has problems with alpha channel textures
		void ProcessMatTexture(long MatCt, FILE *fVRML, Object3DEffect *CurObj);

	}; // class ExportFormatVRML

class ExportFormatVRMLSTL : public ExportFormatVRML
	{
	public:
		double ScaleFactor;
		float BuildEnvelope[3], XStep, ZStep;

		ExportFormatVRMLSTL(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource);
		~ExportFormatVRMLSTL();
		int Gen_IFS_Elevs(FILE *fRaw, FILE *fVRML, float ElevAdjust, long xdim, long ydim);
		float MinElev(FILE *RawElevs, long Cols, long Rows);
		virtual int PackageExport(NameList **FileNamesCreated);

	}; // class ExportFormatVRMLSTL

class ExportFormatSTL : public ExportFormat
	{
	public:
		float BuildEnvelope[3];

		ExportFormatSTL(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource);
		~ExportFormatSTL();
		float MinElev(FILE *RawElevs, long Cols, long Rows);
		virtual int PackageExport(NameList **FileNamesCreated);

	}; // class ExportFormatSTL

class ExportFormatOpenFlight : public ExportFormat
	{
	public:
		double bias_x, bias_y, FlapElev, ElevAdjust;
		FILE *fOF_Master;
		class OF_Header MasterHeader;
		class OF_ExtRef XRef;
		class OF_Group  Group;
		class OF_PopLevel  Pop;
		class OF_PushLevel Push;
		class OF_Continuation Continue;
		float *MinTable;
		float Ambient;
		unsigned long EmitFlags, EmitFlaps;
		long flaps_x, flaps_y, Tiled;

		void Cleanup(long Tiled, const char *OutputFilePath, NameList **FileNamesCreated);
		ExportFormatOpenFlight(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource);
		~ExportFormatOpenFlight();
		int ExportCameras(FILE *fOF, EffectList *CameraList);
		int ExportLights(long which);
		long ExportTerrain(const char *RawElev, FILE *Output, long HasTexture, long XTile, long YTile, double sw_x, double sw_y,
			long numLevels, class OF_TexturePalette *TP);
		long ExportTerrainFace(FILE *RawElev, FILE *Output, long HasTexture, long XTile, long YTile, long xsize, long ysize,
			double sw_x, double sw_y, long levelCode, class OF_TexturePalette *TP);
		long ExportTerrainMesh(FILE *RawElev, FILE *Output, long HasTexture);
		float MinElev(FILE *RawElevs, long Cols, long Rows);
		virtual int PackageExport(NameList **FileNamesCreated);
		int Process3DObjects(NameList **FileNamesCreated, const char *OutputFilePath, FILE *fOF_Parent,
			long XTile, long YTile, double sw_x, double sw_y);
		int ProcessFoliageList(NameList **FileNamesCreated, const char *OutputFilePath, FILE *fOF_Parent,
			long XTile, long YTile, double sw_x, double sw_y);
		void ProcessMatTexture(long MatCt, FILE *fObj, Object3DEffect *CurObj);
		virtual int ProcessSky(NameList **FileNamesCreated, const char *OutputFilePath);
		virtual int ProhibitAlphaSave(void)	{return (1);};
		virtual int CautionAlphaSave(void)	{return (1);};	// OF has problems with alpha channel textures
		int ProcessVectors(const char *OutputFilePath);
		char *TimeStamp(void);

	}; // class ExportFormatOpenFlight

class ExportFormatGIS : public ExportFormat
	{
	public:
		class PathAndFile fPaF, pPaF, vPaF;	// foliage, point & vector
		FILE *fGrid;
		//char RelativeFoliageFileName[255], RelativeSpeciesFileName[255], RelativeSkyFileName[255], RelativeStructFileName[255];
		//Zipper *ZipBuilder;

		ExportFormatGIS(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource);
		~ExportFormatGIS();
		void Cleanup(const char *OutputFilePath, NameList **FileNamesCreated);
		virtual int SanityCheck(void);
		virtual int PackageExport(NameList **FileNamesCreated);
		virtual int ProcessFoliageList(NameList **FileNamesCreated, const char *OutputFilePath);
		//virtual int ProcessSky(NameList **FileNamesCreated, const char *OutputFilePath);
		virtual int ProhibitAlphaSave(void)	{return (1);};
		//virtual int ProcessLights(FILE *FScene);
		//int ExportAmbient(Atmosphere *CurHaze, FILE *FScene);
		//virtual int Process3DObjects(NameList **FileNamesCreated, FILE *FScene, const char *OutputFilePath, int NVKeyID);
		//virtual int FindCopyable3DObjFile(Object3DEffect *Object3D);
		//int ExportHaze(Atmosphere *CurHaze, FILE *FScene);
		//virtual int ProcessCameras(FILE *FScene);

	}; // class ExportFormatGIS

class ExportFormatWW : public ExportFormat
	{
	public:
		FILE *fGrid;
		class PathAndFile fPaF, pPaF, vPaF;	// foliage, point & vector
		unsigned long startLevel;
		//char RelativeFoliageFileName[255], RelativeSpeciesFileName[255], RelativeSkyFileName[255], RelativeStructFileName[255];
		//Zipper *ZipBuilder;

		ExportFormatWW(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource);
		~ExportFormatWW();
		void Cleanup(const char *OutputFilePath, NameList **FileNamesCreated);
		int ExportTerrain(NameList **fileNamesCreated);
		int ExtractTileElevs(const char *rawTerrainFile, long fullEdgeRes, long edgeTiles, long xTile, long yTile);
		virtual int SanityCheck(void);
		virtual int PackageExport(NameList **FileNamesCreated);
		virtual int ProcessFoliageList(NameList **FileNamesCreated, const char *OutputFilePath);
		//virtual int ProcessSky(NameList **FileNamesCreated, const char *OutputFilePath);
		virtual int ProhibitAlphaSave(void)	{return (1);};
		//virtual int ProcessLights(FILE *FScene);
		//int ExportAmbient(Atmosphere *CurHaze, FILE *FScene);
		virtual int Process3DObjects(NameList **FileNamesCreated, const char *OutputFilePath);
		//virtual int FindCopyable3DObjFile(Object3DEffect *Object3D);
		//int ExportHaze(Atmosphere *CurHaze, FILE *FScene);
		//virtual int ProcessCameras(FILE *FScene);

	}; // class ExportFormatWW

class ExportFormatWCSVNS : public ExportFormat
	{
	public:
		NewProject* NewExportProj;
		Database*	ExportDB;
		EffectsLib*	ExportEffects;
		ImageLib*	ExportImages;
		Project*	ExportProj;
		CoordSys*	ExportCoordSys;
		PathAndFile xPaF;
		//class PathAndFile fPaF, pPaF, vPaF;	// foliage, point & vector
		//FILE *fGrid;
		//char RelativeFoliageFileName[255], RelativeSpeciesFileName[255], RelativeSkyFileName[255], RelativeStructFileName[255];
		//Zipper *ZipBuilder;

		ExportFormatWCSVNS(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource);
		~ExportFormatWCSVNS();
		void Cleanup(const char *OutputFilePath, NameList **FileNamesCreated);
		void DoInterCommon(void);
		//virtual int SanityCheck(void);
		virtual int PackageExport(NameList **FileNamesCreated);
		virtual int ProcessFoliageList(NameList **FileNamesCreated, const char *OutputFilePath);
		virtual int ProcessSky(NameList **FileNamesCreated, const char *OutputFilePath);
		//virtual int ProcessLights(FILE *FScene);
		//int ExportAmbient(Atmosphere *CurHaze, FILE *FScene);
		virtual int Process3DObjects(NameList **FileNamesCreated, const char *OutputFilePath);
		//virtual int FindCopyable3DObjFile(Object3DEffect *Object3D);
		//int ExportHaze(Atmosphere *CurHaze, FILE *FScene);
		//virtual int ProcessCameras(FILE *FScene);
		long ProcessImages(const char *OutputFilePath, NameList **FileNamesCreated);
		void ProcessProjPrefsInfo(Project *ProjectDest, Project *ProjectSource);
		int ProcessVectors(void);
		Joe *JoeDBCopy(Joe *DupeMe, Database *DBHost, Project *ProjHost);

	}; // class ExportFormatWCSVNS

class ExportFormatCustom : public ExportFormat
	{
	public:
		ExportFormatCustom(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource);
		~ExportFormatCustom();

	}; // class ExportFormatCustom

class ExportFormatCOLLADA : public ExportFormat
	{
	private:
		FILE *fDAE;
		long sectionInactive;
		unsigned char curIndent;
		char indentStr[64], logMsg[512], tempStr[256], xsID[256];

	public:
		ExportFormatCOLLADA(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource);
		virtual ~ExportFormatCOLLADA();
		int C_prologue_asset(unsigned long upAxis);
		void Cleanup(NameList **fileNamesCreated);
		char *CreateValidID(const char *string);
		int Export3DObjs(NameList **fileNamesCreated);
		int ProcessLibraryLights(void);
		int ExportTerrain(NameList **fileNamesCreated);
		int ExportVectors(void);
		void IndentLess(void);
		void IndentMore(void);
		int KML_Point(double &x, double &y, double &z);
		virtual int PackageExport(NameList **fileNamesCreated);
		int Process3DObjectImages(void);
		int ProcessFoliageLabelImages(NameList **fileNamesCreated);
		int ProcessLibraryCameras(void);
		int ProcessLibraryGeometries(NameList **fileNamesCreated);
		int ProcessLibraryImages(NameList **fileNamesCreated);
		int ProcessLibraryMaterials(NameList **fileNamesCreated);
		int ProcessLibraryEffects(NameList **fileNamesCreated);
		int ProcessLibraryVisualScenes(NameList **fileNamesCreated);
		void ProcessPolys(Object3DEffect *curObj);
		int ProcessTerrainImages(NameList **fileNamesCreated);
		virtual int SanityCheck(void);

	}; // class ExportFormatCOLLADA

#ifdef WCS_BUILD_FBX
class ExportFormatFBX : public ExportFormat
	{
	public:
		ExportFormatFBX(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource);
		virtual ~ExportFormatFBX();
		virtual int SanityCheck(void);
		virtual int PackageExport(NameList **FileNamesCreated);
		virtual int Process3DObjects(NameList **FileNamesCreated, const char *OutputFilePath);
		virtual int ProcessSky(NameList **FileNamesCreated, const char *OutputFilePath);

		bool createOriginNode(void);
		bool CreateTerrainMesh(NameList **FileNamesCreated, KFbxNode* &lTerrain);
		bool DoSceneInfo(void);
		bool ExportCameras(EffectList *CameraList, KFbxNode* &lCameras);
		bool ExportFoliage(NameList **FileNamesCreated, const char *OutputFilePath, KFbxNode* &lFoliage);
		int ExportHaze(Atmosphere *CurHaze, long HazeNum);
		int ExportLights(EffectList *LightList, KFbxNode* &lLights);
		int ExportObjects(Object3DInstance *ObjectList, KFbxNode* &l3DObjects);
		bool SaveScene(void);
		bool SetGlobalSettings(void);

	private:
		double centerCoordZ;
		KFbxSdkManager* pSdkManager;
		KFbxScene* pScene;
		KFbxExporter* fbxExporter;
		//int matIndex;	// 0 reserved for "White", increment counter for all other additionals
		int texIndex;	// Next available texture number - increment after each new texture added
		unsigned long phongNum;
		char takeName[8];

	}; // class ExportFormatFBX
#endif // WCS_BUILD_FBX

// Google Earth
class ExportFormatKML : public ExportFormat
	{
	private:
		FILE *fKML;
		Zipper *zippy;
		char indentStr[64], tempStr[256];
		unsigned char curIndent;

	public:
		ExportFormatKML(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource);
		~ExportFormatKML();
		void BindMaterials(long matCt, FILE *fDAE, Object3DEffect *curObj);
		int C_prologue_asset(FILE *fDAE, unsigned long upAxis);
		void Cleanup(NameList **fileNamesCreated);
		int CreateTree(const char *speciesName);
		int DefineStyles(NameList **fileNamesCreated);
		int Describe(SXQueryActionList *actionList, GeneralEffect *MyEffect, Joe *MyVector);
		int Export3DObjs(NameList **fileNamesCreated);
		int ExportCameras(void);
		int ExportFoliage(NameList **fileNamesCreated);
		int ExportGroundTextures(NameList **fileNamesCreated);
		int ExportLabels(NameList **fileNamesCreated);
		int ExportScreenOverlays(void);
		int ExportTerrain(NameList **fileNamesCreated);
		int ExportVectors(void);
		void IndentLess(void);
		void IndentMore(void);
		int KML_Point(double &x, double &y, double &z);
		virtual int PackageExport(NameList **fileNamesCreated);
		void ProcessGeometries(FILE *fDAE, Object3DEffect *curObj);
		void ProcessMatTexture1(long matCt, FILE *fDAE, Object3DEffect *curObj);
		void ProcessMatTexture2(long matCt, FILE *fDAE, Object3DEffect *curObj);
		void ProcessMatTexture3(long matCt, FILE *fDAE, Object3DEffect *curObj);
		void ProcessPolys(FILE *fDAE, Object3DEffect *curObj);
		virtual int SanityCheck(void);
		int StylesAndMap(const char *baseName, const char *ext, float scale);
		//virtual int ProcessFoliageList(NameList **FileNamesCreated, const char *OutputFilePath)	{return (1);};
		//virtual int ProcessSky(NameList **FileNamesCreated, const char *OutputFilePath)	{return (1);};
		//virtual int Process3DObjects(NameList **FileNamesCreated, const char *OutputFilePath)	{return (1);};
		//virtual FormatSpecificFile OpenSceneFile(const char *PathName, const char *FileName)	{return (0);};
		//virtual FormatSpecificFile OpenObjectFile(const char *PathName, const char *FileName)	{return (0);};
		//virtual int CloseFile(FormatSpecificFile fFile)	{return (1);};
		//virtual int SaveAllObjects(int OneFile, FormatSpecificFile CurFile)	{return (1);};
		//virtual int SaveOneObject(FormatSpecificFile fFile, Object3DEffect *Object3D, const char *SaveToPath, 
		//	int SaveMaterials, float *BoundMin, float *BoundMax)	{return (1);};
		//virtual int CautionAlphaSave(void)	{return (0);};
		//virtual int ProhibitAlphaSave(void)	{return (0);};
		//virtual int FindCopyable3DObjFile(Object3DEffect *Object3D)	{return (0);};
		//int CheckAndShortenFileName(char *OutputName, const char *FilePath, const char *FileName, long MaxLen);
		//int CopyExistingFile(const char *OriginalFullPath, const char *PathName, const char *FileName);

	}; // class ExportFormatKML

#endif // WCS_EXPORTFORMAT_H
