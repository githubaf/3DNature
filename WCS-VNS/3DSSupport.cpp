// 3DSSupport.cpp
// Code for 3D Studio file export
// Built on 11/4/96 by Gary Huber
// Modified 3/29/00 by GRH.
// Copyright 1996-2000 Questar Productions

#include "stdafx.h"
//#define WCS_SUPPORT_3DS
#include "MathSupport.h"
#include "Requester.h"
#include "Render.h"
#include "DEM.h"
#include "Project.h"
#include "Log.h"
#include "Database.h"
#include "Joe.h"
#include "Useful.h"
#include "SceneExportGUI.h"
#include "SceneImportGUI.h"
#include "Points.h"
#include "EffectsLib.h"
#include "AppMem.h"
#include "GraphData.h"
#include "Toolbar.h"

#ifdef WCS_SUPPORT_3DS
#include "3dsftk.h"

void SceneExportGUI::ThreeDSScene_Export(ImportInfo *LWInfo, RenderData *Rend)
{
database3ds *Database3ds = NULL;
meshset3ds *MeshSet3ds = NULL;
atmosphere3ds *Atmosphere3ds = NULL;
//background3ds *Background3ds = NULL;
viewport3ds *Viewport3ds = NULL;
material3ds *Material3ds = NULL;
mesh3ds *Mesh3ds = NULL;
light3ds *Light3ds = NULL;
camera3ds *Camera3ds = NULL;
kfsets3ds *KFSets3ds = NULL;
kfmesh3ds *KFMesh3ds = NULL;
kfspot3ds *KFSpot3ds = NULL;
kfcamera3ds *KFCamera3ds = NULL;

file3ds *File3ds = NULL;
char ScenePath[256], SceneFile[64], filename[256], Ptrn[32];
long Vertices = 0, Polys = 0, i, Rows, Cols, RowInt, ColInt;
double Heading, Pitch, Bank, ViewAngle;
VertexDEM Vert, FP, CV;
DEM Topo;
Joe *MyTurn;
VectorPoint *Point;
Light *CurLight;
Atmosphere *DefAtmo;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;

// check center X and Center Y 

if (Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX].CurValue != .5 ||
	Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY].CurValue != .5)
	{
	if (! UserMessageOKCAN("Scene Export", "Center X and/or Center Y Camera Parameters\nare not in the center of the image!\nThis offset will not be reflected in the exported scene.", 1))
		{
		return;
		} // if cancel
	} // if

if (LWInfo->Path[0] == 0)
	{
	if (ProjHost->altobjectpath[0] == 0)
		strcpy(ProjHost->altobjectpath, "WCSProjects:");
	strcpy(ScenePath, ProjHost->altobjectpath);
	} /* if */
else
	{
	strcpy(ScenePath, LWInfo->Path);
	} // else
strcpy(SceneFile, ProjHost->projectname);
if (! strcmp(&SceneFile[strlen(SceneFile) - 5], ".proj"))
	SceneFile[strlen(SceneFile) - 5] = 0;
if (LWInfo->ExportItem == 2)
	{
	strcat(SceneFile, ".3ds");
	strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD("3ds"));
	} // if DEM only
else
	{
	strcat(SceneFile, ".prj");
	strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD("prj"));
//	strcat(SceneFile, ".3ds");
//	strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD("3ds"));
	} // else
if (! GetFileNamePtrn(1, "Scene Path/File", ScenePath, SceneFile, Ptrn, 64))
	{
	return;
	} /* if */
strmfp(filename, ProjHost->MungPath(ScenePath), SceneFile);
strcpy(LWInfo->Path, ProjHost->MungPath(ScenePath));

SetupForExport(LWInfo, Rend);

LWInfo->MaxVerts = min(LWInfo->MaxVerts, 32767);
if (LWInfo->MaxPolys < 2 || LWInfo->MaxVerts < 4)
	return;

