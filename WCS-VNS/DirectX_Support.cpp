// DirectX_Support.cpp
// Code for DirectX object creation
// Created from scratch 08/28/06 by FPW2
// Copyright 2006 3D Nature, LLC

// Objects are created in left-handed system, with Y-up
//   Y Z
//   |/
//   *- X

// Code not heavily commented, since you're going to have to understand the format to figure anything out anyways :)

#include "stdafx.h"
#include "DirectX_Support.h"

extern WCSApp *GlobalApp;

// F2_NOTE: Need to take care of double-sided flag
// F2_NOTE: ToDo - normals

/*===========================================================================*/

DX_model::DX_model(SceneExporter *MasterSource)
{

DX_file = NULL;
Master = MasterSource;
status = DX_INIT_FAILED;
rval = 0;

} // DX_model::DX_model

/*===========================================================================*/

DX_model::~DX_model()
{

if (DX_file)
	{
	fclose(DX_file);
	DX_file = NULL;
	} // if

} // DX_model::~DX_model

/*===========================================================================*/

int DX_model::CreateHeaders()
{

// DirectX header
rval = fputs("xof 0302txt 0032\n\n", DX_file);

/*** SDK indicates we should be able to define our own header.  Other apps don't seem to like this.  Grrr...
// Our own header as we define it
fputs("template Header {\n <3D82AB43-62DA-11cf-AB39-0020AF71E433>\n", DX_file);
fputs(" DWORD flags;\n STRING appName;\n WORD major;\n STRING appExtra;\n}\n\n", DX_file);

// Our header data
sprintf(tempStr, "Header {\n\t1;\n\t\"%s\";\n\t%s;\n\t\"%s\";\n}\n\n", APP_TITLE, APP_VERS, "with Scene Express");
rval = fputs(tempStr, DX_file);
***/

fputs("template Header {\n <3D82AB43-62DA-11cf-AB39-0020AF71E433>\n", DX_file);
fputs(" WORD major;\n WORD minor;\n DWORD flags;\n}\n\n", DX_file);

sprintf(tempStr, "Header {\n	1; // Major version\n	0; // Minor version\n	1; // Flags\n}\n\n");
rval = fputs(tempStr, DX_file);

return rval;

} // DX_model::CreateHeader

/*===========================================================================*/

int DX_model::CreateIni(Object3DInstance *curInstance)
{
FILE *ini;
rval = 0;

if (curInstance->MyObj && curInstance->MyObj->Name)
	{
	strcpy(tempFullPath, (char *)Master->OutPath.GetPath());
	strcat(tempFullPath, "\\");
	strcat(tempFullPath, FixName(curInstance->MyObj->Name));
	strcat(tempFullPath, ".ini");
	if (ini = PROJ_fopen(tempFullPath, "w"))
		{
//Lat(deg)	Lon(deg)	Scale (x)	Orientation(deg) Altitude(m)	File name	vertExaggerable (ie altitude is bound to vert. exaggeration param.)
		sprintf(tempStr, "%f;%f;1;%f;%f;%s.x;1\n", (float)curInstance->WCSGeographic[1], (float)curInstance->WCSGeographic[0],
			90.0f + (float)curInstance->Euler[1], (float)curInstance->WCSGeographic[2], nameStr);	// corrected name is still in buffer
		rval = fputs(tempStr, ini);
		fclose(ini);
		} // if
	} // if

return rval;

} // DX_model::CreateIni

/*===========================================================================*/

