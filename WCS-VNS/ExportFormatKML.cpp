// ExportFormatKML.cpp
// Code module for KML (Google Earth) export code
// Created on 03/08/06 by FPW2
// Copyright 2006 3D Nature, LLC. All rights reserved.
//
// Last version of COLLADA used for dev: 1.4.1 2nd edition (with some 1.5 changes)
// Last version of KML used for dev: 2.2.0
// Last version of Google Earth used for dev: 5.0.11337.1968 (beta) (Jan 29, 2009)

#include "stdafx.h"
#include "EffectsLib.h"
#include "ExportFormat.h"
#include "PathAndFile.h"
#include "Project.h"
#include "UsefulZip.h"
#include "FeatureConfig.h"
#include "RequesterBasic.h"
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
#include "Project.h"

#define GE50

#define COLLADA_VERSION
// F2_NOTE: Either COLLADA docs appear to be wrong on Z_UP notation.  Or "in" doesn't refer to into monitor.  Positive y is INTO monitor.
// COLLADA group confirms the docs are wrong

// 3D Objects are created in Y_UP format, the way that we store them internally.
// Foliage and Terrain are created in Z_UP format - the way Google Earth expects them.

// COLLADA uses a Right-Handed Coordinate System
// http://mathworld.wolfram.com/Right-HandedCoordinateSystem.html
// Y_UP: Right Axis is +X, Up Axis is +Y, Into Monitor is -Z
// Z_UP: Right Axis is +X, Up Axis is +Z, Into Monitor is +Y

// Known issues:
//
// Check <lambert> code once spec newer than 1.4.1 comes out.
//
// Crossboard trees still have issues (Google working on solution).
//
// Labels move around in scene now that we have removed the double image height hack.
//

//#define GE_CLOCKWISE_WINDING
//#define COLLADA_TREE_SCALE 100.0f	// For sizing our master nodes at the origin and their instances
//#define NO_KML_CLEANUP

extern WCSApp *GlobalApp;

static float elevMod;
static int derr;
static char msgGEE[] = "Google Earth Exporter";
static char matName[64];
static char objName[64];
//static char texName[64];
static bool warningCheck = false;

/*===========================================================================*/

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

} // FixName

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

} // FixMatName

/*===========================================================================*/

/***
static void FixTexName(const char *name)
{
char ltexName[64];

strcpy(ltexName, name);
for (size_t i = 0; i < strlen(name); i++)
	{
	// If it's an alphanumeric char, we're OK.  Otherwise, replace it with an underscore.
	if (isalnum(ltexName[i]))
		continue;
	else
		ltexName[i] = '_';
	} // for

texName[0] = NULL;
texName[1] = NULL;

// First character must be a letter
if (isdigit(ltexName[0]) || (ltexName[0] == '_'))
	texName[0] = 'T';

strcat(texName, ltexName);

} // FixTexName
***/

/*===========================================================================*/

ExportFormatKML::ExportFormatKML(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource)
: ExportFormat(MasterSource, ProjectSource, EffectsSource, DBSource, ImageSource)
{

fKML = NULL;
curIndent = 0;
zippy = NULL;
indentStr[0] = NULL;
elevMod = 0.f;

} // ExportFormatKML::ExportFormatKML

/*===========================================================================*/

ExportFormatKML::~ExportFormatKML()
{

} // ExportFormatKML::~ExportFormatKML

/*===========================================================================*/

void ExportFormatKML::BindMaterials(long matCt, FILE *fDAE, Object3DEffect *curObj)
{
MaterialEffect *mat;

if (! curObj->NameTable[matCt].Mat)
	curObj->NameTable[matCt].Mat = (MaterialEffect *)GlobalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, curObj->NameTable[matCt].Name);

if (mat = curObj->NameTable[matCt].Mat)
	{
	FixMatName(mat->Name);
	fprintf(fDAE, "\t\t\t\t\t\t\t<instance_material symbol=\"%s-material\" target=\"#%s-material\">\n", matName, matName);
	fprintf(fDAE, "\t\t\t\t\t\t\t\t<bind_vertex_input semantic=\"TEX0\" input_semantic=\"TEXCOORD\" input_set=\"0\"/>\n");
	fprintf(fDAE, "\t\t\t\t\t\t\t</instance_material>\n");
	} // if

} // ExportFormatKML::BindMaterials

/*===========================================================================*/

