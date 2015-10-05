// Scenario.cpp
// For managing RenderScenario Effects
// Built from scratch on 9/12/02 by Gary R. Huber
// Copyright 2002 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "EffectsLib.h"
#include "Application.h"
#include "Conservatory.h"
#include "Joe.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Raster.h"
#include "requester.h"
#include "ScenarioEditGUI.h"
#include "DBEditGUI.h"
#include "ImageLibGUI.h"
#include "AppMem.h"
#include "Render.h"
#include "Database.h"
#include "Security.h"
#include "Toolbar.h"
#include "Project.h"
#include "IncludeExcludeList.h"
#include "Lists.h"
#include "FeatureConfig.h"

RenderScenario::RenderScenario()
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_SCENARIO;
SetDefaults();

} // RenderScenario::RenderScenario

/*===========================================================================*/

RenderScenario::RenderScenario(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_SCENARIO;
SetDefaults();

} // RenderScenario::RenderScenario

/*===========================================================================*/

RenderScenario::RenderScenario(RasterAnimHost *RAHost, EffectsLib *Library, RenderScenario *Proto)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_SCENARIO;
Prev = Library->LastScenario;
if (Library->LastScenario)
	{
	Library->LastScenario->Next = this;
	Library->LastScenario = this;
	} // if
else
	{
	Library->Scenario = Library->LastScenario = this;
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
	strcpy(NameBase, "Render Scenario");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // RenderScenario::RenderScenario

/*===========================================================================*/

RenderScenario::~RenderScenario()
{
RasterAnimHostBooleanList *NextItem;
NumberList *NextNumber;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->SCN && GlobalApp->GUIWins->SCN->GetActive() == this)
		{
		delete GlobalApp->GUIWins->SCN;
		GlobalApp->GUIWins->SCN = NULL;
		} // if
	} // if

while (Items)
	{
	NextItem = Items;
	Items = (RasterAnimHostBooleanList *)Items->Next;
	delete NextItem;
	} // while

while (DBItemNumbers)
	{
	NextNumber = DBItemNumbers->Next;
	delete DBItemNumbers;
	DBItemNumbers = NextNumber;
	} // if

if (InitialStates)
	AppMem_Free(InitialStates, NumItems * sizeof (char));
InitialStates = NULL;

} // RenderScenario::~RenderScenario

/*===========================================================================*/

void RenderScenario::SetDefaults(void)
{
double RangeDefaults[3] = {1.0, 0.0, 1.0};

RegenShadows = 1;
Items = NULL;
DBItemNumbers = NULL;
InitialStates = NULL;
NumItems = 0;
MasterControl.SetDefaults(this, (char)0, 1.0);
MasterControl.SetRangeDefaults(RangeDefaults);
MasterControl.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
MasterControl.AddNode(0.0, 1.0, 0.0);
MasterControl.ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

} // RenderScenario::SetDefaults

/*===========================================================================*/

