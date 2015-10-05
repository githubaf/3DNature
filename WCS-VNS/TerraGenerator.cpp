// TerraGenerator.cpp
// For managing Terrain Generator Effects
// Built from scratch on 12/14/00 by Gary R. Huber
// Copyright 2000 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "AppMem.h"
#include "Conservatory.h"
#include "TerraGeneratorEditGUI.h"
#include "Project.h"
#include "Types.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Log.h"
#include "Raster.h"
#include "GraphData.h"
#include "requester.h"
#include "Render.h"
#include "Database.h"
#include "Security.h"
#include "Toolbar.h"
#include "DragnDropListGUI.h"
#include "DEM.h"
#include "Lists.h"
#include "FeatureConfig.h"

TerraGenSampleData::TerraGenSampleData()
{

PlanetRad = MetersPerDegLat = CenterLat = CenterLon = YVInc = XHInc = BaseElev = ElevRange = 
	SeedXOffset = SeedYOffset = HighDEMLon = StartY = 0.0;
Running = 0;
y = 0;

} // TerraGenSampleData::TerraGenSampleData

/*===========================================================================*/
/*===========================================================================*/

TerrainType::TerrainType(void)
: RasterAnimHost(NULL)
{

SetDefaults();

} // TerrainType::TerrainType

/*===========================================================================*/

TerrainType::TerrainType(RasterAnimHost *RAHost)
: RasterAnimHost(RAHost)
{

SetDefaults();

} // TerrainType::TerrainType

/*===========================================================================*/

void TerrainType::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_TERRAINTYPE_NUMANIMPAR] = {0.0, 5000.0};
double RangeDefaults[WCS_EFFECTS_TERRAINTYPE_NUMANIMPAR][3] = {1000000.0, -1000000.0, 100.0,
															1000000.0, 0.0, 10.0};
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	TexRoot[Ct] = NULL;
	} // for
Seed = 999;

AnimPar[WCS_EFFECTS_TERRAINTYPE_ANIMPAR_BASEELEV].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_TERRAINTYPE_ANIMPAR_ELEVRANGE].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);

} // TerrainType::SetDefaults

/*===========================================================================*/

TerrainType::~TerrainType()
{
long Ct;
RootTexture *DelTex;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (DelTex = TexRoot[Ct])
		{
		TexRoot[Ct] = NULL;
		delete DelTex;
		} // if
	} // for

} // TerrainType::~TerrainType

/*===========================================================================*/

void TerrainType::Copy(TerrainType *CopyTo, TerrainType *CopyFrom)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].Copy((AnimCritter *)&CopyTo->AnimPar[Ct], (AnimCritter *)&CopyFrom->AnimPar[Ct]);
	} // for
/*
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (CopyTo->TexRoot[Ct])
		{
		delete CopyTo->TexRoot[Ct];		// removes links to images
		CopyTo->TexRoot[Ct] = NULL;
		} // if
	if (CopyFrom->TexRoot[Ct])
		{
		if (CopyTo->TexRoot[Ct] = new RootTexture(CopyTo, CopyFrom->TexRoot[Ct]->ApplyToEcosys, CopyFrom->TexRoot[Ct]->ApplyToColor, CopyFrom->TexRoot[Ct]->ApplyToDisplace))
			{
			CopyTo->TexRoot[Ct]->Copy(CopyTo->TexRoot[Ct], CopyFrom->TexRoot[Ct]);
			} // if
		} // if
	} // for
*/
CopyTo->Seed = CopyFrom->Seed;
RootTextureParent::Copy(CopyTo, CopyFrom);

} // TerrainType::Copy

/*===========================================================================*/

ULONG TerrainType::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

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
					case WCS_EFFECTS_TERRAINTYPE_BASEELEV:
						{
						BytesRead = AnimPar[WCS_EFFECTS_TERRAINTYPE_ANIMPAR_BASEELEV].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_TERRAINTYPE_ELEVRANGE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_TERRAINTYPE_ANIMPAR_ELEVRANGE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GENERATOR_SEED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Seed, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_TERRAINTYPE_TEXDISPLACEMENT:
						{
						if (TexRoot[WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
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

} // TerrainType::Load

/*===========================================================================*/

unsigned long int TerrainType::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_EFFECTS_TERRAINTYPE_NUMANIMPAR] = {WCS_EFFECTS_TERRAINTYPE_BASEELEV,
																	WCS_EFFECTS_TERRAINTYPE_ELEVRANGE};
unsigned long int TextureItemTag[WCS_EFFECTS_TERRAINTYPE_NUMTEXTURES] = {WCS_EFFECTS_TERRAINTYPE_TEXDISPLACEMENT};

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
				} // if anim param saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // for

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (TexRoot[Ct])
		{
		ItemTag = TextureItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = TexRoot[Ct]->Save(ffile))
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
					} // if anim param saved
				else
					goto WriteError;
				} // if size written
			else
				goto WriteError;
			} // if tag written
		else
			goto WriteError;
		} // if
	} // for

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GENERATOR_SEED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Seed)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // TerrainType::Save

/*===========================================================================*/

short TerrainType::AnimateShadows(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetNumNodes(0) > 1)
		return (1);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for

return (0);

} // TerrainType::AnimateShadows

/*===========================================================================*/

char *TerrainTypeCritterNames[WCS_EFFECTS_TERRAINTYPE_NUMANIMPAR] = {"Base Elevation (m)", "Elevation Range (m)"};
char *TerrainTypeTextureNames[WCS_EFFECTS_TERRAINTYPE_NUMTEXTURES] = {"Displacement (%)"};

char *TerrainType::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (TerrainTypeCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (TerrainTypeTextureNames[Ct]);
		} // if
	} // for

return ("");

} // TerrainType::GetCritterName

/*===========================================================================*/

char *TerrainType::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as a Terrain Type Texture! Remove anyway?");

} // TerrainType::OKRemoveRaster

/*===========================================================================*/

char *TerrainType::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? TerrainTypeTextureNames[TexNumber]: (char*)"");

} // TerrainType::GetTextureName

/*===========================================================================*/

void TerrainType::GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace)
{

ApplyToColor = 0;
ApplyToDisplace = 1;

} // TerrainType::GetTextureApplication

/*===========================================================================*/

RootTexture *TerrainType::NewRootTexture(long TexNumber)
{
char ApplyToColor = 0;
char ApplyToDisplace = 1;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // TerrainType::NewRootTexture

/*===========================================================================*/

int TerrainType::RemoveRAHost(RasterAnimHost *RemoveMe)
{
char Ct;
NotifyTag Changes[2];
int Removed = 0;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (RemoveMe == GetTexRootPtr(Ct))
		{
		delete GetTexRootPtr(Ct);
		SetTexRootPtr(Ct, NULL);
		Removed = 1;
		} // if
	} // for

if (Removed)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

return (Removed);

} // TerrainType::RemoveRAHost

