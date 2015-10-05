// SXQueryAction.cpp
// Code module for SceneExporter Click-to-Query Action.
// Built from scratch on 12/10/04 by Gary R. Huber
// Copyright 2004 3D Nature LLC. All rights reserved.

#include "stdafx.h"
#include "SXQueryAction.h"
#include "EffectsIO.h"
#include "UsefulIO.h"
#include "Types.h"
#include "AppMem.h"
#include "Application.h"
#include "Raster.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Conservatory.h"
#include "DBEditGUI.h"
#include "Toolbar.h"
#include "Requester.h"
#include "UsefulPathString.h"
#include "Database.h"
#include "SXQueryItem.h"
#include "Project.h"
#include "UsefulIO.h"
#include "ExportControlGUI.h"
#include "Lists.h"

// define this if you want ascii output for action query files
// also found in SXQueryItem.cpp
//#define WCS_SXACTIONQUERY_ASCIIFILE

extern char *Attrib_TextSymbol;

FILE *SXQueryAction::ActionFileRecords;
FILE *SXQueryAction::ActionFile;

SXQueryAction::SXQueryAction()
{

ActionType = WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYLABEL;
ActionText = NULL;
Next = NULL;
Items = NULL;
DBItemNumbers = NULL;
RecordNum = -1;
RecordFreq = WCS_SXQUERYITEM_FREQ_UNKNOWN;
VectorList = NULL;
EffectList = NULL;
VectorEffectList = NULL;
OutputFilePath = NULL;

} // SXQueryAction::SXQueryAction

/*===========================================================================*/

SXQueryAction::~SXQueryAction()
{

RemoveStrings();
RemoveItems();
RemoveDBItemNumbers();
DestroyVectorList();

} // SXQueryAction::~SXQueryAction

/*===========================================================================*/

void SXQueryAction::Copy(SXQueryAction *CopyFrom)
{
RasterAnimHostList *NextItem, **ToItem;
RasterAnimHostProperties Prop;

ActionType = CopyFrom->ActionType;
CreateString(CopyFrom->ActionText);

RemoveItems();
NextItem = CopyFrom->Items;
// assign pointers only, do not make new items
if (GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
	{
	ToItem = &Items;
	while (NextItem)
		{
		if (NextItem->Me)
			{
			// check to see if the item still exists
			if (GlobalApp->CopyFromEffectsLib->IsEffectValid((GeneralEffect *)NextItem->Me, FALSE)
				|| GlobalApp->AppDB->ValidateJoe(NextItem->Me))
				{
				if (*ToItem = new RasterAnimHostList())
					{
					(*ToItem)->Me = NextItem->Me;
					ToItem = &(*ToItem)->Next;
					} // if
				} // if
			} // if
		NextItem = NextItem->Next;
		} // while
	} // if
else
	{
	// only link items if they already exist
	ToItem = &Items;
	Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER | WCS_RAHOST_MASKBIT_NAME;
	while (NextItem)
		{
		if (NextItem->Me)
			{
			NextItem->Me->GetRAHostProperties(&Prop);
			if (GlobalApp->CopyToEffectsLib->FindByName(Prop.TypeNumber, Prop.Name))
				{
				if (*ToItem = new RasterAnimHostList())
					{
					(*ToItem)->Me = NextItem->Me;
					ToItem = &(*ToItem)->Next;
					} // if
				} // if
			} // if
		NextItem = NextItem->Next;
		} // while
	} // else

} // SXQueryAction::Copy

/*===========================================================================*/

void SXQueryAction::RemoveStrings(void)
{

if (ActionText)
	AppMem_Free(ActionText, strlen(ActionText) + 1);
ActionText = NULL;

} // SXQueryAction::RemoveStrings

/*===========================================================================*/

void SXQueryAction::RemoveItems(void)
{
RasterAnimHostList *NextItem;

while (Items)
	{
	NextItem = Items->Next;
	delete Items;
	Items = NextItem;
	} // while

} // SXQueryAction::RemoveItems

/*===========================================================================*/

void SXQueryAction::RemoveDBItemNumbers(void)
{
NumberList *NextNumber;

while (DBItemNumbers)
	{
	NextNumber = DBItemNumbers->Next;
	delete DBItemNumbers;
	DBItemNumbers = NextNumber;
	} // while

} // SXQueryAction::RemoveDBItemNumbers

/*===========================================================================*/

void SXQueryAction::DestroyVectorList(void)
{
SXQueryVectorList *CurVectorList;
SXQueryEffectList *CurEffectList;
SXQueryVectorEffectList *CurVectorEffectList;

while (VectorList)
	{
	CurVectorList = VectorList->Next;
	delete VectorList;
	VectorList = CurVectorList;
	} // while
while (EffectList)
	{
	CurEffectList = EffectList->Next;
	delete EffectList;
	EffectList = CurEffectList;
	} // while
while (VectorEffectList)
	{
	CurVectorEffectList = VectorEffectList->Next;
	delete VectorEffectList;
	VectorEffectList = CurVectorEffectList;
	} // while

} // SXQueryAction::DestroyVectorList

/*===========================================================================*/

char *SXQueryAction::CreateString(char *NewString)
{
long Len;

if (ActionText)
	AppMem_Free(ActionText, strlen(ActionText) + 1);
ActionText = NULL;
if (NewString && (Len = (long)strlen(NewString)))
	{
	if (ActionText = (char *)AppMem_Alloc(Len + 1, 0))
		{
		strcpy(ActionText, NewString);
		} // if
	} // if

return (ActionText);

} // SXQueryAction::CreateString

/*===========================================================================*/

unsigned long int SXQueryAction::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
unsigned long int ItemTag = 0, Size, BytesRead, TotalRead = 0;
unsigned long JoeID;
long EffectType = 0;
RasterAnimHostList **CurItem = &Items;
NumberList **CurNumber = &DBItemNumbers;
union MultiVal MV;
char TempBuffer[2048];
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
			//read block size from file
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
					case WCS_EFFECTS_SXACTIONQUERY_ACTIONTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&ActionType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXACTIONQUERY_ACTIONTEXT:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempBuffer, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempBuffer[0])
							CreateString(TempBuffer);
						break;
						}
					case WCS_EFFECTS_SXACTIONQUERY_EFFECTTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&EffectType, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXACTIONQUERY_EFFECTNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							if (*CurItem = new RasterAnimHostList())
								{
								if ((*CurItem)->Me = GlobalApp->LoadToEffectsLib->FindByName(EffectType, MatchName))
									CurItem = &(*CurItem)->Next;
								else
									{
									delete *CurItem;
									*CurItem = NULL;
									} // else
								} // if
							} // if
						break;
						}
					case WCS_EFFECTS_SXACTIONQUERY_JOENUMBER:
						{
						BytesRead = ReadBlock(ffile, (char *)&JoeID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (JoeID > 0)
							{
							if (*CurNumber = new NumberList(JoeID))
								{
								CurNumber = &(*CurNumber)->Next;
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

} // SXQueryAction::Load

/*===========================================================================*/

unsigned long int SXQueryAction::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
#ifdef WCS_BUILD_VNS
unsigned long JoeID;
#endif // WCS_BUILD_VNS
RasterAnimHostList *CurItem;
RasterAnimHostProperties Prop;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXACTIONQUERY_ACTIONTYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ActionType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if (ActionText)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXACTIONQUERY_ACTIONTEXT, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(ActionText) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)ActionText)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

CurItem = Items;
Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER |	WCS_RAHOST_MASKBIT_NAME;
while (CurItem)
	{
	if (CurItem->Me)
		{
		// find out what type of object it is
		CurItem->Me->GetRAHostProperties(&Prop);
		if (Prop.Name && Prop.Name[0])
			{
			// if and effect write out the effect class then the name
			if (Prop.TypeNumber >= WCS_EFFECTSSUBCLASS_LAKE && Prop.TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
				{
				if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXACTIONQUERY_EFFECTTYPE, WCS_BLOCKSIZE_CHAR,
					WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
					WCS_BLOCKTYPE_LONGINT, (char *)&Prop.TypeNumber)) == NULL)
				   goto WriteError;
				TotalWritten += BytesWritten;
				if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXACTIONQUERY_EFFECTNAME, WCS_BLOCKSIZE_CHAR,
					WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Prop.Name) + 1),
					WCS_BLOCKTYPE_CHAR, (char *)Prop.Name)) == NULL)
					goto WriteError;
				TotalWritten += BytesWritten;
				} // if effect
			// if a Joe write out the best name or if there is none skip it
			// won't compile under WCS due to lack of UniqueLoadSaveID member of Joe
			#ifdef WCS_BUILD_VNS
			else if (Prop.TypeNumber >= WCS_RAHOST_OBJTYPE_VECTOR && Prop.TypeNumber <= WCS_RAHOST_OBJTYPE_CONTROLPT)
				{
				if ((JoeID = ((Joe *)CurItem->Me)->UniqueLoadSaveID) > 0)
					{
					if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXACTIONQUERY_JOENUMBER, WCS_BLOCKSIZE_CHAR,
						WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
						WCS_BLOCKTYPE_LONGINT, (char *)&JoeID)) == NULL)
						goto WriteError;
					TotalWritten += BytesWritten;
					} // if
				} // else if joe
			#endif // WCS_BUILD_VNS
			} // if
		} // if
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

} // SXQueryAction::Save