int ExportFormatKML::C_prologue_asset(FILE *fDAE, unsigned long upAxis)
{
struct tm *newTime;
time_t szClock;
float lat, lon;
char timeString[32];

// prologue
fprintf(fDAE, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
//fprintf(fDAE, "<COLLADA xmlns=\"http://www.collada.org/2005/11/COLLADASchema\" version=\"1.4.0\">\n");
//fprintf(fDAE, "<COLLADA xmlns=\"http://www.collada.org/2005/11/COLLADASchema\" version=\"1.4.1\">\n");
fprintf(fDAE, "<COLLADA xmlns=\"http://www.collada.org/2008/03/COLLADASchema\" version=\"1.5\">\n");

// asset
fprintf(fDAE, "\t<asset>\n\t\t<contributor>\n");
sprintf(tempStr, "\t\t\t<author>%s</author>\n", GlobalApp->MainProj->UserName);
UTF8Encode(fDAE, tempStr);
sprintf(tempStr, "\t\t\t<author_email>%s</author_email>\n", GlobalApp->MainProj->UserEmail);
UTF8Encode(fDAE, tempStr);
fprintf(fDAE, "\t\t\t<authoring_tool>%s %s (%s)</authoring_tool>\n", APP_TITLE, ExtVersion, ExtAboutBuild);
strcpy(tempStr, GlobalApp->MainProj->projectname);
StripExtension(tempStr);
fprintf(fDAE, "\t\t\t<comments>Created from Project:%s, Exporter:%s</comments>\n", tempStr, Master->Name);
fprintf(fDAE, "\t\t</contributor>\n");
// COLLADA 1.5 element
fprintf(fDAE, "\t\t<coverage>\n");
fprintf(fDAE, "\t\t\t<geographic_location>\n");
lat = float((Master->RBounds.AnimPar[0].CurValue + Master->RBounds.AnimPar[1].CurValue) * 0.5);
lon = float((Master->RBounds.AnimPar[2].CurValue + Master->RBounds.AnimPar[3].CurValue) * 0.5);
fprintf(fDAE, "\t\t\t\t<longitude>%f</longitude>\n", lon);
fprintf(fDAE, "\t\t\t\t<latitude>%f</latitude>\n", lat);
//fprintf(fDAE, "\t\t\t\t<altitude mode=\"absolute\">0</altutude>\n");
fprintf(fDAE, "\t\t\t\t<altitude mode=\"relativeToGround\">0</altitude>\n");
fprintf(fDAE, "\t\t\t</geographic_location>\n");
fprintf(fDAE, "\t\t</coverage>\n");
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

} // ExportFormatKML::C_prologue_asset

/*===========================================================================*/

// remove all temp files
void ExportFormatKML::Cleanup(NameList **fileNamesCreated)
{
#ifndef NO_KML_CLEANUP
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

// remove ground texture image
if (Master->ExportTexture && !Master->ExportTerrain)
	{
	fileType = WCS_EXPORTCONTROL_FILETYPE_TEX1;
	if ((Master->TexTilesX > 1) || (Master->TexTilesY > 1))
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

// remove terrain temp file(s)
if (Master->ExportTerrain && Master->ExportTexture)
	{
	fileType = WCS_EXPORTCONTROL_FILETYPE_FINALTERRAIN;
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

// clean up image textures that are still in the scene export root dir (only if textured terrain has been exported)
if (Master->ExportTerrain && Master->ExportTexture)
	{
	if ((Master->TexTilesX > 1) || (Master->TexTilesY > 1))
		{
		fileType = WCS_EXPORTCONTROL_FILETYPE_TEX1;
		cleanupFileName = NULL;
		while (cleanupFileName = (*fileNamesCreated)->FindNextNameOfType(fileType, cleanupFileName))
			{
			strmfp(fullFileName, Master->OutPath.GetPath(), cleanupFileName);
			PROJ_remove(fullFileName);
			} // while
		} // if tiles
	else
		{
		fileType = WCS_EXPORTCONTROL_FILETYPE_TEX1;
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
#endif // !NO_KML_CLEANUP

} // ExportFormatKML::Cleanup

/*===========================================================================*/

int ExportFormatKML::CreateTree(const char *speciesName)
{
FILE *fTrees;
char *dot3;
int Success = 1;
char fullPathAndName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN], pathName[WCS_PATHANDFILE_PATH_LEN];

strcpy(fullPathAndName, Master->OutPath.GetPath());
strcat(fullPathAndName, "/models/");
strcat(fullPathAndName, speciesName);
strcat(fullPathAndName, ".dae");

dot3 = ImageSaverLibrary::GetDefaultExtension(Master->FoliageImageFormat);

if (fTrees = PROJ_fopen(fullPathAndName, "w"))
	{
	//float tx, ty;
	float vx, vy, vz;

	C_prologue_asset(fTrees, 2);


	// library_effects
	sprintf(tempStr, "\t<library_effects>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t<effect id=\"%s-effect\">\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t<profile_COMMON>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t<newparam sid=\"%s-surface\">\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t<surface type=\"2D\">\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t<init_from>%s-image</init_from>\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t<format>A8R8G8B8</format>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t</surface>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t</newparam>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t<newparam sid=\"%s-sampler\">\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t<sampler2D>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t<source>%s-surface</source>\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t<wrap_s>CLAMP</wrap_s>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t<wrap_t>CLAMP</wrap_t>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t<minfilter>LINEAR_MIPMAP_LINEAR</minfilter>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t<magfilter>LINEAR</magfilter>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t</sampler2D>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t</newparam>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t<technique sid=\"COMMON\">\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t<phong>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t<emission>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t\t<color>0.0 0.0 0.0 1</color>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t</emission>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t<ambient>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t\t<color>0.0 0.0 0.0 1</color>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t</ambient>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t<diffuse>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t\t<texture texture=\"%s-sampler\" texcoord=\"TEX0\"/>\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t</diffuse>\n");
	fputs(tempStr, fTrees);
	/***
	sprintf(tempStr, "\t\t\t\t\t\t<specular>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t\t<color>0.0 0.0 0.0 1</color>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t</specular>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t<shininess>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t\t<float>20.0</float>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t</shininess>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t<reflectivity>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t\t<float>0.1</float>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t</reflectivity>\n");
	fputs(tempStr, fTrees);
	***/
	sprintf(tempStr, "\t\t\t\t\t\t<transparent>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t\t<color>1 1 1 1</color>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t</transparent>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t<transparency>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t\t<float>0.0</float>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t</transparency>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t</phong>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t</technique>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t<extra>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t<technique profile=\"GOOGLEEARTH\">\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t<double_sided>1</double_sided>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t</technique>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t</extra>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t</profile_COMMON>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t</effect>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t</library_effects>\n");
	fputs(tempStr, fTrees);


	// library_geometries
	sprintf(tempStr, "\t<library_geometries>\n");
	fputs(tempStr, fTrees);

	sprintf(tempStr, "\t\t<geometry id=\"%s-geometry\">\n", matName);
	fputs(tempStr, fTrees);

	sprintf(tempStr, "\t\t\t<mesh>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t<source id=\"%s-geometry-position\">\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t<float_array id=\"%s-geometry-position-array\" count=\"18\">", matName);
	fputs(tempStr, fTrees);

	vx = 0.5f;
	vy = 0.0f;
	vz = 0.0f;
	/*** xz plane verts
	5 --- 4 --- 3
	|\    |\    |
	|  \  |  \  |
	|    \|    \|
	0 --- 1 --- 2		(origin at vert 1, z up)
	***/
	sprintf(tempStr, "%f %f %f ", -vx, vy, vz);		// 0
	fputs(tempStr, fTrees);
	sprintf(tempStr, "%f %f %f ", 0.0f, vy, vz);	// 1
	fputs(tempStr, fTrees);
	sprintf(tempStr, "%f %f %f ", vx, vy, vz);		// 2
	fputs(tempStr, fTrees);
	vz = 1.0f;
	sprintf(tempStr, "%f %f %f ", vx, vy, vz);		// 3
	fputs(tempStr, fTrees);
	sprintf(tempStr, "%f %f %f ", 0.0f, vy, vz);	// 4
	fputs(tempStr, fTrees);
	sprintf(tempStr, "%f %f %f ", -vx, vy, vz);		// 5
	fputs(tempStr, fTrees);

	sprintf(tempStr, "</float_array>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t<technique_common>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t<accessor source=\"#%s-geometry-position-array\" count=\"6\" stride=\"3\">\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t\t<param name=\"X\" type=\"float\"/>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t\t<param name=\"Y\" type=\"float\"/>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t\t<param name=\"Z\" type=\"float\"/>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t</accessor>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t</technique_common>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t</source>\n");
	fputs(tempStr, fTrees);

	/***
	sprintf(tempStr, "\t\t\t\t<source id=\"%s-geometry-normal\">\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t<float_array id=\"%s-geometry-normal-array\" count=\"3\">0.0 -1.0 0.0</float_array>\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t<technique_common>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t<accessor source=\"#%s-geometry-normal-array\" count=\"1\" stride=\"3\">\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t\t<param name=\"X\" type=\"float\"/>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t\t<param name=\"Y\" type=\"float\"/>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t\t<param name=\"Z\" type=\"float\"/>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t</accessor>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t</technique_common>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t</source>\n");
	fputs(tempStr, fTrees);
	***/

	sprintf(tempStr, "\t\t\t\t<source id=\"%s-geometry-uv\">\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t<float_array id=\"%s-geometry-uv-array\" count=\"12\">0.0 0.0 0.5 0.0 1.0 0.0 1.0 1.0 0.5 1.0 0.0 1.0</float_array>\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t<technique_common>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t<accessor source=\"#%s-geometry-uv-array\" count=\"6\" stride=\"2\">\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t\t<param name=\"S\" type=\"float\"/>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t\t<param name=\"T\" type=\"float\"/>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t</accessor>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t</technique_common>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t</source>\n");
	fputs(tempStr, fTrees);

	sprintf(tempStr, "\t\t\t\t<vertices id=\"%s-geometry-vertex\">\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t<input semantic=\"POSITION\" source=\"#%s-geometry-position\"/>\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t</vertices>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t<triangles material=\"%s-material\" count=\"4\">\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t<input semantic=\"VERTEX\" source=\"#%s-geometry-vertex\" offset=\"0\"/>\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t<input semantic=\"TEXCOORD\" source=\"#%s-geometry-uv\" offset=\"0\" set=\"1\"/>\n", matName);
	fputs(tempStr, fTrees);
	// clockwise only seems to work fine
	sprintf(tempStr, "\t\t\t\t\t<p> 5 1 0 5 4 1 4 2 1 4 3 2 </p>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t</triangles>\n");
	fputs(tempStr, fTrees);

	sprintf(tempStr, "\t\t\t</mesh>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t</geometry>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t</library_geometries>\n");
	fputs(tempStr, fTrees);


	// library_images
	sprintf(tempStr, "\t<library_images>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t<image id=\"%s-image\">\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t<init_from>../images/%s%s</init_from>\n", matName, dot3);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t</image>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t</library_images>\n");
	fputs(tempStr, fTrees);


	// library_materials
	sprintf(tempStr, "\t<library_materials>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t<material id=\"%s-material\">\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t<instance_effect url=\"#%s-effect\"/>\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t</material>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t</library_materials>\n");
	fputs(tempStr, fTrees);


	// library_visual_scenes
	sprintf(tempStr, "\t<library_visual_scenes>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t<visual_scene id=\"Scene_Express_Foliage\">\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t<node id=\"%s-foliage\">\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t<scale>1.0 1.0 1.0</scale>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t<instance_geometry url=\"#%s-geometry\">\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t<bind_material>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t<technique_common>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t\t<instance_material symbol=\"%s-material\" target=\"#%s-material\">\n", matName, matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t\t\t<bind_vertex_input semantic=\"TEX0\" input_semantic=\"TEXCOORD\" input_set=\"0\"/>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t\t</instance_material>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t\t</technique_common>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t\t</bind_material>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t</instance_geometry>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t</node>\n");
	fputs(tempStr, fTrees);

	// compute offset in meters from center of export
	//tx = (float)(folData.XYZ[0] * Master->ExportRefData.ExportLonScale);
	//ty = (float)(folData.XYZ[1] * Master->ExportRefData.ExportLatScale);
	sprintf(tempStr, "\t\t\t<node>\n");
	fputs(tempStr, fTrees);
	//sprintf(tempStr, "\t\t\t\t<translate>%f %f %f</translate>\n", tx, ty, 0.0f);
	//fputs(tempStr, fTrees);
	//w = (float)pointData.Width * COLLADA_TREE_SCALE;
	//h = (float)pointData.Height * COLLADA_TREE_SCALE;
	//if (pointData.FlipX)
	//	w *= -1.0f;
	//strcpy(imageName, curRast->GetUserName());
	//FixMatName(imageName);
	//strcat(matName, "_Fol");
	sprintf(tempStr, "\t\t\t\t<instance_node url=\"#%s-foliage\"/>\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t</node>\n");
	fputs(tempStr, fTrees);

	// make the crossboard another instance
	sprintf(tempStr, "\t\t\t<node>\n");
	fputs(tempStr, fTrees);
	//sprintf(tempStr, "\t\t\t\t<translate>%f %f %f</translate>\n", tx, ty, 0.0f);
	//fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t<rotate>0.0 0.0 1.0 90.0</rotate>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t\t<instance_node url=\"#%s-foliage\"/>\n", matName);
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t\t</node>\n");
	fputs(tempStr, fTrees);

	sprintf(tempStr, "\t\t</visual_scene>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t</library_visual_scenes>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t<scene>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t\t<instance_visual_scene url=\"#Scene_Express_Foliage\"/>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "\t</scene>\n");
	fputs(tempStr, fTrees);
	sprintf(tempStr, "</COLLADA>\n");
	fputs(tempStr, fTrees);
	fclose(fTrees);

	strcpy(fullPathAndName, Master->OutPath.GetPath());
	strcat(fullPathAndName, "/");
	strcat(matName, dot3);
	strcat(fullPathAndName, matName);
	strcpy(pathName, Master->OutPath.GetPath());
	strcat(pathName, "/images/");
	CopyExistingFile(fullPathAndName, pathName, matName);

	if (zippy)
		{
		//strcpy(fullPathAndName, Master->OutPath.GetPath());
		//GlobalApp->MainProj->MungPath(fullPathAndName);
		//strcat(fullPathAndName, "/images/");
		strcpy(fullPathAndName, "images/");	// Do NOT add leading slash to zip string!
		strcat(fullPathAndName, matName);
		zippy->AddToZipList(fullPathAndName);

		//strcpy(fullPathAndName, Master->OutPath.GetPath());
		//GlobalApp->MainProj->MungPath(fullPathAndName);
		//strcat(fullPathAndName, "/models/");
		strcpy(fullPathAndName, "models/");	// Do NOT add leading slash to zip string!
		strcat(fullPathAndName, speciesName);
		StripExtension(fullPathAndName);
		strcat(fullPathAndName, ".dae");
		zippy->AddToZipList(fullPathAndName);
		} // if

	} // if fTrees
else
	Success = 0;

return Success;

} // ExportFormatKML::CreateTree

/*===========================================================================*/

int ExportFormatKML::DefineStyles(NameList **fileNamesCreated)
{
double minHeight = FLT_MAX, rescale = 1.0;
FILE *ffile;
long fileType;
int Success = 1;
const char *indexFileName, *dataFileName;
RealtimeFoliageIndex index;
RealtimeFoliageCellData rfcd;
RealtimeFoliageData folData;
char fileName[512];
char testFileVersion;

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
					FoliagePreviewData pointData;

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
				strmfp(fileName, Master->OutPath.GetPath(), dataFileName);
				if (ffile = PROJ_fopen(fileName, "rb"))
					{
					fgets(fileName, 64, ffile);
					// version
					fread((char *)&testFileVersion, sizeof(char), 1, ffile);
					// Pointless version check -- we know we wrote it
					if (testFileVersion == index.FileVersion)
						{
						double minHeight = FLT_MAX, rescale = 1.0;
						Raster *curRast;
						short *didStyle;
						long datPt, filePos;
						FoliagePreviewData pointData;

						if (Master->FormatExtension && (Master->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_GE))
							rescale = ((SXExtensionGE *)Master->FormatExtension)->LabelRescale / 100.0;

						filePos = ftell(ffile);
						// scan for label heights first
						for (datPt = 0; datPt < index.CellDat->DatCt; datPt++)
							{
							if (folData.ReadFoliageRecord(ffile, index.FileVersion))
								{
								folData.InterpretFoliageRecord(NULL, GlobalApp->AppImages, &pointData);
								if ((folData.BitInfo & WCS_REALTIME_FOLDAT_BITINFO_LABEL) && (curRast = Images->FindByID(folData.ElementID)))
									{
									if (pointData.Height < minHeight)
										minHeight = pointData.Height;
									} // if
								} // if
							} // for
						fseek(ffile, filePos, SEEK_SET);
						
						// create a flag slot for all possible ElementID's (positive's only)
						if (didStyle = (short *)AppMem_Alloc(32768, APPMEM_CLEAR))
							{
							for (datPt = 0; datPt < index.CellDat->DatCt; datPt++)
								{
								if (folData.ReadFoliageRecord(ffile, index.FileVersion))
									{
									folData.InterpretFoliageRecord(NULL, GlobalApp->AppImages, &pointData);

									if (((folData.BitInfo & WCS_REALTIME_FOLDAT_BITINFO_LABEL)) && (curRast = Images->FindByID(folData.ElementID)))
										{
										if (! didStyle[folData.ElementID])
											{
											double relSize;
											char imageName[64];

											didStyle[folData.ElementID] = 1;
											strcpy(imageName, curRast->GetUserName());
											ImageSaverLibrary::StripImageExtension(imageName);
											strcat(imageName, "_Fol");
											ReplaceChar(imageName, '.', '_');
											// ok, now the fun stuff
											relSize = pointData.Height / minHeight;
											StylesAndMap(imageName, ImageSaverLibrary::GetDefaultExtension(Master->FoliageImageFormat),
												(float)(rescale * relSize));
											} // if
										} // if
									} // if
								} // for
							AppMem_Free(didStyle, 32768);
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

return Success;

} // ExportFormatKML::DefineStyles

/*===========================================================================*/

int ExportFormatKML::Describe(SXQueryActionList *actionList, GeneralEffect *MyEffect, Joe *MyVector)
{
PathAndFile paf;
bool describing = false;
char tempFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];
char ActionCommand[16384];

switch (actionList->QueryAction->ActionType)
	{
	case WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYDATATABLE:
	case WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYIMAGE:
	case WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYWEBPAGE:
	case WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYIMAGEINTERNAL:
		sprintf(tempStr, "%s<description><![CDATA[", indentStr);
		fputs(tempStr, fKML);
		describing = true;
		break;
	default:
		break;
	} // switch

if (describing)
	{
	switch (actionList->QueryAction->ActionType)
		{
		case WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYDATATABLE:
			actionList->QueryAction->ReplaceStringTokens(ActionCommand, actionList->QueryAction->ActionText, MyEffect, MyVector);
			sprintf(tempStr, "%s", ActionCommand);
			fputs(tempStr, fKML);
			break;
		case WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYIMAGE:
			actionList->QueryAction->ReplaceStringTokens(ActionCommand, actionList->QueryAction->ActionText, MyEffect, MyVector);
			sprintf(tempStr, "<a href=\"%s\">Display Image</a>", ActionCommand);
			fputs(tempStr, fKML);
			break;
		case WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYWEBPAGE:
			actionList->QueryAction->ReplaceStringTokens(ActionCommand, actionList->QueryAction->ActionText, MyEffect, MyVector);
			sprintf(tempStr, "<a href=\"%s\">%s</a>", ActionCommand, ActionCommand);
			fputs(tempStr, fKML);
			break;
		case WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYIMAGEINTERNAL:
			actionList->QueryAction->ReplaceStringTokens(ActionCommand, actionList->QueryAction->ActionText, MyEffect, MyVector);
			BreakFileName(ActionCommand, paf.Path, WCS_PATHANDFILE_PATH_LEN, paf.Name, WCS_PATHANDFILE_NAME_LEN);
			strcpy(tempFullPath, (char *)Master->OutPath.GetPath());
			strcat(tempFullPath, "images\\");
			PROJ_mkdir(tempFullPath);
			CopyExistingFile(ActionCommand, tempFullPath, paf.Name);
			sprintf(tempStr, "<img src=\"images/%s\">", paf.Name);
			fputs(tempStr, fKML);
			warningCheck = true;
			break;
		default:
			break;
		} // switch

	sprintf(tempStr, "]]></description>\n");
	fputs(tempStr, fKML);
	} // if

return 1;

} // ExportFormatKML::Describe

/*===========================================================================*/

int ExportFormatKML::Export3DObjs(NameList **fileNamesCreated)
{
Camera *curCam = NULL;
Object3DInstance *curInstance = Master->ObjectInstanceList;
Object3DEffect *object3D;
Renderer *rend = NULL;
RenderOpt *curOpt = NULL;
RenderJob *curRJ = NULL;
char *elevAdjust;
unsigned long coordsCount = 0;
int success = 0;

if (elevAdjust = GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("GE_ElevMod"))
	elevMod = (float)atof(elevAdjust);

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
	PlanetOpt *DefaultPlanetOpt;
	char pathName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];
	char modelName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];
	char tempFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

	DefaultPlanetOpt = (PlanetOpt *)rend->EffectsBase->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, rend->DBase);

	rend->RendData.Interactive = rend->ProjectBase->Interactive;
	rend->RendData.DBase = rend->DBase;
	rend->RendData.EffectsBase = rend->EffectsBase;
	rend->RendData.ProjectBase = rend->ProjectBase;
	rend->RendData.DefCoords = Master->Coords;
	rend->RendData.ElevDatum = DefaultPlanetOpt->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue;
	rend->RendData.Exageration = DefaultPlanetOpt->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].CurValue;
	rend->RendData.PlanetRad = rend->EffectsBase->GetPlanetRadius();

	sprintf(tempStr, "%s<Folder>\n%s\t<name>3D Objects</name>\n", indentStr, indentStr);
	fputs(tempStr, fKML);
	IndentMore();

	strcpy(tempFullPath, (char *)Master->OutPath.GetPath());
	strcpy(pathName, GlobalApp->MainProj->MungPath(tempFullPath));
	strcat(pathName, "/models/");
	derr = PROJ_mkdir(pathName);
	if ((ERROR_SUCCESS != derr) && (ERROR_FILE_EXISTS != derr) && (ERROR_ALREADY_EXISTS != derr))
		{
		char msg[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN + 128];

		sprintf(&msg[0], "%s: Unable to create directory '%s'. Object '%s' skipped", msgGEE, pathName, curInstance->MyObj->Name);
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, msg);
		curInstance = NULL;	// we couldn't make our subdir, so abort processing on this object
		} // if

	while (curInstance)
		{
		object3D = curInstance->MyObj;
		if (curInstance->IsBounded(Master->FetchRBounds()) &&
			((object3D->Vertices && object3D->Polygons && object3D->NameTable) || object3D->OpenInputFile(NULL, FALSE, FALSE, FALSE)))
			{
			FILE *fDAE;
			long createIt = 1;

			// FPW2 - 05/10/07: Testing on current GE beta (4.0.2080 May 10, 2007) still shows this limitation, although it no longer crashes GE
			// FPW2 - 03/06/08: Hey, this limitation is now documented!
			//if (false)
			if ((object3D->NumVertices > 21845) || (object3D->NumPolys > 21845))
				{
				char msg[256];

				createIt = 0;
				sprintf(&msg[0], "%s: Object '%s' has too many vertices or polygons (> 21845) for Google Earth - skipped.", msgGEE, object3D->Name);
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
				double dimx, dimy, dimz, eyeDist, heading;
				SXQueryActionList *actionList = NULL;
				BOOL absolute;
				long matCt;

				PolygonData Poly;
				VertexData VertData;
				unsigned long Flags;

				absolute = GlobalApp->MainProj->Prefs.PublicQueryConfigOptTrue("GE_Absolute");

				// create link in main file
				sprintf(tempStr, "%s<Placemark>\n", indentStr);
				fputs(tempStr, fKML);
				IndentMore();
				sprintf(tempStr, "%s<name>%s</name>\n", indentStr, object3D->Name);
				fputs(tempStr, fKML);
				// 10/22/06 CXH add descriptions, for what they're worth
				#ifdef WCS_BUILD_SX2
				if (curInstance->ClickQueryObjectID >= 0)
					{
					// this doesn't work for Walls. The 3D object is synthetic and has no ActionList.
					actionList = object3D->ActionList;
					if (actionList)
						Describe(actionList, object3D, NULL); // we don't have a vector pointer that I know of
					} // if
				#endif // WCS_BUILD_SX2

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
				sprintf(tempStr, "%s\t<tilt>70.0</tilt>\n", indentStr);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s\t<range>%lf</range>\n", indentStr, eyeDist);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s</LookAt>\n", indentStr);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s<Model>\n", indentStr);
				fputs(tempStr, fKML);
				IndentMore();
				if (absolute)
					sprintf(tempStr, "%s<altitudeMode>absolute</altitudeMode>\n", indentStr);
				else
					sprintf(tempStr, "%s<altitudeMode>relativeToGround</altitudeMode>\n", indentStr);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s<Location>\n", indentStr);
				fputs(tempStr, fKML);
				sprintf(tempStr, "  %s<longitude>%lf</longitude>\n", indentStr, -curInstance->WCSGeographic[0]);
				fputs(tempStr, fKML);
				sprintf(tempStr, "  %s<latitude>%lf</latitude>\n", indentStr, curInstance->WCSGeographic[1]);
				fputs(tempStr, fKML);
				if (absolute)
					sprintf(tempStr, "  %s<altitude>%lf</altitude>\n", indentStr, curInstance->WCSGeographic[2]);
				else
					{
					// determine terrain elevation under object's origin so we can calculate relative-to-GE-terrain offset
 					Poly.Lon = VertData.Lon = curInstance->WCSGeographic[0];
 					Poly.Lat = VertData.Lat = curInstance->WCSGeographic[1];
 					Poly.Elev = VertData.Elev = curInstance->WCSGeographic[2];
 					Poly.LonSeed = (ULONG)((VertData.Lon - WCS_floor(VertData.Lon)) * ULONG_MAX);
 					Poly.LatSeed = (ULONG)((VertData.Lat - WCS_floor(VertData.Lat)) * ULONG_MAX);
	 
 					Poly.Object = NULL; // object3D;
 					Poly.Vector = NULL; // object3D->Joes->Me;
 					Poly.VectorType = NULL; // WCS_TEXTURE_VECTOREFFECTTYPE_LINE;
	 
 					Flags = (WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_RELEL | 
 						WCS_VERTEXDATA_FLAG_WATERELEV | WCS_VERTEXDATA_FLAG_WATERDEPTH | WCS_VERTEXDATA_FLAG_LAKEAPPLIED |
 						WCS_VERTEXDATA_FLAG_STREAMAPPLIED | WCS_VERTEXDATA_FLAG_WAVEHEIGHT | WCS_VERTEXDATA_FLAG_NORMAL);
 					rend->RendData.Interactive->VertexDataPoint(&rend->RendData, &VertData, &Poly, Flags);

					sprintf(tempStr, "  %s<altitude>%lf</altitude>\n", indentStr, curInstance->WCSGeographic[2] - VertData.Elev + elevMod);
					} // else

				fputs(tempStr, fKML);
				sprintf(tempStr, "%s</Location>\n", indentStr);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s<Orientation>\n", indentStr);
				fputs(tempStr, fKML);
				heading = fmod(180.0 + curInstance->Euler[1], 360.0);
				sprintf(tempStr, "%s\t<heading>%lf</heading>\n", indentStr, heading);
				fputs(tempStr, fKML);
				sprintf(tempStr, "%s\t<tilt>%lf</tilt>\n", indentStr, curInstance->Euler[0]);
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
				C_prologue_asset(fDAE, 1);

				// library effects
				fprintf(fDAE, "\t<library_effects>\n");
				for (matCt = 0; matCt < object3D->NumMaterials; matCt++)
					ProcessMatTexture3(matCt, fDAE, object3D);
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
						ProcessMatTexture1(matCt, fDAE, object3D);
					fprintf(fDAE, "\t</library_images>\n");
					} // if

				// library materials
				fprintf(fDAE, "\t<library_materials>\n");
				for (matCt = 0; matCt < object3D->NumMaterials; matCt++)
					ProcessMatTexture2(matCt, fDAE, object3D);
				fprintf(fDAE, "\t</library_materials>\n");

				// library visual scenes
				fprintf(fDAE, "\t<library_visual_scenes>\n");
				fprintf(fDAE, "\t\t<visual_scene id=\"Scene_Express_Scene\">\n");
				fprintf(fDAE, "\t\t\t<node id=\"%s\">\n", objName);
				fprintf(fDAE, "\t\t\t\t<scale sid=\"scale\">%f %f %f</scale>\n", curInstance->Scale[0], curInstance->Scale[1], curInstance->Scale[2]);
				fprintf(fDAE, "\t\t\t\t<instance_geometry url=\"#%s-geometry\">\n", objName);
				fprintf(fDAE, "\t\t\t\t\t<bind_material>\n");
				fprintf(fDAE, "\t\t\t\t\t\t<technique_common>\n");
				for (matCt = 0; matCt < object3D->NumMaterials; matCt++)
					BindMaterials(matCt, fDAE, object3D);
				fprintf(fDAE, "\t\t\t\t\t\t</technique_common>\n");
				fprintf(fDAE, "\t\t\t\t\t</bind_material>\n");
				fprintf(fDAE, "\t\t\t\t</instance_geometry>\n");
				fprintf(fDAE, "\t\t\t</node>\n");
				fprintf(fDAE, "\t\t</visual_scene>\n");
				fprintf(fDAE, "\t</library_visual_scenes>\n");

				// scene
				fprintf(fDAE, "\t<scene>\n");
				fprintf(fDAE, "\t\t<instance_visual_scene url=\"#Scene_Express_Scene\"/>\n");
				fprintf(fDAE, "\t</scene>\n");

				// finish it
				fprintf(fDAE, "</COLLADA>\n");
				fclose(fDAE);

				if (zippy)
					{
					strcpy(tempFullPath, "models/");	// Do NOT add leading slash to zip string!
					strcat(tempFullPath, modelName);
					zippy->AddToZipList(tempFullPath);
					} // if

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
	rend->Cleanup(true, false, TRUE, false);
	delete rend;
	} // if
if (curOpt)
	delete curOpt;
if (curRJ)
	delete curRJ;
if (curCam)
	delete curCam;

return (success);

} // ExportFormatKML::Export3DObjs

/*===========================================================================*/

#ifdef OLDER_KML

int ExportFormatKML::ExportCameras(void)
{
class EffectList *camList = Master->Cameras;
class Camera *cam;

sprintf(tempStr, "%s<Folder>\n%s\t<name>Cameras</name>\n", indentStr, indentStr);
fputs(tempStr, fKML);
IndentMore();
while (camList)
	{
	double alt, camLat, camLon, range;
	Point3d orient;
	bool doRange;

	doRange = true;
	cam = (class Camera *)camList->Me;
	sprintf(tempStr, "%s<Placemark>\n", indentStr);
	fputs(tempStr, fKML);
	IndentMore();
	sprintf(tempStr, "%s<name>%s</name>\n", indentStr, cam->Name);
	fputs(tempStr, fKML);
	sprintf(tempStr, "%s<LookAt>\n", indentStr);
	fputs(tempStr, fKML);
	Master->RBounds.DefDegToRBounds(cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue,
		cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue, camLon, camLat);
	sprintf(tempStr, "%s\t<longitude>%f</longitude>\n", indentStr, camLon);
	fputs(tempStr, fKML);
	sprintf(tempStr, "%s\t<latitude>%f</latitude>\n", indentStr, camLat);
	fputs(tempStr, fKML);
	alt = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue - Master->ExportRefData.RefElev;
	if (cam->CameraType == WCS_EFFECTS_CAMERATYPE_UNTARGETED)
		{
		orient[0] = -cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].CurValue;
		orient[1] = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].CurValue;
		double Radius;
		#ifdef WCS_BUILD_VNS
		Radius = Master->Coords->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].CurValue;
		#else //!WCS_BUILD_VNS
		Radius = GlobalApp->AppEffects->GetPlanetRadius();
		#endif //!WCS_BUILD_VNS
		range = FindDistance(cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue, cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue,
			cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].CurValue, cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].CurValue, Radius);
		range *= 0.5;	// since we can't set FOV, compensate to make us a bit closer
		doRange = false;
		} // if untargeted
	else if (cam->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED)
		{
		orient[0] = -cam->CamPitch;
		orient[1] = cam->CamHeading;
		double Radius;
		#ifdef WCS_BUILD_VNS
		Radius = Master->Coords->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].CurValue;
		#else //!WCS_BUILD_VNS
		Radius = GlobalApp->AppEffects->GetPlanetRadius();
		#endif //!WCS_BUILD_VNS
		range = FindDistance(cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue, cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue,
			cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].CurValue, cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].CurValue,
			Radius);
		range *= 0.5;	// since we can't set FOV, compensate to make us a bit closer
		doRange = false;
		} // else if targeted
	else if (cam->CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD)
		{
		orient[0] = -cam->CamPitch;
		orient[1] = cam->CamHeading;
		range = alt;
		doRange = false;
		} // else if overhead
	else if (cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
		{
		orient[0] = -90.0;
		orient[1] = 0.0;
		range = alt;
		doRange = false;
		} // else if planimetric
	if (cam->CameraType == WCS_EFFECTS_CAMERATYPE_ALIGNED)
		{
		orient[0] = -cam->CamPitch;
		orient[1] = cam->CamHeading;
		//alt = 123.45;	// fix
		} // if untargeted
	orient[0] += 90.0;	// 0 is straight down, 90 is horizon
	if (orient[0] > 90.0)
		orient[0] = 90.0;
	else if (orient[0] < 0.0)
		orient[0] = 0.0;
	//orient[2] = 0.0;
	if (doRange)
		{
		if (orient[0] <= 45.0)	// keep range within reason
			{
			range = cos(orient[0] * PiOver180) / alt;
			} // if
		else
			range = alt;
		} // if
	sprintf(tempStr, "%s\t<range>%f</range>\n", indentStr, range);
	fputs(tempStr, fKML);
	sprintf(tempStr, "%s\t<tilt>%f</tilt>\n", indentStr, orient[0]);
	fputs(tempStr, fKML);
	sprintf(tempStr, "%s\t<heading>%f</heading>\n", indentStr, orient[1]);
	fputs(tempStr, fKML);
	sprintf(tempStr, "%s</LookAt>\n", indentStr);
	fputs(tempStr, fKML);
	IndentLess();
	sprintf(tempStr, "%s</Placemark>\n", indentStr);
	fputs(tempStr, fKML);

	camList = camList->Next;
	} // while CamList

