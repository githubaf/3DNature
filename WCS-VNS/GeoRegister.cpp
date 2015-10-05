// GeoRegister.cpp
// For managing WCS Geographic Registration
// Built from scratch on 04/16/99 by Gary R. Huber
// Copyright 1999 Questar Productions

#include "stdafx.h"
#include "Useful.h"
#include "Application.h"
#include "GeoRegister.h"
#include "EffectsIO.h"
#include "Conservatory.h"
#include "DragnDropListGUI.h"
#include "requester.h"
#include "Toolbar.h"
#include "DBEditGUI.h"
#include "EffectsLib.h"
#include "Render.h"
#include "Database.h"

GeoRegister::GeoRegister(RasterAnimHost *RAHost)
: RasterAnimHost(RAHost)
{
double EffectDefault[WCS_EFFECTS_GEOREGISTER_NUMANIMPAR] = {1.0, 0.0, 1.0, 0.0};
double RangeDefaults[WCS_EFFECTS_GEOREGISTER_NUMANIMPAR][3] = {FLT_MAX, -FLT_MAX, 1.0,
														FLT_MAX, -FLT_MAX, 1.0,
														FLT_MAX, -FLT_MAX, 1.0,
														FLT_MAX, -FLT_MAX, 1.0};
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for

AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);

} // GeoRegister::GeoRegister

/*===========================================================================*/

void GeoRegister::Copy(GeoRegister *CopyTo, GeoRegister *CopyFrom)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].Copy((AnimCritter *)&CopyTo->AnimPar[Ct], (AnimCritter *)&CopyFrom->AnimPar[Ct]);
	} // for
RasterAnimHost::Copy(CopyTo, CopyFrom);

} // GeoRegister::Copy

/*===========================================================================*/

ULONG GeoRegister::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
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
					default:
					  break;
					} // switch

				switch (ItemTag & 0xffff0000)
					{
					case WCS_EFFECTS_GEOREGISTER_NORTH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GEOREGISTER_SOUTH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GEOREGISTER_WEST:
						{
						BytesRead = AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_GEOREGISTER_EAST:
						{
						BytesRead = AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].Load(ffile, Size, ByteFlip);
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

} // GeoRegister::Load

/*===========================================================================*/

unsigned long int GeoRegister::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_EFFECTS_GEOREGISTER_NUMANIMPAR] = {WCS_EFFECTS_GEOREGISTER_NORTH, 
																WCS_EFFECTS_GEOREGISTER_SOUTH,
																WCS_EFFECTS_GEOREGISTER_WEST,
																WCS_EFFECTS_GEOREGISTER_EAST};

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

} // GeoRegister::Save

/*===========================================================================*/

char *GeoRegisterCritterNames[WCS_EFFECTS_GEOREGISTER_NUMANIMPAR] = {"North (deg)", "South (deg)",
															"West (deg)", "East (deg)"};

char *GeoRegister::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (GeoRegisterCritterNames[Ct]);
	} // for
return ("");

} // GeoRegister::GetCritterName

/*===========================================================================*/

int GeoRegister::SetToTime(double Time)
{
long Found = 0, Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for

return (Found);

} // GeoRegister::SetToTime

/*===========================================================================*/

int GeoRegister::GetRAHostAnimated(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for

return (0);

} // GeoRegister::GetRAHostAnimated

/*===========================================================================*/

short GeoRegister::AnimateShadows(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetNumNodes(0) > 1)
		return (1);
	} // for

return (0);

} // GeoRegister::AnimateShadows

/*===========================================================================*/

char GeoRegister::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (GetNumAnimParams() > 0 && DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	return (1);

return (0);

} // GeoRegister::GetRAHostDropOK

/*===========================================================================*/

int GeoRegister::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
RasterAnimHostProperties Prop;
RasterAnimHost *TargetList[30];
long Ct, NumListItems = 0;
NotifyTag Changes[2];
int Success = 0;
char QueryStr[256], NameStr[128], WinNum;

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (GeoRegister *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (GeoRegister *)DropSource->DropSource);
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

