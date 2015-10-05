// PostProcess.cpp
// For managing PostProcess Effects
// Built from scratch RasterTerraffectorEffect.cpp on 06/12/97 by Gary R. Huber
// Copyright 1997 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "Conservatory.h"
#include "PostProcEditGUI.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Raster.h"
#include "Requester.h"
#include "Database.h"
#include "Security.h"
#include "PostProcessEvent.h"
#include "Lists.h"
#include "FeatureConfig.h"

PostProcess::PostProcess()
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_POSTPROC;
SetDefaults();

} // PostProcess::PostProcess

/*===========================================================================*/

PostProcess::PostProcess(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_POSTPROC;
SetDefaults();

} // PostProcess::PostProcess

/*===========================================================================*/

PostProcess::PostProcess(RasterAnimHost *RAHost, EffectsLib *Library, PostProcess *Proto)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_POSTPROC;
Prev = Library->LastPostProc;
if (Library->LastPostProc)
	{
	Library->LastPostProc->Next = this;
	Library->LastPostProc = this;
	} // if
else
	{
	Library->PostProc = Library->LastPostProc = this;
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
	strcpy(NameBase, "Post Process");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // PostProcess::PostProcess

/*===========================================================================*/

PostProcess::~PostProcess()
{
PostProcessEvent *NextEvent;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->PPR && GlobalApp->GUIWins->PPR->GetActive() == this)
		{
		delete GlobalApp->GUIWins->PPR;
		GlobalApp->GUIWins->PPR = NULL;
		} // if
	} // if

while (Events)
	{
	NextEvent = Events;
	Events = Events->Next;
	delete NextEvent;
	} // while

} // PostProcess::~PostProcess

/*===========================================================================*/

void PostProcess::SetDefaults(void)
{

BeforeReflection = 0;
Events = NULL;

} // PostProcess::SetDefaults

/*===========================================================================*/

void PostProcess::Copy(PostProcess *CopyTo, PostProcess *CopyFrom)
{
PostProcessEvent *CurrentFrom = CopyFrom->Events, **ToPtr, *NextEvent;

// delete existing events
while (CopyTo->Events)
	{
	NextEvent = CopyTo->Events;
	CopyTo->Events = CopyTo->Events->Next;
	delete NextEvent;
	} // while

ToPtr = &CopyTo->Events;

while (CurrentFrom)
	{
	if (*ToPtr = PostProcessEvent::NewEvent(CopyTo, CurrentFrom->GetType()))
		{
		(*ToPtr)->Copy(*ToPtr, CurrentFrom);
		} // if
	ToPtr = &(*ToPtr)->Next;
	CurrentFrom = CurrentFrom->Next;
	} // while

CopyTo->BeforeReflection = CopyFrom->BeforeReflection;
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // PostProcess::Copy

/*===========================================================================*/

ULONG PostProcess::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
char EventName[WCS_POSTPROC_MAXNAMELENGTH];
union MultiVal MV;
PostProcessEvent *CurEvent, **LoadTo = &Events;

while (Events)
	{
	CurEvent = Events;
	Events = Events->Next;
	delete CurEvent;
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
					default:
						break;
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
					case WCS_EFFECTS_POSTPROC_BEFOREREFLECTION:
						{
						BytesRead = ReadBlock(ffile, (char *)&BeforeReflection, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_POSTPROC_EVENTTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)EventName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_POSTPROC_EVENT:
						{
						if (*LoadTo = PostProcessEvent::NewEvent(this, PostProcessEvent::GetEventTypeFromName(EventName)))
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

if (BeforeReflection && ! CheckBeforeReflectionsLegal())
	BeforeReflection = 0;

return (TotalRead);

} // PostProcess::Load

/*===========================================================================*/

unsigned long int PostProcess::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
char *EventName;
PostProcessEvent *CurEvent;


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
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_POSTPROC_BEFOREREFLECTION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&BeforeReflection)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

CurEvent = Events;
while (CurEvent)
	{
	EventName = PostProcessEvent::PostProcEventNames[CurEvent->GetType()];
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_POSTPROC_EVENTTYPE, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(EventName) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)EventName)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	ItemTag = WCS_EFFECTS_POSTPROC_EVENT + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = CurEvent->Save(ffile))
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
				} // if PostProcessEvent saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	CurEvent = CurEvent->Next;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // PostProcess::Save

/*===========================================================================*/

void PostProcess::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->PPR)
	{
	delete GlobalApp->GUIWins->PPR;
	}
GlobalApp->GUIWins->PPR = new PostProcEditGUI(GlobalApp->AppEffects, this);
if(GlobalApp->GUIWins->PPR)
	{
	GlobalApp->GUIWins->PPR->Open(GlobalApp->MainProj);
	}

} // PostProcess::Edit

/*===========================================================================*/