InitDatabase3ds(&Database3ds);
if (Database3ds && ! ftkerr3ds)
	{
	CreateNewDatabase3ds(Database3ds, ProjectFile);
//	CreateNewDatabase3ds(Database3ds, MeshFile);
	if (Database3ds->topchunk)
		{
		// object settings
		InitMeshSet3ds(&MeshSet3ds);
		if (MeshSet3ds && ! ftkerr3ds)
			{
			MeshSet3ds->masterscale = (float3ds)(ConvertFromMeters(1.0, WCS_USEFUL_UNIT_INCH) / LWInfo->UnitScale);
			MeshSet3ds->shadow.type = UseShadowMap;				// this doesn't seem to get written into the database or file
			if (DefAtmo = (Atmosphere *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_ATMOSPHERE, 0, NULL))
				{
				MeshSet3ds->ambientlight.r = (float3ds)(DefAtmo->TopAmbientColor.GetCurValue(0));
				MeshSet3ds->ambientlight.g = (float3ds)(DefAtmo->TopAmbientColor.GetCurValue(1));
				MeshSet3ds->ambientlight.b = (float3ds)(DefAtmo->TopAmbientColor.GetCurValue(2));
				} // if
			PutMeshSet3ds(Database3ds, MeshSet3ds);
			ReleaseMeshSet3ds(&MeshSet3ds);
			} // if
		else
			{
			if (MeshSet3ds)
				ReleaseMeshSet3ds(&MeshSet3ds);		
			UserMessageOK("3D Studio Scene Export", "Unable to initialize 3DS Mesh Settings!\nOperation terminated.");
			goto EndIt;
			} // if
		if (ftkerr3ds)
			{
			UserMessageOK("3D Studio Scene Export", "Error creating 3DS Mesh Settings chunk!\nOperation terminated.");
			goto EndIt;
			}

		// atmosphere settings
		if (LWInfo->ExportItem <= 1)
			{
			if (DefAtmo = (Atmosphere *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_ATMOSPHERE, 0, NULL))
				{
				InitAtmosphere3ds(&Atmosphere3ds);
				if (Atmosphere3ds && ! ftkerr3ds)
					{
					if (DefAtmo->HazeEnabled)
						{
						Atmosphere3ds->fog.nearplane = (float3ds)(DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].CurValue);
						Atmosphere3ds->fog.neardensity = (float3ds)(DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTARTINTENSITY].CurValue);
						Atmosphere3ds->fog.farplane = (float3ds)(DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZESTART].CurValue + DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZERANGE].CurValue);
						Atmosphere3ds->fog.fardensity = (float3ds)(DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_HAZEENDINTENSITY].CurValue);
						Atmosphere3ds->fog.fogcolor.r = (float3ds)(DefAtmo->HazeColor.GetClampedCompleteValue(0));
						Atmosphere3ds->fog.fogcolor.g = (float3ds)(DefAtmo->HazeColor.GetClampedCompleteValue(1));
						Atmosphere3ds->fog.fogcolor.b = (float3ds)(DefAtmo->HazeColor.GetClampedCompleteValue(2));
						Atmosphere3ds->fog.fogbgnd = False3ds;
						} // if
					if (DefAtmo->FogEnabled)
						{
						Atmosphere3ds->layerfog.zmin = (float3ds)DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEV].CurValue;
						Atmosphere3ds->layerfog.zmax = (float3ds)DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGHIGHELEV].CurValue;
						Atmosphere3ds->layerfog.density = (float3ds)(DefAtmo->AnimPar[WCS_EFFECTS_ATMOSPHERE_ANIMPAR_FOGLOWELEVINTENSITY].CurValue);
						Atmosphere3ds->layerfog.fogcolor.r = (float3ds)(DefAtmo->FogColor.GetClampedCompleteValue(0));
						Atmosphere3ds->layerfog.fogcolor.r = (float3ds)(DefAtmo->FogColor.GetClampedCompleteValue(1));
						Atmosphere3ds->layerfog.fogcolor.r = (float3ds)(DefAtmo->FogColor.GetClampedCompleteValue(2));
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
					UserMessageOK("3D Studio Scene Export", "Unable to initialize 3DS Atmosphere Settings!\nOperation terminated.");
					goto EndIt;
					} // if
				if (ftkerr3ds)
					{
					UserMessageOK("3D Studio Scene Export", "Error creating 3DS Atmosphere Settings chunk!\nOperation terminated.");
					goto EndIt;
					}
				} // if
			} // if

		// viewport
		if (LWInfo->ExportItem <= 1)
			{
			InitViewport3ds(&Viewport3ds);
			if (Viewport3ds && ! ftkerr3ds)
				{
				Viewport3ds->type = CameraView3ds;
				//Viewport3ds->size.xpos = 0;
				//Viewport3ds->size.ypos = 0;
				//Viewport3ds->size.width = 1000;
				//Viewport3ds->size.height = 1000;
				Viewport3ds->ortho.center.x = (float3ds)0.0;
				Viewport3ds->ortho.center.y = (float3ds)0.0;
				Viewport3ds->ortho.center.z = (float3ds)0.0;
				Viewport3ds->ortho.zoom = (float3ds)0.05;		// 500 pixels = 10km
				Viewport3ds->user.center.x = (float3ds)0.0;
				Viewport3ds->user.center.y = (float3ds)0.0;
				Viewport3ds->user.center.z = (float3ds)0.0;
				Viewport3ds->user.zoom = (float3ds)0.05;		// 500 pixels = 10km
				//Viewport3ds->user.horang = (float3ds)20.0;
				//Viewport3ds->user.verang = (float3ds)30.0;
				strncpy(Viewport3ds->camera.name, Rend->Cam->GetName(), 10);
				Viewport3ds->camera.name[10] = 0;
				PutViewport3ds(Database3ds, Viewport3ds);
				ReleaseViewport3ds(&Viewport3ds);
				} // if
			else
				{
				if (Viewport3ds)
					ReleaseViewport3ds(&Viewport3ds);		
				UserMessageOK("3D Studio Scene Export", "Unable to initialize 3DS Viewport Settings!\nOperation terminated.");
				goto EndIt;
				} // if
			if (ftkerr3ds)
				{
				UserMessageOK("3D Studio Scene Export", "Error creating 3DS Viewport Settings chunk!\nOperation terminated.");
				goto EndIt;
				}
			} // if

		// material
		if (LWInfo->ExportItem == 1 || LWInfo->ExportItem == 2)
			{
			InitMaterial3ds(&Material3ds);
			if (Material3ds && ! ftkerr3ds)
				{
				strcpy(Material3ds->name, "WCSDEM");
				Material3ds->ambient.r = (float3ds)(.75);
				Material3ds->ambient.g = (float3ds)(.75);
				Material3ds->ambient.b = (float3ds)(.75);
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
				PutMaterial3ds(Database3ds, Material3ds);
				ReleaseMaterial3ds(&Material3ds);
				} // if
			else
				{
				if (Material3ds)
					ReleaseMaterial3ds(&Material3ds);		
				UserMessageOK("3D Studio Scene Export", "Unable to initialize 3DS Material Settings!\nOperation terminated.");
				goto EndIt;
				} // if
			if (ftkerr3ds)
				{
				UserMessageOK("3D Studio Scene Export", "Error creating 3DS Material Settings chunk!\nOperation terminated.");
				goto EndIt;
				}
			} // if
	
		if (LWInfo->ExportItem != 2)
			{
			InitKfSets3ds(&KFSets3ds);
			if (KFSets3ds && ! ftkerr3ds)
				{
				KFSets3ds->anim.length = (ulong3ds)(.5 + Rend->FrameRate * GlobalApp->MCP->GetMaxFrame() / GlobalApp->MainProj->Interactive->GetFrameRate());
				KFSets3ds->anim.length = max(30, KFSets3ds->anim.length);
				KFSets3ds->anim.curframe = (ulong3ds)(GlobalApp->MainProj->Interactive->GetActiveFrame());
				if (KFSets3ds->anim.curframe >= KFSets3ds->anim.length)
					KFSets3ds->anim.curframe = KFSets3ds->anim.length - 1;
				PutKfSets3ds(Database3ds, KFSets3ds);
				ReleaseKfSets3ds(&KFSets3ds);
				} // if
			else
				{
				if (KFSets3ds)
					ReleaseKfSets3ds(&KFSets3ds);		
				UserMessageOK("3D Studio Scene Export", "Unable to initialize 3DS Keyframe Settings!\nOperation terminated.");
				goto EndIt;
				} // if
			if (ftkerr3ds)
				{
				UserMessageOK("3D Studio Scene Export", "Error creating 3DS Keyframe Settings chunk!\nOperation terminated.");
				goto EndIt;
				} // if
			} // if

		// objects
		// this is the parent dummy object for rotation to horizontal

		InitObjectMotion3ds(&KFMesh3ds, 1, 1, 0, 0, 0);
		if (KFMesh3ds && ! ftkerr3ds)
			{
			strcpy(KFMesh3ds->name, "$$$DUMMY");
			KFMesh3ds->parent[0] = 0;
			strcpy(KFMesh3ds->instance, "Parent");
			KFMesh3ds->flags1 = 0;
			KFMesh3ds->flags2 = KfNodeHasPath3ds;
			KFMesh3ds->npkeys = 1;
			KFMesh3ds->npflag = TrackSingle3ds;
			KFMesh3ds->pkeys[0].time = 0;
			KFMesh3ds->pkeys[0].rflags = 0;
			KFMesh3ds->pos[0].x = (float3ds)0.0;
			KFMesh3ds->pos[0].y = (float3ds)0.0;
			KFMesh3ds->pos[0].z = (float3ds)0.0;
			KFMesh3ds->nrkeys = 1;
			KFMesh3ds->nrflag = TrackSingle3ds;
			KFMesh3ds->rkeys[0].time = 0;
			KFMesh3ds->rkeys[0].rflags = 0;
			KFMesh3ds->rot[0].angle = (float3ds)(0.0);
			KFMesh3ds->rot[0].x = (float3ds)1.0;
			KFMesh3ds->rot[0].y = (float3ds)0.0;
			KFMesh3ds->rot[0].z = (float3ds)0.0;
			KFMesh3ds->boundmin.x = KFMesh3ds->boundmin.y = KFMesh3ds->boundmin.z = (float3ds)(-10.0);
			KFMesh3ds->boundmax.x = KFMesh3ds->boundmax.y = KFMesh3ds->boundmax.z = (float3ds)(10.0);
			PutObjectMotion3ds(Database3ds, KFMesh3ds);
			ReleaseObjectMotion3ds(&KFMesh3ds);
			} // if
		else
			{
			if (KFMesh3ds)
				ReleaseObjectMotion3ds(&KFMesh3ds);		
			UserMessageOK("3D Studio Scene Export", "Unable to initialize 3DS Mesh Object Keyframe!");
			goto EndIt;
			} // else
		if (ftkerr3ds)
			{
			UserMessageOK("3D Studio Scene Export", "Error creating 3DS Mesh Object Keyframe chunk!\nOperation terminated.");
			goto EndIt;
			} // if

		if (LWInfo->ExportItem == 1 || LWInfo->ExportItem == 2)
			{
			DBHost->ResetGeoClip();
			for (MyTurn = DBHost->GetFirst(); MyTurn; MyTurn = DBHost->GetNext(MyTurn))
 				{
				if (MyTurn->TestFlags(WCS_JOEFLAG_ACTIVATED))
					{
					if (MyTurn->TestFlags(WCS_JOEFLAG_ISDEM))
						{
						if (ThreeDSObject_VertexCount(MyTurn, Vertices, Polys,
							Rows, Cols, RowInt, ColInt,
							LWInfo->MaxVerts, LWInfo->MaxPolys, &Topo))
							{
							InitMeshObj3ds(&Mesh3ds, (ushort3ds)Vertices, (ushort3ds)Polys, InitNoExtras3ds);
							if (Mesh3ds && ! ftkerr3ds)
								{
								strncpy(Mesh3ds->name, MyTurn->FileName(), 10);
								Mesh3ds->name[10] = 0;
								TrimTrailingSpaces(Mesh3ds->name);
								Mesh3ds->nvertices = (ushort3ds)Vertices;
								Mesh3ds->nvflags = 0;
								Mesh3ds->ntextverts = 0;
								Mesh3ds->usemapinfo = False3ds;
								Mesh3ds->nfaces = (ushort3ds)Polys;
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
								if (ThreeDSObject_FillData(&Vert,
									Vertices, Polys, -LWInfo->RefLon, LWInfo, Rend,
									Rows, Cols, RowInt, ColInt,
									Mesh3ds->vertexarray, Mesh3ds->facearray, &Topo))
									{
									if (Mesh3ds->matarray = (objmat3ds *)AppMem_Alloc(sizeof (objmat3ds), APPMEM_CLEAR))
										{
										Mesh3ds->nmats = 1;
										strcpy(Mesh3ds->matarray[0].name, "WCSDEM");
										if (Mesh3ds->matarray[0].faceindex = (ushort3ds *)AppMem_Alloc(sizeof (ushort3ds) * Mesh3ds->nfaces, APPMEM_CLEAR))
											{
											Mesh3ds->matarray[0].nfaces = Mesh3ds->nfaces;
											for (i = 0; i < Mesh3ds->matarray[0].nfaces; i ++)
												Mesh3ds->matarray[0].faceindex[i] = (ushort3ds)i;
											} // if
										} // if
									PutMesh3ds(Database3ds, Mesh3ds);
									if (Mesh3ds->matarray[0].faceindex)
										AppMem_Free(Mesh3ds->matarray[0].faceindex, sizeof (ushort3ds) * Mesh3ds->nfaces);
									Mesh3ds->matarray[0].faceindex = NULL;
									Mesh3ds->matarray[0].nfaces = 0;
									if (Mesh3ds->matarray)
										AppMem_Free(Mesh3ds->matarray, sizeof (objmat3ds));
									Mesh3ds->matarray = NULL;
									Mesh3ds->nmats = 0;
									// make key frame for mesh object
									InitObjectMotion3ds(&KFMesh3ds, 1, 0, 0, 0, 0);
									if (KFMesh3ds && ! ftkerr3ds)
										{
										ThreeDSObject_FindBounds(Mesh3ds->nvertices, Mesh3ds->vertexarray, &KFMesh3ds->boundmin, &KFMesh3ds->boundmax);
										strncpy(KFMesh3ds->name, MyTurn->FileName(), 10);
										KFMesh3ds->name[10] = 0;
										TrimTrailingSpaces(KFMesh3ds->name);
										strcpy(KFMesh3ds->parent, "$$$DUMMY");
										strcat(KFMesh3ds->parent, ".Parent");
										KFMesh3ds->flags1 = 0;
										KFMesh3ds->flags2 = KfNodeHasPath3ds;
										KFMesh3ds->npkeys = 1;
										KFMesh3ds->npflag = TrackSingle3ds;
										KFMesh3ds->pkeys[0].time = 0;
										KFMesh3ds->pkeys[0].rflags = 0;
										FindPosVector(Vert.XYZ, Vert.XYZ, LWInfo->RefPt);
										RotatePoint(Vert.XYZ, LWInfo->RotMatx);
										KFMesh3ds->pos[0].x = (float3ds)(Vert.XYZ[0] * LWInfo->UnitScale);
										KFMesh3ds->pos[0].y = (float3ds)(Vert.XYZ[2] * LWInfo->UnitScale);
										KFMesh3ds->pos[0].z = (float3ds)(Vert.XYZ[1] * LWInfo->UnitScale);
										PutObjectMotion3ds(Database3ds, KFMesh3ds);
										ReleaseObjectMotion3ds(&KFMesh3ds);
										} // if
									else
										{
										if (KFMesh3ds)
											ReleaseObjectMotion3ds(&KFMesh3ds);		
										UserMessageOK("3D Studio Scene Export", "Unable to initialize 3DS Mesh Object Keyframe!");
										goto EndIt;
										} // else
									if (ftkerr3ds)
										{
										UserMessageOK("3D Studio Scene Export", "Error creating 3DS Mesh Object Keyframe chunk!\nOperation terminated.");
										goto EndIt;
										} // if
									} // if data filled
								RelMeshObj3ds(&Mesh3ds);
								} // if
							else
								{
								if (Mesh3ds)
									RelMeshObj3ds(&Mesh3ds);		
								UserMessageOK("3D Studio Scene Export", "Unable to initialize 3DS Mesh Object!\nOperation terminated.");
								Topo.FreeRawElevs();
								goto EndIt;
								} // else
							if (ftkerr3ds)
								{
								UserMessageOK("3D Studio Scene Export", "Error creating 3DS Mesh Object chunk!\nOperation terminated.");
								Topo.FreeRawElevs();
								goto EndIt;
								} // if
							Topo.FreeRawElevs();
							} // if vertex count
						} // if
					else if (MyTurn->GetFirstRealPoint() && MyTurn->GetNumRealPoints() > 0)
						{
						InitObjectMotion3ds(&KFMesh3ds, MyTurn->GetNumRealPoints(), 1, 0, 0, 0);
						if (KFMesh3ds && ! ftkerr3ds)
							{
							strcpy(KFMesh3ds->name, "$$$DUMMY");
							strcpy(KFMesh3ds->parent, "$$$DUMMY");
							strcat(KFMesh3ds->parent, ".Parent");
							strncpy(KFMesh3ds->instance, MyTurn->GetBestName(), 10);
							KFMesh3ds->instance[10] = 0;
							TrimTrailingSpaces(KFMesh3ds->instance);
							KFMesh3ds->flags1 = 0;
							KFMesh3ds->flags2 = KfNodeHasPath3ds;
							KFMesh3ds->npkeys = MyTurn->GetNumRealPoints();
							KFMesh3ds->npflag = TrackSingle3ds;

							// identify if there is a coordsys attached to this object
							if (MyAttr = (JoeCoordSys *)MyTurn->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
								MyCoords = MyAttr->Coord;
							else
								MyCoords = NULL;
							for (Point = MyTurn->GetFirstRealPoint(), i = 0; Point; Point = Point->Next, i ++)
								{
								// Convert to Lat/Lon using coordsys
								Point->ProjToDefDeg(MyCoords, &Vert);
								Vert.Elev = (Vert.Elev - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
								Vert.Lon -= LWInfo->RefLon;
								Rend->DefCoords->DegToCart(&Vert);
								FindPosVector(Vert.XYZ, Vert.XYZ, LWInfo->RefPt);
								RotatePoint(Vert.XYZ, LWInfo->RotMatx);

								KFMesh3ds->pkeys[i].time = i;
								KFMesh3ds->pkeys[i].rflags = 0;
								KFMesh3ds->pos[i].x = (float3ds)(Vert.XYZ[0] * LWInfo->UnitScale);
								KFMesh3ds->pos[i].y = (float3ds)(Vert.XYZ[2] * LWInfo->UnitScale);
								KFMesh3ds->pos[i].z = (float3ds)(Vert.XYZ[1] * LWInfo->UnitScale);
								} // for
							KFMesh3ds->nrkeys = 1;
							KFMesh3ds->nrflag = TrackSingle3ds;
							KFMesh3ds->rkeys[0].time = 0;
							KFMesh3ds->rkeys[0].rflags = 0;	
							KFMesh3ds->rot[0].angle = (float3ds)(0.0);
							KFMesh3ds->rot[0].x = (float3ds)1.0;
							KFMesh3ds->rot[0].y = (float3ds)0.0;
							KFMesh3ds->rot[0].z = (float3ds)0.0;
							KFMesh3ds->boundmin.x = KFMesh3ds->boundmin.y = KFMesh3ds->boundmin.z = (float3ds)(-1.0);
							KFMesh3ds->boundmax.x = KFMesh3ds->boundmax.y = KFMesh3ds->boundmax.z = (float3ds)(1.0);
							PutObjectMotion3ds(Database3ds, KFMesh3ds);
							ReleaseObjectMotion3ds(&KFMesh3ds);
							} // if
						else
							{
							if (KFMesh3ds)
								ReleaseObjectMotion3ds(&KFMesh3ds);		
							UserMessageOK("3D Studio Scene Export", "Unable to initialize 3DS Mesh Object Keyframe!");
							goto EndIt;
							} // else
						if (ftkerr3ds)
							{
							UserMessageOK("3D Studio Scene Export", "Error creating 3DS Mesh Object Keyframe chunk!\nOperation terminated.");
							goto EndIt;
							} // if
						} // else if
					} // if
				} // for
			} // if

		// light
		if (LWInfo->ExportItem <= 1)
			{
			for (CurLight = (Light *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_LIGHT); CurLight; CurLight = (Light *)CurLight->Next)
				{
				if (CurLight->Enabled)
					{
					InitSpotlight3ds(&Light3ds);
					if (Light3ds && ! ftkerr3ds)
						{
						strncpy(Light3ds->name, CurLight->GetName(), 10);
						Light3ds->name[10] = 0;
						Vert.Elev = min(5000000.0, CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].CurValue);
						Vert.Lat = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue;
						Vert.Lon = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].CurValue;
						Vert.Lon -= LWInfo->RefLon;
						Rend->DefCoords->DegToCart(&Vert);
						FindPosVector(Vert.XYZ, Vert.XYZ, LWInfo->RefPt);
						RotatePoint(Vert.XYZ, LWInfo->RotMatx);
						Light3ds->pos.x = (float3ds)(Vert.XYZ[0] * LWInfo->UnitScale);
						Light3ds->pos.y = (float3ds)(Vert.XYZ[2] * LWInfo->UnitScale);
						Light3ds->pos.z = (float3ds)(Vert.XYZ[1] * LWInfo->UnitScale);
						Light3ds->color.r = (float3ds)(CurLight->Color.GetCurValue(0));
						Light3ds->color.g = (float3ds)(CurLight->Color.GetCurValue(1));
						Light3ds->color.b = (float3ds)(CurLight->Color.GetCurValue(2));
						Light3ds->multiplier = (float3ds)CurLight->Color.Intensity.GetCurValue(0);
						Light3ds->attenuation.on = False3ds;
						Light3ds->dloff = False3ds;
						Light3ds->spot->target.x = (float3ds)0.0;
						Light3ds->spot->target.y = (float3ds)0.0;
						Light3ds->spot->target.z = (float3ds)0.0;
						Light3ds->spot->cone.overshoot = True3ds;
						PutSpotlight3ds(Database3ds, Light3ds);
						if (! ThreeDSLight_SetKeys(&KFSpot3ds, LWInfo, Rend, CurLight))
							{
							ReleaseLight3ds(&Light3ds);
							UserMessageOK("3D Studio Scene Export", "Unable to initialize 3DS Light Object Keyframes!\nOperation terminated.");
							goto EndIt;
							} // if
						else if (KFSpot3ds)
							{
							strncpy(KFSpot3ds->name, CurLight->GetName(), 10);
							KFSpot3ds->name[10] = 0;
							strcpy(KFSpot3ds->parent, "$$$DUMMY");
							strcat(KFSpot3ds->parent, ".Parent");
							strcpy(KFSpot3ds->tparent, "$$$DUMMY");
							strcat(KFSpot3ds->tparent, ".Parent");
							KFSpot3ds->flags1 = 0;
							KFSpot3ds->flags2 = KfNodeHasPath3ds;
							PutSpotlightMotion3ds(Database3ds, KFSpot3ds);
							ReleaseSpotlightMotion3ds(&KFSpot3ds);
							} // if
						ReleaseLight3ds(&Light3ds);
						} // if
					else
						{
						if (Light3ds)
							ReleaseLight3ds(&Light3ds);		
						UserMessageOK("3D Studio Scene Export", "Unable to initialize 3DS Light Object!\nOperation terminated.");
						goto EndIt;
						} // else
					if (ftkerr3ds)
						{
						UserMessageOK("3D Studio Scene Export", "Error creating 3DS Light Object chunk!\nOperation terminated.");
						goto EndIt;
						}
					} // if
				} // for
			} // if

		// camera
		if (LWInfo->ExportItem != 2)
			{
			Rend->Cam->InitToRender(Rend->Opt, NULL);
			Rend->Cam->InitFrameToRender(EffectsHost, Rend);
			InitCamera3ds(&Camera3ds);
			if (Camera3ds && ! ftkerr3ds)
				{
				strncpy(Camera3ds->name, Rend->Cam->GetName(), 10);
				Camera3ds->name[10] = 0;
				Vert.Elev = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue;
				Vert.Lat = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue;
				Vert.Lon = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue;
				Vert.Lon -= LWInfo->RefLon;
				Rend->DefCoords->DegToCart(&Vert);
				FindPosVector(Vert.XYZ, Vert.XYZ, LWInfo->RefPt);
				RotatePoint(Vert.XYZ, LWInfo->RotMatx);
				Camera3ds->position.x = (float3ds)(Vert.XYZ[0] * LWInfo->UnitScale);
				Camera3ds->position.y = (float3ds)(Vert.XYZ[2] * LWInfo->UnitScale);
				Camera3ds->position.z = (float3ds)(Vert.XYZ[1] * LWInfo->UnitScale);
				Vert.CopyXYZ(Rend->Cam->TargPos);
				Vert.MultiplyXYZ(100.0);	// this will create a pseudo target 100m from camera
				Vert.AddXYZ(Rend->Cam->CamPos);
				Vert.RotateY(-LWInfo->RefLon);
				FindPosVector(Vert.XYZ, Vert.XYZ, LWInfo->RefPt);
				RotatePoint(Vert.XYZ, LWInfo->RotMatx);
				Camera3ds->target.x = (float3ds)(Vert.XYZ[0] * LWInfo->UnitScale);
				Camera3ds->target.y = (float3ds)(Vert.XYZ[2] * LWInfo->UnitScale);
				Camera3ds->target.z = (float3ds)(Vert.XYZ[1] * LWInfo->UnitScale);

				FP.CopyXYZ(Rend->Cam->TargPos);
				CV.CopyXYZ(Rend->Cam->CamVertical);
				FP.RotateY(-LWInfo->RefLon);
				CV.RotateY(-LWInfo->RefLon);
				RotatePoint(FP.XYZ, LWInfo->RotMatx);
				RotatePoint(CV.XYZ, LWInfo->RotMatx);

				Heading = FP.FindAngleYfromZ();
				FP.RotateY(-Heading);
				Pitch = FP.FindAngleXfromZ();
				CV.RotateY(-Heading);
				CV.RotateX(-Pitch);
				Bank = CV.FindAngleZfromY();

				if (fabs(Bank) >= 360.0)
					Bank = fmod(Bank, 360.0);	// retains the original sign
				if (Bank < 0.0)
					Bank += 360.0;
				Camera3ds->roll = (float3ds)(Bank);
				ViewAngle = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue;
				if (ViewAngle < .005)
					ViewAngle = .005;
				if (ViewAngle > 175.0)
					ViewAngle = 175.0;
				Camera3ds->fov = (float3ds)ViewAngle;
				Camera3ds->showcone = True3ds;
				Camera3ds->ranges.cam_near = (float3ds)(0.0);		// these two are the default values until I learn what they do.
				Camera3ds->ranges.cam_far = (float3ds)(100000.0);
				PutCamera3ds(Database3ds, Camera3ds);
				if (! ThreeDSCamera_SetKeys(&KFCamera3ds, LWInfo, Rend))
					{
					ReleaseCamera3ds(&Camera3ds);
					UserMessageOK("3D Studio Scene Export", "Unable to initialize 3DS Camera Object Keyframes!\nOperation terminated.");
					goto EndIt;
					} // if
				else if (KFCamera3ds)
					{
					strncpy(KFCamera3ds->name, Rend->Cam->GetName(), 10);
					KFCamera3ds->name[10] = 0;
					strcpy(KFCamera3ds->parent, "$$$DUMMY");
					strcat(KFCamera3ds->parent, ".Parent");
					strcpy(KFCamera3ds->tparent, "$$$DUMMY");
					strcat(KFCamera3ds->tparent, ".Parent");
					KFCamera3ds->flags1 = 0;
					KFCamera3ds->flags2 = KfNodeHasPath3ds;
					PutCameraMotion3ds(Database3ds, KFCamera3ds);
					ReleaseCameraMotion3ds(&KFCamera3ds);
					} // if
				ReleaseCamera3ds(&Camera3ds);
				} // if
			else
				{
				if (Camera3ds)
					ReleaseCamera3ds(&Camera3ds);
				UserMessageOK("3D Studio Scene Export", "Unable to initialize 3DS Camera Object!\nOperation terminated.");
				goto EndIt;
				} // else
			if (ftkerr3ds)
				{
				UserMessageOK("3D Studio Scene Export", "Error creating 3DS Camera Object chunk!\nOperation terminated.");
				goto EndIt;
				}
/*
			if (! ThreeDSCamTarget_SetKeys(&KFMesh3ds, LWInfo, Rend))
				{
				UserMessageOK("3D Studio Scene Export", "Unable to initialize 3DS Camera Target Object Keyframes!\nOperation terminated.");
				goto EndIt;
				} // if
			if (KFMesh3ds && ! ftkerr3ds)
				{
				strcpy(KFMesh3ds->name, "$$$DUMMY");
				strcpy(KFMesh3ds->parent, "$$$DUMMY");
				strcat(KFMesh3ds->parent, ".Parent");
				strcpy(KFMesh3ds->instance, "Target01");
				KFMesh3ds->flags1 = 0;
				KFMesh3ds->flags2 = KfNodeHasPath3ds;
				PutObjectMotion3ds(Database3ds, KFMesh3ds);
				ReleaseObjectMotion3ds(&KFMesh3ds);
				} // if
			else
				{
				if (KFMesh3ds)
					ReleaseObjectMotion3ds(&KFMesh3ds);		
				UserMessageOK("3D Studio Scene Export", "Unable to initialize 3DS Camera Target Object Keyframe!");
				goto EndIt;
				} // else
			if (ftkerr3ds)
				{
				UserMessageOK("3D Studio Scene Export", "Error creating 3DS Camera Target Object Keyframe chunk!\nOperation terminated.");
				goto EndIt;
				} // if
*/
			} // if

		// save it!
		if (File3ds = OpenFile3ds((const char3ds *)filename, "w"))
			{
			WriteDatabase3ds(File3ds, Database3ds);
			if (ftkerr3ds)
				{
				UserMessageOK("3D Studio Scene Export", "Error saving 3DS Database to file!\nOperation terminated.");
				} // if
			CloseAllFiles3ds();

//			strcpy(Name, "WCSTest.txt");
//			strmfp(filename, Path, Name);
//			if (DumpFile = fopen (filename, "w"))
//				{
//				DumpDatabase3ds(DumpFile, Database3ds);
//				fclose(DumpFile);
//				} // if
			} // if
		else
			{
			UserMessageOK("3D Studio Scene Export", "Error opening 3DS Database file for output!\nOperation terminated.");
			} // else
		} // if topchunk created
	else
		{
		UserMessageOK("3D Studio Scene Export", "Unable to initialize 3DS Database Top Chunk!\nOperation terminated.");
		} // if
