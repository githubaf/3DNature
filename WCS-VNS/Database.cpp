// Database.cpp
// GIS Database object for World Construction Set
// Written from scratch on 03/20/95 by Chris "Xenon" Hanson.
// Effects attribute loading and Effects initialization added 5/97 by Gary R. Huber
// Copyright 1995 by Questar Productions. All rights reserved.

#include "stdafx.h"
#include "Application.h"
#include "Requester.h"
#include "Project.h"
#include "AppMem.h"
#include "Database.h"
#include "Points.h"
#include "Useful.h"
#include "Layers.h"
#include "DEM.h"
#include "Joe.h"
#include "Log.h"
#include "EffectsLib.h"
#include "Raster.h"
#include "DBFilterEvent.h"
#include "WCSVersion.h"
#include "Lists.h"
#include "FeatureConfig.h"

//#define OLD_DEBUG_CODE

//#define DATABASE_CHUNK_DEBUG 1
#ifdef DATABASE_CHUNK_DEBUG
static long fpos;
static char debugStr[256];
#endif // DATABASE_CHUNK_DEBUG

extern unsigned short Default8[];

#define WCS_DATABASE_JOE_SBUF_SIZE	600

NotifyTag DBNewEvent[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_NEW, 0, 0), 0};
NotifyTag DBPreLoadEvent[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_PRELOAD, 0, 0), 0};
NotifyTag DBLoadEvent[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_LOAD, 0, 0), 0};
NotifyTag DBAddEvent[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_ADDOBJ, 0, 0), 0};
NotifyTag DBPreChgActEvent[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_PRECHANGEACTIVE, 0, 0), 0};
NotifyTag DBChgActEvent[] = {MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_CHANGEACTIVE, 0, 0), 0};

/*===========================================================================*/

Database::Database(void)
{

SuppressNotifiesDuringLoad = 0;

StaticRoot	= new ("StaticRoot") Joe;
DynamicRoot = new ("DynamicRoot") Joe;
LogRoot		= new ("LogRoot") Joe;

if (StaticRoot && DynamicRoot && LogRoot)
	{
	StaticRoot->SetFlags(WCS_JOEFLAG_HASKIDS);
	DynamicRoot->SetFlags(WCS_JOEFLAG_HASKIDS);
	LogRoot->SetFlags(WCS_JOEFLAG_HASKIDS);
	
	StaticRoot->GroupInfo.GroupChild = StaticRoot->GroupInfo.JChild = NULL;
	StaticRoot->GroupInfo.NumKids = StaticRoot->GroupInfo.NumGroups = 0;

	DynamicRoot->GroupInfo.GroupChild = DynamicRoot->GroupInfo.JChild = NULL;
	DynamicRoot->GroupInfo.NumKids = DynamicRoot->GroupInfo.NumGroups = 0;

	LogRoot->GroupInfo.GroupChild = LogRoot->GroupInfo.JChild = NULL;
	LogRoot->GroupInfo.NumKids = LogRoot->GroupInfo.NumGroups = 0;
	} // if

GenericPrev = ActiveObj = NULL;
ResetGeoClip();

} // Database::Database

/*===========================================================================*/

Database::~Database(void)
{

SuppressNotifiesDuringLoad = 1;

if (StaticRoot && DynamicRoot && LogRoot)
	{
	DestroyAll();
	} // if

if (LogRoot)
	{
	delete LogRoot;
	LogRoot = NULL;
	} // if
if (DynamicRoot)
	{
	delete DynamicRoot;
	DynamicRoot = NULL;
	} // if
if (StaticRoot)
	{
	delete StaticRoot;
	StaticRoot = NULL;
	} // if

} // Database::~Database

/*===========================================================================*/

int Database::InitValid(void)
{

return(StaticRoot && DynamicRoot && LogRoot);

} // Database:InitValid

/*===========================================================================*/

void Database::DestroyAll(void)
{
LayerStub *LayerStrip;

if (StaticRoot)
	{
	ScanKill(StaticRoot);
	StaticRoot->NWLat = -90.0; StaticRoot->NWLon = -180.0;
	StaticRoot->SELat = 90.0; StaticRoot->SELon = 180.0;
	while (LayerStrip = StaticRoot->FirstLayer())
		{
		StaticRoot->RemoveObjectFromLayer(TRUE, NULL, LayerStrip);
		} // for
	} // if

if (DynamicRoot)
	{
	ScanKill(DynamicRoot);
	DynamicRoot->NWLat = -90.0; DynamicRoot->NWLon = -180.0;
	DynamicRoot->SELat = 90.0; DynamicRoot->SELon = 180.0;
	while (LayerStrip = DynamicRoot->FirstLayer())
		{
		DynamicRoot->RemoveObjectFromLayer(TRUE, NULL, LayerStrip);
		} // for
	} // if

if (LogRoot)
	{
	ScanKill(LogRoot);
	LogRoot->NWLat = -90.0; LogRoot->NWLon = -180.0;
	LogRoot->SELat = 90.0; LogRoot->SELon = 180.0;
	while (LayerStrip = LogRoot->FirstLayer())
		{
		LogRoot->RemoveObjectFromLayer(TRUE, NULL, LayerStrip);
		} // for
	} // if

MasterPoint.DestroyAll();
DBLayers.DestroyAll();
ActiveObj = NULL;

// Tell everyone else what just happened
if (!SuppressNotifiesDuringLoad)
	{
	GenerateNotify(DBNewEvent);
	} // if

} // Database::DestroyAll

/*===========================================================================*/

void Database::RemoveAll(Project *CurProj, EffectsLib *Effects, long ObjType)
{
Joe *NextJoe, *CurJoe;
RasterAnimHostProperties Prop;
int RemoveFiles = 0;

Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;

ResetGeoClip();

if (ObjType == WCS_RAHOST_OBJTYPE_DEM)
	{
	if ((RemoveFiles = UserMessageYNCAN("Database: Remove DEMs",
		"Delete DEM elevation files from disk\nas well as remove DEMs from the Database?", 1)) == NULL)
		return;
	RemoveFiles = RemoveFiles == 2 ? 1: 0;
	} // if

CurJoe = GetFirst();

while (CurJoe)
	{
	if (! CurJoe->TestFlags(WCS_JOEFLAG_HASKIDS))
		{
		CurJoe->GetRAHostProperties(&Prop);
		if (Prop.TypeNumber == ObjType)
			{
			NextJoe = GetNext(CurJoe);
			if (! RemoveRAHost(CurJoe, CurProj, Effects, RemoveFiles, 0))
				{
				if (ObjType == WCS_RAHOST_OBJTYPE_VECTOR && ! UserMessageOKCAN("Remove Vectors", "Continue removing remaining Vectors?"))
					return;
				else if (ObjType == WCS_RAHOST_OBJTYPE_DEM && ! UserMessageOKCAN("Remove DEMs", "Continue removing remaining DEMs?"))
					return;
				else if (ObjType == WCS_RAHOST_OBJTYPE_CONTROLPT && ! UserMessageOKCAN("Remove Control Points", "Continue removing remaining Control Points?"))
					return;
				} // if remove failed
			CurJoe = NextJoe;
			} // if
		else
			CurJoe = GetNext(CurJoe);
		} // if
	else
		CurJoe = GetNext(CurJoe);
	} // for

} // Database::RemoveAll

/*===========================================================================*/

void Database::FreeVecSegmentData(void)
{
Joe *CurJoe;

ResetGeoClip();

CurJoe = GetFirst();

while (CurJoe)
	{
	CurJoe->FreeRenderData();
	CurJoe = GetNext(CurJoe);
	} // while CurJoe

} // Database::FreeVecSegmentData

/*===========================================================================*/

unsigned long Database::HowManyObjs(void)
{

return(StaticRoot->GroupInfo.NumKids + DynamicRoot->GroupInfo.NumKids + LogRoot->GroupInfo.NumKids + 3);

} // Database::HowManyObjs

/*===========================================================================*/

void Database::ScanKill(Joe *Origin)
{
Joe *Kill, *Scan, *NextScan;
LayerStub *LayerKill, *LayerScan;
JoeAttribute *KillAttrib, *NextVictim;

if (Origin)
	{
	NextScan = Origin;
	Kill = NULL;
	while (NextScan)
		{
#ifdef OLD_DEBUG_CODE
		if (NextScan->Name())
			{
			printf("[%s]\n", NextScan->Name());
			} // if
		else if (NextScan->FileName())
			{
			printf("[%s]\n", NextScan->FileName());
			} // else if
#endif // OLD_DEBUG_CODE
		Scan = NextScan;
		NextScan = Kill = NULL;
		if (Scan->TestFlags(WCS_JOEFLAG_HASKIDS))
			{
			Scan->GroupInfo.NumKids = Scan->GroupInfo.NumGroups = 0;
			// no need to test first case
			// if (Scan->GroupInfo.GroupChild)
				{
				NextScan = Scan->GroupInfo.GroupChild;
				Scan->GroupInfo.GroupChild = NULL; // Don't come back
				} // if
			if (!NextScan)
				{
				NextScan = Scan->GroupInfo.JChild;
				Scan->GroupInfo.JChild = NULL;
				} // if
			} // if
		if (!NextScan)
			{
			if (Scan != Origin)
				{
				if (Scan->NextJoe)
					{
					Kill = Scan;
					NextScan = Scan->NextJoe;
					} // if
				else
					{
//					if (Scan->Parent != Origin)
//						{
						Kill = Scan;
						NextScan = Scan->Parent;
//						} // if
					} // else
				} // if
			} // else
		if (Kill)
			{
			if (Kill->Points())
				{
				// The Points must be freed from this scope, since the PointsAllocator
				// is in the scope of the Database object, and is unreachable from
				// within the scope of the Joe object.
				MasterPoint.DeAllocate(Kill->Points());
				Kill->Points(NULL);
				} // if
			if (Kill->LayerList)
				{
				LayerKill = Kill->LayerList;
				while (LayerKill)
					{
					LayerScan = LayerKill->NextLayerInObject();
					delete LayerKill;
					LayerKill = LayerScan;
					} // while
				Kill->LayerList = NULL;
				} // if
			if (!Kill->TestFlags(WCS_JOEFLAG_HASKIDS))
				{ // Check Extended attributes
				for (KillAttrib = Kill->AttribInfo.FirstAttrib; KillAttrib; KillAttrib = NextVictim)
					{
					NextVictim = KillAttrib->NextAttrib;
					//AppMem_Free(KillAttrib, KillAttrib->AttribClassSize);
					// Remember, we're calling delete on the base JoeAttribute class, not the derived class,
					// so if we add any fancy dynamic storage to any derived classes, we'll need a
					// virtual destructor...
					delete KillAttrib;
					} // for
				Kill->AttribInfo.FirstAttrib = NULL;
				} // if
			delete Kill;
			} // if
		} // while
	} // if

} // Database::ScanKill

/*===========================================================================*/

void Database::GetBounds(double &North, double &South, double &East, double &West)
{

North = -90.0;
South = 90.0;
East = 180.0;
West = -180.0;

if (StaticRoot->NWLat > North)
	North = StaticRoot->NWLat;
if (StaticRoot->SELat < South)
	South = StaticRoot->SELat;
if (StaticRoot->NWLon > West)
	West = StaticRoot->NWLon;
if (StaticRoot->SELon < East)
	East = StaticRoot->SELon;

if (DynamicRoot->NWLat > North)
	North = DynamicRoot->NWLat;
if (DynamicRoot->SELat < South)
	South = DynamicRoot->SELat;
if (DynamicRoot->NWLon > West)
	West = DynamicRoot->NWLon;
if (DynamicRoot->SELon < East)
	East = DynamicRoot->SELon;

if (LogRoot->NWLat > North)
	North = LogRoot->NWLat;
if (LogRoot->SELat < South)
	South = LogRoot->SELat;
if (LogRoot->NWLon > West)
	West = LogRoot->NWLon;
if (LogRoot->SELon < East)
	East = LogRoot->SELon;

} // DataBase::GetBounds

/*===========================================================================*/

void Database::SetGeoClip(double North, double South, double East, double West)
{

GeoClipNorth = North + .001;
GeoClipSouth = South - .001;
GeoClipEast = East - .001;
GeoClipWest = West + .001;

} // Database::SetGeoClip

/*===========================================================================*/

void Database::ResetGeoClip(void)
{

GeoClipNorth = 100000.0;
GeoClipSouth = -100000.0;
GeoClipEast = -100000.0;
GeoClipWest = 100000.0;

} // Database::ResetGeoClip

/*===========================================================================*/

Joe *Database::GeoClipMatch(Joe *Me)
{

if (Me)
	{
#ifdef OLD_DEBUG_CODE
 printf("[N:%f<=%f, S:%f>=%f,\n E:%f>=%f, W:%f<=%f\n",
  Me->NWLat, GeoClipNorth, Me->SELat, GeoClipSouth,
  Me->SELon, GeoClipEast, Me->NWLon, GeoClipWest);
 if (((double)Me->NWLat <= GeoClipNorth)) printf("N");
 if (((double)Me->SELat >= GeoClipSouth)) printf("S");
 if (((double)Me->SELon >= GeoClipEast)) printf("E");
 if (((double)Me->NWLon <= GeoClipWest)) printf("W");
 printf("\n");
#endif // OLD_DEBUG_CODE

#ifdef OLD_CLIP_METHOD
	if
	 (
	 ( (Me->SELat <= GeoClipNorth) && (Me->NWLat >= GeoClipSouth) ) &&
	 ( (Me->SELon <= GeoClipWest ) && (Me->NWLon >= GeoClipEast ) )
	 )
		{
		return(Me);
		} // if
#else // !OLD_CLIP_METHOD
	if ((GeoClipNorth > 10000.0) || (GeoClipSouth < -10000.0) || (GeoClipEast > 10000.0) || (GeoClipWest < 10000.0))
		return(Me);
	if ((Me->SELat > GeoClipNorth))
		return(NULL);
	if ((Me->NWLat < GeoClipSouth))
		return(NULL);
	if ((Me->NWLon < GeoClipEast))
		return(NULL);
	if ((Me->SELon > GeoClipWest))
		return(NULL);
	return(Me);
#endif // !OLD_CLIP_METHOD
	} //if
return(NULL);

} // Database::GeoClipMatch

/*===========================================================================*/

Joe *Database::GetFirst(unsigned long Flags)
{
Joe *TestRoot = NULL;

if (Flags & WCS_DATABASE_DYNAMIC)
	{
// printf("D");
	if (!(TestRoot = DynamicRoot->GroupInfo.GroupChild))
		{
		TestRoot = DynamicRoot->GroupInfo.JChild;
		} // if
	} // if
else if (Flags & WCS_DATABASE_LOG)
	{
// printf("L");
	if (!(TestRoot = LogRoot->GroupInfo.GroupChild))
		{
		TestRoot = LogRoot->GroupInfo.JChild;
		} // if
	} // else if
else
	{
// printf("S");
	if (!(TestRoot = StaticRoot->GroupInfo.GroupChild))
		{
		TestRoot = StaticRoot->GroupInfo.JChild;
		} // if
	} // else
//printf("\n");

while (TestRoot)
	{
	if (GeoClipMatch(TestRoot))
		{
		//printf(" *\n");
		GenericPrev = TestRoot;
		return(TestRoot);
		} //if
	else
		{
		//printf("[...]");
		TestRoot = TestRoot->NextJoe;
		} // else
	} // while
//printf(" !\n");

if (Flags & WCS_DATABASE_DYNAMIC)
	{
	//printf("D");
	TestRoot = DynamicRoot->GroupInfo.JChild;
	} // if
else if (Flags & WCS_DATABASE_LOG)
	{
	//printf("L");
	TestRoot = LogRoot->GroupInfo.JChild;
	} // else if
else
	{
	//printf("S");
	TestRoot = StaticRoot->GroupInfo.JChild;
	} // else
//printf("\n");


while (TestRoot)
	{
	if (GeoClipMatch(TestRoot))
		{
		//printf(" *\n");
		GenericPrev = TestRoot;
		return(TestRoot);
		} //if
	else
		{
		//printf("[...]");
		TestRoot = TestRoot->NextJoe;
		} // else
	} // while

return(NULL);

} // Database::GetFirst

/*===========================================================================*/

Joe *Database::GimmeNextGroup(Joe *Previous)
{ // We don't support GenericPrev in this method

if (Previous)
	{
	if (Previous->TestFlags(WCS_JOEFLAG_HASKIDS))
		{
		if (Previous->GroupInfo.GroupChild)
			{
			return(Previous->GroupInfo.GroupChild);
			} // if
		else
			{
			if (Previous->NextJoe)
				{
				return(Previous->NextJoe);
				} // if
			else
				{
				if (Previous->Parent)
					{
					return(Previous->WalkUpGroup());
					} // if
				} // else
			} // else
		} // if
	} // if

return(NULL);

} // Database::GimmeNextGroup

/*===========================================================================*/

Joe* FASTCALL Database::GimmeNext(Joe *Previous)
{

if (Previous == NULL)
	{
	Previous = GenericPrev;
	} // if

if (Previous)
	{
	if (Previous->TestFlags(WCS_JOEFLAG_HASKIDS))
		{
		if (Previous->GroupInfo.GroupChild)
			{
			return(GenericPrev = Previous->GroupInfo.GroupChild);
			} // if
		else if (Previous->GroupInfo.JChild)
			{
			return(GenericPrev = Previous->GroupInfo.JChild);
			} // if
		} // if
	if (Previous->NextJoe)
		{
		return(GenericPrev = Previous->NextJoe);
		} // if
	else
		{ // go do parent
		if (Previous->Parent)
			{
			return(Previous->WalkUp());
			} // if		
		} // else
	} // if

return(NULL);

} // Database::GimmeNext

/*===========================================================================*/

Joe *Database::GimmeNextSameLevel(Joe *Me)
{

if (Me)
	{
	if (Me->NextJoe)
		{
		return(Me->NextJoe);
		} //if
	else
		{
		if (Me->Parent)
			{
			return(Me->WalkUp());
			} // if
		else
			{
			return(NULL);
			} // else
		} // if
	} // if

return(NULL);

} // Database::GimmeNextSameLevel

/*===========================================================================*/

Joe *Database::GetNext(Joe *Previous)
{
Joe *ClipTest;

if (Previous == NULL)
	{
	Previous = GenericPrev;
	} // if
if (Previous)
	{
	ClipTest = GimmeNext(Previous);
	while (ClipTest)
		{
		if (GeoClipMatch(ClipTest))
			{
			return(GenericPrev = ClipTest);
			} // if
		else
			{
			ClipTest = GimmeNextSameLevel(ClipTest);
			} // else
		} // while
	return(NULL);
	} // if

return(NULL);

} // Database::GetNext

/*===========================================================================*/

Joe *Database::GetNextSameLevel(Joe *Previous)
{
Joe *ClipTest;

if (Previous == NULL)
	{
	Previous = GenericPrev;
	} // if
if (Previous)
	{
	ClipTest = GimmeNextSameLevel(Previous);
	while (ClipTest)
		{
		if (GeoClipMatch(ClipTest))
			{
			return(ClipTest);
			} // if
		else
			{
			ClipTest = GimmeNextSameLevel(ClipTest);
			} // else
		} // while
	} // if

return(NULL);

} // Database::GetNextSameLevel