IndentLess();
sprintf(tempStr, "%s</Folder>\n", indentStr);
fputs(tempStr, fKML);

return 1;

} // ExportFormatKML::ExportCameras

#else // older KML

#ifdef GE50

int ExportFormatKML::ExportCameras(void)
{
double cx, cy, tx, ty, heading, tilt, roll, lastTime = 0.0, loopTime, endTime, loopInt = .1;
EffectList *camList;
Camera *curCamera;
RenderData rendData(NULL);
RasterAnimHostProperties prop;
int success = 1;
bool animated;

// export cameras
if (Master->ExportCameras && (camList = Master->Cameras))
	{
	while (camList)
		{
		if (camList->Me)
			{
			curCamera = (Camera *)camList->Me;

			if (rendData.InitToView(EffectsHost, ProjectHost, DBHost, ProjectHost->Interactive, NULL, curCamera, 320, 320))
				{
				// is camera animated?
				prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_KEYRANGE);
				prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATED;
				prop.ItemOperator = WCS_KEYOPERATION_ALLKEYS;
				prop.TypeNumber = WCS_EFFECTSSUBCLASS_CAMERA;
				curCamera->GetRAHostProperties(&prop);
				animated = (prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED) ? true: false;
				endTime = prop.KeyNodeRange[1];

				if (animated)
					{
					sprintf(tempStr, "%s<gx:Tour>\n", indentStr);
					fputs(tempStr, fKML);
					IndentMore();
					sprintf(tempStr, "%s<name>%s</name>\n", indentStr, curCamera->Name);
					fputs(tempStr, fKML);
					sprintf(tempStr, "%s<gx:Playlist>\n", indentStr);
					fputs(tempStr, fKML);
					IndentMore();
					} // if
				else
					{
					sprintf(tempStr, "%s<Placemark>\n", indentStr);
					fputs(tempStr, fKML);
					IndentMore();
					sprintf(tempStr, "%s<name>%s</name>\n", indentStr, curCamera->Name);
					fputs(tempStr, fKML);
					} // else

				for (loopTime = 0.0; loopTime <= endTime; loopTime += loopInt)
					{
					if (loopTime != 0.0)
						{
						sprintf(tempStr, "%s<gx:Wait><gx:duration>%lf</gx:duration></gx:Wait>\n", indentStr, loopTime - lastTime);
						fputs(tempStr, fKML);
						lastTime = loopTime;
						} // if
					rendData.RenderTime = loopTime;
					curCamera->SetToTime(loopTime);
					if (curCamera->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED && curCamera->TargetObj)
						curCamera->TargetObj->SetToTime(loopTime);
					curCamera->InitFrameToRender(GlobalApp->AppEffects, &rendData);

					// make a false target 1000m from camera
					curCamera->TargPos->XYZ[0] = curCamera->CamPos->XYZ[0] + curCamera->TargPos->XYZ[0] * 1000.0;
					curCamera->TargPos->XYZ[1] = curCamera->CamPos->XYZ[1] + curCamera->TargPos->XYZ[1] * 1000.0;
					curCamera->TargPos->XYZ[2] = curCamera->CamPos->XYZ[2] + curCamera->TargPos->XYZ[2] * 1000.0;

					#ifdef WCS_BUILD_VNS
					rendData.DefCoords->CartToDeg(curCamera->CamPos);
					rendData.DefCoords->CartToDeg(curCamera->TargPos);
					#else // WCS_BUILD_VNS
					curCamera->CamPos->CartToDeg(RendData.PlanetRad);
					curCamera->TargPos->CartToDeg(RendData.PlanetRad);
					#endif // WCS_BUILD_VNS

					Master->RBounds.DefDegToRBounds(curCamera->CamPos->Lat, curCamera->CamPos->Lon, cx, cy);
					Master->RBounds.DefDegToRBounds(curCamera->TargPos->Lat, curCamera->TargPos->Lon, tx, ty);

					roll = curCamera->CamBank;
					if (fabs(roll) > 180.0)
						roll = fmod(roll, 180.0);
					if (roll < 0.0)
						roll += 180.0;

					heading = curCamera->CamHeading;
					if (fabs(heading) > 360.0)
						heading = fmod(heading, 360.0);
					if (heading < 0.0)
						heading += 360.0;

					// tilt: 0 = down, 90 = horizon, > 90 = sky (clamped to 180)
					tilt = 90.0 - curCamera->CamPitch;
					if (tilt < 0.0)
						tilt += 360.0;
					if (fabs(tilt) > 180.0)
						tilt = 180.0;

					if (animated)
						{
						sprintf(tempStr, "%s<gx:FlyTo>\n", indentStr);
						fputs(tempStr, fKML);
						IndentMore();
						sprintf(tempStr, "%s<gx:flyToMode>smooth</gx:flyToMode>\n", indentStr);
						fputs(tempStr, fKML);
						} // if

					sprintf(tempStr, "%s<Camera>\n", indentStr);
					fputs(tempStr, fKML);
					IndentMore();
					sprintf(tempStr, "%s<longitude>%lf</longitude>\n", indentStr, cx);
					fputs(tempStr, fKML);
					sprintf(tempStr, "%s<latitude>%lf</latitude>\n", indentStr, cy);
					fputs(tempStr, fKML);
					sprintf(tempStr, "%s<altitude>%lf</altitude>\n", indentStr, curCamera->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue);
					fputs(tempStr, fKML);
					sprintf(tempStr, "%s<heading>%lf</heading>\n", indentStr, heading);
					fputs(tempStr, fKML);
					sprintf(tempStr, "%s<tilt>%lf</tilt>\n", indentStr, tilt);
					fputs(tempStr, fKML);
					sprintf(tempStr, "%s<roll>%lf</roll>\n", indentStr, roll);
					fputs(tempStr, fKML);
					sprintf(tempStr, "%s<altitudeMode>absolute</altitudeMode>\n", indentStr);
					fputs(tempStr, fKML);
					IndentLess();
					sprintf(tempStr, "%s</Camera>\n", indentStr);
					fputs(tempStr, fKML);
					if (animated)
						{
						IndentLess();
						sprintf(tempStr, "%s</gx:FlyTo>\n", indentStr);
						fputs(tempStr, fKML);
						} // if

					/***
					// Slight offset so camera ends up with North up.  Copying X to try to prevent any rotation.
					if ((WCS_EFFECTS_CAMERATYPE_OVERHEAD == curCamera->CameraType) || (WCS_EFFECTS_CAMERATYPE_PLANIMETRIC == curCamera->CameraType))
						{
						tx = cx;
						ty = cy + 0.00001;
						} // if
					***/
					} // for

				if (animated)
					{
					IndentLess();
					sprintf(tempStr, "%s</gx:Playlist>\n", indentStr);
					fputs(tempStr, fKML);
					IndentLess();
					sprintf(tempStr, "%s</gx:Tour>\n", indentStr);
					fputs(tempStr, fKML);
					} // if
				else
					{
					IndentLess();
					sprintf(tempStr, "%s</Placemark>\n", indentStr);
					fputs(tempStr, fKML);
					} // else

				} // if RendData
			else
				success = 0;
			} // if
		camList = camList->Next;
		} // while
	} // if

