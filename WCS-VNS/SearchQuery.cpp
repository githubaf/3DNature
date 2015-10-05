// SearchQuery.cpp
// For managing WCS Search Queriess
// Built from scratch on 12/22/00 by Gary R. Huber
// Copyright 2000 Questar Productions

#include "stdafx.h"
#include "Application.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Database.h"
#include "Useful.h"
#include "Requester.h"
#include "Joe.h"
#include "EffectsIO.h"
#include "DBFilterEvent.h"
#include "Conservatory.h"
#include "SearchQueryEditGUI.h"
#include "Notify.h"
#include "Project.h"
#include "Security.h"
#include "DBEditGUI.h"

SearchQuery::SearchQuery()
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_SEARCHQUERY;
SetDefaults();

} // SearchQuery::SearchQuery

/*===========================================================================*/

SearchQuery::SearchQuery(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_SEARCHQUERY;
SetDefaults();

} // SearchQuery::SearchQuery

/*===========================================================================*/

SearchQuery::SearchQuery(RasterAnimHost *RAHost, EffectsLib *Library, SearchQuery *Proto)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_SEARCHQUERY;
Prev = Library->LastSearch;
if (Library->LastSearch)
	{
	Library->LastSearch->Next = this;
	Library->LastSearch = this;
	} // if
else
	{
	Library->Search = Library->LastSearch = this;
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
	strcpy(NameBase, "Search Query");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // SearchQuery::SearchQuery

/*===========================================================================*/

SearchQuery::~SearchQuery()
{
DBFilterEvent *CurFilter;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->SQU && GlobalApp->GUIWins->SQU->GetActive() == this)
		{
		delete GlobalApp->GUIWins->SQU;
		GlobalApp->GUIWins->SQU = NULL;
		} // if
	} // if

while (Filters)
	{
	CurFilter = Filters;
	Filters = Filters->Next;
	delete CurFilter;
	} // while

} // SearchQuery::~SearchQuery

/*===========================================================================*/

void SearchQuery::SetDefaults(void)
{

Filters = new DBFilterEvent();

} // SearchQuery::SetDefaults

/*===========================================================================*/

void SearchQuery::Copy(SearchQuery *CopyTo, SearchQuery *CopyFrom)
{
DBFilterEvent *CurrentFrom = CopyFrom->Filters, **ToPtr, *CurFilter;

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

GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // SearchQuery::Copy

/*===========================================================================*/

ULONG SearchQuery::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
DBFilterEvent *CurFilter, **LoadTo = &Filters;
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

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
					case WCS_EFFECTS_SEARCHQUERY_DBFILTER:
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

} // SearchQuery::Load

/*===========================================================================*/

unsigned long int SearchQuery::Save(FILE *ffile)
{
DBFilterEvent *Current;
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

Current = Filters;
while (Current)
	{
	ItemTag = WCS_EFFECTS_SEARCHQUERY_DBFILTER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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


ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // SearchQuery::Save

/*===========================================================================*/

void SearchQuery::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->SQU)
	{
	delete GlobalApp->GUIWins->SQU;
	}
GlobalApp->GUIWins->SQU = new SearchQueryEditGUI(GlobalApp->AppEffects, GlobalApp->MainProj, GlobalApp->AppDB, this);
if(GlobalApp->GUIWins->SQU)
	{
	GlobalApp->GUIWins->SQU->Open(GlobalApp->MainProj);
	}

} // SearchQuery::Edit

/*===========================================================================*/

DBFilterEvent *SearchQuery::AddFilter(DBFilterEvent *AddMe)
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
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
return (*CurFilter);

} // SearchQuery::AddFilter

/*===========================================================================*/

void SearchQuery::RemoveFilter(DBFilterEvent *RemoveMe)
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

} // SearchQuery::RemoveFilter

/*===========================================================================*/

int SearchQuery::ApproveJoe(Joe *ApproveMe)
{
DBFilterEvent *CurFilter = Filters;
int Approved = 0;

while (CurFilter)
	{
	if (! Approved && CurFilter->EventType == WCS_DBFILTER_EVENTTYPE_ADD)
		Approved = (CurFilter->PassJoe(ApproveMe) > 0);
	else if (Approved && CurFilter->EventType == WCS_DBFILTER_EVENTTYPE_SUB)
		Approved = (CurFilter->PassJoe(ApproveMe) >= 0);
	CurFilter = CurFilter->Next;
	} // while

return (Approved);

} // SearchQuery::ApproveJoe

/*===========================================================================*/