/*===========================================================================*/

/*
Joe *Database::WalkUp(Joe *Me)
{
while (Me)
	{
	if (Me->NextJoe)
		{
		return(Me->NextJoe);
		} // if
	else
		{
		Me = Me->Parent;
		} // else
	} // while
return(NULL);
} // Database::WalkUp
*/

/*===========================================================================*/

Joe *Database::AddJoe(Joe *Me, unsigned long Flags, Project *CurProj)
{
Joe *MyRoot = NULL;

//printf("%08X::",Flags);
if (Flags & WCS_DATABASE_STATIC)
	{
	//printf("S");
	MyRoot = StaticRoot;
	} // if
if (Flags & WCS_DATABASE_DYNAMIC)
	{
	//printf("D");
	MyRoot = DynamicRoot;
	} //if
if (Flags & WCS_DATABASE_LOG)
	{
	//printf("L");
	MyRoot = LogRoot;
	} //if

if (MyRoot)
	{
	return(Database::AddJoe(Me, MyRoot, CurProj));	
	} //if
else
	{
	return(NULL); // Sorry Charlie
	} //else

} // Database::AddJoe

/*===========================================================================*/

Joe *Database::AddJoe(Joe *Me, Joe *MyParent, Project *CurProj)
{
Joe *Ret;

if (MyParent && Me)
	{
	Ret = MyParent->AddChild(Me);
	if (!SuppressNotifiesDuringLoad)
		{
		if (CurProj)
			{
			if ((Me->TestRenderFlags()) && (Me->TestFlags(WCS_JOEFLAG_ISDEM)))
				{
				CurProj->SetFRDInvalid(1);
				} // if
			} // if
		GenerateNotify(DBAddEvent);
		} // if
	return(Ret);
	} // if

return(NULL);

} // Database::AddJoe

/*===========================================================================*/

unsigned long Database::SaveV2(FILE *SaveFile, unsigned long SaveFlags)
{

if (SaveFlags & WCS_DATABASE_STATIC)
	{
	return(SaveV2(SaveFile, StaticRoot));
	} // if
else if (SaveFlags & WCS_DATABASE_DYNAMIC)
	{
	return(SaveV2(SaveFile, DynamicRoot));
	} // else if
else if (SaveFlags & WCS_DATABASE_LOG)
	{
	return(SaveV2(SaveFile, LogRoot));
	} // else if

return(0);

} // Database::SaveV2(Name, Flags)

/*===========================================================================*/

unsigned long Database::SaveV2(FILE *SaveFile, Joe *SaveRoot)
{
void *JoeBuf = NULL;
Joe *SaveChild, *SkipNext;
unsigned long Out32, JumpBack, KidsToSave, PlaceHold, SizeHold,
 /*InningScore, FinalScore = 0, */ StartPos, EndPos;
unsigned short Out16;
char Success;

if (!(SaveRoot && SaveFile))
	{
	return(0);
	} // if

StartPos = ftell(SaveFile);

if (!SaveRoot->TestFlags(WCS_JOEFLAG_HASKIDS))
	{
	return(0);
	} // if

Success = 0;
Out32 = 01234567; // Architecture Byte Order constant
if (fwrite(&Out32, 4, 1, SaveFile))
	{
// The following odd hack allows us to embed databases into Project files
//	if (fwrite("WCS FILE", 8, 1, SaveFile))
	if (1)
		{
		Out16 = 2; // Major/version integer
		if (fwrite(&Out16, 2, 1, SaveFile))
			{
			Out16 = 1; // Minor/revision integer
			if (fwrite(&Out16, 2, 1, SaveFile))
				{
				Success = 1;
				} // if
			} // if
		} // if
	} // if

if (!Success)
	{
	UserMessageOK("Database Save", "File IO Error writing Database header.");
	return(0);
	} // if

if (DBLayers.HowManyLayers())
	{
	if (DBLayers.WriteLayerHeader(SaveFile) == 0)
		{
		UserMessageOK("Database Save Error","Could not write Layer Header information.");
		return(0);
		} // if
	} // if

if (fwrite("WCSDBASE", 8, 1, SaveFile))
	{
#ifdef DATABASE_CHUNK_DEBUG
	sprintf(debugStr, "SaveV2 - WCSDBASE\n");
	OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
	Out32 = 0; // PlaceHolder;
	SizeHold = ftell(SaveFile);
	fwrite(&Out32, 4, 1, SaveFile); // Placeholder for WCSDBASE byte size counter
	PlaceHold = ftell(SaveFile);
	if (fwrite(&Out32, 4, 1, SaveFile))
		{
		// To find max depth of saveable group tree
		Out32 = Out16 = 1; // MaxDepth and Depth counters
		KidsToSave = 0; // Max Group counter

		SaveChild = SaveRoot;
		while (SaveChild)
			{
			if (!SaveChild->TemplateItem) KidsToSave++;
			SkipNext = NULL;
			if (SaveChild->TestFlags(WCS_JOEFLAG_HASKIDS) && !SaveChild->TestFlags(WCS_JOEFLAG_EXTFILE))
				{
				if (SaveChild->GroupInfo.JChild)
					{
					if (!(/*InningScore = */WriteTwig(SaveFile, SaveChild)))
						{
						return(0); // error writing twig
						} // if
					/* FinalScore += InningScore; */
					} // if
				else
					{
					// if we have no non-group children, this would be unitialized and will blow up the loader when it fseeks to it
					SaveChild->Utility = NULL;
					} // else
				if (SaveChild->GroupInfo.GroupChild)
					{
					SkipNext = SaveChild->GroupInfo.GroupChild;
					Out16++;
					if (Out16 > Out32)
						{
						Out32 = Out16;
						} // if
					} // if
				} // if
			if (!SkipNext)
				{
				if (SaveChild->NextJoe)
					{
					if (SaveChild != SaveRoot)
						{
						SkipNext = SaveChild->NextJoe;
						} // if
					} // if
				} // if
			if (!SkipNext)
				{ // Walk up
				while (SaveChild)
					{
					Out16--;
					if (SaveChild = SaveChild->Parent) // single-equals, yes
						{
						if (SaveChild != SaveRoot)
							{
							if (SaveChild->NextJoe)
								{
								SkipNext = SaveChild->NextJoe;
								break;
								} // if
							} // if
						else
							{
							break;
							} // else
						} // if
					} // while
				} // if
			SaveChild = SkipNext;
			} // while
		
		// write some interim header/TOC stuff
		JumpBack = ftell(SaveFile);
		fseek(SaveFile, PlaceHold, SEEK_SET);
		fwrite(&JumpBack, 4, 1, SaveFile);
		fseek(SaveFile, JumpBack, SEEK_SET);
		fwrite(&Out32, 4, 1, SaveFile);
		fwrite(&KidsToSave, 4, 1, SaveFile);

		SaveChild = SaveRoot;
		while (SaveChild)
			{
			SkipNext = NULL;
			if (!SaveChild->TemplateItem)
				{
				if (!(JoeBuf = SaveChild->WriteToFile(SaveFile, JoeBuf)))
					{
					return(0); // Crunch.
					} // if
				} // if
			/* FinalScore++; */
			if (SaveChild->TestFlags(WCS_JOEFLAG_HASKIDS) &&
			 !SaveChild->TestFlags(WCS_JOEFLAG_EXTFILE))
				{
				if (SaveChild->GroupInfo.GroupChild)
					{
					SkipNext = SaveChild->GroupInfo.GroupChild;
					} // if
				} // if
			if (!SkipNext)
				{
				if (SaveChild->NextJoe)
					{
					if (SaveChild != SaveRoot)
						{
						SkipNext = SaveChild->NextJoe;
						} // if
					} // if
				} // if
			if (!SkipNext)
				{ // Walk up
				while (SaveChild)
					{
					if (SaveChild = SaveChild->Parent) // single-equals, yes
						{
						if (SaveChild != SaveRoot)
							{
							if (SaveChild->NextJoe)
								{
								SkipNext = SaveChild->NextJoe;
								break;
								} // if
							} // if
						else
							{
							break;
							} // else
						} // if
					} // while
				} // if
			SaveChild = SkipNext;
			} // while
		if (JoeBuf)
			{
			SaveRoot->WriteFileCleanUp(JoeBuf);
			JoeBuf = NULL;
			} // if
		// Write 4-byte ignorable padding. We're not actually writing
		// the data that JoeBuf points to (that would be illegal),
		// we're actually writing the contents of the pointer, which
		// should be NULL at this point.
		fwrite(&JoeBuf, 4, 1, SaveFile);
		// Find out current position, calculate how many bytes we wrote,
		// skip back to the Size placeholder, write that value, and
		// jump back to where we are now.
		PlaceHold = ftell(SaveFile);
		Out32 = (PlaceHold - SizeHold) - 4;
		fseek(SaveFile, SizeHold, SEEK_SET); // jump way back
		fwrite(&Out32, 4, 1, SaveFile); // write size
		fseek(SaveFile, PlaceHold, SEEK_SET); // jump back
		EndPos = ftell(SaveFile);
#ifdef DATABASE_CHUNK_DEBUG
		sprintf(debugStr, "finished Database::SaveV2(normal return of %u)\n\n", EndPos - StartPos);
		OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
		return(EndPos - StartPos);
		} // if
	} // if

return(0);

} // Database::SaveV2(FILE, Joe)

/*===========================================================================*/

unsigned long Database::WriteTwig(FILE *SaveFile, Joe *SaveRoot)
{
Joe *TwigWalk;
void *Trivial = NULL;
unsigned long TwigResidents = 0;

if (SaveFile && SaveRoot)
	{
	// first count the children at this level
	for (TwigWalk = SaveRoot->GroupInfo.JChild; TwigWalk; TwigWalk = TwigWalk->NextJoe)
		{
		if (!TwigWalk->TemplateItem) TwigResidents++;
		} // for
	if (TwigResidents)
		{
		SaveRoot->Utility = ftell(SaveFile);
#ifdef DATABASE_CHUNK_DEBUG
		sprintf(debugStr, "WriteTwig(TwigResidents, Name/Filename): %u, %s/%s\n", TwigResidents, SaveRoot->Name(), SaveRoot->FName);
		OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
		if (fwrite(&TwigResidents, 4, 1, SaveFile))
			{
			TwigWalk = SaveRoot->GroupInfo.JChild;
			while (TwigWalk)
				{
				if (!TwigWalk->TemplateItem)
					{
					if (Trivial = TwigWalk->WriteToFile(SaveFile, Trivial))
						{
						TwigWalk = TwigWalk->NextJoe;
						} // if
					else
						{
						return(0); // fall out
						} // else
					} // if
				else
					{
					// move on w/o saving
					TwigWalk = TwigWalk->NextJoe;
					} // else
				} // while
			if (Trivial)
				{
				SaveRoot->WriteFileCleanUp(Trivial);
				Trivial = NULL;
				} // if
			fwrite(&Trivial, 4, 1, SaveFile);
			return(TwigResidents); // Success
			} // if
		} // if
	else
		{
		SaveRoot->Utility = NULL;
		return(1);
		} // else
	} // if

return(0);

} // Database::WriteTwig

/*===========================================================================*/

unsigned long Database::LoadV2(FILE *LoadFile, unsigned long LoadBytes, unsigned long LoadFlags,
	Project *CurProj, EffectsLib *CurEffects)
{
unsigned long LoadResult = 0;

if (LoadFlags & WCS_DATABASE_LOG)
	{
	LoadResult = LoadV2(LoadFile, LoadBytes, LogRoot, CurProj, CurEffects);
	LogRoot->TemplateItem = 0;
	//return(LoadResult);
	} // else if
else if (LoadFlags & WCS_DATABASE_DYNAMIC)
	{
	LoadResult = LoadV2(LoadFile, LoadBytes, DynamicRoot, CurProj, CurEffects);
	DynamicRoot->TemplateItem = 0;
	//return(LoadResult);
	} // else if
else if (LoadFlags & WCS_DATABASE_STATIC)
//else // (LoadFlags & WCS_DATABASE_STATIC)
	{
	LoadResult = LoadV2(LoadFile, LoadBytes, StaticRoot, CurProj, CurEffects);
	StaticRoot->TemplateItem = 0;
	//return(LoadResult);
	} // if

return(LoadResult);

} // Database::LoadV2(LoadName, Flags)

/*===========================================================================*/

unsigned long Database::LoadV2(FILE *LoadFile, unsigned long LoadBytes, Joe *LoadRoot,
	Project *CurProj, EffectsLib *CurEffects)
{
LayerEntry **LayerIndex = NULL, *TempMake;
Joe *UtilErase, *LoadPoint;
unsigned long In32, LayersToLoad = 0, Count, JoesToLoad, StartPos, EndPos;
long LPri;
unsigned short In16, Version, Revision;
unsigned char LNameLen, Success = 0, EndianFlop = 0;
char StringBuf[260];

SuppressNotifiesDuringLoad = 1;

if (LoadRoot && LoadFile)
	{
	StartPos = ftell(LoadFile);
	Success = 1;
	for (UtilErase = LoadRoot; UtilErase;)
		{
		if (UtilErase->Parent)
			{
			UtilErase = UtilErase->Parent;
			} // if
		else
			{
			break;
			} // else
		} // for
	while (UtilErase)
		{
		if (UtilErase->TestFlags(WCS_JOEFLAG_HASKIDS))
			{
			UtilErase->Utility = NULL;
			UtilErase = GimmeNextGroup(UtilErase);
			} // if
		else
			{
			Success = 0;
			} // else
		} // while

	if (!Success)
		{
		SuppressNotifiesDuringLoad = 0;
		EndPos = ftell(LoadFile);
		return(EndPos - StartPos);
		} // if
	Success = 0;

	if (fread(&In32, 4, 1, LoadFile))
		{
		if (In32 == 0x00053977)
			{ // No Endian swap necessary
			} // if
		else if (In32 == 0x77390500)
			{ // Need to Endian-Swap
			EndianFlop = 1;
			} // else if
		else
			{
			UserMessageOK("Database Load Error", "Unknown file format or Architecture Byte Order.");
			SuppressNotifiesDuringLoad = 0;
			EndPos = ftell(LoadFile);
			return(EndPos - StartPos);
			} // else
// The following quickie hacks are to embed databases into project files

//		if (fread(StringBuf, 8, 1, LoadFile))
		if (1)
			{
//			StringBuf[8] = NULL;
//			if (!strcmp(StringBuf, "WCS FILE"))
			if (1)
				{
				if (fread(&In16, 2, 1, LoadFile))
					{
					if (EndianFlop)
						{
						SimpleEndianFlip16U(In16, &Version);
						} // if
					else
						{
						Version = In16;
						} // else
					// Read revision number
					if (fread(&In16, 2, 1, LoadFile))
						{
						if (EndianFlop)
							{
							SimpleEndianFlip16U(In16, &Revision);
							} // if
						else
							{
							Revision = In16;
							} // else
						Success = 1;
						} // if
					if (Version != 2)
						{
						sprintf(StringBuf, "V%d.%d Database format not supported.", Version, Revision);
						UserMessageOK("Database Load Error", StringBuf);
						} // if
					} // if
				} // if
			else
				{
				UserMessageOK("Database Load Error", "Not a "APP_TLA" file.");
				} // else
			} // if
		} // if
	} // if

if (!Success)
	{
	UserMessageOK("Database Load Error", "File IO.");
	EndPos = ftell(LoadFile);
	SuppressNotifiesDuringLoad = 0;
	return(EndPos - StartPos);
	} // if

// Done reading header stuff, look for Layers stuff
Success = 0;
if (fread(StringBuf, 8, 1, LoadFile))
	{
	StringBuf[8] = NULL;
	if (!strcmp(StringBuf, "WCSLAYER"))
		{
#ifdef DATABASE_CHUNK_DEBUG
		sprintf(debugStr, "LoadV2 - WCSLAYER\n");
		OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
		// Read and ignore byte-size
		fread(&In32, 4, 1, LoadFile);
		// read and build layers
		if (fread(&In32, 4, 1, LoadFile))
			{
			if (EndianFlop)
				{
				SimpleEndianFlip32U(In32, &LayersToLoad);
				} // if
			else
				{
				LayersToLoad = In32;
				} // else
			if (LayerIndex = new LayerEntry *[LayersToLoad])
				{
				for (Count = 0; Count < LayersToLoad; Count++)
					{
					if (fread(&In32, 4, 1, LoadFile))
						{ // In32 holds flags momentarily
						if (EndianFlop)
							{
							SimpleEndianFlip32U(In32, &In32);
							} // if
						if (fread(&LPri, 4, 1, LoadFile))
							{
							if (EndianFlop)
								{
								SimpleEndianFlip32S(LPri, &LPri);
								} // if
							if (fread(&LNameLen, 1, 1, LoadFile))
								{
								if (fread(StringBuf, LNameLen, 1, LoadFile))
									{
									StringBuf[LNameLen] = NULL;
#ifdef DATABASE_CHUNK_DEBUG
									sprintf(debugStr, "  Layer: %s\n", StringBuf);
									OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
									if (TempMake = DBLayers.MatchMakeLayer(StringBuf, LPri))
										{
										TempMake->SetFlags(In32);
										LayerIndex[Count] = TempMake;
										Success = 1;
										} // if
									} // if
								} // if
							} // if
						} // if
					if (!Success)
						{
						delete [] LayerIndex;
						LayerIndex = NULL;
						} // if
					} // for
				} // if
			} // if
		if (Success)
			{
			StringBuf[0] = StringBuf[8] = NULL;
			fread(StringBuf, 8, 1, LoadFile);
			} // if
		else
			{
			UserMessageOK("Database Load Error", "Unable to read LayerTable.");
			EndPos = ftell(LoadFile);
			SuppressNotifiesDuringLoad = 0;
			return(EndPos - StartPos);
			} // else
		} // if
	if (!strcmp((char *)StringBuf, "WCSDBASE"))
		{
#ifdef DATABASE_CHUNK_DEBUG
		sprintf(debugStr, "LoadV2 - WCSDBASE\n");
		OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
		// Read and ignore byte-size
		fread(&In32, 4, 1, LoadFile);
		// read and add objects
		if (fread(&In32, 4, 1, LoadFile))
			{
			if (EndianFlop)
				{
				SimpleEndianFlip32U(In32, &In32);
				} // if
			fseek(LoadFile, In32, SEEK_SET);
#ifdef DATABASE_CHUNK_DEBUG
			fpos = ftell(LoadFile);
			sprintf(debugStr, "Database::LoadV2(Read & Add Objects: fpos = %d)\n", fpos);
			OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
			if (fread(&In32, 4, 1, LoadFile))
				{
				if (EndianFlop)
					{
					SimpleEndianFlip32U(In32, &JoesToLoad);
					} // if
				else
					{
					JoesToLoad = In32;
					} // else
#ifdef DATABASE_CHUNK_DEBUG
				sprintf(debugStr, "  JoesToLoad: %u\n", JoesToLoad);
				OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
				if (fread(&In32, 4, 1, LoadFile))
					{
					if (EndianFlop)
						{
						SimpleEndianFlip32U(In32, &Count);
						} // if
					else
						{
						Count = In32;
						} // else
#ifdef DATABASE_CHUNK_DEBUG
				sprintf(debugStr, "  Count: %u\n", Count);
				OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
// 				printf("\nLoading %d Group Joes.\n", Count);
					if (LoadGroup(LoadFile, LoadRoot, JoesToLoad, EndianFlop, Version, Revision, CurProj, CurEffects) == Count)
						{
						LoadPoint = LoadRoot;
						while (LoadPoint)
							{
							if (LoadPoint->Utility)
								{
								if (!fseek(LoadFile, LoadPoint->Utility, SEEK_SET))
									{
									//printf("POS: %d\n", ftell(LoadFile));
#ifdef DATABASE_CHUNK_DEBUG
									sprintf(debugStr, "-->Seeking to %u\n", LoadPoint->Utility);
									OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
									if (fread(&In32, 4, 1, LoadFile))
										{
										if (EndianFlop)
											{
											SimpleEndianFlip32U(In32, &JoesToLoad);
											} // if
										else
											{
											JoesToLoad = In32;
											} // else
										//printf("%d Objects to be read.\n", JoesToLoad);
										if (JoesToLoad)
											{
#ifdef DATABASE_CHUNK_DEBUG
											sprintf(debugStr, "JoesToLoad = %u\n", JoesToLoad);
											OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
												// Load a few (JoesToLoad) objects
// 										printf("Loading %d Attrib Joes.\n", JoesToLoad);
											In32 = LoadAttrib(LoadFile, LoadPoint, JoesToLoad, LayersToLoad,
											 LayerIndex, EndianFlop, Version, Revision, CurProj, CurEffects);
											if (In32 == JoesToLoad)
												{
												Count += In32;
												} // if
											else
												{
#ifdef DATABASE_CHUNK_DEBUG
												sprintf(debugStr, "Did not load proper number of objects: %u\n", In32);
												OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
												printf("Did not load proper number of objects: %u\n", In32);
												} // else
											} // if
										} // if
									} // if
								} // if
							LoadPoint = GimmeNextGroup(LoadPoint);
							} // while
						LoadRoot->UpdateGroupBounds();
						sprintf(StringBuf, "%d objects.", Count);
						GlobalApp->StatusLog->PostStockError(WCS_LOG_MSG_DBS_LOAD, StringBuf);
						EndPos = ftell(LoadFile);
						fseek(LoadFile, StartPos + LoadBytes, SEEK_SET);
						SuppressNotifiesDuringLoad = 0;
						GenerateNotify(DBLoadEvent);
						if (LayerIndex) delete [] LayerIndex;
						LayerIndex = NULL;
#ifdef DATABASE_CHUNK_DEBUG
				sprintf(debugStr, "finished DataBase::LoadV2(LoadBytes = %u)", LoadBytes);
				OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
						return(LoadBytes);
						} // if
					} // if
				} // if
			} // if

		SuppressNotifiesDuringLoad = 0;
		if (LayerIndex) delete [] LayerIndex;
#ifdef DATABASE_CHUNK_DEBUG
		sprintf(debugStr, "finished DataBase::LoadV2(err return 1)");
		OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
		return(0);
		} // if
	} // if

SuppressNotifiesDuringLoad = 0;
if (LayerIndex) delete [] LayerIndex;
#ifdef DATABASE_CHUNK_DEBUG
sprintf(debugStr, "finished DataBase::LoadV2(err return 2)");
OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
return(0);

} // DataBase::LoadV2(FILE, Joe)