int DX_model::CreateMaterials(Object3DInstance *curInstance)
{
MaterialEffect *mat;
Object3DEffect *curObj;
long i;

rval = DX_MISSING_OBJ;

if (curInstance->MyObj)
	{
	curObj = curInstance->MyObj;

	for (i = 0; i < curObj->NumMaterials; i++)
		{
		if (! curObj->NameTable[i].Mat)
			curObj->NameTable[i].Mat = (MaterialEffect *)GlobalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, curObj->NameTable[i].Name);

		if (mat = curObj->NameTable[i].Mat)
			{

			sprintf(tempStr, "Material %s {\n", FixName(mat->Name));
			fputs(tempStr, DX_file);
			sprintf(tempStr, "\t%f;%f;%f;%f;;\n",
				(float)mat->DiffuseColor.CurValue[0], (float)mat->DiffuseColor.CurValue[1], (float)mat->DiffuseColor.CurValue[2], 1.0f);
			fputs(tempStr, DX_file);
			sprintf(tempStr, "\t%f;\n", (float)mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].CurValue);
			fputs(tempStr, DX_file);
			sprintf(tempStr, "\t%f;%f;%f;;\n",
				(float)mat->SpecularColor.CurValue[0], (float)mat->SpecularColor.CurValue[1], (float)mat->SpecularColor.CurValue[2]);
			fputs(tempStr, DX_file);
			sprintf(tempStr, "\t0.0;0.0;0.0;;\n");
			fputs(tempStr, DX_file);

			if ((i == 0) && curObj->VertexUVWAvailable)
				{
				sprintf(tempStr, "\tTextureFilename {\n\t\t\"%s%s\";\n\t}\n",
					nameStr, ImageSaverLibrary::GetDefaultExtension(Master->TextureImageFormat));
				fputs(tempStr, DX_file);
				} // if

			sprintf(tempStr, "}\n\n");
			rval = fputs(tempStr, DX_file);
			} // if mat

		} // for i

	} // if MyObj

return rval;

} // DX_model::CreateMaterials

/*===========================================================================*/

int DX_model::CreateMesh(Object3DInstance *curInstance)
{
Object3DEffect *curObj;
long entered, i, j, k;

rval = DX_MISSING_OBJ;

if (curInstance->MyObj)
	{
	curObj = curInstance->MyObj;

	sprintf(tempStr, "Mesh %s {\n", FixName(curObj->Name));
	fputs(tempStr, DX_file);

	// write vertices data
	sprintf(tempStr, "\t%d;\n", curObj->NumVertices);
	fputs(tempStr, DX_file);
	j = curObj->NumVertices - 1;
	for (i = 0; i < j; i++)
		{
		sprintf(tempStr, "\t%f;%f;%f;,\n",
			(float)-curObj->Vertices[i].xyz[0], (float)curObj->Vertices[i].xyz[1], (float)curObj->Vertices[i].xyz[2]);
		fputs(tempStr, DX_file);
		} // for
	sprintf(tempStr, "\t%f;%f;%f;;\n\n",
		(float)-curObj->Vertices[j].xyz[0], (float)curObj->Vertices[j].xyz[1], (float)curObj->Vertices[j].xyz[2]);
	fputs(tempStr, DX_file);

	// write faces data
	sprintf(tempStr, "\t%d;\n", curObj->NumPolys);
	fputs(tempStr, DX_file);
	entered = 0;
	for (i = 0; i < curObj->NumPolys; i++)
		{
		if (entered)
			{
			sprintf(tempStr, ",\n");
			fputs(tempStr, DX_file);
			}
		sprintf(tempStr, "\t%d;", curObj->Polygons[i].NumVerts);
		fputs(tempStr, DX_file);
		j = curObj->Polygons[i].NumVerts - 1;
		for (k = 0; k < j; k++)
			{
			sprintf(tempStr, "%d,", curObj->Polygons[i].VertRef[k]);
			fputs(tempStr, DX_file);
			} // for k
		sprintf(tempStr, "%d;", curObj->Polygons[i].VertRef[j]);
		fputs(tempStr, DX_file);
		entered = 1;
		} // for i
	sprintf(tempStr, ";\n\n");
	fputs(tempStr, DX_file);

	CreateMeshMaterialList(curInstance);

	if (curObj->VertexUVWAvailable)
		CreateMeshTextureCoords(curObj);

	sprintf(tempStr, "}\n");
	fputs(tempStr, DX_file);
	} // if MyObj

return rval;

} // DX_model::CreateMesh

/*===========================================================================*/

