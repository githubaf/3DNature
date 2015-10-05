// ExportFormat3DS.cpp
// Code module for 3DS export code
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

ExportFormat3DS::ExportFormat3DS(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource)
: ExportFormat(MasterSource, ProjectSource, EffectsSource, DBSource, ImageSource)
{

Database3ds = NULL;
File3ds = NULL;

} // ExportFormat3DS::ExportFormat3DS

/*===========================================================================*/

ExportFormat3DS::~ExportFormat3DS()
{

} // ExportFormat3DS::~ExportFormat3DS

/*===========================================================================*/

int ExportFormat3DS::PackageExport(NameList **FileNamesCreated)
{
long ObjectNum;
EffectList *CurCamera, *CurLight, *CurHaze;
FormatSpecificFile CurFile;
int Success = 1;

// open project file
if (CurFile = OpenSceneFile(Master->OutPath.GetPath(), Master->OutPath.GetName()))
	{
	// export haze
	if (Master->ExportHaze && (CurHaze = Master->Haze))
		{
		ObjectNum = 0;
		while (CurHaze && Success)
			{
			if (CurHaze->Me)
				Success = ExportHaze((Atmosphere *)CurHaze->Me, ObjectNum ++);
			CurHaze = CurHaze->Next;
			} // while
		} // if
	// export lights
	if (Success)
		{
		if (Master->ExportLights && (CurLight = Master->Lights))
			{
			ObjectNum = 0;
			while (CurLight && Success)
				{
				if (CurLight->Me)
					Success = ExportLight((Light *)CurLight->Me, ObjectNum ++);
				CurLight = CurLight->Next;
				} // while
			} // if
		} // if
	// export cameras
	if (Success)
		{
		if (Master->ExportCameras && (CurCamera = Master->Cameras))
			{
			ObjectNum = 0;
			while (CurCamera && Success)
				{
				if (CurCamera->Me)
					Success = ExportCamera((Camera *)CurCamera->Me, ObjectNum ++);
				CurCamera = CurCamera->Next;
				} // while
			} // if
		} // if
	// export objects (terrain, sky, walls, 3d objects, foliage, vectors)
	if (Success)
		Success = SaveAllObjects(TRUE, CurFile);	// TRUE=OneFile
	if (! CloseFile(CurFile))
		Success = 0;
	} // if
else
	Success = 0;

return (Success);

} // ExportFormat3DS::PackageExport

/*===========================================================================*/

FormatSpecificFile ExportFormat3DS::OpenSceneFile(const char *PathName, const char *FileName)
{
char FullFileName[512];

// copy file name
strcpy(FullFileName, FileName);
// append extension
strcat(FullFileName, ".prj");

return (InitFile3DS(PathName, FullFileName));

} // ExportFormat3DS::OpenSceneFile

/*===========================================================================*/

FormatSpecificFile ExportFormat3DS::OpenObjectFile(const char *PathName, const char *FileName)
{
char FullFileName[512];

// copy file name
strcpy(FullFileName, FileName);
// append extension
strcat(FullFileName, ".3ds");

return (InitFile3DS(PathName, FullFileName));

} // ExportFormat3DS::OpenObjectFile

/*===========================================================================*/

FormatSpecificFile ExportFormat3DS::InitFile3DS(const char *PathName, const char *FileName)
{
char FullFileName[512];
meshset3ds *MeshSet3ds = NULL;

strmfp(FullFileName, PathName, FileName);
strcpy(FullFileName, GlobalApp->MainProj->MungPath(FullFileName));

InitDatabase3ds(&Database3ds);
if (Database3ds && ! ftkerr3ds)
	{
	CreateNewDatabase3ds(Database3ds, MeshFile);
	if (Database3ds->topchunk)
		{
		// object settings
		InitMeshSet3ds(&MeshSet3ds);
		if (MeshSet3ds && ! ftkerr3ds)
			{
			MeshSet3ds->masterscale = (float3ds)(ConvertFromMeters(1.0, WCS_USEFUL_UNIT_INCH));
			MeshSet3ds->ambientlight.r = (float3ds)(0);
			MeshSet3ds->ambientlight.g = (float3ds)(0);
			MeshSet3ds->ambientlight.b = (float3ds)(0);
			PutMeshSet3ds(Database3ds, MeshSet3ds);
			ReleaseMeshSet3ds(&MeshSet3ds);
			} // if
		else
			{
			if (MeshSet3ds)
				ReleaseMeshSet3ds(&MeshSet3ds);		
			UserMessageOK("3DS Export", "Unable to initialize 3DS Mesh Settings!\nOperation terminated.");
			goto EndIt;
			} // if
		if (ftkerr3ds)
			{
			UserMessageOK("3DS Export", "Error creating 3DS Mesh Settings chunk!\nOperation terminated.");
			goto EndIt;
			}
		} // if topchunk
	else
		{
		UserMessageOK("3DS Export", "Unable to initialize 3DS Database Top Chunk!\nOperation terminated.");
		goto EndIt;
		} // if
	} // if
else
	{
	UserMessageOK("3DS Export", "Unable to initialize 3DS Database!\nOperation terminated.");
EndIt:
	if (Database3ds)
		ReleaseDatabase3ds(&Database3ds);
	} // else

if (Database3ds)
	{
	if (File3ds = OpenFile3ds((const char3ds *)FullFileName, "w"))
		{
		return (File3ds);
		} // if
	else
		{
		UserMessageOK("3DS Export", "Error opening 3DS Database file for output!\nOperation terminated.");
		ReleaseDatabase3ds(&Database3ds);
		} // else
	} // if

return (NULL);

} // ExportFormat3DS::InitFile3DS

/*===========================================================================*/

int ExportFormat3DS::CloseFile(FormatSpecificFile fFile)
{
int Success = 1;

if (Database3ds)
	{
	if (fFile)
		{
		WriteDatabase3ds((file3ds *)fFile, Database3ds);
		if (ftkerr3ds)
			{
			UserMessageOK("3DS Export", "Error saving 3DS Database to file!\nOperation terminated.");
			Success = 0;
			} // if
		CloseAllFiles3ds();

//		strcpy(Name, "WCSTest.txt");
//		strmfp(filename, Path, Name);
//		if (DumpFile = fopen (filename, "w"))
//			{
//			DumpDatabase3ds(DumpFile, Database3ds);
//			fclose(DumpFile);
//			} // if
		} // if
	} // if

return (Success);

} // ExportFormat3DS::CloseFile

/*===========================================================================*/

int ExportFormat3DS::SaveAllObjects(int OneFile, FormatSpecificFile CurFile)
{
float BoundMin[3], BoundMax[3];
long TotalInstances, ObjsDone, ObjCt, DoIt, MatsDone, MatDoIt, MatCt, BaseMatCt, CheckMatCt, MegaMatCt = 0, TotalMats = 0;
Object3DInstance *CurInstance, *CurKFInstance;
Object3DEffect *CurObj, **ObjectsDoneList;
MaterialEffect **MaterialsDoneList;
int Success = 1;
char TempFullPath[512];
PathAndFile SceneAux;

SceneAux.SetPath((char *)Master->OutPath.GetPath());

if (CurInstance = Master->ObjectInstanceList)
	{
	while (CurInstance)
		{
		if (CurObj = CurInstance->MyObj)
			TotalMats += CurObj->NumMaterials;
		CurInstance = CurInstance->Next;
		} // while
	} // if

if (TotalMats && (CurInstance = Master->ObjectInstanceList))
	{
	if ((TotalInstances = CurInstance->CountBoundedInstances(Master->FetchRBounds())) > 0)
		{
		if (ObjectsDoneList = (Object3DEffect **)AppMem_Alloc(min(TotalInstances, Master->Unique3DObjectInstances) * sizeof (Object3DEffect *), APPMEM_CLEAR))
			{
			if (MaterialsDoneList = (MaterialEffect **)AppMem_Alloc(TotalMats * sizeof (MaterialEffect *), APPMEM_CLEAR))
				{
				ObjsDone = MatsDone = 0;
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
								// figure out a file name
								sprintf(TempFullPath, "OB%06d", ObjsDone ++);
								SceneAux.SetName(CurObj->ShortName[0] ? CurObj->ShortName: TempFullPath);
								// open an object file
								if (CurFile || (CurFile = OpenObjectFile(SceneAux.GetPath(), SceneAux.GetName())))
									{
									// if one file, may need to save the materials for the object
									BaseMatCt = MegaMatCt;
									if (OneFile)
										{
										for (MatCt = 0; MatCt < CurObj->NumMaterials; MatCt ++)
											{
											MatDoIt = 1;
											for (CheckMatCt = 0; CheckMatCt < MatsDone; CheckMatCt ++)
												{
												if (! CurObj->NameTable[MatCt].Mat)
													CurObj->NameTable[MatCt].Mat = (MaterialEffect *)GlobalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, CurObj->NameTable[MatCt].Name);
												if (CurObj->NameTable[MatCt].Mat == MaterialsDoneList[CheckMatCt])
													MatDoIt = 0;
												} // for
											if (MatDoIt)
												{
												MaterialsDoneList[MatsDone ++] = CurObj->NameTable[MatCt].Mat;
												if (! SaveOneMaterial(CurObj, &CurObj->NameTable[MatCt], MegaMatCt ++, SceneAux.GetPath()))
													{
													Success = 0;
													break;
													} // if
												} // if
											} // for
										} // if
									else
										MegaMatCt += CurObj->NumMaterials;
									// save object
									if (Success && ! SaveOneObject(CurFile, CurObj, SceneAux.GetPath(), ! OneFile, BaseMatCt, BoundMin, BoundMax))	// !OneFile=SaveMaterials
										Success = 0;
									// close file
									if (! OneFile)
										{
										CloseFile(CurFile);
										CurFile = NULL;
										} // if
									else
										{
										CurKFInstance = Master->ObjectInstanceList;
										ObjCt = 0;
										while (CurKFInstance && Success)
											{
											if (CurObj == CurKFInstance->MyObj)
												{
												if (CurKFInstance->IsBounded(Master->FetchRBounds()))
													{
													// save object position and rotation
													Success = SaveObjectMotion(CurObj, CurKFInstance, &Master->ExportRefData, BoundMin, BoundMax, ObjCt ++);
													} // if
												} // if
											CurKFInstance = CurKFInstance->Next;
											} // while
										} // else
									} // if
								else
									Success = 0;
								} // if
							} // if
						} // if object
					CurInstance = CurInstance->Next;
					} // while
				AppMem_Free(MaterialsDoneList, TotalMats * sizeof (MaterialEffect *));
				} // if
			else
				Success = 0;
			AppMem_Free(ObjectsDoneList, min(TotalInstances, Master->Unique3DObjectInstances) * sizeof (Object3DEffect *));
			} // if
		else
			Success = 0;
		} // if
	} // if

