// RenderJob.cpp
// For managing WCS RenderJobs
// Built from scratch on 03/24/99 by Gary R. Huber
// Copyright 1999 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Project.h"
#include "Interactive.h"
#include "Application.h"
#include "Conservatory.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "GraphData.h"
#include "requester.h"
#include "RenderJobEditGUI.h"
#include "Database.h"
#include "Raster.h"
#include "Lists.h"

/*===========================================================================*/

RenderJob::RenderJob()
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_RENDERJOB;
SetDefaults();

} // RenderJob::RenderJob

/*===========================================================================*/

RenderJob::RenderJob(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_RENDERJOB;
SetDefaults();

} // RenderJob::RenderJob

/*===========================================================================*/

RenderJob::RenderJob(RasterAnimHost *RAHost, EffectsLib *Library, RenderJob *Proto)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_RENDERJOB;
Prev = Library->LastRenderJob;
if (Library->LastRenderJob)
	{
	Library->LastRenderJob->Next = this;
	Library->LastRenderJob = this;
	} // if
else
	{
	Library->RenderJobs = Library->LastRenderJob = this;
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
	strcpy(NameBase, "Render Job");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // RenderJob::RenderJob

/*===========================================================================*/

RenderJob::~RenderJob()
{
EffectList *NextScenario;
NameList *NextName;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->RJG && GlobalApp->GUIWins->RJG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->RJG;
		GlobalApp->GUIWins->RJG = NULL;
		} // if
	} // if

while (Scenarios)
	{
	NextScenario = Scenarios->Next;
	delete Scenarios;
	Scenarios = NextScenario;
	} // while

while (ScenarioNames)
	{
	NextName = ScenarioNames->Next;
	delete ScenarioNames;
	ScenarioNames = NextName;
	} // if

} // RenderJob::~RenderJob

/*===========================================================================*/

void RenderJob::SetDefaults(void)
{

Cam = NULL;
Options = NULL;
Scenarios = NULL;
ScenarioNames = NULL;

} // RenderJob::SetDefaults

/*===========================================================================*/

void RenderJob::Copy(RenderJob *CopyTo, RenderJob *CopyFrom)
{
#ifdef WCS_RENDER_SCENARIOS
long Result = -1;
EffectList *NextScenario, **ToScenario;
NotifyTag Changes[2];

while (CopyTo->Scenarios)
	{
	NextScenario = CopyTo->Scenarios->Next;
	CopyTo->Scenarios = NextScenario;
	delete CopyTo->Scenarios;
	} // if
NextScenario = CopyFrom->Scenarios;
ToScenario = &CopyTo->Scenarios;
while (NextScenario)
	{
	if (NextScenario->Me)
		{
		if (*ToScenario = new EffectList())
			{
			if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib || GlobalApp->CopyToEffectsLib != GlobalApp->AppEffects)
				{
				(*ToScenario)->Me = GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(NextScenario->Me);
				} // if no need to make another copy, its all in the family
			else
				{
				if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(NextScenario->Me->EffectType, NextScenario->Me->Name))
					{
					Result = UserMessageCustom("Copy Render Job", "How do you wish to resolve Render Scenario name collisions?\n\nLink to existing Render Scenarios, replace existing Render Scenarios, or create new Render Scenarios?",
						"Link", "Create", "Overwrite", 1);
					} // if
				if (Result <= 0)
					{
					(*ToScenario)->Me = GlobalApp->CopyToEffectsLib->AddEffect(NextScenario->Me->EffectType, NULL, NextScenario->Me);
					} // if create new
				else if (Result == 1)
					{
					(*ToScenario)->Me = GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(NextScenario->Me);
					} // if link to existing
				else if ((*ToScenario)->Me = GlobalApp->CopyToEffectsLib->FindByName(NextScenario->Me->EffectType, NextScenario->Me->Name))
					{
					((RenderScenario *)(*ToScenario)->Me)->Copy((RenderScenario *)(*ToScenario)->Me, (RenderScenario *)NextScenario->Me);
					Changes[0] = MAKE_ID((*ToScenario)->Me->GetNotifyClass(), (*ToScenario)->Me->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, (*ToScenario)->Me);
					} // else if found and overwrite
				else
					{
					(*ToScenario)->Me = GlobalApp->CopyToEffectsLib->AddEffect(NextScenario->Me->EffectType, NULL, NextScenario->Me);
					} // else
				} // else better copy or overwrite it since its important to get just the right ecosystem
			if ((*ToScenario)->Me)
				ToScenario = &(*ToScenario)->Next;
			else
				{
				delete *ToScenario;
				*ToScenario = NULL;
				} // if
			} // if
		} // if
	NextScenario = NextScenario->Next;
	} // while
