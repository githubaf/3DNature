// DEMMerger.cpp
// For managing DEMMerger Effects
// Built from scratch on 9/24/02 by Gary R. Huber
// Copyright 2002 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "EffectsLib.h"
#include "Application.h"
#include "Conservatory.h"
#include "Joe.h"
#include "DEM.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Raster.h"
#include "requester.h"
#include "DEMMergeGUI.h"
#include "AppMem.h"
#include "Render.h"
#include "Database.h"
#include "Project.h"
#include "Security.h"
#include "FeatureConfig.h"
#include "Lists.h"

static double epsilon = 0.0000001;	// damn floating point
//static char mergedbg[256];

DEMMerger::DEMMerger()
: GeneralEffect(NULL), NormalBounds(this), HiResBounds(this)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_DEMMERGER;
SetDefaults();

} // DEMMerger::DEMMerger

/*===========================================================================*/

DEMMerger::DEMMerger(RasterAnimHost *RAHost)
: GeneralEffect(RAHost), NormalBounds(this), HiResBounds(this)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_DEMMERGER;
SetDefaults();

} // DEMMerger::DEMMerger

/*===========================================================================*/

DEMMerger::DEMMerger(RasterAnimHost *RAHost, EffectsLib *Library, DEMMerger *Proto)
: GeneralEffect(RAHost), NormalBounds(this), HiResBounds(this)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_DEMMERGER;
Prev = Library->LastMerger;
if (Library->LastMerger)
	{
	Library->LastMerger->Next = this;
	Library->LastMerger = this;
	} // if
else
	{
	Library->Merger = Library->LastMerger = this;
	} // else
Name[0] = NULL;
SetDefaults();
if (Proto)
	{
	Copy(this, Proto);
	Name[0] = NULL;
	strcpy(NameBase, Proto->Name);
	} // if
else
	{
	strcpy(NameBase, "DEM Merger");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // DEMMerger::DEMMerger

/*===========================================================================*/

DEMMerger::~DEMMerger()
{
EffectList *CurQuery;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->MRG && GlobalApp->GUIWins->MRG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->MRG;
		GlobalApp->GUIWins->MRG = NULL;
		} // if
	} // if

while (Queries)
	{
	CurQuery = Queries;
	Queries = Queries->Next;
	delete CurQuery;
	} // while

MergeCoordSys = NULL;
if (BCS_CoordSys)
	delete BCS_CoordSys;

} // DEMMerger::~DEMMerger

/*===========================================================================*/

void DEMMerger::SetDefaults(void)
{
double EffectDefaultG[WCS_EFFECTS_DEMMERGER_NUMANIMPAR] = {1/3600.0, 1/3600.0, 2.0};	// 1 arc-second
double EffectDefaultP[WCS_EFFECTS_DEMMERGER_NUMANIMPAR] = {30.0, 30.0, 2.0};			// 30m
double RangeDefaults[WCS_EFFECTS_DEMMERGER_NUMANIMPAR][3] = {FLT_MAX, 0.0001, .1,	// MERGEXRES
															FLT_MAX, 0.0001, .1,	// MERGEYRES
															255.0, 2.0, 1.0};		// DIVIDER
PlanetOpt *PO;
long Ct;

MergeCoordSys = NULL;
BCS_CoordSys = new CoordSys;
if (PO = (PlanetOpt *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_PLANETOPT))
	MergeCoordSys = PO->Coords;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (MergeCoordSys && MergeCoordSys->Method.GCTPMethod != 0)	// Projected?
		AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefaultP[Ct]);
	else
		AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefaultG[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for

HiResMergeXMax = HiResMergeYMax = -FLT_MIN;
HiResMergeXMin = HiResMergeYMin = FLT_MAX;
MergeXMax = MergeYMax = -FLT_MIN;
MergeXMin = MergeYMin = FLT_MAX;
XCellSize = XOrigin = YCellSize = YOrigin = 0.0;
NullVal = 0.0f;
MergeWidth = MergeHeight = HighResMergeWidth = HighResMergeHeight = 0;
if (GlobalApp && GlobalApp->MainProj && GlobalApp->MainProj->ProjectLoaded)
	{
	strcpy(DEMPath, GlobalApp->MainProj->dirname);
	strcpy(HiResPath, GlobalApp->MainProj->dirname);
	} // if
else
	{
	DEMPath[0] = 0;
	HiResPath[0] = 0;
	} // else
strcpy(DEMName, "Merged");
strcpy(HiResName, "HiResMerge");
GoodBounds = MultiRes = 0;
Queries = NULL;

if (MergeCoordSys && MergeCoordSys->Method.GCTPMethod != 0)	// Projected?
	{
	AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEXRES].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEYRES].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	} // if
else
	{
	AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEXRES].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
	AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEYRES].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
	} // else

} // DEMMerger::SetDefaults

/*===========================================================================*/

void DEMMerger::Copy(DEMMerger *CopyTo, DEMMerger *CopyFrom)
{
EffectList *CurQuery;
#ifdef WCS_BUILD_VNS
long Result = -1;
EffectList **ToQuery;
NotifyTag Changes[2];
#endif // WCS_BUILD_VNS

CopyTo->MergeCoordSys = NULL;

while (CopyTo->Queries)
	{
	CurQuery = CopyTo->Queries;
	CopyTo->Queries = CopyTo->Queries->Next;
	delete CurQuery;
	} // if
#ifdef WCS_BUILD_VNS
CurQuery = CopyFrom->Queries;
ToQuery = &CopyTo->Queries;
while (CurQuery)
	{
	if (CurQuery->Me)
		{
		if (*ToQuery = new EffectList())
			{
			if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
				{
				(*ToQuery)->Me = GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CurQuery->Me);
				} // if no need to make another copy, its all in the family
			else
				{
				if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(CurQuery->Me->EffectType, CurQuery->Me->Name))
					{
					Result = UserMessageCustom("Copy DEM Merger", "How do you wish to resolve Search Query name collisions?\n\nLink to existing Search Queries, replace existing Search Queries, or create new Search Queries?",
						"Link", "Create", "Overwrite", 1);
					} // if
				if (Result <= 0)
					{
					(*ToQuery)->Me = GlobalApp->CopyToEffectsLib->AddEffect(CurQuery->Me->EffectType, NULL, CurQuery->Me);
					} // if create new
				else if (Result == 1)
					{
					(*ToQuery)->Me = GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CurQuery->Me);
					} // if link to existing
				else if ((*ToQuery)->Me = GlobalApp->CopyToEffectsLib->FindByName(CurQuery->Me->EffectType, CurQuery->Me->Name))
					{
					((SearchQuery *)(*ToQuery)->Me)->Copy((SearchQuery *)(*ToQuery)->Me, (SearchQuery *)CurQuery->Me);
					Changes[0] = MAKE_ID((*ToQuery)->Me->GetNotifyClass(), (*ToQuery)->Me->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, (*ToQuery)->Me);
					} // else if found and overwrite
				else
					{
					(*ToQuery)->Me = GlobalApp->CopyToEffectsLib->AddEffect(CurQuery->Me->EffectType, NULL, CurQuery->Me);
					} // else
				} // else better copy or overwrite it since its important to get just the right search query
			if ((*ToQuery)->Me)
				ToQuery = &(*ToQuery)->Next;
			else
				{
				delete *ToQuery;
				*ToQuery = NULL;
				} // if
			} // if
		} // if
	CurQuery = CurQuery->Next;
	} // while

if (CopyFrom->MergeCoordSys)
	{
	if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
		{
		CopyTo->MergeCoordSys = (CoordSys *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->MergeCoordSys);
		} // if no need to make another copy, its all in the family
	else
		{
		if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->MergeCoordSys->EffectType, CopyFrom->MergeCoordSys->Name))
			{
			Result = UserMessageCustom("Copy DEM Merger", "How do you wish to resolve Coordinate System name collisions?\n\nLink to existing Coordinate Systems, replace existing Systems, or create new Systems?",
				"Link", "Create", "Overwrite", 1);
			} // if
		if (Result <= 0)
			{
			CopyTo->MergeCoordSys = (CoordSys *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->MergeCoordSys->EffectType, NULL, CopyFrom->MergeCoordSys);
			} // if create new
		else if (Result == 1)
			{
			CopyTo->MergeCoordSys = (CoordSys *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->MergeCoordSys);
			} // if link to existing
		else if (CopyTo->MergeCoordSys = (CoordSys *)GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->MergeCoordSys->EffectType, CopyFrom->MergeCoordSys->Name))
			{
			CopyTo->MergeCoordSys->Copy(CopyTo->MergeCoordSys, CopyFrom->MergeCoordSys);
			Changes[0] = MAKE_ID(CopyTo->MergeCoordSys->GetNotifyClass(), CopyTo->MergeCoordSys->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, CopyTo->MergeCoordSys);
			} // else if found and overwrite
		else
			{
			CopyTo->MergeCoordSys = (CoordSys *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->MergeCoordSys->EffectType, NULL, CopyFrom->MergeCoordSys);
			} // else
		} // else better copy or overwrite it since its important to get just the right object
	} // if
#endif // WCS_BUILD_VNS

CopyTo->MergeXMax = CopyFrom->MergeXMax;
CopyTo->MergeXMin = CopyFrom->MergeXMin;
CopyTo->MergeYMax = CopyFrom->MergeYMax;
CopyTo->MergeYMin = CopyFrom->MergeYMin;
CopyTo->HiResMergeXMax = CopyFrom->HiResMergeXMax;
CopyTo->HiResMergeXMin = CopyFrom->HiResMergeXMin;
CopyTo->HiResMergeYMax = CopyFrom->HiResMergeYMax;
CopyTo->HiResMergeYMin = CopyFrom->HiResMergeYMin;
CopyTo->XCellSize = CopyFrom->XCellSize;
CopyTo->XOrigin = CopyFrom->XOrigin;
CopyTo->YCellSize = CopyFrom->YCellSize;
CopyTo->YOrigin = CopyFrom->YOrigin;
CopyTo->NullVal = CopyFrom->NullVal;
CopyTo->MergeWidth = CopyFrom->MergeWidth;
CopyTo->MergeHeight = CopyFrom->MergeHeight;
CopyTo->HighResMergeWidth = CopyFrom->HighResMergeWidth;
CopyTo->HighResMergeHeight = CopyFrom->HighResMergeHeight;
CopyTo->GoodBounds = CopyFrom->GoodBounds;
CopyTo->MultiRes = CopyFrom->MultiRes;
strcpy(CopyTo->DEMName, CopyFrom->DEMName);
strcpy(CopyTo->DEMPath, CopyFrom->DEMPath);
strcpy(CopyTo->HiResName, CopyFrom->HiResName);
strcpy(CopyTo->HiResPath, CopyFrom->HiResPath);
NormalBounds.Copy(&CopyTo->NormalBounds, &CopyFrom->NormalBounds);
HiResBounds.Copy(&CopyTo->HiResBounds, &CopyFrom->HiResBounds);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // DEMMerger::Copy

/*===========================================================================*/

ULONG DEMMerger::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
#ifdef WCS_BUILD_VNS
char MatchName[WCS_EFFECT_MAXNAMELENGTH];
EffectList **CurQuery = &Queries;
#endif // WCS_BUILD_VNS