EndIt:
	ReleaseDatabase3ds(&Database3ds);
	} // if
else
	{
	if (Database3ds)
		ReleaseDatabase3ds(&Database3ds);
	UserMessageOK("3D Studio Scene Export", "Unable to initialize 3DS Database!\nOperation terminated.");
	} // else

} // SceneExportGUI::ThreeDSScene_Export

/*===========================================================================*/

int SceneExportGUI::ThreeDSObject_VertexCount(Joe *JoeObj, long &Vertices, long &Polys,
	long &LWRows, long &LWCols, long &LWRowInt, long &LWColInt,
	long MaxVertices, long MaxPolys, DEM *Topo)
{
int Done = 0;
char filename[256], DEMPath[256], DEMName[64], Ptrn[32];
//long Lr, Lc, LWr, LWc, SamplePt, RejectVert;

// select DEM for export
if (! JoeObj)	// note this will never happen, the name is always passed
	{
	strcpy(Ptrn, WCS_REQUESTER_PARTIALWILD("elev"));
	strcpy(DEMPath, ProjHost->dirname);
	DEMName[0] = 0;
	if (! GetFileNamePtrn(0, "DEM file to export", DEMPath, DEMName, Ptrn, 64))
		{
		return (0);
		} // if 
	strmfp(filename, DEMPath, DEMName);
	if (!Topo->LoadDEM(filename, 0, NULL))
		{
		UserMessageOK(DEMName,
			"Error loading DEM Object!\nOperation terminated.");
		return (0);
		} // if open/read fails 
	} // if no object name supplied 
else
	{
	strcpy(DEMName, JoeObj->FileName());
	if (! Topo->AttemptLoadDEM(JoeObj, 0, ProjHost))
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_WNG_OPEN_FAIL, DEMName);
		UserMessageOK("DEMName",
			"Error loading DEM Object!\nObject not saved.");
		return (0);
		} // if 
	} // else find the object and read it 

