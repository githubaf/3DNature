// Database.h
// Header file for Database.cpp
// Built from scratch on 03/20/95 by Chris "Xenon" Hanson
// Copyright 1995

class Database;
//class Joe;
class LoadStack;

#ifndef WCS_DATABASE_H
#define WCS_DATABASE_H

#include "Joe.h"
#include "Points.h"
#include "Layers.h"
#include "DEM.h"

class Database
	{
//	private:
	public:
		Joe *StaticRoot, *DynamicRoot, *LogRoot;
		double GeoClipNorth, GeoClipSouth, GeoClipEast, GeoClipWest;
		Joe *GenericPrev;
		unsigned long int NextFlags;

		Joe *GimmeNext(Joe *Previous);
		Joe *GimmeNextGroup(Joe *Previous);
		Joe *GimmeNextSameLevel(Joe *Previous);
		unsigned long int WriteTwig(FILE *SaveFile, Joe *SaveRoot);

		unsigned long int LoadAttrib(FILE *LoadFile, Joe *LoadRoot,
		 unsigned long int Number, unsigned long int LIdx,
		 LayerEntry **LayerIndex, char Flop);

		unsigned long int LoadGroup(FILE *LoadFile,
		 Joe *LoadRoot, unsigned long int Depth, char Flop);

		Joe *ParseJoe(char *JoeBuf, unsigned long int JBSize,
		 char *StringBuf, char Flop, LoadStack *Tree,
		 unsigned long int NLayers, LayerEntry **LayerIndex,
		 Joe *LoadRoot, Joe *LoadTarget, Joe *PrevJoe);

		void ScanKill(Joe *Origin);
		void BoundUpTree(Joe *Origin);

//	public:
		PointAllocator MasterPoint;
		LayerTable DBLayers;

		Database();
		~Database();
		int InitValid(void);
		void DestroyAll(void);
		unsigned long int HowManyObjs(void);
		void GetBounds(double &North, double &South, double &East, double &West);
		void SetGeoClip(double North, double South, double East, double West);
		void ResetGeoClip(void);
		Joe *GeoClipMatch(Joe *Me);
		Joe *GetFirst(unsigned long int Flags = NULL);
		Joe *GetNext(Joe *Previous = NULL);
		Joe *GetNextSameLevel(Joe *Previous = NULL);
		Joe *AddJoe(Joe *Me, unsigned long int Flags);
		Joe *AddJoe(Joe *Me, Joe *MyParent);
		unsigned long int SaveV2(const char *SaveName, unsigned long int SaveFlags);
		unsigned long int SaveV2(const char *SaveName, Joe *SaveRoot);
		unsigned long int SaveV2(FILE *SaveFile, Joe *SaveRoot);
		unsigned long int LoadV2(const char *LoadName, unsigned long int LoadFlags = NULL);
		unsigned long int LoadV2(const char *LoadName, Joe *LoadRoot);
		unsigned long int LoadV2(FILE *LoadFile, Joe *LoadRoot);
		unsigned long int Import(char *FileName, Joe *Level, unsigned char Type);
		unsigned long int ImportDLG(FILE *, Joe *);
		unsigned long int ImportPPL(FILE *, Joe *);
		unsigned long int ImportGNIS(FILE *, Joe *);
		unsigned long int ImportCOUNTY(FILE *, Joe *);
		DEM *ObtainDEM(Joe *, unsigned int DownSample);
		void ReleaseDEM(DEM *);




	}; // Database

// Flag bits for the different Database trees
#define WCS_DATABASE_STATIC		(1 << 0)
#define WCS_DATABASE_DYNAMIC	(1 << 1)
#define WCS_DATABASE_LOG		(1 << 2)

#define WCS_DATABASE_ALL		(WCS_DATABASE_STATIC | WCS_DATABASE_DYNAMIC | WCS_DATABASE_LOG)

// Types for Import
#define WCS_DATABASE_IMPORT_DLG		1
#define WCS_DATABASE_IMPORT_PPL		2
#define WCS_DATABASE_IMPORT_GNIS		3
#define WCS_DATABASE_IMPORT_COUNTY	4

// Used to handle recursion non-recursively in loading
class LoadStack
	{
	public:
		Joe *LevelParent;
		unsigned long int AttribKids, GroupKids;
		inline LoadStack() {LevelParent = NULL; AttribKids = GroupKids = 0;};
	}; // LoadStack

#endif // WCS_DATABASE_H

