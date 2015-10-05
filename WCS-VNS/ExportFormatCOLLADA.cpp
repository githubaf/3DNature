// ExportFormatCOLLADA.cpp
// Code module for COLLADA export code
// Created from ExportFormatCOLLADA.cpp on 08/14/06 by FPW2
// Copyright 2006 3D Nature, LLC. All rights reserved.

#include "stdafx.h"
#include "EffectsLib.h"
#include "ExportFormat.h"
#include "PathAndFile.h"
#include "Project.h"
#include "UsefulZip.h"
#include "FeatureConfig.h"
#include "Requester.h"
#include "ExportControlGUI.h"
#include "Raster.h"
#include "ImageOutputEvent.h"
#include "AppMem.h"
#include "Lists.h"
#include "UsefulGeo.h"
#include "UsefulIO.h"
#include "Log.h"
#include "Realtime.h"
#include "SXExtension.h"
#include "SXQueryAction.h"
#include "SXQueryItem.h"
#include "WCSVersion.h"

// COLLADA uses a Right-Handed Coordinate System
// http://mathworld.wolfram.com/Right-HandedCoordinateSystem.html
// F2_NOTE: Put Up-axis as Advanced Config Opt
// X_UP: Right Axis is -Y, Up Axis is +X, Into Monitor is -Z
// Y_UP: Right Axis is +X, Up Axis is +Y, Into Monitor is -Z
// Z_UP: Right Axis is +X, Up Axis is +Z, Into Monitor is +Y

//#define COLLADA_TREE_SCALE 100.0f	// For sizing our master nodes at the origin and their instances

extern WCSApp *GlobalApp;

//static bool warningCheck = false;
static char objName[64];
static char matName[64];
static char msgCE[] = "COLLADA Exporter";

/*===========================================================================*/

/***
static void FixName(const char *name)
{
char lobjName[64];

strcpy(lobjName, name);
for (size_t i = 0; i < strlen(name); i++)
	{
	// If it's an alphanumeric char, we're OK.  Otherwise, replace it with an underscore.
	if (isalnum(lobjName[i]))
		continue;
	else
		lobjName[i] = '_';
	} // for

objName[0] = NULL;
objName[1] = NULL;

// First character must be a letter
if (isdigit(lobjName[0]) || (lobjName[0] == '_'))
	objName[0] = 'O';

strcat(objName, lobjName);

} // fixName
***/

/*===========================================================================*/

static void FixMatName(const char *name)
{
char lmatName[64];

strcpy(lmatName, name);
for (size_t i = 0; i < strlen(name); i++)
	{
	// If it's an alphanumeric char, we're OK.  Otherwise, replace it with an underscore.
	if (isalnum(lmatName[i]))
		continue;
	else
		lmatName[i] = '_';
	} // for

matName[0] = NULL;
matName[1] = NULL;

// First character must be a letter
if (isdigit(lmatName[0]) || (lmatName[0] == '_'))
	matName[0] = 'M';

strcat(matName, lmatName);

} // fixMatName

/*===========================================================================*/

ExportFormatCOLLADA::ExportFormatCOLLADA(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource)
: ExportFormat(MasterSource, ProjectSource, EffectsSource, DBSource, ImageSource)
{

curIndent = 0;
indentStr[0] = NULL;

} // ExportFormatCOLLADA::ExportFormatCOLLADA

/*===========================================================================*/

ExportFormatCOLLADA::~ExportFormatCOLLADA()
{

} // ExportFormatCOLLADA::~ExportFormatCOLLADA

/*===========================================================================*/

int ExportFormatCOLLADA::C_prologue_asset(unsigned long upAxis)
{
struct tm *newTime;
time_t szClock;
char timeString[32];

// prologue
fprintf(fDAE, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
fprintf(fDAE, "<COLLADA xmlns=\"http://www.collada.org/2005/11/COLLADASchema\" version=\"1.4.1\">\n");

// asset
fprintf(fDAE, "\t<asset>\n\t\t<contributor>\n");
fprintf(fDAE, "\t\t\t<author>%s</author>\n", GlobalApp->MainProj->UserName);
fprintf(fDAE, "\t\t\t<authoring_tool>%s %s with Scene Express</authoring_tool>\n", APP_TITLE, APP_VERS);
fprintf(fDAE, "\t\t</contributor>\n");
// create a time string in ISO8601 format
time(&szClock); // Get time in seconds
newTime = gmtime(&szClock); // Convert time to struct tm form (use UTC, so we don't have to bother with time zone info)
sprintf(timeString, "%d-%02d-%02dT%02d:%02d:%02dZ", newTime->tm_year + 1900, newTime->tm_mon + 1, newTime->tm_mday,
	newTime->tm_hour, newTime->tm_min, newTime->tm_sec);		// create ISO8601 format string from struct tm
fprintf(fDAE, "\t\t<created>%s</created>\n", timeString);		// required tag - date in ISO8601 format
fprintf(fDAE, "\t\t<modified>%s</modified>\n", timeString);		// required tag - date in ISO8601 format
fprintf(fDAE, "\t\t<up_axis>");
if (upAxis == 0)
	fprintf(fDAE, "X_UP");
else if (upAxis == 1)
	fprintf(fDAE, "Y_UP");
else
	fprintf(fDAE, "Z_UP");
fprintf(fDAE, "</up_axis>\n");
return(fprintf(fDAE, "\t</asset>\n"));

} // ExportFormatCOLLADA::C_prologue_asset

/*===========================================================================*/

// remove all temp files
void ExportFormatCOLLADA::Cleanup(NameList **fileNamesCreated)
{
const char *cleanupFileName;
long fileType;
char fullFileName[512];

// remove foliage temp files
fileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILEIDX;
if (cleanupFileName = (*fileNamesCreated)->FindNameOfType(fileType))
	{
	strmfp(fullFileName, Master->OutPath.GetPath(), cleanupFileName);
	PROJ_remove(fullFileName);

	fileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILE;
	if (cleanupFileName = (*fileNamesCreated)->FindNameOfType(fileType))
		{
		strmfp(fullFileName, Master->OutPath.GetPath(), cleanupFileName);
		PROJ_remove(fullFileName);
		} // if
	} // if

// remove terrain temp file(s)
fileType = WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN;
if (Master->ExportTerrain && Master->ExportTexture)
	{
	if ((Master->DEMTilesX > 1) || (Master->DEMTilesY > 1))
		{
		cleanupFileName = NULL;
		cleanupFileName = (*fileNamesCreated)->FindNextNameOfType(fileType, cleanupFileName);
		while (cleanupFileName)
			{
			strmfp(fullFileName, Master->OutPath.GetPath(), cleanupFileName);
			PROJ_remove(fullFileName);
			cleanupFileName = (*fileNamesCreated)->FindNextNameOfType(fileType, cleanupFileName);
			} // while
		} // if Tiles
	else
		{
		cleanupFileName = (*fileNamesCreated)->FindNameOfType(fileType);
		strmfp(fullFileName, Master->OutPath.GetPath(), cleanupFileName);
		PROJ_remove(fullFileName);
		} // else Tiles
	} // if

// clean up image textures that are still in the scene export root dir
if (Master->ExportTexture)
	{
	fileType = WCS_EXPORTCONTROL_FILETYPE_TEX1;
	if ((Master->DEMTilesX > 1) || (Master->DEMTilesY > 1))
		{
		cleanupFileName = NULL;
		while (cleanupFileName = (*fileNamesCreated)->FindNextNameOfType(fileType, cleanupFileName))
			{
			strmfp(fullFileName, Master->OutPath.GetPath(), cleanupFileName);
			PROJ_remove(fullFileName);
			} // while
		} // if tiles
	else
		{
		cleanupFileName = (*fileNamesCreated)->FindNameOfType(fileType);
		strmfp(fullFileName, Master->OutPath.GetPath(), cleanupFileName);
		PROJ_remove(fullFileName);
		} // else tiles
	} // if

for (cleanupFileName = (*fileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_3DOBJTEX); cleanupFileName; cleanupFileName = (*fileNamesCreated)->FindNextNameOfType(WCS_EXPORTCONTROL_FILETYPE_3DOBJTEX, cleanupFileName))
	{
	strmfp(fullFileName, Master->OutPath.GetPath(), cleanupFileName);
	PROJ_remove(fullFileName);
	} // for

for (cleanupFileName = (*fileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_FOLIAGETEX); cleanupFileName; cleanupFileName = (*fileNamesCreated)->FindNextNameOfType(WCS_EXPORTCONTROL_FILETYPE_FOLIAGETEX, cleanupFileName))
	{
	strmfp(fullFileName, Master->OutPath.GetPath(), cleanupFileName);
	PROJ_remove(fullFileName);
	} // for

for (cleanupFileName = (*fileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_ROOFTEX); cleanupFileName; cleanupFileName = (*fileNamesCreated)->FindNextNameOfType(WCS_EXPORTCONTROL_FILETYPE_ROOFTEX, cleanupFileName))
	{
	strmfp(fullFileName, Master->OutPath.GetPath(), cleanupFileName);
	PROJ_remove(fullFileName);
	} // for

for (cleanupFileName = (*fileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_WALLTEX); cleanupFileName; cleanupFileName = (*fileNamesCreated)->FindNextNameOfType(WCS_EXPORTCONTROL_FILETYPE_WALLTEX, cleanupFileName))
	{
	strmfp(fullFileName, Master->OutPath.GetPath(), cleanupFileName);
	PROJ_remove(fullFileName);
	} // for

} // ExportFormatCOLLADA::Cleanup

/*===========================================================================*/

char *ExportFormatCOLLADA::CreateValidID(const char *string)
{
char tempID[256];

strcpy(tempID, string);
for (size_t i = 0; i < strlen(string); i++)
	{
	// If it's an alphanumeric char, we're OK.  Otherwise, replace it with an underscore.
	if (isalnum(tempID[i]))
		continue;
	else
		tempID[i] = '_';
	} // for

xsID[0] = NULL;
xsID[1] = NULL;

// First character must be a letter
if (isdigit(tempID[0]) || (tempID[0] == '_'))
	xsID[0] = 'I';

strcat(xsID, tempID);
return(xsID);

} // CreateValidID::CreateValidID

/*===========================================================================*/

int ExportFormatCOLLADA::Export3DObjs(NameList **fileNamesCreated)
{
#ifdef GORILLA
Renderer *rend = NULL;
RenderOpt *curOpt = NULL;
RenderJob *curRJ = NULL;
Camera *curCam = NULL;
int success = 0;
unsigned long coordsCount = 0;
Object3DInstance *curInstance = Master->ObjectInstanceList;
Object3DEffect *object3D;

/***
if (rend)
	{
	ConfigureTB(NativeWin, IDC_SKETCH, NULL, NULL, NULL);
	ConfigureTB(NativeWin, IDC_SKETCHPREV, NULL, NULL, NULL);
	delete rend;
	rend = NULL;
	SetPreview(TempPreview);
	} // if
***/

// create a camera, renderopt, render job
// create a renderer too
if ((rend = new Renderer()) && (curOpt = new RenderOpt) && (curRJ = new RenderJob) && (curCam = new Camera))
	{
	curRJ->Cam = curCam;
	curRJ->Options = curOpt;
	// turn off anything that would take a while to initialize and does not affect 3d objects
	curOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] = 
	curOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAINPARAM] = 
	curOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE] = 
	curOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE] = 
	curOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM] = 
	curOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_FENCE] = 
	curOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_SNOW] = 
	curOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_GROUND] = 
	curOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAFFECTOR] = 
	curOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_RASTERTA] = 
	curOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ENVIRONMENT] = 
	curOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_CMAP] = 
	curOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE] = 
	curOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT] = 
	curOpt->EffectEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM] = 0;
	// initialize it
	if (rend->InitForProcessing(curRJ, GlobalApp, EffectsHost, Images, DBHost, ProjectHost, GlobalApp->StatusLog, FALSE))	// false=not just elevations, need 3do's init'ed
		success = 1;
	} // if