// I believe TransferToVerticesLatLon is safe to do in non-Coordinate-System-aware builds
// Ensure that the point remains within the DEM after whatever precision errors occur during coord transforms
// so that VertexDataPoint will find a valid elevation
Topo->TransferToVerticesLatLon(TRUE);

// prepare object for export 
LWRows = Topo->LonEntries();
LWCols = Topo->LatEntries();
Vertices = LWRows * LWCols;
Polys = (LWRows - 1) * (LWCols - 1) * 2;
LWColInt = LWRowInt = 1;

if (Vertices > 32768 || Vertices >= MaxVertices || Polys >= MaxPolys)
	Done = 0;
else
	Done = 1;

while (! Done && LWRows > 3 && LWCols > 3)
	{
	if (LWRows > LWCols)
		{
		LWRows = LWRows / 2 + LWRows % 2;
		LWRowInt *= 2;
		} // if 
	else
		{
		LWCols = LWCols / 2 + LWCols % 2;
		LWColInt *= 2;
		} // else  
	Vertices = LWRows * LWCols;
	Polys = (LWRows - 1) * (LWCols - 1) * 2;
	if (Vertices < 32768 && Vertices <= MaxVertices && Polys <= MaxPolys)
		Done = 1;
	} // while need to decimate DEM points 

// elliminate NULL vertices and associated polygons
/*
#ifdef WCS_BUILD_VNS
for (LWr = Lr = 0; LWr < LWRows; Lr += LWRowInt, LWr ++)
	{
	for (LWc = Lc = 0; LWc < LWCols; Lc += LWColInt, LWc ++)
		{
		SamplePt = Lr * Topo->LatEntries() + Lc;
		RejectVert = 0;
		if (Topo->TestNullReject(SamplePt))
			{
			RejectVert = 1;
			Vertices --;
			} // if
		if (LWr < LWRows - 1 && LWc < LWCols - 1)
			{
			if (RejectVert || Topo->TestNullReject(SamplePt + LWColInt) || Topo->TestNullReject(SamplePt + LWRowInt * Topo->LatEntries()))
				{
				Polys -= 2;
				} // if
			else if (Topo->TestNullReject(SamplePt + LWRowInt * Topo->LatEntries() + LWColInt))
				{
				Polys --;
				} // if
			} // if 
		} // for LWc=0... 
	} // for LWr=0... 
#endif // WCS_BUILD_VNS
*/

return (Vertices > 2 && Polys > 0);

} // SceneExportGUI::ThreeDSObject_VertexCount

/*===========================================================================*/