while (ItemTag != WCS_PARAM_DONE)
	{
	/* read block descriptor tag from file */
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			/* read block size from file */
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch

				switch (ItemTag & 0xffff0000)
					{
					case WCS_EFFECTS_BROWSEDATA:
						{
						if (BrowseInfo)
							BrowseInfo->FreeAll();
						else
							BrowseInfo = new BrowseData();
						if (BrowseInfo)
							BytesRead = BrowseInfo->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_NAME:
						{
						BytesRead = ReadBlock(ffile, (char *)Name, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_DEMMERGER_DEMPATH:
						{
						BytesRead = ReadBlock(ffile, (char *)DEMPath, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_DEMMERGER_DEMNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)DEMName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_DEMMERGER_HIRESPATH:
						{
						BytesRead = ReadBlock(ffile, (char *)HiResPath, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_DEMMERGER_HIRESNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)HiResName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_DEMMERGER_MULTIRES:
						{
						BytesRead = ReadBlock(ffile, (char *)&MultiRes, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_DEMMERGER_MERGEXRES:
						{
						BytesRead = AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEXRES].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_DEMMERGER_MERGEYRES:
						{
						BytesRead = AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEYRES].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_DEMMERGER_DIVIDER:
						{
						BytesRead = AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_DIVIDER].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_DEMMERGER_NORMALBOUNDS:
						{
						BytesRead = NormalBounds.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_DEMMERGER_HIRESBOUNDS:
						{
						BytesRead = HiResBounds.Load(ffile, Size, ByteFlip);
						break;
						}
					#ifdef WCS_BUILD_VNS
					case WCS_EFFECTS_DEMMERGER_QUERYNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							if (*CurQuery = new EffectList())
								{
								if ((*CurQuery)->Me = GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_SEARCHQUERY, MatchName))
									CurQuery = &(*CurQuery)->Next;
								else
									{
									delete *CurQuery;
									*CurQuery = NULL;
									} // else
								} // if
							} // if
						break;
						}
					case WCS_EFFECTS_DEMMERGER_MERGECOORDSYSNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							MergeCoordSys = (CoordSys *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_COORDSYS, MatchName);
							} // if
						break;
						}
					#endif // WCS_BUILD_VNS
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read
			else
				break;
			} // if not done flag
		} // if tag block read
	else
		break;
	} // while

SetGoodBoundsEtc();

return (TotalRead);

} // DEMMerger::Load

/*===========================================================================*/

unsigned long DEMMerger::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long AnimItemTag[WCS_EFFECTS_DEMMERGER_NUMANIMPAR] = {WCS_EFFECTS_DEMMERGER_MERGEXRES, 
																WCS_EFFECTS_DEMMERGER_MERGEYRES,
																WCS_EFFECTS_DEMMERGER_DIVIDER};
EffectList *CurQuery;

if (BrowseInfo)
	{
	ItemTag = WCS_EFFECTS_BROWSEDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = BrowseInfo->Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if browse data saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_NAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Name) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Name)) == NULL)
   goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Enabled)) == NULL)
   goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_DEMMERGER_DEMPATH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(DEMPath) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)DEMPath)) == NULL)
   goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_DEMMERGER_DEMNAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(DEMName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)DEMName)) == NULL)
   goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_DEMMERGER_HIRESPATH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(HiResPath) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)HiResPath)) == NULL)
   goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_DEMMERGER_HIRESNAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(HiResName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)HiResName)) == NULL)
   goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_DEMMERGER_MULTIRES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&MultiRes)) == NULL)
   goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	ItemTag = AnimItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = AnimPar[Ct].Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block
				else
					goto WriteError;
				} //* if anim param saved
			else
				goto WriteError;
			} // if size written
		else
			goto WriteError;
		} // if tag written
	else
		goto WriteError;
	} // for

CurQuery = Queries;
while (CurQuery)
	{
	if (CurQuery->Me)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_DEMMERGER_QUERYNAME, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(CurQuery->Me->GetName()) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)CurQuery->Me->GetName())) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	CurQuery = CurQuery->Next;
	} // while

ItemTag = WCS_EFFECTS_DEMMERGER_NORMALBOUNDS + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = NormalBounds.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if NormalBounds saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_EFFECTS_DEMMERGER_HIRESBOUNDS + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = HiResBounds.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if HiResBounds saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

#ifdef WCS_BUILD_VNS
if (MergeCoordSys)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_DEMMERGER_MERGECOORDSYSNAME, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(MergeCoordSys->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)MergeCoordSys->GetName())) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
#endif // WCS_BUILD_VNS

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
   goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // DEMMerger::Save

/*===========================================================================*/

void DEMMerger::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->MRG)
	{
	delete GlobalApp->GUIWins->MRG;
	} // if
GlobalApp->GUIWins->MRG = new DEMMergeGUI(GlobalApp->AppEffects, GlobalApp->AppDB, GlobalApp->MainProj, GlobalApp->MainProj->Interactive, this);
if(GlobalApp->GUIWins->MRG)
	{
	GlobalApp->GUIWins->MRG->Open(GlobalApp->MainProj);
	} // if

} // DEMMerger::Edit

/*===========================================================================*/

char *DEMMergerCritterNames[WCS_EFFECTS_DEMMERGER_NUMANIMPAR] = {"E-W Resolution (m)", "N-S Resolution (m)",
															"Divider"};

char *DEMMerger::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (DEMMergerCritterNames[Ct]);
	} // for
if (Test == &NormalBounds)
	return ("Bounds");
if (Test == &HiResBounds)
	return ("Hi-Res Bounds");

return ("");

} // DEMMerger::GetCritterName

/*===========================================================================*/

EffectList *DEMMerger::AddQuery(GeneralEffect *AddMe)
{
EffectList **CurQuery = &Queries;
NotifyTag Changes[2];

if (AddMe)
	{
	while (*CurQuery)
		{
		if ((*CurQuery)->Me == AddMe)
			return (NULL);
		CurQuery = &(*CurQuery)->Next;
		} // while
	if (*CurQuery = new EffectList())
		{
		(*CurQuery)->Me = AddMe;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	ScanForRes(GlobalApp->AppDB);
	return (*CurQuery);
	} // if

return (NULL);

} // DEMMerger::AddQuery

/*===========================================================================*/

int DEMMerger::SetCoords(CoordSys *NewCoords)
{
NotifyTag Changes[2];

MergeCoordSys = NewCoords;

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (1);

} // DEMMerger::SetCoords

/*===========================================================================*/

long DEMMerger::InitImageIDs(long &ImageID)
{
long NumImages = 0;
EffectList *CurQuery = Queries;

while (CurQuery)
	{
	if (CurQuery->Me)
		NumImages += CurQuery->Me->InitImageIDs(ImageID);
	CurQuery = CurQuery->Next;
	} // while
if (MergeCoordSys)
	NumImages += MergeCoordSys->InitImageIDs(ImageID);
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // DEMMerger::InitImageIDs

/*===========================================================================*/

int DEMMerger::BuildFileComponentsList(EffectList **QueryList, EffectList **CoordsList)
{
EffectList **ListPtr, *CurQuery = Queries;

while (CurQuery)
	{
	if (CurQuery->Me)
		{
		ListPtr = QueryList;
		while (*ListPtr)
			{
			if ((*ListPtr)->Me == CurQuery->Me)
				break;
			ListPtr = &(*ListPtr)->Next;
			} // if
		if (! (*ListPtr))
			{
			if (*ListPtr = new EffectList())
				(*ListPtr)->Me = CurQuery->Me;
			else
				return (0);
			} // if
		} // if
	CurQuery = CurQuery->Next;
	} // while

if (MergeCoordSys)
	{
	ListPtr = CoordsList;
	while (*ListPtr)
		{
		if ((*ListPtr)->Me == MergeCoordSys)
			break;
		ListPtr = &(*ListPtr)->Next;
		} // if
	if (! (*ListPtr))
		{
		if (*ListPtr = new EffectList())
			(*ListPtr)->Me = MergeCoordSys;
		else
			return (0);
		} // if
	} // if

return (1);

} // DEMMerger::BuildFileComponentsList

/*===========================================================================*/

int DEMMerger::RemoveRAHost(RasterAnimHost *RemoveMe)
{
EffectList *CurQuery = Queries, *PrevQuery = NULL;
NotifyTag Changes[2];

while (CurQuery)
	{
	if (CurQuery->Me == (GeneralEffect *)RemoveMe)
		{
		if (PrevQuery)
			PrevQuery->Next = CurQuery->Next;
		else
			Queries = CurQuery->Next;

		delete CurQuery;

		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

		return (1);
		} // if
	PrevQuery = CurQuery;
	CurQuery = CurQuery->Next;
	} // while

if (MergeCoordSys == (GeneralEffect *)RemoveMe)
	{
	MergeCoordSys = NULL;

	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

	return (1);
	} // if

return (GeneralEffect::RemoveRAHost(RemoveMe));

} // DEMMerger::RemoveRAHost

/*===========================================================================*/

char DEMMerger::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
#ifdef WCS_BUILD_VNS
if (DropType == WCS_EFFECTSSUBCLASS_SEARCHQUERY
	|| DropType == WCS_EFFECTSSUBCLASS_COORDSYS)
	return (1);
#endif // WCS_BUILD_VNS)

return (0);

} // DEMMerger::GetRAHostDropOK

/*===========================================================================*/

int DEMMerger::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
int Success;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (DEMMerger *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (DEMMerger *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
#ifdef WCS_BUILD_VNS
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_SEARCHQUERY)
	{
	Success = -1;
	sprintf(QueryStr, "Add %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		if (AddQuery((GeneralEffect *)DropSource->DropSource))
			{
			Success = 1;
			} // if
		} // if
	} // else if
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_COORDSYS)
	{
	Success = SetCoords((CoordSys *)DropSource->DropSource);
	} // else if
#endif // WCS_BUILD_VNS
else
	{
	Success = GeneralEffect::ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // DEMMerger::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long DEMMerger::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ANIMATED)
	{
	if (GetRAHostAnimated())
		Flags |= WCS_RAHOST_FLAGBIT_ANIMATED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EDITNAME)
	{
	if (! RAParent)
		Flags |= WCS_RAHOST_FLAGBIT_EDITNAME;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_DEMMERGE | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // DEMMerger::GetRAFlags

/*===========================================================================*/

RasterAnimHost *DEMMerger::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;
#ifdef WCS_BUILD_VNS
EffectList *CurQuery;
#endif // WCS_BUILD_VNS

if (! Current)
	return (&NormalBounds);
if (Current == &NormalBounds)
	return (&HiResBounds);
if (Current == &HiResBounds)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Found)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for
#ifdef WCS_BUILD_VNS
CurQuery = Queries;
while (CurQuery)
	{
	if (Found)
		return (CurQuery->Me);
	if (Current == CurQuery->Me)
		Found = 1;
	CurQuery = CurQuery->Next;
	} // while
if (Found && MergeCoordSys)
	return (MergeCoordSys);
if (MergeCoordSys && Current == MergeCoordSys)
	Found = 1;
#endif // WCS_BUILD_VNS

return (NULL);

} // DEMMerger::GetRAHostChild

/*===========================================================================*/

int DEMMerger::GrabAllQueries(void)
{
int Success = 0;
EffectList *CurQuery;
SearchQuery *CurEffect;
NotifyTag Changes[2];

while (CurQuery = Queries)
	{
	Queries = Queries->Next;
	delete CurQuery;
	} // while
Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

if (CurEffect = (SearchQuery *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_SEARCHQUERY))
	{
	while (CurEffect)
		{
		AddQuery(CurEffect);	// sends its own notify
		Success = 1;
		CurEffect = (SearchQuery *)CurEffect->Next;
		} // while
	} // if

return (Success);

} // DEMMerger::GrabAllQueries

/*===========================================================================*/

void DEMMerger::SetGoodBoundsEtc(void)
{
double NBound, SBound, EBound, WBound, HiResN, HiResS, HiResE, HiResW, MergeXRes, MergeYRes, Divider;
CoordSys *TempMergeSys;

NBound = NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue;
SBound = NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
EBound = NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue;
WBound = NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue;
HiResN = HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue;
HiResS = HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
HiResE = HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue;
HiResW = HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue;
MergeXRes = AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEXRES].CurValue;
MergeYRes = AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEYRES].CurValue;
Divider = AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_DIVIDER].CurValue;

GoodBounds = FALSE;

if (MergeCoordSys)
	TempMergeSys = MergeCoordSys;
else
	TempMergeSys = GlobalApp->AppEffects->FetchDefaultCoordSys();
TempMergeSys->GetGeographic();

if (TempMergeSys->Geographic)
	{
	double MergeGeoXRes, MergeGeoYRes;

	MergeGeoXRes = MergeXRes;
	MergeGeoYRes = MergeYRes;
	// round merge bounds off to even multiples of resolution
	NBound = ceil(NBound / MergeGeoYRes) * MergeGeoYRes;	// round up
	SBound = floor(SBound / MergeGeoYRes) * MergeGeoYRes;	// round down
	EBound = ceil(EBound / MergeGeoXRes) * MergeGeoXRes;	// round up
	WBound = floor(WBound / MergeGeoXRes) * MergeGeoXRes;	// round down
	//XOrigin = WBound;
	//YOrigin = SBound;
	MergeHeight = (unsigned long)((NBound - SBound) / MergeGeoYRes + 1 + epsilon);
	MergeWidth = (unsigned long)((WBound - EBound) / MergeGeoXRes + 1 + epsilon);
	// round hires bounds off to even multiples of low resolution
	HiResN = ceil(HiResN / MergeGeoYRes) * MergeGeoYRes;	// round up
	HiResS = floor(HiResS / MergeGeoYRes) * MergeGeoYRes;	// round down
	HiResE = ceil(HiResE / MergeGeoXRes) * MergeGeoXRes;	// round up
	HiResW = floor(HiResW / MergeGeoXRes) * MergeGeoXRes;	// round down
	//HighRes.XOrigin = HiResW;
	//HighRes.YOrigin = HiResS;
	HighResMergeHeight = (unsigned long)((HiResN - HiResS) / (MergeGeoYRes / Divider) + 1);
	HighResMergeWidth = (unsigned long)((HiResW - HiResE) / (MergeGeoXRes / Divider) + 1);
	if ((NBound > SBound) && (WBound > EBound))
		GoodBounds = TRUE;
	} // if
