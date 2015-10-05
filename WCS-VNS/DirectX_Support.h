// DirectX_Support.h
// Header for DirectX Support
// Created from scratch 08/28/06 by FPW2
// Copyright 2006 3D Nature, LLC

#include "stdafx.h"
#include "Application.h"
#include "EffectsLib.h"
#include "ImageOutputEvent.h"
#include "Project.h"
#include "Render.h"
#include "WCSVersion.h"

#define DX_INIT_FAILED	0
#define DX_SUCCESS		1
#define DX_CREATE_FAIL	-1
#define DX_MISSING_OBJ	-2

class DX_model
	{
	private:
		FILE *DX_file;
		SceneExporter *Master;
		int rval, status;
		char nameStr[64], tempStr[256];
		char tempFullPath[WCS_PATHANDFILE_PATH_PLUS_NAME_LEN];

		int CreateHeaders(void);
		int CreateIni(Object3DInstance *curInstance);
		int CreateMaterials(Object3DInstance *curInstance);
		int CreateMesh(Object3DInstance *curInstance);
		int CreateMeshMaterialList(Object3DInstance *curInstance);
		int CreateMeshTextureCoords(Object3DEffect *curObj);
		int CreateTemplates(void);
		int CreateTemplateColorRGB(void);
		int CreateTemplateColorRGBA(void);
		int CreateTemplateMaterial(void);
		int CreateTemplateMesh(void);
		int CreateTemplateMeshFace(void);
		int CreateTemplateMeshMaterialList(void);
		int CreateTemplateTextureFilename(void);
		int CreateTemplateVector(void);
		char *FixName(const char *name);

	public:
		DX_model(SceneExporter *MasterSource);
		~DX_model();
		int ExportModel(Object3DInstance *curInstance);

	}; // class DX_model
