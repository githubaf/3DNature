// DataOpsUseful.h
// Common code used during reading and writing of data
// Written by Frank Weed II on 9/24/99
// Copyright 1999 3D Nature

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_DATAOPSUSEFUL_H
#define WCS_DATAOPSUSEFUL_H

#include "requester.h"
#include "ImportThing.h"
#include "DataOpsDefs.h"

#define IMWIZ_IN_DIR		Importing->InDir
#define IMWIZ_IN_FILE		Importing->InFile
#define IMWIZ_LOADNAME		Importing->LoadName
#define INPUT_FORMAT		Importing->InFormat
#define OUTPUT_FORMAT		Importing->OutFormat
#define INVALUE_FORMAT		Importing->ValueFmt
#define INVALUE_SIZE		Importing->ValueBytes
#define INBYTE_ORDER		Importing->ByteOrder
#define READ_ORDER			Importing->SwapRC
#define ROWS_EQUAL			Importing->RowsEqual
#define GRID_UNITS			Importing->GridUnits
#define ELEV_UNITS			Importing->ElevUnits
#define INPUT_HEADER		Importing->HdrSize
#define INPUT_FILESIZE		Importing->InFileSize
#define INPUT_ROWS			Importing->InRows
#define INPUT_COLS			Importing->InCols
#define INPUT_FLOOR			Importing->Floor
#define INPUT_CEILING		Importing->Ceiling
#define ACTIVE_FLOOR		Importing->UseFloor
#define ACTIVE_CEILING		Importing->UseCeiling
#define CROP_LEFT			Importing->CropLeft
#define CROP_RIGHT			Importing->CropRight
#define CROP_TOP			Importing->CropTop
#define CROP_BOTTOM			Importing->CropBottom
#define CROP_ROWS			((INPUT_FORMAT == DEM_DATA2_INPUT_WCSDEM || INPUT_FORMAT == DEM_DATA2_INPUT_DTED || INPUT_FORMAT == DEM_DATA2_INPUT_MDEM) ? INPUT_ROWS - CROP_LEFT - CROP_RIGHT: INPUT_ROWS - CROP_TOP - CROP_BOTTOM)		
#define CROP_COLS			((INPUT_FORMAT == DEM_DATA2_INPUT_WCSDEM || INPUT_FORMAT == DEM_DATA2_INPUT_DTED || INPUT_FORMAT == DEM_DATA2_INPUT_MDEM) ? INPUT_COLS - CROP_TOP - CROP_BOTTOM: INPUT_COLS - CROP_LEFT - CROP_RIGHT)		
#define INPUT_FILENAME		Importing->InFile
#define INPUT_DIRECTORY		Importing->InDir
#define OUTPUT_ROWMAPS		Importing->OutDEMWE
#define OUTPUT_COLMAPS		Importing->OutDEMNS
#define OUTPUT_HILAT		Importing->NBound
#define OUTPUT_LOLAT		Importing->SBound
#define OUTPUT_HILON		Importing->WBound
#define OUTPUT_LOLON		Importing->EBound
#define OUTPUT_NAMEBASE		Importing->NameBase
#define OUTPUT_DIRECTORY	Importing->OutDir
#define OUTVALUE_FORMAT		Importing->OutValFmt
#define OUTVALUE_SIZE		Importing->OutValBytes
#define OUTPUT_ROWS			Importing->OutRows
#define OUTPUT_COLS			Importing->OutCols
#define SPLINE_CONSTRAIN	Importing->SplineConstrain
#define SCALEOP				Importing->ScaleOp
#define SCALETYPE			Importing->ScaleType
#define SCALE_MAXEL			Importing->ScaleMMMax
#define SCALE_MINEL			Importing->ScaleMMMin
#define SCALE_VALU1			Importing->ScaleOVIn
#define SCALE_ELEV1			Importing->ScaleOVOut
#define SCALE_SCALE			Importing->ScaleOVSet
#define SCALE_VALU2			Importing->ScaleTVV1In
#define SCALE_ELEV2			Importing->ScaleTVV1Out
#define SCALE_VALU3			Importing->ScaleTVV2In
#define SCALE_ELEV3			Importing->ScaleTVV2Out
#define SCALE_TESTMIN		Importing->TestMin
#define SCALE_TESTMAX		Importing->TestMax

#define DUPROW				1

#endif // WCS_DATAOPSUSEFUL_H