//
if (success)
	{
	char pathName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];
	char modelName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];
	char tempFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

	sprintf(tempStr, "%s<Folder>\n%s\t<name>3D Objects</name>\n", indentStr, indentStr);
	fputs(tempStr, fKML);
	IndentMore();

	strcpy(tempFullPath, (char *)Master->OutPath.GetPath());
	strcpy(pathName, GlobalApp->MainProj->MungPath(tempFullPath));
	strcat(pathName, "/models/");
	if (PROJ_mkdir(pathName) && (errno != EEXIST)) // try to create subdir - ok if it already exists
		{
		char msg[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN + 128];

		curInstance = NULL;	// we couldn't make our subdir, so abort processing on this object
		sprintf(&msg[0], "%s: Unable to create directory '%s'. Object '%s' skipped", msgCE, pathName, curInstance->MyObj->Name);
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, msg);
		} // if

	while (curInstance)
		{
		object3D = curInstance->MyObj;
		if (curInstance->IsBounded(Master->FetchRBounds()) &&
			((object3D->Vertices && object3D->Polygons && object3D->NameTable) || object3D->OpenInputFile(NULL, FALSE, FALSE, FALSE)))
			{
			long createIt = 1;

			if ((object3D->NumVertices > 21845) || (object3D->NumPolys > 21845))
				{
				char msg[256];

				createIt = 0;
				sprintf(&msg[0], "%s: Object '%s' has too many vertices or polygons (> 21845) for Google Earth - skipped.", msgCE, object3D->Name);
				GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, msg);
				} // if
			else
				{
				strcpy(tempFullPath, (char *)Master->OutPath.GetPath());
				strcpy(pathName, GlobalApp->MainProj->MungPath(tempFullPath));
				strcat(pathName, "/models/");
				FixName(object3D->Name);
				strcpy(modelName, objName);
				strcat(modelName, ".dae");
				strcpy(tempFullPath, pathName);
				strcat(tempFullPath, modelName);
				} // else

			if (createIt && (fDAE = fopen(tempFullPath, "w")))
				{
				double dimx, dimy, dimz, eyeDist;
				long matCt;

				// create link in main file
				sprintf(tempStr, "%s<Placemark>\n", indentStr);
				fputs(tempStr, fKML);
				IndentMore();
				sprintf(tempStr, "%s<name>%s</name>\n", indentStr, object3D->Name);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s<LookAt>\n", indentStr);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s\t<longitude>%lf</longitude>\n", indentStr, -curInstance->WCSGeographic[0]);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s\t<latitude>%lf</latitude>\n", indentStr, curInstance->WCSGeographic[1]);
				fputs(tempStr, fKML);
				// come up with some reasonable distance to be from our object - 2x max dimension seems to work fine
				dimx = object3D->ObjectBounds[0] - object3D->ObjectBounds[1];
				dimy = object3D->ObjectBounds[2] - object3D->ObjectBounds[3];
				dimz = object3D->ObjectBounds[4] - object3D->ObjectBounds[5];
				eyeDist = MAX3(dimx, dimy, dimz) * 2.0;
				sprintf(tempStr, "%s\t<range>%lf</range>\n", indentStr, eyeDist);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s\t<tilt>70.0</tilt>\n", indentStr);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s</LookAt>\n", indentStr);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s<Model>\n", indentStr);
				fputs(tempStr, fKML);
				IndentMore();
				sprintf(tempStr, "%s<altitudeMode>relativeToGround</altitudeMode>\n", indentStr);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s<Location>\n", indentStr);
				fputs(tempStr, fKML);
				sprintf(tempStr, "  %s<longitude>%lf</longitude>\n", indentStr, -curInstance->WCSGeographic[0]);
				fputs(tempStr, fKML);
				sprintf(tempStr, "  %s<latitude>%lf</latitude>\n", indentStr, curInstance->WCSGeographic[1]);
				fputs(tempStr, fKML);
				sprintf(tempStr, "  %s<altitude>%lf</altitude>\n", indentStr, 0.0); //curInstance->WCSGeographic[2]);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s</Location>\n", indentStr);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s<Orientation>\n", indentStr);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s\t<heading>%lf</heading>\n", indentStr, curInstance->Euler[1]);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s\t<tilt>%lf</tilt>\n", indentStr, 90.0 - curInstance->Euler[0]);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s\t<roll>%lf</roll>\n", indentStr, -curInstance->Euler[2]);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s</Orientation>\n", indentStr);
				fputs(tempStr, fKML);
				/***
				sprintf(tempStr, "%s<Scale>\n", indentStr);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s\t<x>%f</x>\n", indentStr, curInstance->Scale[0]);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s\t<y>%f</y>\n", indentStr, curInstance->Scale[1]);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s\t<z>%f</z>\n", indentStr, curInstance->Scale[2]);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s</Scale>\n", indentStr);
				fputs(tempStr, fKML);
				***/
				sprintf(tempStr, "%s<Link>\n", indentStr);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s\t<href>models/%s</href>\n", indentStr, modelName);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s</Link>\n", indentStr);
				fputs(tempStr, fKML);
				IndentLess();
				sprintf(tempStr, "%s</Model>\n", indentStr);
				fputs(tempStr, fKML);
				IndentLess();
				sprintf(tempStr, "%s</Placemark>\n", indentStr);
				fputs(tempStr, fKML);

				// prologue & asset
				C_prologue_asset(1);

				// library images
				if (object3D->VertexUVWAvailable)
					{
					fprintf(fDAE, "\t<library_images>\n");
					for (matCt = 0; matCt < object3D->NumMaterials; matCt++)
						ProcessLibraryImages(fileNamesCreated);
					fprintf(fDAE, "\t</library_images>\n");
					} // if

				// library materials
				fprintf(fDAE, "\t<library_materials>\n");
				for (matCt = 0; matCt < object3D->NumMaterials; matCt++)
					ProcessLibraryMaterials(fileNamesCreated);
				fprintf(fDAE, "\t</library_materials>\n");

				// library effects
				fprintf(fDAE, "\t<library_effects>\n");
				for (matCt = 0; matCt < object3D->NumMaterials; matCt++)
					ProcessLibraryEffects(fileNamesCreated);
				fprintf(fDAE, "\t</library_effects>\n");

				// library geometries
				fprintf(fDAE, "\t<library_geometries>\n");
				ProcessGeometries(fileNamesCreated);
				fprintf(fDAE, "\t</library_geometries>\n");

				// library visual scenes
				fprintf(fDAE, "\t<library_visual_scenes>\n");
				fprintf(fDAE, "\t\t<visual_scene id=\"Scene_Express_Scene\">\n");
				fprintf(fDAE, "\t\t\t<node id=\"%s\">\n", objName);
				fprintf(fDAE, "\t\t\t\t<scale sid=\"scale\">%f %f %f</scale>\n", curInstance->Scale[0], curInstance->Scale[1], curInstance->Scale[2]);
				fprintf(fDAE, "\t\t\t\t<instance_geometry url=\"#%s-geometry\"/>\n", objName);
				fprintf(fDAE, "\t\t\t</node>\n");
				fprintf(fDAE, "\t\t</visual_scene>\n");
				fprintf(fDAE, "\t</library_visual_scenes>\n");

				} // if fDAE
			} // if bounded & stuff
		curInstance = curInstance->Next;
		} // while

	IndentLess();
	sprintf(tempStr, "%s</Folder>\n", indentStr);
	fputs(tempStr, fKML);
	} // if success

if (rend)
	{
	rend->Cleanup(TRUE, FALSE, TRUE, FALSE);
	delete rend;
	} // if
if (curOpt)
	delete curOpt;
if (curRJ)
	delete curRJ;
if (curCam)
	delete curCam;

return (success);
#else // GORILLA
return 1;
#endif // GORILLA
} // ExportFormatCOLLADA::Export3DObjs

/*===========================================================================*/

int ExportFormatCOLLADA::ProcessLibraryCameras(void)
{
class EffectList *camList = Master->Cameras;
class Camera *cam;
int rval = 1;

// orthographic or perspective only
sectionInactive = 1;

while (camList)
	{
	if (sectionInactive)
		{
		sectionInactive = 0;
		fputs("\t<library_cameras>\n", fDAE);
		} // if
	cam = (class Camera *)camList->Me;
	camList = camList->Next;
	if ((cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC) || (cam->Orthographic))
		{
		// F2_NOTE: finish this
		} // if
	// F2_NOTE: Need to check for unsupportable camera types
	else
		{
		sprintf(tempStr, "\t\t<camera id=\"%s-camera\">\n", CreateValidID(cam->Name));
		fputs(tempStr, fDAE);
		sprintf(tempStr, "\t\t\t<optics>\n");
		fputs(tempStr, fDAE);
		sprintf(tempStr, "\t\t\t\t<technique_common>\n");
		fputs(tempStr, fDAE);
		sprintf(tempStr, "\t\t\t\t\t<perspective>\n");
		fputs(tempStr, fDAE);
		sprintf(tempStr, "\t\t\t\t\t\t<yfov>%f</yfov>\n", (float)cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue);
		fputs(tempStr, fDAE);
		sprintf(tempStr, "\t\t\t\t\t\t<aspect_ratio>1.0</aspect_ratio>\n");
		fputs(tempStr, fDAE);
		sprintf(tempStr, "\t\t\t\t\t\t<znear>1.0</znear>\n");
		fputs(tempStr, fDAE);
		sprintf(tempStr, "\t\t\t\t\t\t<zfar>32767.0</zfar>\n");
		fputs(tempStr, fDAE);
		sprintf(tempStr, "\t\t\t\t\t</perspective>\n");
		fputs(tempStr, fDAE);
		sprintf(tempStr, "\t\t\t\t</technique_common>\n");
		fputs(tempStr, fDAE);
		sprintf(tempStr, "\t\t\t</optics>\n");
		fputs(tempStr, fDAE);
		sprintf(tempStr, "\t\t</camera>\n");
		fputs(tempStr, fDAE);
		} // else
	} // while camList

if (! sectionInactive)
	rval = fputs("\t</library_cameras>\n", fDAE);

return(rval >= 0);

} // ExportFormatCOLLADA::ProcessLibraryCameras