return (Success);

} // ExportFormat3DS::SaveAllObjects

/*===========================================================================*/

int ExportFormat3DS::SaveOneMaterial(Object3DEffect *Object3D, 
	ObjectMaterialEntry *NameTable, long MatCt, const char *SaveToPath)
{
material3ds *Material3ds = NULL;
int Success = 1;

InitMaterial3ds(&Material3ds);
if (Material3ds && ! ftkerr3ds)
	{
	SetMaterialProperties(Material3ds, Object3D, NameTable, MatCt, SaveToPath);
	PutMaterial3ds(Database3ds, Material3ds);
	ReleaseMaterial3ds(&Material3ds);
	} // if
else
	{
	if (Material3ds)
		ReleaseMaterial3ds(&Material3ds);		
	UserMessageOK("3DS Export", "Unable to initialize 3DS Material Settings!\nOperation terminated.");
	Success = 0;
	} // if
if (ftkerr3ds)
	{
	UserMessageOK("3DS Export", "Error creating 3DS Material Settings chunk!\nOperation terminated.");
	Success = 0;
	}

return (Success);

} // ExportFormat3DS::SaveOneMaterial

/*===========================================================================*/

int ExportFormat3DS::SaveOneObject(FormatSpecificFile fFile, Object3DEffect *Object3D, const char *SaveToPath, 
	int SaveMaterials, long BaseMatCt, float *BoundMin, float *BoundMax)
{
long NumVerts, NumPolys, MatCt, PolyCt, PolyCt3Sided, MatPolyCt, NumMatPolys, MaxPolyCt;
mesh3ds *Mesh3ds = NULL;
material3ds *Material3ds = NULL;
int Success = 1;

if ((Object3D->Vertices && Object3D->Polygons && Object3D->NameTable) || Object3D->OpenInputFile(NULL, FALSE, FALSE, FALSE))
	{
	// sanity check on object first
	NumVerts = Object3D->NumVertices;
	NumPolys = Object3D->Count3SidedPolygons();

	if (NumVerts > 0 && NumPolys > 0)
		{
		if (NumVerts > 65536)
			{
			return (UserMessageYN(Object3D->GetName(), "Too many vertices to save as 3DS. Limit is 65,536 per object. Continue saving other objects?"));
			} // if too many vertices
		if (NumPolys > 65536)
			{
			return (UserMessageYN(Object3D->GetName(), "Too many polygons to save as 3DS. Limit is 65,536 per object. Continue saving other objects?"));
			} // if too many vertices

		if (SaveMaterials)
			{
			InitMaterial3ds(&Material3ds);
			if (Material3ds && ! ftkerr3ds)
				{
				for (MatCt = 0; MatCt < Object3D->NumMaterials; MatCt ++)
					{
					SetMaterialProperties(Material3ds, Object3D, &Object3D->NameTable[MatCt], BaseMatCt + MatCt, SaveToPath);
					PutMaterial3ds(Database3ds, Material3ds);
					} // for
				ReleaseMaterial3ds(&Material3ds);
				} // if
			else
				{
				if (Material3ds)
					ReleaseMaterial3ds(&Material3ds);		
				UserMessageOK("3DS Export", "Unable to initialize 3DS Material Settings!\nOperation terminated.");
				Success = 0;
				goto EndIt;
				} // if
			if (ftkerr3ds)
				{
				UserMessageOK("3DS Export", "Error creating 3DS Material Settings chunk!\nOperation terminated.");
				Success = 0;
				goto EndIt;
				}
			} // if

		InitMeshObj3ds(&Mesh3ds, (ushort3ds)NumVerts, (ushort3ds)NumPolys, Object3D->VertexUVWAvailable ? InitTextArray3ds: InitNoExtras3ds);
		if (Mesh3ds && ! ftkerr3ds)
			{
			if (Object3D->ShortName[0])
				strncpy(Mesh3ds->name, Object3D->ShortName, 10);
			else
				strncpy(Mesh3ds->name, Object3D->GetName(), 10);
			Mesh3ds->name[10] = 0;
			TrimTrailingSpaces(Mesh3ds->name);
			Mesh3ds->nvertices = (ushort3ds)NumVerts;
			Mesh3ds->nvflags = 0;
			Mesh3ds->ntextverts = Mesh3ds->textarray ? (ushort3ds)NumVerts: 0;
			Mesh3ds->usemapinfo = False3ds;
			Mesh3ds->nfaces = (ushort3ds)NumPolys;
			Mesh3ds->locmatrix[0] = (float3ds)1.0;
			Mesh3ds->locmatrix[1] = (float3ds)0.0;
			Mesh3ds->locmatrix[2] = (float3ds)0.0;
			Mesh3ds->locmatrix[3] = (float3ds)0.0;
			Mesh3ds->locmatrix[4] = (float3ds)1.0;
			Mesh3ds->locmatrix[5] = (float3ds)0.0;
			Mesh3ds->locmatrix[6] = (float3ds)0.0;
			Mesh3ds->locmatrix[7] = (float3ds)0.0;
			Mesh3ds->locmatrix[8] = (float3ds)1.0;
			Mesh3ds->locmatrix[9] = (float3ds)0.0;
			Mesh3ds->locmatrix[10] = (float3ds)0.0;
			Mesh3ds->locmatrix[11] = (float3ds)0.0;

			// the nitty gritty of the object saver

			// the mesh
			if (CreateObjectMesh(Object3D, Mesh3ds->vertexarray, Mesh3ds->facearray, Mesh3ds->textarray))
				{
				// materials
				if (Mesh3ds->matarray = (objmat3ds *)AppMem_Alloc(Object3D->NumMaterials * sizeof (objmat3ds), APPMEM_CLEAR))
					{
					Mesh3ds->nmats = (ushort3ds)Object3D->NumMaterials;
					for (MatCt = 0; MatCt < Object3D->NumMaterials; MatCt ++)
						{
						// have to come up with the same name here as was used to save the material
						if (Object3D->NameTable[MatCt].Mat && Object3D->NameTable[MatCt].Mat->ShortName[0])
							{
							strncpy(Mesh3ds->matarray[MatCt].name, Object3D->NameTable[MatCt].Mat->ShortName, 16);
							Mesh3ds->matarray[MatCt].name[16] = 0;
							} // if
						else
							{
							sprintf(Mesh3ds->matarray[MatCt].name, "OM%06d", BaseMatCt + MatCt);
							} // else
						// assign materials to polygons
						if ((NumMatPolys = Object3D->CountMaterialPolygons(MatCt)) > 0)
							{
							if (Mesh3ds->matarray[MatCt].faceindex = (ushort3ds *)AppMem_Alloc(NumMatPolys * sizeof (ushort3ds), APPMEM_CLEAR))
								{
								Mesh3ds->matarray[MatCt].nfaces = (ushort3ds)NumMatPolys;
								for (PolyCt = MatPolyCt = PolyCt3Sided = 0; PolyCt < Object3D->NumPolys; PolyCt ++)
									{
									if (Object3D->Polygons[PolyCt].NumVerts >= 3)
										{
										if (Object3D->Polygons[PolyCt].Material == MatCt)
											{
											if (Object3D->Polygons[PolyCt].NumVerts == 3)
												{
												Mesh3ds->matarray[MatCt].faceindex[MatPolyCt] = (ushort3ds)PolyCt3Sided;
												PolyCt3Sided ++;
												MatPolyCt ++;
												} // if
											else
												{
												// a larger polygon, how many 3pt polygons did it make?
												MaxPolyCt = MatPolyCt + Object3D->Polygons[PolyCt].NumVerts - 2;
												for (; MatPolyCt < MaxPolyCt; MatPolyCt ++)
													{
													Mesh3ds->matarray[MatCt].faceindex[MatPolyCt] = (ushort3ds)PolyCt3Sided;
													PolyCt3Sided ++;
													} // for
												} // else
											} // if
										else
											PolyCt3Sided += Object3D->Polygons[PolyCt].NumVerts - 2;
										} // if
									} // for
								} // if
							} // if
						} // for
					} // if

				if (BoundMin && BoundMax)
					FindObjectMeshBounds(Mesh3ds->nvertices, Mesh3ds->vertexarray, (float3ds *)BoundMin, (float3ds *)BoundMax);
				PutMesh3ds(Database3ds, Mesh3ds);
				for (MatCt = 0; MatCt < Object3D->NumMaterials; MatCt ++)
					{
					if (Mesh3ds->matarray[MatCt].faceindex)
						AppMem_Free(Mesh3ds->matarray[MatCt].faceindex, sizeof (ushort3ds) * Mesh3ds->matarray[MatCt].nfaces);
					Mesh3ds->matarray[MatCt].faceindex = NULL;
					Mesh3ds->matarray[MatCt].nfaces = 0;
					} // for
				if (Mesh3ds->matarray)
					AppMem_Free(Mesh3ds->matarray, Object3D->NumMaterials * sizeof (objmat3ds));
				Mesh3ds->matarray = NULL;
				Mesh3ds->nmats = 0;
				if (ftkerr3ds)
					{
					UserMessageOK("3DS Export", "Error creating 3DS Mesh Object Keyframe chunk!\nOperation terminated.");
					Success = 0;
					} // if
				} // if data filled
			RelMeshObj3ds(&Mesh3ds);
			} // if
		else
			{
			if (Mesh3ds)
				RelMeshObj3ds(&Mesh3ds);		
			UserMessageOK("3DS Export", "Unable to initialize 3DS Mesh Object!\nOperation terminated.");
			Success = 0;
			} // else
		} // if a useful number of vertices and polygons
	else
		{
		// too few vertices or polygons
		UserMessageOK("3DS Export", "Error creating 3DS Object! Too few vertices or polygons in the model.\nOperation terminated.");
		Success = 0;
		} // else
	} // if object loaded
else
	{
	// object couldn't be loaded
	UserMessageOK("3DS Export", "Error creating 3DS Object! Unable to load the model geometry.\nOperation terminated.");
	Success = 0;
	} // else

EndIt:

return (Success);

} // ExportFormat3DS::SaveOneObject

/*===========================================================================*/

