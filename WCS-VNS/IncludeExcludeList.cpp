// IncludeExcludeList.cpp
// Code file for IncludeExcludeList management
// Built from scratch on 2/13/02 by Gary R. Huber
// Copyright 2002 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "IncludeExcludeList.h"
#include "Application.h"
#include "Useful.h"
#include "EffectsLib.h"
#include "Notify.h"
#include "Requester.h"
#include "AppMem.h"

NameList::NameList(char *NewName, long NewClass)
{

Name = NULL;
Next = NULL;
ItemClass = 0;

if (NewName)
	{
	if (Name = (char *)AppMem_Alloc(strlen(NewName) + 1, 0))
		strcpy(Name, NewName);
	ItemClass = NewClass;
	} // if

} // NameList::NameList

/*===========================================================================*/

NameList::~NameList()
{

if (Name)
	{
	AppMem_Free(Name, strlen(Name) + 1);
	Name = NULL;
	} // if

} // NameList::~NameList

/*===========================================================================*/

const char *NameList::FindNameOfType(long SearchClass)
{
NameList *CurList = this;

while (CurList)
	{
	if (CurList->ItemClass == SearchClass)
		return (CurList->Name);
	CurList = CurList->Next;
	} // while

return (NULL);

} // NameList::FindNameOfType

/*===========================================================================*/

const char *NameList::FindNextNameOfType(long SearchClass, const char *CurName)
{
NameList *CurList = this;
int Found = 0;

while (CurList)
	{
	if (CurList->ItemClass == SearchClass)
		{
		if (Found || ! CurName)
			return (CurList->Name);
		if (! stricmp(CurName, CurList->Name))
			Found = 1;
		} // if
	CurList = CurList->Next;
	} // while

return (NULL);

} // NameList::FindNameOfType

/*===========================================================================*/

int NameList::FindNameExists(long SearchClass, const char *CurName)
{
NameList *CurList = this;

while (CurList)
	{
	if (CurList->ItemClass == SearchClass)
		{
		if (! stricmp(CurName, CurList->Name))
			return (1);
		} // if
	CurList = CurList->Next;
	} // while

return (0);

} // NameList::FindNameExists

/*===========================================================================*/
/*===========================================================================*/

IncludeExcludeList::IncludeExcludeList(RasterAnimHost *RAHost)
: RasterAnimHost(RAHost)
{

Include = 0; 
Enabled = 0; 
RAHList = NULL;
ItemNames = NULL;

} // IncludeExcludeList::IncludeExcludeList

/*===========================================================================*/

IncludeExcludeList::~IncludeExcludeList()
{

DeleteItemList();
DeleteNameList();

} // IncludeExcludeList::~IncludeExcludeList

/*===========================================================================*/

void IncludeExcludeList::DeleteItemList(void)
{
RasterAnimHostList *CurRAH = RAHList, *DelRAH;

while (CurRAH)
	{
	DelRAH = CurRAH;
	CurRAH = CurRAH->Next;
	delete DelRAH;
	} // if

RAHList = NULL;

} // IncludeExcludeList::DeleteItemList

/*===========================================================================*/

void IncludeExcludeList::DeleteNameList(void)
{
NameList *CurName = ItemNames, *DelName;

while (CurName)
	{
	DelName = CurName;
	CurName = CurName->Next;
	delete DelName;
	} // if

ItemNames = NULL;

} // IncludeExcludeList::DeleteNameList

/*===========================================================================*/

int IncludeExcludeList::PassTest(RasterAnimHost *TestMe)
{
RasterAnimHostList *CurRAH = RAHList;

if (Enabled)
	{
	while (CurRAH)
		{
		if (CurRAH->Me == TestMe)
			return (Include);
		CurRAH = CurRAH->Next;
		} // if

	return (! Include);
	} // if

return (1);

} // IncludeExcludeList::PassTest

/*===========================================================================*/