/*===========================================================================*/

void SXQueryAction::ResolveDBLoadLinkages(Database *HostDB, Joe **UniqueIDTable, unsigned long HighestDBID)
{
NumberList *CurDBItem = DBItemNumbers;
RasterAnimHostList **CurItem = &Items;

while (*CurItem)
	{
	CurItem = &(*CurItem)->Next;
	} // while

while (CurDBItem)
	{
	if (CurDBItem->Number > 0 && CurDBItem->Number <= HighestDBID)
		{
		if (*CurItem = new RasterAnimHostList())
			{
			if ((*CurItem)->Me = UniqueIDTable[CurDBItem->Number])
				CurItem = &(*CurItem)->Next;
			else
				{
				delete *CurItem;
				*CurItem = NULL;
				} // else
			} // if
		} // if
	CurDBItem = CurDBItem->Next;
	} // if

RemoveDBItemNumbers();

} // SXQueryAction::ResolveDBLoadLinkages

/*===========================================================================*/

RasterAnimHostList *SXQueryAction::AddItem(RasterAnimHost *AddMe, int GenNotify, RasterAnimHost *NotifyObj)
{
RasterAnimHostList **CurItem = &Items;
RasterAnimHostProperties Prop;
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
		if (GenNotify)
			{
			Changes[0] = MAKE_ID(NotifyObj->GetNotifyClass(), NotifyObj->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, NotifyObj->GetRAHostRoot());
			} // if
		} // if
	return (*CurItem);
	} // if

return (NULL);

} // SXQueryAction::AddItem

/*===========================================================================*/

long SXQueryAction::AddDBItem(RasterAnimHost *NotifyObj)
{
Joe *CurJoe;
long LastOBN = -1, Added = 0;
NotifyTag Changes[2];

if (GlobalApp->GUIWins->DBE)
	{
	if (UserMessageOKCAN("Add Database Objects", "If desired Objects are selected in the Database Editor click \"OK\". If not then click \"Cancel\", multi-select the Objects and click \"Add Items\" again."))
		{
		while (CurJoe = GlobalApp->GUIWins->DBE->GetNextSelected(LastOBN))
			{
			if (AddItem(CurJoe, FALSE, NotifyObj))
				Added ++;
			} // if
		if (Added)
			{
			Changes[0] = MAKE_ID(NotifyObj->GetNotifyClass(), NotifyObj->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, NotifyObj->GetRAHostRoot());
			} // else
		else
			{
			UserMessageOK("Add Database Objects", "There are no Objects selected in the Database Editor!");
			} // else
		} // if
	} // if
else
	{
	UserMessageOK("Add Database Objects", "When the Database Editor opens, select the desired Objects and then click \"Add Items\" again.");
	GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		WCS_TOOLBAR_ITEM_DBG, 0);
	} // else

return (Added);

} // SXQueryAction::AddDBItem

/*===========================================================================*/

long SXQueryAction::AddItemsByClass(EffectsLib *HostEffects, Database *HostDB, long ItemClass,
	RasterAnimHost *NotifyObj)
{
long NumAdded = 0;
GeneralEffect *CurEffect;
Joe *CurJoe;
NotifyTag Changes[2];

if (ItemClass >= WCS_EFFECTSSUBCLASS_LAKE && ItemClass < WCS_MAXIMPLEMENTED_EFFECTS)
	{
	for (CurEffect = HostEffects->GetListPtr(ItemClass); CurEffect; CurEffect = CurEffect->Next)
		{
		if (AddItem(CurEffect, 0, NotifyObj))
			NumAdded ++;
		} // for
	} // if
else if (ItemClass == WCS_RAHOST_OBJTYPE_VECTOR)
	{
	HostDB->ResetGeoClip();
	for (CurJoe = HostDB->GetFirst(); CurJoe; CurJoe = HostDB->GetNext(CurJoe))
		{
		if (! CurJoe->TestFlags(WCS_JOEFLAG_HASKIDS))
			{
			if (! CurJoe->TestFlags(WCS_JOEFLAG_ISDEM) && ! CurJoe->TestFlags(WCS_JOEFLAG_ISCONTROL))
				{
				if (AddItem(CurJoe, 0, NotifyObj))
					NumAdded ++;
				} // if
			} // if
		} // for
	} // else if

if (NumAdded)
	{
	Changes[0] = MAKE_ID(NotifyObj->GetNotifyClass(), NotifyObj->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, NotifyObj->GetRAHostRoot());
	} // if

return (NumAdded);

} // SXQueryAction::AddItemsByClass

/*===========================================================================*/

long SXQueryAction::AddItemsByQuery(SearchQuery *TestMe, int GenNotify, RasterAnimHost *NotifyObj)
{
long NumAdded = 0;
Database *HostDB = GlobalApp->AppDB;
Joe *CurJoe;
NotifyTag Changes[2];

HostDB->ResetGeoClip();
for (CurJoe = HostDB->GetFirst(); CurJoe; CurJoe = HostDB->GetNext(CurJoe))
	{
	if (! CurJoe->TestFlags(WCS_JOEFLAG_HASKIDS))
		{
		if (! CurJoe->TestFlags(WCS_JOEFLAG_ISDEM) && ! CurJoe->TestFlags(WCS_JOEFLAG_ISCONTROL))
			{
			if (TestMe->ApproveJoe(CurJoe))
				{
				if (AddItem(CurJoe, 0, NotifyObj))
					NumAdded ++;
				} // if
			} // if
		} // if
	} // for

if (GenNotify && NumAdded)
	{
	Changes[0] = MAKE_ID(NotifyObj->GetNotifyClass(), NotifyObj->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, NotifyObj->GetRAHostRoot());
	} // if

return (NumAdded);

} // SXQueryAction::AddItemsByQuery