int SceneExportGUI::ThreeDSCamTarget_VertexCount(long &Vertices, long &Polys)
{

Vertices = 3;
Polys = 1;

return (1);

} // SceneExportGUI::ThreeDSCamTarget_VertexCount

/*===========================================================================*/

int SceneExportGUI::ThreeDSObject_FillData(VertexDEM *PP,
	long Vertices, long Polys, double LonRot, ImportInfo *LWInfo, RenderData *Rend,
	long LWRows, long LWCols, long LWRowInt, long LWColInt,
	point3ds *VtxData, face3ds *PolyData, DEM *Topo)
{
#ifdef WCS_VIEW_TERRAFFECTORS
VertexData VTD;
PolygonData PGD;
#endif // WCS_VIEW_TERRAFFECTORS
//int Done = 0;
//double NClamp, SClamp, EClamp, WClamp;
long zip, pzip, LWr, LWc, Lr, Lc, CurVtx, SamplePt;
double ptelev, PivotPt[3], DefSeaLevel;
VertexDEM Vert;
LakeEffect *DefOcean;
CoordSys *MyCoords;
JoeCoordSys *MyAttr;

#ifdef WCS_VIEW_TERRAFFECTORS
// Setup for terraffector eval
//Rend.PlanetRad = GlobalApp->AppEffects->GetPlanetRadius();
//Rend.EarthLatScaleMeters = LatScale(Rend.PlanetRad);
//Rend.EffectsBase = GlobalApp->AppEffects;
//Rend.ElevDatum = VC->Planet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue;
//Rend.Exageration = VC->Planet->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].CurValue;
//Rend.DefCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
Rend->TexRefLon = Rend->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X);
Rend->TexRefLat = Rend->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y);
Rend->TexRefElev = Rend->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Z);
Rend->RefLonScaleMeters = Rend->EarthLatScaleMeters * cos(Rend->TexRefLat * PiOver180);
Rend->TexData.MetersPerDegLat = Rend->EarthLatScaleMeters;
Rend->TexData.Datum = Rend->ElevDatum;
Rend->TexData.Exageration = Rend->Exageration;
Rend->ExagerateElevLines = ((PlanetOpt *)Rend->EffectsBase->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, NULL))->EcoExageration;
//Rend.DBase = LocalDB;
#endif // WCS_VIEW_TERRAFFECTORS

if (Topo->MoJoe && (MyAttr = (JoeCoordSys *)Topo->MoJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) )
	MyCoords = MyAttr->Coord;
else
	MyCoords = NULL;