else
	{
	double mod, res2;

	// round merge bounds off to even multiples of resolution
	res2 = MergeYRes * 0.5;
	mod = fmod(NBound + res2, MergeYRes);
	NBound += res2 - mod + epsilon;											// round up
	NBound = (unsigned long)((NBound / MergeYRes) * MergeYRes + epsilon);
	SBound = (unsigned long)((SBound / MergeYRes) * MergeYRes + epsilon);	// round down
	res2 = MergeXRes * 0.5;
	mod = fmod(EBound + res2, MergeXRes);
	EBound += res2 - mod + epsilon;											// round up
	EBound = (unsigned long)((EBound / MergeXRes) * MergeXRes + epsilon);
	WBound = (unsigned long)((WBound / MergeXRes) * MergeXRes + epsilon);	// round down
	XOrigin = WBound;
	YOrigin = SBound;
	MergeHeight = (unsigned long)((NBound - SBound) / MergeYRes + 1.0 + epsilon);
	MergeWidth = (unsigned long)((EBound - WBound) / MergeXRes + 1.0 + epsilon);
	// round hires bounds off to even multiples of low resolution
	res2 = MergeYRes * 0.5;
	mod = fmod(HiResN + res2, MergeYRes);
	HiResN += res2 - mod + epsilon;											// round up
	HiResN = (unsigned long)((HiResN / MergeYRes) * MergeYRes + epsilon);
	HiResS = (unsigned long)((HiResS / MergeYRes) * MergeYRes + epsilon);	// round down
	res2 = MergeXRes * 0.5;
	mod = fmod(EBound + res2, MergeXRes);
	HiResE += res2 - mod + epsilon;											// round up
	HiResE = (unsigned long)((HiResE / MergeXRes) * MergeXRes + epsilon);
	HiResW = (unsigned long)((HiResW / MergeXRes) * MergeXRes + epsilon);	// round down
	//HighRes.XOrigin = HiResW;
	//HighRes.YOrigin = HiResS;
	HighResMergeHeight = (unsigned long)((HiResN - HiResS) / (MergeYRes / Divider) + 1.0 + epsilon);
	HighResMergeWidth = (unsigned long)((HiResE - HiResW) / (MergeXRes / Divider) + 1.0 + epsilon);
	if ((NBound > SBound) && (EBound > WBound))
		GoodBounds = TRUE;
	} // else

} // DEMMerger::SetGoodBoundsEtc

/*===========================================================================*/