if (NumListItems > 0)
	{
	if ((WinNum = GlobalApp->MCP->GetAvailableDragnDropListNumber()) >= 0)
		{
		GlobalApp->GUIWins->DDL[WinNum] = new DragnDropListGUI(WinNum, DropSource, TargetList, NumListItems, NULL, NULL);
		if(GlobalApp->GUIWins->DDL[WinNum])
			{
			GlobalApp->GUIWins->DDL[WinNum]->Open(GlobalApp->MainProj);
			}
		} // if
	else
		UserMessageOK("Open Drag 'n Drop List", "All List windows are in use.\nTry dropping on a target deeper in the hierarchy\n and closer to your desired target.");
	} // if

return (Success);

} // GeoRegister::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long GeoRegister::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_GEOREGISTER | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // GeoRegister::GetRAFlags

/*===========================================================================*/

void GeoRegister::GetRAHostProperties(RasterAnimHostProperties *Prop)
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
if (Prop->PropMask & WCS_RAHOST_MASKBIT_POPCLASS)
	{
	GetPopClassFlags(Prop);
	} // get pop-up menu classes

} // GeoRegister::GetRAHostProperties

/*===========================================================================*/

int GeoRegister::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
NotifyTag Changes[2];
int Success = 0;

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

return (Success);

} // GeoRegister::SetRAHostProperties

/*===========================================================================*/

int GeoRegister::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil)
{
long Ct;

AnimAffil = NULL;

if (ChildA)
	{
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		if (ChildA == GetAnimPtr(Ct))
			{
			AnimAffil = (AnimCritter *)ChildA;
			return (1);
			} // if
		} // for
	} // if

return (0);

} // GeoRegister::GetAffiliates

/*===========================================================================*/

int GeoRegister::GetPopClassFlags(RasterAnimHostProperties *Prop)
{
AnimCritter *AnimAffil = NULL;

Prop->PopClassFlags = 0;
Prop->PopExistsFlags = 0;
Prop->PopEnabledFlags = 0;

if (GetAffiliates(Prop->ChildA, Prop->ChildB, AnimAffil))
	{
	return (RasterAnimHost::GetPopClassFlags(Prop, AnimAffil, NULL, NULL));
	} // if

return (0);

} // GeoRegister::GetPopClassFlags

/*===========================================================================*/

int GeoRegister::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long int MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil))
	{
	return (RasterAnimHost::AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, NULL, NULL));
	} // if

return(0);

} // GeoRegister::AddSRAHBasePopMenus

/*===========================================================================*/

int GeoRegister::HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil))
	{
	return (RasterAnimHost::HandleSRAHPopMenuSelection(Action, AnimAffil, NULL, NULL, NULL, NULL));
	} // if

return(0);

} // GeoRegister::HandleSRAHPopMenuSelection

/*===========================================================================*/

RasterAnimHost *GeoRegister::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
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

return (NULL);

} // GeoRegister::GetRAHostChild

/*===========================================================================*/

RasterAnimHost *GeoRegister::GetNextGroupSibling(RasterAnimHost *FindMyBrother)
{

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH))
	return (GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH))
	return (GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST))
	return (GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST))
	return (GetAnimPtr(WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH));

return (NULL);

} // GeoRegister::GetNextGroupSibling

/*===========================================================================*/

long GeoRegister::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long NumAnimPar, AnimNum, Found = 0;
AnimDoubleTime *AnimPtr;

if (NumAnimPar = GetNumAnimParams())
	{
	for (AnimNum = 0; AnimNum < NumAnimPar; AnimNum ++)
		{
		if (AnimPtr = GetAnimPtr(AnimNum))
			{
			if (AnimPtr->GetMinMaxDist(MinDist, MaxDist))
				{
				if (MinDist < TestFirst)
					TestFirst = MinDist;
				if (MaxDist > TestLast)
					TestLast = MaxDist;
				Found = 1;
				} // if
			} // if
		} // for
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

} // GeoRegister::GetKeyFrameRange

/*===========================================================================*/