if (DefOcean = (LakeEffect *)EffectsHost->GetDefaultEffect(WCS_EFFECTSSUBCLASS_LAKE, 0, NULL))
	{
	DefSeaLevel = DefOcean->AnimPar[WCS_EFFECTS_LAKE_ANIMPAR_ELEV].CurValue;
	if (Rend->ExagerateElevLines)
		DefSeaLevel = (DefSeaLevel - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
	} // if
else
	DefSeaLevel = - FLT_MAX;

// if the first point is a NULL value continue to look for another point
SamplePt = 0;
#ifdef WCS_BUILD_VNS
for (LWr = Lr = 0; LWr < LWRows; Lr += LWRowInt, LWr ++)
	{
	SamplePt = Lr * Topo->LatEntries();
	for (LWc = Lc = 0; LWc < LWCols; Lc += LWColInt, LWc ++, SamplePt += LWColInt)
		{
		if (! Topo->TestNullReject(SamplePt))
			break;
		} // for
	} // for
#endif // WCS_BUILD_VNS
ptelev = Topo->SampleRaw(SamplePt) * Topo->ElScale() / ELSCALE_METERS;
ptelev = (ptelev - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
if (! LWInfo->Bathymetry && ptelev <= DefSeaLevel)
	ptelev = DefSeaLevel;
Vert.xyz[2] = Vert.Elev = ptelev;
Vert.xyz[1] = Vert.Lat = Topo->Southest();
Vert.xyz[0] = Vert.Lon = Topo->Westest();
if (MyCoords)
	MyCoords->ProjToDefDeg(&Vert);
Vert.Lon += LonRot;
Rend->DefCoords->DegToCart(&Vert);
PivotPt[0] = Vert.XYZ[0];
PivotPt[1] = Vert.XYZ[1];
PivotPt[2] = Vert.XYZ[2];
if (PP)
	{
	PP->CopyXYZ(&Vert);
	} // if VertexDEM provided

// precision errors cause FindAndLoadDEM to flush too often
// This problem is believed fixed by ensuring the vertices fall within the DEM bounds in TransferToVerticesLatLon()
//NClamp = Topo->MoJoe->NWLat;
//SClamp = Topo->MoJoe->SELat;
//EClamp = Topo->MoJoe->SELon;
//WClamp = Topo->MoJoe->NWLon;

for (zip = pzip = LWr = Lr = 0; LWr < LWRows; Lr += LWRowInt, LWr ++)
	{
	SamplePt = Lr * Topo->LatEntries();
	for (LWc = Lc = 0; LWc < LWCols; Lc += LWColInt, LWc ++, zip ++, SamplePt += LWColInt)
		{
		/*
		#ifdef WCS_BUILD_VNS
		SamplePt = Lr * Topo->LatEntries() + Lc;
		if (Topo->TestNullReject(SamplePt))
			continue;
		#endif // WCS_BUILD_VNS
		*/
		// these have been forced to be in bounds during Topo->TransferToVerticesLatLon()
		Vert.xyz[1] = Vert.Lat = Topo->Vertices[SamplePt].Lat;
		Vert.xyz[0] = Vert.Lon = Topo->Vertices[SamplePt].Lon;
		//Vert.xyz[1] = Vert.Lat = Topo->Southest() + Lc * Topo->LatStep();
		//Vert.xyz[0] = Vert.Lon = Topo->Westest() - Lr * Topo->LonStep();
		//if (MyCoords)
		//	MyCoords->ProjToDefDeg(&Vert);

		#ifdef WCS_VIEW_TERRAFFECTORS
		VTD.Lat = Vert.Lat;
		VTD.Lon = Vert.Lon;
		//if (VTD.Lat < SClamp)
		//	VTD.Lat = SClamp;
		//if (VTD.Lat > NClamp)
		//	VTD.Lat = NClamp;
		//if (VTD.Lon < EClamp)
		//	VTD.Lon = EClamp;
		//if (VTD.Lon > WClamp)
		//	VTD.Lon = WClamp;
		//if (MyCoords)
		//	MyCoords->ProjToDefDeg(&VTD);
		GlobalApp->MainProj->Interactive->VertexDataPoint(Rend, &VTD, &PGD,
		 WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED);
		if (VTD.Elev > 1000000.0) // sanity check
			{
			VTD.Elev = 0;
			} // if
		ptelev = VTD.Elev;
		#else // !WCS_VIEW_TERRAFFECTORS
		ptelev = Topo->Sample(Lr, Lc) * Topo->ElScale() / ELSCALE_METERS;
		ptelev = (ptelev - Rend->ElevDatum) * Rend->Exageration + Rend->ElevDatum;
		#endif // !WCS_VIEW_TERRAFFECTORS

		Vert.Lon += LonRot;

		if (! LWInfo->Bathymetry && ptelev <= DefSeaLevel)
			ptelev = DefSeaLevel;
		Vert.Elev = ptelev;
		Rend->DefCoords->DegToCart(&Vert);

		FindPosVector(Vert.XYZ, Vert.XYZ, PivotPt);
		RotatePoint(Vert.XYZ, LWInfo->RotMatx);

		VtxData[zip].x = (float3ds)(Vert.XYZ[0] * LWInfo->UnitScale);
		VtxData[zip].y = (float3ds)(Vert.XYZ[2] * LWInfo->UnitScale);
		VtxData[zip].z = (float3ds)(Vert.XYZ[1] * LWInfo->UnitScale);

		if (LWr < LWRows - 1 && LWc < LWCols - 1)
			{
			CurVtx = LWr * LWCols + LWc;
			/*
			#ifdef WCS_BUILD_VNS
			if (! (Topo->TestNullReject(SamplePt + LWColInt) || Topo->TestNullReject(SamplePt + LWRowInt * Topo->LatEntries())))
				{
			#endif // WCS_BUILD_VNS
			*/
				// we're trying to get the surface normals to point upward so reverse the vertex order
				PolyData[pzip].v1 = (ushort3ds)(CurVtx);
				PolyData[pzip].v2 = (ushort3ds)(CurVtx + LWCols);
				PolyData[pzip].v3 = (ushort3ds)(CurVtx + 1);
				//PolyData[pzip].v1 = (ushort3ds)(CurVtx);
				//PolyData[pzip].v2 = (ushort3ds)(CurVtx + 1);
				//PolyData[pzip].v3 = (ushort3ds)(CurVtx + LWCols);
				PolyData[pzip].flag = 0x0007;	//0x0005;
				pzip ++;
			/*
			#ifdef WCS_BUILD_VNS
				if (! (Topo->TestNullReject(SamplePt + LWRowInt * Topo->LatEntries() + LWColInt)))
					{
			#endif // WCS_BUILD_VNS
			*/
					PolyData[pzip].v1 = (ushort3ds)(CurVtx + 1);
					PolyData[pzip].v2 = (ushort3ds)(CurVtx + LWCols);
					PolyData[pzip].v3 = (ushort3ds)(CurVtx + 1 + LWCols);
					//PolyData[pzip].v1 = (ushort3ds)(CurVtx + 1);
					//PolyData[pzip].v2 = (ushort3ds)(CurVtx + 1 + LWCols);
					//PolyData[pzip].v3 = (ushort3ds)(CurVtx + LWCols);
					PolyData[pzip].flag = 0x0007;	//0x0003;
					pzip ++;
			/*
			#ifdef WCS_BUILD_VNS
					} // if
				} // if
			#endif // WCS_BUILD_VNS
			*/
			} // if 
		} // for LWc=0... 
	} // for LWr=0... 

return (1);

} // SceneExportGUI::ThreeDSObject_FillData() 

/*===========================================================================*/

int SceneExportGUI::ThreeDSCamTarget_FillData(point3ds *VertexData, face3ds *PolyData)
{

if (VertexData && PolyData)
	{
	VertexData[0].x = (float3ds)(0.0);
	VertexData[0].y = (float3ds)(0.0);
	VertexData[0].z = (float3ds)(0.0);
	VertexData[1].x = (float3ds)(0.0);
	VertexData[1].y = (float3ds)(0.0);
	VertexData[1].z = (float3ds)(0.0);
	VertexData[2].x = (float3ds)(0.0);
	VertexData[2].y = (float3ds)(0.0);
	VertexData[2].z = (float3ds)(0.0);

	PolyData[0].v1 = (ushort3ds)(0);
	PolyData[0].v2 = (ushort3ds)(1);
	PolyData[0].v3 = (ushort3ds)(2);
	PolyData[0].flag = 0x0007;	//0x0005;

	return (1);
	} // if

return (0);

} // SceneExportGUI::ThreeDSCamTarget_FillData() 

/*===========================================================================*/

void SceneExportGUI::ThreeDSObject_FindBounds(ushort3ds nvertices, point3ds *vertexarray, point3ds *boundmin, point3ds *boundmax)
{
ushort3ds Point;

boundmin->x = boundmin->y = (float3ds)(10000000.0);
boundmin->z = (float3ds)(10000000.0);
boundmax->x = boundmax->y = (float3ds)(-10000000.0);
boundmax->z = (float3ds)(-10000000.0);

for (Point = 0; Point < nvertices; Point ++)
	{
	if (vertexarray[Point].x < boundmin->x)
		boundmin->x = vertexarray[Point].x;
	if (vertexarray[Point].y < boundmin->y)
		boundmin->y = vertexarray[Point].y;
	if (vertexarray[Point].z < boundmin->z)
		boundmin->z = vertexarray[Point].z;

	if (vertexarray[Point].x > boundmax->x)
		boundmax->x = vertexarray[Point].x;
	if (vertexarray[Point].y > boundmax->y)
		boundmax->y = vertexarray[Point].y;
	if (vertexarray[Point].z > boundmax->z)
		boundmax->z = vertexarray[Point].z;
	} // for

} // SceneExportGUI::ThreeDSObject_FindBounds

/*===========================================================================*/

int SceneExportGUI::ThreeDSLight_SetKeys(kfspot3ds **KFSpot3dsPtr, ImportInfo *LWInfo, RenderData *Rend, Light *CurLight)
{
VertexDEM Vert;
kfspot3ds *KFSpot3ds = NULL;
GraphNode *GrNode;
RasterAnimHostProperties Prop;
int j, KeyFrames, KeysExist, LastKeyFrame, NextKeyFrame, success = 1;

if (KeysExist = (CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetNumNodes(0) > 0
	|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].GetNumNodes(0) > 0
	|| CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].GetNumNodes(0) > 0))
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
	Prop.ItemOperator = WCS_KEYOPERATION_CUROBJGROUP;
	LastKeyFrame = -2;
	KeyFrames = 0;

	while (1)	//lint !e716
		{
		NextKeyFrame = -1;
		Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
		Prop.NewKeyNodeRange[0] = -DBL_MAX;
		Prop.NewKeyNodeRange[1] = DBL_MAX;
		CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetRAHostProperties(&Prop);
		if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
			NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
		if (NextKeyFrame > LastKeyFrame)
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

InitSpotlightMotion3ds(KFSpot3dsPtr, KeyFrames, 1, 1, 1, 1, 1);
if ((KFSpot3ds = *KFSpot3dsPtr) && ! ftkerr3ds)
	{
	if (KeysExist)
		{
		j = 0;
		LastKeyFrame = -2;
		while (j < KeyFrames)
			{
			NextKeyFrame = -1;
			Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
			Prop.NewKeyNodeRange[0] = -DBL_MAX;
			Prop.NewKeyNodeRange[1] = DBL_MAX;
			CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].GetRAHostProperties(&Prop);
			if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
				NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
			if (NextKeyFrame > LastKeyFrame)
				{
				if (GrNode = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].FindNearestSiblingNode(NextKeyFrame / Rend->FrameRate, NULL))
					{
					CurLight->SetToTime(NextKeyFrame / Rend->FrameRate);
					Vert.Lat = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue;
					Vert.Lon = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].CurValue;
					Vert.Elev = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].CurValue;
					Vert.Lon -= LWInfo->RefLon;
					Rend->DefCoords->DegToCart(&Vert);
					FindPosVector(Vert.XYZ, Vert.XYZ, LWInfo->RefPt);
					RotatePoint(Vert.XYZ, LWInfo->RotMatx);
					KFSpot3ds->pos[j].x = (float3ds)(Vert.XYZ[0] * LWInfo->UnitScale);
					KFSpot3ds->pos[j].y = (float3ds)(Vert.XYZ[2] * LWInfo->UnitScale);
					KFSpot3ds->pos[j].z = (float3ds)(Vert.XYZ[1] * LWInfo->UnitScale);
					KFSpot3ds->pkeys[j].time = (ulong3ds)NextKeyFrame;
					KFSpot3ds->pkeys[j].rflags = (GrNode->TCB[0] != 0.0) | ((GrNode->TCB[1] != 0.0) << 1) | ((GrNode->TCB[2] != 0.0) << 2);
					KFSpot3ds->pkeys[j].tension = (float3ds)GrNode->TCB[0];
					KFSpot3ds->pkeys[j].continuity = (float3ds)GrNode->TCB[1];
					KFSpot3ds->pkeys[j].bias = (float3ds)GrNode->TCB[2];
					j ++;

					LastKeyFrame = NextKeyFrame;
					} // if
				else
					break;
				} // if
			else
				break;
			} // while
		} // if KeysExist
	else
		{
		Vert.Lat = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue;
		Vert.Lon = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].CurValue;
		Vert.Elev = CurLight->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_ELEV].CurValue;
		Vert.Lon -= LWInfo->RefLon;
		Rend->DefCoords->DegToCart(&Vert);
		FindPosVector(Vert.XYZ, Vert.XYZ, LWInfo->RefPt);
		RotatePoint(Vert.XYZ, LWInfo->RotMatx);
		KFSpot3ds->pos[0].x = (float3ds)(Vert.XYZ[0] * LWInfo->UnitScale);
		KFSpot3ds->pos[0].y = (float3ds)(Vert.XYZ[2] * LWInfo->UnitScale);
		KFSpot3ds->pos[0].z = (float3ds)(Vert.XYZ[1] * LWInfo->UnitScale);
		KFSpot3ds->pkeys[0].time = (ulong3ds)0;
		KFSpot3ds->pkeys[0].rflags = 0;
		} // if ! KeysExist
	KFSpot3ds->tpos[0].x = (float3ds)0.0;
	KFSpot3ds->tpos[0].y = (float3ds)0.0;
	KFSpot3ds->tpos[0].z = (float3ds)0.0;
	KFSpot3ds->color[0].r = (float3ds)(CurLight->Color.GetClampedCompleteValue(0));
	KFSpot3ds->color[0].g = (float3ds)(CurLight->Color.GetClampedCompleteValue(1));
	KFSpot3ds->color[0].b = (float3ds)(CurLight->Color.GetClampedCompleteValue(2));
	KFSpot3ds->hot[0] = (float3ds)44.0;
	KFSpot3ds->fall[0] = (float3ds)45.0;
	KFSpot3ds->roll[0] = (float3ds)0.0;
	KFSpot3ds->tkeys[0].time = (ulong3ds)0;
	KFSpot3ds->rkeys[0].time = (ulong3ds)0;
	KFSpot3ds->ckeys[0].time = (ulong3ds)0;
	KFSpot3ds->hkeys[0].time = (ulong3ds)0;
	KFSpot3ds->fkeys[0].time = (ulong3ds)0;
	} // if light key frames
else
	{
	if (KFSpot3ds)
		ReleaseSpotlightMotion3ds(KFSpot3dsPtr);
	success = 0;
	} // else if

return (success);

} // SceneExportGUI::ThreeDSLight_SetKeys

/*===========================================================================*/

int SceneExportGUI::ThreeDSCamera_SetKeys(kfcamera3ds **KFCamera3dsPtr, ImportInfo *LWInfo, RenderData *Rend)
{
long MaxFrames, TotalKeys, CameraKeys = 0, BankKeys, FovKeys, LastKeyFrame, NextKeyFrame, i, j, success = 1;
ThreeDSMotion *LWM = NULL;
kfcamera3ds *KFCamera3ds = NULL;
GraphNode *GrNode;
RasterAnimHostProperties Prop;

// determine if key frames exist for the camera motion path 

TotalKeys = 0;
LastKeyFrame = -2;
while ((LastKeyFrame = Rend->Cam->GetNextMotionKeyFrame(LastKeyFrame, Rend->FrameRate)) >= 0)
	CameraKeys ++;
BankKeys = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].GetNumNodes(0);
FovKeys = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].GetNumNodes(0);

TotalKeys = CameraKeys + BankKeys + FovKeys;
if (TotalKeys < 1)
	{
	return (1);
	} // no motion key frames 

// allocate some essential resources 
Rend->Cam->InitToRender(Rend->Opt, NULL);


if ((LWM = new ThreeDSMotion()) == NULL)
	{
	success = 0;
	goto EndWave;
	} // if out of memory 