/*===========================================================================*/

int SXQueryAction::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource, int GenNotify, RasterAnimHost *NotifyObj)
{

if (AcceptItemType(DropSource->TypeNumber))
	{
	// handle search queries differently than other items
	if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_SEARCHQUERY)
		{
		if (AddItemsByQuery((SearchQuery *)DropSource->DropSource, GenNotify, NotifyObj))
			return (1);
		} // if
	else if (AddItem(DropSource->DropSource, GenNotify, NotifyObj))
		return (1);
	} // if

return (0);

} // SXQueryAction::ProcessRAHostDragDrop

/*===========================================================================*/

int SXQueryAction::RemoveRAHost(RasterAnimHost *RemoveMe, RasterAnimHost *NotifyObj)
{
RasterAnimHostList *CurItem = Items, *PrevItem = NULL;
NotifyTag Changes[2];

while (CurItem)
	{
	if (CurItem->Me == RemoveMe)
		{
		if (PrevItem)
			PrevItem->Next = CurItem->Next;
		else
			Items = CurItem->Next;

		delete CurItem;

		Changes[0] = MAKE_ID(NotifyObj->GetNotifyClass(), NotifyObj->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, NotifyObj->GetRAHostRoot());

		return (1);
		} // if
	PrevItem = CurItem;
	CurItem = CurItem->Next;
	} // while

return (0);

} // SXQueryAction::RemoveRAHost

/*===========================================================================*/

int SXQueryAction::InitToExport(const char *OutputFilePathSource)
{
RasterAnimHostList *CurItem = Items;
RasterAnimHostProperties Prop;

RecordNum = -1;
RecordFreq = WCS_SXQUERYITEM_FREQ_UNKNOWN;
DestroyVectorList();
CloseActionFile();
// The directory where all the files should be created is:
OutputFilePath = OutputFilePathSource;

Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
while (CurItem)
	{
	// for some reason casting CurItem->Me as an SXQueryItem and calling SetClickQueryEnabled on it causes the 
	// object to become corrupted unless it is called on the actual derived object type.
	CurItem->Me->GetRAHostProperties(&Prop);
	switch (Prop.TypeNumber)
		{
		case WCS_EFFECTSSUBCLASS_FOLIAGE:
			{
			if (! ((FoliageEffect *)CurItem->Me)->SXQuerySetupForExport(this, OutputFilePath))
				return (0);
			break;
			} // 
		case WCS_EFFECTSSUBCLASS_OBJECT3D:
			{
			if (! ((Object3DEffect *)CurItem->Me)->SXQuerySetupForExport(this, OutputFilePath))
				return (0);
			break;
			} // 
		case WCS_EFFECTSSUBCLASS_FENCE:
			{
			if (! ((Fence *)CurItem->Me)->SXQuerySetupForExport(this, OutputFilePath))
				return (0);
			break;
			} // 
		case WCS_EFFECTSSUBCLASS_LABEL:
			{
			if (! ((Label *)CurItem->Me)->SXQuerySetupForExport(this, OutputFilePath))
				return (0);
			break;
			} // 
		case WCS_RAHOST_OBJTYPE_VECTOR:
			{
			// add SXQueryItem attribute, enable instant query-enabled checking
			if (! ((Joe *)CurItem->Me)->SXQuerySetupForExport(this, OutputFilePath))
				return (0);
			break;
			} // WCS_RAHOST_OBJTYPE_VECTOR
		} // switch
	CurItem = CurItem->Next;
	} // while

return (1);

} // SXQueryAction::InitToExport

/*===========================================================================*/

void SXQueryAction::CleanupFromExport(void)
{
RasterAnimHostList *CurItem = Items;
RasterAnimHostProperties Prop;

DestroyVectorList();
OutputFilePath = NULL;
CloseActionFile();

Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
while (CurItem)
	{
	// for some reason casting CurItem->Me as an SXQueryItem and calling SetClickQueryEnabled on it causes the 
	// object to become corrupted unless it is called on the actual derived object type.
	CurItem->Me->GetRAHostProperties(&Prop);
	switch (Prop.TypeNumber)
		{
		case WCS_EFFECTSSUBCLASS_FOLIAGE:
			{
			((FoliageEffect *)CurItem->Me)->SXQueryCleanupFromExport();
			break;
			} // WCS_EFFECTSSUBCLASS_FOLIAGE
		case WCS_EFFECTSSUBCLASS_OBJECT3D:
			{
			// disable instant query-enabled checking
			((Object3DEffect *)CurItem->Me)->SXQueryCleanupFromExport();
			break;
			} // WCS_EFFECTSSUBCLASS_OBJECT3D
		case WCS_EFFECTSSUBCLASS_FENCE:
			{
			// disable instant query-enabled checking
			((Fence *)CurItem->Me)->SXQueryCleanupFromExport();
			break;
			} // WCS_EFFECTSSUBCLASS_FENCE
		case WCS_EFFECTSSUBCLASS_LABEL:
			{
			// disable instant query-enabled checking
			((Label *)CurItem->Me)->SXQueryCleanupFromExport();
			break;
			} // WCS_EFFECTSSUBCLASS_LABEL
		case WCS_RAHOST_OBJTYPE_VECTOR:
			{
			// remove SXQueryItem attribute
			((Joe *)CurItem->Me)->SXQueryCleanupFromExport();
			break;
			} // WCS_RAHOST_OBJTYPE_VECTOR
		} // switch
	CurItem = CurItem->Next;
	} // while

} // SXQueryAction::CleanupFromExport

/*===========================================================================*/

bool SXQueryAction::AcceptItemType(long DropID)
{

if (DropID == WCS_EFFECTSSUBCLASS_FOLIAGE ||
	DropID == WCS_EFFECTSSUBCLASS_OBJECT3D ||
	DropID == WCS_EFFECTSSUBCLASS_FENCE ||
	DropID == WCS_EFFECTSSUBCLASS_LABEL ||
	DropID == WCS_EFFECTSSUBCLASS_SEARCHQUERY ||
	DropID == WCS_RAHOST_OBJTYPE_VECTOR)
	return (true);

return (false);

} // SXQueryAction::AcceptItemType

/*===========================================================================*/

char SXQueryAction::GetRecordFrequency(void)
{
const char *TestText;

if (RecordFreq == WCS_SXQUERYITEM_FREQ_UNKNOWN)
	{
	RecordFreq = WCS_SXQUERYITEM_FREQ_NORECORD;
	if (TestText = GetActionText())
		{
		RecordFreq = WCS_SXQUERYITEM_FREQ_PERACTION;
		if (TestTextForInstanceInfo(TestText))
			RecordFreq = WCS_SXQUERYITEM_FREQ_PERINSTANCE;
		else if (TestTextForVectorInfo(TestText))
			{
			if (TestTextForComponentInfo(TestText))
				RecordFreq = WCS_SXQUERYITEM_FREQ_PERVECTORPERCOMPONENT;
			else
				RecordFreq = WCS_SXQUERYITEM_FREQ_PERVECTOR;
			} // else
		else if (TestTextForComponentInfo(TestText))
			RecordFreq = WCS_SXQUERYITEM_FREQ_PERCOMPONENT;
		} // if
	} // if

return (RecordFreq);

} // SXQueryAction::GetRecordFrequency

