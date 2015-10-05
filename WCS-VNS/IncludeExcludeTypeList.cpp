// IncludeExcludeTypeList.cpp
// Code file for IncludeExcludeTypeList management
// Built from scratch on 2/13/02 by Gary R. Huber
// Copyright 2002 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "IncludeExcludeTypeList.h"
#include "Application.h"
#include "Useful.h"
#include "EffectsLib.h"
#include "Notify.h"
#include "Requester.h"
#include "AppMem.h"

TypeList::TypeList(char NewType)
{

Next = NULL;
TypeNumber = NewType;

} // TypeList::TypeList

/*===========================================================================*/
/*===========================================================================*/

// static method
char *IncludeExcludeTypeList::GetTypeName(unsigned char MatchNumber)
{

switch (MatchNumber)
	{
	case WCS_INCLEXCLTYPE_TERRAIN:
		{
		return ("Terrain");
		} // 
	case WCS_INCLEXCLTYPE_SNOW:
		{
		return ("Snow");
		} // 
	case WCS_INCLEXCLTYPE_WATER:
		{
		return ("Water");
		} // 
	case WCS_INCLEXCLTYPE_SKY:
		{
		return ("Sky");
		} // 
	case WCS_INCLEXCLTYPE_CLOUD:
		{
		return ("Cloud");
		} // 
	case WCS_INCLEXCLTYPE_CELESTIAL:
		{
		return ("Celestial Object");
		} // 
	case WCS_INCLEXCLTYPE_STAR:
		{
		return ("Star");
		} // 
	case WCS_INCLEXCLTYPE_3DOBJECT:
		{
		return ("3D Object");
		} // 
	case WCS_INCLEXCLTYPE_FOLIAGE:
		{
		return ("Foliage (Image Object)");
		} // 
	case WCS_INCLEXCLTYPE_FENCE:
		{
		return ("Wall");
		} // 
	case WCS_INCLEXCLTYPE_BACKGROUND:
		{
		return ("Camera Background");
		} // 
	case WCS_INCLEXCLTYPE_POSTPROC:
		{
		return ("Post Process Event");
		} // 
	case WCS_INCLEXCLTYPE_VECTOR:
		{
		return ("Vector");
		} // 
	default:
		break;
	} // switch

return (NULL);

} // IncludeExcludeTypeList::GetTypeName

/*===========================================================================*/

IncludeExcludeTypeList::IncludeExcludeTypeList(RasterAnimHost *RAHost)
: RasterAnimHost(RAHost)
{

Include = 0; 
Enabled = 0; 
ItemTypes = NULL;

} // IncludeExcludeTypeList::IncludeExcludeTypeList

/*===========================================================================*/

IncludeExcludeTypeList::~IncludeExcludeTypeList()
{

DeleteTypeList();

} // IncludeExcludeTypeList::~IncludeExcludeTypeList

/*===========================================================================*/

void IncludeExcludeTypeList::DeleteTypeList(void)
{
TypeList *CurType = ItemTypes, *DelType;

while (CurType)
	{
	DelType = CurType;
	CurType = CurType->Next;
	delete DelType;
	} // if

ItemTypes = NULL;

} // IncludeExcludeTypeList::DeleteTypeList

/*===========================================================================*/

int IncludeExcludeTypeList::PassTest(unsigned char TestMe)
{
TypeList *CurType = ItemTypes;

if (Enabled)
	{
	while (CurType)
		{
		if (CurType->TypeNumber == TestMe)
			return (Include);
		CurType = CurType->Next;
		} // if

	return (! Include);
	} // if

return (1);

} // IncludeExcludeTypeList::PassTest

/*===========================================================================*/

