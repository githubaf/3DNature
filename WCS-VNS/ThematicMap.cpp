// ThematicMap.cpp
// For managing Cmap Effects
// Built from scratch ThematicMap.cpp on 06/12/97 by Gary R. Huber
// Copyright 1997 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Application.h"
#include "Conservatory.h"
#include "Joe.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Raster.h"
#include "requester.h"
#include "ThematicMapEditGUI.h"
#include "Toolbar.h"
#include "AppMem.h"
#include "Render.h"
#include "DEM.h"
#include "Database.h"
#include "Project.h"
#include "Interactive.h"
#include "Lists.h"
#include "FeatureConfig.h"


void ThematicOwner::Copy(ThematicOwner *CopyTo, ThematicOwner *CopyFrom)
{
long Ct, Result = -1;
#ifdef WCS_THEMATIC_MAP
NotifyTag Changes[2];
#endif // WCS_THEMATIC_MAP

for (Ct = 0; Ct < GetNumThemes(); Ct ++)
	{
	CopyTo->SetThemePtr(Ct, NULL);
	#ifdef WCS_THEMATIC_MAP
	if (CopyFrom->GetTheme(Ct))
		{
		if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib || GlobalApp->CopyToEffectsLib != GlobalApp->AppEffects)
			{
			CopyTo->SetThemePtr(Ct, (ThematicMap *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->GetTheme(Ct)));
			} // if no need to make another copy, its all in the family
		else
			{
			if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->GetTheme(Ct)->EffectType, CopyFrom->GetTheme(Ct)->Name))
				{
				Result = UserMessageCustom("Copy Thematic Map", "How do you wish to resolve Thematic Map name collisions?\n\nLink to existing Thematic Maps, replace existing Thematic Maps, or create new Thematic Maps?",
					"Link", "Create", "Overwrite", 1);
				} // if
			if (Result <= 0)
				{
				CopyTo->SetThemePtr(Ct, (ThematicMap *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->GetTheme(Ct)->EffectType, NULL, CopyFrom->GetTheme(Ct)));
				} // if create new
			else if (Result == 1)
				{
				CopyTo->SetThemePtr(Ct, (ThematicMap *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->GetTheme(Ct)));
				} // if link to existing
			else if (CopyTo->SetThemePtr(Ct, (ThematicMap *)GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->GetTheme(Ct)->EffectType, CopyFrom->GetTheme(Ct)->Name)))
				{
				CopyTo->GetTheme(Ct)->Copy(CopyTo->GetTheme(Ct), CopyFrom->GetTheme(Ct));
				Changes[0] = MAKE_ID(CopyTo->GetTheme(Ct)->GetNotifyClass(), CopyTo->GetTheme(Ct)->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, CopyTo->GetTheme(Ct));
				} // else if found and overwrite
			else
				{
				CopyTo->SetThemePtr(Ct, (ThematicMap *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->GetTheme(Ct)->EffectType, NULL, CopyFrom->GetTheme(Ct)));
				} // else
			} // else better copy or overwrite it since its important to get just the right object
		} // if
	#endif // WCS_THEMATIC_MAP
	} // for

} // ThematicOwner::Copy

/*===========================================================================*/

long ThematicOwner::GetThemeNumberFromAddr(ThematicMap **Addr)
{
long Ct;

for (Ct = 0; Ct < GetNumThemes(); Ct ++)
	{
	if (GetThemeAddr(Ct) == Addr)
		{
		return (Ct);
		} // if
	} // for

return (-1);

} // ThematicOwner::GetThemeNumberFromAddr

/*===========================================================================*/

int ThematicOwner::GetThemeUnique(long ThemeNum)
{
ThematicMap *CurTheme;
long Ct;

if (CurTheme = GetTheme(ThemeNum))
	{
	for (Ct = 0; Ct < ThemeNum; Ct ++)
		{
		if (GetTheme(Ct) == CurTheme)
			{
			if (Ct < ThemeNum)
				return (0);
			} // if
		} // for
	return (1);
	} // if

return (0);

} // ThematicOwner::GetThemeUnique