/*===========================================================================*/

unsigned long Database::LoadGroup(FILE *LoadFile, Joe *LoadRoot, unsigned long Depth,
	char Flop, unsigned short Version, unsigned short Revision, Project *CurProj, EffectsLib *CurEffects)
{
LoadStack *Stack;
char *ParseBuf = NULL, *SBuf = NULL;
Joe *Branch;
unsigned long JoeFileSize, In32, Ok, RecurseLevel = 0, PBufSize = 0, TotalLoaded = 0;

SBuf = (char *)AppMem_Alloc(WCS_DATABASE_JOE_SBUF_SIZE, APPMEM_CLEAR);

if (LoadFile && Depth && SBuf)
	{
	if (Stack = new LoadStack[Depth])
		{
		for (Ok = 1; Ok;)
			{
			Ok = 0;
#ifdef DATABASE_CHUNK_DEBUG
			fpos = ftell(LoadFile);
			sprintf(debugStr, "Database::LoadGroup(fpos = %d)\n", fpos);
			OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
			if (fread(&In32, 4, 1, LoadFile))
				{
				if (Flop)
					{
					SimpleEndianFlip32U(In32, &JoeFileSize);
					} // if
				else
					{
					JoeFileSize = In32;
					} // else
				if (JoeFileSize)
					{
#ifdef DATABASE_CHUNK_DEBUG
					sprintf(debugStr, "Database::LoadGroup(JoeFileSize = %u)\n", JoeFileSize);
					OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
// 				printf("J:%d ", JoeFileSize);
					if (JoeFileSize > PBufSize)
						{
						if (ParseBuf)
							{
							AppMem_Free(ParseBuf, PBufSize);
							} // if
						if (ParseBuf = (char *)AppMem_Alloc(JoeFileSize, APPMEM_CLEAR))
							{
							PBufSize = JoeFileSize;
							} // if
						else
							{
							PBufSize = NULL;
							} // else
						} // if
					if (ParseBuf)
						{
#ifdef DATABASE_CHUNK_DEBUG
						fpos = ftell(LoadFile);
						sprintf(debugStr, "Database::LoadGroup(ParseBuf read @ fpos = %d)\n\n", fpos);
						OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
						if (fread(ParseBuf, JoeFileSize, 1, LoadFile))
							{
							if (RecurseLevel)
								{
								if (Branch = ParseJoe(ParseBuf, JoeFileSize, SBuf, Flop, &Stack[RecurseLevel], 0, NULL, NULL, NULL, NULL, Version, Revision, CurProj, CurEffects))
									{
									Ok = 1;
									TotalLoaded++;
									} // if
								} // if
							else
								{
								if (Branch = ParseJoe(ParseBuf, JoeFileSize, SBuf, Flop, &Stack[RecurseLevel], 0, NULL, NULL, LoadRoot, NULL, Version, Revision, CurProj, CurEffects))
									{
									Ok = 1;
									TotalLoaded++;
									} // if
								} // else
							if (Ok)
								{
								if (Stack[RecurseLevel].GroupKids)
									{
									RecurseLevel++;
									Stack[RecurseLevel].LevelParent = Branch;
									} // if
								else
									{
									while (RecurseLevel)
										{
										RecurseLevel --;
										Stack[RecurseLevel].GroupKids--;
										if (Stack[RecurseLevel].GroupKids)
											{
											RecurseLevel++;
											Stack[RecurseLevel].GroupKids = Stack[RecurseLevel].AttribKids = NULL;
											break;
											} // if
										} // while
									if (!RecurseLevel)
										{
										Ok = 0;
										} // if
									} // else
								} // if
							} // if
						} // if
					} // if
				} // if
			} // for
		if (ParseBuf)
			{
			AppMem_Free(ParseBuf, PBufSize);
			ParseBuf = NULL; PBufSize = 0;
			} // if
		delete [] Stack;
		} // if
	if (SBuf)
		{
		AppMem_Free(SBuf, WCS_DATABASE_JOE_SBUF_SIZE);
		SBuf = NULL;
		} // if
	} // if

#ifdef DATABASE_CHUNK_DEBUG
sprintf(debugStr, "finished Database::LoadGroup(TotalLoaded = %u)\n\n", TotalLoaded);
OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
return(TotalLoaded);

} // Database::LoadGroup

/*===========================================================================*/

unsigned long Database::LoadAttrib(FILE *LoadFile, Joe *LoadRoot, unsigned long Number,
	unsigned long LIdx, LayerEntry **LayerIndex, char Flop, unsigned short Version,
	unsigned short Revision, Project *CurProj, EffectsLib *CurEffects)
{
char *LoadBuf = NULL, *StringBuf = NULL;
Joe *PrevLoaded = NULL;
unsigned long TotLoaded = 0, LoadSize, LBufSize = 0;
#ifdef DATABASE_CHUNK_DEBUG
long fTale;
#endif // DATABASE_CHUNK_DEBUG

StringBuf = (char *)AppMem_Alloc(600, APPMEM_CLEAR);

if (LoadFile && LoadRoot && Number && StringBuf)
	{
	if (fread(&LoadSize, 4, 1, LoadFile))
		{
		if (Flop)
			{
			SimpleEndianFlip32U(LoadSize, &LoadSize);
			} // if
#ifdef DATABASE_CHUNK_DEBUG
		fTale = ftell(LoadFile);
		sprintf(debugStr, "  LoadAttrib(LoadSize): %u\n", LoadSize);
		OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
		while (LoadSize)
			{
			LoadSize += 4;
			if (LoadSize > LBufSize)
				{
				if (LoadBuf)
					{
					AppMem_Free(LoadBuf, LBufSize);
					LoadBuf = NULL;
					LBufSize = 0;
					} // if
				if (LoadBuf = (char *)AppMem_Alloc(LoadSize, APPMEM_CLEAR))
					{
					LBufSize = LoadSize;
					} // if
				} // if
			if (LoadBuf)
				{
				if (fread(LoadBuf, LoadSize, 1, LoadFile))
					{
#ifdef DATABASE_CHUNK_DEBUG
					sprintf(debugStr, "  LoadAttrib(LoadBuf): %c\n", LoadBuf);
					OutputDebugString(debugStr);
#endif // DATABASE_CHUNK_DEBUG
					// Parse it
					if (!(PrevLoaded = ParseJoe(LoadBuf, LoadSize - 4, StringBuf, Flop, NULL, LIdx, LayerIndex, LoadRoot, NULL, PrevLoaded, Version, Revision, CurProj, CurEffects)))
						{
						UserMessageOK("Database Load Error", "Problems parsing.");
						} // if
					else
						{
						TotLoaded++;
						memcpy(&LoadSize, &LoadBuf[LoadSize - 4], 4);
						if (Flop)
							{
							SimpleEndianFlip32U(LoadSize, &LoadSize);
							} // if
// 					printf("NextLoadSize: %d.\n", LoadSize);
						} // else
					} // if
				} // if
			else
				break;	// otherwise endless loop of trying to allocate ever larger blocks
			} // while
		} // if
	if (StringBuf)
		{
		AppMem_Free(StringBuf, 600);
		StringBuf = NULL;
		} // if
	} // if

if (LoadBuf)
	{
	AppMem_Free(LoadBuf, LBufSize);
	LoadBuf = NULL; LBufSize = 0;
	} // if

return(TotLoaded);

} // Database::LoadAttrib

/*===========================================================================*/