return(success);

} // ExportFormatKML::ExportCameras

#else // GE50

// for KML 2.2+
int ExportFormatKML::ExportCameras(void)
{
class EffectList *camList = Master->Cameras;
class Camera *cam;

sprintf(tempStr, "%s<Folder>\n%s\t<name>Cameras</name>\n", indentStr, indentStr);
fputs(tempStr, fKML);
IndentMore();
while (camList)
	{
	double camAlt, camLat, camLon;
	Point3d orient;

	cam = (class Camera *)camList->Me;
	sprintf(tempStr, "%s<Placemark>\n", indentStr);
	fputs(tempStr, fKML);
	IndentMore();
	sprintf(tempStr, "%s<name>%s</name>\n", indentStr, cam->Name);
	fputs(tempStr, fKML);
	sprintf(tempStr, "%s<Camera>\n", indentStr);
	fputs(tempStr, fKML);
	Master->RBounds.DefDegToRBounds(cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].CurValue,
		cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].CurValue, camLon, camLat);
	sprintf(tempStr, "%s\t<longitude>%f</longitude>\n", indentStr, camLon);
	fputs(tempStr, fKML);
	sprintf(tempStr, "%s\t<latitude>%f</latitude>\n", indentStr, camLat);
	fputs(tempStr, fKML);
	camAlt = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].CurValue - Master->ExportRefData.RefElev;
	sprintf(tempStr, "%s\t<altitude>%f</altitude>\n", indentStr, camAlt);
	fputs(tempStr, fKML);
	if (cam->CameraType == WCS_EFFECTS_CAMERATYPE_UNTARGETED)
		{
		orient[0] = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].CurValue;
		orient[1] = -cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].CurValue;
		orient[2] = cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].CurValue;
		} // if untargeted
	else if (cam->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED)
		{
		orient[0] = cam->CamHeading;
		orient[1] = -cam->CamPitch;
		orient[2] = cam->CamBank;
		} // else if targeted
	else if (cam->CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD)
		{
		orient[0] = cam->CamHeading;
		orient[1] = -cam->CamPitch;
		orient[2] = cam->CamBank;
		} // else if overhead
	else if (cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC)
		{
		orient[0] = 0.0;
		orient[1] = -90.0;
		orient[2] = 0.0;
		} // else if planimetric
	if (cam->CameraType == WCS_EFFECTS_CAMERATYPE_ALIGNED)
		{
		orient[0] = cam->CamHeading;
		orient[1] = -cam->CamPitch;
		orient[2] = cam->CamBank;
		} // if untargeted
	orient[1] += 90.0;	// 0 is straight down, 90 is horizon
	sprintf(tempStr, "%s\t<heading>%f</heading>\n", indentStr, orient[0]);
	fputs(tempStr, fKML);
	sprintf(tempStr, "%s\t<tilt>%f</tilt>\n", indentStr, orient[1]);
	fputs(tempStr, fKML);
	sprintf(tempStr, "%s\t<roll>%f</roll>\n", indentStr, orient[2]);
	fputs(tempStr, fKML);
	// don't need to write next string as it is the default
	//sprintf(tempStr, "%s\t<altitudeMode>relativeToGround</altitudeMode>\n", indentStr);
	//fputs(tempStr, fKML);
	sprintf(tempStr, "%s</Camera>\n", indentStr);
	fputs(tempStr, fKML);
	IndentLess();
	sprintf(tempStr, "%s</Placemark>\n", indentStr);
	fputs(tempStr, fKML);

	camList = camList->Next;
	} // while CamList

