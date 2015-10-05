// Layers.cpp
// 

#include "stdafx.h"
#include "AppMem.h"
#include "Layers.h"
#include "Joe.h"
#include "Notify.h"
#include "Useful.h"
#include "Application.h"
#include "EffectsLib.h"
#include "Requester.h"

// LayerStub stuff
void LayerStub::SetIEEEAttribVal(double NewIEEEVal)
{
if(ThisObjectsLayer)
	{
	if(ThisObjectsLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
		{
		if(!ThisObjectsLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE))
			{
			LAttrib.IEEEVal = NewIEEEVal;
			} // if
		} // if
	} // if
} // LayerStub::SetIEEEAttribVal

double LayerStub::GetIEEEAttribVal(void)
{
if(ThisObjectsLayer)
	{
	if(ThisObjectsLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
		{
		if(!ThisObjectsLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE))
			{
			return(LAttrib.IEEEVal);
			} // if
		} // if
	} // if
return(0.0);
} // LayerStub::GetIEEEAttribVal

void LayerStub::SetTextAttribVal(const char *NewTextVal)
{
if(ThisObjectsLayer)
	{
	if(ThisObjectsLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
		{
		if(ThisObjectsLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE))
			{
			// Allocate here...
			if(LAttrib.TextVal.TextBuf = (char *)AppMem_Alloc(strlen(NewTextVal) + 1, NULL))
				{
				strcpy(LAttrib.TextVal.TextBuf, NewTextVal);
				LAttrib.TextVal.TextBufLen = (unsigned long)(strlen(NewTextVal) + 1);
				} // if
			else
				{
				LAttrib.TextVal.TextBufLen = 0;
				} // else
			} // if
		} // if
	} // if

} // LayerStub::SetTextAttribVal

char *LayerStub::GetTextAttribVal(void)
{
if(ThisObjectsLayer)
	{
	if(ThisObjectsLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
		{
		if(ThisObjectsLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE))
			{
			return(LAttrib.TextVal.TextBuf);
			} // if
		} // if
	} // if
return(NULL);
} // LayerStub::GetTextAttribVal



LayerStub::~LayerStub()
{
// Unlink all of the pointers to and from other LayerStubs,
// and to other objects. Other code will have to handle clearing
// references TO this object from other classes.

// First, free any dynamically-allocated memory for an attribute

if(ThisObjectsLayer)
	{
	if(ThisObjectsLayer->TestFlags(WCS_LAYER_ISATTRIBUTE))
		{
		if(ThisObjectsLayer->TestFlags(WCS_LAYER_TEXTATTRIBUTE) && LAttrib.TextVal.TextBuf && LAttrib.TextVal.TextBufLen)
			{
			AppMem_Free(LAttrib.TextVal.TextBuf, LAttrib.TextVal.TextBufLen);
			LAttrib.TextVal.TextBuf = NULL;
			LAttrib.TextVal.TextBufLen = 0;
			} // if
		} // if
	} // if

// These are actually redundant, the memory is about to go away.
// ThisObjectsLayer = NULL;

// Don't worry about NextStubSameObject, the Joe class will have done that
//if(ObjectReferredTo)
//      {
//      ObjectReferredTo->RemoveLayerStub(this);
//      } // if

if(ThisObjectsLayer)
	{
	if(ThisObjectsLayer->ChainStart == this)
		{
		ThisObjectsLayer->ChainStart = NextObjectStubSameLayer;
		} // if
	} // if

if(NextObjectStubSameLayer)
        {
        NextObjectStubSameLayer->PrevObjectStubSameLayer = PrevObjectStubSameLayer;
        } // if
if(PrevObjectStubSameLayer)
        {
        PrevObjectStubSameLayer->NextObjectStubSameLayer = NextObjectStubSameLayer;
        } // if
// if there is no PrevObj entry, it means we're the first in the list.

} // ~LayerStub



// LayerEntry Stuff
// (All LayerEntry methods are inline...)


// LayerTable Stuff