RasterAnimHostList *IncludeExcludeList::AddRAHost(RasterAnimHost *AddMe)
{
RasterAnimHostList **CurItem = &RAHList;
NotifyTag Changes[2];

if (AddMe)
	{
	while (*CurItem)
		{
		if ((*CurItem)->Me == AddMe)
			return (NULL);
		CurItem = &(*CurItem)->Next;
		} // while
	if (*CurItem = new RasterAnimHostList())
		{
		(*CurItem)->Me = AddMe;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	return (*CurItem);
	} // if

return (NULL);

} // IncludeExcludeList::AddRAHost

/*===========================================================================*/

int IncludeExcludeList::RemoveRAHost(RasterAnimHost *RemoveMe)
{
RasterAnimHostList *CurRAH = RAHList, *LastRAH;
NotifyTag Changes[2];
int Removed = 0;

LastRAH = NULL;
while (CurRAH)
	{
	RepeatFirst:
	if (CurRAH->Me == RemoveMe)
		{
		if (LastRAH)
			{
			LastRAH->Next = CurRAH->Next;
			delete CurRAH;
			Removed ++;
			CurRAH = LastRAH;
			} // if
		else
			{
			RAHList = CurRAH->Next;
			delete CurRAH;
			Removed ++;
			CurRAH = RAHList;
			if (! CurRAH)
				break;
			goto RepeatFirst;
			} // else
		} // if
	LastRAH = CurRAH;
	CurRAH = CurRAH->Next;
	} // while

if (Removed)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

return (Removed);

} // IncludeExcludeList::RemoveRAHost

/*===========================================================================*/

void IncludeExcludeList::Copy(IncludeExcludeList *CopyTo, IncludeExcludeList *CopyFrom)
{
RasterAnimHostList *NextItem, **ToItem;
long Result = -1;

while (CopyTo->RAHList)
	{
	NextItem = CopyTo->RAHList;
	CopyTo->RAHList = CopyTo->RAHList->Next;
	delete NextItem;
	} // if
NextItem = CopyFrom->RAHList;
ToItem = &CopyTo->RAHList;
while (NextItem)
	{
	if (*ToItem = new RasterAnimHostList())
		{
		if (GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
			{
			(*ToItem)->Me = GlobalApp->CopyToEffectsLib->MatchNameMakeEffect((GeneralEffect *)NextItem->Me);
			} // if no need to make another copy, its all in the family
		if ((*ToItem)->Me)
			ToItem = &(*ToItem)->Next;
		else
			{
			delete *ToItem;
			*ToItem = NULL;
			} // if
		} // if
	NextItem = NextItem->Next;
	} // while

CopyTo->Enabled = CopyFrom->Enabled;
CopyTo->Include = CopyFrom->Include;
RasterAnimHost::Copy(CopyTo, CopyFrom);

} // IncludeExcludeList::Copy

/*===========================================================================*/

ULONG IncludeExcludeList::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
NameList **CurItem = &ItemNames;
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
long ItemClass;
union MultiVal MV;
char MatchName[WCS_EFFECT_MAXNAMELENGTH];

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
					default:
						break;
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_INCLUDEEXCLUDE_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_INCLUDEEXCLUDE_INCLUDE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Include, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_INCLUDEEXCLUDE_ITEMCLASS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ItemClass, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_INCLUDEEXCLUDE_ITEMNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							if (*CurItem = new NameList(MatchName, ItemClass))
								{
								CurItem = &(*CurItem)->Next;
								} // if
							} // if
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

} // IncludeExcludeList::Load

/*===========================================================================*/

unsigned long int IncludeExcludeList::Save(FILE *ffile)
{
RasterAnimHostProperties Prop;
RasterAnimHostList *CurItem;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_INCLUDEEXCLUDE_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Enabled)) == NULL)
   goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_INCLUDEEXCLUDE_INCLUDE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Include)) == NULL)
   goto WriteError;
TotalWritten += BytesWritten;

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPENUMBER;

CurItem = RAHList;
while (CurItem)
	{
	if (CurItem->Me)
		{
		CurItem->Me->GetRAHostProperties(&Prop);
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_INCLUDEEXCLUDE_ITEMCLASS, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
			WCS_BLOCKTYPE_LONGINT, (char *)&Prop.TypeNumber)) == NULL)
		   goto WriteError;
		TotalWritten += BytesWritten;
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_INCLUDEEXCLUDE_ITEMNAME, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Prop.Name) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)Prop.Name)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	CurItem = CurItem->Next;
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

} // IncludeExcludeList::Save

/*===========================================================================*/