PostProcessEvent *PostProcess::AddEvent(PostProcessEvent *AddMe)
{
PostProcessEvent **CurEvent = &Events;
NotifyTag Changes[2];

while (*CurEvent)
	{
	CurEvent = &(*CurEvent)->Next;
	} // while
if (*CurEvent = PostProcessEvent::NewEvent(this, AddMe ? AddMe->GetType(): WCS_POSTPROCEVENT_TYPE_GAMMA))
	{
	if (AddMe)
		(*CurEvent)->Copy(*CurEvent, AddMe);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
return (*CurEvent);

} // PostProcess::AddEvent

/*===========================================================================*/

PostProcessEvent *PostProcess::ChangeEventType(PostProcessEvent *ChangeMe, unsigned char NewType)
{
PostProcessEvent **CurEvent = &Events, *NextEvent, *DelEvent, *MyNewEvent;
NotifyTag Changes[2];

while (*CurEvent)
	{
	if (*CurEvent == ChangeMe)
		break;
	CurEvent = &(*CurEvent)->Next;
	} // while
if (*CurEvent)
	{
	if (MyNewEvent = PostProcessEvent::NewEvent(this, NewType))
		{
		if (! BeforeReflection || ! MyNewEvent->InhibitRenderBeforeReflections())
			{
			DelEvent = *CurEvent;
			NextEvent = DelEvent->Next;
			DelEvent->Next = NULL;
			*CurEvent = MyNewEvent;
			MyNewEvent->Next = NextEvent;
			delete DelEvent;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		else
			{
			UserMessageOK("Post Process Event Type", "Selected Post Process Event type is incompatible with the \"Evaluate Before Reflections\" setting.");
			} // else
		} // if
	} // if
return (*CurEvent);

} // PostProcess::ChangeEventType

/*===========================================================================*/

char *PostProcess::GetCritterName(RasterAnimHost *Test)
{

return ("");

} // PostProcess::GetCritterName

/*===========================================================================*/

char *PostProcess::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as an Post Process Texture! Remove anyway?");

} // PostProcess::OKRemoveRaster

/*===========================================================================*/

int PostProcess::RemoveRAHost(RasterAnimHost *RemoveMe)
{
PostProcessEvent *CurEvent = Events, *PrevEvent = NULL;
NotifyTag Changes[2];

while (CurEvent)
	{
	if (CurEvent == (PostProcessEvent *)RemoveMe)
		{
		if (PrevEvent)
			PrevEvent->Next = CurEvent->Next;
		else
			Events = CurEvent->Next;

		delete CurEvent;

		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

		return (1);
		} // if
	PrevEvent = CurEvent;
	CurEvent = CurEvent->Next;
	} // while

return (GeneralEffect::RemoveRAHost(RemoveMe));

} // PostProcess::RemoveRAHost

/*===========================================================================*/

char PostProcess::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_POSTPROCEVENT)
	return (1);

return (0);

} // PostProcess::GetRAHostDropOK

/*===========================================================================*/