/*===========================================================================*/

bool SXQueryAction::TestTextForInstanceInfo(const char *TestText)
{
// currently there are no instance variables possible
return (false);

} // SXQueryAction::TestTextForInstanceInfo

/*===========================================================================*/

bool SXQueryAction::TestTextForVectorInfo(const char *TestText)
{
const char *CurChar = TestText;

while (CurChar = FindNextCharInstance((char *)CurChar, '&'))
	{
	// found the & character
	// test the next characters to see what info the substitution refers to
	CurChar ++;
	switch (*CurChar)
		{
		case 'V':
			{
			return (true);
			} // vector data
		case 'A':
			{
			return (true);
			} // vector attribute data
		} // switch
	// advance the pointer
	CurChar ++;
	} // while

return (false);

} // SXQueryAction::TestTextForVectorInfo

/*===========================================================================*/

bool SXQueryAction::TestTextForComponentInfo(const char *TestText)
{
const char *CurChar = TestText;

while (CurChar = FindNextCharInstance((char *)CurChar, '&'))
	{
	// found the & character
	// test the next characters to see what info the substitution refers to
	CurChar ++;
	switch (*CurChar)
		{
		case 'O':
			{
			return (true);
			} // object data
		} // switch
	// advance the pointer
	CurChar ++;
	} // while

return (false);

} // SXQueryAction::TestTextForComponentInfo

/*===========================================================================*/

long SXQueryAction::GetRecordNumber(GeneralEffect *MyEffect, Joe *MyVector, long &ActionFileID, NameList **FileNamesCreated)
{

switch (RecordFreq)
	{
	case WCS_SXQUERYITEM_FREQ_PERACTION:
		{
		// for each action see if a new action record needs to be generated
		// then write an object record incorporating all the actions
		if (RecordNum < 0)
			{
			RecordNum = CreateActionRecord(MyEffect, MyVector, ActionFileID, FileNamesCreated);
			} // if
		return (RecordNum);
		} // 
	case WCS_SXQUERYITEM_FREQ_PERCOMPONENT:
		{
		SXQueryEffectList *FoundEffect;

		// check and see if this vector has been done already
		if (FoundEffect = SearchEffectList(MyEffect))
			return (FoundEffect->RecordNum);
		// not found so add vector
		if (FoundEffect = AddEffectList(MyEffect))
			{
			FoundEffect->RecordNum = CreateActionRecord(MyEffect, MyVector, ActionFileID, FileNamesCreated);
			return (FoundEffect->RecordNum);
			} // if
		// failure
		return (-1);
		} // 
	case WCS_SXQUERYITEM_FREQ_PERVECTOR:
		{
		SXQueryVectorList *FoundVector;

		// check and see if this vector has been done already
		if (FoundVector = SearchVectorList(MyVector))
			return (FoundVector->RecordNum);
		// not found so add vector
		if (FoundVector = AddVectorList(MyVector))
			{
			FoundVector->RecordNum = CreateActionRecord(MyEffect, MyVector, ActionFileID, FileNamesCreated);
			return (FoundVector->RecordNum);
			} // if
		// failure
		return (-1);
		} // 
	case WCS_SXQUERYITEM_FREQ_PERVECTORPERCOMPONENT:
		{
		SXQueryVectorEffectList *FoundVectorEffect;

		// check and see if this vector has been done already
		if (FoundVectorEffect = SearchVectorEffectList(MyEffect, MyVector))
			return (FoundVectorEffect->RecordNum);
		// not found so add vector
		if (FoundVectorEffect = AddVectorEffectList(MyEffect, MyVector))
			{
			FoundVectorEffect->RecordNum = CreateActionRecord(MyEffect, MyVector, ActionFileID, FileNamesCreated);
			return (FoundVectorEffect->RecordNum);
			} // if
		// failure
		return (-1);
		} // 
	case WCS_SXQUERYITEM_FREQ_PERINSTANCE:
		{
		// no questions asked, create a new record, 
		return (CreateActionRecord(MyEffect, MyVector, ActionFileID, FileNamesCreated));
		} // 
	default:	// includes WCS_SXQUERYITEM_FREQ_NORECORD
		{
		// no record is to be written for this object
		return (-1);
		} // 
	} // switch

//return (RecordNum);	// Lint points out that we can't ever reach this

} // SXQueryAction::GetRecordNumber

/*===========================================================================*/

int SXQueryAction::CreateActionRecord(GeneralEffect *MyEffect, Joe *MyVector, long &ActionFileID, NameList **FileNamesCreated)
{
int Success = 1, RecordCreated = ActionFileID;
char ActionCommand[16384];

/* for testing
if (MyEffect && MyVector)
	sprintf(ActionCommand, "%s, %s...%s", ActionText, MyEffect->GetName(), MyVector->GetBestName());
else if (MyEffect)
	sprintf(ActionCommand, "%s, %s", ActionText, MyEffect->GetName());
else if (MyVector)
	sprintf(ActionCommand, "%s, ...%s", ActionText, MyVector->GetBestName());
else
	sprintf(ActionCommand, "%s, No Effect or Vector", ActionText);
*/

ReplaceStringTokens(ActionCommand, ActionText, MyEffect, MyVector);

if (Success = WriteActionRecord(ActionCommand, FileNamesCreated))
	{
	ActionFileID ++;
	return (RecordCreated);
	} // if
return (-1);

} // SXQueryAction::CreateActionRecord

/*===========================================================================*/

SXQueryVectorList *SXQueryAction::AddVectorList(Joe *AddVector)
{
SXQueryVectorList **CurListPtr = &VectorList;

while (*CurListPtr)
	{
	CurListPtr = &(*CurListPtr)->Next;
	} // if

return (*CurListPtr = new SXQueryVectorList(AddVector));

} // SXQueryAction::AddVectorList

/*===========================================================================*/

SXQueryVectorList *SXQueryAction::SearchVectorList(Joe *SearchVector)
{
SXQueryVectorList *CurList = VectorList;

while (CurList)
	{
	if (CurList->QueryVector == SearchVector)
		return CurList;
	CurList = CurList->Next;
	} // if

return (NULL);

} // SXQueryAction::SearchVectorList

/*===========================================================================*/

SXQueryEffectList *SXQueryAction::AddEffectList(GeneralEffect *AddEffect)
{
SXQueryEffectList **CurListPtr = &EffectList;

while (*CurListPtr)
	{
	CurListPtr = &(*CurListPtr)->Next;
	} // if

return (*CurListPtr = new SXQueryEffectList(AddEffect));

} // SXQueryAction::AddEffectList

/*===========================================================================*/

SXQueryEffectList *SXQueryAction::SearchEffectList(GeneralEffect *SearchEffect)
{
SXQueryEffectList *CurList = EffectList;

while (CurList)
	{
	if (CurList->QueryEffect == SearchEffect)
		return CurList;
	CurList = CurList->Next;
	} // if

return (NULL);

} // SXQueryAction::SearchEffectList

/*===========================================================================*/

SXQueryVectorEffectList *SXQueryAction::AddVectorEffectList(GeneralEffect *AddEffect, Joe *AddVector)
{
SXQueryVectorEffectList **CurListPtr = &VectorEffectList;

while (*CurListPtr)
	{
	CurListPtr = &(*CurListPtr)->Next;
	} // if

return (*CurListPtr = new SXQueryVectorEffectList(AddEffect, AddVector));

} // SXQueryAction::AddVectorEffectList