/*===========================================================================*/

int TerrainType::SetToTime(double Time)
{
long Found = 0, Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for

return (Found);

} // TerrainType::SetToTime

/*===========================================================================*/

long TerrainType::InitImageIDs(long &ImageID)
{
char Ct;
long NumImages = 0;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		NumImages += GetTexRootPtr(Ct)->InitImageIDs(ImageID);
		} // if
	} // for

return (NumImages);

} // TerrainType::InitImageIDs

/*===========================================================================*/

int TerrainType::GetRAHostAnimated(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for

return (0);

} // TerrainType::GetRAHostAnimated

/*===========================================================================*/

long TerrainType::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Ct, Found = 0;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetMinMaxDist(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetKeyFrameRange(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // for

if (Found)
	{
	FirstKey = TestFirst;
	LastKey = TestLast;
	} // if
else
	{
	FirstKey = LastKey = 0;
	} // else

return (Found);

} // TerrainType::GetKeyFrameRange

/*===========================================================================*/

char TerrainType::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (GetNumAnimParams() > 0 && DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	return (1);
if (GetNumTextures() > 0 && (DropType == WCS_RAHOST_OBJTYPE_ROOTTEXTURE || DropType == WCS_RAHOST_OBJTYPE_TEXTURE))
	return (1);

return (0);

} // TerrainType::GetRAHostDropOK

/*===========================================================================*/

int TerrainType::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char WinNum, QueryStr[256], NameStr[128];
int Success = 0;
long Ct, NumListItems = 0;
RasterAnimHostProperties Prop;
RasterAnimHost *TargetList[30];
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (TerrainType *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (TerrainType *)DropSource->DropSource);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		TargetList[Ct] = GetAnimPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ROOTTEXTURE || DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumTextures(); Ct ++)
		{
		TargetList[Ct] = GetTexRootPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if

if (NumListItems > 0)
	{
	if ((WinNum = GlobalApp->MCP->GetAvailableDragnDropListNumber()) >= 0)
		{
		GlobalApp->GUIWins->DDL[WinNum] = new DragnDropListGUI(WinNum, DropSource, TargetList, NumListItems, NULL, NULL);
		if (GlobalApp->GUIWins->DDL[WinNum])
			{
			GlobalApp->GUIWins->DDL[WinNum]->Open(GlobalApp->MainProj);
			}
		} // if
	else
		UserMessageOK("Open Drag 'n Drop List", "All List windows are in use.\nTry dropping on a target deeper in the hierarchy\n and closer to your desired target.");
	} // if

return (Success);

} // TerrainType::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long TerrainType::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_TERRAINGEN | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // TerrainType::GetRAFlags

/*===========================================================================*/

void TerrainType::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = "";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPE)
	{
	Prop->Type = GetRAHostTypeString();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPENUMBER)
	{
	Prop->TypeNumber = GetRAHostTypeNumber();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPOK)
	{
	Prop->DropOK = GetRAHostDropOK(Prop->TypeNumber);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	GetKeyFrameRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1]);
	} // if
else if (Prop->PropMask & WCS_RAHOST_MASKBIT_NEXTKEY)
	{
	GetNextAnimNode(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_FILEINFO)
	{
	Prop->Path = EffectsLib::DefaultPaths[WCS_EFFECTSSUBCLASS_TERRAFFECTOR];
	Prop->Ext = "ttp";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_POPCLASS)
	{
	GetPopClassFlags(Prop);
	} // get pop-up menu classes

} // TerrainType::GetRAHostProperties

/*===========================================================================*/

int TerrainType::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPSOURCE)
	{
	Success = ProcessRAHostDragDrop(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	if (Success = ScaleDeleteAnimNodes(Prop))
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_LOADFILE)
	{
	return(LoadObject(Prop->fFile, 0, Prop->ByteFlip));
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_SAVEFILE)
	{
	return(SaveObject(Prop->fFile));
	} // if

return (Success);

} // TerrainType::SetRAHostProperties

/*===========================================================================*/

int TerrainType::BuildFileComponentsList(EffectList **Coords)
{
#ifdef WCS_BUILD_VNS
long Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		if (! GetTexRootPtr(Ct)->BuildFileComponentsList(Coords))
			return (0);
		} // if
	} // for
#endif // WCS_BUILD_VNS

return (1);

} // TerrainType::BuildFileComponentsList

/*===========================================================================*/

int TerrainType::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
	RootTexture **&TexAffil)
{
long Ct;

AnimAffil = NULL;
TexAffil = NULL;

if (ChildA)
	{
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		if (ChildA == GetAnimPtr(Ct))
			{
			AnimAffil = (AnimCritter *)ChildA;
			if (Ct == WCS_EFFECTS_TERRAINTYPE_ANIMPAR_ELEVRANGE)
				TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT);
			return (1);
			} // if
		} // for
	} // if
else if (ChildB)
	{
	for (Ct = 0; Ct < GetNumTextures(); Ct ++)
		{
		if (ChildB == (RasterAnimHost **)GetTexRootPtrAddr(Ct))
			{
			TexAffil = (RootTexture **)ChildB;
			if (Ct == WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT)
				AnimAffil = GetAnimPtr(WCS_EFFECTS_TERRAINTYPE_ANIMPAR_ELEVRANGE);
			return (1);
			} // if
		} // for
	} // else if


return (0);

} // TerrainType::GetAffiliates

/*===========================================================================*/

int TerrainType::GetPopClassFlags(RasterAnimHostProperties *Prop)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;

Prop->PopClassFlags = 0;
Prop->PopExistsFlags = 0;
Prop->PopEnabledFlags = 0;

if (GetAffiliates(Prop->ChildA, Prop->ChildB, AnimAffil, TexAffil))
	{
	return (RasterAnimHost::GetPopClassFlags(Prop, AnimAffil, TexAffil, NULL));
	} // if

return (0);

} // TerrainType::GetPopClassFlags

/*===========================================================================*/

int TerrainType::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long int MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil))
	{
	return (RasterAnimHost::AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, TexAffil, NULL));
	} // if

return(0);

} // TerrainType::AddSRAHBasePopMenus

/*===========================================================================*/