int ExportFormat3DS::CreateObjectMesh(Object3DEffect *Object3D, point3ds *VtxData, face3ds *PolyData, textvert3ds *TexData)
{
long VertCt, PolyCtIn, PolyCtOut, FlipNormals, LowVert, HighVert;
ObjectPerVertexMap *UVMap = NULL;

if (TexData)
	{
	UVMap = &Object3D->UVWTable[0];	// only the first map gets written
	} // if

for (VertCt = 0; VertCt < Object3D->NumVertices; VertCt ++)
	{
	// VtxData is an array of x,y,z triplets, careful of order
	VtxData[VertCt].x = (float3ds)Object3D->Vertices[VertCt].xyz[0];
	VtxData[VertCt].y = (float3ds)Object3D->Vertices[VertCt].xyz[2];
	VtxData[VertCt].z = (float3ds)Object3D->Vertices[VertCt].xyz[1];
	if (UVMap)
		{
		if (UVMap->CoordsValid[VertCt])
			{
			TexData[VertCt].u = (float3ds)UVMap->CoordsArray[0][VertCt];
			TexData[VertCt].v = (float3ds)UVMap->CoordsArray[1][VertCt];
			} // if
		else
			TexData[VertCt].u = TexData[VertCt].v = (float3ds)0.0;
		} // if
	} // for

for (PolyCtIn = PolyCtOut = 0; PolyCtIn < Object3D->NumPolys; PolyCtIn ++)
	{
	// only polygons with 3 or more vertices are eligible
	if (Object3D->Polygons[PolyCtIn].NumVerts >= 3)
		{
		if (! Object3D->NameTable[Object3D->Polygons[PolyCtIn].Material].Mat)
			Object3D->NameTable[Object3D->Polygons[PolyCtIn].Material].Mat = (MaterialEffect *)GlobalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, Object3D->NameTable[Object3D->Polygons[PolyCtIn].Material].Name);
		FlipNormals = Object3D->NameTable[Object3D->Polygons[PolyCtIn].Material].Mat ?
			Object3D->NameTable[Object3D->Polygons[PolyCtIn].Material].Mat->FlipNormal: 0;
		// PolyData is an array of vertex numbers. Only three vertices to a polygon. Vertex order is important
		if (Object3D->Polygons[PolyCtIn].NumVerts == 3)
			{
			if (! FlipNormals)
				{
				PolyData[PolyCtOut].v1 = (ushort3ds)Object3D->Polygons[PolyCtIn].VertRef[0];
				PolyData[PolyCtOut].v2 = (ushort3ds)Object3D->Polygons[PolyCtIn].VertRef[2];
				PolyData[PolyCtOut].v3 = (ushort3ds)Object3D->Polygons[PolyCtIn].VertRef[1];
				} // if normal order
			else
				{
				PolyData[PolyCtOut].v1 = (ushort3ds)Object3D->Polygons[PolyCtIn].VertRef[0];
				PolyData[PolyCtOut].v2 = (ushort3ds)Object3D->Polygons[PolyCtIn].VertRef[1];
				PolyData[PolyCtOut].v3 = (ushort3ds)Object3D->Polygons[PolyCtIn].VertRef[2];
				} // else
			PolyData[PolyCtOut].flag = 0x0007;
			PolyCtOut ++;
			} // if
		else
			{
			LowVert = 1;
			HighVert = 2;

			while (HighVert < Object3D->Polygons[PolyCtIn].NumVerts)
				{
				if (! FlipNormals)
					{
					PolyData[PolyCtOut].v1 = (ushort3ds)Object3D->Polygons[PolyCtIn].VertRef[0];
					PolyData[PolyCtOut].v2 = (ushort3ds)Object3D->Polygons[PolyCtIn].VertRef[HighVert];
					PolyData[PolyCtOut].v3 = (ushort3ds)Object3D->Polygons[PolyCtIn].VertRef[LowVert];
					} // if normal order
				else
					{
					PolyData[PolyCtOut].v1 = (ushort3ds)Object3D->Polygons[PolyCtIn].VertRef[0];
					PolyData[PolyCtOut].v2 = (ushort3ds)Object3D->Polygons[PolyCtIn].VertRef[LowVert];
					PolyData[PolyCtOut].v3 = (ushort3ds)Object3D->Polygons[PolyCtIn].VertRef[HighVert];
					} // else
				PolyData[PolyCtOut].flag = 0x0007;
				LowVert = HighVert;
				HighVert ++;
				PolyCtOut ++;
				} // while
			} // else
		} // if
	} // for

return (1);

} // ExportFormat3DS::CreateObjectMesh

/*===========================================================================*/

void ExportFormat3DS::FindObjectMeshBounds(ushort3ds nvertices, point3ds *vertexarray, float *boundmin, float *boundmax)
{
ushort3ds Point;

boundmin[0] = boundmin[1] = (float3ds)(10000000.0);
boundmin[2] = (float3ds)(10000000.0);
boundmax[0] = boundmax[1] = (float3ds)(-10000000.0);
boundmax[2] = (float3ds)(-10000000.0);

for (Point = 0; Point < nvertices; Point ++)
	{
	if (vertexarray[Point].x < boundmin[0])
		boundmin[0] = vertexarray[Point].x;
	if (vertexarray[Point].y < boundmin[1])
		boundmin[1] = vertexarray[Point].y;
	if (vertexarray[Point].z < boundmin[2])
		boundmin[2] = vertexarray[Point].z;

	if (vertexarray[Point].x > boundmax[0])
		boundmax[0] = vertexarray[Point].x;
	if (vertexarray[Point].y > boundmax[1])
		boundmax[1] = vertexarray[Point].y;
	if (vertexarray[Point].z > boundmax[2])
		boundmax[2] = vertexarray[Point].z;
	} // for

} // ExportFormat3DS::FindObjectMeshBounds

/*===========================================================================*/

int ExportFormat3DS::SaveObjectMotion(Object3DEffect *Object3D, Object3DInstance *CurInstance, 
	ExportReferenceData *RefData, float *BoundsMin, float *BoundsMax, long InstanceNum)
{
Point4d AxisAngle;
kfmesh3ds *KFMesh3ds = NULL;
int Success = 1;
VertexBase Vert1, Vert2;

InitObjectMotion3ds(&KFMesh3ds, 1, 1, 1, 0, 0);
if (KFMesh3ds && ! ftkerr3ds)
	{
	KFMesh3ds->boundmin.x = BoundsMin[0];
	KFMesh3ds->boundmin.y = BoundsMin[1];
	KFMesh3ds->boundmin.z = BoundsMin[2];
	KFMesh3ds->boundmax.x = BoundsMax[0];
	KFMesh3ds->boundmax.y = BoundsMax[1];
	KFMesh3ds->boundmax.z = BoundsMax[2];
	strncpy(KFMesh3ds->name, Object3D->ShortName[0] ? Object3D->ShortName: Object3D->Name, 10);
	KFMesh3ds->name[10] = 0;
	TrimTrailingSpaces(KFMesh3ds->name);
	sprintf(KFMesh3ds->instance, "%08d", InstanceNum);
	KFMesh3ds->flags1 = 0;
	KFMesh3ds->flags2 = KfNodeHasPath3ds;
	KFMesh3ds->npkeys = 1;
	KFMesh3ds->nrkeys = 1;
	KFMesh3ds->nskeys = 1;
	KFMesh3ds->npflag = TrackSingle3ds;
	KFMesh3ds->pkeys[0].time = 0;
	KFMesh3ds->pkeys[0].rflags = 0;
	KFMesh3ds->rkeys[0].time = 0;
	KFMesh3ds->rkeys[0].rflags = 0;
	KFMesh3ds->skeys[0].time = 0;
	KFMesh3ds->skeys[0].rflags = 0;
	// grh modified 
	//KFMesh3ds->pos[0].x = (float3ds)(-(CurInstance->Geographic[0] - RefData->RefLon) * RefData->LonScale);
	//KFMesh3ds->pos[0].y = (float3ds)((CurInstance->Geographic[1] - RefData->RefLat) * RefData->LatScale);
	//KFMesh3ds->pos[0].z = (float3ds)((CurInstance->Geographic[2] - RefData->RefElev) * RefData->ElevScale);
	KFMesh3ds->pos[0].x = (float3ds)((CurInstance->ExportXYZ[0]) * RefData->ExportLonScale);
	KFMesh3ds->pos[0].y = (float3ds)((CurInstance->ExportXYZ[1]) * RefData->ExportLatScale);
	KFMesh3ds->pos[0].z = (float3ds)((CurInstance->ExportXYZ[2]) * RefData->ElevScale);
	// derive the rotation axis and find rotation angle
	QuaternionToAxisAngle(AxisAngle, CurInstance->Quaternion);
	KFMesh3ds->rot[0].angle = (float3ds)(AxisAngle[3] * PiOver180);
	KFMesh3ds->rot[0].x = (float3ds)(AxisAngle[0]);
	KFMesh3ds->rot[0].y = (float3ds)(-AxisAngle[2]);
	KFMesh3ds->rot[0].z = (float3ds)(-AxisAngle[1]);
	if (KFMesh3ds->rot[0].angle < 0.0)
		{
		KFMesh3ds->rot[0].angle = -KFMesh3ds->rot[0].angle;
		KFMesh3ds->rot[0].x = -KFMesh3ds->rot[0].x;
		KFMesh3ds->rot[0].y = -KFMesh3ds->rot[0].y;
		KFMesh3ds->rot[0].z = -KFMesh3ds->rot[0].z;
		} // if
	KFMesh3ds->scale[0].x = (float3ds)(CurInstance->Scale[0]);
	KFMesh3ds->scale[0].y = (float3ds)(CurInstance->Scale[2]);
	KFMesh3ds->scale[0].z = (float3ds)(CurInstance->Scale[1]);
	PutObjectMotion3ds(Database3ds, KFMesh3ds);
	ReleaseObjectMotion3ds(&KFMesh3ds);
	} // if
else
	{
	if (KFMesh3ds)
		ReleaseObjectMotion3ds(&KFMesh3ds);		
	UserMessageOK("3DS Export", "Unable to initialize 3DS Mesh Object Keyframe!");
	Success = 0;
	} // else
if (ftkerr3ds)
	{
	UserMessageOK("3DS Export", "Error creating 3DS Mesh Object Keyframe chunk!");
	Success = 0;
	} // if

return (Success);

} // ExportFormat3DS::SaveObjectMotion

/*===========================================================================*/

void ExportFormat3DS::SetMaterialProperties(material3ds *Material3ds, Object3DEffect *Object3D, 
	ObjectMaterialEntry *NameTable, long MatCt, const char *SaveToPath)
{
MaterialEffect *Mat;
RootTexture *RootTex;
Texture *UVTex;
Raster *Rast;
bitmap3ds *tmap;
int DiffuseAlphaAvailable = 0;
char ImageShortName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN], OriginalFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