void RenderScenario::Copy(RenderScenario *CopyTo, RenderScenario *CopyFrom)
{
RasterAnimHostBooleanList *NextItem, **ToItem;

while (CopyTo->Items)
	{
	NextItem = CopyTo->Items;
	CopyTo->Items = (RasterAnimHostBooleanList *)CopyTo->Items->Next;
	delete NextItem;
	} // if
NextItem = CopyFrom->Items;
// assign pointers only, do not make new items
if (GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
	{
	ToItem = &CopyTo->Items;
	while (NextItem)
		{
		if (NextItem->Me)
			{
			// <<<>>>gh check to see if the item still exists
			if (*ToItem = new RasterAnimHostBooleanList())
				{
				(*ToItem)->Me = NextItem->Me;
				ToItem = (RasterAnimHostBooleanList **)&(*ToItem)->Next;
				} // if
			} // if
		NextItem = (RasterAnimHostBooleanList *)NextItem->Next;
		} // while
	} // if

CopyTo->MasterControl.Copy(&CopyTo->MasterControl, &CopyFrom->MasterControl);
CopyTo->RegenShadows = CopyFrom->RegenShadows;
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // RenderScenario::Copy

/*===========================================================================*/

ULONG RenderScenario::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
long EffectType = 0;
unsigned long ImageID;
union MultiVal MV;
char MatchName[WCS_EFFECT_MAXNAMELENGTH];
RasterAnimHostBooleanList **CurItem = &Items;
NumberList **CurNumber = &DBItemNumbers;

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
					} /* switch */

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
					case WCS_EFFECTS_SCENARIO_REGENSHADOWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&RegenShadows, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENARIO_MASTERCONTROL:
						{
						BytesRead = MasterControl.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENARIO_EFFECTTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&EffectType, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SCENARIO_ITEMBOOLEAN:
						{
						if (*CurItem || (*CurItem = new RasterAnimHostBooleanList()))
							BytesRead = ReadBlock(ffile, (char *)&(*CurItem)->TrueFalse, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_SCENARIO_ITEMJOEBOOLEAN:
						{
						if (*CurNumber || (*CurNumber = new NumberList()))
							BytesRead = ReadBlock(ffile, (char *)&(*CurNumber)->TrueFalse, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_SCENARIO_EFFECTNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							if (*CurItem || (*CurItem = new RasterAnimHostBooleanList()))
								{
								if ((*CurItem)->Me = GlobalApp->LoadToEffectsLib->FindByName(EffectType, MatchName))
									CurItem = (RasterAnimHostBooleanList **)&(*CurItem)->Next;
								else
									{
									delete *CurItem;
									*CurItem = NULL;
									} // else
								} // if
							} // if
						break;
						}
					case WCS_EFFECTS_SCENARIO_JOENUMBER:
						{
						//BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						BytesRead = ReadBlock(ffile, (char *)&ImageID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (ImageID > 0)
							{
							if (*CurNumber || (*CurNumber = new NumberList()))
								{
								(*CurNumber)->Number = ImageID;
								CurNumber = &(*CurNumber)->Next;
								} // if
							} // if
						break;
						}
					case WCS_EFFECTS_SCENARIO_IMAGEID:
						{
						BytesRead = ReadBlock(ffile, (char *)&ImageID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (ImageID > 0 && (*CurItem || (*CurItem = new RasterAnimHostBooleanList())))
							{
							(*CurItem)->Me = GlobalApp->LoadToImageLib->FindByID(ImageID);
							CurItem = (RasterAnimHostBooleanList **)&(*CurItem)->Next;
							} // if
						break;
						}
					#ifdef WCS_BUILD_VNS
					case WCS_EFFECTS_QUERY:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Search = (SearchQuery *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_SEARCHQUERY, MatchName);
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
					} /* switch */

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} /* if size block read */
			else
				break;
			} /* if not done flag */
		} /* if tag block read */
	else
		break;
	} /* while */

if (*CurItem && ! (*CurItem)->Me)
	{
	// for some reason added an item that never had a pointer read into it
	delete *CurItem;
	*CurItem = NULL;
	} // else
if (*CurNumber && ! (*CurNumber)->Number)
	{
	// for some reason added an item that never had a pointer read into it
	delete *CurNumber;
	*CurNumber = NULL;
	} // else

return (TotalRead);

} // RenderScenario::Load

/*===========================================================================*/

unsigned long int RenderScenario::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
unsigned long ImageID;
RasterAnimHostBooleanList *CurItem;
RasterAnimHostProperties Prop;

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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENARIO_REGENSHADOWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RegenShadows)) == NULL)
   goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_EFFECTS_SCENARIO_MASTERCONTROL + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = MasterControl.Save(ffile))
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
			} // if MasterControl saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

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
			// write out the boolean value expressing whether the item obeys or contradicts the master control
			// if an effect write out the effect class then the name
			if (Prop.TypeNumber >= WCS_EFFECTSSUBCLASS_LAKE && Prop.TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
				{
				if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENARIO_ITEMBOOLEAN, WCS_BLOCKSIZE_CHAR,
					WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
					WCS_BLOCKTYPE_CHAR, (char *)&CurItem->TrueFalse)) == NULL)
					goto WriteError;
				TotalWritten += BytesWritten;
				if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENARIO_EFFECTTYPE, WCS_BLOCKSIZE_CHAR,
					WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
					WCS_BLOCKTYPE_LONGINT, (char *)&Prop.TypeNumber)) == NULL)
				   goto WriteError;
				TotalWritten += BytesWritten;
				if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENARIO_EFFECTNAME, WCS_BLOCKSIZE_CHAR,
					WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Prop.Name) + 1),
					WCS_BLOCKTYPE_CHAR, (char *)Prop.Name)) == NULL)
					goto WriteError;
				TotalWritten += BytesWritten;
				} // if effect
			// if an image write out the image ID
			else if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_RASTER)
				{
				if ((ImageID = ((Raster *)CurItem->Me)->GetID()) > 0)
					{
					if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENARIO_ITEMBOOLEAN, WCS_BLOCKSIZE_CHAR,
						WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
						WCS_BLOCKTYPE_CHAR, (char *)&CurItem->TrueFalse)) == NULL)
						goto WriteError;
					TotalWritten += BytesWritten;
					if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENARIO_IMAGEID, WCS_BLOCKSIZE_CHAR,
						WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
						WCS_BLOCKTYPE_LONGINT, (char *)&ImageID)) == NULL)
						goto WriteError;
					TotalWritten += BytesWritten;
					} // if
				} // else if image
			// if a Joe write out the best name or if there is none skip it
			// won't compile under WCS due to lack of UniqueLoadSaveID member of Joe
			#ifdef WCS_BUILD_VNS
			else if (Prop.TypeNumber >= WCS_RAHOST_OBJTYPE_VECTOR && Prop.TypeNumber <= WCS_RAHOST_OBJTYPE_CONTROLPT)
				{
				if ((ImageID = ((Joe *)CurItem->Me)->UniqueLoadSaveID) > 0)
					{
					if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENARIO_ITEMJOEBOOLEAN, WCS_BLOCKSIZE_CHAR,
						WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
						WCS_BLOCKTYPE_CHAR, (char *)&CurItem->TrueFalse)) == NULL)
						goto WriteError;
					TotalWritten += BytesWritten;
					if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SCENARIO_JOENUMBER, WCS_BLOCKSIZE_CHAR,
						WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
						WCS_BLOCKTYPE_LONGINT, (char *)&ImageID)) == NULL)
						goto WriteError;
					TotalWritten += BytesWritten;
					} // if
				} // else if joe
			#endif // WCS_BUILD_VNS
			} // if
		} // if
	CurItem = (RasterAnimHostBooleanList *)CurItem->Next;
	} // while

