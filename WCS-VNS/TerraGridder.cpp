// TerraGridder.cpp
// For managing WCS Terrain Gridders
// Built from scratch on 3/1/00 by Gary R. Huber
// Copyright 2000 Questar Productions

#include "stdafx.h"
#include "Application.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Database.h"
#include "Useful.h"
#include "Requester.h"
#include "Raster.h"
#include "Project.h"
#include "Joe.h"
#include "EffectsIO.h"
#include "DBFilterEvent.h"
#include "Conservatory.h"
#include "TerraGridderEditGUI.h"
#include "Notify.h"
#include "AppMem.h"
#include "DEM.h"
#include "DataOpsDefs.h"
#include "Log.h"
#include "FeatureConfig.h"
#include "Security.h"
#include "DEFG/DEFG.h"
#include "Lists.h"

class DEFG;

TerraGridder::TerraGridder()
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_GRIDDER;
SetDefaults();

Defg = NULL;

} // TerraGridder::TerraGridder

/*===========================================================================*/

TerraGridder::TerraGridder(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_GRIDDER;
SetDefaults();

} // TerraGridder::TerraGridder

/*===========================================================================*/

TerraGridder::TerraGridder(RasterAnimHost *RAHost, EffectsLib *Library, TerraGridder *Proto)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_GRIDDER;
Prev = Library->LastGridder;
if (Library->LastGridder)
	{
	Library->LastGridder->Next = this;
	Library->LastGridder = this;
	} // if
else
	{
	Library->Gridder = Library->LastGridder = this;
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
	strcpy(NameBase, "Terrain Gridder");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // TerraGridder::TerraGridder

/*===========================================================================*/

TerraGridder::~TerraGridder()
{
DBFilterEvent *CurFilter;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->GRD && GlobalApp->GUIWins->GRD->GetActive() == this)
		{
		delete GlobalApp->GUIWins->GRD;
		GlobalApp->GUIWins->GRD = NULL;
		} // if
	} // if

if (NoiseMap)
	AppMem_Free(NoiseMap, NoiseSize);
NoiseMap = NULL;

while (Filters)
	{
	CurFilter = Filters;
	Filters = Filters->Next;
	delete CurFilter;
	} // while

} // TerraGridder::~TerraGridder

/*===========================================================================*/

void TerraGridder::SetDefaults(void)
{

if (Filters = new DBFilterEvent())
	SetFilterDefaults(Filters);

NRows = NCols = 100;
RandSeed = 1000;
Scope = 5;
NoiseSize = 0;
XPow = YPow = 0.0;
Delta = 100.0;
H = 1.0;
UseNoise = 0;
Units = 1;
EWMaps = NSMaps = 1;
Floating = 1;
Densify = 1;

NoiseMap = NULL;
//TC = NULL;
TCJoeList = NULL;
TCCount = 0;
TCJoeCount = 0;
Coords = NULL;

strcpy(NNG.grd_dir, GlobalApp->MainProj->dirname);

} // TerraGridder::SetDefaults

/*===========================================================================*/

void TerraGridder::SetFilterDefaults(DBFilterEvent *CurFilter)
{

CurFilter->PassVector = CurFilter->PassDEM = CurFilter->PassDisabled = 0;

} // TerraGridder::SetFilterDefaults

/*===========================================================================*/

void TerraGridder::Copy(TerraGridder *CopyTo, TerraGridder *CopyFrom)
{
DBFilterEvent *CurrentFrom = CopyFrom->Filters, **ToPtr, *CurFilter;
long Result = -1;
#ifdef WCS_BUILD_VNS
NotifyTag Changes[2];
#endif // WCS_BUILD_VNS

CopyTo->Coords = NULL;
#ifdef WCS_BUILD_VNS
if (CopyFrom->Coords)
	{
	if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
		{
		CopyTo->Coords = (CoordSys *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->Coords);
		} // if no need to make another copy, its all in the family
	else
		{
		if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->Coords->EffectType, CopyFrom->Coords->Name))
			{
			Result = UserMessageCustom("Copy Terrain Gridder", "How do you wish to resolve Coordinate System name collisions?\n\nLink to existing Coordinate Systems, replace existing Systems, or create new Systems?",
				"Link", "Create", "Overwrite", 1);
			} // if
		if (Result <= 0)
			{
			CopyTo->Coords = (CoordSys *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->Coords->EffectType, NULL, CopyFrom->Coords);
			} // if create new
		else if (Result == 1)
			{
			CopyTo->Coords = (CoordSys *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->Coords);
			} // if link to existing
		else if (CopyTo->Coords = (CoordSys *)GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->Coords->EffectType, CopyFrom->Coords->Name))
			{
			CopyTo->Coords->Copy(CopyTo->Coords, CopyFrom->Coords);
			Changes[0] = MAKE_ID(CopyTo->Coords->GetNotifyClass(), CopyTo->Coords->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, CopyTo->Coords);
			} // else if found and overwrite
		else
			{
			CopyTo->Coords = (CoordSys *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->Coords->EffectType, NULL, CopyFrom->Coords);
			} // else
		} // else better copy or overwrite it since its important to get just the right object
	} // if
#endif // WCS_BUILD_VNS