int DX_model::CreateMeshMaterialList(Object3DInstance *curInstance)
{
MaterialEffect *mat;
Object3DEffect *curObj;
long entered, i, indent;

rval = DX_MISSING_OBJ;

if (curInstance->MyObj)
	{
	curObj = curInstance->MyObj;

	sprintf(tempStr, "\tMeshMaterialList {\n");
	fputs(tempStr, DX_file);
	sprintf(tempStr, "\t\t%d;\n", curObj->NumMaterials);
	fputs(tempStr, DX_file);
	sprintf(tempStr, "\t\t%d;\n", curObj->NumPolys);
	fputs(tempStr, DX_file);

	entered = 0;
	indent = 1;
	for (i = 0; i < curObj->NumPolys; i++)
		{

		if (entered)
			{
			// we'll only write 32 values per line
			if (i % 32)
				sprintf(tempStr, ",");
			else
				{
				sprintf(tempStr, ",\n");
				indent = 1;
				} // if
			fputs(tempStr, DX_file);
			} // if

		if (indent)
			{
			sprintf(tempStr, "\t\t%d", curObj->Polygons[i].Material);
			indent = 0;
			} // if
		else
			{
			sprintf(tempStr, "%d", curObj->Polygons[i].Material);
			} // else
		fputs(tempStr, DX_file);

		entered = 1;
		} // for i
	sprintf(tempStr, ";;\n");
	fputs(tempStr, DX_file);

	for (i = 0; i < curObj->NumMaterials; i++)
		{
		if (! curObj->NameTable[i].Mat)
			curObj->NameTable[i].Mat = (MaterialEffect *)GlobalApp->AppEffects->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, curObj->NameTable[i].Name);

		if (mat = curObj->NameTable[i].Mat)
			{
			sprintf(tempStr, "\t\t{%s}\n", FixName(mat->Name));
			rval = fputs(tempStr, DX_file);
			} // if
		} // for

	sprintf(tempStr, "\t}\n\n");
	fputs(tempStr, DX_file);
	} // if MyObj

return rval;

} // DX_model::CreateMeshMaterialList

/*===========================================================================*/

int DX_model::CreateMeshTextureCoords(Object3DEffect *curObj)
{
ObjectPerVertexMap *uvMap;
long entered, i, j;

uvMap = &curObj->UVWTable[0];

sprintf(tempStr, "\tMeshTextureCoords {\n");
fputs(tempStr, DX_file);
sprintf(tempStr, "\t\t%d;\n", uvMap->NumNodes);
fputs(tempStr, DX_file);

j = uvMap->NumNodes - 1;
entered = 0;
for (i = 0; i < j; i++)
	{
	if (entered)
		{
		sprintf(tempStr, ",\n");
		fputs(tempStr, DX_file);
		} // if

	sprintf(tempStr, "\t\t%f;%f;", (float)uvMap->CoordsArray[0][i], 1.0f - (float)uvMap->CoordsArray[1][i]);
	fputs(tempStr, DX_file);

	entered = 1;
	} // for
sprintf(tempStr, ",\n\t\t%f;%f;;\n\t}\n\n", (float)uvMap->CoordsArray[0][j], 1.0f - (float)uvMap->CoordsArray[1][j]);
rval = fputs(tempStr, DX_file);

return rval;

} // DX_model::CreateMeshTextureCoords

/*===========================================================================*/

int DX_model::CreateTemplates()
{

rval = CreateTemplateColorRGB();

rval = CreateTemplateColorRGBA();

rval = CreateTemplateTextureFilename();

rval = CreateTemplateMaterial();

rval = CreateTemplateMeshMaterialList();

rval = CreateTemplateVector();

rval = CreateTemplateMeshFace();

rval = CreateTemplateMesh();

return rval;

} // DX_model::CreateTemplates()

/*===========================================================================*/

int DX_model::CreateTemplateColorRGB()
{

sprintf(tempStr, "template ColorRGB {\n <D3E16E81-7835-11cf-8F52-0040333594A3>\n");
fputs(tempStr, DX_file);
sprintf(tempStr, " FLOAT red;\n FLOAT green;\n FLOAT blue;\n}\n\n");
rval = fputs(tempStr, DX_file);

return rval;

} // DX_model::CreateTemplateColorRGB()

/*===========================================================================*/

int DX_model::CreateTemplateColorRGBA()
{

sprintf(tempStr, "template ColorRGBA {\n  <35FF44E0-6C7C-11cf-8F52-0040333594A3>\n");
fputs(tempStr, DX_file);
sprintf(tempStr, " FLOAT red;\n FLOAT green;\n FLOAT blue;\n FLOAT alpha;\n}\n\n");
rval = fputs(tempStr, DX_file);

return rval;

} // DX_model::CreateTemplateColorRGBA

/*===========================================================================*/

int DX_model::CreateTemplateMaterial()
{

sprintf(tempStr, "template Material {\n <3D82AB4D-62DA-11cf-AB39-0020AF71E433>\n");
fputs(tempStr, DX_file);
sprintf(tempStr, " ColorRGBA faceColor;\n FLOAT power;\n ColorRGB specularColor;\n ColorRGB emissiveColor;\n [...]\n}\n\n");
rval = fputs(tempStr, DX_file);

return rval;

} // DX_model::CreateTemplateMaterial