int TerrainType::HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
int Handled = 0, OpenEditor = 1;
long ItemNumber;
double TempSize;
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;
NotifyTag Changes[2];

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil))
	{
	// handle texture creation especially so we can set the texture size
	if (! strcmp((char *)Action, "TX_CREATE") && TexAffil && ! (*TexAffil))
		{
		if ((ItemNumber = GetTexNumberFromAddr(TexAffil)) >= 0 && NewRootTexture(ItemNumber))
			{
			if (GlobalApp->GUIWins->TGN)
				{
				TempSize = GlobalApp->GUIWins->TGN->GetMinDimension();
				TempSize = max(1.0, TempSize);
				(*TexAffil)->PreviewSize.SetValue(TempSize * .5);
				if ((*TexAffil)->Tex || ((*TexAffil)->Tex = (Texture *)new HybridMultiFractalNoiseTexture((*TexAffil),
					WCS_TEXTURE_ROOT, (*TexAffil)->ApplyToEcosys, (*TexAffil)->ApplyToColor, NULL, NULL)))
					{
					(*TexAffil)->Tex->TexSize[0].SetValue(TempSize * .5);
					(*TexAffil)->Tex->TexSize[1].SetValue(TempSize * .5);
					(*TexAffil)->Tex->TexSize[2].SetValue(TempSize * .5);
					} // if
				OpenEditor = (! GlobalApp->GUIWins->TGN->CreatingInitialSetup());
				} // if
			if (OpenEditor)
				(*TexAffil)->EditRAHost();
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		Handled = 1;
		} // if
	else
		Handled = RasterAnimHost ::HandleSRAHPopMenuSelection(Action, AnimAffil, TexAffil, NULL, this, NULL);
	return (Handled);
	} // if

return(0);

} // TerrainType::HandleSRAHPopMenuSelection

/*===========================================================================*/

RasterAnimHost *TerrainType::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;

if (! Current)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Found)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Found && GetTexRootPtr(Ct))
		return (GetTexRootPtr(Ct));
	if (Current == GetTexRootPtr(Ct))
		Found = 1;
	} // for

return (NULL);

} // TerrainType::GetRAHostChild

/*===========================================================================*/

int TerrainType::GetDeletable(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (1);
		} // if
	} // for

return (0);

} // TerrainType::GetDeletable

/*===========================================================================*/

int TerrainType::InitToRender(void)
{
char Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (TexRoot[Ct] && TexRoot[Ct]->Enabled)
		{
		if (! TexRoot[Ct]->InitAAChain())
			{
			return (0);
			} // if
		} // if
	} // for

return (1);

} // TerrainType::InitToRender

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int TerrainType::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
TerrainType *CurrentType = NULL;
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
						} // if DEMBnds
					else if (! strnicmp(ReadBuf, "Images", 8))
						{
						BytesRead = GlobalApp->LoadToImageLib->Load(ffile, Size, NULL);
						} // if Images
					else if (! strnicmp(ReadBuf, "CoordSys", 8))
						{
						if (CurrentCoordSys = new CoordSys(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentCoordSys->Load(ffile, Size, ByteFlip);
							}
						} // if CoordSys
					else if (! strnicmp(ReadBuf, "TerraTyp", 8))
						{
						if (CurrentType = new TerrainType(NULL))
							{
							if ((BytesRead = CurrentType->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if Strata
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

if (Success == 1 && CurrentType)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentType);
	delete CurrentType;
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

} // TerrainType::LoadObject

/*===========================================================================*/

int TerrainType::SaveObject(FILE *ffile)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
EffectList *CurEffect, *Coords = NULL;
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

if (BuildFileComponentsList(&Coords))
	{
	#ifdef WCS_BUILD_VNS
	CurEffect = Coords;
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
	#endif // WCS_BUILD_VNS

	while (Coords)
		{
		CurEffect = Coords;
		Coords = Coords->Next;
		delete CurEffect;
		} // while
	} // if

// TerrainType
strcpy(StrBuf, "TerraTyp");
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
			} // if TerrainType saved 
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

} // TerrainType::SaveObject

/*===========================================================================*/
/*===========================================================================*/

TerraGenerator::TerraGenerator()
: GeneralEffect(NULL), TerraType(this), GeoReg(this)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_GENERATOR;
SetDefaults();

} // TerraGenerator::TerraGenerator

/*===========================================================================*/

TerraGenerator::TerraGenerator(RasterAnimHost *RAHost)
: GeneralEffect(RAHost), TerraType(this), GeoReg(this)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_GENERATOR;
SetDefaults();

} // TerraGenerator::TerraGenerator

/*===========================================================================*/

TerraGenerator::TerraGenerator(RasterAnimHost *RAHost, EffectsLib *Library, TerraGenerator *Proto)
: GeneralEffect(RAHost), TerraType(this), GeoReg(this)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_GENERATOR;
Prev = Library->LastGenerator;
if (Library->LastGenerator)
	{
	Library->LastGenerator->Next = this;
	Library->LastGenerator = this;
	} // if
else
	{
	Library->Generator = Library->LastGenerator = this;
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
	strcpy(NameBase, "Terrain Generator");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // TerraGenerator::TerraGenerator

/*===========================================================================*/

TerraGenerator::~TerraGenerator()
{

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->TGN && GlobalApp->GUIWins->TGN->GetActive() == this)
		{
		delete GlobalApp->GUIWins->TGN;
		GlobalApp->GUIWins->TGN = NULL;
		} // if
	} // if

} // TerraGenerator::~TerraGenerator

/*===========================================================================*/