#ifdef WCS_BUILD_VNS
if (Search)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_QUERY, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Search->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Search->GetName())) == NULL)
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

} // RenderScenario::Save

/*===========================================================================*/

void RenderScenario::ResolveDBLoadLinkages(Database *HostDB, Joe **UniqueIDTable, unsigned long HighestDBID)
{
NumberList *CurDBItem = DBItemNumbers, *NextNumber;
RasterAnimHostBooleanList **CurItem = &Items;

while (*CurItem)
	{
	CurItem = (RasterAnimHostBooleanList **)&(*CurItem)->Next;
	} // while

while (CurDBItem)
	{
	if (CurDBItem->Number > 0 && CurDBItem->Number <= HighestDBID)
		{
		if (*CurItem = new RasterAnimHostBooleanList())
			{
			(*CurItem)->TrueFalse = CurDBItem->TrueFalse;
			//if ((*CurItem)->Me = HostDB->FindByBestName(CurDBItem->Name))
			if ((*CurItem)->Me = UniqueIDTable[CurDBItem->Number])
				CurItem = (RasterAnimHostBooleanList **)&(*CurItem)->Next;
			else
				{
				delete *CurItem;
				*CurItem = NULL;
				} // else
			} // if
		} // if
	CurDBItem = CurDBItem->Next;
	} // if

while (DBItemNumbers)
	{
	NextNumber = DBItemNumbers;
	DBItemNumbers = DBItemNumbers->Next;
	delete NextNumber;
	} // if

} // RenderScenario::ResolveDBLoadLinkages

/*===========================================================================*/

void RenderScenario::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->SCN)
	{
	delete GlobalApp->GUIWins->SCN;
	}
GlobalApp->GUIWins->SCN = new ScenarioEditGUI(GlobalApp->AppEffects, GlobalApp->AppDB, GlobalApp->AppImages, this);
if(GlobalApp->GUIWins->SCN)
	{
	GlobalApp->GUIWins->SCN->Open(GlobalApp->MainProj);
	}

} // RenderScenario::Edit

/*===========================================================================*/

char *RenderScenario::GetCritterName(RasterAnimHost *Test)
{

if (Test == &MasterControl)
	return ("Master Control Profile");

return ("");

} // RenderScenario::GetCritterName

/*===========================================================================*/

long RenderScenario::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Found = 0;