/*===========================================================================*/

int ExportFormatCOLLADA::ProcessLibraryLights(void)
{
class EffectList *liteList = Master->Lights;
class Light *lite;
int rval = 1;

sectionInactive = 1;

while (liteList)
	{
	lite = (class Light *)liteList->Me;
	if (lite->Enabled)
		{
		float r, g, b, tmp;

		if (sectionInactive)
			{
			sectionInactive = 0;
			fputs("\t<library_lights>\n", fDAE);
			} // if
		sprintf(tempStr, "\t\t<light id=\"%s-light\">\n", CreateValidID(lite->Name));
		fputs(tempStr, fDAE);
		sprintf(tempStr, "\t\t\t<technique_common>\n");
		fputs(tempStr, fDAE);

		r = (float)lite->Color.CurValue[0];
		g = (float)lite->Color.CurValue[1];
		b = (float)lite->Color.CurValue[2];
		switch (lite->LightType)
			{
			default:
			case WCS_EFFECTS_LIGHTTYPE_PARALLEL:
				sprintf(tempStr, "\t\t\t\t<directional>\n");
				fputs(tempStr, fDAE);
				sprintf(tempStr, "\t\t\t\t\t<color>%f %f %f</color>\n", r, g, b);
				fputs(tempStr, fDAE);
				sprintf(tempStr, "\t\t\t\t</directional>\n");
				fputs(tempStr, fDAE);
				if (! lite->Distant)
					{
					sprintf(logMsg, "%s: Treating light '%s' as distant", msgCE, lite->Name);
					GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, logMsg);
					} // if
				break;
			case WCS_EFFECTS_LIGHTTYPE_OMNI:
				tmp = (float)lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_FALLOFFEXP].CurValue;
				sprintf(tempStr, "\t\t\t\t<point>\n");
				fputs(tempStr, fDAE);
				sprintf(tempStr, "\t\t\t\t\t<color>%f %f %f</color>\n", r, g, b);
				fputs(tempStr, fDAE);
				// convert our falloff exponent of 0..5 to something in the COLLADA model
				if (tmp > 2.0f)
					{
					tmp = 2.0f;
					sprintf(logMsg, "%s: Falloff exponent of light '%s' limited to 2.0", msgCE, lite->Name);
					GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, logMsg);
					} // if
				if (tmp <= 1.0f)
					{
					sprintf(tempStr, "\t\t\t\t\t<constant_attenuation>%f</constant_attenuation>\n", 1.0f - tmp);
					fputs(tempStr, fDAE);
					sprintf(tempStr, "\t\t\t\t\t<linear_attenuation>%f</linear_attenuation>\n", tmp);
					fputs(tempStr, fDAE);
					} // if
				else
					{
					sprintf(tempStr, "\t\t\t\t\t<linear_attenuation>%f</linear_attenuation>\n", 2.0f - tmp);
					fputs(tempStr, fDAE);
					sprintf(tempStr, "\t\t\t\t\t<quadratic_attenuation>%f</quadratic_attenuation>\n", tmp - 1.0f);
					fputs(tempStr, fDAE);
					} // else
				sprintf(tempStr, "\t\t\t\t</point>\n");
				fputs(tempStr, fDAE);
				break;
			case WCS_EFFECTS_LIGHTTYPE_SPOT:
				tmp = (float)lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_FALLOFFEXP].CurValue;
				sprintf(tempStr, "\t\t\t\t<spot>\n");
				fputs(tempStr, fDAE);
				sprintf(tempStr, "\t\t\t\t\t<color>%f %f %f</color>\n", r, g, b);
				fputs(tempStr, fDAE);
				// convert our falloff exponent of 0..5 to something in the COLLADA model
				if (tmp > 2.0f)
					{
					tmp = 2.0f;
					sprintf(logMsg, "%s: Falloff exponent of light '%s' limited to 2.0", msgCE, lite->Name);
					GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, logMsg);
					} // if
				if (tmp <= 1.0f)
					{
					sprintf(tempStr, "\t\t\t\t\t<constant_attenuation>%f</constant_attenuation>\n", 1.0f - tmp);
					fputs(tempStr, fDAE);
					sprintf(tempStr, "\t\t\t\t\t<linear_attenuation>%f</linear_attenuation>\n", tmp);
					fputs(tempStr, fDAE);
					} // if
				else
					{
					sprintf(tempStr, "\t\t\t\t\t<linear_attenuation>%f</linear_attenuation>\n", 2.0f - tmp);
					fputs(tempStr, fDAE);
					sprintf(tempStr, "\t\t\t\t\t<quadratic_attenuation>%f</quadratic_attenuation>\n", tmp - 1.0f);
					fputs(tempStr, fDAE);
					} // else
				sprintf(tempStr, "\t\t\t\t</spot>\n");
				fputs(tempStr, fDAE);
				break;
			} // switch LightType

		sprintf(tempStr, "\t\t\t</technique_common>\n");
		fputs(tempStr, fDAE);
		sprintf(tempStr, "\t\t</light>\n");
		fputs(tempStr, fDAE);
		} // if Enabled
	liteList = liteList->Next;
	} // while liteList

if (! sectionInactive)
	rval = fputs("\t</library_lights>\n", fDAE);

return(rval >= 0);

} // ExportFormatCOLLADA::ProcessLibraryLights

/*===========================================================================*/

int ExportFormatCOLLADA::ExportTerrain(NameList **fileNamesCreated)
{
#ifdef GORILLA
double geoCtrLon, geoCtrLat, lonScale;
FILE *fRaw;
//GradientCritter *gradCrit;
//MaterialEffect *mat;
float *nElevs = NULL, *sElevs = NULL;
const char *baseName, *rawTerrainFile, *textureFile;
//size_t rowSize;
int success = 1;
float elevAdjust;
//unsigned long polyColor;
long fileType, x, y;
long xTile, yTile;
long xLimit, yLimit;
long tiles = 0;
long tilesizex, tilesizey;
char pathName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];
//char modelName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];
char tempFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

/***

DEM triangles viewed from Above:

0 --- 1 --- 2 ... (nElevs)
|\    |\    |
|  \  |  \  |
|    \|    \|
* --- * --- * ... (sElevs)

***/

geoCtrLon = Master->ExportRefData.ExportRefLon;
geoCtrLat = Master->ExportRefData.ExportRefLat;
lonScale = Master->ExportRefData.ExportLonScale;
elevAdjust = (float)(-Master->ExportRefData.RefElev);

if (Master->BoundsType == WCS_EFFECTS_SCENEEXPORTER_BOUNDSTYPE_CELLEDGES)
	{
	xLimit = Master->DEMResX - 1;
	yLimit = Master->DEMResY - 1;
	} // if
else
	{
	xLimit = Master->DEMResX;
	yLimit = Master->DEMResY;
	} // else

if ((Master->DEMTilesX > 1) || (Master->DEMTilesY > 1))
	{
	tiles = 1;
	fileType = WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN;
	rawTerrainFile = NULL;
	rawTerrainFile = (*fileNamesCreated)->FindNextNameOfType(fileType, rawTerrainFile);
	fRaw = PROJ_fopen(rawTerrainFile, "rb");

	fileType = WCS_EXPORTCONTROL_FILETYPE_TEX1;
	textureFile = NULL;
	textureFile = (*fileNamesCreated)->FindNextNameOfType(fileType, textureFile);

	//tilesizex = (long)((xLimit + Master->DEMTilesX - 1) / Master->DEMTilesX);
	//tilesizey = (long)((yLimit + Master->DEMTilesY - 1) / Master->DEMTilesY);
	tilesizex = xLimit;
	tilesizey = yLimit;
	} // if tiles
else
	{
	fileType = WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN;
	rawTerrainFile = (*fileNamesCreated)->FindNameOfType(fileType);
	fRaw = PROJ_fopen(rawTerrainFile, "rb");

	fileType = WCS_EXPORTCONTROL_FILETYPE_TEX1;
	textureFile = (*fileNamesCreated)->FindNameOfType(fileType);
	tilesizex = xLimit;
	tilesizey = yLimit;
	} // else tiles

baseName = Master->OutPath.GetName();

strcpy(tempFullPath, (char *)Master->OutPath.GetPath());
strcpy(pathName, GlobalApp->MainProj->MungPath(tempFullPath));
strcat(pathName, "/models/");
if (PROJ_mkdir(pathName) && (errno != EEXIST)) // try to create subdir - ok if it already exists
	{
	char msg[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN + 128];

	sprintf(&msg[0], "KML Exporter: Unable to create directory '%s'. Terrain skipped", pathName);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, msg);
	success = 0;
	} // if

// XSI COLLADA importer can't handle paths correctly, so we put the images in the same directory as the models for now
if (success && Master->ExportTexture)
	{
	strcpy(tempFullPath, (char *)Master->OutPath.GetPath());
	strcpy(pathName, GlobalApp->MainProj->MungPath(tempFullPath));
	strcat(pathName, "/models/");
	if (PROJ_mkdir(pathName) && (errno != EEXIST)) // try to create subdir - ok if it already exists
		{
		char msg[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN + 128];

		sprintf(&msg[0], "KML Exporter: Unable to create directory '%s'. Terrain skipped", pathName);
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, msg);
		success = 0;
		} // if
	} // if

//fileType = WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN;
//rawTerrainFile = (*fileNamesCreated)->FindNameOfType(fileType);
//if (!(fRaw = PROJ_fopen(rawTerrainFile, "rb")))
//	success = 0;