// delete existing filters
while (CopyTo->Filters)
	{
	CurFilter = CopyTo->Filters;
	CopyTo->Filters = CopyTo->Filters->Next;
	delete CurFilter;
	} // while

ToPtr = &CopyTo->Filters;

while (CurrentFrom)
	{
	if (*ToPtr = new DBFilterEvent())
		{
		(*ToPtr)->Copy(*ToPtr, CurrentFrom);
		} // if
	ToPtr = &(*ToPtr)->Next;
	CurrentFrom = CurrentFrom->Next;
	} // while

CopyTo->Floating = CopyFrom->Floating;
CopyTo->NNG.igrad = CopyFrom->NNG.igrad;
CopyTo->NNG.extrap = CopyFrom->NNG.extrap;
CopyTo->NNG.non_neg = CopyFrom->NNG.non_neg;
CopyTo->UseNoise = CopyFrom->UseNoise;
CopyTo->Densify = CopyFrom->Densify;
CopyTo->NNG.yterm = CopyFrom->NNG.yterm;
CopyTo->NNG.ystart = CopyFrom->NNG.ystart;
CopyTo->NNG.xstart = CopyFrom->NNG.xstart;
CopyTo->NNG.xterm = CopyFrom->NNG.xterm;
CopyTo->NNG.horilap = CopyFrom->NNG.horilap;
CopyTo->NNG.vertlap = CopyFrom->NNG.vertlap;
CopyTo->NNG.x_nodes = CopyFrom->NNG.x_nodes;
CopyTo->NNG.y_nodes = CopyFrom->NNG.y_nodes;
CopyTo->EWMaps = CopyFrom->EWMaps;
CopyTo->NSMaps = CopyFrom->NSMaps;
CopyTo->NNG.nuldat = CopyFrom->NNG.nuldat;
CopyTo->NNG.bI = CopyFrom->NNG.bI;
CopyTo->NNG.bJ = CopyFrom->NNG.bJ;
CopyTo->RandSeed = CopyFrom->RandSeed;
CopyTo->Scope = CopyFrom->Scope;
CopyTo->Delta = CopyFrom->Delta;
CopyTo->H = CopyFrom->H;
strcpy(CopyTo->NNG.grd_file, CopyFrom->NNG.grd_file);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // TerraGridder::Copy

/*===========================================================================*/

ULONG TerraGridder::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
DBFilterEvent *CurFilter, **LoadTo = &Filters;
char CoordsName[256];