#endif // WCS_RENDER_SCENARIOS

CopyTo->Cam = NULL;
CopyTo->Options = NULL;
if (CopyFrom->Cam)
	{
	CopyTo->Cam = (Camera *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->Cam);
	} // if
if (CopyFrom->Options)
	{
	CopyTo->Options = (RenderOpt *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->Options);
	} // if
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // RenderJob::Copy

/*===========================================================================*/

int RenderJob::SetCamera(Camera *NewCamera)
{
NotifyTag Changes[2];

Cam = NewCamera;
Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (1);

} // RenderJob::SetCamera

/*===========================================================================*/

int RenderJob::SetRenderOpt(RenderOpt *NewRenderOpt)
{
NotifyTag Changes[2];

Options = NewRenderOpt;
Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (1);

} // RenderJob::SetRenderOpt

/*===========================================================================*/

ULONG RenderJob::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char TempName[256];
EffectList **CurScenario = &Scenarios;
NameList **CurName = &ScenarioNames;

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
					case WCS_EFFECTS_RENDERJOB_CAMERANAME:
						{
						BytesRead = ReadBlock(ffile, (char *)TempName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						Cam = (Camera *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_CAMERA, TempName);
						break;
						}
					case WCS_EFFECTS_RENDERJOB_RENDEROPTNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)TempName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						Options = (RenderOpt *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_RENDEROPT, TempName);
						break;
						}
					#ifdef WCS_RENDER_SCENARIOS
					case WCS_EFFECTS_RENDERJOB_SCENARIONAME:
						{
						BytesRead = ReadBlock(ffile, (char *)TempName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempName[0])
							{
							if (*CurScenario = new EffectList())
								{
								if ((*CurScenario)->Me = GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_SCENARIO, TempName))
									CurScenario = &(*CurScenario)->Next;
								else
									{
									delete *CurScenario;
									*CurScenario = NULL;
									if (*CurName = new NameList(TempName, WCS_EFFECTSSUBCLASS_SCENARIO))
										{
										CurName = &(*CurName)->Next;
										} // if
									} // else
								} // if
							} // if
						break;
						}
					#endif // WCS_RENDER_SCENARIOS
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

} // RenderJob::Load

/*===========================================================================*/

unsigned long int RenderJob::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
#ifdef WCS_RENDER_SCENARIOS
EffectList *CurScenario;
#endif // WCS_RENDER_SCENARIOS

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

if (Cam)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDERJOB_CAMERANAME, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Cam->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Cam->GetName())) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

if (Options)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDERJOB_RENDEROPTNAME, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Options->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Options->GetName())) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

#ifdef WCS_RENDER_SCENARIOS
CurScenario = Scenarios;
while (CurScenario)
	{
	if (CurScenario->Me)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDERJOB_SCENARIONAME, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(CurScenario->Me->GetName()) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)CurScenario->Me->GetName())) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	CurScenario = CurScenario->Next;
	} // while
#endif // WCS_RENDER_SCENARIOS

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // RenderJob::Save

/*===========================================================================*/