if (! NameTable->Mat)
	NameTable->Mat = (MaterialEffect *)GlobalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, NameTable->Name);
if (Mat = NameTable->Mat)
	{
	if (Mat->ShortName[0])
		{
		strncpy(Material3ds->name, Mat->ShortName, 16);
		Material3ds->name[16] = 0;
		} // if
	else
		{
		sprintf(Material3ds->name, "OM%06d", MatCt);
		strcpy(Mat->ShortName, Material3ds->name);
		} // else
	Material3ds->ambient.r = (float3ds)(0.0);
	Material3ds->ambient.g = (float3ds)(0.0);
	Material3ds->ambient.b = (float3ds)(0.0);
	Material3ds->diffuse.r = (float3ds)(Mat->DiffuseColor.CurValue[0]);
	Material3ds->diffuse.g = (float3ds)(Mat->DiffuseColor.CurValue[1]);
	Material3ds->diffuse.b = (float3ds)(Mat->DiffuseColor.CurValue[2]);
	Material3ds->specular.r = (float3ds)(Mat->SpecularColor.CurValue[0]);
	Material3ds->specular.g = (float3ds)(Mat->SpecularColor.CurValue[1]);
	Material3ds->specular.b = (float3ds)(Mat->SpecularColor.CurValue[2]);
	Material3ds->shininess = (float3ds)(Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].CurValue / 200.0);
	if (Material3ds->shininess > 1.0)
		Material3ds->shininess = (float3ds)1.0;
	Material3ds->shinstrength = (float3ds)(Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY].CurValue);
	if (Material3ds->shinstrength > 1.0)
		Material3ds->shinstrength = (float3ds)1.0;
	Material3ds->transparency = (float3ds)Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue;
	Material3ds->selfillum = Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].CurValue > 0.0 ? 1: 0;
	Material3ds->selfillumpct = (float3ds)Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].CurValue;
	Material3ds->shading = (Mat->Shading == WCS_EFFECT_MATERIAL_SHADING_FLAT ? (shadetype3ds)1: (shadetype3ds)3);
	Material3ds->twosided = (byte3ds)Mat->DoubleSided;

	if (Object3D->VertexUVWAvailable)
		{
		if (RootTex = Mat->GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
			{
			if (UVTex = RootTex->Tex)
				{
				if (UVTex->TexType == WCS_TEXTURE_TYPE_UVW)
					{
					tmap = &Material3ds->texture.map;
					if (UVTex->Img && (Rast = UVTex->Img->GetRaster()))
						{
						if (CheckAndShortenFileName(ImageShortName, Rast->GetPath(), Rast->GetName(), 12))
							Rast->PAF.SetName(ImageShortName);
						strncpy(tmap->name, ImageShortName, 12);		// name of the image file, crammed into 12 spaces
						strmfp(OriginalFullPath, Rast->GetPath(), Rast->GetName());
						CopyExistingFile(OriginalFullPath, SaveToPath, ImageShortName);
						DiffuseAlphaAvailable = (Rast->AlphaAvailable && Rast->AlphaEnabled);
						} // if
					tmap->name[12] = 0;
					tmap->percent = (float3ds)(UVTex->TexParam[WCS_TEXTURE_OPACITY].CurValue * .01);
					tmap->tiling = (tiletype3ds)(UVTex->TileWidth ? 1: 2);	//1=TileMap: 2=DecalMap
					tmap->ignorealpha = 1;
					tmap->mirror = UVTex->FlipWidth;
					tmap->negative = UVTex->ImageNeg;
					tmap->uscale = 1.0f;
					tmap->vscale = 1.0f;
					tmap->uoffset = 0.0f;
					tmap->voffset = 0.0f;
					tmap->rotation = 0.0f;
					tmap->blur = 0.0f;

					tmap = &Material3ds->specmap.map;
					if (UVTex->Img && (Rast = UVTex->Img->GetRaster()))
						{
						strncpy(tmap->name, ImageShortName, 12);		// name of the image file, crammed into 12 spaces
						} // if
					tmap->name[12] = 0;
					tmap->percent = (float3ds)(UVTex->TexParam[WCS_TEXTURE_OPACITY].CurValue * .01);
					tmap->tiling = (tiletype3ds)(UVTex->TileWidth ? 1: 2);	//1=TileMap: 2=DecalMap
					tmap->ignorealpha = 1;
					tmap->mirror = UVTex->FlipWidth;
					tmap->negative = UVTex->ImageNeg;
					tmap->uscale = 1.0f;
					tmap->vscale = 1.0f;
					tmap->uoffset = 0.0f;
					tmap->voffset = 0.0f;
					tmap->rotation = 0.0f;
					tmap->blur = 0.0f;
					} // if
				} // if
			} // if
		if (RootTex = Mat->GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY))
			{
			if (UVTex = RootTex->Tex)
				{
				if (UVTex->TexType == WCS_TEXTURE_TYPE_UVW)
					{
					tmap = &Material3ds->opacity.map;
					if (UVTex->Img && (Rast = UVTex->Img->GetRaster()))
						{
						if (CheckAndShortenFileName(ImageShortName, Rast->GetPath(), Rast->GetName(), 12))
							Rast->PAF.SetName(ImageShortName);
						strncpy(tmap->name, ImageShortName, 12);		// name of the image file, crammed into 12 spaces
						strmfp(OriginalFullPath, Rast->GetPath(), Rast->GetName());
						CopyExistingFile(OriginalFullPath, SaveToPath, ImageShortName);
						} // if
					tmap->name[12] = 0;
					tmap->percent = (float3ds)(UVTex->TexParam[WCS_TEXTURE_OPACITY].CurValue * .01);
					tmap->tiling = (tiletype3ds)(UVTex->TileWidth ? 1: 2);	//1=TileMap: 2=DecalMap
					tmap->ignorealpha = 1;
					tmap->mirror = UVTex->FlipWidth;
					tmap->negative = 1 - UVTex->ImageNeg;	// 3ds is in terms of opacity instead of transparency
					tmap->uscale = 1.0f;
					tmap->vscale = 1.0f;
					tmap->uoffset = 0.0f;
					tmap->voffset = 0.0f;
					tmap->rotation = 0.0f;
					tmap->blur = 0.0f;
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
						tmap = &Material3ds->opacity.map;
						if (UVTex->Img && (Rast = UVTex->Img->GetRaster()))
							{
							if (CheckAndShortenFileName(ImageShortName, Rast->GetPath(), Rast->GetName(), 12))
								Rast->PAF.SetName(ImageShortName);
							strncpy(tmap->name, ImageShortName, 12);		// name of the image file, crammed into 12 spaces
							} // if
						tmap->name[12] = 0;
						tmap->source = Alpha;
						tmap->percent = (float3ds)1.0;
						tmap->tiling = (tiletype3ds)(UVTex->TileWidth ? 1: 2);	//1=TileMap: 2=DecalMap
						tmap->ignorealpha = 0;
						tmap->mirror = UVTex->FlipWidth;
						tmap->negative = UVTex->ImageNeg;
						tmap->uscale = 1.0f;
						tmap->vscale = 1.0f;
						tmap->uoffset = 0.0f;
						tmap->voffset = 0.0f;
						tmap->rotation = 0.0f;
						tmap->blur = 0.0f;
						} // if
					} // if
				} // if
			} // else
		} // if
	} // if
else
	{
	sprintf(Material3ds->name, "OM%06d", MatCt);
	Material3ds->ambient.r = (float3ds)(0.0);
	Material3ds->ambient.g = (float3ds)(0.0);
	Material3ds->ambient.b = (float3ds)(0.0);
	Material3ds->diffuse.r = (float3ds)(.75);
	Material3ds->diffuse.g = (float3ds)(.75);
	Material3ds->diffuse.b = (float3ds)(.75);
	Material3ds->specular.r = (float3ds)(.75);
	Material3ds->specular.g = (float3ds)(.75);
	Material3ds->specular.b = (float3ds)(.75);
	Material3ds->shininess = (float3ds)(0.0);
	Material3ds->shinstrength = (float3ds)(0.0);
	Material3ds->shading = (shadetype3ds)1;						// typedefs don't work
	Material3ds->twosided = True3ds;
	} // else

} // ExportFormat3DS::SetMaterialProperties

/*===========================================================================*/

int ExportFormat3DS::ExportHaze(Atmosphere *CurHaze, long HazeNum)
{
atmosphere3ds *Atmosphere3ds = NULL;
int Success = 1;

InitAtmosphere3ds(&Atmosphere3ds);
if (Atmosphere3ds && ! ftkerr3ds)
	{
	if (CurHaze->HazeEnabled)
		{
		Atmosphere3ds->fog.nearplane = (float3ds)(CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].CurValue);
		Atmosphere3ds->fog.neardensity = (float3ds)(CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY].CurValue);
		Atmosphere3ds->fog.farplane = (float3ds)(CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].CurValue + CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE].CurValue);
		Atmosphere3ds->fog.fardensity = (float3ds)(CurHaze->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY].CurValue);
		Atmosphere3ds->fog.fogcolor.r = (float3ds)(CurHaze->HazeColor.GetClampedCompleteValue(0));
		Atmosphere3ds->fog.fogcolor.g = (float3ds)(CurHaze->HazeColor.GetClampedCompleteValue(1));
		Atmosphere3ds->fog.fogcolor.b = (float3ds)(CurHaze->HazeColor.GetClampedCompleteValue(2));
		Atmosphere3ds->fog.fogbgnd = False3ds;
		} // if
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
	PutAtmosphere3ds(Database3ds, Atmosphere3ds);
	ReleaseAtmosphere3ds(&Atmosphere3ds);
	} // if
else
	{
	if (Atmosphere3ds)
		ReleaseAtmosphere3ds(&Atmosphere3ds);		
	UserMessageOK("3DS Export", "Unable to initialize 3DS Atmosphere Settings!\nOperation terminated.");
	Success = 0;
	} // if
if (ftkerr3ds)
	{
	UserMessageOK("3DS Export", "Error creating 3DS Atmosphere Settings chunk!\nOperation terminated.");
	Success = 0;
	}

return (Success);

} // ExportFormat3DS::ExportHaze

/*===========================================================================*/