TypeList *IncludeExcludeTypeList::AddType(unsigned char AddMe)
{
TypeList **CurType = &ItemTypes;
NotifyTag Changes[2];

while (*CurType)
	{
	if ((*CurType)->TypeNumber == AddMe)
		return (NULL);
	CurType = &(*CurType)->Next;
	} // while
if (*CurType = new TypeList())
	{
	(*CurType)->TypeNumber = AddMe;
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
return (*CurType);

} // IncludeExcludeTypeList::AddType

/*===========================================================================*/

int IncludeExcludeTypeList::RemoveType(unsigned char RemoveMe)
{
TypeList *CurType = ItemTypes, *LastType;
int Removed = 0;
NotifyTag Changes[2];

LastType = NULL;
while (CurType)
	{
	RepeatFirst:
	if (CurType->TypeNumber == RemoveMe)
		{
		if (LastType)
			{
			LastType->Next = CurType->Next;
			delete CurType;
			Removed ++;
			CurType = LastType;
			} // if
		else
			{
			ItemTypes = CurType->Next;
			delete CurType;
			Removed ++;
			CurType = ItemTypes;
			if (! CurType)
				break;
			goto RepeatFirst;
			} // else
		} // if
	LastType = CurType;
	CurType = CurType->Next;
	} // while

if (Removed)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

return (Removed);

} // IncludeExcludeTypeList::RemoveType

/*===========================================================================*/

void IncludeExcludeTypeList::Copy(IncludeExcludeTypeList *CopyTo, IncludeExcludeTypeList *CopyFrom)
{
long Result = -1;
TypeList *NextItem, **ToItem;

while (CopyTo->ItemTypes)
	{
	NextItem = CopyTo->ItemTypes;
	CopyTo->ItemTypes = CopyTo->ItemTypes->Next;
	delete NextItem;
	} // if
NextItem = CopyFrom->ItemTypes;
ToItem = &CopyTo->ItemTypes;
while (NextItem)
	{
	if (*ToItem = new TypeList())
		{
		(*ToItem)->TypeNumber = NextItem->TypeNumber;
		} // if
	NextItem = NextItem->Next;
	} // while

CopyTo->Enabled = CopyFrom->Enabled;
CopyTo->Include = CopyFrom->Include;
RasterAnimHost::Copy(CopyTo, CopyFrom);

} // IncludeExcludeTypeList::Copy

/*===========================================================================*/

ULONG IncludeExcludeTypeList::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
unsigned char TypeNumber;
union MultiVal MV;
TypeList **CurItem = &ItemTypes;

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
					case WCS_INCLUDEEXCLUDE_TYPENUMBER:
						{
						BytesRead = ReadBlock(ffile, (char *)&TypeNumber, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (*CurItem = new TypeList(TypeNumber))
							{
							CurItem = &(*CurItem)->Next;
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

} // IncludeExcludeTypeList::Load

/*===========================================================================*/

unsigned long int IncludeExcludeTypeList::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
TypeList *CurItem;

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

CurItem = ItemTypes;
while (CurItem)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_INCLUDEEXCLUDE_TYPENUMBER, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (char *)&CurItem->TypeNumber)) == NULL)
	   goto WriteError;
	TotalWritten += BytesWritten;
	CurItem = CurItem->Next;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // IncludeExcludeTypeList::Save

/*===========================================================================*/

char *IncludeExcludeTypeList::GetCritterName(RasterAnimHost *Test)
{

return ("");

} // IncludeExcludeTypeList::GetCritterName

/*===========================================================================*/

char IncludeExcludeTypeList::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (DropType == WCS_EFFECTSSUBCLASS_OBJECT3D ||
	DropType == WCS_EFFECTSSUBCLASS_LAKE ||
	DropType == WCS_EFFECTSSUBCLASS_ECOSYSTEM ||
	DropType == WCS_EFFECTSSUBCLASS_CLOUD ||
	DropType == WCS_EFFECTSSUBCLASS_FOLIAGE ||
	DropType == WCS_EFFECTSSUBCLASS_STREAM ||
	DropType == WCS_EFFECTSSUBCLASS_CELESTIAL ||
	DropType == WCS_EFFECTSSUBCLASS_STARFIELD ||
	DropType == WCS_EFFECTSSUBCLASS_GROUND ||
	DropType == WCS_EFFECTSSUBCLASS_SNOW ||
	DropType == WCS_EFFECTSSUBCLASS_SKY ||
	DropType == WCS_EFFECTSSUBCLASS_CAMERA ||
	DropType == WCS_EFFECTSSUBCLASS_POSTPROC ||
	DropType == WCS_EFFECTSSUBCLASS_FENCE ||
	DropType == WCS_RAHOST_OBJTYPE_CONTROLPT ||
	DropType == WCS_RAHOST_OBJTYPE_VECTOR)
	return (1);