void RenderJob::ResolveLoadLinkages(EffectsLib *Lib)
{
#ifdef WCS_RENDER_SCENARIOS
NameList *CurScenario = ScenarioNames, *NextName;
EffectList **CurItem = &Scenarios;

while (*CurItem)
	{
	CurItem = &(*CurItem)->Next;
	} // while

while (CurScenario)
	{
	if (*CurItem = new EffectList())
		{
		if ((*CurItem)->Me = Lib->FindByName(CurScenario->ItemClass, CurScenario->Name))
			CurItem = &(*CurItem)->Next;
		else
			{
			delete *CurItem;
			*CurItem = NULL;
			} // else
		} // if
	CurScenario = CurScenario->Next;
	} // if

CurScenario = ScenarioNames;
while (CurScenario)
	{
	NextName = CurScenario->Next;
	delete CurScenario;
	CurScenario = NextName;
	} // if
ScenarioNames = NULL;
#endif // WCS_RENDER_SCENARIOS

} // RenderJob::ResolveLoadLinkages

/*===========================================================================*/

void RenderJob::Edit(void)
{

if(GlobalApp->GUIWins->RJG)
	{
	delete GlobalApp->GUIWins->RJG;
	}
GlobalApp->GUIWins->RJG = new RenderJobEditGUI(GlobalApp->AppEffects, this);
if(GlobalApp->GUIWins->RJG)
	{
	GlobalApp->GUIWins->RJG->Open(GlobalApp->MainProj);
	}

} // RenderJob::Edit

/*===========================================================================*/

int RenderJob::RemoveRAHost(RasterAnimHost *RemoveMe)
{
EffectList *CurScenario = Scenarios, *PrevScenario = NULL;
NotifyTag Changes[2];
int Removed = 0;

if (Cam == (Camera *)RemoveMe)
	{
	Cam = NULL;
	Removed = 1;
	} // if
if (Options == (RenderOpt *)RemoveMe)
	{
	Options = NULL;
	Removed = 1;
	} // if

#ifdef WCS_RENDER_SCENARIOS
if (! Removed)
	{
	while (CurScenario)
		{
		if (CurScenario->Me == (GeneralEffect *)RemoveMe)
			{
			if (PrevScenario)
				PrevScenario->Next = CurScenario->Next;
			else
				Scenarios = CurScenario->Next;
			delete CurScenario;
			Removed = 1;
			break;
			} // if
		PrevScenario = CurScenario;
		CurScenario = CurScenario->Next;
		} // while
	} // if
#endif // WCS_RENDER_SCENARIOS

if (Removed)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

return (Removed);

} // RenderJob::RemoveRAHost

/*===========================================================================*/