int ExportFormat3DS::ExportLight(Light *CurLight, long LightNum)
{
double Elev, VecLen, X, Y;
light3ds *Light3ds = NULL;
kfspot3ds *KFSpot3ds = NULL;
kfomni3ds *KFOmni3ds = NULL;
int Success = 1, AnimLight;
RenderData RendData(NULL);
RasterAnimHostProperties Prop;

if (RendData.InitToView(EffectsHost, ProjectHost, DBHost, ProjectHost->Interactive, NULL, NULL, 320, 320))
	{
	// is camera animated?
	Prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
	Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATED;
	Prop.ItemOperator = WCS_KEYOPERATION_ALLKEYS;
	Prop.TypeNumber = WCS_EFFECTSSUBCLASS_LIGHT;
	CurLight->GetRAHostProperties(&Prop);
	AnimLight = (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED) ? 1: 0;

	// determine light position from lat/lon or from light-aim vector
	// is it a spotlight or omni
	if (CurLight->LightType == WCS_EFFECTS_LIGHTTYPE_OMNI)
		{
		InitLight3ds(&Light3ds);
		if (Light3ds && ! ftkerr3ds)
			{
			sprintf(Light3ds->name, "OMNI%04d", LightNum);
			Elev = min(5000000.0, CurLight->LightPos->Elev);
			// convert WCS coords into export coords
			Master->RBounds.DefDegToRBounds(CurLight->LightPos->Lat, CurLight->LightPos->Lon, X, Y);
			Light3ds->pos.x = (float3ds)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale);
			Light3ds->pos.y = (float3ds)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
			Light3ds->pos.z = (float3ds)((Elev - Master->ExportRefData.RefElev) * Master->ExportRefData.ElevScale);
			Light3ds->color.r = (float3ds)(CurLight->Color.GetCurValue(0));
			Light3ds->color.g = (float3ds)(CurLight->Color.GetCurValue(1));
			Light3ds->color.b = (float3ds)(CurLight->Color.GetCurValue(2));
			Light3ds->multiplier = (float3ds)CurLight->Color.Intensity.GetCurValue(0);
			Light3ds->attenuation.on = (CurLight->MaxIllumDist < FLT_MAX ? True3ds: False3ds);
			Light3ds->attenuation.inner = (float3ds)1.0;
			Light3ds->attenuation.outer = (float3ds)(CurLight->MaxIllumDist < FLT_MAX ? CurLight->MaxIllumDist: FLT_MAX);
			Light3ds->dloff = (CurLight->Enabled ? False3ds: True3ds);
			PutOmnilight3ds(Database3ds, Light3ds);
			if (AnimLight)
				{
				if (ExportOmnilightKeys(CurLight, &KFOmni3ds, &RendData))
					{
					if (KFOmni3ds)
						{
						strcpy(KFOmni3ds->name, Light3ds->name);
						KFOmni3ds->flags1 = 0;
						KFOmni3ds->flags2 = KfNodeHasPath3ds;
						PutOmnilightMotion3ds(Database3ds, KFOmni3ds);
						} // if
					// else no keyframes found for some reason - not an error
					} // if
				else 
					{
					UserMessageOK("3DS Export", "Unable to initialize 3DS Omnilight Object keyframes!\nOperation terminated.");
					Success = 0;
					} // if
				if (KFOmni3ds)
					ReleaseOmnilightMotion3ds(&KFOmni3ds);
				} // if
			ReleaseLight3ds(&Light3ds);
			} // if
		else
			{
			if (Light3ds)
				ReleaseLight3ds(&Light3ds);		
			UserMessageOK("3DS Export", "Unable to initialize 3DS Light Object!\nOperation terminated.");
			Success = 0;
			} // else
		} // if
	else
		{
		InitSpotlight3ds(&Light3ds);
		if (Light3ds && ! ftkerr3ds)
			{
			sprintf(Light3ds->name, "SPOT%04d", LightNum);
			// LightAim points at the light
			NegateVector(CurLight->LightAim->XYZ);
			// find distance to light, if too large, recalc position closer
			VecLen = VectorMagnitude(CurLight->LightPos->XYZ);
			if (VecLen > 500000)
				{
				UnitVector(CurLight->LightPos->XYZ);
				MultiplyPoint3d(CurLight->LightPos->XYZ, CurLight->LightPos->XYZ, 500000.0 + RendData.PlanetRad);
				} // if
			if (CurLight->Distant)
				{
				// re-aim light at center of export area
				CurLight->LightAim->Lat = Master->ExportRefData.WCSRefLat;
				CurLight->LightAim->Lon = Master->ExportRefData.WCSRefLon;
				CurLight->LightAim->Elev = Master->ExportRefData.RefElev;
				} // if
			else
				{
				// make a false target 100m from light
				CurLight->LightAim->XYZ[0] = CurLight->LightPos->XYZ[0] + CurLight->LightAim->XYZ[0] * 100.0;
				CurLight->LightAim->XYZ[1] = CurLight->LightPos->XYZ[1] + CurLight->LightAim->XYZ[1] * 100.0;
				CurLight->LightAim->XYZ[2] = CurLight->LightPos->XYZ[2] + CurLight->LightAim->XYZ[2] * 100.0;
				#ifdef WCS_BUILD_VNS
				RendData.DefCoords->CartToDeg(CurLight->LightAim);
				#else // WCS_BUILD_VNS
				CurLight->LightAim->CartToDeg(RendData.PlanetRad);
				#endif // WCS_BUILD_VNS
				} // else
			#ifdef WCS_BUILD_VNS
			RendData.DefCoords->CartToDeg(CurLight->LightPos);
			#else // WCS_BUILD_VNS
			CurLight->LightPos->CartToDeg(RendData.PlanetRad);
			#endif // WCS_BUILD_VNS

			Master->RBounds.DefDegToRBounds(CurLight->LightPos->Lat, CurLight->LightPos->Lon, X, Y);
			Light3ds->pos.x = (float3ds)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale);
			Light3ds->pos.y = (float3ds)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
			Light3ds->pos.z = (float3ds)((CurLight->LightPos->Elev - Master->ExportRefData.RefElev) * Master->ExportRefData.ElevScale);

			Master->RBounds.DefDegToRBounds(CurLight->LightAim->Lat, CurLight->LightAim->Lon, X, Y);
			Light3ds->spot->target.x = (float3ds)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale);
			Light3ds->spot->target.y = (float3ds)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
			Light3ds->spot->target.z = (float3ds)((CurLight->LightAim->Elev - Master->ExportRefData.RefElev) * Master->ExportRefData.ElevScale);

			Light3ds->color.r = (float3ds)(CurLight->Color.GetCurValue(0));
			Light3ds->color.g = (float3ds)(CurLight->Color.GetCurValue(1));
			Light3ds->color.b = (float3ds)(CurLight->Color.GetCurValue(2));
			Light3ds->multiplier = (float3ds)CurLight->Color.Intensity.GetCurValue(0);
			Light3ds->attenuation.on = (CurLight->MaxIllumDist < FLT_MAX ? True3ds: False3ds);
			Light3ds->attenuation.inner = (float3ds)1.0;
			Light3ds->attenuation.outer = (float3ds)(CurLight->MaxIllumDist < FLT_MAX ? CurLight->MaxIllumDist: FLT_MAX);
			Light3ds->dloff = (CurLight->Enabled ? False3ds: True3ds);
			if (CurLight->LightType == WCS_EFFECTS_LIGHTTYPE_SPOT)
				{
				Light3ds->spot->hotspot = (float3ds)min(160.0, CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].CurValue);
				Light3ds->spot->falloff = (float3ds)min(160.0, CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].CurValue + CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONEEDGE].CurValue);
				} // if
			else
				{
				Light3ds->spot->hotspot = (float3ds)(160.0);
				Light3ds->spot->falloff = (float3ds)(160.0);
				} // else
			Light3ds->spot->cone.overshoot = True3ds;
			Light3ds->spot->cone.type = Circular;
			PutSpotlight3ds(Database3ds, Light3ds);
			if (AnimLight)
				{
				if (ExportSpotlightKeys(CurLight, &KFSpot3ds, &RendData))
					{
					if (KFSpot3ds)
						{
						strcpy(KFSpot3ds->name, Light3ds->name);
						KFSpot3ds->flags1 = 0;
						KFSpot3ds->flags2 = KfNodeHasPath3ds;
						PutSpotlightMotion3ds(Database3ds, KFSpot3ds);
						} // if
					// else no keyframes found for some reason - not an error
					} // if
				else 
					{
					UserMessageOK("3DS Export", "Unable to initialize 3DS Spotlight Object keyframes!\nOperation terminated.");
					Success = 0;
					} // if
				if (KFSpot3ds)
					ReleaseSpotlightMotion3ds(&KFSpot3ds);
				} // if
			ReleaseLight3ds(&Light3ds);
			} // if
		else
			{
			if (Light3ds)
				ReleaseLight3ds(&Light3ds);		
			UserMessageOK("3DS Export", "Unable to initialize 3DS Light Object!\nOperation terminated.");
			Success = 0;
			} // else
		} // if
	if (ftkerr3ds)
		{
		UserMessageOK("3DS Export", "Error creating 3DS Light Object chunk!\nOperation terminated.");
		Success = 0;
		}
	} // if

return (Success);

} // ExportFormat3DS::ExportLight

/*===========================================================================*/

int ExportFormat3DS::ExportOmnilightKeys(Light *CurLight, kfomni3ds **KFOmni3dsPtr, RenderData *RendData)
{
double FrameRate, X, Y, Elev;
int j, KeyFrames, KeysExist, LastKeyFrame, NextKeyFrame, Success = 1;
VertexDEM Vert;
kfomni3ds *KFOmni3ds = NULL;
GraphNode *GrNode;
RasterAnimHostProperties Prop;

FrameRate = GlobalApp->MainProj->Interactive->GetFrameRate();

if (KeysExist = (CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetNumNodes(0) > 1
	|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].GetNumNodes(0) > 1
	|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].GetNumNodes(0) > 1
	|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].GetNumNodes(0) > 1
	|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_PITCH].GetNumNodes(0) > 1
	|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_FALLOFFEXP].GetNumNodes(0) > 1
	|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].GetNumNodes(0) > 1
	|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONEEDGE].GetNumNodes(0) > 1
	|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LIGHTRADIUS].GetNumNodes(0) > 1
	|| CurLight->Color.GetNumNodes(0) > 1
	|| CurLight->Color.GetNumNodes(1) > 1
	|| CurLight->Color.GetNumNodes(2) > 1))
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
	Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
	LastKeyFrame = -2;
	KeyFrames = 0;

	while (1)	//lint !e716
		{
		NextKeyFrame = -1;
		Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / FrameRate;
		Prop.NewKeyNodeRange[0] = -DBL_MAX;
		Prop.NewKeyNodeRange[1] = DBL_MAX;
		CurLight->GetRAHostProperties(&Prop);
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
	} // if