int GeoRegister::SnapToDEMBounds(CoordSys *MyCoords)
{
double Nrth, Sth, Wst, Est;
int GotDefaultBounds = 0;

if (GlobalApp->GUIWins->DBE)
	{
	if (UserMessageOKCAN("Bounds: Snap to DEMs", "If desired DEMs are selected in the Database Editor click \"OK\". If not then click \"Cancel\", multi-select the DEMs and click \"Snap to Selected DEMs\" again."))
		{
		if (// a coord sys exists and each of the selected DEMs is in the same coord sys
			(MyCoords && GlobalApp->GUIWins->DBE->MatchSelectedCoordSys(MyCoords, 1) &&
			GlobalApp->GUIWins->DBE->GetSelectedNativeBounds(Nrth, Sth, Wst, Est, 1)) ||
			// or get the bounds in the default CS, whatever that may currently be
			(GotDefaultBounds = GlobalApp->GUIWins->DBE->GetSelectedBounds(Nrth, Sth, Wst, Est, 1)))
			{
			// if a coord sys exists but the bounds were obtained in default coords
			// we need to translate the bounds into the requested coord sys and 
			// take the outsidemost of them
			if (MyCoords && GotDefaultBounds)
				{
				FindOutsideBoundsFromDefDeg(MyCoords, Nrth, Sth, Wst, Est);
				} // if
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(Nrth);
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(Sth);
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(Wst);
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(Est);
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].ValueChanged();
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].ValueChanged();
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].ValueChanged();
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].ValueChanged();
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
			return (1);
			} // if
		else
			{
			UserMessageOK("Bounds: Snap to DEMs", "There are no DEMs selected in the Database Editor!");
			} // else
		} // if
	return (0);
	} // if
else
	{
	UserMessageOK("Bounds: Snap to DEMs", "When the Database Editor opens, select the desired DEMs and then click \"Snap to Selected DEMs\" again.");
	GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		WCS_TOOLBAR_ITEM_DBG, 0);
	return (0);
	} // else

} // GeoRegister::SnapToDEMBounds

/*===========================================================================*/

int GeoRegister::SnapToDBObjs(CoordSys *MyCoords)
{
double Nrth, Sth, Wst, Est;
int GotDefaultBounds = 0;

if (GlobalApp->GUIWins->DBE)
	{
	if (UserMessageOKCAN("Bounds: Snap to Database Objects", "If desired Database Objects are selected in the Database Editor click \"OK\". If not then click \"Cancel\", multi-select the Database Objects and click \"Snap to Selected Database Objects\" again."))
		{
		if (// a coord sys exists and each of the selected DEMs is in the same coord sys
			(MyCoords && GlobalApp->GUIWins->DBE->MatchSelectedCoordSys(MyCoords, 0) &&
			GlobalApp->GUIWins->DBE->GetSelectedNativeBounds(Nrth, Sth, Wst, Est, 0)) ||
			// or get the bounds in the default CS, whatever that may currently be
			(GotDefaultBounds = GlobalApp->GUIWins->DBE->GetSelectedBounds(Nrth, Sth, Wst, Est, 0)))
			{
			// if a coord sys exists but the bounds were obtained in default coords
			// we need to translate the bounds into the requested coord sys and 
			// take the outsidemost of them
			if (MyCoords && GotDefaultBounds)
				{
				FindOutsideBoundsFromDefDeg(MyCoords, Nrth, Sth, Wst, Est);
				} // if
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(Nrth);
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(Sth);
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(Wst);
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(Est);
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].ValueChanged();
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].ValueChanged();
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].ValueChanged();
			AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].ValueChanged();
			return (1);
			} // if
		else
			{
			UserMessageOK("Bounds: Snap to Database Objects", "There are no Database Objects selected in the Database Editor!");
			} // else
		} // if
	return (0);
	} // if
else
	{
	UserMessageOK("Bounds: Snap to Database Objects", "When the Database Editor opens, select the desired Database Objects and then click \"Snap to Selected Database Objects\" again.");
	GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		WCS_TOOLBAR_ITEM_DBG, 0);
	return (0);
	} // else

} // GeoRegister::SnapToDBObjs

/*===========================================================================*/

int GeoRegister::SnapToDatabaseBounds(Database *SnapBase)
{
DEMBounds CurBounds;

if (SnapBase->FillDEMBounds(&CurBounds))
	{
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(CurBounds.North);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(CurBounds.South);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(CurBounds.West);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(CurBounds.East);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].ValueChanged();
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].ValueChanged();
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].ValueChanged();
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].ValueChanged();
	return (1);
	} // if

return (0);

} // GeoRegister::SnapToDatabaseBounds

/*===========================================================================*/