// Project 4 corners of DEM into the merge coordinate space & update bounds if needed
void DEMMerger::UpdateMergeBounds(DEM *CheckMe, double &XMin, double &YMin, double &XMax, double &YMax)
{
CoordSys *TempMergeSys, *DEMCoordSys;
JoeCoordSys *MyAttr;
VertexDEM Vert;
long projectMe = true;

if (MergeCoordSys)
	TempMergeSys = MergeCoordSys;
else
	TempMergeSys = GlobalApp->AppEffects->FetchDefaultCoordSys();
TempMergeSys->GetGeographic();

if (CheckMe->MoJoe && (MyAttr = (JoeCoordSys *)CheckMe->MoJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
	{
	DEMCoordSys = MyAttr->Coord;

	if (DEMCoordSys->Equals(TempMergeSys))
		{
		projectMe = false;
		} // if

	Vert.xyz[0] = CheckMe->Westest();
	Vert.xyz[1] = CheckMe->Northest();
	if (projectMe)
		{
		DEMCoordSys->ProjToCart(&Vert);
		TempMergeSys->CartToProj(&Vert);
		} // if
	if (Vert.xyz[0] < XMin)
		XMin = Vert.xyz[0];
	if (Vert.xyz[0] > XMax)
		XMax = Vert.xyz[0];
	if (Vert.xyz[1] < YMin)
		YMin = Vert.xyz[1];
	if (Vert.xyz[1] > YMax)
		YMax = Vert.xyz[1];

	Vert.xyz[0] = CheckMe->Eastest();
	Vert.xyz[1] = CheckMe->Northest();
	if (projectMe)
		{
		DEMCoordSys->ProjToCart(&Vert);
		TempMergeSys->CartToProj(&Vert);
		} // if
	if (Vert.xyz[0] < XMin)
		XMin = Vert.xyz[0];
	if (Vert.xyz[0] > XMax)
		XMax = Vert.xyz[0];
	if (Vert.xyz[1] < YMin)
		YMin = Vert.xyz[1];
	if (Vert.xyz[1] > YMax)
		YMax = Vert.xyz[1];

	Vert.xyz[0] = CheckMe->Eastest();
	Vert.xyz[1] = CheckMe->Southest();
	if (projectMe)
		{
		DEMCoordSys->ProjToCart(&Vert);
		TempMergeSys->CartToProj(&Vert);
		} // if
	if (Vert.xyz[0] < XMin)
		XMin = Vert.xyz[0];
	if (Vert.xyz[0] > XMax)
		XMax = Vert.xyz[0];
	if (Vert.xyz[1] < YMin)
		YMin = Vert.xyz[1];
	if (Vert.xyz[1] > YMax)
		YMax = Vert.xyz[1];

	Vert.xyz[0] = CheckMe->Westest();
	Vert.xyz[1] = CheckMe->Southest();
	if (projectMe)
		{
		DEMCoordSys->ProjToCart(&Vert);
		TempMergeSys->CartToProj(&Vert);
		} // if
	if (Vert.xyz[0] < XMin)
		XMin = Vert.xyz[0];
	if (Vert.xyz[0] > XMax)
		XMax = Vert.xyz[0];
	if (Vert.xyz[1] < YMin)
		YMin = Vert.xyz[1];
	if (Vert.xyz[1] > YMax)
		YMax = Vert.xyz[1];

	} // if
else
	{
	XMax = CheckMe->Westest();
	XMin = CheckMe->Eastest();
	YMax = CheckMe->Northest();
	YMin = CheckMe->Southest();
	} // else

} // DEMMerger::UpdateMergeBounds

/*===========================================================================*/

#ifdef GORILLA
float DEMMerger::GaussFix(float *Elevs, unsigned long Rows, unsigned long CtrCol, unsigned long CtrRow)
{
double GaussSum = 0.0, GaussWeight = 0.0, weight;
double GaussKernal[5][5] =
	{{1.0, 2.0, 3.0, 2.0, 1.0},
	{2.0, 7.0, 11.0, 7.0, 2.0},
	{3.0, 11.0, 17.0, 11.0, 3.0},
	{2.0, 7.0, 11.0, 7.0, 2.0},
	{1.0, 2.0, 3.0, 2.0, 1.0}};
/***
double GaussKernal[9][9] = {
	{ 0.0,  0.0,  1.0,  1.0,  1.0,  1.0,  1.0,  0.0,  0.0},
	{ 0.0,  1.0,  2.0,  3.0,  3.0,  3.0,  2.0,  1.0,  0.0},
	{ 1.0,  2.0,  3.0,  6.0,  7.0,  6.0,  3.0,  2.0,  1.0},
	{ 1.0,  3.0,  6.0,  9.0, 11.0,  9.0,  6.0,  3.0,  1.0},
	{ 1.0,  3.0,  7.0, 11.0, 12.0, 11.0,  7.0,  3.0,  1.0},
	{ 1.0,  3.0,  6.0,  9.0, 11.0,  9.0,  6.0,  3.0,  1.0},
	{ 1.0,  2.0,  3.0,  6.0,  7.0,  6.0,  3.0,  2.0,  1.0},
	{ 0.0,  1.0,  2.0,  3.0,  3.0,  3.0,  2.0,  1.0,  0.0},
	{ 0.0,  0.0,  1.0,  1.0,  1.0,  1.0,  1.0,  0.0,  0.0}};
***/
float *cell;
unsigned long xkern, ykern;

for (ykern = 0; ykern < 5; ykern++)
	{
	for (xkern = 0; xkern < 5; xkern++)
		{
		// array + x * rows + rows - y
		cell = Elevs + (CtrCol - 2 + xkern) * Rows + Rows - (CtrRow - 2 + ykern) - 1;
		if (*cell != NullVal)
			{
			weight = GaussKernal[ykern][xkern];
			GaussWeight += weight;
			GaussSum += *cell * weight;
			} // if
		} // for ykern
	} // for xkern

if (GaussWeight == 0.0)
	return NullVal;	// we're stuck in a sea of unset data
else
	return (float)(GaussSum / GaussWeight);

} // DEMMerger::GaussFix
#endif // GORILLA

/*===========================================================================*/

void DEMMerger::Merge(Database *DBHost, Project *ProjHost, EffectsLib *EffectsHost, InterCommon *InteractHost)
{
double NBound, SBound, EBound, WBound, MergeXRes, MergeYRes;
double dem_N, dem_S, dem_E, dem_W;
double bigbox_N, bigbox_S, bigbox_E, bigbox_W;	// the merged raster bounds
double smlbox_N, smlbox_S, smlbox_E, smlbox_W;	// the active DEM bounds
double xstep, ystep, xstep2, ystep2;
double elev;
unsigned long smlbox_Ncell, smlbox_Scell, smlbox_Ecell, smlbox_Wcell;	// the active DEM bounds as x/y cell coords
unsigned long firstX, firstY, lastX, lastY;	// col & row bounds of non-NULL data region
int Reject;
float *cell;
//float *elevs;
unsigned long dems, i, passes, x, y;
VertexDEM Vert;
SearchQuery *Query;
DEM *Topo = NULL;
BusyWin *BWDM = NULL;
Joe **Approved = NULL, *Clip;
EffectList *CurQuery;
CoordSys *HostCoords, *TempMergeSys;
JoeCoordSys *MyAttr;
char busytitle[64], errmsg[256], filename[256], SaveName[80];

SetGoodBoundsEtc();
if (! GoodBounds)
	{
	UserMessageOK("DEM Merge", "Improper bounds. Please set bounds for merge and try again.");
	return;
	} // if
if (! Queries)
	{
	UserMessageOK("DEM Merge", "No Search Queries. Please select Search Queries for merge and try again.");
	return;
	} // if

if (MergeCoordSys)
	TempMergeSys = MergeCoordSys;
else
	{
	TempMergeSys = GlobalApp->AppEffects->FetchDefaultCoordSys();
	} // else
DBHost->ResetGeoClip();

// see how many dems there are, and allocate an array of joe pointers of that size
dems = 1;	// need to store a NULL pointer too
for (Clip = DBHost->GetFirst(); Clip; Clip = DBHost->GetNext(Clip))
	{
	if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
		dems++;
	} // for
if (Approved = (Joe **)AppMem_Alloc((dems + 1) * sizeof(Joe *), APPMEM_CLEAR))
	{
	if (Topo = new DEM)
		{
		long geographic = false;

		// run search queries on database and make a list of approved joes
		CurQuery = Queries;
		while (CurQuery)
			{
			if (Query = (SearchQuery *)CurQuery->Me)
				{
				for (Clip = DBHost->GetFirst(); Clip; Clip = DBHost->GetNext(Clip))
					{
					if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
						{
						if (Query->ApproveJoe(Clip))
							{
							for (i = 0; i < dems; i++)
								{
								if (Approved[i] == NULL)	// found an empty slot
									{
									Approved[i++] = Clip;
									break;
									} // if
								else if (Approved[i] == Clip)	// see if we've already approved it
									break;
								} // for
							} // if approved
						} // if active dem
					} // for clip
				} // if
			CurQuery = CurQuery->Next;
			} // while query

		passes = i;

		NBound = NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue;
		SBound = NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
		EBound = NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue;
		WBound = NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue;
		MergeXRes = AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEXRES].CurValue;
		MergeYRes = AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEYRES].CurValue;

		Topo->SetLonEntries(MergeWidth);
		Topo->SetLatEntries(MergeHeight);
		if (TempMergeSys->Method.GCTPMethod == 0)
			{
			XCellSize = xstep = MergeXRes;
			YCellSize = ystep = MergeYRes;
			XOrigin = WBound;
			YOrigin = SBound;
			Topo->SetLonStep(xstep);
			xstep = -xstep;		// keep things simple in the raster loop
			Topo->SetLatStep(ystep);
			Topo->pNorthWest.Lon = XOrigin;
			Topo->pSouthEast.Lon = XOrigin - (MergeWidth - 1) * XCellSize;
			geographic = true;
			} // if
		else
			{
			XCellSize = MergeXRes;
			YCellSize = MergeYRes;
			xstep = XCellSize;
			ystep = YCellSize;

			XOrigin = (unsigned long)((WBound / MergeXRes) * MergeXRes + epsilon);
			YOrigin = (unsigned long)((SBound / MergeYRes) * MergeYRes + epsilon);
			Topo->SetLonStep(-MergeXRes);
			Topo->SetLatStep(MergeYRes);
			Topo->pNorthWest.Lon = XOrigin;
			Topo->pSouthEast.Lon = XOrigin + (MergeWidth - 1) * XCellSize;
			} // else
		Topo->pSouthEast.Lat = YOrigin;
		Topo->pNorthWest.Lat = YOrigin + (MergeHeight - 1) * YCellSize;
		xstep2 = fabs(xstep) * 0.5;
		ystep2 = ystep * 0.5;

		if (!(Topo->AllocRawMap()))
			{
			UserMessageOK("DEM Merge", "Unable to allocate raster.  Try a smaller area");
			goto EndMerge;
			} // if

		// set bigbox coords
		bigbox_W = XOrigin;
		bigbox_E = XOrigin + (MergeWidth - 1) * xstep;
		bigbox_S = YOrigin;
		bigbox_N = YOrigin + (MergeHeight - 1) * ystep;

		// setup's done, proceed with merging
		if (InteractHost->ActiveDEM)
			delete InteractHost->ActiveDEM;
		InteractHost->ActiveDEM = new DEM;

		NullVal = -12345.0f;

		// set valid data markers
		firstX = MergeWidth - 1;
		lastX = 0;
		firstY = MergeHeight - 1;
		lastY = 0;
		// init raster to nulls
		cell = Topo->Map();
		for (x = 0; x < MergeWidth; x++)
			{
			for (y = 0; y < MergeHeight; y++)
				*cell++ = NullVal;
			} // for
		i = 0;
		while (Approved[i])
			{
			if (InteractHost->ActiveDEM && InteractHost->ActiveDEM->AttemptLoadDEM(Approved[i], 1, ProjHost))
				{
				double yatmp;

				if (InteractHost->ActiveDEM->MoJoe &&
					(MyAttr = (JoeCoordSys *)InteractHost->ActiveDEM->MoJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
					HostCoords = MyAttr->Coord;
				else
					HostCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
				// get this DEM's bounds
				dem_N = InteractHost->ActiveDEM->Northest();
				dem_S = InteractHost->ActiveDEM->Southest();
				dem_E = InteractHost->ActiveDEM->Eastest();
				dem_W = InteractHost->ActiveDEM->Westest();
				// compute bounding box coords
				// NW corner
				Vert.xyz[0] = dem_W;
				Vert.xyz[1] = dem_N;
				if (TempMergeSys != HostCoords)
					{
					HostCoords->ProjToCart(&Vert);
					TempMergeSys->CartToProj(&Vert);
					} // if
				smlbox_N = Vert.xyz[1];
				smlbox_W = Vert.xyz[0];
				// SE corner
				Vert.xyz[0] = dem_E;
				Vert.xyz[1] = dem_S;
				if (TempMergeSys != HostCoords)
					{
					HostCoords->ProjToCart(&Vert);
					TempMergeSys->CartToProj(&Vert);
					} // if
				smlbox_S = Vert.xyz[1];
				smlbox_E = Vert.xyz[0];
				// NE corner
				Vert.xyz[0] = dem_E;
				Vert.xyz[1] = dem_N;
				if (TempMergeSys != HostCoords)
					{
					HostCoords->ProjToCart(&Vert);
					TempMergeSys->CartToProj(&Vert);
					} // if
				if (Vert.xyz[1] > smlbox_N)
					smlbox_N = Vert.xyz[1];
				if (geographic && (Vert.xyz[0] < smlbox_E))
					smlbox_E = Vert.xyz[0];
				else if (Vert.xyz[0] > smlbox_E)
					smlbox_E = Vert.xyz[0];
				// SW corner
				Vert.xyz[0] = dem_W;
				Vert.xyz[1] = dem_S;
				if (TempMergeSys != HostCoords)
					{
					HostCoords->ProjToCart(&Vert);
					TempMergeSys->CartToProj(&Vert);
					} // if
				if (Vert.xyz[1] < smlbox_S)
					smlbox_S = Vert.xyz[1];
				if (geographic && (Vert.xyz[0] > smlbox_W))
					smlbox_W = Vert.xyz[0];
				else if (Vert.xyz[0] < smlbox_W)
					smlbox_W = Vert.xyz[0];
				// relate smallbox to bigbox cells
				//smlbox_Wcell = (unsigned long)((smlbox_W - XOrigin) / xstep);
				yatmp = (smlbox_W - XOrigin) / xstep + xstep2;
				if (yatmp < 0.0)
					yatmp = 0.0;
				smlbox_Wcell = (unsigned long)yatmp;
				smlbox_Ecell = (unsigned long)((smlbox_E - XOrigin) / xstep + xstep2);
				if (smlbox_Ecell >= MergeWidth)
					smlbox_Ecell = MergeWidth - 1;
				//smlbox_Scell = (unsigned long)((smlbox_S - YOrigin) / ystep);
				yatmp = (smlbox_S - YOrigin) / ystep + ystep2;
				if (yatmp < 0.0)
					yatmp = 0.0;
				smlbox_Scell = (unsigned long)yatmp;
				smlbox_Ncell = (unsigned long)((smlbox_N - YOrigin) / ystep + ystep2);
				if (smlbox_Ncell >= MergeHeight)
					smlbox_Ncell = MergeHeight - 1;
				/***
				// adjust smallbox bounds by 3 cells to give us some slack
				if (smlbox_Wcell < 3)
					smlbox_Wcell = 0;
				else
					smlbox_Wcell -= 3;
				smlbox_Ecell += 3;
				if (smlbox_Ecell >= MergeWidth)
					smlbox_Ecell = MergeWidth - 1;
				if (smlbox_Scell < 3)
					smlbox_Scell = 0;
				else
					smlbox_Scell -= 3;
				smlbox_Ncell += 3;
				if (smlbox_Ncell >= MergeHeight)
					smlbox_Ncell = MergeHeight - 1;
				***/

				sprintf(busytitle, "Computing (%u of %u)", i + 1, passes);
				BWDM = new BusyWin(busytitle, smlbox_Ecell - smlbox_Wcell, 'BWDM', 0);
				Vert.xyz[0] = XOrigin + smlbox_Wcell * xstep;
				//sprintf(mergedbg, "Topo->Map() = %p .. %p\n", Topo->Map(), Topo->Map() + Topo->MapSize() - 1);
				//OutputDebugString(mergedbg);
				for (x = smlbox_Wcell; x <= smlbox_Ecell; x++, Vert.xyz[0] += xstep)
					{
					cell = Topo->Map() + MergeHeight * x;
					//sprintf(mergedbg, "  x = %u, cell = %p\n", x, cell);
					//OutputDebugString(mergedbg);
					Vert.xyz[1] = YOrigin + smlbox_Scell * ystep;
					// cell += smlbox_Scell _IS_ needed - trust me :)
					for (y = smlbox_Scell, cell += smlbox_Scell; y <= smlbox_Ncell; y++, Vert.xyz[1] += ystep)
						{
						// if we haven't filled this cell yet
						//sprintf(mergedbg, "    y = %u, cell = %p\n", y, cell);
						//OutputDebugString(mergedbg);
						if (*cell == NullVal)
							{
							if (TempMergeSys != HostCoords)
								{
								TempMergeSys->ProjToDeg(&Vert);
								// ask for an elev point in a DEM that passes our query
								elev = InteractHost->SQElevationPointNULLReject(Vert.Lat, Vert.Lon, Reject, TempMergeSys);
								} // if
							else
								{
								elev = InteractHost->SQElevationPointNULLReject(Vert.xyz[1], Vert.xyz[0], Reject, TempMergeSys);
								} // else
							// if we have a real elev value & there's nothing already stored at that location
							if (!Reject)
								{
								*cell = (float)elev;
								if (x < firstX)
									firstX = x;
								if (x > lastX)
									lastX = x;
								if (y < firstY)
									firstY = y;
								if (y > lastY)
									lastY = y;
								} // if
							} // if
						cell++;
						} // for y
					if (BWDM)
						{
						BWDM->Update(x - smlbox_Wcell);
						if (BWDM->CheckAbort())
							{
							delete BWDM;
							BWDM = NULL;
							goto EndMerge;
							} // if
						} // if
					} // for x
				} // if DEM loaded
			else
				{
				sprintf(&errmsg[0], "Couldn't load %s - skipped", Approved[i]->FName);
				GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
				} // else
			i++;
			if (BWDM) delete BWDM;
			BWDM = NULL;
			} // while

		// F2NOTE: Gauss fix has more problems than solutions.  Rethink if this is really feasible under all conditions.
		/***
		// Gauss fix the merge
		elevs = Topo->Map();
		if (lastY > (firstY + 4))
			{
			BWDM = new BusyWin("Gap filling", lastY - firstY - 4, 'BWDM', 0);
			for (x = firstX + 2; x < (lastX - 2); x++)
				{
				for (y = firstY; y < (lastY - 2); y++)
					{
					cell = elevs + x * MergeHeight + MergeHeight - y - 1;
					if (*cell == NullVal)
						*cell = GaussFix(elevs, MergeHeight, x, y);
					} // for y
				if (BWDM)
					{
					BWDM->Update(x);
					if (BWDM->CheckAbort())
						{
						break;
						} // if
					} // if
				} // for x
			if (BWDM) delete BWDM;
			BWDM = NULL;
			} // if
			***/

		Topo->SetNullValue(NullVal);
		Topo->SetNullReject(1);
		Topo->pElScale = ELSCALE_METERS;
		Topo->pElDatum = 0.0;
		Topo->FindElMaxMin();
		//Topo->FillVoids();

		strcpy(SaveName, DEMName);
		DBHost->AddDEMToDatabase("DEM Merge", SaveName, Topo, MergeCoordSys, ProjHost, EffectsHost);
		strcat(SaveName, ".elev");
		strmfp(filename, DEMPath, SaveName);
		if (! Topo->SaveDEM(filename, GlobalApp->StatusLog))
			{
			UserMessageOK("DEM Merge", "Unable to save raster!");
			} // if
		Topo->FreeRawElevs();

		// Turn off all DEMs used for merging
		i = 0;
		while (Approved[i])
			{
			Clip = Approved[i];
			Clip->ClearFlags(WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED);
			i++;
			} // while
		} // if DEM
	else
		{
		UserMessageOK("DEM Merge", "Unable to allocate DEM object");
		} // else DEM
	} // if


EndMerge:
if (Approved)
	AppMem_Free(Approved, (dems + 1) * sizeof(Joe *));
if (Topo)
	delete Topo;

} // DEMMerger::Merge

/*===========================================================================*/

void DEMMerger::MergeMultiRes(Database *DBHost, Project *ProjHost, EffectsLib *EffectsHost, InterCommon *InteractHost)
{
double HiResXRes, HiResYRes;
double elev, xstep, ystep, NBound, SBound, EBound, WBound, HiResN, HiResS, HiResE, HiResW, MergeXRes, MergeYRes, Divider;
double dem_N, dem_S, dem_E, dem_W;
double bigbox_N, bigbox_S, bigbox_E, bigbox_W;	// the merged raster bounds
double smlbox_N, smlbox_S, smlbox_E, smlbox_W;	// the active DEM bounds
unsigned long smlbox_Ncell, smlbox_Scell, smlbox_Ecell, smlbox_Wcell;	// the active DEM bounds as x/y cell coords
int Reject;
float *cell;
unsigned long dems, i, nulle, nulln, nulls, nullw, passes = 0, x, y;
unsigned long div, xx, yy;
VertexDEM Vert;
SearchQuery *Query;
DEM *HRTopo = NULL, *Topo = NULL;
BusyWin *BWDM = NULL;
Joe **Approved = NULL, *Clip;
EffectList *CurQuery;
CoordSys *HostCoords, *TempMergeSys;
JoeCoordSys *MyAttr;
DEMMerger HighRes;
long PosW = 0;
char busytitle[64], errmsg[256], filename[256], SaveName[80];

SetGoodBoundsEtc();
if (! GoodBounds)
	{
	UserMessageOK("DEM Merge", "Improper bounds. Please set bounds for merge and try again.");
	return;
	} // if
if (! Queries)
	{
	UserMessageOK("DEM Merge", "No Search Queries. Please select Search Queries for merge and try again.");
	return;
	} // if

if (MergeCoordSys)
	TempMergeSys = MergeCoordSys;
else
	{
	TempMergeSys = GlobalApp->AppEffects->FetchDefaultCoordSys();
	} // else
TempMergeSys->GetGeographic();
DBHost->ResetGeoClip();

// see how many dems there are, and allocate an array of joe pointers of that size
dems = 1;	// need to store a NULL pointer too
for (Clip = DBHost->GetFirst(); Clip; Clip = DBHost->GetNext(Clip))
	{
	if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
		dems++;
	} // for
Approved = (Joe **)AppMem_Alloc((dems + 1) * sizeof(Joe *), APPMEM_CLEAR);
if (! Approved)
	return;

// run search queries on database and make a list of approved joes, starting with high res
CurQuery = Queries;
while (CurQuery)
	{
	if (Query = (SearchQuery *)CurQuery->Me)
		{
		for (Clip = DBHost->GetFirst(); Clip; Clip = DBHost->GetNext(Clip))
			{
			if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				if (Query->ApproveJoe(Clip))
					{
					for (i = 0; i < dems; i++)
						{
						if (Approved[i] == NULL)	// found an empty slot
							{
							passes++;
							Approved[i++] = Clip;
							break;
							} // if
						else if (Approved[i] == Clip)	// see if we've already approved it
							break;
						} // for
					} // if approved
				} // if active dem
			} // for clip
		} // if
	CurQuery = CurQuery->Next;
	} // while

//passes = i;

NBound = NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue;
SBound = NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
EBound = NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue;
WBound = NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue;
HiResN = HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue;
HiResS = HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
HiResE = HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue;
HiResW = HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue;
MergeXRes = AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEXRES].CurValue;
MergeYRes = AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEYRES].CurValue;
Divider = AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_DIVIDER].CurValue;