IndentLess();
sprintf(tempStr, "%s</Folder>\n", indentStr);
fputs(tempStr, fKML);

return 1;

} // ExportFormatKML::ExportCameras

#endif // GE50

#endif // older_KML

/*===========================================================================*/

// Create KML link for each foliage stem
int ExportFormatKML::ExportFoliage(NameList **fileNamesCreated)
{
double ref_x, ref_y;
FILE *ffile;
const char *indexFileName, *dataFileName;
char *dot3, *elevAdjust;
long fileType;
int Success = 1;
RealtimeFoliageIndex index;
RealtimeFoliageCellData rfcd;
RealtimeFoliageData folData;
char fileName[512], pathName[512];
char testFileVersion;

if (elevAdjust = GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("GE_ElevMod"))
	elevMod = (float)atof(elevAdjust);

ref_x = Master->ExportRefData.ExportRefLon;
ref_y = Master->ExportRefData.ExportRefLat;

dot3 = ImageSaverLibrary::GetDefaultExtension(Master->FoliageImageFormat);

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
				long proceed = 1;

				strcpy(pathName, Master->OutPath.GetPath());
				strcat(pathName, "/images/");
				PROJ_mkdir(pathName);

				strmfp(fileName, Master->OutPath.GetPath(), dataFileName);
				if (ffile = PROJ_fopen(fileName, "rb"))
					{
					fgets(fileName, 64, ffile);
					// version
					fread((char *)&testFileVersion, sizeof(char), 1, ffile);
					strcpy(pathName, Master->OutPath.GetPath());
					strcat(pathName, "/models/");
					// try to create subdir - ok if it already exists
					derr = PROJ_mkdir(pathName);
					if ((ERROR_SUCCESS != derr) && (ERROR_FILE_EXISTS != derr) && (ERROR_ALREADY_EXISTS != derr))
						{
						char msg[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN + 128];

						proceed = 0;	// we couldn't make our subdir, so abort processing on this object
						sprintf(&msg[0], "%s: Unable to create directory '%s'. Skipping foliage", msgGEE, pathName);
						GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, msg);
						} // if
		
					if (proceed && (testFileVersion == index.FileVersion))
						{
						Raster *curRast;
						FoliagePreviewData pointData;
						long datPt;
						short curID, firstID = 32767, lastID = 0;
						char junk[80];

						// scan for ID range first
						for (datPt = 0; datPt < index.CellDat->DatCt; datPt++)
							{
							if (folData.ReadFoliageRecord(ffile, index.FileVersion))
								{
								folData.InterpretFoliageRecord(NULL, GlobalApp->AppImages, &pointData);
								if (!(folData.BitInfo & WCS_REALTIME_FOLDAT_BITINFO_LABEL))
									{
									if (folData.ElementID < firstID)
										firstID = folData.ElementID;
									if (folData.ElementID > lastID)
										lastID = folData.ElementID;
									} // if
								} // if
							} // if

						// Write to parent file
						sprintf(tempStr, "%s<Folder>\n", indentStr);
						fputs(tempStr, fKML);
						IndentMore();
						sprintf(tempStr, "%s<name>Foliage</name>\n", indentStr);
						fputs(tempStr, fKML);

						// Each species gets it's own placemark
						for (curID = firstID; curID <= lastID; curID++)
							{
							long created = false;

							rewind(ffile);
							fgets(junk, 64, ffile);
							// version
							fread((char *)&testFileVersion, sizeof (char), 1, ffile);
							// Pointless version check -- we know we wrote it
							if (testFileVersion == index.FileVersion)
								{
								bool absolute;

								absolute = GlobalApp->MainProj->Prefs.PublicQueryConfigOptTrue("GE_Absolute");
								for (datPt = 0; datPt < index.CellDat->DatCt; datPt ++)
									{
									if (folData.ReadFoliageRecord(ffile, index.FileVersion))
										{
										if (folData.InterpretFoliageRecord(NULL, Images, &pointData)) // don't need full decoding of 3dobjects, just height, etc
											{
											if ((folData.ElementID == curID) && (curRast = Images->FindByID(folData.ElementID)) &&
												(curRast->InitFlags & WCS_RASTER_INITFLAGS_FOLIAGELOADED))
												{
												float tx, ty, tz, w, h;
												char DAEName[64];

												tx = (float)(folData.XYZ[0] + -Master->ExportRefData.WCSRefLon);
												ty = (float)(folData.XYZ[1] + Master->ExportRefData.WCSRefLat);
												if (absolute)
													tz = (float)(folData.XYZ[2] + Master->ExportRefData.RefElev);
												else
													tz = 0.0f + elevMod;

												w = (float)pointData.Width;
												h = (float)pointData.Height;
												if (pointData.FlipX)
													w *= -1.0f;

												if (! created)
													{
													strcpy(matName, curRast->GetUserName());
													StripExtension(matName);
													FixMatName(matName);
													sprintf(tempStr, "%s<Placemark>\n", indentStr);
													fputs(tempStr, fKML);
													sprintf(tempStr, "%s<name>%s</name>\n", indentStr, matName);
													fputs(tempStr, fKML);
													IndentMore();
													sprintf(tempStr, "%s<MultiGeometry>\n", indentStr);
													fputs(tempStr, fKML);
													IndentMore();
													created = 1;

													// Generate a COLLADA file for each species
													strcat(matName, "_Fol");
													strcpy(DAEName, matName);
													strcat(DAEName, ".dae");
													CreateTree(matName);
													} // if

												sprintf(tempStr, "%s<Model>\n", indentStr);
												fputs(tempStr, fKML);
												IndentMore();
												if (absolute)
													sprintf(tempStr, "%s<altitudeMode>absolute</altitudeMode>\n", indentStr);
												else
													sprintf(tempStr, "%s<altitudeMode>relativeToGround</altitudeMode>\n", indentStr);
												fputs(tempStr, fKML);
												sprintf(tempStr, "%s<Location>\n", indentStr);
												fputs(tempStr, fKML);
												sprintf(tempStr, "  %s<longitude>%lf</longitude>\n", indentStr, tx);
												fputs(tempStr, fKML);
												sprintf(tempStr, "  %s<latitude>%lf</latitude>\n", indentStr, ty);
												fputs(tempStr, fKML);
												sprintf(tempStr, "  %s<altitude>%f</altitude>\n", indentStr, tz);
												fputs(tempStr, fKML);
												sprintf(tempStr, "%s</Location>\n", indentStr);
												fputs(tempStr, fKML);
												/***
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
												***/
												sprintf(tempStr, "%s<Scale>\n", indentStr);
												fputs(tempStr, fKML);
												sprintf(tempStr, "%s\t<x>%f</x>\n", indentStr, w);
												fputs(tempStr, fKML);
												sprintf(tempStr, "%s\t<y>%f</y>\n", indentStr, w);
												fputs(tempStr, fKML);
												sprintf(tempStr, "%s\t<z>%f</z>\n", indentStr, h);
												fputs(tempStr, fKML);
												sprintf(tempStr, "%s</Scale>\n", indentStr);
												fputs(tempStr, fKML);
												sprintf(tempStr, "%s<Link>\n", indentStr);
												fputs(tempStr, fKML);
												sprintf(tempStr, "%s\t<href>models/%s</href>\n", indentStr, DAEName);
												fputs(tempStr, fKML);
												sprintf(tempStr, "%s</Link>\n", indentStr);
												fputs(tempStr, fKML);
												IndentLess();
												sprintf(tempStr, "%s</Model>\n", indentStr);
												fputs(tempStr, fKML);
												} // if
											} // if
										} // if
									} // for
								} // if
							// Write to parent file
							if (created)
								{
								IndentLess();
								sprintf(tempStr, "%s</MultiGeometry>\n", indentStr);
								fputs(tempStr, fKML);
								IndentLess();
								sprintf(tempStr, "%s</Placemark>\n", indentStr);
								fputs(tempStr, fKML);
								created = false;
								} // if
							} // for

						IndentLess();
						sprintf(tempStr, "%s</Folder>\n", indentStr);
						fputs(tempStr, fKML);

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

return (Success);

} // ExportFormatKML::ExportFoliage

/*===========================================================================*/

int ExportFormatKML::ExportGroundTextures(NameList **fileNamesCreated)
{
double n, s, e, w;
const char *textureName = NULL;
long fileType;
bool tiled = false;
char fullPathAndName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN], pathName[WCS_PATHANDFILE_PATH_LEN];
char drawOrder = 0;

if (Master->FormatExtension && (Master->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_GE))
	drawOrder = ((SXExtensionGE *)Master->FormatExtension)->DrawOrder;

if ((Master->TexTilesX > 1) || (Master->TexTilesY > 1))
	tiled = true;

sprintf(tempStr, "%s<Folder>\n%s\t<name>Ground Textures</name>\n", indentStr, indentStr);
fputs(tempStr, fKML);
IndentMore();

strcpy(pathName, Master->OutPath.GetPath());
strcat(pathName, "/images/");
PROJ_mkdir(pathName);

fileType = WCS_EXPORTCONTROL_FILETYPE_TEX1;

for (long y = 0; y < Master->TexTilesY; y++)
	{
	for (long x = 0; x < Master->TexTilesX; x++)
		{
		Master->RBounds.DeriveTileCoords(Master->TexResY, Master->TexResX, 
			Master->TexTilesY, Master->TexTilesX, y, x, Master->TexTileOverlap);

		w = Master->RBounds.ULcorner.x;
		n = Master->RBounds.ULcorner.y;
		e = Master->RBounds.LRcorner.x;
		s = Master->RBounds.LRcorner.y;

		sprintf(tempStr, "%s<GroundOverlay>\n", indentStr);
		fputs(tempStr, fKML);
		IndentMore();

		if (tiled)
			{
			textureName = (*fileNamesCreated)->FindNextNameOfType(fileType, textureName);
			sprintf(tempStr, "%s<name>ExportTexture_%dy_%dx</name>\n", indentStr, y, x);
			fputs(tempStr, fKML);
			} // if
		else
			{
			textureName = (*fileNamesCreated)->FindNameOfType(fileType);
			sprintf(tempStr, "%s<name>ExportTexture</name>\n", indentStr);
			fputs(tempStr, fKML);
			} // else

		sprintf(tempStr, "%s<drawOrder>%d</drawOrder>\n", indentStr, drawOrder);
		fputs(tempStr, fKML);
		sprintf(tempStr, "%s<Icon>\n%s\t<href>images/%s</href>\n%s</Icon>\n", indentStr, indentStr, textureName, indentStr);
		fputs(tempStr, fKML);
		strcpy(fullPathAndName, Master->OutPath.GetPath());
		strcat(fullPathAndName, "/");
		strcat(fullPathAndName, textureName);
		strcpy(pathName, Master->OutPath.GetPath());
		strcat(pathName, "/images/");
		CopyExistingFile(fullPathAndName, pathName, textureName);

		if (zippy)
			{
			sprintf(tempStr, "images/%s", textureName);	// Do NOT add leading slash to zip string!
			zippy->AddToZipList(tempStr);
			} // if

		sprintf(tempStr, "%s<LatLonBox>\n%s\t<north>%f</north>\n%s\t<south>%f</south>\n%s\t<east>%f</east>\n%s\t<west>%f</west>\n%s</LatLonBox>\n",
			indentStr, indentStr, n, indentStr, s, indentStr, e, indentStr, w, indentStr);
		fputs(tempStr, fKML);

		IndentLess();

		sprintf(tempStr, "%s</GroundOverlay>\n", indentStr);
		fputs(tempStr, fKML);
		} // for
	} // for

IndentLess();
sprintf(tempStr, "%s</Folder>\n", indentStr);
fputs(tempStr, fKML);

return 1;

} // ExportFormatKML::ExportGroundTexture