int GeoRegister::SnapToBounds(Database *SnapBase, Project *CurProj, CoordSys *MyCoords, int DEMOnly)
{
double Nrth, Sth, Wst, Est;
int GotDefaultBounds = 0;

if (// a coord sys exists and each of the selected DEMs is in the same coord sys
	(MyCoords && SnapBase->MatchCoordSys(MyCoords, DEMOnly) &&
	SnapBase->GetNativeBounds(Nrth, Sth, Wst, Est, DEMOnly, CurProj)) ||
	// or get the bounds in the default CS, whatever that may currently be
	(GotDefaultBounds = SnapBase->GetBounds(Nrth, Sth, Wst, Est, DEMOnly)))
	{
	// if a coord sys exists but the bounds were obtained in default coords
	// we need to translate the bounds into the requested coord sys and 
	// take the outsidemost of them
	if (MyCoords && GotDefaultBounds)
		{
		FindOutsideBoundsFromDefDeg(MyCoords, Nrth, Sth, Wst, Est);
		} // if
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(Nrth);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(Sth);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(Wst);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(Est);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].ValueChanged();
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].ValueChanged();
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].ValueChanged();
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].ValueChanged();
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	return (1);
	} // if

return (0);

} // GeoRegister::SnapToDEMBounds

/*===========================================================================*/

int GeoRegister::TestForDefaultBounds(void)
{

return (fabs(AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - 1.0) < .000001 &&
	fabs(AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue - 0.0) < .000001 &&
	fabs(AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - 1.0) < .000001 &&
	fabs(AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue - 0.0) < .000001);

} // GeoRegister::TestForDefaultBounds

/*===========================================================================*/

void GeoRegister::ValidateBoundsOrder(CoordSys *MyCoords)
{

if (AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue <
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue)
	{
	swmem(&AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue,
		&AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue, sizeof (double));
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].ValueChanged();
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].ValueChanged();
	} // if
#ifdef WCS_BUILD_VNS
if (MyCoords && (! MyCoords->GetGeographic()))
	{
	if (AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue >
		AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue)
		{
		swmem(&AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue,
			&AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue, sizeof (double));
		AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].ValueChanged();
		AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].ValueChanged();
		} // if
	} // if
else
#endif // WCS_BUILD_VNS
	{
	if (AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue <
		AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue)
		{
		swmem(&AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue,
			&AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue, sizeof (double));
		AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].ValueChanged();
		AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].ValueChanged();
		} // if
	} // else

} // GeoRegister::ValidateBoundsOrder

/*===========================================================================*/

int GeoRegister::TestBoundsOrder(void)
{

if (AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue <
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue)
	{
	return (0);
	} // if
if (AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue <
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue)
	{
	return (0);
	} // if

return (1);

} // GeoRegister::TestBoundsOrder

/*===========================================================================*/

void GeoRegister::ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds)
{
double Ratio;

if (OldBounds->North != OldBounds->South)
	{
	Ratio = (AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - OldBounds->South) / (OldBounds->North - OldBounds->South);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(Ratio * (CurBounds->North - CurBounds->South) + CurBounds->South);
	Ratio = (AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue - OldBounds->North) / (OldBounds->South - OldBounds->North);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(Ratio * (CurBounds->South - CurBounds->North) + CurBounds->North);
	} // if

if (OldBounds->West != OldBounds->East)
	{
	Ratio = (AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - OldBounds->East) / (OldBounds->West - OldBounds->East);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(Ratio * (CurBounds->West - CurBounds->East) + CurBounds->East);
	Ratio = (AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue - OldBounds->West) / (OldBounds->East - OldBounds->West);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(Ratio * (CurBounds->East - CurBounds->West) + CurBounds->West);
	} // if

} // GeoRegister::ScaleToDEMBounds

/*===========================================================================*/

// returns center lat and lon in the coord sys passed, coords are assumed to be in that coord system
int GeoRegister::GetCenterLatLon(CoordSys *MyCoords, double &CenterLat, double &CenterLon)
{
VertexDEM Vert;

ValidateBoundsOrder(MyCoords);

CenterLat = .5 * (AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue + AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue);
CenterLon = .5 * (AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue + AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue);

#ifdef WCS_BUILD_VNS
if (MyCoords && (! MyCoords->GetGeographic()))
	{
	Vert.xyz[0] = CenterLon;
	Vert.xyz[1] = CenterLat;
	if (MyCoords->ProjToDeg(&Vert))
		{
		CenterLon = Vert.Lon;
		CenterLat = Vert.Lat;
		} // if
	else
		return (0);
	} // else
#endif // WCS_BUILD_VNS

return (1);

} // GeoRegister::GetCenterLatLon