/*===========================================================================*/

SXQueryVectorEffectList *SXQueryAction::SearchVectorEffectList(GeneralEffect *SearchEffect, Joe *SearchVector)
{
SXQueryVectorEffectList *CurList = VectorEffectList;

while (CurList)
	{
	if (CurList->QueryVector == SearchVector && CurList->QueryEffect == SearchEffect)
		return CurList;
	CurList = CurList->Next;
	} // if

return (NULL);

} // SXQueryAction::SearchVectorEffectList

/*===========================================================================*/

void SXQueryAction::CloseActionFile(void)
{

if (ActionFile)
	fclose(ActionFile);
ActionFile = NULL;
if (ActionFileRecords)
	fclose(ActionFileRecords);
ActionFileRecords = NULL;

} // SXQueryAction::CloseActionFile

/*===========================================================================*/

int SXQueryAction::OpenActionFile(NameList **FileNamesCreated)
{
char FullOutPath[512], FileID[8], IdxFileID[8];
char Super = 1, Major = 0, Minor = 0, Revision = 0;
unsigned long ByteOrder = 0xaabbccdd, HeaderSize;

CloseActionFile();
strcpy(FileID, "NVWANQA");
strcpy(IdxFileID, "NVWANQX");

if (OutputFilePath)
	{
	#ifdef WCS_SXACTIONQUERY_ASCIIFILE
	strmfp(FullOutPath, OutputFilePath, "SXActionIndex.txt");
	ActionFile = PROJ_fopen(FullOutPath, "w");
	strmfp(FullOutPath, OutputFilePath, "SXActionRecords.txt");
	ActionFileRecords = PROJ_fopen(FullOutPath, "w");
	#else // ! WCS_SXACTIONQUERY_ASCIIFILE
	strmfp(FullOutPath, OutputFilePath, "SXActionIndex.nqx");
	ActionFile = PROJ_fopen(FullOutPath, "wb");
	strmfp(FullOutPath, OutputFilePath, "SXActionRecords.nqa");
	ActionFileRecords = PROJ_fopen(FullOutPath, "wb");
	// add items to file name list
	AddNewNameList(FileNamesCreated, "SXActionIndex.nqx", WCS_EXPORTCONTROL_FILETYPE_QUERYACTIONIDX);
	AddNewNameList(FileNamesCreated, "SXActionRecords.nqa", WCS_EXPORTCONTROL_FILETYPE_QUERYACTION);
	// write header
	if (ActionFile && ActionFileRecords)
		{
		fwrite((char *)IdxFileID, strlen(IdxFileID) + 1, 1, ActionFile);
		fwrite((char *)FileID, strlen(FileID) + 1, 1, ActionFileRecords);
		fwrite((char *)&Super, sizeof (char), 1, ActionFile);
		fwrite((char *)&Super, sizeof (char), 1, ActionFileRecords);
		fwrite((char *)&Major, sizeof (char), 1, ActionFile);
		fwrite((char *)&Major, sizeof (char), 1, ActionFileRecords);
		fwrite((char *)&Minor, sizeof (char), 1, ActionFile);
		fwrite((char *)&Minor, sizeof (char), 1, ActionFileRecords);
		fwrite((char *)&Revision, sizeof (char), 1, ActionFile);
		fwrite((char *)&Revision, sizeof (char), 1, ActionFileRecords);
		fwrite((char *)&ByteOrder, sizeof (unsigned long), 1, ActionFile);
		fwrite((char *)&ByteOrder, sizeof (unsigned long), 1, ActionFileRecords);

		HeaderSize = ftell(ActionFile) + sizeof (unsigned long);
		//#ifdef WCS_BYTEORDER_BIGENDIAN
		//SimpleEndianFlip32U(HeaderSize, &HeaderSize);
		//#endif // WCS_BYTEORDER_BIGENDIAN
		//if ((fwrite((char *)&HeaderSize, sizeof (unsigned long), 1, ActionFile)) != 1)
		if(PutL32U(HeaderSize, ActionFile))
			return (0);

		HeaderSize = ftell(ActionFileRecords) + sizeof (unsigned long);
		//#ifdef WCS_BYTEORDER_BIGENDIAN
		//SimpleEndianFlip32U(HeaderSize, &HeaderSize);
		//#endif // WCS_BYTEORDER_BIGENDIAN
		//if ((fwrite((char *)&HeaderSize, sizeof (unsigned long), 1, ActionFileRecords)) != 1)
		if(PutL32U(HeaderSize, ActionFileRecords))
			return (0);

		#endif // ! WCS_SXACTIONQUERY_ASCIIFILE
		return (1);
		} // if
	} // if

return (0);

} // SXQueryAction::OpenActionFile

/*===========================================================================*/

NameList *SXQueryAction::AddNewNameList(NameList **Names, char *NewName, long FileType)
{
NameList **ListPtr;

if (Names)
	{
	ListPtr = Names;
	while (*ListPtr)
		{
		if (! stricmp((*ListPtr)->Name, NewName) && FileType == (*ListPtr)->ItemClass)
			return (*ListPtr);
		ListPtr = &(*ListPtr)->Next;
		} // if
	return (*ListPtr = new NameList(NewName, FileType));
	} // if

return (NULL);

} // SXQueryAction::AddNewNameList

/*===========================================================================*/

int SXQueryAction::WriteActionRecord(char *ActionString, NameList **FileNamesCreated)
{
unsigned long FilePos;
unsigned short StrLen; //, StrLenFlip;

if (ActionString)
	{
	if ((ActionFile && ActionFileRecords) || OpenActionFile(FileNamesCreated))
		{
		FilePos = ftell(ActionFileRecords);
		StrLen = (unsigned short)(strlen(ActionString) + 1);
		#ifdef WCS_SXACTIONQUERY_ASCIIFILE
		fprintf(ActionFileRecords, "%d, %d...%s\n", ActionType, StrLen, ActionString);
		fprintf(ActionFile, "%d\n", FilePos);
		#else // WCS_SXACTIONQUERY_ASCIIFILE
		if (fwrite((char *)&ActionType, sizeof (unsigned char), 1, ActionFileRecords) != 1)
			return (0);
		//#ifdef WCS_BYTEORDER_BIGENDIAN
		//SimpleEndianFlip16U(StrLen, &StrLenFlip);
		//#endif // WCS_BYTEORDER_BIGENDIAN
		//if (fwrite((char *)&StrLenFlip, sizeof (unsigned short), 1, ActionFileRecords) != 1)
		if(PutL16U(StrLen, ActionFileRecords))
			return (0);
		if (fwrite(ActionString, StrLen, 1, ActionFileRecords) != 1)
			return (0);
		//#ifdef WCS_BYTEORDER_BIGENDIAN
		//SimpleEndianFlip32U(FilePos, &FilePos);
		//#endif // WCS_BYTEORDER_BIGENDIAN
		//if (fwrite((char *)&FilePos, sizeof (unsigned long), 1, ActionFile) != 1)
		if(PutL32U(FilePos, ActionFile))
			return (0);
		#endif // WCS_SXACTIONQUERY_ASCIIFILE
		return (1);
		} // if
	return (0);
	} // if

return (1);

} // SXQueryAction::WriteActionRecord

/*===========================================================================*/