if (success)
	{
	float cx, cy, sx, sy, xloc, yloc, xpos, ypos, xspace, yspace;

	fprintf(fKML, "\t<Folder>\n");
	fprintf(fKML, "\t\t<name>Terrain</name>\n");

	for (yTile = 0; yTile < Master->DEMTilesY; yTile++)
		{
		for (xTile = 0; xTile < Master->DEMTilesX; xTile++)
			{
			float du, dv, xdist, ydist;
			char baseTexName[64], fullPathAndName[512], pathName[512], tileName[32];

			if (fRaw)
				{
				long ndx;

				if (tiles)
					{
					Master->RBounds.DeriveTileCoords(Master->DEMResY, Master->DEMResX, Master->DEMTilesY, Master->DEMTilesX,
						yTile, xTile, Master->DEMTileOverlap);
					} // if
				else
					Master->RBounds.DeriveCoords(Master->DEMResY, Master->DEMResX);

				sx = (float)((xLimit * Master->RBounds.CellSizeX) * Master->ExportRefData.ExportLonScale);	// size x
				sy = (float)((yLimit * Master->RBounds.CellSizeY) * Master->ExportRefData.ExportLatScale);
				cx = sx * 0.5f;	// center x
				cy = sy * 0.5f;
				xspace = sx / (xLimit - 1);
				yspace = sy / (yLimit - 1);

				xloc = (float)((Master->RBounds.ULcenter.x + Master->RBounds.LRcenter.x) * 0.5);
				yloc = (float)((Master->RBounds.ULcenter.y + Master->RBounds.LRcenter.y) * 0.5);
				xdist = (tilesizex - 1) * xspace;
				ydist = (tilesizey - 1) * yspace;

				if (Master->ExportTexture)
					{
					strcpy(baseTexName, textureFile);
					(void)StripExtension(baseTexName);

					strcpy(fullPathAndName, Master->OutPath.GetPath());
					strcat(fullPathAndName, "/");
					strcat(fullPathAndName, textureFile);
					strcpy(pathName, Master->OutPath.GetPath());
					strcat(pathName, "/models");
					CopyExistingFile(fullPathAndName, pathName, textureFile);
					} // if
				else
					{
					strcpy(baseTexName, "Terrain");
					} // else

				sprintf(tileName, "models/DEM_%02dx_%02dy.dae", xTile, yTile);

				fprintf(fKML, "\t\t<Placemark>\n");
				fprintf(fKML, "\t\t\t<name>DEM_%02dx_%02dy</name>\n", xTile, yTile);
				// F2_Note: Compute LookAt per tile
				//fprintf(fKML, "\t\t\t<LookAt>\n");
				//fprintf(fKML, "\t\t\t\t<longitude>
				fprintf(fKML, "\t\t\t<Model>\n");
				fprintf(fKML, "\t\t\t\t<altitudeMode>absolute</altitudeMode>\n");
				fprintf(fKML, "\t\t\t\t<Location>\n");
				fprintf(fKML, "\t\t\t\t\t<longitude>%f</longitude>\n", xloc);
				fprintf(fKML, "\t\t\t\t\t<latitude>%f</latitude>\n", yloc);
				fprintf(fKML, "\t\t\t\t\t<altitude>%f</altitude>\n", -elevAdjust);
				fprintf(fKML, "\t\t\t\t</Location>\n");
				fprintf(fKML, "\t\t\t\t<Orientation>\n");
				fprintf(fKML, "\t\t\t\t\t<heading>0.0</heading>\n");
				fprintf(fKML, "\t\t\t\t\t<tilt>0.0</tilt>\n");
				fprintf(fKML, "\t\t\t\t\t<roll>0.0</roll>\n");
				fprintf(fKML, "\t\t\t\t</Orientation>\n");
				fprintf(fKML, "\t\t\t\t<Link>\n");
				fprintf(fKML, "\t\t\t\t\t<href>%s</href>\n", tileName);
				fprintf(fKML, "\t\t\t\t</Link>\n");
				fprintf(fKML, "\t\t\t</Model>\n");
				fprintf(fKML, "\t\t</Placemark>\n");

				if (fCOLLADA = PROJ_fopen(tileName, "w"))
					{
					sprintf(tileName, "DEM_%02dx_%02dy", xTile, yTile);

					C_prologue_asset(2);

					// library_images
					if (Master->ExportTexture)
						{
						sprintf(tempStr, "\t<library_images>\n");
						fputs(tempStr, fCOLLADA);
						sprintf(tempStr, "\t\t<image id=\"%s-image\">\n", baseTexName);
						fputs(tempStr, fCOLLADA);
						//sprintf(tempStr, "\t\t\t<init_from>/images/%s</init_from>\n", textureFile);
						sprintf(tempStr, "\t\t\t<init_from>%s</init_from>\n", textureFile);
						fputs(tempStr, fCOLLADA);
						sprintf(tempStr, "\t\t</image>\n");
						fputs(tempStr, fCOLLADA);
						sprintf(tempStr, "\t</library_images>\n");
						fputs(tempStr, fCOLLADA);
						} // if

					// library_materials
					sprintf(tempStr, "\t<library_materials>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t<material id=\"%s-material\">\n", baseTexName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t<instance_effect url=\"#%s-effect\"/>\n", baseTexName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t</material>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t</library_materials>\n");
					fputs(tempStr, fCOLLADA);

					// library_effects
					sprintf(tempStr, "\t<library_effects>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t<effect id=\"%s-effect\">\n", baseTexName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t<profile_COMMON>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t<technique sid=\"COMMON\">\n");
					fputs(tempStr, fCOLLADA);
					/*** Grrr...  GE doesn't seem to process this correctly
					sprintf(tempStr, "\t\t\t\t\t<constant>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t<emission>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t\t<texture texture=\"%s-image\" texcoord=\"UVcoords\"/>\n", baseTexName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t</emission>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t</constant>\n");
					fputs(tempStr, fCOLLADA);
					***/
					// F2_Note: Constant would be better than Phong for the terrain, but Phong works in the current GE build (4.0.1693 beta)
					sprintf(tempStr, "\t\t\t\t\t<phong>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t<emission>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t\t<color>0.0 0.0 0.0 1</color>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t</emission>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t<ambient>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t\t<color>0.0 0.0 0.0 1</color>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t</ambient>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t<diffuse>\n");
					fputs(tempStr, fCOLLADA);
					if (Master->ExportTexture)
						{
						sprintf(tempStr, "\t\t\t\t\t\t\t<texture texture=\"%s-image\" texcoord=\"UVcoords\"/>\n", baseTexName);
						} // if
					else
						{
						GradientCritter *gradCrit;
						MaterialEffect *mat;
						float r, g, b;

						// Set the poly color from the default ground effect's diffuse color
						if ((gradCrit = EffectsHost->DefaultGround->EcoMat.GetActiveNode()) && (mat = (MaterialEffect *)gradCrit->GetThing()))
							{
							r = (float)(mat->DiffuseColor.CurValue[0] * 255.99);
							g = (float)(mat->DiffuseColor.CurValue[1] * 255.99);
							b = (float)(mat->DiffuseColor.CurValue[2] * 255.99);
							} // if
						else
							{
							r = g = b = 127.0;
							} // else
						sprintf(tempStr, "\t\t\t\t\t\t\t<color>%f %f %f 1</color>\n", r, g, b);
						} // else
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t</diffuse>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t</phong>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t</technique>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t</profile_COMMON>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t</effect>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t</library_effects>\n");
					fputs(tempStr, fCOLLADA);

					// library_geometries
					sprintf(tempStr, "\t<library_geometries>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t<geometry id=\"%s-geometry\">\n", tileName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t<mesh>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t<source id =\"%s-geometry-position\">\n", tileName);
					fputs(tempStr, fCOLLADA);
					// position (verts)
					sprintf(tempStr, "\t\t\t\t\t<float_array id=\"%s-geometry-position-array\" count=\"%d\"> ",
						tileName, xLimit * yLimit * 3);
					fputs(tempStr, fCOLLADA);
					for (ypos = cy, y = 0; y < yLimit; y++)
						{
						for (xpos = -cx, x = xLimit; x > 0; x--)
							{
							float elev;

							fread(&elev, sizeof(float), 1, fRaw);
							elev = elev + elevAdjust;
							fprintf(fCOLLADA, "%f %f %f ", xpos, ypos, elev);
							xpos += xspace;
							} // for x
						ypos -= yspace;	// with Z_UP, the In Axis (Y) is negative {or not!}
						} // for y
					sprintf(tempStr, "</float_array>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t<technique_common>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t<accessor source=\"#%s-geometry-position-array\" count=\"%d\" stride=\"3\">\n",
						tileName, xLimit * yLimit);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t\t<param name=\"X\" type=\"float\"/>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t\t<param name=\"Y\" type=\"float\"/>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t\t<param name=\"Z\" type=\"float\"/>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t</accessor>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t</technique_common>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t</source>\n");
					fputs(tempStr, fCOLLADA);
					// no normal array (flat shaded)
					// UV array
					sprintf(tempStr, "\t\t\t\t<source id=\"%s-geometry-uv\">\n", tileName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t<float_array id=\"%s-geometry-uv-array\" count=\"%d\"> ",
						tileName, xLimit * yLimit * 2);
					fputs(tempStr, fCOLLADA);
					du = 1.0f / (xLimit - 1);
					dv = 1.0f / (yLimit - 1);
					for (ypos = 1.0, y = yLimit; y > 0; y--)
						{
						for (xpos = 0.0, x = xLimit; x > 0; x--)
							{
							fprintf(fCOLLADA, "%f %f ", xpos, ypos);
							xpos += du;
							} // for x
						ypos -= dv;
						} // for y
					sprintf(tempStr, "</float_array>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t<technique_common>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t<accessor source=\"#%s-geometry-uv-array\" count=\"%d\" stride=\"2\">\n",
						tileName, xLimit * yLimit);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t\t<param name=\"S\" type=\"float\"/>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t\t<param name=\"T\" type=\"float\"/>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t</accessor>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t</technique_common>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t</source>\n");
					fputs(tempStr, fCOLLADA);

					// vertices
					sprintf(tempStr, "\t\t\t\t<vertices id=\"%s-geometry-vertex\">\n", tileName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t<input semantic=\"POSITION\" source=\"#%s-geometry-position\"/>\n", tileName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t</vertices>\n");
					fputs(tempStr, fCOLLADA);

					// triangles
					sprintf(tempStr, "\t\t\t\t<triangles material=\"%s-material\" count=\"%d\">\n",
						baseTexName, xLimit * yLimit * 2);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t<input semantic=\"VERTEX\" source=\"#%s-geometry-vertex\" offset=\"0\"/>\n", tileName);
					fputs(tempStr, fCOLLADA);
					// since our vertex & uv arrays are coincident, share the p elements
					sprintf(tempStr, "\t\t\t\t\t<input semantic=\"TEXCOORD\" source=\"#%s-geometry-uv\" offset=\"0\" set=\"0\"/>\n", tileName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t<p> ");
					fputs(tempStr, fCOLLADA);
					for (y = 0; y < yLimit; y++)
						{
						ndx = y * xLimit;
						for (x = 0; x < xLimit; x++)
							{
							#ifdef GE_CLOCKWISE_WINDING
							// create UR triangle (CW)
							fprintf(fCOLLADA, "%d %d %d ", ndx, ndx + 1, ndx + xLimit + 1);
							// create LL triangle (CW)
							fprintf(fCOLLADA, "%d %d %d ", ndx, ndx + xLimit + 1, ndx + xLimit);
							ndx++;
							#else  // GE_CLOCKWISE_WINDING
							// create UR triangle (CCW)
							fprintf(fCOLLADA, "%d %d %d ", ndx, ndx + xLimit + 1, ndx + 1);
							// create LL triangle (CCW)
							fprintf(fCOLLADA, "%d %d %d ", ndx, ndx + xLimit, ndx + xLimit + 1);
							ndx++;
							#endif // GE_CLOCKWISE_WINDING
							} // for x
						} // for y
					sprintf(tempStr, "</p>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t</triangles>\n");
					fputs(tempStr, fCOLLADA);

					sprintf(tempStr, "\t\t\t</mesh>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t</geometry>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t</library_geometries>\n");
					fputs(tempStr, fCOLLADA);

					// library_visual_scenes
					sprintf(tempStr, "\t<library_visual_scenes>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t<visual_scene id=\"Scene_Express_Scene\">\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t<node id=\"%s\">\n", tileName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t<instance_geometry url=\"#%s-geometry\"/>\n", tileName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t</node>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t</visual_scene>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t</library_visual_scenes>\n");
					fputs(tempStr, fCOLLADA);

					// scene
					sprintf(tempStr, "\t<scene>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t<instance_visual_scene url=\"#Scene_Express_Scene\"/>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t</scene>\n");
					fputs(tempStr, fCOLLADA);

					sprintf(tempStr, "</COLLADA>\n");
					if (fputs(tempStr, fCOLLADA) >= 0)
						success = 1;
					fclose(fCOLLADA);

					if (fRaw)
						{
						fclose(fRaw);
						fRaw = NULL;
						} // if

					if (tiles)
						{
						fileType = WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN;
						rawTerrainFile = (*fileNamesCreated)->FindNextNameOfType(fileType, rawTerrainFile);
						fRaw = PROJ_fopen(rawTerrainFile, "rb");

						fileType = WCS_EXPORTCONTROL_FILETYPE_TEX1;
						textureFile = (*fileNamesCreated)->FindNextNameOfType(fileType, textureFile);
						} // if tiles
					} // if fCOLLADA
				} // if fRaw
			} // for xTile
		} // for yTile

	fprintf(fKML, "\t</Folder>\n");
	} // if success

return success;
#else // GORILLA
return 1;
#endif // GORILLA
} // ExportFormatCOLLADA::ExportTerrain