int SearchQuery::OneFilterValid(void)
{
DBFilterEvent *CurFilter = Filters;

while (CurFilter)
	{
	if (CurFilter->EventType == WCS_DBFILTER_EVENTTYPE_ADD)
		{
		if ((CurFilter->Layer && (CurFilter->LayerEquals || CurFilter->LayerSimilar)) || CurFilter->LayerNumeric)
			{
			return (1);
			} // if
		if ((CurFilter->Name && (CurFilter->NameEquals || CurFilter->NameSimilar)) || CurFilter->NameNumeric)
			{
			return (1);
			} // if
		if ((CurFilter->Label && (CurFilter->LabelEquals || CurFilter->LabelSimilar)) || CurFilter->LabelNumeric)
			{
			return (1);
			} // if
		if (CurFilter->Attribute && CurFilter->AttributeTest)
			{
			if (CurFilter->AttributeTest == WCS_DBFILTER_ATTRIBUTE_EXISTS)
				return (1);
			else if (CurFilter->AttributeValue)
				return (1);
			} // if
		if (CurFilter->GeoBndsInside || CurFilter->GeoBndsOutside)
			{
			return (1);
			} // if
		if (CurFilter->GeoPtContained || CurFilter->GeoPtUncontained)
			{
			return (1);
			} // if
		} // if
	CurFilter = CurFilter->Next;
	} // while

return (0);

} // SearchQuery::OneFilterValid

/*===========================================================================*/

unsigned long int SearchQuery::CountSuccessfulVectorCandidates(Database *DBHost)
{
Joe *VecWalk;
unsigned long int RunningCount = 0;

DBHost->ResetGeoClip();

for (VecWalk = DBHost->GetFirst(); VecWalk; VecWalk = DBHost->GetNext(VecWalk))
	{
	if (! VecWalk->TestFlags(WCS_JOEFLAG_HASKIDS) && ! VecWalk->TestFlags(WCS_JOEFLAG_ISDEM))
		{
		if (ApproveJoe(VecWalk))
			{
			RunningCount++;
			} // if
		} // if
	} // for

return(RunningCount);
} // SearchQuery::CountSuccessfulCandidates

/*===========================================================================*/

unsigned long int SearchQuery::CountSuccessfulCandidates(Database *DBHost)
{
Joe *VecWalk;
unsigned long int RunningCount = 0;

DBHost->ResetGeoClip();

for (VecWalk = DBHost->GetFirst(); VecWalk; VecWalk = DBHost->GetNext(VecWalk))
	{
	if (! VecWalk->TestFlags(WCS_JOEFLAG_HASKIDS))
		{
		if (ApproveJoe(VecWalk))
			{
			RunningCount++;
			} // if
		} // if
	} // for

return(RunningCount);
} // SearchQuery::CountSuccessfulCandidates

/*===========================================================================*/


void SearchQuery::HardLinkVectors(Database *DBHost, GeneralEffect *Effect)
{
Joe *VecWalk;
int ApplyToAll = 0, NewApplyToAll;

DBHost->ResetGeoClip();

for (VecWalk = DBHost->GetFirst(); VecWalk; VecWalk = DBHost->GetNext(VecWalk))
	{
	if (VecWalk->TestFlags(WCS_JOEFLAG_ACTIVATED) &&
		! VecWalk->TestFlags(WCS_JOEFLAG_HASKIDS) && ! VecWalk->TestFlags(WCS_JOEFLAG_ISDEM))
		{
		// test to see if it is already in the list
		if (ApproveJoe(VecWalk))
			{
			if ((NewApplyToAll = VecWalk->AddEffect(Effect, ApplyToAll)) == 2)
				ApplyToAll = 2;
			} // if
		} // if
	} // for
		
} // SearchQuery::HardLinkVectors

/*===========================================================================*/