void IncludeExcludeList::ResolveLoadLinkages(EffectsLib *Lib)
{
RasterAnimHost *NewItem;
RasterAnimHostList **CurItem = &RAHList;
NameList *CurName = ItemNames;

DeleteItemList();

while (CurName)
	{
	if (NewItem = Lib->FindByName(CurName->ItemClass, CurName->Name))
		{
		if (*CurItem = new RasterAnimHostList())
			{
			(*CurItem)->Me = NewItem;
			CurItem = &(*CurItem)->Next;
			} // if
		} // if
	CurName = CurName->Next;
	} // while

DeleteNameList();

} // IncludeExcludeList::ResolveLoadLinkages

/*===========================================================================*/

char *IncludeExcludeList::GetCritterName(RasterAnimHost *Test)
{

return ("");

} // IncludeExcludeList::GetCritterName

/*===========================================================================*/

char IncludeExcludeList::GetRAHostDropOK(long DropType)
{
char rVal = 0;

if (DropType == GetRAHostTypeNumber())
	rVal = 1;
else if (DropType == WCS_EFFECTSSUBCLASS_OBJECT3D ||
	DropType == WCS_EFFECTSSUBCLASS_LAKE ||
	DropType == WCS_EFFECTSSUBCLASS_ECOSYSTEM ||
	DropType == WCS_EFFECTSSUBCLASS_CLOUD ||
	DropType == WCS_EFFECTSSUBCLASS_CMAP ||
	DropType == WCS_EFFECTSSUBCLASS_FOLIAGE ||
	DropType == WCS_EFFECTSSUBCLASS_STREAM ||
	//DropType == WCS_EFFECTSSUBCLASS_MATERIAL ||
	DropType == WCS_EFFECTSSUBCLASS_CELESTIAL ||
	//DropType == WCS_EFFECTSSUBCLASS_STARFIELD ||
	DropType == WCS_EFFECTSSUBCLASS_GROUND ||
	//DropType == WCS_EFFECTSSUBCLASS_SNOW ||
	//DropType == WCS_EFFECTSSUBCLASS_SKY ||
	DropType == WCS_EFFECTSSUBCLASS_ATMOSPHERE ||
	DropType == WCS_EFFECTSSUBCLASS_FENCE)
	rVal = 1;

return (rVal);

} // IncludeExcludeList::GetRAHostDropOK

/*===========================================================================*/

int IncludeExcludeList::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
RasterAnimHostProperties Prop;
NotifyTag Changes[2];
int Success = 0;
char QueryStr[256], NameStr[128];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (IncludeExcludeList *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (IncludeExcludeList *)DropSource->DropSource);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber >= WCS_EFFECTSSUBCLASS_LAKE && DropSource->TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
	{
	Success = AddRAHost(DropSource->DropSource) ? 1: 0;
	} // else if

return (Success);

} // IncludeExcludeList::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long IncludeExcludeList::GetRAFlags(unsigned long Mask)
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
if (Mask & WCS_RAHOST_FLAGBIT_CHILDREN)
	{
	if (RAHList)
		Flags |= WCS_RAHOST_FLAGBIT_CHILDREN;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_MATERIAL | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // IncludeExcludeList::GetRAFlags

/*===========================================================================*/

void IncludeExcludeList::GetRAHostProperties(RasterAnimHostProperties *Prop)
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

} // IncludeExcludeList::GetRAHostProperties

/*===========================================================================*/

int IncludeExcludeList::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_ENABLED)
		{
		Enabled = (Prop->Flags & WCS_RAHOST_FLAGBIT_ENABLED) ? 1: 0;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPSOURCE)
	{
	Success = ProcessRAHostDragDrop(Prop);
	} // if

return (Success);

} // IncludeExcludeList::SetRAHostProperties

/*===========================================================================*/

RasterAnimHost *IncludeExcludeList::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
RasterAnimHostList *CurItem;
char Found = 0;

if (! Current)
	Found = 1;
CurItem = RAHList;
while (CurItem)
	{
	if (Found)
		return (CurItem->Me);
	if (Current == CurItem->Me)
		Found = 1;
	CurItem = CurItem->Next;
	} // while

return (NULL);

} // IncludeExcludeList::GetRAHostChild

/*===========================================================================*/