// different procedures if key frame interval is 0 - output only at camera, focus or bank key frames or at frame 0 if no keys
if (LWInfo->KeyFrameInt <= 0)
	{
	InitCameraMotion3ds(KFCamera3dsPtr, CameraKeys > 0 ? CameraKeys: 1, FovKeys > 0 ? FovKeys: 1, BankKeys > 0 ? BankKeys: 1, CameraKeys > 0 ? CameraKeys: 1);
	if ((KFCamera3ds = *KFCamera3dsPtr) && ! ftkerr3ds)
		{
		if (CameraKeys)
			{
			LastKeyFrame = -2;
			j = 0;
			while (((LastKeyFrame = Rend->Cam->GetNextMotionKeyFrame(LastKeyFrame, Rend->FrameRate)) >= 0) && j < CameraKeys)
				{
				LWM->Frame = LastKeyFrame;
				// init camera to this frame position
				if (Rend->Cam->TargetObj)
					Rend->Cam->TargetObj->SetToTime(LWM->Frame / Rend->FrameRate);
				Rend->Cam->SetToTime(LWM->Frame / Rend->FrameRate);
				Rend->Cam->InitFrameToRender(EffectsHost, Rend);
				// determine HPB
				Set_3DS(LWM, LWInfo, LWM->Frame, Rend);
				// determine TCB - we'll need the nearest node for that
				if (GrNode = Rend->Cam->GetNearestMotionNode(LWM->Frame / Rend->FrameRate))
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
				KFCamera3ds->pos[j].x = (float3ds)(LWM->CamXYZ[0] * LWInfo->UnitScale);
				KFCamera3ds->pos[j].y = (float3ds)(LWM->CamXYZ[2] * LWInfo->UnitScale);
				KFCamera3ds->pos[j].z = (float3ds)(LWM->CamXYZ[1] * LWInfo->UnitScale);
				KFCamera3ds->pkeys[j].time = (ulong3ds)LWM->Frame;
				KFCamera3ds->pkeys[j].rflags = (LWM->TCB[0] != 0.0) | ((LWM->TCB[1] != 0.0) << 1) | ((LWM->TCB[2] != 0.0) << 2);
				KFCamera3ds->pkeys[j].tension = (float3ds)LWM->TCB[0];
				KFCamera3ds->pkeys[j].continuity = (float3ds)LWM->TCB[1];
				KFCamera3ds->pkeys[j].bias = (float3ds)LWM->TCB[2];

				KFCamera3ds->tpos[j].x = (float3ds)(LWM->TargetXYZ[0] * LWInfo->UnitScale);
				KFCamera3ds->tpos[j].y = (float3ds)(LWM->TargetXYZ[2] * LWInfo->UnitScale);
				KFCamera3ds->tpos[j].z = (float3ds)(LWM->TargetXYZ[1] * LWInfo->UnitScale);
				KFCamera3ds->tkeys[j].time = (ulong3ds)LWM->Frame;
				KFCamera3ds->tkeys[j].rflags = (LWM->TCB[0] != 0.0) | ((LWM->TCB[1] != 0.0) << 1) | ((LWM->TCB[2] != 0.0) << 2);
				KFCamera3ds->tkeys[j].tension = (float3ds)LWM->TCB[0];
				KFCamera3ds->tkeys[j].continuity = (float3ds)LWM->TCB[1];
				KFCamera3ds->tkeys[j].bias = (float3ds)LWM->TCB[2];
				j ++;
				} // while
			} // if camera motion keys
		else
			{
			LWM->Frame = 0;
			Set_3DS(LWM, LWInfo, LWM->Frame, Rend);
			KFCamera3ds->pos[0].x = (float3ds)(LWM->CamXYZ[0] * LWInfo->UnitScale);
			KFCamera3ds->pos[0].y = (float3ds)(LWM->CamXYZ[2] * LWInfo->UnitScale);
			KFCamera3ds->pos[0].z = (float3ds)(LWM->CamXYZ[1] * LWInfo->UnitScale);
			KFCamera3ds->pkeys[0].time = (ulong3ds)LWM->Frame;
			KFCamera3ds->pkeys[0].rflags = 0;

			KFCamera3ds->tpos[0].x = (float3ds)(LWM->TargetXYZ[0] * LWInfo->UnitScale);
			KFCamera3ds->tpos[0].y = (float3ds)(LWM->TargetXYZ[2] * LWInfo->UnitScale);
			KFCamera3ds->tpos[0].z = (float3ds)(LWM->TargetXYZ[1] * LWInfo->UnitScale);
			KFCamera3ds->tkeys[0].time = (ulong3ds)LWM->Frame;
			KFCamera3ds->tkeys[0].rflags = 0;
			} // else 

		if (BankKeys)
			{
			Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
			Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
			j = 0;
			LastKeyFrame = -2;
			while (j < BankKeys)
				{
				NextKeyFrame = -1;
				Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
				Prop.NewKeyNodeRange[0] = -DBL_MAX;
				Prop.NewKeyNodeRange[1] = DBL_MAX;
				Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].GetRAHostProperties(&Prop);
				if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
					NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
				if (NextKeyFrame > LastKeyFrame)
					{
					if (GrNode = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].FindNearestSiblingNode(NextKeyFrame / Rend->FrameRate, NULL))
						{
						Rend->Cam->SetToTime(NextKeyFrame / Rend->FrameRate);
						LWM->Frame = NextKeyFrame;
						Set_3DS(LWM, LWInfo, LWM->Frame, Rend);
						LWM->TCB[0] = GrNode->TCB[0];
						LWM->TCB[1] = GrNode->TCB[1];
						LWM->TCB[2] = GrNode->TCB[2];
						KFCamera3ds->roll[j] = (float3ds)(LWM->Bank);
						KFCamera3ds->rkeys[j].time = (ulong3ds)LWM->Frame;
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
			} // if KeysExist
		else
			{
			LWM->Frame = 0;
			Set_3DS(LWM, LWInfo, LWM->Frame, Rend);
			KFCamera3ds->roll[0] = (float3ds)(LWM->Bank);
			KFCamera3ds->rkeys[0].time = (ulong3ds)LWM->Frame;
			KFCamera3ds->rkeys[0].rflags = 0;
			} // if ! KeysExist

		if (FovKeys)
			{
			Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
			Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
			j = 0;
			LastKeyFrame = -2;
			while (j < BankKeys)
				{
				NextKeyFrame = -1;
				Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = (LastKeyFrame + .5) / Rend->FrameRate;
				Prop.NewKeyNodeRange[0] = -DBL_MAX;
				Prop.NewKeyNodeRange[1] = DBL_MAX;
				Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].GetRAHostProperties(&Prop);
				if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1] && Prop.NewKeyNodeRange[1] < DBL_MAX)
					NextKeyFrame = (long)(Prop.NewKeyNodeRange[1] * Rend->FrameRate + .5);
				if (NextKeyFrame > LastKeyFrame)
					{
					if (GrNode = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].FindNearestSiblingNode(NextKeyFrame / Rend->FrameRate, NULL))
						{
						Rend->Cam->SetToTime(NextKeyFrame / Rend->FrameRate);
						LWM->Frame = NextKeyFrame;
						Set_3DS(LWM, LWInfo, LWM->Frame, Rend);
						LWM->TCB[0] = GrNode->TCB[0];
						LWM->TCB[1] = GrNode->TCB[1];
						LWM->TCB[2] = GrNode->TCB[2];
						KFCamera3ds->fov[j] = (float3ds)LWM->FOV;
						KFCamera3ds->fkeys[j].time = (ulong3ds)LWM->Frame;
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
			} // if KeysExist
		else
			{
			LWM->Frame = 0;
			Set_3DS(LWM, LWInfo, LWM->Frame, Rend);
			KFCamera3ds->fov[0] = (float3ds)LWM->FOV;
			KFCamera3ds->fkeys[0].time = (ulong3ds)LWM->Frame;
			KFCamera3ds->fkeys[0].rflags = 0;
			} // if ! KeysExist

		} // if
	else
		{
		if (KFCamera3ds)
			ReleaseCameraMotion3ds(KFCamera3dsPtr);
		success = 0;
		} // else if
	} // if only output camera key frames (key frame int <= 0)

else
	{
	MaxFrames = GlobalApp->MCP->GetMaxFrame();
	MaxFrames = (MaxFrames > 0) ? MaxFrames: 1;
	TotalKeys = 1 + (MaxFrames % LWInfo->KeyFrameInt ? 1: 0) + MaxFrames / LWInfo->KeyFrameInt;
	InitCameraMotion3ds(KFCamera3dsPtr, TotalKeys, TotalKeys, TotalKeys, TotalKeys);
	if ((KFCamera3ds = *KFCamera3dsPtr) && ! ftkerr3ds)
		{
		for (i = j = 0; i < MaxFrames + LWInfo->KeyFrameInt; i += LWInfo->KeyFrameInt, j++)
			{
			if (i > MaxFrames)
				i = MaxFrames;
			LWM->Frame = i;
			if (Rend->Cam->TargetObj)
				Rend->Cam->TargetObj->SetToTime(LWM->Frame / Rend->FrameRate);
			Rend->Cam->SetToTime(LWM->Frame / Rend->FrameRate);
			Rend->Cam->InitFrameToRender(EffectsHost, Rend);
			Set_3DS(LWM, LWInfo, i, Rend);

			KFCamera3ds->pos[j].x = (float3ds)(LWM->CamXYZ[0] * LWInfo->UnitScale);
			KFCamera3ds->pos[j].y = (float3ds)(LWM->CamXYZ[2] * LWInfo->UnitScale);
			KFCamera3ds->pos[j].z = (float3ds)(LWM->CamXYZ[1] * LWInfo->UnitScale);
			KFCamera3ds->pkeys[j].time = (ulong3ds)LWM->Frame;
			KFCamera3ds->pkeys[j].rflags = 0;
			KFCamera3ds->tpos[j].x = (float3ds)(LWM->TargetXYZ[0] * LWInfo->UnitScale);
			KFCamera3ds->tpos[j].y = (float3ds)(LWM->TargetXYZ[2] * LWInfo->UnitScale);
			KFCamera3ds->tpos[j].z = (float3ds)(LWM->TargetXYZ[1] * LWInfo->UnitScale);
			KFCamera3ds->tkeys[j].time = (ulong3ds)LWM->Frame;
			KFCamera3ds->tkeys[j].rflags = 0;
			KFCamera3ds->roll[j] = (float3ds)(LWM->Bank);
			KFCamera3ds->rkeys[j].time = (ulong3ds)LWM->Frame;
			KFCamera3ds->rkeys[j].rflags = 0;
			KFCamera3ds->fov[j] = (float3ds)LWM->FOV;
			KFCamera3ds->fkeys[j].time = (ulong3ds)LWM->Frame;
			KFCamera3ds->fkeys[j].rflags = 0;
			} // for i
		} // if
	else
		{
		if (KFCamera3ds)
			ReleaseCameraMotion3ds(KFCamera3dsPtr);
		success = 0;
		} // else
	} // else