Joe *Database::ParseJoe(char *JoeBuf, unsigned long JBSize, char *StringBuf, char Flop,
	LoadStack *Tree, unsigned long NLayers, LayerEntry **LayerIndex, Joe *LoadRoot,
	Joe *LoadTarget, Joe *PrevJoe, unsigned short Version, unsigned short Revision,
	Project *CurProj, EffectsLib *CurEffects)
{
double InFP;
char *JBIdx, *JBEnd, SetSPFP = 0, GrabBounds = 0;
Joe *LoadIn = NULL;
LayerEntry *TextLayer;
VectorPoint *Current;
JoeDEM *MyDEM;
float InSPFP;
unsigned long In32, LInc, LMax, ReadBytes;
long ConvertElevation;
unsigned short In16, HeaderSize;

#define WCS_DATABASE_GROUP_HEADER_SIZE_V2_1		36
#define WCS_DATABASE_MIN_HEADER_SIZE_V2_1		4
#define WCS_DATABASE_FIXED_HEADER_SIZE_V2_0		20

if (JoeBuf && StringBuf)
	{
	// Check flags so we know header size
	memcpy(&In32, &JoeBuf[0], 4);
	if (Flop)
		{
		SimpleEndianFlip32U(In32, &In32);
		} // if

	if ((Version == 2) && (Revision == 1))
		{
		if (In32 & (WCS_JOEFLAG_HASKIDS | WCS_JOEFLAG_ISDEM))
			{
			HeaderSize = WCS_DATABASE_GROUP_HEADER_SIZE_V2_1;
			} // if
		else
			{ // without bounds fields
			HeaderSize = WCS_DATABASE_MIN_HEADER_SIZE_V2_1;
			} // else
		} // if
	else //if ((Version == 2) && (Revision == 0))
		HeaderSize = WCS_DATABASE_FIXED_HEADER_SIZE_V2_0;

	JBIdx = &JoeBuf[HeaderSize + 1]; // used to be 21
	JBEnd = JoeBuf + JBSize;
	StringBuf[0] = NULL;
	if (JoeBuf[HeaderSize]) // used to be 20
		{
		strncpy(StringBuf, &JoeBuf[HeaderSize + 1], JoeBuf[HeaderSize]); /* strncpy(StringBuf, &JoeBuf[21], JoeBuf[20]); */
		StringBuf[JoeBuf[HeaderSize] - 1] = NULL; /* StringBuf[JoeBuf[20] - 1] = NULL; */ // J.I.C.
		JBIdx += JoeBuf[HeaderSize]; /* JBIdx += JoeBuf[20]; */
		} // if
	//printf("\nN: '%s'\n", StringBuf);
	if (JBIdx < JBEnd)
		{
		if (JBIdx[0] == WCS_JOEFILE_FILENAME)
			{
			strcat(StringBuf, "\n");
			JBIdx[1 + JBIdx[1]] = NULL; // Could overwrite on corrupt files
			strcat(StringBuf, &JBIdx[2]);
			JBIdx = &JBIdx[2 + JBIdx[1]];
			//printf("FN: '%s'\n", &JBIdx[2]);
			} // if
		} // if
	
	if (!(LoadIn = LoadTarget))
		{
		LoadIn = new (StringBuf) Joe;
		} // if
	if (LoadIn)
		{
		LoadIn->TemplateItem = GlobalApp->TemplateLoadInProgress;
		//printf("\nC: ");
		if (Tree)
			{
			LoadIn->SetFlags(WCS_JOEFLAG_HASKIDS);
			if (Tree->LevelParent)
				{
				if (!Tree->LevelParent->AddChild(LoadIn))
					{
					UserMessageOK("Database Load", "Error adding child.");
					delete LoadIn;
					LoadIn = NULL;
					return(0);
					} // if
				} // if
			} // if
		if (LoadRoot)
			{
			if (!LoadRoot->AddChild(LoadIn, PrevJoe))
				{
				UserMessageOK("Database Load", "Error adding child.");
				delete LoadIn;
				LoadIn = NULL;
				return(0);
				} // if
			} // if
	// Process fixed-size header info (only on groups)
	memcpy(&In32, &JoeBuf[0], 4);
	if (Flop)
		{
		SimpleEndianFlip32U(In32, &LoadIn->Flags);
		} // if
	else
		{
		LoadIn->Flags = In32;
		} // else
	// Clear certain flags
	LoadIn->Flags &= ~(WCS_JOEFLAG_NAMEREPLACED | WCS_JOEFLAG_MODIFIED | WCS_JOEFLAG_NEWRETRYCACHE);
	if (LoadIn->TestFlags(WCS_JOEFLAG_ISDEM))
		{
		// Dry Land no longer supported in V5 and later,
		// so clear it on load
		LoadIn->Flags &= ~(WCS_JOEFLAG_DEM_NO_SEALEVEL);
		} // if
	if (LoadIn->TestFlags(WCS_JOEFLAG_OLDACTIVATED))
		{
		LoadIn->ClearFlags(WCS_JOEFLAG_OLDACTIVATED);
		LoadIn->SetFlags(WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED);
		} // if

		if ((Version == 2) && (Revision == 1) && (LoadIn->TestFlags(WCS_JOEFLAG_HASKIDS) || LoadIn->TestFlags(WCS_JOEFLAG_ISDEM)) )
			{ // GeoBounds are now double-precision floating-point (double) instead of previous single-precison float
/*			const char *CheckName, *CheckFile;
			CheckName = LoadIn->Name();
			CheckFile = LoadIn->FileName(); */
			// NWLat
			//memcpy(&In32, &JoeBuf[4], 4);
			memcpy(&InFP, &JoeBuf[4], 8);
			if (Flop)
				{
				SimpleEndianFlip64(&InFP, &InFP);
				} // if
			memcpy(&LoadIn->NWLat, &InFP, 8);
			//NWLon
			//memcpy(&In32, &JoeBuf[8], 4);
			memcpy(&InFP, &JoeBuf[12], 8);
			if (Flop)
				{
				SimpleEndianFlip64(&InFP, &InFP);
				} // if
			memcpy(&LoadIn->NWLon, &InFP, 8);
			//SELat
			//memcpy(&In32, &JoeBuf[12], 4);
			memcpy(&InFP, &JoeBuf[20], 8);
			if (Flop)
				{
				SimpleEndianFlip64(&InFP, &InFP);
				} // if
			memcpy(&LoadIn->SELat, &InFP, 8);
			//SELon
			//memcpy(&In32, &JoeBuf[16], 4);
			memcpy(&InFP, &JoeBuf[28], 8);
			if (Flop)
				{
				SimpleEndianFlip64(&InFP, &InFP);
				} // if
			memcpy(&LoadIn->SELon, &InFP, 8);
			} // if database format V2.0 
		else if ((Version == 2) && (Revision == 0))
			{
			if (LoadIn->TestFlags(WCS_JOEFLAG_HASKIDS))
				{
				GrabBounds = 1;
				} // if
			else if (LoadIn->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				DEM TopoLoadCheck;

				if (TopoLoadCheck.AttemptLoadDEM(LoadIn, 0, CurProj))
					{
					LoadIn->NWLat = TopoLoadCheck.Northest();
					LoadIn->NWLon = TopoLoadCheck.Westest();
					LoadIn->SELat = TopoLoadCheck.Southest();
					LoadIn->SELon = TopoLoadCheck.Eastest();
					GrabBounds = 0;
					TopoLoadCheck.FreeRawElevs();
					} // if
				else
					{
					// Single precision is better than nothing.
					GrabBounds = 1;
					} // else
				} // if
			if (GrabBounds)
				{
				/*
				const char *CheckName, *CheckFile;
				CheckName = LoadIn->Name();
				CheckFile = LoadIn->FileName(); */
				// go ahead and read group bounds.
				// single-precision bounds for empty groups are better than nothing.
				// bounds for non-groups will get regenerated when points are done reading.
				// bounds for non-empty groups will get obliterated and regenerated as
				// their children are loaded, and up-propogation occurs when all loading is done.
				// NWLat
				memcpy(&In32, &JoeBuf[4], 4);
				if (Flop)
					{
					SimpleEndianFlip32U(In32, &In32);
					} // if
				memcpy(&InSPFP, &In32, 4);
				LoadIn->NWLat = InSPFP;
				//NWLon
				memcpy(&In32, &JoeBuf[8], 4);
				if (Flop)
					{
					SimpleEndianFlip32U(In32, &In32);
					} // if
				memcpy(&InSPFP, &In32, 4);
				LoadIn->NWLon = InSPFP;
				//SELat
				memcpy(&In32, &JoeBuf[12], 4);
				if (Flop)
					{
					SimpleEndianFlip32U(In32, &In32);
					} // if
				memcpy(&InSPFP, &In32, 4);
				LoadIn->SELat = InSPFP;
				//SELon
				memcpy(&In32, &JoeBuf[16], 4);
				if (Flop)
					{
					SimpleEndianFlip32U(In32, &In32);
					} // if
				memcpy(&InSPFP, &In32, 4);
				LoadIn->SELon = InSPFP;
				} // if
			} // if

		while (JBIdx < JBEnd)
			{
			switch((unsigned char)JBIdx[0])
				{
				case WCS_JOEFILE_LAYERSYM8:
					{
// 				printf("LS8 ");
					if (NLayers)
						{
						// Do it
						LMax = JBIdx[1];
						for (LInc = 0; LInc < LMax; LInc++)
							{
							In32 = (unsigned char)JBIdx[2 + LInc];
							if (In32 < NLayers)
								{
								if (!LoadIn->AddObjectToLayer(LayerIndex[In32]))
									{
									UserMessageOK("Database Load Error", "Failed to add layer!");
									} // if
								else
									{
// 								printf("[%s] ", LayerIndex[In32]->GetName());
									} // else
								} // if
							else
								{
								UserMessageOK("Database Load Error", "Illegal layer number.");
								} // else		
							} // for
						} // if
					else
						{
						UserMessageOK("Database Load Error", "Illegal 8-bit layer info.");
						} // else
					JBIdx = &JBIdx[2 + JBIdx[1]];
					break;
					} // LAYERSYM8
				case WCS_JOEFILE_LAYERSYM16:
					{
					//printf("LS16 ");
					memcpy(&In16, &JBIdx[1], 2);
					if (Flop)
						{
						SimpleEndianFlip16U(In16, &In16);
						} // if
					if (NLayers)
						{
						// Do it
						LMax = In16;
						for (LInc = 0; LInc < LMax; LInc++)
							{
							memcpy(&In16, &JBIdx[3 + (2 * LInc)], 2);
							if (Flop)
								{
								SimpleEndianFlip16U(In16, &In16);
								} // if
							if (In16 < NLayers)
								{
								if (!LoadIn->AddObjectToLayer(LayerIndex[In16]))
									{
									UserMessageOK("Database Load Error","Failed to add layer.");
									} // if
								else
									{
									//UserMessageOK("[%s] ", LayerIndex[In16]->GetName());
									} // else
								} // if
							else
								{
								UserMessageOK("Database Load Error", "Bad Layer-16.");
								} // else
							} // for
						} // if
					else
						{
						UserMessageOK("Database Load Error", "Illegal 16-bit layer info.");
						} // else
					JBIdx += 1 + (2 * (LMax + 1));
					break;
					} // 
				case WCS_JOEFILE_LAYERSYM32:
					{
					//printf("LS32 ");
					memcpy(&In32, &JBIdx[1], 4);
					if (Flop)
						{
						SimpleEndianFlip32U(In32, &In32);
						} // if
					if (NLayers)
						{
						// Do stuff
						LMax = In32;
						for (LInc = 0; LInc < LMax; LInc++)
							{
							memcpy(&In32, &JBIdx[5 + (4 * LInc)], 4);
							if (Flop)
								{
								SimpleEndianFlip32U(In32, &In32);
								} // if
							if (In32 < LMax)
								{
								if (!LoadIn->AddObjectToLayer(LayerIndex[In32]))
									{
									UserMessageOK("Database Load", "Error adding Layer32.");
									} // if
								else
									{
									//printf("[%s] ", LayerIndex[In32]->GetName());
									} // else
								} // if
							} // for
						} // if
					else
						{
						UserMessageOK("Database Load Error", "Illegal 32-bit layer info.");
						} // else
					JBIdx += 1 + (4 * (LMax + 1));
					break;
					} // LAYERSYM32
				case WCS_JOEFILE_LAYERNAME:
					{
					// do something
					//printf("LN ");
					if (JBIdx[1] > 1)
						{
						strncpy(StringBuf, &JBIdx[2], JBIdx[1]);
						StringBuf[JBIdx[1] + 1] = NULL;
						//printf("Layername: %s\n", StringBuf);
						if (TextLayer = DBLayers.MatchMakeLayer(StringBuf))
							{
							if (!LoadIn->AddObjectToLayer(TextLayer))
								{
								UserMessageOK("Database Load Error", "Couldn't AddObjectToLayer.");
								} // if
							} // if
						else
							{
							UserMessageOK("Database Load Error", "Couldn't MatchMakeLayer.");
							} // else
						} // if
					JBIdx = &JBIdx[2 + JBIdx[1]];
					break;
					} // LAYERNAME
				case WCS_JOEFILE_EXTATTRIB:
					{ // Not really implemented
					// DO stuff
					JBIdx += 2; // Class
					JBIdx += (1 + JBIdx[0]); // VarName
					memcpy(&In32, JBIdx, 4);
					if (Flop)
						{
						SimpleEndianFlip32U(In32, &In32);
						} // if
					JBIdx += (4 + In32);
					break;
					} // EXTATTRIB
				case WCS_JOEFILE_BOUND:
					{
					//printf("BD:");
					memcpy(&In32, &JBIdx[1], 4);
					if (Flop)
						{
						SimpleEndianFlip32U(In32, &In32);
						} // if
					if (Tree)
						{
						Tree->GroupKids = In32;
						} // if
					else
						{
						UserMessageOK("Database Load Error", "Bounds inappropriate in non-group object.");
						} // else
					//printf("%d, ", In32);
					memcpy(&In32, &JBIdx[5], 4);
					if (Flop)
						{
						SimpleEndianFlip32U(In32, &In32);
						} // if
					//printf("%d. ", In32);
					LoadIn->Utility = In32; // Offset
					// Do something with it
					JBIdx += 9;
					break;
					} // BOUND
				case WCS_JOEFILE_POINTS:
					{
					//printf("PT:");
					memcpy(&In32, &JBIdx[1], 4);
					if (Flop)
						{
						SimpleEndianFlip32U(In32, &LMax);
						} // if
					else
						{
						LMax = In32;
						} // else
					//printf("%d ", In32);
					JBIdx = &JBIdx[5];
					if (LoadIn->Points(MasterPoint.Allocate(LMax)))
						{
						LoadIn->NumPoints(LMax);
						Current = LoadIn->Points();
						for (LInc = 0; LInc < LMax; LInc++)
							{
							if (Current)
								{
								memcpy(&InFP, &JBIdx[20 * LInc], 8);
								if (Flop)
									{
									SimpleEndianFlip64(&InFP, &Current->Latitude);
									} // if
								else
									{
									Current->Latitude = InFP;
									} // else
								memcpy(&InFP, &JBIdx[(20 * LInc) + 8], 8);
								if (Flop)
									{
									SimpleEndianFlip64(&InFP, &Current->Longitude);
									} // if
								else
									{
									Current->Longitude = InFP;
									} // else
								memcpy(&In32, &JBIdx[(20 * LInc) + 16], 4);
								if (Flop)
									{
									SimpleEndianFlip32U(In32, &In32);
									} // if
								if (LoadIn->TestFlags(WCS_JOEFLAG_VEC_SPFP_ELEV))
									{
									memcpy(&Current->Elevation, &In32, 4);
									} // if
								else
									{
									// Don't set the flag yet, we'll set it when we're done loading the object...
									SetSPFP = 1;
									memcpy(&ConvertElevation, &In32, 4);
									Current->Elevation = (float)ConvertElevation;
									} // else
								// We now generate bounds on load for non-group objects
								// Now this is done after loading so as to use CoordSys reprojection
								//if (Current->Latitude  > LoadIn->NWLat) LoadIn->NWLat = Current->Latitude;
								//if (Current->Latitude  < LoadIn->SELat) LoadIn->SELat = Current->Latitude;
								//if (Current->Longitude > LoadIn->NWLon) LoadIn->NWLon = Current->Longitude;
								//if (Current->Longitude < LoadIn->SELon) LoadIn->SELon = Current->Longitude;
								} // if
							else
								{
								break;
								} // else
							Current = Current->Next;
							} // for
						} // if
					JBIdx = &JBIdx[20 * LMax];
					break;
					} // POINTS
				case WCS_JOEFILE_ATTRIBINFO:
					{ // Canonical types
					LMax = JBIdx[1]; // Count
					JBIdx = &JBIdx[2];
					for (LInc = 0; LInc < LMax; LInc++)
						{
						if (LInc == 0)
							{
							memcpy(&In16, &JBIdx[0], 2);
							if (Flop) SimpleEndianFlip16U(In16, &In16);
							LoadIn->AttribInfo.CanonicalTypeMajor = In16;
							memcpy(&In16, &JBIdx[2], 2);
							if (Flop) SimpleEndianFlip16U(In16, &In16);
							LoadIn->AttribInfo.CanonicalTypeMinor = In16;
							JBIdx = &JBIdx[4];
							} // if
						if (LInc == 1)
							{
							memcpy(&In16, &JBIdx[0], 2);
							if (Flop) SimpleEndianFlip16U(In16, &In16);
							LoadIn->AttribInfo.SecondTypeMajor = In16;
							memcpy(&In16, &JBIdx[2], 2);
							if (Flop) SimpleEndianFlip16U(In16, &In16);
							LoadIn->AttribInfo.SecondTypeMinor = In16;
							JBIdx = &JBIdx[4];
							} // if
						} // for
					break;
					} // WCS_JOEFILE_ATTRIBINFO
				case WCS_JOEFILE_LINEINFO:
					{
					//printf("LI ");
					// Do stuff here
					if (!LoadIn->TestFlags(WCS_JOEFLAG_HASKIDS))
						{
						unsigned char TempDrawPen;
						LoadIn->AttribInfo.LineWidth = JBIdx[1];
						LoadIn->AttribInfo.PointStyle = JBIdx[2];
						memcpy(&In16, &JBIdx[3], 2);
						if (Flop)
							{
							SimpleEndianFlip16U(In16, &LoadIn->AttribInfo.LineStyle);
							} // if
						else
							{
							LoadIn->AttribInfo.LineStyle = In16;
							} // else
						LoadIn->AttribInfo.RedGun	 = JBIdx[5];
						LoadIn->AttribInfo.GreenGun = JBIdx[6];
						LoadIn->AttribInfo.BlueGun  = JBIdx[7];
						// drawpen is deprecated but still in file
						//LoadIn->AttribInfo.DrawPen  = JBIdx[8];
						TempDrawPen  = JBIdx[8];
						if ((Version == 2) && (Revision == 0))
							{
							// Convert PenNum to RGB
							if ((LoadIn->AttribInfo.RedGun == 180) && (LoadIn->AttribInfo.GreenGun == 180) && (LoadIn->AttribInfo.BlueGun == 180))
								{ // try the view color
								LoadIn->AttribInfo.RedGun   = RedPart  (Default8[TempDrawPen]);
								LoadIn->AttribInfo.GreenGun = GreenPart(Default8[TempDrawPen]);
								LoadIn->AttribInfo.BlueGun  = BluePart (Default8[TempDrawPen]);
								} // if
							} // if
						} // if
					JBIdx = &JBIdx[9];
					break;
					} // LINEINFO
				case WCS_JOEFILE_DEMINFOEXTENDV3:
				case WCS_JOEFILE_DEMINFOEXTEND:
				case WCS_JOEFILE_DEMINFO:
					{
					//printf("DEM ");
					//if (MyDEM = (JoeDEM *)AppMem_Alloc(sizeof(JoeDEM), APPMEM_CLEAR))
					if (MyDEM = new JoeDEM)
						{
						// Don't look too hard at this. It's intentional.
						//MyDEM->JoeDEM::InitClear();
						MyDEM->MaxFract = JBIdx[1];
						memcpy(&MyDEM->MaxEl, &JBIdx[2], 2);
						if (Flop) SimpleEndianFlip16S(MyDEM->MaxEl, &MyDEM->MaxEl);

						memcpy(&MyDEM->MinEl, &JBIdx[4], 2);
						if (Flop) SimpleEndianFlip16S(MyDEM->MinEl, &MyDEM->MinEl);

						memcpy(&MyDEM->NWAlt, &JBIdx[6], 2);
						if (Flop) SimpleEndianFlip16S(MyDEM->NWAlt, &MyDEM->NWAlt);

						memcpy(&MyDEM->NEAlt, &JBIdx[8], 2);
						if (Flop) SimpleEndianFlip16S(MyDEM->NEAlt, &MyDEM->NEAlt);

						memcpy(&MyDEM->SEAlt, &JBIdx[10], 2);
						if (Flop) SimpleEndianFlip16S(MyDEM->SEAlt, &MyDEM->SEAlt);

						memcpy(&MyDEM->SWAlt, &JBIdx[12], 2);
						if (Flop) SimpleEndianFlip16S(MyDEM->SWAlt, &MyDEM->SWAlt);

						memcpy(&MyDEM->SumElDif, &JBIdx[14], 4);
						if (Flop) SimpleEndianFlip32F(&MyDEM->SumElDif, &MyDEM->SumElDif);

						memcpy(&MyDEM->SumElDifSq, &JBIdx[18], 4);
						if (Flop) SimpleEndianFlip32F(&MyDEM->SumElDifSq, &MyDEM->SumElDifSq);

						memcpy(&MyDEM->ElScale, &JBIdx[22], 8);
						if (Flop) SimpleEndianFlip64(&MyDEM->ElScale, &MyDEM->ElScale);
						
						// what happened to reading ElDatum?
						} // if
					if (JBIdx[0] == WCS_JOEFILE_DEMINFOEXTENDV3)
						{
						memcpy(&MyDEM->pLonEntries, &JBIdx[38], 4);
						if (Flop) SimpleEndianFlip32U(MyDEM->pLonEntries, &MyDEM->pLonEntries);
						memcpy(&MyDEM->pLatEntries, &JBIdx[42], 4);
						if (Flop) SimpleEndianFlip32U(MyDEM->pLatEntries, &MyDEM->pLatEntries);

						memcpy(&MyDEM->pElMaxEl, &JBIdx[46], 4);
						if (Flop) SimpleEndianFlip32F(&MyDEM->pElMaxEl, &MyDEM->pElMaxEl);
						memcpy(&MyDEM->pElMinEl, &JBIdx[50], 4);
						if (Flop) SimpleEndianFlip32F(&MyDEM->pElMinEl, &MyDEM->pElMinEl);
						memcpy(&MyDEM->pNullValue, &JBIdx[54], 4);
						if (Flop) SimpleEndianFlip32F(&MyDEM->pNullValue, &MyDEM->pNullValue);

						memcpy(&MyDEM->pLatStep, &JBIdx[58], 8);
						if (Flop) SimpleEndianFlip64(&MyDEM->pLatStep, &MyDEM->pLatStep);
						memcpy(&MyDEM->pLonStep, &JBIdx[66], 8);
						if (Flop) SimpleEndianFlip64(&MyDEM->pLonStep, &MyDEM->pLonStep);
						memcpy(&MyDEM->DEMGridNS, &JBIdx[74], 8);
						if (Flop) SimpleEndianFlip64(&MyDEM->DEMGridNS, &MyDEM->DEMGridNS);
						memcpy(&MyDEM->DEMGridWE, &JBIdx[82], 8);
						if (Flop) SimpleEndianFlip64(&MyDEM->DEMGridWE, &MyDEM->DEMGridWE);

						JBIdx = &JBIdx[90]; // 38 old, plus 52 ((4 * 2 = 8) + (4 * 3 = 12) + (8 * 4 = 32)), or 8+12+32=52 new
						} // if
					else if (JBIdx[0] == WCS_JOEFILE_DEMINFOEXTEND)
						{
						JBIdx = &JBIdx[38];
						} // else if
					else // WCS_JOEFILE_DEMINFO
						{
						JBIdx = &JBIdx[30];
						} // 
					LoadIn->AddAttribute(MyDEM);
					break;
					} // DEMINFO
				case WCS_JOEFILE_EFFECTINFO:
					{
					if ((ReadBytes = ParseEffectAttribute(JBIdx, LoadIn, Flop, CurEffects)) != 0)
						{
						JBIdx = &JBIdx[ReadBytes];
						} // if an effect attribute
					else
						{
						strcpy(StringBuf, "Error parsing Effects.");
						GlobalApp->StatusLog->PostStockError(WCS_LOG_WNG_ILL_VAL, StringBuf);
						return(LoadIn);
						} // else
					break;
					} // DEMINFO
				case WCS_JOEFILE_IEEELAYERATTRIB:
					{
					double IEEEDataVal;
					LayerStub *LS;

					// stole this code from LAYER32
					//printf("IEEE32 ");
					memcpy(&In32, &JBIdx[1], 4);
					if (Flop)
						{
						SimpleEndianFlip32U(In32, &In32);
						} // if
					memcpy(&IEEEDataVal, &JBIdx[5], 8);
					if (Flop)
						{
						SimpleEndianFlip64(&IEEEDataVal, &IEEEDataVal);
						} // if
					if (NLayers)
						{
						LS = LoadIn->CheckAttributeExistance(LayerIndex[In32]);
						if (!LS)
							{
							UserMessageOK("Database Load", "Error adding IEEEAttrib32.");
							} // if
						else
							{
							LS->SetIEEEAttribVal(IEEEDataVal);
							//printf("[IEEEATTR %s] ", LayerIndex[In32]->GetName());
							} // else
						} // if
					else
						{
						UserMessageOK("Database Load Error", "Illegal IEEE 32-bit attrib info.");
						} // else
					JBIdx += 1 + 4 + 8;
					break;
					} // WCS_JOEFILE_IEEELAYERATTRIB
				case WCS_JOEFILE_TEXTLAYERATTRIB:
					{
					char *TextDataBlock = NULL;
					unsigned long TextDataBlockSize;
					LayerStub *LS;

					// stole this code from LAYER32
					//printf("TEXT32 ");
					memcpy(&In32, &JBIdx[1], 4);
					if (Flop)
						{
						SimpleEndianFlip32U(In32, &In32);
						} // if
					memcpy(&TextDataBlockSize, &JBIdx[5], 4);
					if (Flop)
						{
						SimpleEndianFlip32U(TextDataBlockSize, &TextDataBlockSize);
						} // if
					TextDataBlock = &JBIdx[9];
					if (NLayers)
						{
						LS = LoadIn->CheckAttributeExistance(LayerIndex[In32]);
						if (!LS)
							{
							UserMessageOK("Database Load", "Error adding TextAttrib32.");
							} // if
						else
							{
							LS->SetTextAttribVal(TextDataBlock);
							//printf("[TEXTATTR %s] ", LayerIndex[In32]->GetName());
							} // else
						} // if
					else
						{
						UserMessageOK("Database Load Error", "Illegal TEXT 32-bit attrib info.");
						} // else
					JBIdx += 1 + 4 + 4 + TextDataBlockSize;
					break;
					} // WCS_JOEFILE_TEXTLAYERATTRIB
#ifdef WCS_BUILD_VNS
				case WCS_JOEFILE_UNIQUEID:
					{ // only used in VNS2's Scenarios -- will be automatically skipped if unsupported
					//printf("UID:");
					unsigned long JoeUID;

					// JBidx[1] is just a one-byte size counter = 4, which we ignore right now
					memcpy(&In32, &JBIdx[2], 4); // UniqueID
					if (Flop)
						{
						SimpleEndianFlip32U(In32, &JoeUID);
						} // if
					else
						{
						JoeUID = In32;
						} // else
					//printf("%x ", In32);
					JBIdx = &JBIdx[6];
					LoadIn->UniqueLoadSaveID = JoeUID;
					break;
					} // WCS_JOEFILE_UNIQUEID
#endif // WCS_BUILD_VNS
				default:
					{
					// Handle unrecognised tags
					if (((unsigned char)JBIdx[0] >= WCS_JOEFILE_VAR8) && ((unsigned char)JBIdx[0] < WCS_JOEFILE_VAR16))
						{
						JBIdx = &JBIdx[1 + 1 + JBIdx[1]];
						} // if
					else if (((unsigned char)JBIdx[0] >= WCS_JOEFILE_VAR16) && ((unsigned char)JBIdx[0] < WCS_JOEFILE_VAR32))
						{
						memcpy(&In16, &JBIdx[1], 2);
						if (Flop) SimpleEndianFlip16U(In16, &In16);
						JBIdx = &JBIdx[1 + 2 + In16];
						} // else if
					else if (((unsigned char)JBIdx[0] >= WCS_JOEFILE_VAR32) && ((unsigned char)JBIdx[0] < WCS_JOEFILE_MAX))
						{
						memcpy(&In32, &JBIdx[1], 4);
						if (Flop) SimpleEndianFlip32U(In32, &In32);
						JBIdx = &JBIdx[1 + 4 + In32];
						} // else if
					else
						{
						//UserMessageOK("Database Load Error", "Unrecognised Joe Tag.");
						//return(0);
						sprintf(StringBuf, "Unrecognised Joe Tag %d.", JBIdx[0]);
						GlobalApp->StatusLog->PostStockError(WCS_LOG_WNG_ILL_VAL, StringBuf);
						return(LoadIn);
						} // else
					} // default
				} // switch
			} // while
		if (SetSPFP) LoadIn->SetFlags(WCS_JOEFLAG_VEC_SPFP_ELEV);
		// Adjust our parent's bounds
		if (! (LoadIn->TestFlags(WCS_JOEFLAG_HASKIDS) || LoadIn->TestFlags(WCS_JOEFLAG_ISDEM)))
			{
			LoadIn->RecheckBounds();
			} // if
		if (LoadIn->Parent)
			{
			if (LoadIn->Parent->Children() == 1)
				{ // we're the first -- overwrite parent's bounds
				LoadIn->Parent->NWLat = LoadIn->NWLat;
				LoadIn->Parent->SELat = LoadIn->SELat;
				LoadIn->Parent->NWLon = LoadIn->NWLon;
				LoadIn->Parent->SELon = LoadIn->SELon;
				} // if
			else
				{ // expand parent's bounds
				if (LoadIn->NWLat > LoadIn->Parent->NWLat) LoadIn->Parent->NWLat = LoadIn->NWLat;
				if (LoadIn->SELat < LoadIn->Parent->SELat) LoadIn->Parent->SELat = LoadIn->SELat;
				if (LoadIn->NWLon > LoadIn->Parent->NWLon) LoadIn->Parent->NWLon = LoadIn->NWLon;
				if (LoadIn->SELon < LoadIn->Parent->SELon) LoadIn->Parent->SELon = LoadIn->SELon;
				} // else
			} // if
		return(LoadIn);
		} // if
	} // if

// Lint determined that you can not get here with a non-NULL LoadIn,
// so this is redundant. Pretty clever.
/*
if (!LoadTarget)
	{
	if (LoadIn)
		{
		delete LoadIn; // Handle layers?
		LoadIn = NULL;
		} // if
	} // if
*/
return(NULL);

} // Database::ParseJoe