//
// compute high res
//

if (!(HRTopo = new DEM))
	{
	UserMessageOK("DEM Merge", "Unable to allocate DEM object");
	return;
	} // if

if (!(Topo = new DEM))
	{
	delete HRTopo;
	UserMessageOK("DEM Merge", "Unable to allocate DEM object");
	return;
	} // if

if (TempMergeSys->Geographic)
	{
	XCellSize = xstep = MergeXRes;
	YCellSize = ystep = MergeYRes;
	XOrigin = WBound;
	YOrigin = SBound;
	MergeHeight = (unsigned long)((NBound - SBound) / YCellSize + 1 + epsilon);
	MergeWidth = (unsigned long)((WBound - EBound) / XCellSize + 1 + epsilon);
	NBound = YOrigin + (MergeHeight - 1) * YCellSize;
	EBound = XOrigin - (MergeWidth - 1) * XCellSize;
	Topo->SetLonStep(xstep);
	xstep = -xstep;		// keep things simple in the raster loop
	Topo->SetLatStep(ystep);
	Topo->pNorthWest.Lon = XOrigin;
	Topo->pSouthEast.Lon = XOrigin - (MergeWidth - 1) * XCellSize;
	} // if
else
	{
	XCellSize = MergeXRes;
	YCellSize = MergeYRes;
	xstep = XCellSize;
	ystep = YCellSize;

	// round merge bounds off to even multiples of resolution, rounding towards the center of the DEM
	NBound = (unsigned long)(NBound / MergeYRes) * MergeYRes;						// round down
	SBound = (unsigned long)((SBound + MergeYRes * 0.5) / MergeYRes) * MergeYRes;	// round up
	EBound = (unsigned long)(EBound / MergeXRes) * MergeXRes;						// round down
	WBound = (unsigned long)((WBound + MergeXRes * 0.5) / MergeXRes) * MergeXRes;	// round up
	XOrigin = WBound;
	YOrigin = SBound;
	MergeHeight = (unsigned long)((NBound - SBound) / MergeYRes + 1.5);
	MergeWidth = (unsigned long)((EBound - WBound) / MergeXRes + 1.5);
	Topo->SetLonStep(-MergeXRes);
	Topo->SetLatStep(MergeYRes);
	Topo->pNorthWest.Lon = XOrigin;
	Topo->pSouthEast.Lon = XOrigin + (MergeWidth - 1) * XCellSize;
	} // else
Topo->pSouthEast.Lat = YOrigin;
Topo->pNorthWest.Lat = YOrigin + (MergeHeight - 1) * YCellSize;

Topo->SetLonEntries(MergeWidth);
Topo->SetLatEntries(MergeHeight);

// ensure hires is on even multiples of lores
if (TempMergeSys->Geographic)
	{
	double tmp2;

	HiResXRes = XCellSize / Divider;
	HiResYRes = YCellSize / Divider;
	HighRes.XCellSize = HiResXRes;
	HighRes.YCellSize = HiResYRes;
	xstep = HighRes.XCellSize;
	ystep = HighRes.YCellSize;

	// easier debugging when result of fmod is broken into separate variable
	// Snap N South, S North, E West, W East
	tmp2 = fmod(HiResW, XCellSize);
	HighRes.XOrigin = HiResW = HiResW - tmp2;
	tmp2 = fmod(HiResS, YCellSize);
	HighRes.YOrigin = HiResS = HiResS + tmp2;
	tmp2 = fmod(HiResN, YCellSize);
	HiResN = HiResN - tmp2;
	tmp2 = fmod(HiResE, XCellSize);
	if (tmp2 > 0.0)
		HiResE += XCellSize;
	HiResE = HiResE - tmp2;
	HighRes.MergeHeight = (unsigned long)((HiResN - HiResS) / HiResYRes + 1 + epsilon);
	HighRes.MergeWidth = (unsigned long)((HiResW - HiResE) / HiResXRes + 1 + epsilon);
	HRTopo->SetLonStep(xstep);
	xstep = -xstep;
	HRTopo->SetLatStep(ystep);
	HRTopo->pNorthWest.Lon = HighRes.XOrigin;
	HRTopo->pSouthEast.Lon = HighRes.XOrigin - (HighRes.MergeWidth - 1) * HighRes.XCellSize;
	} // if
else
	{
	HiResXRes = MergeXRes / Divider;
	HiResYRes = MergeYRes / Divider;
	HighRes.XCellSize = HiResXRes;
	HighRes.YCellSize = HiResYRes;
	xstep = HighRes.XCellSize;
	ystep = HighRes.YCellSize;

	HighRes.XOrigin = HiResW = (unsigned long)((HiResW + MergeXRes * 0.5) / MergeXRes) * MergeXRes;	// round up
	HighRes.YOrigin = HiResS = (unsigned long)((HiResS + MergeYRes * 0.5) / MergeYRes) * MergeYRes;	// round up
	HiResN = (unsigned long)(HiResN / MergeYRes) * MergeYRes;										// round down
	HiResE = (unsigned long)(HiResE / MergeXRes) * MergeXRes;										// round down
	HighRes.MergeHeight = (unsigned long)((HiResN - HiResS) / HiResYRes + 1.5);
	HighRes.MergeWidth = (unsigned long)((HiResE - HiResW) / HiResXRes + 1.5);
	HRTopo->SetLonStep(-HiResXRes);
	HRTopo->SetLatStep(HiResYRes);
	HRTopo->pNorthWest.Lon = HighRes.XOrigin;
	HRTopo->pSouthEast.Lon = HighRes.XOrigin + (HighRes.MergeWidth - 1) * HighRes.XCellSize;
	} // else
HRTopo->pSouthEast.Lat = HighRes.YOrigin;
HRTopo->pNorthWest.Lat = HighRes.YOrigin + (HighRes.MergeHeight - 1) * HighRes.YCellSize;

HRTopo->SetLonEntries(HighRes.MergeWidth);
HRTopo->SetLatEntries(HighRes.MergeHeight);
if (!(HRTopo->AllocRawMap()))
	{
	delete HRTopo;
	UserMessageOK("DEM Merge", "Unable to allocate raster.  Try a smaller area");
	return;
	} // if

// setup's done, proceed with merging
if (InteractHost->ActiveDEM)
	delete InteractHost->ActiveDEM;
InteractHost->ActiveDEM = new DEM;

NullVal = -12345.0f;

// init raster to nulls
cell = HRTopo->Map();
for (x = 0; x < HighRes.MergeWidth; x++)
	{
	for (y = 0; y < HighRes.MergeHeight; y++)
		*cell++ = NullVal;
	} // for