EffectList *RenderJob::AddScenario(GeneralEffect *AddMe)
{
#ifdef WCS_RENDER_SCENARIOS
EffectList **CurScenario = &Scenarios;
NotifyTag Changes[2];

if (AddMe)
	{
	while (*CurScenario)
		{
		if ((*CurScenario)->Me == AddMe)
			return (NULL);
		CurScenario = &(*CurScenario)->Next;
		} // while
	if (*CurScenario = new EffectList())
		{
		(*CurScenario)->Me = AddMe;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	return (*CurScenario);
	} // if
#endif // WCS_RENDER_SCENARIOS

return (NULL);

} // RenderJob::AddScenario

/*===========================================================================*/

long RenderJob::InitImageIDs(long &ImageID)
{
long NumImages = 0;
EffectList *CurScenario = Scenarios;

if (Cam)
	NumImages += Cam->InitImageIDs(ImageID);
if (Options)
	NumImages += Options->InitImageIDs(ImageID);

#ifdef WCS_RENDER_SCENARIOS
while (CurScenario)
	{
	if (CurScenario->Me)
		NumImages += CurScenario->Me->InitImageIDs(ImageID);
	CurScenario = CurScenario->Next;
	} // while
NumImages += GeneralEffect::InitImageIDs(ImageID);
#endif // WCS_RENDER_SCENARIOS

return (NumImages);

} // RenderJob::InitImageIDs

/*===========================================================================*/

int RenderJob::BuildFileComponentsList(EffectList **Cameras, EffectList **RenderOpts, EffectList **ScenarioList)
{
EffectList **ListPtr;
#ifdef WCS_RENDER_SCENARIOS
EffectList *CurScenario = Scenarios;
#endif // WCS_RENDER_SCENARIOS

if (Cam)
	{
	ListPtr = Cameras;
	while (*ListPtr)
		{
		if ((*ListPtr)->Me == Cam)
			break;
		ListPtr = &(*ListPtr)->Next;
		} // if
	if (! (*ListPtr))
		{
		if (*ListPtr = new EffectList())
			(*ListPtr)->Me = Cam;
		else
			return (0);
		} // if
	} // if
if (Options)
	{
	ListPtr = RenderOpts;
	while (*ListPtr)
		{
		if ((*ListPtr)->Me == Options)
			break;
		ListPtr = &(*ListPtr)->Next;
		} // if
	if (! (*ListPtr))
		{
		if (*ListPtr = new EffectList())
			(*ListPtr)->Me = Options;
		else
			return (0);
		} // if
	} // if

#ifdef WCS_RENDER_SCENARIOS
while (CurScenario)
	{
	if (CurScenario->Me)
		{
		ListPtr = ScenarioList;
		while (*ListPtr)
			{
			if ((*ListPtr)->Me == CurScenario->Me)
				break;
			ListPtr = &(*ListPtr)->Next;
			} // if
		if (! (*ListPtr))
			{
			if (*ListPtr = new EffectList())
				(*ListPtr)->Me = CurScenario->Me;
			else
				return (0);
			} // if
		} // if
	CurScenario = CurScenario->Next;
	} // while
#endif // WCS_RENDER_SCENARIOS

return (1);

} // RenderJob::BuildFileComponentsList

/*===========================================================================*/

char RenderJob::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_EFFECTSSUBCLASS_CAMERA
	|| DropType == WCS_EFFECTSSUBCLASS_RENDEROPT
	#ifdef WCS_RENDER_SCENARIOS
	|| DropType == WCS_EFFECTSSUBCLASS_SCENARIO
	#endif // WCS_RENDER_SCENARIOS
	)
	return (1);

return (0);

} // RenderJob::GetRAHostDropOK

/*===========================================================================*/