/*===========================================================================*/

int GeoRegister::GetMetricWidthHeight(CoordSys *MyCoords, double PlanetRad, double &MetricWidth, double &MetricHeight)
{
double CenterLat, CenterLon;

#ifdef WCS_BUILD_VNS
if (MyCoords && (! MyCoords->GetGeographic()))
	{
	MetricWidth = AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue - AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue;
	MetricHeight = AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
	} // if
else
#endif // WCS_BUILD_VNS
	{
	if (GetCenterLatLon(MyCoords, CenterLat, CenterLon))
		{
		MetricWidth = AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue;
		MetricHeight = AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
		MetricHeight *= LatScale(PlanetRad);
		MetricWidth *= LonScale(PlanetRad, CenterLat);
		} // if
	else
		return (0);
	} // else

return (1);

} // GeoRegister::GetMetricWidthHeight

/*===========================================================================*/

int GeoRegister::SetBoundsFromMetricWidthHeight(CoordSys *MyCoords, double PlanetRad, double MetricWidth, double MetricHeight,
	int FixedNorth, int FixedSouth, int FixedEast, int FixedWest)
{
double CenterLat, CenterLon, half, midpoint;

#ifdef WCS_BUILD_VNS
if (MyCoords && (! MyCoords->GetGeographic()))
	{
	if (FixedEast)
		AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue - MetricWidth);
	else if (FixedWest)
		AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(MetricWidth + AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue);
	else
		{
		half = MetricWidth * 0.5;
		midpoint = (AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue + AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue) * 0.5;
 		AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(midpoint - half);
		AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(midpoint + half);
		} // else

	if (FixedSouth)
		AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(MetricHeight + AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue);
	else if (FixedNorth)
		AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - MetricHeight);
	else
		{
		half = MetricHeight * 0.5;
		midpoint = (AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue + AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue) * 0.5;
 		AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(midpoint + half);
		AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(midpoint - half);
		} // else
	} // if
else
#endif // WCS_BUILD_VNS
	{
	double MWidth, MHeight;
	// iteration required to close in on the right lat/lon center to use
	for (int Ct = 0; Ct < 3; Ct ++)
		{
		if (GetCenterLatLon(MyCoords, CenterLat, CenterLon))
			{
			MHeight = MetricHeight / LatScale(PlanetRad);
			MWidth = MetricWidth / LonScale(PlanetRad, CenterLat);

			if (FixedEast)
				AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(MWidth + AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue);
			else if (FixedWest)
				AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - MWidth);
			else
				{
				half = MWidth * 0.5;
				midpoint = (AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue + AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue) * 0.5;
				AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(midpoint - half);
				AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(midpoint + half);
				} // else

			if (FixedSouth)
				AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(MHeight + AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue);
			else if (FixedNorth)
				AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - MHeight);
			else
				{
				half = MHeight * 0.5;
				midpoint = (AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue + AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue) * 0.5;
				AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(midpoint - half);
				AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(midpoint + half);
				} // else
			} // if
		else
			return (0);
		} // for
	} // else

return (1);

} // GeoRegister::SetBoundsFromMetricWidthHeight

/*===========================================================================*/

int GeoRegister::SetBoundsFromGeographicWidthHeight(double GeographicWidth, double GeographicHeight,
	int FixedNorth, int FixedSouth, int FixedEast, int FixedWest)
{
double midpoint;

if (FixedEast)
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(GeographicWidth + AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue);
else if (FixedWest)
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - GeographicWidth);
else
	{
	midpoint = (AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue + AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue) * 0.5;
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(midpoint + (GeographicWidth * 0.5));
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(midpoint - (GeographicWidth * 0.5));
	} // else

if (FixedSouth)
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(GeographicHeight + AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue);
else if (FixedNorth)
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - GeographicHeight);
else
	{
	midpoint = (AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue + AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue) * 0.5;
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(midpoint + (GeographicHeight * 0.5));
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(midpoint - (GeographicHeight * 0.5));
	} // else

return (1);

} // GeoRegister::SetBoundsFromGeographicWidthHeight