i = 0;
while (Approved[i])
	{
	if (InteractHost->ActiveDEM && InteractHost->ActiveDEM->AttemptLoadDEM(Approved[i], 1, ProjHost))
		{
		if (InteractHost->ActiveDEM->MoJoe &&
			(MyAttr = (JoeCoordSys *)InteractHost->ActiveDEM->MoJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
			HostCoords = MyAttr->Coord;
		else
			HostCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
		// get this DEM's bounds
		dem_N = InteractHost->ActiveDEM->Northest();
		dem_S = InteractHost->ActiveDEM->Southest();
		dem_E = InteractHost->ActiveDEM->Eastest();
		dem_W = InteractHost->ActiveDEM->Westest();
		// compute bounding box coords
		// NW corner
		Vert.xyz[0] = dem_W;
		Vert.xyz[1] = dem_N;
		if (TempMergeSys != HostCoords)
			{
			HostCoords->ProjToCart(&Vert);
			TempMergeSys->CartToProj(&Vert);
			} // if
		smlbox_N = Vert.xyz[1];
		smlbox_W = Vert.xyz[0];
		// SE corner
		Vert.xyz[0] = dem_E;
		Vert.xyz[1] = dem_S;
		if (TempMergeSys != HostCoords)
			{
			HostCoords->ProjToCart(&Vert);
			TempMergeSys->CartToProj(&Vert);
			} // if
		smlbox_S = Vert.xyz[1];
		smlbox_E = Vert.xyz[0];
		// NE corner
		Vert.xyz[0] = dem_E;
		Vert.xyz[1] = dem_N;
		if (TempMergeSys != HostCoords)
			{
			HostCoords->ProjToCart(&Vert);
			TempMergeSys->CartToProj(&Vert);
			} // if
		if (Vert.xyz[1] > smlbox_N)
			smlbox_N = Vert.xyz[1];
		if (Vert.xyz[0] < smlbox_E)
			smlbox_E = Vert.xyz[0];
		// SW corner
		Vert.xyz[0] = dem_W;
		Vert.xyz[1] = dem_S;
		if (TempMergeSys != HostCoords)
			{
			HostCoords->ProjToCart(&Vert);
			TempMergeSys->CartToProj(&Vert);
			} // if
		if (Vert.xyz[1] < smlbox_S)
			smlbox_S = Vert.xyz[1];
		if (Vert.xyz[0] > smlbox_W)
			smlbox_W = Vert.xyz[0];
		// set bigbox coords
		bigbox_W = HighRes.XOrigin;
		bigbox_E = HighRes.XOrigin + (HighRes.MergeWidth - 1) * xstep;
		bigbox_S = HighRes.YOrigin;
		bigbox_N = HighRes.YOrigin + (HighRes.MergeHeight - 1) * ystep;
		// relate smallbox to bigbox cells
		if (smlbox_W < bigbox_W)
			smlbox_Wcell = 0;
		else
			smlbox_Wcell = (unsigned long)((smlbox_W - HighRes.XOrigin) / xstep);
		if (smlbox_E > bigbox_E)
			smlbox_Ecell = HighRes.MergeWidth - 1;
		else
			smlbox_Ecell = (unsigned long)((smlbox_E - HighRes.XOrigin) / xstep);
		if (smlbox_S < bigbox_S)
			smlbox_Scell = 0;
		else
			smlbox_Scell = (unsigned long)((smlbox_S - HighRes.YOrigin) / ystep);
		if (smlbox_N > bigbox_N)
			smlbox_Ncell = HighRes.MergeHeight - 1;
		else
			smlbox_Ncell = (unsigned long)((smlbox_N - HighRes.YOrigin) / ystep);
		// adjust smallbox bounds by 3 cells to give us some slack
		if (smlbox_Wcell < 3)
			smlbox_Wcell = 0;
		else
			smlbox_Wcell -= 3;
		smlbox_Ecell += 3;
		if (smlbox_Ecell >= HighRes.MergeWidth)
			smlbox_Ecell = HighRes.MergeWidth - 1;
		if (smlbox_Scell < 3)
			smlbox_Scell = 0;
		else
			smlbox_Scell -= 3;
		smlbox_Ncell += 3;
		if (smlbox_Ncell >= HighRes.MergeHeight)
			smlbox_Ncell = HighRes.MergeHeight - 1;
		// F2 11/06/03 temp fix
		smlbox_Wcell = 0;
		smlbox_Scell = 0;
		smlbox_Ecell = HighRes.MergeWidth - 1;
		smlbox_Ncell = HighRes.MergeHeight - 1;
						
		sprintf(busytitle, "Computing (%u of %u)", i + 1, passes);
		BWDM = new BusyWin(busytitle, smlbox_Ecell - smlbox_Wcell, 'BWDM', 0);
		Vert.xyz[0] = HighRes.XOrigin + smlbox_Wcell * xstep;
		for (x = smlbox_Wcell; x <= smlbox_Ecell; x++, Vert.xyz[0] += xstep)
			{
			cell = HRTopo->Map() + HighRes.MergeHeight * x;
			Vert.xyz[1] = HighRes.YOrigin + smlbox_Scell * ystep;
			for (y = smlbox_Scell, cell += smlbox_Scell; y <= smlbox_Ncell; y++, Vert.xyz[1] += ystep)
				{
				// if we haven't filled this cell yet
				if (*cell == NullVal)
					{
					if (TempMergeSys != HostCoords)
						{
						TempMergeSys->ProjToDeg(&Vert);
						// ask for an elev point in a DEM that passes our query
						elev = InteractHost->SQElevationPointNULLReject(Vert.Lat, Vert.Lon, Reject, TempMergeSys);
						} // if
					else
						{
						elev = InteractHost->SQElevationPointNULLReject(Vert.xyz[1], Vert.xyz[0], Reject, TempMergeSys);
						} // else
					// if we have a real elev value & there's nothing already stored at that location
					if (!Reject)
						*cell = (float)elev;
					} // if
				cell++;
				} // for y
			if (BWDM)
				{
				BWDM->Update(x - smlbox_Wcell);
				if (BWDM->CheckAbort())
					{
					delete BWDM;
					BWDM = NULL;
					return;
					} // if
				} // if
			} // for x
		} // if DEM loaded
	else
		{
		sprintf(&errmsg[0], "Couldn't load %s - skipped", Approved[i]->FName);
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
		} // else
	i++;
	if (BWDM) delete BWDM;
	BWDM = NULL;
	} // while

// delay saving of HRTopo until we've copied the seam over from the low res set
AppMem_Free(Approved, (dems + 1) * sizeof(Joe *));

//
// compute master lores raster
//

Approved = (Joe **)AppMem_Alloc((dems + 1) * sizeof(Joe *), APPMEM_CLEAR);
if (! Approved)
	return;

// run search queries on database and make a list of approved joes
CurQuery = Queries ? Queries->Next: NULL;
while (CurQuery)
	{
	if (Query = (SearchQuery *)CurQuery->Me)
		{
		for (Clip = DBHost->GetFirst(); Clip; Clip = DBHost->GetNext(Clip))
			{
			if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				if (Query->ApproveJoe(Clip))
					{
					for (i = 0; i < dems; i++)
						{
						if (Approved[i] == NULL)	// found an empty slot
							{
							Approved[i++] = Clip;
							break;
							} // if
						else if (Approved[i] == Clip)	// see if we've already approved it
							break;
						} // for
					} // if approved
				} // if active dem
			} // for clip
		} // if
	CurQuery = CurQuery->Next;
	} // while query

passes = i;

// compute the bounds in cell coords of the NULL data area
nulln = (unsigned long)((HiResN - YOrigin) / YCellSize + epsilon);
nulls = (unsigned long)((HiResS - YOrigin) / YCellSize + epsilon);
if (TempMergeSys->Geographic)
	{
	nulle = (unsigned long)((XOrigin - HiResE) / XCellSize + epsilon);	// damn floating point...
	nullw = (unsigned long)((XOrigin - HiResW) / XCellSize + epsilon);
	} // if
else
	{
	nulle = (unsigned long)((HiResE - XOrigin) / XCellSize + epsilon);
	nullw = (unsigned long)((HiResW - XOrigin) / XCellSize + epsilon);
	} // else

if (!(Topo->AllocRawMap()))
	{
	delete Topo;
	UserMessageOK("DEM Merge", "Unable to allocate raster.  Try a smaller area");
	return;
	}

if (InteractHost->ActiveDEM)
	delete InteractHost->ActiveDEM;
InteractHost->ActiveDEM = new DEM;

NullVal = -12345.0f;

// init raster to nulls
cell = Topo->Map();
for (x = 0; x < MergeWidth; x++)
	{
	for (y = 0; y < MergeHeight; y++)
		*cell++ = NullVal;
	} // for x
i = 0;
if (TempMergeSys->Geographic)
	{
	xstep = -XCellSize;
	PosW = 1;
	}
else
	xstep = XCellSize;
ystep = YCellSize;
while (Approved[i])
	{
	if (InteractHost->ActiveDEM && InteractHost->ActiveDEM->AttemptLoadDEM(Approved[i], 1, ProjHost))
		{

		if (InteractHost->ActiveDEM->MoJoe &&
			(MyAttr = (JoeCoordSys *)InteractHost->ActiveDEM->MoJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
			HostCoords = MyAttr->Coord;
		else
			HostCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
		// get this DEM's bounds
		dem_N = InteractHost->ActiveDEM->Northest();
		dem_S = InteractHost->ActiveDEM->Southest();
		dem_E = InteractHost->ActiveDEM->Eastest();
		dem_W = InteractHost->ActiveDEM->Westest();
		// compute bounding box coords
		// NW corner
		Vert.xyz[0] = dem_W;
		Vert.xyz[1] = dem_N;
		if (TempMergeSys != HostCoords)
			{
			HostCoords->ProjToCart(&Vert);
			TempMergeSys->CartToProj(&Vert);
			} // if
		smlbox_N = Vert.xyz[1];
		smlbox_W = Vert.xyz[0];
		// SE corner
		Vert.xyz[0] = dem_E;
		Vert.xyz[1] = dem_S;
		if (TempMergeSys != HostCoords)
			{
			HostCoords->ProjToCart(&Vert);
			TempMergeSys->CartToProj(&Vert);
			} // if
		smlbox_S = Vert.xyz[1];
		smlbox_E = Vert.xyz[0];
		// NE corner
		Vert.xyz[0] = dem_E;
		Vert.xyz[1] = dem_N;
		if (TempMergeSys != HostCoords)
			{
			HostCoords->ProjToCart(&Vert);
			TempMergeSys->CartToProj(&Vert);
			} // if
		if (Vert.xyz[1] > smlbox_N)
			smlbox_N = Vert.xyz[1];
		if (PosW)
			{
			if (Vert.xyz[0] < smlbox_E)
				smlbox_E = Vert.xyz[0];
			} // if
		else
			{
			if (Vert.xyz[0] > smlbox_E)
				smlbox_E = Vert.xyz[0];
			} // else
		// SW corner
		Vert.xyz[0] = dem_W;
		Vert.xyz[1] = dem_S;
		if (TempMergeSys != HostCoords)
			{
			HostCoords->ProjToCart(&Vert);
			TempMergeSys->CartToProj(&Vert);
			} // if
		if (Vert.xyz[1] < smlbox_S)
			smlbox_S = Vert.xyz[1];
		if (PosW)
			{
			if (Vert.xyz[0] > smlbox_W)
				smlbox_W = Vert.xyz[0];
			} //if
		else
			{
			if (Vert.xyz[0] < smlbox_W)
				smlbox_W = Vert.xyz[0];
			} // else
		// set bigbox coords
		bigbox_W = XOrigin;
		bigbox_E = XOrigin + (MergeWidth - 1) * xstep;
		bigbox_S = YOrigin;
		bigbox_N = YOrigin + (MergeHeight - 1) * ystep;
		// relate smallbox to bigbox cells
		smlbox_Wcell = (unsigned long)((smlbox_W - XOrigin) / xstep);
		smlbox_Ecell = (unsigned long)((smlbox_E - XOrigin) / xstep);
		smlbox_Scell = (unsigned long)((smlbox_S - YOrigin) / ystep);
		smlbox_Ncell = (unsigned long)((smlbox_N - YOrigin) / ystep);
		// adjust smallbox bounds by 3 cells to give us some slack
		if (smlbox_Wcell < 3)
			smlbox_Wcell = 0;
		else
			smlbox_Wcell -= 3;
		smlbox_Ecell += 3;
		if (smlbox_Ecell >= MergeWidth)
			smlbox_Ecell = MergeWidth - 1;
		if (smlbox_Scell < 3)
			smlbox_Scell = 0;
		else
			smlbox_Scell -= 3;
		smlbox_Ncell += 3;
		if (smlbox_Ncell >= MergeHeight)
			smlbox_Ncell = MergeHeight - 1;
						
		sprintf(busytitle, "Computing (%u of %u)", i + 1, passes);
		BWDM = new BusyWin(busytitle, smlbox_Ecell - smlbox_Wcell, 'BWDM', 0);
		Vert.xyz[0] = XOrigin + smlbox_Wcell * xstep;
		for (x = smlbox_Wcell; x < smlbox_Ecell; x++, Vert.xyz[0] += xstep)
			{
			cell = Topo->Map() + MergeHeight * x;
			Vert.xyz[1] = YOrigin + smlbox_Scell * ystep;
			for (y = smlbox_Scell, cell += smlbox_Scell; y < smlbox_Ncell; y++, Vert.xyz[1] += ystep)
				{
				// see if we still need to compute this cell
				if (*cell == NullVal)
					{
					// see if we're in the non-null data area
					if (!(x > nullw && x < nulle && y > nulls && y < nulln))
						{
						if (TempMergeSys != HostCoords)
							{
							TempMergeSys->ProjToDeg(&Vert);
							// ask for an elev point in a DEM that passes our query
							elev = InteractHost->SQElevationPointNULLReject(Vert.Lat, Vert.Lon, Reject, TempMergeSys);
							} // if
						else
							{
							elev = InteractHost->SQElevationPointNULLReject(Vert.xyz[1], Vert.xyz[0], Reject, TempMergeSys);
							} // else
						// if we have a real elev value & there's nothing already stored at that location
						if ((!Reject) && (*cell == NullVal))
							*cell = (float)elev;
						} // if
					} // if
				cell++;
				} // for y
			if (BWDM)
				{
				BWDM->Update(x - smlbox_Wcell);
				if (BWDM->CheckAbort())
					{
					delete BWDM;
					BWDM = NULL;
					return;
					} // if
				} // if
			} // for x
		} // if DEM loaded
	else
		{
		sprintf(&errmsg[0], "Couldn't load %s - skipped", Approved[i]->FName);
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, errmsg);
		} // else
	i++;
	if (BWDM) delete BWDM;
	BWDM = NULL;
	} // while

// copy the seam data over to the high res set
div = (unsigned long)Divider;
for (x = nullw; x < nulle; x++)
	{
	float elev;
	unsigned long dx = x - nullw;
	elev = Topo->GetCell(x, MergeHeight - nulln - 1);
	HRTopo->SetCell(dx * div, 0, elev);
	elev = Topo->GetCell(x, MergeHeight - nulls - 1);
	HRTopo->SetCell(dx * div, HighRes.MergeHeight - 1, elev);
	} // for x
// linearly interpolate it between sample points
for (x = div; x < HighRes.MergeWidth; x += div)
	{
	float delta, elev1, elev2;
	// north row
	elev1 = HRTopo->GetCell(x, 0);
	elev2 = HRTopo->GetCell(x - div, 0);
	if ((elev1 == -12345.0f) || (elev2 == -12345.0f))
		delta = 0.0f;
	else
		delta = (float)((elev2 - elev1) / Divider);
	for (xx = 1; xx < div; xx++)
		{
		elev1 += delta;
		HRTopo->SetCell(x - xx, 0, elev1);
		} // for xx
	// south row
	elev1 = HRTopo->GetCell(x, HighRes.MergeHeight - 1);
	elev2 = HRTopo->GetCell(x - div, HighRes.MergeHeight - 1);
	if ((elev1 == -12345.0f) || (elev2 == -12345.0f))
		delta = 0.0f;
	else
		delta = (float)((elev2 - elev1) / Divider);
	for (xx = 1; xx < div; xx++)
		{
		elev1 += delta;
		HRTopo->SetCell(x - xx, HighRes.MergeHeight - 1, elev1);
		} // for xx
	} // for x
// as above
for (y = nulls; y < nulln; y++)
	{
	float elev;
	unsigned long dy = y - nulls;
	elev = Topo->GetCell(nullw, MergeHeight - y - 1);
	HRTopo->SetCell(0, HighRes.MergeHeight - (dy * div) - 1, elev);
	elev = Topo->GetCell(nulle, MergeHeight - y - 1);
	HRTopo->SetCell(HighRes.MergeWidth - 1, HighRes.MergeHeight - (dy * div) - 1, elev);
	} // for y
for (y = div; y < HighRes.MergeHeight; y += div)
	{
	float delta, elev1, elev2;
	// west row
	elev1 = HRTopo->GetCell(0, y);
	elev2 = HRTopo->GetCell(0, y - div);
	if ((elev1 == -12345.0f) || (elev2 == -12345.0f))
		delta = 0.0f;
	else
		delta = (float)((elev2 - elev1) / Divider);
	for (yy = 1; yy < div; yy++)
		{
		elev1 += delta;
		HRTopo->SetCell(0, y - yy, elev1);
		} // for yy
	// east row
	elev1 = HRTopo->GetCell(HighRes.MergeWidth - 1, y);
	elev2 = HRTopo->GetCell(HighRes.MergeWidth - 1, y - div);
	if ((elev1 == -12345.0f) || (elev2 == -12345.0f))
		delta = 0.0f;
	else
		delta = (float)((elev2 - elev1) / Divider);
	for (yy = 1; yy < div; yy++)
		{
		elev1 += delta;
		HRTopo->SetCell(HighRes.MergeWidth - 1, y - yy, elev1);
		} // for yy
	} // for y

// save the lores now
Topo->SetNullValue(NullVal);
Topo->SetNullReject(1);
Topo->pElScale = ELSCALE_METERS;
Topo->pElDatum = 0.0;
Topo->FindElMaxMin();

strcpy(SaveName, DEMName);
DBHost->AddDEMToDatabase("DEM Merge", SaveName, Topo, MergeCoordSys, ProjHost, EffectsHost);
strcat(SaveName, ".elev");
strmfp(filename, DEMPath, SaveName);
if (! Topo->SaveDEM(filename, GlobalApp->StatusLog))
	{
	UserMessageOK("DEM Merge", "Unable to save raster!");
	} // if

AppMem_Free(Approved, (dems + 1) * sizeof(Joe *));

// save the hires now
HRTopo->SetNullValue(NullVal);
HRTopo->SetNullReject(1);
HRTopo->pElScale = ELSCALE_METERS;
HRTopo->pElDatum = 0.0;
HRTopo->FindElMaxMin();

strcpy(SaveName, HiResName);
DBHost->AddDEMToDatabase("DEM Merge", SaveName, HRTopo, MergeCoordSys, ProjHost, EffectsHost);
strcat(SaveName, ".elev");
strmfp(filename, HiResPath, SaveName);
if (! HRTopo->SaveDEM(filename, GlobalApp->StatusLog))
	{
	UserMessageOK("DEM Merge", "Unable to save raster!");
	} // if

// keep this in a conditional so we don't accidentally do this in a customer build
#ifdef WCS_BUILD_FRANK
//char dumpName[256];

//sprintf(dumpName, "C:/OuterDEM_%dx%d.raw", Topo->LonEntries(), Topo->LatEntries());
//Topo->DumpRaster(dumpName, HRTopo->MinEl(), HRTopo->MaxEl(), true);	// use inner for elev reference
//Topo->DumpRaster(dumpName, 2322.0f, 2905.0f, true);	// use inner for elev reference
//sprintf(dumpName, "C:/InnerDEM_%dx%d.raw", HRTopo->LonEntries(), HRTopo->LatEntries());
//HRTopo->DumpRaster(dumpName, HRTopo->MinEl(), HRTopo->MaxEl(), true);
#endif // WCS_BUILD_FRANK

Topo->FreeRawElevs();

delete Topo;
Topo = NULL;

HRTopo->FreeRawElevs();

delete HRTopo;
HRTopo = NULL;

// F2 added 11/04/04
/***
NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(NBound);
NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(SBound);
NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(EBound);
NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(WBound);
HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(HiResN);
HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(HiResS);
HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(HiResE);
HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(HiResW);
***/

} // DEMMerger::MergeMultiRes

/*===========================================================================*/

void DEMMerger::UpdateBounds(unsigned long HiRes, Database *DBHost, Project *ProjHost, bool setRes)
{
double NBound, SBound, EBound, WBound, HiResN, HiResS, HiResE, HiResW, MergeXRes, MergeYRes, MergeXRes2, MergeYRes2, Divider;
double maxRes, minRes;
SearchQuery *Search;
Joe *MyTurn;
JoeCoordSys *MyAttr;
CoordSys *DEMCoordSys, *TempMergeSys;
EffectList *CurQuery;
DEM *ActiveDEM;
bool geographic = false, updated = false;

if (! Queries)
	return;

if (MergeCoordSys)
	TempMergeSys = MergeCoordSys;
else
	{
	TempMergeSys = GlobalApp->AppEffects->FetchDefaultCoordSys();
	} // else
TempMergeSys->GetGeographic();
DBHost->ResetGeoClip();

// reset our bounds
maxRes = FLT_MIN;
minRes = FLT_MAX;
if (HiRes)
	{
	HiResMergeXMax = HiResMergeYMax = -FLT_MAX;
	HiResMergeXMin = HiResMergeYMin = FLT_MAX;
	GoodBounds = FALSE;
	} // if
else
	{
	MergeXMax = MergeYMax = -FLT_MAX;
	MergeXMin = MergeYMin = FLT_MAX;
	GoodBounds = FALSE;
	} // else

CurQuery = Queries;
while (CurQuery)
	{
	if (Search = (SearchQuery *)CurQuery->Me)
		{
		for (MyTurn = DBHost->GetFirst(); MyTurn; MyTurn = DBHost->GetNext(MyTurn))
 			{
			if (MyTurn->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				if (Search->ApproveJoe(MyTurn))
					{
					/***
					(MyAttr = (JoeCoordSys *)MyTurn->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS));
					if (MyAttr && (MyAttr->Coord))
						{
						DEMCoordSys = MyAttr->Coord;
						if (ActiveDEM = new DEM)
							{
							if (ActiveDEM->AttemptLoadDEM(MyTurn, 1, ProjHost))
								{
								updated = true;
								if (HiRes)
									UpdateMergeBounds(ActiveDEM, HiResMergeXMin, HiResMergeYMin, 
										HiResMergeXMax, HiResMergeYMax);
								else
									UpdateMergeBounds(ActiveDEM, MergeXMin, MergeYMin, MergeXMax, MergeYMax);
								}
							delete ActiveDEM;
							ActiveDEM = NULL;
							} // if ActiveDEM
						} // if has CoordSys
					***/
					(MyAttr = (JoeCoordSys *)MyTurn->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS));
					if (MyAttr && (MyAttr->Coord))
						DEMCoordSys = MyAttr->Coord;
					else
						DEMCoordSys = BCS_CoordSys;
					if (ActiveDEM = new DEM)
						{
						if (ActiveDEM->AttemptLoadDEM(MyTurn, 1, ProjHost))
							{
							updated = true;
							if (setRes)
								{
								double tLat, tLon;

								if (DEMCoordSys->GetGeographic())
									DEMCoordSys->Geographic = 1;
								tLat = ActiveDEM->pLatStep;
								tLon = fabs(ActiveDEM->pLonStep);
								// if both CoordSys are either geographic or non-geographic, we're done with prep
								// if Merge = geographic and DEM = non-geographic
								if (TempMergeSys->Geographic && ! DEMCoordSys->Geographic)
									{
									// rough conversion to arc-seconds based on 30m = 1 arc second
									tLat /= 108000.0;
									tLon /= 108000.0;
									} // if
								// if Merge = non-geographic and DEM = geographic
								else if (! TempMergeSys->Geographic && DEMCoordSys->Geographic)
									{
									// rough conversion to arc-seconds based on 30m = 1 arc second
									tLat *= 108000.0;
									tLon *= 108000.0;
									} // else

								if (HiRes)
									{
									if (tLat < minRes)
										minRes = tLat;
									if (tLon < minRes)
										minRes = tLon;
									} // if
								else
									{
									if (tLat > maxRes)
										maxRes = tLat;
									if (tLon > maxRes)
										maxRes = tLon;
									} // else
								} // if
							if (HiRes)
								UpdateMergeBounds(ActiveDEM, HiResMergeXMin, HiResMergeYMin, HiResMergeXMax, HiResMergeYMax);
							else
								UpdateMergeBounds(ActiveDEM, MergeXMin, MergeYMin, MergeXMax, MergeYMax);
							}
						delete ActiveDEM;
						ActiveDEM = NULL;
						} // if ActiveDEM
					} // if approved
				} // if DEM
			} // for MyTurn
		} // if
	if (HiRes)
		break;
	CurQuery = CurQuery->Next;
	} // for Ct

if (updated)
	{
	if (setRes)
		{
		if (HiRes)
			{
			maxRes = max(AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEXRES].CurValue, AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEYRES].CurValue);
			AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_DIVIDER].CurValue = (int)(maxRes / minRes + 0.5);
			} // if
		else
			{
			AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEXRES].CurValue = maxRes;
			AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEYRES].CurValue = maxRes;
			} // else
		} // if
	MergeXRes = AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEXRES].CurValue;
	MergeYRes = AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEYRES].CurValue;
	MergeXRes2 = MergeXRes * 0.5;
	MergeYRes2 = MergeYRes * 0.5;
	Divider = AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_DIVIDER].CurValue;
	if (TempMergeSys->Geographic)
		geographic = true;
	if (HiRes)
		{
		HiResN = HiResMergeYMax;
		HiResS = HiResMergeYMin;
		if (geographic)
			{
			HiResE = HiResMergeXMin;
			HiResW = HiResMergeXMax;
			} // if
		else
			{
			HiResE = HiResMergeXMax;
			HiResW = HiResMergeXMin;
			} // else
		HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(HiResN);
		HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(HiResS);
		HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(HiResE);
		HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(HiResW);
		GoodBounds = TRUE;
		if (geographic)
			{
			double MergeGeoXRes, MergeGeoYRes;
			// figure out the size in degrees needed for the given cell size given in meters
			/* Removed and replaced by section below to match method in SetGoodBounds()
			MergeGeoXRes = MergeGeoYRes = TempMergeSys->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].CurValue * PiOver180;
			MergeGeoXRes = MergeXRes / MergeGeoXRes;
			//MergeGeoXRes *= cos(((HiResN + HiResS) * 0.5) / (360.0 / TwoPi));
			MergeGeoYRes = MergeYRes / MergeGeoYRes;
			HighResMergeHeight = (unsigned long)((HiResN - HiResS + MergeGeoYRes) * Divider / MergeGeoYRes);
			HighResMergeWidth = (unsigned long)((HiResW - HiResE + MergeGeoXRes) * Divider / MergeGeoXRes);
			*/ 
			MergeGeoXRes = MergeXRes;
			MergeGeoYRes = MergeYRes;
			HighResMergeHeight = (unsigned long)((HiResN - HiResS) / (MergeGeoYRes / Divider) + 1);
			HighResMergeWidth = (unsigned long)((HiResW - HiResE) / (MergeGeoXRes / Divider) + 1);
			} // if
		else
			{
			/* Removed and replaced by section below to match method in SetGoodBounds()
			HighResMergeHeight = (unsigned long)((HiResN - HiResS + MergeYRes) * Divider / MergeYRes);	// x / (y / z) = x * z / y
			HighResMergeWidth = (unsigned long)((HiResE - HiResW + MergeXRes) * Divider / MergeXRes);
			*/
			HighResMergeHeight = (unsigned long)((HiResN - HiResS) / (MergeYRes / Divider) + 1.0 + epsilon);
			HighResMergeWidth = (unsigned long)((HiResE - HiResW) / (MergeXRes / Divider) + 1.0 + epsilon);
			} // else
		HiResBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].ValueChanged();
		} // if
	else
		{
		NBound = MergeYMax;
		SBound = MergeYMin;
		// Round to nearest modulo outward
		NBound = ceil(NBound / MergeYRes) * MergeYRes;
		SBound = floor(SBound / MergeYRes) * MergeYRes;
		if (geographic)
			{
			EBound = MergeXMin;
			WBound = MergeXMax;
			// Round to nearest modulo outward
			EBound = floor((EBound + MergeXRes2) / MergeXRes) * MergeXRes;
			WBound = ceil(WBound / MergeXRes) * MergeXRes;
			} // if
		else
			{
			EBound = MergeXMax;
			WBound = MergeXMin;
			// Round to nearest modulo outward
			EBound = ceil(EBound / MergeXRes) * MergeXRes;
			WBound = floor(WBound / MergeXRes) * MergeXRes;
			} // else
		NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(NBound);
		NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(SBound);
		NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(EBound);
		NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(WBound);
		GoodBounds = TRUE;
		if (geographic)
			{
			/***
			double MergeGeoXRes, MergeGeoYRes;
			// figure out the size in degrees needed for the given cell size given in meters
			MergeGeoXRes = MergeGeoYRes = MergeCoordSys->Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].CurValue * PiOver180;
			MergeGeoXRes = MergeXRes / MergeGeoXRes;
			//MergeGeoXRes *= cos(((NBound + SBound) * 0.5) / (360.0 / TwoPi));
			MergeGeoYRes = MergeYRes / MergeGeoYRes;
			MergeHeight = (unsigned long)((NBound - SBound + MergeGeoYRes) / MergeGeoYRes);	// F2_NOTE: 070105 - Check these
			MergeWidth = (unsigned long)((WBound - EBound + MergeGeoXRes) / MergeGeoXRes);
			***/
			MergeHeight = (unsigned long)((NBound - SBound) / MergeYRes + 1 + epsilon);
			MergeWidth = (unsigned long)((WBound - EBound) / MergeXRes + 1 + epsilon);
			} // if
		else
			{
			MergeHeight = (unsigned long)((NBound - SBound) / MergeYRes + 1 + epsilon);
			MergeWidth = (unsigned long)((EBound - WBound) / MergeXRes + 1 + epsilon);
			} // else
		NormalBounds.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].ValueChanged();
		} // else
	} // if

} // DEMMerger::UpdateBounds