/*===========================================================================*/

int ThematicOwner::BuildFileComponentsList(EffectList **Themes)
{
#ifdef WCS_THEMATIC_MAP
long Ct;
EffectList **ListPtr;

for (Ct = 0; Ct < GetNumThemes(); Ct ++)
	{
	if (GetTheme(Ct))
		{
		ListPtr = Themes;
		while (*ListPtr)
			{
			if ((*ListPtr)->Me == GetTheme(Ct))
				break;
			ListPtr = &(*ListPtr)->Next;
			} // if
		if (! (*ListPtr))
			{
			if (*ListPtr = new EffectList())
				(*ListPtr)->Me = GetTheme(Ct);
			else
				return (0);
			} // if
		} // if
	} // for
#endif // WCS_THEMATIC_MAP

return (1);

} // ThematicOwner::BuildFileComponentsList

/*===========================================================================*/
/*===========================================================================*/

ThematicMap::ThematicMap()
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP;
SetDefaults();

} // ThematicMap::ThematicMap

/*===========================================================================*/

ThematicMap::ThematicMap(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP;
SetDefaults();

} // ThematicMap::ThematicMap

/*===========================================================================*/

ThematicMap::ThematicMap(RasterAnimHost *RAHost, EffectsLib *Library, ThematicMap *Proto)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_THEMATICMAP;
Prev = Library->LastTheme;
if (Library->LastTheme)
	{
	Library->LastTheme->Next = this;
	Library->LastTheme = this;
	} // if
else
	{
	Library->Theme = Library->LastTheme = this;
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
	strcpy(NameBase, "Thematic Map");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // ThematicMap::ThematicMap

/*===========================================================================*/

ThematicMap::~ThematicMap()
{

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->THM && GlobalApp->GUIWins->THM->GetActive() == this)
		{
		delete GlobalApp->GUIWins->THM;
		GlobalApp->GUIWins->THM = NULL;
		} // if
	} // if

} // ThematicMap::~ThematicMap

/*===========================================================================*/

void ThematicMap::SetDefaults(void)
{

NullTreatment = WCS_THEMATICMAP_NULLTREATMENT_IGNORE;
AttribField[0][0] = 0;
AttribField[1][0] = 0;
AttribField[2][0] = 0;
AttribFactor[0] = AttribFactor[1] = AttribFactor[2] = 1.0;
OutputChannels = WCS_THEMATICMAP_OUTCHANNELS_ONE;
NullConstant[0] = NullConstant[1] = NullConstant[2] = 0.0;
ConditionEnabled = 0;

} // ThematicMap::SetDefaults

/*===========================================================================*/