if (GeneralEffect::GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (MasterControl.GetMinMaxDist(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if

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

} // RenderScenario::GetKeyFrameRange

/*===========================================================================*/

RasterAnimHostBooleanList *RenderScenario::AddItem(RasterAnimHost *AddMe, int GenNotify)
{
RasterAnimHostBooleanList **CurItem = &Items;
NotifyTag Changes[2];

if (AddMe)
	{
	while (*CurItem)
		{
		if ((*CurItem)->Me == AddMe)
			return (NULL);
		CurItem = (RasterAnimHostBooleanList **)&(*CurItem)->Next;
		} // while
	if (*CurItem = new RasterAnimHostBooleanList())
		{
		(*CurItem)->Me = AddMe;
		if (GenNotify)
			{
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	return (*CurItem);
	} // if

return (NULL);

} // RenderScenario::AddItem

/*===========================================================================*/

long RenderScenario::AddDBItem(void)
{
long LastOBN = -1, Added = 0;
Joe *CurJoe;
NotifyTag Changes[2];

if (GlobalApp->GUIWins->DBE)
	{
	if (UserMessageOKCAN("Add Database Objects", "If desired Objects are selected in the Database Editor click \"OK\". If not then click \"Cancel\", multi-select the Objects and click \"Add Items\" again."))
		{
		while (CurJoe = GlobalApp->GUIWins->DBE->GetNextSelected(LastOBN))
			{
			if (AddItem(CurJoe, FALSE))
				Added ++;
			} // if
		if (Added)
			{
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
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

} // RenderScenario::AddDBItem

/*===========================================================================*/

long RenderScenario::AddImageItem(void)
{
Raster *CurRast;
NotifyTag Changes[2];
long LastOBN = -1, Added = 0;

if (GlobalApp->GUIWins->ILG)
	{
	if (UserMessageOKCAN("Add Image Objects", "If desired Image Objects are selected in the Image Object Library click \"OK\". If not then click \"Cancel\", multi-select the Image Objects and click \"Add Items\" again."))
		{
		while (CurRast = GlobalApp->GUIWins->ILG->GetNextSelected(LastOBN))
			{
			if (AddItem(CurRast, FALSE))
				Added ++;
			} // if
		if (Added)
			{
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		else
			{
			UserMessageOK("Add Image Objects", "There are no Image Objects selected in the Image Object Library!");
			} // else
		} // if
	} // if
else
	{
	UserMessageOK("Add Image Objects", "When the Image Object Library opens, select the desired Image Objects and then click \"Add Items\" again.");
	GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		WCS_TOOLBAR_ITEM_ILG, 0);
	} // else

return (Added);

} // RenderScenario::AddDBItem

/*===========================================================================*/

void RenderScenario::HardLinkVectors(Database *DBHost)
{
long Added = 0;
Joe *VecWalk;
NotifyTag Changes[2];

if (Search)
	{
	DBHost->ResetGeoClip();

	for (VecWalk = DBHost->GetFirst(); VecWalk; VecWalk = DBHost->GetNext(VecWalk))
		{
		if (! VecWalk->TestFlags(WCS_JOEFLAG_HASKIDS))
			{
			// test to see if it is already in the list
			if (Search->ApproveJoe(VecWalk))
				{
				if (AddItem(VecWalk, FALSE))
					Added ++;
				} // if
			} // if
		} // for
	if (Added)
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		if (UserMessageYN(Name, "Remove Dynamic Linkage now?"))
			RemoveRAHost(Search);
		} // if
	} // if
else
	UserMessageOK(Name, "There is no Search Query attached to this Component.");

} // RenderScenario::HardLinkVectors

/*===========================================================================*/

long RenderScenario::CountVectors(void)
{
long ItemCt = 0;
RasterAnimHostBooleanList *CurItem;
RasterAnimHostProperties Prop;

Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
for (CurItem = Items; CurItem; CurItem = (RasterAnimHostBooleanList *)CurItem->Next)
	{
	CurItem->Me->GetRAHostProperties(&Prop);
	if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR || Prop.TypeNumber == WCS_RAHOST_OBJTYPE_CONTROLPT)
		++ItemCt;
	} // while

return (ItemCt);

} // RenderScenario::CountVectors

/*===========================================================================*/

long RenderScenario::AddItemsByClass(EffectsLib *HostEffects, ImageLib *HostImages, Database *HostDB, long ItemClass)
{
long NumAdded = 0, AddResult;
GeneralEffect *CurEffect;
Raster *CurRast;
Joe *CurJoe;
NotifyTag Changes[2];

if (ItemClass >= WCS_EFFECTSSUBCLASS_LAKE && ItemClass < WCS_MAXIMPLEMENTED_EFFECTS)
	{
	for (CurEffect = HostEffects->GetListPtr(ItemClass); CurEffect; CurEffect = CurEffect->Next)
		{
		if (AddItem(CurEffect, 0))
			NumAdded ++;
		} // for
	} // if
else if (ItemClass == WCS_RAHOST_OBJTYPE_VECTOR)
	{
	HostDB->ResetGeoClip();
	AddResult = UserMessageCustomQuad(Name, "Which types of Database Items do you wish to add to this Scenario?", "All", "DEMs", "Vectors", "Control Points", 0);
	for (CurJoe = HostDB->GetFirst(); CurJoe; CurJoe = HostDB->GetNext(CurJoe))
		{
		if (! CurJoe->TestFlags(WCS_JOEFLAG_HASKIDS))
			{
			if (AddResult == 1 || (AddResult == 0 && CurJoe->TestFlags(WCS_JOEFLAG_ISDEM)) || (AddResult == 3 && CurJoe->TestFlags(WCS_JOEFLAG_ISCONTROL)) || (AddResult == 2 && ! CurJoe->TestFlags(WCS_JOEFLAG_ISDEM) && ! CurJoe->TestFlags(WCS_JOEFLAG_ISCONTROL)))
				{
				if (AddItem(CurJoe, 0))
					NumAdded ++;
				} // if
			} // if
		} // for
	} // else if
else if (ItemClass == WCS_RAHOST_OBJTYPE_RASTER)
	{
	for (CurRast = HostImages->GetFirstRast(); CurRast; CurRast = HostImages->GetNextRast(CurRast))
		{
		if (AddItem(CurRast, 0))
			NumAdded ++;
		} // for
	} // else if

if (NumAdded)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

return (NumAdded);

} // RenderScenario::AddItemsByClass

/*===========================================================================*/

int RenderScenario::RemoveRAHost(RasterAnimHost *RemoveMe)
{
RasterAnimHostBooleanList *CurItem = Items, *PrevItem = NULL;
NotifyTag Changes[2];

while (CurItem)
	{
	if (CurItem->Me == RemoveMe)
		{
		if (PrevItem)
			PrevItem->Next = CurItem->Next;
		else
			Items = (RasterAnimHostBooleanList *)CurItem->Next;

		delete CurItem;

		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

		return (1);
		} // if
	PrevItem = CurItem;
	CurItem = (RasterAnimHostBooleanList *)CurItem->Next;
	} // while

return (GeneralEffect::RemoveRAHost(RemoveMe));

} // RenderScenario::RemoveRAHost

/*===========================================================================*/

char RenderScenario::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_EFFECTSSUBCLASS_LAKE
	|| DropType == WCS_EFFECTSSUBCLASS_ECOSYSTEM
	|| DropType == WCS_EFFECTSSUBCLASS_WAVE
	|| DropType == WCS_EFFECTSSUBCLASS_CLOUD
	|| DropType == WCS_EFFECTSSUBCLASS_ENVIRONMENT
	|| DropType == WCS_EFFECTSSUBCLASS_CMAP
	|| DropType == WCS_EFFECTSSUBCLASS_RASTERTA
	|| DropType == WCS_EFFECTSSUBCLASS_TERRAFFECTOR
	|| DropType == WCS_EFFECTSSUBCLASS_FOLIAGE
	|| DropType == WCS_EFFECTSSUBCLASS_OBJECT3D
	|| DropType == WCS_EFFECTSSUBCLASS_SHADOW
	|| DropType == WCS_EFFECTSSUBCLASS_STREAM
	|| DropType == WCS_EFFECTSSUBCLASS_CELESTIAL
	|| DropType == WCS_EFFECTSSUBCLASS_STARFIELD
	|| DropType == WCS_EFFECTSSUBCLASS_PLANETOPT
	|| DropType == WCS_EFFECTSSUBCLASS_TERRAINPARAM
	|| DropType == WCS_EFFECTSSUBCLASS_GROUND
	|| DropType == WCS_EFFECTSSUBCLASS_SNOW
	|| DropType == WCS_EFFECTSSUBCLASS_SKY
	|| DropType == WCS_EFFECTSSUBCLASS_ATMOSPHERE
	|| DropType == WCS_EFFECTSSUBCLASS_LIGHT
	|| DropType == WCS_EFFECTSSUBCLASS_SEARCHQUERY
	|| DropType == WCS_EFFECTSSUBCLASS_THEMATICMAP
	|| DropType == WCS_EFFECTSSUBCLASS_FENCE
	|| DropType == WCS_EFFECTSSUBCLASS_POSTPROC
	|| DropType == WCS_EFFECTSSUBCLASS_LABEL
	// <<<>>> ADD_NEW_EFFECTS add new effect type if it can be controlled by Scenarios
	|| DropType == WCS_RAHOST_OBJTYPE_VECTOR
	|| DropType == WCS_RAHOST_OBJTYPE_DEM
	|| DropType == WCS_RAHOST_OBJTYPE_CONTROLPT)
	return (1);

return (0);

} // RenderScenario::GetRAHostDropOK

/*===========================================================================*/

int RenderScenario::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
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
	if (this != (RenderScenario *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (RenderScenario *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_LAKE
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_ECOSYSTEM
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_WAVE
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_CLOUD
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_ENVIRONMENT
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_CMAP
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_RASTERTA
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_TERRAFFECTOR
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_FOLIAGE
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_OBJECT3D
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_SHADOW
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_STREAM
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_CELESTIAL
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_STARFIELD
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_PLANETOPT
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_TERRAINPARAM
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_GROUND
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_SNOW
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_SKY
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_ATMOSPHERE
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_LIGHT
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_SEARCHQUERY
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_THEMATICMAP
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_FENCE
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_POSTPROC
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_LABEL
	// <<<>>> ADD_NEW_EFFECTS add new effect type if it can be controlled by Scenarios
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_DEM
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_CONTROLPT)
	{
	Success = -1;
	sprintf(QueryStr, "Add %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		if (AddItem(DropSource->DropSource, TRUE))
			{
			Success = 1;
			} // if
		} // if
	} // else if
else
	{
	Success = GeneralEffect::ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // RenderScenario::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long RenderScenario::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_RENDSCEN | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // RenderScenario::GetRAFlags

/*===========================================================================*/

RasterAnimHost *RenderScenario::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Found = 0;
RasterAnimHostBooleanList *CurItem;
JoeList *CurJoe = Joes;

if (! Current)
	return (&MasterControl);
if (Current == &MasterControl)
	Found = 1;
CurItem = Items;
while (CurItem)
	{
	if (Found)
		return (CurItem->Me);
	if (Current == CurItem->Me)
		Found = 1;
	CurItem = (RasterAnimHostBooleanList *)CurItem->Next;
	} // while

return (NULL);

} // RenderScenario::GetRAHostChild

/*===========================================================================*/

int RenderScenario::AddDerivedPopMenus(PopMenuAdder *PMA, unsigned long int MenuClassFlags)
{

PMA->AddPopMenuItem("Action Now", "ACTIONNOW", 1);

return(1);

} // RenderScenario::AddDerivedPopMenus

/*===========================================================================*/

int RenderScenario::HandlePopMenuSelection(void *Action)
{

// only one action known currently
ActionNow();
return(1);

} // RenderScenario::HandlePopMenuSelection

/*===========================================================================*/

int RenderScenario::ActionNow(void)
{
// this code originally from ScenarioEditGUI::Actionate
int Dummy;
NotifyTag Cryogenics[2];

Cryogenics[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
Cryogenics[1] = NULL;

GlobalApp->AppEx->GenerateNotify(Cryogenics);
ProcessFrameToRender(GlobalApp->MainProj->Interactive->GetActiveTime(), Dummy, GlobalApp->AppDB);
Cryogenics[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
GlobalApp->AppEx->GenerateNotify(Cryogenics);

return(1);

} // RenderScenario::ActionNow

/*===========================================================================*/

int RenderScenario::SetupToRender(double RenderTime, int &RegenShadowMaps, Database *DBHost)
{
long ItemCt;
RasterAnimHostBooleanList *CurItem = Items;
Joe *VecWalk;
RasterAnimHostProperties Prop;
char RenderState;

if (InitialStates)
	AppMem_Free(InitialStates, NumItems * sizeof (char));
InitialStates = NULL;
NumItems = 0;
RegenShadowMaps = 0;

if (Enabled)
	{
	// count items
	while (CurItem)
		{
		NumItems ++;
		CurItem = (RasterAnimHostBooleanList *)CurItem->Next;
		} // while
	// count search query items
	if (Search)
		{
		DBHost->ResetGeoClip();

		for (VecWalk = DBHost->GetFirst(); VecWalk; VecWalk = DBHost->GetNext(VecWalk))
			{
			if (! VecWalk->TestFlags(WCS_JOEFLAG_HASKIDS))
				{
				if (Search->ApproveJoe(VecWalk))
					{
					NumItems ++;
					} // if
				} // if
			} // for
		} // if

	if (NumItems > 0)
		{
		// create list of enabled states as first found
		if (InitialStates = (char *)AppMem_Alloc(NumItems * sizeof (char), APPMEM_CLEAR))
			{
			Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS;
			Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;
			CurItem = Items;
			ItemCt = 0;
			while (CurItem)
				{
				if (CurItem->Me)
					{
					CurItem->Me->GetRAHostProperties(&Prop);
					InitialStates[ItemCt] = Prop.Flags & WCS_RAHOST_FLAGBIT_ENABLED ? 1: 0;
					} // if
				ItemCt ++;
				CurItem = (RasterAnimHostBooleanList *)CurItem->Next;
				} // while
			// store states of search query items
			if (Search)
				{
				for (VecWalk = DBHost->GetFirst(); VecWalk; VecWalk = DBHost->GetNext(VecWalk))
					{
					if (! VecWalk->TestFlags(WCS_JOEFLAG_HASKIDS))
						{
						if (Search->ApproveJoe(VecWalk))
							{
							VecWalk->GetRAHostProperties(&Prop);
							InitialStates[ItemCt] = Prop.Flags & WCS_RAHOST_FLAGBIT_ENABLED ? 1: 0;
							ItemCt ++;
							} // if
						} // if
					} // for
				} // if

			// set enabled states if necessary
			RenderState = MasterControl.GetValue(0, RenderTime) > .5 ? 1: 0;
			CurItem = Items;
			ItemCt = 0;
			while (CurItem)
				{
				if (CurItem->Me)
					{
					// TrueFalse is true (1) if the item state is to be reversed from RenderState
					if (! CurItem->TrueFalse)
						{
						// ItemState needs to match RenderState
						if (InitialStates[ItemCt] != RenderState)
							{
							Prop.Flags = RenderState ? WCS_RAHOST_FLAGBIT_ENABLED: 0;
							CurItem->Me->SetRAHostProperties(&Prop);
							} // if
						} // if
					else
						{
						// ItemState needs to be opposite RenderState
						if (InitialStates[ItemCt] == RenderState)
							{
							Prop.Flags = RenderState ? 0: WCS_RAHOST_FLAGBIT_ENABLED;
							CurItem->Me->SetRAHostProperties(&Prop);
							} // if
						} // else
					} // if
				ItemCt ++;
				CurItem = (RasterAnimHostBooleanList *)CurItem->Next;
				} // while
			// set states of search query items
			if (Search)
				{
				Prop.Flags = RenderState ? WCS_RAHOST_FLAGBIT_ENABLED: 0;
				for (VecWalk = DBHost->GetFirst(); VecWalk; VecWalk = DBHost->GetNext(VecWalk))
					{
					if (! VecWalk->TestFlags(WCS_JOEFLAG_HASKIDS))
						{
						if (Search->ApproveJoe(VecWalk))
							{
							if (InitialStates[ItemCt] != RenderState)
								{
								VecWalk->SetRAHostProperties(&Prop);
								} // if
							ItemCt ++;
							} // if
						} // if
					} // for
				} // if

			} // if
		else
			return (0);
		} // if
	} // if

// regenerate shadow maps on first frame because they might have been left in indeterminate state from previous renders
RegenShadowMaps = RegenShadows;

return (1);

} // RenderScenario::SetupToRender

/*===========================================================================*/

void RenderScenario::CleanupFromRender(Database *DBHost)
{
long ItemCt;
RasterAnimHostBooleanList *CurItem = Items;
Joe *VecWalk;
RasterAnimHostProperties Prop;
char CurrentState;

if (Enabled && InitialStates)
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS;
	Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;
	CurItem = Items;
	ItemCt = 0;
	while (CurItem)
		{
		if (CurItem->Me)
			{
			CurItem->Me->GetRAHostProperties(&Prop);
			CurrentState = Prop.Flags & WCS_RAHOST_FLAGBIT_ENABLED ? 1: 0;
			if (InitialStates[ItemCt] != CurrentState)
				{
				Prop.Flags = InitialStates[ItemCt] ? WCS_RAHOST_FLAGBIT_ENABLED: 0;
				CurItem->Me->SetRAHostProperties(&Prop);
				} // if
			} // if
		ItemCt ++;
		CurItem = (RasterAnimHostBooleanList *)CurItem->Next;
		} // while

	// set states of search query items
	if (Search)
		{
		DBHost->ResetGeoClip();

		for (VecWalk = DBHost->GetFirst(); VecWalk; VecWalk = DBHost->GetNext(VecWalk))
			{
			if (! VecWalk->TestFlags(WCS_JOEFLAG_HASKIDS))
				{
				if (Search->ApproveJoe(VecWalk))
					{
					VecWalk->GetRAHostProperties(&Prop);
					CurrentState = Prop.Flags & WCS_RAHOST_FLAGBIT_ENABLED ? 1: 0;
					if (InitialStates[ItemCt] != CurrentState)
						{
						Prop.Flags = InitialStates[ItemCt] ? WCS_RAHOST_FLAGBIT_ENABLED: 0;
						VecWalk->SetRAHostProperties(&Prop);
						} // if
					ItemCt ++;
					} // if
				} // if
			} // for
		} // if
	AppMem_Free(InitialStates, NumItems * sizeof (char));
	InitialStates = NULL;
	NumItems = 0;
	} // if

} // RenderScenario::CleanupFromRender

/*===========================================================================*/

int RenderScenario::ProcessFrameToRender(double FrameTime, int &RegenShadowMaps, Database *DBHost)
{
long StateChanged = 0;
RasterAnimHostBooleanList *CurItem = Items;
Joe *VecWalk;
double RenderTime;
RasterAnimHostProperties Prop;
char RenderState, CurrentState, TempState;

if (Enabled)
	{
	RenderTime = FrameTime;
	RenderState = MasterControl.GetValue(0, RenderTime) > .5 ? 1: 0;
	Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS;
	Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;
	CurItem = Items;
	while (CurItem)
		{
		if (CurItem->Me)
			{
			TempState = CurItem->TrueFalse ? ! RenderState: RenderState;
			CurItem->Me->GetRAHostProperties(&Prop);
			CurrentState = Prop.Flags & WCS_RAHOST_FLAGBIT_ENABLED ? 1: 0;
			if (CurrentState != TempState)
				{
				Prop.Flags = TempState ? WCS_RAHOST_FLAGBIT_ENABLED: 0;
				CurItem->Me->SetRAHostProperties(&Prop);
				StateChanged = 1;
				} // if
			} // if
		CurItem = (RasterAnimHostBooleanList *)CurItem->Next;
		} // while

	// set states of search query items
	if (Search)
		{
		DBHost->ResetGeoClip();

		for (VecWalk = DBHost->GetFirst(); VecWalk; VecWalk = DBHost->GetNext(VecWalk))
			{
			if (! VecWalk->TestFlags(WCS_JOEFLAG_HASKIDS))
				{
				if (Search->ApproveJoe(VecWalk))
					{
					VecWalk->GetRAHostProperties(&Prop);
					CurrentState = Prop.Flags & WCS_RAHOST_FLAGBIT_ENABLED ? 1: 0;
					if (CurrentState != RenderState)
						{
						Prop.Flags = RenderState ? WCS_RAHOST_FLAGBIT_ENABLED: 0;
						VecWalk->SetRAHostProperties(&Prop);
						StateChanged = 1;
						} // if
					} // if
				} // if
			} // for
		} // if
	} // if

RegenShadowMaps = (RegenShadows && StateChanged);

return (StateChanged);

} // RenderScenario::ProcessFrameToRender

/*===========================================================================*/

int RenderScenario::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{

return (GeneralEffect::InitFrameToRender(Lib, Rend));

} // RenderScenario::InitFrameToRender

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int RenderScenario::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
RenderScenario *CurrentScene = NULL;
SearchQuery *CurrentQuery = NULL;
ThematicMap *CurrentTheme = NULL;
CoordSys *CurrentCoordSys = NULL;

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
					if (! strnicmp(ReadBuf, "CoordSys", 8))
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
					else if (! strnicmp(ReadBuf, "Scenario", 8))
						{
						if (CurrentScene = new RenderScenario(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentScene->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if Scenario
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

if (Success == 1 && CurrentScene)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentScene);
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

} // RenderScenario::LoadObject

/*===========================================================================*/

int RenderScenario::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
EffectList *CurEffect, *Queries = NULL, *Themes = NULL, *Coords = NULL;

if (! ffile)
	return (0);

memset(StrBuf, 0, 9);

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

// Scenario
strcpy(StrBuf, "Scenario");
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
			} // if Scenario saved 
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

} // RenderScenario::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::RenderScenario_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
RenderScenario *Current;

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
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new RenderScenario(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (RenderScenario *)FindDuplicateByName(Current->EffectType, Current))
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

} // EffectsLib::RenderScenario_Load()

/*===========================================================================*/

ULONG EffectsLib::RenderScenario_Save(FILE *ffile)
{
RenderScenario *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

Current = Scenario;
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
					} // if scenario saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (RenderScenario *)Current->Next;
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

} // EffectsLib::RenderScenario_Save()

/*===========================================================================*/
/*===========================================================================*/