int PostProcess::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
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
	if (this != (PostProcess *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (PostProcess *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_POSTPROCEVENT)
	{
	Success = -1;
	sprintf(QueryStr, "Add %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		if (AddEvent((PostProcessEvent *)DropSource->DropSource))
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

} // PostProcess::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long PostProcess::GetRAFlags(unsigned long Mask)
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
if (Mask & WCS_RAHOST_FLAGBIT_EDITNAME)
	{
	if (Events)
		Flags |= WCS_RAHOST_FLAGBIT_CHILDREN;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_POSTPROC | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // PostProcess::GetRAFlags

/*===========================================================================*/

RasterAnimHost *PostProcess::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Found = 0;
PostProcessEvent *CurEvent;

if (! Current)
	Found = 1;
CurEvent = Events;
while (CurEvent)
	{
	if (Found)
		return (CurEvent);
	if (Current == CurEvent)
		Found = 1;
	CurEvent = CurEvent->Next;
	} // while

return (NULL);

} // PostProcess::GetRAHostChild

/*===========================================================================*/

int PostProcess::GetDeletable(RasterAnimHost *Test)
{
char Ct;
PostProcessEvent *CurEvent;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (1);
		} // if
	} // for
CurEvent = Events;
while (CurEvent)
	{
	if (Test == CurEvent)
		return (1);
	CurEvent = CurEvent->Next;
	} // while

return (0);

} // PostProcess::GetDeletable

/*===========================================================================*/

int PostProcess::GetRAHostAnimated(void)
{
PostProcessEvent *Current = Events;

if (GeneralEffect::GetRAHostAnimated())
	return (1);
while (Current)
	{
	if (Current->GetRAHostAnimated())
		return (1);
	Current = Current->Next;
	} // while

return (0);

} // PostProcess::GetRAHostAnimated

/*===========================================================================*/

int PostProcess::SetToTime(double Time)
{
long Found = 0;
PostProcessEvent *Current = Events;

if (GeneralEffect::SetToTime(Time))
	Found = 1;
while (Current)
	{
	if (Current->SetToTime(Time))
		Found = 1;
	Current = Current->Next;
	} // while

return (Found);

} // PostProcess::SetToTime

/*===========================================================================*/

long PostProcess::InitImageIDs(long &ImageID)
{
long NumImages = 0;
PostProcessEvent *CurEvent = Events;

while (CurEvent)
	{
	NumImages += CurEvent->InitImageIDs(ImageID);
	CurEvent = CurEvent->Next;
	} // while
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // PostProcess::InitImageIDs

/*===========================================================================*/

void PostProcess::ResolveLoadLinkages(EffectsLib *Lib)
{
PostProcessEvent *CurEvent = Events;

while (CurEvent)
	{
	CurEvent->ResolveLoadLinkages(Lib);
	CurEvent = CurEvent->Next;
	} // while

GeneralEffect::ResolveLoadLinkages(Lib);

} // PostProcess::ResolveLoadLinkages

/*===========================================================================*/

long PostProcess::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Found = 0;
PostProcessEvent *CurEvent = Events;

if (GeneralEffect::GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
while (CurEvent)
	{
	if (CurEvent->GetKeyFrameRange(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	CurEvent = CurEvent->Next;
	} // while

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

} // PostProcess::GetKeyFrameRange

/*===========================================================================*/

int PostProcess::CheckBeforeReflectionsLegal(void)
{
PostProcessEvent *CurEvent;

CurEvent = Events;
while (CurEvent)
	{
	if (CurEvent->InhibitRenderBeforeReflections())
		{
		return (0);
		} // if
	CurEvent = CurEvent->Next;
	} // while

return (1);

} // PostProcess::CheckBeforeReflectionsLegal

/*===========================================================================*/

int PostProcess::AddRenderBuffers(RenderOpt *Opt, BufferNode *Buffers)
{
PostProcessEvent *CurEvent;

CurEvent = Events;
while (CurEvent)
	{
	if (CurEvent->Enabled)
		{
		if (! CurEvent->AddRenderBuffers(Opt, Buffers))
			return (0);
		} // if
	CurEvent = CurEvent->Next;
	} // while

return (1);

} // PostProcess::AddRenderBuffers

/*===========================================================================*/

int PostProcess::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{
PostProcessEvent *CurEvent;

// add buffers if necessary to perform requested filter operation
if (! GeneralEffect::InitToRender(Opt, Buffers))
	return (0);
CurEvent = Events;
while (CurEvent)
	{
	if (CurEvent->Enabled)
		{
		if (! CurEvent->InitToRender(Opt, Buffers))
			return (0);
		} // if
	CurEvent = CurEvent->Next;
	} // while

return (1);

} // PostProcess::InitToRender

/*===========================================================================*/

int PostProcess::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{
PostProcessEvent *CurEvent;

CurEvent = Events;
while (CurEvent)
	{
	if (CurEvent->Enabled)
		{
		if (! CurEvent->InitFrameToRender(Lib, Rend))
			return (0);
		} // if
	CurEvent = CurEvent->Next;
	} // while

return (GeneralEffect::InitFrameToRender(Lib, Rend));

} // PostProcess::InitFrameToRender

/*===========================================================================*/

int PostProcess::RenderPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, long FrameNum, int UpdateDiagnostics)
{
PostProcessEvent *CurEvent = Events;

InitFrameToRender(GlobalApp->AppEffects, Rend);

while (CurEvent)
	{
	if (CurEvent->Enabled)
		{
		if (! CurEvent->RenderPostProc(Rend, Buffers, FragBlock, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics))
			return (0);
		} // if
	CurEvent = CurEvent->Next;
	} // while

return (1);

} // PostProcess::RenderPostProc

/*===========================================================================*/

int PostProcess::BuildFileComponentsList(EffectList **Coords)
{
PostProcessEvent *CurEvent = Events;

while (CurEvent)
	{
	if (! CurEvent->BuildFileComponentsList(Coords))
		return (0);
	CurEvent = CurEvent->Next;
	} // while

return (1);

} // PostProcess::BuildFileComponentsList

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int PostProcess::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
PostProcess *CurrentProc = NULL;
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
					else if (! strnicmp(ReadBuf, "PostProc", 8))
						{
						if (CurrentProc = new PostProcess(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentProc->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if post process
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

if (Success == 1 && CurrentProc)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentProc);
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

} // PostProcess::LoadObject

/*===========================================================================*/

int PostProcess::SaveObject(FILE *ffile, const char *SuppliedFileName)
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

// PostProcess
strcpy(StrBuf, "PostProc");
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
			} // if PostProcess saved 
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

} // PostProcess::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::PostProcess_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
PostProcess *Current;

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
						if (Current = new PostProcess(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (PostProcess *)FindDuplicateByName(Current->EffectType, Current))
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

} // EffectsLib::PostProcess_Load()

/*===========================================================================*/

ULONG EffectsLib::PostProcess_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
PostProcess *Current;

Current = PostProc;
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
					} // if PostProcess effect saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (PostProcess *)Current->Next;
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

} // EffectsLib::PostProcess_Save()

/*===========================================================================*/