else
	KeyFrames = 1;

InitOmnilightMotion3ds(KFOmni3dsPtr, KeyFrames, KeyFrames);
if ((KFOmni3ds = *KFOmni3dsPtr) && ! ftkerr3ds)
	{
	if (KeysExist)
		{
		j = 0;
		LastKeyFrame = -2;
		while (j < KeyFrames)
			{
			NextKeyFrame = -1;
			Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / FrameRate;
			Prop.NewKeyNodeRange[0] = -DBL_MAX;
			Prop.NewKeyNodeRange[1] = DBL_MAX;
			CurLight->GetRAHostProperties(&Prop);
			if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
				NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * FrameRate + .5);
			if (NextKeyFrame > LastKeyFrame && NextKeyFrame >= 0)
				{
				GrNode = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].FindNearestSiblingNode(NextKeyFrame / FrameRate, NULL);
				CurLight->SetToTime(NextKeyFrame / FrameRate);
				CurLight->InitFrameToRender(GlobalApp->AppEffects, RendData);

				Elev = min(5000000.0, CurLight->LightPos->Elev);
				// convert WCS coords into export coords
				Master->RBounds.DefDegToRBounds(CurLight->LightPos->Lat, CurLight->LightPos->Lon, X, Y);
				KFOmni3ds->pos[j].x = (float3ds)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale);
				KFOmni3ds->pos[j].y = (float3ds)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
				KFOmni3ds->pos[j].z = (float3ds)((Elev - Master->ExportRefData.RefElev) * Master->ExportRefData.ElevScale);
				KFOmni3ds->color[j].r = (float3ds)(CurLight->Color.GetCurValue(0));
				KFOmni3ds->color[j].g = (float3ds)(CurLight->Color.GetCurValue(1));
				KFOmni3ds->color[j].b = (float3ds)(CurLight->Color.GetCurValue(2));

				KFOmni3ds->pkeys[j].time = (ulong3ds)NextKeyFrame;
				if (GrNode)
					{
					KFOmni3ds->pkeys[j].rflags = (GrNode->TCB[0] != 0.0) | ((GrNode->TCB[1] != 0.0) << 1) | ((GrNode->TCB[2] != 0.0) << 2);
					KFOmni3ds->pkeys[j].tension = (float3ds)GrNode->TCB[0];
					KFOmni3ds->pkeys[j].continuity = (float3ds)GrNode->TCB[1];
					KFOmni3ds->pkeys[j].bias = (float3ds)GrNode->TCB[2];
					} // if
				else
					{
					KFOmni3ds->pkeys[j].rflags = 0;
					KFOmni3ds->pkeys[j].tension = KFOmni3ds->pkeys[j].continuity = KFOmni3ds->pkeys[j].bias = (float3ds)0.0;
					} // else
				KFOmni3ds->ckeys[j].time = (ulong3ds)NextKeyFrame;
				j ++;

				LastKeyFrame = NextKeyFrame;
				} // if
			else
				break;
			} // while
		} // if KeysExist
	else
		{
		Elev = min(5000000.0, CurLight->LightPos->Elev);
		// convert WCS coords into export coords
		Master->RBounds.DefDegToRBounds(CurLight->LightPos->Lat, CurLight->LightPos->Lon, X, Y);
		KFOmni3ds->pos[0].x = (float3ds)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale);
		KFOmni3ds->pos[0].y = (float3ds)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
		KFOmni3ds->pos[0].z = (float3ds)((Elev - Master->ExportRefData.RefElev) * Master->ExportRefData.ElevScale);
		KFOmni3ds->color[0].r = (float3ds)(CurLight->Color.GetCurValue(0));
		KFOmni3ds->color[0].g = (float3ds)(CurLight->Color.GetCurValue(1));
		KFOmni3ds->color[0].b = (float3ds)(CurLight->Color.GetCurValue(2));

		KFOmni3ds->pkeys[0].time = (ulong3ds)0;
		KFOmni3ds->pkeys[0].rflags = 0;
		KFOmni3ds->ckeys[0].time = (ulong3ds)0;
		} // if ! KeysExist
	} // if light key frames
else
	{
	if (KFOmni3ds)
		ReleaseOmnilightMotion3ds(KFOmni3dsPtr);
	Success = 0;
	} // else if

return (Success);


} // ExportFormat3DS::ExportOmnilightKeys

/*===========================================================================*/

int ExportFormat3DS::ExportSpotlightKeys(Light *CurLight, kfspot3ds **KFSpot3dsPtr, RenderData *RendData)
{
double FrameRate, VecLen, X, Y;
int j, KeyFrames, KeysExist, LastKeyFrame, NextKeyFrame, Success = 1;
VertexDEM Vert;
kfspot3ds *KFSpot3ds = NULL;
GraphNode *GrNode;
RasterAnimHostProperties Prop;

FrameRate = GlobalApp->MainProj->Interactive->GetFrameRate();

if (KeysExist = (CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetNumNodes(0) > 1
	|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].GetNumNodes(0) > 1
	|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].GetNumNodes(0) > 1
	|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_HEADING].GetNumNodes(0) > 1
	|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_PITCH].GetNumNodes(0) > 1
	|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_FALLOFFEXP].GetNumNodes(0) > 1
	|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].GetNumNodes(0) > 1
	|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONEEDGE].GetNumNodes(0) > 1
	|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LIGHTRADIUS].GetNumNodes(0) > 1
	|| CurLight->Color.GetNumNodes(0) > 1
	|| CurLight->Color.GetNumNodes(1) > 1
	|| CurLight->Color.GetNumNodes(2) > 1))
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
	Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
	LastKeyFrame = -2;
	KeyFrames = 0;

	while (1)	//lint !e716
		{
		NextKeyFrame = -1;
		Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / FrameRate;
		Prop.NewKeyNodeRange[0] = -DBL_MAX;
		Prop.NewKeyNodeRange[1] = DBL_MAX;
		CurLight->GetRAHostProperties(&Prop);
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
	} // if
else
	KeyFrames = 1;

InitSpotlightMotion3ds(KFSpot3dsPtr, KeyFrames, KeyFrames, KeyFrames, KeyFrames, 1, KeyFrames);
if ((KFSpot3ds = *KFSpot3dsPtr) && ! ftkerr3ds)
	{
	if (KeysExist)
		{
		j = 0;
		LastKeyFrame = -2;
		while (j < KeyFrames)
			{
			NextKeyFrame = -1;
			Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / FrameRate;
			Prop.NewKeyNodeRange[0] = -DBL_MAX;
			Prop.NewKeyNodeRange[1] = DBL_MAX;
			CurLight->GetRAHostProperties(&Prop);
			if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
				NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * FrameRate + .5);
			if (NextKeyFrame > LastKeyFrame && NextKeyFrame >= 0)
				{
				GrNode = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].FindNearestSiblingNode(NextKeyFrame / FrameRate, NULL);
				CurLight->SetToTime(NextKeyFrame / FrameRate);
				CurLight->InitFrameToRender(GlobalApp->AppEffects, RendData);

				// LightAim points at the light
				NegateVector(CurLight->LightAim->XYZ);
				// find distance to light, if too large, recalc position closer
				VecLen = VectorMagnitude(CurLight->LightPos->XYZ);
				if (VecLen > 500000)
					{
					UnitVector(CurLight->LightPos->XYZ);
					MultiplyPoint3d(CurLight->LightPos->XYZ, CurLight->LightPos->XYZ, 500000.0 + RendData->PlanetRad);
					} // if
				if (CurLight->Distant)
					{
					// re-aim light at center of export area
					CurLight->LightAim->Lat = Master->ExportRefData.WCSRefLat;
					CurLight->LightAim->Lon = Master->ExportRefData.WCSRefLon;
					CurLight->LightAim->Elev = Master->ExportRefData.RefElev;
					} // if
				else
					{
					// make a false target 100m from light
					CurLight->LightAim->XYZ[0] = CurLight->LightPos->XYZ[0] + CurLight->LightAim->XYZ[0] * 100.0;
					CurLight->LightAim->XYZ[1] = CurLight->LightPos->XYZ[1] + CurLight->LightAim->XYZ[1] * 100.0;
					CurLight->LightAim->XYZ[2] = CurLight->LightPos->XYZ[2] + CurLight->LightAim->XYZ[2] * 100.0;
					#ifdef WCS_BUILD_VNS
					RendData->DefCoords->CartToDeg(CurLight->LightAim);
					#else // WCS_BUILD_VNS
					CurLight->LightAim->CartToDeg(RendData->PlanetRad);
					#endif // WCS_BUILD_VNS
					} // else
				#ifdef WCS_BUILD_VNS
				RendData->DefCoords->CartToDeg(CurLight->LightPos);
				#else // WCS_BUILD_VNS
				CurLight->LightPos->CartToDeg(RendData->PlanetRad);
				#endif // WCS_BUILD_VNS

				Master->RBounds.DefDegToRBounds(CurLight->LightPos->Lat, CurLight->LightPos->Lon, X, Y);
				KFSpot3ds->pos[j].x = (float3ds)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale);
				KFSpot3ds->pos[j].y = (float3ds)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
				KFSpot3ds->pos[j].z = (float3ds)((CurLight->LightPos->Elev - Master->ExportRefData.RefElev) * Master->ExportRefData.ElevScale);

				Master->RBounds.DefDegToRBounds(CurLight->LightAim->Lat, CurLight->LightAim->Lon, X, Y);
				KFSpot3ds->tpos[j].x = (float3ds)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale);
				KFSpot3ds->tpos[j].y = (float3ds)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
				KFSpot3ds->tpos[j].z = (float3ds)((CurLight->LightAim->Elev - Master->ExportRefData.RefElev) * Master->ExportRefData.ElevScale);

				KFSpot3ds->color[j].r = (float3ds)(CurLight->Color.GetCurValue(0));
				KFSpot3ds->color[j].g = (float3ds)(CurLight->Color.GetCurValue(1));
				KFSpot3ds->color[j].b = (float3ds)(CurLight->Color.GetCurValue(2));

				if (CurLight->LightType == WCS_EFFECTS_LIGHTTYPE_SPOT)
					{
					KFSpot3ds->hot[j] = (float3ds)min(160.0, CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].CurValue);
					KFSpot3ds->fall[j] = (float3ds)min(160.0, CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].CurValue + CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONEEDGE].CurValue);
					} // if
				else
					{
					KFSpot3ds->hot[j] = (float3ds)(160.0);
					KFSpot3ds->fall[j] = (float3ds)(160.0);
					} // else

				KFSpot3ds->pkeys[j].time = (ulong3ds)NextKeyFrame;
				if (GrNode)
					{
					KFSpot3ds->pkeys[j].rflags = (GrNode->TCB[0] != 0.0) | ((GrNode->TCB[1] != 0.0) << 1) | ((GrNode->TCB[2] != 0.0) << 2);
					KFSpot3ds->pkeys[j].tension = (float3ds)GrNode->TCB[0];
					KFSpot3ds->pkeys[j].continuity = (float3ds)GrNode->TCB[1];
					KFSpot3ds->pkeys[j].bias = (float3ds)GrNode->TCB[2];
					} // if
				else
					{
					KFSpot3ds->pkeys[j].rflags = 0;
					KFSpot3ds->pkeys[j].tension = KFSpot3ds->pkeys[j].continuity = KFSpot3ds->pkeys[j].bias = (float3ds)0.0;
					} // else
				KFSpot3ds->tkeys[j].time = (ulong3ds)NextKeyFrame;
				KFSpot3ds->ckeys[j].time = (ulong3ds)NextKeyFrame;
				KFSpot3ds->hkeys[j].time = (ulong3ds)NextKeyFrame;
				KFSpot3ds->fkeys[j].time = (ulong3ds)NextKeyFrame;
				j ++;

				LastKeyFrame = NextKeyFrame;
				} // if
			else
				break;
			} // while
		} // if KeysExist
	else
		{
		Master->RBounds.DefDegToRBounds(CurLight->LightPos->Lat, CurLight->LightPos->Lon, X, Y);
		KFSpot3ds->pos[0].x = (float3ds)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale);
		KFSpot3ds->pos[0].y = (float3ds)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
		KFSpot3ds->pos[0].z = (float3ds)((CurLight->LightPos->Elev - Master->ExportRefData.RefElev) * Master->ExportRefData.ElevScale);

		Master->RBounds.DefDegToRBounds(CurLight->LightAim->Lat, CurLight->LightAim->Lon, X, Y);
		KFSpot3ds->tpos[0].x = (float3ds)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale);
		KFSpot3ds->tpos[0].y = (float3ds)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
		KFSpot3ds->tpos[0].z = (float3ds)((CurLight->LightAim->Elev - Master->ExportRefData.RefElev) * Master->ExportRefData.ElevScale);

		KFSpot3ds->color[0].r = (float3ds)(CurLight->Color.GetCurValue(0));
		KFSpot3ds->color[0].g = (float3ds)(CurLight->Color.GetCurValue(1));
		KFSpot3ds->color[0].b = (float3ds)(CurLight->Color.GetCurValue(2));

		if (CurLight->LightType == WCS_EFFECTS_LIGHTTYPE_SPOT)
			{
			KFSpot3ds->hot[0] = (float3ds)min(160.0, CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].CurValue);
			KFSpot3ds->fall[0] = (float3ds)min(160.0, CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONE].CurValue + CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_SPOTCONEEDGE].CurValue);
			} // if
		else
			{
			KFSpot3ds->hot[0] = (float3ds)(160.0);
			KFSpot3ds->fall[0] = (float3ds)(160.0);
			} // else

		KFSpot3ds->pkeys[0].time = (ulong3ds)0;
		KFSpot3ds->pkeys[0].rflags = 0;
		KFSpot3ds->tkeys[0].time = (ulong3ds)0;
		KFSpot3ds->ckeys[0].time = (ulong3ds)0;
		KFSpot3ds->hkeys[0].time = (ulong3ds)0;
		KFSpot3ds->fkeys[0].time = (ulong3ds)0;
		} // if ! KeysExist
	KFSpot3ds->roll[0] = (float3ds)0.0;
	KFSpot3ds->rkeys[0].time = (ulong3ds)0;
	} // if light key frames