void TerraGenerator::SetDefaults(void)
{

Rows = Cols = 301;
RowMaps = ColMaps = 1;
PreviewEnabled = 0;
InitialSetup = 1;
strncpy(DEMName, GlobalApp->MainProj->projectname, WCS_EFFECT_MAXNAMELENGTH);
DEMName[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
StripExtension(DEMName);
PreviewMap = NULL;
PreviewSize = 100;
PreviewJoe = NULL;

} // TerraGenerator::SetDefaults

/*===========================================================================*/

void TerraGenerator::Copy(TerraGenerator *CopyTo, TerraGenerator *CopyFrom)
{

CopyTo->DitchPreview(GlobalApp->AppEffects);
CopyFrom->PreviewSize = CopyFrom->PreviewSize;
CopyFrom->PreviewEnabled = CopyFrom->PreviewEnabled;
CopyFrom->InitialSetup = CopyFrom->InitialSetup;
CopyTo->Rows = CopyFrom->Rows;
CopyTo->Cols = CopyFrom->Cols;
CopyTo->RowMaps = CopyFrom->RowMaps;
CopyTo->ColMaps = CopyFrom->ColMaps;
strcpy(CopyTo->DEMName, CopyFrom->DEMName);
GeoReg.Copy(&CopyTo->GeoReg, &CopyFrom->GeoReg);
TerraType.Copy(&CopyTo->TerraType, &CopyFrom->TerraType);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // TerraGenerator::Copy

/*===========================================================================*/

void TerraGenerator::DitchPreview(EffectsLib *EffectsHost)
{

if (PreviewJoe)
	{
	if (PreviewJoe->RemoveMe(EffectsHost))
		{
		delete PreviewJoe;
		} // if
	} // if
PreviewMap = NULL;
PreviewJoe = NULL;
PreviewEnabled = 0;

} // TerraGenerator::DitchPreview

/*===========================================================================*/

ULONG TerraGenerator::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

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
					case WCS_EFFECTS_GENERATOR_DEMNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)DEMName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GENERATOR_ROWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Rows, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GENERATOR_COLS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Cols, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GENERATOR_ROWMAPS:
						{
						BytesRead = ReadBlock(ffile, (char *)&RowMaps, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GENERATOR_COLMAPS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ColMaps, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GENERATOR_PREVIEWSIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&PreviewSize, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GENERATOR_GEOREG:
						{
						BytesRead = GeoReg.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GENERATOR_TERRATYPE:
						{
						BytesRead = TerraType.Load(ffile, Size, ByteFlip);
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

InitialSetup = 0;

return (TotalRead);

} // TerraGenerator::Load

/*===========================================================================*/

unsigned long int TerraGenerator::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GENERATOR_DEMNAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(DEMName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)DEMName)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GENERATOR_ROWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Rows)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GENERATOR_COLS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Cols)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GENERATOR_ROWMAPS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&RowMaps)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GENERATOR_COLMAPS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&ColMaps)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_GENERATOR_PREVIEWSIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&PreviewSize)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_EFFECTS_GENERATOR_GEOREG + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = GeoReg.Save(ffile))
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
			} // if registration saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_EFFECTS_GENERATOR_TERRATYPE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = TerraType.Save(ffile))
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
			} // if terrain type saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // TerraGenerator::Save

/*===========================================================================*/

void TerraGenerator::Edit(void)
{

DONGLE_INLINE_CHECK()
if (GlobalApp->GUIWins->TGN)
	{
	delete GlobalApp->GUIWins->TGN;
	}
GlobalApp->GUIWins->TGN = new TerraGeneratorEditGUI(GlobalApp->AppEffects, GlobalApp->MainProj, GlobalApp->AppDB, this);
if (GlobalApp->GUIWins->TGN)
	{
	GlobalApp->GUIWins->TGN->Open(GlobalApp->MainProj);
	}

} // TerraGenerator::Edit

/*===========================================================================*/

short TerraGenerator::AnimateShadows(void)
{

if (GeoReg.AnimateShadows())
	return (1);
if (TerraType.AnimateShadows())
	return (1);

return (0);

} // TerraGenerator::AnimateShadows

/*===========================================================================*/

char *TerraGenerator::GetCritterName(RasterAnimHost *Test)
{

if (Test == &GeoReg)
	return ("Terrain Bounds");
if (Test == &TerraType)
	return ("Terrain Type");

return ("");

} // TerraGenerator::GetCritterName

/*===========================================================================*/

char *TerraGenerator::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as a Terrain Generator Texture! Remove anyway?");

} // TerraGenerator::OKRemoveRaster

/*===========================================================================*/

char TerraGenerator::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_GEOREGISTER
	|| DropType == WCS_RAHOST_OBJTYPE_TERRAINTYPE)
	return (1);

return (0);

} // TerraGenerator::GetRAHostDropOK

/*===========================================================================*/