/*===========================================================================*/

void GeoRegister::SetBounds(CoordSys *MyCoords, double LatRange[2], double LonRange[2])
{
VertexDEM Vert;

// can't use the bounds if they are equal
if (LatRange[0] != LatRange[1] && LonRange[0] != LonRange[1])
	{
	if (MyCoords)
		{
		Vert.Lat = LatRange[0];
		Vert.Lon = LonRange[0];
		MyCoords->DefDegToProj(&Vert);
		LatRange[0] = Vert.xyz[1];
		LonRange[0] = Vert.xyz[0];
		Vert.Lat = LatRange[1];
		Vert.Lon = LonRange[1];
		MyCoords->DefDegToProj(&Vert);
		LatRange[1] = Vert.xyz[1];
		LonRange[1] = Vert.xyz[0];
		if (! MyCoords->Initialized)
			MyCoords->Initialize();
		} // if
	if (LatRange[1] < LatRange[0])
		swmem(&LatRange[0], &LatRange[1], sizeof (double));
	if (MyCoords && (! MyCoords->GetGeographic()))
		{
		if (LonRange[1] > LonRange[0])
			swmem(&LonRange[0], &LonRange[1], sizeof (double));
		} // if
	else
		{
		if (LonRange[1] < LonRange[0])
			swmem(&LonRange[0], &LonRange[1], sizeof (double));
		// if bounds appear to wrap more than halfway around earth then probably 
		// want to take the smaller arc
		if (fabs(LonRange[1] - LonRange[0]) > 180.0)
			{
			LonRange[1] -= 360.0;
			} // if

		if (LonRange[1] < LonRange[0])
			swmem(&LonRange[0], &LonRange[1], sizeof (double));
		} // else
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(LatRange[1]);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(LatRange[0]);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(LonRange[1]);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(LonRange[0]);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].ValueChanged();
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].ValueChanged();
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].ValueChanged();
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].ValueChanged();
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
	} // if

} // GeoRegister::SetBounds

/*===========================================================================*/

void GeoRegister::ShiftBounds(CoordSys *MyCoords, double LatShift[2], double LonShift[2]) // shift georeferencing so point 0 relocates to point 1
{
VertexDEM VertA, VertB;
double DeltaX, DeltaY;

// Add the X Delta and YDelta to each of the respective X and Y coords of the
// georeferencing, calculated in the native CS of the georeferenced image

if (MyCoords)
	{
	VertA.Lat = LatShift[0];
	VertA.Lon = LonShift[0];
	MyCoords->DefDegToProj(&VertA);
	VertB.Lat = LatShift[1];
	VertB.Lon = LonShift[1];
	MyCoords->DefDegToProj(&VertB);
	DeltaX = VertB.xyz[0] - VertA.xyz[0];
	DeltaY = VertB.xyz[1] - VertA.xyz[1];
	} // if
else
	{
	DeltaX = LonShift[1] - LonShift[0];
	DeltaY = LatShift[1] - LatShift[0];
	} // else

AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].GetValue(0,0.0) + DeltaY);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].GetValue(0,0.0) + DeltaY);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].GetValue(0,0.0) + DeltaX);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].GetValue(0,0.0) + DeltaX);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].ValueChanged();
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].ValueChanged();
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].ValueChanged();
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].ValueChanged();
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

} // GeoRegister::ShiftBounds

/*===========================================================================*/

void GeoRegister::SetMetricType(CoordSys *MyCoords)
{

if ((! MyCoords) || MyCoords->GetGeographic())
	{
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
	} // if
else
	{
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	} // else

} // GeoRegister::SetMetricType

/*===========================================================================*/