int RenderJob::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
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
	if (this != (RenderJob *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (RenderJob *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_CAMERA)
	{
	Success = -1;
	sprintf(QueryStr, "Apply %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		Cam = (Camera *)DropSource->DropSource;
		Success = 1;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // else if
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_RENDEROPT)
	{
	Success = -1;
	sprintf(QueryStr, "Apply %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		Options = (RenderOpt *)DropSource->DropSource;
		Success = 1;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // else if
#ifdef WCS_RENDER_SCENARIOS
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_SCENARIO)
	{
	Success = -1;
	sprintf(QueryStr, "Add %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		if (AddScenario((GeneralEffect *)DropSource->DropSource))
			{
			Success = 1;
			} // if
		} // if
	} // else if
#endif // WCS_RENDER_SCENARIOS
else
	{
	Success = GeneralEffect::ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // RenderJob::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long RenderJob::GetRAFlags(unsigned long Mask)
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
	if (Cam || Options || Scenarios)
		Flags |= WCS_RAHOST_FLAGBIT_CHILDREN;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_RENDJOB | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // RenderJob::GetRAFlags

/*===========================================================================*/

RasterAnimHost *RenderJob::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Found = 0;
#ifdef WCS_RENDER_SCENARIOS
EffectList *CurScenario;
#endif // WCS_RENDER_SCENARIOS

if (! Current)
	Found = 1;
if (Found && Cam)
	return (Cam);
if (Current == Cam)
	Found = 1;
if (Found && Options)
	return (Options);
if (Current == Options)
	Found = 1;
#ifdef WCS_RENDER_SCENARIOS
CurScenario = Scenarios;
while (CurScenario)
	{
	if (Found)
		return (CurScenario->Me);
	if (Current == CurScenario->Me)
		Found = 1;
	CurScenario = CurScenario->Next;
	} // while
#endif // WCS_RENDER_SCENARIOS

return (NULL);

} // RenderJob::GetRAHostChild

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int RenderJob::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG ItemTag = 0, Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
RenderJob *CurrentRenderJob = NULL;
Camera *CurrentCamera = NULL;
RenderOpt *CurrentRenderOpt = NULL;
RenderScenario *CurrentScenario = NULL;
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
					else if (! strnicmp(ReadBuf, "Camera", 8))
						{
						if (CurrentCamera = new Camera(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentCamera->Load(ffile, Size, ByteFlip);
							}
						} // if Camera
					else if (! strnicmp(ReadBuf, "RendrOpt", 8))
						{
						if (CurrentRenderOpt = new RenderOpt(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentRenderOpt->Load(ffile, Size, ByteFlip);
							}
						} // if RenderOpt
					#ifdef WCS_RENDER_SCENARIOS
					else if (! strnicmp(ReadBuf, "Scenario", 8))
						{
						if (CurrentScenario = new RenderScenario(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentScenario->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;
							}
						} // if Scenario
					#endif // WCS_RENDER_SCENARIOS
					else if (! strnicmp(ReadBuf, "RendrJob", 8))
						{
						if (CurrentRenderJob = new RenderJob(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentRenderJob->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if RenderJob
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

if (Success == 1 && CurrentRenderJob)
	{
	if (OldBoundsLoaded && GlobalApp->AppDB->FillDEMBounds(&CurBounds))
		{
		if (UserMessageYN("Load Render Job", "Do you wish the loaded Render Job's Camera position\n to be scaled to current DEM bounds?"))
			{
			CurrentRenderJob->ScaleToDEMBounds(&OldBounds, &CurBounds);
			} // if
		} // if
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentRenderJob);
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

} // RenderJob::LoadObject

/*===========================================================================*/

int RenderJob::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
EffectList *CurEffect, *CamList = NULL, *OptList = NULL, *ScenarioList = NULL;
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

if (BuildFileComponentsList(&CamList, &OptList, &ScenarioList))
	{
	CurEffect = CamList;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "Camera");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((Camera *)CurEffect->Me)->Save(ffile))
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
						} // if Camera saved 
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

	CurEffect = OptList;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "RendrOpt");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((RenderOpt *)CurEffect->Me)->Save(ffile))
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
						} // if RenderOpt saved 
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

	#ifdef WCS_RENDER_SCENARIOS
	CurEffect = ScenarioList;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
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

					if (BytesWritten = ((RenderScenario *)CurEffect->Me)->Save(ffile))
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
			} // if
		CurEffect = CurEffect->Next;
		} // while
	#endif // WCS_RENDER_SCENARIOS

	while (CamList)
		{
		CurEffect = CamList;
		CamList = CamList->Next;
		delete CurEffect;
		} // while
	while (OptList)
		{
		CurEffect = OptList;
		OptList = OptList->Next;
		delete CurEffect;
		} // while
	while (ScenarioList)
		{
		CurEffect = ScenarioList;
		ScenarioList = ScenarioList->Next;
		delete CurEffect;
		} // while
	} // if

// RenderJob
strcpy(StrBuf, "RendrJob");
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
			} // if RenderJob saved 
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

} // RenderJob::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::RenderJob_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
RenderJob *Current;
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
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new RenderJob(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (RenderJob *)FindDuplicateByName(Current->EffectType, Current))
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

} // EffectsLib::RenderJob_Load()

/*===========================================================================*/

ULONG EffectsLib::RenderJob_Save(FILE *ffile)
{
RenderJob *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

Current = RenderJobs;
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
					} // if RenderJob saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (RenderJob *)Current->Next;
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

} // EffectsLib::RenderJob_Save()

/*===========================================================================*/