/*===========================================================================*/

int DX_model::CreateTemplateMeshFace()
{

sprintf(tempStr, "template MeshFace {\n <3D82AB5F-62DA-11cf-AB39-0020AF71E433>\n");
fputs(tempStr, DX_file);
sprintf(tempStr, " DWORD nFaceVertexIndices;\n array DWORD faceVertexIndices[nFaceVertexIndices];\n}\n\n");
rval = fputs(tempStr, DX_file);

return rval;

} // DX_model::CreateTemplateMeshFace

/*===========================================================================*/

int DX_model::CreateTemplateMesh()
{

sprintf(tempStr, "template Mesh {\n <3D82AB44-62DA-11cf-AB39-0020AF71E433>\n DWORD nVertices;\n");
fputs(tempStr, DX_file);
sprintf(tempStr, " array Vector vertices[nVertices];\n DWORD nFaces;\n array MeshFace faces[nFaces];\n [...]\n}\n\n");
rval = fputs(tempStr, DX_file);

return rval;

} // DX_model::CreateTemplateMesh

/*===========================================================================*/

int DX_model::CreateTemplateMeshMaterialList()
{

sprintf(tempStr, "template MeshMaterialList {\n <F6F23F42-7686-11cf-8F52-0040333594A3>\n");
fputs(tempStr, DX_file);
sprintf(tempStr, " DWORD nMaterials;\n DWORD nFaceIndexes;\n array DWORD faceIndexes[nFaceIndexes];\n [Material]\n}\n\n");
rval = fputs(tempStr, DX_file);

return rval;

} // DX_model::CreateTemplateMeshMaterialList

/*===========================================================================*/

int DX_model::CreateTemplateTextureFilename()
{

sprintf(tempStr, "template TextureFilename {\n <A42790E1-7810-11cf-8F52-0040333594A3>\n");
fputs(tempStr, DX_file);
sprintf(tempStr, " STRING filename;\n}\n\n");
rval = fputs(tempStr, DX_file);

return rval;

} // DX_model::CreateTemplateTextureFilename

/*===========================================================================*/

int DX_model::CreateTemplateVector()
{

sprintf(tempStr, "template Vector {\n  <3D82AB5E-62DA-11cf-AB39-0020AF71E433>\n");
fputs(tempStr, DX_file);
sprintf(tempStr, " FLOAT x;\n FLOAT y;\n FLOAT z;\n}\n\n");
rval = fputs(tempStr, DX_file);

return rval;

} // DX_model::CreateTemplateVector

/*===========================================================================*/

int DX_model::ExportModel(Object3DInstance *curInstance)
{
Object3DEffect *curObj;

if (curInstance->MyObj)
	{
	curObj = curInstance->MyObj;
	status = DX_SUCCESS;
	} // if

if ((status > 0) && curObj->Name)
	{
	strcpy(tempFullPath, (char *)Master->OutPath.GetPath());
	strcat(tempFullPath, "\\");
	strcat(tempFullPath, FixName(curInstance->MyObj->Name));
	strcat(tempFullPath, ".x");
	DX_file = PROJ_fopen(tempFullPath, "w");
	if (! DX_file)
		status = DX_CREATE_FAIL;
	} // if

if (status >= 0)
	status = CreateIni(curInstance);

if (status >= 0)
	status = CreateHeaders();

if (status >= 0)
	status = CreateTemplates();

if (status >= 0)
	status = CreateMaterials(curInstance);

if (status >= 0)
	status = CreateMesh(curInstance);

if (DX_file)
	{
	fclose(DX_file);
	DX_file = NULL;
	} // if

return status;

} // DX_model::ExportModel

/*===========================================================================*/

char* DX_model::FixName(const char *name)
{
char *fixed = NULL;
size_t i;

if (name)
	{
	memset(nameStr, 0, sizeof(nameStr));
	strcpy(nameStr, name);
	for (i = 0; i < strlen(nameStr); i++)
		{
		if (nameStr[i] == ' ')
			nameStr[i] = '_';
		} // for
	fixed = nameStr;
	} // if

return nameStr;

} // DX_model::FixName

/*===========================================================================*/