EndWave:

if (LWM)
	delete LWM;

return (success);

} // SceneExportGUI::ThreeDSCamera_SetKeys()

/*===========================================================================*/

int SceneExportGUI::Set_3DS(ThreeDSMotion *LWM, ImportInfo *LWInfo, long Frame, RenderData *Rend)
{
double Heading, Pitch, Bank;
VertexDEM VP, FP, CV;

VP.CopyXYZ(Rend->Cam->CamPos);
FP.CopyXYZ(Rend->Cam->TargPos);
CV.CopyXYZ(Rend->Cam->CamVertical);
VP.RotateY(-LWInfo->RefLon);
FP.RotateY(-LWInfo->RefLon);
CV.RotateY(-LWInfo->RefLon);
FindPosVector(VP.XYZ, VP.XYZ, LWInfo->RefPt);
RotatePoint(VP.XYZ, LWInfo->RotMatx);
RotatePoint(FP.XYZ, LWInfo->RotMatx);
RotatePoint(CV.XYZ, LWInfo->RotMatx);

// create a dummy target at distance 10m from camera
FP.MultiplyXYZ(10.0);
LWM->TargetXYZ[0] = VP.XYZ[0] + FP.XYZ[0];
LWM->TargetXYZ[1] = VP.XYZ[1] + FP.XYZ[1];
LWM->TargetXYZ[2] = VP.XYZ[2] + FP.XYZ[2];

Heading = FP.FindAngleYfromZ();
FP.RotateY(-Heading);
Pitch = FP.FindAngleXfromZ();
CV.RotateY(-Heading);
CV.RotateX(-Pitch);
Bank = CV.FindAngleZfromY();

LWM->CamXYZ[0] = VP.XYZ[0];	/* X */
LWM->CamXYZ[1] = VP.XYZ[1];	/* Y */
LWM->CamXYZ[2] = VP.XYZ[2];	/* Z */

LWM->Bank = Bank;			/* Bank */

LWM->FOV = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue;
if (LWM->FOV < .001)
	LWM->FOV = .001;
if (LWM->FOV > 179.0)
	LWM->FOV = 179.0;

return (1);

} // SceneExportGUI::Set_3DS() 

/*===========================================================================*/

int SceneImportGUI::UnSet_3DSPoint(ImportInfo *LWInfo, VertexDEM *Vert)
{

Vert->XYZ[0] *= LWInfo->UnitScale;
Vert->XYZ[1] *= LWInfo->UnitScale;
Vert->XYZ[2] *= LWInfo->UnitScale;

RotatePoint(Vert->XYZ, LWInfo->RotMatx);
Vert->XYZ[0] += LWInfo->RefPt[0];
Vert->XYZ[1] += LWInfo->RefPt[1];
Vert->XYZ[2] += LWInfo->RefPt[2];

DefCoords->CartToDeg(Vert);

Vert->Lon += LWInfo->RefLon;

return (1);

} // SceneImportGUI::UnSet_3DSPoint

/*===========================================================================*/

int SceneExportGUI::ThreeDSCamTarget_SetKeys(kfmesh3ds **KFMesh3dsPtr, ImportInfo *LWInfo, RenderData *Rend)
{
long MaxFrames, TotalKeys, CameraKeys = 0, LastKeyFrame, i, j, success = 1;
ThreeDSMotion *LWM = NULL;
kfmesh3ds *KFMesh3ds = NULL;
GraphNode *GrNode;
RasterAnimHostProperties Prop;

// determine if key frames exist for the camera motion path 

TotalKeys = 0;
LastKeyFrame = -2;
while ((LastKeyFrame = Rend->Cam->GetNextMotionKeyFrame(LastKeyFrame, Rend->FrameRate)) >= 0)
	CameraKeys ++;

TotalKeys = CameraKeys;
if (TotalKeys < 1)
	{
	return (1);
	} // no motion key frames 

// allocate some essential resources 
Rend->Cam->InitToRender(Rend->Opt, NULL);


if ((LWM = new ThreeDSMotion()) == NULL)
	{
	success = 0;
	goto EndWave;
	} // if out of memory 

// different procedures if key frame interval is 0 - output only at camera, focus or bank key frames or at frame 0 if no keys
if (LWInfo->KeyFrameInt <= 0)
	{
	InitObjectMotion3ds(KFMesh3dsPtr, CameraKeys > 1 ? CameraKeys: 1, 0, 0, 0, 0);
	if ((KFMesh3ds = *KFMesh3dsPtr) && ! ftkerr3ds)
		{
		if (CameraKeys)
			{
			LastKeyFrame = -2;
			j = 0;
			while (((LastKeyFrame = Rend->Cam->GetNextMotionKeyFrame(LastKeyFrame, Rend->FrameRate)) >= 0) && j < CameraKeys)
				{
				LWM->Frame = LastKeyFrame;
				// init camera to this frame position
				if (Rend->Cam->TargetObj)
					Rend->Cam->TargetObj->SetToTime(LWM->Frame / Rend->FrameRate);
				Rend->Cam->SetToTime(LWM->Frame / Rend->FrameRate);
				Rend->Cam->InitFrameToRender(EffectsHost, Rend);
				// determine HPB
				Set_3DS(LWM, LWInfo, LWM->Frame, Rend);
				// determine TCB - we'll need the nearest node for that
				if (GrNode = Rend->Cam->GetNearestMotionNode(LWM->Frame / Rend->FrameRate))
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
				KFMesh3ds->pos[j].x = (float3ds)(LWM->TargetXYZ[0] * LWInfo->UnitScale);
				KFMesh3ds->pos[j].y = (float3ds)(LWM->TargetXYZ[2] * LWInfo->UnitScale);
				KFMesh3ds->pos[j].z = (float3ds)(LWM->TargetXYZ[1] * LWInfo->UnitScale);
				KFMesh3ds->pkeys[j].time = (ulong3ds)LWM->Frame;
				KFMesh3ds->pkeys[j].rflags = (LWM->TCB[0] != 0.0) | ((LWM->TCB[1] != 0.0) << 1) | ((LWM->TCB[2] != 0.0) << 2);
				KFMesh3ds->pkeys[j].tension = (float3ds)LWM->TCB[0];
				KFMesh3ds->pkeys[j].continuity = (float3ds)LWM->TCB[1];
				KFMesh3ds->pkeys[j].bias = (float3ds)LWM->TCB[2];
				j ++;
				} // while
			} // if camera motion keys
		else
			{
			LWM->Frame = 0;
			Set_3DS(LWM, LWInfo, LWM->Frame, Rend);
			KFMesh3ds->pos[0].x = (float3ds)(LWM->TargetXYZ[0] * LWInfo->UnitScale);
			KFMesh3ds->pos[0].y = (float3ds)(LWM->TargetXYZ[2] * LWInfo->UnitScale);
			KFMesh3ds->pos[0].z = (float3ds)(LWM->TargetXYZ[1] * LWInfo->UnitScale);
			KFMesh3ds->pkeys[0].time = (ulong3ds)LWM->Frame;
			KFMesh3ds->pkeys[0].rflags = 0;
			} // else 

		} // if
	else
		{
		if (KFMesh3ds)
			ReleaseObjectMotion3ds(KFMesh3dsPtr);
		success = 0;
		} // else if
	} // if only output camera key frames (key frame int <= 0)

else
	{
	MaxFrames = GlobalApp->MCP->GetMaxFrame();
	MaxFrames = (MaxFrames > 0) ? MaxFrames: 1;
	TotalKeys = 1 + (MaxFrames % LWInfo->KeyFrameInt ? 1: 0) + MaxFrames / LWInfo->KeyFrameInt;
	InitObjectMotion3ds(KFMesh3dsPtr, TotalKeys, 0, 0, 0, 0);
	if ((KFMesh3ds = *KFMesh3dsPtr) && ! ftkerr3ds)
		{
		for (i = j = 0; i < MaxFrames + LWInfo->KeyFrameInt; i += LWInfo->KeyFrameInt, j++)
			{
			if (i > MaxFrames)
				i = MaxFrames;
			LWM->Frame = i;
			if (Rend->Cam->TargetObj)
				Rend->Cam->TargetObj->SetToTime(LWM->Frame / Rend->FrameRate);
			Rend->Cam->SetToTime(LWM->Frame / Rend->FrameRate);
			Rend->Cam->InitFrameToRender(EffectsHost, Rend);
			Set_3DS(LWM, LWInfo, i, Rend);

			KFMesh3ds->pos[j].x = (float3ds)(LWM->TargetXYZ[0] * LWInfo->UnitScale);
			KFMesh3ds->pos[j].y = (float3ds)(LWM->TargetXYZ[2] * LWInfo->UnitScale);
			KFMesh3ds->pos[j].z = (float3ds)(LWM->TargetXYZ[1] * LWInfo->UnitScale);
			KFMesh3ds->pkeys[j].time = (ulong3ds)LWM->Frame;
			KFMesh3ds->pkeys[j].rflags = 0;
			} // for i
		} // if
	else
		{
		if (KFMesh3ds)
			ReleaseObjectMotion3ds(KFMesh3dsPtr);
		success = 0;
		} // else
	} // else

EndWave:

if (LWM)
	delete LWM;

return (success);

} // SceneExportGUI::ThreeDSCamTarget_SetKeys()

/*===========================================================================*/

#endif // WCS_SUPPORT_3DS