void GeoRegister::FindOutsideBoundsFromDefDeg(CoordSys *MyCoords, double &Nrth, double &Sth, double &Wst, double &Est)
{
double TestNorth, TestSouth, TestEast, TestWest;
VertexDEM Vert;

if (MyCoords)
	{
	TestNorth = -FLT_MAX;
	TestSouth = FLT_MAX;

	// northwest
	Vert.Lat = Nrth;
	Vert.Lon = Wst;
	MyCoords->DefDegToProj(&Vert);
	if (Vert.xyz[1] > TestNorth)
		TestNorth = Vert.xyz[1];
	if (Vert.xyz[1] < TestSouth)
		TestSouth = Vert.xyz[1];
	if (MyCoords->GetGeographic())
		{
		TestWest = -FLT_MAX;
		TestEast = FLT_MAX;
		if (Vert.xyz[0] > TestWest)
			TestWest = Vert.xyz[0];
		if (Vert.xyz[0] < TestEast)
			TestEast = Vert.xyz[0];
		} // if
	else
		{
		TestWest = FLT_MAX;
		TestEast = -FLT_MAX;
		if (Vert.xyz[0] < TestWest)
			TestWest = Vert.xyz[0];
		if (Vert.xyz[0] > TestEast)
			TestEast = Vert.xyz[0];
		} // else

	// northeast
	Vert.Lat = Nrth;
	Vert.Lon = Est;
	MyCoords->DefDegToProj(&Vert);
	if (Vert.xyz[1] > TestNorth)
		TestNorth = Vert.xyz[1];
	if (Vert.xyz[1] < TestSouth)
		TestSouth = Vert.xyz[1];
	if (MyCoords->GetGeographic())
		{
		if (Vert.xyz[0] > TestWest)
			TestWest = Vert.xyz[0];
		if (Vert.xyz[0] < TestEast)
			TestEast = Vert.xyz[0];
		} // if
	else
		{
		if (Vert.xyz[0] < TestWest)
			TestWest = Vert.xyz[0];
		if (Vert.xyz[0] > TestEast)
			TestEast = Vert.xyz[0];
		} // else

	// southwest
	Vert.Lat = Sth;
	Vert.Lon = Wst;
	MyCoords->DefDegToProj(&Vert);
	if (Vert.xyz[1] > TestNorth)
		TestNorth = Vert.xyz[1];
	if (Vert.xyz[1] < TestSouth)
		TestSouth = Vert.xyz[1];
	if (MyCoords->GetGeographic())
		{
		if (Vert.xyz[0] > TestWest)
			TestWest = Vert.xyz[0];
		if (Vert.xyz[0] < TestEast)
			TestEast = Vert.xyz[0];
		} // if
	else
		{
		if (Vert.xyz[0] < TestWest)
			TestWest = Vert.xyz[0];
		if (Vert.xyz[0] > TestEast)
			TestEast = Vert.xyz[0];
		} // else

	// southeast
	Vert.Lat = Sth;
	Vert.Lon = Est;
	MyCoords->DefDegToProj(&Vert);
	if (Vert.xyz[1] > TestNorth)
		TestNorth = Vert.xyz[1];
	if (Vert.xyz[1] < TestSouth)
		TestSouth = Vert.xyz[1];
	if (MyCoords->GetGeographic())
		{
		if (Vert.xyz[0] > TestWest)
			TestWest = Vert.xyz[0];
		if (Vert.xyz[0] < TestEast)
			TestEast = Vert.xyz[0];
		} // if
	else
		{
		if (Vert.xyz[0] < TestWest)
			TestWest = Vert.xyz[0];
		if (Vert.xyz[0] > TestEast)
			TestEast = Vert.xyz[0];
		} // else
	
	Nrth = TestNorth;
	Sth = TestSouth;
	Wst = TestWest;
	Est = TestEast;
	} // if

} // GeoRegister::FindOutsideBoundsFromDefDeg

/*===========================================================================*/

int GeoRegister::GeographicPointContained(CoordSys *MyCoords, CoordSys *PointCoords, double PointLat, double PointLon)
{
VertexDEM Vert;

if (MyCoords && ! MyCoords->Initialized)
	MyCoords->Initialize();

if (MyCoords && PointCoords)
	{
	Vert.Lat = PointLat;
	Vert.Lon = PointLon;
	if (PointCoords->DegToCart(&Vert) && MyCoords->CartToProj(&Vert))
		{
		PointLat = Vert.xyz[1];
		PointLon = Vert.xyz[0];
		} // if
	else
		return (0);
	} // if
if (PointLat >= AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue && PointLat <= AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue)
	{
	if ((! MyCoords) || MyCoords->GetGeographic())
		{
		if (PointLon >= AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue && PointLon <= AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue)
			return (1);
		} // if
	else
		{
		if (PointLon >= AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue && PointLon <= AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue)
			return (1);
		} // else
	} // if

return (0);

} // GeoRegister::GeographicPointContained