/*===========================================================================*/

int ExportFormatKML::ExportLabels(NameList **fileNamesCreated)
{
int Success = 1;
#ifdef WCS_BUILD_RTX
double lx, ly, lz;
double ref_x, ref_y, ref_z;
FILE *ffile;
long fileType;
const char *indexFileName, *dataFileName;
RealtimeFoliageIndex index;
RealtimeFoliageCellData rfcd;
RealtimeFoliageData folData;
char fileName[512];
char fullPathAndName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN], pathName[WCS_PATHANDFILE_PATH_LEN];
char testFileVersion;

ref_x = Master->ExportRefData.ExportRefLon;
ref_y = Master->ExportRefData.ExportRefLat;
ref_z = Master->ExportRefData.RefElev;

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
				strcpy(pathName, Master->OutPath.GetPath());
				strcat(pathName, "/images/");
				PROJ_mkdir(pathName);

				strmfp(fileName, Master->OutPath.GetPath(), dataFileName);
				if (ffile = PROJ_fopen(fileName, "rb"))
					{
					fgets(fileName, 64, ffile);
					// version
					fread((char *)&testFileVersion, sizeof(char), 1, ffile);
					// Pointless version check -- we know we wrote it
					if (testFileVersion == index.FileVersion)
						{
						Raster *curRast;
						static long labelsExist = 0;
						long datPt;
						FoliagePreviewData pointData;

						// output labels
						rewind(ffile);
						fgets(fileName, 64, ffile);
						fread((char *)&testFileVersion, sizeof(char), 1, ffile);
						for (datPt = 0; datPt < index.CellDat->DatCt; datPt++)
							{
							if (folData.ReadFoliageRecord(ffile, index.FileVersion))
								{
								//double relSize;
								SXQueryActionList *actionList = NULL;
								char imageName[64];

								folData.InterpretFoliageRecord(NULL, GlobalApp->AppImages, &pointData);

								if ((folData.BitInfo & WCS_REALTIME_FOLDAT_BITINFO_CLICKQUERY) && folData.MyEffect) // click-to-query
									{
									if (folData.MyEffect->EffectType == WCS_EFFECTSSUBCLASS_LABEL)
										actionList = ((FoliageEffect *)folData.MyEffect)->ActionList;
									} // if

								if ((folData.BitInfo & WCS_REALTIME_FOLDAT_BITINFO_LABEL) && (curRast = Images->FindByID(folData.ElementID)))
									{
									if (! labelsExist)
										{
										labelsExist = 1;
										sprintf(tempStr, "%s<Folder>\n%s\t<name>Labels</name>\n", indentStr, indentStr);
										fputs(tempStr, fKML);
										IndentMore();
										} // if
									//relSize = pointData.Height / minHeight;
									strcpy(imageName, curRast->GetUserName());
									ImageSaverLibrary::StripImageExtension(imageName);
									strcat(imageName, "_Fol");
									ReplaceChar(imageName, '.', '_');
									lx = folData.XYZ[0] + ref_x;
									ly = folData.XYZ[1] + ref_y;
									lz = folData.XYZ[2] * Master->ExportRefData.ElevScale + ref_z;
									sprintf(tempStr, "%s<Placemark>\n", indentStr);
									fputs(tempStr, fKML);
									IndentMore();
									if (actionList)
										Describe(actionList, folData.MyEffect, folData.MyVec);
									sprintf(tempStr, "%s<styleUrl>#sm_%s</styleUrl>\n", indentStr, imageName);
									fputs(tempStr, fKML);
									//sprintf(tempStr, "%s<Style>\n", indentStr);
									//fputs(tempStr, fKML);
									strcat(imageName, ImageSaverLibrary::GetDefaultExtension(Master->FoliageImageFormat));
									strcpy(fullPathAndName, Master->OutPath.GetPath());
									strcat(fullPathAndName, "/");
									strcat(fullPathAndName, imageName);
									strcpy(pathName, Master->OutPath.GetPath());
									strcat(pathName, "/images");
									CopyExistingFile(fullPathAndName, pathName, imageName);
									if (zippy)
										{
										strcpy(fullPathAndName, "images/");	// Do NOT add leading slash to zip string!
										strcat(fullPathAndName, imageName);
										zippy->AddToZipList(fullPathAndName);
										} // if
									//IndentMore();
									//sprintf(tempStr, "%s<LineStyle>\n%s\t<width>2</width>\n%s</LineStyle>\n", indentStr, indentStr, indentStr);
									//sprintf(tempStr, "%s<IconStyle>\n%s\t<scale>%f</scale>\n", indentStr, indentStr, rescale * relSize);
									//fputs(tempStr, fKML);
									//sprintf(tempStr, "%s\t\t<href>%s</href>\n", indentStr, imageName);
									//fputs(tempStr, fKML);
									//IndentLess();
									//sprintf(tempStr, "%s\t</IconStyle>\n%s</Style>\n", indentStr, indentStr);
									//fputs(tempStr, fKML);
									KML_Point(lx, ly, lz);
									IndentLess();
									sprintf(tempStr, "%s</Placemark>\n", indentStr);
									fputs(tempStr, fKML);
									} // if
								} // if
							} // for
						if (labelsExist)
							{
							IndentLess();
							sprintf(tempStr, "%s</Folder>\n", indentStr);
							fputs(tempStr, fKML);
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
#endif // WCS_BUILD_RTX

return (Success);

} // ExportFormatKML::ExportLabels

/*===========================================================================*/

int ExportFormatKML::ExportScreenOverlays(void)
{
int success = 1;
bool netLink = false;
PathAndFile paf;
char url[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];
char tempFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

((SXExtensionGE *)Master->FormatExtension)->overlayFilename.GetPathAndName(url);
if (url[0])
	{
	if (strncmp("http://", url, 7) == 0)
		netLink = true;
	if (! netLink)
		{
		strcpy(tempFullPath, GlobalApp->MainProj->MungPath(url));
		BreakFileName(tempFullPath, paf.Path, WCS_PATHANDFILE_PATH_LEN, paf.Name, WCS_PATHANDFILE_NAME_LEN);
		strcpy(url, (char *)Master->OutPath.GetPath());
		strcat(url, "/images/");
		PROJ_mkdir(url);
		CopyExistingFile(tempFullPath, url, paf.Name);
		} // if

	sprintf(tempStr, "%s<ScreenOverlay>\n", indentStr);
	fputs(tempStr, fKML);
	IndentMore();
	sprintf(tempStr, "%s<name>Screen Overlay</name>\n", indentStr);
	fputs(tempStr, fKML);
	if (netLink)
		{
		sprintf(tempStr, "%s<Icon>\n%s\t<href>%s</href>\n%s</Icon>\n",
				indentStr, indentStr, url, indentStr);
		} // if
	else
		{
		sprintf(tempStr, "%s<Icon>\n%s\t<href>images/%s</href>\n%s</Icon>\n",
				indentStr, indentStr, paf.Name, indentStr);
		} // else
	fputs(tempStr, fKML);
	sprintf(tempStr, "%s<overlayXY x=\"%f\" y = \"%f\" xunits=\"fraction\" yunits=\"fraction\"/>\n",
			indentStr, ((SXExtensionGE *)Master->FormatExtension)->overlayX, ((SXExtensionGE *)Master->FormatExtension)->overlayY);
	fputs(tempStr, fKML);
	sprintf(tempStr, "%s<screenXY x=\"%f\" y = \"%f\" xunits=\"fraction\" yunits=\"fraction\"/>\n",
			indentStr, ((SXExtensionGE *)Master->FormatExtension)->overlayX, ((SXExtensionGE *)Master->FormatExtension)->overlayY);
	fputs(tempStr, fKML);
	IndentLess();
	sprintf(tempStr, "%s</ScreenOverlay>\n", indentStr);
	if (fputs(tempStr, fKML) < 0)
		success = 0;

	if (! netLink)
		{
		strcpy(url, Master->OutPath.GetName());
		strcpy(url, "/images/");
		strcat(url, paf.Name);
		if (zippy && zippy->NotInZipList(url))
			zippy->AddToZipList(url);
		} // if
	} // if

return success;

} // ExportFormatKML::ExportScreenOverlays

/*===========================================================================*/

int ExportFormatKML::ExportTerrain(NameList **fileNamesCreated)
{
double geoCtrLon, geoCtrLat, lonScale;
FILE *fRaw, *fCOLLADA;
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
bool tiled = false;
long tilesizex, tilesizey;
char pathName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];
//char modelName[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];
char tempFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];
//char dmsg[256];

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
	xLimit = Master->OneDEMResX;
	yLimit = Master->OneDEMResY;
	} // if
else
	{
	xLimit = Master->OneDEMResX;
	yLimit = Master->OneDEMResY;
	} // else

if ((Master->DEMTilesX > 1) || (Master->DEMTilesY > 1))
	{
	tiled = true;
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
// try to create subdir - ok if it already exists
derr = PROJ_mkdir(pathName);
if ((ERROR_SUCCESS != derr) && (ERROR_FILE_EXISTS != derr) && (ERROR_ALREADY_EXISTS != derr))
	{
	char msg[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN + 128];

	sprintf(&msg[0], "KML Exporter: Unable to create directory '%s'. Terrain skipped", pathName);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, msg);
	success = 0;
	} // if