void SearchQuery::SelectVectors(Database *DBHost, bool DeselectFirst, bool SetupDB)
{
// code originally from SearchQueryEditGUI::SelectDBItems
Joe *CurJoe, *FirstSelJoe = NULL;
unsigned long int TotalDBItems, CurItemLoop;
BusyWin *BWSL = NULL;

if(!GlobalApp->GUIWins->DBE)
	{
	if(GlobalApp->GUIWins->DBE = new DBEditGUI(GlobalApp->AppDB, GlobalApp->MainProj, GlobalApp->AppEffects))
		{
		if(GlobalApp->GUIWins->DBE->ConstructError)
			{
			delete GlobalApp->GUIWins->DBE;
			GlobalApp->GUIWins->DBE = NULL;
			} // if
		} // if
	} // if
if(GlobalApp->GUIWins->DBE)
	{
	if (SetupDB)
		GlobalApp->GUIWins->DBE->Open(GlobalApp->MainProj);

	// clear selected items
	if (DeselectFirst)
		GlobalApp->GUIWins->DBE->DeselectAll();

	if (SetupDB)
		{
		GlobalApp->AppDB->ResetGeoClip();
		GlobalApp->GUIWins->DBE->HardFreeze();
		} // if
		
	// Count all items in database to size progress display 
	for (TotalDBItems = 0, CurJoe = GlobalApp->AppDB->GetFirst(); CurJoe; CurJoe = GlobalApp->AppDB->GetNext(CurJoe))
		{
		TotalDBItems++;
		} // for

	BWSL = new BusyWin("Selecting", TotalDBItems, 'BWSL', 0);
	for (CurItemLoop = 0, CurJoe = GlobalApp->AppDB->GetFirst(); CurJoe; CurJoe = GlobalApp->AppDB->GetNext(CurJoe))
		{
		if (! CurJoe->TestFlags(WCS_JOEFLAG_HASKIDS) && ! CurJoe->TestFlags(WCS_JOEFLAG_ISDEM))
			{
			if (ApproveJoe(CurJoe))
				{
				// set item selected
				GlobalApp->GUIWins->DBE->ChangeSelection(CurJoe, TRUE);
				if (! FirstSelJoe)
					{
					FirstSelJoe = CurJoe;
					GlobalApp->AppDB->SetActiveObj(FirstSelJoe);
					} // if
				} // if
			if(BWSL && (CurItemLoop & 0xff)) // only update every 256th time, for efficiency
				{
				BWSL->Update(CurItemLoop);
				if (BWSL->CheckAbort())
					{
					break; // bail out of select loop
					}
				} // if
			} // if
		CurItemLoop++;
		} // for

	if(BWSL)
		{
		// in case it didn't update recently
		BWSL->Update(CurItemLoop);
		delete BWSL;
		} // if

	if (SetupDB)
		{
		GlobalApp->GUIWins->DBE->HardUnFreeze();
		GlobalApp->GUIWins->DBE->UpdateTitle();
		} // if
	} // if

} // SearchQuery::SelectVectors

/*===========================================================================*/

void SearchQuery::EnableVectors(Database *DBHost, bool NewState)
{
Joe *VecWalk;
DBFilterEvent *CurFilter;

DBHost->ResetGeoClip();

CurFilter = Filters;
while (CurFilter)
	{
	CurFilter->StashPassDisabled = CurFilter->PassDisabled;
	CurFilter->PassDisabled = 1;
	CurFilter->StashPassEnabled = CurFilter->PassEnabled;
	CurFilter->PassEnabled = 1;
	CurFilter = CurFilter->Next;
	} // while

for (VecWalk = DBHost->GetFirst(); VecWalk; VecWalk = DBHost->GetNext(VecWalk))
	{
	if (! VecWalk->TestFlags(WCS_JOEFLAG_HASKIDS) && ! VecWalk->TestFlags(WCS_JOEFLAG_ISDEM))
		{
		// test to see if it is in the list
		if (ApproveJoe(VecWalk))
			{
			if (NewState)
				VecWalk->SetFlags(WCS_JOEFLAG_ACTIVATED);
			else
				VecWalk->ClearFlags(WCS_JOEFLAG_ACTIVATED);
			} // if
		} // if
	} // for
	
CurFilter = Filters;
while (CurFilter)
	{
	CurFilter->PassDisabled = CurFilter->StashPassDisabled;
	CurFilter->PassEnabled = CurFilter->StashPassEnabled;
	CurFilter = CurFilter->Next;
	} // while

} // SearchQuery::EnableVectors

/*===========================================================================*/

char SearchQuery::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);

return (0);

} // SearchQuery::GetRAHostDropOK

/*===========================================================================*/

int SearchQuery::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
int Success;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];
char QueryStr[256], NameStr[128];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (SearchQuery *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (SearchQuery *)DropSource->DropSource);
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

} // SearchQuery::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long SearchQuery::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_SCHQY | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // SearchQuery::GetRAFlags

/*===========================================================================*/

RasterAnimHost *SearchQuery::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{

return (NULL);

} // SearchQuery::GetRAHostChild

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int SearchQuery::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
EffectsLib *LoadToEffects = NULL;
SearchQuery *CurrentSearchQuery = NULL;
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
DEMBounds OldBounds, CurBounds;
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];

if (! ffile)
	return (0);