else
	{
	if (KFSpot3ds)
		ReleaseSpotlightMotion3ds(KFSpot3dsPtr);
	Success = 0;
	} // else if

return (Success);

} // ExportFormat3DS::ThreeDSLight_SetKeys

/*===========================================================================*/

int ExportFormat3DS::ExportCamera(Camera *CurCamera, long CameraNum)
{
double /* Heading, Pitch, */ Bank, ViewAngle, X, Y;
camera3ds *Camera3ds = NULL;
kfcamera3ds *KFCamera3ds = NULL;
int Success = 1, AnimCam;
RenderData RendData(NULL);
RasterAnimHostProperties Prop;

if (RendData.InitToView(EffectsHost, ProjectHost, DBHost, ProjectHost->Interactive, NULL, CurCamera, 320, 320))
	{
	// is camera animated?
	Prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
	Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATED;
	Prop.ItemOperator = WCS_KEYOPERATION_ALLKEYS;
	Prop.TypeNumber = WCS_EFFECTSSUBCLASS_CAMERA;
	CurCamera->GetRAHostProperties(&Prop);
	AnimCam = (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED) ? 1: 0;

	InitCamera3ds(&Camera3ds);
	if (Camera3ds && ! ftkerr3ds)
		{
		sprintf(Camera3ds->name, "CAM%04d", CameraNum);

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

		Master->RBounds.DefDegToRBounds(CurCamera->CamPos->Lat, CurCamera->CamPos->Lon, X, Y);
		Camera3ds->position.x = (float3ds)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale);
		Camera3ds->position.y = (float3ds)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
		Camera3ds->position.z = (float3ds)((CurCamera->CamPos->Elev - Master->ExportRefData.RefElev) * Master->ExportRefData.ElevScale);

		Master->RBounds.DefDegToRBounds(CurCamera->TargPos->Lat, CurCamera->TargPos->Lon, X, Y);
		Camera3ds->target.x = (float3ds)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale);
		Camera3ds->target.y = (float3ds)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
		Camera3ds->target.z = (float3ds)((CurCamera->TargPos->Elev - Master->ExportRefData.RefElev) * Master->ExportRefData.ElevScale);

		//Heading = CurCamera->CamHeading;
		//Pitch = CurCamera->CamPitch;
		Bank = CurCamera->CamBank;
		if (fabs(Bank) >= 360.0)
			Bank = fmod(Bank, 360.0);	// retains the original sign
		if (Bank < 0.0)
			Bank += 360.0;
		Camera3ds->roll = (float3ds)(Bank);
		ViewAngle = CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue;
		if (ViewAngle < .005)
			ViewAngle = .005;
		if (ViewAngle > 175.0)
			ViewAngle = 175.0;
		Camera3ds->fov = (float3ds)ViewAngle;
		Camera3ds->showcone = True3ds;
		Camera3ds->ranges.cam_near = (float3ds)(0.0);
		Camera3ds->ranges.cam_far = (float3ds)(1000000.0);
		PutCamera3ds(Database3ds, Camera3ds);
		if (AnimCam)
			{
			if (ExportCameraKeys(CurCamera, &KFCamera3ds, &RendData))
				{
				if (KFCamera3ds)
					{
					strcpy(KFCamera3ds->name, Camera3ds->name);
					KFCamera3ds->flags1 = 0;
					KFCamera3ds->flags2 = KfNodeHasPath3ds;
					PutCameraMotion3ds(Database3ds, KFCamera3ds);
					} // if
				// else no keyframes found for some reason - not an error
				} // if
			else 
				{
				UserMessageOK("3DS Export", "Unable to initialize 3DS Camera Object keyframes!\nOperation terminated.");
				Success = 0;
				} // if
			if (KFCamera3ds)
				ReleaseCameraMotion3ds(&KFCamera3ds);
			} // if
		ReleaseCamera3ds(&Camera3ds);
		} // if
	else
		{
		if (Camera3ds)
			ReleaseCamera3ds(&Camera3ds);
		UserMessageOK("3DS Export", "Unable to initialize 3DS Camera Object!\nOperation terminated.");
		Success = 0;
		} // else
	if (ftkerr3ds)
		{
		UserMessageOK("3DS Export", "Error creating 3DS Camera Object chunk!\nOperation terminated.");
		Success = 0;
		}
	} // if RendData
else
	Success = 0;

return (Success);

} // ExportFormat3DS::ExportCamera

/*===========================================================================*/

int ExportFormat3DS::ExportCameraKeys(Camera *CurCamera, kfcamera3ds **KFCamera3dsPtr, RenderData *RendData)
{
double FrameRate;
long TotalKeys, CameraKeys = 0, BankKeys, FovKeys, LastKeyFrame, NextKeyFrame, j;
ThreeDSMotion *LWM = NULL;
kfcamera3ds *KFCamera3ds = NULL;
GraphNode *GrNode;
int Success = 1;
RasterAnimHostProperties Prop;

// determine if key frames exist for the camera motion path 

FrameRate = GlobalApp->MainProj->Interactive->GetFrameRate();
TotalKeys = 0;
LastKeyFrame = -2;
while ((LastKeyFrame = CurCamera->GetNextMotionKeyFrame(LastKeyFrame, FrameRate)) >= 0)
	CameraKeys ++;
BankKeys = CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].GetNumNodes(0);
FovKeys = CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].GetNumNodes(0);

TotalKeys = CameraKeys + BankKeys + FovKeys;
if (TotalKeys < 1)
	{
	return (1);
	} // no motion key frames 

if ((LWM = new ThreeDSMotion()) == NULL)
	{
	return (0);
	} // if out of memory 