/*===========================================================================*/

int ExportFormatCOLLADA::ExportVectors(void)
{
#ifdef GORILLA
double vLat, vLon, vElev;
Joe *CurJoe;
VectorExportItem *VEI;
VectorPoint *vert;
float vWidth;
long n;

sprintf(tempStr, "%s<Folder>\n%s\t<name>Vectors</name>\n", indentStr, indentStr);
fputs(tempStr, fKML);
IndentMore();

for (n = 0; n < Master->NumVecInstances; n++)
	{
	VEI = &Master->VecInstanceList[n];
	CurJoe = VEI->MyJoe;
	sprintf(tempStr, "%s<Placemark>\n%s\t<name>%s</name>\n%s\t<Style>\n%s\t\t<LineStyle>\n",
		indentStr, indentStr, CurJoe->GetBestName(), indentStr, indentStr);
	fputs(tempStr, fKML);
	IndentMore();
	sprintf(tempStr, "%s\t\t<color>ff%02x%02x%02x</color>\n", indentStr, CurJoe->Blue(), CurJoe->Green(), CurJoe->Red());
	fputs(tempStr, fKML);
	vWidth = (float)(CurJoe->GetLineWidth() * Master->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_VECWIDTHMULT].CurValue);
	if (vWidth > 4.0f)
		vWidth = 4.0f;
	sprintf(tempStr, "%s\t\t<width>%f</width>\n%s\t</LineStyle>\n%s</Style>\n", indentStr, vWidth, indentStr, indentStr);
	fputs(tempStr, fKML);
	sprintf(tempStr, "%s<LineString>\n%s\t<tessellate>1</tessellate>\n", indentStr, indentStr);
	fputs(tempStr, fKML);
	sprintf(tempStr, "%s\t<coordinates>\n", indentStr);
	fputs(tempStr, fKML);
	vert = VEI->Points;
	while (vert)
		{
		vLat = vert->Latitude + Master->ExportRefData.ExportRefLat;
		vLon = vert->Longitude + Master->ExportRefData.ExportRefLon;
		vElev = vert->Elevation + Master->ExportRefData.RefElev + Master->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_VECELEVADD].CurValue;
		sprintf(tempStr, "%s\t\t%f, %f, %f\n", indentStr, vLon, vLat, vElev);
		fputs(tempStr, fKML);
		vert = vert->Next;
		} // while vert
	sprintf(tempStr, "%s\t</coordinates>\n", indentStr);
	fputs(tempStr, fKML);
	sprintf(tempStr, "%s</LineString>\n", indentStr);
	fputs(tempStr, fKML);
	IndentLess();
	sprintf(tempStr, "%s</Placemark>\n", indentStr);
	fputs(tempStr, fKML);
	} // while VEI

IndentLess();
sprintf(tempStr, "%s</Folder>\n", indentStr);
fputs(tempStr, fKML);

return 1;
#else // GORILLA
return 1;
#endif // GORILLA

} // ExportFormatCOLLADA::ExportVectors

/*===========================================================================*/

void ExportFormatCOLLADA::IndentLess(void)
{
unsigned long i;

if (curIndent > 0)
	curIndent--;
i = curIndent;
indentStr[i] = NULL;

} // ExportFormatCOLLADA::IndentLess

/*===========================================================================*/

void ExportFormatCOLLADA::IndentMore(void)
{
unsigned long i = curIndent;

curIndent++;
indentStr[i] = '\t';
indentStr[i+1] = 0;

} // ExportFormatCOLLADA::IndentMore

/*===========================================================================*/

int ExportFormatCOLLADA::KML_Point(double &px, double &py, double &pz)
{

#ifdef GORILLA
sprintf(tempStr, "%s<Point>\n%s\t<extrude>1</extrude>\n%s\t<tessellate>0</tessellate>\n", indentStr, indentStr, indentStr);
fputs(tempStr, fKML);
sprintf(tempStr, "%s\t<coordinates>%f,%f,%f</coordinates>\n%s</Point>\n", indentStr, px, py, pz, indentStr);
fputs(tempStr, fKML);
#endif // GORILLA

return 1;

} // ExportFormatCOLLADA::KML_Point

/*===========================================================================*/

int ExportFormatCOLLADA::PackageExport(NameList **fileNamesCreated)
{
int success = 0;
PathAndFile paf;
char tempFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

//warningCheck = false;

strcpy(tempFullPath, (char *)Master->OutPath.GetPath());

paf.SetPath((char *)Master->OutPath.GetPath());
paf.SetName((char *)Master->OutPath.GetName());
paf.GetFramePathAndName(tempFullPath, ".dae", 0, WCS_PATHANDFILE_PATH_PLUS_NAME_LEN, 0);

if (fDAE = PROJ_fopen(tempFullPath, "w"))
	success = 1;

if (success)
	C_prologue_asset(1);	// F2_NOTE: get up axis from advanced config in future

// library_animations

// library_animation_clips

// library_cameras
if (success && Master->ExportCameras)
	success = ProcessLibraryCameras();

// library_effects
if (success)
	success = ProcessLibraryEffects(fileNamesCreated);

// library_geometries
if (success)
	success = ProcessLibraryGeometries(fileNamesCreated);

// library images
if (success)
	success = ProcessLibraryImages(fileNamesCreated);

// library_lights
if (success && Master->ExportLights)
	success = ProcessLibraryLights();

// library_materials
if (success)
	success = ProcessLibraryMaterials(fileNamesCreated);

/***
				// library effects
				fprintf(fDAE, "\t<library_effects>\n");
				for (matCt = 0; matCt < object3D->NumMaterials; matCt++)
					ProcessLibraryEffects(matCt, fDAE, object3D);
				fprintf(fDAE, "\t</library_effects>\n");

				// library geometries
				fprintf(fDAE, "\t<library_geometries>\n");
				ProcessGeometries(fDAE, object3D);
				fprintf(fDAE, "\t</library_geometries>\n");

				// library images
				if (object3D->VertexUVWAvailable)
					{
					fprintf(fDAE, "\t<library_images>\n");
					for (matCt = 0; matCt < object3D->NumMaterials; matCt++)
						ProcessLibraryImages(matCt, fDAE, object3D);
					fprintf(fDAE, "\t</library_images>\n");
					} // if

				// library materials
				fprintf(fDAE, "\t<library_materials>\n");
				for (matCt = 0; matCt < object3D->NumMaterials; matCt++)
					ProcessLibraryMaterials(matCt, fDAE, object3D);
				fprintf(fDAE, "\t</library_materials>\n");
***/

// scene
fprintf(fDAE, "\t<scene>\n");
fprintf(fDAE, "\t\t<instance_visual_scene url=\"#Scene_Express_Scene\"/>\n");
fprintf(fDAE, "\t</scene>\n");

//if (success && Master->ExportTerrain)
//	success = ExportTerrain(fileNamesCreated);

//if (success && Master->ExportVectors && Master->VecInstanceList)
//	success = ExportVectors();

//if (success && Master->ExportFoliage)
//	success = ExportFoliage(fileNamesCreated);

//if (success && Master->ExportLabels)
//	success = ExportLabels(fileNamesCreated);

//if (success && Master->ObjectInstanceList)
//	success = Export3DObjs(fileNamesCreated);

if (fDAE)
	{
	fputs("</COLLADA>\n", fDAE);
	fclose(fDAE);
	} // if

Cleanup(fileNamesCreated);

return success;

} // ExportFormatCOLLADA::PackageExport

/*===========================================================================*/

