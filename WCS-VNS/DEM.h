// DEM.h
// DEM object, part of the database
// Created from scratch on 8/23/95 by Chris "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_DEM_H
#define WCS_DEM_H

class MessageLog;
class Project;
class Joe;
class CoordSys;
struct elmapheaderV101;
struct faces;
class VertexDEM;
class DEFG;
class Database;
class Renderer;
class ViewGUI;
class InterpDEMGUI;
class TerraGridder;
class TerraGenerator;
class NNGrid;
class DEMMerger;
class DEMMergeGUI;
class DEMPaintGUI;
class ImportWizGUI;
class ImageFormatWCSELEV;
class InterCommon;
class SceneExportGUI;
class DEMEval;
class VectorPolygonListDouble;

#include "Types.h"
#include "FeatureConfig.h"

FILE *AttemptOpenObjFile(char *ObjName, Project *OpenFrom);
FILE *AttemptOpenRelElFile(char *ObjName, Project *OpenFrom);
void AttemptDeleteDEMFiles(char *ObjName, Project *OpenFrom);
void AttemptRenameDEMFiles(Joe *OldGuy, char *NewName, Project *From);

// Pick up remainder of core DEM object definition from DEMCore.h with all
// bells & whistles enabled due to WCS_DEM_H being defined
#include "DEMCore.h"

#endif // WCS_DEM_H