/*===========================================================================*/

unsigned long Database::ParseEffectAttribute(char *JBIdx, Joe *LoadIn, char Flop, EffectsLib *CurEffects)
{
char *EffectName = NULL;
GeneralEffect *Effect;
unsigned long BytesToRead, BytesRead = 0;
char Index, Size;

memcpy(&BytesToRead, &JBIdx[1], 4);
if (Flop)
	SimpleEndianFlip32U(BytesToRead, &BytesToRead);
BytesRead = 1 + 4;

switch ((unsigned char)JBIdx[BytesRead])
	{
	case WCS_JOE_ATTRIB_INTERNAL_LAKE:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // switch
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_LAKE, EffectName))
				{
				JoeLake *MyAtrib;

				if (MyAtrib = new JoeLake)
					{
					MyAtrib->Lake = (LakeEffect *)Effect;
					LoadIn->AddAttribute(MyAtrib);
					Effect->AddToJoeList(LoadIn);
					} // if
				break;
				} // if found match
			} // if
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_LAKE
	case WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // switch
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_ECOSYSTEM, EffectName))
				{
				JoeEcosystem *MyAtrib;

				if (MyAtrib = new JoeEcosystem)
					{
					MyAtrib->Eco = (EcosystemEffect *)Effect;
					LoadIn->AddAttribute(MyAtrib);
					Effect->AddToJoeList(LoadIn);
					} // if
				break;
				} // if found match
			} // if
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM
	case WCS_JOE_ATTRIB_INTERNAL_ILLUMINATION:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // switch
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		// do nothing, Illumination Effect is obsolete
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_ILLUMINATION
	case WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // switch
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_RASTERTA, EffectName))
				{
				JoeRasterTA *MyAtrib;

				if (MyAtrib = new JoeRasterTA)
					{
					MyAtrib->Terra = (RasterTerraffectorEffect *)Effect;
					LoadIn->AddAttribute(MyAtrib);
					Effect->AddToJoeList(LoadIn);
					} // if
				break;
				} // if found match
			} // if
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_RASTERTERRAFFECTOR
	case WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // while
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_TERRAFFECTOR, EffectName))
				{
				JoeTerraffector *MyAtrib;

				if (MyAtrib = new JoeTerraffector)
					{
					MyAtrib->Terra = (TerraffectorEffect *)Effect;
					LoadIn->AddAttribute(MyAtrib);
					Effect->AddToJoeList(LoadIn);
					} // if
				break;
				} // if found match
			} // if
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_TERRAFFECTOR
	case WCS_JOE_ATTRIB_INTERNAL_SHADOW:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // while
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_SHADOW, EffectName))
				{
				JoeShadow *MyAtrib;

				if (MyAtrib = new JoeShadow)
					{
					MyAtrib->Shadow = (ShadowEffect *)Effect;
					LoadIn->AddAttribute(MyAtrib);
					Effect->AddToJoeList(LoadIn);
					} // if
				break;
				} // if found match
			} // if
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_SHADOW
	case WCS_JOE_ATTRIB_INTERNAL_FOLIAGE:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // while
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_FOLIAGE, EffectName))
				{
				JoeFoliage *MyAtrib;

				if (MyAtrib = new JoeFoliage)
					{
					MyAtrib->Foliage = (FoliageEffect *)Effect;
					LoadIn->AddAttribute(MyAtrib);
					Effect->AddToJoeList(LoadIn);
					} // if
				break;
				} // if found match
			} // if
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_FOLIAGE
	case WCS_JOE_ATTRIB_INTERNAL_STREAM:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // while
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_STREAM, EffectName))
				{
				JoeStream *MyAtrib;

				if (MyAtrib = new JoeStream)
					{
					MyAtrib->Stream = (StreamEffect *)Effect;
					LoadIn->AddAttribute(MyAtrib);
					Effect->AddToJoeList(LoadIn);
					} // if
				break;
				} // if found match
			} // if
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_STREAM
	case WCS_JOE_ATTRIB_INTERNAL_OBJECT3D:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // while
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_OBJECT3D, EffectName))
				{
				JoeObject3D *MyAtrib;

				if (MyAtrib = new JoeObject3D)
					{
					MyAtrib->Object = (Object3DEffect *)Effect;
					LoadIn->AddAttribute(MyAtrib);
					Effect->AddToJoeList(LoadIn);
					} // if
				break;
				} // if found match
			} // if
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_OBJECT3D
	case WCS_JOE_ATTRIB_INTERNAL_ALIGN3DOBJ:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // switch
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_OBJECT3D, EffectName))
				{
				// this creates the Joe attribute and adds it to the Joe
				((Object3DEffect *)Effect)->SetAlignVec(LoadIn);
				break;
				} // if found match
			} // if
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_ALIGN3DOBJ
	case WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // while
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_TERRAINPARAM, EffectName))
				{
				JoeTerrainParam *MyAtrib;

				if (MyAtrib = new JoeTerrainParam)
					{
					MyAtrib->TerrainPar = (TerrainParamEffect *)Effect;
					LoadIn->AddAttribute(MyAtrib);
					Effect->AddToJoeList(LoadIn);
					} // if
				break;
				} // if found match
			} // if
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_TERRAINPARAM
	case WCS_JOE_ATTRIB_INTERNAL_GROUND:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // while
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_GROUND, EffectName))
				{
				JoeGround *MyAtrib;

				if (MyAtrib = new JoeGround)
					{
					MyAtrib->Ground = (GroundEffect *)Effect;
					LoadIn->AddAttribute(MyAtrib);
					Effect->AddToJoeList(LoadIn);
					} // if
				break;
				} // if found match
			} // if
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_GROUND
	case WCS_JOE_ATTRIB_INTERNAL_SNOW:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // switch
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_SNOW, EffectName))
				{
				JoeSnow *MyAtrib;

				if (MyAtrib = new JoeSnow)
					{
					MyAtrib->SnowBusinessLikeShowBusiness = (SnowEffect *)Effect;
					LoadIn->AddAttribute(MyAtrib);
					Effect->AddToJoeList(LoadIn);
					} // if
				break;
				} // if found match
			} // if
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_SNOW
	case WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // while
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_ENVIRONMENT, EffectName))
				{
				JoeEnvironment *MyAtrib;

				if (MyAtrib = new JoeEnvironment)
					{
					MyAtrib->Env = (EnvironmentEffect *)Effect;
					LoadIn->AddAttribute(MyAtrib);
					Effect->AddToJoeList(LoadIn);
					} // if
				break;
				} // if found match
			} // if
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_ENVIRONMENT
	case WCS_JOE_ATTRIB_INTERNAL_CLOUD:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // while
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_CLOUD, EffectName))
				{
				JoeCloud *MyAtrib;

				if (MyAtrib = new JoeCloud)
					{
					MyAtrib->Cloud = (CloudEffect *)Effect;
					LoadIn->AddAttribute(MyAtrib);
					Effect->AddToJoeList(LoadIn);
					} // if
				break;
				} // if found match
			} // if
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_CLOUD
	case WCS_JOE_ATTRIB_INTERNAL_WAVE:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // while
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_WAVE, EffectName))
				{
				JoeWave *MyAtrib;

				if (MyAtrib = new JoeWave)
					{
					MyAtrib->Wave = (WaveEffect *)Effect;
					LoadIn->AddAttribute(MyAtrib);
					Effect->AddToJoeList(LoadIn);
					} // if
				break;
				} // if found match
			} // if
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_WAVE
	case WCS_JOE_ATTRIB_INTERNAL_CMAP:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // while
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_CMAP, EffectName))
				{
				JoeCmap *MyAtrib;

				if (MyAtrib = new JoeCmap)
					{
					MyAtrib->Cmap = (CmapEffect *)Effect;
					LoadIn->AddAttribute(MyAtrib);
					Effect->AddToJoeList(LoadIn);
					} // if
				break;
				} // if found match
			} // if
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_CMAP
	case WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // while
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		/* in VNS 3 there is no direct linkage between vectors and thematic maps
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, EffectName))
				{
				JoeThematicMap *MyAtrib;

				if (MyAtrib = new JoeThematicMap)
					{
					MyAtrib->Theme = (ThematicMap *)Effect;
					LoadIn->AddAttribute(MyAtrib);
					Effect->AddToJoeList(LoadIn);
					} // if
				break;
				} // if found match
			} // if
		*/
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP
	case WCS_JOE_ATTRIB_INTERNAL_COORDSYS:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // switch
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_COORDSYS, EffectName))
				{
				JoeCoordSys *MyAtrib;

				if (MyAtrib = new JoeCoordSys)
					{
					MyAtrib->Coord = (CoordSys *)Effect;
					LoadIn->AddAttribute(MyAtrib);
					MyAtrib->CoordsJoeList = Effect->AddToJoeList(LoadIn);
					} // if
				break;
				} // if found match
			} // if
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_COORDSYS
	case WCS_JOE_ATTRIB_INTERNAL_FENCE:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // switch
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_FENCE, EffectName))
				{
				JoeFence *MyAtrib;

				if (MyAtrib = new JoeFence)
					{
					MyAtrib->Fnce = (Fence *)Effect;
					LoadIn->AddAttribute(MyAtrib);
					Effect->AddToJoeList(LoadIn);
					} // if
				break;
				} // if found match
			} // if
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_FENCE
	case WCS_JOE_ATTRIB_INTERNAL_LABEL:
		{
		BytesRead ++;
		Index = JBIdx[BytesRead];
		BytesRead ++;
		while (Index)
			{
			Size = JBIdx[BytesRead];
			BytesRead ++;
			switch (Index)
				{
				case WCS_JOEFILE_EFFECTINFO_NAME:
					{
					EffectName = &JBIdx[BytesRead];
					break;
					} // WCS_JOEFILE_EFFECTINFO_NAME
				default:
					break;
				} // switch
			BytesRead += Size;
			Index = JBIdx[BytesRead];
			BytesRead ++;
			} // while
		if (EffectName)
			{
			if (Effect = CurEffects->FindByName(WCS_EFFECTSSUBCLASS_LABEL, EffectName))
				{
				JoeLabel *MyAtrib;

				if (MyAtrib = new JoeLabel)
					{
					MyAtrib->Labl = (Label *)Effect;
					LoadIn->AddAttribute(MyAtrib);
					Effect->AddToJoeList(LoadIn);
					} // if
				break;
				} // if found match
			} // if
		break;
		} // WCS_JOE_ATTRIB_INTERNAL_LABEL
// <<<>>> ADD_NEW_EFFECTS add only if effect can be attached to Joe
	default:
		{
		return (1 + 4 + BytesToRead);
		} // default - not a recognized effect
	} // switch

return (BytesRead);

} // Database::ParseEffectAttribute

/*===========================================================================*/

void Database::BoundUpTree(Joe *Origin)
{
Joe *NextPar;

if (Origin)
	{
	for (NextPar = Origin->Parent; NextPar;)
		{
		if (Origin->NWLat > NextPar->NWLat)
			{
			NextPar->NWLat = Origin->NWLat;
			} // if
		if (Origin->NWLon > NextPar->NWLon)
			{
			NextPar->NWLon = Origin->NWLon;
			} // if
		if (Origin->SELat < NextPar->SELat)
			{
			NextPar->SELat = Origin->SELat;
			} // if
		if (Origin->SELon < NextPar->SELon)
			{
			NextPar->SELon = Origin->SELon;
			} // if
		Origin = NextPar;
		NextPar = NextPar->Parent;
		} // for
	} // if

} // Database::BoundUpTree

/*===========================================================================*/

void Database::ReBoundTree(unsigned long DBFlags)
{

if (DBFlags & WCS_DATABASE_LOG)
	{
	ReBoundTree(LogRoot);
	} // if
if (DBFlags & WCS_DATABASE_DYNAMIC)
	{
	ReBoundTree(DynamicRoot);
	} // if
if (DBFlags & WCS_DATABASE_STATIC)
	{
	ReBoundTree(StaticRoot);
	} // if

} // Database::ReBoundTree(Flags)

/*===========================================================================*/

// Can be used to repair bounds of erroneous databases.
// Somewhat time-intensive
void Database::MegaReBoundTree(unsigned long DBFlags)
{

if (DBFlags & WCS_DATABASE_LOG)
	{
	MegaReBoundTree(LogRoot);
	ReBoundTree(LogRoot);
	} // if
if (DBFlags & WCS_DATABASE_DYNAMIC)
	{
	MegaReBoundTree(DynamicRoot);
	ReBoundTree(DynamicRoot);
	} // if
if (DBFlags & WCS_DATABASE_STATIC)
	{
	MegaReBoundTree(StaticRoot);
	ReBoundTree(StaticRoot);
	} // if

} // Database::MegaReBoundTree(Flags)

/*===========================================================================*/

void Database::ReBoundTree(Joe *Origin)
{
Joe *JoeBob;

for (JoeBob = Origin; JoeBob;)
	{
	if (JoeBob->NWLat == -10000.0 && JoeBob->NWLon == -10000.0 &&
	 JoeBob->SELat == 10000.0 && JoeBob->SELon == 10000.0)
		{
		// Set to non-zero extrema to prevent looping.
		JoeBob->NWLat = -DBL_MAX;
		JoeBob->NWLon = -DBL_MAX;
		JoeBob->SELat = DBL_MAX;
		JoeBob->SELon = DBL_MAX;
		if (JoeBob->TestFlags(WCS_JOEFLAG_HASKIDS))
			{
			JoeBob->Utility = 1; // Meaning we might need to check our own GroupInfo.JChild
			if (JoeBob->GroupInfo.NumGroups && JoeBob->GroupInfo.GroupChild)
				{
				JoeBob = JoeBob->GroupInfo.GroupChild;
				JoeBob->Utility = 1; // Meaning we might need to check the child's GroupInfo.JChild
				} // if
			} // if
		} // if
	else
		{
		if (JoeBob->TestFlags(WCS_JOEFLAG_HASKIDS) && (JoeBob->GroupInfo.JChild) && (JoeBob->Utility))
			{
			JoeBob->Utility = 0; // Meaning we've checked GroupInfo.JChild
			JoeBob = JoeBob->GroupInfo.JChild;
			JoeBob->Utility = 0; // Meaning no need to check the child's GroupInfo.JChild
			} // if
		else
			{
			if (JoeBob->Parent)
				{
				// Update our parent's bounds from ours
				if (JoeBob->NWLat > JoeBob->Parent->NWLat)
					JoeBob->Parent->NWLat = JoeBob->NWLat;

				if (JoeBob->NWLon > JoeBob->Parent->NWLon)
					JoeBob->Parent->NWLon = JoeBob->NWLon;

				if (JoeBob->SELat < JoeBob->Parent->SELat)
					JoeBob->Parent->SELat = JoeBob->SELat;

				if (JoeBob->SELon < JoeBob->Parent->SELon)
					JoeBob->Parent->SELon = JoeBob->SELon;
				
				} // if

			if (JoeBob->NextJoe)
				{
				JoeBob = JoeBob->NextJoe;
				JoeBob->Utility = 1; // Meaning we might need to check his GroupInfo.JChild
				} // if
			else
				{
				JoeBob->Utility = 0; // Clear it out.
				JoeBob = JoeBob->Parent;
				} // else
			} // else
		} // else
	} // for

} // Database::ReBoundTree

/*===========================================================================*/