int TerraGenerator::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
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
	if (this != (TerraGenerator *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (TerraGenerator *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_GEOREGISTER)
	{
	Success = GeoReg.ProcessRAHostDragDrop(DropSource);
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_TERRAINTYPE)
	{
	Success = TerraType.ProcessRAHostDragDrop(DropSource);
	} // else if
else
	{
	Success = GeneralEffect::ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // TerraGenerator::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long TerraGenerator::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_TERRAINGEN | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // TerraGenerator::GetRAFlags

/*===========================================================================*/

RasterAnimHost *TerraGenerator::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{

if (! Current)
	return (&GeoReg);
if (Current == &GeoReg)
	return (&TerraType);

return (NULL);

} // TerraGenerator::GetRAHostChild

/*===========================================================================*/

int TerraGenerator::SetToTime(double Time)
{
long Found = 0;

if (GeneralEffect::SetToTime(Time))
	Found = 1;
if (GeoReg.SetToTime(Time))
	Found = 1;
if (TerraType.SetToTime(Time))
	Found = 1;

return (Found);

} // TerraGenerator::SetToTime

/*===========================================================================*/

int TerraGenerator::BuildFileComponentsList(EffectList **Coords)
{

if (! TerraType.BuildFileComponentsList(Coords))
	return (0);

return (1);

} // TerraGenerator::BuildFileComponentsList

/*===========================================================================*/

long TerraGenerator::InitImageIDs(long &ImageID)
{
long NumImages = 0;

NumImages += TerraType.InitImageIDs(ImageID);
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // TerraGenerator::InitImageIDs

/*===========================================================================*/

// maximum is in array elements [0], minimum in [1]
int TerraGenerator::GetMaterialBoundsXYZ(double XRange[2], double YRange[2], double ZRange[2])
{
double NWLatitude, NWLongitude, SELatitude, SELongitude, RefLat, RefLon, EarthLatScaleMeters, RefLonScaleMeters;

EarthLatScaleMeters = LatScale(GlobalApp->AppEffects->GetPlanetRadius());
RefLat = (GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue + GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue) * .5;
RefLon = (GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue + GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue) * .5;
RefLonScaleMeters = EarthLatScaleMeters * cos(RefLat * PiOver180);

SELongitude = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue;
NWLongitude = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue;
NWLatitude = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue;
SELatitude = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
XRange[0] = -(SELongitude - RefLon) * RefLonScaleMeters;
XRange[1] = -(NWLongitude - RefLon) * RefLonScaleMeters;
YRange[0] = (NWLatitude - RefLat) * EarthLatScaleMeters;
YRange[1] = (SELatitude - RefLat) * EarthLatScaleMeters;

ZRange[0] = ZRange[1] = 0.0;

return (1);

} // TerraGenerator::GetMaterialBoundsXYZ

/*===========================================================================*/

int TerraGenerator::EvalSampleInit(TerraGenSampleData *Samp, EffectsLib *EffectsHost, Project *ProjHost, Database *DBHost)
{
char NameStr[256];
int Found = 0;
double LatPerDEM, LonPerDEM;
JoeDEM *MyDEM;
JoeRAHost *JoeRAH;
Texture *Tex;

if (! PreviewEnabled)
	return (0);

Samp->Running = 0;

Samp->PlanetRad = GlobalApp->AppEffects->GetPlanetRadius();
Samp->MetersPerDegLat = LatScale(Samp->PlanetRad);

if (! GeoReg.TestBoundsOrder())
	return (0);

LatPerDEM = (GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue);
LonPerDEM = (GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue);

Samp->CenterLat = (GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue + GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue) * 0.5;
Samp->CenterLon = (GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue + GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue) * 0.5;
Samp->YVInc = LatPerDEM / (PreviewSize - 1);
Samp->XHInc = LonPerDEM / (PreviewSize - 1);

Samp->BaseElev = TerraType.AnimPar[WCS_EFFECTS_TERRAINTYPE_ANIMPAR_BASEELEV].CurValue;
Samp->ElevRange = TerraType.AnimPar[WCS_EFFECTS_TERRAINTYPE_ANIMPAR_ELEVRANGE].CurValue;

if (TerraType.TexRoot[WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT] &&
	TerraType.TexRoot[WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT]->Enabled &&
	(Tex = TerraType.TexRoot[WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT]->Tex) &&
	Tex->Enabled &&
	Tex->UsesSeed())
	{
	Samp->SeedXOffset = TerraType.Seed * 93.0;	// 93 is arbitrary
	Samp->SeedYOffset = TerraType.Seed * 77.0;	// 77 is arbitrary but not a related factor of 93
	} // if
else
	{
	Samp->SeedXOffset = Samp->SeedYOffset = 0;
	} // else

Samp->y = 0;
Samp->StartY = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
Samp->HighDEMLon = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue; 

for (PreviewJoe = DBHost->GetFirst(WCS_DATABASE_DYNAMIC); PreviewJoe; PreviewJoe = DBHost->GetNext(PreviewJoe))
	{
	if (PreviewJoe->TestFlags(WCS_JOEFLAG_ISDEM))
		{
		if (! stricmp(GetName(), PreviewJoe->Name()))
			{
			Found = 1;
			if (MyDEM = (JoeDEM *)PreviewJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
				{
				PreviewMap = MyDEM->Pristine;
				} // if
			break;
			} // 
		} // if
	} // for

if (PreviewMap)
	{
	if (PreviewMap->LatEntries() != (unsigned long)PreviewSize || PreviewMap->LonEntries() != (unsigned long)PreviewSize)
		PreviewMap->FreeRawElevs();
	} // if

if (PreviewMap || (PreviewMap = new DEM))
	{
	PreviewMap->SetLatEntries(PreviewSize);
	PreviewMap->SetLonEntries(PreviewSize);
	PreviewMap->SetBounds(GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue, Samp->StartY, Samp->HighDEMLon, GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue);
	PreviewMap->SetLatStep(Samp->YVInc);
	PreviewMap->SetLonStep(Samp->XHInc);
	PreviewMap->SetElScale(ELSCALE_METERS);
	PreviewMap->SetElDatum(0.0);
	PreviewMap->SetMaxEl(Samp->BaseElev + Samp->ElevRange);
	PreviewMap->SetMinEl(Samp->BaseElev);

	if (! PreviewMap->Map())
		PreviewMap->AllocRawMap();

	if (Found)
		{
		PreviewJoe->SetFlags  (WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED | WCS_JOEFLAG_ISDEM);
		PreviewJoe->SetLineWidth(1);
		PreviewJoe->SetLineStyle(4);
		PreviewJoe->SetRGB((unsigned char)255, (unsigned char)255, (unsigned char)255);
		if (MyDEM || (MyDEM = new JoeDEM))
			{
			PreviewJoe->NWLat = (float)PreviewMap->Northest();
			PreviewJoe->NWLon = (float)PreviewMap->Westest();
			PreviewJoe->SELon = (float)PreviewMap->Eastest();
			PreviewJoe->SELat = (float)PreviewMap->Southest();
			MyDEM->MaxFract = 9;
			// <<<MAXMINELFLOAT>>>
			MyDEM->MaxEl = (short)PreviewMap->MaxEl();
			MyDEM->MinEl = (short)PreviewMap->MinEl();
			MyDEM->SumElDif = (float)PreviewMap->SumElDif ();
			MyDEM->SumElDifSq = (float)PreviewMap->SumElDifSq();
			MyDEM->ElScale = PreviewMap->ElScale();
			MyDEM->ElDatum = PreviewMap->ElDatum();
			if (! PreviewJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_RAHOST)
				&& (JoeRAH = new JoeRAHost))
				{
				JoeRAH->RAHost = this;
				PreviewJoe->AddAttribute(JoeRAH);
				} // if
			} // if
		PreviewJoe->ZeroUpTree();
		DBHost->ReBoundTree(WCS_DATABASE_DYNAMIC);
		} // if
	else
		{
		sprintf(NameStr, "%s\n", GetName());
		if (PreviewJoe = new (NameStr) Joe)
			{
			PreviewJoe->SetFlags  (WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED | WCS_JOEFLAG_ISDEM);
			PreviewJoe->SetLineWidth(1);
			PreviewJoe->SetLineStyle(4);
			PreviewJoe->SetRGB(255, 255, 255);

			if (! DBHost->AddJoe(PreviewJoe, WCS_DATABASE_DYNAMIC, ProjHost))
				{
				UserMessageOK("Terrain Generator", "Could not add object to Database. Preview not available.");
				delete PreviewJoe;
				PreviewJoe = NULL;
				return (0);
				} // else

			if (MyDEM = new JoeDEM)
				{
				// Don't look too hard at this. It's intentional.
				//MyDEM->JoeDEM::JoeDEM();
				MyDEM->MaxFract = 9;
				// Transfer DEM bounds values into Joe
				PreviewJoe->NWLat = (float)PreviewMap->Northest();
				PreviewJoe->NWLon = (float)PreviewMap->Westest();
				PreviewJoe->SELon = (float)PreviewMap->Eastest();
				PreviewJoe->SELat = (float)PreviewMap->Southest();
				MyDEM->MaxFract = 9;
				// <<<MAXMINELFLOAT>>>
				MyDEM->MaxEl = (short)PreviewMap->MaxEl();
				MyDEM->MinEl = (short)PreviewMap->MinEl();
				MyDEM->SumElDif = (float)PreviewMap->SumElDif ();
				MyDEM->SumElDifSq = (float)PreviewMap->SumElDifSq();
				MyDEM->ElScale = PreviewMap->ElScale();
				MyDEM->ElDatum = PreviewMap->ElDatum();
				MyDEM->Pristine = PreviewMap;
				PreviewMap = NULL;
				
				PreviewJoe->AddAttribute(MyDEM);
				if (JoeRAH = new JoeRAHost)
					{
					JoeRAH->RAHost = this;
					PreviewJoe->AddAttribute(JoeRAH);
					} // if
				} // if
			else
				{
				UserMessageOK("Terrain Generator", "Could not add DEM attribute. Preview not available.");
				// remove from database
				if (PreviewJoe->RemoveMe(EffectsHost))
					{
					delete PreviewJoe;
					} // if
				// null the local pointer anyway because deletion is now in the hands of the 
				// database if it wasn't successfully removed
				PreviewJoe = NULL;
				return (0);
				} // 
			DBHost->BoundUpTree(PreviewJoe);
			} // if
		} // else

	Samp->Running = 1;
	return (1);
	} // if

return (0);

} // TerraGenerator::EvalSampleInit

/*===========================================================================*/

int TerraGenerator::EvalOneSampleLine(TerraGenSampleData *Samp)
{
double SampleX, CalcX, CalcY, MetersPerDegLon, ElevPt, Value[3];
long x;
JoeDEM *MyJoeDEM;
DEM *PreviewToWriteTo;
VertexDEM Vert;

PreviewToWriteTo = PreviewMap;

if (!PreviewToWriteTo && PreviewJoe)
	{
	if (MyJoeDEM = (JoeDEM *)PreviewJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
		{
		PreviewToWriteTo = MyJoeDEM->Pristine;
		} // if
	} // if

if (! PreviewToWriteTo)
	goto AbortPreview;

MetersPerDegLon = LonScale(Samp->PlanetRad, Samp->StartY);
CalcY = (Samp->StartY - Samp->CenterLat) * Samp->MetersPerDegLat;
Value[0] = Value[1] = Value[2] = 0.0;
Samp->TexData.VDEM[0] = &Vert;
Samp->TexData.VDEM[1] = Samp->TexData.VDEM[2] = NULL;
Samp->TexData.VData[0] = Samp->TexData.VData[1] = Samp->TexData.VData[2] = NULL;
Samp->TexData.PData = NULL;
Samp->TexData.MetersPerDegLat = Samp->MetersPerDegLat;
Samp->TexData.TexRefLat = Samp->CenterLat;
Samp->TexData.TexRefLon = Samp->CenterLon;
Samp->TexData.MetersPerDegLon = MetersPerDegLon;

for (x = 0, SampleX = Samp->HighDEMLon; x < PreviewSize; x ++, SampleX -= Samp->XHInc)
	{
	CalcX = (Samp->CenterLon - SampleX) * MetersPerDegLon;
	// fill in texture data
	Vert.xyz[0] = Vert.XYZ[0] = CalcX + Samp->SeedXOffset;
	Vert.xyz[1] = Vert.XYZ[1] = CalcY + Samp->SeedYOffset;
	//Samp->TexData.LowX = Samp->TexData.HighX = CalcX + Samp->SeedXOffset;
	//Samp->TexData.LowY = Samp->TexData.HighY = CalcY + Samp->SeedYOffset; 
	Samp->TexData.Latitude = Vert.Lat = Samp->TexData.TLatRange[0] = Samp->TexData.TLatRange[1] = Samp->TexData.TLatRange[2] = Samp->TexData.TLatRange[3] = Samp->StartY;
	Samp->TexData.Longitude = Vert.Lon = Samp->TexData.TLonRange[0] = Samp->TexData.TLonRange[1] = Samp->TexData.TLonRange[2] = Samp->TexData.TLonRange[3] = SampleX;
	// evaluate texture
	Value[0] = Value[1] = Value[2] = 0.0;
	if (TerraType.TexRoot[WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT] && TerraType.TexRoot[WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT]->Enabled)
		{
		TerraType.TexRoot[WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT]->Eval(Value, &Samp->TexData);
		} // if
	// multiply by elev range and add to elev base
	ElevPt = Samp->ElevRange * Value[0] + Samp->BaseElev;
	// store in elevation array
	PreviewToWriteTo->StoreElevation(Samp->y, x, (float)ElevPt);
	} // for

Samp->y ++;
Samp->StartY += Samp->YVInc;

if (Samp->y < PreviewSize)
	return (0);

AbortPreview:

Samp->Running = 0;

// Once we do this, we NULL out PreviewMap, because MyDEM owns it
// now and is responsible for destroying it.
/* putting this here leads to redundant allocation of PreviewMap and PreviewMap->RawMap since EvalSampleInit may be called
a second time before EvalOneSampleLine ever gets called. MyDEM->Pristine = PreviewMap is now set in EvalSampleInit.
if (PreviewMap)
	{
	if (MyDEM = (JoeDEM *)PreviewJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
		{
		MyDEM->Pristine = PreviewMap;
		PreviewMap = NULL;
		} // if
	} // if
*/
return (1);

} // TerraGenerator::EvalOneSampleLine

/*===========================================================================*/

// a function to instigate the DEM construction and route the DEM output either to a file
// or to a DEM attribute as the situation requires, and to send notifications.
// A new Joe may be added to the database

void TerraGenerator::DoSomethingConstructive(Project *ProjHost, Database *DBHost)
{
char filename[512], ObjName[256], BaseName[256], NameStr[256], OutPath[256];
short Found = 0, Success = 1;
long MapNum, ColMap, RowMap;
Joe *Clip, *Added = NULL;
JoeDEM *MyJoeDEM;
LayerEntry *LE = NULL;
NotifyTag ChangeEvent[2];
DEM *TempDEM = NULL, *NewTempDEM;
BusyWin *BWMD = NULL;

BWMD = new BusyWin("Generating", RowMaps * ColMaps, 'BWMD', 0);

strcpy(OutPath, ProjHost->dirname);
for (ColMap = 0, MapNum = 1; Success && ColMap < ColMaps; ColMap ++)
	{
	for (RowMap = 0; Success && RowMap < RowMaps; RowMap ++, MapNum ++)
		{
		if (NewTempDEM = MakeTerrain(TempDEM, ColMap, RowMap))
			{
			TempDEM = NewTempDEM;
			sprintf(ObjName, "%s.%d", DEMName, MapNum);
			strcpy(BaseName, ObjName);
			strcat(ObjName, ".elev");
			strmfp(filename, OutPath, ObjName);
			if (TempDEM->SaveDEM(filename, NULL))
				{
				Found = 0;
				for (Clip = DBHost->GetFirst(); Clip ; Clip = DBHost->GetNext(Clip))
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
					Clip->SetFlags  (WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED | WCS_JOEFLAG_ISDEM);
					Clip->SetLineWidth(1);
					Clip->SetLineStyle(4);
					Clip->SetRGB((unsigned char)255, (unsigned char)255, (unsigned char)255);
					if (MyJoeDEM = (JoeDEM *)Clip->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_DEM))
						{
						Clip->NWLat = (float)TempDEM->Northest();
						Clip->NWLon = (float)TempDEM->Westest();
						Clip->SELon = (float)TempDEM->Eastest();
						Clip->SELat = (float)TempDEM->Southest();
						MyJoeDEM->MaxFract = 9;
						// <<<MAXMINELFLOAT>>>
						MyJoeDEM->MaxEl = (short)TempDEM->MaxEl();
						MyJoeDEM->MinEl = (short)TempDEM->MinEl();
						MyJoeDEM->SumElDif = (float)TempDEM->SumElDif ();
						MyJoeDEM->SumElDifSq = (float)TempDEM->SumElDifSq();
						MyJoeDEM->ElScale = TempDEM->ElScale();
						MyJoeDEM->ElDatum = TempDEM->ElDatum();
						} // if
					Clip->ZeroUpTree();
					DBHost->ReBoundTree(WCS_DATABASE_STATIC);
					} // if
				else
					{
					sprintf(NameStr, "%s\n%s", BaseName, BaseName);
					if (Clip = new (NameStr) Joe)
						{
						Clip->SetFlags  (WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED | WCS_JOEFLAG_ISDEM);
						Clip->SetLineWidth(1);
						Clip->SetLineStyle(4);
						Clip->SetRGB((unsigned char)255, (unsigned char)255, (unsigned char)255);

						Added = DBHost->AddJoe(Clip, WCS_DATABASE_STATIC, ProjHost);
						if (Added)
							{
							DBHost->SetActiveObj(Clip);
							} // if
						else
							{
							UserMessageOK("Terrain Generator", "Could not add object to Database.");
							Success = 0;
							break;
							} // else

						if (LE = DBHost->DBLayers.MatchMakeLayer("TOP", 0))
							{
							Clip->AddObjectToLayer(LE);
							} // if

						//if (MyJoeDEM = (JoeDEM *)AppMem_Alloc(sizeof(JoeDEM), APPMEM_CLEAR))
						if (MyJoeDEM = new JoeDEM)
							{
							// Don't look too hard at this. It's intentional.
							//MyJoeDEM->JoeDEM::JoeDEM();
							MyJoeDEM->MaxFract = 9;
							// Transfer DEM bounds values into Joe
							Clip->NWLat = (float)TempDEM->Northest();
							Clip->NWLon = (float)TempDEM->Westest();
							Clip->SELon = (float)TempDEM->Eastest();
							Clip->SELat = (float)TempDEM->Southest();
							MyJoeDEM->MaxFract = 9;
							// <<<MAXMINELFLOAT>>>
							MyJoeDEM->MaxEl = (short)TempDEM->MaxEl();
							MyJoeDEM->MinEl = (short)TempDEM->MinEl();
							MyJoeDEM->SumElDif = (float)TempDEM->SumElDif ();
							MyJoeDEM->SumElDifSq = (float)TempDEM->SumElDifSq();
							MyJoeDEM->ElScale = TempDEM->ElScale();
							MyJoeDEM->ElDatum = TempDEM->ElDatum();

							Clip->AddAttribute(MyJoeDEM);
							} // if
						else
							{
							UserMessageOK("Terrain Generator", "Could not add DEM Attribute tag. File saved but DEM not added to Database.");
							Success = 0;
							} // 
						DBHost->BoundUpTree(Clip);
						} // if
					} // else

				if (Clip)
					{
					ChangeEvent[0] = MAKE_ID(WCS_NOTIFYCLASS_DBASE, WCS_NOTIFYDBASE_ADDOBJ, 0, 0);
					ChangeEvent[1] = NULL;
					DBHost->GenerateNotify(ChangeEvent);
					ChangeEvent[0] = MAKE_ID(Clip->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
					ChangeEvent[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(ChangeEvent, NULL);
					}
				} // if saved OK
			else
				{
				UserMessageOK("Terrain Generator", "Error saving DEM! Check Default Directory and \"WCSProjects:\" Master Path.");
				Success = 0;
				break;
				} // else
			} // if
		else
			{
			UserMessageOK("Terrain Generator", "There is no terrain data to save! Terrain generation failed.");
			Success = 0;
			break;
			} // else
		if (BWMD)
			{
			if (BWMD->Update(MapNum))
				{
				Success = 0;
				break;
				} // if
			} // if
		} // for
	} // for

if (TempDEM)
	delete TempDEM;
if (BWMD)
	delete BWMD;

} // TerraGenerator::DoSomethingConstructive

/*===========================================================================*/

// this returns NULL if the function fails, not necessarily if the DEM is NULL
// elev array memory will be allocated if it is not already
DEM *TerraGenerator::MakeTerrain(DEM *MyDEM, long ColMap, long RowMap)
{
double PlanetRad, MetersPerDegLat, MetersPerDegLon, CenterLat, CenterLon, LatInc, LonInc, LatPt, LonPt, ElevPt, 
	ElevRange, BaseElev, Value[3], CalcX, CalcY, SeedXOffset, SeedYOffset, LatPerDEM, LonPerDEM, LowDEMLat, HighDEMLat,
	LowDEMLon, HighDEMLon;
DEM *rVal = NULL;
long LatRow, LonCol, Success = 1, CreateDEM;
TextureData TexData;
VertexDEM Vert;
BusyWin *BWMD = NULL;
Texture *Tex;
char Str[64];


PlanetRad = GlobalApp->AppEffects->GetPlanetRadius();
MetersPerDegLat = LatScale(PlanetRad);
Value[0] = Value[1] = Value[2] = 0.0;

GeoReg.ValidateBoundsOrder(NULL);
if (Rows < 2 || Cols < 2)
	return (NULL);

LatPerDEM = (GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue) / RowMaps;
LonPerDEM = (GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue) / ColMaps;

LowDEMLat = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue + RowMap * LatPerDEM; 
HighDEMLat = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue + (RowMap + 1) * LatPerDEM;
HighDEMLon = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - ColMap * LonPerDEM; 
LowDEMLon = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - (ColMap + 1) * LonPerDEM;

CenterLat = (GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue + GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue) * 0.5;
CenterLon = (GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue + GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue) * 0.5;
LatInc = LatPerDEM / (Rows - 1);
LonInc = LonPerDEM / (Cols - 1);

BaseElev = TerraType.AnimPar[WCS_EFFECTS_TERRAINTYPE_ANIMPAR_BASEELEV].CurValue;
ElevRange = TerraType.AnimPar[WCS_EFFECTS_TERRAINTYPE_ANIMPAR_ELEVRANGE].CurValue;

if (TerraType.TexRoot[WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT] &&
	TerraType.TexRoot[WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT]->Enabled &&
	(Tex = TerraType.TexRoot[WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT]->Tex) &&
	Tex->Enabled &&
	Tex->UsesSeed())
	{
	SeedXOffset = TerraType.Seed * 93.0;	// 93 is arbitrary
	SeedYOffset = TerraType.Seed * 77.0;	// 77 is arbitrary but not a related factor of 93
	} // if
else
	{
	SeedXOffset = SeedYOffset = 0;
	} // else

TexData.VDEM[0] = &Vert;

if (MyDEM)
	{
	if (MyDEM->LatEntries() != (unsigned long)Rows || MyDEM->LonEntries() != (unsigned long)Cols)
		MyDEM->FreeRawElevs();
	CreateDEM = 0;
	} // if
else
	CreateDEM = 1;

// allocate DEM if not passed
if (MyDEM || (MyDEM = new DEM))
	{
	MyDEM->SetLatEntries(Rows);
	MyDEM->SetLonEntries(Cols);
	MyDEM->SetBounds(HighDEMLat, LowDEMLat, HighDEMLon, LowDEMLon);
	MyDEM->SetLatStep(LatInc);
	MyDEM->SetLonStep(LonInc);
	MyDEM->SetElScale(ELSCALE_METERS);
	MyDEM->SetElDatum(0.0);
	TexData.MetersPerDegLat = MetersPerDegLat;
	TexData.TexRefLat = CenterLat;
	TexData.TexRefLon = CenterLon;

	sprintf(Str, "DEM %d/%d", ColMap * RowMaps + RowMap + 1, ColMaps * RowMaps);
	BWMD = new BusyWin(Str, Rows, 'BWMD', 0);

	if (! MyDEM->Map())
		MyDEM->AllocRawMap();
	for (LatRow = 0, LatPt = LowDEMLat; LatRow < Rows; LatRow ++, LatPt += LatInc)
		{
		MetersPerDegLon = LonScale(PlanetRad, LatPt);
		TexData.MetersPerDegLon = MetersPerDegLon;
		CalcY = (LatPt - CenterLat) * MetersPerDegLat;

		for (LonCol = 0, LonPt = HighDEMLon; LonCol < Cols; LonCol ++, LonPt -= LonInc)
			{
			CalcX = (CenterLon - LonPt) * MetersPerDegLon;
			// fill in texture data
			Vert.xyz[0] = Vert.XYZ[0] = CalcX + SeedXOffset;
			Vert.xyz[1] = Vert.XYZ[1] = CalcY + SeedYOffset;
			//TexData.LowX = TexData.HighX = CalcX + SeedXOffset;
			//TexData.LowY = TexData.HighY = CalcY + SeedYOffset; 
			TexData.Latitude = Vert.Lat = TexData.TLatRange[0] = TexData.TLatRange[1] = TexData.TLatRange[2] = TexData.TLatRange[3] = LatPt;
			TexData.Longitude = Vert.Lon = TexData.TLonRange[0] = TexData.TLonRange[1] = TexData.TLonRange[2] = TexData.TLonRange[3] = LonPt;
			// evaluate texture
			Value[0] = 0.0;
			if (TerraType.TexRoot[WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT] && TerraType.TexRoot[WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT]->Enabled)
				{
				TerraType.TexRoot[WCS_EFFECTS_TERRAINTYPE_TEXTURE_DISPLACEMENT]->Eval(Value, &TexData);
				} // if
			// multiply by elev range and add to elev base
			ElevPt = ElevRange * Value[0] + BaseElev;
			// store in elevation array
			MyDEM->StoreElevation(LatRow, LonCol, (float)ElevPt);
			} // for
		if (BWMD)
			{
			if (BWMD->Update(LatRow + 1))
				{
				Success = 0;
				break;
				} // if
			} // if
		} // for
	} // if

if (BWMD)
	delete BWMD;

if (! Success && MyDEM && CreateDEM)
	delete MyDEM;
else
	rVal = MyDEM;

return (rVal);

} // TerraGenerator::MakeTerrain

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int TerraGenerator::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
TerraGenerator *CurrentTG = NULL;
SearchQuery *CurrentQuery = NULL;
ThematicMap *CurrentTheme = NULL;
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
						} // if Images
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
					else if (! strnicmp(ReadBuf, "ThemeMap", 8))
						{
						if (CurrentTheme = new ThematicMap(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentTheme->Load(ffile, Size, ByteFlip);
							}
						} // if thematic map
					else if (! strnicmp(ReadBuf, "TerraGen", 8))
						{
						if (CurrentTG = new TerraGenerator(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentTG->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if 3d object
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

if (Success == 1 && CurrentTG)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentTG);
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

} // TerraGenerator::LoadObject

/*===========================================================================*/

int TerraGenerator::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
EffectList *CurEffect, *Queries = NULL, *Themes = NULL, *Coords = NULL;
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

if (GeneralEffect::BuildFileComponentsList(&Queries, &Themes, &Coords)
	&& BuildFileComponentsList(&Coords))
	{
	#ifdef WCS_BUILD_VNS
	CurEffect = Coords;
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

	CurEffect = Queries;
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

	#ifdef WCS_THEMATIC_MAP
	CurEffect = Themes;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "ThemeMap");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((ThematicMap *)CurEffect->Me)->Save(ffile))
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
						} // if ThemeMap saved 
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
	#endif // WCS_THEMATIC_MAP

	while (Coords)
		{
		CurEffect = Coords;
		Coords = Coords->Next;
		delete CurEffect;
		} // while
	while (Queries)
		{
		CurEffect = Queries;
		Queries = Queries->Next;
		delete CurEffect;
		} // while
	while (Themes)
		{
		CurEffect = Themes;
		Themes = Themes->Next;
		delete CurEffect;
		} // while
	} // if

// TerraGenerator
strcpy(StrBuf, "TerraGen");
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
			} // if TerraGenerator saved 
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

} // TerraGenerator::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::TerraGenerator_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
TerraGenerator *Current;

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
						if (Current = new TerraGenerator(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (TerraGenerator *)FindDuplicateByName(Current->EffectType, Current))
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

} // EffectsLib::TerraGenerator_Load()

/*===========================================================================*/

ULONG EffectsLib::TerraGenerator_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
TerraGenerator *Current;

Current = Generator;
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
					} // if terrain generator saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (TerraGenerator *)Current->Next;
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

} // EffectsLib::TerraGenerator_Save()

/*===========================================================================*/