void SXQueryAction::ReplaceStringTokens(char *MesgCopy, const char *OrigMesg, GeneralEffect *CurEffect, Joe *CurVec)
{
double JoeVal;//, DVal, TargDist;
long LetterCt, OrigLen, AttribTokenLen, AttribBegun, AttribStart, AttribEnd, LastSegStart,// UnitChar, UnitID, 
	PrecisionFound, NextLetter;//, FrameNum, FrameRate;
char AttribName[256], TempStr[WCS_LABELTEXT_MAXLEN], DefaultDigits, TestLetter; //, Units[8];
LayerStub *Stub;
char *JoeStr;
VertexDEM Begin, End;

OrigLen = (long)strlen(OrigMesg);
MesgCopy[0] = 0;

AttribTokenLen = (long)strlen(Attrib_TextSymbol);
AttribBegun = 0;
LastSegStart = 0;

for (LetterCt = 0; LetterCt <= OrigLen; LetterCt ++)
	{
	DefaultDigits = '2';	// default # decimal digits to print
	PrecisionFound = 0;
	if (! OrigMesg[LetterCt])
		{
		strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
		TempStr[LetterCt - LastSegStart] = 0;
		strcat(MesgCopy, TempStr);
		break;
		} // if
	if (OrigMesg[LetterCt] == 10 || OrigMesg[LetterCt] == 13)
		{
		strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
		TempStr[LetterCt - LastSegStart] = 0;
		strcat(MesgCopy, TempStr);
		strcat(MesgCopy, "\n");
		if (OrigMesg[LetterCt + 1] == 10 || OrigMesg[LetterCt + 1] == 13)
			LetterCt ++;
		LastSegStart = LetterCt + 1;
		continue;
		} // if
	if (! strncmp(&OrigMesg[LetterCt], Attrib_TextSymbol, AttribTokenLen))
		{
		if (! AttribBegun)
			{
			AttribBegun = 1;
			AttribStart = LetterCt + AttribTokenLen;
			strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
			TempStr[LetterCt - LastSegStart] = 0;
			strcat(MesgCopy, TempStr);
			} // if
		else
			{
			AttribEnd = LetterCt;
			strncpy(AttribName, &OrigMesg[AttribStart], AttribEnd - AttribStart);
			AttribName[AttribEnd - AttribStart] = 0;
			AttribBegun = 0;
			LastSegStart = AttribEnd + AttribTokenLen;
			// find the attribute on this Joe, it might be text or numeric
			if (CurVec)
				{
				if (Stub = CurVec->CheckTextAttributeExistance(AttribName))
					{
					if (JoeStr = (char *)CurVec->GetTextAttributeValue(Stub))
						{
						strcat(MesgCopy, JoeStr);
						} // if
					} // if
				else if (Stub = CurVec->CheckIEEEAttributeExistance(AttribName))
					{
					if (JoeVal = CurVec->GetIEEEAttributeValue(Stub))
						{
						sprintf(TempStr, "%f", JoeVal);
						TrimZeros(TempStr);
						strcat(MesgCopy, TempStr);
						} // if
					} // if
				} // if
			} // else
		} // if text match
	else if (! strncmp(&OrigMesg[LetterCt], "&*", 2))
		{
		strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
		TempStr[LetterCt - LastSegStart] = 0;
		strcat(MesgCopy, TempStr);
		strcat(MesgCopy, "\n");
		LastSegStart = LetterCt + 2;
		} // else if new line
	/*
	else if (! strncmp(&OrigMesg[LetterCt], "&C", 2))
		{
		NextLetter = LetterCt + 2;
		TestLetter = OrigMesg[NextLetter];
		if ((TestLetter == 'B') || (TestLetter == 'C')
			|| (TestLetter == 'F') || (TestLetter == 'H') 
			|| (TestLetter == 'N') || (TestLetter == 'P') 
			|| (TestLetter == 'X') || (TestLetter == 'Y') 
			|| (TestLetter == 'Z') || (TestLetter == 'T'))
			{
			strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
			TempStr[LetterCt - LastSegStart] = 0;
			strcat(MesgCopy, TempStr);
			if (OrigMesg[NextLetter + 1] == '%')
				{
				NextLetter ++;
				if ((OrigMesg[NextLetter + 1] >= '0') && (OrigMesg[NextLetter + 1] <= '9'))
					{
					NextLetter ++;
					DefaultDigits = OrigMesg[NextLetter];
					PrecisionFound = 1;
					} // if
				} // if
			switch (TestLetter)
				{
				case 'B':	// bank
					{
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].GetCurValue();
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &CB
				case 'F':	// fov
					{
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].GetCurValue();
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &CF
				case 'H':	// heading
					{
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].GetCurValue();
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &CH
				case 'N':	// name
					{
					strcat(MesgCopy, Cam->GetName());
					LetterCt = NextLetter;
					break;
					} // &CN
				case 'P':	// pitch
					{
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].GetCurValue();
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &CP
				case 'X':	// longitude
					{
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetCurValue();
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &CX
				case 'Y':	// latitude
					{
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetCurValue();
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &CY
				case 'Z':	// elevation
					{
					UnitID = WCS_USEFUL_UNIT_METER;
					// look for unit suffix
					if (PrecisionFound)
						{
						UnitChar = 0;
						while ((OrigMesg[NextLetter + 1] != 0) && (OrigMesg[NextLetter + 1] != 13) && (OrigMesg[NextLetter + 1] !=' ') && (OrigMesg[NextLetter + 1] != '&') && (UnitChar < 7))
							{
							NextLetter ++;
							Units[UnitChar ++] = OrigMesg[NextLetter];
							} // while
						Units[UnitChar] = 0;
						UnitID = MatchUnitSuffix(Units);
						if (UnitID < 0)
							UnitID = WCS_USEFUL_UNIT_METER;
						} // if
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetCurValue();
					if (UnitID != WCS_USEFUL_UNIT_METER)
						DVal = ConvertFromMeters(DVal, UnitID);
					sprintf(TempStr, "%.*f%s", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'), GetUnitSuffix(UnitID));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &CZ
				case 'C':	// Compass bearing
					{
					DVal = Cam->CamHeading;
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &CC
				case 'T':	// tilt (including targetting and pitch)
					{
					DVal = Cam->CamPitch;
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &CT
				default:
					break;
				} // switch
			LastSegStart = LetterCt + 1;
			} // if
		} // else if &C
	else if (! strncmp(&OrigMesg[LetterCt], "&F", 2))
		{
		NextLetter = LetterCt + 2;
		TestLetter = OrigMesg[NextLetter];
		if (TestLetter == 'N' || TestLetter == 'S')
			{
			strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
			TempStr[LetterCt - LastSegStart] = 0;
			strcat(MesgCopy, TempStr);
			FrameRate = (unsigned long)(Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue + 0.5);
			FrameNum = (long)(FrameRate * RenderTime);
			switch (TestLetter)
				{
				case 'N':	// frame number
					{
					NextLetter ++;
					if ((OrigMesg[NextLetter] < '1') || (OrigMesg[NextLetter] > '9'))
						break;
					DefaultDigits = OrigMesg[NextLetter];
					sprintf(TempStr, "%0*u", DefaultDigits - '0', FrameNum);
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &FN
				case 'S':	// SMPTE type timecode
					{
					unsigned long h, m, s, f, t;
					if (FrameRate != 0)
						{
						f = (unsigned long)FrameNum % FrameRate;
						t = (unsigned long)FrameNum / FrameRate;	// this frames time in seconds at this frame rate
						h = t / 3600;
						m = (t % 3600) / 60;
						s = t % 60;
						sprintf(TempStr, "%02u:%02u:%02u:%02u", h, m, s, f);
						}
					else
						sprintf(TempStr, "00:00:00:00");
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &FS
				default:
					break;
				} // switch
			LastSegStart = LetterCt + 1;
			} // if
		} // else if &F
	else if (! strncmp(&OrigMesg[LetterCt], "&P", 2))
		{
		NextLetter = LetterCt + 2;
		TestLetter = OrigMesg[NextLetter];
		if (TestLetter == 'N')
			{
			strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
			TempStr[LetterCt - LastSegStart] = 0;
			strcat(MesgCopy, TempStr);
			strcat(MesgCopy, ProjectBase->projectname);
			LetterCt = NextLetter;
			LastSegStart = LetterCt + 1;
			} // if &PN
		} // else if &P
	else if (! strncmp(&OrigMesg[LetterCt], "&R", 2))
		{
		NextLetter = LetterCt + 2;
		TestLetter = OrigMesg[NextLetter];
		if (TestLetter == 'N' || TestLetter == 'D')
			{
			strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
			TempStr[LetterCt - LastSegStart] = 0;
			strcat(MesgCopy, TempStr);
			switch (TestLetter)
				{
				case 'N':	// render opts name
					{
					strcat(MesgCopy, Opt->GetName());
					LetterCt = NextLetter;
					break;
					} // &RN
				case 'D':	// render date & time
					{
					time_t ltime;
					struct tm *now;

					time(&ltime);
					now = localtime(&ltime);
					strftime(TempStr, sizeof(TempStr), "%c", now);
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &RD
				default:
					break;
				} // switch
			LastSegStart = LetterCt + 1;
			} // if
		} // else if &R
	else if (! strncmp(&OrigMesg[LetterCt], "&T", 2))
		{
		NextLetter = LetterCt + 2;
		TestLetter = OrigMesg[NextLetter];
		if ((TestLetter == 'X') || (TestLetter == 'Y')
			|| (TestLetter == 'Z') || (TestLetter == 'D'))
			{
			strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
			TempStr[LetterCt - LastSegStart] = 0;
			strcat(MesgCopy, TempStr);
			if (OrigMesg[NextLetter + 1] == '%')
				{
				NextLetter ++;
				if ((OrigMesg[NextLetter + 1] >= '0') && (OrigMesg[NextLetter + 1] <= '9'))
					{
					NextLetter ++;
					DefaultDigits = OrigMesg[NextLetter];
					PrecisionFound = 1;
					} // if
				} // if
			switch (TestLetter)
				{
				case 'X':	// longitude
					{
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].GetCurValue();
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &TX
				case 'Y':	// latitude
					{
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetCurValue();
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &TY
				case 'Z':	// elevation
					{
					UnitID = WCS_USEFUL_UNIT_METER;
					// look for unit suffix
					if (PrecisionFound)
						{
						UnitChar = 0;
						while ((OrigMesg[NextLetter + 1] != 0) && (OrigMesg[NextLetter + 1] != 13) && (OrigMesg[NextLetter + 1] !=' ') && (OrigMesg[NextLetter + 1] != '&') && (UnitChar < 7))
							{
							NextLetter ++;
							Units[UnitChar ++] = OrigMesg[NextLetter];
							} // while
						Units[UnitChar] = 0;
						UnitID = MatchUnitSuffix(Units);
						if (UnitID < 0)
							UnitID = WCS_USEFUL_UNIT_METER;
						} // if
					DVal = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].GetCurValue();
					if (UnitID != WCS_USEFUL_UNIT_METER)
						DVal = ConvertFromMeters(DVal, UnitID);
					sprintf(TempStr, "%.*f%s", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'), GetUnitSuffix(UnitID));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &TZ
				case 'D':	// distance
					{
					UnitID = WCS_USEFUL_UNIT_METER;
					// look for unit suffix
					if (PrecisionFound)
						{
						UnitChar = 0;
						while ((OrigMesg[NextLetter + 1] != 0) && (OrigMesg[NextLetter + 1] != 13) && (OrigMesg[NextLetter + 1] !=' ') && (OrigMesg[NextLetter + 1] != '&') && (UnitChar < 7))
							{
							NextLetter ++;
							Units[UnitChar ++] = OrigMesg[NextLetter];
							} // while
						Units[UnitChar] = 0;
						UnitID = MatchUnitSuffix(Units);
						if (UnitID < 0)
							UnitID = WCS_USEFUL_UNIT_METER;
						} // if
					TargDist = 0;
					if(Cam->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED)
						{
						Begin.Lat = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetCurValue();
						Begin.Lon = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetCurValue();
						Begin.Elev = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetCurValue();
						End.Lat = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetCurValue();
						End.Lon = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].GetCurValue();
						End.Elev = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].GetCurValue();
						#ifdef WCS_BUILD_VNS
						DefCoords->DegToCart(&Begin);
						DefCoords->DegToCart(&End);
						#else // WCS_BUILD_VNS
						Begin.DegToCart(PlanetRad);
						End.DegToCart(PlanetRad);
						#endif // WCS_BUILD_VNS
						TargDist = PointDistance(Begin.XYZ, End.XYZ);
						} // if
					DVal = TargDist;
					if (UnitID != WCS_USEFUL_UNIT_METER)
						DVal = ConvertFromMeters(DVal, UnitID);
					sprintf(TempStr, "%.*f%s", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'), GetUnitSuffix(UnitID));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &TD
				default:
					break;
				} // switch
			LastSegStart = LetterCt + 1;
			} // if
		} // else if &T
	else if (! strncmp(&OrigMesg[LetterCt], "&U", 2))
		{
		NextLetter = LetterCt + 2;
		TestLetter = OrigMesg[NextLetter];
		if (TestLetter == 'N' || TestLetter == 'E')
			{
			strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
			TempStr[LetterCt - LastSegStart] = 0;
			strcat(MesgCopy, TempStr);
			switch (TestLetter)
				{
				case 'N':	// Name
					{
					strcat(MesgCopy, ProjectBase->UserName);
					LetterCt = NextLetter;
					break;
					} // &UN
				case 'E':	// EMail
					{
					strcat(MesgCopy, ProjectBase->UserEmail);
					LetterCt = NextLetter;
					break;
					} // &UE
				default:
					break;
				} // switch
			LastSegStart = LetterCt + 1;
			} // if
		} // else if &U
	*/
	else if (! strncmp(&OrigMesg[LetterCt], "&V", 2))
		{
		NextLetter = LetterCt + 2;
		TestLetter = OrigMesg[NextLetter];
		if ((TestLetter == 'N') || (TestLetter == 'L'))
		//if ((TestLetter == 'X') || (TestLetter == 'Y')
		//	|| (TestLetter == 'Z') || (TestLetter == 'D') 
		//	|| (TestLetter == 'N') || (TestLetter == 'L'))
			{
			strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
			TempStr[LetterCt - LastSegStart] = 0;
			strcat(MesgCopy, TempStr);
			if (OrigMesg[NextLetter + 1] == '%')
				{
				NextLetter ++;
				if ((OrigMesg[NextLetter + 1] >= '0') && (OrigMesg[NextLetter + 1] <= '9'))
					{
					NextLetter ++;
					DefaultDigits = OrigMesg[NextLetter];
					PrecisionFound = 1;
					} // if
				} // if
			switch (TestLetter)
				{
				/*
				case 'X':	// longitude
					{
					DVal = CurVert->Lon;
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &VX
				case 'Y':	// latitude
					{
					DVal = CurVert->Lat;
					sprintf(TempStr, "%.*f", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &VY
				case 'Z':	// elevation
					{
					UnitID = WCS_USEFUL_UNIT_METER;
					// look for unit suffix
					if (PrecisionFound)
						{
						UnitChar = 0;
						while ((OrigMesg[NextLetter + 1] != 0) && (OrigMesg[NextLetter + 1] != 13) && (OrigMesg[NextLetter + 1] !=' ') && (OrigMesg[NextLetter + 1] != '&') && (UnitChar < 7))
							{
							NextLetter ++;
							Units[UnitChar ++] = OrigMesg[NextLetter];
							} // while
						Units[UnitChar] = 0;
						UnitID = MatchUnitSuffix(Units);
						if (UnitID < 0)
							UnitID = WCS_USEFUL_UNIT_METER;
						} // if
					DVal = CurVert->Elev;
					if (UnitID != WCS_USEFUL_UNIT_METER)
						DVal = ConvertFromMeters(DVal, UnitID);
					sprintf(TempStr, "%.*f%s", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'), GetUnitSuffix(UnitID));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &VZ
				case 'D':	// distance
					{
					UnitID = WCS_USEFUL_UNIT_METER;
					// look for unit suffix
					if (PrecisionFound)
						{
						UnitChar = 0;
						while ((OrigMesg[NextLetter + 1] != 0) && (OrigMesg[NextLetter + 1] != 13) && (OrigMesg[NextLetter + 1] !=' ') && (OrigMesg[NextLetter + 1] != '&') && (UnitChar < 7))
							{
							NextLetter ++;
							Units[UnitChar ++] = OrigMesg[NextLetter];
							} // while
						Units[UnitChar] = 0;
						UnitID = MatchUnitSuffix(Units);
						if (UnitID < 0)
							UnitID = WCS_USEFUL_UNIT_METER;
						} // if
					TargDist = 0;
					Begin.Lat = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetCurValue();
					Begin.Lon = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetCurValue();
					Begin.Elev = Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetCurValue();
					End.Lat = CurVert->Lat;
					End.Lon = CurVert->Lon;
					End.Elev = CurVert->Elev;
					#ifdef WCS_BUILD_VNS
					DefCoords->DegToCart(&Begin);
					DefCoords->DegToCart(&End);
					#else // WCS_BUILD_VNS
					Begin.DegToCart(PlanetRad);
					End.DegToCart(PlanetRad);
					#endif // WCS_BUILD_VNS
					TargDist = PointDistance(Begin.XYZ, End.XYZ);
					DVal = TargDist;
					if (UnitID != WCS_USEFUL_UNIT_METER)
						DVal = ConvertFromMeters(DVal, UnitID);
					sprintf(TempStr, "%.*f%s", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'), GetUnitSuffix(UnitID));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &VD
				*/
				case 'N':	// Name
					{
					if (CurVec)
						strcat(MesgCopy, CurVec->FileName() ? CurVec->FileName(): CurVec->GetBestName());
					LetterCt = NextLetter;
					break;
					} // &VN
				case 'L':	// Label
					{
					if (CurVec)
						strcat(MesgCopy, CurVec->Name() ? CurVec->Name(): CurVec->GetBestName());
					LetterCt = NextLetter;
					break;
					} // &VL
				default:
					break;
				} // switch
			LastSegStart = LetterCt + 1;
			} // if
		} // else if &V
	/*
	else if (! strncmp(&OrigMesg[LetterCt], "&G", 2))
		{
		NextLetter = LetterCt + 2;
		TestLetter = OrigMesg[NextLetter];
		if ((TestLetter == 'Z'))
			{
			strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
			TempStr[LetterCt - LastSegStart] = 0;
			strcat(MesgCopy, TempStr);
			if (OrigMesg[NextLetter + 1] == '%')
				{
				NextLetter ++;
				if ((OrigMesg[NextLetter + 1] >= '0') && (OrigMesg[NextLetter + 1] <= '9'))
					{
					NextLetter ++;
					DefaultDigits = OrigMesg[NextLetter];
					PrecisionFound = 1;
					} // if
				} // if
			switch (TestLetter)
				{
				case 'Z':	// ground elevation
					{
					unsigned long Flags;
					VertexData VertData;
					PolygonData Poly;

					UnitID = WCS_USEFUL_UNIT_METER;
					// look for unit suffix
					if (PrecisionFound)
						{
						UnitChar = 0;
						while ((OrigMesg[NextLetter + 1] != 0) && (OrigMesg[NextLetter + 1] != 13) && (OrigMesg[NextLetter + 1] !=' ') && (OrigMesg[NextLetter + 1] != '&') && (UnitChar < 7))
							{
							NextLetter ++;
							Units[UnitChar ++] = OrigMesg[NextLetter];
							} // while
						Units[UnitChar] = 0;
						UnitID = MatchUnitSuffix(Units);
						if (UnitID < 0)
							UnitID = WCS_USEFUL_UNIT_METER;
						} // if
					VertData.Lat = CurVert->Lat;
					VertData.Lon = CurVert->Lat;
					VertData.Elev = CurVert->Elev;
					Flags = (WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED |
						WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED | 
						WCS_VERTEXDATA_FLAG_WATERELEV | WCS_VERTEXDATA_FLAG_WATERDEPTH | WCS_VERTEXDATA_FLAG_LAKEAPPLIED |
						WCS_VERTEXDATA_FLAG_STREAMAPPLIED | WCS_VERTEXDATA_FLAG_WAVEHEIGHT);
					ProjectBase->Interactive->VertexDataPoint(&RendData, &VertData, &Poly, Flags);
					DVal = VertData.Elev;
					if (UnitID != WCS_USEFUL_UNIT_METER)
						DVal = ConvertFromMeters(DVal, UnitID);
					sprintf(TempStr, "%.*f%s", DefaultDigits - '0', Round(DVal, DefaultDigits - '0'), GetUnitSuffix(UnitID));
					strcat(MesgCopy, TempStr);
					LetterCt = NextLetter;
					break;
					} // &GZ
				default:
					break;
				} // switch
			LastSegStart = LetterCt + 1;
			} // if
		} // else if &G
	*/
	else if (! strncmp(&OrigMesg[LetterCt], "&O", 2))
		{
		NextLetter = LetterCt + 2;
		TestLetter = OrigMesg[NextLetter];
		if ((TestLetter == 'N'))
			{
			strncpy(TempStr, &OrigMesg[LastSegStart], LetterCt - LastSegStart);
			TempStr[LetterCt - LastSegStart] = 0;
			strcat(MesgCopy, TempStr);
			if (OrigMesg[NextLetter + 1] == '%')
				{
				NextLetter ++;
				if ((OrigMesg[NextLetter + 1] >= '0') && (OrigMesg[NextLetter + 1] <= '9'))
					{
					NextLetter ++;
					DefaultDigits = OrigMesg[NextLetter];
					PrecisionFound = 1;
					} // if
				} // if
			switch (TestLetter)
				{
				case 'N':	// Name
					{
					if (CurEffect)
						strcat(MesgCopy, CurEffect->GetName());
					LetterCt = NextLetter;
					break;
					} // &ON
				default:
					break;
				} // switch
			LastSegStart = LetterCt + 1;
			} // if
		} // else if &O
	} // for

} // SXQueryAction::ReplaceStringTokens

/*===========================================================================*/