/*===========================================================================*/

void DEMMerger::ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds)
{

NormalBounds.ScaleToDEMBounds(OldBounds, CurBounds);
HiResBounds.ScaleToDEMBounds(OldBounds, CurBounds);

} // DEMMerger::ScaleToDEMBounds

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int DEMMerger::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
DEMMerger *CurrentMerger = NULL;
SearchQuery *CurrentQuery = NULL;
CoordSys *CurrentCoordSys = NULL;
DEMBounds OldBounds, CurBounds;

if (! ffile)
	return (0);

if (LoadToEffects = new EffectsLib())
	{
	if (LoadToImages = new ImageLib())
		{
		// set some global pointers so that things know what libraries to link to
		GlobalApp->LoadToEffectsLib = LoadToEffects;
		GlobalApp->LoadToImageLib = LoadToImages;

		while (BytesRead && Success)
			{
			// read block descriptor tag from file 
			if (BytesRead = ReadBlock(ffile, (char *)ReadBuf,
				WCS_BLOCKTYPE_CHAR + WCS_BLOCKSIZE_DOUBLE, ByteFlip))
				{
				TotalRead += BytesRead;
				ReadBuf[8] = 0;
				// read block size from file 
				if (BytesRead = ReadBlock(ffile, (char *)&Size,
					WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
					{
					TotalRead += BytesRead;
					BytesRead = 0;
					if (! strnicmp(ReadBuf, "DEMBnds", 8))
						{
						if ((BytesRead = OldBounds.Load(ffile, Size, ByteFlip)) == Size)
							OldBoundsLoaded = 1;
						} // if material
					else if (! strnicmp(ReadBuf, "Images", 8))
						{
						BytesRead = GlobalApp->LoadToImageLib->Load(ffile, Size, NULL);
						} // if material
					else if (! strnicmp(ReadBuf, "CoordSys", 8))
						{
						if (CurrentCoordSys = new CoordSys(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentCoordSys->Load(ffile, Size, ByteFlip);
							}
						} // if CoordSys
					else if (! strnicmp(ReadBuf, "Search", 8))
						{
						if (CurrentQuery = new SearchQuery(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentQuery->Load(ffile, Size, ByteFlip);
							}
						} // if search query
					else if (! strnicmp(ReadBuf, "DEMMerge", 8))
						{
						if (CurrentMerger = new DEMMerger(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentMerger->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if DEMMerger
					else if (! fseek(ffile, Size, SEEK_CUR))
						BytesRead = Size;
					TotalRead += BytesRead;
					if (BytesRead != Size)
						{
						Success = 0;
						break;
						} // if error
					} // if size block read 
				else
					break;
				} // if tag block read 
			else
				break;
			} // while 
		} // if image lib
	else
		Success = 0;
	} // if effects lib
else
	Success = 0;

if (Success == 1 && CurrentMerger)
	{
	if (EffectsLib::LoadQueries && OldBoundsLoaded && GlobalApp->AppDB->FillDEMBounds(&CurBounds))
		{
		if (UserMessageYN("Load DEM Merger", "Do you wish the loaded DEM Merger's bounds\n to be scaled to current DEM bounds?"))
			{
			CurrentMerger->ScaleToDEMBounds(&OldBounds, &CurBounds);
			CurrentMerger->SetGoodBoundsEtc();
			} // if
		} // if
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentMerger);
	strcpy(ReadBuf, Name);
	SetUniqueName(GlobalApp->AppEffects, ReadBuf);
	} // if

if (LoadToEffects)
	delete LoadToEffects;
if (LoadToImages)
	delete LoadToImages;
GlobalApp->CopyFromEffectsLib = GlobalApp->AppEffects;
GlobalApp->CopyFromImageLib = GlobalApp->AppImages;
GlobalApp->LoadToEffectsLib = GlobalApp->AppEffects;
GlobalApp->LoadToImageLib = GlobalApp->AppImages;

return (Success);

} // DEMMerger::LoadObject

/*===========================================================================*/

int DEMMerger::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
EffectList *CurEffect, *QueryList = NULL, *CoordsList = NULL;
DEMBounds CurBounds;

if (! ffile)
	return (0);

memset(StrBuf, 0, 9);

if (GlobalApp->AppDB->FillDEMBounds(&CurBounds))
	{
	strcpy(StrBuf, "DEMBnds");
	if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = CurBounds.Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if dem bounds saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if dem bounds

// Images
GlobalApp->AppImages->ClearRasterIDs();
if (InitImageIDs(ImageID))
	{
	strcpy(StrBuf, "Images");
	if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = GlobalApp->AppImages->Save(ffile, NULL, TRUE))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if Images saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if images

if (BuildFileComponentsList(&QueryList, &CoordsList))
	{
	#ifdef WCS_BUILD_VNS
	CurEffect = CoordsList;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "CoordSys");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((CoordSys *)CurEffect->Me)->Save(ffile))
						{
						TotalWritten += BytesWritten;
						fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
						if (WriteBlock(ffile, (char *)&BytesWritten,
							WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
							{
							fseek(ffile, 0, SEEK_END);
							} // if wrote size of block 
						else
							goto WriteError;
						} // if CoordSys saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		CurEffect = CurEffect->Next;
		} // while

	CurEffect = QueryList;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "Search");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((SearchQuery *)CurEffect->Me)->Save(ffile))
						{
						TotalWritten += BytesWritten;
						fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
						if (WriteBlock(ffile, (char *)&BytesWritten,
							WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
							{
							fseek(ffile, 0, SEEK_END);
							} // if wrote size of block 
						else
							goto WriteError;
						} // if SearchQuery saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		CurEffect = CurEffect->Next;
		} // while
	#endif // WCS_BUILD_VNS

	while (CoordsList)
		{
		CurEffect = CoordsList;
		CoordsList = CoordsList->Next;
		delete CurEffect;
		} // while
	while (QueryList)
		{
		CurEffect = QueryList;
		QueryList = QueryList->Next;
		delete CurEffect;
		} // while
	} // if

// DEM Merger
strcpy(StrBuf, "DEMMerge");
if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
	WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if DEM Merger saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

return (TotalWritten);

WriteError:

return (0);

} // DEMMerger::SaveObject

/*===========================================================================*/

void DEMMerger::ScanForRes(Database *DBHost)
{
double LatStep, LonStep, TmpLatStep, TmpLonStep, Radius;
SearchQuery *Query;
Joe *Clip;
DEM *Topo = NULL;
EffectList *CurQuery;
CoordSys *TempMergeSys;
unsigned long geoMerge = 0, i, set = 0;

if (MergeCoordSys)
	TempMergeSys = MergeCoordSys;
else
	{
	TempMergeSys = GlobalApp->AppEffects->FetchDefaultCoordSys();
	} // else
if (TempMergeSys->GetGeographic())	// Geographic?
	geoMerge = 1;

LatStep = LonStep = -FLT_MAX;
Radius = GlobalApp->AppEffects->GetPlanetRadius();
// run search queries on database and make a list of approved joes
CurQuery = Queries;
while (CurQuery)
	{
	if (Query = (SearchQuery *)CurQuery->Me)
		{
		i = 3;	// scan a maximum of 3 DEMs for each query
		for (Clip = DBHost->GetFirst(); Clip; Clip = DBHost->GetNext(Clip))
			{
			if (Clip->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				if (Query->ApproveJoe(Clip))
					{
					--i;
					if (i == 0)
						break;
					if (Topo = new DEM)
						{
						if (Topo->AttemptLoadDEM(Clip, 0, GlobalApp->MainProj))
							{
							if (geoMerge)
								{
								set = 1;
								if (Topo->LatStep() > LatStep)
									LatStep = Topo->LatStep();
								if (Topo->LonStep() > LonStep)
									LonStep = Topo->LonStep();
								} // if
							else if (Topo->GetDEMCellSizeMeters(TmpLatStep, TmpLonStep))
								{
								set = 1;
								// default to lowest res found
								if (TmpLatStep > LatStep)
									LatStep = TmpLatStep;
								if (TmpLonStep > LonStep)
									LonStep = TmpLonStep;
								} // if
							} // if loaded
						delete Topo;
						Topo = NULL;
						} // if Topo
					} // if approved
				} // if activated DEM
			} // for Clip
		} // if Query
	CurQuery = CurQuery->Next;
	} // while

if (! set)
	{
	if (geoMerge)
		LatStep = LonStep = 1/3600.0;	// 1 arc second
	else
		LatStep = LonStep = 30.0;	// 30m default
	} // if

AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEXRES].SetCurValue(LatStep);
AnimPar[WCS_EFFECTS_DEMMERGER_ANIMPAR_MERGEYRES].SetCurValue(LonStep);

} // DEMMerger::ScanForRes

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::DEMMerger_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
DEMMerger *Current;
int FloatingLoaded = 0;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new DEMMerger(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (DEMMerger *)FindDuplicateByName(Current->EffectType, Current))
								{
								RemoveRAHost(Current, 1);
								Current = NULL;
								} // if
							}
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read
			else
				break;
			} // if not done flag
		} // if tag block read
		else
			break;
	} // while 

return (TotalRead);

} // EffectsLib::DEMMerger_Load()

/*===========================================================================*/

ULONG EffectsLib::DEMMerger_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
DEMMerger *Current;

Current = Merger;
while (Current)
	{
	if (! Current->TemplateItem)
		{
		ItemTag = WCS_EFFECTSBASE_DATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = Current->Save(ffile))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if DEMMerger saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (DEMMerger *)Current->Next;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
goto WriteIt;

WriteError:
TotalWritten = 0UL;

WriteIt:
return (TotalWritten);

} // EffectsLib::DEMMerger_Save()

/*===========================================================================*/
