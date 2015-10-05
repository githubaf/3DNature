// Realtime.h
// Header file for Realtime Data classes
// Created 12/17/01 by Gary R. Huber
// Copyright 2001 3D Nature LLC. All rights reserved.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class EffectsLib;
class ImageLib;
class FoliagePreviewData;
class GeneralEffect;
class Joe;

#ifndef WCS_REALTIME_H
#define WCS_REALTIME_H

#include "GraphData.h"
#include "Ecotype.h"
#include "FeatureConfig.h"

#ifdef WCS_BUILD_SX2
#define WCS_FOLIAGELIST_FILE_VERSION	2
#else // WCS_BUILD_SX2
#define WCS_FOLIAGELIST_FILE_VERSION	1
#endif // WCS_BUILD_SX2
#define WCS_FOLIAGELIST_BASENAME_LEN WCS_EFFECT_MAXNAMELENGTH

class RealtimeFoliageIndex;
class RealtimeFoliageData;
class RealtimeFoliageCellData;

enum
	{
	WCS_REALTIME_CONFIG_MINHEIGHT,	// no nodes
	WCS_REALTIME_CONFIG_MAXHEIGHT,	// no nodes
	WCS_REALTIME_CONFIG_NEARDIST,	// no nodes
	WCS_REALTIME_CONFIG_FARDIST,	// no nodes
	WCS_REALTIME_CONFIG_MAX	// always last
	}; // common non-animated config params

#define WCS_REALTIME_FOLDAT_BITINFO_FLIPX		(1 << 0)
#define WCS_REALTIME_FOLDAT_BITINFO_SHADE3D		(1 << 1)
#define WCS_REALTIME_FOLDAT_BITINFO_LABEL		(1 << 2)
#define WCS_REALTIME_FOLDAT_BITINFO_LEFTPOLE	(1 << 3)
#define WCS_REALTIME_FOLDAT_BITINFO_RIGHTPOLE	(1 << 4)
#define WCS_REALTIME_FOLDAT_BITINFO_CLICKQUERY	(1 << 5)
#define WCS_REALTIME_FOLDAT_BITINFO_VECTORPRESENT	(1 << 6)

class RealTimeFoliageWriteConfig
	{
	public:
		AnimDoubleTime ConfigParams[WCS_REALTIME_CONFIG_MAX];
		long NumFiles, StemsPerCell;
		char Include3DO, IncludeImage, IncludeLabels;
		char BaseName[WCS_FOLIAGELIST_BASENAME_LEN], DirName[512];

		RealTimeFoliageWriteConfig();
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);
	}; // RealTimeFoliageWriteConfig

class RealTimeFoliageDisplayConfig
	{
	public:
		AnimDoubleTime ConfigParams[WCS_REALTIME_CONFIG_MAX];
		char Display3DO, DisplayImage;

		char BaseName[WCS_FOLIAGELIST_BASENAME_LEN];
		RealTimeFoliageDisplayConfig();
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);
	}; // RealTimeFoliageDisplayConfig

class RealtimeFoliageData
	{
	public:
		float XYZ[3];
		short ElementID;
		unsigned short Height;
		unsigned char BitInfo; // New BitInfo flags for labels and which side the pole is on. 0x04=label, 0x08=pole on left, 0x10=pole on right, otherwise it is centered.
		unsigned char ImageColorOpacity, TripleValue[3];
		RealtimeFoliageData *Next;
		#ifdef WCS_BUILD_SX2
		GeneralEffect *MyEffect;
		Joe *MyVec;
		#endif // WCS_BUILD_SX2

		RealtimeFoliageData()	{Next = NULL; ElementID = 0; Height = 0; 
			BitInfo = ImageColorOpacity = TripleValue[0] = TripleValue[1] = TripleValue[2] = 0;
			XYZ[0] = XYZ[0] = XYZ[0] = 0.0f;
			#ifdef WCS_BUILD_SX2
			MyEffect = NULL;
			MyVec = NULL;
			#endif // WCS_BUILD_SX2
			};
		int WriteFoliageRecord(FILE *ffile);
		int WriteFoliageRecordVF(FILE *ffile, RealtimeFoliageIndex *RFI);
		int ReadFoliageRecord(FILE *ffile, char FileVersion);
		int InterpretFoliageRecord(EffectsLib *Effects, ImageLib *Images, FoliagePreviewData *PointData);

	}; // class RealtimeFoliageData



class RealtimeFoliageCellData
	{
	public:
		char FileName[64];
		long DatCt, NumDatLoaded;
		double CellXYZ[3], CellRad;
		RealtimeFoliageData *FolData;

		RealtimeFoliageCellData()	{FileName[0] = 0; FolData = NULL; NumDatLoaded = DatCt = 0; CellXYZ[0] = CellXYZ[1] = CellXYZ[2] = 0.0;};
		~RealtimeFoliageCellData();

		RealtimeFoliageData *AllocFolData(int NumFolData);
		void FreeFolData(void);

		int LoadFolData(RealtimeFoliageIndex *Index, long FileCt);

	}; // class RealtimeFoliageCellData


class RealtimeFoliageIndex
	{
	public:
		char FileVersion;
		long NumCells;
		double RefXYZ[3];
		RealtimeFoliageCellData *CellDat;
		long *RasterInterp;
		long WalkCellNum, WalkDatNum;

		RealtimeFoliageIndex()	{FileVersion = 0; NumCells = 0; RefXYZ[0] = RefXYZ[1] = RefXYZ[2] = 0.0; CellDat = NULL; RasterInterp = NULL;};
		~RealtimeFoliageIndex();

		int LoadFoliageCellData(long FileCt);
		int LoadFoliageIndex(char *Filename, char *Pathname);
		int LoadAllFoliage(void);
		void FreeAllFoliage(void);

		RealtimeFoliageData *FirstWalk(void);
		RealtimeFoliageData *NextWalk(void);

	}; // class RealtimeFoliageIndex

// file IO defines
#define WCS_VIEWINIT_FOLWRITE_NUMFILES			0x00010000
#define WCS_VIEWINIT_FOLWRITE_STEMSPERCELL		0x00020000
#define WCS_VIEWINIT_FOLWRITE_INCLUDE3DO		0x00030000
#define WCS_VIEWINIT_FOLWRITE_INCLUDEIMAGE		0x00040000
#define WCS_VIEWINIT_FOLWRITE_BASENAME			0x00050000
#define WCS_VIEWINIT_FOLWRITE_MINHEIGHT			0x00060000
#define WCS_VIEWINIT_FOLWRITE_MAXHEIGHT			0x00070000
#define WCS_VIEWINIT_FOLWRITE_NEARDIST			0x00080000
#define WCS_VIEWINIT_FOLWRITE_FARDIST			0x00090000

#define WCS_VIEWINIT_FOLDISPLAY_DISPLAY3DO		0x00010000
#define WCS_VIEWINIT_FOLDISPLAY_DISPLAYIMAGE	0x00020000
#define WCS_VIEWINIT_FOLDISPLAY_BASENAME		0x00030000
#define WCS_VIEWINIT_FOLDISPLAY_MINHEIGHT		0x00040000
#define WCS_VIEWINIT_FOLDISPLAY_MAXHEIGHT		0x00050000
#define WCS_VIEWINIT_FOLDISPLAY_NEARDIST		0x00060000
#define WCS_VIEWINIT_FOLDISPLAY_FARDIST			0x00070000

#endif // WCS_REALTIME_H