return (0);

} // IncludeExcludeTypeList::GetRAHostDropOK

/*===========================================================================*/

int IncludeExcludeTypeList::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
RasterAnimHostProperties Prop;
NotifyTag Changes[2];
int Success = 0;
char QueryStr[256], NameStr[128];
unsigned char NewType = 0;

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (IncludeExcludeTypeList *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (IncludeExcludeTypeList *)DropSource->DropSource);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber >= WCS_EFFECTSSUBCLASS_LAKE && DropSource->TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
	{
	switch (DropSource->TypeNumber)
		{
		case WCS_EFFECTSSUBCLASS_ECOSYSTEM:
		case WCS_EFFECTSSUBCLASS_GROUND:
			{
			NewType = WCS_INCLEXCLTYPE_TERRAIN;
			break;
			} // 
		case WCS_EFFECTSSUBCLASS_SNOW:
			{
			NewType = WCS_INCLEXCLTYPE_SNOW;
			break;
			} // 
		case WCS_EFFECTSSUBCLASS_LAKE:
		case WCS_EFFECTSSUBCLASS_STREAM:
			{
			NewType = WCS_INCLEXCLTYPE_WATER;
			break;
			} // 
		case WCS_EFFECTSSUBCLASS_SKY:
			{
			NewType = WCS_INCLEXCLTYPE_SKY;
			break;
			} // 
		case WCS_EFFECTSSUBCLASS_CLOUD:
			{
			NewType = WCS_INCLEXCLTYPE_CLOUD;
			break;
			} // 
		case WCS_EFFECTSSUBCLASS_CELESTIAL:
			{
			NewType = WCS_INCLEXCLTYPE_CELESTIAL;
			break;
			} // 
		case WCS_EFFECTSSUBCLASS_STARFIELD:
			{
			NewType = WCS_INCLEXCLTYPE_STAR;
			break;
			} // 
		case WCS_EFFECTSSUBCLASS_OBJECT3D:
			{
			NewType = WCS_INCLEXCLTYPE_3DOBJECT;
			break;
			} // 
		case WCS_EFFECTSSUBCLASS_FOLIAGE:
			{
			NewType = WCS_INCLEXCLTYPE_FOLIAGE;
			break;
			} // 
		case WCS_EFFECTSSUBCLASS_FENCE:
			{
			NewType = WCS_INCLEXCLTYPE_FENCE;
			break;
			} // 
		case WCS_EFFECTSSUBCLASS_CAMERA:
			{
			NewType = WCS_INCLEXCLTYPE_BACKGROUND;
			break;
			} // 
		case WCS_EFFECTSSUBCLASS_POSTPROC:
			{
			NewType = WCS_INCLEXCLTYPE_POSTPROC;
			break;
			} // 
		case WCS_RAHOST_OBJTYPE_VECTOR:
		case WCS_RAHOST_OBJTYPE_CONTROLPT:
			{
			NewType = WCS_INCLEXCLTYPE_VECTOR;
			break;
			} // 
		default:
			break;
		} // switch
	Success = AddType(NewType) ? 1: 0;
	} // else if

return (Success);

} // IncludeExcludeTypeList::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long IncludeExcludeTypeList::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_MATERIAL | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // IncludeExcludeTypeList::GetRAFlags

/*===========================================================================*/

void IncludeExcludeTypeList::GetRAHostProperties(RasterAnimHostProperties *Prop)
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

} // IncludeExcludeTypeList::GetRAHostProperties

/*===========================================================================*/

int IncludeExcludeTypeList::SetRAHostProperties(RasterAnimHostProperties *Prop)
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

} // IncludeExcludeTypeList::SetRAHostProperties

/*===========================================================================*/

RasterAnimHost *IncludeExcludeTypeList::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{

return (NULL);

} // IncludeExcludeTypeList::GetRAHostChild

/*===========================================================================*/