LayerTable::LayerTable()
{
InitClear();
} // LayerTable::LayerTable

void LayerTable::InitClear(void)
{
for(NumLayers = 0; NumLayers < 256; NumLayers++)
        {
        LayerHash[NumLayers] = NULL;
        } // for
NumLayers = MaxLayer = 0; LayerList = NULL;
} // LayerTable::InitClear

void LayerTable::DestroyAll(void)
{
LayerEntry *ToDieFor, *MarkedForDeath = NULL;

for(ToDieFor = FirstEntry(); ToDieFor; ToDieFor = MarkedForDeath)
        {
        MarkedForDeath = NextEntry(ToDieFor);
        delete ToDieFor;
        } // for

InitClear();
} // LayerTable::DestroyAll

LayerEntry *LayerTable::NewLayer(char *Name, unsigned int Pri)
{
LayerEntry *Me;
unsigned char HashVal;
Me = new LayerEntry;
if(Me)
        {
        if(Me->SetName(Name))
                {
				if((Name[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_DBL) || (Name[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_TXT))
					{
					Me->SetFlags(WCS_LAYER_ISATTRIBUTE);
					if(Name[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_DBL)
						{
						Me->ClearFlags(WCS_LAYER_TEXTATTRIBUTE);
						} // if
					if(Name[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_TXT)
						{
						Me->SetFlags(WCS_LAYER_TEXTATTRIBUTE);
						} // if
					} // if
				#ifdef WCS_BUILD_V3
				// new tail-append code
				if(LayerList)
					{
					for(LayerEntry *LayerAppend = LayerList; LayerAppend; LayerAppend = LayerAppend->NextLayer)
						{
						if(LayerAppend->NextLayer == NULL)
							{
							// this is the end of the line, append here
							Me->PrevLayer = LayerAppend;
							Me->NextLayer = NULL;
							LayerAppend->NextLayer = Me;
							break;
							} // if
						} // for
					} // if
				else
					{ // doesn't matter if we insert or append, there's nothing there yet
					Me->PrevLayer = NULL;
					if(Me->NextLayer = LayerList)
						{
						Me->NextLayer->PrevLayer = Me;
						} // if
					LayerList = Me;
					} // else
				#else // !WCS_BUILD_V3, must be VNS 2 or 1
				// old head-insert code
                Me->PrevLayer = NULL;
                if(Me->NextLayer = LayerList)
					{
					Me->NextLayer->PrevLayer = Me;
					} // if
                LayerList = Me;
				#endif // !WCS_BUILD_V3
                HashVal = MakeHash(Name);
//                if(LayerHash[HashVal] == NULL)
//                        {
                        Me->NextHash = LayerHash[HashVal];
//                        } // if
                LayerHash[HashVal] = Me;
                NumLayers++;
                return(Me);
                } // if
        else
                {
                delete(Me);
                return(NULL);
                } // else
        } // if
else
        {
        return(NULL);
        } // else

} //LayerTable::NewLayer

int LayerTable::RemoveLayer(LayerEntry *Me)
{
unsigned char MyHash;
NotifyTag Changes[2];

if (Me)
        {
        if (Me->FirstStub())
                {
                return(0);
                } // if
        MyHash = MakeHash(Me->LayerName.StrData);
        if (LayerHash[MyHash] == Me)
                {
                LayerHash[MyHash] = Me->NextHash;
                } // if
        else
                {
                if (! RemoveLayerFromHash(Me, MyHash))
                        {
                        return(0);
                        } // else
                } // else

        if (Me->NextLayer)
                {
                Me->NextLayer->PrevLayer = Me->PrevLayer;
                } // if
        if (Me->PrevLayer)
                {
                Me->PrevLayer->NextLayer = Me->NextLayer;
                } // if
		if (LayerList == Me)
				{
				LayerList = Me->NextLayer;
				} // if
        NumLayers--;

		Changes[0] = MAKE_ID(0xff, Me->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, NULL);
        return(1); // success
        } // if

return(0); // failure

} // LayerTable::RemoveLayer


void LayerTable::RemoveUnusedLayers(void)
{
LayerEntry *CurEntry, *StashedNextEntry;
NotifyTag Changes[2];

for (CurEntry = FirstEntry(); CurEntry; CurEntry = StashedNextEntry)
	{
	StashedNextEntry = NextEntry(CurEntry);
	if (RemoveLayer(CurEntry))	// fails if layer has LayerStubs
		{
		Changes[0] = MAKE_ID(0xff, CurEntry->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
		Changes[1] = NULL;
		delete CurEntry;
		GlobalApp->AppEx->GenerateNotify(Changes, NULL);
		} // if
	} // for

} // LayerTable::RemoveUnusedLayers


unsigned char LayerTable::MakeHash(char *Name)
{
unsigned short int StrIndex, HashSixteen;

if(Name)
        {
        for(StrIndex = HashSixteen = 0; Name[StrIndex]; StrIndex++)
                {
                HashSixteen = (HashSixteen + ((StrIndex + 1) * Name[StrIndex])) & 0xffff;
                } // for
        return((unsigned char)(HashSixteen % 256));
        } // if

return(0);

} // LayerTable::MakeHash

LayerEntry *LayerTable::RenameLayer(LayerEntry *Me, char *NewName)
{
unsigned char OldHash, NewHash;

if(Me)
	{
    OldHash = MakeHash(Me->LayerName.StrData);
    NewHash = MakeHash(NewName);
    if(OldHash != NewHash)
		{
		// Need to move around in hash table
		if(RemoveLayerFromHash(Me, OldHash))
			{
			Me->NextHash = LayerHash[NewHash];
			LayerHash[NewHash] = Me;
			} // if
		else
			{
			return(NULL);
			} // else
		} //if
	if(Me->LayerName.Set(NewName))
		{
		return(Me);
		} // if
	else
		{
		// failed to rename, put things sane
		// This will result in an unnamed layer in the correct hash spot
		LayerHash[NewHash] = Me->NextHash;
		NewHash = MakeHash(Me->LayerName.StrData);
		Me->NextHash = LayerHash[NewHash];
		LayerHash[NewHash] = Me;
		return(NULL);
		} // if

	} // if

return(NULL);

} // LayerTable::RenameLayer


int LayerTable::RemoveLayerFromHash(LayerEntry *Me, unsigned char Hash)
{
LayerEntry *SeekHash;

for(SeekHash = LayerHash[Hash]; SeekHash; SeekHash = SeekHash->NextHash)
	{
	if(SeekHash->NextHash == Me)
		{
		SeekHash->NextHash = Me->NextHash;
		return(1); // success
		} // if
	} // for

return(0); // failure

} // LayerTable::RemoveLayerFromHash

LayerEntry *LayerTable::MatchLayer(char *MatchName)
{
LayerEntry *TestMatch;
unsigned char MatchHash;

if(MatchName)
        {
        MatchHash = MakeHash(MatchName);
        for(TestMatch = LayerHash[MatchHash]; TestMatch;
         TestMatch = TestMatch->NextHash)
                {
                if(strcmp(TestMatch->LayerName.StrData, MatchName) == 0)
                        {
                        return(TestMatch);
                        } // if
                } // for
        return(NULL); // no match
        } // if
else
        {
        return(NULL);
        } // else
} // LayerTable::MatchLayer

LayerEntry *LayerTable::MatchMakeLayer(char *MatchName, unsigned int Pri)
{
LayerEntry *Me;
NotifyTag Changes[2];

if(Me = MatchLayer(MatchName))
        {
        return(Me);
        } // if
else
        {
        if (Me = NewLayer(MatchName, Pri))
			{
			Changes[0] = MAKE_ID(0xff, Me->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, NULL);
			} // if
		return (Me);
        } // else
} // LayerTable::MatchMakeLayer

LayerStub *LayerTable::AddObjectToLayer(Joe *Me, char *DestLayerName)
{
LayerEntry *MyLayer;

if(DestLayerName)
        {
        MyLayer = MatchMakeLayer(DestLayerName, 0);
        if(MyLayer)
                {
                return(Me->AddObjectToLayer(MyLayer));
                } // if
        else
                {
                return(NULL);
                } // else
        } // if
else
        {
        return(NULL);
        } // else
} // LayerTable:AddObjectToLayer(char *)

unsigned long int LayerTable::WriteLayerHeader(FILE *SaveFile)
{
LayerEntry *LayerSkim;
unsigned char LNameSize, Success;
unsigned long int Gen32, Out32, SizeHolder, BackFuture;

if(SaveFile && NumLayers) // No need if no layers
        {
        if(fwrite("WCSLAYER", 8, 1, SaveFile))
                {
				Out32 = 0;
				SizeHolder = ftell(SaveFile);
				fwrite(&Out32, 4, 1, SaveFile); // write dummy size placeholder
                Out32 = NumLayers;
                if(fwrite(&Out32, 4, 1, SaveFile))
                        {
                        Out32 = 0; // reuse as counter
                        for(LayerSkim = FirstEntry(); LayerSkim;
                         LayerSkim = NextEntry(LayerSkim))
                                {
                                Success = 0;
                                if(fwrite(&LayerSkim->Flags, 4, 1, SaveFile))
                                        {
                                        if(fwrite(&LayerSkim->LayerPri, 4, 1, SaveFile))
                                                {
                                                LNameSize = (unsigned char)(strlen(LayerSkim->LayerName.StrData) + 1);
                                                if(fwrite(&LNameSize, 1, 1, SaveFile))
                                                        {
                                                        if(LNameSize)
                                                                {
                                                                if(fwrite(LayerSkim->LayerName.StrData, LNameSize, 1, SaveFile))
                                                                        {
                                                                        Success = 1;
                                                                        } // if
                                                                } // if
                                                        } // if
                                                } // if
                                        } // if
                                if(Success)
                                        {
                                        LayerSkim->LayerNum = Out32++;
                                        } // if
                                else
                                        {
                                        break;
                                        } // else
                                } // for
                        if(Out32 == NumLayers)
                                {
								// Note current pos, calculate how many bytes we wrote,
								// jump way back to rewrite size counter, jump way forward
								// to where we just were.
								BackFuture = ftell(SaveFile);
								Gen32 = (BackFuture - SizeHolder) - 4;
								fseek(SaveFile, SizeHolder, SEEK_SET); // jump way back
								fwrite(&Gen32, 4, 1, SaveFile); // write size
								fseek(SaveFile, BackFuture, SEEK_SET); // jump back to the future
                                return(Gen32);
                                } // if
                        } // if
                } // if
        } // if
return(0);
} // LayerTable::WriteLayerHeader

/*===========================================================================*/

unsigned long LayerEntry::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EDITNAME)
	{
	Flags |= WCS_RAHOST_FLAGBIT_EDITNAME;
	} // if

Mask &= (WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // LayerEntry::GetRAFlags

/*===========================================================================*/

void LayerEntry::GetRAHostProperties(RasterAnimHostProperties *Prop)
{
LayerStub *CurStub;
Joe *MyObj;
long Enabled = 0, Children = 0;
RasterAnimHostProperties JoeProp;

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBITS_ICONTYPE)
		{
		Prop->Flags |= WCS_RAHOST_ICONTYPE_LAYER;
		} // if
	if ((Prop->FlagsMask & WCS_RAHOST_FLAGBIT_CHILDREN) || (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_ENABLED))
		{
		JoeProp.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_TYPENUMBER;
		JoeProp.FlagsMask = WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_ENABLED;
		for (CurStub = FirstStub(); CurStub; CurStub = CurStub->NextObjectInLayer())
			{
			if (MyObj = CurStub->MyObject())
				{
				if (! MyObj->TestFlags(WCS_JOEFLAG_HASKIDS))
					{
					MyObj->GetRAHostProperties(&JoeProp);
					if (! Prop->ChildTypeFilter || JoeProp.TypeNumber == Prop->ChildTypeFilter)
						{
						Children = 1;
						if (JoeProp.Flags & WCS_RAHOST_FLAGBIT_ENABLED)
							{
							Enabled = 1;
							break;	// all we need is one
							} // if
						} // if type match
					} // if
				} // if
			} // for
		if (Children && (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_CHILDREN))
			Prop->Flags |= WCS_RAHOST_FLAGBIT_CHILDREN;
		if (Enabled && (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_ENABLED))
			Prop->Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
		} // if children or enabled flag
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = (char *)GetName();
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
	Prop->Path = "";
	Prop->Ext = "";
	} // if

} // LayerEntry::GetRAHostProperties

/*===========================================================================*/

int LayerEntry::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_ENABLED)
		{
		SetJoeFlags(WCS_JOEFLAG_ACTIVATED, Prop->Flags & WCS_RAHOST_FLAGBIT_ENABLED ? false : true); // operates as either SetJoeFlags or ClearJoeFlags
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	// <<<>>> need to create a function that changes the layer name
	//SetName(Prop->Name);
	Success = 1;
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
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
	return (-2);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_SAVEFILE)
	{
	return (-2);
	} // if

return (Success);

} // LayerEntry::SetRAHostProperties

/*===========================================================================*/

char LayerEntry::GetRAHostDropOK(long DropType)
{

if (DropType == WCS_RAHOST_OBJTYPE_VECTOR
	|| DropType == WCS_RAHOST_OBJTYPE_DEM
	|| DropType == WCS_RAHOST_OBJTYPE_CONTROLPT
	|| DropType == WCS_EFFECTSSUBCLASS_LAKE
	|| DropType == WCS_EFFECTSSUBCLASS_ECOSYSTEM
	|| DropType == WCS_EFFECTSSUBCLASS_WAVE
	|| DropType == WCS_EFFECTSSUBCLASS_CLOUD
	|| DropType == WCS_EFFECTSSUBCLASS_ENVIRONMENT
//	|| DropType == WCS_EFFECTSSUBCLASS_CMAP
	|| DropType == WCS_EFFECTSSUBCLASS_ILLUMINATION
	|| DropType == WCS_EFFECTSSUBCLASS_RASTERTA
	|| DropType == WCS_EFFECTSSUBCLASS_TERRAFFECTOR
	|| DropType == WCS_EFFECTSSUBCLASS_FOLIAGE
	|| DropType == WCS_EFFECTSSUBCLASS_OBJECT3D
	|| DropType == WCS_EFFECTSSUBCLASS_SHADOW
	|| DropType == WCS_EFFECTSSUBCLASS_STREAM
//	|| DropType == WCS_EFFECTSSUBCLASS_TERRAINPARAM
	|| DropType == WCS_EFFECTSSUBCLASS_GROUND
	|| DropType == WCS_EFFECTSSUBCLASS_THEMATICMAP
	|| DropType == WCS_EFFECTSSUBCLASS_COORDSYS
	|| DropType == WCS_EFFECTSSUBCLASS_FENCE
	|| DropType == WCS_EFFECTSSUBCLASS_LABEL
	|| DropType == WCS_EFFECTSSUBCLASS_SNOW)
// <<<>>> ADD_NEW_EFFECTS if the effect can be attached to a vector
	return (1);

return (0);

} // LayerEntry::GetRAHostDropOK

/*===========================================================================*/

RasterAnimHost *LayerEntry::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Found = 0;
LayerStub *CurStub;
Joe *MyObj;
RasterAnimHostProperties JoeProp;

if (! Current)
	Found = 1;

for (CurStub = FirstStub(); CurStub; CurStub = CurStub->NextObjectInLayer())
	{
	if (MyObj = CurStub->MyObject())
		{
		if (! MyObj->TestFlags(WCS_JOEFLAG_HASKIDS))
			{
			MyObj->GetRAHostProperties(&JoeProp);
			if (! ChildTypeFilter || JoeProp.TypeNumber == ChildTypeFilter)
				{
				if (Found)
					return (MyObj);
				} // if type match
			if (Current == MyObj)
				Found = 1;
			} // if
		} // if
	} // for

return (NULL);

} // LayerEntry::GetRAHostChild