int ExportFormatCOLLADA::Process3DObjectImages(void)
{
MaterialEffect *mat;
Object3DInstance *curInstance = Master->ObjectInstanceList;
Object3DEffect *curObj;
Raster *rast;
RootTexture *rootTex;
Texture *uvTex;
int success = 1;
static long createDir = 1;	// we need to create the images subdir the first time through here
long diffuseAlphaAvailable, useImage;
char fullPathAndName[512], pathName[512], textureName[256];

while (curInstance && curInstance->IsBounded(Master->FetchRBounds()))
	{
	curObj = curInstance->MyObj;
	if (curObj->Enabled)
		{
		diffuseAlphaAvailable = useImage = 0;
		for (long matCt = 0; matCt < curObj->NumMaterials; matCt++)
			{
			if (! curObj->NameTable[matCt].Mat)
				curObj->NameTable[matCt].Mat = (MaterialEffect *)GlobalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, curObj->NameTable[matCt].Name);

			if (mat = curObj->NameTable[matCt].Mat)
				{
				if (curObj->VertexUVWAvailable)
					{
					if (rootTex = mat->GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
						{
						if (uvTex = rootTex->Tex)
							{
							if (uvTex->TexType == WCS_TEXTURE_TYPE_UVW)
								{
								if (uvTex->Img && (rast = uvTex->Img->GetRaster()))
									{
									if (CheckAndShortenFileName(textureName, rast->GetPath(), rast->GetName(), 12))
										rast->PAF.SetName(textureName);
									useImage = 1;
									} // if
								} // if
							} // if
						} // if
					if (rootTex = mat->GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY))
						{
						if (uvTex = rootTex->Tex)
							{
							if (uvTex->TexType == WCS_TEXTURE_TYPE_UVW)
								{
								if (uvTex->Img && (rast = uvTex->Img->GetRaster()))
									{
									if (CheckAndShortenFileName(textureName, rast->GetPath(), rast->GetName(), 12))
										rast->PAF.SetName(textureName);
									useImage = 1;
									} // if
								} // if
							} // if
						} // if
					else if (diffuseAlphaAvailable)	// F2 Note: This isn't being set anywhere!
						{
						if (rootTex = mat->GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
							{
							if (uvTex = rootTex->Tex)
								{
								if (uvTex->TexType == WCS_TEXTURE_TYPE_UVW)
									{
									if (uvTex->Img && (rast = uvTex->Img->GetRaster()))
										{
										if (CheckAndShortenFileName(textureName, rast->GetPath(), rast->GetName(), 12))
											rast->PAF.SetName(textureName);
										useImage = 1;
										} // if
									} // if
								} // if
							} // if
						} // else
					} // if
				} // if
			else
				{
				sprintf(textureName, "OM%06d", matCt);
				useImage = 1;
				} // else
			} // for matCt

		if (useImage)
			{
			char noExtenName[256];

			if (sectionInactive)
				{
				sprintf(tempStr, "\t<library_images>\n");
				fputs(tempStr, fDAE);
				sectionInactive = 0;
				} // if

			rast->GetPathAndName(fullPathAndName);
			// our images are going to end up in a subdirectory called images
			strcpy(pathName, Master->OutPath.GetPath());
			strcat(pathName, "/images");
			if (createDir)
				{
				PROJ_mkdir(pathName);
				createDir = 0;
				} // if
			CopyExistingFile(fullPathAndName, pathName, textureName);

			strcpy(noExtenName, textureName);
			(void)StripExtension(noExtenName);
			fprintf(fDAE, "\t\t<image id=\"%s-image\">\n", noExtenName);
			fprintf(fDAE, "\t\t\t<init_from>images/%s</init_from>\n", textureName);
			success = fprintf(fDAE, "\t\t</image>\n");
			} // if useImage

		} // if Enabled

	curInstance = curInstance->Next;
	} // while curInstance

return (success >= 0);

} // ExportFormatCOLLADA::Process3DObjectImages

/*===========================================================================*/

int ExportFormatCOLLADA::ProcessFoliageLabelImages(NameList **fileNamesCreated)
{
const char *imageName;
int success = 1;
long createDir = 1;
char id[128], fullPathAndName[512], pathName[512];

for (imageName = (*fileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_FOLIAGETEX); imageName; imageName = (*fileNamesCreated)->FindNextNameOfType(WCS_EXPORTCONTROL_FILETYPE_FOLIAGETEX, imageName))
	{
	strmfp(fullPathAndName, Master->OutPath.GetPath(), imageName);

	strcpy(id, imageName);
	(void)StripExtension(id);
	strcpy(id, CreateValidID(id));
	sprintf(tempStr, "\t\t<image id=\"%s-image\">\n", id);
	fputs(tempStr, fDAE);
	sprintf(tempStr, "\t\t\t<init_from>images/%s</init_from>\n", imageName);
	fputs(tempStr, fDAE);
	sprintf(tempStr, "\t\t</image>\n");
	fputs(tempStr, fDAE);

	strcpy(pathName, Master->OutPath.GetPath());
	strcat(pathName, "/images");
	if (createDir)
		{
		PROJ_mkdir(pathName);
		createDir = 0;
		} // if
	CopyExistingFile(fullPathAndName, pathName, imageName);
	} // for

return success;

#ifdef GORILLA
FILE *ffile;
long fileType;
int Success = 1;
const char *indexFileName, *dataFileName;
RealtimeFoliageIndex index;
RealtimeFoliageCellData rfcd;
RealtimeFoliageData folData;
char fileName[128], fullPathAndName[512], pathName[512];
char testFileVersion;

for (cleanupFileName = (*fileNamesCreated)->FindNameOfType(WCS_EXPORTCONTROL_FILETYPE_FOLIAGETEX); cleanupFileName; cleanupFileName = (*fileNamesCreated)->FindNextNameOfType(WCS_EXPORTCONTROL_FILETYPE_FOLIAGETEX, cleanupFileName))
	{
	strmfp(fullFileName, Master->OutPath.GetPath(), cleanupFileName);
	PROJ_remove(fullFileName);
	} // for


// files may not exist. If they don't then user must not have chosen foliage export features
fileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILEIDX;
if (indexFileName = (*fileNamesCreated)->FindNameOfType(fileType))
	{
	fileType = WCS_EXPORTCONTROL_FILETYPE_FOLFILE;
	if (dataFileName = (*fileNamesCreated)->FindNameOfType(fileType))
		{
		// find and open the index file
		strmfp(fileName, Master->OutPath.GetPath(), indexFileName);
		if (ffile = PROJ_fopen(fileName, "rb"))
			{
			// read file descriptor, no need to keep it around unless you want to
			fgets(fileName, 256, ffile);
			// version
			fread((char *)&index.FileVersion, sizeof(char), 1, ffile);
			// number of files
			fread((char *)&index.NumCells, sizeof(long), 1, ffile);
			// reference XYZ
			fread((char *)&index.RefXYZ[0], sizeof(double), 1, ffile);
			fread((char *)&index.RefXYZ[1], sizeof(double), 1, ffile);
			fread((char *)&index.RefXYZ[2], sizeof(double), 1, ffile);

			if (index.NumCells > 0)
				{
				// only one cell data entry is provided
				if (index.CellDat = &rfcd)
					{
					// file name
					fgets(index.CellDat->FileName, 64, ffile);
					// center XYZ
					fread((char *)&index.CellDat->CellXYZ[0], sizeof(double), 1, ffile);
					fread((char *)&index.CellDat->CellXYZ[1], sizeof(double), 1, ffile);
					fread((char *)&index.CellDat->CellXYZ[2], sizeof(double), 1, ffile);
					// half cube cell dimension
					fread((char *)&index.CellDat->CellRad, sizeof(double), 1, ffile);
					// number of trees in file
					fread((char *)&index.CellDat->DatCt, sizeof(long), 1, ffile);
					} // if
				} // if some cells to read
			fclose(ffile);

			if ((index.NumCells > 0) && (index.CellDat->DatCt > 0))
				{
				long createDir = 1, proceed = 1;

				strmfp(fileName, Master->OutPath.GetPath(), dataFileName);
				if (ffile = PROJ_fopen(fileName, "rb"))
					{
					fgets(fileName, 64, ffile);
					// version
					fread((char *)&testFileVersion, sizeof(char), 1, ffile);
					strcpy(pathName, Master->OutPath.GetPath());
					strcat(pathName, "/images");
					if (PROJ_mkdir(pathName) && (errno != EEXIST)) // try to create subdir - ok if it already exists
						{
						char msg[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN + 128];

						proceed = 0;	// we couldn't make our subdir, so abort processing on this object
						sprintf(&msg[0], "%s: Unable to create directory '%s'. Skipping foliage", msgCE, pathName);
						GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, msg);
						} // if
		
					if (proceed && (testFileVersion == index.FileVersion))
						{
						Raster *curRast;
						char *dot3;
						FoliagePreviewData pointData;
						long datPt;
						char id[128], imageName[128];

						dot3 = ImageSaverLibrary::GetDefaultExtension(Master->FoliageImageFormat);
						for (datPt = 0; datPt < index.CellDat->DatCt; datPt++)
							{
							if (folData.ReadFoliageRecord(ffile, index.FileVersion))
								{
								folData.InterpretFoliageRecord(NULL, GlobalApp->AppImages, &pointData);
								curRast = Images->FindByID(folData.ElementID);
								if (curRast)
									{
									strcpy(imageName, curRast->GetUserName());
									StripExtension(imageName);
									strcpy(id, CreateValidID(imageName));
									strcat(id, "_Fol");
									strcat(imageName, "_Fol");
									strcat(imageName, dot3);
									} // if
								} // if

							if (curRast)
								{
								sprintf(tempStr, "\t\t<image id=\"%s-image\">\n", id);
								fputs(tempStr, fDAE);
								sprintf(tempStr, "\t\t\t<init_from>images/%s</init_from>\n", imageName);
								fputs(tempStr, fDAE);
								sprintf(tempStr, "\t\t</image>\n");
								fputs(tempStr, fDAE);

								strcpy(pathName, Master->OutPath.GetPath());
								strcat(pathName, "/images");
								if (createDir)
									{
									PROJ_mkdir(pathName);
									createDir = 0;
									} // if
								CopyExistingFile(fullPathAndName, pathName, imageName);
								} // if
							} // if
						} // if
					fclose(ffile);
					} // if
				} // if
			} // if
		} // if
	} // if

// must do this as it's not dynamically allocated and RealtimeFoliageIndex
// destructor will blow chunks if we don't
index.CellDat = NULL;

return (Success >= 0);

#endif // GORILLA

} // ExportFormatCOLLADA::ProcessFoliageLabelImages

/*===========================================================================*/