// Can be used to repair bounds of erroneous databases.
// Somewhat time-intensive
void Database::MegaReBoundTree(Joe *Origin)
{
Joe *JoeBob;
BusyWin *BWRB = NULL;
int ReStep = 0;

BWRB = new BusyWin("Mega Rebound", HowManyObjs(), 'BWRB', 0);

for (JoeBob = Origin; JoeBob; JoeBob = GimmeNext(JoeBob))
	{
	if (JoeBob->TestFlags(WCS_JOEFLAG_HASKIDS))
		{
		JoeBob->NWLat = -10000.0;
		JoeBob->NWLon = -10000.0;
		JoeBob->SELat = 10000.0;
		JoeBob->SELon = 10000.0;
		} // if
	BWRB->Update(ReStep + 1);
	if (BWRB->CheckAbort())
		{
		JoeBob = NULL;
		} // if
	} // for

if (BWRB) delete BWRB;
BWRB = NULL;

} // Database::MegaReBoundTree

/*===========================================================================*/

Joe *Database::SetActiveObj(Joe *NewActive)
{
Joe *OldDude;
OldDude = ActiveObj;

if (!SuppressNotifiesDuringLoad)
	{
	GenerateNotify(DBPreChgActEvent, ActiveObj);
	} // if
ActiveObj = NewActive;
if (!SuppressNotifiesDuringLoad)
	{
	// this notify is a clone of the one generated by RasterAnimHost::SetActiveRAHost(ActiveObj) below and is redundent
	//GenerateNotify(DBChgActEvent, ActiveObj);
	} // if
RasterAnimHost::SetActiveRAHost(ActiveObj);

return(OldDude);

} // Database::SetActiveObj

/*===========================================================================*/

Joe *Database::NewObject(Project *CurProj, char *NewNameStr)
{
Joe *NewGuy = NULL;
char NameStr[80];

if (NewNameStr)
	strcpy(NameStr, NewNameStr);
else
	NameStr[0] = 0;
if (((NewNameStr && NameStr[0]) || GetInputString("Enter a name for the new object", "", NameStr)) && NameStr[0])
	{
	if (NewGuy = new (NameStr) Joe)
		{
		NewGuy->SetFlags(WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED);
		if (ActiveObj && ! ActiveObj->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			NewGuy->SetLineStyle(ActiveObj->GetLineStyle());
			NewGuy->SetLineWidth(ActiveObj->GetLineWidth());
			NewGuy->SetRGB(ActiveObj->Red(), ActiveObj->Green(), ActiveObj->Blue());
			NewGuy->SetFlags(ActiveObj->GetFlags() & (WCS_JOEFLAG_ISCONTROL | WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED));
			} // if
		else
			{
			NewGuy->SetLineStyle(4);
			NewGuy->SetLineWidth(1);
			NewGuy->SetRGB(180, 180, 180);
			} // else
		if (NewGuy = AddJoe(NewGuy, WCS_DATABASE_STATIC, CurProj))
			SetActiveObj(NewGuy);
		} // if
	} // if

return (NewGuy);

} // Database::NewObject()

/*===========================================================================*/

Joe *Database::AddDEMToDatabase(char *PathName, char *FileName, Project *CurProj, EffectsLib *CurEffects)
{
CoordSys *MyCoords = NULL;
DEM Topo;
char rootfile[256], filename[512], extension[32];

strcpy (rootfile, FileName);
stcgfe(extension, rootfile);
rootfile[strlen(rootfile) - strlen(extension) - (strlen(extension) > 0 ? 1: 0)] = 0;
TrimTrailingSpaces(rootfile);

strmfp(filename, PathName, FileName);
if (! Topo.LoadDEM(filename, 1, &MyCoords))
	{
	return (NULL);
	} // if error reading DEM file 

if (! CurProj->DirList_ItemExists(CurProj->DL, PathName))
	CurProj->DirList_Add(CurProj->DL, PathName, 0);

return (AddDEMToDatabase("Add DEM", rootfile, &Topo, MyCoords, CurProj, CurEffects));

} // Database::AddDEMToDatabase

/*===========================================================================*/