while (Filters)
	{
	CurFilter = Filters;
	Filters = Filters->Next;
	delete CurFilter;
	} // while

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
					case WCS_EFFECTS_PRIORITY:
						{
						BytesRead = ReadBlock(ffile, (char *)&Priority, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_FLOATING:
						{
						BytesRead = ReadBlock(ffile, (char *)&Floating, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_GRADIENTS:
						{
						BytesRead = ReadBlock(ffile, (char *)&NNG.igrad, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_EXTRAPOLATE:
						{
						BytesRead = ReadBlock(ffile, (char *)&NNG.extrap, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_NONNEG:
						{
						BytesRead = ReadBlock(ffile, (char *)&NNG.non_neg, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_USENOISE:
						{
						BytesRead = ReadBlock(ffile, (char *)&UseNoise, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_DENSIFY:
						{
						BytesRead = ReadBlock(ffile, (char *)&Densify, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_NORTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&NNG.yterm, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_SOUTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&NNG.ystart, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_WEST:
						{
						BytesRead = ReadBlock(ffile, (char *)&NNG.xstart, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_EAST:
						{
						BytesRead = ReadBlock(ffile, (char *)&NNG.xterm, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_HORILAP:
						{
						BytesRead = ReadBlock(ffile, (char *)&NNG.horilap, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_VERTLAP:
						{
						BytesRead = ReadBlock(ffile, (char *)&NNG.vertlap, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_XNODES:
						{
						BytesRead = ReadBlock(ffile, (char *)&NNG.x_nodes, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_YNODES:
						{
						BytesRead = ReadBlock(ffile, (char *)&NNG.y_nodes, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_EWMAPS:
						{
						BytesRead = ReadBlock(ffile, (char *)&EWMaps, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_NSMAPS:
						{
						BytesRead = ReadBlock(ffile, (char *)&NSMaps, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_NULLDAT:
						{
						BytesRead = ReadBlock(ffile, (char *)&NNG.nuldat, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					#ifdef WCS_BUILD_VNS
					case WCS_EFFECTS_GRIDDER_DEFGSMOOTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&NNG.bI, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					#else // !VNS
					case WCS_EFFECTS_GRIDDER_BI:
						{
						BytesRead = ReadBlock(ffile, (char *)&NNG.bI, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					#endif // !VNS
					case WCS_EFFECTS_GRIDDER_BJ:
						{
						BytesRead = ReadBlock(ffile, (char *)&NNG.bJ, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_RANDSEED:
						{
						BytesRead = ReadBlock(ffile, (char *)&RandSeed, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_SCOPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Scope, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_DELTA:
						{
						BytesRead = ReadBlock(ffile, (char *)&Delta, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_H:
						{
						BytesRead = ReadBlock(ffile, (char *)&H, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_DEMNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)NNG.grd_file, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GRIDDER_DBFILTER:
						{
						if (*LoadTo = new DBFilterEvent())
							{
							BytesRead = (*LoadTo)->Load(ffile, Size, ByteFlip);
							LoadTo = &(*LoadTo)->Next;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_GRIDDER_COORDSYSNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)CoordsName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						#ifdef WCS_BUILD_VNS
						if (CoordsName[0])
							{
							Coords = (CoordSys *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_COORDSYS, CoordsName);
							} // if
						#endif // WCS_BUILD_VNS
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

} // TerraGridder::Load

/*===========================================================================*/

unsigned long int TerraGridder::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
DBFilterEvent *Current;

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
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_PRIORITY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Priority)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_FLOATING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Floating)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_GRADIENTS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NNG.igrad)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_EXTRAPOLATE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NNG.extrap)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_NONNEG, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NNG.non_neg)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_USENOISE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&UseNoise)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_DENSIFY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Densify)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_NORTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NNG.yterm)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_SOUTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NNG.ystart)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_WEST, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NNG.xstart)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_EAST, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NNG.xterm)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_HORILAP, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NNG.horilap)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_VERTLAP, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NNG.vertlap)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_XNODES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NNG.x_nodes)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_YNODES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NNG.y_nodes)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_EWMAPS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&EWMaps)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_NSMAPS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NSMaps)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_NULLDAT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NNG.nuldat)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
#ifdef WCS_BUILD_VNS
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_DEFGSMOOTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NNG.bI)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
#else // !VNS
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_BI, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NNG.bI)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
#endif // !VNS
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_BJ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NNG.bJ)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_RANDSEED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&RandSeed)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_SCOPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Scope)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_DELTA, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Delta)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_H, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&H)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_DEMNAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(NNG.grd_file) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)NNG.grd_file)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

Current = Filters;
while (Current)
	{
	ItemTag = WCS_EFFECTS_GRIDDER_DBFILTER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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
				} // if DBFilterEvent saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	Current = Current->Next;
	} // while

#ifdef WCS_BUILD_VNS
if (Coords)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GRIDDER_COORDSYSNAME, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Coords->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Coords->GetName())) == NULL)
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

} // TerraGridder::Save

/*===========================================================================*/

void TerraGridder::Edit(void)
{

DONGLE_INLINE_CHECK()
if (GlobalApp->GUIWins->GRD)
	{
	delete GlobalApp->GUIWins->GRD;
	}
GlobalApp->GUIWins->GRD = new TerraGridderEditGUI(GlobalApp->AppEffects, GlobalApp->MainProj, GlobalApp->AppDB, this);
if (GlobalApp->GUIWins->GRD)
	{
	GlobalApp->GUIWins->GRD->Open(GlobalApp->MainProj);
	}

} // TerraGridder::Edit

/*===========================================================================*/

DBFilterEvent *TerraGridder::AddFilter(DBFilterEvent *AddMe)
{
DBFilterEvent **CurFilter = &Filters;
NotifyTag Changes[2];

while (*CurFilter)
	{
	CurFilter = &(*CurFilter)->Next;
	} // while
if (*CurFilter = new DBFilterEvent())
	{
	if (AddMe)
		(*CurFilter)->Copy(*CurFilter, AddMe);
	else
		SetFilterDefaults(*CurFilter);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
return (*CurFilter);

} // TerraGridder::AddFilter

/*===========================================================================*/

void TerraGridder::RemoveFilter(DBFilterEvent *RemoveMe)
{
DBFilterEvent *CurFilter = Filters, *PrevFilter = NULL;
NotifyTag Changes[2];

while (CurFilter)
	{
	if (CurFilter == (DBFilterEvent *)RemoveMe)
		{
		if (PrevFilter)
			PrevFilter->Next = CurFilter->Next;
		else
			Filters = CurFilter->Next;

		delete CurFilter;

		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

		return;
		} // if
	PrevFilter = CurFilter;
	CurFilter = CurFilter->Next;
	} // while

} // TerraGridder::RemoveFilter

/*===========================================================================*/

char TerraGridder::GetRAHostDropOK(long DropType)
{

#ifdef WCS_BUILD_VNS
if (DropType == WCS_EFFECTSSUBCLASS_COORDSYS)
	return (1);
#endif // WCS_BUILD_VNS
if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);

return (0);

} // TerraGridder::GetRAHostDropOK

/*===========================================================================*/

int TerraGridder::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
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
	if (this != (TerraGridder *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (TerraGridder *)DropSource->DropSource);
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

} // TerraGridder::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long TerraGridder::GetRAFlags(unsigned long Mask)
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
if (Mask & WCS_RAHOST_FLAGBIT_CHILDREN)
	{
	//Flags |= WCS_RAHOST_FLAGBIT_CHILDREN;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_TERRAINGRID | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // TerraGridder::GetRAFlags

/*===========================================================================*/

RasterAnimHost *TerraGridder::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Found = 0;

if (! Current)
	Found = 1;
#ifdef WCS_BUILD_VNS
if (Found && Coords)
	return (Coords);
#endif // WCS_BUILD_VNS

return (NULL);

} // TerraGridder::GetRAHostChild

/*===========================================================================*/

int TerraGridder::GetDeletable(RasterAnimHost *Test)
{

if (Test == Coords)
	return (1);

return (0);

} // TerraGridder::GetDeletable

/*===========================================================================*/

int TerraGridder::SetCoords(CoordSys *NewCoords)
{
NotifyTag Changes[2];

Coords = NewCoords;

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (1);

} // TerraGridder::SetCoords

/*===========================================================================*/

int TerraGridder::RemoveRAHost(RasterAnimHost *RemoveMe)
{
NotifyTag Changes[2];

if (Coords == (CoordSys *)RemoveMe)
	{
	Coords = NULL;

	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

	return (1);
	} // if

return (GeneralEffect::RemoveRAHost(RemoveMe));

} // TerraGridder::RemoveRAHost

/*===========================================================================*/

long TerraGridder::InitImageIDs(long &ImageID)
{
long NumImages = 0;

if (Coords)
	NumImages += Coords->InitImageIDs(ImageID);
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // TerraGridder::InitImageIDs

/*===========================================================================*/

int TerraGridder::BuildFileComponentsList(EffectList **CoordSystems)
{
EffectList **ListPtr;

if (Coords)
	{
	ListPtr = CoordSystems;
	while (*ListPtr)
		{
		if ((*ListPtr)->Me == Coords)
			break;
		ListPtr = &(*ListPtr)->Next;
		} // if
	if (! (*ListPtr))
		{
		if (*ListPtr = new EffectList())
			(*ListPtr)->Me = Coords;
		else
			return (0);
		} // if
	} // if

return (1);

} // TerraGridder::BuildFileComponentsList

/*===========================================================================*/

static int GettingBounds;

int TerraGridder::GetDefaultBounds(Database *DBHost)
{
int Approved, Geographic = 1;
double TestNorth, TestWest, TestSouth, TestEast;
DBFilterEvent *CurFilter;
Joe *CurJoe;
NotifyTag Changes[2];

// prevent recursion during ConfigureWidgets();
if (GettingBounds) return(!Floating);

GettingBounds = 1;
DBHost->ResetGeoClip();

#ifdef WCS_BUILD_VNS
Geographic = ! (Coords && Coords->Method.GCTPMethod);
#endif // WCS_BUILD_VNS

if (Geographic)
	{
	NNG.yterm = -FLT_MAX;	// north
	NNG.ystart = FLT_MAX;	// south
	NNG.xstart = -FLT_MAX;	// west
	NNG.xterm = FLT_MAX;	// east
	} // if
else
	{
	NNG.yterm = -FLT_MAX;	// north
	NNG.ystart = FLT_MAX;	// south
	NNG.xstart = FLT_MAX;	// west
	NNG.xterm = -FLT_MAX;	// east
	} // else

for (CurJoe = DBHost->GetFirst(); CurJoe; CurJoe = DBHost->GetNext(CurJoe))
	{
	Approved = 0;
	CurFilter = Filters;
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
		if (Coords)
			{
			if (CurJoe->GetBoundsProjected(Coords, TestNorth, TestWest, TestSouth, TestEast, false))
				{
				if (TestNorth > NNG.yterm)
					NNG.yterm = TestNorth;
				if (TestSouth < NNG.ystart)
					NNG.ystart = TestSouth;
				if (Geographic)
					{
					if (TestWest > NNG.xstart)
						NNG.xstart = TestWest;
					if (TestEast < NNG.xterm)
						NNG.xterm = TestEast;
					} // if
				else
					{
					if (TestWest < NNG.xstart)
						NNG.xstart = TestWest;
					if (TestEast > NNG.xterm)
						NNG.xterm = TestEast;
					} // else
				} // if
			} // if
		else
			{
			CurJoe->GetTrueBounds(TestNorth, TestWest, TestSouth, TestEast);
			if (TestNorth > NNG.yterm)
				NNG.yterm = TestNorth;
			if (TestSouth < NNG.ystart)
				NNG.ystart = TestSouth;
			if (TestWest > NNG.xstart)
				NNG.xstart = TestWest;
			if (TestEast < NNG.xterm)
				NNG.xterm = TestEast;
			} // if
		} // if
	} // for

if (NNG.yterm <= NNG.ystart)
	{
	if (Geographic)
		NNG.yterm = 1.0;
	else
		NNG.yterm = 10.0;
	NNG.ystart = 0.0;
	Floating = 1;
	} // if
if (Geographic && NNG.xstart <= NNG.xterm)
	{
	NNG.xstart = 1.0;
	NNG.xterm = 0.0;
	Floating = 1;
	} // if
else if (! Geographic && NNG.xstart >= NNG.xterm)
	{
	NNG.xstart = 0.0;
	NNG.xterm = 10.0;
	Floating = 1;
	} // if

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

GettingBounds = 0;
return (! Floating);

} // TerraGridder::GetDefaultBounds

/*===========================================================================*/

void TerraGridder::SetBounds(double LatRange[2], double LonRange[2])
{
NotifyTag Changes[2];
VertexDEM Vert;
double DataCoords[8];

// this will set new grid lat/lon bounds
/*
// ensure that latitude is within bounds
if (LatRange[0] > 90.0)
	LatRange[0] = 90.0;
if (LatRange[0] < -90.0)
	LatRange[0] = -90.0;
if (LatRange[1] > 90.0)
	LatRange[1] = 90.0;
if (LatRange[1] < -90.0)
	LatRange[1] = -90.0;
// can't use the bounds if they are equal
if (LatRange[0] != LatRange[1] && LonRange[0] != LonRange[1])
	{
	if (LatRange[1] < LatRange[0])
		swmem(&LatRange[0], &LatRange[1], sizeof (double));
	if (LonRange[1] < LonRange[0])
		swmem(&LonRange[0], &LonRange[1], sizeof (double));
	// if bounds appear to wrap more than halfway around earth then probably 
	// want to take the smaller arc
	if (fabs(LonRange[1] - LonRange[0]) > 180.0)
		{
		LonRange[1] -= 360.0;
		} // if
	NNG.yterm = LatRange[1];
	NNG.ystart = LatRange[0];
	NNG.xstart = LonRange[1];
	NNG.xterm = LonRange[0];
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
*/
// ensure that latitude is within bounds
if (LatRange[0] > 90.0)
	LatRange[0] = 90.0;
if (LatRange[0] < -90.0)
	LatRange[0] = -90.0;
if (LatRange[1] > 90.0)
	LatRange[1] = 90.0;
if (LatRange[1] < -90.0)
	LatRange[1] = -90.0;
// can't use the bounds if they are equal
if (LatRange[0] != LatRange[1] && LonRange[0] != LonRange[1])
	{
	if (LatRange[1] < LatRange[0])
		swmem(&LatRange[0], &LatRange[1], sizeof (double));
	if (LonRange[1] < LonRange[0])
		swmem(&LonRange[0], &LonRange[1], sizeof (double));
	// if bounds appear to wrap more than halfway around earth then probably 
	// want to take the smaller arc
	if (fabs(LonRange[1] - LonRange[0]) > 180.0)
		{
		LonRange[1] -= 360.0;
		} // if
	// unproject all corners and figure out which ones to use
	Vert.Lat = DataCoords[1] = LatRange[0];
	Vert.Lon = DataCoords[0] = LonRange[0];
	if (Coords)
		{
		Coords->DefDegToProj(&Vert);
		DataCoords[0] = Vert.xyz[0];
		DataCoords[1] = Vert.xyz[1];
		} // if 

	Vert.Lat = DataCoords[3] = LatRange[0];
	Vert.Lon = DataCoords[2] = LonRange[1];
	if (Coords)
		{
		Coords->DefDegToProj(&Vert);
		DataCoords[2] = Vert.xyz[0];
		DataCoords[3] = Vert.xyz[1];
		} // if 

	Vert.Lat = DataCoords[5] = LatRange[1];
	Vert.Lon = DataCoords[4] = LonRange[0];
	if (Coords)
		{
		Coords->DefDegToProj(&Vert);
		DataCoords[4] = Vert.xyz[0];
		DataCoords[5] = Vert.xyz[1];
		} // if 

	Vert.Lat = DataCoords[7] = LatRange[1];
	Vert.Lon = DataCoords[6] = LonRange[1];
	if (Coords)
		{
		Coords->DefDegToProj(&Vert);
		DataCoords[6] = Vert.xyz[0];
		DataCoords[7] = Vert.xyz[1];
		} // if 

	LatRange[0] = min(DataCoords[1], DataCoords[3]);
	LatRange[0] = min(LatRange[0], DataCoords[5]);
	LatRange[0] = min(LatRange[0], DataCoords[7]);
	LatRange[1] = max(DataCoords[1], DataCoords[3]);
	LatRange[1] = max(LatRange[1], DataCoords[5]);
	LatRange[1] = max(LatRange[1], DataCoords[7]);
	if (! Coords || ! Coords->Method.GCTPMethod)
		{
		LonRange[0] = min(DataCoords[0], DataCoords[2]);
		LonRange[0] = min(LonRange[0], DataCoords[4]);
		LonRange[0] = min(LonRange[0], DataCoords[6]);
		LonRange[1] = max(DataCoords[0], DataCoords[2]);
		LonRange[1] = max(LonRange[1], DataCoords[4]);
		LonRange[1] = max(LonRange[1], DataCoords[6]);
		} // if
	else
		{
		LonRange[0] = max(DataCoords[0], DataCoords[2]);
		LonRange[0] = max(LonRange[0], DataCoords[4]);
		LonRange[0] = max(LonRange[0], DataCoords[6]);
		LonRange[1] = min(DataCoords[0], DataCoords[2]);
		LonRange[1] = min(LonRange[1], DataCoords[4]);
		LonRange[1] = min(LonRange[1], DataCoords[6]);
		} // else

	NNG.yterm = LatRange[1];
	NNG.ystart = LatRange[0];
	NNG.xstart = LonRange[1];
	NNG.xterm = LonRange[0];
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if


} // TerraGridder::SetBounds

/*===========================================================================*/

void TerraGridder::SetFloating(char NewFloating)
{

if (Floating && NewFloating)
	{
	GetDefaultBounds(GlobalApp->AppDB);
	} // if
else if (! NewFloating)
	Floating = 0;

} // TerraGridder::SetFloating

/*===========================================================================*/

int TerraGridder::MakeGrid(EffectsLib *EffectsHost, Project *ProjHost, Database *DBHost)
{
double MapWidth, MapHeight, OrigNorth, OrigSouth, OrigEast, OrigWest;
short KeepGoing = 1;
long MapRow, MapCol, MapNum, MaxNumMaps;
BOOL restricted = FALSE;
double TGXStart, TGXTerm, TGYStart, TGYTerm;
double GridBeginTime, GridEndTime, GridElapsedTime = 0.0;
unsigned long int NumTiles = 0;
float *RowUnify = NULL, *ColUnify = NULL;
unsigned long int ColUnifySize = 0, RowUnifySize = 0;
int RetVal = 0;
char BaseName[64];
char TimeMsg[200];
char Proceed = 0;

NumTiles = EWMaps * NSMaps;

GridBeginTime = GetSystemTimeFP();


if (NumTiles > 1) 	// we'll try allocating points on a per-tile basis
	{ // allocate 2d edge unification buffers
	unsigned long int FullNumCols, FullNumRows, NumUnifyRows, NumUnifyCols, InitLoop;
	NumUnifyRows = NSMaps + 1; // fence post issue
	NumUnifyCols = EWMaps + 1; // fence post issue
	FullNumRows = (NSMaps * (NNG.y_nodes - 1)) + 1;
	FullNumCols = (EWMaps * (NNG.x_nodes - 1)) + 1;
	RowUnifySize = FullNumCols * NumUnifyRows;
	ColUnifySize = FullNumRows * NumUnifyCols;
	if (RowUnify = (float *)AppMem_Alloc(RowUnifySize * sizeof(float), APPMEM_CLEAR))
		{
		for(InitLoop = 0; InitLoop < RowUnifySize; InitLoop++)
			{
			RowUnify[InitLoop] = -FLT_MAX;
			} // for
		if (ColUnify = (float *)AppMem_Alloc(ColUnifySize * sizeof(float), APPMEM_CLEAR))
			{
			for(InitLoop = 0; InitLoop < ColUnifySize; InitLoop++)
				{
				ColUnify[InitLoop] = -FLT_MAX;
				} // for
			Proceed = 1;
			} // if
		} // if
	} // if
else
	{
	Proceed = (AllocControlPts(DBHost) ? 1 : 0);
	} // else

if (Proceed)
	{
	#ifdef WCS_BUILD_DEMO
	if (EWMaps > 1)
		{
		restricted = TRUE;
		EWMaps = 1;
		}
	if (NSMaps > 1)
		{
		restricted = TRUE;
		NSMaps = 1;
		}
	if (NNG.x_nodes > 300)
		{
		restricted = TRUE;
		NNG.x_nodes = 300;
		}
	if (NNG.y_nodes > 300)
		{
		restricted = TRUE;
		NNG.y_nodes = 300;
		}
	#endif
	if (Coords && Coords->Method.GCTPMethod)
		{
		// projected, just copy values over
		TGXStart = simplemin(NNG.xstart, NNG.xterm);
		TGXTerm  = simplemax(NNG.xstart, NNG.xterm);
		TGYStart = simplemin(NNG.ystart, NNG.yterm);
		TGYTerm  = simplemax(NNG.ystart, NNG.yterm);
		} // if
	else
		{
		// geographic, negate X (Lon) from WCS poswest to GIS poseast
		TGXStart = simplemin(-NNG.xstart, -NNG.xterm);
		TGXTerm  = simplemax(-NNG.xstart, -NNG.xterm);
		TGYStart = simplemin(NNG.ystart, NNG.yterm);
		TGYTerm  = simplemax(NNG.ystart, NNG.yterm);
		} // else
	MapWidth = (TGXTerm - TGXStart) / EWMaps;
	MapHeight = (TGYTerm - TGYStart) / NSMaps;
	OrigNorth = TGYTerm;
	OrigSouth = TGYStart;
	OrigEast = TGXTerm;
	OrigWest = TGXStart;

	TGXTerm = TGXStart;

	strcpy(BaseName, NNG.grd_file);
	MaxNumMaps = EWMaps * NSMaps;

	if (restricted)
		UserMessageDemo("The gridding size has been reduced for this demo.");

	Proceed = 0;
	for (MapCol = 0; MapCol < EWMaps; MapCol ++)
		{
		TGXStart = TGXTerm;
		TGXTerm += MapWidth;
		TGYTerm = OrigSouth;

		for (MapRow = 0; MapRow < NSMaps; MapRow ++)
			{
			TGYStart = TGYTerm;
			TGYTerm += MapHeight;
			MapNum = MapCol * NSMaps + MapRow;
			if (MaxNumMaps > 1)
				{
				if (MaxNumMaps < 10)
					sprintf(NNG.grd_file, "%s%01d", BaseName, MapNum);
				else if (MaxNumMaps < 100)
					sprintf(NNG.grd_file, "%s%02d", BaseName, MapNum);
				else if (MaxNumMaps < 1000)
					sprintf(NNG.grd_file, "%s%03d", BaseName, MapNum);
				else if (MaxNumMaps < 10000)
					sprintf(NNG.grd_file, "%s%04d", BaseName, MapNum);
				else if (MaxNumMaps < 100000)
					sprintf(NNG.grd_file, "%s%05d", BaseName, MapNum);
				else
					sprintf(NNG.grd_file, "%s%08d", BaseName, MapNum);

				// allocate points only for this tile
				FreeControlPts();
				Proceed = (AllocControlPts(DBHost) ? 1 : 0);
				} // if
			else
				{
				Proceed = 1;
				} // else
			if (Proceed && InitializeNNGrid())
				{
				Units = 1;	// meters
				if (MaxNumMaps > 1)
					{
					Defg->SetupUnifyInfo(RowUnify, ColUnify, MapRow, MapCol, NSMaps, EWMaps);
					} // if
				else
					{
					// clear these fields so we don't accidentally try to Unify edges when we shouldn't
					Defg->SetupUnifyInfo(NULL, NULL, 0, 0, 0, 0);
					} // else
				KeepGoing = Defg->DoGrid(DBHost, ProjHost, EffectsHost, this, TCJoeList, TCJoeCount, TCCount, &NNG, TGXStart, TGXTerm, TGYStart, TGYTerm, Coords);
				UnInitializeNNGrid();
				if (! KeepGoing)
					break;
				} // if
			} // for
		if (! KeepGoing)
			break;
		} // for

	strcpy(NNG.grd_file, BaseName);
	FreeControlPts();

	GridEndTime = GetSystemTimeFP();
	GridElapsedTime = (GridEndTime - GridBeginTime);
	sprintf(TimeMsg, "Grid Job Total Points: %d.\n", TCCount);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, TimeMsg);
	sprintf(TimeMsg, "Grid Job Elapsed Time: %f seconds.\n", GridElapsedTime);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, TimeMsg);
	RetVal = KeepGoing;
	} // if

if (RowUnify)
	{
	AppMem_Free(RowUnify, RowUnifySize * sizeof(float));
	RowUnify = NULL;
	} // if
if (ColUnify)
	{
	AppMem_Free(ColUnify, ColUnifySize * sizeof(float));
	ColUnify = NULL;
	} // if

return(RetVal);

} // TerraGridder::MakeGrid

/*===========================================================================*/

int TerraGridder::AllocControlPts(Database *DBHost)
{
int Approved, DontGrid = 0;
long PtsAdded = 0;
DBFilterEvent *CurFilter;
Joe *CurJoe;
VectorPoint *PLink;
BusyWin *BWRB = NULL;
int ReStep = 0, AlreadyCounted = 0, TCJoeTempCount = 0, JoeCount = 0, PointCount = 0, PointLoop;
//JoeCoordSys *MyAttr;
//CoordSys *MyCoords;
VertexDEM CurVert;

double StartTime, EndTime, ElapsedTime = 0.0;

// here is where we scan the database for in-bounds vertices

DBHost->ResetGeoClip();

StartTime = GetSystemTimeFP();

for(PointCount = AlreadyCounted = 0; AlreadyCounted < 2; AlreadyCounted++)
	{
	if (AlreadyCounted)
		{
		if (PointCount)
			{
			// need to allocate block
			//TC = new Point3d[PointCount];
			TCJoeList = new Joe *[JoeCount];
			TCCount = PointCount;
			TCJoeCount = JoeCount;
			if (!TCJoeList)
				{ // alloc failed
				return(0);
				} // if
			else
				{
				// can't recall if operator new [] is guaranteed to clear array
				memset(TCJoeList, 0, (JoeCount * sizeof(Joe *)));
				} // else
			PointLoop = 0;
			// two passes through the dataset, one to count, one to process
			if (BWRB) delete BWRB;
			BWRB = NULL;
			BWRB = new BusyWin("Selecting Objects", DBHost->StaticRoot->GroupInfo.NumKids, 'BWSE', 0);
			} // if
		else
			{
			break; // no points
			} // else
		} // if
	else
		{
		// two passes through the dataset, one to count, one to process
		BWRB = new BusyWin("Counting Objects", DBHost->StaticRoot->GroupInfo.NumKids, 'BWSE', 0);
		} // else
	ReStep = 0;
	for (CurJoe = DBHost->GetFirst(); CurJoe && ! DontGrid; CurJoe = DBHost->GetNext(CurJoe))
		{
		if (CurJoe->GetFirstRealPoint())
			{
			Approved = 0;
			CurFilter = Filters;
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
				if (AlreadyCounted)
					{ // do the real work
					TCJoeList[TCJoeTempCount++] = CurJoe;
					PtsAdded += CurJoe->GetNumRealPoints();
					/*
					if (MyAttr = (JoeCoordSys *)CurJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
						MyCoords = MyAttr->Coord;
					else
						MyCoords = NULL;
					for (PLink = CurJoe->Points->Next; PLink; PLink = PLink->Next)
						{
						if (PLink->ProjToDefDeg(MyCoords, &CurVert))
							{
							//TC[PtsAdded][0] = CurVert.Lon;
							//TC[PtsAdded][1] = CurVert.Lat;
							//TC[PtsAdded++][2] = CurVert.Elev;
							} // if
						else
							{
							UserMessageOK((char *)CurJoe->GetBestName(), "Unable to unproject vector vertices.");
							DontGrid = 1;
							break;
							} // else
						} // for
					*/
					} // if
				else
					{ // just count
					JoeCount++;
					for (PLink = CurJoe->GetFirstRealPoint(); PLink; PLink = PLink->Next)
						{
						PointCount++;
						} // for
					} // else
				} // if
			} // if
		if (BWRB && BWRB->Update(++ReStep))
			{
			CurJoe = NULL;
			} // if
		} // for
	} // for

if (BWRB) delete BWRB;
BWRB = NULL;

EndTime = GetSystemTimeFP();
ElapsedTime = (EndTime - StartTime);


if (PtsAdded < 3)
	{
	if ((EWMaps * NSMaps) == 1) // don't do a modal stop if we're gridding multiple tiles
		{
		UserMessageOK("Terrain Gridder", "The gridder must have at least three\nElevation Control Points in order to make a DEM.");
		} // if
	else
		{
		char TilePointsMsg[200], ObjName[64], BaseName[64];
		strcpy(ObjName, NNG.grd_file);
		strcpy(BaseName, ObjName);
		//strcat(ObjName, ".elev");
		sprintf(TilePointsMsg, "Terrain Gridder: %s -- The gridder must have at least three Elevation Control Points in order to make a DEM.", ObjName);
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, TilePointsMsg);


		} // else
	FreeControlPts();
	return (0);
	} // if
if (DontGrid)
	{
	UserMessageOK("Terrain Gridder", "An error was encountered projecting vertices. Gridding has been halted.");
	FreeControlPts();
	return (0);
	} // if

return (1);

} // TerraGridder::AllocControlPts

/*===========================================================================*/

void TerraGridder::FreeControlPts(void)
{

if (TCJoeList)
	{
	delete [] TCJoeList;
	TCJoeList = NULL;
	TCJoeCount = 0;
	} //while

} // TerraGridder::FreeControlPts

/*===========================================================================*/

int TerraGridder::InitializeNNGrid(void)
{

if (NNG.x_nodes <= 1 || NNG.y_nodes <= 1)
	{
	UserMessageOK("Terrain Gridder", "Output rows and columns must both be greater than one!\nOperation terminated.");
	return (0);
	} // if

Defg = new DEFG;

return (1);
} // TerraGridder::InitializeNNGrid

/*===========================================================================*/

void TerraGridder::UnInitializeNNGrid(void)
{
delete Defg;
Defg = NULL;
} // TerraGridder::UnInitializeNNGrid

/*===========================================================================*/

int TerraGridder::SetImportData(struct ImportData *ImpData)
{

if (ImpData)
	{
	NNG.yterm = ImpData->OutputHighLat;
	NNG.ystart = ImpData->OutputLowLat;
	NNG.xterm = ImpData->OutputLowLon;
	NNG.xstart = ImpData->OutputHighLon;
	NNG.x_nodes = ImpData->OutputCols;
	NNG.y_nodes = ImpData->OutputRows;
	EWMaps = ImpData->OutputWEMaps;
	NSMaps = ImpData->OutputNSMaps;
	strncpy(NNG.grd_file, ImpData->OutName, 64);
	NNG.grd_file[63] = 0;
	return (1);
	} // if

return (0);

} // TerraGridder::SetImportData() 

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int TerraGridder::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
TerraGridder *CurrentTerraGridder = NULL;
#ifdef WCS_BUILD_VNS
CoordSys *CurrentCoords = NULL;
#endif // WCS_BUILD_VNS
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
						} // if DEMBnds
					#ifdef WCS_BUILD_VNS
					else if (! strnicmp(ReadBuf, "CoordSys", 8))
						{
						if (CurrentCoords = new CoordSys(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentCoords->Load(ffile, Size, ByteFlip);
							}
						} // if Camera
					#endif // WCS_BUILD_VNS
					else if (! strnicmp(ReadBuf, "Gridder", 8))
						{
						if (CurrentTerraGridder = new TerraGridder(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentTerraGridder->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if TerraGridder
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

if (Success == 1 && CurrentTerraGridder)
	{
	strcpy(CurrentTerraGridder->NNG.grd_dir, GlobalApp->MainProj->dirname);
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentTerraGridder);
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

} // TerraGridder::LoadObject

/*===========================================================================*/

int TerraGridder::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
#ifdef WCS_BUILD_VNS
EffectList *CurEffect, *CoordsList = NULL;
#endif // WCS_BUILD_VNS
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

#ifdef WCS_BUILD_VNS
if (BuildFileComponentsList(&CoordsList))
	{
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

	while (CoordsList)
		{
		CurEffect = CoordsList;
		CoordsList = CoordsList->Next;
		delete CurEffect;
		} // while
	} // if
#endif // WCS_BUILD_VNS

// TerraGridder
strcpy(StrBuf, "Gridder");
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
			} // if TerraGridder saved 
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

} // TerraGridder::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::TerraGridder_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
TerraGridder *Current;

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
						if (Current = new TerraGridder(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (TerraGridder *)FindDuplicateByName(Current->EffectType, Current))
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

} // EffectsLib::TerraGridder_Load()

/*===========================================================================*/

ULONG EffectsLib::TerraGridder_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
TerraGridder *Current;

Current = Gridder;
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
					} // if TerraGridder saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (TerraGridder *)Current->Next;
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

} // EffectsLib::TerraGridder_Save()

/*===========================================================================*/