int ExportFormatCOLLADA::ProcessLibraryGeometries(NameList **fileNamesCreated)
{
#ifdef GORILLA

curObj->CopyCoordinatesToXYZ();

fprintf(fDAE, "\t\t<geometry id=\"%s-geometry\">\n", objName);
fprintf(fDAE, "\t\t\t<mesh>\n");

// geometry
fprintf(fDAE, "\t\t\t\t<source id=\"%s-geometry-position\">\n", objName);
fprintf(fDAE, "\t\t\t\t\t<float_array id=\"%s-geometry-position-array\" count=\"%d\">", objName, curObj->NumVertices * 3);
for (long vertCt = 0; vertCt < curObj->NumVertices; vertCt++)
	{
	fprintf(fDAE, " %f %f %f", (float)curObj->Vertices[vertCt].xyz[0], (float)curObj->Vertices[vertCt].xyz[1], (float)-curObj->Vertices[vertCt].xyz[2]);
	} // for
fprintf(fDAE, " </float_array>\n");
fprintf(fDAE, "\t\t\t\t\t<technique_common>\n");
fprintf(fDAE, "\t\t\t\t\t\t<accessor source=\"#%s-geometry-position-array\" count=\"%d\" stride=\"3\">\n", objName, curObj->NumVertices);
fprintf(fDAE, "\t\t\t\t\t\t\t<param name=\"X\" type=\"float\"/>\n");
fprintf(fDAE, "\t\t\t\t\t\t\t<param name=\"Y\" type=\"float\"/>\n");
fprintf(fDAE, "\t\t\t\t\t\t\t<param name=\"Z\" type=\"float\"/>\n");
fprintf(fDAE, "\t\t\t\t\t\t</accessor>\n");
fprintf(fDAE, "\t\t\t\t\t</technique_common>\n");
fprintf(fDAE, "\t\t\t\t</source>\n");

// normals
fprintf(fDAE, "\t\t\t\t<source id=\"%s-geometry-normal\">\n", objName);
fprintf(fDAE, "\t\t\t\t\t<float_array id=\"%s-geometry-normal-array\" count=\"%d\">", objName, curObj->NumPolys * 3);
for (long polyCt = 0; polyCt < curObj->NumPolys; polyCt++)
	{
	curObj->Polygons[polyCt].Normalize(curObj->Vertices);
	fprintf(fDAE, " %f %f %f", (float)curObj->Polygons[polyCt].Normal[0], (float)curObj->Polygons[polyCt].Normal[1], (float)curObj->Polygons[polyCt].Normal[2]);
	} // for
fprintf(fDAE, " </float_array>\n");
fprintf(fDAE, "\t\t\t\t\t<technique_common>\n");
fprintf(fDAE, "\t\t\t\t\t\t<accessor source=\"#%s-geometry-normal-array\" count=\"%d\" stride=\"3\">\n", objName, curObj->NumPolys);
fprintf(fDAE, "\t\t\t\t\t\t\t<param name=\"X\" type=\"float\"/>\n");
fprintf(fDAE, "\t\t\t\t\t\t\t<param name=\"Y\" type=\"float\"/>\n");
fprintf(fDAE, "\t\t\t\t\t\t\t<param name=\"Z\" type=\"float\"/>\n");
fprintf(fDAE, "\t\t\t\t\t\t</accessor>\n");
fprintf(fDAE, "\t\t\t\t\t</technique_common>\n");
fprintf(fDAE, "\t\t\t\t</source>\n");

// uv
if (curObj->VertexUVWAvailable)
	{
	ObjectPerVertexMap *uvMap;

	uvMap = &curObj->UVWTable[0];
	fprintf(fDAE, "\t\t\t\t<source id=\"%s-geometry-uv\">\n", objName);
	fprintf(fDAE, "\t\t\t\t\t<float_array id=\"%s-geometry-uv-array\" count=\"%d\">", objName, uvMap->NumNodes * 2);
	for (long nodeCt = 0; nodeCt < uvMap->NumNodes; nodeCt++)
		{
		fprintf(fDAE, " %f %f", (float)uvMap->CoordsArray[0][nodeCt], (float)uvMap->CoordsArray[1][nodeCt]);
		} // for
	fprintf(fDAE, " </float_array>\n");
	fprintf(fDAE, "\t\t\t\t\t<technique_common>\n");
	fprintf(fDAE, "\t\t\t\t\t\t<accessor source=\"#%s-geometry-uv-array\" count=\"%d\" stride=\"2\">\n", objName, uvMap->NumNodes);
	fprintf(fDAE, "\t\t\t\t\t\t\t<param name=\"S\" type=\"float\"/>\n");
	fprintf(fDAE, "\t\t\t\t\t\t\t<param name=\"T\" type=\"float\"/>\n");
	fprintf(fDAE, "\t\t\t\t\t\t</accessor>\n");
	fprintf(fDAE, "\t\t\t\t\t</technique_common>\n");
	fprintf(fDAE, "\t\t\t\t</source>\n");
	} // if uv


fprintf(fDAE, "\t\t\t\t<vertices id=\"%s-geometry-vertex\">\n", objName);
fprintf(fDAE, "\t\t\t\t\t<input semantic=\"POSITION\" source=\"#%s-geometry-position\"/>\n", objName);
fprintf(fDAE, "\t\t\t\t</vertices>\n");
ProcessPolys(curObj);
fprintf(fDAE, "\t\t\t</mesh>\n");
fprintf(fDAE, "\t\t</geometry>\n");

#else // GORILLA
return 1;
#endif // GORILLA

} // ExportFormatCOLLADA::ProcessLibraryGeometries

/*===========================================================================*/

int ExportFormatCOLLADA::ProcessLibraryImages(NameList **fileNamesCreated)
{
int success = 1;

sectionInactive = 1;
if (Master->ObjectInstanceList)
	success = Process3DObjectImages();

if (success && Master->ExportTexture)
	success = ProcessTerrainImages(fileNamesCreated);

if (success && (Master->ExportFoliage || Master->ExportLabels))
	success = ProcessFoliageLabelImages(fileNamesCreated);

if (! sectionInactive)
	{
	sprintf(tempStr, "\t</library_images>\n");
	fputs(tempStr, fDAE);
	} // if

return (success >= 0);

} // ExportFormatCOLLADA::ProcessLibraryImages

/*===========================================================================*/

int ExportFormatCOLLADA::ProcessLibraryMaterials(NameList **fileNamesCreated)
{
#ifdef GORILLA
MaterialEffect *mat;
//char textureName[256];

if (! curObj->NameTable[matCt].Mat)
	curObj->NameTable[matCt].Mat = (MaterialEffect *)GlobalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, curObj->NameTable[matCt].Name);

if (mat = curObj->NameTable[matCt].Mat)
	{
	FixMatName(mat->Name);
	fprintf(fDAE, "\t\t<material id=\"%s-material\">\n", matName);
	fprintf(fDAE, "\t\t\t<instance_effect url=\"#%s-effect\"/>\n", matName);
	fprintf(fDAE, "\t\t</material>\n");
	} // if
#else // GORILLA
return 1;
#endif // GORILLA

} // ExportFormatCOLLADA::ProcessLibraryMaterials

/*===========================================================================*/

int ExportFormatCOLLADA::ProcessLibraryEffects(NameList **fileNamesCreated)
{
#ifdef GORILLA
MaterialEffect *mat;

if (! curObj->NameTable[matCt].Mat)
	curObj->NameTable[matCt].Mat = (MaterialEffect *)GlobalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, curObj->NameTable[matCt].Name);

if (mat = curObj->NameTable[matCt].Mat)
	{
	float fval0, fval1, fval2;//, fval3;
	char tab6[] = "\t\t\t\t\t\t", tab7[] = "\t\t\t\t\t\t\t";

	if (mat->Shading) // if it's not invisible
		{
		// intro
		FixMatName(mat->Name);
		fprintf(fDAE, "\t\t<effect id=\"%s-effect\">\n", matName);
		fprintf(fDAE, "\t\t\t<profile_COMMON>\n\t\t\t\t<technique sid=\"COMMON\">\n");

		// light model
		if (mat->Shading == 1)		// flat
			{
			fprintf(fDAE, "\t\t\t\t\t<lambert>\n");
			// emission
			fval0 = (float)curObj->NameTable[matCt].Mat->DiffuseColor.CurValue[0];
			fval1 = (float)curObj->NameTable[matCt].Mat->DiffuseColor.CurValue[1];
			fval2 = (float)curObj->NameTable[matCt].Mat->DiffuseColor.CurValue[2];
			fprintf(fDAE, "%s<emission>\n", tab6);
			fprintf(fDAE, "%s<color> %f %f %f 1</color>\n", tab7, fval0, fval1, fval2);
			fprintf(fDAE, "%s</emission>\n", tab6);
			} // if
		else if (mat->Shading == 2)	// phong
			{
			fprintf(fDAE, "\t\t\t\t\t<phong>\n");
			// emission
			fval0 = (float)curObj->NameTable[matCt].Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].CurValue;
			fprintf(fDAE, "%s<emission>\n", tab6);
			fprintf(fDAE, "%s<color> %f %f %f 1</color>\n", tab7, fval0, fval0, fval0);
			fprintf(fDAE, "%s</emission>\n", tab6);

			// ambient
			fprintf(fDAE, "%s<ambient>\n", tab6);
			fprintf(fDAE, "%s<color>0.0 0.0 0.0 1</color>\n", tab7);
			fprintf(fDAE, "%s</ambient>\n", tab6);

			// diffuse
			if ((matCt == 0) && curObj->VertexUVWAvailable)
				{
				fprintf(fDAE, "%s<diffuse>\n", tab6);
				fprintf(fDAE, "%s<texture texture=\"%s-image\" texcoord=\"diffuse_TEXCOORD\"></texture>\n", tab7, mat->Name);
				fprintf(fDAE, "%s</diffuse>\n", tab6);
				} // if textured
			else
				{
				fval0 = (float)curObj->NameTable[matCt].Mat->DiffuseColor.CurValue[0];
				fval1 = (float)curObj->NameTable[matCt].Mat->DiffuseColor.CurValue[1];
				fval2 = (float)curObj->NameTable[matCt].Mat->DiffuseColor.CurValue[2];
				fprintf(fDAE, "%s<diffuse>\n", tab6);
				fprintf(fDAE, "%s<color> %f %f %f 1</color>\n", tab7, fval0, fval1, fval2);
				fprintf(fDAE, "%s</diffuse>\n", tab6);
				} // else untextured

			/***
			// specular
			fval0 = (float)curObj->NameTable[matCt].Mat->SpecularColor.CurValue[0];
			fval1 = (float)curObj->NameTable[matCt].Mat->SpecularColor.CurValue[1];
			fval2 = (float)curObj->NameTable[matCt].Mat->SpecularColor.CurValue[2];
			fval3 = (float)curObj->NameTable[matCt].Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARCOLORPCT].CurValue;
			if (fval3 > 0.0f)
				{
				fprintf(fDAE, "%s<specular>\n", tab6);
				fprintf(fDAE, "%s<color> %f %f %f 1</color>\n", tab7, fval0 * fval3, fval1 * fval3, fval2 * fval3);
				fprintf(fDAE, "%s</specular>\n", tab6);

				// shininess
				fval0 = (float)curObj->NameTable[matCt].Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].CurValue;
				fprintf(fDAE, "%s<shininess>\n", tab6);
				fprintf(fDAE, "%s<float>%f</float>\n", tab7, fval0);
				fprintf(fDAE, "%s</shininess>\n", tab6);
				} // if
			***/

			/***
			// specular - values suitable for Google Earth
			fval0 = 0.33f;
			fprintf(fDAE, "%s<specular>\n", tab6);
			fprintf(fDAE, "%s<color> %f %f %f 1</color>\n", tab7, fval0, fval0, fval0);
			fprintf(fDAE, "%s</specular>\n", tab6);

			// shininess - values suitable for Google Earth
			fval0 = 20.0f;
			fprintf(fDAE, "%s<shininess>\n", tab6);
			fprintf(fDAE, "%s<float>%f</float>\n", tab7, fval0);
			fprintf(fDAE, "%s</shininess>\n", tab6);
			***/
			} // else if

		/***
		// reflectivity
		fval0 = (float)curObj->NameTable[matCt].Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY].CurValue;
		fprintf(fDAE, "%s<reflectivity>\n", tab6);
		fprintf(fDAE, "%s<float>%f</float>\n", tab7, fval0);
		fprintf(fDAE, "%s</reflectivity>\n", tab6);
		***/

		/***
		// reflectivity - values suitable for Google Earth
		fval0 = 0.1f;
		fprintf(fDAE, "%s<reflectivity>\n", tab6);
		fprintf(fDAE, "%s<float>%f</float>\n", tab7, fval0);
		fprintf(fDAE, "%s</reflectivity>\n", tab6);

		// transparent - values suitable for Google Earth
		fprintf(fDAE, "%s<transparent>\n", tab6);
		fprintf(fDAE, "%s<color>1 1 1 1</color>\n", tab7);
		fprintf(fDAE, "%s</transparent>\n", tab6);

		// transparency
		fval0 = (float)curObj->NameTable[matCt].Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue;
		fprintf(fDAE, "%s<transparency>\n", tab6);
		fprintf(fDAE, "%s<float>%f</float>\n", tab7, fval0);
		fprintf(fDAE, "%s</transparency>\n", tab6);
		***/

		// light model - pt2
		if (mat->Shading == 1)		// flat
			fprintf(fDAE, "\t\t\t\t\t</lambert>\n");
		else if (mat->Shading == 2)
			fprintf(fDAE, "\t\t\t\t\t</phong>\n");

		// extro
		fprintf(fDAE, "\t\t\t\t</technique>\n\t\t\t</profile_COMMON>\n\t\t</effect>\n");
		} // if mat->Shading

	} // if mat