if (LoadToEffects = new EffectsLib())
	{
	// set some global pointers so that things know what libraries to link to
	GlobalApp->LoadToEffectsLib = LoadToEffects;

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
				else if (! strnicmp(ReadBuf, "Search", 8))
					{
					if (CurrentSearchQuery = new SearchQuery(NULL, LoadToEffects, NULL))
						{
						if ((BytesRead = CurrentSearchQuery->Load(ffile, Size, ByteFlip)) == Size)
							Success = 1;	// we got our man
						}
					} // if SearchQuery
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
	} // if effects lib
else
	Success = 0;

if (Success == 1 && CurrentSearchQuery)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	Copy(this, CurrentSearchQuery);
	strcpy(ReadBuf, Name);
	SetUniqueName(GlobalApp->AppEffects, ReadBuf);
	} // if

if (LoadToEffects)
	delete LoadToEffects;
GlobalApp->CopyFromEffectsLib = GlobalApp->AppEffects;
GlobalApp->LoadToEffectsLib = GlobalApp->AppEffects;

return (Success);

} // SearchQuery::LoadObject

/*===========================================================================*/

int SearchQuery::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
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

// SearchQuery
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
			} // if SearchQuery saved 
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

} // SearchQuery::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::SearchQuery_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
SearchQuery *Current;

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
						if (Current = new SearchQuery(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (SearchQuery *)FindDuplicateByName(Current->EffectType, Current))
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

} // EffectsLib::SearchQuery_Load()

/*===========================================================================*/

ULONG EffectsLib::SearchQuery_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
SearchQuery *Current;

Current = Search;
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
	Current = (SearchQuery *)Current->Next;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // EffectsLib::SearchQuery_Save

/*===========================================================================*/


int SearchQuery::ActionNow(void)
{
// code originally from SearchQueryEditGUI::SelectDBItems
Joe *CurJoe, *FirstSelJoe = NULL;
unsigned long int TotalDBItems, CurItemLoop;
BusyWin *BWSL = NULL;

if(!GlobalApp->GUIWins->DBE)
	{
	if(GlobalApp->GUIWins->DBE = new DBEditGUI(GlobalApp->AppDB, GlobalApp->MainProj, GlobalApp->AppEffects))
		{
		if(GlobalApp->GUIWins->DBE->ConstructError)
			{
			delete GlobalApp->GUIWins->DBE;
			GlobalApp->GUIWins->DBE = NULL;
			} // if
		} // if
	} // if
if(GlobalApp->GUIWins->DBE)
	{
	GlobalApp->GUIWins->DBE->Open(GlobalApp->MainProj);

	// clear selected items
	GlobalApp->GUIWins->DBE->DeselectAll();

	GlobalApp->AppDB->ResetGeoClip();

	GlobalApp->GUIWins->DBE->HardFreeze();
	// Count all items in database to size progress display 
	for (TotalDBItems = 0, CurJoe = GlobalApp->AppDB->GetFirst(); CurJoe; CurJoe = GlobalApp->AppDB->GetNext(CurJoe))
		{
		TotalDBItems++;
		} // for

	BWSL = new BusyWin("Selecting", TotalDBItems, 'BWSL', 0);
	for (CurItemLoop = 0, CurJoe = GlobalApp->AppDB->GetFirst(); CurJoe; CurJoe = GlobalApp->AppDB->GetNext(CurJoe))
		{
		if (ApproveJoe(CurJoe))
			{
			// set item selected
			GlobalApp->GUIWins->DBE->ChangeSelection(CurJoe, TRUE);
			if (! FirstSelJoe)
				{
				FirstSelJoe = CurJoe;
				GlobalApp->AppDB->SetActiveObj(FirstSelJoe);
				} // if
			} // if
		if(BWSL && (CurItemLoop & 0xff)) // only update every 256th time, for efficiency
			{
			BWSL->Update(CurItemLoop);
			if (BWSL->CheckAbort())
				{
				break; // bail out of select loop
				}
			} // if
		CurItemLoop++;
		} // for
	if(BWSL)
		{
		// in case it didn't update recently
		BWSL->Update(CurItemLoop);
		delete BWSL;
		} // if

	GlobalApp->GUIWins->DBE->HardUnFreeze();
	GlobalApp->GUIWins->DBE->UpdateTitle();
	} // if

return(1);

} // SearchQuery::ActionNow

/*===========================================================================*/

int SearchQuery::AddDerivedPopMenus(PopMenuAdder *PMA, unsigned long int MenuClassFlags)
{

PMA->AddPopMenuItem("Select Now", "FLOAT", 1);

return(1);

} // SearchQuery::AddDerivedPopMenus

/*===========================================================================*/

int SearchQuery::HandlePopMenuSelection(void *Action)
{
// only one action known currently

ActionNow();

return(1);

} // SearchQuery::HandlePopMenuSelection