void ThematicMap::Copy(ThematicMap *CopyTo, ThematicMap *CopyFrom)
{

CopyTo->OutputChannels = CopyFrom->OutputChannels;
CopyTo->NullTreatment = CopyFrom->NullTreatment;
CopyTo->ConditionEnabled = CopyFrom->ConditionEnabled;
CopyTo->AttribFactor[0] = CopyFrom->AttribFactor[0];
CopyTo->AttribFactor[1] = CopyFrom->AttribFactor[1];
CopyTo->AttribFactor[2] = CopyFrom->AttribFactor[2];
CopyTo->NullConstant[0] = CopyFrom->NullConstant[0];
CopyTo->NullConstant[1] = CopyFrom->NullConstant[1];
CopyTo->NullConstant[2] = CopyFrom->NullConstant[2];
strcpy(CopyTo->AttribField[0], CopyFrom->AttribField[0]);
strcpy(CopyTo->AttribField[1], CopyFrom->AttribField[1]);
strcpy(CopyTo->AttribField[2], CopyFrom->AttribField[2]);
Condition.Copy(&CopyTo->Condition, &CopyFrom->Condition);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // ThematicMap::Copy

/*===========================================================================*/

char *ThematicMap::GetCritterName(RasterAnimHost *Test)
{

return ("");

} // ThematicMap::GetCritterName

/*===========================================================================*/

int ThematicMap::SetAttribute(LayerEntry *NewAttrib, int AttribNum)
{
NotifyTag Changes[2];

if (AttribNum < 3)
	{
	if (NewAttrib)
		{
		strncpy(AttribField[AttribNum], NewAttrib->GetName(), WCS_EFFECT_MAXNAMELENGTH);
		AttribField[AttribNum][WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
		} // if
	else
		AttribField[AttribNum][0] = 0;
	} // if
else if (AttribNum == 3)
	{
	if (NewAttrib)
		Condition.NewAttribute((char *)NewAttrib->GetName());
	else
		Condition.FreeAttribute();
	} // else
Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (1);

} // ThematicMap::SetAttribute

/*===========================================================================*/

ULONG ThematicMap::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
unsigned long ImageID = 0;

Condition.FreeAll();

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
					case WCS_EFFECTS_THEMATICMAP_OUTPUTCHANNELS:
						{
						BytesRead = ReadBlock(ffile, (char *)&OutputChannels, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_THEMATICMAP_NULLTREATMENT:
						{
						BytesRead = ReadBlock(ffile, (char *)&NullTreatment, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_THEMATICMAP_CONDITIONENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&ConditionEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_THEMATICMAP_ATTRIBFACTOR0:
						{
						BytesRead = ReadBlock(ffile, (char *)&AttribFactor[0], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_THEMATICMAP_ATTRIBFACTOR1:
						{
						BytesRead = ReadBlock(ffile, (char *)&AttribFactor[1], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_THEMATICMAP_ATTRIBFACTOR2:
						{
						BytesRead = ReadBlock(ffile, (char *)&AttribFactor[2], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_THEMATICMAP_ATTRIBFIELD0:
						{
						BytesRead = ReadBlock(ffile, (char *)AttribField[0], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_THEMATICMAP_ATTRIBFIELD1:
						{
						BytesRead = ReadBlock(ffile, (char *)AttribField[1], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_THEMATICMAP_ATTRIBFIELD2:
						{
						BytesRead = ReadBlock(ffile, (char *)AttribField[2], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_THEMATICMAP_NULLCONSTANT0:
						{
						BytesRead = ReadBlock(ffile, (char *)&NullConstant[0], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_THEMATICMAP_NULLCONSTANT1:
						{
						BytesRead = ReadBlock(ffile, (char *)&NullConstant[1], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_THEMATICMAP_NULLCONSTANT2:
						{
						BytesRead = ReadBlock(ffile, (char *)&NullConstant[2], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_THEMATICMAP_CONDITION:
						{
						BytesRead = Condition.Load(ffile, Size, ByteFlip);
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

} // ThematicMap::Load

/*===========================================================================*/

unsigned long int ThematicMap::Save(FILE *ffile)
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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_THEMATICMAP_OUTPUTCHANNELS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&OutputChannels)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_THEMATICMAP_NULLTREATMENT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&NullTreatment)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_THEMATICMAP_CONDITIONENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ConditionEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_THEMATICMAP_ATTRIBFACTOR0, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&AttribFactor[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_THEMATICMAP_ATTRIBFACTOR1, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&AttribFactor[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_THEMATICMAP_ATTRIBFACTOR2, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&AttribFactor[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_THEMATICMAP_ATTRIBFIELD0, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(AttribField[0]) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)AttribField[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_THEMATICMAP_ATTRIBFIELD1, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(AttribField[1]) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)AttribField[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_THEMATICMAP_ATTRIBFIELD2, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(AttribField[2]) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)AttribField[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_THEMATICMAP_NULLCONSTANT0, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NullConstant[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_THEMATICMAP_NULLCONSTANT1, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NullConstant[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_THEMATICMAP_NULLCONSTANT2, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NullConstant[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_EFFECTS_THEMATICMAP_CONDITION + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Condition.Save(ffile))
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

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // ThematicMap::Save

/*===========================================================================*/

void ThematicMap::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->THM)
	{
	delete GlobalApp->GUIWins->THM;
	}
GlobalApp->GUIWins->THM = new ThematicMapEditGUI(GlobalApp->AppEffects, GlobalApp->MainProj, GlobalApp->AppDB, this);
if(GlobalApp->GUIWins->THM)
	{
	GlobalApp->GUIWins->THM->Open(GlobalApp->MainProj);
	}

} // ThematicMap::Edit

/*===========================================================================*/

char ThematicMap::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);

return (0);

} // ThematicMap::GetRAHostDropOK

/*===========================================================================*/

int ThematicMap::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
int Success = 0;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (ThematicMap *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (ThematicMap *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else
	{
	Success = GeneralEffect::ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // ThematicMap::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long ThematicMap::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_THEMATIC | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // ThematicMap::GetRAFlags

/*===========================================================================*/

RasterAnimHost *ThematicMap::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{

return (NULL);

} // ThematicMap::GetRAHostChild

/*===========================================================================*/

bool ThematicMap::Eval(double Value[3], Joe *CurVec)
{
LayerStub *Stub[3];
bool AttrsFound;

if (CurVec && AttribField[0][0] && (! ConditionEnabled || Condition.PassJoe(CurVec)))
	{
	// find the attributes
	AttrsFound = false;
	// search for the required attribute or attributes and their value or values
	if (Stub[0] = CurVec->CheckIEEEAttributeExistance(AttribField[0]))
		{
		if (OutputChannels == WCS_THEMATICMAP_OUTCHANNELS_THREE && AttribField[1][0] && AttribField[2][0])
			{
			if (Stub[1] = CurVec->CheckIEEEAttributeExistance(AttribField[1]))
				{
				if (Stub[2] = CurVec->CheckIEEEAttributeExistance(AttribField[2]))
					{
					AttrsFound = true;
					} // if
				} // if
			} // if
		else
			{
			AttrsFound = true;
			Stub[1] = Stub[2] = NULL;
			} // else only one channel
		} // if
	// if all required attributes were found
	if (AttrsFound)
		{
		// get attribute values and multiply by scale factor
		if (Stub[0])
			Value[0] = CurVec->GetIEEEAttributeValue(Stub[0]) * AttribFactor[0];
		if (Stub[1])
			Value[1] = CurVec->GetIEEEAttributeValue(Stub[1]) * AttribFactor[1];
		if (Stub[2])
			Value[2] = CurVec->GetIEEEAttributeValue(Stub[2]) * AttribFactor[2];
		return (1);
		} // if
	} // else if

if (NullTreatment == WCS_THEMATICMAP_NULLTREATMENT_CONSTANT)
	{
	Value[0] = NullConstant[0];
	if (OutputChannels == WCS_THEMATICMAP_OUTCHANNELS_THREE)
		{
		Value[1] = NullConstant[1];
		Value[2] = NullConstant[2];
		} // if
	else
		{
		Value[1] = Value[2] = Value[0];
		} // else
	return (true);
	} // if

return (false);

} // ThematicMap::Eval

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int ThematicMap::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
ThematicMap *CurrentTheme = NULL;
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
						} // if DEM Bounds
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
							if ((BytesRead = CurrentTheme->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if Thematic Map
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

if (Success == 1 && CurrentTheme)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentTheme);
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

} // ThematicMap::LoadObject

/*===========================================================================*/

int ThematicMap::SaveObject(FILE *ffile, const char *SuppliedFileName)
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

if (GeneralEffect::BuildFileComponentsList(&Queries, &Themes, &Coords))
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
	} // if

// Thematic Map
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
			} // if Thematic Map saved 
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

} // ThematicMap::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::ThematicMap_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
ThematicMap *Current;

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
						if (Current = new ThematicMap(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (ThematicMap *)FindDuplicateByName(Current->EffectType, Current))
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

} // EffectsLib::ThematicMap_Load()

/*===========================================================================*/

ULONG EffectsLib::ThematicMap_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
ThematicMap *Current;

Current = Theme;
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
					} // if ThematicMap saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (ThematicMap *)Current->Next;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // EffectsLib::ThematicMap_Save()

/*===========================================================================*/