InitCameraMotion3ds(KFCamera3dsPtr, CameraKeys > 0 ? CameraKeys: 1, FovKeys > 0 ? FovKeys: 1, BankKeys > 0 ? BankKeys: 1, CameraKeys > 0 ? CameraKeys: 1);
if ((KFCamera3ds = *KFCamera3dsPtr) && ! ftkerr3ds)
	{
	if (CameraKeys)
		{
		LastKeyFrame = -2;
		j = 0;
		while (((LastKeyFrame = CurCamera->GetNextMotionKeyFrame(LastKeyFrame, FrameRate)) >= 0) && j < CameraKeys)
			{
			// init camera to this frame position
			if (CurCamera->TargetObj)
				CurCamera->TargetObj->SetToTime(LastKeyFrame / FrameRate);
			CurCamera->SetToTime(LastKeyFrame / FrameRate);
			CurCamera->InitFrameToRender(GlobalApp->AppEffects, RendData);
			// determine HPB
			Set_3DSCamKey(CurCamera, LWM, RendData);
			// determine TCB - we'll need the nearest node for that
			if (GrNode = CurCamera->GetNearestMotionNode(LastKeyFrame / FrameRate))
				{
				LWM->TCB[0] = GrNode->TCB[0];
				LWM->TCB[1] = GrNode->TCB[1];
				LWM->TCB[2] = GrNode->TCB[2];
				} // if node found
			else
				{
				LWM->TCB[0] = 0.0;
				LWM->TCB[1] = 0.0;
				LWM->TCB[2] = 0.0;
				} // else
			KFCamera3ds->pos[j].x = (float3ds)(LWM->CamXYZ[0]);
			KFCamera3ds->pos[j].y = (float3ds)(LWM->CamXYZ[1]);
			KFCamera3ds->pos[j].z = (float3ds)(LWM->CamXYZ[2]);
			KFCamera3ds->pkeys[j].time = (ulong3ds)LastKeyFrame;
			KFCamera3ds->pkeys[j].rflags = (LWM->TCB[0] != 0.0) | ((LWM->TCB[1] != 0.0) << 1) | ((LWM->TCB[2] != 0.0) << 2);
			KFCamera3ds->pkeys[j].tension = (float3ds)LWM->TCB[0];
			KFCamera3ds->pkeys[j].continuity = (float3ds)LWM->TCB[1];
			KFCamera3ds->pkeys[j].bias = (float3ds)LWM->TCB[2];

			KFCamera3ds->tpos[j].x = (float3ds)(LWM->TargetXYZ[0]);
			KFCamera3ds->tpos[j].y = (float3ds)(LWM->TargetXYZ[1]);
			KFCamera3ds->tpos[j].z = (float3ds)(LWM->TargetXYZ[2]);
			KFCamera3ds->tkeys[j].time = (ulong3ds)LastKeyFrame;
			KFCamera3ds->tkeys[j].rflags = (LWM->TCB[0] != 0.0) | ((LWM->TCB[1] != 0.0) << 1) | ((LWM->TCB[2] != 0.0) << 2);
			KFCamera3ds->tkeys[j].tension = (float3ds)LWM->TCB[0];
			KFCamera3ds->tkeys[j].continuity = (float3ds)LWM->TCB[1];
			KFCamera3ds->tkeys[j].bias = (float3ds)LWM->TCB[2];
			j ++;
			} // while
		} // if camera motion keys
	else
		{
		// already initialized
		Set_3DSCamKey(CurCamera, LWM, RendData);
		KFCamera3ds->pos[0].x = (float3ds)(LWM->CamXYZ[0]);
		KFCamera3ds->pos[0].y = (float3ds)(LWM->CamXYZ[1]);
		KFCamera3ds->pos[0].z = (float3ds)(LWM->CamXYZ[2]);
		KFCamera3ds->pkeys[0].time = (ulong3ds)0;
		KFCamera3ds->pkeys[0].rflags = 0;

		KFCamera3ds->tpos[0].x = (float3ds)(LWM->TargetXYZ[0]);
		KFCamera3ds->tpos[0].y = (float3ds)(LWM->TargetXYZ[1]);
		KFCamera3ds->tpos[0].z = (float3ds)(LWM->TargetXYZ[2]);
		KFCamera3ds->tkeys[0].time = (ulong3ds)0;
		KFCamera3ds->tkeys[0].rflags = 0;
		} // else ! camera motion keys

	if (BankKeys)
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
		Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
		j = 0;
		LastKeyFrame = -2;
		while (j < BankKeys)
			{
			NextKeyFrame = -1;
			Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / FrameRate;
			Prop.NewKeyNodeRange[0] = -DBL_MAX;
			Prop.NewKeyNodeRange[1] = DBL_MAX;
			CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].GetRAHostProperties(&Prop);
			if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
				NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * FrameRate + .5);
			if (NextKeyFrame > LastKeyFrame && NextKeyFrame >= 0)
				{
				if (GrNode = CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].FindNearestSiblingNode(NextKeyFrame / FrameRate, NULL))
					{
					CurCamera->SetToTime(NextKeyFrame / FrameRate);
					CurCamera->InitFrameToRender(GlobalApp->AppEffects, RendData);
					Set_3DSCamKey(CurCamera, LWM, RendData);
					LWM->TCB[0] = GrNode->TCB[0];
					LWM->TCB[1] = GrNode->TCB[1];
					LWM->TCB[2] = GrNode->TCB[2];
					KFCamera3ds->roll[j] = (float3ds)(LWM->Bank);
					KFCamera3ds->rkeys[j].time = (ulong3ds)NextKeyFrame;
					KFCamera3ds->rkeys[j].rflags = (LWM->TCB[0] != 0.0) | ((LWM->TCB[1] != 0.0) << 1) | ((LWM->TCB[2] != 0.0) << 2);
					KFCamera3ds->rkeys[j].tension = (float3ds)LWM->TCB[0];
					KFCamera3ds->rkeys[j].continuity = (float3ds)LWM->TCB[1];
					KFCamera3ds->rkeys[j].bias = (float3ds)LWM->TCB[2];
					j ++;

					LastKeyFrame = NextKeyFrame;
					} // if
				else
					break;
				} // if
			else
				break;
			} // while
		} // if BankKeys
	else
		{
		Set_3DSCamKey(CurCamera, LWM, RendData);
		KFCamera3ds->roll[0] = (float3ds)(LWM->Bank);
		KFCamera3ds->rkeys[0].time = (ulong3ds)0;
		KFCamera3ds->rkeys[0].rflags = 0;
		} // if ! BankKeys

	if (FovKeys)
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
		Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
		j = 0;
		LastKeyFrame = -2;
		while (j < BankKeys)
			{
			NextKeyFrame = -1;
			Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / FrameRate;
			Prop.NewKeyNodeRange[0] = -DBL_MAX;
			Prop.NewKeyNodeRange[1] = DBL_MAX;
			CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].GetRAHostProperties(&Prop);
			if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
				NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * FrameRate + .5);
			if (NextKeyFrame > LastKeyFrame && NextKeyFrame >= 0)
				{
				if (GrNode = CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].FindNearestSiblingNode(NextKeyFrame / FrameRate, NULL))
					{
					CurCamera->SetToTime(NextKeyFrame / FrameRate);
					Set_3DSCamKey(CurCamera, LWM, RendData);
					LWM->TCB[0] = GrNode->TCB[0];
					LWM->TCB[1] = GrNode->TCB[1];
					LWM->TCB[2] = GrNode->TCB[2];
					KFCamera3ds->fov[j] = (float3ds)LWM->FOV;
					KFCamera3ds->fkeys[j].time = (ulong3ds)NextKeyFrame;
					KFCamera3ds->fkeys[j].rflags = (LWM->TCB[0] != 0.0) | ((LWM->TCB[1] != 0.0) << 1) | ((LWM->TCB[2] != 0.0) << 2);
					KFCamera3ds->fkeys[j].tension = (float3ds)LWM->TCB[0];
					KFCamera3ds->fkeys[j].continuity = (float3ds)LWM->TCB[1];
					KFCamera3ds->fkeys[j].bias = (float3ds)LWM->TCB[2];
					j ++;

					LastKeyFrame = NextKeyFrame;
					} // if
				else
					break;
				} // if
			else
				break;
			} // while
		} // if FovKeys
	else
		{
		Set_3DSCamKey(CurCamera, LWM, RendData);
		KFCamera3ds->fov[0] = (float3ds)LWM->FOV;
		KFCamera3ds->fkeys[0].time = (ulong3ds)0;
		KFCamera3ds->fkeys[0].rflags = 0;
		} // if ! FovKeys

	} // if
else
	{
	if (KFCamera3ds)
		ReleaseCameraMotion3ds(KFCamera3dsPtr);
	Success = 0;
	} // else

if (LWM)
	delete LWM;

return (Success);

} // ExportFormat3DS::ExportCameraKeys

/*===========================================================================*/

int ExportFormat3DS::Set_3DSCamKey(Camera *CurCamera, ThreeDSMotion *LWM, RenderData *RendData)
{
double X, Y, Bank;

// make a false target 10m from camera
CurCamera->TargPos->XYZ[0] = CurCamera->CamPos->XYZ[0] + CurCamera->TargPos->XYZ[0] * 10.0;
CurCamera->TargPos->XYZ[1] = CurCamera->CamPos->XYZ[1] + CurCamera->TargPos->XYZ[1] * 10.0;
CurCamera->TargPos->XYZ[2] = CurCamera->CamPos->XYZ[2] + CurCamera->TargPos->XYZ[2] * 10.0;

#ifdef WCS_BUILD_VNS
RendData->DefCoords->CartToDeg(CurCamera->CamPos);
RendData->DefCoords->CartToDeg(CurCamera->TargPos);
#else // WCS_BUILD_VNS
CurCamera->CamPos->CartToDeg(RendData->PlanetRad);
CurCamera->TargPos->CartToDeg(RendData->PlanetRad);
#endif // WCS_BUILD_VNS

Master->RBounds.DefDegToRBounds(CurCamera->CamPos->Lat, CurCamera->CamPos->Lon, X, Y);
LWM->CamXYZ[0] = (float3ds)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale);
LWM->CamXYZ[1] = (float3ds)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
LWM->CamXYZ[2] = (float3ds)((CurCamera->CamPos->Elev - Master->ExportRefData.RefElev) * Master->ExportRefData.ElevScale);

Master->RBounds.DefDegToRBounds(CurCamera->TargPos->Lat, CurCamera->TargPos->Lon, X, Y);
LWM->TargetXYZ[0] = (float3ds)((X - Master->ExportRefData.ExportRefLon) * Master->ExportRefData.ExportLonScale);
LWM->TargetXYZ[1] = (float3ds)((Y - Master->ExportRefData.ExportRefLat) * Master->ExportRefData.ExportLatScale);
LWM->TargetXYZ[2] = (float3ds)((CurCamera->TargPos->Elev - Master->ExportRefData.RefElev) * Master->ExportRefData.ElevScale);

Bank = CurCamera->CamBank;
if (fabs(Bank) >= 360.0)
	Bank = fmod(Bank, 360.0);	// retains the original sign
if (Bank < 0.0)
	Bank += 360.0;
LWM->Bank = Bank;

LWM->FOV = CurCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue;
if (LWM->FOV < .001)
	LWM->FOV = .001;
if (LWM->FOV > 179.0)
	LWM->FOV = 179.0;

return (1);

} // ExportFormat3DS::Set_3DSCamKey() 

/*===========================================================================*/

int ExportFormat3DS::FindFile(Object3DEffect *Object3D, char *FoundPath)
{
char Extension[256];
FILE *ffile;

if (stcgfe(Extension, Object3D->FileName))
	{
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

} // ExportFormat3DS::FindFile