#else // GORILLA
return 1;
#endif // GORILLA
} // ExportFormatCOLLADA::ProcessLibraryEffects

/*===========================================================================*/

int ExportFormatCOLLADA::ProcessLibraryVisualScenes(NameList **fileNamesCreated)
{

/***
// library_visual_scenes
fprintf(fDAE, "\t<library_visual_scenes>\n");
fprintf(fDAE, "\t\t<visual_scene id=\"Scene_Express_Scene\">\n");
fprintf(fDAE, "\t\t\t<node id=\"%s\">\n", objName);
fprintf(fDAE, "\t\t\t\t<scale sid=\"scale\">%f %f %f</scale>\n", curInstance->Scale[0], curInstance->Scale[1], curInstance->Scale[2]);
fprintf(fDAE, "\t\t\t\t<instance_geometry url=\"#%s-geometry\"/>\n", objName);
fprintf(fDAE, "\t\t\t</node>\n");
fprintf(fDAE, "\t\t</visual_scene>\n");
fprintf(fDAE, "\t</library_visual_scenes>\n");
***/

return 1;

} // ExportFormatCOLLADA::ProcessLibraryVisualScenes

/*===========================================================================*/

void ExportFormatCOLLADA::ProcessPolys(Object3DEffect *curObj)
{
MaterialEffect *mat;
long flipNormals, lowVert, highVert, numTris;

// need to write all polys of a given material together
for (long matNum = 0; matNum < curObj->NumMaterials; matNum++)
	{
	// first pass gets the number of triangles
	numTris = 0;
	for (long polyCt = 0; polyCt < curObj->NumPolys; polyCt++)
		{
		if ((matNum == curObj->Polygons[polyCt].Material) && (curObj->Polygons[polyCt].NumVerts > 0))
			{
			// make sure the poly has at least 3 verts
			if (curObj->Polygons[polyCt].NumVerts >= 3)
				{
				if (curObj->Polygons[polyCt].NumVerts == 3)
					numTris++;
				else
					numTris += curObj->Polygons[polyCt].NumVerts - 2;
				} // if
			} // if
		} // for
	if (numTris != 0)
		{
		short doubleSided = curObj->NameTable[matNum].Mat->DoubleSided;

		if (doubleSided)
			numTris *= 2;

		// second pass generates the triangle definitions
		if (mat = curObj->NameTable[matNum].Mat)
			{
			long hasUVW = curObj->VertexUVWAvailable;

			FixMatName(mat->Name);
			fprintf(fDAE, "\t\t\t\t<triangles material=\"%s-material\" count=\"%d\">\n", matName, numTris);
			fprintf(fDAE, "\t\t\t\t\t<input semantic=\"VERTEX\" source=\"#%s-geometry-vertex\" offset=\"0\"/>\n", objName);
			fprintf(fDAE, "\t\t\t\t\t<input semantic=\"NORMAL\" source=\"#%s-geometry-normal\" offset=\"1\"/>\n", objName);
			if ((matNum == 0) && hasUVW)
				fprintf(fDAE, "\t\t\t\t\t<input semantic=\"TEXCOORD\" source=\"#%s-geometry-uv\" offset=\"0\" set=\"0\"/>\n", objName);
			fprintf(fDAE, "\t\t\t\t\t<p>");

			for (long polyCt = 0; polyCt < curObj->NumPolys; polyCt++)
				{
				if ((matNum == curObj->Polygons[polyCt].Material) && (curObj->Polygons[polyCt].NumVerts > 0))
					{
					// make sure the poly has at least 3 verts
					if (curObj->Polygons[polyCt].NumVerts >= 3)
						{
						flipNormals = curObj->NameTable[curObj->Polygons[polyCt].Material].Mat ?
							curObj->NameTable[curObj->Polygons[polyCt].Material].Mat->FlipNormal : 0;

						if (curObj->Polygons[polyCt].NumVerts == 3)
							{
							if ((! flipNormals) || doubleSided)
								{
								fprintf(fDAE, " %d %d %d %d %d %d",
									curObj->Polygons[polyCt].VertRef[0], polyCt,
									curObj->Polygons[polyCt].VertRef[2], polyCt,
									curObj->Polygons[polyCt].VertRef[1], polyCt);
								} // if
							if (flipNormals || doubleSided)
								{
								fprintf(fDAE, " %d %d %d %d %d %d",
									curObj->Polygons[polyCt].VertRef[0], polyCt,
									curObj->Polygons[polyCt].VertRef[1], polyCt,
									curObj->Polygons[polyCt].VertRef[2], polyCt);
								} // else
							} // if
						else
							{
							lowVert = 1; highVert = 2;

							while (highVert < curObj->Polygons[polyCt].NumVerts)
								{
								if ((! flipNormals) || doubleSided)
									{
									fprintf(fDAE, " %d %d %d %d %d %d",
										curObj->Polygons[polyCt].VertRef[0], polyCt,
										curObj->Polygons[polyCt].VertRef[highVert], polyCt,
										curObj->Polygons[polyCt].VertRef[lowVert], polyCt);
									} // if
								if (flipNormals || doubleSided)
									{
									fprintf(fDAE, " %d %d %d %d %d %d",
										curObj->Polygons[polyCt].VertRef[0], polyCt,
										curObj->Polygons[polyCt].VertRef[lowVert], polyCt,
										curObj->Polygons[polyCt].VertRef[highVert], polyCt);
									} // else
								lowVert = highVert;
								highVert++;
								} // while
							} // else
						} // if
					} // if
				} // for
			fprintf(fDAE, " </p>\n");
			fprintf(fDAE, "\t\t\t\t</triangles>\n");
			} // if
		} // if
	} // for


} // ExportFormatCOLLADA::ProcessPolys

/*===========================================================================*/

int ExportFormatCOLLADA::ProcessTerrainImages(NameList **fileNamesCreated)
{
const char *imageName;
int success;
long createDir = 1, fileType, tiled;
char fullPathAndName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];
char noExtenName[WCS_PATHANDFILE_NAME_LEN];
char pathName[WCS_PATHANDFILE_PATH_LEN];

if (sectionInactive)
	{
	sprintf(tempStr, "\t<library_images>\n");
	fputs(tempStr, fDAE);
	sectionInactive = 0;
	} // if

fileType = WCS_EXPORTCONTROL_FILETYPE_TEX1;
if ((Master->DEMTilesX > 1) || (Master->DEMTilesY > 1))
	{
	tiled = 1;
	imageName = NULL;
	imageName = (*fileNamesCreated)->FindNextNameOfType(fileType, imageName);
	} // if tiles
else
	{
	tiled = 0;
	imageName = (*fileNamesCreated)->FindNameOfType(fileType);
	} // else tiles
strmfp(fullPathAndName, Master->OutPath.GetPath(), imageName);

while (imageName)
	{
	strcpy(pathName, Master->OutPath.GetPath());
	strcat(pathName, "/images");
	if (createDir)
		{
		PROJ_mkdir(pathName);
		createDir = 0;
		} // if
	CopyExistingFile(fullPathAndName, pathName, imageName);

	strcpy(noExtenName, imageName);
	(void)StripExtension(noExtenName);
	fprintf(fDAE, "\t\t<image id=\"%s-image\">\n", noExtenName);
	fprintf(fDAE, "\t\t\t<init_from>images/%s</init_from>\n", imageName);
	success = fprintf(fDAE, "\t\t</image>\n");

	if (tiled)
		{
		imageName = (*fileNamesCreated)->FindNextNameOfType(fileType, imageName);
		} // if
	} // while

return (success >= 0);

} // ExportFormatCOLLADA::ProcessTerrainImages

/*===========================================================================*/

int ExportFormatCOLLADA::SanityCheck(void)
{
int rVal = 1;

return rVal;

} // ExportFormatCOLLADA::SanityCheck

/*===========================================================================*/