if (success && Master->ExportTexture)
	{
	strcpy(tempFullPath, (char *)Master->OutPath.GetPath());
	strcpy(pathName, GlobalApp->MainProj->MungPath(tempFullPath));
	strcat(pathName, "/images/");
	// try to create subdir - ok if it already exists
	derr = PROJ_mkdir(pathName);
	if ((ERROR_SUCCESS != derr) && (ERROR_FILE_EXISTS != derr) && (ERROR_ALREADY_EXISTS != derr))
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

	sx = (float)((xLimit * Master->RBounds.CellSizeX) * Master->ExportRefData.ExportLonScale);	// size x
	sy = (float)((yLimit * Master->RBounds.CellSizeY) * Master->ExportRefData.ExportLatScale);
	cx = sx * 0.5f;	// center x
	cy = sy * 0.5f;
	xspace = sx / (xLimit - 1);
	yspace = sy / (yLimit - 1);

	for (yTile = 0; yTile < Master->DEMTilesY; yTile++)
		{
		for (xTile = 0; xTile < Master->DEMTilesX; xTile++)
			{
			float du, dv, xdist, ydist;
			char baseTexName[64], fullPathAndName[512], pathName[512], tileName[32];

			if (fRaw)
				{
				long ndx;

				if (tiled)
					{
					// 070615 - this should be correct, but doesn't give results I need.  Gary's out of town right now, so hacking results myself
					//Master->RBounds.DeriveTileCoords(Master->DEMResY, Master->DEMResX, Master->DEMTilesY, Master->DEMTilesX,
					//	yTile, xTile, Master->DEMTileOverlap);
					Master->RBounds.DeriveCoords(Master->DEMResY, Master->DEMResX);
					// assume cell edges only for now
					xdist = (float)(Master->RBounds.LRcorner.x - Master->RBounds.ULcorner.x);
					ydist = (float)(Master->RBounds.ULcorner.y - Master->RBounds.LRcorner.y);
					xdist /= Master->DEMTilesX;
					ydist /= Master->DEMTilesY;
					xloc = (float)(Master->RBounds.ULcorner.x + xTile * xdist + xdist * 0.5);
					yloc = (float)(Master->RBounds.ULcorner.y - yTile * ydist - ydist * 0.5);
					} // if
				else
					{
					Master->RBounds.DeriveCoords(Master->DEMResY, Master->DEMResX);
					xloc = (float)((Master->RBounds.ULcenter.x + Master->RBounds.LRcenter.x) * 0.5);
					yloc = (float)((Master->RBounds.ULcenter.y + Master->RBounds.LRcenter.y) * 0.5);
					} // else

				//xloc = (float)((Master->RBounds.ULcenter.x + Master->RBounds.LRcenter.x) * 0.5);
				//yloc = (float)((Master->RBounds.ULcenter.y + Master->RBounds.LRcenter.y) * 0.5);
				//xdist = (tilesizex - 1) * xspace;
				//ydist = (tilesizey - 1) * yspace;

				if (Master->ExportTexture)
					{
					strcpy(baseTexName, textureFile);
					(void)StripExtension(baseTexName);

					strcpy(fullPathAndName, Master->OutPath.GetPath());
					strcat(fullPathAndName, "/");
					strcat(fullPathAndName, textureFile);
					strcpy(pathName, Master->OutPath.GetPath());
					strcat(pathName, "/images/");
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
					if (zippy)
						{
						strcpy(tempFullPath, tileName);
						zippy->AddToZipList(tempFullPath);
						} // if

					sprintf(tileName, "DEM_%02dx_%02dy", xTile, yTile);

					C_prologue_asset(fCOLLADA, 2);

					// library_effects
					sprintf(tempStr, "\t<library_effects>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t<effect id=\"%s-effect\">\n", baseTexName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t<profile_COMMON>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t<newparam sid=\"%s-surface\">\n", baseTexName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t<surface type=\"2D\">\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t<init_from>%s-image</init_from>\n", baseTexName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t<format>A8R8G8B8</format>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t</surface>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t</newparam>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t<newparam sid=\"%s-sampler\">\n", baseTexName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t<sampler2D>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t<source>%s-surface</source>\n", baseTexName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t<minfilter>LINEAR_MIPMAP_LINEAR</minfilter>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t<magfilter>LINEAR</magfilter>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t</sampler2D>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t</newparam>\n");
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
						sprintf(tempStr, "\t\t\t\t\t\t\t<texture texture=\"%s-sampler\" texcoord=\"TEX0\"/>\n", baseTexName);
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
						baseTexName, (xLimit - 1) * (yLimit - 1) * 2);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t<input semantic=\"VERTEX\" source=\"#%s-geometry-vertex\" offset=\"0\"/>\n", tileName);
					fputs(tempStr, fCOLLADA);
					// since our vertex & uv arrays are coincident, share the p elements
					sprintf(tempStr, "\t\t\t\t\t<input semantic=\"TEXCOORD\" source=\"#%s-geometry-uv\" offset=\"0\" set=\"0\"/>\n", tileName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t<p> ");
					fputs(tempStr, fCOLLADA);
					for (y = 0; y < (yLimit - 1); y++)
						{
						ndx = y * xLimit;
						for (x = 0; x < (xLimit - 1); x++)
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

					// library_images
					if (Master->ExportTexture)
						{
						sprintf(tempStr, "\t<library_images>\n");
						fputs(tempStr, fCOLLADA);
						sprintf(tempStr, "\t\t<image id=\"%s-image\">\n", baseTexName);
						fputs(tempStr, fCOLLADA);
						sprintf(tempStr, "\t\t\t<init_from>../images/%s</init_from>\n", textureFile);
						fputs(tempStr, fCOLLADA);
						//strcpy(pathName, Master->OutPath.GetPath());
						//strcat(pathName, "/images");
						//CopyExistingFile(fullPathAndName, pathName, textureFile);
						if (zippy)
							{
							strcpy(tempFullPath, "images/");	// Do NOT add leading slash to zip string!
							strcat(tempFullPath, textureFile);
							zippy->AddToZipList(tempFullPath);
							} // if
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

					// library_visual_scenes
					sprintf(tempStr, "\t<library_visual_scenes>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t<visual_scene id=\"Scene_Express_Scene\">\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t<node id=\"%s\">\n", tileName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t<instance_geometry url=\"#%s-geometry\">\n", tileName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t<bind_material>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t<technique_common>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t\t<instance_material symbol=\"%s-material\" target=\"#%s-material\">\n",
						baseTexName, baseTexName);
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t\t\t<bind_vertex_input semantic=\"TEX0\" input_semantic=\"TEXCOORD\" input_set=\"0\"/>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t\t</instance_material>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t\t</technique_common>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t\t</bind_material>\n");
					fputs(tempStr, fCOLLADA);
					sprintf(tempStr, "\t\t\t\t</instance_geometry>\n");
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

					if (tiled)
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

} // ExportFormatKML::ExportTerrain

/*===========================================================================*/

int ExportFormatKML::ExportVectors(void)
{
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

} // ExportFormatKML::ExportVectors

/*===========================================================================*/

void ExportFormatKML::IndentLess(void)
{
unsigned long i;

if (curIndent > 0)
	curIndent--;
i = curIndent;
indentStr[i] = NULL;

} // ExportFormatKML::IndentLess

/*===========================================================================*/

void ExportFormatKML::IndentMore(void)
{
unsigned long i = curIndent;

curIndent++;
indentStr[i] = '\t';
indentStr[i+1] = 0;

} // ExportFormatKML::IndentMore

/*===========================================================================*/

int ExportFormatKML::KML_Point(double &px, double &py, double &pz)
{

sprintf(tempStr, "%s<Point>\n%s\t<extrude>1</extrude>\n%s\t<tessellate>0</tessellate>\n", indentStr, indentStr, indentStr);
fputs(tempStr, fKML);
sprintf(tempStr, "%s\t<coordinates>%f,%f,%f</coordinates>\n%s</Point>\n", indentStr, px, py, pz, indentStr);
fputs(tempStr, fKML);

return 1;

} // ExportFormatKML::KML_Point

/*===========================================================================*/

int ExportFormatKML::PackageExport(NameList **fileNamesCreated)
{
int success = 1;
PathAndFile paf;
char tempFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

warningCheck = false;

strcpy(tempFullPath, (char *)Master->OutPath.GetPath());

if (Master->ZipItUp)
	{
	if (zippy = new Zipper)
		{
		strcpy(tempFullPath, (char *)Master->OutPath.GetPath());
		zippy->SetBaseDirectory(GlobalApp->MainProj->MungPath(tempFullPath));
		} // if
	} // if

paf.SetPath((char *)Master->OutPath.GetPath());
paf.SetName((char *)Master->OutPath.GetName());
paf.GetFramePathAndName(tempFullPath, ".kml", 0, WCS_PATHANDFILE_PATH_PLUS_NAME_LEN, 0);

fKML = PROJ_fopen(tempFullPath, "w");
fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", fKML);
#ifdef GE50
fputs("<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\" xmlns:kml=\"http://www.opengis.net/kml/2.2\" xmlns:atom=\"http://www.w3.org/2005/Atom\">\n", fKML);
#else // GE50
//fputs("<kml xmlns=\"http://earth.google.com/kml/2.1\">\n", fKML);
fputs("<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n", fKML);
#endif // GE50
sprintf(tempStr, "<!-- Generated by " APP_TITLE " %s (%s) -->\n", ExtVersion, ExtAboutBuild);
fputs(tempStr, fKML); 

if (Master->FormatExtension && (Master->FormatExtension->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_GE) &&
	(((SXExtensionGE *)Master->FormatExtension)->Message[0]))
	{
	sprintf(tempStr, "%s<NetworkLinkControl>\n%s\t<message>%s</message>\n%s</NetworkLinkControl>\n",
		indentStr, indentStr, ((SXExtensionGE *)Master->FormatExtension)->Message, indentStr);
	fputs(tempStr, fKML);
	} // if

fputs("<Document>\n", fKML);
IndentMore();
sprintf(tempStr, "%s<name>%s</name>\n", indentStr, Master->OutPath.GetName());
fputs(tempStr, fKML);
sprintf(tempStr, "%s<description><![CDATA[A <a href=\"http://3dnature.com/scene.html\">Scene Express</a> export created by ", indentStr);
fputs(tempStr, fKML);
UTF8Encode(fKML, GlobalApp->MainProj->UserName);
sprintf(tempStr, "]]></description>\n");
fputs(tempStr, fKML);

if (Master->ExportLabels)
	success = DefineStyles(fileNamesCreated);

if (success)
	success = ExportScreenOverlays();

if (success && Master->ExportCameras)
	success = ExportCameras();

if (success && Master->ExportTerrain)
	success = ExportTerrain(fileNamesCreated);

if (success && Master->ExportTexture && !Master->ExportTerrain)
	success = ExportGroundTextures(fileNamesCreated);

if (success && Master->ExportVectors && Master->VecInstanceList)
	success = ExportVectors();

if (success && Master->ExportFoliage)
	success = ExportFoliage(fileNamesCreated);

if (success && Master->ExportLabels)
	success = ExportLabels(fileNamesCreated);

if (success && Master->ObjectInstanceList)
	success = Export3DObjs(fileNamesCreated);

IndentLess();	// indents not really used anymore though
fputs("</Document>\n", fKML);
fputs("</kml>\n", fKML);
fclose(fKML);

Cleanup(fileNamesCreated);

if (zippy)
	{
	sprintf(tempFullPath, "%s.kml", (char *)Master->OutPath.GetName());
	zippy->AddToZipList(tempFullPath);
	zippy->SetRemoveStateOnAll(true);
	sprintf(tempFullPath, "%s/%s.kmz", (char *)Master->OutPath.GetPath(), (char *)Master->OutPath.GetName());
	zippy->ZipToFile(GlobalApp->MainProj->MungPath(tempFullPath)); 
	delete zippy;
	zippy = NULL;

	sprintf(tempFullPath, "%s/images", (char *)Master->OutPath.GetPath());
	PROJ_rmdir(tempFullPath);

	sprintf(tempFullPath, "%s/models", (char *)Master->OutPath.GetPath());
	PROJ_rmdir(tempFullPath);
	} // if

if (warningCheck && (!Master->ZipItUp))
	UserMessageOK(msgGEE, "Internally displayed images only work in zipped exports.  Manually zip the files yourself, or re-export with compression enabled.");

return success;

} // ExportFormatKML::PackageExport

/*===========================================================================*/

void ExportFormatKML::ProcessGeometries(FILE *fDAE, Object3DEffect *curObj)
{

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
	fprintf(fDAE, " %f %f %f", (float)curObj->Polygons[polyCt].Normal[0], (float)curObj->Polygons[polyCt].Normal[1], (float)-curObj->Polygons[polyCt].Normal[2]);
	//fprintf(fDAE, " %f %f %f", (float)-curObj->Polygons[polyCt].Normal[0], (float)-curObj->Polygons[polyCt].Normal[1], (float)curObj->Polygons[polyCt].Normal[2]);
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
ProcessPolys(fDAE, curObj);
fprintf(fDAE, "\t\t\t</mesh>\n");
fprintf(fDAE, "\t\t</geometry>\n");

} // ExportFormatKML::ProcessGeometries

/*===========================================================================*/

// This pass generates any library images
void ExportFormatKML::ProcessMatTexture1(long matCt, FILE *fDAE, Object3DEffect *curObj)
{
MaterialEffect *mat;
RootTexture *rootTex;
Texture *uvTex;
Raster *rast;
long useImage = 0;
long diffuseAlphaAvailable = 0;
char fullPathAndName[512], pathName[512], textureName[256];

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

if (useImage)
	{
	//char noExtenName[256];

	rast->GetPathAndName(fullPathAndName);
	// our images are going to end up in a subdirectory called images
	strcpy(pathName, Master->OutPath.GetPath());
	strcat(pathName, "/images/");
	PROJ_mkdir(pathName);
	CopyExistingFile(fullPathAndName, pathName, textureName);

	//strcpy(noExtenName, textureName);
	//(void)StripExtension(noExtenName);
	//fprintf(fDAE, "      <image id=\"%s-image\" name=\"%s-image\">\n", textureName, textureName);
	//fprintf(fDAE, "      <image id=\"%s-image\">\n", noExtenName);
	FixMatName(mat->Name);
	fprintf(fDAE, "      <image id=\"%s-image\">\n", matName);
	fprintf(fDAE, "         <init_from>../images/%s</init_from>\n", textureName);
	fprintf(fDAE, "      </image>\n");

	if (zippy)
		{
		strcpy(fullPathAndName, "images/");	// Do NOT add leading slash to zip string!
		strcat(fullPathAndName, textureName);
		zippy->AddToZipList(fullPathAndName);
		} // if

	} // if

} // ExportFormatKML::ProcessMatTexture1

/*===========================================================================*/

// This pass generates any library materials
void ExportFormatKML::ProcessMatTexture2(long matCt, FILE *fDAE, Object3DEffect *curObj)
{
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

} // ExportFormatKML::ProcessMatTexture2

/*===========================================================================*/

// This pass generates any library effects
void ExportFormatKML::ProcessMatTexture3(long matCt, FILE *fDAE, Object3DEffect *curObj)
{
MaterialEffect *mat;
RootTexture *rootTex;
Texture *uvTex;
Raster *rast;
long useImage = 0;
char textureName[256];

if (! curObj->NameTable[matCt].Mat)
	curObj->NameTable[matCt].Mat = (MaterialEffect *)GlobalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, curObj->NameTable[matCt].Name);

if (mat = curObj->NameTable[matCt].Mat)
	{
	float fval0, fval1, fval2;//, fval3;
	char tab6[] = "\t\t\t\t\t\t", tab7[] = "\t\t\t\t\t\t\t";

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
		//else if (diffuseAlphaAvailable)
		} // if

	if (mat->Shading) // if it's not invisible
		{
		// intro
		FixMatName(mat->Name);
		fprintf(fDAE, "\t\t<effect id=\"%s-effect\">\n", matName);
		fprintf(fDAE, "\t\t\t<profile_COMMON>\n");

		// texture surface & sampler for images
		if (useImage)
			{
			fprintf(fDAE, "\t\t\t\t<newparam sid=\"%s-surface\">\n", matName);
			fprintf(fDAE, "\t\t\t\t\t<surface type=\"2D\">\n");
			fprintf(fDAE, "\t\t\t\t\t\t<init_from>%s-image</init_from>\n", matName);
			fprintf(fDAE, "\t\t\t\t\t\t<format>A8R8G8B8</format>\n");
			fprintf(fDAE, "\t\t\t\t\t</surface>\n");
			fprintf(fDAE, "\t\t\t\t</newparam>\n");

			fprintf(fDAE, "\t\t\t\t<newparam sid=\"%s-sampler\">\n", matName);
			fprintf(fDAE, "\t\t\t\t\t<sampler2D>\n");
			fprintf(fDAE, "\t\t\t\t\t\t<source>%s-surface</source>\n", matName);
			fprintf(fDAE, "\t\t\t\t\t\t<minfilter>LINEAR_MIPMAP_LINEAR</minfilter>\n");
			fprintf(fDAE, "\t\t\t\t\t\t<magfilter>LINEAR</magfilter>\n");
			fprintf(fDAE, "\t\t\t\t\t</sampler2D>\n");
			fprintf(fDAE, "\t\t\t\t</newparam>\n");
			} // if

		// light model
		fprintf(fDAE, "\t\t\t\t<technique sid=\"COMMON\">\n");
		if (mat->Shading == 1)		// flat
			{
			fprintf(fDAE, "\t\t\t\t\t<lambert>\n");
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
			if (useImage)
				{
				fprintf(fDAE, "%s<diffuse>\n", tab6);
				//FixTexName(StripExtension(textureName));
				//fprintf(fDAE, "%s<texture texture=\"%s-sampler\" texcoord=\"TEX0\"></texture>\n", tab7, textureName);
				fprintf(fDAE, "%s<texture texture=\"%s-sampler\" texcoord=\"TEX0\"></texture>\n", tab7, matName);
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
			if (useImage)
				{
				fprintf(fDAE, "%s<diffuse>\n", tab6);
				//FixTexName(StripExtension(textureName));
				//fprintf(fDAE, "%s<texture texture=\"%s-sampler\" texcoord=\"TEX0\"/>\n", tab7, textureName);
				fprintf(fDAE, "%s<texture texture=\"%s-sampler\" texcoord=\"TEX0\"/>\n", tab7, matName);
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

} // ExportFormatKML::ProcessMatTexture3

/*===========================================================================*/

void ExportFormatKML::ProcessPolys(FILE *fDAE, Object3DEffect *curObj)
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
		// second pass generates the triangle definitions
		if (mat = curObj->NameTable[matNum].Mat)
			{
			long hasUVW = curObj->VertexUVWAvailable;
			short doubleSided = curObj->NameTable[matNum].Mat->DoubleSided;

			if (doubleSided)
				numTris *= 2;

			FixMatName(mat->Name);
			fprintf(fDAE, "\t\t\t\t<triangles material=\"%s-material\" count=\"%d\">\n", matName, numTris);
			fprintf(fDAE, "\t\t\t\t\t<input semantic=\"VERTEX\" source=\"#%s-geometry-vertex\" offset=\"0\"/>\n", objName);
			fprintf(fDAE, "\t\t\t\t\t<input semantic=\"NORMAL\" source=\"#%s-geometry-normal\" offset=\"1\"/>\n", objName);
			if (mat->PixelTexturesExist && hasUVW)
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

} // ExportFormatKML::ProcessPolys

/*===========================================================================*/

int ExportFormatKML::SanityCheck(void)
{
#ifdef WCS_BUILD_VNS
PlanetOpt *defPlanetOpt;
int rVal = 0;
CoordSys csWGS84;

// Exporter CoordSys needs to be Geo WGS84
csWGS84.SetSystem("Geographic - WGS 84");

if (Master->Coords)
	{
	rVal = Master->Coords->Equals(&csWGS84);
	if (rVal == 0)
		UserMessageOK(msgGEE, "Please set the export CoordSys to Geographic WGS 84");
	} // if

defPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 0, NULL);
if (rVal && defPlanetOpt->Coords)
	{
	rVal = defPlanetOpt->Coords->Equals(&csWGS84);
	if (rVal == 0)
		UserMessageOK(msgGEE, "Please set the Planet Options CoordSys to Geographic WGS 84");
	} // if

#else
int rVal = 1;
#endif // WCS_BUILD_VNS

if (rVal && Master->ExportTerrain)
	{
	long polys = Master->OneDEMResX * Master->OneDEMResY;

	if (polys > 10922)	// width * height * 2 must be <= 21845 due to 16 bit limit of GE
		{
		sprintf(tempStr, "Too many polys in terrain model (%d) for Google Earth.  Tile width * height needs to be less than or equal to 10922.", polys);
		UserMessageOK(msgGEE, tempStr);
		rVal = 0;
		} // if
	} // if

return rVal;

} // ExportFormatKML::SanityCheck

/*===========================================================================*/

int ExportFormatKML::StylesAndMap(const char *baseName, const char *ext, float scale)
{

// highlight style
sprintf(tempStr, "%s<Style id=\"h_%s\">\n", indentStr, baseName);
fputs(tempStr, fKML);
IndentMore();
sprintf(tempStr, "%s<IconStyle>\n", indentStr);
fputs(tempStr, fKML);
IndentMore();
sprintf(tempStr, "%s<Border id=\"khBorderDefault\">\n%s</Border>\n", indentStr, indentStr);
fputs(tempStr, fKML);
sprintf(tempStr, "%s<Icon>\n", indentStr);
fputs(tempStr, fKML);
IndentMore();
sprintf(tempStr, "%s<href>images/%s%s</href>\n", indentStr, baseName, ext);
fputs(tempStr, fKML);
IndentLess();
sprintf(tempStr, "%s</Icon>\n", indentStr);
fputs(tempStr, fKML);
sprintf(tempStr, "%s<scale>%f</scale>\n", indentStr, scale);
fputs(tempStr, fKML);
IndentLess();
sprintf(tempStr, "%s</IconStyle>\n", indentStr);
fputs(tempStr, fKML);
IndentLess();
sprintf(tempStr, "%s</Style>\n", indentStr);
fputs(tempStr, fKML);

// normal style
sprintf(tempStr, "%s<Style id=\"n_%s\">\n", indentStr, baseName);
fputs(tempStr, fKML);
IndentMore();
sprintf(tempStr, "%s<IconStyle>\n", indentStr);
fputs(tempStr, fKML);
IndentMore();
sprintf(tempStr, "%s<Border id=\"khBorderDefault\">\n%s</Border>\n", indentStr, indentStr);
fputs(tempStr, fKML);
sprintf(tempStr, "%s<Icon>\n", indentStr);
fputs(tempStr, fKML);
IndentMore();
sprintf(tempStr, "%s<href>images/%s%s</href>\n", indentStr, baseName, ext);
fputs(tempStr, fKML);
IndentLess();
sprintf(tempStr, "%s</Icon>\n", indentStr);
fputs(tempStr, fKML);
sprintf(tempStr, "%s<scale>%f</scale>\n", indentStr, scale);
fputs(tempStr, fKML);
IndentLess();
sprintf(tempStr, "%s</IconStyle>\n", indentStr);
fputs(tempStr, fKML);
IndentLess();
sprintf(tempStr, "%s</Style>\n", indentStr);
fputs(tempStr, fKML);

// style map
sprintf(tempStr, "%s<StyleMap id=\"sm_%s\">\n", indentStr, baseName);
fputs(tempStr, fKML);
IndentMore();

sprintf(tempStr, "%s<Pair>\n%s\t<key>normal</key>\n", indentStr, indentStr);
fputs(tempStr, fKML);
sprintf(tempStr, "%s\t<styleUrl>#n_%s</styleUrl>\n%s</Pair>\n", indentStr, baseName, indentStr);
fputs(tempStr, fKML);

sprintf(tempStr, "%s<Pair>\n%s\t<key>highlight</key>\n", indentStr, indentStr);
fputs(tempStr, fKML);
sprintf(tempStr, "%s\t<styleUrl>#h_%s</styleUrl>\n%s</Pair>\n", indentStr, baseName, indentStr);
fputs(tempStr, fKML);

IndentLess();
sprintf(tempStr, "%s</StyleMap>\n", indentStr);
fputs(tempStr, fKML);

return 1;

} // ExportFormatKML::StylesAndMap