Joe *Database::AddDEMToDatabase(char *SourceStr, char *BaseName, DEM *Topo, CoordSys *MyNewCoords, Project *CurProj, EffectsLib *CurEffects)
{
Joe *Clip, *Added;
JoeDEM *MyDEM;
LayerEntry *LE;
#ifdef WCS_BUILD_VNS
JoeCoordSys *MyAttr;
CoordSys *AttachCoordSys, *MyOldCoords;
#endif // WCS_BUILD_VNS
NotifyTag ChangeEvent[2];
short Found = 0;
char NameStr[128]; // might want to be bigger?

for (Clip = GetFirst(); Clip ; Clip = GetNext(Clip))
	{
	if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
		{
		if (! stricmp(BaseName, Clip->FileName()))
			{
			Found = 1;
			break;
			} // 
		} // if
	} // for

if (Found)
	{
	Topo->MoJoe = Clip;
	Clip->SetFlags  (WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED | WCS_JOEFLAG_ISDEM | WCS_JOEFLAG_NEWRETRYCACHE);
	Clip->SetLineWidth(1);
	Clip->SetLineStyle(4);
	Clip->SetRGB(255, 255, 255);
	if (MyDEM = (JoeDEM *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
		{
		MyDEM->MaxFract = 7;
		MyDEM->MaxEl = (short)Topo->pElMaxEl;
		MyDEM->MinEl = (short)Topo->pElMinEl;
		MyDEM->SumElDif = (float)Topo->pElSumElDif;
		MyDEM->SumElDifSq = (float)Topo->pElSumElDifSq;
		MyDEM->ElScale = Topo->pElScale;
		MyDEM->ElDatum = Topo->pElDatum;
		// Transfer DEM bounds values into Joe
		Clip->NWLat = Topo->pNorthWest.Lat;
		Clip->NWLon = Topo->pNorthWest.Lon;
		Clip->SELon = Topo->pSouthEast.Lon;
		Clip->SELat = Topo->pSouthEast.Lat;
		#ifdef WCS_BUILD_VNS
		// if a different coord sys already attached, will need to remove it
		if (MyAttr = (JoeCoordSys *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
			MyOldCoords = MyAttr->Coord;
		else
			MyOldCoords = NULL;
		#endif // WCS_BUILD_VNS
		// don't attach CS if it is the EffectsLib's default CS
		if (MyNewCoords && MyNewCoords != CurEffects->FetchDefaultCoordSys())
			{
			#ifdef WCS_BUILD_VNS
			if (AttachCoordSys = CurEffects->CompareMakeCoordSys(MyNewCoords, 1))
				{
				// add new one if necessary, this removes the old one if necessary, rebounds the DEM
				if (MyOldCoords != AttachCoordSys)
					{
					Clip->AddEffect(AttachCoordSys, 1);
					} // if
				} // if
			#else // WCS_BUILD_VNS
			UserMessageOK(SourceStr, "You have added a DEM to the Database that contained Coordinate System information that WCS cannot use.");
			#endif // WCS_BUILD_VNS
			} // if
		#ifdef WCS_BUILD_VNS
		else if (MyAttr && MyAttr->CoordsJoeList && MyOldCoords)
			{
			// CS specified is NULL or the default one in EffectsLib - do not link to it or create a duplicate CS.
			// new CS is the default so remove the old linkage
			// this calls RemoveEffectAttribute on the Joe which removes JoeCoordSys 
			// from Joe and sends notify on Joe, deletes JoeCoordSys and JoeList and sends notify on CS.
			// RecheckBounds gets called before notify on Joe.
			MyOldCoords->RemoveFromJoeList(((JoeCoordSys *)MyAttr)->CoordsJoeList);
			// MyAttr has been deleted so NULL it.
			MyAttr = NULL;
			} // else if
		#endif // WCS_BUILD_VNS
		} // if
	Clip->ZeroUpTree();
	ReBoundTree(WCS_DATABASE_STATIC);
	} // if
else
	{
	sprintf(NameStr, "%s\n%s", BaseName, BaseName);
	if (Clip = new (NameStr) Joe)
		{
		Clip->SetFlags  (WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED | WCS_JOEFLAG_ISDEM | WCS_JOEFLAG_NEWRETRYCACHE);
		Clip->SetLineWidth(1);
		Clip->SetLineStyle(4);
		Clip->SetRGB(255, 255, 255);
		Topo->MoJoe = Clip;

		Added = AddJoe(Clip, WCS_DATABASE_STATIC, CurProj);
		if (Added)
			{
			SetActiveObj(Clip);
			} // if
		else
			{
			UserMessageOK(SourceStr, "Could not add object to Database.\nOperation terminated.");
			return (NULL);
			} // else

		if (LE = DBLayers.MatchMakeLayer("TOP", 0))
			{
			Clip->AddObjectToLayer(LE);
			} // if

		if (MyDEM = new JoeDEM)
			{
			MyDEM->MaxFract = 7;
			MyDEM->MaxEl = (short)Topo->pElMaxEl;
			MyDEM->MinEl = (short)Topo->pElMinEl;
			MyDEM->SumElDif = (float)Topo->pElSumElDif;
			MyDEM->SumElDifSq = (float)Topo->pElSumElDifSq;
			MyDEM->ElScale = Topo->pElScale;
			MyDEM->ElDatum = Topo->pElDatum;
			Clip->AddAttribute(MyDEM);
			// Transfer DEM bounds values into Joe
			Clip->NWLat = Topo->pNorthWest.Lat;
			Clip->NWLon = Topo->pNorthWest.Lon;
			Clip->SELon = Topo->pSouthEast.Lon;
			Clip->SELat = Topo->pSouthEast.Lat;
			// don't attach CS if it is the EffectsLib's default CS
			if (MyNewCoords && MyNewCoords != CurEffects->FetchDefaultCoordSys())
				{
				#ifdef WCS_BUILD_VNS
				if (AttachCoordSys = CurEffects->CompareMakeCoordSys(MyNewCoords, 1))
					{
					Clip->AddEffect(AttachCoordSys, 1);
					//Clip->RecheckBounds();
					} // if
				#else // WCS_BUILD_VNS
				UserMessageOK(SourceStr, "You have added a DEM to the Database that contained Coordinate System information that WCS cannot use.");
				#endif // WCS_BUILD_VNS
				} // if
			} // if
		else
			{
			UserMessageOK(SourceStr, "Could not add DEM Attribute tag.");
			} // 
		BoundUpTree(Clip);
		} // if
	} // else

ChangeEvent[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_ADDOBJ, 0, 0);
ChangeEvent[1] = NULL;
GenerateNotify(ChangeEvent);
if (Clip)
	{
	ChangeEvent[0] = MAKE_ID(Clip->GetNotifyClass(), Clip->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
	ChangeEvent[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(ChangeEvent, Clip->GetRAHostRoot());
	} // if

return (Clip);

} // Database::AddDEMToDatabase

/*===========================================================================*/

Joe *Database::FindByBestName(char *MatchName)
{
Joe *CurJoe;
const char *TestName;

ResetGeoClip();
for (CurJoe = GetFirst(); CurJoe; CurJoe = GetNext(CurJoe))
	{
	if (! CurJoe->TestFlags(WCS_JOEFLAG_HASKIDS))
		{
		TestName = CurJoe->GetBestName();
		if (! strcmp(TestName, MatchName))
			return (CurJoe);
		} // if
	} // for

return (NULL);

} // Database::FindByBestName

/*===========================================================================*/

#ifdef WCS_BUILD_V3
long Database::InitEffects(Project *CurProj, EffectsLib *Lib, EffectEval *EvalEffects, char *EffectEnabled, short ElevationOnly, double MetersPerDegLat)
#else // WCS_BUILD_V3
long Database::InitEffects(Project *CurProj, EffectsLib *Lib, char *EffectEnabled, short ElevationOnly, double MetersPerDegLat)
#endif // WCS_BUILD_V3
{
double North = -100000.0, South = 100000.0, East = 100000.0, West = -100000.0, Radius, NWLat, NWLon, SELat, SELon,
	RefLat, RefLon;
GeneralEffect *CurEffect;
Joe *VecWalk;
JoeList *CurJL;
RenderJoeList **RJLPtr, **SearchRJLPtr, *RJL = NULL, *SearchRJL = NULL, *RJLCur;
long UsedMem = 0, UsedMemStart, Success = 1, FoliageFileInUse = 0, MostMemoryToUse;
long EffectClass = WCS_EFFECTSSUBCLASS_LAKE;
#ifdef WCS_BUILD_VNS
long Found;
#endif // WCS_BUILD_VNS
short TempPriority, TempEvalOrder;

ResetGeoClip();

RefLat = CurProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y);
RefLon = CurProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X);

#ifdef WCS_VECPOLY_EFFECTS
MostMemoryToUse = LONG_MAX;	// in VNS 3 this limit is irrelevant.
#else // WCS_VECPOLY_EFFECTS
MostMemoryToUse = Lib->MaxMem * 1000000;
#endif // WCS_VECPOLY_EFFECTS

while (Success && EffectClass < WCS_MAXIMPLEMENTED_EFFECTS)
	{
	#ifdef WCS_VECPOLY_EFFECTS
	if (EffectEnabled[EffectClass] && Lib->TestInitToRender(EffectClass, ElevationOnly) 
		&& Lib->GetEffectTypePrecedence(EffectClass) < 0)
	#else // WCS_VECPOLY_EFFECTS
	if (EffectEnabled[EffectClass] && Lib->TestInitToRender(EffectClass, ElevationOnly))
	#endif // WCS_VECPOLY_EFFECTS
		{
		UsedMemStart = UsedMem;
		North = -100000.0;
		South = 100000.0;
		East = 100000.0;
		West = -100000.0;
		RJLPtr = &RJL;
		SearchRJLPtr = &SearchRJL;
		if (CurEffect = Lib->GetListPtr(EffectClass))
			{
			while (CurEffect)
				{
				TempPriority = CurEffect->Priority;
				TempEvalOrder = 0;
				Radius = 0.0;
				if (CurEffect->Enabled)
					{
					if (EffectClass == WCS_EFFECTSSUBCLASS_TERRAFFECTOR)
						{
						TerraffectorEffect *TEffect = (TerraffectorEffect *)CurEffect;
						double FirstDist;

						// we need max secton distance before InitFrameToRender is called which would initialize it
						TEffect->ADSection.GetMinMaxDist(FirstDist, TEffect->MaxSectionDist);
						Radius = max(TEffect->AnimPar[WCS_EFFECTS_TERRAFFECTOR_ANIMPAR_RADIUS].CurValue, TEffect->MaxSectionDist);
						Radius = Radius / MetersPerDegLat;
						TempEvalOrder = TEffect->EvalOrder;
						} // if
					else if (EffectClass == WCS_EFFECTSSUBCLASS_STREAM)
						{
						Radius = ((StreamEffect *)CurEffect)->AnimPar[WCS_EFFECTS_STREAM_ANIMPAR_RADIUS].CurValue / MetersPerDegLat;
						} // if
					else if (EffectClass == WCS_EFFECTSSUBCLASS_RASTERTA)
						{
						TempEvalOrder = ((RasterTerraffectorEffect *)CurEffect)->EvalOrder;
						} // if
					else if (EffectClass == WCS_EFFECTSSUBCLASS_OBJECT3D)
						{
						if (((Object3DEffect *)CurEffect)->ShadowsOnly)
							{
							CurEffect = CurEffect->Next;
							continue;
							} // if
						} // if
					else if (EffectClass == WCS_EFFECTSSUBCLASS_FENCE)
						{
						if (! ((((Fence *)CurEffect)->PostsEnabled) || 
							(((Fence *)CurEffect)->SpansEnabled) || 
							(((Fence *)CurEffect)->SkinFrame) || 
							(((Fence *)CurEffect)->RoofEnabled)))
							{
							CurEffect = CurEffect->Next;
							continue;
							} // if
						} // if
					else if (EffectClass == WCS_EFFECTSSUBCLASS_FOLIAGE)
						{
						if (((FoliageEffect *)CurEffect)->UseFoliageFile)
							{
							FoliageFileInUse = 1;
							CurEffect = CurEffect->Next;
							continue;
							} // if
						} // if
					if (CurJL = CurEffect->Joes)
						{
						while (CurJL)
							{
							if ((VecWalk = CurJL->Me) && VecWalk->TestFlags(WCS_JOEFLAG_ACTIVATED) && 
								! (VecWalk->TestFlags(WCS_JOEFLAG_HASKIDS)) &&
								! (VecWalk->TestFlags(WCS_JOEFLAG_ISDEM)) &&
								VecWalk->GetFirstRealPoint() && VecWalk->GetNumRealPoints() > 0)
								{
								if (EffectClass != WCS_EFFECTSSUBCLASS_OBJECT3D &&
									EffectClass != WCS_EFFECTSSUBCLASS_FOLIAGE &&
									EffectClass != WCS_EFFECTSSUBCLASS_FENCE &&
									EffectClass != WCS_EFFECTSSUBCLASS_LABEL)
									{
									VecWalk->GetTrueBounds(NWLat, NWLon, SELat, SELon);
									if (NWLat + Radius > North)
										North = NWLat + Radius;
									if (SELat - Radius < South)
										South = SELat - Radius;
									if (NWLon + Radius > West)
										West = NWLon + Radius;
									if (SELon - Radius < East)
										East = SELon - Radius;
									} // if
								if (*RJLPtr = new RenderJoeList())
									{
									(*RJLPtr)->Me = VecWalk;
									(*RJLPtr)->Effect = CurEffect;
									(*RJLPtr)->Priority = TempPriority;
									(*RJLPtr)->EvalOrder = TempEvalOrder;
									} // if
								else
									{
									Success = 0;
									break;
									} // else
								RJLPtr = (RenderJoeList **)&(*RJLPtr)->Next;
								} // if
							CurJL = CurJL->Next;
							} // while
						} // if Joes
					#ifdef WCS_BUILD_VNS
					#ifdef WCS_SEARCH_QUERY
					// if there is a search query run the database through it
					if (CurEffect->Search)
						{
						for (VecWalk = GetFirst(); VecWalk; VecWalk = GetNext(VecWalk))
							{
							if (VecWalk->TestFlags(WCS_JOEFLAG_ACTIVATED) &&
								! VecWalk->TestFlags(WCS_JOEFLAG_HASKIDS) && ! VecWalk->TestFlags(WCS_JOEFLAG_ISDEM))
								{
								if (VecWalk->GetFirstRealPoint() && VecWalk->GetNumRealPoints() > 0 && CurEffect->Search->ApproveJoe(VecWalk))
									{
									// test to see if it is already in the list
									Found = 0;
									RJLCur = RJL;
									while (RJLCur)
										{
										if (RJLCur->Me == VecWalk && RJLCur->Effect == CurEffect)
											{
											Found = 1;
											break;
											} // if
										RJLCur = (RenderJoeList *)RJLCur->Next;
										} // while
									if (! Found)
										{
										if (EffectClass != WCS_EFFECTSSUBCLASS_OBJECT3D &&
											EffectClass != WCS_EFFECTSSUBCLASS_FOLIAGE &&
											EffectClass != WCS_EFFECTSSUBCLASS_FENCE &&
											EffectClass != WCS_EFFECTSSUBCLASS_LABEL)
											{
											VecWalk->GetTrueBounds(NWLat, NWLon, SELat, SELon);
											if (NWLat + Radius > North)
												North = NWLat + Radius;
											if (SELat - Radius < South)
												South = SELat - Radius;
											if (NWLon + Radius > West)
												West = NWLon + Radius;
											if (SELon - Radius < East)
												East = SELon - Radius;
											} // if
										if (*SearchRJLPtr = new RenderJoeList())
											{
											(*SearchRJLPtr)->Me = VecWalk;
											(*SearchRJLPtr)->Effect = CurEffect;
											(*SearchRJLPtr)->Priority = TempPriority;
											(*SearchRJLPtr)->EvalOrder = TempEvalOrder;
											} // if
										else
											{
											Success = 0;
											break;
											} // else
										SearchRJLPtr = (RenderJoeList **)&(*SearchRJLPtr)->Next;
										} // if
									} // if
								} // if
							} // for
						} // if
					#endif // WCS_SEARCH_QUERY
					#endif // WCS_BUILD_VNS
					} // if
				CurEffect = CurEffect->Next;
				} // while
			*RJLPtr = SearchRJL;	// link the two lists together
			if (Success && (RJL || (EffectClass == WCS_EFFECTSSUBCLASS_OBJECT3D)
				|| (EffectClass == WCS_EFFECTSSUBCLASS_FOLIAGE && FoliageFileInUse)))
				{
				switch (EffectClass)
					{
					case WCS_EFFECTSSUBCLASS_LAKE:
						{
						Success = (long)Lib->LakeBase.Init((GeneralEffect *)Lib->Lake, RJL, North, South, East, West, 
							MetersPerDegLat, RefLat, RefLon, MostMemoryToUse, UsedMem);
						break;
						} // WCS_EFFECTSSUBCLASS_LAKE
					case WCS_EFFECTSSUBCLASS_ECOSYSTEM:
						{
						#ifdef WCS_BUILD_V3
						Success = (long)Lib->EcosystemBase.Init(EvalEffects, RJL, North, South, East, West, 
							MetersPerDegLat, RefLat, RefLon, MostMemoryToUse, UsedMem);
						#else // WCS_BUILD_V3
						Success = (long)Lib->EcosystemBase.Init((GeneralEffect *)Lib->Ecosystem, RJL, North, South, East, West, 
							MetersPerDegLat, RefLat, RefLon, MostMemoryToUse, UsedMem);
						#endif // WCS_BUILD_V3
						break;
						} // WCS_EFFECTSSUBCLASS_ECOSYSTEM
					case WCS_EFFECTSSUBCLASS_RASTERTA:
						{
						Success = (long)Lib->RasterTerraffectorBase.Init((GeneralEffect *)Lib->RasterTA, RJL, North, South, East, West, 
							MetersPerDegLat, RefLat, RefLon, MostMemoryToUse, UsedMem);
						break;
						} // WCS_EFFECTSSUBCLASS_RASTERTA
					case WCS_EFFECTSSUBCLASS_TERRAFFECTOR:
						{
						Success = (long)Lib->TerraffectorBase.Init((GeneralEffect *)Lib->Terraffector, RJL, North, South, East, West, 
							MetersPerDegLat, RefLat, RefLon, MostMemoryToUse, UsedMem);
						break;
						} // WCS_EFFECTSSUBCLASS_TERRAFFECTOR
					case WCS_EFFECTSSUBCLASS_SHADOW:
						{
						Success = (long)Lib->ShadowBase.Init((GeneralEffect *)Lib->Shadow, RJL, North, South, East, West, 
							MetersPerDegLat, RefLat, RefLon, MostMemoryToUse, UsedMem);
						break;
						} // WCS_EFFECTSSUBCLASS_SHADOW
					case WCS_EFFECTSSUBCLASS_FOLIAGE:
						{
						Success = (long)Lib->FoliageBase.Init(Lib->Foliage, RJL);
						break;
						} // WCS_EFFECTSSUBCLASS_FOLIAGE
					case WCS_EFFECTSSUBCLASS_STREAM:
						{
						Success = (long)Lib->StreamBase.Init((GeneralEffect *)Lib->Stream, RJL, North, South, East, West, 
							MetersPerDegLat, RefLat, RefLon, MostMemoryToUse, UsedMem);
						break;
						} // WCS_EFFECTSSUBCLASS_STREAM
					case WCS_EFFECTSSUBCLASS_OBJECT3D:
						{
						Success = (long)Lib->Object3DBase.Init(Lib->Object3D, RJL);
						break;
						} // WCS_EFFECTSSUBCLASS_OBJECT3D
					case WCS_EFFECTSSUBCLASS_TERRAINPARAM:
						{
						Success = (long)Lib->TerrainParamBase.Init((GeneralEffect *)Lib->TerrainParam, RJL, North, South, East, West, 
							MetersPerDegLat, RefLat, RefLon, MostMemoryToUse, UsedMem);
						break;
						} // WCS_EFFECTSSUBCLASS_TERRAINPARAM
					case WCS_EFFECTSSUBCLASS_GROUND:
						{
						Success = (long)Lib->GroundBase.Init((GeneralEffect *)Lib->Ground, RJL, North, South, East, West, 
							MetersPerDegLat, RefLat, RefLon, MostMemoryToUse, UsedMem);
						break;
						} // WCS_EFFECTSSUBCLASS_GROUND
					case WCS_EFFECTSSUBCLASS_SNOW:
						{
						Success = (long)Lib->SnowBase.Init((GeneralEffect *)Lib->Snow, RJL, North, South, East, West, 
							MetersPerDegLat, RefLat, RefLon, MostMemoryToUse, UsedMem);
						break;
						} // WCS_EFFECTSSUBCLASS_SNOW
					case WCS_EFFECTSSUBCLASS_ENVIRONMENT:
						{
						Success = (long)Lib->EnvironmentBase.Init((GeneralEffect *)Lib->Environment, RJL, North, South, East, West, 
							MetersPerDegLat, RefLat, RefLon, MostMemoryToUse, UsedMem);
						break;
						} // WCS_EFFECTSSUBCLASS_ENVIRONMENT
					case WCS_EFFECTSSUBCLASS_CMAP:
						{
						//Success = (long)Lib->CmapBase.Init((GeneralEffect *)Lib->Cmap, RJL, North, South, East, West, MetersPerDegLat, MostMemoryToUse, UsedMem);
						break;
						} // WCS_EFFECTSSUBCLASS_CMAP
					case WCS_EFFECTSSUBCLASS_FENCE:
						{
						Success = (long)Lib->FnceBase.Init(this, RJL);
						break;
						} // WCS_EFFECTSSUBCLASS_FENCE
					case WCS_EFFECTSSUBCLASS_LABEL:
						{
						Success = (long)Lib->LablBase.Init(RJL);
						break;
						} // WCS_EFFECTSSUBCLASS_LABEL
// <<<>>> ADD_NEW_EFFECTS add only if effect can be attached to Joe and GeoRasters must be created or other render init steps taken
					} // switch
				if (UsedMemStart < UsedMem)
					Lib->LogInitMemoryUsage(EffectClass, UsedMemStart, UsedMem);
				} // if
			} // if
		} // if the whole bloody group is enabled
	while (RJL)
		{
		RJLCur = (RenderJoeList *)RJL->Next;
		delete RJL;
		RJL = RJLCur;
		} // while
	//RJL = NULL; redundant
	SearchRJL = NULL;
	EffectClass ++;
	} // while

if (UsedMem > 0)
	Lib->LogInitMemoryUsage(-1, 0, UsedMem);

return (Success);

} // Database::InitEffects

/*===========================================================================*/

RenderJoeList *Database::CreateRenderJoeList(EffectsLib *Lib, long EffectClass)
{
short TempPriority, TempEvalOrder;
long Success = 1;
Joe *VecWalk;
RenderJoeList **RJLPtr, **SearchRJLPtr, *RJL = NULL, *SearchRJL = NULL;
JoeList *CurJL;
GeneralEffect *CurEffect;
#ifdef WCS_BUILD_VNS
long Found;
RenderJoeList *RJLCur;
#endif // WCS_BUILD_VNS

ResetGeoClip();

RJLPtr = &RJL;
SearchRJLPtr = &SearchRJL;
if (CurEffect = Lib->GetListPtr(EffectClass))
	{
	while (CurEffect)
		{
		TempPriority = CurEffect->Priority;
		TempEvalOrder = 0;
		if (CurEffect->Enabled)
			{
			if (EffectClass == WCS_EFFECTSSUBCLASS_TERRAFFECTOR)
				{
				TempEvalOrder = ((TerraffectorEffect *)CurEffect)->EvalOrder;
				} // if
			else if (EffectClass == WCS_EFFECTSSUBCLASS_RASTERTA)
				{
				TempEvalOrder = ((RasterTerraffectorEffect *)CurEffect)->EvalOrder;
				} // if
			else if (EffectClass == WCS_EFFECTSSUBCLASS_OBJECT3D)
				{
				if (((Object3DEffect *)CurEffect)->ShadowsOnly)
					{
					CurEffect = CurEffect->Next;
					continue;
					} // if
				} // if
			else if (EffectClass == WCS_EFFECTSSUBCLASS_FENCE)
				{
				if (!( (((Fence *)CurEffect)->PostsEnabled) || 
					(((Fence *)CurEffect)->SpansEnabled) || 
					(((Fence *)CurEffect)->SkinFrame) || 
					(((Fence *)CurEffect)->RoofEnabled)))
					{
					CurEffect = CurEffect->Next;
					continue;
					} // if
				} // if
			if (CurJL = CurEffect->Joes)
				{
				while (CurJL)
					{
					if ((VecWalk = CurJL->Me) && VecWalk->TestFlags(WCS_JOEFLAG_ACTIVATED) && 
						! (VecWalk->TestFlags(WCS_JOEFLAG_HASKIDS)) &&
						! (VecWalk->TestFlags(WCS_JOEFLAG_ISDEM)) &&
						VecWalk->GetFirstRealPoint() && VecWalk->GetNumRealPoints() > 0)
						{
						if (*RJLPtr = new RenderJoeList())
							{
							(*RJLPtr)->Me = VecWalk;
							(*RJLPtr)->Effect = CurEffect;
							(*RJLPtr)->Priority = TempPriority;
							(*RJLPtr)->EvalOrder = TempEvalOrder;
							} // if
						else
							{
							Success = 0;
							break;
							} // else
						RJLPtr = (RenderJoeList **)&(*RJLPtr)->Next;
						} // if
					CurJL = CurJL->Next;
					} // while
				} // if Joes
			#ifdef WCS_BUILD_VNS
			#ifdef WCS_SEARCH_QUERY
			// if there is a search query run the database through it
			if (CurEffect->Search)
				{
				for (VecWalk = GetFirst(); VecWalk; VecWalk = GetNext(VecWalk))
					{
					if (VecWalk->TestFlags(WCS_JOEFLAG_ACTIVATED) &&
						! VecWalk->TestFlags(WCS_JOEFLAG_HASKIDS) && ! VecWalk->TestFlags(WCS_JOEFLAG_ISDEM))
						{
						if (VecWalk->GetFirstRealPoint() && VecWalk->GetNumRealPoints() > 0 && CurEffect->Search->ApproveJoe(VecWalk))
							{
							// test to see if it is already in the list
							Found = 0;
							RJLCur = RJL;
							while (RJLCur)
								{
								if (RJLCur->Me == VecWalk && RJLCur->Effect == CurEffect)
									{
									Found = 1;
									break;
									} // if
								RJLCur = (RenderJoeList *)RJLCur->Next;
								} // while
							if (! Found)
								{
								if (*SearchRJLPtr = new RenderJoeList())
									{
									(*SearchRJLPtr)->Me = VecWalk;
									(*SearchRJLPtr)->Effect = CurEffect;
									(*SearchRJLPtr)->Priority = TempPriority;
									(*SearchRJLPtr)->EvalOrder = TempEvalOrder;
									} // if
								else
									{
									Success = 0;
									break;
									} // else
								SearchRJLPtr = (RenderJoeList **)&(*SearchRJLPtr)->Next;
								} // if
							} // if
						} // if
					} // for
				} // if
			#endif // WCS_SEARCH_QUERY
			#endif // WCS_BUILD_VNS
			} // if
		CurEffect = CurEffect->Next;
		} // while
	} // if

*RJLPtr = SearchRJL;	// link the two lists together
return (RJL);

} // Database::CreateRenderJoeList

/*===========================================================================*/

// ignores items that have preview disabled
GeneralEffect *Database::GetNextPointEffect(long EffectType, Joe *&Current, VectorPoint *&Point, double &Elev, double &Lat, double &Lon)
{
int ResetNULL;
GeneralEffect *Effect;
Joe *VecWalk;
JoeAttribute *MyAttrib;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM MyVert;

ResetGeoClip();

if (! (VecWalk = Current))
	{
	VecWalk = GetFirst();
	Point = NULL;
	} // if

for (; Current = VecWalk; VecWalk = GetNext(VecWalk))
	{
	if (VecWalk->TestFlags(WCS_JOEFLAG_HASKIDS) || ! VecWalk->TestFlags(WCS_JOEFLAG_ACTIVATED))
		{
		continue;
		} // if 
	if ((MyAttrib = VecWalk->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, (unsigned char)EffectType)) && VecWalk->GetNumRealPoints() > 0)
		{
		ResetNULL = 0;
		if (! Point)
			{
			Point = VecWalk->GetFirstRealPoint();
			ResetNULL = 1;
			}
		else
			Point = Point->Next;
		if (Point)
			{
			if (EffectType == WCS_JOE_ATTRIB_INTERNAL_OBJECT3D)
				{
				Effect = ((JoeObject3D *)MyAttrib)->Object;
				if (! ((Object3DEffect *)Effect)->DrawEnabled)
					{
					if (ResetNULL)
						Point = NULL;
					continue;
					} // if
				} // if
			else if (EffectType == WCS_JOE_ATTRIB_INTERNAL_FOLIAGE)
				{
				Effect = ((JoeFoliage *)MyAttrib)->Foliage;
				if (! ((FoliageEffect *)Effect)->PreviewEnabled)
					{
					if (ResetNULL)
						Point = NULL;
					continue;
					} // if
				} // else if
			else	// unknown effect type cannot be cast
				return (NULL);
			if (Effect->Enabled)
				{
				if (MyAttr = (JoeCoordSys *)VecWalk->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
					MyCoords = MyAttr->Coord;
				else
					MyCoords = NULL;
				if (Point->ProjToDefDeg(MyCoords, &MyVert))
					{
					Lat = MyVert.Lat;
					Lon = MyVert.Lon;
					Elev = MyVert.Elev;
					return (Effect);
					} // if
				} // if
			Point = NULL;	// go on to next vector
			} // if
		} // if
	} // for

return (NULL);

} // Database::GetNextPointEffect

/*===========================================================================*/

double Database::TerrainWidth(void)
{
double MaxLon = -10000.0, MinLon = 10000.0;
Joe *Walk;

ResetGeoClip();

for (Walk = GetFirst(); Walk; Walk = GetNext(Walk))
 	{
	if (Walk->TestFlags(WCS_JOEFLAG_ISDEM) && Walk->TestFlags(WCS_JOEFLAG_ACTIVATED))
		{
		if (Walk->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
			{
			if (Walk->NWLon > MaxLon)
				MaxLon = Walk->NWLon;
			if (Walk->SELon < MinLon)
				MinLon = Walk->SELon;
			} // if
		} // if
	} // for

if (MaxLon == -10000.0)
	return (0.0);
if (MaxLon > MinLon)
	return (MaxLon - MinLon);
return (MinLon - MaxLon);

} // Database::TerrainWidth

/*===========================================================================*/

Joe *Database::SetFirstActive(void)
{
Joe *Iterate;

for (Iterate = GetFirst(); Iterate; Iterate = GetNext(Iterate))
	{
	if (! Iterate->TestFlags(WCS_JOEFLAG_HASKIDS)) // Ignore Group objects
		{
		return (SetActiveObj(Iterate));
		} // if no kids
	} // for

return (NULL);

} // Database::SetFirstActive

/*===========================================================================*/

int Database::GetDEMElevRange(float &MaxEl, float &MinEl)
{
Joe *Iterate;
JoeDEM *MyDEM;
int Found = 0;

MaxEl = -1000000.0f;
MinEl = 1000000.0f;
for (Iterate = GetFirst(); Iterate; Iterate = GetNext(Iterate))
	{
	if (! Iterate->TestFlags(WCS_JOEFLAG_HASKIDS)) // Ignore Group objects
		{
		if (Iterate->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			if (Iterate->TestFlags(WCS_JOEFLAG_ACTIVATED))
				{
				if (MyDEM = (JoeDEM *)Iterate->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
					{
					if (MyDEM->MaxEl > MaxEl)
						MaxEl = MyDEM->MaxEl;
					if (MyDEM->MinEl < MinEl)
						MinEl = MyDEM->MinEl;
					Found = 1;
					} // if
				} // if
			} // if
		} // if no kids
	} // for

if (! Found)
	{
	MaxEl = MinEl = 0.0f;
	} // if

return (Found);

} // Database::GetDEMElevRange

/*===========================================================================*/

int Database::GetDEMExtents(double &MaxLat, double &MinLat, double &MaxLon, double &MinLon)
{
Joe *Iterate;
int Found = 0;

MaxLat = MaxLon = -DBL_MAX;
MinLat = MinLon = DBL_MAX;
for (Iterate = GetFirst(); Iterate; Iterate = GetNext(Iterate))
	{
	if (! Iterate->TestFlags(WCS_JOEFLAG_HASKIDS)) // Ignore Group objects
		{
		if (Iterate->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			if (Iterate->TestFlags(WCS_JOEFLAG_ACTIVATED))
				{
				if (Iterate->NWLat > -FLT_MAX && Iterate->NWLon > -FLT_MAX && 
					Iterate->SELat < FLT_MAX && Iterate->SELon < FLT_MAX)
					{
					if (Iterate->NWLat > MaxLat)
						MaxLat = Iterate->NWLat;
					if (Iterate->SELat < MinLat)
						MinLat = Iterate->SELat;
					if (Iterate->NWLon > MaxLon)
						MaxLon = Iterate->NWLon;
					if (Iterate->SELon < MinLon)
						MinLon = Iterate->SELon;
					Found = 1;
					} // if
				} // if
			} // if
		} // if no kids
	} // for

if (! Found)
	{
	MaxLat = MinLat = 0.0;
	MaxLon = MinLon = 0.0;
	} // if

return (Found);

} // Database::GetDEMExtents

/*===========================================================================*/

unsigned long Database::GetMinDEMCellSizeMetersMaxCount(double &CellSizeNS, double &CellSizeWE, unsigned long MaxCount, Project *CurProj)
{
double NSDimension, WEDimension;
Joe *Iterate;
unsigned long Found = 0, TotalDEMs = 0, SkipRate = 1, SkipCount;
DEM Topo;

CellSizeNS = CellSizeWE = 1000000.0;

if (MaxCount)
	{
	for (Iterate = GetFirst(); Iterate; Iterate = GetNext(Iterate))
		{
		if (! Iterate->TestFlags(WCS_JOEFLAG_HASKIDS)) // Ignore Group objects
			{
			if (Iterate->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				TotalDEMs++;
				} // if
			} // if no kids
		} // for
	SkipRate = (int)((double)TotalDEMs / (double)MaxCount);
	} // if

if (SkipRate < 1) SkipRate = 1;

for (SkipCount = 0, Iterate = GetFirst(); Iterate; Iterate = GetNext(Iterate))
	{
	if (! Iterate->TestFlags(WCS_JOEFLAG_HASKIDS)) // Ignore Group objects
		{
		if (Iterate->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			if (SkipCount == 0)
				{
				if (Topo.AttemptLoadDEM(Iterate, 0, CurProj))
					{
					if (Topo.GetDEMCellSizeMeters(NSDimension, WEDimension))
						{
						if (WEDimension < CellSizeWE)
							CellSizeWE = WEDimension;
						if (NSDimension < CellSizeNS)
							CellSizeNS = NSDimension;
						Found++;
						} // if
					} // if
				} // if
			SkipCount++;
			if (SkipCount == SkipRate)
				{
				SkipCount = 0;
				} // else if
			} // if
		} // if no kids
	} // for

if (! Found)
	{
	CellSizeNS = CellSizeWE = 0.0;
	} // if

return (Found);

} // Database::GetMinDEMCellSizeMetersMaxCount

/*===========================================================================*/

int Database::GetMinDEMCellSizeMeters(double &CellSizeNS, double &CellSizeWE, Project *CurProj)
{
double NSDimension, WEDimension;
Joe *Iterate;
DEM Topo;
int Found = 0;

CellSizeNS = CellSizeWE = 1000000.0;

for (Iterate = GetFirst(); Iterate; Iterate = GetNext(Iterate))
	{
	if (! Iterate->TestFlags(WCS_JOEFLAG_HASKIDS)) // Ignore Group objects
		{
		if (Iterate->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			if (Topo.AttemptLoadDEM(Iterate, 0, CurProj))
				{
				if (Topo.GetDEMCellSizeMeters(NSDimension, WEDimension))
					{
					if (WEDimension < CellSizeWE)
						CellSizeWE = WEDimension;
					if (NSDimension < CellSizeNS)
						CellSizeNS = NSDimension;
					Found = 1;
					} // if
				} // if
			} // if
		} // if no kids
	} // for

if (! Found)
	{
	CellSizeNS = CellSizeWE = 0.0;
	} // if

return (Found);

} // Database::GetMinDEMCellSizeMeters

/*===========================================================================*/

int Database::FillDEMBounds(DEMBounds *MyBounds)
{
float MaxEl, MinEl;

if (GetDEMExtents(MyBounds->North, MyBounds->South, MyBounds->West, MyBounds->East) && GetDEMElevRange(MaxEl, MinEl))
	{
	MyBounds->HighElev = MaxEl;
	MyBounds->LowElev = MinEl;
	return (1);
	} // if

MyBounds->North = MyBounds->South = MyBounds->West = MyBounds->East = MyBounds->HighElev = MyBounds->LowElev = 0.0;

return (0);

} // Database::FillDEMBounds

/*===========================================================================*/

long Database::MatchCoordSys(CoordSys *TestCoords, int DEMOnly)
{
Joe *Iterate;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
long Found = 0;

ResetGeoClip();

for (Iterate = GetFirst(); Iterate; Iterate = GetNext(Iterate))
	{
	if (! Iterate->TestFlags(WCS_JOEFLAG_HASKIDS)) // Ignore Group objects
		{
		if (! DEMOnly || Iterate->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			if (MyAttr = (JoeCoordSys *)Iterate->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
				MyCoords = MyAttr->Coord;
			else
				MyCoords = NULL;
			if (MyCoords != TestCoords)
				return (0);
			Found ++;
			} // if
		} // if
	} // for

return (Found);

} // Database::MatchCoordSys

/*===========================================================================*/

// pays no attention to enabled state
long Database::GetBounds(double &NewNorth, double &NewSouth, double &NewWest, double &NewEast, int DEMOnly)
{
double Bounds;
Joe *Iterate;
int Found = 0;

NewNorth = -FLT_MAX;
NewWest = -FLT_MAX;
NewSouth = FLT_MAX;
NewEast = FLT_MAX;

ResetGeoClip();

for (Iterate = GetFirst(); Iterate; Iterate = GetNext(Iterate))
	{
	if (! Iterate->TestFlags(WCS_JOEFLAG_HASKIDS)) // Ignore Group objects
		{
		if (! DEMOnly || Iterate->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			Found ++;
			if ((Bounds = Iterate->GetNorth()) > NewNorth)
				NewNorth = Bounds;
			if ((Bounds = Iterate->GetSouth()) < NewSouth)
				NewSouth = Bounds;
			if ((Bounds = Iterate->GetWest()) > NewWest)
				NewWest = Bounds;
			if ((Bounds = Iterate->GetEast()) < NewEast)
				NewEast = Bounds;
			} // if
		} // if
	} // for

if (! Found)
	{
	NewNorth = NewSouth = NewWest = NewEast = 0.0;
	} // if

return (Found);

} // Database::GetBounds

/*===========================================================================*/

long Database::GetNativeBounds(double &NewNorth, double &NewSouth, double &NewWest, double &NewEast, int DEMOnly, Project *CurProj)
{
double CurNorth, CurSouth, CurWest, CurEast;
Joe *Iterate;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
int Found = 0;
DEM Topo;

NewNorth = -FLT_MAX;
NewSouth = FLT_MAX;

ResetGeoClip();

for (Iterate = GetFirst(); Iterate; Iterate = GetNext(Iterate))
	{
	if (! Iterate->TestFlags(WCS_JOEFLAG_HASKIDS)) // Ignore Group objects
		{
		if (! Iterate->TestFlags(WCS_JOEFLAG_HASKIDS))
			{
			if (! DEMOnly || Iterate->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				if (Found == 0)
					{
					if (MyAttr = (JoeCoordSys *)Iterate->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
						MyCoords = MyAttr->Coord;
					else
						MyCoords = NULL;
					if ((! MyCoords) || MyCoords->GetGeographic())
						{
						NewWest = -FLT_MAX;
						NewEast = FLT_MAX;
						} // if
					else
						{
						NewWest = FLT_MAX;
						NewEast = -FLT_MAX;
						} // else
					} // if
				Found ++;
				// load DEM if necessary
				if (Iterate->TestFlags(WCS_JOEFLAG_ISDEM))
					{
					if (Topo.AttemptLoadDEM(Iterate, 1, CurProj))
						{
						CurNorth = Topo.Northest();
						CurSouth = Topo.Southest();
						CurWest = Topo.Westest();
						CurEast = Topo.Eastest();
						Topo.FreeRawElevs();
						} // if
					} // if DEM
				// otherwise fetch vector bounds from vertices
				else
					Iterate->GetBoundsProjected(MyCoords, CurNorth, CurWest, CurSouth, CurEast, false);

				if (CurNorth > NewNorth)
					NewNorth = CurNorth;
				if (CurSouth < NewSouth)
					NewSouth = CurSouth;
				// if no CS or geographic
				if ((! MyCoords) || MyCoords->GetGeographic())
					{
					if (CurWest > NewWest)
						NewWest = CurWest;
					if (CurEast < NewEast)
						NewEast = CurEast;
					} // if
				else
					{
					if (CurWest < NewWest)
						NewWest = CurWest;
					if (CurEast > NewEast)
						NewEast = CurEast;
					} // else
				} // if
			} // if
		} // if
	} // for j

if (! Found)
	{
	NewNorth = NewSouth = NewWest = NewEast = 0.0;
	} // if

return (Found);

} // Database::GetNativeBounds

/*===========================================================================*/

void Database::ApplicationSetTime(double Time, long Frame, double FrameFraction)
{

} // Database::ApplicationSetTime

/*===========================================================================*/

int Database::RemoveRAHost(RasterAnimHost *RemoveMe, Project *CurProj, EffectsLib *Effects, int DoRemoveAll, int SuppressNitnoidRequest)
{
Joe *Current;
LayerEntry *Entry, *NextEntry = NULL;
LayerStub *CurStub, *NextStub;
int RemoveFiles = 1, GoAhead = 0;
NotifyTag Changes[2];
RasterAnimHostProperties Prop;
char QueryStr[256];

ResetGeoClip();

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
RemoveMe->GetRAHostProperties(&Prop);

sprintf(QueryStr, "Remove %s %s from the Project?", Prop.Name, Prop.Type);

if (SuppressNitnoidRequest)
	GoAhead = 1;
else
	GoAhead = UserMessageOKCAN(Prop.Name, QueryStr);

if (GoAhead)
	{
	for (Current = GetFirst(); Current; Current = GetNext(Current))
		{
		if (Current == (Joe *)RemoveMe)
			{
			if (! Current->TestFlags(WCS_JOEFLAG_HASKIDS))
				{
				if (Current->TestFlags(WCS_JOEFLAG_ISDEM))
					{
					if (DoRemoveAll)
						RemoveFiles = 2;
					else if ((RemoveFiles = UserMessageYNCAN("Database: Remove DEM",
						"Delete DEM elevation files from disk\nas well as remove DEM from the Database?", 1)) == NULL)
						return (0);
					} // if
				if (Current->RemoveMe(Effects))
					{
					if (RemoveFiles == 2)
						{
						AttemptDeleteDEMFiles((char *)Current->FileName(), CurProj);
						} // if
					Changes[0] = MAKE_ID(RemoveMe->GetNotifyClass(), RemoveMe->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, NULL);
					delete Current;
					return (1);
					} // if
				} // if
			} // if
		} // for

	for (Entry = DBLayers.FirstEntry(); Entry; Entry = Entry ? DBLayers.NextEntry(Entry): NextEntry)
		{
		if (Entry == (LayerEntry *)RemoveMe)
			{
			CurStub = Entry->FirstStub();
			while (CurStub)
				{
				NextStub = CurStub->NextObjectInLayer();
				if (CurStub->MyObject())
					{
					CurStub->MyObject()->RemoveObjectFromLayer(FALSE, Entry);
					} // if
				CurStub = NextStub;
				} // for
			NextEntry = DBLayers.NextEntry(Entry);
			if (DBLayers.RemoveLayer(Entry))
				{
				// send general notify so that SAG completely rebuilds
				Changes[0] = MAKE_ID(0xff, Entry->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
				Changes[1] = NULL;
				delete Entry;
				Entry = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, NULL);
				} // if
			} // if
		} // for
	} // if

return (0);

} // Database::RemoveRAHost

/*===========================================================================*/

Joe *Database::ValidateJoe(RasterAnimHost *JoeRAH)
{
Joe *Result = NULL;

Result = InternalValidateJoe(JoeRAH, WCS_DATABASE_STATIC);

if (!Result)
	{
	Result = InternalValidateJoe(JoeRAH, WCS_DATABASE_DYNAMIC);
	} // if
if (!Result)
	{
	Result = InternalValidateJoe(JoeRAH, WCS_DATABASE_LOG);
	} // if

return(Result);

} // Database::ValidateJoe

/*===========================================================================*/

Joe *Database::InternalValidateJoe(RasterAnimHost *JoeRAH, unsigned long Flags)
{
Joe *MatchScan, *MatchMe, *Matched = NULL;

// MatchMe may not really be a Joe, so never look at it directly
MatchMe = (Joe *)JoeRAH;

ResetGeoClip();

for (MatchScan = GetFirst(Flags); MatchScan; MatchScan = GetNext(MatchScan))
	{
	if (MatchScan == MatchMe)
		{
		Matched = MatchMe;
		break;
		} // if
	} // if

return(Matched);

} // Database::InternalValidateJoe

/*===========================================================================*/

// Stuff moved here from DatOpsPopGUI upon the demise of that file.

int Database::SetJoeProperties(DBFilterEvent *DBFilter, JoeProperties *JoeProp)
{
DBFilterEvent *CurFilter;
Joe *CurJoe;
int Approved, PropSet = 0;

ResetGeoClip();

for (CurJoe = GetFirst(); CurJoe; CurJoe = GetNext(CurJoe))
	{
	Approved = 0;
	CurFilter = DBFilter;
	while (CurFilter)
		{
		if (! Approved && CurFilter->EventType == WCS_DBFILTER_EVENTTYPE_ADD)
			Approved = (CurFilter->PassJoe(CurJoe) > 0);
		else if (Approved && CurFilter->EventType == WCS_DBFILTER_EVENTTYPE_SUB)
			Approved = (CurFilter->PassJoe(CurJoe) >= 0);
		CurFilter = CurFilter->Next;
		} // while
	if (Approved)
		{
		if (CurJoe->SetProperties(JoeProp))
			PropSet = 1;
		} // if
	} // for

return (PropSet);

} // Database::SetJoeProperties

/*===========================================================================*/

JoeProperties::JoeProperties()
{

Mask = 0;
Effect = NULL;
EffectType = 0;
LineStyle = LineWeight = Red = Green = Blue = Enabled = DrawEnabled = RenderEnabled = Selected = 0;

} // JoeProperties::JoeProperties

/*===========================================================================*/

VectorPoint *Database::NearestControlPointSearch(double Lat, double Lon, double PlanetRad)
{
double NewDist, FoundDist = DBL_MAX, NewClip[4], PtLatScale, PtLonScale;
VectorPoint *PtLink, *FoundPt = NULL;
Joe *CurJoe;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM MyVert;
int SetClip;

ResetGeoClip();

PtLatScale = 1.0 / LatScale(PlanetRad);
PtLonScale = LonScale(PlanetRad, Lat);
PtLonScale = (PtLonScale <= 0.0) ? 1.0: 1.0 / PtLonScale;

for (CurJoe = GetFirst(); CurJoe; CurJoe = GetNext(CurJoe))
	{
	if (PtLink = CurJoe->GetFirstRealPoint())
		{
		SetClip = 0;
		if (MyAttr = (JoeCoordSys *)CurJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
			MyCoords = MyAttr->Coord;
		else
			MyCoords = NULL;
		for (; PtLink; PtLink = PtLink->Next)
			{
			if (PtLink->ProjToDefDeg(MyCoords, &MyVert))
				{
				if ((NewDist = FindDistance(Lat, Lon, MyVert.Lat, MyVert.Lon, PlanetRad)) < FoundDist)
					{
					FoundDist = NewDist;
					FoundPt = PtLink;
					SetClip = 1;
					} // if
				} // if
			} // for
		if (SetClip)
			{
			NewClip[0] = Lat + FoundDist * PtLatScale;	// north
			NewClip[1] = Lat - FoundDist * PtLatScale;	// south
			NewClip[2] = Lon - FoundDist * PtLonScale;	// east
			NewClip[3] = Lon + FoundDist * PtLonScale;	// west
			SetGeoClip(NewClip[0], NewClip[1], NewClip[2], NewClip[3]);
			} // if
		} // for
	} // for

return (FoundPt);

} // Database::NearestControlPointSearch

/*===========================================================================*/

Joe *Database::GetNextByQuery(SearchQuery *Search, Joe *Current)
{
Joe *NextJoe;

for (NextJoe = Current ? GetNext(Current): GetFirst(); NextJoe; NextJoe = GetNext(NextJoe))
	{
	if (Search->ApproveJoe(NextJoe))
		break;
	} // for

return (NextJoe);

} // Database::GetNextByQuery

/*===========================================================================*/

int Database::AreThereVectors(void)
{
Joe *CurJoe;

for (CurJoe = GetFirst(); CurJoe; CurJoe = GetNext(CurJoe))
	{
	if (! CurJoe->TestFlags(WCS_JOEFLAG_HASKIDS) && CurJoe->GetRAHostTypeNumber() == WCS_RAHOST_OBJTYPE_VECTOR)
		return (1);
	} // for

return (0);

} // Database::AreThereVectors

/*===========================================================================*/

void Database::UpdateProjectedJoeBounds(void)
{
Joe *CurJoe;

ResetGeoClip();

for (CurJoe = GetFirst(); CurJoe; CurJoe = GetNext(CurJoe))
	{
	if (CurJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS))
		{
		CurJoe->ZeroUpTree();
		} // if
	} // for
for (CurJoe = GetFirst(); CurJoe; CurJoe = GetNext(CurJoe))
	{
	if (CurJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS))
		{
		CurJoe->RecheckBounds();
		BoundUpTree(CurJoe);
		} // if
	} // for

// <<<>>> send notification that vectors have changed

} // Database::UpdateProjectedJoeBounds

/*===========================================================================*/

void Database::RemoveUnusedLayers(void)
{

DBLayers.RemoveUnusedLayers();

} // Database::RemoveUnusedLayers

/*===========================================================================*/

// for use by VNS2's Scenario loader and saver
unsigned long Database::EnumerateUniqueLoadSaveIDs(int ClearOnly)
{
unsigned long NumEnumerated = 0;
#ifdef WCS_BUILD_VNS
Joe *EnumJoe;

ResetGeoClip();

for (EnumJoe = GetFirst(); EnumJoe; EnumJoe = GetNext(EnumJoe))
	{
	// we can assign IDs to Group Joes as well, though they won't be used by scenarios, it simplifies the walk at the
	// expense of bloating the loader lookup table by a bit.
	EnumJoe->UniqueLoadSaveID = ++NumEnumerated; // this may be the first time in about a decade I've used the preincrement notation
	} // for
#endif // WCS_BUILD_VNS

return(NumEnumerated);

} // Database::EnumerateUniqueLoadSaveIDs

/*===========================================================================*/

unsigned long Database::CountHighestUniqueID(void)
{
unsigned long HighestID = 0;
#ifdef WCS_BUILD_VNS
Joe *EnumJoe;

ResetGeoClip();

for (EnumJoe = GetFirst(); EnumJoe; EnumJoe = GetNext(EnumJoe))
	{
	// should we ignore groupjoe IDs to try to compact the table a teeny bit? Probably won't help.
	if (EnumJoe->UniqueLoadSaveID > HighestID)
		{
		HighestID = EnumJoe->UniqueLoadSaveID;
		} // if
	} // for
#endif // WCS_BUILD_VNS

return(HighestID);

} // Database::CountHighestUniqueID

/*===========================================================================*/

Joe **Database::PopulateUniqueIDTable(unsigned long HighestID)
{
#ifdef WCS_BUILD_VNS
Joe **UniqueTable = NULL;

if (HighestID > 0)
	{
	// table entries will be cleared to 0 on allocation
	if (UniqueTable = new Joe * [HighestID + 1])
		{
		Joe *EnumJoe;

		ResetGeoClip();
		for (EnumJoe = GetFirst(); EnumJoe; EnumJoe = GetNext(EnumJoe))
			{
			// slot 0 always unused
			if (EnumJoe->UniqueLoadSaveID <= HighestID)
				{
				UniqueTable[EnumJoe->UniqueLoadSaveID] = EnumJoe;
				} // if
			} // for
		return(UniqueTable);
		} // if
	} // if
#endif // WCS_BUILD_VNS

return(NULL);

} // Database::PopulateUniqueIDTable

/*===========================================================================*/

void Database::FreeUniqueIDTable(Joe **UniqueTable)
{

#ifdef WCS_BUILD_VNS
if (UniqueTable)
	{
	delete [] UniqueTable;
	} // if
#endif // WCS_BUILD_VNS

} // Database::FreeUniqueIDTable