/*===========================================================================*/

int LayerEntry::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
int Success = 0, ApplyToAll = 0, Result;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];
LayerStub *CurStub;
Joe *CurJoe;

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_DEM
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_CONTROLPT)
	{
	sprintf(QueryStr, "Add %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		((Joe *)DropSource->DropSource)->AddObjectToLayer(this);
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if
else if (DropSource->TypeNumber >= WCS_EFFECTSSUBCLASS_LAKE && DropSource->TypeNumber < WCS_MAXIMPLEMENTED_EFFECTS)
	{
	sprintf(QueryStr, "Apply %s %s to all members of %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		for (CurStub = FirstStub(); CurStub; CurStub = CurStub->NextObjectInLayer())
			{
			if (CurJoe = CurStub->MyObject())
				{
				if (! CurJoe->TestFlags(WCS_JOEFLAG_HASKIDS))
					{
					if (Result = CurJoe->AddEffect((GeneralEffect *)DropSource->DropSource, ApplyToAll))
						{	// this takes care of notification
						Success = 1;
						if (Result == 2)
							ApplyToAll = 1;
						} // if
					} // if
				} // if
			} // for
		} // if
	} // else if

return (Success);

} // LayerEntry::ProcessRAHostDragDrop

/*===========================================================================*/

int LayerEntry::RemoveRAHost(RasterAnimHost *RemoveMe)
{
int Removed = 0;
NotifyTag Changes[2];
LayerStub *CurStub;

if (CurStub = FirstStub())
	{
	while (CurStub)
		{
		if (RemoveMe == (RasterAnimHost *)CurStub->MyObject())
			{
			if (((Joe *)RemoveMe)->RemoveObjectFromLayer(TRUE, this))
				{
				Removed = 1;
				Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
				break;
				} // if
			} // if
		CurStub = CurStub->NextObjectInLayer();
		} // while
	} // if

return (Removed);

} // LayerEntry::RemoveRAHost

/*===========================================================================*/

void LayerEntry::SetJoeFlags(unsigned long int FlagSet, bool PerformClear)
{
Joe *CurJoe;
LayerStub *CurStub;
char DEMsChanged = 0, ControlPtsChanged = 0, VectorsChanged = 0;
NotifyTag Changes[2];

for (CurStub = FirstStub(); CurStub; CurStub = CurStub->NextObjectInLayer())
	{
	if (CurJoe = CurStub->MyObject())
		{
		if (! CurJoe->TestFlags(WCS_JOEFLAG_HASKIDS))
			{
			if(PerformClear)
				{
				CurJoe->ClearFlags(FlagSet);
				} // if
			else
				{
				CurJoe->SetFlags(FlagSet);
				} // else
			if (CurJoe->TestFlags(WCS_JOEFLAG_ISDEM))
				DEMsChanged = 1;
			else if (CurJoe->TestFlags(WCS_JOEFLAG_ISCONTROL))
				ControlPtsChanged = 1;
			else
				VectorsChanged = 1;
			} // if
		} // if
	} // for

Changes[1] = NULL;
if (DEMsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
if (ControlPtsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
if (VectorsChanged)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if

} // LayerEntry::SetJoeFlags


/*===========================================================================*/


int LayerEntry::AddDerivedPopMenus(PopMenuAdder *PMA, unsigned long int MenuClassFlags)
{

PMA->AddPopMenuItem("Purge Unused Layers", "PURGELAYERS", 0); // handled in S@G, not by our class, therefore not coded as derived
PMA->AddPopMenuItem("Select Layer Members", "SELECTLAYER", 0);
PMA->AddPopMenuItem("Delete Layer", "DELETE", 0); // different phrasing of what appears in S@G, same functionality

return(1);
} // LayerEntry::AddDerivedPopMenus

/*===========================================================================*/
